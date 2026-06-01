#include "furlang/arena.hpp"

#include <algorithm>
#include <cstdlib>

namespace furlang {

arena::region* arena::region::create(std::size_t capacity) {
    arena::region* region =
        reinterpret_cast<arena::region*>(std::malloc(sizeof(*region) + (sizeof(value_type) * capacity))); // NOLINT
    if (region == nullptr) return nullptr;
    region->next     = nullptr;
    region->capacity = capacity;
    region->used     = 0;
    return region;
}

arena::arena(std::size_t minCapacity)
  : m_minCapacity(minCapacity) {}

arena::~arena() {
    region* next = nullptr;
    for (region* region = m_head; region != nullptr; region = next) {
        next = region->next;
        std::free(region); // NOLINT
    }
}

arena::arena(arena&& other) noexcept
  : m_minCapacity(other.m_minCapacity), m_head(other.m_head), m_tail(other.m_tail) {
    other.m_minCapacity = 0;
    other.m_head        = nullptr;
    other.m_tail        = nullptr;
}

arena& arena::operator=(arena&& other) noexcept {
    if (this == &other) return *this;
    m_minCapacity       = other.m_minCapacity;
    m_head              = other.m_head;
    m_tail              = other.m_tail;
    other.m_minCapacity = 0;
    other.m_head        = nullptr;
    other.m_tail        = nullptr;
    return *this;
}

void arena::reset() {
    for (region* region = m_head; region != nullptr; region = region->next) {
        region->used = 0;
    }
}

void* arena::allocate(std::size_t size, std::size_t count) {
    std::size_t sizeInBytes = size * count;
    std::size_t sizeInWords = (sizeInBytes + sizeof(region::value_type) - 1) / sizeof(region::value_type);

    if (m_head == nullptr) {
        m_head = m_tail = region::create(std::min(sizeInWords, m_minCapacity));
    }

    region* region = m_head;
    while (region != nullptr && region->free() < sizeInWords)
        region = region->next;

    if (region == nullptr) {
        region = region::create(std::min(sizeInWords, m_minCapacity));
        m_tail = m_tail->next = region;
    }

    uintptr_t* allocated = &region->storage[region->used];
    region->used        += sizeInWords;
    return allocated;
}

} // namespace furlang