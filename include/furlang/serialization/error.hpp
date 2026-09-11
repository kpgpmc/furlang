#ifndef FURLANG_SERIALIZATION_ERROR_HPP
#define FURLANG_SERIALIZATION_ERROR_HPP

#include <cstddef>
#include <string>

namespace furlang {
namespace serialization {

enum class error_code {
    EndOfFile,
    InvalidData,
    InvalidTag,
    InvalidVersion,
    IntegerOverflow,
    SizeLimit,
    DuplicateId,
    UnknownId,
    TypeMismatch,
    Unsupported,
};

struct error {
    error_code  code;
    std::string message;

    std::size_t offset = 0;
};

} // namespace serialization
} // namespace furlang

#endif // FURLANG_SERIALIZATION_ERROR_HPP
