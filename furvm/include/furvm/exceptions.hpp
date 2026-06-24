#ifndef FURVM_EXCEPTIONS_HPP
#define FURVM_EXCEPTIONS_HPP

#include <exception>

namespace furvm {

class bad_thing_access : public std::exception {
public:
    bad_thing_access()           = default;
    ~bad_thing_access() override = default;

    /**
     * @brief Move constructor.
     */
    bad_thing_access(bad_thing_access&&) noexcept = default;

    /**
     * @brief Move constructor.
     */
    bad_thing_access& operator=(bad_thing_access&&) noexcept = default;

    /**
     * @brief Copy constructor.
     */
    bad_thing_access(const bad_thing_access&) = default;

    /**
     * @brief Copy constructor.
     */
    bad_thing_access& operator=(const bad_thing_access&) = default;
public:
    /**
     * @brief Returns a C-style string describing the cause of the error.
     *
     * @return The cause of the error.
     */
    const char* what() const noexcept override { return "bad thing access"; }
};

/**
 * @brief Bad constant access exception.
 */
class bad_constant_access : public std::exception {
public:
    bad_constant_access()           = default;
    ~bad_constant_access() override = default;

    /**
     * @brief Move constructor.
     */
    bad_constant_access(bad_constant_access&&) noexcept = default;

    /**
     * @brief Move constructor.
     */
    bad_constant_access& operator=(bad_constant_access&&) noexcept = default;

    /**
     * @brief Copy constructor.
     */
    bad_constant_access(const bad_constant_access&) = default;

    /**
     * @brief Copy constructor.
     */
    bad_constant_access& operator=(const bad_constant_access&) = default;
public:
    /**
     * @brief Returns a C-style string describing the cause of the error.
     *
     * @return The cause of the error.
     */
    const char* what() const noexcept override { return "bad constant access"; }
};

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
