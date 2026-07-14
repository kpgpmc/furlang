#include "furvm/executor.hpp"

#include "furvm/context.hpp" // IWYU pragma: keep
#include "furvm/exceptions.hpp"
#include "furvm/function.hpp" // IWYU pragma: keep
#include "furvm/fwd.hpp"
#include "furvm/instruction.hpp"
#include "furvm/thing.hpp"

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace furvm {

thing_type executor::thing_type_impl(mod_h mod, mod_type type) const {
    while (type.type == mod_type::Import) {
        auto imprt = std::move(type.value.imprt);
        mod        = m_context->at(imprt.modId);
        type       = *mod->type_at(imprt.typeId);
    }
    switch (static_cast<enum thing_type::type>(type.type)) {
    case thing_type::S8:
    case thing_type::S16:
    case thing_type::S32:
    case thing_type::S64:
    case thing_type::U8:
    case thing_type::U16:
    case thing_type::U32:
    case thing_type::U64: return { static_cast<enum thing_type::type>(type.type) };
    case thing_type::Ptr: return { thing_type::Ptr, thing_type(mod, *mod->type_at(type.value.typeRef)) };
    case thing_type::Array: {
        return { static_cast<enum thing_type::type>(type.type),
            { thing_type(mod, *mod->type_at(type.value.array.typeId)), type.value.array.size } };
    }
    default: throw std::runtime_error("invalid thing type");
    }
}

thing_type* executor::thing_type(const mod_h& mod, const mod_type& type) const {
    struct thing_type thingType = thing_type_impl(mod, type);
    return m_context->thing_type_store().insert(thingType);
}

void executor::push_frame(const mod_h& mod, function function) {
    mod_h modInst = mod;
    while (function.type() == function_t::Import) {
        modInst  = m_context->at(function.imp().mod);
        function = *modInst->function_at(function.imp().function);
    }

    auto                 signature = function.signature();
    std::vector<thing_h> args;
    args.reserve(signature.params.size());
    for (const auto& param : signature.params) {
        auto arg = pop_thing();
        if (arg->type() != *thing_type(mod, *param)) throw std::runtime_error("function argument type mismatch");
        args.push_back(std::move(arg));
    }

    struct thing_type* returnType = nullptr;
    if (function.signature().returnType.has_value())
        returnType = thing_type(mod, *function.signature().returnType.value()); // NOLINT

    switch (function.type()) {
    case function_t::Normal: {
        m_frames.emplace(
            (struct executor::frame){ mod, function.position(), m_stack.size(), returnType, std::move(args) });
    } break;
    case function_t::Native: {
        m_frames.emplace((struct executor::frame){ mod, 0, m_stack.size(), returnType, std::move(args) });
        modInst->get_native_function(function.native())(*this);
        pop_frame();
    } break;
    default: throw std::runtime_error("unexpected function type");
    }
}

struct executor::frame executor::pop_frame() {
    if (m_frames.empty()) throw stack_underflow();
    struct executor::frame frame = m_frames.top();
    m_frames.pop();
    thing_h returnValue;
    if (frame.returnType != nullptr) returnValue = pop_thing();
    if (m_stack.size() != frame.stackBase) throw std::runtime_error("unexhausted stack");
    if (frame.returnType != nullptr) push_thing(std::move(returnValue));
    return frame;
}

struct executor::frame executor::frame() const {
    return m_frames.top();
}

thing_h executor::push_thing(::furvm::thing<>&& thing) {
    return m_stack.emplace(m_context->emplace_thing(std::move(thing)));
}

thing_h executor::pop_thing() {
    if (m_frames.top().stackBase >= m_stack.size()) throw stack_underflow();
    thing_h top = std::move(m_stack.top());
    m_stack.pop();
    return top;
}

thing_h executor::thing() const {
    if (m_frames.top().stackBase >= m_stack.size()) throw stack_underflow();
    return m_stack.top();
}

void executor::store_thing(variable_t variable, const thing_h& thing) {
    auto& frame = m_frames.top();
    if (frame.variables.size() <= variable) frame.variables.resize(variable + 1);
    frame.variables[variable] = thing;
}

void executor::store_thing(variable_t variable, thing_h&& thing) {
    auto& frame = m_frames.top();
    if (frame.variables.size() <= variable) frame.variables.resize(variable + 1);
    frame.variables[variable] = std::move(thing);
}

thing_h executor::load_thing(variable_t variable) const {
    const auto& frame = m_frames.top();
    return frame.variables[variable];
}

void executor::step() {
    if ((m_flags & executor_flags::Suspended) == executor_flags::Suspended) return;

    struct frame& frame = m_frames.top();

    instruction_t instr = static_cast<instruction_t>((*frame.mod).byte(frame.position++));
    switch (instr) {
    case instruction_t::NoOperation: break;
    case instruction_t::PushB2I: {
        push_thing({ (struct thing_type){ thing_type::S32 }, m_context->thing_alloc() })->get<int>() =
            frame.mod->byte(frame.position++);
    } break;
    case instruction_t::Array: {
        mod_type_id typeId = static_cast<mod_type_id>(frame.mod->byte(frame.position)) |
                             (static_cast<mod_type_id>(frame.mod->byte(frame.position + 1)) << 8) |
                             (static_cast<mod_type_id>(frame.mod->byte(frame.position + 2)) << 16) |
                             (static_cast<mod_type_id>(frame.mod->byte(frame.position + 3)) << 24);
        frame.position    += 4;

        const auto& type = *thing_type(frame.mod, *frame.mod->type_at(typeId));
        if (type.type != thing_type::Array || type.value.array.type == nullptr || type.value.array.type == &type)
            throw std::runtime_error("invalid array type");

        auto array = push_thing({ type, m_context->thing_alloc() });

        if (type.value.array.size == 0) {
            auto         sizeThing = pop_thing();
            std::int64_t size      = sizeThing->integer();
            array->resize(size);
        }
    } break;
    case instruction_t::Get: {
        auto index = pop_thing();
        auto array = pop_thing();
        push_thing(array->at(index->integer()));
    } break;
    case instruction_t::Set: {
        auto index                  = pop_thing();
        auto array                  = pop_thing();
        auto element                = pop_thing();
        array->at(index->integer()) = std::move(element->clone());
    } break;
    case instruction_t::Drop: {
        pop_thing();
    } break;
    case instruction_t::Duplicate: {
        push_thing(thing());
    } break;
    case instruction_t::Clone: {
        push_thing(std::move(thing()->clone()));
    } break;
    case instruction_t::Reference: {
        auto thing = pop_thing();
        push_thing({ (struct thing_type){ thing_type::Ref, m_context->thing_type_store().insert(thing->type()) },
                       m_context->thing_alloc() })
            ->reference(*thing);
    } break;
    case instruction_t::Add: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs->add(*rhs));
    } break;
    case instruction_t::Sub: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs->sub(*rhs));
    } break;
    case instruction_t::Mul: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs->mul(*rhs));
    } break;
    case instruction_t::Div: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs->div(*rhs));
    } break;
    case instruction_t::Mod: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs->mod(*rhs));
    } break;
    case instruction_t::Equals: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs->equals(*rhs));
    } break;
    case instruction_t::NotEquals: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs->not_equals(*rhs));
    } break;
    case instruction_t::LessThan: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs->less_than(*rhs));
    } break;
    case instruction_t::GreaterThan: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs->greater_than(*rhs));
    } break;
    case instruction_t::LessEqual: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs->less_equals(*rhs));
    } break;
    case instruction_t::GreaterEqual: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs->greater_equals(*rhs));
    } break;
    case instruction_t::Pointerof: {
        auto thing = pop_thing();
        auto ptr =
            push_thing({ (struct thing_type){ thing_type::Ptr, m_context->thing_type_store().at(thing->type().id) },
                m_context->thing_alloc() });
        ptr->get<void*>() = thing->raw();
    } break;
    case instruction_t::Sizeof: {
        auto thing = pop_thing();
        auto size  = push_thing({ (struct thing_type){ thing_type::U64 }, m_context->thing_alloc() });
        switch (thing->type().type) {
        case thing_type::S8:
        case thing_type::S16:
        case thing_type::S32:
        case thing_type::S64:
        case thing_type::U8:
        case thing_type::U16:
        case thing_type::U32:
        case thing_type::U64:
            size->get<thing_type::u64>() = static_cast<thing_type::u64>(thing_type::primitive_size(thing->type().type));
            break;
        case thing_type::Ptr: size->get<thing_type::u64>() = static_cast<thing_type::u64>(sizeof(void*)); break;
        case thing_type::Array:
            /* TODO: Return actual memory size of the array
             * By the memory size I mean the length times sizeof single element.
             */
            size->get<thing_type::u64>() = thing->length();
            break;
        default: throw std::runtime_error("unreachable");
        }
    } break;
    case instruction_t::Lengthof: {
        auto thing  = pop_thing();
        auto length = push_thing({ (struct thing_type){ thing_type::U64 }, m_context->thing_alloc() });
        length->get<thing_type::u64>() = thing->length();
    } break;
    case instruction_t::Load: {
        variable_t variable = static_cast<std::uint16_t>(frame.mod->byte(frame.position)) |
                              (static_cast<std::uint16_t>(frame.mod->byte(frame.position + 1)) << 8);
        frame.position     += 2;
        push_thing(load_thing(variable));
    } break;
    case instruction_t::Store: {
        variable_t variable = static_cast<std::uint16_t>(frame.mod->byte(frame.position)) |
                              (static_cast<std::uint16_t>(frame.mod->byte(frame.position + 1)) << 8);
        frame.position     += 2;
        store_thing(variable, std::move(pop_thing()));
    } break;
    case instruction_t::Call: {
        function_id funcId = static_cast<std::uint16_t>(frame.mod->byte(frame.position)) |
                             (static_cast<std::uint16_t>(frame.mod->byte(frame.position + 1)) << 8);
        frame.position    += 2;
        push_frame(frame.mod, *frame.mod->function_at(funcId));
    } break;
    case instruction_t::Jump: {
        frame.position += ((std::int8_t)frame.mod->byte(frame.position)) + 1;
    } break;
    case instruction_t::JumpNotZero: {
        byte offset = frame.mod->byte(frame.position++);
        auto cond   = pop_thing();
        if (cond->integer() != 0) frame.position += (std::int8_t)offset;
    } break;
    case instruction_t::Return: {
        pop_frame();
        if (m_frames.empty()) m_flags = m_flags | executor_flags::Done;
    } break;
    case instruction_t::PushConstant: throw std::runtime_error("unimplemented");
    default: throw std::runtime_error("unknown instruction");
    }
}

} // namespace furvm
