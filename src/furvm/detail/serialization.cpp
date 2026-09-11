#include "furvm/detail/serialization.hpp"

#include <istream>
#include <ostream>

namespace furvm::detail {

namespace {

template <typename T, std::size_t... I>
void serialize_integral_impl(std::ostream& os, std::make_unsigned_t<T> u_value, std::index_sequence<I...>) {
    ((os << static_cast<char>((u_value >> (I * 8)) & 0xFF)), ...);
}

template <typename T, std::size_t... I>
void load_integral_impl(const char* bs, std::make_unsigned_t<T>& u_value, std::index_sequence<I...>) {
    using UnsignedT = std::make_unsigned_t<T>;
    ((u_value |= (static_cast<UnsignedT>(static_cast<std::uint8_t>(bs[I])) << (I * 8))), ...);
}

template <typename T>
std::ostream& serialize_integral(std::ostream& os, T value) {
    static_assert(std::is_integral_v<T>, "Type must be an integral type.");
    using UnsignedT = std::make_unsigned_t<T>;

    serialize_integral_impl<T>(os, static_cast<UnsignedT>(value), std::make_index_sequence<sizeof(T)>{});
    return os;
}

template <typename T>
std::istream& load_integral(std::istream& is, T& value) {
    static_assert(std::is_integral_v<T>, "Type must be an integral type.");

    char bs[sizeof(T)];
    if (!is.read(bs, sizeof(T))) {
        return is;
    }

    using UnsignedT = std::make_unsigned_t<T>;
    UnsignedT uVal  = 0;

    load_integral_impl<T>(bs, uVal, std::make_index_sequence<sizeof(T)>{});

    value = static_cast<T>(uVal);
    return is;
}

} // namespace

std::ostream& serialize(std::ostream& os, std::int8_t value) {
    return serialize_integral(os, value);
}

std::ostream& serialize(std::ostream& os, std::int16_t value) {
    return serialize_integral(os, value);
}

std::ostream& serialize(std::ostream& os, std::int32_t value) {
    return serialize_integral(os, value);
}

std::ostream& serialize(std::ostream& os, std::int64_t value) {
    return serialize_integral(os, value);
}

std::ostream& serialize(std::ostream& os, std::uint8_t value) {
    return serialize_integral(os, value);
}

std::ostream& serialize(std::ostream& os, std::uint16_t value) {
    return serialize_integral(os, value);
}

std::ostream& serialize(std::ostream& os, std::uint32_t value) {
    return serialize_integral(os, value);
}

std::ostream& serialize(std::ostream& os, std::uint64_t value) {
    return serialize_integral(os, value);
}

std::ostream& serialize(std::ostream& os, const std::string& value) {
    return serialize(os, static_cast<std::uint16_t>(value.size()))
        .write(value.data(), static_cast<std::streamsize>(value.size()));
}

std::istream& load(std::istream& is, std::int8_t& value) {
    return load_integral(is, value);
}

std::istream& load(std::istream& is, std::int16_t& value) {
    return load_integral(is, value);
}

std::istream& load(std::istream& is, std::int32_t& value) {
    return load_integral(is, value);
}

std::istream& load(std::istream& is, std::int64_t& value) {
    return load_integral(is, value);
}

std::istream& load(std::istream& is, std::uint8_t& value) {
    return load_integral(is, value);
}

std::istream& load(std::istream& is, std::uint16_t& value) {
    return load_integral(is, value);
}

std::istream& load(std::istream& is, std::uint32_t& value) {
    return load_integral(is, value);
}

std::istream& load(std::istream& is, std::uint64_t& value) {
    return load_integral(is, value);
}

std::istream& load(std::istream& is, std::string& value) {
    std::uint16_t length = 0;
    load(is, length);

    value.resize(length);
    return is.read(value.data(), static_cast<std::streamsize>(length));
}

} // namespace furvm::detail
