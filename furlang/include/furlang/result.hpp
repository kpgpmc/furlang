#ifndef FURLANG_RESULT_HPP
#define FURLANG_RESULT_HPP

#include <exception>
#include <utility>

namespace furlang {

/**
 * @brief Bad result access exception.
 */
class bad_result_access : public std::exception {
public:
    bad_result_access()           = default;
    ~bad_result_access() override = default;

    /**
     * @brief Move constructor.
     */
    bad_result_access(bad_result_access&&) noexcept = default;

    /**
     * @brief Move constructor.
     */
    bad_result_access& operator=(bad_result_access&&) noexcept = default;

    /**
     * @brief Copy constructor.
     */
    bad_result_access(const bad_result_access&) = default;

    /**
     * @brief Copy constructor.
     */
    bad_result_access& operator=(const bad_result_access&) = default;
public:
    /**
     * @brief Returns a C-style string describing the cause of the error.
     *
     * @return The cause of the error.
     */
    const char* what() const noexcept override { return "bad result access"; }
};

/**
 * @brief Result.
 *
 * Result stores either value or error.
 *
 * @tparam R Value type.
 * @tparam E Error type.
 */
template <typename R, typename E>
class result {
public:
    using value_type            = std::remove_reference_t<R>; /**< Value type. */
    using value_reference       = value_type&;                /**< Value reference type. */
    using value_const_reference = const value_type&;          /**< Value const reference type. */
    using value_pointer         = value_type*;                /**< Value pointer type. */
    using value_const_pointer   = const value_type*;          /**< Value const pointer type. */
    using error_type            = std::remove_reference_t<E>; /**< Error type. */
    using error_reference       = error_type&;                /**< Error reference type. */
    using error_const_reference = const error_type&;          /**< Error const reference type. */
public:
    /**
     * @brief Construct a new result.
     *
     * @param value Value to copy.
     */
    result(const value_type& value) { new (&m_value.result) value_type(value); }

    /**
     * @brief Construct a new result.
     *
     * @param value Value to move.
     */
    result(value_type&& value) { new (&m_value.result) value_type(std::move(value)); }

    /**
     * @brief Construct a new result.
     *
     * @param args Variadic arguments to construct the value with.
     */
    template <typename... Args>
    result(Args&&... args) {
        new (&m_value.result) value_type(std::forward<Args>(args)...);
    }

    /**
     * @brief Construct a new error result.
     *
     * @param error Error to copy.
     */
    explicit result(const error_type& error)
      : m_error(true) {
        new (&m_value.error) error_type(error);
    }

    /**
     * @brief Construct a new error result.
     *
     * @param error Error to move.
     */
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

    /**
     * @brief Move constructor.
     */
    result(result&& other) noexcept
      : m_error(other.m_error) {
        if (m_error) {
            new (&m_value.error) error_type(std::move(other.m_value.error));
        } else {
            new (&m_value.result) value_type(std::move(other.m_value.result));
        }
    }

    /**
     * @brief Move constructor.
     */
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

    /**
     * @brief Copy constructor.
     */
    result(const result& other)
      : m_error(other.m_error) {
        if (m_error) {
            new (&m_value.error) error_type(other.m_value.error);
        } else {
            new (&m_value.result) value_type(other.m_value.result);
        }
    }

    /**
     * @brief Copy constructor.
     */
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
    /**
     * @brief Checks if this result contains a value.
     *
     * @return true if contains a value.
     */
    operator bool() const { return !m_error; }

    /**
     * @brief Checks if this result contains an error.
     *
     * @return true if contains an error.
     */
    bool operator!() const { return m_error; }

    /**
     * @brief Compares two results for equality.
     *
     * @param rhs Result to compare against.
     * @return true if the results are equal.
     */
    bool operator==(const result& rhs) const {
        return m_error == rhs.m_error && m_error ? m_value.error == rhs.m_value.error
                                                 : m_value.result == rhs.m_value.result;
    }

    /**
     * @brief Compares two results for inequality.
     *
     * @param rhs Result to compare against.
     * @return true if the results are not equal.
     */
    bool operator!=(const result& rhs) const { return !this->operator==(rhs); }

    /**
     * @brief Compares a result with a value for equality.
     *
     * @param rhs Value to compare against.
     * @return true if the values are equal.
     */
    bool operator==(const value_type& rhs) const { return !m_error && m_value.result == rhs; }

    /**
     * @brief Compares a result with a value for inequality.
     *
     * @param rhs Value to compare against.
     * @return true if the values are not equal.
     */
    bool operator!=(const value_type& rhs) const { return !this->operator==(rhs); }

    /**
     * @brief Returns a reference to value.
     *
     * @return Reference to the value.
     */
    value_reference operator*() { return value(); }

    /**
     * @brief Returns a reference to value.
     *
     * @return Const reference to the value.
     */
    value_const_reference operator*() const { return value(); }

    /**
     * @brief Returns a pointer to value.
     *
     * @return Pointer to the value.
     */
    value_pointer operator->() { return &value(); }

    /**
     * @brief Returns a pointer to value.
     *
     * @return Const pointer to the value.
     */
    value_const_pointer operator->() const { return &value(); }
public:
    /**
     * @brief Checks if this result has a value.
     *
     * @return true if has a value.
     */
    bool has_value() const { return !m_error; }

    /**
     * @brief Checks if this result has an error.
     *
     * @return true if has an error.
     */
    bool has_error() const { return m_error; }

    /**
     * @brief Returns a reference to value.
     *
     * @return Reference to the value.
     */
    value_reference value() {
        if (m_error) throw bad_result_access();
        return m_value.result;
    }

    /**
     * @brief Returns a reference to value.
     *
     * @return Const reference to the value.
     */
    value_const_reference value() const {
        if (m_error) throw bad_result_access();
        return m_value.result;
    }

    /**
     * @brief Returns a reference to error.
     *
     * @return Reference to the error.
     */
    error_reference error() {
        if (!m_error) throw bad_result_access();
        return m_value.error;
    }

    /**
     * @brief Returns a reference to error.
     *
     * @return Const reference to the error.
     */
    error_const_reference error() const {
        if (!m_error) throw bad_result_access();
        return m_value.error;
    }
public:
    /**
     * @brief Sets this results value.
     *
     * @param value Value to copy.
     */
    void set_value(const value_type& value) {
        if (m_error) m_value.error.~error_type();
        new (&m_value.result) value_type(value);
    }

    /**
     * @brief Sets this results value.
     *
     * @param value Value to move.
     */
    void set_value(value_type&& value) {
        if (m_error) m_value.error.~error_type();
        new (&m_value.result) value_type(std::move(value));
    }

    /**
     * @brief Sets this results error.
     *
     * @param error Error to copy.
     */
    void set_error(const error_type& error) {
        if (!m_error) m_value.result.~value_type();
        new (&m_value.error) error_type(error);
    }

    /**
     * @brief Sets this results error.
     *
     * @param error Error to move.
     */
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