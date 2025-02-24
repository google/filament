// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/native/PerStage.h"

namespace dawn::native {

// Tests for StageBit
TEST(PerStage, StageBit) {
    ASSERT_EQ(StageBit(SingleShaderStage::Vertex), wgpu::ShaderStage::Vertex);
    ASSERT_EQ(StageBit(SingleShaderStage::Fragment), wgpu::ShaderStage::Fragment);
    ASSERT_EQ(StageBit(SingleShaderStage::Compute), wgpu::ShaderStage::Compute);
}

// Basic test for the PerStage container
TEST(PerStage, PerStage) {
    PerStage<int> data;

    // Store data using wgpu::ShaderStage
    data[SingleShaderStage::Vertex] = 42;
    data[SingleShaderStage::Fragment] = 3;
    data[SingleShaderStage::Compute] = -1;

    // Load it using wgpu::ShaderStage
    ASSERT_EQ(data[wgpu::ShaderStage::Vertex], 42);
    ASSERT_EQ(data[wgpu::ShaderStage::Fragment], 3);
    ASSERT_EQ(data[wgpu::ShaderStage::Compute], -1);
}

// Test IterateStages with kAllStages
TEST(PerStage, IterateAllStages) {
    PerStage<int> counts;
    counts[SingleShaderStage::Vertex] = 0;
    counts[SingleShaderStage::Fragment] = 0;
    counts[SingleShaderStage::Compute] = 0;

    for (auto stage : IterateStages(kAllStages)) {
        counts[stage]++;
    }

    ASSERT_EQ(counts[wgpu::ShaderStage::Vertex], 1);
    ASSERT_EQ(counts[wgpu::ShaderStage::Fragment], 1);
    ASSERT_EQ(counts[wgpu::ShaderStage::Compute], 1);
}

// Test IterateStages with one stage
TEST(PerStage, IterateOneStage) {
    PerStage<int> counts;
    counts[SingleShaderStage::Vertex] = 0;
    counts[SingleShaderStage::Fragment] = 0;
    counts[SingleShaderStage::Compute] = 0;

    for (auto stage : IterateStages(wgpu::ShaderStage::Fragment)) {
        counts[stage]++;
    }

    ASSERT_EQ(counts[wgpu::ShaderStage::Vertex], 0);
    ASSERT_EQ(counts[wgpu::ShaderStage::Fragment], 1);
    ASSERT_EQ(counts[wgpu::ShaderStage::Compute], 0);
}

// Test IterateStages with no stage
TEST(PerStage, IterateNoStages) {
    PerStage<int> counts;
    counts[SingleShaderStage::Vertex] = 0;
    counts[SingleShaderStage::Fragment] = 0;
    counts[SingleShaderStage::Compute] = 0;

    for (auto stage : IterateStages(wgpu::ShaderStage::Fragment & wgpu::ShaderStage::Vertex)) {
        counts[stage]++;
    }

    ASSERT_EQ(counts[wgpu::ShaderStage::Vertex], 0);
    ASSERT_EQ(counts[wgpu::ShaderStage::Fragment], 0);
    ASSERT_EQ(counts[wgpu::ShaderStage::Compute], 0);
}

}  // namespace dawn::native
