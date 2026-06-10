#ifndef FURVM_THING_HPP
#define FURVM_THING_HPP

#include "furvm/fwd.hpp"

namespace furvm {

enum class thing_t : std::uint8_t {
    Int32,
};

/**
 * @brief Returns data size of a thing.
 *
 * @param type Type of the thing.
 * @return The data size of the thing.
 */
std::size_t thing_type_size(thing_t type);

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

class thing final {
    friend class executor;
private:
    /**
     * @brief A private token for the private constructor.
     *
     * Also `rzecz` in Polish translates to `thing` I think.
     */
    struct rzecz {
        explicit rzecz() = default;
    };
public:
    using nref_t = std::size_t; /**< Type of reference count. */

    static constexpr thing_handle GENERATION_SIZE = 12; /**< Bit size of generation part in thing_handle. */
public:
    /**
     * @brief Private constructor.
     *
     * @param id
     * @param type
     * @param context
     */
    thing(rzecz, thing_handle id, thing_t type, const context_p& context);

    ~thing();

    /**
     * @brief Move constructor.
     */
    thing(thing&&) noexcept = default;

    /**
     * @brief Move constructor.
     */
    thing& operator=(thing&&) noexcept = default;

    thing(const thing&)            = delete;
    thing& operator=(const thing&) = delete;
public:
    /**
     * @brief Returns a new thing.
     *
     * @param context Furvm context.
     * @param type Type of the new thing.
     * @return Shared pointer to the new thing.
     */
    static thing_p create(const context_p& context, thing_t type);
public:
    /**
     * @brief Returns an int32 value from this thing.
     *
     * @return The value.
     */
    std::int32_t& int32();

    /**
     * @brief Returns an int32 value from this thing.
     *
     * @return The value.
     */
    const std::int32_t& int32() const;
public:
    /**
     * @brief Increments reference count of this thing by one.
     */
    void add_reference() { ++m_refCount; }

    /**
     * @brief Decrements reference count of this thing by one.
     */
    void remove_reference() { --m_refCount; }

    /**
     * @brief Returns reference count of this thing.
     *
     * @return The reference count.
     */
    constexpr nref_t reference_count() const { return m_refCount; }
private:
    thing_handle m_id;
    thing_t      m_type;
    context_p    m_context;

    nref_t m_refCount = 0;
    void*  m_data     = nullptr;
};

} // namespace furvm

#endif // FURVM_THING_HPP