#ifndef FURVM_HANDLE_HPP
#define FURVM_HANDLE_HPP

#include <limits>
#include <type_traits>
#include <utility>

namespace furvm {

namespace detail {

template <typename Id, typename = void>
struct dead_id {
    static constexpr Id value = {};
};

template <typename Id>
struct dead_id<Id, std::enable_if_t<std::is_integral_v<Id>>> {
    static constexpr Id value = std::numeric_limits<Id>::max();
};

} // namespace detail

template <typename Id, typename T>
class handle {
public:
    using id_type = Id;

    using value_type      = T;
    using reference       = T&;
    using const_reference = const T&;
    using pointer         = T*;
    using const_pointer   = const T*;
public:
    /**
     * @brief Returns a new handle.
     *
     * @param id Id of the handle.
     * @param value Value of the handle.
     */
    template <typename IdF, typename Value>
    handle(IdF&& id, Value&& value)
      : m_id(std::forward<IdF>(id)), m_value(std::forward<Value>(value)) {}

    handle()
      : m_id(detail::dead_id<Id>::value), m_value() {}
public:
    handle& operator=(value_type&& value) noexcept {
        m_value = std::move(value);
        return *this;
    }

    handle& operator=(const value_type& value) noexcept {
        m_value = value;
        return *this;
    }
public:
    /**
     * @brief Returns the handle's id.
     *
     * @return The id.
     */
    constexpr Id id() const { return m_id; }

    /**
     * @brief Returns the handle's pointer.
     *
     * @return The pointer.
     */
    reference value() { return m_value; }

    /**
     * @brief Returns the handle's pointer.
     *
     * @return The pointer.
     */
    const_reference value() const { return m_value; }
public:
    /**
     * @brief Returns the handle's id.
     *
     * @return The id.
     */
    explicit operator Id() const { return m_id; }
public:
    /**
     * @brief Returns a reference to the handle's value.
     *
     * @return The value.
     */
    reference operator*() { return m_value; }

    /**
     * @brief Returns a reference to the handle's value.
     *
     * @return The value.
     */
    const_reference operator*() const { return m_value; }

    /**
     * @brief Returns a pointer to the handle's value.
     *
     * @return The value pointer.
     */
    pointer operator->() { return &m_value; }

    /**
     * @brief Returns a pointer to the handle's value.
     *
     * @return The value pointer.
     */
    const_pointer operator->() const { return &m_value; }
private:
    id_type    m_id;
    value_type m_value;
};

} // namespace furvm

#endif // FURVM_HANDLE_HPP
