// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/writer/common/option_helpers.h"

#include <gtest/gtest.h>

namespace tint::spirv::writer {
namespace {

TEST(SpirvWriterOptionHelpersTest, Empty) {
    Options options;
    auto res = ValidateBindingOptions(options);
    EXPECT_EQ(res, Success);
}

TEST(SpirvWriterOptionHelpersTest, ValidDisjointMappings) {
    Options options;
    options.bindings.uniform.emplace(BindingPoint{0, 0}, BindingPoint{0, 1});
    options.bindings.storage.emplace(BindingPoint{0, 2}, BindingPoint{0, 3});
    options.bindings.texture.emplace(BindingPoint{0, 4}, BindingPoint{0, 5});
    options.bindings.storage_texture.emplace(BindingPoint{0, 6}, BindingPoint{0, 7});
    options.bindings.texel_buffer.emplace(BindingPoint{1, 0}, BindingPoint{1, 1});
    options.bindings.sampler.emplace(BindingPoint{1, 2}, BindingPoint{1, 3});

    auto res = ValidateBindingOptions(options);
    EXPECT_EQ(res, Success);
}

TEST(SpirvWriterOptionHelpersTest, SameSourceDifferentTarget) {
    Options options;
    options.bindings.uniform.emplace(BindingPoint{0, 0}, BindingPoint{0, 1});
    options.bindings.storage.emplace(BindingPoint{0, 0}, BindingPoint{0, 2});

    auto res = ValidateBindingOptions(options);
    EXPECT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason,
              R"(error: found duplicate WGSL binding point: [group: 0, binding: 0]
note: when processing storage)");
}

TEST(SpirvWriterOptionHelpersTest, DuplicateSpirvBindingPoint) {
    Options options;
    options.bindings.uniform.emplace(BindingPoint{0, 0}, BindingPoint{1, 1});
    options.bindings.storage.emplace(BindingPoint{0, 1}, BindingPoint{1, 1});

    auto res = ValidateBindingOptions(options);
    EXPECT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason,
              R"(error: found duplicate SPIR-V binding point: [group: 1, binding: 1]
note: when processing storage)");
}

TEST(SpirvWriterOptionHelpersTest, TexelBufferOverlap) {
    Options options;
    options.bindings.texel_buffer.emplace(BindingPoint{0, 0}, BindingPoint{1, 1});
    options.bindings.texture.emplace(BindingPoint{0, 1}, BindingPoint{1, 1});

    auto res = ValidateBindingOptions(options);
    EXPECT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason,
              R"(error: found duplicate SPIR-V binding point: [group: 1, binding: 1]
note: when processing texel_buffer)");
}

}  // namespace
}  // namespace tint::spirv::writer
