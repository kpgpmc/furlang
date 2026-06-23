#ifndef FURC_FRONT_POST_PROCESS_HPP
#define FURC_FRONT_POST_PROCESS_HPP

#include "furlang/ir/module.hpp"

#include <vector>

namespace furc {
namespace front {

/**
 * @brief Post process pipeline.
 */
class post_process {
public:
    enum stage { // NOLINT
        Ssa,
        Sccp,
        Adce,
        DeSsa,
    };
public:
    post_process()  = default;
    ~post_process() = default;

    post_process(post_process&&) noexcept            = default;
    post_process& operator=(post_process&&) noexcept = default;
    post_process(const post_process&)                = delete;
    post_process& operator=(const post_process&)     = delete;
public:
    void push_stage(stage stage) { m_stages.push_back(stage); }
public:
    void process(furlang::ir::mod& mod);
private:
    std::vector<stage> m_stages;
};

} // namespace front
} // namespace furc

#endif // FURC_FRONT_POST_PROCESS_HPP
