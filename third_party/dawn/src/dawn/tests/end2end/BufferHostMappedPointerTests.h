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

#ifndef SRC_DAWN_TESTS_END2END_BUFFERHOSTMAPPEDPOINTERTESTS_H_
#define SRC_DAWN_TESTS_END2END_BUFFERHOSTMAPPEDPOINTERTESTS_H_

#include <utility>
#include <vector>

#include "dawn/tests/DawnTest.h"

namespace dawn {

class BufferHostMappedPointerTestBackend {
  public:
    // The name used in gtest parameterization.
    virtual const char* Name() const = 0;

    std::pair<wgpu::Buffer, void*> CreateHostMappedBuffer(wgpu::Device device,
                                                          wgpu::BufferUsage usage,
                                                          size_t size);

    virtual std::pair<wgpu::Buffer, void*> CreateHostMappedBuffer(
        wgpu::Device device,
        wgpu::BufferUsage usage,
        size_t size,
        std::function<void(void*)> Populate) = 0;
};

inline std::ostream& operator<<(std::ostream& o, BufferHostMappedPointerTestBackend* backend) {
    o << backend->Name();
    return o;
}

using Backend = BufferHostMappedPointerTestBackend*;
DAWN_TEST_PARAM_STRUCT(BufferHostMappedPointerTestParams, Backend);

class BufferHostMappedPointerTests : public DawnTestWithParams<BufferHostMappedPointerTestParams> {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override;
    void SetUp() override;

    uint32_t mRequiredAlignment;
};

}  // namespace dawn

#endif  // SRC_DAWN_TESTS_END2END_BUFFERHOSTMAPPEDPOINTERTESTS_H_
