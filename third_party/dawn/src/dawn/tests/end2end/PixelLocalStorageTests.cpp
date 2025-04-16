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

#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class PixelLocalStorageTests : public DawnTest {
  protected:
    wgpu::Limits GetRequiredLimits(const wgpu::Limits& supported) override {
        wgpu::Limits required = {};
        required.maxStorageBuffersInFragmentStage = supported.maxStorageBuffersInFragmentStage;
        required.maxStorageBuffersPerShaderStage = supported.maxStorageBuffersPerShaderStage;
        return required;
    }

    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(
            !device.HasFeature(wgpu::FeatureName::PixelLocalStorageCoherent) &&
            !device.HasFeature(wgpu::FeatureName::PixelLocalStorageNonCoherent));

        DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageBuffersInFragmentStage < 1);

        supportsCoherent = device.HasFeature(wgpu::FeatureName::PixelLocalStorageCoherent);
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::PixelLocalStorageCoherent})) {
            requiredFeatures.push_back(wgpu::FeatureName::PixelLocalStorageCoherent);
            supportsCoherent = true;
        }
        if (SupportsFeatures({wgpu::FeatureName::PixelLocalStorageNonCoherent})) {
            requiredFeatures.push_back(wgpu::FeatureName::PixelLocalStorageNonCoherent);
        }
        return requiredFeatures;
    }

    struct StorageSpec {
        uint64_t offset;
        wgpu::TextureFormat format;
        wgpu::LoadOp loadOp = wgpu::LoadOp::Clear;
        wgpu::StoreOp storeOp = wgpu::StoreOp::Store;
        wgpu::Color clearValue = {0, 0, 0, 0};
        bool discardAfterInit = false;
    };

    enum class CheckMethod {
        StorageBuffer,
        ReadStorageAttachments,
        RenderAttachment,
    };

    struct PLSSpec {
        uint64_t totalSize;
        std::vector<StorageSpec> attachments;
        CheckMethod checkMethod = CheckMethod::ReadStorageAttachments;
    };

    // Builds a shader module with multiple entry points used for testing PLS.
    //
    //  - A trivial vertex entrypoint to render a point.
    //  - Various fragment entrypoints using a pixel_local block matching the `spec`.
    //    - An accumulator entrypoint adding (slot + 1) to each pls slot so we can check that
    //      access to the PLS is correctly synchronized.
    //    - An entrypoint copying the PLS data to a storage buffer for readback.
    //    - An entrypoint copying the PLS data to a render attachment for readback.
    wgpu::ShaderModule MakeTestModule(const PLSSpec& spec) const {
        std::vector<const char*> plsTypes;
        plsTypes.resize(spec.totalSize / kPLSSlotByteSize, "u32");
        for (const auto& attachment : spec.attachments) {
            switch (attachment.format) {
                case wgpu::TextureFormat::R32Uint:
                    plsTypes[attachment.offset / kPLSSlotByteSize] = "u32";
                    break;
                case wgpu::TextureFormat::R32Sint:
                    plsTypes[attachment.offset / kPLSSlotByteSize] = "i32";
                    break;
                case wgpu::TextureFormat::R32Float:
                    plsTypes[attachment.offset / kPLSSlotByteSize] = "f32";
                    break;
                default:
                    DAWN_UNREACHABLE();
            }
        }

        std::ostringstream o;
        o << R"(
            enable chromium_experimental_pixel_local;

            @vertex fn vs() -> @builtin(position) vec4f {
                return vec4f(0, 0, 0, 0.5);
            }

        )";
        o << "struct PLS {\n";
        for (size_t i = 0; i < plsTypes.size(); i++) {
            // e.g.: a0 : u32,
            o << "  a" << i << " : " << plsTypes[i] << ",\n";
        }
        o << "}\n";
        o << "var<pixel_local> pls : PLS;\n";
        o << "@fragment fn accumulator() {\n";
        for (size_t i = 0; i < plsTypes.size(); i++) {
            // e.g.: pls.a0 = pls.a0 + 1;
            o << "    pls.a" << i << " = pls.a" << i << " + " << (i + 1) << ";\n";
        }
        o << "}\n";
        o << "\n";
        o << "@group(0) @binding(0) var<storage, read_write> readbackStorageBuffer : array<u32>;\n";
        o << "@fragment fn readbackToStorageBuffer() {\n";
        for (size_t i = 0; i < plsTypes.size(); i++) {
            // e.g.: readbackStorageBuffer[0] = u32(pls.a0);
            o << "    readbackStorageBuffer[" << i << "] = u32(pls.a" << i << ");\n";
        }
        o << "}\n";
        o << "\n";
        o << "@fragment fn copyToColorAttachment() -> @location(0) vec4f {\n";
        o << "    var result : vec4f;\n";
        for (size_t i = 0; i < plsTypes.size(); i++) {
            // e.g.: result[0] = f32(pls.a0) / 255.0;
            o << "    result[" << i << "] = f32(pls.a" << i << ") / 255.0;\n";
        }
        o << "    return result;";
        o << "}\n";

        return utils::CreateShaderModule(device, o.str().c_str());
    }

    wgpu::PipelineLayout MakeTestLayout(const PLSSpec& spec, wgpu::BindGroupLayout bgl = {}) const {
        std::vector<wgpu::PipelineLayoutStorageAttachment> storageAttachments;
        for (const auto& attachmentSpec : spec.attachments) {
            wgpu::PipelineLayoutStorageAttachment attachment;
            attachment.format = attachmentSpec.format;
            attachment.offset = attachmentSpec.offset;
            storageAttachments.push_back(attachment);
        }

        wgpu::PipelineLayoutPixelLocalStorage pls;
        pls.totalPixelLocalStorageSize = spec.totalSize;
        pls.storageAttachmentCount = storageAttachments.size();
        pls.storageAttachments = storageAttachments.data();

        wgpu::PipelineLayoutDescriptor plDesc;
        plDesc.nextInChain = &pls;
        plDesc.bindGroupLayoutCount = 0;
        if (bgl != nullptr) {
            plDesc.bindGroupLayoutCount = 1;
            plDesc.bindGroupLayouts = &bgl;
        }

        return device.CreatePipelineLayout(&plDesc);
    }

    std::vector<wgpu::Texture> MakeTestStorageAttachments(const PLSSpec& spec) const {
        std::vector<wgpu::Texture> attachments;
        for (size_t i = 0; i < spec.attachments.size(); i++) {
            const StorageSpec& attachmentSpec = spec.attachments[i];

            wgpu::TextureDescriptor desc;
            desc.format = attachmentSpec.format;
            desc.size = {1, 1};
            desc.usage = wgpu::TextureUsage::StorageAttachment | wgpu::TextureUsage::CopySrc |
                         wgpu::TextureUsage::CopyDst;
            if (attachmentSpec.discardAfterInit) {
                desc.usage |= wgpu::TextureUsage::RenderAttachment;
            }

            wgpu::Texture attachment = device.CreateTexture(&desc);

            // Initialize the attachment with 1s if LoadOp is Load, copying from another texture
            // so that we avoid adding the extra RenderAttachment usage to the storage attachment.
            if (attachmentSpec.loadOp == wgpu::LoadOp::Load) {
                desc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
                wgpu::Texture clearedTexture = device.CreateTexture(&desc);

                wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

                // The pass that clears clearedTexture.
                utils::ComboRenderPassDescriptor rpDesc({clearedTexture.CreateView()});
                rpDesc.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
                rpDesc.cColorAttachments[0].clearValue = attachmentSpec.clearValue;
                wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rpDesc);
                pass.End();

                // Copy clearedTexture -> attachment.
                wgpu::TexelCopyTextureInfo src = utils::CreateTexelCopyTextureInfo(clearedTexture);
                wgpu::TexelCopyTextureInfo dst = utils::CreateTexelCopyTextureInfo(attachment);
                wgpu::Extent3D copySize = {1, 1, 1};
                encoder.CopyTextureToTexture(&src, &dst, &copySize);

                wgpu::CommandBuffer commands = encoder.Finish();
                queue.Submit(1, &commands);
            }

            // Discard after initialization to check that the lazy zero init is actually triggered
            // (and it's not just that the resource happened to be zeroes already).
            if (attachmentSpec.discardAfterInit) {
                utils::ComboRenderPassDescriptor rpDesc({attachment.CreateView()});
                rpDesc.cColorAttachments[0].loadOp = wgpu::LoadOp::Load;
                rpDesc.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;

                wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
                wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rpDesc);
                pass.End();
                wgpu::CommandBuffer commands = encoder.Finish();
                queue.Submit(1, &commands);
            }

            attachments.push_back(attachment);
        }

        return attachments;
    }

    wgpu::RenderPassEncoder BeginTestRenderPass(
        const PLSSpec& spec,
        const wgpu::CommandEncoder& encoder,
        const std::vector<wgpu::Texture>& storageAttachments,
        wgpu::Texture colorAttachment) const {
        std::vector<wgpu::RenderPassStorageAttachment> attachmentDescs;
        for (size_t i = 0; i < spec.attachments.size(); i++) {
            const StorageSpec& attachmentSpec = spec.attachments[i];

            wgpu::RenderPassStorageAttachment attachment;
            attachment.storage = storageAttachments[i].CreateView();
            attachment.offset = attachmentSpec.offset;
            attachment.loadOp = attachmentSpec.loadOp;
            attachment.storeOp = attachmentSpec.storeOp;
            attachment.clearValue = attachmentSpec.clearValue;
            attachmentDescs.push_back(attachment);
        }

        wgpu::RenderPassPixelLocalStorage rpPlsDesc;
        rpPlsDesc.totalPixelLocalStorageSize = spec.totalSize;
        rpPlsDesc.storageAttachmentCount = attachmentDescs.size();
        rpPlsDesc.storageAttachments = attachmentDescs.data();

        wgpu::RenderPassDescriptor rpDesc;
        rpDesc.nextInChain = &rpPlsDesc;
        rpDesc.colorAttachmentCount = 0;
        rpDesc.depthStencilAttachment = nullptr;

        wgpu::RenderPassColorAttachment rpColor;
        if (colorAttachment != nullptr) {
            rpColor.view = colorAttachment.CreateView();
            rpColor.loadOp = wgpu::LoadOp::Clear;
            rpColor.clearValue = {0, 0, 0, 0};
            rpColor.storeOp = wgpu::StoreOp::Store;

            rpDesc.colorAttachments = &rpColor;
            rpDesc.colorAttachmentCount = 1;
        }

        return encoder.BeginRenderPass(&rpDesc);
    }

    uint32_t ComputeExpectedValue(const PLSSpec& spec, size_t slot) {
        for (const StorageSpec& attachment : spec.attachments) {
            if (attachment.offset / kPLSSlotByteSize != slot) {
                continue;
            }

            // Compute the expected value depending on load/store ops by "replaying" the operations
            // that would be done.
            int32_t expectedValue = 0;
            if (!attachment.discardAfterInit) {
                expectedValue = attachment.clearValue.r;
            }
            expectedValue += (slot + 1) * kIterations;

            if (attachment.storeOp == wgpu::StoreOp::Discard) {
                expectedValue = 0;
            }

            return expectedValue;
        }

        // This is not an explicit storage attachment.
        return (slot + 1) * kIterations;
    }

    void CheckByReadingStorageAttachments(const PLSSpec& spec,
                                          const std::vector<wgpu::Texture>& storageAttachments) {
        for (size_t i = 0; i < spec.attachments.size(); i++) {
            const StorageSpec& attachmentSpec = spec.attachments[i];
            uint32_t slot = attachmentSpec.offset / kPLSSlotByteSize;

            uint32_t expectedValue = ComputeExpectedValue(spec, slot);

            switch (spec.attachments[i].format) {
                case wgpu::TextureFormat::R32Float:
                    EXPECT_TEXTURE_EQ(static_cast<float>(expectedValue), storageAttachments[i],
                                      {0, 0});
                    break;
                case wgpu::TextureFormat::R32Uint:
                case wgpu::TextureFormat::R32Sint:
                    EXPECT_TEXTURE_EQ(expectedValue, storageAttachments[i], {0, 0});
                    break;
                default:
                    DAWN_UNREACHABLE();
            }
        }
    }

    void CheckByReadingColorAttachment(const PLSSpec& spec, wgpu::Texture color) {
        std::array<uint32_t, 4> expected = {0, 0, 0, 0};
        for (size_t slot = 0; slot < spec.totalSize / kPLSSlotByteSize; slot++) {
            expected[slot] = ComputeExpectedValue(spec, slot);
        }

        utils::RGBA8 expectedColor(expected[0], expected[1], expected[2], expected[3]);
        EXPECT_TEXTURE_EQ(expectedColor, color, {0, 0});
    }

    void CheckByReadingStorageBuffer(const PLSSpec& spec, wgpu::Buffer buffer) {
        for (size_t slot = 0; slot < spec.totalSize / kPLSSlotByteSize; slot++) {
            uint32_t expectedValue = ComputeExpectedValue(spec, slot);
            EXPECT_BUFFER_U32_EQ(expectedValue, buffer, slot * kPLSSlotByteSize);
        }
    }

    bool RequiresColorAttachment(const PLSSpec& spec) {
        return spec.attachments.empty() || spec.checkMethod == CheckMethod::RenderAttachment;
    }

    void SetColorTargets(const PLSSpec& spec,
                         utils::ComboRenderPipelineDescriptor* desc,
                         bool writesColor) {
        if (RequiresColorAttachment(spec)) {
            desc->cFragment.targetCount = 1;
            desc->cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
            desc->cTargets[0].writeMask =
                writesColor ? wgpu::ColorWriteMask::All : wgpu::ColorWriteMask::None;
        } else {
            desc->cFragment.targetCount = 0;
        }
    }

    void DoTest(const PLSSpec& spec) {
        wgpu::ShaderModule module = MakeTestModule(spec);

        // Make the pipeline that will draw a point that adds i to the i-th slot of the PLS.
        wgpu::RenderPipeline accumulatorPipeline;
        {
            utils::ComboRenderPipelineDescriptor desc;
            desc.layout = MakeTestLayout(spec);
            desc.vertex.module = module;
            desc.cFragment.module = module;
            desc.cFragment.entryPoint = "accumulator";
            desc.primitive.topology = wgpu::PrimitiveTopology::PointList;
            SetColorTargets(spec, &desc, false);
            accumulatorPipeline = device.CreateRenderPipeline(&desc);
        }

        wgpu::RenderPipeline checkPipeline;
        wgpu::BindGroup checkBindGroup;
        wgpu::Buffer readbackStorageBuffer;

        if (spec.checkMethod == CheckMethod::StorageBuffer) {
            // Make the pipeline copying the PLS to the storage buffer.
            wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
                device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});

            utils::ComboRenderPipelineDescriptor desc;
            desc.layout = MakeTestLayout(spec, bgl);
            desc.vertex.module = module;
            desc.cFragment.module = module;
            desc.cFragment.entryPoint = "readbackToStorageBuffer";
            desc.primitive.topology = wgpu::PrimitiveTopology::PointList;
            SetColorTargets(spec, &desc, false);
            checkPipeline = device.CreateRenderPipeline(&desc);

            wgpu::BufferDescriptor bufDesc;
            bufDesc.size = spec.totalSize;
            bufDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
            readbackStorageBuffer = device.CreateBuffer(&bufDesc);

            checkBindGroup = utils::MakeBindGroup(device, bgl, {{0, readbackStorageBuffer}});
        }
        if (spec.checkMethod == CheckMethod::RenderAttachment) {
            // Make the pipeline copying the PLS to the render attachment.
            utils::ComboRenderPipelineDescriptor desc;
            desc.layout = MakeTestLayout(spec);
            desc.vertex.module = module;
            desc.cFragment.module = module;
            desc.cFragment.entryPoint = "copyToColorAttachment";
            desc.primitive.topology = wgpu::PrimitiveTopology::PointList;
            SetColorTargets(spec, &desc, true);
            checkPipeline = device.CreateRenderPipeline(&desc);
        }

        // Make all the attachments.
        std::vector<wgpu::Texture> storageAttachments = MakeTestStorageAttachments(spec);

        wgpu::Texture colorAttachment;
        if (RequiresColorAttachment(spec)) {
            wgpu::TextureDescriptor desc;
            desc.size = {1, 1};
            desc.format = wgpu::TextureFormat::RGBA8Unorm;
            desc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
            colorAttachment = device.CreateTexture(&desc);
        }

        {
            // Build the render pass with the specified storage attachments
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass =
                BeginTestRenderPass(spec, encoder, storageAttachments, colorAttachment);

            // Draw the points accumulating to PLS, with a PLS barrier if needed.
            pass.SetPipeline(accumulatorPipeline);
            if (supportsCoherent) {
                pass.Draw(kIterations);
            } else {
                for (uint32_t i = 0; i < kIterations; i++) {
                    pass.Draw(1);
                    pass.PixelLocalStorageBarrier();
                }
            }

            // Run the checkPipeline, if any.
            if (checkPipeline != nullptr) {
                pass.SetPipeline(checkPipeline);
                if (checkBindGroup != nullptr) {
                    pass.SetBindGroup(0, checkBindGroup);
                }
                pass.Draw(1);
            }

            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);
        }

        switch (spec.checkMethod) {
            case CheckMethod::StorageBuffer:
                CheckByReadingStorageBuffer(spec, readbackStorageBuffer);
                break;
            case CheckMethod::ReadStorageAttachments:
                CheckByReadingStorageAttachments(spec, storageAttachments);
                break;
            case CheckMethod::RenderAttachment:
                CheckByReadingColorAttachment(spec, colorAttachment);
                break;
        }

        // Youpi!
    }

    static constexpr uint32_t kIterations = 10;
    bool supportsCoherent;
};

// Test that the various supported PLS format work for accumulation.
TEST_P(PixelLocalStorageTests, Formats) {
    for (const auto format : {wgpu::TextureFormat::R32Uint, wgpu::TextureFormat::R32Sint,
                              wgpu::TextureFormat::R32Float}) {
        PLSSpec spec = {4, {{0, format}}};
        DoTest(spec);
    }
}

// Tests the storage attachment load ops
TEST_P(PixelLocalStorageTests, LoadOp) {
    // Test LoadOp::Clear with a couple values.
    {
        PLSSpec spec = {4, {{0, wgpu::TextureFormat::R32Uint}}};
        spec.attachments[0].loadOp = wgpu::LoadOp::Clear;

        spec.attachments[0].clearValue.r = 42;
        DoTest(spec);

        spec.attachments[0].clearValue.r = 38;
        DoTest(spec);
    }

    {
        PLSSpec spec = {4, {{0, wgpu::TextureFormat::R32Sint}}};
        spec.attachments[0].loadOp = wgpu::LoadOp::Clear;

        spec.attachments[0].clearValue.r = 42;
        DoTest(spec);

        spec.attachments[0].clearValue.r = -38;
        DoTest(spec);
    }

    {
        PLSSpec spec = {4, {{0, wgpu::TextureFormat::R32Float}}};
        spec.attachments[0].loadOp = wgpu::LoadOp::Clear;

        spec.attachments[0].clearValue.r = 4.0;
        DoTest(spec);

        spec.attachments[0].clearValue.r = -3.0;
        DoTest(spec);
    }

    // Test LoadOp::Load (the test helper clears the texture to clearValue).
    {
        PLSSpec spec = {4, {{0, wgpu::TextureFormat::R32Uint}}};
        spec.attachments[0].clearValue.r = 18;
        spec.attachments[0].loadOp = wgpu::LoadOp::Load;
        DoTest(spec);
    }
}

// Tests the storage attachment store ops
TEST_P(PixelLocalStorageTests, StoreOp) {
    // Test StoreOp::Store.
    {
        PLSSpec spec = {4, {{0, wgpu::TextureFormat::R32Uint}}};
        spec.attachments[0].storeOp = wgpu::StoreOp::Store;
        DoTest(spec);
    }

    // Test StoreOp::Discard.
    {
        PLSSpec spec = {4, {{0, wgpu::TextureFormat::R32Uint}}};
        spec.attachments[0].storeOp = wgpu::StoreOp::Discard;
        DoTest(spec);
    }
}

// Test lazy zero initialization of the storage attachments.
TEST_P(PixelLocalStorageTests, ZeroInit) {
    // Discard causes the storage attachment to be lazy zeroed.
    {
        PLSSpec spec = {4, {{0, wgpu::TextureFormat::R32Uint}}};
        spec.attachments[0].storeOp = wgpu::StoreOp::Discard;
        DoTest(spec);
    }

    // Discard before using as a storage attachment, it should be lazy-cleared.
    {
        PLSSpec spec = {4, {{0, wgpu::TextureFormat::R32Uint}}};
        spec.attachments[0].loadOp = wgpu::LoadOp::Load;
        spec.attachments[0].clearValue.r = 18;
        spec.attachments[0].discardAfterInit = true;
        DoTest(spec);
    }
}

// Test many explicit storage attachments.
TEST_P(PixelLocalStorageTests, MultipleStorageAttachments) {
    PLSSpec spec = {16,
                    {
                        {0, wgpu::TextureFormat::R32Sint},
                        {4, wgpu::TextureFormat::R32Uint},
                        {8, wgpu::TextureFormat::R32Float},
                        {12, wgpu::TextureFormat::R32Sint},
                    }};
    DoTest(spec);
}

// Test explicit storage attachments in inverse offset order
TEST_P(PixelLocalStorageTests, InvertedOffsetOrder) {
    PLSSpec spec = {8,
                    {
                        {4, wgpu::TextureFormat::R32Uint},
                        {0, wgpu::TextureFormat::R32Sint},
                    }};
    DoTest(spec);
}

// Test implicit pixel local slot.
TEST_P(PixelLocalStorageTests, ImplicitSlot) {
    PLSSpec spec = {4, {}, CheckMethod::StorageBuffer};
    DoTest(spec);
}

// Test multiple implicit pixel local slot.
TEST_P(PixelLocalStorageTests, MultipleImplicitSlot) {
    PLSSpec spec = {16, {}, CheckMethod::StorageBuffer};
    DoTest(spec);
}

// Test mixed implicit / explicit pixel local slot.
TEST_P(PixelLocalStorageTests, MixedImplicitExplicit) {
    {
        PLSSpec spec = {16,
                        {{4, wgpu::TextureFormat::R32Uint}, {8, wgpu::TextureFormat::R32Float}},
                        CheckMethod::StorageBuffer};
        DoTest(spec);
    }
    {
        PLSSpec spec = {16,
                        {{4, wgpu::TextureFormat::R32Uint}, {12, wgpu::TextureFormat::R32Float}},
                        CheckMethod::StorageBuffer};
        DoTest(spec);
    }
    {
        PLSSpec spec = {16,
                        {{0, wgpu::TextureFormat::R32Uint}, {12, wgpu::TextureFormat::R32Float}},
                        CheckMethod::StorageBuffer};
        DoTest(spec);
    }
}

// Test using PLS and then copying it to a render attachment, fully implicit version.
TEST_P(PixelLocalStorageTests, CopyToRenderAttachment) {
    {
        PLSSpec spec = {4, {}, CheckMethod::RenderAttachment};
        DoTest(spec);
    }
    {
        PLSSpec spec = {16, {}, CheckMethod::RenderAttachment};
        DoTest(spec);
    }
}

// Test using PLS and then copying it to a render attachment, fully implicit version.
TEST_P(PixelLocalStorageTests, CopyToRenderAttachmentWithStorageAttachments) {
    {
        PLSSpec spec = {4, {{0, wgpu::TextureFormat::R32Float}}, CheckMethod::RenderAttachment};
        DoTest(spec);
    }
    {
        PLSSpec spec = {16, {{8, wgpu::TextureFormat::R32Uint}}, CheckMethod::RenderAttachment};
        DoTest(spec);
    }
}

// Test using PLS in multiple render passes with different sizes in one command buffer, fully
// implicit version.
TEST_P(PixelLocalStorageTests, ImplicitPLSAndLargerRenderAttachmentsInOneCommandBuffer) {
    // Prepare the shader modules with one fragment shader entry point that increases the pixel
    // local storage variables and another fragment shader entry point that computes the output to
    // the color attachment.
    wgpu::ShaderModule wgslModule = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_pixel_local;
            @vertex
            fn vsMain(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array(
                    vec2f(-1.0, -1.0),
                    vec2f(-1.0,  1.0),
                    vec2f( 1.0, -1.0),
                    vec2f( 1.0,  1.0),
                    vec2f(-1.0,  1.0),
                    vec2f( 1.0, -1.0));
                return vec4f(pos[VertexIndex % 6], 0.5, 1.0);
            }

            struct PLS {
                a : u32,
                b : u32,
            };
            var<pixel_local> pls : PLS;
            @fragment fn accumulate() -> @location(0) vec4u {
                pls.a = pls.a + 1;
                pls.b = pls.b + 2;
                return vec4u(0, 0, 0, 1u);
            }

            @fragment fn copyToColorAttachment() -> @location(0) vec4u {
                return vec4u(pls.a + pls.b, 0, 0, 1u);
            })");
    constexpr uint32_t kPixelLocalStorageSize = kPLSSlotByteSize * 2;

    // Create render pipelines in the test
    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    {
        pipelineDescriptor.vertex.module = wgslModule;
        pipelineDescriptor.cFragment.module = wgslModule;
        pipelineDescriptor.cFragment.entryPoint = "accumulate";
        pipelineDescriptor.cFragment.targetCount = 1;
        pipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::R32Uint;
        pipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;

        // Create pipeline layout with pixel local storage
        wgpu::PipelineLayoutPixelLocalStorage pixelLocalStoragePipelineLayout;
        pixelLocalStoragePipelineLayout.totalPixelLocalStorageSize = kPixelLocalStorageSize;
        pixelLocalStoragePipelineLayout.storageAttachmentCount = 0;
        wgpu::PipelineLayoutDescriptor pipelineLayoutDesciptor;
        pipelineLayoutDesciptor.nextInChain = &pixelLocalStoragePipelineLayout;
        pipelineLayoutDesciptor.bindGroupLayoutCount = 0;
        pipelineDescriptor.layout = device.CreatePipelineLayout(&pipelineLayoutDesciptor);
    }
    // Create the render pipeline to update pixel local storage values
    wgpu::RenderPipeline accumulatePipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    // Create the render pipeline to compute the final output with the current pixel local storage
    // values
    pipelineDescriptor.cFragment.entryPoint = "copyToColorAttachment";
    wgpu::RenderPipeline copyToColorAttachmentPipeline =
        device.CreateRenderPipeline(&pipelineDescriptor);

    // Create two color attachments with different sizes
    wgpu::Texture color1;
    wgpu::Texture color2;
    constexpr wgpu::Extent3D kColorSize1 = {1, 1, 1};
    constexpr wgpu::Extent3D kColorSize2 = {2, 2, 1};
    {
        wgpu::TextureDescriptor colorDescriptor;
        colorDescriptor.size = kColorSize1;
        colorDescriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
        colorDescriptor.format = wgpu::TextureFormat::R32Uint;
        color1 = device.CreateTexture(&colorDescriptor);
        colorDescriptor.size = kColorSize2;
        color2 = device.CreateTexture(&colorDescriptor);
    }

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    // Use first color attachment with smaller size
    wgpu::RenderPassDescriptor renderPassDescriptor;
    wgpu::RenderPassPixelLocalStorage renderPassPixelLocalStorageDescriptor;
    wgpu::RenderPassColorAttachment renderPassColor;
    {
        renderPassPixelLocalStorageDescriptor.totalPixelLocalStorageSize = kPixelLocalStorageSize;
        renderPassPixelLocalStorageDescriptor.storageAttachmentCount = 0;
        renderPassDescriptor.nextInChain = &renderPassPixelLocalStorageDescriptor;
        renderPassColor.view = color1.CreateView();
        renderPassColor.loadOp = wgpu::LoadOp::Clear;
        renderPassColor.clearValue = {0, 0, 0, 0};
        renderPassColor.storeOp = wgpu::StoreOp::Store;
        renderPassDescriptor.colorAttachmentCount = 1;
        renderPassDescriptor.colorAttachments = &renderPassColor;
    }
    wgpu::RenderPassEncoder pass1 = encoder.BeginRenderPass(&renderPassDescriptor);
    pass1.SetPipeline(accumulatePipeline);
    if (supportsCoherent) {
        pass1.Draw(kIterations * 6);
    } else {
        for (uint32_t i = 0; i < kIterations; i++) {
            pass1.Draw(6);
            pass1.PixelLocalStorageBarrier();
        }
    }
    pass1.SetPipeline(copyToColorAttachmentPipeline);
    pass1.Draw(6);
    pass1.End();

    // Use second color attachment with larger size
    renderPassColor.view = color2.CreateView();
    wgpu::RenderPassEncoder pass2 = encoder.BeginRenderPass(&renderPassDescriptor);
    pass2.SetPipeline(accumulatePipeline);
    if (supportsCoherent) {
        pass2.Draw(kIterations * 6 * 2);
    } else {
        for (uint32_t i = 0; i < kIterations * 2; i++) {
            pass2.Draw(6);
            pass2.PixelLocalStorageBarrier();
        }
    }

    pass2.SetPipeline(copyToColorAttachmentPipeline);
    pass2.Draw(6);
    pass2.End();

    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    // Check the data in color1 and color2
    constexpr uint32_t kExpectedResult1 = kIterations * 3;
    EXPECT_TEXTURE_EQ(&kExpectedResult1, color1, {0, 0, 0}, {1, 1, 1});

    constexpr std::array<uint32_t, 2> kExpectedResult2 = {{kIterations * 6, kIterations * 6}};
    EXPECT_TEXTURE_EQ(kExpectedResult2.data(), color2, {0, 0, 0}, {2, 1, 1});
    EXPECT_TEXTURE_EQ(kExpectedResult2.data(), color2, {0, 1, 0}, {2, 1, 1});
}

DAWN_INSTANTIATE_TEST(PixelLocalStorageTests, D3D11Backend(), MetalBackend());

}  // anonymous namespace
}  // namespace dawn
