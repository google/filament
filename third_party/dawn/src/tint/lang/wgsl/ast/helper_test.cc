// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/helper_test.h"

namespace tint::ast {

using namespace tint::core::number_suffixes;  // NOLINT

using AstCheckIdentifierTest = TestHelper;

TEST_F(AstCheckIdentifierTest, NonTemplated) {
    CheckIdentifier(Ident("abc"), "abc");
}

TEST_F(AstCheckIdentifierTest, TemplatedScalars) {
    CheckIdentifier(Ident("abc", 1_i, 2_u, 3_f, 4_h, 5_a, 6._a, true),  //
                    Template("abc", 1_i, 2_u, 3_f, 4_h, 5_a, 6._a, true));
}

TEST_F(AstCheckIdentifierTest, TemplatedIdentifiers) {
    CheckIdentifier(Ident("abc", "one", "two", "three"),  //
                    Template("abc", "one", "two", "three"));
}

TEST_F(AstCheckIdentifierTest, NestedTemplate) {
    CheckIdentifier(Ident("abc", "pre", Ident("nested", 42_a), "post"),  //
                    Template("abc", "pre", Template("nested", 42_a), "post"));
}

}  // namespace tint::ast
