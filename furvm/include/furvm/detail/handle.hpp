#ifndef FURVM_DETAIL_HANDLE_HPP
#define FURVM_DETAIL_HANDLE_HPP

#include <type_traits>

namespace furvm {
namespace detail {

template <typename Header, typename = void>
struct header_has_refcount : std::false_type {};

template <typename Header>
struct header_has_refcount<Header,
    std::void_t<decltype(std::declval<Header&>().acquire()),
        decltype(std::declval<Header&>().release()),
        decltype(std::declval<Header&>().reference_count())>> : std::true_type {};

template <typename Header>
static constexpr auto header_has_refcount_v = header_has_refcount<Header>::value;

} // namespace detail
} // namespace furvm

#endif // FURVM_DETAIL_HANDLE_HPP
