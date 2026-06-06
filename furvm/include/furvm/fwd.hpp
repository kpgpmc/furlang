#ifndef FURVM_FWD_HPP
#define FURVM_FWD_HPP

#include <cstddef> // IWYU pragma: export
#include <cstdint> // IWYU pragma: export

/**
 * @brief Furlang's virtual machine.
 */
namespace furvm {

using byte = std::uint8_t;

// constant.hpp

/**
 * @brief Constant index.
 *
 * An index to the constant in module's constant pool.
 */
using constant_index = std::uint16_t;

/**
 * @enum constant_t
 * @brief Constant type.
 */
enum class constant_t : std::uint8_t;

/**
 * @class constant
 * @brief Constant.
 */
class constant;

// instruction.hpp

/**
 * @enum instruction_t
 * @brief Furvm's instruction type.
 */
enum class instruction_t : byte;

/**
 * @struct instruction
 * @brief Furvm's instruction.
 */
struct instruction;

// module.hpp

/**
 * @class mod
 * @brief Module.
 *
 * A furvm module. Translation unit of furlang.
 */
class mod;

/**
 * @brief Furvm module's index.
 */
using module_handle = std::uint32_t;

// context.hpp

/**
 * @class context
 * @brief Context.
 *
 * A furvm context.
 */
class context;

} // namespace furvm

#endif // FURVM_FWD_HPP