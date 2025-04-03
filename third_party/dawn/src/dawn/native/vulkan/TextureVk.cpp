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

#include "dawn/native/vulkan/TextureVk.h"

#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Math.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/EnumMaskIterator.h"
#include "dawn/native/Error.h"
#include "dawn/native/VulkanBackend.h"
#include "dawn/native/vulkan/BufferVk.h"
#include "dawn/native/vulkan/CommandBufferVk.h"
#include "dawn/native/vulkan/CommandRecordingContextVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/PhysicalDeviceVk.h"
#include "dawn/native/vulkan/QueueVk.h"
#include "dawn/native/vulkan/ResourceHeapVk.h"
#include "dawn/native/vulkan/ResourceMemoryAllocatorVk.h"
#include "dawn/native/vulkan/SharedFenceVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

namespace {
// Converts an Dawn texture dimension to a Vulkan image view type.
// Contrary to image types, image view types include arrayness and cubemapness
VkImageViewType VulkanImageViewType(wgpu::TextureViewDimension dimension) {
    switch (dimension) {
        case wgpu::TextureViewDimension::e1D:
            return VK_IMAGE_VIEW_TYPE_1D;
        case wgpu::TextureViewDimension::e2D:
            return VK_IMAGE_VIEW_TYPE_2D;
        case wgpu::TextureViewDimension::e2DArray:
            return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case wgpu::TextureViewDimension::Cube:
            return VK_IMAGE_VIEW_TYPE_CUBE;
        case wgpu::TextureViewDimension::CubeArray:
            return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        case wgpu::TextureViewDimension::e3D:
            return VK_IMAGE_VIEW_TYPE_3D;

        case wgpu::TextureViewDimension::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

// Reserved texture usages to represent mixed read-only/writable depth-stencil texture usages
// when combining the planes of depth-stencil textures. They can be combined with other in-pass
// readonly usages like wgpu::TextureUsage::TextureBinding.
// TODO(dawn:2172): Consider making a bespoke enum instead of hackily extending TextureUsage.
constexpr wgpu::TextureUsage kDepthReadOnlyStencilWritableAttachment =
    kReservedTextureUsage | static_cast<wgpu::TextureUsage>(1 << 30);
constexpr wgpu::TextureUsage kDepthWritableStencilReadOnlyAttachment =
    kReservedTextureUsage | static_cast<wgpu::TextureUsage>(1 << 29);

// Merge two usages for depth and stencil into a single combined usage that uses the reserved
// texture usages above. This is used to handle combining Aspect::Depth and Aspect::Stencil into a
// single Aspect::CombinedDepthStencil.
wgpu::TextureUsage MergeDepthStencilUsage(wgpu::TextureUsage depth, wgpu::TextureUsage stencil) {
    // Aspects that are RenderAttachment cannot be anything else at the same time. This lets us
    // check if we are in one of the RenderAttachment + (ReadOnlyAttachment|readonly usage) cases
    // and know only the aspect with the readonly attachment might contain extra usages like
    // TextureBinding.
    DAWN_ASSERT(depth == wgpu::TextureUsage::RenderAttachment ||
                IsSubset(depth, ~wgpu::TextureUsage::RenderAttachment));
    DAWN_ASSERT(stencil == wgpu::TextureUsage::RenderAttachment ||
                IsSubset(stencil, ~wgpu::TextureUsage::RenderAttachment));

    if (depth == wgpu::TextureUsage::RenderAttachment && stencil & kReadOnlyRenderAttachment) {
        return kDepthWritableStencilReadOnlyAttachment | (stencil & ~kReadOnlyRenderAttachment);
    } else if (depth & kReadOnlyRenderAttachment &&
               stencil == wgpu::TextureUsage::RenderAttachment) {
        return kDepthReadOnlyStencilWritableAttachment | (depth & ~kReadOnlyRenderAttachment);
    } else {
        // Not one of the reserved usage special cases, we can just combine the aspect's usage the
        // simple way!
        return depth | stencil;
    }
}

// Computes which vulkan access type could be required for the given Dawn usage.
// TODO(crbug.com/dawn/269): We shouldn't need any access usages for srcAccessMask when
// the previous usage is readonly because an execution dependency is sufficient.
VkAccessFlags VulkanAccessFlags(wgpu::TextureUsage usage, const Format& format) {
    if (usage & kReservedTextureUsage) {
        // Handle the special readonly usages for mixed depth-stencil.
        DAWN_ASSERT(IsSubset(kDepthReadOnlyStencilWritableAttachment, usage) ||
                    IsSubset(kDepthWritableStencilReadOnlyAttachment, usage));

        // Add any additional access flags for the non-attachment part of the usage.
        const wgpu::TextureUsage nonAttachmentUsages =
            usage &
            ~(kDepthReadOnlyStencilWritableAttachment | kDepthWritableStencilReadOnlyAttachment);
        return VulkanAccessFlags(nonAttachmentUsages, format) |
               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }

    VkAccessFlags flags = 0;
    if (usage & wgpu::TextureUsage::CopySrc) {
        flags |= VK_ACCESS_TRANSFER_READ_BIT;
    }
    if (usage & wgpu::TextureUsage::CopyDst) {
        flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    if (usage & (wgpu::TextureUsage::TextureBinding | kReadOnlyStorageTexture)) {
        flags |= VK_ACCESS_SHADER_READ_BIT;
    }
    if (usage & kWriteOnlyStorageTexture) {
        flags |= VK_ACCESS_SHADER_WRITE_BIT;
    }
    if (usage & wgpu::TextureUsage::StorageBinding) {
        flags |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    }
    if (usage & wgpu::TextureUsage::RenderAttachment) {
        if (format.HasDepthOrStencil()) {
            flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        } else {
            flags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        }
    }
    if (usage & kReadOnlyRenderAttachment) {
        flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    }

    if (usage & kResolveAttachmentLoadingUsage) {
        // - The texture will be used as input attachment in the first subpass and loaded with
        // VK_ATTACHMENT_LOAD_OP_LOAD. This requires VK_ACCESS_COLOR_ATTACHMENT_READ_BIT access.
        // - It will also be read as subpass input in fragment shader. This requires
        // VK_ACCESS_INPUT_ATTACHMENT_READ_BIT.
        flags |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    }

    if (usage & kPresentAcquireTextureUsage) {
        // The present acquire usage is only used internally by the swapchain and is never used in
        // combination with other usages.
        DAWN_ASSERT(usage == kPresentAcquireTextureUsage);
        // The Vulkan spec has the following note:
        //
        //   When the presentable image will be accessed by some stage S, the recommended idiom
        //   for ensuring correct synchronization is:
        //
        //   The VkSubmitInfo used to submit the image layout transition for execution includes
        //   vkAcquireNextImageKHR::semaphore in its pWaitSemaphores member, with the
        //   corresponding element of pWaitDstStageMask including S.
        //
        //   The synchronization command that performs any necessary image layout transition
        //   includes S in both the srcStageMask and dstStageMask.
        //
        // There is no mention of an access flag because there is no access flag associated with
        // the presentation engine, so we leave it to 0.
        flags |= 0;
    }
    if (usage & kPresentReleaseTextureUsage) {
        // The present release usage is only used internally by the swapchain and is never used in
        // combination with other usages.
        DAWN_ASSERT(usage == kPresentReleaseTextureUsage);
        // The Vulkan spec has the following note:
        //
        //   When transitioning the image to VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR or
        //   VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, there is no need to delay subsequent
        //   processing, or perform any visibility operations (as vkQueuePresentKHR performs
        //   automatic visibility operations). To achieve this, the dstAccessMask member of
        //   the VkImageMemoryBarrier should be set to 0, and the dstStageMask parameter
        //   should be set to VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT.
        //
        // So on the transition to Present we don't need an access flag. The other
        // direction doesn't matter because swapchain textures always start a new frame
        // as uninitialized.
        flags |= 0;
    }

    return flags;
}

// Computes which Vulkan pipeline stage can access a texture in the given Dawn usage
VkPipelineStageFlags VulkanPipelineStage(wgpu::TextureUsage usage,
                                         wgpu::ShaderStage shaderStage,
                                         const Format& format) {
    if (usage & kReservedTextureUsage) {
        // Handle the special readonly usages for mixed depth-stencil.
        DAWN_ASSERT(IsSubset(kDepthReadOnlyStencilWritableAttachment, usage) ||
                    IsSubset(kDepthWritableStencilReadOnlyAttachment, usage));

        // Convert all the reserved attachment usages into just RenderAttachment.
        const wgpu::TextureUsage nonAttachmentUsages =
            usage &
            ~(kDepthReadOnlyStencilWritableAttachment | kDepthWritableStencilReadOnlyAttachment);
        return VulkanPipelineStage(nonAttachmentUsages | wgpu::TextureUsage::RenderAttachment,
                                   shaderStage, format);
    }

    VkPipelineStageFlags flags = 0;

    if (usage == wgpu::TextureUsage::None) {
        // This only happens when a texture is initially created (and for srcAccessMask) in
        // which case there is no need to wait on anything to stop accessing this texture.
        return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    if (usage & (wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst)) {
        flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    if (usage & kShaderTextureUsages) {
        if (shaderStage & wgpu::ShaderStage::Vertex) {
            flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        }
        if (shaderStage & wgpu::ShaderStage::Fragment) {
            flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        if (shaderStage & wgpu::ShaderStage::Compute) {
            flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        }
    }
    if (usage & kResolveAttachmentLoadingUsage) {
        // - The texture will be used as input attachment in the first subpass and loaded with
        // VK_ATTACHMENT_LOAD_OP_LOAD. This happens at VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        // stage.
        // - It will also be read as subpass input in fragment shader.
        flags |=
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    if (usage & (wgpu::TextureUsage::RenderAttachment | kReadOnlyRenderAttachment)) {
        if (format.HasDepthOrStencil()) {
            flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                     VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        } else {
            flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
    }

    if (usage & kPresentAcquireTextureUsage) {
        // The present acquire usage is only used internally by the swapchain and is never used in
        // combination with other usages.
        DAWN_ASSERT(usage == kPresentAcquireTextureUsage);
        // The vkAcquireNextImageKHR method is a read operation in Vulkan which completes
        // before the semaphore/fence out parameters are signaled. This means that future uses
        // of the texture must performs a memory barriers that synchronizes with that
        // semaphore/barrier. Dawn uses the ALL_COMMANDS_BIT stage for the semaphore, however
        // such a semaphore doesn't synchronize with a subsequent BOTTOM_OF_PIPE or NONE
        // srcStage vkPipelineBarrier because there are no common stages. Instead we also use
        // VK_PIPELINE_STAGE_ALL_COMMANDS_BIT for srcStage for presentable images, ensuring
        // correct ordering. This explains the idiom noted in the Vulkan spec:
        //
        //   When the presentable image will be accessed by some stage S, the recommended idiom
        //   for ensuring correct synchronization is:
        //
        //   The VkSubmitInfo used to submit the image layout transition for execution includes
        //   vkAcquireNextImageKHR::semaphore in its pWaitSemaphores member, with the
        //   corresponding element of pWaitDstStageMask including S.
        //
        //   The synchronization command that performs any necessary image layout transition
        //   includes S in both the srcStageMask and dstStageMask.
        flags |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }
    if (usage & kPresentReleaseTextureUsage) {
        // The present release usage is only used internally by the swapchain and is never used in
        // combination with other usages.
        DAWN_ASSERT(usage == kPresentReleaseTextureUsage);
        // The Vulkan spec has the following note:
        //
        //   When transitioning the image to VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR or
        //   VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, there is no need to delay subsequent
        //   processing, or perform any visibility operations (as vkQueuePresentKHR performs
        //   automatic visibility operations). To achieve this, the dstAccessMask member of
        //   the VkImageMemoryBarrier should be set to 0, and the dstStageMask parameter
        //   should be set to VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT.
        //
        // So on the transition to Present we use the "bottom of pipe" stage. The other
        // direction doesn't matter because swapchain textures always start a new frame
        // as uninitialized.
        flags |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }

    // A zero value isn't a valid pipeline stage mask
    DAWN_ASSERT(flags != 0);
    return flags;
}

VkImageMemoryBarrier BuildMemoryBarrier(const Texture* texture,
                                        wgpu::TextureUsage lastUsage,
                                        wgpu::TextureUsage usage,
                                        const SubresourceRange& range) {
    const Format& format = texture->GetFormat();

    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcAccessMask = VulkanAccessFlags(lastUsage, format);
    barrier.dstAccessMask = VulkanAccessFlags(usage, format);
    barrier.oldLayout = VulkanImageLayout(format, lastUsage);
    barrier.newLayout = VulkanImageLayout(format, usage);
    barrier.image = texture->GetHandle();
    barrier.subresourceRange.aspectMask = VulkanAspectMask(range.aspects);
    barrier.subresourceRange.baseMipLevel = range.baseMipLevel;
    barrier.subresourceRange.levelCount = range.levelCount;
    barrier.subresourceRange.baseArrayLayer = range.baseArrayLayer;
    barrier.subresourceRange.layerCount = range.layerCount;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    return barrier;
}

void FillVulkanCreateInfoSizesAndType(const Texture& texture, VkImageCreateInfo* info) {
    const Extent3D& size = texture.GetBaseSize();

    info->mipLevels = texture.GetNumMipLevels();
    info->samples = VulkanSampleCount(texture.GetSampleCount());

    // Fill in the image type, and paper over differences in how the array layer count is
    // specified between WebGPU and Vulkan.
    switch (texture.GetDimension()) {
        case wgpu::TextureDimension::Undefined:
            DAWN_UNREACHABLE();

        case wgpu::TextureDimension::e1D:
            info->imageType = VK_IMAGE_TYPE_1D;
            info->extent = {size.width, 1, 1};
            info->arrayLayers = 1;
            break;

        case wgpu::TextureDimension::e2D:
            info->imageType = VK_IMAGE_TYPE_2D;
            info->extent = {size.width, size.height, 1};
            info->arrayLayers = size.depthOrArrayLayers;
            break;

        case wgpu::TextureDimension::e3D:
            info->imageType = VK_IMAGE_TYPE_3D;
            info->extent = {size.width, size.height, size.depthOrArrayLayers};
            info->arrayLayers = 1;
            break;
    }
}

Aspect ComputeCombinedAspect(Device* device, const Format& format) {
    // In early Vulkan versions it is not possible to transition depth and stencil separately so
    // textures with Depth|Stencil will be promoted to a single CombinedDepthStencil aspect
    // internally.
    if (format.aspects == (Aspect::Depth | Aspect::Stencil)) {
        return Aspect::CombinedDepthStencil;
    }
    // Same thing for Stencil8 if it is emulated with a depth-stencil format and not directly S8.
    if (format.format == wgpu::TextureFormat::Stencil8 &&
        !device->IsToggleEnabled(Toggle::VulkanUseS8)) {
        return Aspect::CombinedDepthStencil;
    }

    // Some multiplanar images cannot have planes transitioned separately and instead Vulkan
    // requires that the "Color" aspect be used for barriers, so Plane0|Plane1|Plane2 is promoted to
    // just Color. The Vulkan spec requires: "If image has a single-plane color format or is not
    // disjoint, then the aspectMask member of subresourceRange must be VK_IMAGE_ASPECT_COLOR_BIT.".
    if (format.IsMultiPlanar()) {
        return Aspect::Color;
    }

    // No need to combine aspects.
    return Aspect::None;
}

}  // namespace

#define SIMPLE_FORMAT_MAPPING(X)                                                      \
    X(wgpu::TextureFormat::R8Unorm, VK_FORMAT_R8_UNORM)                               \
    X(wgpu::TextureFormat::R8Snorm, VK_FORMAT_R8_SNORM)                               \
    X(wgpu::TextureFormat::R8Uint, VK_FORMAT_R8_UINT)                                 \
    X(wgpu::TextureFormat::R8Sint, VK_FORMAT_R8_SINT)                                 \
                                                                                      \
    X(wgpu::TextureFormat::R16Unorm, VK_FORMAT_R16_UNORM)                             \
    X(wgpu::TextureFormat::R16Snorm, VK_FORMAT_R16_SNORM)                             \
    X(wgpu::TextureFormat::R16Uint, VK_FORMAT_R16_UINT)                               \
    X(wgpu::TextureFormat::R16Sint, VK_FORMAT_R16_SINT)                               \
    X(wgpu::TextureFormat::R16Float, VK_FORMAT_R16_SFLOAT)                            \
    X(wgpu::TextureFormat::RG8Unorm, VK_FORMAT_R8G8_UNORM)                            \
    X(wgpu::TextureFormat::RG8Snorm, VK_FORMAT_R8G8_SNORM)                            \
    X(wgpu::TextureFormat::RG8Uint, VK_FORMAT_R8G8_UINT)                              \
    X(wgpu::TextureFormat::RG8Sint, VK_FORMAT_R8G8_SINT)                              \
                                                                                      \
    X(wgpu::TextureFormat::R32Uint, VK_FORMAT_R32_UINT)                               \
    X(wgpu::TextureFormat::R32Sint, VK_FORMAT_R32_SINT)                               \
    X(wgpu::TextureFormat::R32Float, VK_FORMAT_R32_SFLOAT)                            \
    X(wgpu::TextureFormat::RG16Unorm, VK_FORMAT_R16G16_UNORM)                         \
    X(wgpu::TextureFormat::RG16Snorm, VK_FORMAT_R16G16_SNORM)                         \
    X(wgpu::TextureFormat::RG16Uint, VK_FORMAT_R16G16_UINT)                           \
    X(wgpu::TextureFormat::RG16Sint, VK_FORMAT_R16G16_SINT)                           \
    X(wgpu::TextureFormat::RG16Float, VK_FORMAT_R16G16_SFLOAT)                        \
    X(wgpu::TextureFormat::RGBA8Unorm, VK_FORMAT_R8G8B8A8_UNORM)                      \
    X(wgpu::TextureFormat::RGBA8UnormSrgb, VK_FORMAT_R8G8B8A8_SRGB)                   \
    X(wgpu::TextureFormat::RGBA8Snorm, VK_FORMAT_R8G8B8A8_SNORM)                      \
    X(wgpu::TextureFormat::RGBA8Uint, VK_FORMAT_R8G8B8A8_UINT)                        \
    X(wgpu::TextureFormat::RGBA8Sint, VK_FORMAT_R8G8B8A8_SINT)                        \
    X(wgpu::TextureFormat::BGRA8Unorm, VK_FORMAT_B8G8R8A8_UNORM)                      \
    X(wgpu::TextureFormat::BGRA8UnormSrgb, VK_FORMAT_B8G8R8A8_SRGB)                   \
    X(wgpu::TextureFormat::RGB10A2Uint, VK_FORMAT_A2B10G10R10_UINT_PACK32)            \
    X(wgpu::TextureFormat::RGB10A2Unorm, VK_FORMAT_A2B10G10R10_UNORM_PACK32)          \
    X(wgpu::TextureFormat::RG11B10Ufloat, VK_FORMAT_B10G11R11_UFLOAT_PACK32)          \
    X(wgpu::TextureFormat::RGB9E5Ufloat, VK_FORMAT_E5B9G9R9_UFLOAT_PACK32)            \
                                                                                      \
    X(wgpu::TextureFormat::RG32Uint, VK_FORMAT_R32G32_UINT)                           \
    X(wgpu::TextureFormat::RG32Sint, VK_FORMAT_R32G32_SINT)                           \
    X(wgpu::TextureFormat::RG32Float, VK_FORMAT_R32G32_SFLOAT)                        \
    X(wgpu::TextureFormat::RGBA16Unorm, VK_FORMAT_R16G16B16A16_UNORM)                 \
    X(wgpu::TextureFormat::RGBA16Snorm, VK_FORMAT_R16G16B16A16_SNORM)                 \
    X(wgpu::TextureFormat::RGBA16Uint, VK_FORMAT_R16G16B16A16_UINT)                   \
    X(wgpu::TextureFormat::RGBA16Sint, VK_FORMAT_R16G16B16A16_SINT)                   \
    X(wgpu::TextureFormat::RGBA16Float, VK_FORMAT_R16G16B16A16_SFLOAT)                \
                                                                                      \
    X(wgpu::TextureFormat::RGBA32Uint, VK_FORMAT_R32G32B32A32_UINT)                   \
    X(wgpu::TextureFormat::RGBA32Sint, VK_FORMAT_R32G32B32A32_SINT)                   \
    X(wgpu::TextureFormat::RGBA32Float, VK_FORMAT_R32G32B32A32_SFLOAT)                \
                                                                                      \
    X(wgpu::TextureFormat::Depth16Unorm, VK_FORMAT_D16_UNORM)                         \
    X(wgpu::TextureFormat::Depth32Float, VK_FORMAT_D32_SFLOAT)                        \
    X(wgpu::TextureFormat::Depth32FloatStencil8, VK_FORMAT_D32_SFLOAT_S8_UINT)        \
                                                                                      \
    X(wgpu::TextureFormat::BC1RGBAUnorm, VK_FORMAT_BC1_RGBA_UNORM_BLOCK)              \
    X(wgpu::TextureFormat::BC1RGBAUnormSrgb, VK_FORMAT_BC1_RGBA_SRGB_BLOCK)           \
    X(wgpu::TextureFormat::BC2RGBAUnorm, VK_FORMAT_BC2_UNORM_BLOCK)                   \
    X(wgpu::TextureFormat::BC2RGBAUnormSrgb, VK_FORMAT_BC2_SRGB_BLOCK)                \
    X(wgpu::TextureFormat::BC3RGBAUnorm, VK_FORMAT_BC3_UNORM_BLOCK)                   \
    X(wgpu::TextureFormat::BC3RGBAUnormSrgb, VK_FORMAT_BC3_SRGB_BLOCK)                \
    X(wgpu::TextureFormat::BC4RSnorm, VK_FORMAT_BC4_SNORM_BLOCK)                      \
    X(wgpu::TextureFormat::BC4RUnorm, VK_FORMAT_BC4_UNORM_BLOCK)                      \
    X(wgpu::TextureFormat::BC5RGSnorm, VK_FORMAT_BC5_SNORM_BLOCK)                     \
    X(wgpu::TextureFormat::BC5RGUnorm, VK_FORMAT_BC5_UNORM_BLOCK)                     \
    X(wgpu::TextureFormat::BC6HRGBFloat, VK_FORMAT_BC6H_SFLOAT_BLOCK)                 \
    X(wgpu::TextureFormat::BC6HRGBUfloat, VK_FORMAT_BC6H_UFLOAT_BLOCK)                \
    X(wgpu::TextureFormat::BC7RGBAUnorm, VK_FORMAT_BC7_UNORM_BLOCK)                   \
    X(wgpu::TextureFormat::BC7RGBAUnormSrgb, VK_FORMAT_BC7_SRGB_BLOCK)                \
                                                                                      \
    X(wgpu::TextureFormat::ETC2RGB8Unorm, VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK)          \
    X(wgpu::TextureFormat::ETC2RGB8UnormSrgb, VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK)       \
    X(wgpu::TextureFormat::ETC2RGB8A1Unorm, VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK)      \
    X(wgpu::TextureFormat::ETC2RGB8A1UnormSrgb, VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK)   \
    X(wgpu::TextureFormat::ETC2RGBA8Unorm, VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK)       \
    X(wgpu::TextureFormat::ETC2RGBA8UnormSrgb, VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK)    \
    X(wgpu::TextureFormat::EACR11Unorm, VK_FORMAT_EAC_R11_UNORM_BLOCK)                \
    X(wgpu::TextureFormat::EACR11Snorm, VK_FORMAT_EAC_R11_SNORM_BLOCK)                \
    X(wgpu::TextureFormat::EACRG11Unorm, VK_FORMAT_EAC_R11G11_UNORM_BLOCK)            \
    X(wgpu::TextureFormat::EACRG11Snorm, VK_FORMAT_EAC_R11G11_SNORM_BLOCK)            \
                                                                                      \
    X(wgpu::TextureFormat::ASTC4x4Unorm, VK_FORMAT_ASTC_4x4_UNORM_BLOCK)              \
    X(wgpu::TextureFormat::ASTC4x4UnormSrgb, VK_FORMAT_ASTC_4x4_SRGB_BLOCK)           \
    X(wgpu::TextureFormat::ASTC5x4Unorm, VK_FORMAT_ASTC_5x4_UNORM_BLOCK)              \
    X(wgpu::TextureFormat::ASTC5x4UnormSrgb, VK_FORMAT_ASTC_5x4_SRGB_BLOCK)           \
    X(wgpu::TextureFormat::ASTC5x5Unorm, VK_FORMAT_ASTC_5x5_UNORM_BLOCK)              \
    X(wgpu::TextureFormat::ASTC5x5UnormSrgb, VK_FORMAT_ASTC_5x5_SRGB_BLOCK)           \
    X(wgpu::TextureFormat::ASTC6x5Unorm, VK_FORMAT_ASTC_6x5_UNORM_BLOCK)              \
    X(wgpu::TextureFormat::ASTC6x5UnormSrgb, VK_FORMAT_ASTC_6x5_SRGB_BLOCK)           \
    X(wgpu::TextureFormat::ASTC6x6Unorm, VK_FORMAT_ASTC_6x6_UNORM_BLOCK)              \
    X(wgpu::TextureFormat::ASTC6x6UnormSrgb, VK_FORMAT_ASTC_6x6_SRGB_BLOCK)           \
    X(wgpu::TextureFormat::ASTC8x5Unorm, VK_FORMAT_ASTC_8x5_UNORM_BLOCK)              \
    X(wgpu::TextureFormat::ASTC8x5UnormSrgb, VK_FORMAT_ASTC_8x5_SRGB_BLOCK)           \
    X(wgpu::TextureFormat::ASTC8x6Unorm, VK_FORMAT_ASTC_8x6_UNORM_BLOCK)              \
    X(wgpu::TextureFormat::ASTC8x6UnormSrgb, VK_FORMAT_ASTC_8x6_SRGB_BLOCK)           \
    X(wgpu::TextureFormat::ASTC8x8Unorm, VK_FORMAT_ASTC_8x8_UNORM_BLOCK)              \
    X(wgpu::TextureFormat::ASTC8x8UnormSrgb, VK_FORMAT_ASTC_8x8_SRGB_BLOCK)           \
    X(wgpu::TextureFormat::ASTC10x5Unorm, VK_FORMAT_ASTC_10x5_UNORM_BLOCK)            \
    X(wgpu::TextureFormat::ASTC10x5UnormSrgb, VK_FORMAT_ASTC_10x5_SRGB_BLOCK)         \
    X(wgpu::TextureFormat::ASTC10x6Unorm, VK_FORMAT_ASTC_10x6_UNORM_BLOCK)            \
    X(wgpu::TextureFormat::ASTC10x6UnormSrgb, VK_FORMAT_ASTC_10x6_SRGB_BLOCK)         \
    X(wgpu::TextureFormat::ASTC10x8Unorm, VK_FORMAT_ASTC_10x8_UNORM_BLOCK)            \
    X(wgpu::TextureFormat::ASTC10x8UnormSrgb, VK_FORMAT_ASTC_10x8_SRGB_BLOCK)         \
    X(wgpu::TextureFormat::ASTC10x10Unorm, VK_FORMAT_ASTC_10x10_UNORM_BLOCK)          \
    X(wgpu::TextureFormat::ASTC10x10UnormSrgb, VK_FORMAT_ASTC_10x10_SRGB_BLOCK)       \
    X(wgpu::TextureFormat::ASTC12x10Unorm, VK_FORMAT_ASTC_12x10_UNORM_BLOCK)          \
    X(wgpu::TextureFormat::ASTC12x10UnormSrgb, VK_FORMAT_ASTC_12x10_SRGB_BLOCK)       \
    X(wgpu::TextureFormat::ASTC12x12Unorm, VK_FORMAT_ASTC_12x12_UNORM_BLOCK)          \
    X(wgpu::TextureFormat::ASTC12x12UnormSrgb, VK_FORMAT_ASTC_12x12_SRGB_BLOCK)       \
                                                                                      \
    X(wgpu::TextureFormat::R8BG8Biplanar420Unorm, VK_FORMAT_G8_B8R8_2PLANE_420_UNORM) \
    X(wgpu::TextureFormat::R8BG8Biplanar422Unorm, VK_FORMAT_G8_B8R8_2PLANE_422_UNORM) \
    X(wgpu::TextureFormat::R8BG8Biplanar444Unorm, VK_FORMAT_G8_B8R8_2PLANE_444_UNORM) \
    X(wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm,                               \
      VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16)                            \
    X(wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm,                               \
      VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16)                            \
    X(wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm,                               \
      VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16)

// Converts Dawn texture format to Vulkan formats.
VkFormat VulkanImageFormat(const Device* device, wgpu::TextureFormat format) {
    switch (format) {
#define X(wgpuFormat, vkFormat) \
    case wgpuFormat:            \
        return vkFormat;
        SIMPLE_FORMAT_MAPPING(X)
#undef X
        case wgpu::TextureFormat::Depth24PlusStencil8:
            // Depth24PlusStencil8 maps to either of these two formats because only requires
            // that one of the two be present. The VulkanUseD32S8 toggle combines the wish of
            // the environment, default to using D32S8, and availability information so we know
            // that the format is available.
            if (device->IsToggleEnabled(Toggle::VulkanUseD32S8)) {
                return VK_FORMAT_D32_SFLOAT_S8_UINT;
            } else {
                return VK_FORMAT_D24_UNORM_S8_UINT;
            }

        case wgpu::TextureFormat::Depth24Plus:
            return VK_FORMAT_D32_SFLOAT;

        case wgpu::TextureFormat::Stencil8:
            // Try to use the stencil8 format if possible, otherwise use whatever format we can
            // use that contains a stencil8 component.
            if (device->IsToggleEnabled(Toggle::VulkanUseS8)) {
                return VK_FORMAT_S8_UINT;
            } else {
                return VulkanImageFormat(device, wgpu::TextureFormat::Depth24PlusStencil8);
            }

        case wgpu::TextureFormat::External:
            // The VkFormat is Undefined when TextureFormat::External is passed for YCbCr samplers.
            return VK_FORMAT_UNDEFINED;

        // R8BG8A8Triplanar420Unorm format is only supported on macOS.
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
        case wgpu::TextureFormat::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

// Converts Dawn texture format to Vulkan formats.
VkFormat ColorVulkanImageFormat(wgpu::TextureFormat format) {
    switch (format) {
#define X(wgpuFormat, vkFormat) \
    case wgpuFormat:            \
        return vkFormat;
        SIMPLE_FORMAT_MAPPING(X)
#undef X
        default:
            return VK_FORMAT_UNDEFINED;
    }
    DAWN_UNREACHABLE();
}

ResultOrError<wgpu::TextureFormat> FormatFromVkFormat(const Device* device, VkFormat vkFormat) {
    switch (vkFormat) {
#define X(wgpuFormat, vkFormat) \
    case vkFormat:              \
        return wgpuFormat;
        SIMPLE_FORMAT_MAPPING(X)
#undef X
        case VK_FORMAT_S8_UINT:
            if (device->IsToggleEnabled(Toggle::VulkanUseS8)) {
                return wgpu::TextureFormat::Stencil8;
            }
            break;

        case VK_FORMAT_D24_UNORM_S8_UINT:
            if (!device->IsToggleEnabled(Toggle::VulkanUseD32S8)) {
                return wgpu::TextureFormat::Depth24PlusStencil8;
            }
            break;

        default:
            break;
    }
    return DAWN_VALIDATION_ERROR("Unsupported VkFormat %x", vkFormat);
}

#undef SIMPLE_FORMAT_MAPPING

// Converts the Dawn usage flags to Vulkan usage flags. Also needs the format to choose
// between color and depth attachment usages.
VkImageUsageFlags VulkanImageUsage(const DeviceBase* device,
                                   wgpu::TextureUsage usage,
                                   const Format& format) {
    VkImageUsageFlags flags = 0;

    if (usage & wgpu::TextureUsage::CopySrc) {
        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (usage & wgpu::TextureUsage::CopyDst) {
        flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if (usage & wgpu::TextureUsage::TextureBinding) {
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
        // If the sampled texture is a depth/stencil texture, its image layout will be set
        // to DEPTH_STENCIL_READ_ONLY_OPTIMAL in order to support readonly depth/stencil
        // attachment. That layout requires DEPTH_STENCIL_ATTACHMENT_BIT image usage.
        if (format.HasDepthOrStencil() && format.isRenderable) {
            flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
    }
    if (usage & wgpu::TextureUsage::StorageBinding) {
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if (usage & wgpu::TextureUsage::RenderAttachment) {
        if (format.HasDepthOrStencil()) {
            flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        } else {
            flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            if (!format.IsMultiPlanar() && (usage & wgpu::TextureUsage::TextureBinding) &&
                device->HasFeature(Feature::DawnLoadResolveTexture)) {
                // Automatically set "input attachment" usage so that the texture would be
                // used in ExpandResolveTexture subpass.
                flags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
            }
        }
    }

    // Choosing Vulkan image usages should not know about kReadOnlyRenderAttachment because that's
    // a property of when the image is used, not of the creation.
    DAWN_ASSERT(!(usage & kReadOnlyRenderAttachment));

    return flags;
}

// Chooses which Vulkan image layout should be used for the given Dawn usage. Note that this
// layout must match the layout given to various Vulkan operations as well as the layout given
// to descriptor set writes.
VkImageLayout VulkanImageLayout(const Format& format, wgpu::TextureUsage usage) {
    if (usage == wgpu::TextureUsage::None) {
        return VK_IMAGE_LAYOUT_UNDEFINED;
    }

    if (!wgpu::HasZeroOrOneBits(usage)) {
        // sampled | (some sort of readonly depth-stencil aspect) is the only possible multi-bit
        // usage, if more appear we will need additional special-casing.
        DAWN_ASSERT(IsSubset(
            usage, wgpu::TextureUsage::TextureBinding | kDepthReadOnlyStencilWritableAttachment |
                       kDepthWritableStencilReadOnlyAttachment | kReadOnlyRenderAttachment));

        if (IsSubset(kDepthReadOnlyStencilWritableAttachment, usage)) {
            return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
        } else if (IsSubset(kDepthWritableStencilReadOnlyAttachment, usage)) {
            return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
        } else {
            DAWN_ASSERT(
                IsSubset(usage, kReadOnlyRenderAttachment | wgpu::TextureUsage::TextureBinding));
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        }
    }

    // Usage has a single bit so we can switch on its value directly.
    switch (usage) {
        case wgpu::TextureUsage::CopyDst:
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

            // The layout returned here is the one that will be used at bindgroup creation time.
        case wgpu::TextureUsage::TextureBinding:
        case kResolveAttachmentLoadingUsage:
            // The sampled image can be used as a readonly depth/stencil attachment at the same
            // time if it is a depth/stencil renderable format, so the image layout need to be
            // VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL.
            if (format.HasDepthOrStencil() && format.isRenderable) {
                return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            }
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            // Vulkan texture copy functions require the image to be in _one_  known layout.
            // Depending on whether parts of the texture have been transitioned to only CopySrc
            // or a combination with something else, the texture could be in a combination of
            // GENERAL and TRANSFER_SRC_OPTIMAL. This would be a problem, so we make CopySrc use
            // GENERAL.
            // TODO(crbug.com/dawn/851): We no longer need to transition resources all at
            // once and can instead track subresources so we should lift this limitation.
        case wgpu::TextureUsage::CopySrc:
            // Read-only and write-only storage textures must use general layout because load
            // and store operations on storage images can only be done on the images in
            // VK_IMAGE_LAYOUT_GENERAL layout.
        case wgpu::TextureUsage::StorageBinding:
        case kReadOnlyStorageTexture:
        case kWriteOnlyStorageTexture:
            return VK_IMAGE_LAYOUT_GENERAL;

        case wgpu::TextureUsage::RenderAttachment:
            if (format.HasDepthOrStencil()) {
                return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            } else {
                return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }

        case kReadOnlyRenderAttachment:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        case kPresentReleaseTextureUsage:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        case kPresentAcquireTextureUsage:
            // We always consider images being acquired from the swapchain as uninitialized,
            // so we can use the UNDEFINED Vulkan image layout.
            return VK_IMAGE_LAYOUT_UNDEFINED;

        case wgpu::TextureUsage::TransientAttachment:
            // Will be covered by RenderAttachment above, as specification of
            // TransientAttachment requires that RenderAttachment also be
            // specified.
            DAWN_UNREACHABLE();
            break;

        case wgpu::TextureUsage::StorageAttachment:
            // TODO(dawn:1704): Support PLS on Vulkan.
            DAWN_UNREACHABLE();

        case wgpu::TextureUsage::None:
            break;
    }
    DAWN_UNREACHABLE();
}

VkImageLayout VulkanImageLayoutForDepthStencilAttachment(const Format& format,
                                                         bool depthReadOnly,
                                                         bool stencilReadOnly) {
    wgpu::TextureUsage depth = wgpu::TextureUsage::None;
    if (format.HasDepth()) {
        depth = depthReadOnly ? kReadOnlyRenderAttachment : wgpu::TextureUsage::RenderAttachment;
    }

    wgpu::TextureUsage stencil = wgpu::TextureUsage::None;
    if (format.HasStencil()) {
        stencil =
            stencilReadOnly ? kReadOnlyRenderAttachment : wgpu::TextureUsage::RenderAttachment;
    }

    return VulkanImageLayout(format, MergeDepthStencilUsage(depth, stencil));
}

VkSampleCountFlagBits VulkanSampleCount(uint32_t sampleCount) {
    switch (sampleCount) {
        case 1:
            return VK_SAMPLE_COUNT_1_BIT;
        case 4:
            return VK_SAMPLE_COUNT_4_BIT;
    }
    DAWN_UNREACHABLE();
}

MaybeError ValidateVulkanImageCanBeWrapped(const DeviceBase*,
                                           const UnpackedPtr<TextureDescriptor>& descriptor) {
    DAWN_INVALID_IF(descriptor->dimension != wgpu::TextureDimension::e2D,
                    "Texture dimension (%s) is not %s.", descriptor->dimension,
                    wgpu::TextureDimension::e2D);

    DAWN_INVALID_IF(descriptor->mipLevelCount != 1, "Mip level count (%u) is not 1.",
                    descriptor->mipLevelCount);

    DAWN_INVALID_IF(descriptor->size.depthOrArrayLayers != 1, "Array layer count (%u) is not 1.",
                    descriptor->size.depthOrArrayLayers);

    DAWN_INVALID_IF(descriptor->sampleCount != 1, "Sample count (%u) is not 1.",
                    descriptor->sampleCount);

    return {};
}

bool IsSampleCountSupported(const dawn::native::vulkan::Device* device,
                            const VkImageCreateInfo& imageCreateInfo) {
    DAWN_ASSERT(device);

    VkPhysicalDevice vkPhysicalDevice =
        ToBackend(device->GetPhysicalDevice())->GetVkPhysicalDevice();
    VkImageFormatProperties properties;
    if (device->fn.GetPhysicalDeviceImageFormatProperties(
            vkPhysicalDevice, imageCreateInfo.format, imageCreateInfo.imageType,
            imageCreateInfo.tiling, imageCreateInfo.usage, imageCreateInfo.flags,
            &properties) != VK_SUCCESS) {
        DAWN_UNREACHABLE();
    }

    return properties.sampleCounts & imageCreateInfo.samples;
}

Texture::Texture(Device* device, const UnpackedPtr<TextureDescriptor>& descriptor)
    : TextureBase(device, descriptor),
      mCombinedAspect(ComputeCombinedAspect(device, GetFormat())),
      // A usage of none will make sure the texture is transitioned before its first use as
      // required by the Vulkan spec.
      mSubresourceLastSyncInfos(
          mCombinedAspect != Aspect::None ? mCombinedAspect : GetFormat().aspects,
          GetArrayLayers(),
          GetNumMipLevels(),
          TextureSyncInfo{wgpu::TextureUsage::None, wgpu::ShaderStage::None}) {}

void Texture::SetLabelHelper(const char* prefix) {
    SetDebugName(ToBackend(GetDevice()), mHandle, prefix, GetLabel());
}

void Texture::SetLabelImpl() {
    SetLabelHelper("Dawn_InternalTexture");
}

void Texture::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the texture is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the texture.
    // - It may be called when the last ref to the texture is dropped and the texture
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the texture since there are no other live refs.
    mHandle = VK_NULL_HANDLE;

    TextureBase::DestroyImpl();
}

VkImage Texture::GetHandle() const {
    return mHandle;
}

bool Texture::CanReuseWithoutBarrier(wgpu::TextureUsage lastUsage,
                                     wgpu::TextureUsage usage,
                                     wgpu::ShaderStage lastShaderStage,
                                     wgpu::ShaderStage shaderStage) {
    // Reuse the texture directly and avoid encoding barriers when it isn't needed.
    bool lastReadOnly = IsSubset(lastUsage, kReadOnlyTextureUsages);
    return lastReadOnly && lastUsage == usage && IsSubset(shaderStage, lastShaderStage);
}

void Texture::TransitionUsageForPass(CommandRecordingContext* recordingContext,
                                     const TextureSubresourceSyncInfo& textureSyncInfos,
                                     std::vector<VkImageMemoryBarrier>* imageBarriers,
                                     VkPipelineStageFlags* srcStages,
                                     VkPipelineStageFlags* dstStages) {
    if (!UseCombinedAspects()) {
        TransitionUsageForPassImpl(recordingContext, textureSyncInfos, imageBarriers, srcStages,
                                   dstStages);
        return;
    }
    // We need to combine aspects for the transition, use a new subresource storage that will
    // contain the combined usages for the aspects.
    SubresourceStorage<TextureSyncInfo> combinedUsages(mCombinedAspect, GetArrayLayers(),
                                                       GetNumMipLevels());
    if (mCombinedAspect == Aspect::CombinedDepthStencil) {
        // For depth-stencil we can't just combine the aspect with an | operation because there
        // needs to be special handling for readonly aspects. Instead figure out which aspect is
        // currently being added (and which one is already present) and call the custom merging
        // function for depth-stencil.
        textureSyncInfos.Iterate([&](const SubresourceRange& range, TextureSyncInfo syncInfo) {
            SubresourceRange updateRange = range;
            updateRange.aspects = mCombinedAspect;
            Aspect aspectsToMerge = range.aspects;
            combinedUsages.Update(
                updateRange, [&](const SubresourceRange&, TextureSyncInfo* combinedInfo) {
                    if (aspectsToMerge == Aspect::Depth) {
                        combinedInfo->usage =
                            MergeDepthStencilUsage(syncInfo.usage, combinedInfo->usage);
                    } else if (aspectsToMerge == Aspect::Stencil) {
                        combinedInfo->usage =
                            MergeDepthStencilUsage(combinedInfo->usage, syncInfo.usage);
                    } else {
                        DAWN_ASSERT(aspectsToMerge == (Aspect::Depth | Aspect::Stencil));
                        combinedInfo->usage = syncInfo.usage;
                    }
                    combinedInfo->shaderStages |= syncInfo.shaderStages;
                });
        });
    } else {
        // Combine aspect's usages with the | operation.
        textureSyncInfos.Iterate([&](const SubresourceRange& range, TextureSyncInfo syncInfo) {
            SubresourceRange updateRange = range;
            updateRange.aspects = mCombinedAspect;
            combinedUsages.Update(updateRange,
                                  [&](const SubresourceRange&, TextureSyncInfo* combinedInfo) {
                                      combinedInfo->usage |= syncInfo.usage;
                                      combinedInfo->shaderStages |= syncInfo.shaderStages;
                                  });
        });
    }
    TransitionUsageForPassImpl(recordingContext, combinedUsages, imageBarriers, srcStages,
                               dstStages);
}

void Texture::TransitionUsageForPassImpl(
    CommandRecordingContext* recordingContext,
    const SubresourceStorage<TextureSyncInfo>& subresourceSyncInfos,
    std::vector<VkImageMemoryBarrier>* imageBarriers,
    VkPipelineStageFlags* srcStages,
    VkPipelineStageFlags* dstStages) {
    size_t transitionBarrierStart = imageBarriers->size();
    const Format& format = GetFormat();

    wgpu::TextureUsage allNewUsages = wgpu::TextureUsage::None;
    wgpu::TextureUsage allLastUsages = wgpu::TextureUsage::None;

    wgpu::ShaderStage allNewShaderStages = wgpu::ShaderStage::None;
    wgpu::ShaderStage allLastShaderStages = wgpu::ShaderStage::None;

    mSubresourceLastSyncInfos.Merge(subresourceSyncInfos, [&](const SubresourceRange& range,
                                                              TextureSyncInfo* lastSyncInfo,
                                                              const TextureSyncInfo& newSyncInfo) {
        wgpu::TextureUsage newUsage = newSyncInfo.usage;
        if (newSyncInfo.shaderStages == wgpu::ShaderStage::None) {
            // If the image isn't used in any shader stages, ignore shader usages. Eg. ignore a
            // texture binding that isn't actually sampled in any shader.
            newUsage &= ~kShaderTextureUsages;
        }

        if (newUsage == wgpu::TextureUsage::None ||
            CanReuseWithoutBarrier(lastSyncInfo->usage, newUsage, lastSyncInfo->shaderStages,
                                   newSyncInfo.shaderStages)) {
            return;
        }

        imageBarriers->push_back(BuildMemoryBarrier(this, lastSyncInfo->usage, newUsage, range));

        allLastUsages |= lastSyncInfo->usage;
        allNewUsages |= newUsage;

        allLastShaderStages |= lastSyncInfo->shaderStages;
        allNewShaderStages |= newSyncInfo.shaderStages;

        if (lastSyncInfo->usage == newUsage &&
            IsSubset(lastSyncInfo->usage, kReadOnlyTextureUsages)) {
            // Read only usage and no layout transition. We can keep previous shader stages so
            // future uses in those stages don't insert barriers.
            lastSyncInfo->shaderStages |= newSyncInfo.shaderStages;
        } else {
            // Image was altered by write or layout transition. We need to clear previous shader
            // stages so future uses in those stages will insert barriers.
            lastSyncInfo->shaderStages = newSyncInfo.shaderStages;
        }
        lastSyncInfo->usage = newUsage;
    });

    TweakTransition(recordingContext, imageBarriers, transitionBarrierStart);

    // Skip adding pipeline stages if no barrier was needed to avoid putting TOP_OF_PIPE in the
    // destination stages.
    if (allNewUsages != wgpu::TextureUsage::None) {
        *srcStages |= VulkanPipelineStage(allLastUsages, allLastShaderStages, format);
        *dstStages |= VulkanPipelineStage(allNewUsages, allNewShaderStages, format);
    }
}

void Texture::TransitionUsageNow(CommandRecordingContext* recordingContext,
                                 wgpu::TextureUsage usage,
                                 wgpu::ShaderStage shaderStages,
                                 const SubresourceRange& range) {
    std::vector<VkImageMemoryBarrier> barriers;

    VkPipelineStageFlags srcStages = 0;
    VkPipelineStageFlags dstStages = 0;

    TransitionUsageAndGetResourceBarrier(usage, shaderStages, range, &barriers, &srcStages,
                                         &dstStages);

    TweakTransition(recordingContext, &barriers, 0);

    if (!barriers.empty()) {
        DAWN_ASSERT(srcStages != 0 && dstStages != 0);
        ToBackend(GetDevice())
            ->fn.CmdPipelineBarrier(recordingContext->commandBuffer, srcStages, dstStages, 0, 0,
                                    nullptr, 0, nullptr, barriers.size(), barriers.data());
    }
}

void Texture::UpdateUsage(wgpu::TextureUsage usage,
                          wgpu::ShaderStage shaderStages,
                          const SubresourceRange& range) {
    std::vector<VkImageMemoryBarrier> barriers;

    VkPipelineStageFlags srcStages = 0;
    VkPipelineStageFlags dstStages = 0;

    TransitionUsageAndGetResourceBarrier(usage, shaderStages, range, &barriers, &srcStages,
                                         &dstStages);

    // barriers are ignored.
}

void Texture::TransitionUsageAndGetResourceBarrier(wgpu::TextureUsage usage,
                                                   wgpu::ShaderStage shaderStages,
                                                   const SubresourceRange& range,
                                                   std::vector<VkImageMemoryBarrier>* imageBarriers,
                                                   VkPipelineStageFlags* srcStages,
                                                   VkPipelineStageFlags* dstStages) {
    if (UseCombinedAspects()) {
        SubresourceRange updatedRange = range;
        updatedRange.aspects = mCombinedAspect;
        TransitionUsageAndGetResourceBarrierImpl(usage, shaderStages, updatedRange, imageBarriers,
                                                 srcStages, dstStages);
    } else {
        TransitionUsageAndGetResourceBarrierImpl(usage, shaderStages, range, imageBarriers,
                                                 srcStages, dstStages);
    }
}

void Texture::TransitionUsageAndGetResourceBarrierImpl(
    wgpu::TextureUsage usage,
    wgpu::ShaderStage shaderStages,
    const SubresourceRange& range,
    std::vector<VkImageMemoryBarrier>* imageBarriers,
    VkPipelineStageFlags* srcStages,
    VkPipelineStageFlags* dstStages) {
    DAWN_ASSERT(imageBarriers != nullptr);
    const Format& format = GetFormat();

    if (shaderStages == wgpu::ShaderStage::None) {
        // If the image isn't used in any shader stages, ignore shader usages. Eg. ignore a texture
        // binding that isn't actually sampled in any shader.
        usage &= ~kShaderTextureUsages;
    }

    wgpu::TextureUsage allLastUsages = wgpu::TextureUsage::None;
    wgpu::ShaderStage allLastShaderStages = wgpu::ShaderStage::None;
    mSubresourceLastSyncInfos.Update(
        range, [&](const SubresourceRange& range, TextureSyncInfo* lastSyncInfo) {
            if (CanReuseWithoutBarrier(lastSyncInfo->usage, usage, lastSyncInfo->shaderStages,
                                       shaderStages)) {
                return;
            }

            imageBarriers->push_back(BuildMemoryBarrier(this, lastSyncInfo->usage, usage, range));

            allLastUsages |= lastSyncInfo->usage;
            allLastShaderStages |= lastSyncInfo->shaderStages;

            if (lastSyncInfo->usage == usage && IsSubset(usage, kReadOnlyTextureUsages)) {
                // Read only usage and no layout transition. We can keep previous shader stages so
                // future uses in those stages don't insert barriers.
                lastSyncInfo->shaderStages |= shaderStages;
            } else {
                // Image was altered by write or layout transition. We need to clear previous shader
                // stages so future uses in those stages will insert barriers.
                lastSyncInfo->shaderStages = shaderStages;
            }
            lastSyncInfo->usage = usage;
        });

    *srcStages |= VulkanPipelineStage(allLastUsages, allLastShaderStages, format);
    *dstStages |= VulkanPipelineStage(usage, shaderStages, format);
}

MaybeError Texture::ClearTexture(CommandRecordingContext* recordingContext,
                                 const SubresourceRange& range,
                                 TextureBase::ClearValue clearValue) {
    Device* device = ToBackend(GetDevice());

    const bool isZero = clearValue == TextureBase::ClearValue::Zero;
    uint32_t uClearColor = isZero ? 0 : 1;
    float fClearColor = isZero ? 0.f : 1.f;

    VkImageSubresourceRange imageRange = {};
    imageRange.levelCount = 1;
    imageRange.layerCount = 1;

    if ((GetInternalUsage() & wgpu::TextureUsage::RenderAttachment) && GetFormat().IsColor() &&
        !GetFormat().IsMultiPlanar()) {
        TransitionUsageNow(recordingContext, wgpu::TextureUsage::RenderAttachment,
                           wgpu::ShaderStage::None, range);

        for (uint32_t level = range.baseMipLevel; level < range.baseMipLevel + range.levelCount;
             ++level) {
            for (uint32_t layer = range.baseArrayLayer;
                 layer < range.baseArrayLayer + range.layerCount; ++layer) {
                Aspect aspects = Aspect::None;
                for (Aspect aspect : IterateEnumMask(range.aspects)) {
                    if (clearValue == TextureBase::ClearValue::Zero &&
                        IsSubresourceContentInitialized(
                            SubresourceRange::SingleMipAndLayer(level, layer, aspect))) {
                        // Skip lazy clears if already initialized.
                        continue;
                    }
                    aspects |= aspect;
                }

                if (aspects == Aspect::None) {
                    continue;
                }

                Extent3D mipSize = GetMipLevelSingleSubresourcePhysicalSize(level, aspects);
                BeginRenderPassCmd beginCmd{};
                beginCmd.width = mipSize.width;
                beginCmd.height = mipSize.height;

                TextureViewDescriptor viewDesc = {};
                viewDesc.format = GetFormat().format;
                viewDesc.dimension = wgpu::TextureViewDimension::e2D;
                viewDesc.baseMipLevel = level;
                viewDesc.mipLevelCount = 1u;
                viewDesc.baseArrayLayer = layer;
                viewDesc.arrayLayerCount = 1u;
                viewDesc.usage = wgpu::TextureUsage::RenderAttachment;

                ColorAttachmentIndex ca0(uint8_t(0));
                DAWN_TRY_ASSIGN(beginCmd.colorAttachments[ca0].view,
                                TextureView::Create(this, Unpack(&viewDesc)));

                RenderPassColorAttachment colorAttachment{};
                colorAttachment.view = beginCmd.colorAttachments[ca0].view.Get();
                beginCmd.colorAttachments[ca0].clearColor = colorAttachment.clearValue = {
                    fClearColor, fClearColor, fClearColor, fClearColor};
                beginCmd.colorAttachments[ca0].loadOp = colorAttachment.loadOp =
                    wgpu::LoadOp::Clear;
                beginCmd.colorAttachments[ca0].storeOp = colorAttachment.storeOp =
                    wgpu::StoreOp::Store;

                RenderPassDescriptor passDesc{};
                passDesc.colorAttachmentCount = 1u;
                passDesc.colorAttachments = &colorAttachment;
                beginCmd.attachmentState = device->GetOrCreateAttachmentState(Unpack(&passDesc));

                DAWN_TRY(
                    RecordBeginRenderPass(recordingContext, ToBackend(GetDevice()), &beginCmd));
                ToBackend(GetDevice())->fn.CmdEndRenderPass(recordingContext->commandBuffer);
            }
        }
    } else if (GetFormat().HasDepthOrStencil()) {
        TransitionUsageNow(recordingContext, wgpu::TextureUsage::CopyDst, wgpu::ShaderStage::None,
                           range);

        for (uint32_t level = range.baseMipLevel; level < range.baseMipLevel + range.levelCount;
             ++level) {
            imageRange.baseMipLevel = level;
            for (uint32_t layer = range.baseArrayLayer;
                 layer < range.baseArrayLayer + range.layerCount; ++layer) {
                Aspect aspects = Aspect::None;
                for (Aspect aspect : IterateEnumMask(range.aspects)) {
                    if (clearValue == TextureBase::ClearValue::Zero &&
                        IsSubresourceContentInitialized(
                            SubresourceRange::SingleMipAndLayer(level, layer, aspect))) {
                        // Skip lazy clears if already initialized.
                        continue;
                    }
                    aspects |= aspect;
                }

                if (aspects == Aspect::None) {
                    continue;
                }

                imageRange.aspectMask = VulkanAspectMask(aspects);
                imageRange.baseMipLevel = level;
                imageRange.baseArrayLayer = layer;

                VkClearDepthStencilValue clearDepthStencilValue[1];
                clearDepthStencilValue[0].depth = fClearColor;
                clearDepthStencilValue[0].stencil = uClearColor;
                device->fn.CmdClearDepthStencilImage(recordingContext->commandBuffer, GetHandle(),
                                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                     clearDepthStencilValue, 1, &imageRange);
            }
        }
    } else {
        if (range.aspects == Aspect::None) {
            return {};
        }

        TransitionUsageNow(recordingContext, wgpu::TextureUsage::CopyDst, wgpu::ShaderStage::None,
                           range);

        // need to clear the texture with a copy from buffer
        DAWN_ASSERT(range.aspects == Aspect::Color || range.aspects == Aspect::Plane0 ||
                    range.aspects == Aspect::Plane1 || range.aspects == Aspect::Plane2);
        const TexelBlockInfo& blockInfo = GetFormat().GetAspectInfo(range.aspects).block;

        Extent3D largestMipSize =
            GetMipLevelSingleSubresourcePhysicalSize(range.baseMipLevel, range.aspects);

        uint32_t bytesPerRow = Align((largestMipSize.width / blockInfo.width) * blockInfo.byteSize,
                                     device->GetOptimalBytesPerRowAlignment());
        uint64_t uploadSize = bytesPerRow * (largestMipSize.height / blockInfo.height) *
                              largestMipSize.depthOrArrayLayers;

        DAWN_TRY(device->GetDynamicUploader()->WithUploadReservation(
            uploadSize, blockInfo.byteSize, [&](UploadReservation reservation) -> MaybeError {
                memset(reservation.mappedPointer, uClearColor, uploadSize);

                std::vector<VkBufferImageCopy> regions;
                for (uint32_t level = range.baseMipLevel;
                     level < range.baseMipLevel + range.levelCount; ++level) {
                    Extent3D copySize =
                        GetMipLevelSingleSubresourcePhysicalSize(level, range.aspects);
                    imageRange.baseMipLevel = level;
                    for (uint32_t layer = range.baseArrayLayer;
                         layer < range.baseArrayLayer + range.layerCount; ++layer) {
                        if (clearValue == TextureBase::ClearValue::Zero &&
                            IsSubresourceContentInitialized(
                                SubresourceRange::SingleMipAndLayer(level, layer, range.aspects))) {
                            // Skip lazy clears if already initialized.
                            continue;
                        }

                        TexelCopyBufferLayout dataLayout;
                        dataLayout.offset = reservation.offsetInBuffer;
                        dataLayout.rowsPerImage = copySize.height / blockInfo.height;
                        dataLayout.bytesPerRow = bytesPerRow;
                        TextureCopy textureCopy;
                        textureCopy.aspect = range.aspects;
                        textureCopy.mipLevel = level;
                        textureCopy.origin = {0, 0, layer};
                        textureCopy.texture = this;

                        regions.push_back(
                            ComputeBufferImageCopyRegion(dataLayout, textureCopy, copySize));
                    }
                }

                device->fn.CmdCopyBufferToImage(recordingContext->commandBuffer,
                                                ToBackend(reservation.buffer)->GetHandle(),
                                                GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                regions.size(), regions.data());
                return {};
            }));
    }

    if (clearValue == TextureBase::ClearValue::Zero) {
        SetIsSubresourceContentInitialized(true, range);
        device->IncrementLazyClearCountForTesting();
    }
    return {};
}

MaybeError Texture::EnsureSubresourceContentInitialized(CommandRecordingContext* recordingContext,
                                                        const SubresourceRange& range) {
    if (!GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
        return {};
    }
    if (!IsSubresourceContentInitialized(range)) {
        // If subresource has not been initialized, clear it to black as it could contain dirty
        // bits from recycled memory
        DAWN_TRY(ClearTexture(recordingContext, range, TextureBase::ClearValue::Zero));
    }
    return {};
}

VkImageLayout Texture::GetCurrentLayout(Aspect aspect,
                                        uint32_t arrayLayer,
                                        uint32_t mipLevel) const {
    DAWN_ASSERT(GetFormat().aspects == Aspect::Color);
    return VulkanImageLayout(GetFormat(),
                             mSubresourceLastSyncInfos.Get(aspect, arrayLayer, mipLevel).usage);
}

bool Texture::UseCombinedAspects() const {
    return mCombinedAspect != Aspect::None;
}

Aspect Texture::GetDisjointVulkanAspects() const {
    if (UseCombinedAspects()) {
        return mCombinedAspect;
    }
    return GetFormat().aspects;
}

void Texture::TweakTransition(CommandRecordingContext* recordingContext,
                              std::vector<VkImageMemoryBarrier>* barriers,
                              size_t transitionBarrierStart) {}

MaybeError Texture::OnBeforeSubmit(CommandRecordingContext*) {
    DAWN_UNREACHABLE();
}
MaybeError Texture::OnAfterSubmit() {
    DAWN_UNREACHABLE();
}

//
// InternalTexture
//

// static
ResultOrError<Ref<InternalTexture>> InternalTexture::Create(
    Device* device,
    const UnpackedPtr<TextureDescriptor>& descriptor,
    VkImageUsageFlags extraUsages) {
    Ref<InternalTexture> texture = AcquireRef(new InternalTexture(device, descriptor));
    DAWN_TRY(texture->Initialize(extraUsages));
    return std::move(texture);
}

MaybeError InternalTexture::Initialize(VkImageUsageFlags extraUsages) {
    Device* device = ToBackend(GetDevice());

    // If this triggers, it means it's time to add tests and implement support for readonly
    // depth-stencil attachments that are also used as readonly storage bindings in the pass.
    // Have fun! :)
    DAWN_ASSERT(
        !(GetFormat().HasDepthOrStencil() && (GetUsage() & wgpu::TextureUsage::StorageBinding)));

    // Create the Vulkan image "container". We don't need to check that the format supports the
    // combination of sample, usage etc. because validation should have been done in the Dawn
    // frontend already based on the minimum supported formats in the Vulkan spec
    VkImageCreateInfo createInfo = {};
    FillVulkanCreateInfoSizesAndType(*this, &createInfo);
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.format = VulkanImageFormat(device, GetFormat().format);
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.usage = VulkanImageUsage(device, GetInternalUsage(), GetFormat()) | extraUsages;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    std::vector<VkFormat> viewFormats;
    bool requiresViewFormatsList = GetViewFormats().any();
    // As current SPIR-V SPEC doesn't support 'bgra8' as a valid image format, to support the
    // STORAGE usage of BGRA8Unorm we have to create an RGBA8Unorm image view on the BGRA8Unorm
    // storage texture and polyfill it as RGBA8Unorm in Tint. See http://crbug.com/dawn/1641 for
    // more details.
    if (createInfo.format == VK_FORMAT_B8G8R8A8_UNORM &&
        createInfo.usage & VK_IMAGE_USAGE_STORAGE_BIT) {
        viewFormats.push_back(VK_FORMAT_R8G8B8A8_UNORM);
        requiresViewFormatsList = true;
    }
    if (GetFormat().IsMultiPlanar() || requiresViewFormatsList) {
        // Multi-planar image needs to have VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT in order to be able
        // to create per-plane view. See
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageCreateFlagBits.html
        //
        // Note: we cannot include R8 & RG8 in the viewFormats list of
        // G8_B8R8_2PLANE_420_UNORM. The Vulkan validation layer will disallow that.
        createInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    }

    // Add the view format list only when the usage does not have storage. Otherwise, the VVL will
    // say creation of the texture is invalid.
    // See https://github.com/gpuweb/gpuweb/issues/4426.
    VkImageFormatListCreateInfo imageFormatListInfo = {};
    PNextChainBuilder createInfoChain(&createInfo);
    if (requiresViewFormatsList && device->GetDeviceInfo().HasExt(DeviceExt::ImageFormatList) &&
        !(createInfo.usage & VK_IMAGE_USAGE_STORAGE_BIT)) {
        createInfoChain.Add(&imageFormatListInfo, VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO);
        viewFormats.push_back(VulkanImageFormat(device, GetFormat().format));
        for (FormatIndex i : IterateBitSet(GetViewFormats())) {
            const Format& viewFormat = device->GetValidInternalFormat(i);
            viewFormats.push_back(VulkanImageFormat(device, viewFormat.format));
        }

        imageFormatListInfo.viewFormatCount = viewFormats.size();
        imageFormatListInfo.pViewFormats = viewFormats.data();
    }

    DAWN_ASSERT(IsSampleCountSupported(device, createInfo));

    if (GetArrayLayers() >= 6 && GetBaseSize().width == GetBaseSize().height) {
        createInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    if (createInfo.imageType == VK_IMAGE_TYPE_3D &&
        createInfo.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
        createInfo.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    }

    // We always set VK_IMAGE_USAGE_TRANSFER_DST_BIT unconditionally because the Vulkan images
    // that are used in vkCmdClearColorImage() must have been created with this flag, which is
    // also required for the implementation of robust resource initialization.
    createInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    DAWN_TRY(CheckVkOOMThenSuccess(
        device->fn.CreateImage(device->GetVkDevice(), &createInfo, nullptr, &*mHandle),
        "CreateImage"));

    // Create the image memory and associate it with the container
    VkMemoryRequirements requirements;
    device->fn.GetImageMemoryRequirements(device->GetVkDevice(), mHandle, &requirements);

    bool forceDisableSubAllocation =
        (device->IsToggleEnabled(
            Toggle::DisableSubAllocationFor2DTextureWithCopyDstOrRenderAttachment)) &&
        GetDimension() == wgpu::TextureDimension::e2D &&
        (GetInternalUsage() & (wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment));
    auto memoryKind = (GetInternalUsage() & wgpu::TextureUsage::TransientAttachment)
                          ? MemoryKind::LazilyAllocated
                          : MemoryKind::DeviceLocal;
    DAWN_TRY_ASSIGN(mMemoryAllocation, device->GetResourceMemoryAllocator()->Allocate(
                                           requirements, memoryKind, forceDisableSubAllocation));

    DAWN_TRY(CheckVkSuccess(
        device->fn.BindImageMemory(device->GetVkDevice(), mHandle,
                                   ToBackend(mMemoryAllocation.GetResourceHeap())->GetMemory(),
                                   mMemoryAllocation.GetOffset()),
        "BindImageMemory"));

    // crbug.com/1361662
    // This works around an Intel Gen12 mesa bug due to CCS ambiguates stomping on each other.
    // https://gitlab.freedesktop.org/mesa/mesa/-/issues/7301#note_1826367
    if (device->IsToggleEnabled(Toggle::VulkanClearGen12TextureWithCCSAmbiguateOnCreation)) {
        auto format = GetFormat().format;
        bool textureIsBuggy =
            format == wgpu::TextureFormat::R8Unorm || format == wgpu::TextureFormat::R8Snorm ||
            format == wgpu::TextureFormat::R8Uint || format == wgpu::TextureFormat::R8Sint ||
            // These are flaky.
            format == wgpu::TextureFormat::RG16Sint || format == wgpu::TextureFormat::RGBA16Sint ||
            format == wgpu::TextureFormat::RGBA32Float;
        textureIsBuggy &= GetNumMipLevels() > 1;
        textureIsBuggy &= GetDimension() == wgpu::TextureDimension::e2D;
        textureIsBuggy &= IsPowerOfTwo(GetBaseSize().width) && IsPowerOfTwo(GetBaseSize().height);
        if (textureIsBuggy) {
            DAWN_TRY(ClearTexture(ToBackend(GetDevice()->GetQueue())->GetPendingRecordingContext(),
                                  GetAllSubresources(), TextureBase::ClearValue::Zero));
        }
    }

    if (device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
        DAWN_TRY(ClearTexture(ToBackend(GetDevice()->GetQueue())->GetPendingRecordingContext(),
                              GetAllSubresources(), TextureBase::ClearValue::NonZero));
    }

    SetLabelImpl();

    return {};
}

void InternalTexture::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the texture is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the texture.
    // - It may be called when the last ref to the texture is dropped and the texture
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the texture since there are no other live refs.
    Device* device = ToBackend(GetDevice());

    device->GetFencedDeleter()->DeleteWhenUnused(mHandle);
    mHandle = VK_NULL_HANDLE;

    device->GetResourceMemoryAllocator()->Deallocate(&mMemoryAllocation);
    mMemoryAllocation = ResourceMemoryAllocation();

    Texture::DestroyImpl();
}

//
// SwapChainTexture
//

// static
Ref<SwapChainTexture> SwapChainTexture::Create(Device* device,
                                               const UnpackedPtr<TextureDescriptor>& descriptor,
                                               VkImage nativeImage) {
    Ref<SwapChainTexture> texture = AcquireRef(new SwapChainTexture(device, descriptor));
    texture->Initialize(nativeImage);
    return texture;
}

void SwapChainTexture::Initialize(VkImage nativeImage) {
    mHandle = nativeImage;
    mSubresourceLastSyncInfos.Fill({kPresentAcquireTextureUsage, wgpu::ShaderStage::None});
    SetLabelHelper("Dawn_SwapChainTexture");
}

//
// ImportedTextureBase
//

ImportedTextureBase::~ImportedTextureBase() {
    // The external semaphore can be queried even after device loss or destroy (so applications can
    // keep relying on them for synchronization) so only destroy it in the destructor instead of
    // DestroyImpl.
    if (mExternalSemaphoreHandle != kNullExternalSemaphoreHandle) {
        ToBackend(GetDevice())
            ->GetExternalSemaphoreService()
            ->CloseHandle(mExternalSemaphoreHandle);
        mExternalSemaphoreHandle = kNullExternalSemaphoreHandle;
    }
}

void ImportedTextureBase::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the texture is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the texture.
    // - It may be called when the last ref to the texture is dropped and the texture
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the texture since there are no other live refs.
    if (mPendingSemaphore != VK_NULL_HANDLE) {
        ToBackend(GetDevice())->GetFencedDeleter()->DeleteWhenUnused(mPendingSemaphore);
        mPendingSemaphore = VK_NULL_HANDLE;
    }
    Texture::DestroyImpl();
}

void ImportedTextureBase::TransitionEagerlyForExport(CommandRecordingContext* recordingContext) {
    mExternalState = ExternalState::EagerlyTransitioned;

    // Get any usage, ideally the last one to do nothing
    DAWN_ASSERT(GetNumMipLevels() == 1 && GetArrayLayers() == 1);
    const SubresourceRange range = {GetDisjointVulkanAspects(), {0, 1}, {0, 1}};
    const TextureSyncInfo syncInfo = mSubresourceLastSyncInfos.Get(range.aspects, 0, 0);

    std::vector<VkImageMemoryBarrier> barriers;
    VkPipelineStageFlags srcStages = 0;
    VkPipelineStageFlags dstStages = 0;

    // Same usage as last.
    TransitionUsageAndGetResourceBarrier(syncInfo.usage, syncInfo.shaderStages, range, &barriers,
                                         &srcStages, &dstStages);

    DAWN_ASSERT(barriers.size() == 1);
    VkImageMemoryBarrier& barrier = barriers[0];
    // The barrier must be paired with another barrier that will specify the dst access mask on the
    // importing queue.
    barrier.dstAccessMask = 0;

    if (mDesiredExportLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
        barrier.newLayout = mDesiredExportLayout;
    }

    Device* device = ToBackend(GetDevice());
    barrier.srcQueueFamilyIndex = device->GetGraphicsQueueFamily();
    barrier.dstQueueFamilyIndex = mExportQueueFamilyIndex;

    // We don't know when the importing queue will need the texture, so pass
    // VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT to ensure the barrier happens-before any usage in the
    // importing queue.
    dstStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    device->fn.CmdPipelineBarrier(recordingContext->commandBuffer, srcStages, dstStages, 0, 0,
                                  nullptr, 0, nullptr, 1, &barrier);
}

bool ImportedTextureBase::CanReuseWithoutBarrier(wgpu::TextureUsage lastUsage,
                                                 wgpu::TextureUsage usage,
                                                 wgpu::ShaderStage lastShaderStage,
                                                 wgpu::ShaderStage shaderStage) {
    return mLastExternalState == mExternalState &&
           Texture::CanReuseWithoutBarrier(lastUsage, usage, lastShaderStage, shaderStage);
}

void ImportedTextureBase::TweakTransition(CommandRecordingContext* recordingContext,
                                          std::vector<VkImageMemoryBarrier>* barriers,
                                          size_t transitionBarrierStart) {
    DAWN_ASSERT(GetNumMipLevels() == 1 && GetArrayLayers() == 1);

    mLastSharedTextureMemoryUsageSerial = GetDevice()->GetQueue()->GetPendingCommandSerial();

    // transitionBarrierStart specify the index where barriers for current transition start in
    // the vector. barriers->size() - transitionBarrierStart is the number of barriers that we
    // have already added into the vector during current transition.
    DAWN_ASSERT(barriers->size() - transitionBarrierStart <= 1);

    if (mExternalState == ExternalState::PendingAcquire ||
        mExternalState == ExternalState::EagerlyTransitioned) {
        recordingContext->specialSyncTextures.insert(this);
        if (barriers->size() == transitionBarrierStart) {
            barriers->push_back(BuildMemoryBarrier(
                this, wgpu::TextureUsage::None, wgpu::TextureUsage::None,
                SubresourceRange::SingleMipAndLayer(0, 0, GetDisjointVulkanAspects())));
        }

        VkImageMemoryBarrier* barrier = &(*barriers)[transitionBarrierStart];
        // Transfer texture from external queue to graphics queue
        barrier->srcQueueFamilyIndex = mExportQueueFamilyIndex;
        barrier->dstQueueFamilyIndex = ToBackend(GetDevice())->GetGraphicsQueueFamily();

        // srcAccessMask means nothing when importing. Queue transfers require a barrier on
        // both the importing and exporting queues. The exporting queue should have specified
        // this.
        barrier->srcAccessMask = 0;

        // Save the desired layout. We may need to transition through an intermediate
        // |mPendingAcquireLayout| first.
        VkImageLayout desiredLayout = barrier->newLayout;

        if (mExternalState == ExternalState::PendingAcquire) {
            bool isInitialized = IsSubresourceContentInitialized(GetAllSubresources());

            // We don't care about the pending old layout if the texture is uninitialized. The
            // driver is free to discard it. Also it is invalid to transition to layout UNDEFINED or
            // PREINITIALIZED. If the embedder provided no new layout, or we don't care about the
            // previous contents, we can skip the layout transition.
            // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#VUID-VkImageMemoryBarrier-newLayout-01198
            if (!isInitialized || mPendingAcquireNewLayout == VK_IMAGE_LAYOUT_UNDEFINED ||
                mPendingAcquireNewLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
                barrier->oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier->newLayout = desiredLayout;
            } else {
                barrier->oldLayout = mPendingAcquireOldLayout;
                barrier->newLayout = mPendingAcquireNewLayout;
            }
        } else {
            // In case of ExternalState::EagerlyTransitioned, the layouts of the texture's queue
            // release were always same. So we exactly match that here for the queue acquire.
            // The spec text:
            // If the transfer is via an image memory barrier, and an image layout transition is
            // desired, then the values of oldLayout and newLayout in the release operation's memory
            // barrier must be equal to values of oldLayout and newLayout in the acquire operation's
            // memory barrier.
            barrier->newLayout = barrier->oldLayout;
        }

        // If these are unequal, we need an another barrier to transition the layout.
        if (barrier->newLayout != desiredLayout) {
            VkImageMemoryBarrier layoutBarrier;
            layoutBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            layoutBarrier.pNext = nullptr;
            layoutBarrier.image = GetHandle();
            layoutBarrier.subresourceRange = barrier->subresourceRange;

            // Transition from the acquired new layout to the desired layout.
            layoutBarrier.oldLayout = barrier->newLayout;
            layoutBarrier.newLayout = desiredLayout;

            layoutBarrier.srcAccessMask = 0;
            layoutBarrier.dstAccessMask = barrier->dstAccessMask;
            layoutBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            layoutBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            barriers->push_back(layoutBarrier);
        }

        mExternalState = ExternalState::Acquired;
    }

    mLastExternalState = mExternalState;
}

MaybeError ImportedTextureBase::EndAccess(ExternalSemaphoreHandle* handle,
                                          VkImageLayout* releasedOldLayout,
                                          VkImageLayout* releasedNewLayout) {
    DAWN_ASSERT(GetNumMipLevels() == 1 && GetArrayLayers() == 1);

    // Release the texture
    mExternalState = ExternalState::Released;

    // Compute the layouts for the queue transition for export. desiredLayout == UNDEFINED is a tag
    // value used to export with whatever the current layout is. However queue transitioning to the
    // UNDEFINED layout is disallowed so we handle the case where currentLayout is UNDEFINED by
    // promoting to GENERAL.
    VkImageLayout currentLayout = GetCurrentLayout(GetDisjointVulkanAspects());
    VkImageLayout targetLayout;
    if (currentLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
        targetLayout = currentLayout;
    } else {
        targetLayout = VK_IMAGE_LAYOUT_GENERAL;
    }

    // We have to manually trigger a transition if the texture hasn't been actually used or if we
    // need a layout transition.
    // TODO(dawn:1509): Avoid the empty submit.
    if (mExternalSemaphoreHandle == kNullExternalSemaphoreHandle || targetLayout != currentLayout) {
        mDesiredExportLayout = targetLayout;

        Queue* queue = ToBackend(GetDevice()->GetQueue());
        CommandRecordingContext* recordingContext = queue->GetPendingRecordingContext();
        recordingContext->specialSyncTextures.insert(this);
        DAWN_TRY(queue->SubmitPendingCommands());

        currentLayout = targetLayout;
    }
    DAWN_ASSERT(mExternalSemaphoreHandle != kNullExternalSemaphoreHandle);

    // Write out the layouts and signal semaphore
    *releasedOldLayout = currentLayout;
    *releasedNewLayout = targetLayout;
    *handle = mExternalSemaphoreHandle;
    mExternalSemaphoreHandle = kNullExternalSemaphoreHandle;
    return {};
}

MaybeError ImportedTextureBase::OnBeforeSubmit(CommandRecordingContext* context) {
    // On every submit using an exportable texture, we eagerly prepare for an export. This avoids
    // the need to schedule an almost empty submit just for exporting later. To export we both need
    // to transition the resource to export it, and need an exportable semaphore for future
    // synchronization.
    TransitionEagerlyForExport(context);

    // Create the external semaphore and add it to be signaled, but only mark it pending: if
    // anything fails during the submit we still keep the previously signaled exportable semaphore.
    // Note that VUID-VkSemaphoreGetFdInfoKHR-handleType-01135 requires that only signaled
    // semaphores be exported, so the export must be done in OnAfterSubmit.
    auto semaphoreService = ToBackend(GetDevice())->GetExternalSemaphoreService();
    DAWN_TRY_ASSIGN(mPendingSemaphore, semaphoreService->CreateExportableSemaphore());
    context->signalSemaphores.push_back(mPendingSemaphore);

    return {};
}

MaybeError ImportedTextureBase::OnAfterSubmit() {
    Device* device = ToBackend(GetDevice());

    // The submit succeeded, we can replace the previous external semaphore with the pending one.
    if (mExternalSemaphoreHandle != kNullExternalSemaphoreHandle) {
        device->GetExternalSemaphoreService()->CloseHandle(mExternalSemaphoreHandle);
    }

    DAWN_TRY_ASSIGN(mExternalSemaphoreHandle,
                    device->GetExternalSemaphoreService()->ExportSemaphore(mPendingSemaphore));
    ToBackend(GetDevice())->GetFencedDeleter()->DeleteWhenUnused(mPendingSemaphore);
    mPendingSemaphore = VK_NULL_HANDLE;

    return {};
}

//
// ExternalVkImageTexture
//

// static
ResultOrError<Ref<ExternalVkImageTexture>> ExternalVkImageTexture::Create(
    Device* device,
    const ExternalImageDescriptorVk* descriptor,
    const UnpackedPtr<TextureDescriptor>& textureDescriptor,
    external_memory::Service* externalMemoryService) {
    Ref<ExternalVkImageTexture> texture =
        AcquireRef(new ExternalVkImageTexture(device, textureDescriptor));
    DAWN_TRY(texture->Initialize(descriptor, externalMemoryService));
    return texture;
}

void ExternalVkImageTexture::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the texture is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the texture.
    // - It may be called when the last ref to the texture is dropped and the texture
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the texture since there are no other live refs.
    Device* device = ToBackend(GetDevice());

    device->GetFencedDeleter()->DeleteWhenUnused(mHandle);
    mHandle = VK_NULL_HANDLE;

    if (mExternalAllocation != VK_NULL_HANDLE) {
        device->GetFencedDeleter()->DeleteWhenUnused(mExternalAllocation);
        mExternalAllocation = VK_NULL_HANDLE;
    }

    ImportedTextureBase::DestroyImpl();
}

MaybeError ExternalVkImageTexture::Initialize(const ExternalImageDescriptorVk* descriptor,
                                              external_memory::Service* externalMemoryService) {
    Device* device = ToBackend(GetDevice());
    VkFormat format = VulkanImageFormat(device, GetFormat().format);
    VkImageUsageFlags usage = VulkanImageUsage(device, GetInternalUsage(), GetFormat());

    [[maybe_unused]] bool supportsDisjoint;
    DAWN_INVALID_IF(
        !externalMemoryService->SupportsCreateImage(descriptor, format, usage, &supportsDisjoint),
        "Creating an image from external memory is not supported.");
    // The creation of mSubresourceLastUsage assumes that multi-planar are always disjoint and sets
    // the combined aspect without checking for disjoint support.
    // TODO(dawn:1548): Support multi-planar images with the DISJOINT feature and potentially allow
    // acting on planes individually? Always using Color is valid even for disjoint images.
    DAWN_ASSERT(!GetFormat().IsMultiPlanar() || mCombinedAspect == Aspect::Color);

    mExternalState = ExternalState::PendingAcquire;
    mLastExternalState = ExternalState::Released;
    mExportQueueFamilyIndex = externalMemoryService->GetQueueFamilyIndex(descriptor->GetType());

    mPendingAcquireOldLayout = descriptor->releasedOldLayout;
    mPendingAcquireNewLayout = descriptor->releasedNewLayout;

    VkImageCreateInfo baseCreateInfo = {};
    FillVulkanCreateInfoSizesAndType(*this, &baseCreateInfo);
    baseCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    baseCreateInfo.format = format;
    baseCreateInfo.usage = usage;
    baseCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    baseCreateInfo.queueFamilyIndexCount = 0;
    baseCreateInfo.pQueueFamilyIndices = nullptr;

    // We always set VK_IMAGE_USAGE_TRANSFER_DST_BIT unconditionally because the Vulkan images
    // that are used in vkCmdClearColorImage() must have been created with this flag, which is
    // also required for the implementation of robust resource initialization.
    baseCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkImageFormatListCreateInfo imageFormatListInfo = {};
    PNextChainBuilder createInfoChain(&baseCreateInfo);
    std::vector<VkFormat> viewFormats;
    if (GetViewFormats().any()) {
        baseCreateInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
        if (device->GetDeviceInfo().HasExt(DeviceExt::ImageFormatList)) {
            createInfoChain.Add(&imageFormatListInfo,
                                VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO);
            for (FormatIndex i : IterateBitSet(GetViewFormats())) {
                const Format& viewFormat = device->GetValidInternalFormat(i);
                viewFormats.push_back(VulkanImageFormat(device, viewFormat.format));
            }

            imageFormatListInfo.viewFormatCount = viewFormats.size();
            imageFormatListInfo.pViewFormats = viewFormats.data();
        }
    }

    DAWN_TRY_ASSIGN(mHandle, externalMemoryService->CreateImage(descriptor, baseCreateInfo));
    SetLabelHelper("Dawn_ExternalVkImageTexture");

    return {};
}

MaybeError ExternalVkImageTexture::BindExternalMemory(const ExternalImageDescriptorVk* descriptor,
                                                      VkDeviceMemory externalMemoryAllocation,
                                                      std::vector<VkSemaphore> waitSemaphores) {
    Device* device = ToBackend(GetDevice());
    DAWN_TRY(CheckVkSuccess(
        device->fn.BindImageMemory(device->GetVkDevice(), mHandle, externalMemoryAllocation, 0),
        "BindImageMemory (external)"));

    // Don't clear imported texture if already initialized
    if (descriptor->isInitialized) {
        SetIsSubresourceContentInitialized(true, GetAllSubresources());
    }

    // Success, acquire all the external objects.
    mExternalAllocation = externalMemoryAllocation;
    mWaitRequirements = std::move(waitSemaphores);
    return {};
}

MaybeError ExternalVkImageTexture::ExportExternalTexture(VkImageLayout desiredLayout,
                                                         ExternalSemaphoreHandle* handle,
                                                         VkImageLayout* releasedOldLayout,
                                                         VkImageLayout* releasedNewLayout) {
    DAWN_INVALID_IF(mExternalState == ExternalState::Released,
                    "Can't export a signal semaphore from signaled texture %s.", this);

    DAWN_INVALID_IF(mExternalAllocation == VK_NULL_HANDLE,
                    "Can't export a signal semaphore from destroyed or non-external texture %s.",
                    this);

    DAWN_INVALID_IF(desiredLayout != VK_IMAGE_LAYOUT_UNDEFINED,
                    "desiredLayout (%d) was not VK_IMAGE_LAYOUT_UNDEFINED", desiredLayout);

    DAWN_TRY(EndAccess(handle, releasedOldLayout, releasedNewLayout));

    // Destroy the texture so it can't be used again
    Destroy();
    return {};
}

MaybeError ExternalVkImageTexture::OnBeforeSubmit(CommandRecordingContext* context) {
    context->waitSemaphores.insert(context->waitSemaphores.end(), mWaitRequirements.begin(),
                                   mWaitRequirements.end());
    mWaitRequirements.clear();
    return ImportedTextureBase::OnBeforeSubmit(context);
}

//
// SharedTexture
//

// static
ResultOrError<Ref<SharedTexture>> SharedTexture::Create(
    SharedTextureMemory* memory,
    const UnpackedPtr<TextureDescriptor>& textureDescriptor) {
    Ref<SharedTexture> texture =
        AcquireRef(new SharedTexture(ToBackend(memory->GetDevice()), textureDescriptor));
    texture->Initialize(memory);
    return texture;
}

void SharedTexture::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the texture is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the texture.
    // - It may be called when the last ref to the texture is dropped and the texture
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the texture since there are no other live refs.
    mSharedTextureMemoryObjects = {};

    ImportedTextureBase::DestroyImpl();
}

void SharedTexture::Initialize(SharedTextureMemory* memory) {
    mSharedResourceMemoryContents = memory->GetContents();
    mSharedTextureMemoryObjects = {memory->GetVkImage(), memory->GetVkDeviceMemory()};
    mHandle = mSharedTextureMemoryObjects.vkImage->Get();
    mExportQueueFamilyIndex = memory->GetQueueFamilyIndex();
}

void SharedTexture::SetPendingAcquire(VkImageLayout pendingAcquireOldLayout,
                                      VkImageLayout pendingAcquireNewLayout) {
    DAWN_ASSERT(GetSharedResourceMemoryContents() != nullptr);
    mExternalState = ExternalState::PendingAcquire;
    mLastExternalState = ExternalState::PendingAcquire;

    mPendingAcquireOldLayout = pendingAcquireOldLayout;
    mPendingAcquireNewLayout = pendingAcquireNewLayout;
}

MaybeError SharedTexture::OnBeforeSubmit(CommandRecordingContext* context) {
    Device* device = ToBackend(GetDevice());
    SharedResourceMemoryContents* contents = GetSharedResourceMemoryContents();

    SharedTextureMemoryBase::PendingFenceList fences;
    contents->AcquirePendingFences(&fences);

    for (const auto& fence : fences) {
        // All semaphores are binary semaphores.
        DAWN_ASSERT(fence.signaledValue == 1u);
        ExternalSemaphoreHandle semaphoreHandle = ToBackend(fence.object)->GetHandle().Get();

        VkSemaphore semaphore;
        DAWN_TRY_ASSIGN(semaphore,
                        device->GetExternalSemaphoreService()->ImportSemaphore(semaphoreHandle));
        context->waitSemaphores.push_back(semaphore);
    }

    return ImportedTextureBase::OnBeforeSubmit(context);
}

//
// TextureView
//

// static
ResultOrError<Ref<TextureView>> TextureView::Create(
    TextureBase* texture,
    const UnpackedPtr<TextureViewDescriptor>& descriptor) {
    Ref<TextureView> view = AcquireRef(new TextureView(texture, descriptor));
    DAWN_TRY(view->Initialize(descriptor));
    return view;
}

MaybeError TextureView::Initialize(const UnpackedPtr<TextureViewDescriptor>& descriptor) {
    if ((GetInternalUsage() & ~(wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst)) == 0) {
        // If the texture view has no other usage than CopySrc and CopyDst, then it can't
        // actually be used as a render pass attachment or sampled/storage texture. The Vulkan
        // validation errors warn if you create such a vkImageView, so return early.
        return {};
    }

    // Texture could be destroyed by the time we make a view.
    if (GetTexture()->IsDestroyed()) {
        return {};
    }

    Device* device = ToBackend(GetTexture()->GetDevice());
    VkImageViewCreateInfo createInfo = GetCreateInfo(descriptor->format, descriptor->dimension);

    VkImageViewUsageCreateInfo usageInfo = {};
    usageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO;
    usageInfo.usage = VulkanImageUsage(device, GetInternalUsage(), GetFormat());
    createInfo.pNext = &usageInfo;

    VkSamplerYcbcrConversionInfo samplerYCbCrInfo = {};
    if (auto* yCbCrVkDescriptor = descriptor.Get<YCbCrVkDescriptor>()) {
        mIsYCbCr = true;
        mYCbCrVkDescriptor = yCbCrVkDescriptor->WithTrivialFrontendDefaults();
        mYCbCrVkDescriptor.nextInChain = nullptr;

        DAWN_TRY_ASSIGN(mSamplerYCbCrConversion,
                        CreateSamplerYCbCrConversionCreateInfo(mYCbCrVkDescriptor, device));

        samplerYCbCrInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO;
        samplerYCbCrInfo.pNext = nullptr;
        samplerYCbCrInfo.conversion = mSamplerYCbCrConversion;

        createInfo.pNext = &samplerYCbCrInfo;
    }

    DAWN_TRY(CheckVkSuccess(
        device->fn.CreateImageView(device->GetVkDevice(), &createInfo, nullptr, &*mHandle),
        "CreateImageView"));

    // We should create an image view with format RGBA8Unorm on the BGRA8Unorm texture when the
    // texture is used as storage texture. See http://crbug.com/dawn/1641 for more details.
    if (createInfo.format == VK_FORMAT_B8G8R8A8_UNORM &&
        (GetTexture()->GetInternalUsage() & wgpu::TextureUsage::StorageBinding)) {
        createInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        DAWN_TRY(CheckVkSuccess(device->fn.CreateImageView(device->GetVkDevice(), &createInfo,
                                                           nullptr, &*mHandleForBGRA8UnormStorage),
                                "CreateImageView for BGRA8Unorm storage"));
    }

    SetLabelImpl();

    return {};
}

TextureView::~TextureView() {}

void TextureView::DestroyImpl() {
    Device* device = ToBackend(GetTexture()->GetDevice());

    if (mSamplerYCbCrConversion != VK_NULL_HANDLE) {
        device->GetFencedDeleter()->DeleteWhenUnused(mSamplerYCbCrConversion);
        mSamplerYCbCrConversion = VK_NULL_HANDLE;
    }

    if (mHandle != VK_NULL_HANDLE) {
        device->GetFencedDeleter()->DeleteWhenUnused(mHandle);
        mHandle = VK_NULL_HANDLE;
    }

    if (mHandleForBGRA8UnormStorage != VK_NULL_HANDLE) {
        device->GetFencedDeleter()->DeleteWhenUnused(mHandleForBGRA8UnormStorage);
        mHandleForBGRA8UnormStorage = VK_NULL_HANDLE;
    }

    for (auto& handle : mHandlesFor2DViewOn3D) {
        if (handle != VK_NULL_HANDLE) {
            device->GetFencedDeleter()->DeleteWhenUnused(handle);
            handle = VK_NULL_HANDLE;
        }
    }
}

VkImageView TextureView::GetHandle() const {
    return mHandle;
}

VkImageView TextureView::GetHandleForBGRA8UnormStorage() const {
    return mHandleForBGRA8UnormStorage;
}

VkImageViewCreateInfo TextureView::GetCreateInfo(wgpu::TextureFormat format,
                                                 wgpu::TextureViewDimension dimension,
                                                 uint32_t depthSlice) const {
    Device* device = ToBackend(GetTexture()->GetDevice());

    VkImageViewCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.image = ToBackend(GetTexture())->GetHandle();
    createInfo.viewType = VulkanImageViewType(dimension);

    const Format& textureFormat = GetTexture()->GetFormat();
    if (textureFormat.HasStencil() &&
        (textureFormat.HasDepth() || !device->IsToggleEnabled(Toggle::VulkanUseS8))) {
        // Unlike multi-planar formats, depth-stencil formats have multiple aspects but are not
        // created with VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT.
        // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkImageViewCreateInfo.html#VUID-VkImageViewCreateInfo-image-01762
        // Without, VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, the view format must match the texture
        // format.
        createInfo.format = VulkanImageFormat(device, textureFormat.format);
    } else {
        createInfo.format = VulkanImageFormat(device, format);
    }

    createInfo.components = VkComponentMapping{VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
                                               VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};

    const SubresourceRange& subresources = GetSubresourceRange();
    createInfo.subresourceRange.baseMipLevel = subresources.baseMipLevel;
    createInfo.subresourceRange.levelCount = subresources.levelCount;
    createInfo.subresourceRange.baseArrayLayer = subresources.baseArrayLayer + depthSlice;
    createInfo.subresourceRange.layerCount = subresources.layerCount;
    createInfo.subresourceRange.aspectMask = VulkanAspectMask(subresources.aspects);

    return createInfo;
}

ResultOrError<VkImageView> TextureView::GetOrCreate2DViewOn3D(uint32_t depthSlice) {
    DAWN_ASSERT(GetTexture()->GetDimension() == wgpu::TextureDimension::e3D);
    DAWN_ASSERT(depthSlice < GetSingleSubresourceVirtualSize().depthOrArrayLayers);

    if (mHandlesFor2DViewOn3D.empty()) {
        mHandlesFor2DViewOn3D.resize(GetSingleSubresourceVirtualSize().depthOrArrayLayers);
    }

    if (mHandlesFor2DViewOn3D[depthSlice] != VK_NULL_HANDLE) {
        return static_cast<VkImageView>(mHandlesFor2DViewOn3D[depthSlice]);
    }

    Device* device = ToBackend(GetTexture()->GetDevice());
    VkImageViewCreateInfo createInfo =
        GetCreateInfo(GetFormat().format, wgpu::TextureViewDimension::e2D, depthSlice);

    VkImageView view;
    DAWN_TRY(CheckVkSuccess(
        device->fn.CreateImageView(device->GetVkDevice(), &createInfo, nullptr, &*view),
        "CreateImageView for 2D view on 3D image"));

    mHandlesFor2DViewOn3D[depthSlice] = view;

    return view;
}

bool TextureView::IsYCbCr() const {
    return mIsYCbCr;
}

YCbCrVkDescriptor TextureView::GetYCbCrVkDescriptor() const {
    DAWN_ASSERT(IsYCbCr());
    return mYCbCrVkDescriptor;
}

void TextureView::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mHandle, "Dawn_TextureView", GetLabel());
}

}  // namespace dawn::native::vulkan
