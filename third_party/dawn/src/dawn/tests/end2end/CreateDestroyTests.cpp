// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/tests/DawnTest.h"

namespace dawn {
namespace {

class CreateDestroyTests : public DawnTest {};

TEST_P(CreateDestroyTests, Buffer) {
    wgpu::BufferDescriptor desc;
    desc.size = 42;
    desc.usage = wgpu::BufferUsage::CopyDst;

    auto buffer = device.CreateBuffer(&desc);
    buffer.Destroy();
}

TEST_P(CreateDestroyTests, Texture) {
    wgpu::TextureDescriptor desc;
    desc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
    desc.size = {2, 2, 1};
    desc.format = wgpu::TextureFormat::RGBA8Unorm;

    auto texture = device.CreateTexture(&desc);
    texture.Destroy();
}

TEST_P(CreateDestroyTests, QuerySet) {
    wgpu::QuerySetDescriptor desc;
    desc.type = wgpu::QueryType::Occlusion;
    desc.count = 42;

    auto querySet = device.CreateQuerySet(&desc);
    querySet.Destroy();
}

TEST_P(CreateDestroyTests, ZeroSizedQuerySet) {
    wgpu::QuerySetDescriptor desc;
    desc.type = wgpu::QueryType::Occlusion;
    desc.count = 0;

    auto querySet = device.CreateQuerySet(&desc);
    querySet.Destroy();
}

DAWN_INSTANTIATE_TEST(CreateDestroyTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      WebGPUBackend());

}  // anonymous namespace
}  // namespace dawn
