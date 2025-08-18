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

#include "dawn/native/vulkan/ResolveTextureLoadingUtilsVk.h"

#include <sstream>
#include <string>
#include <utility>

#include "absl/container/inlined_vector.h"
#include "dawn/common/Assert.h"
#include "dawn/common/Enumerator.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/InternalPipelineStore.h"
#include "dawn/native/utils/WGPUHelpers.h"
#include "dawn/native/vulkan/BindGroupLayoutVk.h"
#include "dawn/native/vulkan/BindGroupVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/PipelineLayoutVk.h"
#include "dawn/native/vulkan/RenderPipelineVk.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/native/webgpu_absl_format.h"

namespace dawn::native::vulkan {

namespace {

constexpr char kBlitToColorVS[] = R"(

@vertex fn vert_fullscreen_quad(
  @builtin(vertex_index) vertex_index : u32,
) -> @builtin(position) vec4f {
  const pos = array(
      vec2f(-1.0, -1.0),
      vec2f( 3.0, -1.0),
      vec2f(-1.0,  3.0));
  return vec4f(pos[vertex_index], 0.0, 1.0);
}
)";

std::string GenerateFS(const BlitColorToColorWithDrawPipelineKey& pipelineKey) {
    std::ostringstream outputStructStream;
    std::ostringstream assignOutputsStream;
    std::ostringstream finalStream;

    finalStream << "enable chromium_internal_input_attachments;";

    for (auto i : pipelineKey.attachmentsToExpandResolve) {
        finalStream << absl::StrFormat(
            "@group(0) @binding(%u) @input_attachment_index(%u) var srcTex%u : "
            "input_attachment<f32>;\n",
            i, i, i);

        outputStructStream << absl::StrFormat("@location(%u) output%u : vec4f,\n", i, i);

        assignOutputsStream << absl::StrFormat(
            "\toutputColor.output%u = inputAttachmentLoad(srcTex%u);\n", i, i);
    }

    finalStream << "struct OutputColor {\n" << outputStructStream.str() << "}\n\n";
    finalStream << R"(
@fragment fn blit_to_color() -> OutputColor {
    var outputColor : OutputColor;
)" << assignOutputsStream.str()
                << R"(
    return outputColor;
})";

    return finalStream.str();
}

ResultOrError<Ref<RenderPipelineBase>> GetOrCreateColorBlitPipeline(
    DeviceBase* device,
    const BlitColorToColorWithDrawPipelineKey& pipelineKey,
    uint8_t colorAttachmentCount) {
    InternalPipelineStore* store = device->GetInternalPipelineStore();
    {
        auto it = store->expandResolveTexturePipelines.find(pipelineKey);
        if (it != store->expandResolveTexturePipelines.end()) {
            return it->second;
        }
    }

    // vertex shader.
    Ref<ShaderModuleBase> vshaderModule;
    DAWN_TRY_ASSIGN(vshaderModule, utils::CreateShaderModule(device, kBlitToColorVS));

    // fragment shader's source will depend on pipeline key.
    std::string fsCode = GenerateFS(pipelineKey);
    Ref<ShaderModuleBase> fshaderModule;
    DAWN_TRY_ASSIGN(fshaderModule, utils::CreateShaderModule(
                                       device, fsCode.c_str(),
                                       {tint::wgsl::Extension::kChromiumInternalInputAttachments}));

    FragmentState fragmentState = {};
    fragmentState.module = fshaderModule.Get();

    // Color target states.
    PerColorAttachment<ColorTargetState> colorTargets = {};
    PerColorAttachment<wgpu::ColorTargetStateExpandResolveTextureDawn> msaaExpandResolveStates{};

    for (auto [i, target] : Enumerate(colorTargets)) {
        target.format = pipelineKey.colorTargetFormats[i];
        // We shouldn't change the color targets that are not involved in.
        if (pipelineKey.resolveTargetsMask[i]) {
            target.nextInChain = &msaaExpandResolveStates[i];
            msaaExpandResolveStates[i].enabled = pipelineKey.attachmentsToExpandResolve[i];
            if (msaaExpandResolveStates[i].enabled) {
                target.writeMask = wgpu::ColorWriteMask::All;
            } else {
                target.writeMask = wgpu::ColorWriteMask::None;
            }
        } else {
            target.writeMask = wgpu::ColorWriteMask::None;
        }
    }

    fragmentState.targetCount = colorAttachmentCount;
    fragmentState.targets = colorTargets.data();

    RenderPipelineDescriptor renderPipelineDesc = {};
    renderPipelineDesc.label = "blit_color_to_color";
    renderPipelineDesc.vertex.module = vshaderModule.Get();
    renderPipelineDesc.fragment = &fragmentState;

    // Depth stencil state.
    DepthStencilState depthStencilState = {};
    if (pipelineKey.depthStencilFormat != wgpu::TextureFormat::Undefined) {
        depthStencilState.format = pipelineKey.depthStencilFormat;
        depthStencilState.depthWriteEnabled = wgpu::OptionalBool::False;

        renderPipelineDesc.depthStencil = &depthStencilState;
    }

    // Multisample state.
    DAWN_ASSERT(pipelineKey.sampleCount > 1);
    renderPipelineDesc.multisample.count = pipelineKey.sampleCount;

    renderPipelineDesc.layout = nullptr;

    Ref<RenderPipelineBase> pipeline;
    DAWN_TRY_ASSIGN(
        pipeline, device->CreateRenderPipeline(&renderPipelineDesc, /*allowInternalBinding=*/true));

    store->expandResolveTexturePipelines.emplace(pipelineKey, pipeline);
    return pipeline;
}

}  // namespace

MaybeError BeginRenderPassAndExpandResolveTextureWithDraw(Device* device,
                                                          CommandRecordingContext* commandContext,
                                                          const BeginRenderPassCmd* renderPass,
                                                          const VkRenderPassBeginInfo& beginInfo) {
    DAWN_ASSERT(device->IsLockedByCurrentThreadIfNeeded());

    // Construct pipeline key
    BlitColorToColorWithDrawPipelineKey pipelineKey;
    ColorAttachmentIndex colorAttachmentCount =
        GetHighestBitIndexPlusOne(renderPass->attachmentState->GetColorAttachmentsMask());
    for (ColorAttachmentIndex colorIdx : renderPass->attachmentState->GetColorAttachmentsMask()) {
        const auto& colorAttachment = renderPass->colorAttachments[colorIdx];
        const auto& view = colorAttachment.view;
        DAWN_ASSERT(view != nullptr);
        const Format& format = view->GetFormat();
        TextureComponentType baseType = format.GetAspectInfo(Aspect::Color).baseType;
        // Blitting integer textures are not currently supported.
        DAWN_ASSERT(baseType == TextureComponentType::Float);

        if (colorAttachment.loadOp == wgpu::LoadOp::ExpandResolveTexture) {
            // TODO(42240662): Handle the cases where resolveTarget is altered by workarounds such
            // as ResolveMultipleAttachmentInSeparatePasses/AlwaysResolveIntoZeroLevelAndLayer. We
            // need to careful handle such cases because the render pass' compatibility could be
            // affected as well.
            DAWN_INVALID_IF(colorAttachment.resolveTarget == nullptr,
                            "resolveTarget at %d has been removed by some workarounds. %s doesn't "
                            "support this yet.",
                            colorIdx, colorAttachment.loadOp);
            DAWN_ASSERT(colorAttachment.resolveTarget->GetLayerCount() == 1u);
            DAWN_ASSERT(colorAttachment.resolveTarget->GetDimension() ==
                        wgpu::TextureViewDimension::e2D);
            pipelineKey.attachmentsToExpandResolve.set(colorIdx);
        }
        pipelineKey.resolveTargetsMask.set(colorIdx, colorAttachment.resolveTarget != nullptr);

        pipelineKey.colorTargetFormats[colorIdx] = format.format;
        pipelineKey.sampleCount = view->GetTexture()->GetSampleCount();
    }

    DAWN_ASSERT(pipelineKey.attachmentsToExpandResolve.any());

    pipelineKey.depthStencilFormat = wgpu::TextureFormat::Undefined;
    if (renderPass->depthStencilAttachment.view != nullptr) {
        pipelineKey.depthStencilFormat =
            renderPass->depthStencilAttachment.view->GetFormat().format;
    }

    Ref<RenderPipelineBase> pipeline;
    DAWN_TRY_ASSIGN(pipeline, GetOrCreateColorBlitPipeline(
                                  device, pipelineKey, static_cast<uint8_t>(colorAttachmentCount)));

    RenderPipeline* pipelineVk = ToBackend(pipeline.Get());

    // Construct bind group.
    Ref<BindGroupLayoutBase> bgl;
    DAWN_TRY_ASSIGN(bgl, pipelineVk->GetBindGroupLayout(0));

    Ref<BindGroupBase> bindGroup;
    absl::InlinedVector<BindGroupEntry, kMaxColorAttachments> bgEntries = {};

    for (auto colorIdx : pipelineKey.attachmentsToExpandResolve) {
        const auto& colorAttachment = renderPass->colorAttachments[colorIdx];
        bgEntries.push_back({});
        auto& bgEntry = bgEntries.back();
        bgEntry.binding = static_cast<uint8_t>(colorIdx);
        bgEntry.textureView = colorAttachment.resolveTarget.Get();

        // Transition the resolve texture
        auto* textureVk = static_cast<Texture*>(colorAttachment.resolveTarget->GetTexture());
        textureVk->TransitionUsageNow(commandContext, kResolveAttachmentLoadingUsage,
                                      wgpu::ShaderStage::Fragment,
                                      colorAttachment.resolveTarget->GetSubresourceRange());
    }

    BindGroupDescriptor bgDesc = {};
    bgDesc.layout = bgl.Get();
    bgDesc.entryCount = bgEntries.size();
    bgDesc.entries = bgEntries.data();
    DAWN_TRY_ASSIGN(bindGroup, device->CreateBindGroup(&bgDesc, UsageValidationMode::Internal));
    BindGroup* bindGroupVk = ToBackend(bindGroup.Get());

    // Start the render pass
    VkCommandBuffer commandBuffer = commandContext->commandBuffer;

    device->fn.CmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Draw to perform the blit.
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(renderPass->width);
    viewport.height = static_cast<float>(renderPass->height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    device->fn.CmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent.width = renderPass->width;
    scissor.extent.height = renderPass->height;
    device->fn.CmdSetScissor(commandBuffer, 0, 1, &scissor);

    device->fn.CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                               *pipelineVk->GetHandle());
    device->fn.CmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                     *pipelineVk->GetVkLayout(), 0, 1, &*bindGroupVk->GetHandle(),
                                     0, nullptr);
    device->fn.CmdDraw(commandBuffer, 3, 1, 0, 0);

    device->fn.CmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

    // Subpass dependency automatically transitions the layouts of the resolve textures
    // to RenderAttachment. So we need to notify TextureVk and don't need to use any explicit
    // barriers.
    for (auto colorIdx : pipelineKey.attachmentsToExpandResolve) {
        const auto& colorAttachment = renderPass->colorAttachments[colorIdx];
        auto* textureVk = static_cast<Texture*>(colorAttachment.resolveTarget->GetTexture());
        textureVk->UpdateUsage(wgpu::TextureUsage::RenderAttachment, wgpu::ShaderStage::Fragment,
                               colorAttachment.resolveTarget->GetSubresourceRange());
    }

    return {};
}
}  // namespace dawn::native::vulkan
