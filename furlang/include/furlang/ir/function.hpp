#ifndef FURLANG_IR_FUNCTION_HPP
#define FURLANG_IR_FUNCTION_HPP

#include "furlang/ir/block.hpp"

#include <memory>
#include <vector>

namespace furlang {
namespace ir {

class function {
public:
    using value_type = std::shared_ptr<block>;
public:
    function(const std::string& name)
      : m_name(name) {}

    function(std::string&& name)
      : m_name(std::move(name)) {}
public:
    const std::string& name() const { return m_name; }

    value_type push() { return m_blocks.emplace_back(std::make_shared<block>()); }

    const std::vector<value_type>& blocks() const { return m_blocks; }
private:
    std::string             m_name;
    std::vector<value_type> m_blocks;
};

} // namespace ir
} // namespace furlang

#endif // FURLANG_IR_FUNCTION_HPP