#include "furvm/executor.hpp"

#include "furvm/context.hpp" // IWYU pragma: keep
#include "furvm/exceptions.hpp"
#include "furvm/instruction.hpp"
#include "furvm/thing.hpp"

#include <stdexcept>

namespace furvm {

executor::executor(egzekutor, executor_handle id, const context_p& context)
  : m_id(id), m_context(context) {}

executor::~executor() {
    m_context->m_executors[m_id] = nullptr;
}

executor_p executor::create(const context_p& context) {
    auto ex = std::make_shared<executor>(egzekutor{}, context->m_executors.size(), context);
    context->m_executors.push_back(ex);
    return std::move(ex);
}

void executor::push_frame(const mod_p& mod, std::size_t position) {
    m_frames.emplace((struct executor::frame){ mod, position, m_stack.size() });
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

void executor::step() {
    if ((m_flags & executor_flags::Suspended) == executor_flags::Suspended) return;

    struct frame& frame = m_frames.top();

    instruction_t instr = static_cast<instruction_t>(frame.mod->byte(frame.position++));
    switch (instr) {
    case instruction_t::NoOperation: break;
    case instruction_t::PushB2I: {
        auto thing     = thing::create(m_context, thing_t::Int32);
        thing->int32() = frame.mod->byte(frame.position++);
        m_stack.push(std::move(thing));
    } break;
    case instruction_t::Drop: {
        pop_thing();
    } break;
    case instruction_t::Duplicate: {
        push_thing(thing());
    } break;
    case instruction_t::Clone: {
        push_thing(thing::clone(thing()));
    } break;
    case instruction_t::Return: {
        pop_frame();
        if (m_frames.empty()) m_flags = m_flags | executor_flags::Done;
    } break;
    case instruction_t::ReturnValue: {
        auto value = pop_thing();
        pop_frame();
        m_stack.push(std::move(value));
    } break;
    case instruction_t::PushConstant: throw std::runtime_error("unimplemented");
    default: throw std::runtime_error("unknown instruction");
    }
}

} // namespace furvm