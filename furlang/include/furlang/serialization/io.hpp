#ifndef FURLANG_SERIALIZATION_IO_HPP
#define FURLANG_SERIALIZATION_IO_HPP

#include "furlang/result.hpp"
#include "furlang/serialization/error.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace furlang {
namespace serialization {

class writer {
public:
    writer()          = default;
    virtual ~writer() = default;

    writer(writer&&) noexcept            = default;
    writer& operator=(writer&&) noexcept = default;
    writer(const writer&)                = default;
    writer& operator=(const writer&)     = default;
public:
    virtual result<error> write_s8(std::int8_t value)    = 0;
    virtual result<error> write_u8(std::uint8_t value)   = 0;
    virtual result<error> write_s16(std::int16_t value)  = 0;
    virtual result<error> write_u16(std::uint16_t value) = 0;
    virtual result<error> write_s32(std::int32_t value)  = 0;
    virtual result<error> write_u32(std::uint32_t value) = 0;
    virtual result<error> write_s64(std::int64_t value)  = 0;
    virtual result<error> write_u64(std::uint64_t value) = 0;

    result<error> write_int(std::int8_t value) { return write_s8(value); }
    result<error> write_int(std::uint8_t value) { return write_u8(value); }
    result<error> write_int(std::int16_t value) { return write_s16(value); }
    result<error> write_int(std::uint16_t value) { return write_u16(value); }
    result<error> write_int(std::int32_t value) { return write_s32(value); }
    result<error> write_int(std::uint32_t value) { return write_u32(value); }
    result<error> write_int(std::int64_t value) { return write_s64(value); }
    result<error> write_int(std::uint64_t value) { return write_u64(value); }

    virtual result<error> write_string(const char* string)        = 0;
    virtual result<error> write_string(std::string_view string)   = 0;
    virtual result<error> write_string(const std::string& string) = 0;
};

class reader {
public:
    reader()          = default;
    virtual ~reader() = default;

    reader(reader&&) noexcept            = default;
    reader& operator=(reader&&) noexcept = default;
    reader(const reader&)                = default;
    reader& operator=(const reader&)     = default;
public:
    virtual result<error, std::int8_t>   read_s8()  = 0;
    virtual result<error, std::uint8_t>  read_u8()  = 0;
    virtual result<error, std::int16_t>  read_s16() = 0;
    virtual result<error, std::uint16_t> read_u16() = 0;
    virtual result<error, std::int32_t>  read_s32() = 0;
    virtual result<error, std::uint32_t> read_u32() = 0;
    virtual result<error, std::int64_t>  read_s64() = 0;
    virtual result<error, std::uint64_t> read_u64() = 0;

    result<error, std::int8_t>   read_int(std::int8_t) { return read_s8(); }
    result<error, std::uint8_t>  read_int(std::uint8_t) { return read_u8(); }
    result<error, std::int16_t>  read_int(std::int16_t) { return read_s16(); }
    result<error, std::uint16_t> read_int(std::uint16_t) { return read_u16(); }
    result<error, std::int32_t>  read_int(std::int32_t) { return read_s32(); }
    result<error, std::uint32_t> read_int(std::uint32_t) { return read_u32(); }
    result<error, std::int64_t>  read_int(std::int64_t) { return read_s64(); }
    result<error, std::uint64_t> read_int(std::uint64_t) { return read_u64(); }

    virtual result<error, std::string> read_string() = 0;

    virtual std::size_t offset() const = 0;
};

enum class endianness {
    Little = 0,
    Big    = 1,
};

class byte_writer : public writer {
public:
    byte_writer(endianness endianness = endianness::Big)
      : m_endianness(endianness) {}
public:
    result<error> write_s8(std::int8_t value) override;
    result<error> write_u8(std::uint8_t value) override;
    result<error> write_s16(std::int16_t value) override;
    result<error> write_u16(std::uint16_t value) override;
    result<error> write_s32(std::int32_t value) override;
    result<error> write_u32(std::uint32_t value) override;
    result<error> write_s64(std::int64_t value) override;
    result<error> write_u64(std::uint64_t value) override;

    result<error> write_string(const char* string) override;
    result<error> write_string(std::string_view string) override;
    result<error> write_string(const std::string& string) override;
private:
    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    result<error> write_integral_le(T value) {
        return write_integral_le(value, std::make_index_sequence<sizeof(T)>{});
    }

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>, std::size_t... I>
    result<error> write_integral_le(T value, std::index_sequence<I...>) {
        auto usig = static_cast<std::make_unsigned_t<T>>(value);
        (m_bytes.push_back(usig >> (I * 8)), ...);
        return {};
    }
private:
    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    void write_integral_be(T value) {
        write_integral_be(value, std::make_index_sequence<sizeof(T)>{});
    }

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>, std::size_t... I>
    void write_integral_be(T value, std::index_sequence<I...>) {
        auto usig = static_cast<std::make_unsigned_t<T>>(value);
        (m_bytes.push_back(usig >> ((sizeof(T) - 1 - I) * 8)), ...);
    }
private:
    endianness                m_endianness;
    std::vector<std::uint8_t> m_bytes;
};

class byte_reader : public reader {
public:
    byte_reader(const std::uint8_t* bytes, std::size_t length, endianness endianness = endianness::Big)
      : m_endianness(endianness), m_bytes(bytes), m_length(length) {}
public:
    result<error, std::int8_t>   read_s8() override;
    result<error, std::uint8_t>  read_u8() override;
    result<error, std::int16_t>  read_s16() override;
    result<error, std::uint16_t> read_u16() override;
    result<error, std::int32_t>  read_s32() override;
    result<error, std::uint32_t> read_u32() override;
    result<error, std::int64_t>  read_s64() override;
    result<error, std::uint64_t> read_u64() override;

    result<error, std::string> read_string() override;

    std::size_t offset() const override;
private:
    result<error, std::uint8_t> read_byte() {
        if (m_offset >= m_length)
            return result<error, std::uint8_t>::error(error{ error_code::EndOfFile, "", m_offset });
        return { m_bytes[m_offset++] };
    }
private:
    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    result<error, T> read_integral_le() {
        using U = std::make_unsigned_t<T>;
        U usig  = 0;
        for (std::size_t i = 0; i < sizeof(T); ++i) {
            auto res = read_byte();
            if (res.has_error()) return res;
            usig |= static_cast<U>(res.value()) << (i * 8);
        }
        return { static_cast<T>(usig) };
    }
private:
    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    result<error, T> read_integral_be() {
        using U = std::make_unsigned_t<T>;
        U usig  = 0;
        for (std::size_t i = 0; i < sizeof(T); ++i) {
            auto res = read_byte();
            if (res.has_error()) return res;
            usig |= static_cast<U>(res.value()) << ((sizeof(T) - 1 - i) * 8);
        }
        return { static_cast<T>(usig) };
    }
private:
    endianness          m_endianness;
    const std::uint8_t* m_bytes;
    std::size_t         m_length;
    std::size_t         m_offset = 0;
};

} // namespace serialization
} // namespace furlang

#endif // FURLANG_SERIALIZATION_IO_HPP
