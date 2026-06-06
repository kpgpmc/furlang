#ifndef FURVM_EXECUTOR_HPP
#define FURVM_EXECUTOR_HPP

#include "furvm/fwd.hpp"

#include <memory>

namespace furvm {

enum class executor_flags : std::uint32_t {
    Suspended = (1 << 0), /**< Execution suspended. */
};

static inline executor_flags operator|(executor_flags lhs, executor_flags rhs) {
    return executor_flags(static_cast<std::uint32_t>(lhs) | static_cast<std::uint32_t>(rhs));
}

static inline executor_flags operator&(executor_flags lhs, executor_flags rhs) {
    return executor_flags(static_cast<std::uint32_t>(lhs) & static_cast<std::uint32_t>(rhs));
}

static inline executor_flags operator~(executor_flags flags) {
    return executor_flags(~static_cast<std::uint32_t>(flags));
}

class executor {
private:
    /**
     * @brief A private token for the private constructor.
     *
     * Also `egzekutor` in Polish translates to `executor` I think.
     */
    struct egzekutor {
        explicit egzekutor() = default;
    };
public:
    /**
     * @brief Private constructor.
     *
     * @param id
     * @param context
     */
    executor(egzekutor, executor_handle id, const std::shared_ptr<context>& context);

    ~executor();

    /**
     * @brief Move constructor.
     */
    executor(executor&&) = default;

    /**
     * @brief Move constructor.
     */
    executor& operator=(executor&&) = default;

    executor(const executor&)            = delete;
    executor& operator=(const executor&) = delete;
public:
    /**
     * @brief Returns a new executor.
     *
     * @param context Furvm context.
     */
    static std::shared_ptr<executor> create(const std::shared_ptr<context>& context);
public:
    /**
     * @brief Returns an id of this executor.
     *
     * @return The id.
     */
    executor_handle id() const { return m_id; }

    /**
     * @brief Returns flags of this executor.
     *
     * @return The flags.
     */
    executor_flags flags() const { return m_flags; }
private:
    executor_handle          m_id;
    executor_flags           m_flags{}; // NOLINT(bugprone-invalid-enum-default-initialization)
    std::shared_ptr<context> m_context;
};

} // namespace furvm

#endif // FURVM_EXECUTOR_HPP