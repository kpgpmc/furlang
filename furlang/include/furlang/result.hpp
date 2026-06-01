#ifndef FURLANG_RESULT_HPP
#define FURLANG_RESULT_HPP

#include <exception>
#include <utility>

namespace furlang {

class bad_result_access : public std::exception {
public:
    bad_result_access()           = default;
    ~bad_result_access() override = default;

    bad_result_access(bad_result_access&&) noexcept            = default;
    bad_result_access& operator=(bad_result_access&&) noexcept = default;
    bad_result_access(const bad_result_access&)                = default;
    bad_result_access& operator=(const bad_result_access&)     = default;
public:
    const char* what() const noexcept override { return "bad result access"; }
};

template <typename R, typename E>
class result {
public:
    using value_type            = std::remove_reference_t<R>;
    using value_reference       = value_type&;
    using value_const_reference = const value_type&;
    using value_pointer         = value_type*;
    using value_const_pointer   = const value_type*;
    using error_type            = std::remove_reference_t<E>;
    using error_reference       = error_type&;
    using error_const_reference = const error_type&;
public:
    result(const value_type& value) { new (&m_value.result) value_type(value); }

    result(value_type&& value) { new (&m_value.result) value_type(std::move(value)); }

    template <typename... Args>
    result(Args&&... args) {
        new (&m_value.result) value_type(std::forward<Args>(args)...);
    }

    explicit result(const error_type& error)
      : m_error(true) {
        new (&m_value.error) error_type(error);
    }

    explicit result(error_type&& error)
      : m_error(true) {
        new (&m_value.error) error_type(std::move(error));
    }

    ~result() {
        if (m_error) {
            m_value.error.~error_type();
        } else {
            m_value.result.~value_type();
        }
    }

    result(result&& other) noexcept
      : m_error(other.m_error) {
        if (m_error) {
            new (&m_value.error) error_type(std::move(other.m_value.error));
        } else {
            new (&m_value.result) value_type(std::move(other.m_value.result));
        }
    }

    result& operator=(result&& other) noexcept {
        if (this == &other) return *this;
        m_error = other.m_error;
        if (m_error) {
            new (&m_value.error) error_type(std::move(other.m_value.error));
        } else {
            new (&m_value.result) value_type(std::move(other.m_value.result));
        }
        return *this;
    }

    result(const result& other)
      : m_error(other.m_error) {
        if (m_error) {
            new (&m_value.error) error_type(other.m_value.error);
        } else {
            new (&m_value.result) value_type(other.m_value.result);
        }
    }

    result& operator=(const result& other) {
        if (this == &other) return *this;
        m_error = other.m_error;
        if (m_error) {
            new (&m_value.error) error_type(other.m_value.error);
        } else {
            new (&m_value.result) value_type(other.m_value.result);
        }
        return *this;
    }
public:
         operator bool() const { return !m_error; }
    bool operator!() const { return m_error; }

    bool operator==(const result& rhs) const {
        return m_error == rhs.m_error && m_error ? m_value.error == rhs.m_value.error
                                                 : m_value.result == rhs.m_value.result;
    }

    bool operator!=(const result& rhs) const { return !this->operator==(rhs); }

    value_reference       operator*() { return value(); }
    value_const_reference operator*() const { return value(); }

    value_pointer       operator->() { return &value(); }
    value_const_pointer operator->() const { return &value(); }
public:
    bool has_value() const { return !m_error; }
    bool has_error() const { return m_error; }

    value_reference value() {
        if (m_error) throw bad_result_access();
        return m_value.result;
    }

    value_const_reference value() const {
        if (m_error) throw bad_result_access();
        return m_value.result;
    }

    error_reference error() {
        if (!m_error) throw bad_result_access();
        return m_value.error;
    }

    error_const_reference error() const {
        if (!m_error) throw bad_result_access();
        return m_value.error;
    }
public:
    void set_value(const value_type& value) {
        if (m_error) m_value.error.~error_type();
        new (&m_value.result) value_type(value);
    }

    void set_value(value_type&& value) {
        if (m_error) m_value.error.~error_type();
        new (&m_value.result) value_type(std::move(value));
    }

    void set_error(const error_type& error) {
        if (!m_error) m_value.result.~value_type();
        new (&m_value.error) error_type(error);
    }

    void set_error(error_type&& error) {
        if (!m_error) m_value.result.~value_type();
        new (&m_value.error) error_type(std::move(error));
    }
private:
    union value {
        value_type result;
        error_type error;

        value() {}
        ~value() {}

        value(value&&) noexcept {}
        value& operator=(value&&) noexcept {}
        value(const value&) {}
        value& operator=(const value&) {}
    } m_value;
    bool m_error = false;
};

} // namespace furlang

#endif // FURLANG_RESULT_HPP