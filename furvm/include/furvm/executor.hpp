#ifndef FURVM_EXECUTOR_HPP
#define FURVM_EXECUTOR_HPP

#include "furvm/fwd.hpp"
#include "furvm/module.hpp" // IWYU pragma: keep
#include "furvm/thing.hpp"  // IWYU pragma: keep

#include <functional>
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

using executor_callback = std::function<void(executor&)>;

class executor {
    friend class context;
private:
    executor(context* context)
      : m_context(context) {}
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

        thing_type*          returnType; /**< Return type. */
        std::vector<thing<>> variables;  /**< Frame variables. */
    };
public:
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
    template <typename CallbackFwd>
    void set_new_frame_callback(CallbackFwd&& callback) {
        m_newFrameCb = std::forward<CallbackFwd>(callback);
    }
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
    frame top_frame() const;

    const std::stack<frame>& frames() const { return m_frames; }
public:
    /**
     * @brief Pushes a thing onto the stack.
     *
     * Registers a new thing and pushes its handle onto the stack.
     *
     * @param thing Thing.
     * @return The pushed handle.
     */
    thing<>& push_thing(thing<>&& thing);

    thing<>& push_thing(const thing<>& thing);

    /**
     * @brief Pops a thing from the stack.
     *
     * @return A handle to the popped thing.
     */
    thing<> pop_thing();

    /**
     * @brief Returns the top thing on the stack.
     *
     * @return A handle to the top thing.
     */
    thing<>& top_thing();

    const thing<>& top_thing() const;
public:
    /**
     * @brief Stores a thing in a frame variable.
     *
     * @param variable Id of the variable in which the handle will be put.
     * @param thing Thing handle.
     */
    void store_thing(variable_t variable, const thing<>& thing);

    /**
     * @brief Stores a thing in a frame variable.
     *
     * @param variable Id of the variable in which the handle will be put.
     * @param thing Thing handle.
     */
    void store_thing(variable_t variable, thing<>&& thing);

    /**
     * @brief Returns a thing stored in a variable.
     *
     * @param variable Id of the variable from which the handle will be fetched.
     * @return A handle stored in the variable.
     */
    thing<>& load_thing(variable_t variable);

    const thing<>& load_thing(variable_t variable) const;
public:
    /**
     * @brief Executes next instruction.
     */
    void step();
private:
    thing_type thing_type_impl(mod_h mod, mod_type type) const;

    thing_type* mod_to_thing_type(const mod_h& mod, const mod_type& type) const;

    thing<> make_reference(const thing<>& thing) const;
private:
    executor_flags m_flags{}; // NOLINT(bugprone-invalid-enum-default-initialization)
    context*       m_context;

    std::stack<frame>   m_frames;
    std::stack<thing<>> m_stack;

    executor_callback m_newFrameCb = nullptr;
};

} // namespace furvm

#endif // FURVM_EXECUTOR_HPP
