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

#include "dawn/native/vulkan/RenderPassCache.h"

#include "absl/container/inlined_vector.h"
#include "dawn/common/Enumerator.h"
#include "dawn/common/HashUtils.h"
#include "dawn/common/Range.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

namespace {
VkAttachmentLoadOp VulkanAttachmentLoadOp(wgpu::LoadOp op) {
    switch (op) {
        case wgpu::LoadOp::Load:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        case wgpu::LoadOp::Clear:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case wgpu::LoadOp::ExpandResolveTexture:
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        case wgpu::LoadOp::Undefined:
            DAWN_UNREACHABLE();
            break;
    }
    DAWN_UNREACHABLE();
}

VkAttachmentStoreOp VulkanAttachmentStoreOp(wgpu::StoreOp op) {
    // TODO(crbug.com/dawn/485): return STORE_OP_STORE_NONE_QCOM if the device has required
    // extension.
    switch (op) {
        case wgpu::StoreOp::Store:
            return VK_ATTACHMENT_STORE_OP_STORE;
        case wgpu::StoreOp::Discard:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        case wgpu::StoreOp::Undefined:
            DAWN_UNREACHABLE();
            break;
    }
    DAWN_UNREACHABLE();
}

void InitializeLoadResolveSubpassDependencies(
    absl::InlinedVector<VkSubpassDependency, 2>* subpassDependenciesOut) {
    VkSubpassDependency dependencies[2];
    // Dependency for resolve texture's read -> resolve texture's write.
    dependencies[0].srcSubpass = 0;
    dependencies[0].dstSubpass = 1;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Dependency for color write in subpass 0 -> color write in subpass 1
    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = 1;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    subpassDependenciesOut->insert(subpassDependenciesOut->end(), std::begin(dependencies),
                                   std::end(dependencies));
}
}  // anonymous namespace

// RenderPassCacheQuery

void RenderPassCacheQuery::SetColor(ColorAttachmentIndex index,
                                    wgpu::TextureFormat format,
                                    wgpu::LoadOp loadOp,
                                    wgpu::StoreOp storeOp,
                                    bool hasResolveTarget) {
    colorMask.set(index);
    colorFormats[index] = format;
    colorLoadOp[index] = loadOp;
    colorStoreOp[index] = storeOp;
    resolveTargetMask[index] = hasResolveTarget;
    expandResolveMask.set(index, loadOp == wgpu::LoadOp::ExpandResolveTexture);
}

void RenderPassCacheQuery::SetDepthStencil(wgpu::TextureFormat format,
                                           wgpu::LoadOp depthLoadOpIn,
                                           wgpu::StoreOp depthStoreOpIn,
                                           bool depthReadOnlyIn,
                                           wgpu::LoadOp stencilLoadOpIn,
                                           wgpu::StoreOp stencilStoreOpIn,
                                           bool stencilReadOnlyIn) {
    hasDepthStencil = true;
    depthStencilFormat = format;
    depthLoadOp = depthLoadOpIn;
    depthStoreOp = depthStoreOpIn;
    depthReadOnly = depthReadOnlyIn;
    stencilLoadOp = stencilLoadOpIn;
    stencilStoreOp = stencilStoreOpIn;
    stencilReadOnly = stencilReadOnlyIn;
}

void RenderPassCacheQuery::SetSampleCount(uint32_t sampleCountIn) {
    sampleCount = sampleCountIn;
}

// RenderPassCache

RenderPassCache::RenderPassCache(Device* device) : mDevice(device) {}

RenderPassCache::~RenderPassCache() {
    std::lock_guard<std::mutex> lock(mMutex);
    for (auto [_, renderPassInfo] : mCache) {
        mDevice->fn.DestroyRenderPass(mDevice->GetVkDevice(), renderPassInfo.renderPass, nullptr);
    }

    mCache.clear();
}

ResultOrError<RenderPassCache::RenderPassInfo> RenderPassCache::GetRenderPass(
    const RenderPassCacheQuery& query) {
    std::lock_guard<std::mutex> lock(mMutex);
    auto it = mCache.find(query);
    if (it != mCache.end()) {
        return RenderPassInfo(it->second);
    }

    RenderPassInfo renderPass;
    DAWN_TRY_ASSIGN(renderPass, CreateRenderPassForQuery(query));
    mCache.emplace(query, renderPass);
    return renderPass;
}

ResultOrError<RenderPassCache::RenderPassInfo> RenderPassCache::CreateRenderPassForQuery(
    const RenderPassCacheQuery& query) {
    // The Vulkan subpasses want to know the layout of the attachments with VkAttachmentRef.
    // Precompute them as they must be pointer-chained in VkSubpassDescription.
    // Note that both colorAttachmentRefs and resolveAttachmentRefs can be sparse with holes
    // filled with VK_ATTACHMENT_UNUSED.
    PerColorAttachment<VkAttachmentReference> colorAttachmentRefs;
    PerColorAttachment<VkAttachmentReference> resolveAttachmentRefs;
    PerColorAttachment<VkAttachmentReference> inputAttachmentRefs;
    VkAttachmentReference depthStencilAttachmentRef;

    for (auto i : Range(kMaxColorAttachmentsTyped)) {
        colorAttachmentRefs[i].attachment = VK_ATTACHMENT_UNUSED;
        resolveAttachmentRefs[i].attachment = VK_ATTACHMENT_UNUSED;
        inputAttachmentRefs[i].attachment = VK_ATTACHMENT_UNUSED;
        // The Khronos Vulkan validation layer will complain if not set
        colorAttachmentRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        resolveAttachmentRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        inputAttachmentRefs[i].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    // Contains the attachment description that will be chained in the create info
    // The order of all attachments in attachmentDescs is "color-depthstencil-resolve".
    constexpr uint8_t kMaxAttachmentCount = kMaxColorAttachments * 2 + 1;
    std::array<VkAttachmentDescription, kMaxAttachmentCount> attachmentDescs = {};

    VkSampleCountFlagBits vkSampleCount = VulkanSampleCount(query.sampleCount);

    uint32_t attachmentCount = 0;
    ColorAttachmentIndex highestColorAttachmentIndexPlusOne(static_cast<uint8_t>(0));
    for (auto i : query.colorMask) {
        auto& attachmentRef = colorAttachmentRefs[i];
        auto& attachmentDesc = attachmentDescs[attachmentCount];

        attachmentRef.attachment = attachmentCount;
        attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachmentDesc.flags = 0;
        attachmentDesc.format = VulkanImageFormat(mDevice, query.colorFormats[i]);
        attachmentDesc.samples = vkSampleCount;
        attachmentDesc.loadOp = VulkanAttachmentLoadOp(query.colorLoadOp[i]);
        attachmentDesc.storeOp = VulkanAttachmentStoreOp(query.colorStoreOp[i]);
        attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachmentCount++;
        highestColorAttachmentIndexPlusOne =
            ColorAttachmentIndex(static_cast<uint8_t>(static_cast<uint8_t>(i) + 1u));
    }

    VkAttachmentReference* depthStencilAttachment = nullptr;
    if (query.hasDepthStencil) {
        const Format& dsFormat = mDevice->GetValidInternalFormat(query.depthStencilFormat);

        depthStencilAttachment = &depthStencilAttachmentRef;
        depthStencilAttachmentRef.attachment = attachmentCount;
        depthStencilAttachmentRef.layout = VulkanImageLayoutForDepthStencilAttachment(
            dsFormat, query.depthReadOnly, query.stencilReadOnly);

        // Build the attachment descriptor.
        auto& attachmentDesc = attachmentDescs[attachmentCount];
        attachmentDesc.flags = 0;
        attachmentDesc.format = VulkanImageFormat(mDevice, dsFormat.format);
        attachmentDesc.samples = vkSampleCount;

        attachmentDesc.loadOp = VulkanAttachmentLoadOp(query.depthLoadOp);
        attachmentDesc.storeOp = VulkanAttachmentStoreOp(query.depthStoreOp);
        attachmentDesc.stencilLoadOp = VulkanAttachmentLoadOp(query.stencilLoadOp);
        attachmentDesc.stencilStoreOp = VulkanAttachmentStoreOp(query.stencilStoreOp);

        // There is only one subpass, so it is safe to set both initialLayout and finalLayout to
        // the only subpass's layout.
        attachmentDesc.initialLayout = depthStencilAttachmentRef.layout;
        attachmentDesc.finalLayout = depthStencilAttachmentRef.layout;

        attachmentCount++;
    }

    uint32_t resolveAttachmentCount = 0;
    ColorAttachmentIndex highestInputAttachmentIndex(static_cast<uint8_t>(0));

    for (auto i : query.resolveTargetMask) {
        auto& resolveAttachmentRef = resolveAttachmentRefs[i];
        auto& resolveAttachmentDesc = attachmentDescs[attachmentCount];

        resolveAttachmentRef.attachment = attachmentCount;
        resolveAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        resolveAttachmentDesc.flags = 0;
        resolveAttachmentDesc.format = VulkanImageFormat(mDevice, query.colorFormats[i]);
        resolveAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        resolveAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        if (query.expandResolveMask.test(i)) {
            resolveAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            resolveAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            inputAttachmentRefs[i].attachment = resolveAttachmentRefs[i].attachment;

            highestInputAttachmentIndex = i;
        } else {
            resolveAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            resolveAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        resolveAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachmentCount++;
        resolveAttachmentCount++;
    }

    absl::InlinedVector<VkSubpassDescription, 2> subpassDescs;
    absl::InlinedVector<VkSubpassDependency, 2> subpassDependencies;
    if (query.expandResolveMask.any()) {
        // To simulate ExpandResolveTexture, we use two subpasses. The first subpass will read the
        // resolve texture as input attachment.
        subpassDescs.push_back({});
        VkSubpassDescription& subpassDesc = subpassDescs.back();
        subpassDesc.flags = 0;
        subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDesc.inputAttachmentCount = static_cast<uint8_t>(highestInputAttachmentIndex) + 1;
        subpassDesc.pInputAttachments = inputAttachmentRefs.data();
        subpassDesc.colorAttachmentCount = static_cast<uint8_t>(highestColorAttachmentIndexPlusOne);
        subpassDesc.pColorAttachments = colorAttachmentRefs.data();
        subpassDesc.pDepthStencilAttachment = depthStencilAttachment;

        InitializeLoadResolveSubpassDependencies(&subpassDependencies);
    }

    // Create the VkSubpassDescription that will be chained in the VkRenderPassCreateInfo
    subpassDescs.push_back({});
    VkSubpassDescription& subpassDesc = subpassDescs.back();
    subpassDesc.flags = 0;
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.inputAttachmentCount = 0;
    subpassDesc.pInputAttachments = nullptr;
    subpassDesc.colorAttachmentCount = static_cast<uint8_t>(highestColorAttachmentIndexPlusOne);
    subpassDesc.pColorAttachments = colorAttachmentRefs.data();

    // Qualcomm GPUs have a driver bug on some devices where passing a zero-length array to the
    // resolveAttachments causes a VK_ERROR_OUT_OF_HOST_MEMORY. nullptr must be passed instead.
    if (resolveAttachmentCount) {
        subpassDesc.pResolveAttachments = resolveAttachmentRefs.data();
    } else {
        subpassDesc.pResolveAttachments = nullptr;
    }

    subpassDesc.pDepthStencilAttachment = depthStencilAttachment;
    subpassDesc.preserveAttachmentCount = 0;
    subpassDesc.pPreserveAttachments = nullptr;

    // Chain everything in VkRenderPassCreateInfo
    VkRenderPassCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.attachmentCount = attachmentCount;
    createInfo.pAttachments = attachmentDescs.data();
    createInfo.subpassCount = subpassDescs.size();
    createInfo.pSubpasses = subpassDescs.data();
    createInfo.dependencyCount = subpassDependencies.size();
    createInfo.pDependencies = subpassDependencies.data();

    // Create the render pass from the zillion parameters
    RenderPassInfo renderPassInfo;
    renderPassInfo.mainSubpass = subpassDescs.size() - 1;
    renderPassInfo.uniqueId = nextRenderPassId++;
    DAWN_TRY(CheckVkSuccess(mDevice->fn.CreateRenderPass(mDevice->GetVkDevice(), &createInfo,
                                                         nullptr, &*renderPassInfo.renderPass),
                            "CreateRenderPass"));
    return renderPassInfo;
}

// RenderPassCache

// If you change these, remember to also update StreamImplVk.cpp

size_t RenderPassCache::CacheFuncs::operator()(const RenderPassCacheQuery& query) const {
    size_t hash = Hash(query.colorMask);

    HashCombine(&hash, Hash(query.resolveTargetMask));

    for (auto i : query.colorMask) {
        HashCombine(&hash, query.colorFormats[i], query.colorLoadOp[i], query.colorStoreOp[i]);
    }
    HashCombine(&hash, query.expandResolveMask);

    HashCombine(&hash, query.hasDepthStencil);
    if (query.hasDepthStencil) {
        HashCombine(&hash, query.depthStencilFormat, query.depthLoadOp, query.depthStoreOp,
                    query.depthReadOnly, query.stencilLoadOp, query.stencilStoreOp,
                    query.stencilReadOnly);
    }

    HashCombine(&hash, query.sampleCount);

    return hash;
}

bool RenderPassCache::CacheFuncs::operator()(const RenderPassCacheQuery& a,
                                             const RenderPassCacheQuery& b) const {
    if (a.colorMask != b.colorMask) {
        return false;
    }

    if (a.resolveTargetMask != b.resolveTargetMask) {
        return false;
    }

    if (a.sampleCount != b.sampleCount) {
        return false;
    }

    for (auto i : a.colorMask) {
        if ((a.colorFormats[i] != b.colorFormats[i]) || (a.colorLoadOp[i] != b.colorLoadOp[i]) ||
            (a.colorStoreOp[i] != b.colorStoreOp[i])) {
            return false;
        }
    }

    if (a.expandResolveMask != b.expandResolveMask) {
        return false;
    }

    if (a.hasDepthStencil != b.hasDepthStencil) {
        return false;
    }

    if (a.hasDepthStencil) {
        if ((a.depthStencilFormat != b.depthStencilFormat) || (a.depthLoadOp != b.depthLoadOp) ||
            (a.stencilLoadOp != b.stencilLoadOp) || (a.depthStoreOp != b.depthStoreOp) ||
            (a.depthReadOnly != b.depthReadOnly) || (a.stencilStoreOp != b.stencilStoreOp) ||
            (a.stencilReadOnly != b.stencilReadOnly)) {
            return false;
        }
    }

    return true;
}
}  // namespace dawn::native::vulkan
