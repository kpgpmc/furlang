#include "furvm/executor.hpp"

#include "furvm/context.hpp" // IWYU pragma: keep
#include "furvm/exceptions.hpp"
#include "furvm/function.hpp" // IWYU pragma: keep
#include "furvm/fwd.hpp"
#include "furvm/instruction.hpp"
#include "furvm/thing.hpp"

#include <cstdint>
#include <stdexcept>

namespace furvm {

void executor::push_frame(const mod_h& mod, function function) {
    while (function.type() == function_t::Import) {
        function = *m_context->module_at(function.imp().mod)->function_at(function.imp().function);
    }
    switch (function.type()) {
    case function_t::Normal: {
        m_frames.emplace((struct executor::frame){ mod, function.position(), m_stack.size() });
    } break;
    case function_t::Native:
    default: throw std::runtime_error("unexpected function type");
    }
}

struct executor::frame executor::pop_frame() {
    if (m_frames.empty()) throw stack_underflow();
    struct executor::frame frame = m_frames.top();
    m_frames.pop();
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
    frame.variables.resize(variable + 1);
    frame.variables[variable] = thing;
}

void executor::store_thing(variable_t variable, thing_h&& thing) {
    auto& frame = m_frames.top();
    frame.variables.resize(variable + 1);
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
        push_thing({ thing_t::Int32, m_context->thing_alloc() })->int32() = frame.mod->byte(frame.position++);
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

        const function_h& function = frame.mod->function_at(funcId);
        switch (function->type()) {
        case function_t::Normal:
        case function_t::Import: push_frame(frame.mod, *function); break;
        case function_t::Native: function->native()(*this); break;
        }
    } break;
    case instruction_t::Jump: {
        frame.position += frame.mod->byte(frame.position++);
    } break;
    case instruction_t::JumpNotZero: {
        byte offset = frame.mod->byte(frame.position++);
        auto cond   = pop_thing();
        if (cond->int32() != 0) frame.position += offset;
    } break;
    case instruction_t::Return: {
        pop_frame();
        if (m_frames.empty()) m_flags = m_flags | executor_flags::Done;
    } break;
    case instruction_t::ReturnValue: {
        auto value = pop_thing();
        pop_frame();
        push_thing(std::move(value));
    } break;
    case instruction_t::PushConstant: throw std::runtime_error("unimplemented");
    default: throw std::runtime_error("unknown instruction");
    }
}

} // namespace furvm
