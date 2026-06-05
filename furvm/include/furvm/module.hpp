#ifndef FURVM_MODULE_HPP
#define FURVM_MODULE_HPP

namespace furvm {

class mod {
public:
    mod()  = default;
    ~mod() = default;

    /**
     * @brief Move constructor.
     */
    mod(mod&&) = default;

    /**
     * @brief Move constructor.
     */
    mod& operator=(mod&&) = default;

    mod(const mod&)            = delete;
    mod& operator=(const mod&) = delete;
private:
};

} // namespace furvm

#endif // FURVM_MODULE_HPP