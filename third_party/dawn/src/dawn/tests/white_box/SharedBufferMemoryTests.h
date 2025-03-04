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

#ifndef SRC_DAWN_TESTS_WHITE_BOX_SHAREDBUFFERMEMORYTESTS_H_
#define SRC_DAWN_TESTS_WHITE_BOX_SHAREDBUFFERMEMORYTESTS_H_

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "dawn/tests/DawnTest.h"

namespace dawn {

class SharedBufferMemoryTestBackend {
  public:
    virtual void SetUp() {}
    virtual void TearDown() {}

    // The required features for testing this backend.
    virtual std::vector<wgpu::FeatureName> RequiredFeatures(const wgpu::Adapter& device) const = 0;

    // Create one basic shared buffer memory. It should support most operations.
    virtual wgpu::SharedBufferMemory CreateSharedBufferMemory(const wgpu::Device& device,
                                                              wgpu::BufferUsage usages,
                                                              uint32_t bufferSize,
                                                              uint32_t data = 0) = 0;

    // Creates a SharedFence from a backend-specific fence type.
    wgpu::SharedFence ImportFenceTo(const wgpu::Device& importingDevice,
                                    const wgpu::SharedFence& fence);
};

using Backend = SharedBufferMemoryTestBackend*;
DAWN_TEST_PARAM_STRUCT(SharedBufferMemoryTestParams, Backend);

class SharedBufferMemoryTests : public DawnTestWithParams<SharedBufferMemoryTestParams> {
  public:
    void SetUp() override;
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override;
};
}  // namespace dawn

#endif  // SRC_DAWN_TESTS_WHITE_BOX_SHAREDBUFFERMEMORYTESTS_H_
