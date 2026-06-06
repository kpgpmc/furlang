#include "furvm/executor.hpp"

#include "furvm/context.hpp"

namespace furvm {

executor::executor(egzekutor, executor_handle id, const std::shared_ptr<context>& context)
  : m_id(id), m_context(context) {}

executor::~executor() {
    m_context->m_executors[m_id] = nullptr;
}

std::shared_ptr<executor> executor::create(const std::shared_ptr<context>& context) {
    auto ex = std::make_shared<executor>(egzekutor{}, context->m_executors.size(), context);
    context->m_executors.push_back(ex);
    return std::move(ex);
}

void executor::push_frame(const std::shared_ptr<mod>& mod, std::size_t position) {
    m_frames.emplace((struct executor::frame){ mod, position, m_stack.size() });
}

struct executor::frame executor::pop_frame() {
    struct executor::frame frame = m_frames.top();
    m_frames.pop();
    return frame;
}

struct executor::frame executor::frame() const {
    return m_frames.top();
}

void executor::push_thing(const std::shared_ptr<class thing>& thing) {
    m_stack.push(thing);
}

std::shared_ptr<class thing> executor::pop_thing() {
    if (m_frames.top().stackBase >= m_stack.size()) return {};
    std::shared_ptr<class thing> top = std::move(m_stack.top());
    m_stack.pop();
    return top;
}

std::shared_ptr<class thing> executor::thing() const {
    if (m_frames.top().stackBase >= m_stack.size()) return {};
    return m_stack.top();
}

} // namespace furvm