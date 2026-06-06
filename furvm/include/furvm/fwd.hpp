#ifndef FURVM_FWD_HPP
#define FURVM_FWD_HPP

#include <cstddef> // IWYU pragma: export
#include <cstdint> // IWYU pragma: export
#include <memory>

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
 * @brief An alias to a module shared pointer.
 */
using mod_p = std::shared_ptr<mod>;

/**
 * @brief Furvm module's index.
 */
using module_handle = std::uint32_t;

// thing.hpp

/**
 * @enum thing_t
 * @brief Thing type.
 */
enum class thing_t : std::uint8_t;

/**
 * @class bad_thing_access
 * @brief Bad thing access exception.
 */
class bad_thing_access;

/**
 * @class thing
 * @brief Furvm thing.
 *
 * A stack element. Think of it like of a value in C++ or I guess a class in java.
 */
class thing;

/**
 * @brief An alias to a thing shared pointer.
 */
using thing_p = std::shared_ptr<thing>;

/**
 * @brief Furvm thing's index.
 */
using thing_handle = std::uint32_t;

// executor.hpp

/**
 * @enum executor_flags
 * @brief Flags of an executor.
 */
enum class executor_flags : std::uint32_t;

/**
 * @class executor
 * @brief Furvm executor.
 *
 * Furvm executors are like threads.
 */
class executor;

/**
 * @brief An alias to a executor shared pointer.
 */
using executor_p = std::shared_ptr<executor>;

/**
 * @brief Furvm executor's index.
 */
using executor_handle = std::uint32_t;

// context.hpp

/**
 * @class context
 * @brief Context.
 *
 * A furvm context.
 */
class context;

/**
 * @brief An alias to a context shared pointer.
 */
using context_p = std::shared_ptr<context>;

} // namespace furvm

#endif // FURVM_FWD_HPP