#include "furvm/detail/serialization.hpp"

#include <ostream>

namespace furvm::detail {

std::ostream& serialize(std::ostream& os, std::int8_t value) {
    return os << (char)((value >> 0ULL) & 0xFF);
}

std::ostream& serialize(std::ostream& os, std::int16_t value) {
    return os << (char)((value >> 0ULL) & 0xFF) << (char)((value >> 8ULL) & 0xFF);
}

std::ostream& serialize(std::ostream& os, std::int32_t value) {
    return os << (char)((value >> 0ULL) & 0xFF) << (char)((value >> 8ULL) & 0xFF) << (char)((value >> 16ULL) & 0xFF)
              << (char)((value >> 24ULL) & 0xFF);
}

std::ostream& serialize(std::ostream& os, std::int64_t value) {
    return os << (char)((value >> 0ULL) & 0xFF) << (char)((value >> 8ULL) & 0xFF) << (char)((value >> 16ULL) & 0xFF)
              << (char)((value >> 24ULL) & 0xFF) << (char)((value >> 32ULL) & 0xFF) << (char)((value >> 40ULL) & 0xFF)
              << (char)((value >> 48ULL) & 0xFF) << (char)((value >> 56ULL) & 0xFF);
}

std::ostream& serialize(std::ostream& os, std::uint8_t value) {
    return os << (char)((value >> 0ULL) & 0xFF);
}

std::ostream& serialize(std::ostream& os, std::uint16_t value) {
    return os << (char)((value >> 0ULL) & 0xFF) << (char)((value >> 8ULL) & 0xFF);
}

std::ostream& serialize(std::ostream& os, std::uint32_t value) {
    return os << (char)((value >> 0ULL) & 0xFF) << (char)((value >> 8ULL) & 0xFF) << (char)((value >> 16ULL) & 0xFF)
              << (char)((value >> 24ULL) & 0xFF);
}

std::ostream& serialize(std::ostream& os, std::uint64_t value) {
    return os << (char)((value >> 0ULL) & 0xFF) << (char)((value >> 8ULL) & 0xFF) << (char)((value >> 16ULL) & 0xFF)
              << (char)((value >> 24ULL) & 0xFF) << (char)((value >> 32ULL) & 0xFF) << (char)((value >> 40ULL) & 0xFF)
              << (char)((value >> 48ULL) & 0xFF) << (char)((value >> 56ULL) & 0xFF);
}

std::ostream& serialize(std::ostream& os, const std::string& value) {
    return serialize(os, std::uint16_t(value.size())).write(value.data(), static_cast<std::streamsize>(value.size()));
}

} // namespace furvm::detail
