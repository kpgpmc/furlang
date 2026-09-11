#include "furlang/serialization/io.hpp"

#include <cstring>

namespace furlang::serialization {

result<error> byte_writer::write_s8(std::int8_t value) {
    if (m_endianness == endianness::Little)
        write_integral_le(value);
    else
        write_integral_be(value);
    return {};
}

result<error> byte_writer::write_u8(std::uint8_t value) {
    if (m_endianness == endianness::Little)
        write_integral_le(value);
    else
        write_integral_be(value);
    return {};
}

result<error> byte_writer::write_s16(std::int16_t value) {
    if (m_endianness == endianness::Little)
        write_integral_le(value);
    else
        write_integral_be(value);
    return {};
}

result<error> byte_writer::write_u16(std::uint16_t value) {
    if (m_endianness == endianness::Little)
        write_integral_le(value);
    else
        write_integral_be(value);
    return {};
}

result<error> byte_writer::write_s32(std::int32_t value) {
    if (m_endianness == endianness::Little)
        write_integral_le(value);
    else
        write_integral_be(value);
    return {};
}

result<error> byte_writer::write_u32(std::uint32_t value) {
    if (m_endianness == endianness::Little)
        write_integral_le(value);
    else
        write_integral_be(value);
    return {};
}

result<error> byte_writer::write_s64(std::int64_t value) {
    if (m_endianness == endianness::Little)
        write_integral_le(value);
    else
        write_integral_be(value);
    return {};
}

result<error> byte_writer::write_u64(std::uint64_t value) {
    if (m_endianness == endianness::Little)
        write_integral_le(value);
    else
        write_integral_be(value);
    return {};
}

result<error> byte_writer::write_string(const char* string) {
    write_u16(std::strlen(string));
    m_bytes.insert(m_bytes.end(), string, string + std::strlen(string));
    return {};
}

result<error> byte_writer::write_string(std::string_view string) {
    write_u16(string.length());
    m_bytes.insert(m_bytes.end(), string.begin(), string.end());
    return {};
}

result<error> byte_writer::write_string(const std::string& string) {
    write_u16(string.length());
    m_bytes.insert(m_bytes.end(), string.begin(), string.end());
    return {};
}

result<error, std::int8_t> byte_reader::read_s8() {
    return m_endianness == endianness::Little ? read_integral_le<std::int8_t>() : read_integral_be<std::int8_t>();
}

result<error, std::uint8_t> byte_reader::read_u8() {
    return m_endianness == endianness::Little ? read_integral_le<std::uint8_t>() : read_integral_be<std::uint8_t>();
}

result<error, std::int16_t> byte_reader::read_s16() {
    return m_endianness == endianness::Little ? read_integral_le<std::int16_t>() : read_integral_be<std::int16_t>();
}

result<error, std::uint16_t> byte_reader::read_u16() {
    return m_endianness == endianness::Little ? read_integral_le<std::uint16_t>() : read_integral_be<std::uint16_t>();
}

result<error, std::int32_t> byte_reader::read_s32() {
    return m_endianness == endianness::Little ? read_integral_le<std::int32_t>() : read_integral_be<std::int32_t>();
}

result<error, std::uint32_t> byte_reader::read_u32() {
    return m_endianness == endianness::Little ? read_integral_le<std::uint32_t>() : read_integral_be<std::uint32_t>();
}

result<error, std::int64_t> byte_reader::read_s64() {
    return m_endianness == endianness::Little ? read_integral_le<std::int64_t>() : read_integral_be<std::int64_t>();
}

result<error, std::uint64_t> byte_reader::read_u64() {
    return m_endianness == endianness::Little ? read_integral_le<std::uint64_t>() : read_integral_be<std::uint64_t>();
}

result<error, std::string> byte_reader::read_string() {
    std::string str;
    auto        length = read_u16();
    if (length.has_error()) return length;
    str.resize(length.value());
    if (m_offset + str.length() >= m_length)
        return result<error, std::string>::error(error{ error_code::EndOfFile, "", m_length });
    str.assign(reinterpret_cast<const char*>(m_bytes) + m_offset, str.length());
    m_offset += str.length();
    return str;
}

std::size_t byte_reader::offset() const {
    return m_offset;
}

} // namespace furlang::serialization
