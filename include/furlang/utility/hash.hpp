#ifndef FURLANG_UTILITY_HASH_HPP
#define FURLANG_UTILITY_HASH_HPP

#include <cstddef>
#include <functional>

namespace furlang {
namespace utility {

// Source - https://stackoverflow.com/a/27952689
// Posted by Yakk - Adam Nevraumont, modified by community. See post 'Timeline' for change history
// Retrieved 2026-07-07, License - CC BY-SA 4.0
static inline std::size_t hash_combine(std::size_t lhs, std::size_t rhs) {
    if constexpr (sizeof(std::size_t) >= 8) {
        lhs ^= rhs + 0x517cc1b727220a95 + (lhs << 6) + (lhs >> 2);
    } else {
        lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    }

    return lhs;
}

// Source - https://stackoverflow.com/a/20602159
// Posted by Casey, modified by community. See post 'Timeline' for change history
// Retrieved 2026-07-07, License - CC BY-SA 3.0
template <typename T, typename U, typename FirstHash = std::hash<T>, typename SecondHash = std::hash<U>>
struct pair_hash {
    std::size_t operator()(const std::pair<T, U>& pair) const {
        return hash_combine(FirstHash()(pair.first), SecondHash()(pair.second));
    }
};

template <typename T, typename Hash = std::hash<T>>
struct vector_hash {
    std::size_t operator()(const std::vector<T>& vec) const {
        std::size_t seed = 0;
        for (const auto& element : vec) {
            hash_combine(seed, Hash()(element));
        }
        return seed;
    }
};

} // namespace utility
} // namespace furlang

#endif // FURLANG_UTILITY_HASH_HPP
