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
      : m_location(error.location()), m_error(error.error()) {}

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
    bool has_error() const { return !m_value.has_value(); }

    value_type move() {
        value_type value = std::move(*m_value);
        m_value.reset();
        return value;
    }

    Error error() const { return m_error; }

    reference       operator*() { return m_value.value(); }         // NOLINT(bugprone-unchecked-optional-access)
    const_reference operator*() const { return m_value.value(); }   // NOLINT(bugprone-unchecked-optional-access)
    pointer         operator->() { return &m_value.value(); }       // NOLINT(bugprone-unchecked-optional-access)
    const_pointer   operator->() const { return &m_value.value(); } // NOLINT(bugprone-unchecked-optional-access)
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

    friend struct data;

    friend std::ostream& operator<<(std::ostream& os, const handle& result);
public:
    using value_type      = T;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using reference       = T&;
    using const_reference = const T&;
public:
    handle() = default;

    template <typename U, typename = std::enable_if_t<std::is_base_of_v<value_type, U>>>
    handle(location location, std::shared_ptr<U> value)
      : m_data(data(location, std::move(value))) {}

    template <typename U,
        typename = std::enable_if_t<std::is_base_of_v<value_type, U> || std::is_base_of_v<U, value_type>>>
    handle(const handle<U*, Error>& other)
      : m_data(data{ other }) {}

    template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<value_type, Args...>>>
    handle(location location, Args&&... args)
      : m_data(data(location, std::forward<Args>(args)...)) {}

    template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<value_type, Args...>>>
    handle(location location, furlang::arena& arena, Args&&... args)
      : m_data(data(location, arena, std::forward<Args>(args)...)) {}

    handle(location location, Error&& error)
      : m_data(data(location, std::move(error))) {}

    template <typename U>
    handle(const handle<U, Error>& error)
      : m_data(data(error)) {}

    ~handle() = default;

    handle(handle&& other) noexcept
      : m_data(std::move(other.m_data)) {}

    template <typename U, typename = std::enable_if_t<std::is_base_of_v<value_type, U>>>
    handle(handle<U*, Error>&& other) noexcept
      : m_data(data{ std::move(other) }) {}

    handle(const handle& other)
      : m_data(other.m_data) {}

    handle& operator=(handle&& other) noexcept {
        if (this == &other) return *this;
        m_data = std::move(other.m_data);
        return *this;
    }

    handle& operator=(const handle& other) {
        if (this == &other) return *this;
        m_data = other.m_data;
        return *this;
    }
public:
    location location() const { return m_data->location; }

    bool present() const { return m_data.has_value() && m_data->value != nullptr; }
    bool has_error() const { return m_data.has_value() && m_data->value == nullptr; }

    std::shared_ptr<value_type> shared() const { return m_data->value; }
    Error                       error() const { return m_data->error; }

    reference       operator*() { return *m_data->value; }
    const_reference operator*() const { return *m_data->value; }
    pointer         operator->() { return m_data->value.get(); }
    const_pointer   operator->() const { return m_data->value.get(); }
public:
    friend std::ostream& operator<<(std::ostream& os, const handle& result) {
        if (!result.m_data.has_value()) return os << "handle empty";
        os << result.m_data->location << ": ";
        if (result.m_data->value != nullptr) {
            os << *result.m_data->value;
        } else {
            os << "ERROR: " << result.m_data->error;
        }
        return os;
    }
private:
    struct data {
        struct location             location;
        std::shared_ptr<value_type> value = nullptr;
        Error                       error = {};

        template <typename U, typename = std::enable_if_t<std::is_base_of_v<value_type, U>>>
        data(struct location location, std::shared_ptr<U> value)
          : location(location), value(std::move(value)) {}

        template <typename U,
            typename = std::enable_if_t<std::is_base_of_v<value_type, U> || std::is_base_of_v<U, value_type>>>
        data(handle<U*, Error>&& other)        // NOLINT
          : location(other.m_data->location) { // NOLINT(bugprone-unchecked-optional-access)
            if constexpr (std::is_base_of_v<value_type, U>) {
                value = std::static_pointer_cast<value_type>(
                    std::move(other.m_data->value)); // NOLINT(bugprone-unchecked-optional-access)
            } else {
                value = std::dynamic_pointer_cast<value_type>(
                    std::move(other.m_data->value)); // NOLINT(bugprone-unchecked-optional-access)
            }
        }

        template <typename U,
            typename = std::enable_if_t<std::is_base_of_v<value_type, U> || std::is_base_of_v<U, value_type>>>
        data(const handle<U*, Error>& other)
          : location(other.m_data->location) {
            if constexpr (std::is_base_of_v<value_type, U>) {
                value = std::static_pointer_cast<value_type>(std::move(other.m_data->value));
            } else {
                value = std::dynamic_pointer_cast<value_type>(std::move(other.m_data->value));
            }
        }

        template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<value_type, Args...>>>
        data(struct location location, Args&&... args)
          : location(location), value(std::make_shared<value_type>(std::forward<Args>(args)...)) {}

        template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<value_type, Args...>>>
        data(struct location location, furlang::arena& arena, Args&&... args)
          : location(location), value(arena.allocate_shared<value_type>(std::forward<Args>(args)...)) {}

        data(struct location location, Error&& error)
          : location(location), error(std::move(error)) {}

        template <typename U>
        data(const handle<U, Error>& error)
          : location(error.location()), error(error.error()) {}

        ~data() = default;

        data(data&& other) noexcept
          : location(other.location), value(std::move(other.value)), error(std::move(other.error)) {}

        data(const data& other)
          : location(other.location), value(other.value), error(other.error) {}

        data& operator=(data&& other) noexcept {
            if (this == &other) return *this;
            location = other.location;
            value    = std::move(other.value);
            error    = std::move(other.error);
            return *this;
        }

        data& operator=(const data& other) noexcept {
            if (this == &other) return *this;
            location = other.location;
            value    = other.value;
            error    = other.error;
            return *this;
        }
    };
    std::optional<data> m_data = {};
};

} // namespace furc

#endif // FURC_HANDLE_HPP