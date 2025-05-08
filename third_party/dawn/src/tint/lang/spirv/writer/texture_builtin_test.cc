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

#include "src/tint/lang/core/builtin_fn.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/spirv/writer/common/helper_test.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::spirv::writer {
namespace {

enum TextureType {
    kSampledTexture,
    kMultisampledTexture,
    kDepthTexture,
    kDepthMultisampledTexture,
    kStorageTexture,
};

enum SamplerUsage {
    kNoSampler,
    kSampler,
    kComparisonSampler,
};

/// A typed argument or result for a texture builtin.
struct NameAndType {
    /// The name.
    const char* name;
    /// The vector width of the value (1 means scalar).
    uint32_t width;
    /// The element type of the value.
    TestElementType type;
};

/// A parameterized texture builtin function test case.
struct TextureBuiltinTestCase {
    /// The texture type.
    TextureType texture_type;
    /// The dimensionality of the texture.
    core::type::TextureDimension dim;
    /// The texel type of the texture.
    TestElementType texel_type;
    /// The builtin function arguments.
    Vector<NameAndType, 4> args;
    /// The result type.
    NameAndType result;
    /// The expected SPIR-V instruction strings.
    Vector<const char*, 2> instructions;
};

template <typename STREAM, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& out, TextureType type) {
    switch (type) {
        case kSampledTexture:
            out << "SampleTexture";
            break;
        case kMultisampledTexture:
            out << "MultisampleTexture";
            break;
        case kDepthTexture:
            out << "DepthTexture";
            break;
        case kDepthMultisampledTexture:
            out << "DepthMultisampledTexture";
            break;
        case kStorageTexture:
            out << "StorageTexture";
            break;
    }
    return out;
}

std::string PrintCase(testing::TestParamInfo<TextureBuiltinTestCase> cc) {
    StringStream ss;
    ss << cc.param.texture_type << cc.param.dim << "_" << cc.param.texel_type;
    for (const auto& arg : cc.param.args) {
        ss << "_" << arg.name;
    }
    return ss.str();
}

class TextureBuiltinTest : public SpirvWriterTestWithParam<TextureBuiltinTestCase> {
  protected:
    const core::type::Texture* MakeTextureType(TextureType type,
                                               core::type::TextureDimension dim,
                                               TestElementType texel_type) {
        switch (type) {
            case kSampledTexture:
                return ty.sampled_texture(dim, MakeScalarType(texel_type));
            case kMultisampledTexture:
                return ty.Get<core::type::MultisampledTexture>(dim, MakeScalarType(texel_type));
            case kDepthTexture:
                return ty.Get<core::type::DepthTexture>(dim);
            case kDepthMultisampledTexture:
                return ty.Get<core::type::DepthMultisampledTexture>(dim);
            case kStorageTexture:
                core::TexelFormat format;
                switch (texel_type) {
                    case kF32:
                        format = core::TexelFormat::kR32Float;
                        break;
                    case kI32:
                        format = core::TexelFormat::kR32Sint;
                        break;
                    case kU32:
                        format = core::TexelFormat::kR32Uint;
                        break;
                    default:
                        return nullptr;
                }
                return ty.storage_texture(dim, format, core::Access::kWrite);
        }
        return nullptr;
    }

    void Run(enum core::BuiltinFn function, SamplerUsage sampler) {
        auto params = GetParam();

        auto* result_ty = MakeScalarType(params.result.type);
        if (function == core::BuiltinFn::kTextureStore) {
            result_ty = ty.void_();
        }
        if (params.result.width > 1) {
            result_ty = ty.vec(result_ty, params.result.width);
        }

        Vector<core::ir::FunctionParam*, 4> func_params;

        auto* t = b.FunctionParam(
            "t", MakeTextureType(params.texture_type, params.dim, params.texel_type));
        func_params.Push(t);
        core::ir::FunctionParam* s = nullptr;
        if (sampler == kSampler) {
            s = b.FunctionParam("s", ty.sampler());
            func_params.Push(s);
        } else if (sampler == kComparisonSampler) {
            s = b.FunctionParam("s", ty.comparison_sampler());
            func_params.Push(s);
        }

        auto* func = b.Function("foo", result_ty);
        func->SetParams(std::move(func_params));

        b.Append(func->Block(), [&] {
            uint32_t arg_value = 1;

            Vector<core::ir::Value*, 4> args;
            if (function == core::BuiltinFn::kTextureGather &&
                params.texture_type != kDepthTexture) {
                // Special case for textureGather, which has a component argument first.
                auto* component = MakeScalarValue(kU32, arg_value++);
                args.Push(component);
                mod.SetName(component, "component");
            }
            args.Push(t);
            if (s) {
                args.Push(s);
            }

            for (const auto& arg : params.args) {
                auto* value = MakeScalarValue(arg.type, arg_value++);
                if (arg.width > 1) {
                    value = b.Splat(ty.vec(value->Type(), arg.width), value);
                }
                args.Push(value);
                mod.SetName(value, arg.name);
            }
            auto* result = b.Call(result_ty, function, std::move(args));
            if (result_ty->Is<core::type::Void>()) {
                b.Return(func);
            } else {
                b.Return(func, result);
                mod.SetName(result, "result");
            }
        });

        Options options;
        options.disable_image_robustness = true;
        ASSERT_TRUE(Generate(options)) << Error() << output_;
        for (auto& inst : params.instructions) {
            EXPECT_INST(inst);
        }
    }
};

////////////////////////////////////////////////////////////////
//// textureSample
////////////////////////////////////////////////////////////////
using TextureSample = TextureBuiltinTest;
TEST_P(TextureSample, Emit) {
    Run(core::BuiltinFn::kTextureSample, kSampler);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    TextureSample,
    testing::Values(
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k1d,
            /* texel type */ kF32,
            {{"coord", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleImplicitLod %v4float %10 %coord None",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleImplicitLod %v4float %10 %coords None",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleImplicitLod %v4float %10 %coords ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "OpImageSampleImplicitLod %v4float %10 %16 None",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "OpImageSampleImplicitLod %v4float %10 %16 ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k3d,
            /* texel type */ kF32,
            {{"coords", 3, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleImplicitLod %v4float %10 %coords None",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k3d,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"offset", 3, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleImplicitLod %v4float %10 %coords ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleImplicitLod %v4float %10 %coords None",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %12",
                "OpImageSampleImplicitLod %v4float %10 %15 None",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "OpImageSampleImplicitLod %v4float %9 %coords None",
                "%result = OpCompositeExtract %float",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"offset", 2, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "OpImageSampleImplicitLod %v4float %9 %coords ConstOffset %offset",
                "%result = OpCompositeExtract %float",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "OpImageSampleImplicitLod %v4float %9 %coords None",
                "%result = OpCompositeExtract %float",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v3float %coords %11",
                "OpImageSampleImplicitLod %v4float %9 %15 None",
                "%result = OpCompositeExtract %float",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"offset", 2, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v3float %coords %11",
                "OpImageSampleImplicitLod %v4float %9 %15 ConstOffset %offset",
                "%result = OpCompositeExtract %float",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %11",
                "OpImageSampleImplicitLod %v4float %9 %15 None",
            },
        }),
    PrintCase);

////////////////////////////////////////////////////////////////
//// textureSampleBias
////////////////////////////////////////////////////////////////
using TextureSampleBias = TextureBuiltinTest;
TEST_P(TextureSampleBias, Emit) {
    Run(core::BuiltinFn::kTextureSampleBias, kSampler);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    TextureSampleBias,
    testing::Values(
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"bias", 1, kF32}},
            {"result", 4, kF32},
            {
                "OpExtInst %float %11 NClamp %bias %float_n16 %float_15_9899998",
                "OpSampledImage %16 %t %s",
                "OpImageSampleImplicitLod %v4float %15 %coords Bias %10",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"bias", 1, kF32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "OpExtInst %float %11 NClamp %bias %float_n16 %float_15_9899998",
                "OpSampledImage %16 %t %s",
                "OpImageSampleImplicitLod %v4float %15 %coords Bias|ConstOffset %10 %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"bias", 1, kF32}},
            {"result", 4, kF32},
            {
                "OpExtInst %float %11 NClamp %bias %float_n16 %float_15_9899998",
                "OpSampledImage %16 %t %s",
                "OpConvertSToF %float %array_idx",
                "OpCompositeConstruct %v3float %coords %17",
                "OpImageSampleImplicitLod %v4float %15 %21 Bias %10",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"bias", 1, kF32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "OpExtInst %float %11 NClamp %bias %float_n16 %float_15_9899998",
                "OpSampledImage %16 %t %s",
                "OpConvertSToF %float %array_idx",
                "OpCompositeConstruct %v3float %coords %17",
                "OpImageSampleImplicitLod %v4float %15 %21 Bias|ConstOffset %10 %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k3d,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"bias", 1, kF32}},
            {"result", 4, kF32},
            {
                "OpExtInst %float %11 NClamp %bias %float_n16 %float_15_9899998",
                "OpSampledImage %16 %t %s",
                "OpImageSampleImplicitLod %v4float %15 %coords Bias %10",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k3d,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"bias", 1, kF32}, {"offset", 3, kI32}},
            {"result", 4, kF32},
            {
                "OpExtInst %float %11 NClamp %bias %float_n16 %float_15_9899998",
                "OpSampledImage %16 %t %s",
                "OpImageSampleImplicitLod %v4float %15 %coords Bias|ConstOffset %10 %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"bias", 1, kF32}},
            {"result", 4, kF32},
            {
                "OpExtInst %float %11 NClamp %bias %float_n16 %float_15_9899998",
                "OpSampledImage %16 %t %s",
                "OpImageSampleImplicitLod %v4float %15 %coords Bias %10",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}, {"bias", 1, kF32}},
            {"result", 4, kF32},
            {
                "OpExtInst %float %11 NClamp %bias %float_n16 %float_15_9899998",
                "OpSampledImage %16 %t %s",
                "OpConvertSToF %float %array_idx",
                "OpCompositeConstruct %v4float %coords %17",
                "OpImageSampleImplicitLod %v4float %15 %20 Bias %10",
            },
        }),
    PrintCase);

////////////////////////////////////////////////////////////////
//// textureSampleGrad
////////////////////////////////////////////////////////////////
using TextureSampleGrad = TextureBuiltinTest;
TEST_P(TextureSampleGrad, Emit) {
    Run(core::BuiltinFn::kTextureSampleGrad, kSampler);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    TextureSampleGrad,
    testing::Values(
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"ddx", 2, kF32}, {"ddy", 2, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleExplicitLod %v4float %10 %coords Grad %ddx %ddy",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"ddx", 2, kF32}, {"ddy", 2, kF32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleExplicitLod %v4float %10 %coords Grad|ConstOffset %ddx %ddy %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"ddx", 2, kF32}, {"ddy", 2, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "OpImageSampleExplicitLod %v4float %10 %16 Grad %ddx %ddy",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32},
             {"array_idx", 1, kI32},
             {"ddx", 2, kF32},
             {"ddy", 2, kF32},
             {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "OpImageSampleExplicitLod %v4float %10 %16 Grad|ConstOffset %ddx %ddy %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k3d,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"ddx", 3, kF32}, {"ddy", 3, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleExplicitLod %v4float %10 %coords Grad %ddx %ddy",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k3d,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"ddx", 3, kF32}, {"ddy", 3, kF32}, {"offset", 3, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleExplicitLod %v4float %10 %coords Grad|ConstOffset %ddx %ddy %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"ddx", 3, kF32}, {"ddy", 3, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleExplicitLod %v4float %10 %coords Grad %ddx %ddy",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}, {"ddx", 3, kF32}, {"ddy", 3, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %12",
                "OpImageSampleExplicitLod %v4float %10 %15 Grad %ddx %ddy",
            },
        }),
    PrintCase);

////////////////////////////////////////////////////////////////
//// textureSampleLevel
////////////////////////////////////////////////////////////////
using TextureSampleLevel = TextureBuiltinTest;
TEST_P(TextureSampleLevel, Emit) {
    Run(core::BuiltinFn::kTextureSampleLevel, kSampler);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    TextureSampleLevel,
    testing::Values(
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"lod", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleExplicitLod %v4float %10 %coords Lod %lod",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"lod", 1, kF32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleExplicitLod %v4float %10 %coords Lod|ConstOffset %lod %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"lod", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "OpImageSampleExplicitLod %v4float %10 %16 Lod %lod",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"lod", 1, kF32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "OpImageSampleExplicitLod %v4float %10 %16 Lod|ConstOffset %lod %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k3d,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"lod", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleExplicitLod %v4float %10 %coords Lod %lod",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k3d,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"lod", 1, kF32}, {"offset", 3, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleExplicitLod %v4float %10 %coords Lod|ConstOffset %lod %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"lod", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleExplicitLod %v4float %10 %coords Lod %lod",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}, {"lod", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %12",
                "OpImageSampleExplicitLod %v4float %10 %15 Lod %lod",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"lod", 1, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %lod",
                "OpImageSampleExplicitLod %v4float %9 %coords Lod %11",
                "%result = OpCompositeExtract %float",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"lod", 1, kI32}, {"offset", 2, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %lod",
                "OpImageSampleExplicitLod %v4float %9 %coords Lod|ConstOffset %11 %offset",
                "%result = OpCompositeExtract %float",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"lod", 1, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v3float %coords %11",
                "%19 = OpConvertSToF %float %lod",
                "OpImageSampleExplicitLod %v4float %9 %15 Lod %19",
                "%result = OpCompositeExtract %float",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"lod", 1, kI32}, {"offset", 2, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v3float %coords %11",
                "%19 = OpConvertSToF %float %lod",
                "OpImageSampleExplicitLod %v4float %9 %15 Lod|ConstOffset %19 %offset",
                "%result = OpCompositeExtract %float",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"lod", 1, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %lod",
                "OpImageSampleExplicitLod %v4float %9 %coords Lod %11",
                "%result = OpCompositeExtract %float",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}, {"lod", 1, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %11",
                "%19 = OpConvertSToF %float %lod",
                "OpImageSampleExplicitLod %v4float %9 %15 Lod %19",
            },
        }),
    PrintCase);

////////////////////////////////////////////////////////////////
//// textureSampleCompare
////////////////////////////////////////////////////////////////
using TextureSampleCompare = TextureBuiltinTest;
TEST_P(TextureSampleCompare, Emit) {
    Run(core::BuiltinFn::kTextureSampleCompare, kComparisonSampler);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    TextureSampleCompare,
    testing::Values(
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"depth", 1, kF32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "OpImageSampleDrefImplicitLod %float %9 %coords %depth",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"depth", 1, kF32}, {"offset", 2, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "OpImageSampleDrefImplicitLod %float %9 %coords %depth ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"depth", 1, kF32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v3float %coords %11",
                "OpImageSampleDrefImplicitLod %float %9 %15 %depth",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"depth", 1, kF32}, {"offset", 2, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v3float %coords %11",
                "OpImageSampleDrefImplicitLod %float %9 %15 %depth ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"depth", 1, kF32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "OpImageSampleDrefImplicitLod %float %9 %coords %depth",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}, {"depth", 1, kF32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %11",
                "OpImageSampleDrefImplicitLod %float %9 %15 %depth",
            },
        }),
    PrintCase);

////////////////////////////////////////////////////////////////
//// textureSampleCompareLevel
////////////////////////////////////////////////////////////////
using TextureSampleCompareLevel = TextureBuiltinTest;
TEST_P(TextureSampleCompareLevel, Emit) {
    Run(core::BuiltinFn::kTextureSampleCompareLevel, kComparisonSampler);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    TextureSampleCompareLevel,
    testing::Values(
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"depth_l0", 1, kF32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "OpImageSampleDrefExplicitLod %float %9 %coords %depth_l0 Lod %float_0",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"depth_l0", 1, kF32}, {"offset", 2, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "OpImageSampleDrefExplicitLod %float %9 %coords %depth_l0 Lod|ConstOffset %float_0 "
                "%offset",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"depth_l0", 1, kF32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v3float %coords %11",
                "OpImageSampleDrefExplicitLod %float %9 %15 %depth_l0 Lod %float_0",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32},
             {"array_idx", 1, kI32},
             {"depth_l0", 1, kF32},
             {"offset", 2, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v3float %coords %11",
                "OpImageSampleDrefExplicitLod %float %9 %15 %depth_l0 Lod|ConstOffset %float_0 "
                "%offset",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"depth_l0", 1, kF32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "OpImageSampleDrefExplicitLod %float %9 %coords %depth_l0 Lod %float_0",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}, {"depth_l0", 1, kF32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %11",
                "OpImageSampleDrefExplicitLod %float %9 %15 %depth_l0 Lod %float_0",
            },
        }),
    PrintCase);

////////////////////////////////////////////////////////////////
//// textureGather
////////////////////////////////////////////////////////////////
using TextureGather = TextureBuiltinTest;
TEST_P(TextureGather, Emit) {
    Run(core::BuiltinFn::kTextureGather, kSampler);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    TextureGather,
    testing::Values(
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageGather %v4float %10 %coords %component None",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageGather %v4float %10 %coords %component ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "%result = OpImageGather %v4float %10 %16 %component None",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "%result = OpImageGather %v4float %10 %16 %component ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageGather %v4float %10 %coords %component None",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %12",
                "%result = OpImageGather %v4float %10 %15 %component None",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageGather %v4float %10 %coords %uint_0 None",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageGather %v4float %10 %coords %uint_0 ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageGather %v4float %10 %coords %uint_0 None",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "%result = OpImageGather %v4float %10 %16 %uint_0 None",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "%result = OpImageGather %v4float %10 %16 %uint_0 ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %12",
                "%result = OpImageGather %v4float %10 %15 %uint_0 None",
            },
        },

        // Test some textures with integer texel types.
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kI32,
            {{"coords", 2, kF32}},
            {"result", 4, kI32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageGather %v4int %10 %coords %component None",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kU32,
            {{"coords", 2, kF32}},
            {"result", 4, kU32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageGather %v4uint %10 %coords %component None",
            },
        }),
    PrintCase);

////////////////////////////////////////////////////////////////
//// textureGatherCompare
////////////////////////////////////////////////////////////////
using TextureGatherCompare = TextureBuiltinTest;
TEST_P(TextureGatherCompare, Emit) {
    Run(core::BuiltinFn::kTextureGatherCompare, kComparisonSampler);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    TextureGatherCompare,
    testing::Values(
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"depth", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageDrefGather %v4float %10 %coords %depth None",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"depth", 1, kF32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageDrefGather %v4float %10 %coords %depth ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"depth", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "%result = OpImageDrefGather %v4float %10 %16 %depth None",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"depth", 1, kF32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "%result = OpImageDrefGather %v4float %10 %16 %depth ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"depth", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageDrefGather %v4float %10 %coords %depth None",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            core::type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}, {"depth", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %12",
                "%result = OpImageDrefGather %v4float %10 %15 %depth None",
            },
        }),
    PrintCase);

////////////////////////////////////////////////////////////////
//// textureLoad
////////////////////////////////////////////////////////////////
using TextureLoad = TextureBuiltinTest;
TEST_P(TextureLoad, Emit) {
    Run(core::BuiltinFn::kTextureLoad, kNoSampler);
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
                         TextureLoad,
                         testing::Values(
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::k1d,
                                 /* texel type */ kF32,
                                 {{"coord", 1, kI32}, {"lod", 1, kI32}},
                                 {"result", 4, kF32},
                                 {
                                     "OpImageFetch %v4float %t %coord Lod %lod",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {{"coords", 2, kI32}, {"lod", 1, kI32}},
                                 {"result", 4, kF32},
                                 {
                                     "OpImageFetch %v4float %t %coords Lod %lod",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {{"coords", 2, kI32}, {"array_idx", 1, kI32}, {"lod", 1, kI32}},
                                 {"result", 4, kF32},
                                 {
                                     "%10 = OpCompositeConstruct %v3int %coords %array_idx",
                                     "OpImageFetch %v4float %t %10 Lod %lod",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::k3d,
                                 /* texel type */ kF32,
                                 {{"coords", 3, kI32}, {"lod", 1, kI32}},
                                 {"result", 4, kF32},
                                 {
                                     "OpImageFetch %v4float %t %coords Lod %lod",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kMultisampledTexture,
                                 core::type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {{"coords", 2, kI32}, {"sample_idx", 1, kI32}},
                                 {"result", 4, kF32},
                                 {
                                     "OpImageFetch %v4float %t %coords Sample %sample_idx",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 core::type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {{"coords", 2, kI32}, {"lod", 1, kI32}},
                                 {"result", 1, kF32},
                                 {
                                     "OpImageFetch %v4float %t %coords Lod %lod",
                                     "%result = OpCompositeExtract %float",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 core::type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {{"coords", 2, kI32}, {"array_idx", 1, kI32}, {"lod", 1, kI32}},
                                 {"result", 1, kF32},
                                 {
                                     "%9 = OpCompositeConstruct %v3int %coords %array_idx",
                                     "OpImageFetch %v4float %t %9 Lod %lod",
                                     "%result = OpCompositeExtract %float",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kDepthMultisampledTexture,
                                 core::type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {{"coords", 2, kI32}, {"sample_idx", 1, kI32}},
                                 {"result", 1, kF32},
                                 {
                                     "OpImageFetch %v4float %t %coords Sample %sample_idx",
                                     "%result = OpCompositeExtract %float",
                                 },
                             },

                             // Test some textures with integer texel types.
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::k2d,
                                 /* texel type */ kI32,
                                 {{"coords", 2, kI32}, {"lod", 1, kI32}},
                                 {"result", 4, kI32},
                                 {
                                     "OpImageFetch %v4int %t %coords Lod %lod",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::k2d,
                                 /* texel type */ kU32,
                                 {{"coords", 2, kI32}, {"lod", 1, kI32}},
                                 {"result", 4, kU32},
                                 {
                                     "OpImageFetch %v4uint %t %coords Lod %lod",
                                 },
                             }),
                         PrintCase);

////////////////////////////////////////////////////////////////
//// textureStore
////////////////////////////////////////////////////////////////
using TextureStore = TextureBuiltinTest;
TEST_P(TextureStore, Emit) {
    Run(core::BuiltinFn::kTextureStore, kNoSampler);
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
                         TextureStore,
                         testing::Values(
                             TextureBuiltinTestCase{
                                 kStorageTexture,
                                 core::type::TextureDimension::k1d,
                                 /* texel type */ kF32,
                                 {{"coord", 1, kI32}, {"texel", 4, kF32}},
                                 {},
                                 {
                                     "OpImageWrite %t %coord %texel None",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kStorageTexture,
                                 core::type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {{"coords", 2, kI32}, {"texel", 4, kF32}},
                                 {},
                                 {
                                     "OpImageWrite %t %coords %texel None",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kStorageTexture,
                                 core::type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {{"coords", 2, kI32}, {"array_idx", 1, kI32}, {"texel", 4, kF32}},
                                 {},
                                 {
                                     "%10 = OpCompositeConstruct %v3int %coords %array_idx",
                                     "OpImageWrite %t %10 %texel None",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kStorageTexture,
                                 core::type::TextureDimension::k3d,
                                 /* texel type */ kF32,
                                 {{"coords", 3, kI32}, {"texel", 4, kF32}},
                                 {},
                                 {
                                     "OpImageWrite %t %coords %texel None",
                                 },
                             },

                             // Test some textures with integer texel types.
                             TextureBuiltinTestCase{
                                 kStorageTexture,
                                 core::type::TextureDimension::k2d,
                                 /* texel type */ kI32,
                                 {{"coords", 2, kI32}, {"texel", 4, kI32}},
                                 {},
                                 {
                                     "OpImageWrite %t %coords %texel None",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kStorageTexture,
                                 core::type::TextureDimension::k2d,
                                 /* texel type */ kU32,
                                 {{"coords", 2, kI32}, {"texel", 4, kU32}},
                                 {},
                                 {
                                     "OpImageWrite %t %coords %texel None",
                                 },
                             }),
                         PrintCase);

////////////////////////////////////////////////////////////////
//// textureDimensions
////////////////////////////////////////////////////////////////
using TextureDimensions = TextureBuiltinTest;
TEST_P(TextureDimensions, Emit) {
    Run(core::BuiltinFn::kTextureDimensions, kNoSampler);
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
                         TextureDimensions,
                         testing::Values(
                             // 1D implicit Lod.
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::k1d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {"%result = OpImageQuerySizeLod %uint %t %uint_0"},
                             },
                             TextureBuiltinTestCase{
                                 kStorageTexture,
                                 core::type::TextureDimension::k1d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {"%result = OpImageQuerySize %uint %t"},
                             },

                             // 1D explicit Lod.
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::k1d,
                                 /* texel type */ kF32,
                                 {{"lod", 1, kU32}},
                                 {"result", 1, kU32},
                                 {"%result = OpImageQuerySizeLod %uint %t %lod"},
                             },

                             // 2D implicit Lod.
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySizeLod %v2uint %t %uint_0"},
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {
                                     "%9 = OpImageQuerySizeLod %v3uint %t %uint_0",
                                     "%result = OpVectorShuffle %v2uint %9 %9 0 1",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::kCube,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySizeLod %v2uint %t %uint_0"},
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::kCubeArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {
                                     "%9 = OpImageQuerySizeLod %v3uint %t %uint_0",
                                     "%result = OpVectorShuffle %v2uint %9 %9 0 1",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kMultisampledTexture,
                                 core::type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySize %v2uint %t"},
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 core::type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySizeLod %v2uint %t %uint_0"},
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 core::type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {
                                     "%9 = OpImageQuerySizeLod %v3uint %t %uint_0",
                                     "%result = OpVectorShuffle %v2uint %9 %9 0 1",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 core::type::TextureDimension::kCube,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySizeLod %v2uint %t %uint_0"},
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 core::type::TextureDimension::kCubeArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {
                                     "%9 = OpImageQuerySizeLod %v3uint %t %uint_0",
                                     "%result = OpVectorShuffle %v2uint %9 %9 0 1",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kDepthMultisampledTexture,
                                 core::type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySize %v2uint %t"},
                             },
                             TextureBuiltinTestCase{
                                 kStorageTexture,
                                 core::type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySize %v2uint %t"},
                             },
                             TextureBuiltinTestCase{
                                 kStorageTexture,
                                 core::type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {
                                     "%9 = OpImageQuerySize %v3uint %t",
                                     "%result = OpVectorShuffle %v2uint %9 %9 0 1",
                                 },
                             },

                             // 2D explicit Lod.
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {{"lod", 1, kU32}},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySizeLod %v2uint %t %lod"},
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {{"lod", 1, kU32}},
                                 {"result", 2, kU32},
                                 {
                                     "%9 = OpImageQuerySizeLod %v3uint %t %lod",
                                     "%result = OpVectorShuffle %v2uint %9 %9 0 1",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::kCube,
                                 /* texel type */ kF32,
                                 {{"lod", 1, kU32}},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySizeLod %v2uint %t %lod"},
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::kCubeArray,
                                 /* texel type */ kF32,
                                 {{"lod", 1, kU32}},
                                 {"result", 2, kU32},
                                 {
                                     "%9 = OpImageQuerySizeLod %v3uint %t %lod",
                                     "%result = OpVectorShuffle %v2uint %9 %9 0 1",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 core::type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {{"lod", 1, kU32}},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySizeLod %v2uint %t %lod"},
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 core::type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {{"lod", 1, kU32}},
                                 {"result", 2, kU32},
                                 {
                                     "%9 = OpImageQuerySizeLod %v3uint %t %lod",
                                     "%result = OpVectorShuffle %v2uint %9 %9 0 1",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 core::type::TextureDimension::kCube,
                                 /* texel type */ kF32,
                                 {{"lod", 1, kU32}},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySizeLod %v2uint %t %lod"},
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 core::type::TextureDimension::kCubeArray,
                                 /* texel type */ kF32,
                                 {{"lod", 1, kU32}},
                                 {"result", 2, kU32},
                                 {
                                     "%9 = OpImageQuerySizeLod %v3uint %t %lod",
                                     "%result = OpVectorShuffle %v2uint %9 %9 0 1",
                                 },
                             },

                             // 3D implicit lod.
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::k3d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 3, kU32},
                                 {"%result = OpImageQuerySizeLod %v3uint %t %uint_0"},
                             },

                             // 3D explicit lod.
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::k3d,
                                 /* texel type */ kF32,
                                 {{"lod", 1, kU32}},
                                 {"result", 3, kU32},
                                 {"%result = OpImageQuerySizeLod %v3uint %t %lod"},
                             }),
                         PrintCase);

////////////////////////////////////////////////////////////////
//// textureNumLayers
////////////////////////////////////////////////////////////////
using TextureNumLayers = TextureBuiltinTest;
TEST_P(TextureNumLayers, Emit) {
    Run(core::BuiltinFn::kTextureNumLayers, kNoSampler);
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
                         TextureNumLayers,
                         testing::Values(
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {
                                     "%8 = OpImageQuerySizeLod %v3uint %t %uint_0",
                                     "%result = OpCompositeExtract %uint %8 2",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::kCubeArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {
                                     "%8 = OpImageQuerySizeLod %v3uint %t %uint_0",
                                     "%result = OpCompositeExtract %uint %8 2",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 core::type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {
                                     "%8 = OpImageQuerySizeLod %v3uint %t %uint_0",
                                     "%result = OpCompositeExtract %uint %8 2",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 core::type::TextureDimension::kCubeArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {
                                     "%8 = OpImageQuerySizeLod %v3uint %t %uint_0",
                                     "%result = OpCompositeExtract %uint %8 2",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kStorageTexture,
                                 core::type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {
                                     "%8 = OpImageQuerySize %v3uint %t",
                                     "%result = OpCompositeExtract %uint %8 2",
                                 },
                             }),
                         PrintCase);

////////////////////////////////////////////////////////////////
//// textureNumLevels
////////////////////////////////////////////////////////////////
using TextureNumLevels = TextureBuiltinTest;
TEST_P(TextureNumLevels, Emit) {
    Run(core::BuiltinFn::kTextureNumLevels, kNoSampler);
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
                         TextureNumLevels,
                         testing::Values(
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::k1d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {"%result = OpImageQueryLevels %uint %t"},
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {"%result = OpImageQueryLevels %uint %t"},
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {"%result = OpImageQueryLevels %uint %t"},
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::k3d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {"%result = OpImageQueryLevels %uint %t"},
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::kCube,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {"%result = OpImageQueryLevels %uint %t"},
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 core::type::TextureDimension::kCubeArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {"%result = OpImageQueryLevels %uint %t"},
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 core::type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {"%result = OpImageQueryLevels %uint %t"},
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 core::type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {"%result = OpImageQueryLevels %uint %t"},
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 core::type::TextureDimension::kCube,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {"%result = OpImageQueryLevels %uint %t"},
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 core::type::TextureDimension::kCubeArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {"%result = OpImageQueryLevels %uint %t"},
                             }),
                         PrintCase);

////////////////////////////////////////////////////////////////
//// textureNumSamples
////////////////////////////////////////////////////////////////
using TextureNumSamples = TextureBuiltinTest;
TEST_P(TextureNumSamples, Emit) {
    Run(core::BuiltinFn::kTextureNumSamples, kNoSampler);
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
                         TextureNumSamples,
                         testing::Values(
                             TextureBuiltinTestCase{
                                 kMultisampledTexture,
                                 core::type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {"%result = OpImageQuerySamples %uint %t"},
                             },
                             TextureBuiltinTestCase{
                                 kDepthMultisampledTexture,
                                 core::type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {"%result = OpImageQuerySamples %uint %t"},
                             }),
                         PrintCase);

////////////////////////////////////////////////////////////////
//// textureSampleBaseClampToEdge
////////////////////////////////////////////////////////////////

TEST_F(SpirvWriterTest, TextureSampleBaseClampToEdge_2d_f32) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());

    Vector<core::ir::FunctionParam*, 4> args;
    args.Push(b.FunctionParam("texture", texture_ty));
    args.Push(b.FunctionParam("sampler", ty.sampler()));
    args.Push(b.FunctionParam("coords", ty.vec2<f32>()));

    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams(args);
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSampleBaseClampToEdge, args);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%18 = OpConstantComposite %v2float %float_0_5 %float_0_5");
    EXPECT_INST("%21 = OpConstantComposite %v2float %float_1 %float_1");
    EXPECT_INST(R"(
         %12 = OpImageQuerySizeLod %v2uint %texture %uint_0
         %16 = OpConvertUToF %v2float %12
         %17 = OpFDiv %v2float %18 %16
         %20 = OpFSub %v2float %21 %17
         %23 = OpExtInst %v2float %24 NClamp %coords %17 %20
         %25 = OpSampledImage %26 %texture %sampler
     %result = OpImageSampleExplicitLod %v4float %25 %23 Lod %float_0
)");
}

////////////////////////////////////////////////////////////////
//// Storage textures with bgra8unorm texel formats
////////////////////////////////////////////////////////////////

TEST_F(SpirvWriterTest, Bgra8Unorm_textureStore) {
    auto format = core::TexelFormat::kBgra8Unorm;
    auto* texture_ty =
        ty.storage_texture(core::type::TextureDimension::k2d, format, core::Access::kWrite);

    auto* texture = b.FunctionParam("texture", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
    auto* value = b.FunctionParam("value", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({texture, coords, value});
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, texture, coords, value);
        b.Return(func);
    });

    Options options;
    options.disable_image_robustness = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(
         %13 = OpVectorShuffle %v4float %value %value 2 1 0 3
               OpImageWrite %texture %coords %13 None
)");
}

////////////////////////////////////////////////////////////////
//// Texture robustness enabled.
////////////////////////////////////////////////////////////////

TEST_F(SpirvWriterTest, TextureDimensions_WithRobustness) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());

    auto* texture = b.FunctionParam("texture", texture_ty);
    auto* level = b.FunctionParam("level", ty.i32());
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({texture, level});
    b.Append(func->Block(), [&] {
        auto* dims = b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, texture, level);
        b.Return(func, dims);
        mod.SetName(dims, "dims");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
         %11 = OpImageQueryLevels %uint %texture
         %12 = OpISub %uint %11 %uint_1
         %14 = OpBitcast %uint %level
         %15 = OpExtInst %uint %16 UMin %14 %12
       %dims = OpImageQuerySizeLod %v2uint %texture %15
)");
}

TEST_F(SpirvWriterTest, TextureLoad_WithRobustness) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());

    auto* texture = b.FunctionParam("texture", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
    auto* level = b.FunctionParam("level", ty.i32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({texture, coords, level});
    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, texture, coords, level);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
         %13 = OpImageQueryLevels %uint %texture
         %14 = OpISub %uint %13 %uint_1
         %16 = OpBitcast %uint %level
         %17 = OpExtInst %uint %18 UMin %16 %14
         %19 = OpImageQuerySizeLod %v2uint %texture %17
         %20 = OpISub %v2uint %19 %21
         %22 = OpExtInst %v2uint %18 UMin %coords %20
     %result = OpImageFetch %v4float %texture %22 Lod %17
)");
}

}  // namespace
}  // namespace tint::spirv::writer
