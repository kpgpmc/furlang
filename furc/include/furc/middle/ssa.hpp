#ifndef FURC_MIDDLE_SSA_HPP
#define FURC_MIDDLE_SSA_HPP

#include "furc/middle/ir.hpp"

namespace furc {

class ssa {
    ssa() = delete;
public:
    static void process(ir_module& mod);
    static void destruct(ir_module& mod);
};

} // namespace furc

#endif // FURC_MIDDLE_SSA_HPP
