#ifndef FURVM_DETAIL_HANDLE_HPP
#define FURVM_DETAIL_HANDLE_HPP

#include <type_traits>

namespace furvm {
namespace detail {

/**
 * @brief Default specialization for header_has_refcount type trait.
 */
template <typename Header, typename = void>
struct header_has_refcount : std::false_type {};

/**
 * @brief Specialization for header_has_refcount type trait.
 */
template <typename Header>
struct header_has_refcount<Header,
    std::void_t<decltype(std::declval<Header&>().acquire()),
        decltype(std::declval<Header&>().release()),
        decltype(std::declval<Header&>().reference_count())>> : std::true_type {};

/**
 * @brief An alias for header_has_refcount's value.
 */
template <typename Header>
static constexpr auto header_has_refcount_v = header_has_refcount<Header>::value;

} // namespace detail
} // namespace furvm

#endif // FURVM_DETAIL_HANDLE_HPP
