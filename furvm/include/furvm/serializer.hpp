#ifndef FURVM_SERIALIZER_HPP
#define FURVM_SERIALIZER_HPP

#include "furvm/fwd.hpp"

#include <ostream>

namespace furvm {

class serializer {
public:
    static constexpr byte          MODULE_MAGIC[4] = { 'F', 'u', 'r', 'M' };
    static constexpr std::uint32_t VERSION         = 0;
public:
    static bool serialize_module(std::ostream& os, const mod_p& mod);
};

} // namespace furvm

#endif // FURVM_SERIALIZER_HPP
