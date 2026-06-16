#include "furvm/serializer.hpp"

#include "furvm/function.hpp" // IWYU pragma: keep
#include "furvm/fwd.hpp"
#include "furvm/module.hpp" // IWYU pragma: keep

#include <array>
#include <cstring>
#include <istream>
#include <optional>
#include <ostream>
#include <stdexcept>

namespace furvm {

template <typename T>
static inline void write_raw(std::ostream& os, const T& value) {
    os.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

template <typename T>
static inline void read_raw(std::istream& is, T& value) {
    is.read(reinterpret_cast<char*>(&value), sizeof(T));
}

static inline void write_u64(std::ostream& os, std::uint64_t value) {
    os.put(static_cast<char>((value >> 0ULL) & 0xFF));
    os.put(static_cast<char>((value >> 8ULL) & 0xFF));
    os.put(static_cast<char>((value >> 16ULL) & 0xFF));
    os.put(static_cast<char>((value >> 24ULL) & 0xFF));
    os.put(static_cast<char>((value >> 32ULL) & 0xFF));
    os.put(static_cast<char>((value >> 40ULL) & 0xFF));
    os.put(static_cast<char>((value >> 48ULL) & 0xFF));
    os.put(static_cast<char>((value >> 56ULL) & 0xFF));
}

static inline void read_u64(std::istream& is, std::uint64_t& value) {
    value = (static_cast<std::uint64_t>(is.get()) << 0) | (static_cast<std::uint64_t>(is.get()) << 8) |
            (static_cast<std::uint64_t>(is.get()) << 16) | (static_cast<std::uint64_t>(is.get()) << 24) |
            (static_cast<std::uint64_t>(is.get()) << 32) | (static_cast<std::uint64_t>(is.get()) << 40) |
            (static_cast<std::uint64_t>(is.get()) << 48) | (static_cast<std::uint64_t>(is.get()) << 56);
}

static inline void write_u32(std::ostream& os, std::uint32_t value) {
    os.put(static_cast<char>((value >> 0ULL) & 0xFF));
    os.put(static_cast<char>((value >> 8ULL) & 0xFF));
    os.put(static_cast<char>((value >> 16ULL) & 0xFF));
    os.put(static_cast<char>((value >> 24ULL) & 0xFF));
}

static inline void read_u32(std::istream& is, std::uint32_t& value) {
    value = static_cast<std::uint32_t>(is.get() << 0) | static_cast<std::uint32_t>(is.get() << 8) |
            static_cast<std::uint32_t>(is.get() << 16) | static_cast<std::uint32_t>(is.get() << 24);
}

static inline void write_u16(std::ostream& os, std::uint16_t value) {
    os.put(static_cast<char>((value >> 0ULL) & 0xFF));
    os.put(static_cast<char>((value >> 8ULL) & 0xFF));
}

static inline void read_u16(std::istream& is, std::uint16_t& value) {
    value = static_cast<std::uint16_t>(is.get() << 0) | static_cast<std::uint16_t>(is.get() << 8);
}

static inline void write_u8(std::ostream& os, std::uint8_t value) {
    os.put(static_cast<char>((value >> 0ULL) & 0xFF));
}

static inline void read_u8(std::istream& is, std::uint8_t& value) {
    value = static_cast<std::uint8_t>(is.get() << 0);
}

bool serializer::serialize_module(std::ostream& os, const mod_p& mod) {
    os.write(reinterpret_cast<const char*>(MODULE_MAGIC), sizeof(MODULE_MAGIC));
    write_u32(os, VERSION);

    write_u32(os, mod->m_functions.size());
    for (function_id id = 0; id < static_cast<function_id>(mod->m_functions.size()); ++id) {
        const auto& func = mod->m_functions[id];
        write_u16(os, id);
        write_u8(os, static_cast<std::uint8_t>(func->type()));
        switch (func->type()) {
        case function_t::Normal: {
            write_u64(os, func->position());
        } break;
        case function_t::Native: {
            // TODO: Replace with a reference to the function's name from constant pool
            throw std::runtime_error("cannot serialize native functions yet");
        } break;
        case function_t::Import: {
            // TODO: Replace with a reference to the module's and function's name from constant pool
            throw std::runtime_error("cannot serialize import functions yet");
        } break;
        }
    }

    write_u64(os, mod->m_bytecode.size());
    os.write(reinterpret_cast<const char*>(mod->m_bytecode.data()),
        static_cast<std::streamsize>(mod->m_bytecode.size()));

    return false;
}

} // namespace furvm
