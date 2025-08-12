// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/symbol/symbol_table.h"

namespace tint::core::type {
namespace {

class TypeIsHostShareableTest : public TestHelper {
  protected:
    Manager ty;
    GenerationID id;
    SymbolTable st{id};
};

TEST_F(TypeIsHostShareableTest, Void) {
    EXPECT_FALSE(ty.void_()->IsHostShareable());
}

TEST_F(TypeIsHostShareableTest, Bool) {
    EXPECT_FALSE(ty.bool_()->IsHostShareable());
}

TEST_F(TypeIsHostShareableTest, NumericScalar) {
    EXPECT_TRUE(ty.i32()->IsHostShareable());
    EXPECT_TRUE(ty.u32()->IsHostShareable());
    EXPECT_TRUE(ty.f32()->IsHostShareable());
    EXPECT_TRUE(ty.f16()->IsHostShareable());
}

TEST_F(TypeIsHostShareableTest, NumericVector) {
    EXPECT_TRUE(ty.vec2<i32>()->IsHostShareable());
    EXPECT_TRUE(ty.vec3<i32>()->IsHostShareable());
    EXPECT_TRUE(ty.vec4<i32>()->IsHostShareable());
    EXPECT_TRUE(ty.vec2<u32>()->IsHostShareable());
    EXPECT_TRUE(ty.vec3<u32>()->IsHostShareable());
    EXPECT_TRUE(ty.vec4<u32>()->IsHostShareable());
    EXPECT_TRUE(ty.vec2<f32>()->IsHostShareable());
    EXPECT_TRUE(ty.vec3<f32>()->IsHostShareable());
    EXPECT_TRUE(ty.vec4<f32>()->IsHostShareable());
    EXPECT_TRUE(ty.vec2<f16>()->IsHostShareable());
    EXPECT_TRUE(ty.vec3<f16>()->IsHostShareable());
    EXPECT_TRUE(ty.vec4<f16>()->IsHostShareable());
}

TEST_F(TypeIsHostShareableTest, BoolVector) {
    EXPECT_FALSE(ty.vec2<bool>()->IsHostShareable());
    EXPECT_FALSE(ty.vec3<bool>()->IsHostShareable());
    EXPECT_FALSE(ty.vec4<bool>()->IsHostShareable());
    EXPECT_FALSE(ty.vec2<bool>()->IsHostShareable());
    EXPECT_FALSE(ty.vec3<bool>()->IsHostShareable());
    EXPECT_FALSE(ty.vec4<bool>()->IsHostShareable());
    EXPECT_FALSE(ty.vec2<bool>()->IsHostShareable());
    EXPECT_FALSE(ty.vec3<bool>()->IsHostShareable());
    EXPECT_FALSE(ty.vec4<bool>()->IsHostShareable());
}

TEST_F(TypeIsHostShareableTest, Matrix) {
    EXPECT_TRUE(ty.mat2x2<f32>()->IsHostShareable());
    EXPECT_TRUE(ty.mat3x2<f32>()->IsHostShareable());
    EXPECT_TRUE(ty.mat4x2<f32>()->IsHostShareable());
    EXPECT_TRUE(ty.mat2x2<f32>()->IsHostShareable());
    EXPECT_TRUE(ty.mat3x2<f32>()->IsHostShareable());
    EXPECT_TRUE(ty.mat4x2<f32>()->IsHostShareable());
    EXPECT_TRUE(ty.mat2x2<f32>()->IsHostShareable());
    EXPECT_TRUE(ty.mat3x2<f32>()->IsHostShareable());
    EXPECT_TRUE(ty.mat4x2<f32>()->IsHostShareable());

    EXPECT_TRUE(ty.mat2x2<f16>()->IsHostShareable());
    EXPECT_TRUE(ty.mat3x2<f16>()->IsHostShareable());
    EXPECT_TRUE(ty.mat4x2<f16>()->IsHostShareable());
    EXPECT_TRUE(ty.mat2x2<f16>()->IsHostShareable());
    EXPECT_TRUE(ty.mat3x2<f16>()->IsHostShareable());
    EXPECT_TRUE(ty.mat4x2<f16>()->IsHostShareable());
    EXPECT_TRUE(ty.mat2x2<f16>()->IsHostShareable());
    EXPECT_TRUE(ty.mat3x2<f16>()->IsHostShareable());
    EXPECT_TRUE(ty.mat4x2<f16>()->IsHostShareable());
}

TEST_F(TypeIsHostShareableTest, Pointer) {
    auto* ptr = ty.ptr(core::AddressSpace::kPrivate, ty.i32());
    EXPECT_FALSE(ptr->IsHostShareable());
}

TEST_F(TypeIsHostShareableTest, Atomic) {
    EXPECT_TRUE(ty.atomic<i32>()->IsHostShareable());
    EXPECT_TRUE(ty.atomic<u32>()->IsHostShareable());
}

TEST_F(TypeIsHostShareableTest, Array_FixedSized) {
    EXPECT_TRUE((ty.array<i32, 5>()->IsHostShareable()));
    EXPECT_FALSE((ty.array<bool, 5>()->IsHostShareable()));
}

TEST_F(TypeIsHostShareableTest, Array_RuntimeSized) {
    EXPECT_TRUE(ty.array<i32>()->IsHostShareable());
    EXPECT_FALSE(ty.array<bool>()->IsHostShareable());
}

TEST_F(TypeIsHostShareableTest, Struct_HostShareable) {
    auto* inner = ty.Struct(st.New(), tint::Vector{
                                          Manager::StructMemberDesc{st.New(), ty.i32()},
                                          Manager::StructMemberDesc{st.New(), ty.u32()},
                                          Manager::StructMemberDesc{st.New(), ty.f32()},
                                          Manager::StructMemberDesc{st.New(), ty.vec3<f32>()},
                                          Manager::StructMemberDesc{st.New(), ty.mat4x2<f32>()},
                                          Manager::StructMemberDesc{st.New(), ty.array<f32, 4>()},
                                      });
    auto* outer = ty.Struct(st.New(), tint::Vector{
                                          Manager::StructMemberDesc{st.New(), inner},
                                      });
    EXPECT_TRUE(inner->IsConstructible());
    EXPECT_TRUE(inner->IsHostShareable());
    EXPECT_TRUE(outer->IsHostShareable());
}

TEST_F(TypeIsHostShareableTest, Struct_NotHostShareable) {
    auto* inner = ty.Struct(st.New(), tint::Vector{
                                          Manager::StructMemberDesc{st.New(), ty.i32()},
                                          Manager::StructMemberDesc{st.New(), ty.u32()},
                                          Manager::StructMemberDesc{st.New(), ty.f32()},
                                          Manager::StructMemberDesc{st.New(), ty.bool_()},
                                          Manager::StructMemberDesc{st.New(), ty.vec3<f32>()},
                                          Manager::StructMemberDesc{st.New(), ty.mat4x2<f32>()},
                                          Manager::StructMemberDesc{st.New(), ty.array<f32, 4>()},
                                      });
    auto* outer = ty.Struct(st.New(), tint::Vector{
                                          Manager::StructMemberDesc{st.New(), inner},
                                      });
    EXPECT_FALSE(inner->IsHostShareable());
    EXPECT_FALSE(outer->IsHostShareable());
}

}  // namespace
}  // namespace tint::core::type
