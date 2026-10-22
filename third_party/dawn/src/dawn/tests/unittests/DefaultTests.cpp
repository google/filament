// Copyright 2024 The Dawn & Tint Authors
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

#include <gtest/gtest.h>
#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>

// Test that the C init structs match the C++ default values
TEST(DefaultTests, CMatchesCpp) {
    // Test render pipeline descriptor.
    // It's non-trivial and has nested structs too.
    wgpu::RenderPipelineDescriptor cppDesc = {};
    WGPURenderPipelineDescriptor cDesc = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;

    EXPECT_EQ(cppDesc.primitive.topology,
              static_cast<wgpu::PrimitiveTopology>(cDesc.primitive.topology));
    EXPECT_EQ(cppDesc.primitive.stripIndexFormat,
              static_cast<wgpu::IndexFormat>(cDesc.primitive.stripIndexFormat));
    EXPECT_EQ(cppDesc.primitive.frontFace, static_cast<wgpu::FrontFace>(cDesc.primitive.frontFace));
    EXPECT_EQ(cppDesc.primitive.cullMode, static_cast<wgpu::CullMode>(cDesc.primitive.cullMode));
    EXPECT_EQ(cppDesc.multisample.count, cDesc.multisample.count);
    EXPECT_EQ(cppDesc.multisample.mask, cDesc.multisample.mask);
    EXPECT_EQ(cppDesc.multisample.alphaToCoverageEnabled, cDesc.multisample.alphaToCoverageEnabled);
}
