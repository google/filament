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

#include "gmock/gmock.h"

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::resolver {
namespace {

using ResolverUnresolvedIdentifierSuggestions = ResolverTest;

TEST_F(ResolverUnresolvedIdentifierSuggestions, AddressSpace) {
    AST().AddGlobalVariable(create<ast::Var>(
        Ident("v"),                        // name
        ty.i32(),                          // type
        Expr(Source{{12, 34}}, "privte"),  // declared_address_space
        nullptr,                           // declared_access
        nullptr,                           // initializer
        tint::Empty                        // attributes
        ));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: unresolved address space 'privte'
12:34 note: Did you mean 'private'?
Possible values: 'function', 'immediate', 'pixel_local', 'private', 'storage', 'uniform', 'workgroup')");
}

TEST_F(ResolverUnresolvedIdentifierSuggestions, TexelFormat) {
    GlobalVar("v", ty("texture_storage_1d", Expr(Source{{12, 34}}, "rba8unorm"), "read"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: unresolved texel format 'rba8unorm'
12:34 note: Did you mean 'rgba8unorm'?
Possible values: 'bgra8unorm', 'r16float', 'r16sint', 'r16snorm', 'r16uint', 'r16unorm', 'r32float', 'r32sint', 'r32uint', 'r8sint', 'r8snorm', 'r8uint', 'r8unorm', 'rg11b10ufloat', 'rg16float', 'rg16sint', 'rg16snorm', 'rg16uint', 'rg16unorm', 'rg32float', 'rg32sint', 'rg32uint', 'rg8sint', 'rg8snorm', 'rg8uint', 'rg8unorm', 'rgb10a2uint', 'rgb10a2unorm', 'rgba16float', 'rgba16sint', 'rgba16snorm', 'rgba16uint', 'rgba16unorm', 'rgba32float', 'rgba32sint', 'rgba32uint', 'rgba8sint', 'rgba8snorm', 'rgba8uint', 'rgba8unorm')");
}

TEST_F(ResolverUnresolvedIdentifierSuggestions, AccessMode) {
    AST().AddGlobalVariable(create<ast::Var>(Ident("v"),       // name
                                             ty.i32(),         // type
                                             Expr("private"),  // declared_address_space
                                             Expr(Source{{12, 34}}, "reed"),  // declared_access
                                             nullptr,                         // initializer
                                             tint::Empty                      // attributes
                                             ));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: unresolved access 'reed'
12:34 note: Did you mean 'read'?
Possible values: 'read', 'read_write', 'write')");
}

}  // namespace
}  // namespace tint::resolver
