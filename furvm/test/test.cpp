#include "furvm/exceptions.hpp"
#include "furvm/furvm.hpp"
#include "furvm/thing.hpp"

#include "gtest/gtest.h" // IWYU pragma: keep
#include <array>

namespace {

// TODO: Basic program tests (e.g. for loops)

TEST(ThingOps, Add) {
    furvm::thing lhs{ furvm::thing_type{ furvm::thing_type::U32 } };
    lhs.get<furvm::u32>() = 6;
    furvm::thing rhs{ furvm::thing_type{ furvm::thing_type::U32 } };
    rhs.get<furvm::u32>() = 7;

    auto res = lhs.add(rhs);
    ASSERT_EQ(res.type().type, furvm::thing_type::U32);
    EXPECT_EQ(lhs.get<furvm::u32>(), 6);
    EXPECT_EQ(rhs.get<furvm::u32>(), 7);
    EXPECT_EQ(res.get<furvm::u32>(), 6 + 7);
}

TEST(ThingOps, Array) {
    furvm::thing_type innerType = { furvm::thing_type::U32 };

    static constexpr std::size_t                    LENGTH = 10;
    static constexpr std::array<furvm::u32, LENGTH> values = { 0, 1, 2, 3, 4, 5, 6, 7, 6, 7 };

    furvm::thing array{ furvm::thing_type{ furvm::thing_type::Array, { &innerType, LENGTH } } };
    EXPECT_EQ(array.length(), LENGTH);
    for (std::size_t i = 0; i < LENGTH; ++i) {
        auto el              = array.at(i);
        el.get<furvm::u32>() = values[i];
        EXPECT_EQ(array.at(i).integer(), values[i]);
    }
}

TEST(ThingOps, Slice) {
    furvm::thing_type innerType = { furvm::thing_type::U32 };

    static constexpr std::size_t                    LENGTH = 10;
    static constexpr std::array<furvm::u32, LENGTH> values = { 0, 1, 2, 3, 4, 5, 6, 7, 6, 7 };

    furvm::thing array{ furvm::thing_type{ furvm::thing_type::Array, { &innerType, LENGTH } } };
    EXPECT_EQ(array.length(), LENGTH);
    furvm::thing slice = array.slice(0, LENGTH);
    EXPECT_EQ(slice.length(), LENGTH);
    for (std::size_t i = 0; i < LENGTH; ++i) {
        auto el              = slice.at(i);
        el.get<furvm::u32>() = values[i];
        EXPECT_EQ(array.at(i).integer(), values[i]);
        EXPECT_EQ(slice.at(i).integer(), array.at(i).integer());
    }
}

TEST(ThingOps, Iterators) {
    furvm::thing_type innerType = { furvm::thing_type::U32 };

    static constexpr std::size_t                    LENGTH = 10;
    static constexpr std::array<furvm::u32, LENGTH> values = { 0, 1, 2, 3, 4, 5, 6, 7, 6, 7 };

    furvm::thing array{ furvm::thing_type{ furvm::thing_type::Array, { &innerType, LENGTH } } };
    EXPECT_EQ(array.length(), LENGTH);
    for (std::size_t i = 0; i < LENGTH; ++i) {
        auto el              = array.at(i);
        el.get<furvm::u32>() = values[i];
        EXPECT_EQ(array.at(i).integer(), values[i]);
    }

    std::size_t itCount = 0;
    for (auto it = array.begin(); it != array.end(); ++it, ++itCount) {
        auto idx = it - array.begin();
        ASSERT_EQ(idx, itCount);
        ASSERT_LT(idx, LENGTH);
        EXPECT_EQ(it->integer(), values[idx]);
    }
}

TEST(ThingOps, Access) {
    furvm::thing thing{ furvm::thing_type{ furvm::thing_type::U8 } };
    EXPECT_NO_THROW(thing.get<furvm::u8>());
    EXPECT_THROW(thing.get<furvm::s8>(), furvm::bad_thing_access);
    EXPECT_THROW(thing.get<furvm::s32>(), furvm::bad_thing_access);
    EXPECT_THROW(thing.get<void*>(), furvm::bad_thing_access);
}

} // namespace
