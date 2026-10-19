/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "BlobCacheKey.h"

#include <backend/Program.h>

#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <new>
#include <type_traits>
#include <utility>

namespace filament::backend {

namespace {

using SpecConstant = Program::SpecializationConstant;
using SpecConstants = BlobCacheKey::SpecializationConstants;

// Writes `value` into `dst` such that every byte of the variant's object representation that
// is not part of its value (unused union bytes, padding) is set to `fill`. This simulates
// spec constants coming from uninitialized memory.
void putWithGarbage(SpecConstant* dst, unsigned char fill, SpecConstant const& value) {
    alignas(SpecConstant) unsigned char storage[sizeof(SpecConstant)];
    std::memset(storage, fill, sizeof(storage));
    new (storage) SpecConstant(value);
    // SpecConstant is trivially copyable, so copying its object representation is well-defined.
    std::memcpy(static_cast<void*>(dst), storage, sizeof(SpecConstant));
}

SpecConstants makeConstants(unsigned char fill) {
    SpecConstants constants(5);
    putWithGarbage(&constants[0], fill, SpecConstant{ true });
    putWithGarbage(&constants[1], fill, SpecConstant{ int32_t(42) });
    putWithGarbage(&constants[2], fill, SpecConstant{ 1.5f });
    putWithGarbage(&constants[3], fill, SpecConstant{ false });
    putWithGarbage(&constants[4], fill, SpecConstant{ int32_t(-1) });
    return constants;
}

bool sameBytes(BlobCacheKey const& a, BlobCacheKey const& b) {
    return a.size() == b.size() && std::memcmp(a.data(), b.data(), a.size()) == 0;
}

BlobCacheKey makeKey(uint64_t id, SpecConstant const& value) {
    SpecConstants constants(1);
    constants[0] = value;
    return { id, constants };
}

} // namespace

static_assert(std::is_trivially_copyable_v<Program::SpecializationConstant>,
        "putWithGarbage() relies on SpecializationConstant being trivially copyable");

TEST(BlobCacheKeyTest, DefaultIsEmpty) {
    BlobCacheKey const key;
    EXPECT_FALSE(key);
    EXPECT_EQ(key.size(), 0u);
    EXPECT_EQ(key.data(), nullptr);
}

TEST(BlobCacheKeyTest, SizeIsHeaderPlusEightBytesPerConstant) {
    BlobCacheKey const empty{ 1, SpecConstants{} };
    EXPECT_TRUE(empty);
    EXPECT_EQ(empty.size(), sizeof(uint64_t));

    BlobCacheKey const key{ 1, makeConstants(0) };
    EXPECT_EQ(key.size(), sizeof(uint64_t) + 5 * 2 * sizeof(uint32_t));
}

// The key is hashed/compared as raw bytes by the platform blob cache. Bytes of the
// std::variant that are not part of its value must never leak into the key.
TEST(BlobCacheKeyTest, BytesIgnoreVariantGarbage) {
    BlobCacheKey const a{ 0x1234, makeConstants(0xAA) };
    BlobCacheKey const b{ 0x1234, makeConstants(0x55) };
    BlobCacheKey const c{ 0x1234, makeConstants(0x00) };
    EXPECT_TRUE(sameBytes(a, b));
    EXPECT_TRUE(sameBytes(a, c));
}

TEST(BlobCacheKeyTest, ExpectedLayout) {
    SpecConstants constants(3);
    constants[0] = SpecConstant{ true };
    constants[1] = SpecConstant{ int32_t(42) };
    constants[2] = SpecConstant{ 1.5f };
    BlobCacheKey const key{ 0x1122334455667788ull, constants };

    struct Expected {
        uint64_t id;
        uint32_t entries[6];
    } expected{};
    static_assert(sizeof(Expected) == sizeof(uint64_t) + 6 * sizeof(uint32_t));
    expected.id = 0x1122334455667788ull;
    expected.entries[0] = 2;            // bool
    expected.entries[1] = 1;            // true
    expected.entries[2] = 0;            // int32_t
    expected.entries[3] = 42;
    expected.entries[4] = 1;            // float
    float const f = 1.5f;
    std::memcpy(&expected.entries[5], &f, sizeof(f));

    ASSERT_EQ(key.size(), sizeof(Expected));
    EXPECT_EQ(std::memcmp(key.data(), &expected, sizeof(Expected)), 0);
}

TEST(BlobCacheKeyTest, DifferentInputsGiveDifferentKeys) {
    BlobCacheKey const base = makeKey(1, SpecConstant{ int32_t(1) });

    // same value, different id
    EXPECT_FALSE(sameBytes(base, makeKey(2, SpecConstant{ int32_t(1) })));
    // different value
    EXPECT_FALSE(sameBytes(base, makeKey(1, SpecConstant{ int32_t(2) })));
    // same raw bits (1), but a different type must not collide
    EXPECT_FALSE(sameBytes(base, makeKey(1, SpecConstant{ true })));
    // true vs false
    EXPECT_FALSE(sameBytes(makeKey(1, SpecConstant{ true }), makeKey(1, SpecConstant{ false })));
    // identical inputs give identical keys
    EXPECT_TRUE(sameBytes(base, makeKey(1, SpecConstant{ int32_t(1) })));
}

TEST(BlobCacheKeyTest, CopyAndMove) {
    BlobCacheKey original = makeKey(7, SpecConstant{ 3.0f });
    size_t const size = original.size();

    BlobCacheKey const copy = original;
    EXPECT_TRUE(sameBytes(copy, original));

    BlobCacheKey moved = std::move(original);
    EXPECT_EQ(moved.size(), size);
    EXPECT_TRUE(sameBytes(moved, copy));
    EXPECT_FALSE(original); // NOLINT(bugprone-use-after-move): moved-from key must be empty

    BlobCacheKey assigned;
    assigned = std::move(moved);
    EXPECT_TRUE(sameBytes(assigned, copy));
}

} // namespace filament::backend
