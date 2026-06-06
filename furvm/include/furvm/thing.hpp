#ifndef FURVM_THING_HPP
#define FURVM_THING_HPP

#include "furvm/fwd.hpp"

namespace furvm {

enum class thing_t : std::uint8_t {
    Int,
};

/**
 * @brief Returns data size of a thing.
 *
 * @param type Type of the thing.
 * @return The data size of the thing.
 */
std::size_t thing_type_size(thing_t type);

class thing {
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
private:
    thing_handle m_id;
    thing_t      m_type;
    context_p    m_context;

    void* m_data = nullptr;
};

} // namespace furvm

#endif // FURVM_THING_HPP