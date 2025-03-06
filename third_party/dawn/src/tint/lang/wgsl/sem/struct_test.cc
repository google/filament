// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/lang/core/type/texture.h"
#include "src/tint/lang/wgsl/sem/helper_test.h"

namespace tint::sem {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using SemStructTest = TestHelper;

TEST_F(SemStructTest, Creation) {
    auto name = Sym("S");
    auto* impl = create<ast::Struct>(Ident(name), tint::Empty, tint::Empty);
    auto* ptr = impl;
    auto* s = create<sem::Struct>(impl, impl->name->symbol, tint::Empty, 4u /* align */,
                                  8u /* size */, 16u /* size_no_padding */);
    EXPECT_EQ(s->Declaration(), ptr);
    EXPECT_EQ(s->Align(), 4u);
    EXPECT_EQ(s->Size(), 8u);
    EXPECT_EQ(s->SizeNoPadding(), 16u);
}

TEST_F(SemStructTest, Equals) {
    auto* a_impl = create<ast::Struct>(Ident("a"), tint::Empty, tint::Empty);
    auto* a = create<sem::Struct>(a_impl, a_impl->name->symbol, tint::Empty, 4u /* align */,
                                  4u /* size */, 4u /* size_no_padding */);
    auto* b_impl = create<ast::Struct>(Ident("b"), tint::Empty, tint::Empty);
    auto* b = create<sem::Struct>(b_impl, b_impl->name->symbol, tint::Empty, 4u /* align */,
                                  4u /* size */, 4u /* size_no_padding */);

    EXPECT_TRUE(a->Equals(*a));
    EXPECT_FALSE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(core::type::Void{}));
}

TEST_F(SemStructTest, FriendlyName) {
    auto name = Sym("my_struct");
    auto* impl = create<ast::Struct>(Ident(name), tint::Empty, tint::Empty);
    auto* s = create<sem::Struct>(impl, impl->name->symbol, tint::Empty, 4u /* align */,
                                  4u /* size */, 4u /* size_no_padding */);
    EXPECT_EQ(s->FriendlyName(), "my_struct");
}

}  // namespace
}  // namespace tint::sem
