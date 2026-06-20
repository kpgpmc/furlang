#ifndef FURVM_DETAIL_SERIALIZATION_HPP
#define FURVM_DETAIL_SERIALIZATION_HPP

#include <cstdint>
#include <ostream>
#include <string>

namespace furvm {
namespace detail {

/**
 * @brief Serializes an integer.
 *
 * @param os Output stream.
 * @param value Integer.
 * @return The output stream.
 */
std::ostream& serialize(std::ostream& os, std::int8_t value);
std::ostream& serialize(std::ostream& os, std::int16_t value);

/**
 * @brief Serializes an integer.
 *
 * @param os Output stream.
 * @param value Integer.
 * @return The output stream.
 */
std::ostream& serialize(std::ostream& os, std::int32_t value);
std::ostream& serialize(std::ostream& os, std::int64_t value);

/**
 * @brief Serializes an integer.
 *
 * @param os Output stream.
 * @param value Integer.
 * @return The output stream.
 */
std::ostream& serialize(std::ostream& os, std::uint8_t value);

/**
 * @brief Serializes an integer.
 *
 * @param os Output stream.
 * @param value Integer.
 * @return The output stream.
 */
std::ostream& serialize(std::ostream& os, std::uint16_t value);

/**
 * @brief Serializes an integer.
 *
 * @param os Output stream.
 * @param value Integer.
 * @return The output stream.
 */
std::ostream& serialize(std::ostream& os, std::uint32_t value);

/**
 * @brief Serializes an integer.
 *
 * @param os Output stream.
 * @param value Integer.
 * @return The output stream.
 */
std::ostream& serialize(std::ostream& os, std::uint64_t value);

/**
 * @brief Serializes a string.
 *
 * @param os Output stream.
 * @param value String.
 * @return The output stream.
 */
std::ostream& serialize(std::ostream& os, const std::string& value);

} // namespace detail
} // namespace furvm

#endif // FURVM_DETAIL_SERIALIZATION_HPP
