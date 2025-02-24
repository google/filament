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

#include "src/tint/lang/wgsl/resolver/resolver.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/texel_format.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/extension.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/lang/wgsl/sem/index_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/utils/containers/vector.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::resolver {
namespace {

struct SideEffectsTest : ResolverTest {
    template <typename T>
    void MakeSideEffectFunc(const char* name) {
        auto global = Sym();
        GlobalVar(global, ty.Of<T>(), core::AddressSpace::kPrivate);
        auto local = Sym();
        Func(name, tint::Empty, ty.Of<T>(),
             Vector{
                 Decl(Var(local, ty.Of<T>())),
                 Assign(global, local),
                 Return(global),
             });
    }

    template <typename MAKE_TYPE_FUNC>
    void MakeSideEffectFunc(const char* name, MAKE_TYPE_FUNC make_type) {
        auto global = Sym();
        GlobalVar(global, make_type(), core::AddressSpace::kPrivate);
        auto local = Sym();
        Func(name, tint::Empty, make_type(),
             Vector{
                 Decl(Var(local, make_type())),
                 Assign(global, local),
                 Return(global),
             });
    }
};

TEST_F(SideEffectsTest, Phony) {
    auto* expr = Phony();
    auto* body = Assign(expr, 1_i);
    WrapInFunction(body);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Literal) {
    auto* expr = Expr(1_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, VariableUser) {
    auto* var = Decl(Var("a", ty.i32()));
    auto* expr = Expr("a");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().GetVal(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->UnwrapLoad()->Is<sem::VariableUser>());
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_Builtin_NoSE) {
    GlobalVar("a", ty.f32(), core::AddressSpace::kPrivate);
    auto* expr = Call("sqrt", "a");
    Func("f", tint::Empty, ty.void_(), Vector{Ignore(expr)},
         Vector{create<ast::StageAttribute>(ast::PipelineStage::kFragment)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_Builtin_NoSE_WithSEArg) {
    MakeSideEffectFunc<f32>("se");
    auto* expr = Call("dpdx", Call("se"));
    Func("f", tint::Empty, ty.void_(), Vector{Ignore(expr)},
         Vector{create<ast::StageAttribute>(ast::PipelineStage::kFragment)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_Builtin_SE) {
    GlobalVar("a", ty.atomic(ty.i32()), core::AddressSpace::kWorkgroup);
    auto* expr = Call("atomicAdd", AddressOf("a"), 1_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_TRUE(sem->HasSideEffects());
}

namespace builtin_tests {
struct Case {
    const char* name;
    Vector<const char*, 3> args;
    bool has_side_effects;
    bool returns_value;
    ast::PipelineStage pipeline_stage;
};
static Case C(const char* name,
              VectorRef<const char*> args,
              bool has_side_effects,
              bool returns_value,
              ast::PipelineStage stage = ast::PipelineStage::kFragment) {
    Case c;
    c.name = name;
    c.args = std::move(args);
    c.has_side_effects = has_side_effects;
    c.returns_value = returns_value;
    c.pipeline_stage = stage;
    return c;
}
static std::ostream& operator<<(std::ostream& o, const Case& c) {
    o << c.name << "(";
    for (size_t i = 0; i < c.args.Length(); ++i) {
        o << c.args[i];
        if (i + 1 != c.args.Length()) {
            o << ", ";
        }
    }
    o << ")";
    return o;
}

using SideEffectsBuiltinTest = resolver::ResolverTestWithParam<Case>;

TEST_P(SideEffectsBuiltinTest, Test) {
    auto& c = GetParam();

    uint32_t next_binding = 0;
    GlobalVar("f", ty.f32(), tint::core::AddressSpace::kPrivate);
    GlobalVar("i", ty.i32(), tint::core::AddressSpace::kPrivate);
    GlobalVar("u", ty.u32(), tint::core::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), tint::core::AddressSpace::kPrivate);
    GlobalVar("vf", ty.vec3<f32>(), tint::core::AddressSpace::kPrivate);
    GlobalVar("vf2", ty.vec2<f32>(), tint::core::AddressSpace::kPrivate);
    GlobalVar("vi2", ty.vec2<i32>(), tint::core::AddressSpace::kPrivate);
    GlobalVar("vf4", ty.vec4<f32>(), tint::core::AddressSpace::kPrivate);
    GlobalVar("vb", ty.vec3<bool>(), tint::core::AddressSpace::kPrivate);
    GlobalVar("m", ty.mat3x3<f32>(), tint::core::AddressSpace::kPrivate);
    GlobalVar("arr", ty.array<f32, 10>(), tint::core::AddressSpace::kPrivate);
    GlobalVar("storage_arr", ty.array<f32>(), tint::core::AddressSpace::kStorage, Group(0_a),
              Binding(AInt(next_binding++)));
    GlobalVar("workgroup_arr", ty.array<f32, 4>(), tint::core::AddressSpace::kWorkgroup);
    GlobalVar("a", ty.atomic(ty.i32()), tint::core::AddressSpace::kStorage,
              tint::core::Access::kReadWrite, Group(0_a), Binding(AInt(next_binding++)));
    if (c.pipeline_stage != ast::PipelineStage::kCompute) {
        GlobalVar("t2d", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
                  Group(0_a), Binding(AInt(next_binding++)));
        GlobalVar("tdepth2d", ty.depth_texture(core::type::TextureDimension::k2d), Group(0_a),
                  Binding(AInt(next_binding++)));
        GlobalVar("t2d_arr", ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32()),
                  Group(0_a), Binding(AInt(next_binding++)));
        GlobalVar("t2d_multi", ty.multisampled_texture(core::type::TextureDimension::k2d, ty.f32()),
                  Group(0_a), Binding(AInt(next_binding++)));
        GlobalVar(
            "tstorage2d",
            ty.storage_texture(core::type::TextureDimension::k2d,
                               tint::core::TexelFormat::kR32Float, tint::core::Access::kWrite),
            Group(0_a), Binding(AInt(next_binding++)));
        GlobalVar("s2d", ty.sampler(core::type::SamplerKind::kSampler), Group(0_a),
                  Binding(AInt(next_binding++)));
        GlobalVar("scomp", ty.sampler(core::type::SamplerKind::kComparisonSampler), Group(0_a),
                  Binding(AInt(next_binding++)));
    }

    Vector<const ast::Statement*, 4> stmts;
    stmts.Push(Decl(Let("pstorage_arr", AddressOf("storage_arr"))));
    if (c.pipeline_stage == ast::PipelineStage::kCompute) {
        stmts.Push(Decl(Let("pworkgroup_arr", AddressOf("workgroup_arr"))));
    }
    stmts.Push(Decl(Let("pa", AddressOf("a"))));

    Vector<const ast::Expression*, 5> args;
    for (auto& a : c.args) {
        args.Push(Expr(a));
    }
    auto* expr = Call(c.name, args);

    Vector<const ast::Attribute*, 2> attrs;
    attrs.Push(create<ast::StageAttribute>(c.pipeline_stage));
    if (c.pipeline_stage == ast::PipelineStage::kCompute) {
        attrs.Push(WorkgroupSize(Expr(1_u)));
    }

    if (c.returns_value) {
        stmts.Push(Assign(Phony(), expr));
    } else {
        stmts.Push(CallStmt(expr));
    }

    Func("func", tint::Empty, ty.void_(), stmts, attrs);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_EQ(c.has_side_effects, sem->HasSideEffects());
}
INSTANTIATE_TEST_SUITE_P(
    SideEffectsTest_Builtins,
    SideEffectsBuiltinTest,
    testing::ValuesIn(std::vector<Case>{
        // No side-effect builts
        C("abs", Vector{"f"}, false, true),                                               //
        C("acos", Vector{"f"}, false, true),                                              //
        C("acosh", Vector{"f"}, false, true),                                             //
        C("all", Vector{"vb"}, false, true),                                              //
        C("any", Vector{"vb"}, false, true),                                              //
        C("arrayLength", Vector{"pstorage_arr"}, false, true),                            //
        C("asin", Vector{"f"}, false, true),                                              //
        C("asinh", Vector{"f"}, false, true),                                             //
        C("atan", Vector{"f"}, false, true),                                              //
        C("atan2", Vector{"f", "f"}, false, true),                                        //
        C("atanh", Vector{"f"}, false, true),                                             //
        C("atomicLoad", Vector{"pa"}, false, true),                                       //
        C("ceil", Vector{"f"}, false, true),                                              //
        C("clamp", Vector{"f", "f", "f"}, false, true),                                   //
        C("cos", Vector{"f"}, false, true),                                               //
        C("cosh", Vector{"f"}, false, true),                                              //
        C("countLeadingZeros", Vector{"i"}, false, true),                                 //
        C("countOneBits", Vector{"i"}, false, true),                                      //
        C("countTrailingZeros", Vector{"i"}, false, true),                                //
        C("cross", Vector{"vf", "vf"}, false, true),                                      //
        C("degrees", Vector{"f"}, false, true),                                           //
        C("determinant", Vector{"m"}, false, true),                                       //
        C("distance", Vector{"f", "f"}, false, true),                                     //
        C("dot", Vector{"vf", "vf"}, false, true),                                        //
        C("dot4I8Packed", Vector{"u", "u"}, false, true),                                 //
        C("dot4U8Packed", Vector{"u", "u"}, false, true),                                 //
        C("exp", Vector{"f"}, false, true),                                               //
        C("exp2", Vector{"f"}, false, true),                                              //
        C("extractBits", Vector{"i", "u", "u"}, false, true),                             //
        C("faceForward", Vector{"vf", "vf", "vf"}, false, true),                          //
        C("firstLeadingBit", Vector{"u"}, false, true),                                   //
        C("firstTrailingBit", Vector{"u"}, false, true),                                  //
        C("floor", Vector{"f"}, false, true),                                             //
        C("fma", Vector{"f", "f", "f"}, false, true),                                     //
        C("fract", Vector{"vf"}, false, true),                                            //
        C("frexp", Vector{"f"}, false, true),                                             //
        C("insertBits", Vector{"i", "i", "u", "u"}, false, true),                         //
        C("inverseSqrt", Vector{"f"}, false, true),                                       //
        C("ldexp", Vector{"f", "i"}, false, true),                                        //
        C("length", Vector{"vf"}, false, true),                                           //
        C("log", Vector{"f"}, false, true),                                               //
        C("log2", Vector{"f"}, false, true),                                              //
        C("max", Vector{"f", "f"}, false, true),                                          //
        C("min", Vector{"f", "f"}, false, true),                                          //
        C("mix", Vector{"f", "f", "f"}, false, true),                                     //
        C("modf", Vector{"f"}, false, true),                                              //
        C("normalize", Vector{"vf"}, false, true),                                        //
        C("pack2x16float", Vector{"vf2"}, false, true),                                   //
        C("pack2x16snorm", Vector{"vf2"}, false, true),                                   //
        C("pack2x16unorm", Vector{"vf2"}, false, true),                                   //
        C("pack4x8snorm", Vector{"vf4"}, false, true),                                    //
        C("pack4x8unorm", Vector{"vf4"}, false, true),                                    //
        C("pow", Vector{"f", "f"}, false, true),                                          //
        C("radians", Vector{"f"}, false, true),                                           //
        C("reflect", Vector{"vf", "vf"}, false, true),                                    //
        C("refract", Vector{"vf", "vf", "f"}, false, true),                               //
        C("reverseBits", Vector{"u"}, false, true),                                       //
        C("round", Vector{"f"}, false, true),                                             //
        C("select", Vector{"f", "f", "b"}, false, true),                                  //
        C("sign", Vector{"f"}, false, true),                                              //
        C("sin", Vector{"f"}, false, true),                                               //
        C("sinh", Vector{"f"}, false, true),                                              //
        C("smoothstep", Vector{"f", "f", "f"}, false, true),                              //
        C("sqrt", Vector{"f"}, false, true),                                              //
        C("step", Vector{"f", "f"}, false, true),                                         //
        C("tan", Vector{"f"}, false, true),                                               //
        C("tanh", Vector{"f"}, false, true),                                              //
        C("textureDimensions", Vector{"t2d"}, false, true),                               //
        C("textureGather", Vector{"tdepth2d", "s2d", "vf2"}, false, true),                //
        C("textureGatherCompare", Vector{"tdepth2d", "scomp", "vf2", "f"}, false, true),  //
        C("textureLoad", Vector{"t2d", "vi2", "i"}, false, true),                         //
        C("textureNumLayers", Vector{"t2d_arr"}, false, true),                            //
        C("textureNumLevels", Vector{"t2d"}, false, true),                                //
        C("textureNumSamples", Vector{"t2d_multi"}, false, true),                         //
        C("textureSampleCompareLevel",
          Vector{"tdepth2d", "scomp", "vf2", "f"},
          false,
          true),                                                                         //
        C("textureSampleGrad", Vector{"t2d", "s2d", "vf2", "vf2", "vf2"}, false, true),  //
        C("textureSampleLevel", Vector{"t2d", "s2d", "vf2", "f"}, false, true),          //
        C("transpose", Vector{"m"}, false, true),                                        //
        C("trunc", Vector{"f"}, false, true),                                            //
        C("unpack2x16float", Vector{"u"}, false, true),                                  //
        C("unpack2x16snorm", Vector{"u"}, false, true),                                  //
        C("unpack2x16unorm", Vector{"u"}, false, true),                                  //
        C("unpack4x8snorm", Vector{"u"}, false, true),                                   //
        C("unpack4x8unorm", Vector{"u"}, false, true),                                   //
        C("storageBarrier", tint::Empty, false, false, ast::PipelineStage::kCompute),    //
        C("workgroupBarrier", tint::Empty, false, false, ast::PipelineStage::kCompute),  //
        C("textureSample", Vector{"t2d", "s2d", "vf2"}, true, true),                     //
        C("textureSampleBias", Vector{"t2d", "s2d", "vf2", "f"}, true, true),            //
        C("textureSampleCompare", Vector{"tdepth2d", "scomp", "vf2", "f"}, true, true),  //
        C("dpdx", Vector{"f"}, true, true),                                              //
        C("dpdxCoarse", Vector{"f"}, true, true),                                        //
        C("dpdxFine", Vector{"f"}, true, true),                                          //
        C("dpdy", Vector{"f"}, true, true),                                              //
        C("dpdyCoarse", Vector{"f"}, true, true),                                        //
        C("dpdyFine", Vector{"f"}, true, true),                                          //
        C("fwidth", Vector{"f"}, true, true),                                            //
        C("fwidthCoarse", Vector{"f"}, true, true),                                      //
        C("fwidthFine", Vector{"f"}, true, true),                                        //

        // Side-effect builtins
        C("atomicAdd", Vector{"pa", "i"}, true, true),                       //
        C("atomicAnd", Vector{"pa", "i"}, true, true),                       //
        C("atomicCompareExchangeWeak", Vector{"pa", "i", "i"}, true, true),  //
        C("atomicExchange", Vector{"pa", "i"}, true, true),                  //
        C("atomicMax", Vector{"pa", "i"}, true, true),                       //
        C("atomicMin", Vector{"pa", "i"}, true, true),                       //
        C("atomicOr", Vector{"pa", "i"}, true, true),                        //
        C("atomicStore", Vector{"pa", "i"}, true, false),                    //
        C("atomicSub", Vector{"pa", "i"}, true, true),                       //
        C("atomicXor", Vector{"pa", "i"}, true, true),                       //
        C("textureStore", Vector{"tstorage2d", "vi2", "vf4"}, true, false),  //
        C("workgroupUniformLoad",
          Vector{"pworkgroup_arr"},
          true,
          true,
          ast::PipelineStage::kCompute),  //

        // Unimplemented builtins
        // C("quantizeToF16", Vector{"f"}, false), //
        // C("saturate", Vector{"f"}, false), //
    }));

}  // namespace builtin_tests

TEST_F(SideEffectsTest, Call_Function) {
    Func("f", tint::Empty, ty.i32(), Vector{Return(1_i)});
    auto* expr = Call("f");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_TypeConversion_NoSE) {
    auto* var = Decl(Var("a", ty.i32()));
    auto* expr = Call<f32>("a");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_TypeConversion_SE) {
    MakeSideEffectFunc<i32>("se");
    auto* expr = Call<f32>(Call("se"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_TypeInitializer_NoSE) {
    auto* var = Decl(Var("a", ty.f32()));
    auto* expr = Call<f32>("a");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_TypeInitializer_SE) {
    MakeSideEffectFunc<f32>("se");
    auto* expr = Call<f32>(Call("se"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, MemberAccessor_Struct_NoSE) {
    auto* s = Structure("S", Vector{Member("m", ty.i32())});
    auto* var = Decl(Var("a", ty.Of(s)));
    auto* expr = MemberAccessor("a", "m");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, MemberAccessor_Struct_SE) {
    auto* s = Structure("S", Vector{Member("m", ty.i32())});
    MakeSideEffectFunc("se", [&] { return ty.Of(s); });
    auto* expr = MemberAccessor(Call("se"), "m");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, MemberAccessor_Vector) {
    auto* var = Decl(Var("a", ty.vec4<f32>()));
    auto* expr = MemberAccessor("a", "x");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->UnwrapLoad()->Is<sem::MemberAccessorExpression>());
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, MemberAccessor_VectorSwizzleNoSE) {
    auto* var = Decl(Var("a", ty.vec4<f32>()));
    auto* expr = MemberAccessor("a", "xzyw");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Swizzle>());
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, MemberAccessor_VectorSwizzleSE) {
    MakeSideEffectFunc("se", [&] { return ty.vec4<f32>(); });
    auto* expr = MemberAccessor(Call("se"), "xzyw");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Swizzle>());
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Binary_NoSE) {
    auto* a = Decl(Var("a", ty.i32()));
    auto* b = Decl(Var("b", ty.i32()));
    auto* expr = Add("a", "b");
    WrapInFunction(a, b, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Binary_LeftSE) {
    MakeSideEffectFunc<i32>("se");
    auto* b = Decl(Var("b", ty.i32()));
    auto* expr = Add(Call("se"), "b");
    WrapInFunction(b, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Binary_RightSE) {
    MakeSideEffectFunc<i32>("se");
    auto* a = Decl(Var("a", ty.i32()));
    auto* expr = Add("a", Call("se"));
    WrapInFunction(a, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Binary_BothSE) {
    MakeSideEffectFunc<i32>("se1");
    MakeSideEffectFunc<i32>("se2");
    auto* expr = Add(Call("se1"), Call("se2"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Unary_NoSE) {
    auto* var = Decl(Var("a", ty.bool_()));
    auto* expr = Not("a");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Unary_SE) {
    MakeSideEffectFunc<bool>("se");
    auto* expr = Not(Call("se"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, IndexAccessor_NoSE) {
    auto* var = Decl(Var("a", ty.array<i32, 10>()));
    auto* expr = IndexAccessor("a", 0_i);
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, IndexAccessor_ObjSE) {
    MakeSideEffectFunc("se", [&] { return ty.array<i32, 10>(); });
    auto* expr = IndexAccessor(Call("se"), 0_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, IndexAccessor_IndexSE) {
    MakeSideEffectFunc<i32>("se");
    auto* var = Decl(Var("a", ty.array<i32, 10>()));
    auto* expr = IndexAccessor("a", Call("se"));
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, IndexAccessor_BothSE) {
    MakeSideEffectFunc("se1", [&] { return ty.array<i32, 10>(); });
    MakeSideEffectFunc<i32>("se2");
    auto* expr = IndexAccessor(Call("se1"), Call("se2"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Bitcast_NoSE) {
    auto* var = Decl(Var("a", ty.i32()));
    auto* expr = Bitcast<f32>("a");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Bitcast_SE) {
    MakeSideEffectFunc<i32>("se");
    auto* expr = Bitcast<f32>(Call("se"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

}  // namespace
}  // namespace tint::resolver
