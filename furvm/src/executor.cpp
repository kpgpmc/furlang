#include "furvm/executor.hpp"

#include "furvm/context.hpp" // IWYU pragma: keep
#include "furvm/exceptions.hpp"
#include "furvm/function.hpp" // IWYU pragma: keep
#include "furvm/fwd.hpp"
#include "furvm/instruction.hpp"
#include "furvm/thing.hpp"

#include <cstdint>
#include <memory>
#include <stdexcept>

namespace furvm {

void executor::push_frame(const mod_p& mod, function_id function) {
    const auto& func = mod->function_at(function);
    if (func->type() != function_t::Normal) throw std::runtime_error("unexpected function type");
    m_frames.emplace((struct executor::frame){ mod, func->position(), m_stack.size() });
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

void executor::push_thing(const thing_p& thing) {
    thing->add_reference();
    m_stack.push(thing);
}

void executor::push_thing(thing_p&& thing) {
    thing->add_reference();
    m_stack.push(std::move(thing));
}

thing_p executor::pop_thing() {
    if (m_frames.top().stackBase >= m_stack.size()) throw stack_underflow();
    thing_p top = std::move(m_stack.top());
    m_stack.pop();
    top->remove_reference();
    return top;
}

thing_p executor::thing() const {
    if (m_frames.top().stackBase >= m_stack.size()) throw stack_underflow();
    return m_stack.top();
}

void executor::store_thing(variable_t variable, const thing_p& thing) {
    auto& frame = m_frames.top();
    frame.variables.resize(variable + 1);
    if (frame.variables[variable] != nullptr) {
        frame.variables[variable]->remove_reference();
    }
    frame.variables[variable] = thing;
    thing->add_reference();
}

void executor::store_thing(variable_t variable, thing_p&& thing) {
    auto& frame = m_frames.top();
    frame.variables.resize(variable + 1);
    if (frame.variables[variable] != nullptr) {
        frame.variables[variable]->remove_reference();
    }
    thing->add_reference();
    frame.variables[variable] = std::move(thing);
}

thing_p executor::load_thing(variable_t variable) const {
    const auto& frame = m_frames.top();
    return frame.variables[variable];
}

void executor::step() {
    if ((m_flags & executor_flags::Suspended) == executor_flags::Suspended) return;

    struct frame& frame = m_frames.top();

    instruction_t instr = static_cast<instruction_t>(frame.mod->byte(frame.position++));
    switch (instr) {
    case instruction_t::NoOperation: break;
    case instruction_t::PushB2I: {
        auto thing     = std::make_shared<class thing>(m_context, thing_t::Int32);
        thing->int32() = frame.mod->byte(frame.position++);
        push_thing(std::move(thing));
    } break;
    case instruction_t::Drop: {
        pop_thing();
    } break;
    case instruction_t::Duplicate: {
        push_thing(thing());
    } break;
    case instruction_t::Clone: {
        push_thing(std::move(thing::clone(thing())));
    } break;
    case instruction_t::Add: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs + rhs);
    } break;
    case instruction_t::Sub: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs - rhs);
    } break;
    case instruction_t::Mul: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs * rhs);
    } break;
    case instruction_t::Div: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs / rhs);
    } break;
    case instruction_t::Mod: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs % rhs);
    } break;
    case instruction_t::Equals: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs == rhs);
    } break;
    case instruction_t::NotEquals: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs != rhs);
    } break;
    case instruction_t::LessThan: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs < rhs);
    } break;
    case instruction_t::GreaterThan: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs > rhs);
    } break;
    case instruction_t::LessEqual: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs <= rhs);
    } break;
    case instruction_t::GreaterEqual: {
        auto rhs = pop_thing();
        auto lhs = pop_thing();
        push_thing(lhs >= rhs);
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

        const function_p& function = frame.mod->function_at(funcId);
        switch (function->type()) {
        case function_t::Normal: push_frame(frame.mod, funcId); break;
        case function_t::Native: function->native()(*this); break;
        case function_t::Import: {
            const mod_p& impMod = m_context->m_modules.at(function->imported_module());
            push_frame(frame.mod, function->imported_function());
        } break;
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
