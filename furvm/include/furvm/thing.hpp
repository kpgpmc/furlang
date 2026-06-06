#ifndef FURVM_THING_HPP
#define FURVM_THING_HPP

#include "furvm/fwd.hpp"

#include <memory>

namespace furvm {

enum class thing_t : std::uint8_t {
    Int,
};

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
    thing(rzecz, thing_handle id, thing_t type, const std::shared_ptr<context>& context);

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
private:
    thing_handle             m_id;
    thing_t                  m_type;
    std::shared_ptr<context> m_context;

    void* m_data = nullptr;
};

} // namespace furvm

#endif // FURVM_THING_HPP