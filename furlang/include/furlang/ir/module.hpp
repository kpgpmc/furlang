#ifndef FURLANG_IR_MODULE_HPP
#define FURLANG_IR_MODULE_HPP

#include "furlang/ir/function.hpp"

#include <memory>
#include <vector>

namespace furlang {
namespace ir {

class module {
public:
    using value_type = std::unique_ptr<function>;
public:
    module() = default;
public:
    template <typename... Args>
    value_type& push(Args&&... args) {
        return m_functions.emplace_back(std::forward<Args>(args)...);
    }

    std::vector<value_type>&       functions() { return m_functions; }
    const std::vector<value_type>& functions() const { return m_functions; }
private:
    std::vector<value_type> m_functions;
};

} // namespace ir
} // namespace furlang

#endif // FURLANG_IR_MODULE_HPP