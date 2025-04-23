// Copyright 2018 The Dawn & Tint Authors
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

#include <cmath>
#include <string>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class RenderPassDescriptorValidationTest : public ValidationTest {
  public:
    void AssertBeginRenderPassSuccess(const wgpu::RenderPassDescriptor* descriptor) {
        wgpu::CommandEncoder commandEncoder = TestBeginRenderPass(descriptor);
        commandEncoder.Finish();
    }
    void AssertBeginRenderPassError(const wgpu::RenderPassDescriptor* descriptor) {
        wgpu::CommandEncoder commandEncoder = TestBeginRenderPass(descriptor);
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    void AssertBeginRenderPassError(const wgpu::RenderPassDescriptor* descriptor,
                                    testing::Matcher<std::string> errorMatcher) {
        wgpu::CommandEncoder commandEncoder = TestBeginRenderPass(descriptor);
        ASSERT_DEVICE_ERROR(commandEncoder.Finish(), errorMatcher);
    }

  private:
    wgpu::CommandEncoder TestBeginRenderPass(const wgpu::RenderPassDescriptor* descriptor) {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(descriptor);
        renderPassEncoder.End();
        return commandEncoder;
    }
};

wgpu::Texture CreateTexture(wgpu::Device& device,
                            wgpu::TextureDimension dimension,
                            wgpu::TextureFormat format,
                            uint32_t width,
                            uint32_t height,
                            uint32_t arrayLayerCount,
                            uint32_t mipLevelCount,
                            uint32_t sampleCount = 1,
                            wgpu::TextureUsage usage = wgpu::TextureUsage::RenderAttachment) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = dimension;
    descriptor.size.width = width;
    descriptor.size.height = height;
    descriptor.size.depthOrArrayLayers = arrayLayerCount;
    descriptor.sampleCount = sampleCount;
    descriptor.format = format;
    descriptor.mipLevelCount = mipLevelCount;
    descriptor.usage = usage;

    return device.CreateTexture(&descriptor);
}

wgpu::TextureView Create2DAttachment(wgpu::Device& device,
                                     uint32_t width,
                                     uint32_t height,
                                     wgpu::TextureFormat format) {
    wgpu::Texture texture =
        CreateTexture(device, wgpu::TextureDimension::e2D, format, width, height, 1, 1);
    return texture.CreateView();
}

// Using BeginRenderPass with no attachments isn't valid
TEST_F(RenderPassDescriptorValidationTest, Empty) {
    utils::ComboRenderPassDescriptor renderPass({}, nullptr);
    AssertBeginRenderPassError(&renderPass);
}

// A render pass with only one color or one depth attachment is ok
TEST_F(RenderPassDescriptorValidationTest, OneAttachment) {
    // One color attachment
    {
        wgpu::TextureView color = Create2DAttachment(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
        utils::ComboRenderPassDescriptor renderPass({color});

        AssertBeginRenderPassSuccess(&renderPass);
    }
    // One depth-stencil attachment
    {
        wgpu::TextureView depthStencil =
            Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth24PlusStencil8);
        utils::ComboRenderPassDescriptor renderPass({}, depthStencil);

        AssertBeginRenderPassSuccess(&renderPass);
    }
}

// Regression test for chromium:1487788 were cached attachment states used in a pass encoder created
// from an error command encoder are not cleaned up if the device's last reference is dropped before
// the pass.
TEST_F(RenderPassDescriptorValidationTest, ErrorEncoderLingeringAttachmentState) {
    utils::ComboRenderPassDescriptor descriptor(
        {Create2DAttachment(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm)});

    // Purposely add a bad chain to the command encoder to force an error command encoder.
    wgpu::CommandEncoderDescriptor commandEncoderDesc = {};
    wgpu::ChainedStruct chain = {};
    commandEncoderDesc.nextInChain = &chain;

    wgpu::CommandEncoder commandEncoder;
    ASSERT_DEVICE_ERROR(commandEncoder = device.CreateCommandEncoder(&commandEncoderDesc));
    commandEncoder.BeginRenderPass(&descriptor);

    ExpectDeviceDestruction();
    device = nullptr;
}

// Test OOB color attachment indices are handled
TEST_F(RenderPassDescriptorValidationTest, ColorAttachmentOutOfBounds) {
    std::array<wgpu::RenderPassColorAttachment, kMaxColorAttachments + 1> colorAttachments;
    for (uint32_t i = 0; i < colorAttachments.size(); i++) {
        colorAttachments[i].view = Create2DAttachment(device, 1, 1, wgpu::TextureFormat::R8Unorm);
        colorAttachments[i].resolveTarget = nullptr;
        colorAttachments[i].clearValue = {0.0f, 0.0f, 0.0f, 0.0f};
        colorAttachments[i].loadOp = wgpu::LoadOp::Clear;
        colorAttachments[i].storeOp = wgpu::StoreOp::Store;
    }

    // Control case: kMaxColorAttachments is valid.
    {
        wgpu::RenderPassDescriptor renderPass;
        renderPass.colorAttachmentCount = kMaxColorAttachments;
        renderPass.colorAttachments = colorAttachments.data();
        renderPass.depthStencilAttachment = nullptr;
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Error case: kMaxColorAttachments + 1 is an error.
    {
        wgpu::RenderPassDescriptor renderPass;
        renderPass.colorAttachmentCount = kMaxColorAttachments + 1;
        renderPass.colorAttachments = colorAttachments.data();
        renderPass.depthStencilAttachment = nullptr;
        AssertBeginRenderPassError(&renderPass);
    }
}

// Test sparse color attachment validations
TEST_F(RenderPassDescriptorValidationTest, SparseColorAttachment) {
    // Having sparse color attachment is valid.
    {
        std::array<wgpu::RenderPassColorAttachment, 2> colorAttachments;
        colorAttachments[0].view = nullptr;

        colorAttachments[1].view =
            Create2DAttachment(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
        colorAttachments[1].loadOp = wgpu::LoadOp::Load;
        colorAttachments[1].storeOp = wgpu::StoreOp::Store;

        wgpu::RenderPassDescriptor renderPass;
        renderPass.colorAttachmentCount = colorAttachments.size();
        renderPass.colorAttachments = colorAttachments.data();
        renderPass.depthStencilAttachment = nullptr;
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // When all color attachments are null
    {
        std::array<wgpu::RenderPassColorAttachment, 2> colorAttachments;
        colorAttachments[0].view = nullptr;
        colorAttachments[1].view = nullptr;

        // Control case: depth stencil attachment is not null is valid.
        {
            wgpu::TextureView depthStencilView =
                Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth24PlusStencil8);
            wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;
            depthStencilAttachment.view = depthStencilView;
            depthStencilAttachment.depthClearValue = 1.0f;
            depthStencilAttachment.stencilClearValue = 0;
            depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
            depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
            depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Clear;
            depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;

            wgpu::RenderPassDescriptor renderPass;
            renderPass.colorAttachmentCount = colorAttachments.size();
            renderPass.colorAttachments = colorAttachments.data();
            renderPass.depthStencilAttachment = &depthStencilAttachment;
            AssertBeginRenderPassSuccess(&renderPass);
        }

        // Error case: depth stencil attachment being null is invalid.
        {
            wgpu::RenderPassDescriptor renderPass;
            renderPass.colorAttachmentCount = colorAttachments.size();
            renderPass.colorAttachments = colorAttachments.data();
            renderPass.depthStencilAttachment = nullptr;
            AssertBeginRenderPassError(&renderPass);
        }
    }
}

// Check that the render pass color attachment must have the RenderAttachment usage.
TEST_F(RenderPassDescriptorValidationTest, ColorAttachmentInvalidUsage) {
    // Control case: using a texture with RenderAttachment is valid.
    {
        wgpu::TextureView renderView =
            Create2DAttachment(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
        utils::ComboRenderPassDescriptor renderPass({renderView});
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Error case: using a texture with Sampled is invalid.
    {
        wgpu::TextureDescriptor texDesc;
        texDesc.usage = wgpu::TextureUsage::TextureBinding;
        texDesc.size = {1, 1, 1};
        texDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        wgpu::Texture sampledTex = device.CreateTexture(&texDesc);

        utils::ComboRenderPassDescriptor renderPass({sampledTex.CreateView()});
        AssertBeginRenderPassError(&renderPass);
    }
}

// Attachments must have the same size
TEST_F(RenderPassDescriptorValidationTest, SizeMustMatch) {
    wgpu::TextureView color1x1A = Create2DAttachment(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
    wgpu::TextureView color1x1B = Create2DAttachment(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
    wgpu::TextureView color2x2 = Create2DAttachment(device, 2, 2, wgpu::TextureFormat::RGBA8Unorm);

    wgpu::TextureView depthStencil1x1 =
        Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth24PlusStencil8);
    wgpu::TextureView depthStencil2x2 =
        Create2DAttachment(device, 2, 2, wgpu::TextureFormat::Depth24PlusStencil8);

    // Control case: all the same size (1x1)
    {
        utils::ComboRenderPassDescriptor renderPass({color1x1A, color1x1B}, depthStencil1x1);
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // One of the color attachments has a different size
    {
        utils::ComboRenderPassDescriptor renderPass({color1x1A, color2x2});
        AssertBeginRenderPassError(&renderPass);
    }

    // The depth stencil attachment has a different size
    {
        utils::ComboRenderPassDescriptor renderPass({color1x1A, color1x1B}, depthStencil2x2);
        AssertBeginRenderPassError(&renderPass);
    }
}

// Attachments formats must match whether they are used for color or depth-stencil
TEST_F(RenderPassDescriptorValidationTest, FormatMismatch) {
    wgpu::TextureView color = Create2DAttachment(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
    wgpu::TextureView depthStencil =
        Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth24PlusStencil8);

    // Using depth-stencil for color
    {
        utils::ComboRenderPassDescriptor renderPass({depthStencil});
        AssertBeginRenderPassError(&renderPass);
    }

    // Using color for depth-stencil
    {
        utils::ComboRenderPassDescriptor renderPass({}, color);
        AssertBeginRenderPassError(&renderPass);
    }
}

// Depth and stencil storeOps can be different
TEST_F(RenderPassDescriptorValidationTest, DepthStencilStoreOpMismatch) {
    constexpr uint32_t kArrayLayers = 1;
    constexpr uint32_t kLevelCount = 1;
    constexpr uint32_t kSize = 32;
    constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;
    constexpr wgpu::TextureFormat kDepthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;

    wgpu::Texture colorTexture = CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat,
                                               kSize, kSize, kArrayLayers, kLevelCount);
    wgpu::Texture depthStencilTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, kDepthStencilFormat, kSize, kSize,
                      kArrayLayers, kLevelCount);

    wgpu::TextureViewDescriptor descriptor;
    descriptor.dimension = wgpu::TextureViewDimension::e2D;
    descriptor.baseArrayLayer = 0;
    descriptor.arrayLayerCount = kArrayLayers;
    descriptor.baseMipLevel = 0;
    descriptor.mipLevelCount = kLevelCount;
    wgpu::TextureView colorTextureView = colorTexture.CreateView(&descriptor);
    wgpu::TextureView depthStencilView = depthStencilTexture.CreateView(&descriptor);

    // Base case: StoreOps match so render pass is a success
    {
        utils::ComboRenderPassDescriptor renderPass({}, depthStencilView);
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Store;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Store;
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Base case: StoreOps match so render pass is a success
    {
        utils::ComboRenderPassDescriptor renderPass({}, depthStencilView);
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Discard;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Discard;
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // StoreOps mismatch still is a success
    {
        utils::ComboRenderPassDescriptor renderPass({}, depthStencilView);
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Store;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Discard;
        AssertBeginRenderPassSuccess(&renderPass);
    }
}

// Currently only texture views with arrayLayerCount == 1 are allowed to be color and depth
// stencil attachments
TEST_F(RenderPassDescriptorValidationTest, TextureViewLayerCountForColorAndDepthStencil) {
    constexpr uint32_t kLevelCount = 1;
    constexpr uint32_t kSize = 32;
    constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;
    constexpr wgpu::TextureFormat kDepthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;

    constexpr uint32_t kArrayLayers = 10;

    wgpu::Texture colorTexture = CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat,
                                               kSize, kSize, kArrayLayers, kLevelCount);
    wgpu::Texture depthStencilTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, kDepthStencilFormat, kSize, kSize,
                      kArrayLayers, kLevelCount);

    wgpu::TextureViewDescriptor baseDescriptor;
    baseDescriptor.dimension = wgpu::TextureViewDimension::e2DArray;
    baseDescriptor.baseArrayLayer = 0;
    baseDescriptor.arrayLayerCount = kArrayLayers;
    baseDescriptor.baseMipLevel = 0;
    baseDescriptor.mipLevelCount = kLevelCount;

    // Using 2D array texture view with arrayLayerCount > 1 is not allowed for color
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kColorFormat;
        descriptor.arrayLayerCount = 5;

        wgpu::TextureView colorTextureView = colorTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({colorTextureView});
        AssertBeginRenderPassError(&renderPass);
    }

    // Using 2D array texture view with arrayLayerCount > 1 is not allowed for depth stencil
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kDepthStencilFormat;
        descriptor.arrayLayerCount = 5;

        wgpu::TextureView depthStencilView = depthStencilTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({}, depthStencilView);
        AssertBeginRenderPassError(&renderPass);
    }

    // Using 2D array texture view that covers the first layer of the texture is OK for color
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kColorFormat;
        descriptor.baseArrayLayer = 0;
        descriptor.arrayLayerCount = 1;

        wgpu::TextureView colorTextureView = colorTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({colorTextureView});
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Using 2D array texture view that covers the first layer is OK for depth stencil
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kDepthStencilFormat;
        descriptor.baseArrayLayer = 0;
        descriptor.arrayLayerCount = 1;

        wgpu::TextureView depthStencilView = depthStencilTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({}, depthStencilView);
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Using 2D array texture view that covers the last layer is OK for color
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kColorFormat;
        descriptor.baseArrayLayer = kArrayLayers - 1;
        descriptor.arrayLayerCount = 1;

        wgpu::TextureView colorTextureView = colorTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({colorTextureView});
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Using 2D array texture view that covers the last layer is OK for depth stencil
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kDepthStencilFormat;
        descriptor.baseArrayLayer = kArrayLayers - 1;
        descriptor.arrayLayerCount = 1;

        wgpu::TextureView depthStencilView = depthStencilTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({}, depthStencilView);
        AssertBeginRenderPassSuccess(&renderPass);
    }
}

// Check that depthSlice must be set correctly for 3D color attachments and must not be set for
// non-3D color attachments.
TEST_F(RenderPassDescriptorValidationTest, TextureViewDepthSliceForColor) {
    constexpr uint32_t kSize = 8;
    constexpr uint32_t kDepthOrArrayLayers = 4;
    constexpr uint32_t kMipLevelCounts = 4;
    constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::Texture colorTexture3D =
        CreateTexture(device, wgpu::TextureDimension::e3D, kColorFormat, kSize, kSize,
                      kDepthOrArrayLayers, kMipLevelCounts);

    wgpu::TextureView colorView2D = Create2DAttachment(device, kSize, kSize, kColorFormat);

    wgpu::TextureViewDescriptor baseDescriptor;
    baseDescriptor.dimension = wgpu::TextureViewDimension::e3D;
    baseDescriptor.baseArrayLayer = 0;
    baseDescriptor.arrayLayerCount = 1;
    baseDescriptor.baseMipLevel = 0;
    baseDescriptor.mipLevelCount = 1;

    // Control case: It's valid if depthSlice is set within the depth range of a 3D color
    // attachment.
    {
        wgpu::TextureView view = colorTexture3D.CreateView(&baseDescriptor);
        utils::ComboRenderPassDescriptor renderPass({view});
        renderPass.cColorAttachments[0].depthSlice = kDepthOrArrayLayers - 1;
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // It's invalid if depthSlice is not set for a 3D color attachment.
    {
        wgpu::TextureView view = colorTexture3D.CreateView(&baseDescriptor);
        utils::ComboRenderPassDescriptor renderPass({view});
        AssertBeginRenderPassError(&renderPass);
    }

    // It's invalid if depthSlice is out of the depth range of a 3D color attachment.
    {
        wgpu::TextureView view = colorTexture3D.CreateView(&baseDescriptor);
        utils::ComboRenderPassDescriptor renderPass({view});
        renderPass.cColorAttachments[0].depthSlice = kDepthOrArrayLayers;
        AssertBeginRenderPassError(&renderPass);
    }

    // It's invalid if depthSlice is out of the depth range of a 3D color attachment with non-zero
    // mip level.
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.baseMipLevel = 2;
        wgpu::TextureView view = colorTexture3D.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({view});
        renderPass.cColorAttachments[0].depthSlice = kDepthOrArrayLayers >> 2;
        AssertBeginRenderPassError(&renderPass);
    }

    // Control case: It's valid if depthSlice is unset for a non-3D color attachment.
    {
        utils::ComboRenderPassDescriptor renderPass({colorView2D});
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // It's invalid if depthSlice is set for a non-3D color attachment.
    {
        utils::ComboRenderPassDescriptor renderPass({colorView2D});
        renderPass.cColorAttachments[0].depthSlice = 0;
        AssertBeginRenderPassError(&renderPass);
    }
}

// Check that the depth slices of a 3D color attachment cannot overlap in same render pass.
TEST_F(RenderPassDescriptorValidationTest, TextureViewDepthSliceOverlaps) {
    constexpr uint32_t kSize = 8;
    constexpr uint32_t kDepthOrArrayLayers = 2;
    constexpr uint32_t kMipLevelCounts = 2;
    constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::Texture colorTexture3D =
        CreateTexture(device, wgpu::TextureDimension::e3D, kColorFormat, kSize, kSize,
                      kDepthOrArrayLayers, kMipLevelCounts);

    wgpu::TextureViewDescriptor baseDescriptor;
    baseDescriptor.dimension = wgpu::TextureViewDimension::e3D;
    baseDescriptor.baseArrayLayer = 0;
    baseDescriptor.arrayLayerCount = 1;
    baseDescriptor.baseMipLevel = 0;
    baseDescriptor.mipLevelCount = 1;

    // Control case: It's valid if different depth slices of a texture are set in a render pass.
    {
        wgpu::TextureView view = colorTexture3D.CreateView(&baseDescriptor);
        utils::ComboRenderPassDescriptor renderPass({view, view});
        renderPass.cColorAttachments[0].depthSlice = 0;
        renderPass.cColorAttachments[1].depthSlice = 1;
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // It's valid if same depth slice of different mip levels from a texture with size [1, 1, n] is
    // set in a render pass.
    {
        wgpu::Texture texture = CreateTexture(device, wgpu::TextureDimension::e3D, kColorFormat, 1,
                                              1, kDepthOrArrayLayers, kMipLevelCounts);
        wgpu::TextureView view = texture.CreateView(&baseDescriptor);
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.baseMipLevel = 1;
        wgpu::TextureView view2 = texture.CreateView(&descriptor);

        utils::ComboRenderPassDescriptor renderPass({view, view2});
        renderPass.cColorAttachments[0].depthSlice = 0;
        renderPass.cColorAttachments[1].depthSlice = 0;
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // It's valid if same depth slice of different textures is set in a render pass.
    {
        wgpu::Texture otherColorTexture3D =
            CreateTexture(device, wgpu::TextureDimension::e3D, kColorFormat, kSize, kSize,
                          kDepthOrArrayLayers, kMipLevelCounts);

        wgpu::TextureView view = colorTexture3D.CreateView(&baseDescriptor);
        wgpu::TextureView view2 = otherColorTexture3D.CreateView(&baseDescriptor);

        utils::ComboRenderPassDescriptor renderPass({view, view2});
        renderPass.cColorAttachments[0].depthSlice = 0;
        renderPass.cColorAttachments[1].depthSlice = 0;
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // It's invalid if same depth slice of a texture is set twice in a render pass.
    {
        wgpu::TextureView view = colorTexture3D.CreateView(&baseDescriptor);
        utils::ComboRenderPassDescriptor renderPass({view, view});
        renderPass.cColorAttachments[0].depthSlice = 0;
        renderPass.cColorAttachments[1].depthSlice = 0;
        AssertBeginRenderPassError(&renderPass);
    }
}

// Check that the render pass depth attachment should not have any chained structs on it.
TEST_F(RenderPassDescriptorValidationTest, DepthAttachmentChained) {
    wgpu::TextureView renderView =
        Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth32Float);
    utils::ComboRenderPassDescriptor renderPass({}, renderView);
    renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
    renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

    wgpu::ChainedStruct chain = {};
    renderPass.cDepthStencilAttachmentInfo.nextInChain = &chain;

    AssertBeginRenderPassError(&renderPass);
}

// Check that the render pass depth attachment must have the RenderAttachment usage.
TEST_F(RenderPassDescriptorValidationTest, DepthAttachmentInvalidUsage) {
    // Control case: using a texture with RenderAttachment is valid.
    {
        wgpu::TextureView renderView =
            Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth32Float);
        utils::ComboRenderPassDescriptor renderPass({}, renderView);
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Error case: using a texture with Sampled is invalid.
    {
        wgpu::TextureDescriptor texDesc;
        texDesc.usage = wgpu::TextureUsage::TextureBinding;
        texDesc.size = {1, 1, 1};
        texDesc.format = wgpu::TextureFormat::Depth32Float;
        wgpu::Texture sampledTex = device.CreateTexture(&texDesc);
        wgpu::TextureView sampledView = sampledTex.CreateView();

        utils::ComboRenderPassDescriptor renderPass({}, sampledView);
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

        AssertBeginRenderPassError(&renderPass);
    }
}

// Only 2D texture views with mipLevelCount == 1 are allowed to be color attachments
TEST_F(RenderPassDescriptorValidationTest, TextureViewLevelCountForColorAndDepthStencil) {
    constexpr uint32_t kArrayLayers = 1;
    constexpr uint32_t kSize = 32;
    constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;
    constexpr wgpu::TextureFormat kDepthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;

    constexpr uint32_t kLevelCount = 4;

    wgpu::Texture colorTexture = CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat,
                                               kSize, kSize, kArrayLayers, kLevelCount);
    wgpu::Texture depthStencilTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, kDepthStencilFormat, kSize, kSize,
                      kArrayLayers, kLevelCount);

    wgpu::TextureViewDescriptor baseDescriptor;
    baseDescriptor.dimension = wgpu::TextureViewDimension::e2D;
    baseDescriptor.baseArrayLayer = 0;
    baseDescriptor.arrayLayerCount = kArrayLayers;
    baseDescriptor.baseMipLevel = 0;
    baseDescriptor.mipLevelCount = kLevelCount;

    // Using 2D texture view with mipLevelCount > 1 is not allowed for color
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kColorFormat;
        descriptor.mipLevelCount = 2;

        wgpu::TextureView colorTextureView = colorTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({colorTextureView});
        AssertBeginRenderPassError(&renderPass);
    }

    // Using 2D texture view with mipLevelCount > 1 is not allowed for depth stencil
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kDepthStencilFormat;
        descriptor.mipLevelCount = 2;

        wgpu::TextureView depthStencilView = depthStencilTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({}, depthStencilView);
        AssertBeginRenderPassError(&renderPass);
    }

    // Using 2D texture view that covers the first level of the texture is OK for color
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kColorFormat;
        descriptor.baseMipLevel = 0;
        descriptor.mipLevelCount = 1;

        wgpu::TextureView colorTextureView = colorTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({colorTextureView});
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Using 2D texture view that covers the first level is OK for depth stencil
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kDepthStencilFormat;
        descriptor.baseMipLevel = 0;
        descriptor.mipLevelCount = 1;

        wgpu::TextureView depthStencilView = depthStencilTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({}, depthStencilView);
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Using 2D texture view that covers the last level is OK for color
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kColorFormat;
        descriptor.baseMipLevel = kLevelCount - 1;
        descriptor.mipLevelCount = 1;

        wgpu::TextureView colorTextureView = colorTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({colorTextureView});
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Using 2D texture view that covers the last level is OK for depth stencil
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kDepthStencilFormat;
        descriptor.baseMipLevel = kLevelCount - 1;
        descriptor.mipLevelCount = 1;

        wgpu::TextureView depthStencilView = depthStencilTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({}, depthStencilView);
        AssertBeginRenderPassSuccess(&renderPass);
    }
}

// It is not allowed to set resolve target when the color attachment is non-multisampled.
TEST_F(RenderPassDescriptorValidationTest, NonMultisampledColorWithResolveTarget) {
    static constexpr uint32_t kArrayLayers = 1;
    static constexpr uint32_t kLevelCount = 1;
    static constexpr uint32_t kSize = 32;
    static constexpr uint32_t kSampleCount = 1;
    static constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::Texture colorTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers,
                      kLevelCount, kSampleCount);
    wgpu::Texture resolveTargetTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers,
                      kLevelCount, kSampleCount);
    wgpu::TextureView colorTextureView = colorTexture.CreateView();
    wgpu::TextureView resolveTargetTextureView = resolveTargetTexture.CreateView();

    utils::ComboRenderPassDescriptor renderPass({colorTextureView});
    renderPass.cColorAttachments[0].resolveTarget = resolveTargetTextureView;
    AssertBeginRenderPassError(&renderPass);
}

// drawCount must not exceed maxDrawCount
TEST_F(RenderPassDescriptorValidationTest, MaxDrawCount) {
    constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;
    constexpr uint64_t kMaxDrawCount = 16;

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        })");

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fsModule;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    wgpu::TextureDescriptor colorTextureDescriptor;
    colorTextureDescriptor.size = {1, 1};
    colorTextureDescriptor.format = kColorFormat;
    colorTextureDescriptor.usage = wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture colorTexture = device.CreateTexture(&colorTextureDescriptor);

    utils::ComboRenderBundleEncoderDescriptor bundleEncoderDescriptor;
    bundleEncoderDescriptor.colorFormatCount = 1;
    bundleEncoderDescriptor.cColorFormats[0] = kColorFormat;

    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {0, 1, 2});
    wgpu::Buffer indirectBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect, {3, 1, 0, 0});
    wgpu::Buffer indexedIndirectBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect, {3, 1, 0, 0, 0});

    wgpu::RenderPassMaxDrawCount maxDrawCount;
    maxDrawCount.maxDrawCount = kMaxDrawCount;

    // Valid. drawCount is less than the default maxDrawCount.

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(pipeline);

        for (uint64_t i = 0; i <= kMaxDrawCount; i++) {
            renderPass.Draw(3);
        }

        renderPass.End();
        encoder.Finish();
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(pipeline);
        renderPass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);

        for (uint64_t i = 0; i <= kMaxDrawCount; i++) {
            renderPass.DrawIndexed(3);
        }

        renderPass.End();
        encoder.Finish();
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(pipeline);

        for (uint64_t i = 0; i <= kMaxDrawCount; i++) {
            renderPass.DrawIndirect(indirectBuffer, 0);
        }

        renderPass.End();
        encoder.Finish();
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(pipeline);
        renderPass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);

        for (uint64_t i = 0; i <= kMaxDrawCount; i++) {
            renderPass.DrawIndexedIndirect(indexedIndirectBuffer, 0);
        }

        renderPass.End();
        encoder.Finish();
    }

    {
        wgpu::RenderBundleEncoder renderBundleEncoder =
            device.CreateRenderBundleEncoder(&bundleEncoderDescriptor);
        renderBundleEncoder.SetPipeline(pipeline);

        for (uint64_t i = 0; i <= kMaxDrawCount; i++) {
            renderBundleEncoder.Draw(3);
        }

        wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.ExecuteBundles(1, &renderBundle);
        renderPass.End();
        encoder.Finish();
    }

    // Invalid. drawCount counts up with draw calls and
    // it is greater than maxDrawCount.

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});
        renderPassDescriptor.nextInChain = &maxDrawCount;
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(pipeline);

        for (uint64_t i = 0; i <= kMaxDrawCount; i++) {
            renderPass.Draw(3);
        }

        renderPass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});
        renderPassDescriptor.nextInChain = &maxDrawCount;
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(pipeline);
        renderPass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);

        for (uint64_t i = 0; i <= kMaxDrawCount; i++) {
            renderPass.DrawIndexed(3);
        }

        renderPass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});
        renderPassDescriptor.nextInChain = &maxDrawCount;
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(pipeline);

        for (uint64_t i = 0; i <= kMaxDrawCount; i++) {
            renderPass.DrawIndirect(indirectBuffer, 0);
        }

        renderPass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});
        renderPassDescriptor.nextInChain = &maxDrawCount;
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(pipeline);
        renderPass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);

        for (uint64_t i = 0; i <= kMaxDrawCount; i++) {
            renderPass.DrawIndexedIndirect(indexedIndirectBuffer, 0);
        }

        renderPass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        wgpu::RenderBundleEncoder renderBundleEncoder =
            device.CreateRenderBundleEncoder(&bundleEncoderDescriptor);
        renderBundleEncoder.SetPipeline(pipeline);

        for (uint64_t i = 0; i <= kMaxDrawCount; i++) {
            renderBundleEncoder.Draw(3);
        }

        wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});
        renderPassDescriptor.nextInChain = &maxDrawCount;
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.ExecuteBundles(1, &renderBundle);
        renderPass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

class MultisampledRenderPassDescriptorValidationTest : public RenderPassDescriptorValidationTest {
  public:
    utils::ComboRenderPassDescriptor CreateMultisampledRenderPass() {
        return utils::ComboRenderPassDescriptor({CreateMultisampledColorTextureView()});
    }

    wgpu::TextureView CreateMultisampledColorTextureView() {
        return CreateColorTextureView(kSampleCount);
    }

    wgpu::TextureView CreateNonMultisampledColorTextureView() { return CreateColorTextureView(1); }

    static constexpr uint32_t kArrayLayers = 1;
    static constexpr uint32_t kLevelCount = 1;
    static constexpr uint32_t kSize = 32;
    static constexpr uint32_t kSampleCount = 4;
    static constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;

  private:
    wgpu::TextureView CreateColorTextureView(uint32_t sampleCount) {
        wgpu::Texture colorTexture =
            CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat, kSize, kSize,
                          kArrayLayers, kLevelCount, sampleCount);

        return colorTexture.CreateView();
    }
};

// Tests on the use of multisampled textures as color attachments
TEST_F(MultisampledRenderPassDescriptorValidationTest, MultisampledColorAttachments) {
    wgpu::TextureView colorTextureView = CreateNonMultisampledColorTextureView();
    wgpu::TextureView resolveTargetTextureView = CreateNonMultisampledColorTextureView();
    wgpu::TextureView multisampledColorTextureView = CreateMultisampledColorTextureView();

    // It is allowed to use a multisampled color attachment without setting resolve target.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // It is not allowed to use multiple color attachments with different sample counts.
    {
        utils::ComboRenderPassDescriptor renderPass(
            {multisampledColorTextureView, colorTextureView});
        AssertBeginRenderPassError(&renderPass);
    }
}

// It is not allowed to use a multisampled resolve target.
TEST_F(MultisampledRenderPassDescriptorValidationTest, MultisampledResolveTarget) {
    wgpu::TextureView multisampledResolveTargetView = CreateMultisampledColorTextureView();

    utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].resolveTarget = multisampledResolveTargetView;
    AssertBeginRenderPassError(&renderPass);
}

// Test the view dimension of resolve target.
TEST_F(MultisampledRenderPassDescriptorValidationTest, ResolveTargetDimension) {
    wgpu::Texture resolveTexture2D = CreateTexture(
        device, wgpu::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers, kLevelCount);

    // It is allowed to use a 2d texture view as resolve target.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
        renderPass.cColorAttachments[0].resolveTarget = resolveTexture2D.CreateView();
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // It is allowed to use a 2d-array texture view as resolve target.
    {
        wgpu::TextureViewDescriptor viewDesc;
        viewDesc.dimension = wgpu::TextureViewDimension::e2DArray;

        utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
        renderPass.cColorAttachments[0].resolveTarget = resolveTexture2D.CreateView(&viewDesc);
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // It is not allowed to use a 3d texture view as resolve target.
    {
        wgpu::Texture resolveTexture3D =
            CreateTexture(device, wgpu::TextureDimension::e3D, kColorFormat, kSize, kSize,
                          kArrayLayers, kLevelCount);
        utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
        renderPass.cColorAttachments[0].resolveTarget = resolveTexture3D.CreateView();
        AssertBeginRenderPassError(&renderPass);
    }
}

// It is not allowed to use a resolve target with array layer count > 1.
TEST_F(MultisampledRenderPassDescriptorValidationTest, ResolveTargetArrayLayerMoreThanOne) {
    constexpr uint32_t kArrayLayers2 = 2;
    wgpu::Texture resolveTexture = CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat,
                                                 kSize, kSize, kArrayLayers2, kLevelCount);
    wgpu::TextureViewDescriptor viewDesc;
    viewDesc.dimension = wgpu::TextureViewDimension::e2DArray;
    wgpu::TextureView resolveTextureView = resolveTexture.CreateView(&viewDesc);

    utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].resolveTarget = resolveTextureView;
    AssertBeginRenderPassError(&renderPass);
}

// It is not allowed to use a resolve target with mipmap level count > 1.
TEST_F(MultisampledRenderPassDescriptorValidationTest, ResolveTargetMipmapLevelMoreThanOne) {
    constexpr uint32_t kLevelCount2 = 2;
    wgpu::Texture resolveTexture = CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat,
                                                 kSize, kSize, kArrayLayers, kLevelCount2);
    wgpu::TextureView resolveTextureView = resolveTexture.CreateView();

    utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].resolveTarget = resolveTextureView;
    AssertBeginRenderPassError(&renderPass);
}

// It is not allowed to use a resolve target which is created from a texture whose usage does
// not include wgpu::TextureUsage::RenderAttachment.
TEST_F(MultisampledRenderPassDescriptorValidationTest, ResolveTargetUsageNoRenderAttachment) {
    constexpr wgpu::TextureUsage kUsage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
    wgpu::Texture nonColorUsageResolveTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers,
                      kLevelCount, 1, kUsage);
    wgpu::TextureView nonColorUsageResolveTextureView = nonColorUsageResolveTexture.CreateView();

    utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].resolveTarget = nonColorUsageResolveTextureView;
    AssertBeginRenderPassError(&renderPass);
}

// It is not allowed to use a resolve target which is in error state.
TEST_F(MultisampledRenderPassDescriptorValidationTest, ResolveTargetInErrorState) {
    wgpu::Texture resolveTexture = CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat,
                                                 kSize, kSize, kArrayLayers, kLevelCount);
    wgpu::TextureViewDescriptor errorTextureView;
    errorTextureView.dimension = wgpu::TextureViewDimension::e2D;
    errorTextureView.format = kColorFormat;
    errorTextureView.baseArrayLayer = kArrayLayers + 1;
    ASSERT_DEVICE_ERROR(wgpu::TextureView errorResolveTarget =
                            resolveTexture.CreateView(&errorTextureView));

    utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].resolveTarget = errorResolveTarget;
    AssertBeginRenderPassError(&renderPass);
}

// It is allowed to use a multisampled color attachment and a non-multisampled resolve target.
TEST_F(MultisampledRenderPassDescriptorValidationTest, MultisampledColorWithResolveTarget) {
    wgpu::TextureView resolveTargetTextureView = CreateNonMultisampledColorTextureView();

    utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].resolveTarget = resolveTargetTextureView;
    AssertBeginRenderPassSuccess(&renderPass);
}

// It is not allowed to use a resolve target in a format different from the color attachment.
TEST_F(MultisampledRenderPassDescriptorValidationTest, ResolveTargetDifferentFormat) {
    constexpr wgpu::TextureFormat kColorFormat2 = wgpu::TextureFormat::BGRA8Unorm;
    wgpu::Texture resolveTexture = CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat2,
                                                 kSize, kSize, kArrayLayers, kLevelCount);
    wgpu::TextureView resolveTextureView = resolveTexture.CreateView();

    utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].resolveTarget = resolveTextureView;
    AssertBeginRenderPassError(&renderPass);
}

// Tests on the size of the resolve target.
TEST_F(MultisampledRenderPassDescriptorValidationTest,
       ColorAttachmentResolveTargetDimensionMismatch) {
    constexpr uint32_t kSize2 = kSize * 2;
    wgpu::Texture resolveTexture = CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat,
                                                 kSize2, kSize2, kArrayLayers, kLevelCount + 1);

    wgpu::TextureViewDescriptor textureViewDescriptor;
    textureViewDescriptor.nextInChain = nullptr;
    textureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
    textureViewDescriptor.format = kColorFormat;
    textureViewDescriptor.mipLevelCount = 1;
    textureViewDescriptor.baseArrayLayer = 0;
    textureViewDescriptor.arrayLayerCount = 1;

    {
        wgpu::TextureViewDescriptor firstMipLevelDescriptor = textureViewDescriptor;
        firstMipLevelDescriptor.baseMipLevel = 0;

        wgpu::TextureView resolveTextureView = resolveTexture.CreateView(&firstMipLevelDescriptor);

        utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
        renderPass.cColorAttachments[0].resolveTarget = resolveTextureView;
        AssertBeginRenderPassError(&renderPass);
    }

    {
        wgpu::TextureViewDescriptor secondMipLevelDescriptor = textureViewDescriptor;
        secondMipLevelDescriptor.baseMipLevel = 1;

        wgpu::TextureView resolveTextureView = resolveTexture.CreateView(&secondMipLevelDescriptor);

        utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
        renderPass.cColorAttachments[0].resolveTarget = resolveTextureView;
        AssertBeginRenderPassSuccess(&renderPass);
    }
}

// Test the overlaps of multiple resolve target.
TEST_F(MultisampledRenderPassDescriptorValidationTest, ResolveTargetUsedTwice) {
    wgpu::TextureView resolveTextureView = CreateNonMultisampledColorTextureView();
    wgpu::TextureView colorTextureView1 = CreateMultisampledColorTextureView();
    wgpu::TextureView colorTextureView2 = CreateMultisampledColorTextureView();

    // It is allowed to use different resolve targets in a render pass.
    {
        wgpu::TextureView anotherResolveTextureView = CreateNonMultisampledColorTextureView();
        utils::ComboRenderPassDescriptor renderPass =
            utils::ComboRenderPassDescriptor({colorTextureView1, colorTextureView2});
        renderPass.cColorAttachments[0].resolveTarget = resolveTextureView;
        renderPass.cColorAttachments[1].resolveTarget = anotherResolveTextureView;

        AssertBeginRenderPassSuccess(&renderPass);
    }

    // It is not allowed to use a resolve target twice in a render pass.
    {
        utils::ComboRenderPassDescriptor renderPass =
            utils::ComboRenderPassDescriptor({colorTextureView1, colorTextureView2});
        renderPass.cColorAttachments[0].resolveTarget = resolveTextureView;
        renderPass.cColorAttachments[1].resolveTarget = resolveTextureView;

        AssertBeginRenderPassError(&renderPass);
    }
}

// Tests the texture format of the resolve target must support being used as resolve target.
TEST_F(MultisampledRenderPassDescriptorValidationTest, ResolveTargetFormat) {
    for (wgpu::TextureFormat format : utils::kAllTextureFormats) {
        if (!utils::TextureFormatSupportsMultisampling(device, format, UseCompatibilityMode()) ||
            utils::IsDepthOrStencilFormat(format)) {
            continue;
        }

        wgpu::Texture colorTexture =
            CreateTexture(device, wgpu::TextureDimension::e2D, format, kSize, kSize, kArrayLayers,
                          kLevelCount, kSampleCount);
        wgpu::Texture resolveTarget = CreateTexture(device, wgpu::TextureDimension::e2D, format,
                                                    kSize, kSize, kArrayLayers, kLevelCount, 1);

        utils::ComboRenderPassDescriptor renderPass({colorTexture.CreateView()});
        renderPass.cColorAttachments[0].resolveTarget = resolveTarget.CreateView();
        if (utils::TextureFormatSupportsResolveTarget(device, format)) {
            AssertBeginRenderPassSuccess(&renderPass);
        } else {
            AssertBeginRenderPassError(&renderPass);
        }
    }
}

// Tests on the sample count of depth stencil attachment.
TEST_F(MultisampledRenderPassDescriptorValidationTest, DepthStencilAttachmentSampleCount) {
    constexpr wgpu::TextureFormat kDepthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;
    wgpu::Texture multisampledDepthStencilTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, kDepthStencilFormat, kSize, kSize,
                      kArrayLayers, kLevelCount, kSampleCount);
    wgpu::TextureView multisampledDepthStencilTextureView =
        multisampledDepthStencilTexture.CreateView();

    // It is not allowed to use a depth stencil attachment whose sample count is different from
    // the one of the color attachment.
    {
        wgpu::Texture depthStencilTexture =
            CreateTexture(device, wgpu::TextureDimension::e2D, kDepthStencilFormat, kSize, kSize,
                          kArrayLayers, kLevelCount);
        wgpu::TextureView depthStencilTextureView = depthStencilTexture.CreateView();

        utils::ComboRenderPassDescriptor renderPass({CreateMultisampledColorTextureView()},
                                                    depthStencilTextureView);
        AssertBeginRenderPassError(&renderPass);
    }

    {
        utils::ComboRenderPassDescriptor renderPass({CreateNonMultisampledColorTextureView()},
                                                    multisampledDepthStencilTextureView);
        AssertBeginRenderPassError(&renderPass);
    }

    // It is allowed to use a multisampled depth stencil attachment whose sample count is equal
    // to the one of the color attachment.
    {
        utils::ComboRenderPassDescriptor renderPass({CreateMultisampledColorTextureView()},
                                                    multisampledDepthStencilTextureView);
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // It is allowed to use a multisampled depth stencil attachment while there is no color
    // attachment.
    {
        utils::ComboRenderPassDescriptor renderPass({}, multisampledDepthStencilTextureView);
        AssertBeginRenderPassSuccess(&renderPass);
    }
}

// Creating a render pass with DawnRenderPassColorAttachmentRenderToSingleSampled chained struct
// without MSAARenderToSingleSampled feature enabled should result in error.
TEST_F(MultisampledRenderPassDescriptorValidationTest,
       CreateMSAARenderToSingleSampledRenderPassWithoutFeatureEnabled) {
    wgpu::TextureView colorTextureView = CreateNonMultisampledColorTextureView();

    wgpu::DawnRenderPassColorAttachmentRenderToSingleSampled renderToSingleSampledDesc;
    renderToSingleSampledDesc.implicitSampleCount = 4;
    utils::ComboRenderPassDescriptor renderPass({colorTextureView});
    renderPass.cColorAttachments[0].nextInChain = &renderToSingleSampledDesc;

    AssertBeginRenderPassError(&renderPass, testing::HasSubstr("feature is not enabled"));
}

// Creating a render pass with LoadOp::ExpandResolveTexture without DawnLoadResolveTexture feature
// enabled should result in error.
TEST_F(MultisampledRenderPassDescriptorValidationTest, LoadResolveTextureWithoutFeatureEnabled) {
    auto multisampledColorTextureView = CreateMultisampledColorTextureView();
    auto resolveTarget = CreateNonMultisampledColorTextureView();

    auto renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].view = multisampledColorTextureView;
    renderPass.cColorAttachments[0].resolveTarget = resolveTarget;
    renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;

    AssertBeginRenderPassError(&renderPass, testing::HasSubstr("is not enabled"));
}

// Creating a render pass with ExpandResolveRect and no DawnLoadResolveTexture feature
// enabled should result in error.
TEST_F(MultisampledRenderPassDescriptorValidationTest, ExpandResolveRectWithoutFeatureEnabled) {
    auto multisampledColorTextureView = CreateMultisampledColorTextureView();
    auto resolveTarget = CreateNonMultisampledColorTextureView();

    wgpu::RenderPassDescriptorExpandResolveRect rect{};
    rect.x = rect.y = 0;
    rect.width = rect.height = 1;

    auto renderPass = CreateMultisampledRenderPass();
    renderPass.nextInChain = &rect;
    renderPass.cColorAttachments[0].view = multisampledColorTextureView;
    renderPass.cColorAttachments[0].resolveTarget = resolveTarget;
    renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;

    AssertBeginRenderPassError(&renderPass);
}

// Tests that NaN cannot be accepted as a valid color or depth clear value and INFINITY is valid
// in both color and depth clear values.
TEST_F(RenderPassDescriptorValidationTest, UseNaNOrINFINITYAsColorOrDepthClearValue) {
    wgpu::TextureView color = Create2DAttachment(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);

    // Tests that NaN cannot be used in clearColor.
    {
        utils::ComboRenderPassDescriptor renderPass({color}, nullptr);
        renderPass.cColorAttachments[0].clearValue.r = NAN;
        AssertBeginRenderPassError(&renderPass);
    }

    {
        utils::ComboRenderPassDescriptor renderPass({color}, nullptr);
        renderPass.cColorAttachments[0].clearValue.g = NAN;
        AssertBeginRenderPassError(&renderPass);
    }

    {
        utils::ComboRenderPassDescriptor renderPass({color}, nullptr);
        renderPass.cColorAttachments[0].clearValue.b = NAN;
        AssertBeginRenderPassError(&renderPass);
    }

    {
        utils::ComboRenderPassDescriptor renderPass({color}, nullptr);
        renderPass.cColorAttachments[0].clearValue.a = NAN;
        AssertBeginRenderPassError(&renderPass);
    }

    // Tests that INFINITY cannot be used in clearColor.
    {
        utils::ComboRenderPassDescriptor renderPass({color}, nullptr);
        renderPass.cColorAttachments[0].clearValue.r = INFINITY;
        AssertBeginRenderPassError(&renderPass);
    }

    {
        utils::ComboRenderPassDescriptor renderPass({color}, nullptr);
        renderPass.cColorAttachments[0].clearValue.g = INFINITY;
        AssertBeginRenderPassError(&renderPass);
    }

    {
        utils::ComboRenderPassDescriptor renderPass({color}, nullptr);
        renderPass.cColorAttachments[0].clearValue.b = INFINITY;
        AssertBeginRenderPassError(&renderPass);
    }

    {
        utils::ComboRenderPassDescriptor renderPass({color}, nullptr);
        renderPass.cColorAttachments[0].clearValue.a = INFINITY;
        AssertBeginRenderPassError(&renderPass);
    }

    // Tests that NaN cannot be used in depthClearValue if depthLoadOp is Clear.
    {
        wgpu::TextureView depth =
            Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth24Plus);
        utils::ComboRenderPassDescriptor renderPass({color}, depth);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Clear;
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthClearValue = NAN;
        AssertBeginRenderPassError(&renderPass);
    }

    // Tests that INFINITY cannot be used in depthClearValue.
    {
        wgpu::TextureView depth =
            Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth24Plus);
        utils::ComboRenderPassDescriptor renderPass({color}, depth);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Clear;
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthClearValue = INFINITY;
        AssertBeginRenderPassError(&renderPass);
    }

    // TODO(https://crbug.com/dawn/666): Add a test case for clearStencil for stencilOnly
    // once stencil8 is supported.
}

// Tests that depth clear values mut be between 0 and 1, inclusive.
TEST_F(RenderPassDescriptorValidationTest, ValidateDepthClearValueRange) {
    wgpu::TextureView depth = Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth24Plus);

    utils::ComboRenderPassDescriptor renderPass({}, depth);
    renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
    renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

    // 0, 1, and any value in between are be valid.
    renderPass.cDepthStencilAttachmentInfo.depthClearValue = 0;
    AssertBeginRenderPassSuccess(&renderPass);

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = 0.1;
    AssertBeginRenderPassSuccess(&renderPass);

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = 0.5;
    AssertBeginRenderPassSuccess(&renderPass);

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = 0.82;
    AssertBeginRenderPassSuccess(&renderPass);

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = 1;
    AssertBeginRenderPassSuccess(&renderPass);

    // Values less than 0 or greater than 1 are invalid.
    renderPass.cDepthStencilAttachmentInfo.depthClearValue = -1;
    AssertBeginRenderPassError(&renderPass);

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = 2;
    AssertBeginRenderPassError(&renderPass);

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = -0.001;
    AssertBeginRenderPassError(&renderPass);

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = 1.001;
    AssertBeginRenderPassError(&renderPass);

    // Clear values are not validated if the depthLoadOp is Load.
    renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Load;

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = -1;
    AssertBeginRenderPassSuccess(&renderPass);

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = 2;
    AssertBeginRenderPassSuccess(&renderPass);

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = -0.001;
    AssertBeginRenderPassSuccess(&renderPass);

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = 1.001;
    AssertBeginRenderPassSuccess(&renderPass);
}

// Tests that default depthClearValue is required if attachment has a depth aspect and depthLoadOp
// is clear.
TEST_F(RenderPassDescriptorValidationTest, DefaultDepthClearValue) {
    wgpu::TextureView depthView =
        Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth24Plus);
    wgpu::TextureView stencilView = Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Stencil8);

    wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;

    wgpu::RenderPassDescriptor renderPassDescriptor;
    renderPassDescriptor.colorAttachmentCount = 0;
    renderPassDescriptor.colorAttachments = nullptr;
    renderPassDescriptor.depthStencilAttachment = &depthStencilAttachment;

    // Default depthClearValue should be accepted if attachment doesn't have
    // a depth aspect.
    depthStencilAttachment.view = stencilView;
    depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Load;
    depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
    AssertBeginRenderPassSuccess(&renderPassDescriptor);

    // Default depthClearValue should be accepted if depthLoadOp is not clear.
    depthStencilAttachment.view = depthView;
    depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Undefined;
    depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Undefined;
    depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Load;
    depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
    AssertBeginRenderPassSuccess(&renderPassDescriptor);

    // Default depthClearValue should fail the validation
    // if attachment has a depth aspect and depthLoadOp is clear.
    depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
    AssertBeginRenderPassError(&renderPassDescriptor);

    // The validation should pass if valid depthClearValue is provided.
    depthStencilAttachment.depthClearValue = 0.0f;
    AssertBeginRenderPassSuccess(&renderPassDescriptor);
}

// Check the validation rules around depth/stencilReadOnly
TEST_F(RenderPassDescriptorValidationTest, ValidateDepthStencilReadOnly) {
    wgpu::TextureView colorView = Create2DAttachment(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
    wgpu::TextureView depthStencilView =
        Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth24PlusStencil8);
    wgpu::TextureView depthStencilViewNoStencil =
        Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth24Plus);
    wgpu::TextureView stencilView = Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Stencil8);

    using Aspect = wgpu::TextureAspect;
    struct TestSpec {
        wgpu::TextureFormat format;
        Aspect formatAspects;
        Aspect testAspect;
    };

    TestSpec specs[] = {
        {wgpu::TextureFormat::Depth24PlusStencil8, Aspect::All, Aspect::StencilOnly},
        {wgpu::TextureFormat::Depth24PlusStencil8, Aspect::All, Aspect::DepthOnly},
        {wgpu::TextureFormat::Depth24Plus, Aspect::DepthOnly, Aspect::DepthOnly},
        {wgpu::TextureFormat::Stencil8, Aspect::All, Aspect::StencilOnly},
    };
    for (const auto& spec : specs) {
        wgpu::TextureView depthStencil = Create2DAttachment(device, 1, 1, spec.format);
        utils::ComboRenderPassDescriptor renderPass({}, depthStencilView);

        Aspect testAspect = spec.testAspect;
        Aspect otherAspect =
            testAspect == Aspect::DepthOnly ? Aspect::StencilOnly : Aspect::DepthOnly;

        auto Set = [&](Aspect aspect, wgpu::LoadOp loadOp, wgpu::StoreOp storeOp, bool readonly) {
            if (aspect == Aspect::DepthOnly) {
                renderPass.cDepthStencilAttachmentInfo.depthLoadOp = loadOp;
                renderPass.cDepthStencilAttachmentInfo.depthStoreOp = storeOp;
                renderPass.cDepthStencilAttachmentInfo.depthReadOnly = readonly;
            } else {
                DAWN_ASSERT(aspect == Aspect::StencilOnly);
                renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = loadOp;
                renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = storeOp;
                renderPass.cDepthStencilAttachmentInfo.stencilReadOnly = readonly;
            }
        };

        // Tests that a read-only pass with depth/stencilReadOnly both set to true succeeds.
        Set(testAspect, wgpu::LoadOp::Undefined, wgpu::StoreOp::Undefined, true);
        Set(otherAspect, wgpu::LoadOp::Undefined, wgpu::StoreOp::Undefined, true);
        AssertBeginRenderPassSuccess(&renderPass);

        // Tests that readOnly with LoadOp not undefined is invalid.
        Set(testAspect, wgpu::LoadOp::Clear, wgpu::StoreOp::Undefined, true);
        Set(otherAspect, wgpu::LoadOp::Undefined, wgpu::StoreOp::Undefined, true);
        AssertBeginRenderPassError(&renderPass);

        Set(testAspect, wgpu::LoadOp::Load, wgpu::StoreOp::Undefined, true);
        Set(otherAspect, wgpu::LoadOp::Undefined, wgpu::StoreOp::Undefined, true);
        AssertBeginRenderPassError(&renderPass);

        // Tests that readOnly with StoreOp not undefined is invalid.
        Set(testAspect, wgpu::LoadOp::Undefined, wgpu::StoreOp::Store, true);
        Set(otherAspect, wgpu::LoadOp::Undefined, wgpu::StoreOp::Undefined, true);
        AssertBeginRenderPassError(&renderPass);

        Set(testAspect, wgpu::LoadOp::Undefined, wgpu::StoreOp::Discard, true);
        Set(otherAspect, wgpu::LoadOp::Undefined, wgpu::StoreOp::Undefined, true);
        AssertBeginRenderPassError(&renderPass);

        // Test for the aspect's not present in the format, if applicable.
        if (testAspect != spec.formatAspects) {
            // Tests that readOnly with LoadOp not undefined is invalid even if the aspect is not in
            // the format.
            Set(testAspect, wgpu::LoadOp::Undefined, wgpu::StoreOp::Undefined, true);
            Set(otherAspect, wgpu::LoadOp::Clear, wgpu::StoreOp::Undefined, true);
            AssertBeginRenderPassError(&renderPass);

            Set(testAspect, wgpu::LoadOp::Undefined, wgpu::StoreOp::Undefined, true);
            Set(otherAspect, wgpu::LoadOp::Load, wgpu::StoreOp::Undefined, true);
            AssertBeginRenderPassError(&renderPass);

            // Tests that readOnly with StoreOp not undefined is invalid even if the aspect is not
            // in the format.
            Set(testAspect, wgpu::LoadOp::Undefined, wgpu::StoreOp::Undefined, true);
            Set(otherAspect, wgpu::LoadOp::Undefined, wgpu::StoreOp::Store, true);
            AssertBeginRenderPassError(&renderPass);

            Set(testAspect, wgpu::LoadOp::Undefined, wgpu::StoreOp::Undefined, true);
            Set(otherAspect, wgpu::LoadOp::Undefined, wgpu::StoreOp::Discard, true);
            AssertBeginRenderPassError(&renderPass);
        }

        // Test that it is allowed to set only one of the aspects readonly.
        Set(testAspect, wgpu::LoadOp::Undefined, wgpu::StoreOp::Undefined, true);
        Set(otherAspect, wgpu::LoadOp::Load, wgpu::StoreOp::Store, false);
        AssertBeginRenderPassSuccess(&renderPass);

        Set(testAspect, wgpu::LoadOp::Load, wgpu::StoreOp::Store, false);
        Set(otherAspect, wgpu::LoadOp::Undefined, wgpu::StoreOp::Undefined, true);
        AssertBeginRenderPassSuccess(&renderPass);
    }
}

// Check that the depth stencil attachment must use all aspects.
TEST_F(RenderPassDescriptorValidationTest, ValidateDepthStencilAllAspects) {
    wgpu::TextureDescriptor texDesc;
    texDesc.usage = wgpu::TextureUsage::RenderAttachment;
    texDesc.size = {1, 1, 1};

    wgpu::TextureViewDescriptor viewDesc;
    viewDesc.baseMipLevel = 0;
    viewDesc.mipLevelCount = 1;
    viewDesc.baseArrayLayer = 0;
    viewDesc.arrayLayerCount = 1;

    // Using all aspects of a depth+stencil texture is allowed.
    {
        texDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        viewDesc.format = wgpu::TextureFormat::Undefined;
        viewDesc.aspect = wgpu::TextureAspect::All;

        wgpu::TextureView view = device.CreateTexture(&texDesc).CreateView(&viewDesc);
        utils::ComboRenderPassDescriptor renderPass({}, view);
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Using only depth of a depth+stencil texture is an error, case without format
    // reinterpretation.
    {
        texDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        viewDesc.format = wgpu::TextureFormat::Undefined;
        viewDesc.aspect = wgpu::TextureAspect::DepthOnly;

        wgpu::TextureView view = device.CreateTexture(&texDesc).CreateView(&viewDesc);
        utils::ComboRenderPassDescriptor renderPass({}, view);
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

        AssertBeginRenderPassError(&renderPass);
    }

    // Using only depth of a depth+stencil texture is an error, case with format reinterpretation.
    {
        texDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        viewDesc.format = wgpu::TextureFormat::Depth24Plus;
        viewDesc.aspect = wgpu::TextureAspect::DepthOnly;

        wgpu::TextureView view = device.CreateTexture(&texDesc).CreateView(&viewDesc);
        utils::ComboRenderPassDescriptor renderPass({}, view);
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

        AssertBeginRenderPassError(&renderPass);
    }

    // Using only stencil of a depth+stencil texture is an error, case without format
    // reinterpration.
    {
        texDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        viewDesc.format = wgpu::TextureFormat::Undefined;
        viewDesc.aspect = wgpu::TextureAspect::StencilOnly;

        wgpu::TextureView view = device.CreateTexture(&texDesc).CreateView(&viewDesc);
        utils::ComboRenderPassDescriptor renderPass({}, view);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;

        AssertBeginRenderPassError(&renderPass);
    }

    // Using only stencil of a depth+stencil texture is an error, case with format reinterpretation.
    {
        texDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        viewDesc.format = wgpu::TextureFormat::Stencil8;
        viewDesc.aspect = wgpu::TextureAspect::StencilOnly;

        wgpu::TextureView view = device.CreateTexture(&texDesc).CreateView(&viewDesc);
        utils::ComboRenderPassDescriptor renderPass({}, view);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;

        AssertBeginRenderPassError(&renderPass);
    }

    // Using DepthOnly of a depth only texture is allowed.
    {
        texDesc.format = wgpu::TextureFormat::Depth24Plus;
        viewDesc.format = wgpu::TextureFormat::Undefined;
        viewDesc.aspect = wgpu::TextureAspect::DepthOnly;

        wgpu::TextureView view = device.CreateTexture(&texDesc).CreateView(&viewDesc);
        utils::ComboRenderPassDescriptor renderPass({}, view);
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Using StencilOnly of a stencil only texture is allowed.
    {
        texDesc.format = wgpu::TextureFormat::Stencil8;
        viewDesc.format = wgpu::TextureFormat::Undefined;
        viewDesc.aspect = wgpu::TextureAspect::StencilOnly;

        wgpu::TextureView view = device.CreateTexture(&texDesc).CreateView(&viewDesc);
        utils::ComboRenderPassDescriptor renderPass({}, view);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;

        AssertBeginRenderPassSuccess(&renderPass);
    }
}

// Tests validation for per-pixel accounting for render targets. The tests currently assume that the
// default maxColorAttachmentBytesPerSample limit of 32 is used.
TEST_F(RenderPassDescriptorValidationTest, RenderPassColorAttachmentBytesPerSample) {
    struct TestCase {
        std::vector<wgpu::TextureFormat> formats;
        bool success;
    };
    static std::vector<TestCase> kTestCases = {
        // Simple 1 format cases.

        // R8Unorm take 1 byte and are aligned to 1 byte so we can have 8 (max).
        {{wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm,
          wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm,
          wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm},
         true},
        // RGBA8Uint takes 4 bytes and are aligned to 1 byte so we can have 8 (max).
        {{wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Uint,
          wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Uint,
          wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Uint,
          wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Uint},
         true},
        // RGBA8Unorm takes 8 bytes (special case) and are aligned to 1 byte so only 4 allowed.
        {{wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Unorm,
          wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Unorm},
         true},
        {{wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Unorm,
          wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Unorm,
          wgpu::TextureFormat::RGBA8Unorm},
         false},
        // RGBA32Float takes 16 bytes and are aligned to 4 bytes so only 2 are allowed.
        {{wgpu::TextureFormat::RGBA32Float, wgpu::TextureFormat::RGBA32Float}, true},
        {{wgpu::TextureFormat::RGBA32Float, wgpu::TextureFormat::RGBA32Float,
          wgpu::TextureFormat::RGBA32Float},
         false},

        // Different format alignment cases.

        // Alignment causes the first 1 byte R8Unorm to become 4 bytes. So even though 1+4+8+16+1 <
        // 32, the 4 byte alignment requirement of R32Float makes the first R8Unorm become 4 and
        // 4+4+8+16+1 > 32. Re-ordering this so the R8Unorm's are at the end, however is allowed:
        // 4+8+16+1+1 < 32.
        {{wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R32Float,
          wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA32Float,
          wgpu::TextureFormat::R8Unorm},
         false},
        {{wgpu::TextureFormat::R32Float, wgpu::TextureFormat::RGBA8Unorm,
          wgpu::TextureFormat::RGBA32Float, wgpu::TextureFormat::R8Unorm,
          wgpu::TextureFormat::R8Unorm},
         true},
    };

    for (const TestCase& testCase : kTestCases) {
        std::vector<wgpu::TextureView> colorAttachmentInfo;
        for (size_t i = 0; i < testCase.formats.size(); i++) {
            colorAttachmentInfo.push_back(Create2DAttachment(device, 1, 1, testCase.formats.at(i)));
        }
        utils::ComboRenderPassDescriptor descriptor(colorAttachmentInfo);
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&descriptor);
        renderPassEncoder.End();
        if (testCase.success) {
            commandEncoder.Finish();
        } else {
            ASSERT_DEVICE_ERROR(commandEncoder.Finish());
        }
    }
}

// TODO(cwallez@chromium.org): Constraints on attachment aliasing?

class MSAARenderToSingleSampledRenderPassDescriptorValidationTest
    : public MultisampledRenderPassDescriptorValidationTest {
  protected:
    void SetUp() override {
        MultisampledRenderPassDescriptorValidationTest::SetUp();

        mRenderToSingleSampledDesc.implicitSampleCount = kSampleCount;
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::MSAARenderToSingleSampled};
    }

    utils::ComboRenderPassDescriptor CreateMultisampledRenderToSingleSampledRenderPass(
        wgpu::TextureView colorAttachment,
        wgpu::TextureView depthStencilAttachment = nullptr) {
        utils::ComboRenderPassDescriptor renderPass({colorAttachment}, depthStencilAttachment);
        renderPass.cColorAttachments[0].nextInChain = &mRenderToSingleSampledDesc;

        return renderPass;
    }

    // Create a view for a texture that can be used with a MSAA render to single sampled render
    // pass.
    wgpu::TextureView CreateCompatibleColorTextureView() {
        wgpu::Texture colorTexture = CreateTexture(
            device, wgpu::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers,
            kLevelCount, /*sampleCount=*/1,
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding);

        return colorTexture.CreateView();
    }

  private:
    wgpu::DawnRenderPassColorAttachmentRenderToSingleSampled mRenderToSingleSampledDesc;
};

// Test that using a valid color attachment with enabled MSAARenderToSingleSampled doesn't raise any
// error.
TEST_F(MSAARenderToSingleSampledRenderPassDescriptorValidationTest, ColorAttachmentValid) {
    // Create a texture with sample count = 1.
    auto textureView = CreateCompatibleColorTextureView();

    auto renderPass = CreateMultisampledRenderToSingleSampledRenderPass(textureView);
    AssertBeginRenderPassSuccess(&renderPass);
}

// When MSAARenderToSingleSampled is enabled for a color attachment, it must be created with
// TextureBinding usage.
TEST_F(MSAARenderToSingleSampledRenderPassDescriptorValidationTest, ColorAttachmentInvalidUsage) {
    // Create a texture with sample count = 1.
    auto texture =
        CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers,
                      kLevelCount, /*sampleCount=*/1, wgpu::TextureUsage::RenderAttachment);

    auto renderPass = CreateMultisampledRenderToSingleSampledRenderPass(texture.CreateView());
    AssertBeginRenderPassError(&renderPass, testing::HasSubstr("usage"));
}

// When MSAARenderToSingleSampled is enabled for a color attachment, there must be no explicit
// resolve target specified for it.
TEST_F(MSAARenderToSingleSampledRenderPassDescriptorValidationTest, ErrorSettingResolveTarget) {
    // Create a texture with sample count = 1.
    auto textureView1 = CreateCompatibleColorTextureView();
    auto textureView2 = CreateCompatibleColorTextureView();

    auto renderPass = CreateMultisampledRenderToSingleSampledRenderPass(textureView1);
    renderPass.cColorAttachments[0].resolveTarget = textureView2;
    AssertBeginRenderPassError(&renderPass, testing::HasSubstr("as a resolve target"));
}

// Using unsupported implicit sample count in DawnRenderPassColorAttachmentRenderToSingleSampled
// chained struct should result in error.
TEST_F(MSAARenderToSingleSampledRenderPassDescriptorValidationTest, UnsupportedSampleCountError) {
    // Create a texture with sample count = 1.
    auto textureView = CreateCompatibleColorTextureView();

    // Create a render pass with implicit sample count = 3. Which is not supported.
    wgpu::DawnRenderPassColorAttachmentRenderToSingleSampled renderToSingleSampledDesc;
    renderToSingleSampledDesc.implicitSampleCount = 3;
    utils::ComboRenderPassDescriptor renderPass({textureView});
    renderPass.cColorAttachments[0].nextInChain = &renderToSingleSampledDesc;

    AssertBeginRenderPassError(&renderPass,
                               testing::HasSubstr("implicit sample count (3) is not supported"));

    // Create a render pass with implicit sample count = 1. Which is also not supported.
    renderToSingleSampledDesc.implicitSampleCount = 1;
    renderPass.cColorAttachments[0].nextInChain = &renderToSingleSampledDesc;

    AssertBeginRenderPassError(&renderPass,
                               testing::HasSubstr("implicit sample count (1) is not supported"));
}

// When MSAARenderToSingleSampled is enabled in a color attachment, there should be an error if a
// color attachment's format doesn't support resolve. Example, RGBA8Sint format.
TEST_F(MSAARenderToSingleSampledRenderPassDescriptorValidationTest, UnresolvableColorFormatError) {
    // Create a texture with sample count = 1.
    auto texture =
        CreateTexture(device, wgpu::TextureDimension::e2D, wgpu::TextureFormat::RGBA8Sint, kSize,
                      kSize, kArrayLayers, kLevelCount, /*sampleCount=*/1,
                      wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding);

    auto renderPass = CreateMultisampledRenderToSingleSampledRenderPass(texture.CreateView());
    AssertBeginRenderPassError(&renderPass, testing::HasSubstr("does not support resolve"));
}

// Depth stencil attachment's sample count must match the one specified in color attachment's
// implicitSampleCount.
TEST_F(MSAARenderToSingleSampledRenderPassDescriptorValidationTest, DepthStencilSampleCountValid) {
    // Create a color texture with sample count = 1.
    auto colorTextureView = CreateCompatibleColorTextureView();

    // Create depth stencil texture with sample count = 4.
    auto depthStencilTexture = CreateTexture(
        device, wgpu::TextureDimension::e2D, wgpu::TextureFormat::Depth24PlusStencil8, kSize, kSize,
        1, 1, /*sampleCount=*/kSampleCount, wgpu::TextureUsage::RenderAttachment);

    auto renderPass = CreateMultisampledRenderToSingleSampledRenderPass(
        colorTextureView, depthStencilTexture.CreateView());

    AssertBeginRenderPassSuccess(&renderPass);
}

// Using depth stencil attachment with sample count not matching the implicit sample count will
// result in error.
TEST_F(MSAARenderToSingleSampledRenderPassDescriptorValidationTest,
       DepthStencilSampleCountNotMatchImplicitSampleCount) {
    // Create a color texture with sample count = 1.
    auto colorTextureView = CreateCompatibleColorTextureView();

    // Create depth stencil texture with sample count = 1. Which doesn't match implicitSampleCount=4
    // specified in mRenderToSingleSampledDesc.
    auto depthStencilTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, wgpu::TextureFormat::Depth24PlusStencil8,
                      kSize, kSize, 1, 1, /*sampleCount=*/1, wgpu::TextureUsage::RenderAttachment);

    auto renderPass = CreateMultisampledRenderToSingleSampledRenderPass(
        colorTextureView, depthStencilTexture.CreateView());

    AssertBeginRenderPassError(&renderPass, testing::HasSubstr("does not match the sample count"));
}

class DawnLoadResolveTextureValidationTest : public MultisampledRenderPassDescriptorValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::DawnLoadResolveTexture, wgpu::FeatureName::TransientAttachments};
    }

    // Create a view for a resolve texture that can be used with LoadOp::ExpandResolveTexture.
    wgpu::TextureView CreateCompatibleResolveTextureView() {
        wgpu::Texture colorTexture = CreateTexture(
            device, wgpu::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers,
            kLevelCount, /*sampleCount=*/1,
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding);

        return colorTexture.CreateView();
    }
};

// Test that using a valid resolve texture with LoadOp::ExpandResolveTexture doesn't raise
// any error.
TEST_F(DawnLoadResolveTextureValidationTest, ResolveTargetValid) {
    auto multisampledColorTextureView = CreateMultisampledColorTextureView();

    // Create a resolve texture with sample count = 1.
    auto resolveTarget = CreateCompatibleResolveTextureView();

    auto renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].view = multisampledColorTextureView;
    renderPass.cColorAttachments[0].resolveTarget = resolveTarget;
    renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;
    AssertBeginRenderPassSuccess(&renderPass);
}

// Test that LoadOp::ExpandResolveTexture can be used even if the MSAA attachment is transient.
TEST_F(DawnLoadResolveTextureValidationTest, CompatibleWithTransientMSAATexture) {
    auto multisampledColorTexture = CreateTexture(
        device, wgpu::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers, kLevelCount,
        /*sampleCount=*/kSampleCount,
        wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TransientAttachment);
    auto multisampledColorTextureView = multisampledColorTexture.CreateView();

    // Create a resolve texture with sample count = 1.
    auto resolveTarget = CreateCompatibleResolveTextureView();

    auto renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].view = multisampledColorTextureView;
    renderPass.cColorAttachments[0].resolveTarget = resolveTarget;
    renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;
    renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;
    AssertBeginRenderPassSuccess(&renderPass);
}

// When LoadOp::ExpandResolveTexture is used, a resolve texture view must be set.
TEST_F(DawnLoadResolveTextureValidationTest, ResolveTargetMustBeSet) {
    auto multisampledColorTextureView = CreateMultisampledColorTextureView();

    // Error case: texture view is set but resolveTarget is not set.
    auto renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].view = multisampledColorTextureView;
    renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;
    AssertBeginRenderPassError(&renderPass, testing::HasSubstr("resolve target"));
}

// When LoadOp::ExpandResolveTexture is used, the attached texture view must be multisampled.
TEST_F(DawnLoadResolveTextureValidationTest, ResolveTargetInvalidSampleCount) {
    // Create a texture with sample count = 1.
    auto colorTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers,
                      kLevelCount, /*sampleCount=*/1, wgpu::TextureUsage::RenderAttachment);

    // Create a resolve texture with sample count = 1.
    auto resolveTarget = CreateCompatibleResolveTextureView();

    auto renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].view = colorTexture.CreateView();
    renderPass.cColorAttachments[0].resolveTarget = resolveTarget;
    renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;
    AssertBeginRenderPassError(&renderPass, testing::HasSubstr("sample count"));
}

// When LoadOp::ExpandResolveTexture is used, the resolve texture must be created with
// TextureBinding usage.
TEST_F(DawnLoadResolveTextureValidationTest, ResolveTargetInvalidUsage) {
    auto multisampledColorTextureView = CreateMultisampledColorTextureView();

    // Create a texture with sample count = 1.
    auto resolveTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers,
                      kLevelCount, /*sampleCount=*/1, wgpu::TextureUsage::RenderAttachment);

    auto renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].view = multisampledColorTextureView;
    renderPass.cColorAttachments[0].resolveTarget = resolveTexture.CreateView();
    renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;
    AssertBeginRenderPassError(&renderPass, testing::HasSubstr("usage"));
}

// When LoadOp::ExpandResolveTexture is enabled in a color attachment, there should be an error if a
// color attachment's format doesn't support resolve. Example, RGBA8Sint format.
TEST_F(DawnLoadResolveTextureValidationTest, UnresolvableColorFormatError) {
    auto multisampledTexture = CreateTexture(
        device, wgpu::TextureDimension::e2D, wgpu::TextureFormat::RGBA8Sint, kSize, kSize,
        kArrayLayers, kLevelCount, /*sampleCount=*/4, wgpu::TextureUsage::RenderAttachment);

    // Create a texture with sample count = 1.
    auto resolveTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, wgpu::TextureFormat::RGBA8Sint, kSize,
                      kSize, kArrayLayers, kLevelCount, /*sampleCount=*/1,
                      wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding);

    auto renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].view = multisampledTexture.CreateView();
    renderPass.cColorAttachments[0].resolveTarget = resolveTexture.CreateView();
    renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;
    AssertBeginRenderPassError(&renderPass,
                               testing::HasSubstr("does not support being used as resolve target"));
}

// The LoadOp is NOT currently supported on depth/stencil attachment.
TEST_F(DawnLoadResolveTextureValidationTest, OnlyLoadingColorAttachmentIsSupported) {
    auto multisampledColorTextureView = CreateMultisampledColorTextureView();
    auto resolveTarget = CreateCompatibleResolveTextureView();

    // Error case: Use ExpandResolveTexture on depth/stencil attachment.
    {
        // Create depth stencil texture with sample count = 4.
        auto depthStencilTexture = CreateTexture(
            device, wgpu::TextureDimension::e2D, wgpu::TextureFormat::Depth24PlusStencil8, kSize,
            kSize, 1, 1, /*sampleCount=*/kSampleCount, wgpu::TextureUsage::RenderAttachment);

        auto renderPass = utils::ComboRenderPassDescriptor({multisampledColorTextureView},
                                                           depthStencilTexture.CreateView());
        renderPass.cColorAttachments[0].resolveTarget = resolveTarget;
        renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;

        renderPass.cDepthStencilAttachmentInfo.depthLoadOp =
            renderPass.cDepthStencilAttachmentInfo.stencilLoadOp =
                wgpu::LoadOp::ExpandResolveTexture;

        AssertBeginRenderPassError(&renderPass,
                                   testing::HasSubstr("not supported on depth/stencil attachment"));
    }
}

// Creating a render pass with ExpandResolveRect and no DawnPartialLoadResolveTexture feature
// enabled should result in error.
TEST_F(DawnLoadResolveTextureValidationTest, ExpandResolveRectWithoutFeatureEnabled) {
    auto multisampledColorTextureView = CreateMultisampledColorTextureView();
    auto resolveTarget = CreateCompatibleResolveTextureView();

    wgpu::RenderPassDescriptorExpandResolveRect rect{};
    rect.x = rect.y = 0;
    rect.width = rect.height = 1;

    auto renderPass = CreateMultisampledRenderPass();
    renderPass.nextInChain = &rect;
    renderPass.cColorAttachments[0].view = multisampledColorTextureView;
    renderPass.cColorAttachments[0].resolveTarget = resolveTarget;
    renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;

    AssertBeginRenderPassError(&renderPass);
}

class DawnPartialLoadResolveTextureValidationTest : public DawnLoadResolveTextureValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        auto features = DawnLoadResolveTextureValidationTest::GetRequiredFeatures();
        features.push_back(wgpu::FeatureName::DawnPartialLoadResolveTexture);
        return features;
    }
};

// Test that using a valid ExpandResolveRect with LoadOp::ExpandResolveTexture doesn't raise
// any error.
TEST_F(DawnPartialLoadResolveTextureValidationTest, ExpandResolveRectValid) {
    auto multisampledTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers,
                      kLevelCount, /*sampleCount=*/4, wgpu::TextureUsage::RenderAttachment);

    // Create a resolve texture with sample count = 1.
    auto resolveTarget = CreateCompatibleResolveTextureView();

    wgpu::RenderPassDescriptorExpandResolveRect rect{};
    rect.x = rect.y = 0;
    rect.width = rect.height = 1;

    auto renderPass = CreateMultisampledRenderPass();
    renderPass.nextInChain = &rect;
    renderPass.cColorAttachments[0].view = multisampledTexture.CreateView();
    renderPass.cColorAttachments[0].resolveTarget = resolveTarget;
    renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;
    AssertBeginRenderPassSuccess(&renderPass);
}

// Test that using a valid ExpandResolveRect with LoadOp::ExpandResolveTexture, jointed with another
// attachment without LoadOp::ExpandResolveTexture doesn't raise any error.
TEST_F(DawnPartialLoadResolveTextureValidationTest, ExpandResolveRectMixedLoadOpsValid) {
    auto multisampledTextureView1 =
        CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers,
                      kLevelCount, /*sampleCount=*/4, wgpu::TextureUsage::RenderAttachment)
            .CreateView();
    auto multisampledTextureView2 =
        CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers,
                      kLevelCount, /*sampleCount=*/4, wgpu::TextureUsage::RenderAttachment)
            .CreateView();

    // Create a resolve texture with sample count = 1.
    auto resolveTarget1 = CreateCompatibleResolveTextureView();
    auto resolveTarget2 = CreateCompatibleResolveTextureView();

    wgpu::RenderPassDescriptorExpandResolveRect rect{};
    rect.x = rect.y = 0;
    rect.width = rect.height = 1;

    utils::ComboRenderPassDescriptor renderPass(
        {multisampledTextureView1, multisampledTextureView2});

    renderPass.nextInChain = &rect;
    renderPass.cColorAttachments[0].resolveTarget = resolveTarget1;
    renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;
    renderPass.cColorAttachments[1].resolveTarget = resolveTarget2;
    renderPass.cColorAttachments[1].loadOp = wgpu::LoadOp::Load;
    AssertBeginRenderPassSuccess(&renderPass);
}

// The area of ExpandResolveRect must be within the texture size.
TEST_F(DawnPartialLoadResolveTextureValidationTest, ExpandResolveRectInvalidSize) {
    auto multisampledTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers,
                      kLevelCount, /*sampleCount=*/4, wgpu::TextureUsage::RenderAttachment);

    // Create a resolve texture with sample count = 1.
    auto resolveTarget = CreateCompatibleResolveTextureView();

    wgpu::RenderPassDescriptorExpandResolveRect rect{};
    rect.x = rect.y = 0;
    rect.width = rect.height = kSize;

    auto renderPass = CreateMultisampledRenderPass();
    renderPass.nextInChain = &rect;
    renderPass.cColorAttachments[0].view = multisampledTexture.CreateView();
    renderPass.cColorAttachments[0].resolveTarget = resolveTarget;
    renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;
    AssertBeginRenderPassSuccess(&renderPass);

    rect.width = kSize + 1;
    AssertBeginRenderPassError(&renderPass);

    rect.height = kSize + 1;
    rect.width = 1;
    AssertBeginRenderPassError(&renderPass);
}

// ExpandResolveRect must be used with wgpu::LoadOp::ExpandResolveTexture.
TEST_F(DawnPartialLoadResolveTextureValidationTest, ExpandResolveRectInvalidLoadOp) {
    auto multisampledTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers,
                      kLevelCount, /*sampleCount=*/4, wgpu::TextureUsage::RenderAttachment);

    // Create a resolve texture with sample count = 1.
    auto resolveTarget = CreateCompatibleResolveTextureView();

    wgpu::RenderPassDescriptorExpandResolveRect rect{};
    rect.x = rect.y = 0;
    rect.width = rect.height = 1;

    auto renderPass = CreateMultisampledRenderPass();
    renderPass.nextInChain = &rect;
    renderPass.cColorAttachments[0].view = multisampledTexture.CreateView();
    renderPass.cColorAttachments[0].resolveTarget = resolveTarget;
    renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Load;

    AssertBeginRenderPassError(&renderPass);
}

}  // anonymous namespace
}  // namespace dawn
