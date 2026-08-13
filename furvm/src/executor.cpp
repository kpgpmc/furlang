#include "furvm/executor.hpp"

#include "furlang/view.hpp"
#include "furvm/context.hpp" // IWYU pragma: keep
#include "furvm/exceptions.hpp"
#include "furvm/function.hpp" // IWYU pragma: keep
#include "furvm/fwd.hpp"
#include "furvm/instruction.hpp"
#include "furvm/thing.hpp"

#include <cassert>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>
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
    case thing_type::Ptr: return { thing_type::Ptr, mod_to_thing_type(mod, *mod->type_at(type.value.typeRef)) };
    case thing_type::Array: {
        return { static_cast<enum thing_type::type>(type.type),
            { mod_to_thing_type(mod, *mod->type_at(type.value.array.typeId)), type.value.array.size } };
    }
    default: throw std::runtime_error("invalid thing type");
    }
}

thing_type* executor::mod_to_thing_type(const mod_h& mod, const mod_type& type) const {
    struct thing_type thingType = thing_type_impl(mod, type);
    return m_context->tt_store().insert(thingType);
}

thing<> executor::make_reference(const thing<>& thing) const {
    furvm::thing<> ref = { (struct thing_type){ thing_type::Ref, m_context->tt_store().insert(thing.type()) },
        m_context->thing_alloc() };
    ref.reference(thing);
    return std::move(ref);
}

void executor::push_frame(const mod_h& mod, function function) {
    mod_h modInst = mod;
    while (function.type() == function_t::Import) {
        modInst  = m_context->at(function.imp().mod);
        function = *modInst->function_at(function.imp().function);
    }

    auto                 signature = function.signature();
    std::vector<thing<>> args;
    args.reserve(signature.params.size());
    for (const auto& param : signature.params) {
        auto arg = pop_thing();
        if (arg.true_type() != *mod_to_thing_type(mod, *param))
            throw std::runtime_error("function argument type mismatch");
        args.push_back(std::move(arg));
    }

    struct thing_type* returnType = nullptr;
    if (function.signature().returnType.has_value())
        returnType = mod_to_thing_type(mod, *function.signature().returnType.value()); // NOLINT

    switch (function.type()) {
    case function_t::Normal: {
        m_frames.emplace(
            (struct executor::frame){ mod, function.position(), m_stack.size(), returnType, std::move(args) });
    } break;
    case function_t::Native: {
        m_frames.emplace((struct executor::frame){ mod, 0, m_stack.size(), returnType, std::move(args) });
        modInst->get_native_function(function.native())(*this);
        pop_frame();
        return;
    }
    default: throw std::runtime_error("unexpected function type");
    }
    if (m_newFrameCb) m_newFrameCb(*this);
}

struct executor::frame executor::pop_frame() {
    if (m_frames.empty()) throw stack_underflow();
    struct executor::frame frame = m_frames.top();
    m_frames.pop();
    std::optional<thing<>> returnValue;
    if (frame.returnType != nullptr) {
        returnValue = pop_thing();
        if (returnValue->type() != *frame.returnType) throw std::runtime_error("function return type mismatch");
    }
    if (m_stack.size() != frame.stackBase) throw std::runtime_error("unexhausted stack");
    if (returnValue.has_value()) push_thing(std::move(returnValue.value()));
    return frame;
}

struct executor::frame executor::top_frame() const {
    return m_frames.top();
}

thing<>& executor::push_thing(thing<>&& thing) {
    return m_stack.emplace(std::move(thing));
}

thing<>& executor::push_thing(const thing<>& thing) {
    return m_stack.emplace(thing);
}

thing<> executor::pop_thing() {
    if (m_frames.top().stackBase >= m_stack.size()) throw stack_underflow();
    auto top = std::move(m_stack.top());
    m_stack.pop();
    return std::move(top);
}

thing<>& executor::top_thing() {
    if (m_frames.top().stackBase >= m_stack.size()) throw stack_underflow();
    return m_stack.top();
}

const thing<>& executor::top_thing() const {
    if (m_frames.top().stackBase >= m_stack.size()) throw stack_underflow();
    return m_stack.top();
}

void executor::store_thing(variable_t variable, const thing<>& thing) {
    auto& frame = m_frames.top();
    if (frame.variables.size() <= variable) frame.variables.resize(variable + 1);
    frame.variables[variable] = thing;
}

void executor::store_thing(variable_t variable, thing<>&& thing) {
    auto& frame = m_frames.top();
    if (frame.variables.size() <= variable) frame.variables.resize(variable + 1);
    frame.variables[variable] = std::move(thing);
}

thing<>& executor::load_thing(variable_t variable) {
    auto& frame = m_frames.top();
    return frame.variables[variable];
}

const thing<>& executor::load_thing(variable_t variable) const {
    const auto& frame = m_frames.top();
    return frame.variables[variable];
}

void executor::step() {
    if ((m_flags & executor_flags::Suspended) == executor_flags::Suspended) return;

    struct frame& frame = m_frames.top();

    instruction instr{};
    frame.position += instr.read(frame.mod->bytecode_view().subview(frame.position));
    switch (instr.type) {
    case instruction_t::NoOperation: break;
    case instruction_t::PushS8: {
        push_thing({ (struct thing_type){ thing_type::S8 }, m_context->thing_alloc() }).get<thing_type::s8>() =
            instr.arg.s8;
    } break;
    case instruction_t::PushU8: {
        push_thing({ (struct thing_type){ thing_type::U8 }, m_context->thing_alloc() }).get<thing_type::u8>() =
            instr.arg.u8;
    } break;
    case instruction_t::PushS16: {
        push_thing({ (struct thing_type){ thing_type::S16 }, m_context->thing_alloc() }).get<thing_type::s16>() =
            instr.arg.s16;
    } break;
    case instruction_t::PushU16: {
        push_thing({ (struct thing_type){ thing_type::U16 }, m_context->thing_alloc() }).get<thing_type::u16>() =
            instr.arg.u16;
    } break;
    case instruction_t::PushS32: {
        push_thing({ (struct thing_type){ thing_type::S32 }, m_context->thing_alloc() }).get<thing_type::s32>() =
            instr.arg.s8; // NOLINT
    } break;
    case instruction_t::PushU32: {
        push_thing({ (struct thing_type){ thing_type::U32 }, m_context->thing_alloc() }).get<thing_type::u32>() =
            static_cast<thing_type::u32>(instr.arg.u8);
    } break;
    case instruction_t::Array: {
        const auto& type = *mod_to_thing_type(frame.mod, *frame.mod->type_at(instr.arg.u32));
        if (type.type != thing_type::Array || type.value.array.type == nullptr || type.value.array.type == &type)
            throw std::runtime_error("invalid array type");

        auto& array = push_thing({ type, m_context->thing_alloc() });

        if (type.value.array.size == 0) {
            auto         sizeThing = pop_thing();
            std::int64_t size      = sizeThing.integer();
            array.resize(size);
        }
    } break;
    case instruction_t::Get: {
        auto index = pop_thing();
        auto array = pop_thing();
        push_thing(array.at(index.integer()));
    } break;
    case instruction_t::Set: {
        auto element = pop_thing();
        auto index   = pop_thing();
        auto array   = pop_thing();
        array.at(index.integer()).assign(std::move(element));
    } break;
    case instruction_t::Drop: {
        pop_thing();
    } break;
    case instruction_t::Duplicate: {
        push_thing(top_thing());
    } break;
    case instruction_t::Swap: {
        auto thing1 = pop_thing();
        auto thing2 = pop_thing();
        push_thing(std::move(thing1));
        push_thing(std::move(thing2));
    } break;
    case instruction_t::Clone: {
        push_thing(top_thing());
    } break;
    case instruction_t::Reference: {
        push_thing(std::move(make_reference(pop_thing())));
    } break;
    case instruction_t::Add: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs.add(rhs));
    } break;
    case instruction_t::Sub: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs.sub(rhs));
    } break;
    case instruction_t::Mul: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs.mul(rhs));
    } break;
    case instruction_t::Div: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs.div(rhs));
    } break;
    case instruction_t::Mod: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs.mod(rhs));
    } break;
    case instruction_t::Equals: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs.equals(rhs));
    } break;
    case instruction_t::NotEquals: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs.not_equals(rhs));
    } break;
    case instruction_t::LessThan: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs.less_than(rhs));
    } break;
    case instruction_t::GreaterThan: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs.greater_than(rhs));
    } break;
    case instruction_t::LessEqual: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs.less_equals(rhs));
    } break;
    case instruction_t::GreaterEqual: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs.greater_equals(rhs));
    } break;
    case instruction_t::Pointerof: {
        auto thing = pop_thing();
        push_thing({ (struct thing_type){ thing_type::Ptr, m_context->tt_store().at(thing.type().id) },
                       m_context->thing_alloc() })
            .get<void*>() = thing.raw();
    } break;
    case instruction_t::Sizeof: {
        auto  thing = pop_thing();
        auto& size  = push_thing({ (struct thing_type){ thing_type::U64 }, m_context->thing_alloc() });
        switch (thing.type().type) {
        case thing_type::S8:
        case thing_type::S16:
        case thing_type::S32:
        case thing_type::S64:
        case thing_type::U8:
        case thing_type::U16:
        case thing_type::U32:
        case thing_type::U64:
            size.get<thing_type::u64>() = static_cast<thing_type::u64>(thing_type::primitive_size(thing.type().type));
            break;
        case thing_type::Ptr: size.get<thing_type::u64>() = static_cast<thing_type::u64>(sizeof(void*)); break;
        case thing_type::Array:
            /* TODO: Return actual memory size of the array
             * By the memory size I mean the length times sizeof single element.
             */
            size.get<thing_type::u64>() = thing.length();
            break;
        default: throw std::runtime_error("unreachable");
        }
    } break;
    case instruction_t::Lengthof: {
        auto thing = pop_thing();
        push_thing({ (struct thing_type){ thing_type::U64 }, m_context->thing_alloc() }).get<thing_type::u64>() =
            thing.length();
    } break;
    case instruction_t::Load: {
        push_thing(make_reference(load_thing(instr.arg.u16)));
    } break;
    case instruction_t::Store: {
        store_thing(instr.arg.u16, std::move(pop_thing()));
    } break;
    case instruction_t::Call: {
        push_frame(frame.mod, *frame.mod->function_at(instr.arg.u16));
    } break;
    case instruction_t::Jump: {
        frame.position += instr.arg.s8;
    } break;
    case instruction_t::JumpNotZero: {
        if (pop_thing().integer() != 0) frame.position += instr.arg.s8;
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
