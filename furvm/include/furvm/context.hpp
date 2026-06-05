#ifndef FURVM_CONTEXT_HPP
#define FURVM_CONTEXT_HPP

namespace furvm {

/**
 * @brief Context.
 *
 * A furvm context.
 */
class context {
public:
    context()  = default;
    ~context() = default;

    /**
     * @brief Move constructor.
     */
    context(context&&) = default;

    /**
     * @brief Move constructor.
     */
    context& operator=(context&&) = default;

    context(const context&)            = delete;
    context& operator=(const context&) = delete;
private:
};

} // namespace furvm

#endif // FURVM_CONTEXT_HPP