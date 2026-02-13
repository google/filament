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

#include "src/tint/lang/msl/writer/common/printer_support.h"
#include "gtest/gtest.h"

namespace tint::msl::writer {
namespace {

struct MslBuiltinData {
    core::BuiltinValue builtin;
    const char* attribute_name;
};
inline std::ostream& operator<<(std::ostream& out, MslBuiltinData data) {
    StringStream str;
    str << data.builtin;
    out << str.str();
    return out;
}

using MslBuiltinConversionTest = testing::TestWithParam<MslBuiltinData>;
TEST_P(MslBuiltinConversionTest, Emit) {
    auto params = GetParam();
    EXPECT_EQ(BuiltinToAttribute(params.builtin), std::string(params.attribute_name));
}

INSTANTIATE_TEST_SUITE_P(
    MslWriterTest,
    MslBuiltinConversionTest,
    testing::Values(
        MslBuiltinData{core::BuiltinValue::kPosition, "position"},
        MslBuiltinData{core::BuiltinValue::kVertexIndex, "vertex_id"},
        MslBuiltinData{core::BuiltinValue::kInstanceIndex, "instance_id"},
        MslBuiltinData{core::BuiltinValue::kFrontFacing, "front_facing"},
        MslBuiltinData{core::BuiltinValue::kFragDepth, "depth(any)"},
        MslBuiltinData{core::BuiltinValue::kLocalInvocationId, "thread_position_in_threadgroup"},
        MslBuiltinData{core::BuiltinValue::kLocalInvocationIndex, "thread_index_in_threadgroup"},
        MslBuiltinData{core::BuiltinValue::kGlobalInvocationId, "thread_position_in_grid"},
        MslBuiltinData{core::BuiltinValue::kWorkgroupId, "threadgroup_position_in_grid"},
        MslBuiltinData{core::BuiltinValue::kNumWorkgroups, "threadgroups_per_grid"},
        MslBuiltinData{core::BuiltinValue::kSampleIndex, "sample_id"},
        MslBuiltinData{core::BuiltinValue::kSampleMask, "sample_mask"},
        MslBuiltinData{core::BuiltinValue::kPointSize, "point_size"},
        MslBuiltinData{core::BuiltinValue::kPrimitiveId, "primitive_id"},
        MslBuiltinData{core::BuiltinValue::kBarycentricCoord, "barycentric_coord"}));

}  // namespace
}  // namespace tint::msl::writer
