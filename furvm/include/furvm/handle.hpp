#ifndef FURVM_HANDLE_HPP
#define FURVM_HANDLE_HPP

#include "furvm/detail/handle.hpp"
#include "furvm/fwd.hpp"

#include <atomic>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace furvm {

template <typename Handle, typename = void>
class handle_container;

// TODO: Implement generational indexes

template <typename Id>
class refcount_header {
public:
    using id_type       = Id;
    using refcount_type = std::uint32_t;
public:
    template <typename IdFwd>
    refcount_header(IdFwd&& id, refcount_type refCount)
      : m_id(std::forward<IdFwd>(id)), m_refCount(refCount) {}
public:
    refcount_type reference_count() const { return m_refCount; }

    void acquire() { ++m_refCount; }

    void release() { --m_refCount; }
public:
    id_type id() const { return m_id; }
private:
    id_type                    m_id;
    std::atomic<refcount_type> m_refCount;
};

template <typename Id>
class generic_header {
public:
    using id_type = Id;
public:
    generic_header(id_type id)
      : m_id(id) {}
public:
    id_type id() const { return m_id; }
private:
    id_type m_id;
};

template <typename Value, typename Header = refcount_header<std::uint32_t>>
class handle {
public:
    using value_type      = Value;
    using reference       = Value&;
    using const_reference = const Value&;
    using pointer         = Value*;
    using const_pointer   = const Value*;
public:
    using id_type = typename Header::id_type;
public:
    using pair_type = std::pair<Header, Value>;
public:
    handle() = default;

    handle(pair_type* value)
      : m_value(value) {
        if constexpr (detail::header_has_refcount_v<Header>) {
            m_value->first.acquire();
        }
    }

    ~handle() {
        if constexpr (detail::header_has_refcount_v<Header>) {
            if (m_value != nullptr) m_value->first.release();
        }
        m_value = nullptr;
    }

    handle(handle&& other) noexcept
      : m_value(other.m_value) {
        other.m_value = nullptr;
    }

    handle& operator=(handle&& other) noexcept {
        if (this == &other) return *this;
        m_value       = other.m_value;
        other.m_value = nullptr;
        return *this;
    }

    handle(const handle& other)
      : m_value(other.m_value) {
        m_value->first.acquire();
    }

    handle& operator=(const handle& other) {
        if (this == &other) return *this;
        m_value = other.m_value;
        m_value->first.acquire();
        return *this;
    }
public:
    id_type id() const { return m_value->first.id(); }

    bool empty() const { return m_value == nullptr; }

    template <typename ReferenceCount = typename Header::refcount_type>
    ReferenceCount reference_count() const {
        return m_value->first.reference_count();
    }

    pointer       operator->() { return &m_value->second; }
    const_pointer operator->() const { return &m_value->second; }

    reference       operator*() { return m_value->second; }
    const_reference operator*() const { return m_value->second; }

    reference       value() { return m_value->second; }
    const_reference value() const { return m_value->second; }
private:
    pair_type* m_value = nullptr;
};

template <typename Handle>
class handle_container<Handle, std::enable_if_t<!std::is_integral_v<typename Handle::id_type>>> {
private:
    using pair_type = typename Handle::pair_type;
public:
    using value_type  = std::remove_cv_t<std::remove_reference_t<Handle>>;
    using const_value = std::add_const_t<value_type>;

    using id_type = typename Handle::id_type;
public:
    handle_container()  = default;
    ~handle_container() = default;

    handle_container(handle_container&&) noexcept            = default;
    handle_container& operator=(handle_container&&) noexcept = default;

    handle_container(const handle_container&)            = delete;
    handle_container& operator=(const handle_container&) = delete;
public:
    template <typename IdFwd,
        typename... Args,
        typename = std::enable_if_t<std::is_constructible_v<typename pair_type::second_type, Args...>>>
    value_type emplace(IdFwd&& id, Args&&... args) {
        id_type idFwd = std::forward<IdFwd>(id);
        if (auto it = m_pairs.find(idFwd); it != m_pairs.end()) delete it->second;
        auto pair = new pair_type(std::piecewise_construct,
            std::forward_as_tuple(idFwd, 0),
            std::forward_as_tuple(std::forward<Args>(args)...));
        m_pairs.emplace(std::move(idFwd), pair);
        return { pair };
    }

    template <typename IdFwd>
    value_type at(IdFwd&& id) {
        return { m_pairs.at(std::forward<IdFwd>(id)) };
    }

    template <typename IdFwd>
    const_value at(IdFwd&& id) const {
        return { m_pairs.at(std::forward<IdFwd>(id)) };
    }

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
    using pair_type = typename Handle::pair_type;
public:
    using value_type  = std::remove_cv_t<std::remove_reference_t<Handle>>;
    using const_value = std::add_const_t<value_type>;

    using id_type = typename Handle::id_type;
public:
    handle_container()  = default;
    ~handle_container() = default;

    handle_container(handle_container&&) noexcept            = default;
    handle_container& operator=(handle_container&&) noexcept = default;

    handle_container(const handle_container&)            = delete;
    handle_container& operator=(const handle_container&) = delete;
public:
    template <typename... Args,
        typename = std::enable_if_t<std::is_constructible_v<typename pair_type::second_type, Args...>>>
    value_type emplace(id_type id, Args&&... args) {
        if (id >= m_pairs.size()) {
            m_pairs.resize(id + 1, nullptr);
        } else if (m_pairs[id] != nullptr) {
            delete m_pairs[id];
        }

        pair_type* newPair = new pair_type(std::piecewise_construct,
            std::forward_as_tuple(id, 0),
            std::forward_as_tuple(std::forward<Args>(args)...));

        m_pairs[id] = newPair;
        return { newPair };
    }

    template <typename... Args,
        typename = std::enable_if_t<std::is_constructible_v<typename Handle::pair_type::second_type, Args...>>>
    value_type emplace_back(Args&&... args) {
        return emplace(static_cast<id_type>(m_pairs.size()), std::forward<Args>(args)...);
    }

    value_type  at(id_type id) { return { m_pairs.at(id) }; }
    const_value at(id_type id) const { return { m_pairs.at(id) }; }

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
