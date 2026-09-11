#ifndef FURLANG_VIEW_HPP
#define FURLANG_VIEW_HPP

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace furlang {

template <typename T>
class view {
public:
    constexpr view() noexcept = default;

    constexpr view(const T* data, std::size_t size) noexcept
      : m_data(data), m_size(size) {}
public:
    constexpr view subview(std::size_t offset, std::size_t count = std::numeric_limits<std::size_t>::max()) {
        if (count > 0 && offset >= m_size) throw std::runtime_error("offset too large");
        return { m_data + offset, std::min(m_size - offset, count) };
    }

    const T& operator[](std::size_t offset) const {
        if (offset >= m_size) throw std::runtime_error("out of bounds");
        return m_data[offset];
    }

    constexpr const T*    data() const { return m_data; }
    constexpr std::size_t size() const { return m_size; }
private:
    const T*    m_data = nullptr;
    std::size_t m_size = 0;
};

} // namespace furlang

#endif // FURLANG_VIEW_HPP
