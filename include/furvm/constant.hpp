#ifndef FURVM_CONSTANT_HPP
#define FURVM_CONSTANT_HPP

#include "furvm/fwd.hpp"

#include <cstdint>
#include <string_view>

namespace furvm {

// TODO: Array constants
struct constant {
    enum type_e {
        S32 = 0,
        U32,
        S64,
        U64,
        String,
    } type = S32;
    union {
        std::int32_t     s32;
        std::uint32_t    u32;
        std::int64_t     s64;
        std::uint64_t    u64;
        std::string_view string;
    };
};

} // namespace furvm

#endif // FURVM_CONSTANT_HPP
