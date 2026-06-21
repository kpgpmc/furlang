#ifndef FURVM_HANDLE_HPP
#define FURVM_HANDLE_HPP

#include "furvm/detail/handle.hpp"
#include "furvm/fwd.hpp"

#include <atomic>
#include <functional>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace furvm {

// TODO: Implement generational indexes

template <typename Id>
class refcount_header {
public:
    using id_type       = Id;            /**< Id type. */
    using refcount_type = std::uint32_t; /**< Reference count type. */
public:
    /**
     * @brief Constructs a reference counting header.
     *
     * @param id Identifier of the handle's value.
     * @param refCount Handle's reference count.
     * @param onRelease Callback function.
     */
    template <typename IdFwd, typename Func>
    refcount_header(IdFwd&& id, refcount_type refCount, Func&& onRelease)
      : m_id(std::forward<IdFwd>(id)), m_refCount(refCount), m_onRelease(std::forward<Func>(onRelease)) {}
public:
    /**
     * @brief Returns the header's reference count.
     *
     * @return The reference count.
     */
    refcount_type reference_count() const { return m_refCount; }

    /**
     * @brief Increments the header's reference count.
     */
    void acquire() { ++m_refCount; }

    /**
     * @brief Decrements the header's reference count.
     *
     * If the reference count reaches 0, the onRelease callback passed in the constructor will be called.
     */
    void release() {
        --m_refCount;
        if (m_refCount == 0) m_onRelease(m_id);
    }
public:
    /**
     * @brief Returns the header's identifier.
     *
     * @return The identifier.
     */
    id_type id() const { return m_id; }
private:
    id_type                             m_id;
    std::atomic<refcount_type>          m_refCount;
    std::function<void(const id_type&)> m_onRelease;
};

template <typename Id>
class generic_header {
public:
    using id_type = Id; /**< Id type. */
public:
    /**
     * @brief Constructs a generic header.
     *
     * @param id Identifier of the handle.
     */
    generic_header(id_type id)
      : m_id(id) {}
public:
    /**
     * @brief Returns the header's identifier.
     */
    id_type id() const { return m_id; }
private:
    id_type m_id;
};

template <typename Value, typename Header = refcount_header<std::uint32_t>>
class handle {
public:
    using value_type      = Value;        /** Value type. */
    using reference       = Value&;       /** Reference type. */
    using const_reference = const Value&; /** Constant reference type. */
    using pointer         = Value*;       /** Pointer type. */
    using const_pointer   = const Value*; /** Constant pointer type. */
public:
    using id_type = typename Header::id_type; /** Id type of the header. */
public:
    using pair_type = std::pair<Header, Value>; /** Type of a header-value pair. */
public:
    handle() = default;

    /**
     * @brief Constructs a handle.
     *
     * @param value A pointer to the header-value pair.
     */
    handle(pair_type* value)
      : m_value(value) {
        if constexpr (detail::header_has_refcount_v<Header>) {
            m_value->first.acquire();
        }
    }

    /**
     * @brief Destructs a handle.
     */
    ~handle() {
        if constexpr (detail::header_has_refcount_v<Header>) {
            if (m_value != nullptr) m_value->first.release();
        }
        m_value = nullptr;
    }

    /**
     * @brief Move constructor.
     */
    handle(handle&& other) noexcept
      : m_value(other.m_value) {
        other.m_value = nullptr;
    }

    /**
     * @brief Move constructor.
     */
    handle& operator=(handle&& other) noexcept {
        if (this == &other) return *this;
        m_value       = other.m_value;
        other.m_value = nullptr;
        return *this;
    }

    /**
     * @brief Copy constructor.
     */
    handle(const handle& other)
      : m_value(other.m_value) {
        m_value->first.acquire();
    }

    /**
     * @brief Copy constructor.
     */
    handle& operator=(const handle& other) {
        if (this == &other) return *this;
        m_value = other.m_value;
        m_value->first.acquire();
        return *this;
    }
public:
    /**
     * @brief Returns an identifier of the handle's header.
     *
     * @return The header's identifier.
     */
    id_type id() const { return m_value->first.id(); }

    /**
     * @brief Returns whether the handle is empty.
     *
     * @return true if the handle is empty.
     */
    bool empty() const { return m_value == nullptr; }

    /**
     * @brief Returns the handle's header reference count.
     *
     * @return The reference count.
     */
    template <typename ReferenceCount = typename Header::refcount_type>
    ReferenceCount reference_count() const {
        return m_value->first.reference_count();
    }

    /**
     * @brief Returns a pointer to the handle's value.
     *
     * @return The value pointer.
     */
    pointer operator->() { return &m_value->second; }

    /**
     * @brief Returns a pointer to the handle's value.
     *
     * @return The value pointer.
     */
    const_pointer operator->() const { return &m_value->second; }

    /**
     * @brief Returns a reference to the handle's value.
     *
     * @return The value reference.
     */
    reference operator*() { return m_value->second; }

    /**
     * @brief Returns a reference to the handle's value.
     *
     * @return The value reference.
     */
    const_reference operator*() const { return m_value->second; }

    /**
     * @brief Returns a reference to the handle's value.
     *
     * @return The value reference.
     */
    reference value() { return m_value->second; }

    /**
     * @brief Returns a reference to the handle's value.
     *
     * @return The value reference.
     */
    const_reference value() const { return m_value->second; }
private:
    pair_type* m_value = nullptr;
};

template <typename Handle>
class handle_container<Handle, std::enable_if_t<!std::is_integral_v<typename Handle::id_type>>> {
private:
    using pair_type = typename Handle::pair_type; /**< Handle's pair type. */
public:
    using value_type  = std::remove_cv_t<std::remove_reference_t<Handle>>; /**< Handle type. */
    using const_value = std::add_const_t<value_type>;                      /**< Constant handle type. */

    using id_type = typename Handle::id_type; /**< Handle's header identifier type. */
public:
    handle_container()  = default;
    ~handle_container() = default;

    handle_container(handle_container&&) noexcept            = default;
    handle_container& operator=(handle_container&&) noexcept = default;

    handle_container(const handle_container&)            = delete;
    handle_container& operator=(const handle_container&) = delete;
public:
    /**
     * @brief Emplaces a new value.
     *
     * @param id Identifier of the emplaced value.
     * @param args Arguments passed to the Handle's value type constructor.
     * @return A handle to the emplaced value.
     */
    template <typename IdFwd,
        typename... Args,
        typename = std::enable_if_t<std::is_constructible_v<typename pair_type::second_type, Args...>>>
    value_type emplace(IdFwd&& id, Args&&... args) {
        id_type idFwd = std::forward<IdFwd>(id);
        if (auto it = m_pairs.find(idFwd); it != m_pairs.end()) delete it->second;
        auto pair = new pair_type(std::piecewise_construct,
            std::forward_as_tuple(idFwd, 0, [&](const id_type& id) { erase(id); }),
            std::forward_as_tuple(std::forward<Args>(args)...));
        m_pairs.emplace(std::move(idFwd), pair);
        return { pair };
    }

    /**
     * @brief Returns a handle to the container's value.
     *
     * @param id Idenfifier of the value.
     * @return The value.
     */
    template <typename IdFwd>
    value_type at(IdFwd&& id) {
        return { m_pairs.at(std::forward<IdFwd>(id)) };
    }

    /**
     * @brief Returns a handle to the container's value.
     *
     * @param id Idenfifier of the value.
     * @return The value.
     */
    template <typename IdFwd>
    const_value at(IdFwd&& id) const {
        return { m_pairs.at(std::forward<IdFwd>(id)) };
    }

    /**
     * @brief Erases a value from the container.
     *
     * @param id Identifier of the value.
     */
    template <typename IdFwd>
    void erase(IdFwd&& id) {
        auto it = m_pairs.find(std::forward<IdFwd>(id));
        if (it == m_pairs.end()) return;
        delete it->second;
        m_pairs.erase(it);
    }
private:
    std::unordered_map<id_type, pair_type*> m_pairs;
};

template <typename Handle>
class handle_container<Handle, std::enable_if_t<std::is_integral_v<typename Handle::id_type>>> {
private:
    using pair_type = typename Handle::pair_type; /**< Handle's pair type. */
public:
    using value_type  = std::remove_cv_t<std::remove_reference_t<Handle>>; /**< Handle type. */
    using const_value = std::add_const_t<value_type>;                      /**< Constant handle type. */

    using id_type = typename Handle::id_type; /**< Handle's header identifier type. */
public:
    handle_container()  = default;
    ~handle_container() = default;

    handle_container(handle_container&&) noexcept            = default;
    handle_container& operator=(handle_container&&) noexcept = default;

    handle_container(const handle_container&)            = delete;
    handle_container& operator=(const handle_container&) = delete;
public:
    /**
     * @brief Emplaces a new value.
     *
     * @param id Identifier of the emplaced value.
     * @param args Arguments passed to the Handle's value type constructor.
     * @return A handle to the emplaced value.
     */
    template <typename... Args,
        typename = std::enable_if_t<std::is_constructible_v<typename pair_type::second_type, Args...>>>
    value_type emplace(id_type id, Args&&... args) {
        if (id >= m_pairs.size()) {
            m_pairs.resize(id + 1, nullptr);
        } else if (m_pairs[id] != nullptr) {
            delete m_pairs[id];
        }

        pair_type* newPair = new pair_type(std::piecewise_construct,
            std::forward_as_tuple(id, 0, [&](const id_type& id) { erase(id); }),
            std::forward_as_tuple(std::forward<Args>(args)...));

        m_pairs[id] = newPair;
        return { newPair };
    }

    /**
     * @brief Emplaces a new value.
     *
     * Emplaces a new value with an automatically-assigned identifier.
     *
     * @param args Arguments passed to the Handle's value type constructor.
     * @return A handle to the emplaced value.
     */
    template <typename... Args,
        typename = std::enable_if_t<std::is_constructible_v<typename Handle::pair_type::second_type, Args...>>>
    value_type emplace_back(Args&&... args) {
        return emplace(static_cast<id_type>(m_pairs.size()), std::forward<Args>(args)...);
    }

    /**
     * @brief Returns a handle to a value.
     *
     * @param id Identifier of the value.
     * @return The value.
     */
    value_type at(id_type id) { return { m_pairs.at(id) }; }

    /**
     * @brief Returns a handle to a value.
     *
     * @param id Identifier of the value.
     * @return The value.
     */
    const_value at(id_type id) const { return { m_pairs.at(id) }; }

    /**
     * @brief Erases a value from the container.
     *
     * @param id Identifier of the value.
     */
    void erase(id_type id) {
        if (id >= m_pairs.size()) return;
        delete m_pairs[id];
        m_pairs[id] = nullptr;
    }
private:
    std::vector<pair_type*> m_pairs;
};

} // namespace furvm

#endif // FURVM_HANDLE_HPP
