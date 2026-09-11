#ifndef FURC_BACK_FURVM_HPP
#define FURC_BACK_FURVM_HPP

#include "furc/middle/ir.hpp"
#include "furvm/module.hpp"

namespace furc {

class furvm_generator final {
public:
    static furvm::mod generate(const ir_module& mod);
};

} // namespace furc

#endif // FURC_BACK_FURVM_HPP
