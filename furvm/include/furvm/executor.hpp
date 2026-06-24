#ifndef FURVM_EXECUTOR_HPP
#define FURVM_EXECUTOR_HPP

#include "furvm/fwd.hpp"
#include "furvm/module.hpp" // IWYU pragma: keep
#include "furvm/thing.hpp"  // IWYU pragma: keep

#include <stack>
#include <utility>
#include <vector>

namespace furvm {

enum class executor_flags : std::uint32_t {
    Suspended = (1 << 0), /**< Execution suspended. */
    Done      = (1 << 1), /**< Execution is finished. */
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
public:
    /**
     * @brief Executor frame.
     *
     * Call frame.
     */
    struct frame {
        mod_h       mod;       /**< Handle to the frame's module. */
        std::size_t position;  /**< Cursor to a current instruction in the bytecode. */
        std::size_t stackBase; /**< Snapshot of the stack size before this frame. */

        std::vector<thing_h> variables; /**< Frame variables. */
    };
public:
    /**
     * @brief Returns a new executor.
     *
     * @param context Context.
     */
    executor(const context_p& context)
      : m_context(context) {}

    ~executor() = default;

    /**
     * @brief Move constructor.
     */
    executor(executor&&) noexcept = default;

    /**
     * @brief Move constructor.
     */
    executor& operator=(executor&&) noexcept = default;

    /**
     * @brief Copy constructor.
     */
    executor(const executor&) = default;

    /**
     * @brief Copy constructor.
     */
    executor& operator=(const executor&) = default;
public:
    /**
     * @brief Returns flags of this executor.
     *
     * @return The flags.
     */
    executor_flags flags() const { return m_flags; }
public:
    /**
     * @brief Pushes a new frame.
     *
     * @param mod Handle to the frame's module.
     * @param function Frame's function.
     */
    void push_frame(const mod_h& mod, function function);

    /**
     * @brief Pops the top frame.
     *
     * @return The popped frame.
     */
    frame pop_frame();

    /**
     * @brief Returns the top frame.
     *
     * @return The frame.
     */
    frame frame() const;
public:
    /**
     * @brief Pushes a thing handle onto the stack.
     *
     * @param handle Thing handle.
     */
    template <typename HandleFwd>
    void push_thing(HandleFwd&& handle) {
        m_stack.emplace(std::forward<HandleFwd>(handle));
    }

    /**
     * @brief Pushes a thing onto the stack.
     *
     * Registers a new thing and pushes its handle onto the stack.
     *
     * @param thing Thing.
     * @return The pushed handle.
     */
    thing_h push_thing(class thing<>&& thing);

    /**
     * @brief Pops a thing from the stack.
     *
     * @return A handle to the popped thing.
     */
    thing_h pop_thing();

    /**
     * @brief Returns the top thing on the stack.
     *
     * @return A handle to the top thing.
     */
    thing_h thing() const;
public:
    /**
     * @brief Stores a thing in a frame variable.
     *
     * @param variable Id of the variable in which the handle will be put.
     * @param thing Thing handle.
     */
    void store_thing(variable_t variable, const thing_h& thing);

    /**
     * @brief Stores a thing in a frame variable.
     *
     * @param variable Id of the variable in which the handle will be put.
     * @param thing Thing handle.
     */
    void store_thing(variable_t variable, thing_h&& thing);

    /**
     * @brief Returns a thing stored in a variable.
     *
     * @param variable Id of the variable from which the handle will be fetched.
     * @return A handle stored in the variable.
     */
    thing_h load_thing(variable_t variable) const;
public:
    /**
     * @brief Executes next instruction.
     */
    void step();
private:
    executor_flags m_flags{}; // NOLINT(bugprone-invalid-enum-default-initialization)
    context_p      m_context;

    std::stack<struct frame> m_frames;
    std::stack<thing_h>      m_stack;
};

} // namespace furvm

#endif // FURVM_EXECUTOR_HPP
