// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/core/type/buffer.h"

#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/manager.h"

namespace tint::core::type {
namespace {

using BufferTest = TestHelper;

TEST_F(BufferTest, Creation_Unsized) {
    {
        Manager ty;
        auto* b = ty.unsized_buffer();
        EXPECT_EQ(b->Size(), 0u);
        EXPECT_EQ(b->FriendlyName(), "buffer");
    }
    {
        core::type::Manager mgr;
        auto* b = mgr.unsized_buffer();
        EXPECT_EQ(b->Size(), 0u);
        EXPECT_EQ(b->FriendlyName(), "buffer");
    }
}

TEST_F(BufferTest, Creation_Sized) {
    {
        Manager ty;
        auto* b = ty.buffer(16);
        EXPECT_EQ(b->Size(), 16u);
        EXPECT_EQ(b->FriendlyName(), "buffer<16>");
    }
    {
        core::type::Manager mgr;
        auto* b = mgr.buffer(32);
        EXPECT_EQ(b->Size(), 32u);
        EXPECT_EQ(b->FriendlyName(), "buffer<32>");
    }
}

TEST_F(BufferTest, Hash) {
    Manager ty;
    auto* b1 = ty.unsized_buffer();
    auto* b2 = ty.unsized_buffer();
    auto* b3 = ty.buffer(16);
    auto* b4 = ty.buffer(16);

    EXPECT_EQ(b1, b2);
    EXPECT_NE(b1, b3);
    EXPECT_EQ(b3, b4);
}

TEST_F(BufferTest, Equals) {
    Manager ty;
    auto* b1 = ty.unsized_buffer();
    auto* b2 = ty.unsized_buffer();
    auto* b3 = ty.buffer(16);
    auto* b4 = ty.buffer(16);

    EXPECT_EQ(b1, b2);
    EXPECT_NE(b1, b3);
    EXPECT_EQ(b3, b4);
}

TEST_F(BufferTest, Clone) {
    Manager ty;
    auto* b1 = ty.unsized_buffer();
    auto* b2 = ty.buffer(16);

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    {
        auto* c = b1->Clone(ctx);
        EXPECT_EQ(c->Size(), 0u);
    }
    {
        auto* c = b2->Clone(ctx);
        EXPECT_EQ(c->Size(), 16u);
    }
}

}  // namespace
}  // namespace tint::core::type
