#ifndef FURVM_THING_HPP
#define FURVM_THING_HPP

#include "furvm/context.hpp" // IWYU pragma: keep
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
public:
    using nref_t = std::size_t; /**< Type of reference count. */

    static constexpr thing_id GENERATION_SIZE = 12; /**< Bit size of generation part in thing_handle. */
public:
    thing(const context_p& context, thing_t type);

    ~thing();

    /**
     * @brief Move constructor.
     */
    thing(thing&&) noexcept;

    /**
     * @brief Move constructor.
     */
    thing& operator=(thing&&) noexcept;

    thing(const thing&)            = delete;
    thing& operator=(const thing&) = delete;
public:
    /**
     * @brief Adds two things together.
     *
     * @param lhs Left-hand-side thing.
     * @param rhs Right-hand-side thing.
     * @return Shared pointer to result thing.
     */
    friend thing_p operator+(const thing_p& lhs, const thing_p& rhs);

    /**
     * @brief Subtracts two things together.
     *
     * @param lhs Left-hand-side thing.
     * @param rhs Right-hand-side thing.
     * @return Shared pointer to result thing.
     */
    friend thing_p operator-(const thing_p& lhs, const thing_p& rhs);

    /**
     * @brief Multiplies two things together.
     *
     * @param lhs Left-hand-side thing.
     * @param rhs Right-hand-side thing.
     * @return Shared pointer to result thing.
     */
    friend thing_p operator*(const thing_p& lhs, const thing_p& rhs);

    /**
     * @brief Divides two things together.
     *
     * @param lhs Left-hand-side thing.
     * @param rhs Right-hand-side thing.
     * @return Shared pointer to result thing.
     */
    friend thing_p operator/(const thing_p& lhs, const thing_p& rhs);

    /**
     * @brief Modulos two things together.
     *
     * @param lhs Left-hand-side thing.
     * @param rhs Right-hand-side thing.
     * @return Shared pointer to result thing.
     */
    friend thing_p operator%(const thing_p& lhs, const thing_p& rhs);

    /**
     * @brief Compares two things for equality.
     *
     * @param lhs Left-hand-side thing.
     * @param rhs Right-hand-side thing.
     * @return Shared pointer to result thing.
     */
    friend thing_p operator==(const thing_p& lhs, const thing_p& rhs);

    /**
     * @brief Compares two things for equality.
     *
     * @param lhs Left-hand-side thing.
     * @param rhs Right-hand-side thing.
     * @return Shared pointer to result thing.
     */
    friend thing_p operator!=(const thing_p& lhs, const thing_p& rhs);
    /**
     * @brief Compares if the left thing is less than the right thing.
     *
     * @param lhs Left-hand-side thing.
     * @param rhs Right-hand-side thing.
     * @return Shared pointer to result thing.
     */
    friend thing_p operator<(const thing_p& lhs, const thing_p& rhs);

    /**
     * @brief Compares if the left thing is greater than the right thing.
     *
     * @param lhs Left-hand-side thing.
     * @param rhs Right-hand-side thing.
     * @return Shared pointer to result thing.
     */
    friend thing_p operator>(const thing_p& lhs, const thing_p& rhs);

    /**
     * @brief Compares if the left thing is less than or equal to the right thing.
     *
     * @param lhs Left-hand-side thing.
     * @param rhs Right-hand-side thing.
     * @return Shared pointer to result thing.
     */
    friend thing_p operator<=(const thing_p& lhs, const thing_p& rhs);

    /**
     * @brief Compares if the left thing is greater than or equal to the right thing.
     *
     * @param lhs Left-hand-side thing.
     * @param rhs Right-hand-side thing.
     * @return Shared pointer to result thing.
     */
    friend thing_p operator>=(const thing_p& lhs, const thing_p& rhs);
public:
    /**
     * @brief Returns a clone of the thing.
     *
     * @param thing Thing to clone.
     * @return Shared pointer to a clone of the thing.
     */
    static thing_p clone(const thing_p& thing);
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
    context_p m_context;
    thing_t   m_type;

    nref_t m_refCount = 0;
    void*  m_data     = nullptr;
};

} // namespace furvm

#endif // FURVM_THING_HPP
