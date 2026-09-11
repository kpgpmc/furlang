#ifndef FURLANG_SERIALIZATION_CODEC_HPP
#define FURLANG_SERIALIZATION_CODEC_HPP

#include "furlang/result.hpp"
#include "furlang/serialization/io.hpp"

#include <type_traits>
#include <utility>

namespace furlang {
namespace serialization {

template <typename Codec, typename T>
using codec_encode_result_t = decltype(std::declval<Codec>().encode(std::declval<writer&>(), std::declval<const T&>()));

template <typename Codec, typename T>
using codec_decode_result_t = decltype(std::declval<Codec>().decode(std::declval<reader&>()));

template <typename Codec, typename T, typename = void>
struct is_codec : std::false_type {};

template <typename Codec, typename T>
struct is_codec<Codec, T, std::void_t<codec_encode_result_t<Codec, T>, codec_decode_result_t<Codec, T>>>
  : std::true_type {};

template <typename Codec, typename T>
constexpr bool is_codec_v = is_codec<Codec, T>::value;

template <typename Codec, typename T, typename = std::enable_if_t<is_codec_v<Codec, T>>>
result<error> encode(Codec& codec, writer& writer, const T& value) {
    return codec.encode(writer, value);
}

template <typename Codec, typename T, typename = std::enable_if_t<is_codec_v<Codec, T>>>
result<error, T> decode(Codec& codec, reader& reader) {
    return codec.decode(reader);
}

template <typename T, typename = void>
class codec;

template <typename T>
struct codec<T, std::enable_if_t<std::is_integral_v<T>>> {
    result<error>    encode(writer& writer, const T& value) { return writer.write_int(value); }
    result<error, T> decode(reader& reader) { return reader.read_int(T{}); }
};

template <>
struct codec<std::string> {
    result<error> encode(writer& writer, const std::string& value) { return writer.write_string(value); }

    result<error, std::string> decode(reader& reader) { return reader.read_string(); }
};

} // namespace serialization
} // namespace furlang

#endif // FURLANG_SERIALIZATION_CODEC_HPP
