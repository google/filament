// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <utility>

#include "dawn/common/Math.h"
#include "dawn/native/Blob.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace dawn::native {
namespace {

// Test that a blob starts empty.
TEST(BlobTests, DefaultEmpty) {
    Blob b;
    EXPECT_TRUE(b.Empty());
    EXPECT_EQ(b.Data(), nullptr);
    EXPECT_EQ(b.Size(), 0u);
}

// Test that you can create a blob with a size in bytes and write/read its contents.
TEST(BlobTests, SizedCreation) {
    Blob b = CreateBlob(10);
    EXPECT_FALSE(b.Empty());
    EXPECT_EQ(b.Size(), 10u);
    ASSERT_NE(b.Data(), nullptr);
    // We should be able to copy 10 bytes into the blob.
    char data[10] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
    memcpy(b.Data(), data, sizeof(data));
    // And retrieve the exact contents back.
    EXPECT_EQ(memcmp(b.Data(), data, sizeof(data)), 0);
}

// Test that you can create a zero-sized blob.
TEST(BlobTests, EmptySizedCreation) {
    Blob b = CreateBlob(0);
    EXPECT_TRUE(b.Empty());
    EXPECT_EQ(b.Data(), nullptr);
    EXPECT_EQ(b.Size(), 0u);
}

// Test that you can create a blob with UnsafeCreateWithDeleter, and the deleter is
// called on destruction.
TEST(BlobTests, UnsafeCreateWithDeleter) {
    unsigned char data[13] = "hello world!";
    testing::StrictMock<testing::MockFunction<void()>> mockDeleter;
    {
        // Make a blob with a mock deleter.
        Blob b = Blob::UnsafeCreateWithDeleter(data, sizeof(data), [&] { mockDeleter.Call(); });
        // Check the contents.
        EXPECT_FALSE(b.Empty());
        EXPECT_EQ(b.Size(), sizeof(data));
        ASSERT_EQ(b.Data(), data);
        EXPECT_EQ(memcmp(b.Data(), data, sizeof(data)), 0);

        // |b| is deleted when this scope exits.
        EXPECT_CALL(mockDeleter, Call());
    }
}

// Test that you can create a blob with UnsafeCreateWithDeleter with zero size but non-null data.
// The deleter is still called on destruction, and the blob is normalized to be empty.
TEST(BlobTests, UnsafeCreateWithDeleterZeroSize) {
    unsigned char data[13] = "hello world!";
    testing::StrictMock<testing::MockFunction<void()>> mockDeleter;
    {
        // Make a blob with a mock deleter.
        Blob b = Blob::UnsafeCreateWithDeleter(data, 0, [&] { mockDeleter.Call(); });
        // Check the contents.
        EXPECT_TRUE(b.Empty());
        EXPECT_EQ(b.Size(), 0u);
        // Data still points to the data.
        EXPECT_EQ(b.Data(), data);

        // |b| is deleted when this scope exits.
        EXPECT_CALL(mockDeleter, Call());
    }
}

// Test that you can create a blob with UnsafeCreateWithDeleter that points to nothing.
// The deleter should still be called.
TEST(BlobTests, UnsafeCreateWithDeleterEmpty) {
    testing::StrictMock<testing::MockFunction<void()>> mockDeleter;
    {
        // Make a blob with a mock deleter.
        Blob b = Blob::UnsafeCreateWithDeleter(nullptr, 0, [&] { mockDeleter.Call(); });
        // Check the contents.
        EXPECT_TRUE(b.Empty());
        EXPECT_EQ(b.Size(), 0u);
        EXPECT_EQ(b.Data(), nullptr);

        // |b| is deleted when this scope exits.
        EXPECT_CALL(mockDeleter, Call());
    }
}

// Test that move construction moves the data from one blob into the new one.
TEST(BlobTests, MoveConstruct) {
    // Create the blob.
    Blob b1 = CreateBlob(10);
    char data[10] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
    memcpy(b1.Data(), data, sizeof(data));

    // Move construct b2 from b1.
    Blob b2(std::move(b1));

    // Data should be moved.
    EXPECT_FALSE(b2.Empty());
    EXPECT_EQ(b2.Size(), 10u);
    ASSERT_NE(b2.Data(), nullptr);
    EXPECT_EQ(memcmp(b2.Data(), data, sizeof(data)), 0);
}

// Test that move assignment moves the data from one blob into another.
TEST(BlobTests, MoveAssign) {
    // Create the blob.
    Blob b1 = CreateBlob(10);
    char data[10] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
    memcpy(b1.Data(), data, sizeof(data));

    // Move assign b2 from b1.
    Blob b2;
    b2 = std::move(b1);

    // Data should be moved.
    EXPECT_FALSE(b2.Empty());
    EXPECT_EQ(b2.Size(), 10u);
    ASSERT_NE(b2.Data(), nullptr);
    EXPECT_EQ(memcmp(b2.Data(), data, sizeof(data)), 0);
}

// Test that move assignment can replace the contents of the moved-to blob.
TEST(BlobTests, MoveAssignOver) {
    // Create the blob.
    Blob b1 = CreateBlob(10);
    char data[10] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
    memcpy(b1.Data(), data, sizeof(data));

    // Create another blob with a mock deleter.
    testing::StrictMock<testing::MockFunction<void()>> mockDeleter;
    Blob b2 = Blob::UnsafeCreateWithDeleter(nullptr, 0, [&] { mockDeleter.Call(); });

    // Move b1 into b2, replacing b2's contents, and expect the deleter to be called.
    EXPECT_CALL(mockDeleter, Call());
    b2 = std::move(b1);

    // Data should be moved.
    EXPECT_FALSE(b2.Empty());
    EXPECT_EQ(b2.Size(), 10u);
    ASSERT_NE(b2.Data(), nullptr);
    EXPECT_EQ(memcmp(b2.Data(), data, sizeof(data)), 0);
}

}  // namespace
}  // namespace dawn::native
