#ifndef FURVM_EXECUTOR_HPP
#define FURVM_EXECUTOR_HPP

#include "furvm/fwd.hpp"

#include <stack>
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
        mod_h       mod;       /**< Shared pointer to a module with the bytecode. */
        std::size_t position;  /**< Cursor to a current instruction in the bytecode. */
        std::size_t stackBase; /**< Snapshot of the stack size before this frame. */

        std::vector<thing_p> variables; /**< Frame variables. */
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
     * @param mod Module.
     * @param function Function handle.
     */
    void push_frame(const mod_h& mod, const function_h& function);

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
     * @brief Pushes a thing onto the stack.
     *
     * @param thing The thing to push.
     */
    void push_thing(const thing_p& thing);

    /**
     * @brief Pushes a thing onto the stack.
     *
     * @param thing The thing to push.
     */
    void push_thing(thing_p&& thing);

    /**
     * @brief Pops a thing from the stack.
     *
     * @return The popped thing.
     */
    thing_p pop_thing();

    /**
     * @brief Returns the top thing on the stack.
     *
     * @return The thing.
     */
    thing_p thing() const;
public:
    /**
     * @brief Stores a thing in a variable.
     *
     * @param variable Variable to store the thing in.
     * @param thing Thing to store.
     */
    void store_thing(variable_t variable, const thing_p& thing);

    /**
     * @brief Stores a thing in a variable.
     *
     * @param variable Variable to store the thing in.
     * @param thing Thing to store.
     */
    void store_thing(variable_t variable, thing_p&& thing);

    /**
     * @brief Returns a thing stored in a variable.
     *
     * @param variable Variable where the thing is stored.
     * @return The thing stored in the variable.
     */
    thing_p load_thing(variable_t variable) const;
public:
    /**
     * @brief Executes next instruction.
     */
    void step();
private:
    executor_flags m_flags{}; // NOLINT(bugprone-invalid-enum-default-initialization)
    context_p      m_context;

    std::stack<struct frame> m_frames;
    std::stack<thing_p>      m_stack;
};

} // namespace furvm

#endif // FURVM_EXECUTOR_HPP
