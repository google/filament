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

#include "gtest/gtest.h"
#include "spirv/unified1/spirv.h"
#include "src/tint/lang/spirv/writer/common/spv_dump_test.h"

namespace tint::spirv::writer {
namespace {

using SpirvWriterModuleTest = testing::Test;

TEST_F(SpirvWriterModuleTest, TracksIdBounds) {
    Module m;

    for (size_t i = 0; i < 5; i++) {
        EXPECT_EQ(m.NextId(), i + 1);
    }

    EXPECT_EQ(6u, m.IdBound());
}

TEST_F(SpirvWriterModuleTest, Capabilities_Dedup) {
    Module m;

    m.PushCapability(SpvCapabilityShader);
    m.PushCapability(SpvCapabilityShader);
    m.PushCapability(SpvCapabilityShader);

    EXPECT_EQ(DumpInstructions(m.Capabilities()), "OpCapability Shader\n");
}

TEST_F(SpirvWriterModuleTest, DeclareExtension) {
    Module m;

    m.PushExtension("SPV_KHR_integer_dot_product");

    EXPECT_EQ(DumpInstructions(m.Extensions()), "OpExtension \"SPV_KHR_integer_dot_product\"\n");
}

}  // namespace
}  // namespace tint::spirv::writer
