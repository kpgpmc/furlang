#include "furc/back/furvm/target.hpp"

#include "furvm/function.hpp"

#include <stdexcept>

namespace furc {

furvm::mod furvm_generator::generate(const ir_module& mod) {
    if (!mod.variables.empty()) throw std::runtime_error("global variables are not supported for furvm yet");

    furvm::mod furvmMod;
    for (const auto& func : mod.functions) {
        furvmMod.emplace_function(furvm::function_sig{ {}, {} }, furvmMod.bytecode().size());
    }

    return furvmMod;
}

} // namespace furc
