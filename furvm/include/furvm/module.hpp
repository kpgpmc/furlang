#ifndef FURVM_MODULE_HPP
#define FURVM_MODULE_HPP

#include "furlang/utility/hash.hpp"
#include "furvm/function.hpp"
#include "furvm/fwd.hpp"
#include "furvm/handle.hpp"

#include <functional>
#include <istream>
#include <ostream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace furvm {

struct mod_type {
    struct array {
        mod_type_id typeId;
        std::size_t size;
    };

    struct imprt {
        mod_id      modId;
        mod_type_id typeId;
    };

    enum type {
        S8 = 0,
        S16,
        S32,
        S64,
        U8,
        U16,
        U32,
        U64,
        Array,

        Import,
        Count,
    } type;
    union value {
        std::nullptr_t null = nullptr;
        array          array;
        imprt          imprt;

        value() = default;

        value(mod_type_id id, std::size_t size)
          : array({}) {
            array.typeId = id;
            array.size   = size;
        }

        template <typename ModIdFwd, typename = std::enable_if_t<std::is_constructible_v<mod_id, ModIdFwd>>>
        value(ModIdFwd&& modId, mod_type_id typeId)
          : imprt({}) {
            imprt.modId  = std::forward<ModIdFwd>(modId);
            imprt.typeId = typeId;
        }

        ~value() {}

        value(value&& other)                 = delete;
        value& operator=(value&& other)      = delete;
        value(const value& other)            = delete;
        value& operator=(const value& other) = delete;
    } value;

    mod_type(enum type type)
      : type(type) {}

    mod_type(mod_type_id id, std::size_t size)
      : type(Array), value(id, size) {}

    template <typename ModIdFwd, typename = std::enable_if_t<std::is_constructible_v<mod_id, ModIdFwd>>>
    mod_type(ModIdFwd&& modId, mod_type_id typeId)
      : type(Import), value(std::forward<ModIdFwd>(modId), typeId) {}

    ~mod_type() {
        switch (type) {
        case Array: value.array.~array(); break;
        case Import: value.imprt.~imprt(); break;
        default: break;
        }
    }

    mod_type(mod_type&& other) noexcept
      : type(other.type) {
        switch (type) {
        case Array: new (&value.array) array(other.value.array); break;
        case Import: new (&value.imprt) imprt(std::move(other.value.imprt)); break;
        default: break;
        }
        other.type = Count;
    }

    mod_type& operator=(mod_type&& other) noexcept {
        if (this == &other) return *this;
        type = other.type;
        switch (type) {
        case Array: new (&value.array) array(other.value.array); break;
        case Import: new (&value.imprt) imprt(std::move(other.value.imprt)); break;
        default: break;
        }
        other.type = Count;
        return *this;
    }

    mod_type(const mod_type& other)
      : type(other.type) {
        switch (type) {
        case Array: new (&value.array) array(other.value.array); break;
        case Import: new (&value.imprt) imprt(other.value.imprt); break;
        default: break;
        }
    }

    mod_type& operator=(const mod_type& other) {
        if (this == &other) return *this;
        type = other.type;
        switch (type) {
        case Array: new (&value.array) array(other.value.array); break;
        case Import: new (&value.imprt) imprt(other.value.imprt); break;
        default: break;
        }
        return *this;
    }
};

class mod {
    friend class function;
    friend class serializer;
public:
    using bytecode_t = std::vector<byte>; /**< An alias to a vector of bytes. */

    static constexpr char MAGIC[4] = { 'F', 'u', 'r', 'M' }; /** Furvm module file magic. */

    using native_function = std::function<void(executor&)>;
public:
    /**
     * @brief Constructs a module.
     *
     * @param name Name of the module.
     * @param args Arguments forwarded to bytecode's constructor.
     */
    template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<bytecode_t, Args...>>>
    mod(Args&&... args)
      : m_bytecode(std::forward<Args>(args)...) {}

    ~mod() = default;

    /**
     * @brief Move constructor.
     */
    mod(mod&&) = default;

    /**
     * @brief Move constructor.
     */
    mod& operator=(mod&&) = default;

    mod(const mod&)            = delete;
    mod& operator=(const mod&) = delete;
public:
    /**
     * @brief Returns a byte from bytecode of this module.
     *
     * @param offset An offset of the byte.
     * @return The byte.
     */
    constexpr byte byte(std::size_t offset) const { return m_bytecode.at(offset); }

    /**
     * @brief Returns the module's bytecode.
     *
     * @return A reference to the bytecode.
     */
    constexpr bytecode_t& bytecode() { return m_bytecode; }

    /**
     * @brief Returns the module's bytecode.
     *
     * @return A constant reference to the bytecode.
     */
    constexpr const bytecode_t& bytecode() const { return m_bytecode; }
public:
    /**
     * @brief Emplaces a function in the module's function container.
     *
     * Emplaces the function in module's function container and name to function map and public functions map.
     *
     * @param args Arguments forwarded into the container's emplace_back function.
     * @return A handle to the emplaced function.
     */
    template <typename... Args>
    function_h emplace_function(Args&&... args) {
        function_h function;
        if constexpr (std::is_constructible_v<class function, Args...>) {
            function = std::move(m_functions.emplace_back(std::forward<Args>(args)...));
        } else {
            function = std::move(m_functions.emplace(std::forward<Args>(args)...));
        }
        return std::move(function);
    }

    /**
     * @brief Emplaces a function in the module's function container.
     *
     * Emplaces the function in module's function container and name to function map.
     *
     * @param name Name of the function.
     * @param args Arguments forwarded into the container's emplace_back function.
     * @return A handle to the emplaced function.
     */
    template <typename NameFwd,
        typename... Args,
        typename = std::enable_if_t<std::is_constructible_v<std::string, NameFwd>>>
    function_h emplace_function(NameFwd&& name, Args&&... args) {
        function_h function;
        if constexpr (std::is_constructible_v<class function, Args...>) {
            function = std::move(m_functions.emplace_back(std::forward<Args>(args)...));
        } else {
            function = std::move(m_functions.emplace(std::forward<Args>(args)...));
        }
        auto pair                       = std::make_pair(std::forward<NameFwd>(name), function->signature());
        m_functionMap[function.id()]    = pair;
        m_functionSigs[std::move(pair)] = function.id();
        return std::move(function);
    }

    /**
     * @brief Returns a function from the module.
     *
     * @param id Identifier of the function.
     * @return A handle to the function.
     */
    auto function_at(function_id id) { return m_functions.at(id); }

    /**
     * @brief Returns a function from the module.
     *
     * @param id Identifier of the function.
     * @return A handle to the function.
     */
    auto function_at(function_id id) const { return m_functions.at(id); }

    /**
     * @brief Returns a function from the module.
     *
     * @param name Name of the function.
     * @return A handle to the function.
     */
    template <typename NameFwd,
        typename SigFwd,
        typename = std::enable_if_t<std::is_constructible_v<std::string, NameFwd> &&
                                    std::is_constructible_v<function_sig, SigFwd>>>
    auto function_at(NameFwd&& name, SigFwd&& signature) {
        return function_at(
            m_functionSigs.at(std::make_pair<>(std::forward<NameFwd>(name), std::forward<SigFwd>(signature))));
    }

    /**
     * @brief Erases a function from the module's function container.
     *
     * @param id Identifier of the function.
     */
    void erase_function(function_id id) {
        m_functions.erase(id);
        if (auto it = m_functionMap.find(id); it != m_functionMap.end()) {
            m_functionSigs.erase(it->second);
            m_functionMap.erase(it);
        }
    }
public:
    template <typename NameFwd, typename Func>
    void set_native_function(NameFwd&& name, Func&& func) {
        m_nativeFunctions.emplace(std::forward<NameFwd>(name), std::forward<Func>(func));
    }

    template <typename NameFwd>
    native_function get_native_function(NameFwd&& name) const {
        return m_nativeFunctions.at(std::forward<NameFwd>(name));
    }
public:
    /**
     * @brief Emplaces a type in the context.
     *
     * @param args Arguments forwarded to the type constructor.
     * @return The emplaced type.
     */
    template <typename... Args>
    auto emplace_type(Args&&... args) {
        if constexpr (std::is_constructible_v<mod_type, Args...>) {
            return m_types.emplace_back(std::forward<Args>(args)...);
        } else {
            return m_types.emplace(std::forward<Args>(args)...);
        }
    }

    /**
     * @brief Returns a type from the context.
     *
     * @param args type's id.
     * @return A handle to the type.
     */
    template <typename... Args>
    auto type_at(Args&&... args) {
        return m_types.at(std::forward<Args>(args)...);
    }

    /**
     * @brief Returns a type from the context.
     *
     * @param args type's id.
     * @return A handle to the type.
     */
    template <typename... Args>
    auto type_at(Args&&... args) const {
        return m_types.at(std::forward<Args>(args)...);
    }

    /**
     * @brief Erases a type from the context.
     *
     * @param args type's id.
     */
    template <typename... Args>
    void erase_type(Args&&... args) {
        m_types.erase(std::forward<Args>(args)...);
    }
public:
    /**
     * @brief Prints the module in a bytecode form to an output stream.
     *
     * @param os Output stream.
     * @return The output stream.
     */
    std::ostream& serialize(std::ostream& os) const;

    /**
     * @brief Loads a module in a bytecode form from an input stream.
     *
     * @param is Input stream.
     * @return The loaded module.
     */
    static mod load(std::istream& is);
private:
    bytecode_t m_bytecode;

    using pair_type = std::pair<std::string, function_sig>;
    using pair_hash =
        furlang::utility::pair_hash<std::string, function_sig, std::hash<std::string>, detail::function_sig_hash>;
    std::unordered_map<pair_type, function_id, pair_hash> m_functionSigs;
    std::unordered_map<function_id, pair_type>            m_functionMap;
    handle_container<function_h>                          m_functions;

    handle_container<mod_type_h> m_types;

    std::unordered_map<std::string, native_function> m_nativeFunctions;
};

} // namespace furvm

#endif // FURVM_MODULE_HPP
