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

#include "src/tint/lang/wgsl/sem/builtin_fn.h"

#include "gtest/gtest.h"

namespace tint::sem {
namespace {

struct BuiltinData {
    const char* name;
    wgsl::BuiltinFn builtin;
};

inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
    out << data.name;
    return out;
}

using BuiltinFunctionTest = testing::TestWithParam<BuiltinData>;

TEST_P(BuiltinFunctionTest, Parse) {
    auto param = GetParam();
    EXPECT_EQ(wgsl::ParseBuiltinFn(param.name), param.builtin);
}

INSTANTIATE_TEST_SUITE_P(
    BuiltinFunctionTest,
    BuiltinFunctionTest,
    testing::Values(BuiltinData{"abs", wgsl::BuiltinFn::kAbs},
                    BuiltinData{"acos", wgsl::BuiltinFn::kAcos},
                    BuiltinData{"all", wgsl::BuiltinFn::kAll},
                    BuiltinData{"any", wgsl::BuiltinFn::kAny},
                    BuiltinData{"arrayLength", wgsl::BuiltinFn::kArrayLength},
                    BuiltinData{"asin", wgsl::BuiltinFn::kAsin},
                    BuiltinData{"atan", wgsl::BuiltinFn::kAtan},
                    BuiltinData{"atan2", wgsl::BuiltinFn::kAtan2},
                    BuiltinData{"ceil", wgsl::BuiltinFn::kCeil},
                    BuiltinData{"clamp", wgsl::BuiltinFn::kClamp},
                    BuiltinData{"cos", wgsl::BuiltinFn::kCos},
                    BuiltinData{"cosh", wgsl::BuiltinFn::kCosh},
                    BuiltinData{"countOneBits", wgsl::BuiltinFn::kCountOneBits},
                    BuiltinData{"cross", wgsl::BuiltinFn::kCross},
                    BuiltinData{"determinant", wgsl::BuiltinFn::kDeterminant},
                    BuiltinData{"distance", wgsl::BuiltinFn::kDistance},
                    BuiltinData{"dot", wgsl::BuiltinFn::kDot},
                    BuiltinData{"dot4I8Packed", wgsl::BuiltinFn::kDot4I8Packed},
                    BuiltinData{"dot4U8Packed", wgsl::BuiltinFn::kDot4U8Packed},
                    BuiltinData{"dpdx", wgsl::BuiltinFn::kDpdx},
                    BuiltinData{"dpdxCoarse", wgsl::BuiltinFn::kDpdxCoarse},
                    BuiltinData{"dpdxFine", wgsl::BuiltinFn::kDpdxFine},
                    BuiltinData{"dpdy", wgsl::BuiltinFn::kDpdy},
                    BuiltinData{"dpdyCoarse", wgsl::BuiltinFn::kDpdyCoarse},
                    BuiltinData{"dpdyFine", wgsl::BuiltinFn::kDpdyFine},
                    BuiltinData{"exp", wgsl::BuiltinFn::kExp},
                    BuiltinData{"exp2", wgsl::BuiltinFn::kExp2},
                    BuiltinData{"faceForward", wgsl::BuiltinFn::kFaceForward},
                    BuiltinData{"floor", wgsl::BuiltinFn::kFloor},
                    BuiltinData{"fma", wgsl::BuiltinFn::kFma},
                    BuiltinData{"fract", wgsl::BuiltinFn::kFract},
                    BuiltinData{"frexp", wgsl::BuiltinFn::kFrexp},
                    BuiltinData{"fwidth", wgsl::BuiltinFn::kFwidth},
                    BuiltinData{"fwidthCoarse", wgsl::BuiltinFn::kFwidthCoarse},
                    BuiltinData{"fwidthFine", wgsl::BuiltinFn::kFwidthFine},
                    BuiltinData{"inverseSqrt", wgsl::BuiltinFn::kInverseSqrt},
                    BuiltinData{"ldexp", wgsl::BuiltinFn::kLdexp},
                    BuiltinData{"length", wgsl::BuiltinFn::kLength},
                    BuiltinData{"log", wgsl::BuiltinFn::kLog},
                    BuiltinData{"log2", wgsl::BuiltinFn::kLog2},
                    BuiltinData{"max", wgsl::BuiltinFn::kMax},
                    BuiltinData{"min", wgsl::BuiltinFn::kMin},
                    BuiltinData{"mix", wgsl::BuiltinFn::kMix},
                    BuiltinData{"modf", wgsl::BuiltinFn::kModf},
                    BuiltinData{"normalize", wgsl::BuiltinFn::kNormalize},
                    BuiltinData{"pow", wgsl::BuiltinFn::kPow},
                    BuiltinData{"reflect", wgsl::BuiltinFn::kReflect},
                    BuiltinData{"reverseBits", wgsl::BuiltinFn::kReverseBits},
                    BuiltinData{"round", wgsl::BuiltinFn::kRound},
                    BuiltinData{"select", wgsl::BuiltinFn::kSelect},
                    BuiltinData{"sign", wgsl::BuiltinFn::kSign},
                    BuiltinData{"sin", wgsl::BuiltinFn::kSin},
                    BuiltinData{"sinh", wgsl::BuiltinFn::kSinh},
                    BuiltinData{"smoothstep", wgsl::BuiltinFn::kSmoothstep},
                    BuiltinData{"sqrt", wgsl::BuiltinFn::kSqrt},
                    BuiltinData{"step", wgsl::BuiltinFn::kStep},
                    BuiltinData{"storageBarrier", wgsl::BuiltinFn::kStorageBarrier},
                    BuiltinData{"tan", wgsl::BuiltinFn::kTan},
                    BuiltinData{"tanh", wgsl::BuiltinFn::kTanh},
                    BuiltinData{"textureDimensions", wgsl::BuiltinFn::kTextureDimensions},
                    BuiltinData{"textureLoad", wgsl::BuiltinFn::kTextureLoad},
                    BuiltinData{"textureNumLayers", wgsl::BuiltinFn::kTextureNumLayers},
                    BuiltinData{"textureNumLevels", wgsl::BuiltinFn::kTextureNumLevels},
                    BuiltinData{"textureNumSamples", wgsl::BuiltinFn::kTextureNumSamples},
                    BuiltinData{"textureSample", wgsl::BuiltinFn::kTextureSample},
                    BuiltinData{"textureSampleBias", wgsl::BuiltinFn::kTextureSampleBias},
                    BuiltinData{"textureSampleCompare", wgsl::BuiltinFn::kTextureSampleCompare},
                    BuiltinData{"textureSampleCompareLevel",
                                wgsl::BuiltinFn::kTextureSampleCompareLevel},
                    BuiltinData{"textureSampleGrad", wgsl::BuiltinFn::kTextureSampleGrad},
                    BuiltinData{"textureSampleLevel", wgsl::BuiltinFn::kTextureSampleLevel},
                    BuiltinData{"trunc", wgsl::BuiltinFn::kTrunc},
                    BuiltinData{"unpack2x16float", wgsl::BuiltinFn::kUnpack2X16Float},
                    BuiltinData{"unpack2x16snorm", wgsl::BuiltinFn::kUnpack2X16Snorm},
                    BuiltinData{"unpack2x16unorm", wgsl::BuiltinFn::kUnpack2X16Unorm},
                    BuiltinData{"unpack4x8snorm", wgsl::BuiltinFn::kUnpack4X8Snorm},
                    BuiltinData{"unpack4x8unorm", wgsl::BuiltinFn::kUnpack4X8Unorm},
                    BuiltinData{"workgroupBarrier", wgsl::BuiltinFn::kWorkgroupBarrier},
                    BuiltinData{"workgroupUniformLoad", wgsl::BuiltinFn::kWorkgroupUniformLoad}));

TEST_F(BuiltinFunctionTest, ParseNoMatch) {
    EXPECT_EQ(wgsl::ParseBuiltinFn("not_builtin"), wgsl::BuiltinFn::kNone);
}

}  // namespace
}  // namespace tint::sem
