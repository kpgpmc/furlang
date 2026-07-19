#include "furlang/arena.hpp"
#include "furvm/furvm.hpp"
#include "furvm/thing.hpp"
#include "furvm/thing_allocator.hpp"

#include "gtest/gtest.h" // IWYU pragma: keep

namespace {

// TODO: Basic program tests (e.g. for loops)

TEST(Things, Ops) {
    furlang::arena                    arena;
    furvm::thing_allocator<std::byte> alloc{ arena };

    furvm::thing lhs{ furvm::thing_type{ furvm::thing_type::U32 }, alloc };
    lhs.get<furvm::thing_type::u32>() = 6;
    furvm::thing rhs{ furvm::thing_type{ furvm::thing_type::U32 }, alloc };
    rhs.get<furvm::thing_type::u32>() = 7;

    auto res = lhs.add(rhs);
    ASSERT_EQ(res.type().type, furvm::thing_type::U32);
    EXPECT_EQ(lhs.get<furvm::thing_type::u32>(), 6);
    EXPECT_EQ(rhs.get<furvm::thing_type::u32>(), 7);
    EXPECT_EQ(res.get<furvm::thing_type::u32>(), 6 + 7);
}

} // namespace
