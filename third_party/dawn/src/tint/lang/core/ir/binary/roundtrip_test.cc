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

#include "src/tint/lang/core/ir/ir_helper_test.h"

#include "src/tint/lang/core/io_attributes.h"
#include "src/tint/lang/core/ir/binary/decode.h"
#include "src/tint/lang/core/ir/binary/encode.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"

namespace tint::core::ir::binary {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

template <typename T = testing::Test>
class IRBinaryRoundtripTestBase : public IRTestParamHelper<T> {
  public:
    std::pair<std::string, std::string> Roundtrip() {
        auto pre = Disassembler(this->mod).Plain();
        auto encoded = EncodeToBinary(this->mod);
        if (encoded != Success) {
            return {pre, encoded.Failure().reason.Str()};
        }
        auto decoded = Decode(encoded->Slice());
        if (decoded != Success) {
            return {pre, decoded.Failure().reason.Str()};
        }
        auto post = Disassembler(decoded.Get()).Plain();
        return {pre, post};
    }
};

#define RUN_TEST()                      \
    {                                   \
        auto [pre, post] = Roundtrip(); \
        EXPECT_EQ(pre, post);           \
    }                                   \
    TINT_REQUIRE_SEMICOLON

using IRBinaryRoundtripTest = IRBinaryRoundtripTestBase<>;
TEST_F(IRBinaryRoundtripTest, EmptyModule) {
    RUN_TEST();
}

////////////////////////////////////////////////////////////////////////////////
// Root block
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRBinaryRoundtripTest, RootBlock_Var_private_i32_Unnamed) {
    b.Append(b.ir.root_block, [&] { b.Var<private_, i32>(); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, RootBlock_Var_workgroup_f32_Named) {
    b.Append(b.ir.root_block, [&] { b.Var<workgroup, f32>("WG"); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, RootBlock_Var_storage_binding) {
    b.Append(b.ir.root_block, [&] {
        auto* v = b.Var<storage, f32>();
        v->SetBindingPoint(10, 20);
    });
    RUN_TEST();
}

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRBinaryRoundtripTest, Fn_i32_ret) {
    b.Function("Function", ty.i32());
    RUN_TEST();
}

using IRBinaryRoundtripTest_FnPipelineStage = IRBinaryRoundtripTestBase<Function::PipelineStage>;
TEST_P(IRBinaryRoundtripTest_FnPipelineStage, Test) {
    b.Function("Function", ty.i32(), GetParam());
    RUN_TEST();
}
INSTANTIATE_TEST_SUITE_P(,
                         IRBinaryRoundtripTest_FnPipelineStage,
                         testing::Values(Function::PipelineStage::kCompute,
                                         Function::PipelineStage::kFragment,
                                         Function::PipelineStage::kVertex));

TEST_F(IRBinaryRoundtripTest, Fn_WorkgroupSize) {
    b.ComputeFunction("Function", 1_u, 2_u, 3_u);
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Fn_Parameters) {
    auto* fn = b.Function("Function", ty.void_());
    auto* p0 = b.FunctionParam(ty.i32());
    auto* p1 = b.FunctionParam(ty.u32());
    auto* p2 = b.FunctionParam(ty.f32());
    b.ir.SetName(p1, "p1");
    fn->SetParams({p0, p1, p2});
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Fn_ParameterAttributes) {
    auto* fn = b.Function("Function", ty.void_());
    auto* p0 = b.FunctionParam(ty.i32());
    auto* p1 = b.FunctionParam(ty.u32());
    auto* p2 = b.FunctionParam(ty.f32());
    auto* p3 = b.FunctionParam(ty.bool_());
    p0->SetBuiltin(BuiltinValue::kGlobalInvocationId);
    p1->SetInvariant(true);
    p2->SetLocation(10);
    p2->SetColor(50);
    p2->SetInterpolation(Interpolation{InterpolationType::kFlat, InterpolationSampling::kCenter});
    p3->SetBindingPoint(20, 30);
    fn->SetParams({p0, p1, p2, p3});
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Fn_ReturnBuiltin) {
    auto* fn = b.Function("Function", ty.void_());
    fn->SetReturnBuiltin(BuiltinValue::kFragDepth);
    b.ir.SetName(fn, "Function");
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Fn_ReturnLocation) {
    auto* fn = b.Function("Function", ty.void_());
    fn->SetReturnLocation(42);
    b.ir.SetName(fn, "Function");
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Fn_ReturnLocation_Interpolation) {
    auto* fn = b.Function("Function", ty.void_());
    fn->SetReturnLocation(0);
    fn->SetReturnInterpolation(core::Interpolation{
        core::InterpolationType::kPerspective,
        core::InterpolationSampling::kCentroid,
    });
    b.ir.SetName(fn, "Function");
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Fn_ReturnInvariant) {
    auto* fn = b.Function("Function", ty.void_());
    fn->SetReturnInvariant(true);
    b.ir.SetName(fn, "Function");
    RUN_TEST();
}

////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRBinaryRoundtripTest, bool) {
    b.Append(b.ir.root_block, [&] { b.Var<private_, bool>(); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, i32) {
    b.Append(b.ir.root_block, [&] { b.Var<private_, i32>(); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, u32) {
    b.Append(b.ir.root_block, [&] { b.Var<private_, u32>(); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, f32) {
    b.Append(b.ir.root_block, [&] { b.Var<private_, f32>(); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, f16) {
    b.Append(b.ir.root_block, [&] { b.Var<private_, f16>(); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, vec2_f32) {
    b.Append(b.ir.root_block, [&] { b.Var<private_, vec2<f32>>(); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, vec3_i32) {
    b.Append(b.ir.root_block, [&] { b.Var<private_, vec3<i32>>(); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, vec4_bool) {
    b.Append(b.ir.root_block, [&] { b.Var<private_, vec4<bool>>(); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, mat4x2_f32) {
    b.Append(b.ir.root_block, [&] { b.Var<private_, vec4<mat4x2<f32>>>(); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, mat2x4_f16) {
    b.Append(b.ir.root_block, [&] { b.Var<private_, vec4<mat2x4<f16>>>(); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, ptr_function_f32_read_write) {
    auto p = b.FunctionParam<ptr<function, f32, read_write>>("p");
    auto* fn = b.Function("Function", ty.void_());
    fn->SetParams({p});
    b.Append(fn->Block(), [&] { b.Return(fn); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, ptr_workgroup_i32_read) {
    auto p = b.FunctionParam<ptr<workgroup, i32, read>>("p");
    auto* fn = b.Function("Function", ty.void_());
    fn->SetParams({p});
    b.Append(fn->Block(), [&] { b.Return(fn); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, array_i32_4) {
    b.Append(b.ir.root_block, [&] { b.Var<private_, array<i32, 4>>(); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, array_i32_runtime_sized) {
    b.Append(b.ir.root_block, [&] { b.Var<storage, array<i32>>(); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, struct) {
    Vector members{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("a"), ty.i32(), /* index */ 0u,
                                         /* offset */ 0u, /* align */ 4u, /* size */ 4u,
                                         core::IOAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("b"), ty.f32(), /* index */ 1u,
                                         /* offset */ 4u, /* align */ 4u, /* size */ 32u,
                                         core::IOAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("c"), ty.u32(), /* index */ 2u,
                                         /* offset */ 36u, /* align */ 4u, /* size */ 4u,
                                         core::IOAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("d"), ty.u32(), /* index */ 3u,
                                         /* offset */ 64u, /* align */ 32u, /* size */ 4u,
                                         core::IOAttributes{}),
    };
    auto* S = ty.Struct(b.ir.symbols.New("S"), std::move(members));
    b.Append(b.ir.root_block, [&] { b.Var(ty.ptr<function, read_write>(S)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, IOAttributes) {
    core::IOAttributes attrs{};
    attrs.location = 1;
    attrs.blend_src = 2;
    attrs.color = 3;
    attrs.builtin = core::BuiltinValue::kFragDepth;
    attrs.interpolation = core::Interpolation{
        core::InterpolationType::kLinear,
        core::InterpolationSampling::kCentroid,
    };
    attrs.invariant = true;
    Vector members{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("a"), ty.i32(), /* index */ 0u,
                                         /* offset */ 0u, /* align */ 4u, /* size */ 4u, attrs),
    };
    auto* S = ty.Struct(b.ir.symbols.New("S"), std::move(members));
    b.Append(b.ir.root_block, [&] { b.Var(ty.ptr<function, read_write>(S)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, atomic_i32) {
    b.Append(b.ir.root_block, [&] { b.Var<storage, atomic<i32>>(); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, depth_texture) {
    auto* tex = ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d);
    b.Append(b.ir.root_block, [&] { b.Var(ty.ptr(handle, tex, read)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, sampled_texture) {
    auto* tex = ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k3d, ty.i32());
    b.Append(b.ir.root_block, [&] { b.Var(ty.ptr(handle, tex, read)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, multisampled_texture) {
    auto* tex =
        ty.Get<core::type::MultisampledTexture>(core::type::TextureDimension::k2d, ty.f32());
    b.Append(b.ir.root_block, [&] { b.Var(ty.ptr(handle, tex, read)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, depth_multisampled_texture) {
    auto* tex = ty.Get<core::type::DepthMultisampledTexture>(core::type::TextureDimension::k2d);
    b.Append(b.ir.root_block, [&] { b.Var(ty.ptr(handle, tex, read)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, storage_texture) {
    auto* tex = ty.Get<core::type::StorageTexture>(core::type::TextureDimension::k2dArray,
                                                   core::TexelFormat::kRg32Float,
                                                   core::Access::kReadWrite, ty.f32());
    b.Append(b.ir.root_block, [&] { b.Var(ty.ptr(handle, tex, read)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, external_texture) {
    auto* tex = ty.Get<core::type::ExternalTexture>();
    b.Append(b.ir.root_block, [&] { b.Var(ty.ptr(handle, tex, read)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, sampler) {
    auto* sampler = ty.Get<core::type::Sampler>(core::type::SamplerKind::kSampler);
    b.Append(b.ir.root_block, [&] { b.Var(ty.ptr(handle, sampler, read)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, comparision_sampler) {
    auto* sampler = ty.Get<core::type::Sampler>(core::type::SamplerKind::kComparisonSampler);
    b.Append(b.ir.root_block, [&] { b.Var(ty.ptr(handle, sampler, read)); });
    RUN_TEST();
}

////////////////////////////////////////////////////////////////////////////////
// Instructions
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRBinaryRoundtripTest, Return) {
    auto* fn = b.Function("Function", ty.void_());
    b.Append(fn->Block(), [&] { b.Return(fn); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_bool) {
    auto* fn = b.Function("Function", ty.bool_());
    b.Append(fn->Block(), [&] { b.Return(fn, true); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_i32) {
    auto* fn = b.Function("Function", ty.i32());
    b.Append(fn->Block(), [&] { b.Return(fn, 42_i); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_u32) {
    auto* fn = b.Function("Function", ty.u32());
    b.Append(fn->Block(), [&] { b.Return(fn, 42_u); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_f32) {
    auto* fn = b.Function("Function", ty.f32());
    b.Append(fn->Block(), [&] { b.Return(fn, 42_f); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_f16) {
    auto* fn = b.Function("Function", ty.f16());
    b.Append(fn->Block(), [&] { b.Return(fn, 42_h); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_vec3f_Composite) {
    auto* fn = b.Function("Function", ty.vec3<f32>());
    b.Append(fn->Block(), [&] { b.Return(fn, b.Composite<vec3<f32>>(1_f, 2_f, 3_f)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_vec3f_Splat) {
    auto* fn = b.Function("Function", ty.vec3<f32>());
    b.Append(fn->Block(), [&] { b.Return(fn, b.Splat<vec3<f32>>(1_f)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_mat2x3f_Composite) {
    auto* fn = b.Function("Function", ty.mat2x3<f32>());
    b.Append(fn->Block(), [&] {
        b.Return(fn, b.Composite<mat2x3<f32>>(b.Composite<vec3<f32>>(1_f, 2_f, 3_f),
                                              b.Composite<vec3<f32>>(4_f, 5_f, 6_f)));
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_mat2x3f_Splat) {
    auto* fn = b.Function("Function", ty.mat2x3<f32>());
    b.Append(fn->Block(), [&] { b.Return(fn, b.Splat<mat2x3<f32>>(b.Splat<vec3<f32>>(1_f))); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_array_f32_Composite) {
    auto* fn = b.Function("Function", ty.array<f32, 3>());
    b.Append(fn->Block(), [&] { b.Return(fn, b.Composite<array<f32, 3>>(1_f, 2_f, 3_f)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_array_f32_Splat) {
    auto* fn = b.Function("Function", ty.array<f32, 3>());
    b.Append(fn->Block(), [&] { b.Return(fn, b.Splat<array<f32, 3>>(1_f)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Construct) {
    auto* fn = b.Function("Function", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Construct<vec3<f32>>(1_f, 2_f, 3_f);
        b.Return(fn);
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Discard) {
    auto* fn = b.Function("Function", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Discard();
        b.Return(fn);
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Let) {
    auto* fn = b.Function("Function", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Let("Let", b.Constant(42_i));
        b.Return(fn);
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Var) {
    auto* fn = b.Function("Function", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Var<function>("Var", b.Constant(42_i));
        b.Return(fn);
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Access) {
    auto* fn = b.Function("Function", ty.f32());
    b.Append(fn->Block(),
             [&] { b.Return(fn, b.Access<f32>(b.Construct<mat4x4<f32>>(), 1_u, 2_u)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, UserCall) {
    auto* fn_a = b.Function("A", ty.f32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 42_f); });
    auto* fn_b = b.Function("B", ty.f32());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, b.Call(fn_a)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, BuiltinCall) {
    auto* fn = b.Function("Function", ty.f32());
    b.Append(fn->Block(), [&] { b.Return(fn, b.Call<i32>(core::BuiltinFn::kMax, 1_i, 2_i)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Load) {
    auto p = b.FunctionParam<ptr<function, f32, read_write>>("p");
    auto* fn = b.Function("Function", ty.f32());
    fn->SetParams({p});
    b.Append(fn->Block(), [&] { b.Return(fn, b.Load(p)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Store) {
    auto p = b.FunctionParam<ptr<function, f32, read_write>>("p");
    auto* fn = b.Function("Function", ty.void_());
    fn->SetParams({p});
    b.Append(fn->Block(), [&] {
        b.Store(p, 42_f);
        b.Return(fn);
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, LoadVectorElement) {
    auto p = b.FunctionParam<ptr<function, vec3<f32>, read_write>>("p");
    auto* fn = b.Function("Function", ty.f32());
    fn->SetParams({p});
    b.Append(fn->Block(), [&] { b.Return(fn, b.LoadVectorElement(p, 1_i)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, StoreVectorElement) {
    auto p = b.FunctionParam<ptr<function, vec3<f32>, read_write>>("p");
    auto* fn = b.Function("Function", ty.void_());
    fn->SetParams({p});
    b.Append(fn->Block(), [&] {
        b.StoreVectorElement(p, 1_u, 42_f);
        b.Return(fn);
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, UnaryOp) {
    auto* x = b.FunctionParam<bool>("x");
    auto* fn = b.Function("Function", ty.bool_());
    fn->SetParams({x});
    b.Append(fn->Block(), [&] { b.Return(fn, b.Not<bool>(x)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, BinaryOp) {
    auto* x = b.FunctionParam<f32>("x");
    auto* y = b.FunctionParam<f32>("y");
    auto* fn = b.Function("Function", ty.f32());
    fn->SetParams({x, y});
    b.Append(fn->Block(), [&] { b.Return(fn, b.Add<f32>(x, y)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Swizzle) {
    auto* x = b.FunctionParam<vec4<f32>>("x");
    auto* fn = b.Function("Function", ty.vec3<f32>());
    fn->SetParams({x});
    b.Append(fn->Block(), [&] {
        b.Return(fn, b.Swizzle<vec3<f32>>(x, Vector<uint32_t, 3>{1, 0, 2}));
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Bitcast) {
    auto* x = b.FunctionParam<vec4<f32>>("x");
    auto* fn = b.Function("Function", ty.vec4<u32>());
    fn->SetParams({x});
    b.Append(fn->Block(), [&] { b.Return(fn, b.Bitcast<vec4<u32>>(x)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Convert) {
    auto* x = b.FunctionParam<vec4<f32>>("x");
    auto* fn = b.Function("Function", ty.vec4<u32>());
    fn->SetParams({x});
    b.Append(fn->Block(), [&] { b.Return(fn, b.Convert<vec4<u32>>(x)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, IfTrue) {
    auto* cond = b.FunctionParam<bool>("cond");
    auto* x = b.FunctionParam<i32>("x");
    auto* fn = b.Function("Function", ty.i32());
    fn->SetParams({cond, x});
    b.Append(fn->Block(), [&] {
        auto* if_ = b.If(cond);
        b.Append(if_->True(), [&] { b.Return(fn, x); });
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, IfFalse) {
    auto* cond = b.FunctionParam<bool>("cond");
    auto* x = b.FunctionParam<i32>("x");
    auto* fn = b.Function("Function", ty.i32());
    fn->SetParams({cond, x});
    b.Append(fn->Block(), [&] {
        auto* if_ = b.If(cond);
        b.Append(if_->False(), [&] { b.Return(fn, x); });
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, IfTrueFalse) {
    auto* cond = b.FunctionParam<bool>("cond");
    auto* x = b.FunctionParam<i32>("x");
    auto* y = b.FunctionParam<i32>("y");
    auto* fn = b.Function("Function", ty.i32());
    fn->SetParams({cond, x, y});
    b.Append(fn->Block(), [&] {
        auto* if_ = b.If(cond);
        b.Append(if_->True(), [&] { b.Return(fn, x); });
        b.Append(if_->False(), [&] { b.Return(fn, y); });
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, IfResults) {
    auto* cond = b.FunctionParam<bool>("cond");
    auto* fn = b.Function("Function", ty.i32());
    fn->SetParams({cond});
    b.Append(fn->Block(), [&] {
        auto* if_ = b.If(cond);
        auto* res_a = b.InstructionResult<i32>();
        auto* res_b = b.InstructionResult<f32>();
        if_->SetResults(Vector{res_a, res_b});
        b.Append(if_->True(), [&] { b.ExitIf(if_, 1_i, 2_f); });
        b.Append(if_->False(), [&] { b.ExitIf(if_, 3_i, 4_f); });
        b.Return(fn, res_a);
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Switch) {
    auto* x = b.FunctionParam<i32>("x");
    auto* fn = b.Function("Function", ty.i32());
    fn->SetParams({x});
    b.Append(fn->Block(), [&] {
        auto* switch_ = b.Switch(x);
        b.Append(b.Case(switch_, {b.Constant(1_i)}), [&] { b.Return(fn, 1_i); });
        b.Append(b.Case(switch_, {b.Constant(2_i), b.Constant(3_i)}), [&] { b.Return(fn, 2_i); });
        b.Append(b.Case(switch_, {nullptr}), [&] { b.Return(fn, 3_i); });
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, SwitchResults) {
    auto* x = b.FunctionParam<i32>("x");
    auto* fn = b.Function("Function", ty.i32());
    fn->SetParams({x});
    b.Append(fn->Block(), [&] {
        auto* switch_ = b.Switch(x);
        auto* res = b.InstructionResult<i32>();
        switch_->SetResults(Vector{res});
        b.Append(b.Case(switch_, {b.Constant(1_i)}), [&] { b.ExitSwitch(switch_, 1_i); });
        b.Append(b.Case(switch_, {b.Constant(2_i), b.Constant(3_i)}),
                 [&] { b.ExitSwitch(switch_, 2_i); });
        b.Append(b.Case(switch_, {nullptr}), [&] { b.ExitSwitch(switch_, 3_i); });
        b.Return(fn, res);
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, LoopBody) {
    auto* fn = b.Function("Function", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Return(fn, 1_i); });
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, LoopInitBody) {
    auto* fn = b.Function("Function", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            b.Let("L", 1_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.Return(fn, 2_i); });
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, LoopInitBodyCont) {
    auto* fn = b.Function("Function", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            b.Let("L", 1_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, false); });
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, LoopResults) {
    auto* fn = b.Function("Function", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();
        auto* res = b.InstructionResult<i32>();
        loop->SetResults(Vector{res});
        b.Append(loop->Body(), [&] { b.ExitLoop(loop, 1_i); });
        b.Return(fn, res);
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, LoopBlockParams) {
    auto* fn = b.Function("Function", ty.void_());
    b.Append(fn->Block(), [&] {
        auto* loop_res_a = b.InstructionResult(ty.i32());
        auto* loop_res_b = b.InstructionResult(ty.f32());
        auto* loop = b.Loop();
        loop->SetResults(Vector{loop_res_a, loop_res_b});
        b.Append(loop->Initializer(), [&] {
            b.Let("L", 1_i);
            b.NextIteration(loop);
        });
        auto* x = b.BlockParam<i32>("x");
        auto* y = b.BlockParam<f32>("y");
        loop->Body()->SetParams({x, y});
        b.Append(loop->Body(), [&] { b.Continue(loop, 1_u, true); });
        auto* z = b.BlockParam<u32>("z");
        auto* w = b.BlockParam<bool>("w");
        loop->Continuing()->SetParams({z, w});
        b.Append(loop->Continuing(), [&] {
            b.BreakIf(loop,
                      /* condition */ false,
                      /* next iter */ b.Values(3_i, 4_f),
                      /* exit */ b.Values(5_u, 6_i));
        });
        b.Return(fn);
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Unreachable) {
    auto* fn = b.Function("Function", ty.i32());
    b.Append(fn->Block(), [&] { b.Unreachable(); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, InputAttachment) {
    b.Append(b.ir.root_block, [&] {
        auto* input_type = ty.Get<core::type::InputAttachment>(ty.i32());
        auto* v = b.Var(ty.ptr(handle, input_type, read));
        v->SetBindingPoint(10, 20);
        v->SetInputAttachmentIndex(11);

        auto* fn = b.Function("Function", ty.vec4<i32>());
        b.Append(fn->Block(),
                 [&] { b.Return(fn, b.Call<i32>(core::BuiltinFn::kInputAttachmentLoad, v)); });
    });
    RUN_TEST();
}

}  // namespace
}  // namespace tint::core::ir::binary
