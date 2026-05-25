#ifndef FURC_HANDLE_HPP
#define FURC_HANDLE_HPP

#include "furc/diag.hpp"
#include "furlang/arena.hpp"

#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <type_traits>

namespace furc {

template <typename T, typename Error = std::string, typename = void>
class handle;

template <typename T, typename Error>
class handle<T, Error> {
    template <typename, typename, typename>
    friend class handle;

    friend std::ostream& operator<<(std::ostream& os, const handle& result);
public:
    using value_type      = std::remove_reference_t<T>;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using reference       = T&;
    using const_reference = const T&;
public:
    handle(location location, value_type&& value)
      : m_location(location), m_value(std::move(value)) {}

    template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<value_type, Args...>>>
    handle(location location, Args&&... args)
      : m_location(location), m_value(value_type(std::forward<Args>(args)...)) {}

    handle(location location, Error&& error)
      : m_location(location), m_error(std::move(error)) {}

    template <typename U>
    handle(const handle<U, Error>& error)
      : m_location(error.location()), m_error(error) {}

    ~handle() = default;

    handle(handle&& other) noexcept
      : m_location(other.m_location), m_value(std::move(other.m_value)), m_error(std::move(other.m_error)) {
        other.m_value.reset();
    }

    handle(const handle&) = delete;

    handle& operator=(handle&& other) noexcept {
        if (this == &other) return *this;
        m_location = other.m_location;
        m_value    = other.m_value;
        m_error    = std::move(other.m_error);
        other.m_value.reset();
        return *this;
    }

    handle& operator=(const handle&) = delete;
public:
    location location() const { return m_location; }

    bool present() const { return m_value.has_value(); }
    bool error() const { return !m_value.has_value(); }

    value_type move() {
        value_type value = *m_value;
        m_value.reset();
        return value;
    }

    operator reference() { return *m_value; }
    operator const_reference() const { return *m_value; }
    operator Error() const { return m_error; }

    reference       operator*() { return *m_value; }
    const_reference operator*() const { return *m_value; }
    pointer         operator->() { return &*m_value; }
    const_pointer   operator->() const { return &*m_value; }
public:
    bool operator==(const T& rhs) const { return m_value.has_value() && *m_value == rhs; }
public:
    friend std::ostream& operator<<(std::ostream& os, const handle& result) {
        os << result.m_location << ": ";
        if (result.m_value.has_value()) {
            os << *result.m_value;
        } else {
            os << "ERROR: " << result.m_error;
        }
        return os;
    }
private:
    struct location           m_location;
    std::optional<value_type> m_value = {};
    Error                     m_error;
};

template <typename T, typename Error>
class handle<T*, Error> {
    template <typename, typename, typename>
    friend class handle;

    friend std::ostream& operator<<(std::ostream& os, const handle& result);
public:
    using value_type      = T;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using reference       = T&;
    using const_reference = const T&;
public:
    template <typename U = value_type, typename = std::enable_if_t<std::is_base_of_v<value_type, U>>>
    handle(location location, std::shared_ptr<U> value)
      : m_location(location), m_value(value) {}

    template <typename U, typename = std::enable_if_t<std::is_base_of_v<U, value_type>>>
    handle(const handle<U*, Error>& other)
      : m_location(other.m_location), m_value(std::dynamic_pointer_cast<value_type>(other.m_value)) {}

    template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<value_type, Args...>>>
    handle(location location, Args&&... args)
      : m_location(location), m_value(std::make_shared<value_type>(std::forward<Args>(args)...)) {}

    template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<value_type, Args...>>>
    handle(location location, furlang::arena& arena, Args&&... args)
      : m_location(location), m_value(arena.allocate_shared<value_type>(std::forward<Args>(args)...)) {}

    handle(location location, Error&& error)
      : m_location(location), m_error(std::move(error)) {}

    template <typename U>
    handle(const handle<U, Error>& error)
      : m_location(error.location()), m_error(error) {}

    ~handle() = default;

    handle(handle&& other) noexcept
      : m_location(other.m_location), m_value(std::move(other.m_value)), m_error(std::move(other.m_error)) {
        other.m_value = nullptr;
    }

    template <typename U = value_type,
        typename         = std::enable_if_t<std::is_base_of_v<value_type, U> || std::is_base_of_v<U, value_type>>>
    handle(handle<U*, Error>&& other) noexcept // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
      : m_location(other.m_location), m_value(std::move(other.m_value)), m_error(std::move(other.m_error)) {
        other.m_value = nullptr;
    }

    handle(const handle& other)
      : m_location(other.m_location), m_value(other.m_value), m_error(other.m_error) {}

    handle& operator=(handle&& other) noexcept {
        if (this == &other) return *this;
        m_location    = other.m_location;
        m_value       = std::move(other.m_value);
        m_error       = std::move(other.m_error);
        other.m_value = nullptr;
        return *this;
    }

    handle& operator=(const handle& other) {
        if (this == &other) return *this;
        m_location = other.m_location;
        m_value    = other.m_value;
        m_error    = other.m_error;
        return *this;
    }
public:
    location location() const { return m_location; }

    bool present() const { return m_value != nullptr; }
    bool error() const { return m_value == nullptr; }

    std::shared_ptr<value_type> shared() { return m_value; }

    operator reference() { return *m_value; }
    operator const_reference() const { return *m_value; }
    operator Error() const { return m_error; }

    reference       operator*() { return *m_value; }
    const_reference operator*() const { return *m_value; }
    pointer         operator->() { return m_value.get(); }
    const_pointer   operator->() const { return m_value.get(); }
public:
    friend std::ostream& operator<<(std::ostream& os, const handle& result) {
        os << result.m_location << ": ";
        if (result.m_value != nullptr) {
            os << *result.m_value;
        } else {
            os << "ERROR: " << result.m_error;
        }
        return os;
    }
private:
    struct location             m_location;
    std::shared_ptr<value_type> m_value = nullptr;
    Error                       m_error;
};

} // namespace furc

#endif // FURC_HANDLE_HPP