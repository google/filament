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

#include <d3d11.h>
#include <wrl/client.h>

#include <vector>

#include "dawn/native/D3D11Backend.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class D3D11MultithreadTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (UsesWire()) {
            return {};
        }
        return {wgpu::FeatureName::D3D11MultithreadProtected,
                wgpu::FeatureName::ImplicitDeviceSynchronization};
    }
};

// Test that when D3D11MultithreadProtected is enabled, we can use Dawn and D3D11 context directly
// from multiple threads without crashing.
TEST_P(D3D11MultithreadTests, DawnAndDirectD3D11) {
    Microsoft::WRL::ComPtr<ID3D11Device> dxDevice = native::d3d11::GetD3D11Device(device.Get());

    constexpr int kNumOfThreads = 4;
    utils::RunInParallel(
        kNumOfThreads,
        [&](uint32_t) {
            // Background thread uses Dawn's API.
            for (int i = 0; i < 5; ++i) {
                wgpu::TextureDescriptor desc = {};
                desc.size = {1, 1, 1};
                desc.format = wgpu::TextureFormat::RGBA8Unorm;
                desc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
                wgpu::Texture texture = device.CreateTexture(&desc);

                utils::ComboRenderPassDescriptor renderPass({texture.CreateView()});
                renderPass.cColorAttachments[0].clearValue = {1.0f, 0.0f, 0.0f, 1.0f};
                renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;

                wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
                wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
                pass.End();
                wgpu::CommandBuffer commands = encoder.Finish();
                queue.Submit(1, &commands);

                EXPECT_TEXTURE_EQ(utils::RGBA8::kRed, texture, {0, 0});
            }
        },
        [&]() {
            // Main thread uses D3D11 directly
            Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
            dxDevice->GetImmediateContext(&context);
            for (int i = 0; i < 5; ++i) {
                D3D11_BUFFER_DESC desc = {};
                desc.ByteWidth = 256;
                desc.Usage = D3D11_USAGE_DEFAULT;
                desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
                HRESULT hr = dxDevice->CreateBuffer(&desc, nullptr, &buffer);
                EXPECT_TRUE(SUCCEEDED(hr));
                EXPECT_NE(buffer.Get(), nullptr);

                D3D11_MAPPED_SUBRESOURCE mapped;
                desc.Usage = D3D11_USAGE_STAGING;
                desc.BindFlags = 0;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
                Microsoft::WRL::ComPtr<ID3D11Buffer> staging;
                hr = dxDevice->CreateBuffer(&desc, nullptr, &staging);
                EXPECT_TRUE(SUCCEEDED(hr));

                context->CopyResource(staging.Get(), buffer.Get());
                hr = context->Map(staging.Get(), 0, D3D11_MAP_READ_WRITE, 0, &mapped);
                EXPECT_TRUE(SUCCEEDED(hr));
                memset(mapped.pData, 0xDA, desc.ByteWidth);
                context->Unmap(staging.Get(), 0);

                context->CopyResource(buffer.Get(), staging.Get());

                // Verify copy
                context->CopyResource(staging.Get(), buffer.Get());
                hr = context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped);
                EXPECT_TRUE(SUCCEEDED(hr));
                uint8_t* data = static_cast<uint8_t*>(mapped.pData);
                for (uint32_t j = 0; j < desc.ByteWidth; ++j) {
                    EXPECT_EQ(data[j], 0xDA);
                }
                context->Unmap(staging.Get(), 0);
            }
        });
}

DAWN_INSTANTIATE_TEST(D3D11MultithreadTests,
                      D3D11Backend(),
                      D3D11Backend({"d3d11_use_unmonitored_fence"}),
                      D3D11Backend({"d3d11_delay_flush_to_gpu"}));

}  // anonymous namespace
}  // namespace dawn
