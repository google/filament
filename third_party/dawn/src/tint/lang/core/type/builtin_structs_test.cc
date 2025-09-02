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

#include "src/tint/lang/core/type/builtin_structs.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/utils/generation_id.h"
#include "src/tint/utils/symbol/symbol_table.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::type {
namespace {

enum ElementType {
    kAFloat,
    kAInt,
    kF32,
    kF16,
    kI32,
    kU32,
};

template <typename T>
class BuiltinStructsTest : public testing::TestWithParam<T> {
  protected:
    BuiltinStructsTest() : symbols(GenerationID::New()) {}

    const Type* Make(ElementType t) {
        switch (t) {
            case kAFloat:
                return ty.AFloat();
            case kAInt:
                return ty.AInt();
            case kF32:
                return ty.f32();
            case kF16:
                return ty.f16();
            case kI32:
                return ty.i32();
            case kU32:
                return ty.u32();
        }
        return nullptr;
    }

    Manager ty;
    SymbolTable symbols;
};

struct FrexpCase {
    ElementType in;
    ElementType fract;
    ElementType exp;
};
class BuiltinFrexpResultStructTest : public BuiltinStructsTest<FrexpCase> {
  protected:
    void Run(const Type* in_type, const Type* fract_type, const Type* exp_type) {
        auto* result = CreateFrexpResult(ty, symbols, in_type);

        auto* str = As<Struct>(result);
        ASSERT_NE(str, nullptr);
        ASSERT_EQ(str->Members().Length(), 2u);
        EXPECT_EQ(str->Members()[0]->Name().Name(), "fract");
        EXPECT_EQ(str->Members()[0]->Type(), fract_type);
        EXPECT_EQ(str->Members()[1]->Name().Name(), "exp");
        EXPECT_EQ(str->Members()[1]->Type(), exp_type);
        EXPECT_TRUE(str->IsWgslInternal());
    }
};
TEST_P(BuiltinFrexpResultStructTest, Scalar) {
    auto params = GetParam();
    Run(Make(params.in), Make(params.fract), Make(params.exp));
}
TEST_P(BuiltinFrexpResultStructTest, Vec2) {
    auto params = GetParam();
    Run(ty.vec2(Make(params.in)), ty.vec2(Make(params.fract)), ty.vec2(Make(params.exp)));
}
TEST_P(BuiltinFrexpResultStructTest, Vec3) {
    auto params = GetParam();
    Run(ty.vec3(Make(params.in)), ty.vec3(Make(params.fract)), ty.vec3(Make(params.exp)));
}
TEST_P(BuiltinFrexpResultStructTest, Vec4) {
    auto params = GetParam();
    Run(ty.vec4(Make(params.in)), ty.vec4(Make(params.fract)), ty.vec4(Make(params.exp)));
}

INSTANTIATE_TEST_SUITE_P(BuiltinFrexpResultStructTest,
                         BuiltinFrexpResultStructTest,
                         testing::Values(FrexpCase{kAFloat, kAFloat, kAInt},
                                         FrexpCase{kF32, kF32, kI32},
                                         FrexpCase{kF16, kF16, kI32}));

class BuiltinModfResultStructTest : public BuiltinStructsTest<ElementType> {
  protected:
    void Run(const Type* type) {
        auto* result = CreateModfResult(ty, symbols, type);

        auto* str = As<Struct>(result);
        ASSERT_NE(str, nullptr);
        ASSERT_EQ(str->Members().Length(), 2u);
        EXPECT_EQ(str->Members()[0]->Name().Name(), "fract");
        EXPECT_EQ(str->Members()[0]->Type(), type);
        EXPECT_EQ(str->Members()[1]->Name().Name(), "whole");
        EXPECT_EQ(str->Members()[1]->Type(), type);
        EXPECT_TRUE(str->IsWgslInternal());
    }
};
TEST_P(BuiltinModfResultStructTest, Scalar) {
    Run(Make(GetParam()));
}
TEST_P(BuiltinModfResultStructTest, Vec2) {
    Run(ty.vec2(Make(GetParam())));
}
TEST_P(BuiltinModfResultStructTest, Vec3) {
    Run(ty.vec3(Make(GetParam())));
}
TEST_P(BuiltinModfResultStructTest, Vec4) {
    Run(ty.vec4(Make(GetParam())));
}
INSTANTIATE_TEST_SUITE_P(BuiltinModfResultStructTest,
                         BuiltinModfResultStructTest,
                         testing::Values(kAFloat, kF32, kF16));

class BuiltinAtomicCompareExchangeResultStructTest : public BuiltinStructsTest<ElementType> {
  protected:
    void Run(const Type* type) {
        auto* result = CreateAtomicCompareExchangeResult(ty, symbols, type);

        auto* str = As<Struct>(result);
        ASSERT_NE(str, nullptr);
        ASSERT_EQ(str->Members().Length(), 2u);
        EXPECT_EQ(str->Members()[0]->Name().Name(), "old_value");
        EXPECT_EQ(str->Members()[0]->Type(), type);
        EXPECT_EQ(str->Members()[1]->Name().Name(), "exchanged");
        EXPECT_EQ(str->Members()[1]->Type(), ty.bool_());
        EXPECT_TRUE(str->IsWgslInternal());
    }
};
TEST_P(BuiltinAtomicCompareExchangeResultStructTest, Scalar) {
    Run(Make(GetParam()));
}
INSTANTIATE_TEST_SUITE_P(BuiltinAtomicCompareExchangeResultStructTest,
                         BuiltinAtomicCompareExchangeResultStructTest,
                         testing::Values(kI32, kU32));

using BuiltinStructTest = testing::Test;

TEST_F(BuiltinStructTest, FrexpResult_NoClashWithUserDeclaredStruct) {
    Manager ty;
    SymbolTable symbols(GenerationID::New());

    auto* user_struct = ty.Struct(symbols.New(ToString(BuiltinType::kFrexpResultF32)),
                                  {
                                      {symbols.New("fract"), ty.f32()},
                                      {symbols.New("exp"), ty.i32()},
                                  });

    auto* internal_struct = CreateFrexpResult(ty, symbols, ty.f32());

    ASSERT_NE(internal_struct, nullptr);
    EXPECT_NE(internal_struct, user_struct);
    EXPECT_TRUE(internal_struct->IsWgslInternal());
    EXPECT_FALSE(user_struct->IsWgslInternal());
}

TEST_F(BuiltinStructTest, ModfResult_NoClashWithUserDeclaredStruct) {
    Manager ty;
    SymbolTable symbols(GenerationID::New());

    auto* user_struct = ty.Struct(symbols.New(ToString(BuiltinType::kModfResultVec4F16)),
                                  {
                                      {symbols.New("fract"), ty.vec4<f16>()},
                                      {symbols.New("whole"), ty.vec4<f16>()},
                                  });

    auto* internal_struct = CreateModfResult(ty, symbols, ty.vec4<f16>());

    ASSERT_NE(internal_struct, nullptr);
    EXPECT_NE(internal_struct, user_struct);
    EXPECT_TRUE(internal_struct->IsWgslInternal());
    EXPECT_FALSE(user_struct->IsWgslInternal());
}

TEST_F(BuiltinStructTest, AtomicCompareExchangeResult_NoClashWithUserDeclaredStruct) {
    Manager ty;
    SymbolTable symbols(GenerationID::New());

    auto* user_struct =
        ty.Struct(symbols.New(ToString(BuiltinType::kAtomicCompareExchangeResultU32)),
                  {
                      {symbols.New("old_value"), ty.u32()},
                      {symbols.New("exchanged"), ty.bool_()},
                  });

    auto* internal_struct = CreateAtomicCompareExchangeResult(ty, symbols, ty.u32());

    ASSERT_NE(internal_struct, nullptr);
    EXPECT_NE(internal_struct, user_struct);
    EXPECT_TRUE(internal_struct->IsWgslInternal());
    EXPECT_FALSE(user_struct->IsWgslInternal());
}

}  // namespace
}  // namespace tint::core::type
