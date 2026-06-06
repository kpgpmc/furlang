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

} // namespace furvm