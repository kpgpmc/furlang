#ifndef FURVM_EXCEPTIONS_HPP
#define FURVM_EXCEPTIONS_HPP

#include <exception>

namespace furvm {

class stack_underflow : public std::exception {
public:
    stack_underflow()           = default;
    ~stack_underflow() override = default;

    /**
     * @brief Move constructor.
     */
    stack_underflow(stack_underflow&&) noexcept = default;

    /**
     * @brief Move constructor.
     */
    stack_underflow& operator=(stack_underflow&&) noexcept = default;

    /**
     * @brief Copy constructor.
     */
    stack_underflow(const stack_underflow&) = default;

    /**
     * @brief Copy constructor.
     */
    stack_underflow& operator=(const stack_underflow&) = default;
public:
    /**
     * @brief Returns a C-style string describing the cause of the error.
     *
     * @return The cause of the error.
     */
    const char* what() const noexcept override { return "stack underflow"; }
};

} // namespace furvm

#endif // FURVM_EXCEPTIONS_HPP