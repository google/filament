//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_helpers:
//   Helper utility classes that manage Vulkan resources.

#include "libANGLE/renderer/vulkan/vk_helpers.h"

#include "common/aligned_memory.h"
#include "common/utilities.h"
#include "common/vulkan/vk_headers.h"
#include "image_util/loadimage.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/driver_utils.h"
#include "libANGLE/renderer/renderer_utils.h"
#include "libANGLE/renderer/vulkan/BufferVk.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/FramebufferVk.h"
#include "libANGLE/renderer/vulkan/RenderTargetVk.h"
#include "libANGLE/renderer/vulkan/android/vk_android_utils.h"
#include "libANGLE/renderer/vulkan/vk_ref_counted_event.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

namespace rx
{
namespace vk
{
namespace
{
// During descriptorSet cache eviction, we keep it in the cache only if it is recently used. If it
// has not been used in the past kDescriptorSetCacheRetireAge frames, it will be evicted.
constexpr uint32_t kDescriptorSetCacheRetireAge = 10;

// ANGLE_robust_resource_initialization requires color textures to be initialized to zero.
constexpr VkClearColorValue kRobustInitColorValue = {{0, 0, 0, 0}};
// When emulating a texture, we want the emulated channels to be 0, with alpha 1.
constexpr VkClearColorValue kEmulatedInitColorValue = {{0, 0, 0, 1.0f}};
// ANGLE_robust_resource_initialization requires depth to be initialized to 1 and stencil to 0.
// We are fine with these values for emulated depth/stencil textures too.
constexpr VkClearDepthStencilValue kRobustInitDepthStencilValue = {1.0f, 0};

constexpr VkImageAspectFlags kDepthStencilAspects =
    VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;

// Information useful for buffer related barriers
struct BufferMemoryBarrierData
{
    VkPipelineStageFlags pipelineStageFlags;
    // EventStage::InvalidEnum indicates don't use VkEvent for barrier(i.e., use pipelineBarrier
    // instead)
    EventStage eventStage;
};
// clang-format off
constexpr angle::PackedEnumMap<PipelineStage, BufferMemoryBarrierData> kBufferMemoryBarrierData = {
    {PipelineStage::TopOfPipe, {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, EventStage::InvalidEnum}},
    {PipelineStage::DrawIndirect, {VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, EventStage::VertexInput}},
    {PipelineStage::VertexInput, {VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, EventStage::VertexInput}},
    {PipelineStage::VertexShader, {VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, EventStage::VertexShader}},
    {PipelineStage::TessellationControl, {VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, EventStage::InvalidEnum}},
    {PipelineStage::TessellationEvaluation, {VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, EventStage::InvalidEnum}},
    {PipelineStage::GeometryShader, {VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, EventStage::InvalidEnum}},
    {PipelineStage::TransformFeedback, {VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT, EventStage::TransformFeedbackWrite}},
    {PipelineStage::FragmentShadingRate, {0, EventStage::InvalidEnum}},
    {PipelineStage::EarlyFragmentTest, {0, EventStage::InvalidEnum}},
    {PipelineStage::FragmentShader, {VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, EventStage::FragmentShader}},
    {PipelineStage::LateFragmentTest, {0, EventStage::InvalidEnum}},
    {PipelineStage::ColorAttachmentOutput, {0, EventStage::InvalidEnum}},
    {PipelineStage::ComputeShader, {VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, EventStage::ComputeShader}},
    {PipelineStage::Transfer, {VK_PIPELINE_STAGE_TRANSFER_BIT, EventStage::InvalidEnum}},
    {PipelineStage::BottomOfPipe, BufferMemoryBarrierData{VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, EventStage::InvalidEnum}},
    {PipelineStage::Host, {VK_PIPELINE_STAGE_HOST_BIT, EventStage::InvalidEnum}},
};

constexpr gl::ShaderMap<PipelineStage> kPipelineStageShaderMap = {
    {gl::ShaderType::Vertex, PipelineStage::VertexShader},
    {gl::ShaderType::TessControl, PipelineStage::TessellationControl},
    {gl::ShaderType::TessEvaluation, PipelineStage::TessellationEvaluation},
    {gl::ShaderType::Geometry, PipelineStage::GeometryShader},
    {gl::ShaderType::Fragment, PipelineStage::FragmentShader},
    {gl::ShaderType::Compute, PipelineStage::ComputeShader},
};

constexpr ImageLayoutToMemoryBarrierDataMap kImageMemoryBarrierData = {
    {
        ImageLayout::Undefined,
        ImageMemoryBarrierData{
            "Undefined",
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            // Transition to: we don't expect to transition into Undefined.
            0,
            // Transition from: there's no data in the image to care about.
            0,
            ResourceAccess::ReadOnly,
            PipelineStage::InvalidEnum,
            // We do not directly use this layout in SetEvent. We transit to other layout before using
            EventStage::InvalidEnum,
            PipelineStageGroup::Other,
        },
    },
    {
        ImageLayout::ColorWrite,
        ImageMemoryBarrierData{
            "ColorWrite",
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::ColorAttachmentOutput,
            EventStage::Attachment,
            PipelineStageGroup::FragmentOnly,
        },
    },
    {
        ImageLayout::ColorWriteAndInput,
        ImageMemoryBarrierData{
            "ColorWriteAndInput",
            VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::ColorAttachmentOutput,
            EventStage::Attachment,
            PipelineStageGroup::FragmentOnly,
        },
    },
    {
        ImageLayout::MSRTTEmulationColorUnresolveAndResolve,
        ImageMemoryBarrierData{
            "MSRTTEmulationColorUnresolveAndResolve",
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::FragmentShader,
            EventStage::AttachmentAndFragmentShader,
            PipelineStageGroup::FragmentOnly,
        },
    },
    {
        ImageLayout::DepthWriteStencilWrite,
        ImageMemoryBarrierData{
            "DepthWriteStencilWrite",
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            kAllDepthStencilPipelineStageFlags,
            kAllDepthStencilPipelineStageFlags,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::EarlyFragmentTest,
            EventStage::Attachment,
            PipelineStageGroup::FragmentOnly,
        },
    },
    {
        ImageLayout::DepthStencilWriteAndInput,
        ImageMemoryBarrierData{
            "DepthStencilWriteAndInput",
            VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR,
            kAllDepthStencilPipelineStageFlags,
            kAllDepthStencilPipelineStageFlags,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::EarlyFragmentTest,
            EventStage::Attachment,
            PipelineStageGroup::FragmentOnly,
        },
    },
    {
        ImageLayout::DepthWriteStencilRead,
        ImageMemoryBarrierData{
            "DepthWriteStencilRead",
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
            kAllDepthStencilPipelineStageFlags,
            kAllDepthStencilPipelineStageFlags,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::EarlyFragmentTest,
            EventStage::Attachment,
            PipelineStageGroup::FragmentOnly,
        },
    },
    {
        ImageLayout::DepthWriteStencilReadFragmentShaderStencilRead,
        ImageMemoryBarrierData{
            "DepthWriteStencilReadFragmentShaderStencilRead",
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | kAllDepthStencilPipelineStageFlags,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | kAllDepthStencilPipelineStageFlags,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::EarlyFragmentTest,
            EventStage::AttachmentAndFragmentShader,
            PipelineStageGroup::FragmentOnly,
        },
    },
    {
        ImageLayout::DepthWriteStencilReadAllShadersStencilRead,
        ImageMemoryBarrierData{
            "DepthWriteStencilReadAllShadersStencilRead",
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
            kAllShadersPipelineStageFlags | kAllDepthStencilPipelineStageFlags,
            kAllShadersPipelineStageFlags | kAllDepthStencilPipelineStageFlags,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::VertexShader,
            EventStage::AttachmentAndAllShaders,
            PipelineStageGroup::Other,
        },
    },
    {
        ImageLayout::DepthReadStencilWrite,
        ImageMemoryBarrierData{
            "DepthReadStencilWrite",
            VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
            kAllDepthStencilPipelineStageFlags,
            kAllDepthStencilPipelineStageFlags,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::EarlyFragmentTest,
            EventStage::Attachment,
            PipelineStageGroup::FragmentOnly,
        },
    },
    {
        ImageLayout::DepthReadStencilWriteFragmentShaderDepthRead,
        ImageMemoryBarrierData{
            "DepthReadStencilWriteFragmentShaderDepthRead",
            VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | kAllDepthStencilPipelineStageFlags,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | kAllDepthStencilPipelineStageFlags,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::EarlyFragmentTest,
            EventStage::AttachmentAndFragmentShader,
            PipelineStageGroup::FragmentOnly,
        },
    },
    {
        ImageLayout::DepthReadStencilWriteAllShadersDepthRead,
        ImageMemoryBarrierData{
            "DepthReadStencilWriteAllShadersDepthRead",
            VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
            kAllShadersPipelineStageFlags | kAllDepthStencilPipelineStageFlags,
            kAllShadersPipelineStageFlags | kAllDepthStencilPipelineStageFlags,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::VertexShader,
            EventStage::AttachmentAndAllShaders,
            PipelineStageGroup::Other,
        },
    },
    {
        ImageLayout::DepthReadStencilRead,
            ImageMemoryBarrierData{
            "DepthReadStencilRead",
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            kAllDepthStencilPipelineStageFlags,
            kAllDepthStencilPipelineStageFlags,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            PipelineStage::EarlyFragmentTest,
            EventStage::Attachment,
            PipelineStageGroup::FragmentOnly,
        },
    },

    {
        ImageLayout::DepthReadStencilReadFragmentShaderRead,
            ImageMemoryBarrierData{
            "DepthReadStencilReadFragmentShaderRead",
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | kAllDepthStencilPipelineStageFlags,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | kAllDepthStencilPipelineStageFlags,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            PipelineStage::EarlyFragmentTest,
            EventStage::AttachmentAndFragmentShader,
            PipelineStageGroup::FragmentOnly,
        },
    },
    {
        ImageLayout::DepthReadStencilReadAllShadersRead,
            ImageMemoryBarrierData{
            "DepthReadStencilReadAllShadersRead",
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            kAllShadersPipelineStageFlags | kAllDepthStencilPipelineStageFlags,
            kAllShadersPipelineStageFlags | kAllDepthStencilPipelineStageFlags,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            PipelineStage::VertexShader,
            EventStage::AttachmentAndAllShaders,
            PipelineStageGroup::Other,
        },
    },
    {
        ImageLayout::ColorWriteFragmentShaderFeedback,
        ImageMemoryBarrierData{
            "ColorWriteFragmentShaderFeedback",
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::FragmentShader,
            EventStage::AttachmentAndFragmentShader,
            PipelineStageGroup::FragmentOnly,
        },
    },
    {
        ImageLayout::ColorWriteAllShadersFeedback,
        ImageMemoryBarrierData{
            "ColorWriteAllShadersFeedback",
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | kAllShadersPipelineStageFlags,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | kAllShadersPipelineStageFlags,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            ResourceAccess::ReadWrite,
            // In case of multiple destination stages, We barrier the earliest stage
            PipelineStage::VertexShader,
            EventStage::AttachmentAndAllShaders,
            PipelineStageGroup::Other,
        },
    },
    {
        ImageLayout::DepthStencilFragmentShaderFeedback,
        ImageMemoryBarrierData{
            "DepthStencilFragmentShaderFeedback",
            VK_IMAGE_LAYOUT_GENERAL,
            kAllDepthStencilPipelineStageFlags | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            kAllDepthStencilPipelineStageFlags | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::FragmentShader,
            EventStage::AttachmentAndFragmentShader,
            PipelineStageGroup::FragmentOnly,
        },
    },
    {
        ImageLayout::DepthStencilAllShadersFeedback,
        ImageMemoryBarrierData{
            "DepthStencilAllShadersFeedback",
            VK_IMAGE_LAYOUT_GENERAL,
            kAllDepthStencilPipelineStageFlags | kAllShadersPipelineStageFlags,
            kAllDepthStencilPipelineStageFlags | kAllShadersPipelineStageFlags,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            ResourceAccess::ReadWrite,
            // In case of multiple destination stages, We barrier the earliest stage
            PipelineStage::VertexShader,
            EventStage::AttachmentAndAllShaders,
            PipelineStageGroup::Other,
        },
    },
    {
        ImageLayout::DepthStencilResolve,
        ImageMemoryBarrierData{
            "DepthStencilResolve",
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            // Note: depth/stencil resolve uses color output stage and mask!
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::ColorAttachmentOutput,
            EventStage::Attachment,
            PipelineStageGroup::FragmentOnly,
        },
    },
    {
        ImageLayout::MSRTTEmulationDepthStencilUnresolveAndResolve,
        ImageMemoryBarrierData{
            "MSRTTEmulationDepthStencilUnresolveAndResolve",
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            // Note: depth/stencil resolve uses color output stage and mask!
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::FragmentShader,
            EventStage::AttachmentAndFragmentShader,
            PipelineStageGroup::FragmentOnly,
        },
    },
    {
        ImageLayout::Present,
        ImageMemoryBarrierData{
            "Present",
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            // Transition to: do not delay execution of commands in the second synchronization
            // scope. Allow layout transition to be delayed until present semaphore is signaled.
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            // Transition from: use same stages as in Acquire Image Semaphore stage mask in order to
            // build a dependency chain from the Acquire Image Semaphore to the layout transition's
            // first synchronization scope.
            kSwapchainAcquireImageWaitStageFlags,
            // Transition to: vkQueuePresentKHR automatically performs the appropriate memory barriers:
            //
            // > Any writes to memory backing the images referenced by the pImageIndices and
            // > pSwapchains members of pPresentInfo, that are available before vkQueuePresentKHR
            // > is executed, are automatically made visible to the read access performed by the
            // > presentation engine.
            0,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            PipelineStage::BottomOfPipe,
            // We do not directly use this layout in SetEvent.
            EventStage::InvalidEnum,
            PipelineStageGroup::Other,
        },
    },
    {
        ImageLayout::SharedPresent,
        ImageMemoryBarrierData{
            "SharedPresent",
            VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR,
            // All currently possible stages for SharedPresent
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_MEMORY_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::BottomOfPipe,
            EventStage::AttachmentAndFragmentShaderAndTransfer,
            PipelineStageGroup::Other,
        },
    },
    {
        ImageLayout::ExternalPreInitialized,
        ImageMemoryBarrierData{
            "ExternalPreInitialized",
            // Binding a VkImage with an initial layout of VK_IMAGE_LAYOUT_UNDEFINED to external
            // memory whose content has already been defined does not make the content undefined
            // (see 12.8.1.  External Resource Sharing).
            //
            // Note that for external memory objects, if the content is already defined, the
            // ownership rules imply that the first operation on the texture must be a call to
            // glWaitSemaphoreEXT that grants ownership of the image and informs us of the true
            // layout.  If the content is not already defined, the first operation may not be a
            // glWaitSemaphore, but in this case undefined layout is appropriate.
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            // Transition to: we don't expect to transition into PreInitialized.
            0,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_MEMORY_WRITE_BIT,
            ResourceAccess::ReadOnly,
            PipelineStage::InvalidEnum,
            // We do not directly use this layout in SetEvent. We transit to internal layout before using
            EventStage::InvalidEnum,
            PipelineStageGroup::Other,
        },
    },
    {
        ImageLayout::ExternalShadersReadOnly,
        ImageMemoryBarrierData{
            "ExternalShadersReadOnly",
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            // In case of multiple destination stages, We barrier the earliest stage
            PipelineStage::TopOfPipe,
            // We do not directly use this layout in SetEvent. We transit to internal layout before using
            EventStage::InvalidEnum,
            PipelineStageGroup::Other,
        },
    },
    {
        ImageLayout::ExternalShadersWrite,
        ImageMemoryBarrierData{
            "ExternalShadersWrite",
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_SHADER_WRITE_BIT,
            ResourceAccess::ReadWrite,
            // In case of multiple destination stages, We barrier the earliest stage
            PipelineStage::TopOfPipe,
            // We do not directly use this layout in SetEvent. We transit to internal layout before using
            EventStage::InvalidEnum,
            PipelineStageGroup::Other,
        },
    },
    {
        ImageLayout::ForeignAccess,
        ImageMemoryBarrierData{
            "ForeignAccess",
            VK_IMAGE_LAYOUT_GENERAL,
            // Transition to: we don't expect to transition into ForeignAccess, that's done at
            // submission time by the CommandQueue; the following value doesn't matter.
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            // Transition to: see dstStageMask
            0,
            // Transition from: all writes must finish before barrier; it is unknown how the foreign
            // entity has access the memory.
            VK_ACCESS_MEMORY_WRITE_BIT,
            ResourceAccess::ReadWrite,
            // In case of multiple destination stages, We barrier the earliest stage
            PipelineStage::TopOfPipe,
            // We do not directly use this layout in SetEvent. We transit to internal layout before using
            EventStage::InvalidEnum,
            PipelineStageGroup::Other,
        },
    },
    {
        ImageLayout::TransferSrc,
        ImageMemoryBarrierData{
            "TransferSrc",
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_TRANSFER_READ_BIT,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            PipelineStage::Transfer,
            EventStage::Transfer,
            PipelineStageGroup::Other,
        },
    },
    {
        ImageLayout::TransferDst,
        ImageMemoryBarrierData{
            "TransferDst",
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            // Transition to: all writes must happen after barrier.
            VK_ACCESS_TRANSFER_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_TRANSFER_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::Transfer,
            EventStage::Transfer,
            PipelineStageGroup::Other,
        },
    },
    {
        ImageLayout::TransferSrcDst,
        ImageMemoryBarrierData{
            "TransferSrcDst",
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_TRANSFER_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::Transfer,
            EventStage::Transfer,
            PipelineStageGroup::Other,
        },
    },
    {
        ImageLayout::HostCopy,
        ImageMemoryBarrierData{
            "HostCopy",
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            // Transition to: we don't expect to transition into HostCopy on the GPU.
            0,
            // Transition from: the data was initialized in the image by the host.  Note that we
            // only transition to this layout if the image was previously in UNDEFINED, in which
            // case it didn't contain any data prior to the host copy either.
            0,
            ResourceAccess::ReadOnly,
            PipelineStage::InvalidEnum,
            // We do not directly use this layout in SetEvent.
            EventStage::InvalidEnum,
            PipelineStageGroup::Other,
        },
    },
    {
        ImageLayout::VertexShaderReadOnly,
        ImageMemoryBarrierData{
            "VertexShaderReadOnly",
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            PipelineStage::VertexShader,
            EventStage::VertexShader,
            PipelineStageGroup::PreFragmentOnly,
        },
    },
    {
        ImageLayout::VertexShaderWrite,
        ImageMemoryBarrierData{
            "VertexShaderWrite",
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_SHADER_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::VertexShader,
            EventStage::VertexShader,
            PipelineStageGroup::PreFragmentOnly,
        },
    },
    {
        ImageLayout::PreFragmentShadersReadOnly,
        ImageMemoryBarrierData{
            "PreFragmentShadersReadOnly",
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            kPreFragmentStageFlags,
            kPreFragmentStageFlags,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            // In case of multiple destination stages, We barrier the earliest stage
            PipelineStage::VertexShader,
            EventStage::PreFragmentShaders,
            PipelineStageGroup::PreFragmentOnly,
        },
    },
    {
        ImageLayout::PreFragmentShadersWrite,
        ImageMemoryBarrierData{
            "PreFragmentShadersWrite",
            VK_IMAGE_LAYOUT_GENERAL,
            kPreFragmentStageFlags,
            kPreFragmentStageFlags,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_SHADER_WRITE_BIT,
            ResourceAccess::ReadWrite,
            // In case of multiple destination stages, We barrier the earliest stage
            PipelineStage::VertexShader,
            EventStage::PreFragmentShaders,
            PipelineStageGroup::PreFragmentOnly,
        },
    },
    {
        ImageLayout::FragmentShadingRateAttachmentReadOnly,
        ImageMemoryBarrierData{
            "FragmentShadingRateAttachmentReadOnly",
            VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR,
            VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
            VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            PipelineStage::FragmentShadingRate,
            EventStage::FragmentShadingRate,
            PipelineStageGroup::Other,
        },
    },
    {
        ImageLayout::FragmentShaderReadOnly,
        ImageMemoryBarrierData{
            "FragmentShaderReadOnly",
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            PipelineStage::FragmentShader,
            EventStage::FragmentShader,
            PipelineStageGroup::FragmentOnly,
        },
    },
    {
        ImageLayout::FragmentShaderWrite,
        ImageMemoryBarrierData{
            "FragmentShaderWrite",
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_SHADER_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::FragmentShader,
            EventStage::FragmentShader,
            PipelineStageGroup::FragmentOnly,
        },
    },
    {
        ImageLayout::ComputeShaderReadOnly,
        ImageMemoryBarrierData{
            "ComputeShaderReadOnly",
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            PipelineStage::ComputeShader,
            EventStage::ComputeShader,
            PipelineStageGroup::ComputeOnly,
        },
    },
    {
        ImageLayout::ComputeShaderWrite,
        ImageMemoryBarrierData{
            "ComputeShaderWrite",
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_SHADER_WRITE_BIT,
            ResourceAccess::ReadWrite,
            PipelineStage::ComputeShader,
            EventStage::ComputeShader,
            PipelineStageGroup::ComputeOnly,
        },
    },
    {
        ImageLayout::AllGraphicsShadersReadOnly,
        ImageMemoryBarrierData{
            "AllGraphicsShadersReadOnly",
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            kAllShadersPipelineStageFlags,
            kAllShadersPipelineStageFlags,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            // In case of multiple destination stages, We barrier the earliest stage
            PipelineStage::VertexShader,
            EventStage::AllShaders,
            PipelineStageGroup::Other,
        },
    },
    {
        ImageLayout::AllGraphicsShadersWrite,
        ImageMemoryBarrierData{
            "AllGraphicsShadersWrite",
            VK_IMAGE_LAYOUT_GENERAL,
            kAllShadersPipelineStageFlags,
            kAllShadersPipelineStageFlags,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_SHADER_WRITE_BIT,
            ResourceAccess::ReadWrite,
            // In case of multiple destination stages, We barrier the earliest stage
            PipelineStage::VertexShader,
            EventStage::AllShaders,
            PipelineStageGroup::Other,
        },
    },
    {
        ImageLayout::TransferDstAndComputeWrite,
        ImageMemoryBarrierData{
            "TransferDstAndComputeWrite",
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT,
            ResourceAccess::ReadWrite,
            // In case of multiple destination stages, We barrier the earliest stage
            PipelineStage::ComputeShader,
            EventStage::TransferAndComputeShader,
            PipelineStageGroup::Other,
        },
    },
};
// clang-format on

EventStage GetImageLayoutEventStage(ImageLayout layout)
{
    const ImageMemoryBarrierData &barrierData = kImageMemoryBarrierData[layout];
    return barrierData.eventStage;
}

bool HasBothDepthAndStencilAspects(VkImageAspectFlags aspectFlags)
{
    return IsMaskFlagSet(aspectFlags, kDepthStencilAspects);
}

uint8_t GetContentDefinedLayerRangeBits(uint32_t layerStart,
                                        uint32_t layerCount,
                                        uint32_t maxLayerCount)
{
    uint8_t layerRangeBits = layerCount >= maxLayerCount ? static_cast<uint8_t>(~0u)
                                                         : angle::BitMask<uint8_t>(layerCount);
    layerRangeBits <<= layerStart;

    return layerRangeBits;
}

uint32_t GetImageLayerCountForView(const ImageHelper &image)
{
    // Depth > 1 means this is a 3D texture and depth is our layer count
    return image.getExtents().depth > 1 ? image.getExtents().depth : image.getLayerCount();
}

void ReleaseImageViews(ImageViewVector *imageViewVector, GarbageObjects *garbage)
{
    for (ImageView &imageView : *imageViewVector)
    {
        if (imageView.valid())
        {
            garbage->emplace_back(GetGarbage(&imageView));
        }
    }
    imageViewVector->clear();
}

void DestroyImageViews(ImageViewVector *imageViewVector, VkDevice device)
{
    for (ImageView &imageView : *imageViewVector)
    {
        imageView.destroy(device);
    }
    imageViewVector->clear();
}

void ReleaseLayerLevelImageViews(LayerLevelImageViewVector *imageViewVector,
                                 GarbageObjects *garbage)
{
    for (ImageViewVector &layerViews : *imageViewVector)
    {
        for (ImageView &imageView : layerViews)
        {
            if (imageView.valid())
            {
                garbage->emplace_back(GetGarbage(&imageView));
            }
        }
    }
    imageViewVector->clear();
}

void DestroyLayerLevelImageViews(LayerLevelImageViewVector *imageViewVector, VkDevice device)
{
    for (ImageViewVector &layerViews : *imageViewVector)
    {
        for (ImageView &imageView : layerViews)
        {
            imageView.destroy(device);
        }
    }
    imageViewVector->clear();
}

void ReleaseSubresourceImageViews(SubresourceImageViewMap *imageViews, GarbageObjects *garbage)
{
    for (auto &iter : *imageViews)
    {
        std::unique_ptr<ImageView> &imageView = iter.second;
        if (imageView->valid())
        {
            garbage->emplace_back(GetGarbage(imageView.get()));
        }
    }
    imageViews->clear();
}

void DestroySubresourceImageViews(SubresourceImageViewMap *imageViews, VkDevice device)
{
    for (auto &iter : *imageViews)
    {
        std::unique_ptr<ImageView> &imageView = iter.second;
        imageView->destroy(device);
    }
    imageViews->clear();
}

ImageView *GetLevelImageView(ImageViewVector *imageViews, LevelIndex levelVk, uint32_t levelCount)
{
    // Lazily allocate the storage for image views. We allocate the full level count because we
    // don't want to trigger any std::vector reallocations. Reallocations could invalidate our
    // view pointers.
    if (imageViews->empty())
    {
        imageViews->resize(levelCount);
    }
    ASSERT(imageViews->size() > levelVk.get());

    return &(*imageViews)[levelVk.get()];
}

ImageView *GetLevelLayerImageView(LayerLevelImageViewVector *imageViews,
                                  LevelIndex levelVk,
                                  uint32_t layer,
                                  uint32_t levelCount,
                                  uint32_t layerCount)
{
    // Lazily allocate the storage for image views. We allocate the full layer count because we
    // don't want to trigger any std::vector reallocations. Reallocations could invalidate our
    // view pointers.
    if (imageViews->empty())
    {
        imageViews->resize(layerCount);
    }
    ASSERT(imageViews->size() > layer);

    return GetLevelImageView(&(*imageViews)[layer], levelVk, levelCount);
}

// Special rules apply to VkBufferImageCopy with depth/stencil. The components are tightly packed
// into a depth or stencil section of the destination buffer. See the spec:
// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkBufferImageCopy.html
const angle::Format &GetDepthStencilImageToBufferFormat(const angle::Format &imageFormat,
                                                        VkImageAspectFlagBits copyAspect)
{
    if (copyAspect == VK_IMAGE_ASPECT_STENCIL_BIT)
    {
        ASSERT(imageFormat.id == angle::FormatID::D24_UNORM_S8_UINT ||
               imageFormat.id == angle::FormatID::D32_FLOAT_S8X24_UINT ||
               imageFormat.id == angle::FormatID::S8_UINT);
        return angle::Format::Get(angle::FormatID::S8_UINT);
    }

    ASSERT(copyAspect == VK_IMAGE_ASPECT_DEPTH_BIT);

    switch (imageFormat.id)
    {
        case angle::FormatID::D16_UNORM:
            return imageFormat;
        case angle::FormatID::D24_UNORM_X8_UINT:
            return imageFormat;
        case angle::FormatID::D24_UNORM_S8_UINT:
            return angle::Format::Get(angle::FormatID::D24_UNORM_X8_UINT);
        case angle::FormatID::D32_FLOAT:
            return imageFormat;
        case angle::FormatID::D32_FLOAT_S8X24_UINT:
            return angle::Format::Get(angle::FormatID::D32_FLOAT);
        default:
            UNREACHABLE();
            return imageFormat;
    }
}

VkClearValue GetRobustResourceClearValue(const angle::Format &intendedFormat,
                                         const angle::Format &actualFormat)
{
    VkClearValue clearValue = {};
    if (intendedFormat.hasDepthOrStencilBits())
    {
        clearValue.depthStencil = kRobustInitDepthStencilValue;
    }
    else
    {
        clearValue.color = HasEmulatedImageChannels(intendedFormat, actualFormat)
                               ? kEmulatedInitColorValue
                               : kRobustInitColorValue;
    }
    return clearValue;
}

bool IsShaderReadOnlyLayout(const ImageMemoryBarrierData &imageLayout)
{
    // We also use VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL for texture sample from depth
    // texture. See GetImageReadLayout() for detail.
    return imageLayout.layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ||
           imageLayout.layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
}

bool IsAnySubresourceContentDefined(const gl::TexLevelArray<angle::BitSet8<8>> &contentDefined)
{
    for (const angle::BitSet8<8> &levelContentDefined : contentDefined)
    {
        if (levelContentDefined.any())
        {
            return true;
        }
    }
    return false;
}

void ExtendRenderPassInvalidateArea(const gl::Rectangle &invalidateArea, gl::Rectangle *out)
{
    if (out->empty())
    {
        *out = invalidateArea;
    }
    else
    {
        gl::ExtendRectangle(*out, invalidateArea, out);
    }
}

bool CanCopyWithTransferForCopyImage(Renderer *renderer,
                                     ImageHelper *srcImage,
                                     VkImageTiling srcTilingMode,
                                     ImageHelper *dstImage,
                                     VkImageTiling dstTilingMode)
{
    // Neither source nor destination formats can be emulated for copy image through transfer,
    // unless they are emulated with the same format!
    bool isFormatCompatible =
        (!srcImage->hasEmulatedImageFormat() && !dstImage->hasEmulatedImageFormat()) ||
        srcImage->getActualFormatID() == dstImage->getActualFormatID();

    // If neither formats are emulated, GL validation ensures that pixelBytes is the same for both.
    ASSERT(!isFormatCompatible ||
           srcImage->getActualFormat().pixelBytes == dstImage->getActualFormat().pixelBytes);

    return isFormatCompatible &&
           CanCopyWithTransfer(renderer, srcImage->getActualFormatID(), srcTilingMode,
                               dstImage->getActualFormatID(), dstTilingMode);
}

void ReleaseBufferListToRenderer(Context *context, BufferHelperQueue *buffers)
{
    for (std::unique_ptr<BufferHelper> &toFree : *buffers)
    {
        toFree->release(context);
    }
    buffers->clear();
}

void DestroyBufferList(Renderer *renderer, BufferHelperQueue *buffers)
{
    for (std::unique_ptr<BufferHelper> &toDestroy : *buffers)
    {
        toDestroy->destroy(renderer);
    }
    buffers->clear();
}

// Helper functions used below
char GetLoadOpShorthand(RenderPassLoadOp loadOp)
{
    switch (loadOp)
    {
        case RenderPassLoadOp::Clear:
            return 'C';
        case RenderPassLoadOp::Load:
            return 'L';
        case RenderPassLoadOp::None:
            return 'N';
        default:
            return 'D';
    }
}

char GetStoreOpShorthand(RenderPassStoreOp storeOp)
{
    switch (storeOp)
    {
        case RenderPassStoreOp::Store:
            return 'S';
        case RenderPassStoreOp::None:
            return 'N';
        default:
            return 'D';
    }
}

bool IsClear(UpdateSource updateSource)
{
    return updateSource == UpdateSource::Clear ||
           updateSource == UpdateSource::ClearEmulatedChannelsOnly ||
           updateSource == UpdateSource::ClearAfterInvalidate;
}

bool IsClearOfAllChannels(UpdateSource updateSource)
{
    return updateSource == UpdateSource::Clear ||
           updateSource == UpdateSource::ClearAfterInvalidate;
}

angle::Result InitDynamicDescriptorPool(ErrorContext *context,
                                        const DescriptorSetLayoutDesc &descriptorSetLayoutDesc,
                                        const DescriptorSetLayout &descriptorSetLayout,
                                        uint32_t descriptorCountMultiplier,
                                        DynamicDescriptorPool *poolToInit)
{
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
    DescriptorSetLayoutBindingVector bindingVector;
    descriptorSetLayoutDesc.unpackBindings(&bindingVector);

    for (const VkDescriptorSetLayoutBinding &binding : bindingVector)
    {
        if (binding.descriptorCount > 0)
        {
            VkDescriptorPoolSize poolSize = {};
            poolSize.type                 = binding.descriptorType;
            poolSize.descriptorCount      = binding.descriptorCount * descriptorCountMultiplier;
            descriptorPoolSizes.emplace_back(poolSize);
        }
    }

    if (!descriptorPoolSizes.empty())
    {
        ANGLE_TRY(poolToInit->init(context, descriptorPoolSizes.data(), descriptorPoolSizes.size(),
                                   descriptorSetLayout));
    }

    return angle::Result::Continue;
}

bool CheckSubpassCommandBufferCount(uint32_t count)
{
    // When using angle::SharedRingBufferAllocator we must ensure that allocator is attached and
    // detached from the same priv::SecondaryCommandBuffer instance.
    // Custom command buffer (priv::SecondaryCommandBuffer) may contain commands for multiple
    // subpasses, therefore we do not need multiple buffers.
    return (count == 1 || !RenderPassCommandBuffer::ExecutesInline());
}

bool IsAnyLayout(VkImageLayout needle, const VkImageLayout *haystack, uint32_t haystackCount)
{
    const VkImageLayout *haystackEnd = haystack + haystackCount;
    return std::find(haystack, haystackEnd, needle) != haystackEnd;
}

gl::TexLevelMask AggregateSkipLevels(const gl::CubeFaceArray<gl::TexLevelMask> &skipLevels)
{
    gl::TexLevelMask skipLevelsAllFaces = skipLevels[0];
    for (size_t face = 1; face < gl::kCubeFaceCount; ++face)
    {
        skipLevelsAllFaces |= skipLevels[face];
    }
    return skipLevelsAllFaces;
}

// Get layer mask for a particular image level.
ImageLayerWriteMask GetImageLayerWriteMask(uint32_t layerStart, uint32_t layerCount)
{
    ImageLayerWriteMask layerMask = angle::BitMask<uint64_t>(layerCount);
    uint32_t rotateShift          = layerStart % kMaxParallelLayerWrites;
    layerMask = (layerMask << rotateShift) | (layerMask >> (kMaxParallelLayerWrites - rotateShift));
    return layerMask;
}

ImageSubresourceRange MakeImageSubresourceReadRange(gl::LevelIndex level,
                                                    uint32_t levelCount,
                                                    uint32_t layer,
                                                    LayerMode layerMode,
                                                    ImageViewColorspace readColorspace,
                                                    ImageViewColorspace writeColorspace)
{
    ImageSubresourceRange range;

    SetBitField(range.level, level.get());
    SetBitField(range.levelCount, levelCount);
    SetBitField(range.layer, layer);
    SetBitField(range.layerMode, layerMode);
    SetBitField(range.readColorspace, readColorspace == ImageViewColorspace::SRGB ? 1 : 0);
    SetBitField(range.writeColorspace, writeColorspace == ImageViewColorspace::SRGB ? 1 : 0);

    return range;
}

ImageSubresourceRange MakeImageSubresourceDrawRange(gl::LevelIndex level,
                                                    uint32_t layer,
                                                    LayerMode layerMode,
                                                    ImageViewColorspace readColorspace,
                                                    ImageViewColorspace writeColorspace)
{
    ImageSubresourceRange range;

    SetBitField(range.level, level.get());
    SetBitField(range.levelCount, 1);
    SetBitField(range.layer, layer);
    SetBitField(range.layerMode, layerMode);
    SetBitField(range.readColorspace, readColorspace == ImageViewColorspace::SRGB ? 1 : 0);
    SetBitField(range.writeColorspace, writeColorspace == ImageViewColorspace::SRGB ? 1 : 0);

    return range;
}

// Obtain VkClearColorValue from input byte data and actual format.
void GetVkClearColorValueFromBytes(uint8_t *actualData,
                                   const angle::Format &actualFormat,
                                   VkClearValue *clearValueOut)
{
    ASSERT(actualData != nullptr && !actualFormat.hasDepthOrStencilBits());

    *clearValueOut               = {};
    VkClearColorValue colorValue = {{}};
    actualFormat.pixelReadFunction(actualData, reinterpret_cast<uint8_t *>(&colorValue));
    clearValueOut->color = colorValue;
}

// Obtain VkClearDepthStencilValue from input byte data and intended format.
void GetVkClearDepthStencilValueFromBytes(uint8_t *intendedData,
                                          const angle::Format &intendedFormat,
                                          VkClearValue *clearValueOut)
{
    ASSERT(intendedData != nullptr && intendedFormat.hasDepthOrStencilBits());

    *clearValueOut     = {};
    uint32_t dsData[4] = {0};
    double depthValue  = 0;

    intendedFormat.pixelReadFunction(intendedData, reinterpret_cast<uint8_t *>(dsData));
    memcpy(&depthValue, &dsData[0], sizeof(double));
    clearValueOut->depthStencil.depth   = static_cast<float>(depthValue);
    clearValueOut->depthStencil.stencil = dsData[2];
}

VkPipelineStageFlags ConvertShaderBitSetToVkPipelineStageFlags(
    const gl::ShaderBitSet &writeShaderStages)
{
    VkPipelineStageFlags pipelineStageFlags = 0;
    for (gl::ShaderType shaderType : writeShaderStages)
    {
        const PipelineStage stage = GetPipelineStage(shaderType);
        pipelineStageFlags |= kBufferMemoryBarrierData[stage].pipelineStageFlags;
    }
    return pipelineStageFlags;
}
}  // anonymous namespace

// This is an arbitrary max. We can change this later if necessary.
uint32_t DynamicDescriptorPool::mMaxSetsPerPool           = 16;
uint32_t DynamicDescriptorPool::mMaxSetsPerPoolMultiplier = 2;

ImageLayout GetImageLayoutFromGLImageLayout(ErrorContext *context, GLenum layout)
{
    const bool supportsMixedReadWriteDepthStencilLayouts =
        context->getFeatures().supportsMixedReadWriteDepthStencilLayouts.enabled;
    switch (layout)
    {
        case GL_NONE:
            return ImageLayout::Undefined;
        case GL_LAYOUT_GENERAL_EXT:
            return ImageLayout::ExternalShadersWrite;
        case GL_LAYOUT_COLOR_ATTACHMENT_EXT:
            return ImageLayout::ColorWrite;
        case GL_LAYOUT_DEPTH_STENCIL_ATTACHMENT_EXT:
            return ImageLayout::DepthWriteStencilWrite;
        case GL_LAYOUT_DEPTH_STENCIL_READ_ONLY_EXT:
            return ImageLayout::DepthReadStencilRead;
        case GL_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_EXT:
            return supportsMixedReadWriteDepthStencilLayouts ? ImageLayout::DepthReadStencilWrite
                                                             : ImageLayout::DepthWriteStencilWrite;
        case GL_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_EXT:
            return supportsMixedReadWriteDepthStencilLayouts ? ImageLayout::DepthWriteStencilRead
                                                             : ImageLayout::DepthWriteStencilWrite;
        case GL_LAYOUT_SHADER_READ_ONLY_EXT:
            return ImageLayout::ExternalShadersReadOnly;
        case GL_LAYOUT_TRANSFER_SRC_EXT:
            return ImageLayout::TransferSrc;
        case GL_LAYOUT_TRANSFER_DST_EXT:
            return ImageLayout::TransferDst;
        default:
            UNREACHABLE();
            return vk::ImageLayout::Undefined;
    }
}

GLenum ConvertImageLayoutToGLImageLayout(ImageLayout layout)
{
    switch (kImageMemoryBarrierData[layout].layout)
    {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            return GL_NONE;
        case VK_IMAGE_LAYOUT_GENERAL:
            return GL_LAYOUT_GENERAL_EXT;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return GL_LAYOUT_COLOR_ATTACHMENT_EXT;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return GL_LAYOUT_DEPTH_STENCIL_ATTACHMENT_EXT;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            return GL_LAYOUT_DEPTH_STENCIL_READ_ONLY_EXT;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return GL_LAYOUT_SHADER_READ_ONLY_EXT;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return GL_LAYOUT_TRANSFER_SRC_EXT;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            return GL_LAYOUT_TRANSFER_DST_EXT;
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
            return GL_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_EXT;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            return GL_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_EXT;
        default:
            break;
    }
    UNREACHABLE();
    return GL_NONE;
}

VkImageLayout ConvertImageLayoutToVkImageLayout(Renderer *renderer, ImageLayout imageLayout)
{
    const ImageMemoryBarrierData &transition = renderer->getImageMemoryBarrierData(imageLayout);
    VkImageLayout layout                     = transition.layout;

    if (ANGLE_LIKELY(renderer->getFeatures().supportsMixedReadWriteDepthStencilLayouts.enabled))
    {
        return layout;
    }

    // If the layouts are not supported, substitute them with what's available.  This may be
    // less optimal and/or introduce synchronization hazards.
    if (layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
        layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL)
    {
        layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // If the replacement layout causes a feedback loop, use the GENERAL layout
        if ((transition.dstStageMask &
             (VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)) != 0)
        {
            layout = VK_IMAGE_LAYOUT_GENERAL;
        }
    }

    return layout;
}

PipelineStageGroup GetPipelineStageGroupFromStageFlags(VkPipelineStageFlags dstStageMask)
{
    if ((dstStageMask & ~kFragmentAndAttachmentPipelineStageFlags) == 0)
    {
        return PipelineStageGroup::FragmentOnly;
    }
    else if (dstStageMask == VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT)
    {
        return PipelineStageGroup::ComputeOnly;
    }
    else if ((dstStageMask & ~kPreFragmentStageFlags) == 0)
    {
        return PipelineStageGroup::PreFragmentOnly;
    }
    return PipelineStageGroup::Other;
}

void InitializeImageLayoutAndMemoryBarrierDataMap(
    ImageLayoutToMemoryBarrierDataMap *map,
    VkPipelineStageFlags supportedVulkanPipelineStageMask)
{
    *map = kImageMemoryBarrierData;
    for (ImageMemoryBarrierData &barrierData : *map)
    {
        barrierData.srcStageMask &= supportedVulkanPipelineStageMask;
        barrierData.dstStageMask &= supportedVulkanPipelineStageMask;
        ASSERT(barrierData.pipelineStageGroup ==
               GetPipelineStageGroupFromStageFlags(barrierData.dstStageMask));
    }
}

bool FormatHasNecessaryFeature(Renderer *renderer,
                               angle::FormatID formatID,
                               VkImageTiling tilingMode,
                               VkFormatFeatureFlags featureBits)
{
    return (tilingMode == VK_IMAGE_TILING_OPTIMAL)
               ? renderer->hasImageFormatFeatureBits(formatID, featureBits)
               : renderer->hasLinearImageFormatFeatureBits(formatID, featureBits);
}

bool CanCopyWithTransfer(Renderer *renderer,
                         angle::FormatID srcFormatID,
                         VkImageTiling srcTilingMode,
                         angle::FormatID dstFormatID,
                         VkImageTiling dstTilingMode)
{
    // Checks that the formats in the copy transfer have the appropriate tiling and transfer bits
    bool isTilingCompatible           = srcTilingMode == dstTilingMode;
    bool srcFormatHasNecessaryFeature = FormatHasNecessaryFeature(
        renderer, srcFormatID, srcTilingMode, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT);
    bool dstFormatHasNecessaryFeature = FormatHasNecessaryFeature(
        renderer, dstFormatID, dstTilingMode, VK_FORMAT_FEATURE_TRANSFER_DST_BIT);

    return isTilingCompatible && srcFormatHasNecessaryFeature && dstFormatHasNecessaryFeature;
}

void InitializeEventStageToVkPipelineStageFlagsMap(
    EventStageToVkPipelineStageFlagsMap *map,
    VkPipelineStageFlags supportedVulkanPipelineStageMask)
{
    map->fill(0);

    for (const BufferMemoryBarrierData &bufferBarrierData : kBufferMemoryBarrierData)
    {
        const EventStage eventStage = bufferBarrierData.eventStage;
        if (eventStage != EventStage::InvalidEnum)
        {
            (*map)[eventStage] |=
                bufferBarrierData.pipelineStageFlags & supportedVulkanPipelineStageMask;
        }
    }

    for (const ImageMemoryBarrierData &imageBarrierData : kImageMemoryBarrierData)
    {
        const EventStage eventStage = imageBarrierData.eventStage;
        if (eventStage != EventStage::InvalidEnum)
        {
            (*map)[eventStage] |= imageBarrierData.dstStageMask & supportedVulkanPipelineStageMask;
        }
    }
}

// Context implementation
Context::Context(Renderer *renderer)
    : ErrorContext(renderer), mShareGroupRefCountedEventsGarbageRecycler(nullptr)
{}

Context::~Context()
{
    ASSERT(mForeignImagesInUse.empty());
}

void Context::onForeignImageUse(ImageHelper *image)
{
    // The image might be used multiple times in the same frame, |mForeignImagesInUse| is a "set"
    // so the image is tracked only once.
    mForeignImagesInUse.insert(image);
}

void Context::finalizeForeignImage(ImageHelper *image)
{
    // The image must have been marked as in use, otherwise finalize is called while the initial use
    // was missed.
    ASSERT(mForeignImagesInUse.find(image) != mForeignImagesInUse.end());
    // The image must not already be finalized.
    ASSERT(
        std::find_if(mImagesToTransitionToForeign.begin(), mImagesToTransitionToForeign.end(),
                     [image = image->getImage().getHandle()](const VkImageMemoryBarrier &barrier) {
                         return barrier.image == image;
                     }) == mImagesToTransitionToForeign.end());

    mImagesToTransitionToForeign.push_back(image->releaseToForeign(mRenderer));
    mForeignImagesInUse.erase(image);
}

void Context::finalizeAllForeignImages()
{
    mImagesToTransitionToForeign.reserve(mImagesToTransitionToForeign.size() +
                                         mForeignImagesInUse.size());
    while (!mForeignImagesInUse.empty())
    {
        finalizeForeignImage(*mForeignImagesInUse.begin());
    }
}

// PackedClearValuesArray implementation
PackedClearValuesArray::PackedClearValuesArray() : mValues{} {}
PackedClearValuesArray::~PackedClearValuesArray() = default;

PackedClearValuesArray::PackedClearValuesArray(const PackedClearValuesArray &other) = default;
PackedClearValuesArray &PackedClearValuesArray::operator=(const PackedClearValuesArray &rhs) =
    default;

void PackedClearValuesArray::storeColor(PackedAttachmentIndex index, const VkClearValue &clearValue)
{
    mValues[index.get()] = clearValue;
}

void PackedClearValuesArray::storeDepthStencil(PackedAttachmentIndex index,
                                               const VkClearValue &clearValue)
{
    mValues[index.get()] = clearValue;
}

// RenderPassAttachment implementation
RenderPassAttachment::RenderPassAttachment()
{
    reset();
}

void RenderPassAttachment::init(ImageHelper *image,
                                UniqueSerial imageSiblingSerial,
                                gl::LevelIndex levelIndex,
                                uint32_t layerIndex,
                                uint32_t layerCount,
                                VkImageAspectFlagBits aspect)
{
    ASSERT(mImage == nullptr);

    mImage              = image;
    mImageSiblingSerial = imageSiblingSerial;
    mLevelIndex         = levelIndex;
    mLayerIndex         = layerIndex;
    mLayerCount         = layerCount;
    mAspect             = aspect;

    mImage->setRenderPassUsageFlag(RenderPassUsage::RenderTargetAttachment);
}

void RenderPassAttachment::reset()
{
    mImage = nullptr;

    mAccess = ResourceAccess::Unused;

    mInvalidatedCmdCount = kInfiniteCmdCount;
    mDisabledCmdCount    = kInfiniteCmdCount;
    mInvalidateArea      = gl::Rectangle();
}

void RenderPassAttachment::onAccess(ResourceAccess access, uint32_t currentCmdCount)
{
    // Update the access for optimizing this render pass's loadOp
    UpdateAccess(&mAccess, access);

    // Update the invalidate state for optimizing this render pass's storeOp
    if (onAccessImpl(access, currentCmdCount))
    {
        // The attachment is no longer invalid, so restore its content.
        restoreContent();
    }
}

void RenderPassAttachment::invalidate(const gl::Rectangle &invalidateArea,
                                      bool isAttachmentEnabled,
                                      uint32_t currentCmdCount)
{
    // Keep track of the command count in the render pass at the time of invalidation.  If there are
    // more commands in the future, invalidate must be undone.
    mInvalidatedCmdCount = currentCmdCount;

    // Also track the command count if the attachment is currently disabled.
    mDisabledCmdCount = isAttachmentEnabled ? kInfiniteCmdCount : currentCmdCount;

    // Set/extend the invalidate area.
    ExtendRenderPassInvalidateArea(invalidateArea, &mInvalidateArea);
}

void RenderPassAttachment::onRenderAreaGrowth(ContextVk *contextVk,
                                              const gl::Rectangle &newRenderArea)
{
    // Remove invalidate if it's no longer applicable.
    if (mInvalidateArea.empty() || mInvalidateArea.encloses(newRenderArea))
    {
        return;
    }

    ANGLE_VK_PERF_WARNING(contextVk, GL_DEBUG_SEVERITY_LOW,
                          "InvalidateSubFramebuffer discarded due to increased scissor region");

    mInvalidateArea      = gl::Rectangle();
    mInvalidatedCmdCount = kInfiniteCmdCount;
}

void RenderPassAttachment::finalizeLoadStore(ErrorContext *context,
                                             uint32_t currentCmdCount,
                                             bool hasUnresolveAttachment,
                                             bool hasResolveAttachment,
                                             RenderPassLoadOp *loadOp,
                                             RenderPassStoreOp *storeOp,
                                             bool *isInvalidatedOut)
{
    if (mAspect != VK_IMAGE_ASPECT_COLOR_BIT)
    {
        const RenderPassUsage readOnlyAttachmentUsage =
            mAspect == VK_IMAGE_ASPECT_STENCIL_BIT ? RenderPassUsage::StencilReadOnlyAttachment
                                                   : RenderPassUsage::DepthReadOnlyAttachment;
        // Ensure we don't write to a read-only attachment. (ReadOnly -> !Write)
        ASSERT(!mImage->hasRenderPassUsageFlag(readOnlyAttachmentUsage) ||
               !HasResourceWriteAccess(mAccess));
    }

    // If the attachment is invalidated, skip the store op.  If we are not loading or clearing the
    // attachment and the attachment has not been used, auto-invalidate it.
    const bool notLoaded = *loadOp == RenderPassLoadOp::DontCare && !hasUnresolveAttachment;
    if (isInvalidated(currentCmdCount) || (notLoaded && !HasResourceWriteAccess(mAccess)))
    {
        *storeOp          = RenderPassStoreOp::DontCare;
        *isInvalidatedOut = true;
    }
    else if (hasWriteAfterInvalidate(currentCmdCount))
    {
        // The attachment was invalidated, but is now valid.  Let the image know the contents are
        // now defined so a future render pass would use loadOp=LOAD.
        restoreContent();
    }

    // For read only depth stencil, we can use StoreOpNone if available.  DontCare is still
    // preferred, so do this after handling DontCare.
    const bool supportsLoadStoreOpNone =
        context->getFeatures().supportsRenderPassLoadStoreOpNone.enabled;
    const bool supportsStoreOpNone =
        supportsLoadStoreOpNone || context->getFeatures().supportsRenderPassStoreOpNone.enabled;
    if (mAccess == ResourceAccess::ReadOnly && supportsStoreOpNone)
    {
        if (*storeOp == RenderPassStoreOp::Store && *loadOp != RenderPassLoadOp::Clear)
        {
            *storeOp = RenderPassStoreOp::None;
        }
    }

    if (mAccess == ResourceAccess::Unused)
    {
        if (*storeOp != RenderPassStoreOp::DontCare)
        {
            switch (*loadOp)
            {
                case RenderPassLoadOp::Clear:
                    // Cannot optimize away the ops if the attachment is cleared (even if not used
                    // afterwards)
                    break;
                case RenderPassLoadOp::Load:
                    // Make sure the attachment is neither loaded nor stored (as it's neither used
                    // nor invalidated), if possible.
                    if (supportsLoadStoreOpNone)
                    {
                        *loadOp = RenderPassLoadOp::None;
                    }
                    if (supportsStoreOpNone)
                    {
                        *storeOp = RenderPassStoreOp::None;
                    }
                    break;
                case RenderPassLoadOp::DontCare:
                    // loadOp=DontCare should be covered by storeOp=DontCare below.
                    break;
                case RenderPassLoadOp::None:
                default:
                    // loadOp=None is never decided upfront.
                    UNREACHABLE();
                    break;
            }
        }
    }

    if (mAccess == ResourceAccess::Unused || (mAccess == ResourceAccess::ReadOnly && notLoaded))
    {
        // If we are loading or clearing the attachment, but the attachment has not been used,
        // and the data has also not been stored back into attachment, then just skip the
        // load/clear op. If loadOp/storeOp=None is supported, prefer that to reduce the amount
        // of synchronization; DontCare is a write operation, while None is not.
        //
        // Don't optimize away a Load or Clear if there is a resolve attachment. Although the
        // storeOp=DontCare the image content needs to be resolved into the resolve attachment.
        const bool attachmentNeedsToBeResolved =
            hasResolveAttachment &&
            (*loadOp == RenderPassLoadOp::Load || *loadOp == RenderPassLoadOp::Clear);
        if (*storeOp == RenderPassStoreOp::DontCare && !attachmentNeedsToBeResolved)
        {
            if (supportsLoadStoreOpNone && !isInvalidated(currentCmdCount))
            {
                *loadOp  = RenderPassLoadOp::None;
                *storeOp = RenderPassStoreOp::None;
            }
            else
            {
                *loadOp = RenderPassLoadOp::DontCare;
            }
        }
    }
}

void RenderPassAttachment::restoreContent()
{
    // Note that the image may have been deleted since the render pass has started.
    if (mImage)
    {
        ASSERT(mImage->valid());
        if (mAspect == VK_IMAGE_ASPECT_STENCIL_BIT)
        {
            mImage->restoreSubresourceStencilContent(mLevelIndex, mLayerIndex, mLayerCount);
        }
        else
        {
            mImage->restoreSubresourceContent(mLevelIndex, mLayerIndex, mLayerCount);
        }
        mInvalidateArea = gl::Rectangle();
    }
}

bool RenderPassAttachment::hasWriteAfterInvalidate(uint32_t currentCmdCount) const
{
    return (mInvalidatedCmdCount != kInfiniteCmdCount &&
            std::min(mDisabledCmdCount, currentCmdCount) != mInvalidatedCmdCount);
}

bool RenderPassAttachment::isInvalidated(uint32_t currentCmdCount) const
{
    return mInvalidatedCmdCount != kInfiniteCmdCount &&
           std::min(mDisabledCmdCount, currentCmdCount) == mInvalidatedCmdCount;
}

bool RenderPassAttachment::onAccessImpl(ResourceAccess access, uint32_t currentCmdCount)
{
    if (mInvalidatedCmdCount == kInfiniteCmdCount)
    {
        // If never invalidated or no longer invalidated, return early.
        return false;
    }
    if (HasResourceWriteAccess(access))
    {
        // Drawing to this attachment is being enabled.  Assume that drawing will immediately occur
        // after this attachment is enabled, and that means that the attachment will no longer be
        // invalidated.
        mInvalidatedCmdCount = kInfiniteCmdCount;
        mDisabledCmdCount    = kInfiniteCmdCount;
        // Return true to indicate that the store op should remain STORE and that mContentDefined
        // should be set to true;
        return true;
    }
    // Drawing to this attachment is being disabled.
    if (hasWriteAfterInvalidate(currentCmdCount))
    {
        // The attachment was previously drawn while enabled, and so is no longer invalidated.
        mInvalidatedCmdCount = kInfiniteCmdCount;
        mDisabledCmdCount    = kInfiniteCmdCount;
        // Return true to indicate that the store op should remain STORE and that mContentDefined
        // should be set to true;
        return true;
    }

    // Use the latest CmdCount at the start of being disabled.  At the end of the render pass,
    // cmdCountDisabled is <= the actual command count, and so it's compared with
    // cmdCountInvalidated.  If the same, the attachment is still invalidated.
    mDisabledCmdCount = currentCmdCount;
    return false;
}

// CommandBufferHelperCommon implementation.
CommandBufferHelperCommon::CommandBufferHelperCommon()
    : mCommandPool(nullptr), mHasShaderStorageOutput(false), mHasGLMemoryBarrierIssued(false)
{}

CommandBufferHelperCommon::~CommandBufferHelperCommon() {}

void CommandBufferHelperCommon::initializeImpl()
{
    mCommandAllocator.init();
}

void CommandBufferHelperCommon::resetImpl(ErrorContext *context)
{
    ASSERT(!mAcquireNextImageSemaphore.valid());
    mCommandAllocator.resetAllocator();
    ASSERT(!mIsAnyHostVisibleBufferWritten);

    ASSERT(mRefCountedEvents.empty());
    ASSERT(mRefCountedEventCollector.empty());
}

template <class DerivedT>
angle::Result CommandBufferHelperCommon::attachCommandPoolImpl(ErrorContext *context,
                                                               SecondaryCommandPool *commandPool)
{
    if constexpr (!DerivedT::ExecutesInline())
    {
        DerivedT *derived = static_cast<DerivedT *>(this);
        ASSERT(commandPool != nullptr);
        ASSERT(mCommandPool == nullptr);
        ASSERT(!derived->getCommandBuffer().valid());

        mCommandPool = commandPool;

        ANGLE_TRY(derived->initializeCommandBuffer(context));
    }
    return angle::Result::Continue;
}

template <class DerivedT, bool kIsRenderPassBuffer>
angle::Result CommandBufferHelperCommon::detachCommandPoolImpl(
    ErrorContext *context,
    SecondaryCommandPool **commandPoolOut)
{
    if constexpr (!DerivedT::ExecutesInline())
    {
        DerivedT *derived = static_cast<DerivedT *>(this);
        ASSERT(mCommandPool != nullptr);
        ASSERT(derived->getCommandBuffer().valid());

        if constexpr (!kIsRenderPassBuffer)
        {
            ASSERT(!derived->getCommandBuffer().empty());
            ANGLE_TRY(derived->endCommandBuffer(context));
        }

        *commandPoolOut = mCommandPool;
        mCommandPool    = nullptr;
    }
    ASSERT(mCommandPool == nullptr);
    return angle::Result::Continue;
}

template <class DerivedT>
void CommandBufferHelperCommon::releaseCommandPoolImpl()
{
    if constexpr (!DerivedT::ExecutesInline())
    {
        DerivedT *derived = static_cast<DerivedT *>(this);
        ASSERT(mCommandPool != nullptr);

        if (derived->getCommandBuffer().valid())
        {
            ASSERT(derived->getCommandBuffer().empty());
            mCommandPool->collect(&derived->getCommandBuffer());
        }

        mCommandPool = nullptr;
    }
    ASSERT(mCommandPool == nullptr);
}

template <class DerivedT>
void CommandBufferHelperCommon::attachAllocatorImpl(SecondaryCommandMemoryAllocator *allocator)
{
    if constexpr (DerivedT::ExecutesInline())
    {
        auto &commandBuffer = static_cast<DerivedT *>(this)->getCommandBuffer();
        mCommandAllocator.attachAllocator(allocator);
        commandBuffer.attachAllocator(mCommandAllocator.getAllocator());
    }
}

template <class DerivedT>
SecondaryCommandMemoryAllocator *CommandBufferHelperCommon::detachAllocatorImpl()
{
    SecondaryCommandMemoryAllocator *result = nullptr;
    if constexpr (DerivedT::ExecutesInline())
    {
        auto &commandBuffer = static_cast<DerivedT *>(this)->getCommandBuffer();
        commandBuffer.detachAllocator(mCommandAllocator.getAllocator());
        result = mCommandAllocator.detachAllocator(commandBuffer.empty());
    }
    return result;
}

template <class DerivedT>
void CommandBufferHelperCommon::assertCanBeRecycledImpl()
{
    DerivedT *derived = static_cast<DerivedT *>(this);
    ASSERT(mCommandPool == nullptr);
    ASSERT(!mCommandAllocator.hasAllocatorLinks());
    // Vulkan secondary command buffers must be invalid (collected).
    ASSERT(DerivedT::ExecutesInline() || !derived->getCommandBuffer().valid());
    // ANGLEs Custom secondary command buffers must be empty (reset).
    ASSERT(!DerivedT::ExecutesInline() || derived->getCommandBuffer().empty());
}

void CommandBufferHelperCommon::bufferWrite(Context *context,
                                            VkAccessFlags writeAccessType,
                                            PipelineStage writeStage,
                                            BufferHelper *buffer)
{
    VkPipelineStageFlags writePipelineStageFlags =
        kBufferMemoryBarrierData[writeStage].pipelineStageFlags;
    bufferWriteImpl(context, writeAccessType, writePipelineStageFlags, writeStage, buffer);
}

void CommandBufferHelperCommon::bufferWrite(Context *context,
                                            VkAccessFlags writeAccessType,
                                            const gl::ShaderBitSet &writeShaderStages,
                                            BufferHelper *buffer)
{
    VkPipelineStageFlags writePipelineStageFlags =
        ConvertShaderBitSetToVkPipelineStageFlags(writeShaderStages);
    PipelineStage firstWriteStage = GetPipelineStage(writeShaderStages.first());
    bufferWriteImpl(context, writeAccessType, writePipelineStageFlags, firstWriteStage, buffer);
}

void CommandBufferHelperCommon::bufferRead(Context *context,
                                           VkAccessFlags readAccessType,
                                           PipelineStage readStage,
                                           BufferHelper *buffer)
{
    VkPipelineStageFlags readPipelineStageFlags =
        kBufferMemoryBarrierData[readStage].pipelineStageFlags;
    bufferReadImpl(context, readAccessType, readPipelineStageFlags, readStage, buffer);
}

void CommandBufferHelperCommon::bufferRead(Context *context,
                                           VkAccessFlags readAccessType,
                                           const gl::ShaderBitSet &readShaderStages,
                                           BufferHelper *buffer)
{
    for (const gl::ShaderType shaderType : readShaderStages)
    {
        PipelineStage readStage = GetPipelineStage(shaderType);
        VkPipelineStageFlags readPipelineStageFlags =
            kBufferMemoryBarrierData[readStage].pipelineStageFlags;
        bufferReadImpl(context, readAccessType, readPipelineStageFlags, readStage, buffer);
    }
}

void CommandBufferHelperCommon::bufferWriteImpl(Context *context,
                                                VkAccessFlags writeAccessType,
                                                VkPipelineStageFlags writePipelineStageFlags,
                                                PipelineStage writeStage,
                                                BufferHelper *buffer)
{
    buffer->recordWriteBarrier(context, writeAccessType, writePipelineStageFlags, writeStage,
                               mQueueSerial, &mPipelineBarriers, &mEventBarriers,
                               &mRefCountedEventCollector);

    // Make sure host-visible buffer writes result in a barrier inserted at the end of the frame to
    // make the results visible to the host.  The buffer may be mapped by the application in the
    // future.
    if (buffer->isHostVisible())
    {
        mIsAnyHostVisibleBufferWritten = true;
    }

    buffer->recordWriteEvent(context, writeAccessType, writePipelineStageFlags, mQueueSerial,
                             writeStage, &mRefCountedEvents);
}

void CommandBufferHelperCommon::bufferReadImpl(Context *context,
                                               VkAccessFlags readAccessType,
                                               VkPipelineStageFlags readPipelineStageFlags,
                                               PipelineStage readStage,
                                               BufferHelper *buffer)
{
    buffer->recordReadBarrier(context, readAccessType, readPipelineStageFlags, readStage,
                              &mPipelineBarriers, &mEventBarriers, &mRefCountedEventCollector);
    ASSERT(!usesBufferForWrite(*buffer));

    buffer->recordReadEvent(context, readAccessType, readPipelineStageFlags, readStage,
                            mQueueSerial, kBufferMemoryBarrierData[readStage].eventStage,
                            &mRefCountedEvents);
}

void CommandBufferHelperCommon::imageReadImpl(Context *context,
                                              VkImageAspectFlags aspectFlags,
                                              ImageLayout imageLayout,
                                              BarrierType barrierType,
                                              ImageHelper *image)
{
    if (image->isReadBarrierNecessary(context->getRenderer(), imageLayout))
    {
        updateImageLayoutAndBarrier(context, image, aspectFlags, imageLayout, barrierType);
    }
}

void CommandBufferHelperCommon::imageWriteImpl(Context *context,
                                               gl::LevelIndex level,
                                               uint32_t layerStart,
                                               uint32_t layerCount,
                                               VkImageAspectFlags aspectFlags,
                                               ImageLayout imageLayout,
                                               BarrierType barrierType,
                                               ImageHelper *image)
{
    image->onWrite(level, 1, layerStart, layerCount, aspectFlags);
    if (image->isWriteBarrierNecessary(imageLayout, level, 1, layerStart, layerCount))
    {
        updateImageLayoutAndBarrier(context, image, aspectFlags, imageLayout, barrierType);
    }
}

void CommandBufferHelperCommon::updateImageLayoutAndBarrier(Context *context,
                                                            ImageHelper *image,
                                                            VkImageAspectFlags aspectFlags,
                                                            ImageLayout imageLayout,
                                                            BarrierType barrierType)
{
    VkSemaphore semaphore = VK_NULL_HANDLE;
    image->updateLayoutAndBarrier(context, aspectFlags, imageLayout, barrierType, mQueueSerial,
                                  &mPipelineBarriers, &mEventBarriers, &mRefCountedEventCollector,
                                  &semaphore);
    // If image has an ANI semaphore, move it to command buffer so that we can wait for it in
    // next submission.
    if (semaphore != VK_NULL_HANDLE)
    {
        ASSERT(!mAcquireNextImageSemaphore.valid());
        mAcquireNextImageSemaphore.setHandle(semaphore);
    }
}

void CommandBufferHelperCommon::retainImageWithEvent(Context *context, ImageHelper *image)
{
    image->setQueueSerial(mQueueSerial);
    image->updatePipelineStageAccessHistory();

    if (context->getFeatures().useVkEventForImageBarrier.enabled)
    {
        image->setCurrentRefCountedEvent(context, &mRefCountedEvents);
    }
}

template <typename CommandBufferT>
void CommandBufferHelperCommon::flushSetEventsImpl(Context *context, CommandBufferT *commandBuffer)
{
    if (mRefCountedEvents.empty())
    {
        return;
    }

    // Add VkCmdSetEvent here to track the completion of this renderPass.
    mRefCountedEvents.flushSetEvents(context->getRenderer(), commandBuffer);
    // We no longer need event, so garbage collect it.
    mRefCountedEvents.releaseToEventCollector(&mRefCountedEventCollector);
}

template void CommandBufferHelperCommon::flushSetEventsImpl<priv::SecondaryCommandBuffer>(
    Context *context,
    priv::SecondaryCommandBuffer *commandBuffer);
template void CommandBufferHelperCommon::flushSetEventsImpl<VulkanSecondaryCommandBuffer>(
    Context *context,
    VulkanSecondaryCommandBuffer *commandBuffer);

void CommandBufferHelperCommon::executeBarriers(Renderer *renderer, CommandsState *commandsState)
{
    // Add ANI semaphore to the command submission.
    if (mAcquireNextImageSemaphore.valid())
    {
        commandsState->waitSemaphores.emplace_back(mAcquireNextImageSemaphore.release());
        commandsState->waitSemaphoreStageMasks.emplace_back(kSwapchainAcquireImageWaitStageFlags);
    }

    mPipelineBarriers.execute(renderer, &commandsState->primaryCommands);
    mEventBarriers.execute(renderer, &commandsState->primaryCommands);
}

void CommandBufferHelperCommon::addCommandDiagnosticsCommon(std::ostringstream *out)
{
    mPipelineBarriers.addDiagnosticsString(*out);
    mEventBarriers.addDiagnosticsString(*out);
}

// OutsideRenderPassCommandBufferHelper implementation.
OutsideRenderPassCommandBufferHelper::OutsideRenderPassCommandBufferHelper() {}

OutsideRenderPassCommandBufferHelper::~OutsideRenderPassCommandBufferHelper() {}

angle::Result OutsideRenderPassCommandBufferHelper::initialize(ErrorContext *context)
{
    initializeImpl();
    return initializeCommandBuffer(context);
}
angle::Result OutsideRenderPassCommandBufferHelper::initializeCommandBuffer(ErrorContext *context)
{
    // Skip initialization in the Pool-detached state.
    if (!ExecutesInline() && mCommandPool == nullptr)
    {
        return angle::Result::Continue;
    }
    return mCommandBuffer.initialize(context, mCommandPool, false,
                                     mCommandAllocator.getAllocator());
}

angle::Result OutsideRenderPassCommandBufferHelper::reset(
    ErrorContext *context,
    SecondaryCommandBufferCollector *commandBufferCollector)
{
    resetImpl(context);

    // Collect/Reset the command buffer
    commandBufferCollector->collectCommandBuffer(std::move(mCommandBuffer));
    mIsCommandBufferEnded = false;

    // Invalidate the queue serial here. We will get a new queue serial after commands flush.
    mQueueSerial = QueueSerial();

    return initializeCommandBuffer(context);
}

void OutsideRenderPassCommandBufferHelper::imageRead(Context *context,
                                                     VkImageAspectFlags aspectFlags,
                                                     ImageLayout imageLayout,
                                                     ImageHelper *image)
{
    if (image->getResourceUse() >= mQueueSerial)
    {
        // If image is already used by renderPass, it may already set the event to renderPass's
        // event. In this case we already lost the previous event to wait for, thus use pipeline
        // barrier instead of event
        imageReadImpl(context, aspectFlags, imageLayout, BarrierType::Pipeline, image);
    }
    else
    {
        imageReadImpl(context, aspectFlags, imageLayout, BarrierType::Event, image);
        // Usually an image can only used by a RenderPassCommands or OutsideRenderPassCommands
        // because the layout will be different, except with image sampled from compute shader. In
        // this case, the renderPassCommands' read will override the outsideRenderPassCommands'
        retainImageWithEvent(context, image);
    }
}

void OutsideRenderPassCommandBufferHelper::imageWrite(Context *context,
                                                      gl::LevelIndex level,
                                                      uint32_t layerStart,
                                                      uint32_t layerCount,
                                                      VkImageAspectFlags aspectFlags,
                                                      ImageLayout imageLayout,
                                                      ImageHelper *image)
{
    imageWriteImpl(context, level, layerStart, layerCount, aspectFlags, imageLayout,
                   BarrierType::Event, image);
    retainImageWithEvent(context, image);
}

void OutsideRenderPassCommandBufferHelper::retainImage(ImageHelper *image)
{
    // We want explicit control on when VkEvent is used for outsideRPCommands to minimize the
    // overhead, so do not setEvent here.
    image->setQueueSerial(mQueueSerial);
    image->updatePipelineStageAccessHistory();
}

void OutsideRenderPassCommandBufferHelper::trackImageWithEvent(Context *context, ImageHelper *image)
{
    image->setCurrentRefCountedEvent(context, &mRefCountedEvents);
    flushSetEventsImpl(context, &mCommandBuffer);
}

void OutsideRenderPassCommandBufferHelper::collectRefCountedEventsGarbage(
    RefCountedEventsGarbageRecycler *garbageRecycler)
{
    ASSERT(garbageRecycler != nullptr);
    if (!mRefCountedEventCollector.empty())
    {
        garbageRecycler->collectGarbage(mQueueSerial, std::move(mRefCountedEventCollector));
    }
}

angle::Result OutsideRenderPassCommandBufferHelper::flushToPrimary(Context *context,
                                                                   CommandsState *commandsState)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "OutsideRenderPassCommandBufferHelper::flushToPrimary");
    ASSERT(!empty());

    Renderer *renderer = context->getRenderer();

    // Commands that are added to primary before beginRenderPass command
    executeBarriers(renderer, commandsState);

    ANGLE_TRY(endCommandBuffer(context));
    ASSERT(mIsCommandBufferEnded);
    mCommandBuffer.executeCommands(&commandsState->primaryCommands);

    // Call VkCmdSetEvent to track the completion of this renderPass.
    flushSetEventsImpl(context, &commandsState->primaryCommands);

    // Proactively reset all released events before ending command buffer.
    context->getRenderer()->getRefCountedEventRecycler()->resetEvents(
        context, mQueueSerial, &commandsState->primaryCommands);

    // Restart the command buffer.
    return reset(context, &commandsState->secondaryCommands);
}

angle::Result OutsideRenderPassCommandBufferHelper::endCommandBuffer(ErrorContext *context)
{
    ASSERT(ExecutesInline() || mCommandPool != nullptr);
    ASSERT(mCommandBuffer.valid());
    ASSERT(!mIsCommandBufferEnded);

    ANGLE_TRY(mCommandBuffer.end(context));
    mIsCommandBufferEnded = true;

    return angle::Result::Continue;
}

angle::Result OutsideRenderPassCommandBufferHelper::attachCommandPool(
    ErrorContext *context,
    SecondaryCommandPool *commandPool)
{
    return attachCommandPoolImpl<OutsideRenderPassCommandBufferHelper>(context, commandPool);
}

angle::Result OutsideRenderPassCommandBufferHelper::detachCommandPool(
    ErrorContext *context,
    SecondaryCommandPool **commandPoolOut)
{
    return detachCommandPoolImpl<OutsideRenderPassCommandBufferHelper, false>(context,
                                                                              commandPoolOut);
}

void OutsideRenderPassCommandBufferHelper::releaseCommandPool()
{
    releaseCommandPoolImpl<OutsideRenderPassCommandBufferHelper>();
}

void OutsideRenderPassCommandBufferHelper::attachAllocator(
    SecondaryCommandMemoryAllocator *allocator)
{
    attachAllocatorImpl<OutsideRenderPassCommandBufferHelper>(allocator);
}

SecondaryCommandMemoryAllocator *OutsideRenderPassCommandBufferHelper::detachAllocator()
{
    return detachAllocatorImpl<OutsideRenderPassCommandBufferHelper>();
}

void OutsideRenderPassCommandBufferHelper::assertCanBeRecycled()
{
    assertCanBeRecycledImpl<OutsideRenderPassCommandBufferHelper>();
}

std::string OutsideRenderPassCommandBufferHelper::getCommandDiagnostics()
{
    std::ostringstream out;
    addCommandDiagnosticsCommon(&out);

    out << mCommandBuffer.dumpCommands("\\l");

    return out.str();
}

// RenderPassFramebuffer implementation.
void RenderPassFramebuffer::reset()
{
    mInitialFramebuffer.release();
    mImageViews.clear();
    mIsImageless = false;
    mIsDefault   = false;
}

void RenderPassFramebuffer::addResolveAttachment(size_t viewIndex, VkImageView view)
{
    // The initial framebuffer is no longer usable.
    mInitialFramebuffer.release();

    if (viewIndex >= mImageViews.size())
    {
        mImageViews.resize(viewIndex + 1, VK_NULL_HANDLE);
    }

    ASSERT(mImageViews[viewIndex] == VK_NULL_HANDLE);
    mImageViews[viewIndex] = view;
}

angle::Result RenderPassFramebuffer::packResolveViewsAndCreateFramebuffer(
    ErrorContext *context,
    const RenderPass &renderPass,
    Framebuffer *framebufferOut)
{
    // This is only called if the initial framebuffer was not usable.  Since this is called when
    // the render pass is finalized, the render pass that is passed in is the final one (not a
    // compatible one) and the framebuffer that is created is not imageless.
    ASSERT(!mInitialFramebuffer.valid());

    PackViews(&mImageViews);
    mIsImageless = false;

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.flags                   = 0;
    framebufferInfo.renderPass              = renderPass.getHandle();
    framebufferInfo.attachmentCount         = static_cast<uint32_t>(mImageViews.size());
    framebufferInfo.pAttachments            = mImageViews.data();
    framebufferInfo.width                   = mWidth;
    framebufferInfo.height                  = mHeight;
    framebufferInfo.layers                  = mLayers;

    ANGLE_VK_TRY(context, framebufferOut->init(context->getDevice(), framebufferInfo));
    return angle::Result::Continue;
}

void RenderPassFramebuffer::packResolveViewsForRenderPassBegin(
    VkRenderPassAttachmentBeginInfo *beginInfoOut)
{
    // Called when using the initial framebuffer which is imageless
    ASSERT(mInitialFramebuffer.valid());
    ASSERT(mIsImageless);

    PackViews(&mImageViews);

    *beginInfoOut                 = {};
    beginInfoOut->sType           = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO_KHR;
    beginInfoOut->attachmentCount = static_cast<uint32_t>(mImageViews.size());
    beginInfoOut->pAttachments    = mImageViews.data();
}

// static
void RenderPassFramebuffer::PackViews(FramebufferAttachmentsVector<VkImageView> *views)
{
    PackedAttachmentIndex packIndex = kAttachmentIndexZero;
    for (size_t viewIndex = 0; viewIndex < views->size(); ++viewIndex)
    {
        if ((*views)[viewIndex] != VK_NULL_HANDLE)
        {
            (*views)[packIndex.get()] = (*views)[viewIndex];
            ++packIndex;
        }
    }

    views->resize(packIndex.get());
}

// RenderPassCommandBufferHelper implementation.
RenderPassCommandBufferHelper::RenderPassCommandBufferHelper()
    : mCurrentSubpassCommandBufferIndex(0),
      mCounter(0),
      mClearValues{},
      mRenderPassStarted(false),
      mTransformFeedbackCounterBuffers{},
      mTransformFeedbackCounterBufferOffsets{},
      mValidTransformFeedbackBufferCount(0),
      mRebindTransformFeedbackBuffers(false),
      mIsTransformFeedbackActiveUnpaused(false),
      mPreviousSubpassesCmdCount(0),
      mDepthStencilAttachmentIndex(kAttachmentIndexInvalid),
      mColorAttachmentsCount(0),
      mImageOptimizeForPresent(nullptr),
      mImageOptimizeForPresentOriginalLayout(ImageLayout::Undefined)
{}

RenderPassCommandBufferHelper::~RenderPassCommandBufferHelper() {}

angle::Result RenderPassCommandBufferHelper::initialize(ErrorContext *context)
{
    initializeImpl();
    return initializeCommandBuffer(context);
}
angle::Result RenderPassCommandBufferHelper::initializeCommandBuffer(ErrorContext *context)
{
    // Skip initialization in the Pool-detached state.
    if (!ExecutesInline() && mCommandPool == nullptr)
    {
        return angle::Result::Continue;
    }
    return getCommandBuffer().initialize(context, mCommandPool, true,
                                         mCommandAllocator.getAllocator());
}

angle::Result RenderPassCommandBufferHelper::reset(
    ErrorContext *context,
    SecondaryCommandBufferCollector *commandBufferCollector)
{
    resetImpl(context);

    for (PackedAttachmentIndex index = kAttachmentIndexZero; index < mColorAttachmentsCount;
         ++index)
    {
        mColorAttachments[index].reset();
        mColorResolveAttachments[index].reset();
    }

    mDepthAttachment.reset();
    mDepthResolveAttachment.reset();
    mStencilAttachment.reset();
    mStencilResolveAttachment.reset();

    mFragmentShadingRateAtachment.reset();

    mRenderPassStarted                     = false;
    mValidTransformFeedbackBufferCount     = 0;
    mRebindTransformFeedbackBuffers        = false;
    mHasShaderStorageOutput                = false;
    mHasGLMemoryBarrierIssued              = false;
    mPreviousSubpassesCmdCount             = 0;
    mColorAttachmentsCount                 = PackedAttachmentCount(0);
    mDepthStencilAttachmentIndex           = kAttachmentIndexInvalid;
    mImageOptimizeForPresent               = nullptr;
    mImageOptimizeForPresentOriginalLayout = ImageLayout::Undefined;

    ASSERT(CheckSubpassCommandBufferCount(getSubpassCommandBufferCount()));

    // Collect/Reset the command buffers
    for (uint32_t subpass = 0; subpass < getSubpassCommandBufferCount(); ++subpass)
    {
        commandBufferCollector->collectCommandBuffer(std::move(mCommandBuffers[subpass]));
    }

    mCurrentSubpassCommandBufferIndex = 0;

    // Reset the image views used for imageless framebuffer (if any)
    mFramebuffer.reset();

    // Invalidate the queue serial here. We will get a new queue serial when we begin renderpass.
    mQueueSerial = QueueSerial();

    return initializeCommandBuffer(context);
}

void RenderPassCommandBufferHelper::imageRead(ContextVk *contextVk,
                                              VkImageAspectFlags aspectFlags,
                                              ImageLayout imageLayout,
                                              ImageHelper *image)
{
    imageReadImpl(contextVk, aspectFlags, imageLayout, BarrierType::Event, image);
    // As noted in the header we don't support multiple read layouts for Images.
    // We allow duplicate uses in the RP to accommodate for normal GL sampler usage.
    retainImageWithEvent(contextVk, image);
}

void RenderPassCommandBufferHelper::imageWrite(ContextVk *contextVk,
                                               gl::LevelIndex level,
                                               uint32_t layerStart,
                                               uint32_t layerCount,
                                               VkImageAspectFlags aspectFlags,
                                               ImageLayout imageLayout,
                                               ImageHelper *image)
{
    imageWriteImpl(contextVk, level, layerStart, layerCount, aspectFlags, imageLayout,
                   BarrierType::Event, image);
    retainImageWithEvent(contextVk, image);
}

void RenderPassCommandBufferHelper::colorImagesDraw(gl::LevelIndex level,
                                                    uint32_t layerStart,
                                                    uint32_t layerCount,
                                                    ImageHelper *image,
                                                    ImageHelper *resolveImage,
                                                    UniqueSerial imageSiblingSerial,
                                                    PackedAttachmentIndex packedAttachmentIndex)
{
    ASSERT(packedAttachmentIndex < mColorAttachmentsCount);

    image->onRenderPassAttach(mQueueSerial);

    mColorAttachments[packedAttachmentIndex].init(image, imageSiblingSerial, level, layerStart,
                                                  layerCount, VK_IMAGE_ASPECT_COLOR_BIT);

    if (resolveImage)
    {
        resolveImage->onRenderPassAttach(mQueueSerial);
        mColorResolveAttachments[packedAttachmentIndex].init(resolveImage, imageSiblingSerial,
                                                             level, layerStart, layerCount,
                                                             VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void RenderPassCommandBufferHelper::depthStencilImagesDraw(gl::LevelIndex level,
                                                           uint32_t layerStart,
                                                           uint32_t layerCount,
                                                           ImageHelper *image,
                                                           ImageHelper *resolveImage,
                                                           UniqueSerial imageSiblingSerial)
{
    ASSERT(!usesImage(*image));
    ASSERT(!resolveImage || !usesImage(*resolveImage));

    // Because depthStencil buffer's read/write property can change while we build renderpass, we
    // defer the image layout changes until endRenderPass time or when images going away so that we
    // only insert layout change barrier once.
    image->onRenderPassAttach(mQueueSerial);

    mDepthAttachment.init(image, imageSiblingSerial, level, layerStart, layerCount,
                          VK_IMAGE_ASPECT_DEPTH_BIT);
    mStencilAttachment.init(image, imageSiblingSerial, level, layerStart, layerCount,
                            VK_IMAGE_ASPECT_STENCIL_BIT);

    if (resolveImage)
    {
        // Note that the resolve depth/stencil image has the same level/layer index as the
        // depth/stencil image as currently it can only ever come from
        // multisampled-render-to-texture renderbuffers.
        resolveImage->onRenderPassAttach(mQueueSerial);

        mDepthResolveAttachment.init(resolveImage, imageSiblingSerial, level, layerStart,
                                     layerCount, VK_IMAGE_ASPECT_DEPTH_BIT);
        mStencilResolveAttachment.init(resolveImage, imageSiblingSerial, level, layerStart,
                                       layerCount, VK_IMAGE_ASPECT_STENCIL_BIT);
    }
}

void RenderPassCommandBufferHelper::fragmentShadingRateImageRead(ImageHelper *image)
{
    ASSERT(image && image->valid());
    ASSERT(!usesImage(*image));

    image->onRenderPassAttach(mQueueSerial);

    // Initialize RenderPassAttachment for fragment shading rate attachment.
    mFragmentShadingRateAtachment.init(image, {}, gl::LevelIndex(0), 0, 1,
                                       VK_IMAGE_ASPECT_COLOR_BIT);

    image->resetRenderPassUsageFlags();
    image->setRenderPassUsageFlag(RenderPassUsage::FragmentShadingRateReadOnlyAttachment);
}

void RenderPassCommandBufferHelper::onColorAccess(PackedAttachmentIndex packedAttachmentIndex,
                                                  ResourceAccess access)
{
    ASSERT(packedAttachmentIndex < mColorAttachmentsCount);
    mColorAttachments[packedAttachmentIndex].onAccess(access, getRenderPassWriteCommandCount());
}

void RenderPassCommandBufferHelper::onDepthAccess(ResourceAccess access)
{
    mDepthAttachment.onAccess(access, getRenderPassWriteCommandCount());
}

void RenderPassCommandBufferHelper::onStencilAccess(ResourceAccess access)
{
    mStencilAttachment.onAccess(access, getRenderPassWriteCommandCount());
}

void RenderPassCommandBufferHelper::updateDepthReadOnlyMode(RenderPassUsageFlags dsUsageFlags)
{
    ASSERT(mRenderPassStarted);
    updateStartedRenderPassWithDepthStencilMode(&mDepthResolveAttachment, hasDepthWriteOrClear(),
                                                dsUsageFlags,
                                                RenderPassUsage::DepthReadOnlyAttachment);
}

void RenderPassCommandBufferHelper::updateStencilReadOnlyMode(RenderPassUsageFlags dsUsageFlags)
{
    ASSERT(mRenderPassStarted);
    updateStartedRenderPassWithDepthStencilMode(&mStencilResolveAttachment,
                                                hasStencilWriteOrClear(), dsUsageFlags,
                                                RenderPassUsage::StencilReadOnlyAttachment);
}

void RenderPassCommandBufferHelper::updateDepthStencilReadOnlyMode(
    RenderPassUsageFlags dsUsageFlags,
    VkImageAspectFlags dsAspectFlags)
{
    ASSERT(mRenderPassStarted);
    if ((dsAspectFlags & VK_IMAGE_ASPECT_DEPTH_BIT) != 0)
    {
        updateDepthReadOnlyMode(dsUsageFlags);
    }
    if ((dsAspectFlags & VK_IMAGE_ASPECT_STENCIL_BIT) != 0)
    {
        updateStencilReadOnlyMode(dsUsageFlags);
    }
}

void RenderPassCommandBufferHelper::updateStartedRenderPassWithDepthStencilMode(
    RenderPassAttachment *resolveAttachment,
    bool renderPassHasWriteOrClear,
    RenderPassUsageFlags dsUsageFlags,
    RenderPassUsage readOnlyAttachmentUsage)
{
    ASSERT(mRenderPassStarted);
    ASSERT(mDepthAttachment.getImage() == mStencilAttachment.getImage());
    ASSERT(mDepthResolveAttachment.getImage() == mStencilResolveAttachment.getImage());

    // Determine read-only mode for depth or stencil
    const bool readOnlyMode =
        mDepthStencilAttachmentIndex != kAttachmentIndexInvalid &&
        resolveAttachment->getImage() == nullptr &&
        (dsUsageFlags.test(readOnlyAttachmentUsage) || !renderPassHasWriteOrClear);

    // If readOnlyMode is false, we are switching out of read only mode due to depth/stencil write.
    // We must not be in the read only feedback loop mode because the logic in
    // DIRTY_BIT_READ_ONLY_DEPTH_FEEDBACK_LOOP_MODE should ensure we end the previous renderpass and
    // a new renderpass will start with feedback loop disabled.
    ASSERT(readOnlyMode || !dsUsageFlags.test(readOnlyAttachmentUsage));

    ImageHelper *depthStencilImage = mDepthAttachment.getImage();
    if (depthStencilImage)
    {
        if (readOnlyMode)
        {
            depthStencilImage->setRenderPassUsageFlag(readOnlyAttachmentUsage);
        }
        else
        {
            depthStencilImage->clearRenderPassUsageFlag(readOnlyAttachmentUsage);
        }
    }
    // The depth/stencil resolve image is never in read-only mode
}

void RenderPassCommandBufferHelper::finalizeColorImageLayout(
    Context *context,
    ImageHelper *image,
    PackedAttachmentIndex packedAttachmentIndex,
    bool isResolveImage)
{
    ASSERT(packedAttachmentIndex < mColorAttachmentsCount);
    ASSERT(image != nullptr);

    // Do layout change.
    ImageLayout imageLayout;
    if (image->usedByCurrentRenderPassAsAttachmentAndSampler(RenderPassUsage::ColorTextureSampler))
    {
        // texture code already picked layout and inserted barrier
        imageLayout = image->getCurrentImageLayout();
        ASSERT(imageLayout == ImageLayout::ColorWriteFragmentShaderFeedback ||
               imageLayout == ImageLayout::ColorWriteAllShadersFeedback);
    }
    else
    {
        // When color is unresolved, use a layout that includes fragment shader reads.  This is done
        // for all color resolve attachments even if they are not all unresolved for simplicity.  In
        // particular, the GL color index is not available (only the packed index) at this point,
        // but that is needed to query whether the attachment is unresolved or not.
        const bool hasUnresolve =
            isResolveImage && mRenderPassDesc.getColorUnresolveAttachmentMask().any();
        imageLayout = hasUnresolve ? ImageLayout::MSRTTEmulationColorUnresolveAndResolve
                                   : ImageLayout::ColorWrite;
        if (context->getFeatures().preferDynamicRendering.enabled &&
            mRenderPassDesc.hasColorFramebufferFetch())
        {
            // Note MSRTT emulation is not implemented with dynamic rendering.
            ASSERT(imageLayout == ImageLayout::ColorWrite);
            imageLayout = ImageLayout::ColorWriteAndInput;
        }
        else if (image->getCurrentImageLayout() == ImageLayout::SharedPresent)
        {
            // Once you transition to ImageLayout::SharedPresent, you never transition out of it.
            ASSERT(imageLayout == ImageLayout::ColorWrite);
            imageLayout = ImageLayout::SharedPresent;
        }

        updateImageLayoutAndBarrier(context, image, VK_IMAGE_ASPECT_COLOR_BIT, imageLayout,
                                    BarrierType::Event);
    }

    if (!isResolveImage)
    {
        mAttachmentOps.setLayouts(packedAttachmentIndex, imageLayout, imageLayout);
    }

    // Dynamic rendering does not have implicit layout transitions at render pass boundaries.  This
    // optimization is instead done by recording the necessary transition after the render pass
    // directly on the primary command buffer.
    if (mImageOptimizeForPresent == image)
    {
        ASSERT(isDefault());
        ASSERT(context->getFeatures().supportsPresentation.enabled);
        ASSERT(packedAttachmentIndex == kAttachmentIndexZero);

        // Use finalLayout instead of extra barrier for layout change to present.  For dynamic
        // rendering, this is not possible and is done when the render pass is flushed.  However,
        // because this function is expected to finalize the image layout, we still have to pretend
        // the image is in the present layout already.
        mImageOptimizeForPresentOriginalLayout = mImageOptimizeForPresent->getCurrentImageLayout();
        mImageOptimizeForPresent->setCurrentImageLayout(context->getRenderer(),
                                                        ImageLayout::Present);

        if (!context->getFeatures().preferDynamicRendering.enabled)
        {
            if (isResolveImage)
            {
                SetBitField(mAttachmentOps[packedAttachmentIndex].finalResolveLayout,
                            mImageOptimizeForPresent->getCurrentImageLayout());
            }
            else
            {
                SetBitField(mAttachmentOps[packedAttachmentIndex].finalLayout,
                            mImageOptimizeForPresent->getCurrentImageLayout());
            }
            mImageOptimizeForPresent               = nullptr;
            mImageOptimizeForPresentOriginalLayout = ImageLayout::Undefined;
        }
    }

    if (isResolveImage)
    {
        // Note: the color image will have its flags reset after load/store ops are determined.
        image->resetRenderPassUsageFlags();
    }
}

void RenderPassCommandBufferHelper::finalizeColorImageLoadStore(
    Context *context,
    PackedAttachmentIndex packedAttachmentIndex)
{
    PackedAttachmentOpsDesc &ops = mAttachmentOps[packedAttachmentIndex];
    RenderPassLoadOp loadOp      = static_cast<RenderPassLoadOp>(ops.loadOp);
    RenderPassStoreOp storeOp    = static_cast<RenderPassStoreOp>(ops.storeOp);

    // This has to be called after layout been finalized
    ASSERT(ops.initialLayout != static_cast<uint16_t>(ImageLayout::Undefined));

    uint32_t currentCmdCount = getRenderPassWriteCommandCount();
    bool isInvalidated       = false;

    RenderPassAttachment &colorAttachment = mColorAttachments[packedAttachmentIndex];
    colorAttachment.finalizeLoadStore(
        context, currentCmdCount, mRenderPassDesc.getColorUnresolveAttachmentMask().any(),
        mRenderPassDesc.getColorResolveAttachmentMask().any(), &loadOp, &storeOp, &isInvalidated);

    if (isInvalidated)
    {
        ops.isInvalidated = true;
    }

    if (!ops.isInvalidated)
    {
        mColorResolveAttachments[packedAttachmentIndex].restoreContent();
    }

    // If the image is being written to, mark its contents defined.
    // This has to be done after storeOp has been finalized.
    if (storeOp == RenderPassStoreOp::Store)
    {
        colorAttachment.restoreContent();
    }

    SetBitField(ops.loadOp, loadOp);
    SetBitField(ops.storeOp, storeOp);
}

void RenderPassCommandBufferHelper::finalizeDepthStencilImageLayout(Context *context)
{
    ASSERT(mDepthAttachment.getImage() != nullptr);
    ASSERT(mDepthAttachment.getImage() == mStencilAttachment.getImage());

    ImageHelper *depthStencilImage = mDepthAttachment.getImage();

    // Do depth stencil layout change.
    ImageLayout imageLayout;
    bool barrierRequired;

    const bool isDepthAttachmentAndSampler =
        depthStencilImage->usedByCurrentRenderPassAsAttachmentAndSampler(
            RenderPassUsage::DepthTextureSampler);
    const bool isStencilAttachmentAndSampler =
        depthStencilImage->usedByCurrentRenderPassAsAttachmentAndSampler(
            RenderPassUsage::StencilTextureSampler);
    const bool isReadOnlyDepth =
        depthStencilImage->hasRenderPassUsageFlag(RenderPassUsage::DepthReadOnlyAttachment);
    const bool isReadOnlyStencil =
        depthStencilImage->hasRenderPassUsageFlag(RenderPassUsage::StencilReadOnlyAttachment);
    BarrierType barrierType = BarrierType::Event;

    if (isDepthAttachmentAndSampler || isStencilAttachmentAndSampler)
    {
        // texture code already picked layout and inserted barrier
        imageLayout = depthStencilImage->getCurrentImageLayout();

        if ((isDepthAttachmentAndSampler && !isReadOnlyDepth) ||
            (isStencilAttachmentAndSampler && !isReadOnlyStencil))
        {
            ASSERT(imageLayout == ImageLayout::DepthStencilFragmentShaderFeedback ||
                   imageLayout == ImageLayout::DepthStencilAllShadersFeedback);
            barrierRequired = true;
        }
        else
        {
            ASSERT(imageLayout == ImageLayout::DepthWriteStencilReadFragmentShaderStencilRead ||
                   imageLayout == ImageLayout::DepthWriteStencilReadAllShadersStencilRead ||
                   imageLayout == ImageLayout::DepthReadStencilWriteFragmentShaderDepthRead ||
                   imageLayout == ImageLayout::DepthReadStencilWriteAllShadersDepthRead ||
                   imageLayout == ImageLayout::DepthReadStencilReadFragmentShaderRead ||
                   imageLayout == ImageLayout::DepthReadStencilReadAllShadersRead);
            barrierRequired =
                depthStencilImage->isReadBarrierNecessary(context->getRenderer(), imageLayout);
        }
    }
    else
    {
        if (mRenderPassDesc.hasDepthStencilFramebufferFetch())
        {
            imageLayout = ImageLayout::DepthStencilWriteAndInput;
        }
        else if (isReadOnlyDepth)
        {
            imageLayout = isReadOnlyStencil ? ImageLayout::DepthReadStencilRead
                                            : ImageLayout::DepthReadStencilWrite;
        }
        else
        {
            imageLayout = isReadOnlyStencil ? ImageLayout::DepthWriteStencilRead
                                            : ImageLayout::DepthWriteStencilWrite;
        }

        barrierRequired =
            !isReadOnlyDepth || !isReadOnlyStencil ||
            depthStencilImage->isReadBarrierNecessary(context->getRenderer(), imageLayout);
    }

    mAttachmentOps.setLayouts(mDepthStencilAttachmentIndex, imageLayout, imageLayout);

    if (barrierRequired)
    {
        const angle::Format &format = depthStencilImage->getActualFormat();
        ASSERT(format.hasDepthOrStencilBits());
        VkImageAspectFlags aspectFlags = GetDepthStencilAspectFlags(format);
        updateImageLayoutAndBarrier(context, depthStencilImage, aspectFlags, imageLayout,
                                    barrierType);
    }
}

void RenderPassCommandBufferHelper::finalizeDepthStencilResolveImageLayout(Context *context)
{
    ASSERT(mDepthResolveAttachment.getImage() != nullptr);
    ASSERT(mDepthResolveAttachment.getImage() == mStencilResolveAttachment.getImage());

    ImageHelper *depthStencilResolveImage = mDepthResolveAttachment.getImage();

    // When depth/stencil is unresolved, use a layout that includes fragment shader reads.
    ImageLayout imageLayout     = mRenderPassDesc.hasDepthStencilUnresolveAttachment()
                                      ? ImageLayout::MSRTTEmulationDepthStencilUnresolveAndResolve
                                      : ImageLayout::DepthStencilResolve;
    const angle::Format &format = depthStencilResolveImage->getActualFormat();
    ASSERT(format.hasDepthOrStencilBits());
    VkImageAspectFlags aspectFlags = GetDepthStencilAspectFlags(format);

    updateImageLayoutAndBarrier(context, depthStencilResolveImage, aspectFlags, imageLayout,
                                BarrierType::Event);

    // The resolve image can never be read-only.
    ASSERT(!depthStencilResolveImage->hasRenderPassUsageFlag(
        RenderPassUsage::DepthReadOnlyAttachment));
    ASSERT(!depthStencilResolveImage->hasRenderPassUsageFlag(
        RenderPassUsage::StencilReadOnlyAttachment));
    ASSERT(mDepthStencilAttachmentIndex != kAttachmentIndexInvalid);
    const PackedAttachmentOpsDesc &dsOps = mAttachmentOps[mDepthStencilAttachmentIndex];

    // If the image is being written to, mark its contents defined.
    if (!dsOps.isInvalidated && mRenderPassDesc.hasDepthResolveAttachment())
    {
        mDepthResolveAttachment.restoreContent();
    }
    if (!dsOps.isStencilInvalidated && mRenderPassDesc.hasStencilResolveAttachment())
    {
        mStencilResolveAttachment.restoreContent();
    }

    depthStencilResolveImage->resetRenderPassUsageFlags();
}

void RenderPassCommandBufferHelper::finalizeFragmentShadingRateImageLayout(Context *context)
{
    ImageHelper *image      = mFragmentShadingRateAtachment.getImage();
    ImageLayout imageLayout = ImageLayout::FragmentShadingRateAttachmentReadOnly;
    ASSERT(image && image->valid());
    if (image->isReadBarrierNecessary(context->getRenderer(), imageLayout))
    {
        updateImageLayoutAndBarrier(context, image, VK_IMAGE_ASPECT_COLOR_BIT, imageLayout,
                                    BarrierType::Event);
    }
    image->resetRenderPassUsageFlags();
}

void RenderPassCommandBufferHelper::finalizeImageLayout(Context *context,
                                                        const ImageHelper *image,
                                                        UniqueSerial imageSiblingSerial)
{
    if (image->hasRenderPassUsageFlag(RenderPassUsage::RenderTargetAttachment))
    {
        for (PackedAttachmentIndex index = kAttachmentIndexZero; index < mColorAttachmentsCount;
             ++index)
        {
            if (mColorAttachments[index].hasImage(image, imageSiblingSerial))
            {
                finalizeColorImageLayoutAndLoadStore(context, index);
                mColorAttachments[index].reset();
            }
            else if (mColorResolveAttachments[index].hasImage(image, imageSiblingSerial))
            {
                finalizeColorImageLayout(context, mColorResolveAttachments[index].getImage(), index,
                                         true);
                mColorResolveAttachments[index].reset();
            }
        }
    }

    if (mDepthAttachment.hasImage(image, imageSiblingSerial))
    {
        finalizeDepthStencilImageLayoutAndLoadStore(context);
        mDepthAttachment.reset();
        mStencilAttachment.reset();
    }

    if (mDepthResolveAttachment.hasImage(image, imageSiblingSerial))
    {
        finalizeDepthStencilResolveImageLayout(context);
        mDepthResolveAttachment.reset();
        mStencilResolveAttachment.reset();
    }

    if (mFragmentShadingRateAtachment.hasImage(image, imageSiblingSerial))
    {
        finalizeFragmentShadingRateImageLayout(context);
        mFragmentShadingRateAtachment.reset();
    }
}

void RenderPassCommandBufferHelper::finalizeDepthStencilLoadStore(Context *context)
{
    ASSERT(mDepthStencilAttachmentIndex != kAttachmentIndexInvalid);

    PackedAttachmentOpsDesc &dsOps   = mAttachmentOps[mDepthStencilAttachmentIndex];
    RenderPassLoadOp depthLoadOp     = static_cast<RenderPassLoadOp>(dsOps.loadOp);
    RenderPassStoreOp depthStoreOp   = static_cast<RenderPassStoreOp>(dsOps.storeOp);
    RenderPassLoadOp stencilLoadOp   = static_cast<RenderPassLoadOp>(dsOps.stencilLoadOp);
    RenderPassStoreOp stencilStoreOp = static_cast<RenderPassStoreOp>(dsOps.stencilStoreOp);

    // This has to be called after layout been finalized
    ASSERT(dsOps.initialLayout != static_cast<uint16_t>(ImageLayout::Undefined));

    uint32_t currentCmdCount         = getRenderPassWriteCommandCount();
    bool isDepthInvalidated          = false;
    bool isStencilInvalidated        = false;
    bool hasDepthResolveAttachment   = mRenderPassDesc.hasDepthResolveAttachment();
    bool hasStencilResolveAttachment = mRenderPassDesc.hasStencilResolveAttachment();

    mDepthAttachment.finalizeLoadStore(
        context, currentCmdCount, mRenderPassDesc.hasDepthUnresolveAttachment(),
        hasDepthResolveAttachment, &depthLoadOp, &depthStoreOp, &isDepthInvalidated);
    mStencilAttachment.finalizeLoadStore(
        context, currentCmdCount, mRenderPassDesc.hasStencilUnresolveAttachment(),
        hasStencilResolveAttachment, &stencilLoadOp, &stencilStoreOp, &isStencilInvalidated);

    const bool disableMixedDepthStencilLoadOpNoneAndLoad =
        context->getFeatures().disallowMixedDepthStencilLoadOpNoneAndLoad.enabled;

    if (disableMixedDepthStencilLoadOpNoneAndLoad)
    {
        if (depthLoadOp == RenderPassLoadOp::None && stencilLoadOp != RenderPassLoadOp::None)
        {
            depthLoadOp = RenderPassLoadOp::Load;
        }
        if (depthLoadOp != RenderPassLoadOp::None && stencilLoadOp == RenderPassLoadOp::None)
        {
            stencilLoadOp = RenderPassLoadOp::Load;
        }
    }

    if (isDepthInvalidated)
    {
        dsOps.isInvalidated = true;
    }
    if (isStencilInvalidated)
    {
        dsOps.isStencilInvalidated = true;
    }

    // If any aspect is missing, set the corresponding ops to don't care.
    const uint32_t depthStencilIndexGL =
        static_cast<uint32_t>(mRenderPassDesc.depthStencilAttachmentIndex());
    const angle::FormatID attachmentFormatID = mRenderPassDesc[depthStencilIndexGL];
    ASSERT(attachmentFormatID != angle::FormatID::NONE);
    const angle::Format &angleFormat = angle::Format::Get(attachmentFormatID);

    if (angleFormat.depthBits == 0)
    {
        depthLoadOp  = RenderPassLoadOp::DontCare;
        depthStoreOp = RenderPassStoreOp::DontCare;
    }
    if (angleFormat.stencilBits == 0)
    {
        stencilLoadOp  = RenderPassLoadOp::DontCare;
        stencilStoreOp = RenderPassStoreOp::DontCare;
    }

    // If the image is being written to, mark its contents defined.
    // This has to be done after storeOp has been finalized.
    ASSERT(mDepthAttachment.getImage() == mStencilAttachment.getImage());
    if (!mDepthAttachment.getImage()->hasRenderPassUsageFlag(
            RenderPassUsage::DepthReadOnlyAttachment))
    {
        if (depthStoreOp == RenderPassStoreOp::Store)
        {
            mDepthAttachment.restoreContent();
        }
    }
    if (!mStencilAttachment.getImage()->hasRenderPassUsageFlag(
            RenderPassUsage::StencilReadOnlyAttachment))
    {
        if (stencilStoreOp == RenderPassStoreOp::Store)
        {
            mStencilAttachment.restoreContent();
        }
    }

    SetBitField(dsOps.loadOp, depthLoadOp);
    SetBitField(dsOps.storeOp, depthStoreOp);
    SetBitField(dsOps.stencilLoadOp, stencilLoadOp);
    SetBitField(dsOps.stencilStoreOp, stencilStoreOp);
}

void RenderPassCommandBufferHelper::finalizeColorImageLayoutAndLoadStore(
    Context *context,
    PackedAttachmentIndex packedAttachmentIndex)
{
    finalizeColorImageLayout(context, mColorAttachments[packedAttachmentIndex].getImage(),
                             packedAttachmentIndex, false);
    finalizeColorImageLoadStore(context, packedAttachmentIndex);

    mColorAttachments[packedAttachmentIndex].getImage()->resetRenderPassUsageFlags();
}

void RenderPassCommandBufferHelper::finalizeDepthStencilImageLayoutAndLoadStore(Context *context)
{
    finalizeDepthStencilImageLayout(context);
    finalizeDepthStencilLoadStore(context);

    ASSERT(mDepthAttachment.getImage() == mStencilAttachment.getImage());
    mDepthAttachment.getImage()->resetRenderPassUsageFlags();
}

void RenderPassCommandBufferHelper::collectRefCountedEventsGarbage(
    Renderer *renderer,
    RefCountedEventsGarbageRecycler *garbageRecycler)
{
    // For render pass the VkCmdSetEvent works differently from OutsideRenderPassCommands.
    // VkCmdEndRenderPass are called in the primary command buffer, and VkCmdSetEvents has to be
    // issued after VkCmdEndRenderPass. This means VkCmdSetEvent has to be delayed. Because of this,
    // here we simply make a copy of the VkEvent from RefCountedEvent and then add the
    // RefCountedEvent to the garbage collector. No VkCmdSetEvent call is issued here (they will be
    // issued at flushToPrimary time).
    mVkEventArray.init(renderer, mRefCountedEvents);
    mRefCountedEvents.releaseToEventCollector(&mRefCountedEventCollector);

    if (!mRefCountedEventCollector.empty())
    {
        garbageRecycler->collectGarbage(mQueueSerial, std::move(mRefCountedEventCollector));
    }
}

void RenderPassCommandBufferHelper::updatePerfCountersForDynamicRenderingInstance(
    ErrorContext *context,
    angle::VulkanPerfCounters *countersOut)
{
    mRenderPassDesc.updatePerfCounters(context, mFramebuffer.getUnpackedImageViews(),
                                       mAttachmentOps, countersOut);
}

angle::Result RenderPassCommandBufferHelper::beginRenderPass(
    ContextVk *contextVk,
    RenderPassFramebuffer &&framebuffer,
    const gl::Rectangle &renderArea,
    const RenderPassDesc &renderPassDesc,
    const AttachmentOpsArray &renderPassAttachmentOps,
    const PackedAttachmentCount colorAttachmentCount,
    const PackedAttachmentIndex depthStencilAttachmentIndex,
    const PackedClearValuesArray &clearValues,
    const QueueSerial &queueSerial,
    RenderPassCommandBuffer **commandBufferOut)
{
    ASSERT(!mRenderPassStarted);

    mRenderPassDesc              = renderPassDesc;
    mAttachmentOps               = renderPassAttachmentOps;
    mDepthStencilAttachmentIndex = depthStencilAttachmentIndex;
    mColorAttachmentsCount       = colorAttachmentCount;
    mFramebuffer                 = std::move(framebuffer);
    mRenderArea                  = renderArea;
    mClearValues                 = clearValues;
    mQueueSerial                 = queueSerial;
    *commandBufferOut            = &getCommandBuffer();

    mRenderPassStarted = true;
    mCounter++;

    return beginRenderPassCommandBuffer(contextVk);
}

angle::Result RenderPassCommandBufferHelper::beginRenderPassCommandBuffer(ContextVk *contextVk)
{
    VkCommandBufferInheritanceInfo inheritanceInfo;
    VkCommandBufferInheritanceRenderingInfo renderingInfo;
    gl::DrawBuffersArray<VkFormat> colorFormatStorage;

    ANGLE_TRY(RenderPassCommandBuffer::InitializeRenderPassInheritanceInfo(
        contextVk, mFramebuffer.getFramebuffer(), mRenderPassDesc, &inheritanceInfo, &renderingInfo,
        &colorFormatStorage));
    inheritanceInfo.subpass = mCurrentSubpassCommandBufferIndex;

    return getCommandBuffer().begin(contextVk, inheritanceInfo);
}

angle::Result RenderPassCommandBufferHelper::endRenderPass(ContextVk *contextVk)
{
    ANGLE_TRY(endRenderPassCommandBuffer(contextVk));

    for (PackedAttachmentIndex index = kAttachmentIndexZero; index < mColorAttachmentsCount;
         ++index)
    {
        if (mColorAttachments[index].getImage() != nullptr)
        {
            finalizeColorImageLayoutAndLoadStore(contextVk, index);
        }
        if (mColorResolveAttachments[index].getImage() != nullptr)
        {
            finalizeColorImageLayout(contextVk, mColorResolveAttachments[index].getImage(), index,
                                     true);
        }
    }

    if (mFragmentShadingRateAtachment.getImage() != nullptr)
    {
        finalizeFragmentShadingRateImageLayout(contextVk);
    }

    if (mDepthStencilAttachmentIndex != kAttachmentIndexInvalid)
    {
        // Do depth stencil layout change and load store optimization.
        ASSERT(mDepthAttachment.getImage() == mStencilAttachment.getImage());
        ASSERT(mDepthResolveAttachment.getImage() == mStencilResolveAttachment.getImage());
        if (mDepthAttachment.getImage() != nullptr)
        {
            finalizeDepthStencilImageLayoutAndLoadStore(contextVk);
        }
        if (mDepthResolveAttachment.getImage() != nullptr)
        {
            finalizeDepthStencilResolveImageLayout(contextVk);
        }
    }

    return angle::Result::Continue;
}

angle::Result RenderPassCommandBufferHelper::endRenderPassCommandBuffer(ContextVk *contextVk)
{
    return getCommandBuffer().end(contextVk);
}

angle::Result RenderPassCommandBufferHelper::nextSubpass(ContextVk *contextVk,
                                                         RenderPassCommandBuffer **commandBufferOut)
{
    ASSERT(!contextVk->getFeatures().preferDynamicRendering.enabled);

    if (ExecutesInline())
    {
        // When using ANGLE secondary command buffers, the commands are inline and are executed on
        // the primary command buffer.  This means that vkCmdNextSubpass can be intermixed with the
        // rest of the commands, and there is no need to split command buffers.
        //
        // Note also that the command buffer handle doesn't change in this case.
        getCommandBuffer().nextSubpass(VK_SUBPASS_CONTENTS_INLINE);
        return angle::Result::Continue;
    }

    // When using Vulkan secondary command buffers, each subpass's contents must be recorded in a
    // separate command buffer that is vkCmdExecuteCommands'ed in the primary command buffer.
    // vkCmdNextSubpass calls must also be issued in the primary command buffer.
    //
    // To support this, a list of command buffers are kept, one for each subpass.  When moving to
    // the next subpass, the previous command buffer is ended and a new one is initialized and
    // begun.

    // Accumulate command count for tracking purposes.
    mPreviousSubpassesCmdCount = getRenderPassWriteCommandCount();

    ANGLE_TRY(endRenderPassCommandBuffer(contextVk));
    markClosed();

    ++mCurrentSubpassCommandBufferIndex;
    ASSERT(getSubpassCommandBufferCount() <= kMaxSubpassCount);

    ANGLE_TRY(initializeCommandBuffer(contextVk));
    ANGLE_TRY(beginRenderPassCommandBuffer(contextVk));
    markOpen();

    // Return the new command buffer handle
    *commandBufferOut = &getCommandBuffer();
    return angle::Result::Continue;
}

void RenderPassCommandBufferHelper::beginTransformFeedback(size_t validBufferCount,
                                                           const VkBuffer *counterBuffers,
                                                           const VkDeviceSize *counterBufferOffsets,
                                                           bool rebindBuffers)
{
    mValidTransformFeedbackBufferCount = static_cast<uint32_t>(validBufferCount);
    mRebindTransformFeedbackBuffers    = rebindBuffers;

    for (size_t index = 0; index < validBufferCount; index++)
    {
        mTransformFeedbackCounterBuffers[index]       = counterBuffers[index];
        mTransformFeedbackCounterBufferOffsets[index] = counterBufferOffsets[index];
    }
}

void RenderPassCommandBufferHelper::endTransformFeedback()
{
    pauseTransformFeedback();
    mValidTransformFeedbackBufferCount = 0;
}

void RenderPassCommandBufferHelper::invalidateRenderPassColorAttachment(
    const gl::State &state,
    size_t colorIndexGL,
    PackedAttachmentIndex attachmentIndex,
    const gl::Rectangle &invalidateArea)
{
    // Color write is enabled if:
    //
    // - Draw buffer is enabled (this is implicit, as invalidate only affects enabled draw buffers)
    // - Color output is not entirely masked
    // - Rasterizer-discard is not enabled
    const gl::BlendStateExt &blendStateExt = state.getBlendStateExt();
    const bool isColorWriteEnabled =
        blendStateExt.getColorMaskIndexed(colorIndexGL) != 0 && !state.isRasterizerDiscardEnabled();
    mColorAttachments[attachmentIndex].invalidate(invalidateArea, isColorWriteEnabled,
                                                  getRenderPassWriteCommandCount());
}

void RenderPassCommandBufferHelper::invalidateRenderPassDepthAttachment(
    const gl::DepthStencilState &dsState,
    const gl::Rectangle &invalidateArea)
{
    const bool isDepthWriteEnabled = dsState.depthTest && dsState.depthMask;
    mDepthAttachment.invalidate(invalidateArea, isDepthWriteEnabled,
                                getRenderPassWriteCommandCount());
}

void RenderPassCommandBufferHelper::invalidateRenderPassStencilAttachment(
    const gl::DepthStencilState &dsState,
    GLuint framebufferStencilSize,
    const gl::Rectangle &invalidateArea)
{
    const bool isStencilWriteEnabled =
        dsState.stencilTest && (!dsState.isStencilNoOp(framebufferStencilSize) ||
                                !dsState.isStencilBackNoOp(framebufferStencilSize));
    mStencilAttachment.invalidate(invalidateArea, isStencilWriteEnabled,
                                  getRenderPassWriteCommandCount());
}

angle::Result RenderPassCommandBufferHelper::flushToPrimary(Context *context,
                                                            CommandsState *commandsState,
                                                            const RenderPass &renderPass,
                                                            VkFramebuffer framebufferOverride)
{
    Renderer *renderer = context->getRenderer();
    // |framebufferOverride| must only be provided if the initial framebuffer the render pass was
    // started with is not usable (due to the addition of resolve attachments after the fact).
    ASSERT(framebufferOverride == VK_NULL_HANDLE ||
           mFramebuffer.needsNewFramebufferWithResolveAttachments());
    // When a new framebuffer had to be created because of addition of resolve attachments, it's
    // never imageless.
    ASSERT(!(framebufferOverride != VK_NULL_HANDLE && mFramebuffer.isImageless()));

    ANGLE_TRACE_EVENT0("gpu.angle", "RenderPassCommandBufferHelper::flushToPrimary");
    ASSERT(mRenderPassStarted);
    PrimaryCommandBuffer &primary = commandsState->primaryCommands;

    // Commands that are added to primary before beginRenderPass command
    executeBarriers(renderer, commandsState);

    constexpr VkSubpassContents kSubpassContents =
        ExecutesInline() ? VK_SUBPASS_CONTENTS_INLINE
                         : VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS;

    if (!renderPass.valid())
    {
        mRenderPassDesc.beginRendering(context, &primary, mRenderArea, kSubpassContents,
                                       mFramebuffer.getUnpackedImageViews(), mAttachmentOps,
                                       mClearValues, mFramebuffer.getLayers());
    }
    else
    {
        // With imageless framebuffers, the attachments should be also added to beginInfo.
        VkRenderPassAttachmentBeginInfo attachmentBeginInfo = {};
        if (mFramebuffer.isImageless())
        {
            mFramebuffer.packResolveViewsForRenderPassBegin(&attachmentBeginInfo);

            // If nullColorAttachmentWithExternalFormatResolve is true, there will be no color
            // attachment even though mRenderPassDesc indicates so.
            ASSERT((mRenderPassDesc.hasYUVResolveAttachment() &&
                    renderer->nullColorAttachmentWithExternalFormatResolve()) ||
                   attachmentBeginInfo.attachmentCount == mRenderPassDesc.attachmentCount());
        }

        mRenderPassDesc.beginRenderPass(
            context, &primary, renderPass,
            framebufferOverride ? framebufferOverride : mFramebuffer.getFramebuffer().getHandle(),
            mRenderArea, kSubpassContents, mClearValues,
            mFramebuffer.isImageless() ? &attachmentBeginInfo : nullptr);
    }

    // Run commands inside the RenderPass.
    for (uint32_t subpass = 0; subpass < getSubpassCommandBufferCount(); ++subpass)
    {
        if (subpass > 0)
        {
            ASSERT(!context->getFeatures().preferDynamicRendering.enabled);
            primary.nextSubpass(kSubpassContents);
        }
        mCommandBuffers[subpass].executeCommands(&primary);
    }

    if (!renderPass.valid())
    {
        primary.endRendering();

        if (mImageOptimizeForPresent != nullptr)
        {
            // finalizeColorImageLayout forces layout to Present.  If this is not the case, that
            // code was not run (so mImageOptimizeForPresentOriginalLayout is invalid).
            ASSERT(mImageOptimizeForPresent->getCurrentImageLayout() == ImageLayout::Present);

            // Restore the original layout of the image and do the real transition after the render
            // pass ends.
            mImageOptimizeForPresent->setCurrentImageLayout(renderer,
                                                            mImageOptimizeForPresentOriginalLayout);
            mImageOptimizeForPresent->recordWriteBarrierOneOff(renderer, ImageLayout::Present,
                                                               &primary, nullptr);
            mImageOptimizeForPresent               = nullptr;
            mImageOptimizeForPresentOriginalLayout = ImageLayout::Undefined;
        }
    }
    else
    {
        primary.endRenderPass();
    }

    // Now issue VkCmdSetEvents to primary command buffer
    ASSERT(mRefCountedEvents.empty());
    mVkEventArray.flushSetEvents(&primary);

    // Restart the command buffer.
    return reset(context, &commandsState->secondaryCommands);
}

void RenderPassCommandBufferHelper::addColorResolveAttachment(size_t colorIndexGL,
                                                              ImageHelper *image,
                                                              VkImageView view,
                                                              gl::LevelIndex level,
                                                              uint32_t layerStart,
                                                              uint32_t layerCount,
                                                              UniqueSerial imageSiblingSerial)
{
    mFramebuffer.addColorResolveAttachment(colorIndexGL, view);
    mRenderPassDesc.packColorResolveAttachment(colorIndexGL);

    PackedAttachmentIndex packedAttachmentIndex =
        mRenderPassDesc.getPackedColorAttachmentIndex(colorIndexGL);
    ASSERT(mColorResolveAttachments[packedAttachmentIndex].getImage() == nullptr);

    image->onRenderPassAttach(mQueueSerial);
    mColorResolveAttachments[packedAttachmentIndex].init(
        image, imageSiblingSerial, level, layerStart, layerCount, VK_IMAGE_ASPECT_COLOR_BIT);
}

void RenderPassCommandBufferHelper::addDepthStencilResolveAttachment(
    ImageHelper *image,
    VkImageView view,
    VkImageAspectFlags aspects,
    gl::LevelIndex level,
    uint32_t layerStart,
    uint32_t layerCount,
    UniqueSerial imageSiblingSerial)
{
    mFramebuffer.addDepthStencilResolveAttachment(view);
    if ((aspects & VK_IMAGE_ASPECT_DEPTH_BIT) != 0)
    {
        mRenderPassDesc.packDepthResolveAttachment();
    }
    if ((aspects & VK_IMAGE_ASPECT_STENCIL_BIT) != 0)
    {
        mRenderPassDesc.packStencilResolveAttachment();
    }

    image->onRenderPassAttach(mQueueSerial);
    mDepthResolveAttachment.init(image, imageSiblingSerial, level, layerStart, layerCount,
                                 VK_IMAGE_ASPECT_DEPTH_BIT);
    mStencilResolveAttachment.init(image, imageSiblingSerial, level, layerStart, layerCount,
                                   VK_IMAGE_ASPECT_STENCIL_BIT);
}

void RenderPassCommandBufferHelper::resumeTransformFeedback()
{
    ASSERT(isTransformFeedbackStarted());

    uint32_t numCounterBuffers =
        mRebindTransformFeedbackBuffers ? 0 : mValidTransformFeedbackBufferCount;

    mRebindTransformFeedbackBuffers    = false;
    mIsTransformFeedbackActiveUnpaused = true;

    getCommandBuffer().beginTransformFeedback(0, numCounterBuffers,
                                              mTransformFeedbackCounterBuffers.data(),
                                              mTransformFeedbackCounterBufferOffsets.data());
}

void RenderPassCommandBufferHelper::pauseTransformFeedback()
{
    ASSERT(isTransformFeedbackStarted() && isTransformFeedbackActiveUnpaused());
    mIsTransformFeedbackActiveUnpaused = false;
    getCommandBuffer().endTransformFeedback(0, mValidTransformFeedbackBufferCount,
                                            mTransformFeedbackCounterBuffers.data(),
                                            mTransformFeedbackCounterBufferOffsets.data());
}

void RenderPassCommandBufferHelper::updateRenderPassColorClear(PackedAttachmentIndex colorIndexVk,
                                                               const VkClearValue &clearValue)
{
    mAttachmentOps.setClearOp(colorIndexVk);
    mClearValues.storeColor(colorIndexVk, clearValue);
}

void RenderPassCommandBufferHelper::updateRenderPassDepthStencilClear(
    VkImageAspectFlags aspectFlags,
    const VkClearValue &clearValue)
{
    // Don't overwrite prior clear values for individual aspects.
    VkClearValue combinedClearValue = mClearValues[mDepthStencilAttachmentIndex];

    if ((aspectFlags & VK_IMAGE_ASPECT_DEPTH_BIT) != 0)
    {
        mAttachmentOps.setClearOp(mDepthStencilAttachmentIndex);
        combinedClearValue.depthStencil.depth = clearValue.depthStencil.depth;
    }

    if ((aspectFlags & VK_IMAGE_ASPECT_STENCIL_BIT) != 0)
    {
        mAttachmentOps.setClearStencilOp(mDepthStencilAttachmentIndex);
        combinedClearValue.depthStencil.stencil = clearValue.depthStencil.stencil;
    }

    // Bypass special D/S handling. This clear values array stores values packed.
    mClearValues.storeDepthStencil(mDepthStencilAttachmentIndex, combinedClearValue);
}

void RenderPassCommandBufferHelper::growRenderArea(ContextVk *contextVk,
                                                   const gl::Rectangle &newRenderArea)
{
    // The render area is grown such that it covers both the previous and the new render areas.
    gl::GetEnclosingRectangle(mRenderArea, newRenderArea, &mRenderArea);

    // Remove invalidates that are no longer applicable.
    mDepthAttachment.onRenderAreaGrowth(contextVk, mRenderArea);
    mStencilAttachment.onRenderAreaGrowth(contextVk, mRenderArea);
}

angle::Result RenderPassCommandBufferHelper::attachCommandPool(ErrorContext *context,
                                                               SecondaryCommandPool *commandPool)
{
    ASSERT(!mRenderPassStarted);
    ASSERT(getSubpassCommandBufferCount() == 1);
    return attachCommandPoolImpl<RenderPassCommandBufferHelper>(context, commandPool);
}

void RenderPassCommandBufferHelper::detachCommandPool(SecondaryCommandPool **commandPoolOut)
{
    ASSERT(mRenderPassStarted);
    angle::Result result =
        detachCommandPoolImpl<RenderPassCommandBufferHelper, true>(nullptr, commandPoolOut);
    ASSERT(result == angle::Result::Continue);
}

void RenderPassCommandBufferHelper::releaseCommandPool()
{
    ASSERT(!mRenderPassStarted);
    ASSERT(getSubpassCommandBufferCount() == 1);
    releaseCommandPoolImpl<RenderPassCommandBufferHelper>();
}

void RenderPassCommandBufferHelper::attachAllocator(SecondaryCommandMemoryAllocator *allocator)
{
    ASSERT(CheckSubpassCommandBufferCount(getSubpassCommandBufferCount()));
    attachAllocatorImpl<RenderPassCommandBufferHelper>(allocator);
}

SecondaryCommandMemoryAllocator *RenderPassCommandBufferHelper::detachAllocator()
{
    ASSERT(CheckSubpassCommandBufferCount(getSubpassCommandBufferCount()));
    return detachAllocatorImpl<RenderPassCommandBufferHelper>();
}

void RenderPassCommandBufferHelper::assertCanBeRecycled()
{
    ASSERT(!mRenderPassStarted);
    ASSERT(getSubpassCommandBufferCount() == 1);
    assertCanBeRecycledImpl<RenderPassCommandBufferHelper>();
}

std::string RenderPassCommandBufferHelper::getCommandDiagnostics()
{
    std::ostringstream out;
    addCommandDiagnosticsCommon(&out);

    size_t attachmentCount             = mRenderPassDesc.clearableAttachmentCount();
    size_t depthStencilAttachmentCount = mRenderPassDesc.hasDepthStencilAttachment() ? 1 : 0;
    size_t colorAttachmentCount        = attachmentCount - depthStencilAttachmentCount;

    PackedAttachmentIndex attachmentIndexVk(0);
    std::string loadOps, storeOps;

    if (colorAttachmentCount > 0)
    {
        loadOps += " Color: ";
        storeOps += " Color: ";

        for (size_t i = 0; i < colorAttachmentCount; ++i)
        {
            loadOps += GetLoadOpShorthand(
                static_cast<RenderPassLoadOp>(mAttachmentOps[attachmentIndexVk].loadOp));
            storeOps += GetStoreOpShorthand(
                static_cast<RenderPassStoreOp>(mAttachmentOps[attachmentIndexVk].storeOp));
            ++attachmentIndexVk;
        }
    }

    if (depthStencilAttachmentCount > 0)
    {
        ASSERT(depthStencilAttachmentCount == 1);

        loadOps += " Depth/Stencil: ";
        storeOps += " Depth/Stencil: ";

        loadOps += GetLoadOpShorthand(
            static_cast<RenderPassLoadOp>(mAttachmentOps[attachmentIndexVk].loadOp));
        loadOps += GetLoadOpShorthand(
            static_cast<RenderPassLoadOp>(mAttachmentOps[attachmentIndexVk].stencilLoadOp));

        storeOps += GetStoreOpShorthand(
            static_cast<RenderPassStoreOp>(mAttachmentOps[attachmentIndexVk].storeOp));
        storeOps += GetStoreOpShorthand(
            static_cast<RenderPassStoreOp>(mAttachmentOps[attachmentIndexVk].stencilStoreOp));
    }

    if (attachmentCount > 0)
    {
        out << "LoadOp:  " << loadOps << "\\l";
        out << "StoreOp: " << storeOps << "\\l";
    }

    for (uint32_t subpass = 0; subpass < getSubpassCommandBufferCount(); ++subpass)
    {
        if (subpass > 0)
        {
            out << "Next Subpass" << "\\l";
        }
        out << mCommandBuffers[subpass].dumpCommands("\\l");
    }

    return out.str();
}

// CommandBufferRecycler implementation.
template <typename CommandBufferHelperT>
void CommandBufferRecycler<CommandBufferHelperT>::onDestroy()
{
    std::unique_lock<angle::SimpleMutex> lock(mMutex);
    for (CommandBufferHelperT *commandBufferHelper : mCommandBufferHelperFreeList)
    {
        SafeDelete(commandBufferHelper);
    }
    mCommandBufferHelperFreeList.clear();
}

template void CommandBufferRecycler<OutsideRenderPassCommandBufferHelper>::onDestroy();
template void CommandBufferRecycler<RenderPassCommandBufferHelper>::onDestroy();

template <typename CommandBufferHelperT>
angle::Result CommandBufferRecycler<CommandBufferHelperT>::getCommandBufferHelper(
    ErrorContext *context,
    SecondaryCommandPool *commandPool,
    SecondaryCommandMemoryAllocator *commandsAllocator,
    CommandBufferHelperT **commandBufferHelperOut)
{
    std::unique_lock<angle::SimpleMutex> lock(mMutex);
    if (mCommandBufferHelperFreeList.empty())
    {
        CommandBufferHelperT *commandBuffer = new CommandBufferHelperT();
        *commandBufferHelperOut             = commandBuffer;
        ANGLE_TRY(commandBuffer->initialize(context));
    }
    else
    {
        CommandBufferHelperT *commandBuffer = mCommandBufferHelperFreeList.back();
        mCommandBufferHelperFreeList.pop_back();
        *commandBufferHelperOut = commandBuffer;
    }

    ANGLE_TRY((*commandBufferHelperOut)->attachCommandPool(context, commandPool));

    // Attach functions are only used for ring buffer allocators.
    (*commandBufferHelperOut)->attachAllocator(commandsAllocator);

    return angle::Result::Continue;
}

template angle::Result
CommandBufferRecycler<OutsideRenderPassCommandBufferHelper>::getCommandBufferHelper(
    ErrorContext *,
    SecondaryCommandPool *,
    SecondaryCommandMemoryAllocator *,
    OutsideRenderPassCommandBufferHelper **);
template angle::Result CommandBufferRecycler<RenderPassCommandBufferHelper>::getCommandBufferHelper(
    ErrorContext *,
    SecondaryCommandPool *,
    SecondaryCommandMemoryAllocator *,
    RenderPassCommandBufferHelper **);

template <typename CommandBufferHelperT>
void CommandBufferRecycler<CommandBufferHelperT>::recycleCommandBufferHelper(
    CommandBufferHelperT **commandBuffer)
{
    (*commandBuffer)->assertCanBeRecycled();
    (*commandBuffer)->markOpen();

    {
        std::unique_lock<angle::SimpleMutex> lock(mMutex);
        mCommandBufferHelperFreeList.push_back(*commandBuffer);
    }

    *commandBuffer = nullptr;
}

template void
CommandBufferRecycler<OutsideRenderPassCommandBufferHelper>::recycleCommandBufferHelper(
    OutsideRenderPassCommandBufferHelper **);
template void CommandBufferRecycler<RenderPassCommandBufferHelper>::recycleCommandBufferHelper(
    RenderPassCommandBufferHelper **);

// SecondaryCommandBufferCollector implementation.
void SecondaryCommandBufferCollector::collectCommandBuffer(
    priv::SecondaryCommandBuffer &&commandBuffer)
{
    commandBuffer.reset();
}

void SecondaryCommandBufferCollector::collectCommandBuffer(
    VulkanSecondaryCommandBuffer &&commandBuffer)
{
    ASSERT(commandBuffer.valid());
    mCollectedCommandBuffers.emplace_back(std::move(commandBuffer));
}

void SecondaryCommandBufferCollector::releaseCommandBuffers()
{
    // Note: we currently free the command buffers individually, but we could potentially reset the
    // entire command pool.  https://issuetracker.google.com/issues/166793850
    for (VulkanSecondaryCommandBuffer &commandBuffer : mCollectedCommandBuffers)
    {
        commandBuffer.destroy();
    }
    mCollectedCommandBuffers.clear();
}

// DynamicBuffer implementation.
DynamicBuffer::DynamicBuffer()
    : mUsage(0),
      mHostVisible(false),
      mInitialSize(0),
      mNextAllocationOffset(0),
      mSize(0),
      mSizeInRecentHistory(0),
      mAlignment(0),
      mMemoryPropertyFlags(0)
{}

DynamicBuffer::DynamicBuffer(DynamicBuffer &&other)
    : mUsage(other.mUsage),
      mHostVisible(other.mHostVisible),
      mInitialSize(other.mInitialSize),
      mBuffer(std::move(other.mBuffer)),
      mNextAllocationOffset(other.mNextAllocationOffset),
      mSize(other.mSize),
      mSizeInRecentHistory(other.mSizeInRecentHistory),
      mAlignment(other.mAlignment),
      mMemoryPropertyFlags(other.mMemoryPropertyFlags),
      mInFlightBuffers(std::move(other.mInFlightBuffers)),
      mBufferFreeList(std::move(other.mBufferFreeList))
{}

void DynamicBuffer::init(Renderer *renderer,
                         VkBufferUsageFlags usage,
                         size_t alignment,
                         size_t initialSize,
                         bool hostVisible)
{
    mUsage       = usage;
    mHostVisible = hostVisible;
    mMemoryPropertyFlags =
        (hostVisible) ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    if (hostVisible && renderer->getFeatures().preferHostCachedForNonStaticBufferUsage.enabled)
    {
        mMemoryPropertyFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    }

    // Check that we haven't overridden the initial size of the buffer in setMinimumSizeForTesting.
    if (mInitialSize == 0)
    {
        mInitialSize         = initialSize;
        mSize                = 0;
        mSizeInRecentHistory = initialSize;
    }

    // Workaround for the mock ICD not supporting allocations greater than 0x1000.
    // Could be removed if https://github.com/KhronosGroup/Vulkan-Tools/issues/84 is fixed.
    if (renderer->isMockICDEnabled())
    {
        mSize = std::min<size_t>(mSize, 0x1000);
    }

    requireAlignment(renderer, alignment);
}

DynamicBuffer::~DynamicBuffer()
{
    ASSERT(mBuffer == nullptr);
    ASSERT(mInFlightBuffers.empty());
    ASSERT(mBufferFreeList.empty());
}

angle::Result DynamicBuffer::allocateNewBuffer(ErrorContext *context)
{
    context->getPerfCounters().dynamicBufferAllocations++;

    // Allocate the buffer
    ASSERT(!mBuffer);
    mBuffer = std::make_unique<BufferHelper>();

    VkBufferCreateInfo createInfo    = {};
    createInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.flags                 = 0;
    createInfo.size                  = mSize;
    createInfo.usage                 = mUsage;
    createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices   = nullptr;

    return mBuffer->init(context, createInfo, mMemoryPropertyFlags);
}

bool DynamicBuffer::allocateFromCurrentBuffer(size_t sizeInBytes, BufferHelper **bufferHelperOut)
{
    mNextAllocationOffset =
        roundUp<uint32_t>(mNextAllocationOffset, static_cast<uint32_t>(mAlignment));

    ASSERT(bufferHelperOut);
    size_t sizeToAllocate                                      = roundUp(sizeInBytes, mAlignment);
    angle::base::CheckedNumeric<size_t> checkedNextWriteOffset = mNextAllocationOffset;
    checkedNextWriteOffset += sizeToAllocate;

    if (!checkedNextWriteOffset.IsValid() || checkedNextWriteOffset.ValueOrDie() > mSize)
    {
        return false;
    }

    ASSERT(mBuffer != nullptr);
    ASSERT(mHostVisible);
    ASSERT(mBuffer->getMappedMemory());
    mBuffer->setSuballocationOffsetAndSize(mNextAllocationOffset, sizeToAllocate);
    *bufferHelperOut = mBuffer.get();

    mNextAllocationOffset += static_cast<uint32_t>(sizeToAllocate);
    return true;
}

angle::Result DynamicBuffer::allocate(Context *context,
                                      size_t sizeInBytes,
                                      BufferHelper **bufferHelperOut,
                                      bool *newBufferAllocatedOut)
{
    bool newBuffer = !allocateFromCurrentBuffer(sizeInBytes, bufferHelperOut);
    if (newBufferAllocatedOut)
    {
        *newBufferAllocatedOut = newBuffer;
    }

    if (!newBuffer)
    {
        return angle::Result::Continue;
    }

    size_t sizeToAllocate = roundUp(sizeInBytes, mAlignment);

    if (mBuffer)
    {
        // Make sure the buffer is not released externally.
        ASSERT(mBuffer->valid());
        mInFlightBuffers.push_back(std::move(mBuffer));
        ASSERT(!mBuffer);
    }

    Renderer *renderer = context->getRenderer();

    const size_t minRequiredBlockSize = std::max(mInitialSize, sizeToAllocate);

    // The average required buffer size in recent history is used to determine whether the currently
    // used buffer size needs to be reduced (when it goes below 1/8 of the current buffer size).
    constexpr uint32_t kDecayCoeffPercent = 20;
    static_assert(kDecayCoeffPercent >= 0 && kDecayCoeffPercent <= 100);
    mSizeInRecentHistory = (mSizeInRecentHistory * kDecayCoeffPercent +
                            minRequiredBlockSize * (100 - kDecayCoeffPercent) + 50) /
                           100;

    if (sizeToAllocate > mSize || mSizeInRecentHistory < mSize / 8)
    {
        mSize = minRequiredBlockSize;
        // Clear the free list since the free buffers are now either too small or too big.
        ReleaseBufferListToRenderer(context, &mBufferFreeList);
    }

    // The front of the free list should be the oldest. Thus if it is in use the rest of the
    // free list should be in use as well.
    if (mBufferFreeList.empty() ||
        !renderer->hasResourceUseFinished(mBufferFreeList.front()->getResourceUse()))
    {
        ANGLE_TRY(allocateNewBuffer(context));
    }
    else
    {
        mBuffer = std::move(mBufferFreeList.front());
        mBufferFreeList.pop_front();
    }

    ASSERT(mBuffer->getBlockMemorySize() == mSize);

    mNextAllocationOffset = 0;

    ASSERT(mBuffer != nullptr);
    mBuffer->setSuballocationOffsetAndSize(mNextAllocationOffset, sizeToAllocate);
    *bufferHelperOut = mBuffer.get();

    mNextAllocationOffset += static_cast<uint32_t>(sizeToAllocate);
    return angle::Result::Continue;
}

void DynamicBuffer::release(Context *context)
{
    reset();

    ReleaseBufferListToRenderer(context, &mInFlightBuffers);
    ReleaseBufferListToRenderer(context, &mBufferFreeList);

    if (mBuffer)
    {
        mBuffer->release(context);
        mBuffer.reset(nullptr);
    }
}

void DynamicBuffer::updateQueueSerialAndReleaseInFlightBuffers(ContextVk *contextVk,
                                                               const QueueSerial &queueSerial)
{
    for (std::unique_ptr<BufferHelper> &bufferHelper : mInFlightBuffers)
    {
        // This function is used only for internal buffers, and they are all read-only.
        // It's possible this may change in the future, but there isn't a good way to detect that,
        // unfortunately.
        bufferHelper->setQueueSerial(queueSerial);

        // We only keep free buffers that have the same size. Note that bufferHelper's size is
        // suballocation's size. We need to use the whole block memory size here.
        if (bufferHelper->getBlockMemorySize() != mSize)
        {
            bufferHelper->release(contextVk);
        }
        else
        {
            mBufferFreeList.push_back(std::move(bufferHelper));
        }
    }
    mInFlightBuffers.clear();
}

void DynamicBuffer::destroy(Renderer *renderer)
{
    reset();

    DestroyBufferList(renderer, &mInFlightBuffers);
    DestroyBufferList(renderer, &mBufferFreeList);

    if (mBuffer)
    {
        mBuffer->unmap(renderer);
        mBuffer->destroy(renderer);
        mBuffer.reset(nullptr);
    }
}

void DynamicBuffer::requireAlignment(Renderer *renderer, size_t alignment)
{
    ASSERT(alignment > 0);

    size_t prevAlignment = mAlignment;

    // If alignment was never set, initialize it with the atom size limit.
    if (prevAlignment == 0)
    {
        prevAlignment =
            static_cast<size_t>(renderer->getPhysicalDeviceProperties().limits.nonCoherentAtomSize);
        ASSERT(gl::isPow2(prevAlignment));
    }

    // We need lcm(prevAlignment, alignment).  Usually, one divides the other so std::max() could be
    // used instead.  Only known case where this assumption breaks is for 3-component types with
    // 16- or 32-bit channels, so that's special-cased to avoid a full-fledged lcm implementation.

    if (gl::isPow2(prevAlignment * alignment))
    {
        ASSERT(alignment % prevAlignment == 0 || prevAlignment % alignment == 0);

        alignment = std::max(prevAlignment, alignment);
    }
    else
    {
        ASSERT(prevAlignment % 3 != 0 || gl::isPow2(prevAlignment / 3));
        ASSERT(alignment % 3 != 0 || gl::isPow2(alignment / 3));

        prevAlignment = prevAlignment % 3 == 0 ? prevAlignment / 3 : prevAlignment;
        alignment     = alignment % 3 == 0 ? alignment / 3 : alignment;

        alignment = std::max(prevAlignment, alignment) * 3;
    }

    // If alignment has changed, make sure the next allocation is done at an aligned offset.
    if (alignment != mAlignment)
    {
        mNextAllocationOffset = roundUp(mNextAllocationOffset, static_cast<uint32_t>(alignment));
    }

    mAlignment = alignment;
}

void DynamicBuffer::setMinimumSizeForTesting(size_t minSize)
{
    // This will really only have an effect next time we call allocate.
    mInitialSize = minSize;

    // Forces a new allocation on the next allocate.
    mSize                = 0;
    mSizeInRecentHistory = 0;
}

void DynamicBuffer::reset()
{
    mSize                 = 0;
    mSizeInRecentHistory  = 0;
    mNextAllocationOffset = 0;
}

// BufferPool implementation.
BufferPool::BufferPool()
    : mVirtualBlockCreateFlags(vma::VirtualBlockCreateFlagBits::GENERAL),
      mUsage(0),
      mHostVisible(false),
      mSize(0),
      mMemoryTypeIndex(0),
      mTotalMemorySize(0),
      mNumberOfNewBuffersNeededSinceLastPrune(0)
{}

BufferPool::BufferPool(BufferPool &&other)
    : mVirtualBlockCreateFlags(other.mVirtualBlockCreateFlags),
      mUsage(other.mUsage),
      mHostVisible(other.mHostVisible),
      mSize(other.mSize),
      mMemoryTypeIndex(other.mMemoryTypeIndex)
{}

void BufferPool::initWithFlags(Renderer *renderer,
                               vma::VirtualBlockCreateFlags flags,
                               VkBufferUsageFlags usage,
                               VkDeviceSize initialSize,
                               uint32_t memoryTypeIndex,
                               VkMemoryPropertyFlags memoryPropertyFlags)
{
    mVirtualBlockCreateFlags = flags;
    mUsage                   = usage;
    mMemoryTypeIndex         = memoryTypeIndex;
    if (initialSize)
    {
        // Should be power of two
        ASSERT(gl::isPow2(initialSize));
        mSize = initialSize;
    }
    else
    {
        mSize = renderer->getPreferedBufferBlockSize(memoryTypeIndex);
    }
    mHostVisible = ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0);
}

BufferPool::~BufferPool()
{
    ASSERT(mBufferBlocks.empty());
    ASSERT(mEmptyBufferBlocks.empty());
}

void BufferPool::pruneEmptyBuffers(Renderer *renderer)
{
    // First try to walk through mBuffers and move empty buffers to mEmptyBuffer and remove null
    // pointers for allocation performance.
    // The expectation is that we will find none needs to be compacted in most calls.
    bool needsCompact = false;
    for (std::unique_ptr<BufferBlock> &block : mBufferBlocks)
    {
        if (block->isEmpty())
        {
            // We will always free empty buffers that has smaller size. Or if the empty buffer has
            // been found empty for long enough time, or we accumulated too many empty buffers, we
            // also free it.
            if (block->getMemorySize() < mSize)
            {
                mTotalMemorySize -= block->getMemorySize();
                block->destroy(renderer);
                block.reset();
            }
            else
            {
                mEmptyBufferBlocks.push_back(std::move(block));
            }
            needsCompact = true;
        }
    }

    // Now remove the null pointers that left by empty buffers all at once, if any.
    if (needsCompact)
    {
        BufferBlockPointerVector compactedBlocks;
        for (std::unique_ptr<BufferBlock> &block : mBufferBlocks)
        {
            if (block)
            {
                compactedBlocks.push_back(std::move(block));
            }
        }
        mBufferBlocks = std::move(compactedBlocks);
    }

    // Decide how many empty buffers to keep around and trim down the excessive empty buffers. We
    // keep track of how many buffers are needed since last prune. Assume we are in stable state,
    // which means we may still need that many empty buffers in next prune cycle. To reduce chance
    // to call into vulkan driver to allocate new buffers, we try to keep that many empty buffers
    // around, subject to the maximum cap. If we overestimate, next cycle they used fewer buffers,
    // we will trim excessive empty buffers at next prune call. Or if we underestimate, we will end
    // up have to call into vulkan driver allocate new buffers, but next cycle we should correct
    // ourselves to keep enough number of empty buffers around.
    size_t buffersToKeep = std::min(mNumberOfNewBuffersNeededSinceLastPrune,
                                    static_cast<size_t>(kMaxTotalEmptyBufferBytes / mSize));
    while (mEmptyBufferBlocks.size() > buffersToKeep)
    {
        std::unique_ptr<BufferBlock> &block = mEmptyBufferBlocks.back();
        mTotalMemorySize -= block->getMemorySize();
        block->destroy(renderer);
        mEmptyBufferBlocks.pop_back();
    }
    mNumberOfNewBuffersNeededSinceLastPrune = 0;
}

VkResult BufferPool::allocateNewBuffer(ErrorContext *context, VkDeviceSize sizeInBytes)
{
    Renderer *renderer         = context->getRenderer();
    const Allocator &allocator = renderer->getAllocator();

    VkDeviceSize heapSize =
        renderer->getMemoryProperties().getHeapSizeForMemoryType(mMemoryTypeIndex);

    // First ensure we are not exceeding the heapSize to avoid the validation error.
    VK_RESULT_CHECK(sizeInBytes <= heapSize, VK_ERROR_OUT_OF_DEVICE_MEMORY);

    // Double the size until meet the requirement. This also helps reducing the fragmentation. Since
    // this is global pool, we have less worry about memory waste.
    VkDeviceSize newSize = mSize;
    while (newSize < sizeInBytes)
    {
        newSize <<= 1;
    }
    mSize = std::min(newSize, heapSize);

    // Allocate buffer
    VkBufferCreateInfo createInfo    = {};
    createInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.flags                 = 0;
    createInfo.size                  = mSize;
    createInfo.usage                 = mUsage;
    createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices   = nullptr;

    VkMemoryPropertyFlags memoryPropertyFlags;
    allocator.getMemoryTypeProperties(mMemoryTypeIndex, &memoryPropertyFlags);

    DeviceScoped<Buffer> buffer(renderer->getDevice());
    VK_RESULT_TRY(buffer.get().init(context->getDevice(), createInfo));

    DeviceScoped<DeviceMemory> deviceMemory(renderer->getDevice());
    VkMemoryPropertyFlags memoryPropertyFlagsOut;
    VkDeviceSize sizeOut;
    uint32_t memoryTypeIndex;
    VK_RESULT_TRY(AllocateBufferMemory(context, MemoryAllocationType::Buffer, memoryPropertyFlags,
                                       &memoryPropertyFlagsOut, nullptr, &buffer.get(),
                                       &memoryTypeIndex, &deviceMemory.get(), &sizeOut));
    ASSERT(sizeOut >= mSize);

    // Allocate bufferBlock
    std::unique_ptr<BufferBlock> block = std::make_unique<BufferBlock>();
    VK_RESULT_TRY(block->init(context, buffer.get(), memoryTypeIndex, mVirtualBlockCreateFlags,
                              deviceMemory.get(), memoryPropertyFlagsOut, mSize));

    if (mHostVisible)
    {
        VK_RESULT_TRY(block->map(context->getDevice()));
    }

    mTotalMemorySize += block->getMemorySize();
    // Append the bufferBlock into the pool
    mBufferBlocks.push_back(std::move(block));
    context->getPerfCounters().allocateNewBufferBlockCalls++;

    return VK_SUCCESS;
}

VkResult BufferPool::allocateBuffer(ErrorContext *context,
                                    VkDeviceSize sizeInBytes,
                                    VkDeviceSize alignment,
                                    BufferSuballocation *suballocation)
{
    ASSERT(alignment);
    VmaVirtualAllocation allocation;
    VkDeviceSize offset;
    VkDeviceSize alignedSize = roundUp(sizeInBytes, alignment);

    if (alignedSize >= kMaxBufferSizeForSuballocation)
    {
        VkDeviceSize heapSize =
            context->getRenderer()->getMemoryProperties().getHeapSizeForMemoryType(
                mMemoryTypeIndex);
        // First ensure we are not exceeding the heapSize to avoid the validation error.
        VK_RESULT_CHECK(sizeInBytes <= heapSize, VK_ERROR_OUT_OF_DEVICE_MEMORY);

        // Allocate buffer
        VkBufferCreateInfo createInfo    = {};
        createInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.flags                 = 0;
        createInfo.size                  = alignedSize;
        createInfo.usage                 = mUsage;
        createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;

        VkMemoryPropertyFlags memoryPropertyFlags;
        const Allocator &allocator = context->getRenderer()->getAllocator();
        allocator.getMemoryTypeProperties(mMemoryTypeIndex, &memoryPropertyFlags);

        DeviceScoped<Buffer> buffer(context->getDevice());
        VK_RESULT_TRY(buffer.get().init(context->getDevice(), createInfo));

        DeviceScoped<DeviceMemory> deviceMemory(context->getDevice());
        VkMemoryPropertyFlags memoryPropertyFlagsOut;
        VkDeviceSize sizeOut;
        uint32_t memoryTypeIndex;
        VK_RESULT_TRY(AllocateBufferMemory(
            context, MemoryAllocationType::Buffer, memoryPropertyFlags, &memoryPropertyFlagsOut,
            nullptr, &buffer.get(), &memoryTypeIndex, &deviceMemory.get(), &sizeOut));
        ASSERT(sizeOut >= alignedSize);

        suballocation->initWithEntireBuffer(context, buffer.get(), MemoryAllocationType::Buffer,
                                            memoryTypeIndex, deviceMemory.get(),
                                            memoryPropertyFlagsOut, alignedSize, sizeOut);
        if (mHostVisible)
        {
            VK_RESULT_TRY(suballocation->map(context));
        }
        return VK_SUCCESS;
    }

    // We always allocate from reverse order so that older buffers have a chance to be empty. The
    // assumption is that to allocate from new buffers first may have a better chance to leave the
    // older buffers completely empty and we may able to free it.
    for (auto iter = mBufferBlocks.rbegin(); iter != mBufferBlocks.rend();)
    {
        std::unique_ptr<BufferBlock> &block = *iter;
        if (block->isEmpty() && block->getMemorySize() < mSize)
        {
            // Don't try to allocate from an empty buffer that has smaller size. It will get
            // released when pruneEmptyBuffers get called later on.
            ++iter;
            continue;
        }

        if (block->allocate(alignedSize, alignment, &allocation, &offset) == VK_SUCCESS)
        {
            suballocation->init(block.get(), allocation, offset, alignedSize);
            return VK_SUCCESS;
        }
        ++iter;
    }

    // Try to allocate from empty buffers
    while (!mEmptyBufferBlocks.empty())
    {
        std::unique_ptr<BufferBlock> &block = mEmptyBufferBlocks.back();
        if (block->getMemorySize() < mSize)
        {
            mTotalMemorySize -= block->getMemorySize();
            block->destroy(context->getRenderer());
            mEmptyBufferBlocks.pop_back();
        }
        else
        {
            VK_RESULT_TRY(block->allocate(alignedSize, alignment, &allocation, &offset));
            suballocation->init(block.get(), allocation, offset, alignedSize);
            mBufferBlocks.push_back(std::move(block));
            mEmptyBufferBlocks.pop_back();
            mNumberOfNewBuffersNeededSinceLastPrune++;
            return VK_SUCCESS;
        }
    }

    // Failed to allocate from empty buffer. Now try to allocate a new buffer.
    VK_RESULT_TRY(allocateNewBuffer(context, alignedSize));

    // Sub-allocate from the bufferBlock.
    std::unique_ptr<BufferBlock> &block = mBufferBlocks.back();
    VK_RESULT_CHECK(block->allocate(alignedSize, alignment, &allocation, &offset) == VK_SUCCESS,
                    VK_ERROR_OUT_OF_DEVICE_MEMORY);
    suballocation->init(block.get(), allocation, offset, alignedSize);
    mNumberOfNewBuffersNeededSinceLastPrune++;

    return VK_SUCCESS;
}

void BufferPool::destroy(Renderer *renderer, bool orphanNonEmptyBufferBlock)
{
    for (std::unique_ptr<BufferBlock> &block : mBufferBlocks)
    {
        if (block->isEmpty())
        {
            block->destroy(renderer);
        }
        else
        {
            // When orphan is not allowed, all BufferBlocks must be empty.
            ASSERT(orphanNonEmptyBufferBlock);
            renderer->addBufferBlockToOrphanList(block.release());
        }
    }
    mBufferBlocks.clear();

    for (std::unique_ptr<BufferBlock> &block : mEmptyBufferBlocks)
    {
        block->destroy(renderer);
    }
    mEmptyBufferBlocks.clear();
}

VkDeviceSize BufferPool::getTotalEmptyMemorySize() const
{
    VkDeviceSize totalMemorySize = 0;
    for (const std::unique_ptr<BufferBlock> &block : mEmptyBufferBlocks)
    {
        totalMemorySize += block->getMemorySize();
    }
    return totalMemorySize;
}

void BufferPool::addStats(std::ostringstream *out) const
{
    VkDeviceSize totalUnusedBytes = 0;
    VkDeviceSize totalMemorySize  = 0;
    for (size_t i = 0; i < mBufferBlocks.size(); i++)
    {
        const std::unique_ptr<BufferBlock> &block = mBufferBlocks[i];
        vma::StatInfo statInfo;
        block->calculateStats(&statInfo);
        ASSERT(statInfo.basicInfo.blockCount == 1);
        INFO() << "[" << i << "]={" << " allocationCount:" << statInfo.basicInfo.allocationCount
               << " blockBytes:" << statInfo.basicInfo.blockBytes
               << " allocationBytes:" << statInfo.basicInfo.allocationBytes
               << " unusedRangeCount:" << statInfo.unusedRangeCount
               << " allocationSizeMin:" << statInfo.allocationSizeMin
               << " allocationSizeMax:" << statInfo.allocationSizeMax
               << " unusedRangeSizeMin:" << statInfo.unusedRangeSizeMin
               << " unusedRangeSizeMax:" << statInfo.unusedRangeSizeMax << " }";
        VkDeviceSize unusedBytes =
            statInfo.basicInfo.blockBytes - statInfo.basicInfo.allocationBytes;
        totalUnusedBytes += unusedBytes;
        totalMemorySize += block->getMemorySize();
    }
    *out << "mBufferBlocks.size():" << mBufferBlocks.size()
         << " totalUnusedBytes:" << totalUnusedBytes / 1024
         << "KB / totalMemorySize:" << totalMemorySize / 1024 << "KB";
    *out << " emptyBuffers [memorySize:" << getTotalEmptyMemorySize() / 1024 << "KB "
         << " count:" << mEmptyBufferBlocks.size()
         << " needed: " << mNumberOfNewBuffersNeededSinceLastPrune << "]";
}

// DescriptorSetHelper implementation.
void DescriptorSetHelper::destroy(VkDevice device)
{
    if (valid())
    {
        // Since the pool is created without VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, we
        // don't call vkFreeDescriptorSets. We always add to garbage list so that it can be
        // recycled. Since we dont actually know if it is GPU completed, we always just add to the
        // pending garbage list assuming the worst case.
        DescriptorPoolPointer pool(device, mPool);
        DescriptorSetPointer garbage(device, std::move(*this));
        pool->addPendingGarbage(std::move(garbage));
        ASSERT(!valid());
    }
}

// DescriptorPoolHelper implementation.
DescriptorPoolHelper::DescriptorPoolHelper()
    : mMaxDescriptorSets(0), mValidDescriptorSets(0), mFreeDescriptorSets(0)
{}

DescriptorPoolHelper::~DescriptorPoolHelper()
{
    ASSERT(mPendingGarbageList.empty());
    ASSERT(mFinishedGarbageList.empty());
}

angle::Result DescriptorPoolHelper::init(ErrorContext *context,
                                         const std::vector<VkDescriptorPoolSize> &poolSizesIn,
                                         uint32_t maxSets)
{
    Renderer *renderer = context->getRenderer();

    ASSERT(mPendingGarbageList.empty());
    ASSERT(mFinishedGarbageList.empty());

    if (mDescriptorPool.valid())
    {
        mDescriptorPool.destroy(renderer->getDevice());
    }

    // Make a copy of the pool sizes, so we can grow them to satisfy the specified maxSets.
    std::vector<VkDescriptorPoolSize> poolSizes = poolSizesIn;

    for (VkDescriptorPoolSize &poolSize : poolSizes)
    {
        poolSize.descriptorCount *= maxSets;
    }

    VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
    descriptorPoolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.flags                      = 0;
    descriptorPoolInfo.maxSets                    = maxSets;
    descriptorPoolInfo.poolSizeCount              = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes                 = poolSizes.data();

    mMaxDescriptorSets   = maxSets;
    mFreeDescriptorSets  = maxSets;
    mValidDescriptorSets = 0;

    ANGLE_VK_TRY(context, mDescriptorPool.init(renderer->getDevice(), descriptorPoolInfo));

    mRenderer = renderer;

    return angle::Result::Continue;
}

void DescriptorPoolHelper::destroy(VkDevice device)
{
    ASSERT(mValidDescriptorSets == 0);
    ASSERT(mPendingGarbageList.empty());
    ASSERT(mFinishedGarbageList.empty());
    mDescriptorPool.destroy(device);
}

bool DescriptorPoolHelper::allocateVkDescriptorSet(ErrorContext *context,
                                                   const DescriptorSetLayout &descriptorSetLayout,
                                                   VkDescriptorSet *descriptorSetOut)
{
    if (mFreeDescriptorSets > 0)
    {
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool              = mDescriptorPool.getHandle();
        allocInfo.descriptorSetCount          = 1;
        allocInfo.pSetLayouts                 = descriptorSetLayout.ptr();

        VkResult result = mDescriptorPool.allocateDescriptorSets(context->getDevice(), allocInfo,
                                                                 descriptorSetOut);
        ++context->getPerfCounters().descriptorSetAllocations;
        // If fail, it means our own accounting has a bug.
        ASSERT(result == VK_SUCCESS);
        mFreeDescriptorSets--;
        mValidDescriptorSets++;
        return true;
    }

    return false;
}

void DescriptorPoolHelper::cleanupPendingGarbage()
{
    while (!mPendingGarbageList.empty())
    {
        DescriptorSetPointer &garbage = mPendingGarbageList.front();
        if (!mRenderer->hasResourceUseFinished(garbage->getResourceUse()))
        {
            break;
        }
        mFinishedGarbageList.push_back(std::move(garbage));
        mPendingGarbageList.pop_front();
    }
}

bool DescriptorPoolHelper::recycleFromGarbage(Renderer *renderer,
                                              DescriptorSetPointer *descriptorSetOut)
{
    if (mFinishedGarbageList.empty())
    {
        cleanupPendingGarbage();
    }

    if (!mFinishedGarbageList.empty())
    {
        DescriptorSetPointer &garbage = mFinishedGarbageList.front();
        *descriptorSetOut             = std::move(garbage);
        mFinishedGarbageList.pop_front();
        mValidDescriptorSets++;
        return true;
    }

    return false;
}

bool DescriptorPoolHelper::allocateDescriptorSet(ErrorContext *context,
                                                 const DescriptorSetLayout &descriptorSetLayout,
                                                 const DescriptorPoolPointer &pool,
                                                 DescriptorSetPointer *descriptorSetOut)
{
    ASSERT(pool.get() == this);
    VkDescriptorSet descriptorSet;
    if (allocateVkDescriptorSet(context, descriptorSetLayout, &descriptorSet))
    {
        DescriptorSetHelper helper = DescriptorSetHelper(descriptorSet, pool);
        *descriptorSetOut          = DescriptorSetPointer(context->getDevice(), std::move(helper));
        return true;
    }
    return false;
}

void DescriptorPoolHelper::destroyGarbage()
{
    ASSERT(mPendingGarbageList.empty());

    while (!mFinishedGarbageList.empty())
    {
        DescriptorSetPointer &garbage = mFinishedGarbageList.front();
        ASSERT(garbage.unique());
        ASSERT(garbage->valid());
        // Because we do not use VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT when pool is
        // created, We can't free each individual descriptor set before destroying the pool, we
        // simply clear the descriptorSet and the mPool weak pointer so that
        // DescriptorSetHelper::destroy will not find it the garbage being valid  and try to add to
        // garbage list again.
        garbage->mDescriptorSet = VK_NULL_HANDLE;
        garbage->mPool.reset();
        ASSERT(!garbage->valid());
        mFinishedGarbageList.pop_front();
    }
}

// DynamicDescriptorPool implementation.
DynamicDescriptorPool::DynamicDescriptorPool() : mCachedDescriptorSetLayout(VK_NULL_HANDLE) {}

DynamicDescriptorPool::~DynamicDescriptorPool()
{
    ASSERT(mLRUList.empty());
    ASSERT(mDescriptorSetCache.empty());
    ASSERT(mDescriptorPools.empty());
}

DynamicDescriptorPool::DynamicDescriptorPool(DynamicDescriptorPool &&other)
    : DynamicDescriptorPool()
{
    *this = std::move(other);
}

DynamicDescriptorPool &DynamicDescriptorPool::operator=(DynamicDescriptorPool &&other)
{
    std::swap(mDescriptorPools, other.mDescriptorPools);
    std::swap(mPoolSizes, other.mPoolSizes);
    std::swap(mCachedDescriptorSetLayout, other.mCachedDescriptorSetLayout);
    std::swap(mLRUList, other.mLRUList);
    std::swap(mDescriptorSetCache, other.mDescriptorSetCache);
    return *this;
}

angle::Result DynamicDescriptorPool::init(ErrorContext *context,
                                          const VkDescriptorPoolSize *setSizes,
                                          size_t setSizeCount,
                                          const DescriptorSetLayout &descriptorSetLayout)
{
    ASSERT(setSizes);
    ASSERT(setSizeCount);
    ASSERT(mDescriptorPools.empty());
    ASSERT(mCachedDescriptorSetLayout == VK_NULL_HANDLE);

    mPoolSizes.assign(setSizes, setSizes + setSizeCount);
    mCachedDescriptorSetLayout = descriptorSetLayout.getHandle();

    DescriptorPoolPointer newPool = DescriptorPoolPointer::MakeShared(context->getDevice());
    ANGLE_TRY(newPool->init(context, mPoolSizes, mMaxSetsPerPool));

    mDescriptorPools.emplace_back(std::move(newPool));

    return angle::Result::Continue;
}

void DynamicDescriptorPool::destroy(VkDevice device)
{
    // Destroy cache
    mDescriptorSetCache.clear();

    // Destroy LRU list and SharedDescriptorSetCacheKey.
    for (auto it = mLRUList.begin(); it != mLRUList.end();)
    {
        (it->sharedCacheKey)->destroy(device);
        it = mLRUList.erase(it);
    }
    ASSERT(mLRUList.empty());

    for (DescriptorPoolPointer &pool : mDescriptorPools)
    {
        pool->cleanupPendingGarbage();
        pool->destroyGarbage();
        ASSERT(pool.unique());
    }
    mDescriptorPools.clear();

    mCachedDescriptorSetLayout = VK_NULL_HANDLE;
}

bool DynamicDescriptorPool::allocateFromExistingPool(ErrorContext *context,
                                                     const DescriptorSetLayout &descriptorSetLayout,
                                                     DescriptorSetPointer *descriptorSetOut)
{
    for (size_t poolIndex = 0; poolIndex < mDescriptorPools.size(); ++poolIndex)
    {
        DescriptorPoolPointer &pool = mDescriptorPools[poolIndex];
        if (!pool || !pool->valid())
        {
            continue;
        }
        if (pool->allocateDescriptorSet(context, descriptorSetLayout, pool, descriptorSetOut))
        {
            return true;
        }
    }
    return false;
}

bool DynamicDescriptorPool::recycleFromGarbage(Renderer *renderer,
                                               DescriptorSetPointer *descriptorSetOut)
{
    for (DescriptorPoolPointer &pool : mDescriptorPools)
    {
        if (pool->recycleFromGarbage(renderer, descriptorSetOut))
        {
            return true;
        }
    }
    return false;
}

bool DynamicDescriptorPool::evictStaleDescriptorSets(Renderer *renderer,
                                                     uint32_t oldestFrameToKeep,
                                                     uint32_t currentFrame)
{
    ASSERT(oldestFrameToKeep < currentFrame);
    size_t descriptorSetEvicted = 0;
    // Walk LRU list backwards from oldest to most recent, evict anything that earlier than
    // oldestFrameIDToKeep.
    auto it = mLRUList.rbegin();
    while (it != mLRUList.rend())
    {
        DescriptorSetPointer &descriptorSet = it->descriptorSet;
        if (descriptorSet.unique())
        {
            // Stop if it is recently being used.
            if (descriptorSet->getLastUsedFrame() > oldestFrameToKeep)
            {
                break;
            }
            // Stop if GPU is still busy
            if (!renderer->hasResourceUseFinished(descriptorSet->getResourceUse()))
            {
                break;
            }
            // Evict it from the cache and remove it from LRU list.
            bool removed = mDescriptorSetCache.eraseDescriptorSet(it->sharedCacheKey->getDesc());
            ASSERT(removed);

            // Note that erase it from LRU list will "destroy" descriptorSet. Since we
            // never actually destroy descriptorSet, it will just add to the garbage list. Here we
            // want more explicit control to add it to the front of list (because we know it is
            // already GPU completed) instead to the end of the list, so we do it explicitly.
            DescriptorPoolWeakPointer pool = descriptorSet->getPool();
            pool->addFinishedGarbage(std::move(descriptorSet));
            descriptorSetEvicted++;

            // This should destroy descriptorSet, which is already invalid;
            it = decltype(it)(mLRUList.erase(std::next(it).base()));
            mCacheStats.decrementSize();
        }
        else
        {
            // It means it is still bound to one of the programs. Move it to the front of the LRU
            // list to avoid repeatedly hitting it for every eviction.
            mLRUList.splice(mLRUList.begin(), mLRUList, std::next(it).base());
            ++it;
            // Update to currentFrame to maintain LRU order
            descriptorSet->updateLastUsedFrame(currentFrame);
        }
    }

    if (descriptorSetEvicted > 0)
    {
        // If there is any pool that is completely empty, destroy it first so that we can allocate
        // from partial pool.
        checkAndDestroyUnusedPool(renderer);
        return true;
    }

    return false;
}

angle::Result DynamicDescriptorPool::allocateDescriptorSet(
    ErrorContext *context,
    const DescriptorSetLayout &descriptorSetLayout,
    DescriptorSetPointer *descriptorSetOut)
{
    ASSERT(!mDescriptorPools.empty());
    ASSERT(descriptorSetLayout.getHandle() == mCachedDescriptorSetLayout);

    if (allocateFromExistingPool(context, descriptorSetLayout, descriptorSetOut))
    {
        return angle::Result::Continue;
    }

    if (recycleFromGarbage(context->getRenderer(), descriptorSetOut))
    {
        return angle::Result::Continue;
    }

    // Last, try to allocate a new pool (and/or evict an existing pool)
    ANGLE_TRY(allocateNewPool(context));
    bool success = allocateFromExistingPool(context, descriptorSetLayout, descriptorSetOut);
    // Allocate from a new pool must succeed.
    ASSERT(success);

    return angle::Result::Continue;
}

angle::Result DynamicDescriptorPool::getOrAllocateDescriptorSet(
    Context *context,
    uint32_t currentFrame,
    const DescriptorSetDesc &desc,
    const DescriptorSetLayout &descriptorSetLayout,
    DescriptorSetPointer *descriptorSetOut,
    SharedDescriptorSetCacheKey *newSharedCacheKeyOut)
{
    Renderer *renderer = context->getRenderer();
    ASSERT(context->getFeatures().descriptorSetCache.enabled);
    bool success;

    // First scan the descriptorSet cache.
    DescriptorSetLRUListIterator listIterator;
    if (mDescriptorSetCache.getDescriptorSet(desc, &listIterator))
    {
        *descriptorSetOut = listIterator->descriptorSet;
        (*newSharedCacheKeyOut).reset();
        // Move it to the front of the LRU list.
        mLRUList.splice(mLRUList.begin(), mLRUList, listIterator);
        mCacheStats.hit();
        return angle::Result::Continue;
    }

    // Try to allocate from the existing pool (or recycle from garbage list)
    success = allocateFromExistingPool(context, descriptorSetLayout, descriptorSetOut);

    // Try to recycle from the garbage list.
    if (!success)
    {
        success = recycleFromGarbage(context->getRenderer(), descriptorSetOut);
    }

    // Try to evict oldest descriptorSets that has not being used in last
    // kDescriptorSetCacheRetireAge.
    if (!success && currentFrame > kDescriptorSetCacheRetireAge)
    {
        uint32_t oldestFrameToKeep = currentFrame - kDescriptorSetCacheRetireAge;
        if (evictStaleDescriptorSets(renderer, oldestFrameToKeep, currentFrame))
        {
            success = recycleFromGarbage(renderer, descriptorSetOut);
        }
    }

    // Last, try to allocate a new pool
    if (!success)
    {
        ANGLE_TRY(allocateNewPool(context));
        success = allocateFromExistingPool(context, descriptorSetLayout, descriptorSetOut);
        // Allocate from a new pool must succeed.
        ASSERT(success);
    }

    ASSERT(descriptorSetOut->unique());
    ASSERT((*descriptorSetOut)->valid());

    // Let pool know there is a shared cache key created and destroys the shared cache key
    // when it destroys the pool.
    SharedDescriptorSetCacheKey sharedCacheKey = CreateSharedDescriptorSetCacheKey(desc, this);

    // Add to the front of the LRU list and add list iterator to the cache
    mLRUList.push_front({sharedCacheKey, *descriptorSetOut});
    mDescriptorSetCache.insertDescriptorSet(desc, mLRUList.begin());
    mCacheStats.missAndIncrementSize();

    *newSharedCacheKeyOut = sharedCacheKey;
    return angle::Result::Continue;
}

angle::Result DynamicDescriptorPool::allocateNewPool(ErrorContext *context)
{
    static constexpr size_t kMaxPools = 99999;
    ANGLE_VK_CHECK(context, mDescriptorPools.size() < kMaxPools, VK_ERROR_TOO_MANY_OBJECTS);
    // This pool is getting hot, so grow its max size to try and prevent allocating another pool in
    // the future.
    if (mMaxSetsPerPool < kMaxSetsPerPoolMax)
    {
        mMaxSetsPerPool *= mMaxSetsPerPoolMultiplier;
    }
    DescriptorPoolPointer newPool = DescriptorPoolPointer::MakeShared(context->getDevice());
    ANGLE_TRY(newPool->init(context, mPoolSizes, mMaxSetsPerPool));
    mDescriptorPools.emplace_back(std::move(newPool));

    return angle::Result::Continue;
}

void DynamicDescriptorPool::releaseCachedDescriptorSet(Renderer *renderer,
                                                       const DescriptorSetDesc &desc)
{
    ASSERT(renderer->getFeatures().descriptorSetCache.enabled);
    DescriptorSetLRUListIterator listIter;
    // Remove from the cache hash map. Note that we can't delete it until refcount goes to 0
    if (mDescriptorSetCache.eraseDescriptorSet(desc, &listIter))
    {
        DescriptorSetPointer descriptorSet = std::move(listIter->descriptorSet);
        mCacheStats.decrementSize();
        mLRUList.erase(listIter);

        if (descriptorSet.unique())
        {
            DescriptorPoolWeakPointer pool = descriptorSet->getPool();
            pool->addPendingGarbage(std::move(descriptorSet));
        }
    }
}

void DynamicDescriptorPool::destroyCachedDescriptorSet(Renderer *renderer,
                                                       const DescriptorSetDesc &desc)
{
    ASSERT(renderer->getFeatures().descriptorSetCache.enabled);
    DescriptorSetLRUListIterator listIter;
    // Remove from the cache hash map. Note that we can't delete it until refcount goes to 0
    if (mDescriptorSetCache.eraseDescriptorSet(desc, &listIter))
    {
        DescriptorSetPointer descriptorSet = std::move(listIter->descriptorSet);
        mCacheStats.decrementSize();
        mLRUList.erase(listIter);

        if (descriptorSet.unique())
        {
            DescriptorPoolWeakPointer pool = descriptorSet->getPool();
            pool->addFinishedGarbage(std::move(descriptorSet));
            if (pool->canDestroy())
            {
                destroyUnusedPool(renderer, pool);
            }
        }
    }
}

void DynamicDescriptorPool::destroyUnusedPool(Renderer *renderer,
                                              const DescriptorPoolWeakPointer &pool)
{
    ASSERT(renderer->getFeatures().descriptorSetCache.enabled);
    ASSERT(pool->canDestroy());

    // We always keep at least one pool around.
    if (mDescriptorPools.size() < 2)
    {
        return;
    }

    // Erase it from the array
    for (auto it = mDescriptorPools.begin(); it != mDescriptorPools.end(); ++it)
    {
        if (pool.owner_equal(*it))
        {
            ASSERT(pool->valid());
            pool->destroyGarbage();
            ASSERT((*it).unique());
            it = mDescriptorPools.erase(it);
            return;
        }
    }
}

void DynamicDescriptorPool::checkAndDestroyUnusedPool(Renderer *renderer)
{
    ASSERT(renderer->getFeatures().descriptorSetCache.enabled);
    for (auto pool : mDescriptorPools)
    {
        pool->cleanupPendingGarbage();
    }

    // We always keep at least one pool around.
    if (mDescriptorPools.size() < 2)
    {
        return;
    }

    // Erase it from the array
    for (auto it = mDescriptorPools.begin(); it != mDescriptorPools.end();)
    {
        if ((*it)->canDestroy())
        {
            (*it)->destroyGarbage();
            ASSERT((*it).unique());
            it = mDescriptorPools.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// For testing only!
uint32_t DynamicDescriptorPool::GetMaxSetsPerPoolForTesting()
{
    return mMaxSetsPerPool;
}

// For testing only!
void DynamicDescriptorPool::SetMaxSetsPerPoolForTesting(uint32_t maxSetsPerPool)
{
    mMaxSetsPerPool = maxSetsPerPool;
}

// For testing only!
uint32_t DynamicDescriptorPool::GetMaxSetsPerPoolMultiplierForTesting()
{
    return mMaxSetsPerPoolMultiplier;
}

// For testing only!
void DynamicDescriptorPool::SetMaxSetsPerPoolMultiplierForTesting(uint32_t maxSetsPerPoolMultiplier)
{
    mMaxSetsPerPoolMultiplier = maxSetsPerPoolMultiplier;
}

// DynamicallyGrowingPool implementation
template <typename Pool>
DynamicallyGrowingPool<Pool>::DynamicallyGrowingPool()
    : mPoolSize(0), mCurrentPool(0), mCurrentFreeEntry(0)
{}

template <typename Pool>
DynamicallyGrowingPool<Pool>::~DynamicallyGrowingPool() = default;

template <typename Pool>
angle::Result DynamicallyGrowingPool<Pool>::initEntryPool(ErrorContext *contextVk,
                                                          uint32_t poolSize)
{
    ASSERT(mPools.empty());
    mPoolSize         = poolSize;
    mCurrentFreeEntry = poolSize;
    return angle::Result::Continue;
}

template <typename Pool>
void DynamicallyGrowingPool<Pool>::destroyEntryPool(VkDevice device)
{
    for (PoolResource &resource : mPools)
    {
        destroyPoolImpl(device, resource.pool);
    }
    mPools.clear();
}

template <typename Pool>
bool DynamicallyGrowingPool<Pool>::findFreeEntryPool(ContextVk *contextVk)
{
    Renderer *renderer = contextVk->getRenderer();
    for (size_t poolIndex = 0; poolIndex < mPools.size(); ++poolIndex)
    {
        PoolResource &pool = mPools[poolIndex];
        if (pool.freedCount == mPoolSize && renderer->hasResourceUseFinished(pool.getResourceUse()))
        {
            mCurrentPool      = poolIndex;
            mCurrentFreeEntry = 0;

            pool.freedCount = 0;

            return true;
        }
    }

    return false;
}

template <typename Pool>
angle::Result DynamicallyGrowingPool<Pool>::allocateNewEntryPool(ContextVk *contextVk, Pool &&pool)
{
    mPools.emplace_back(std::move(pool), 0);

    mCurrentPool      = mPools.size() - 1;
    mCurrentFreeEntry = 0;

    return angle::Result::Continue;
}

template <typename Pool>
void DynamicallyGrowingPool<Pool>::onEntryFreed(ContextVk *contextVk,
                                                size_t poolIndex,
                                                const ResourceUse &use)
{
    ASSERT(poolIndex < mPools.size() && mPools[poolIndex].freedCount < mPoolSize);
    if (!contextVk->getRenderer()->hasResourceUseFinished(use))
    {
        mPools[poolIndex].mergeResourceUse(use);
    }
    ++mPools[poolIndex].freedCount;
}

template <typename Pool>
angle::Result DynamicallyGrowingPool<Pool>::allocatePoolEntries(ContextVk *contextVk,
                                                                uint32_t entryCount,
                                                                uint32_t *poolIndex,
                                                                uint32_t *currentEntryOut)
{
    if (mCurrentFreeEntry + entryCount > mPoolSize)
    {
        if (!findFreeEntryPool(contextVk))
        {
            Pool newPool;
            ANGLE_TRY(allocatePoolImpl(contextVk, newPool, mPoolSize));
            ANGLE_TRY(allocateNewEntryPool(contextVk, std::move(newPool)));
        }
    }

    *poolIndex       = static_cast<uint32_t>(mCurrentPool);
    *currentEntryOut = mCurrentFreeEntry;

    mCurrentFreeEntry += entryCount;

    return angle::Result::Continue;
}

template <typename Pool>
DynamicallyGrowingPool<Pool>::PoolResource::PoolResource(Pool &&poolIn, uint32_t freedCountIn)
    : pool(std::move(poolIn)), freedCount(freedCountIn)
{}

template <typename Pool>
DynamicallyGrowingPool<Pool>::PoolResource::PoolResource(PoolResource &&other)
    : Resource(std::move(other)), pool(std::move(other.pool)), freedCount(other.freedCount)
{}

// DynamicQueryPool implementation
DynamicQueryPool::DynamicQueryPool() = default;

DynamicQueryPool::~DynamicQueryPool() = default;

angle::Result DynamicQueryPool::init(ContextVk *contextVk, VkQueryType type, uint32_t poolSize)
{
    // SecondaryCommandBuffer's ResetQueryPoolParams would like the query index to fit in 24 bits.
    ASSERT(poolSize < (1 << 24));

    ANGLE_TRY(initEntryPool(contextVk, poolSize));
    mQueryType = type;
    return angle::Result::Continue;
}

void DynamicQueryPool::destroy(VkDevice device)
{
    destroyEntryPool(device);
}

void DynamicQueryPool::destroyPoolImpl(VkDevice device, QueryPool &poolToDestroy)
{
    poolToDestroy.destroy(device);
}

angle::Result DynamicQueryPool::allocateQuery(ContextVk *contextVk,
                                              QueryHelper *queryOut,
                                              uint32_t queryCount)
{
    ASSERT(!queryOut->valid());

    uint32_t currentPool = 0;
    uint32_t queryIndex  = 0;
    ANGLE_TRY(allocatePoolEntries(contextVk, queryCount, &currentPool, &queryIndex));

    queryOut->init(this, currentPool, queryIndex, queryCount);

    return angle::Result::Continue;
}

angle::Result DynamicQueryPool::allocatePoolImpl(ContextVk *contextVk,
                                                 QueryPool &poolToAllocate,
                                                 uint32_t entriesToAllocate)
{
    VkQueryPoolCreateInfo queryPoolInfo = {};
    queryPoolInfo.sType                 = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    queryPoolInfo.flags                 = 0;
    queryPoolInfo.queryType             = this->mQueryType;
    queryPoolInfo.queryCount            = entriesToAllocate;
    queryPoolInfo.pipelineStatistics    = 0;

    if (this->mQueryType == VK_QUERY_TYPE_PIPELINE_STATISTICS)
    {
        queryPoolInfo.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT;
    }

    ANGLE_VK_TRY(contextVk, poolToAllocate.init(contextVk->getDevice(), queryPoolInfo));
    return angle::Result::Continue;
}

void DynamicQueryPool::freeQuery(ContextVk *contextVk, QueryHelper *query)
{
    if (query->valid())
    {
        size_t poolIndex = query->mQueryPoolIndex;
        ASSERT(getQueryPool(poolIndex).valid());

        onEntryFreed(contextVk, poolIndex, query->getResourceUse());

        query->deinit();
    }
}

// QueryResult implementation
void QueryResult::setResults(uint64_t *results, uint32_t queryCount)
{
    ASSERT(mResults[0] == 0 && mResults[1] == 0);

    // Accumulate the query results.  For multiview, where multiple query indices are used to return
    // the results, it's undefined how the results are distributed between indices, but the sum is
    // guaranteed to be the desired result.
    for (uint32_t query = 0; query < queryCount; ++query)
    {
        for (uint32_t perQueryIndex = 0; perQueryIndex < mIntsPerResult; ++perQueryIndex)
        {
            mResults[perQueryIndex] += results[query * mIntsPerResult + perQueryIndex];
        }
    }
}

// QueryHelper implementation
QueryHelper::QueryHelper()
    : mDynamicQueryPool(nullptr),
      mQueryPoolIndex(0),
      mQuery(0),
      mQueryCount(0),
      mStatus(QueryStatus::Inactive)
{}

QueryHelper::~QueryHelper() {}

// Move constructor
QueryHelper::QueryHelper(QueryHelper &&rhs)
    : Resource(std::move(rhs)),
      mDynamicQueryPool(rhs.mDynamicQueryPool),
      mQueryPoolIndex(rhs.mQueryPoolIndex),
      mQuery(rhs.mQuery),
      mQueryCount(rhs.mQueryCount),
      mStatus(rhs.mStatus)
{
    rhs.mDynamicQueryPool = nullptr;
    rhs.mQueryPoolIndex   = 0;
    rhs.mQuery            = 0;
    rhs.mQueryCount       = 0;
    rhs.mStatus           = QueryStatus::Inactive;
}

QueryHelper &QueryHelper::operator=(QueryHelper &&rhs)
{
    Resource::operator=(std::move(rhs));
    std::swap(mDynamicQueryPool, rhs.mDynamicQueryPool);
    std::swap(mQueryPoolIndex, rhs.mQueryPoolIndex);
    std::swap(mQuery, rhs.mQuery);
    std::swap(mQueryCount, rhs.mQueryCount);
    std::swap(mStatus, rhs.mStatus);
    return *this;
}

void QueryHelper::init(const DynamicQueryPool *dynamicQueryPool,
                       const size_t queryPoolIndex,
                       uint32_t query,
                       uint32_t queryCount)
{
    mDynamicQueryPool = dynamicQueryPool;
    mQueryPoolIndex   = queryPoolIndex;
    mQuery            = query;
    mQueryCount       = queryCount;

    ASSERT(mQueryCount <= gl::IMPLEMENTATION_ANGLE_MULTIVIEW_MAX_VIEWS);
}

void QueryHelper::deinit()
{
    mDynamicQueryPool = nullptr;
    mQueryPoolIndex   = 0;
    mQuery            = 0;
    mQueryCount       = 0;
    mUse.reset();
    mStatus = QueryStatus::Inactive;
}

template <typename CommandBufferT>
void QueryHelper::beginQueryImpl(ContextVk *contextVk,
                                 OutsideRenderPassCommandBuffer *resetCommandBuffer,
                                 CommandBufferT *commandBuffer)
{
    ASSERT(mStatus != QueryStatus::Active);
    const QueryPool &queryPool = getQueryPool();
    resetQueryPoolImpl(contextVk, queryPool, resetCommandBuffer);
    commandBuffer->beginQuery(queryPool, mQuery, 0);
    mStatus = QueryStatus::Active;
}

template <typename CommandBufferT>
void QueryHelper::endQueryImpl(ContextVk *contextVk, CommandBufferT *commandBuffer)
{
    ASSERT(mStatus != QueryStatus::Ended);
    commandBuffer->endQuery(getQueryPool(), mQuery);
    mStatus = QueryStatus::Ended;
}

angle::Result QueryHelper::beginQuery(ContextVk *contextVk)
{
    if (contextVk->hasActiveRenderPass())
    {
        ANGLE_TRY(contextVk->flushCommandsAndEndRenderPass(
            RenderPassClosureReason::BeginNonRenderPassQuery));
    }

    OutsideRenderPassCommandBuffer *commandBuffer;
    ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer({}, &commandBuffer));

    ANGLE_TRY(contextVk->handleGraphicsEventLog(rx::GraphicsEventCmdBuf::InOutsideCmdBufQueryCmd));

    beginQueryImpl(contextVk, commandBuffer, commandBuffer);

    return angle::Result::Continue;
}

angle::Result QueryHelper::endQuery(ContextVk *contextVk)
{
    if (contextVk->hasActiveRenderPass())
    {
        ANGLE_TRY(contextVk->flushCommandsAndEndRenderPass(
            RenderPassClosureReason::EndNonRenderPassQuery));
    }

    CommandBufferAccess access;
    OutsideRenderPassCommandBuffer *commandBuffer;
    access.onQueryAccess(this);
    ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

    ANGLE_TRY(contextVk->handleGraphicsEventLog(rx::GraphicsEventCmdBuf::InOutsideCmdBufQueryCmd));

    endQueryImpl(contextVk, commandBuffer);

    return angle::Result::Continue;
}

template <typename CommandBufferT>
void QueryHelper::resetQueryPoolImpl(ContextVk *contextVk,
                                     const QueryPool &queryPool,
                                     CommandBufferT *commandBuffer)
{
    Renderer *renderer = contextVk->getRenderer();
    if (renderer->getFeatures().supportsHostQueryReset.enabled)
    {
        vkResetQueryPoolEXT(contextVk->getDevice(), queryPool.getHandle(), mQuery, mQueryCount);
    }
    else
    {
        commandBuffer->resetQueryPool(queryPool, mQuery, mQueryCount);
    }
}

angle::Result QueryHelper::beginRenderPassQuery(ContextVk *contextVk)
{
    OutsideRenderPassCommandBuffer *outsideRenderPassCommandBuffer;
    ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer({}, &outsideRenderPassCommandBuffer));

    RenderPassCommandBuffer *renderPassCommandBuffer =
        &contextVk->getStartedRenderPassCommands().getCommandBuffer();

    beginQueryImpl(contextVk, outsideRenderPassCommandBuffer, renderPassCommandBuffer);

    return angle::Result::Continue;
}

void QueryHelper::endRenderPassQuery(ContextVk *contextVk)
{
    if (mStatus == QueryStatus::Active)
    {
        endQueryImpl(contextVk, &contextVk->getStartedRenderPassCommands().getCommandBuffer());
        contextVk->getStartedRenderPassCommands().retainResource(this);
    }
}

angle::Result QueryHelper::flushAndWriteTimestamp(ContextVk *contextVk)
{
    if (contextVk->hasActiveRenderPass())
    {
        ANGLE_TRY(
            contextVk->flushCommandsAndEndRenderPass(RenderPassClosureReason::TimestampQuery));
    }

    CommandBufferAccess access;
    OutsideRenderPassCommandBuffer *commandBuffer;
    access.onQueryAccess(this);
    ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));
    writeTimestamp(contextVk, commandBuffer);
    return angle::Result::Continue;
}

void QueryHelper::writeTimestampToPrimary(ContextVk *contextVk, PrimaryCommandBuffer *primary)
{
    // Note that commands may not be flushed at this point.

    const QueryPool &queryPool = getQueryPool();
    resetQueryPoolImpl(contextVk, queryPool, primary);
    primary->writeTimestamp(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, queryPool, mQuery);
}

void QueryHelper::writeTimestamp(ContextVk *contextVk,
                                 OutsideRenderPassCommandBuffer *commandBuffer)
{
    const QueryPool &queryPool = getQueryPool();
    resetQueryPoolImpl(contextVk, queryPool, commandBuffer);
    commandBuffer->writeTimestamp(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, queryPool, mQuery);
}

bool QueryHelper::hasSubmittedCommands() const
{
    return mUse.valid();
}

angle::Result QueryHelper::getUint64ResultNonBlocking(ContextVk *contextVk,
                                                      QueryResult *resultOut,
                                                      bool *availableOut)
{
    ASSERT(valid());
    VkResult result;

    // Ensure that we only wait if we have inserted a query in command buffer. Otherwise you will
    // wait forever and trigger GPU timeout.
    if (hasSubmittedCommands())
    {
        constexpr VkQueryResultFlags kFlags = VK_QUERY_RESULT_64_BIT;
        result                              = getResultImpl(contextVk, kFlags, resultOut);
    }
    else
    {
        result     = VK_SUCCESS;
        *resultOut = 0;
    }

    if (result == VK_NOT_READY)
    {
        *availableOut = false;
        return angle::Result::Continue;
    }
    else
    {
        ANGLE_VK_TRY(contextVk, result);
        *availableOut = true;
    }
    return angle::Result::Continue;
}

angle::Result QueryHelper::getUint64Result(ContextVk *contextVk, QueryResult *resultOut)
{
    ASSERT(valid());
    if (hasSubmittedCommands())
    {
        constexpr VkQueryResultFlags kFlags = VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT;
        ANGLE_VK_TRY(contextVk, getResultImpl(contextVk, kFlags, resultOut));
    }
    else
    {
        *resultOut = 0;
    }
    return angle::Result::Continue;
}

VkResult QueryHelper::getResultImpl(ContextVk *contextVk,
                                    const VkQueryResultFlags flags,
                                    QueryResult *resultOut)
{
    std::array<uint64_t, 2 * gl::IMPLEMENTATION_ANGLE_MULTIVIEW_MAX_VIEWS> results;

    VkDevice device = contextVk->getDevice();
    VkResult result = getQueryPool().getResults(device, mQuery, mQueryCount, sizeof(results),
                                                results.data(), sizeof(uint64_t), flags);

    if (result == VK_SUCCESS)
    {
        resultOut->setResults(results.data(), mQueryCount);
    }

    return result;
}

// SemaphoreHelper implementation
SemaphoreHelper::SemaphoreHelper() : mSemaphorePoolIndex(0), mSemaphore(0) {}

SemaphoreHelper::~SemaphoreHelper() {}

SemaphoreHelper::SemaphoreHelper(SemaphoreHelper &&other)
    : mSemaphorePoolIndex(other.mSemaphorePoolIndex), mSemaphore(other.mSemaphore)
{
    other.mSemaphore = nullptr;
}

SemaphoreHelper &SemaphoreHelper::operator=(SemaphoreHelper &&other)
{
    std::swap(mSemaphorePoolIndex, other.mSemaphorePoolIndex);
    std::swap(mSemaphore, other.mSemaphore);
    return *this;
}

void SemaphoreHelper::init(const size_t semaphorePoolIndex, const Semaphore *semaphore)
{
    mSemaphorePoolIndex = semaphorePoolIndex;
    mSemaphore          = semaphore;
}

void SemaphoreHelper::deinit()
{
    mSemaphorePoolIndex = 0;
    mSemaphore          = nullptr;
}

PipelineStage GetPipelineStage(gl::ShaderType stage)
{
    const PipelineStage pipelineStage = kPipelineStageShaderMap[stage];
    ASSERT(pipelineStage == PipelineStage::VertexShader ||
           pipelineStage == PipelineStage::TessellationControl ||
           pipelineStage == PipelineStage::TessellationEvaluation ||
           pipelineStage == PipelineStage::GeometryShader ||
           pipelineStage == PipelineStage::FragmentShader ||
           pipelineStage == PipelineStage::ComputeShader);
    return pipelineStage;
}

// PipelineBarrier implementation.
void PipelineBarrier::addDiagnosticsString(std::ostringstream &out) const
{
    if (mMemoryBarrierSrcAccess != 0 || mMemoryBarrierDstAccess != 0)
    {
        out << "Src: 0x" << std::hex << mMemoryBarrierSrcAccess << " &rarr; Dst: 0x" << std::hex
            << mMemoryBarrierDstAccess << std::endl;
    }
}

// PipelineBarrierArray implementation.
void PipelineBarrierArray::execute(Renderer *renderer, PrimaryCommandBuffer *primary)
{
    // make a local copy for faster access
    PipelineStagesMask mask = mBarrierMask;
    if (mask.none())
    {
        return;
    }

    if (renderer->getFeatures().preferAggregateBarrierCalls.enabled)
    {
        PipelineStagesMask::Iterator iter = mask.begin();
        PipelineBarrier &barrier          = mBarriers[*iter];
        for (++iter; iter != mask.end(); ++iter)
        {
            barrier.merge(&mBarriers[*iter]);
        }
        barrier.execute(primary);
    }
    else
    {
        for (PipelineStage pipelineStage : mask)
        {
            PipelineBarrier &barrier = mBarriers[pipelineStage];
            barrier.execute(primary);
        }
    }
    mBarrierMask.reset();
}

void PipelineBarrierArray::addDiagnosticsString(std::ostringstream &out) const
{
    out << "Memory Barrier: ";
    for (PipelineStage pipelineStage : mBarrierMask)
    {
        const PipelineBarrier &barrier = mBarriers[pipelineStage];
        if (!barrier.isEmpty())
        {
            barrier.addDiagnosticsString(out);
        }
    }
    out << "\\l";
}

// BufferHelper implementation.
BufferHelper::BufferHelper()
    : mCurrentWriteAccess(0),
      mCurrentReadAccess(0),
      mCurrentWriteStages(0),
      mCurrentReadStages(0),
      mSerial(),
      mClientBuffer(nullptr),
      mIsReleasedToExternal(false)
{}

BufferHelper::~BufferHelper()
{
    // We must have released external buffer properly
    ASSERT(mClientBuffer == nullptr);
}

BufferHelper::BufferHelper(BufferHelper &&other)
{
    *this = std::move(other);
}

BufferHelper &BufferHelper::operator=(BufferHelper &&other)
{
    ReadWriteResource::operator=(std::move(other));

    mSuballocation      = std::move(other.mSuballocation);
    mBufferWithUserSize = std::move(other.mBufferWithUserSize);

    mCurrentDeviceQueueIndex = other.mCurrentDeviceQueueIndex;
    mIsReleasedToExternal    = other.mIsReleasedToExternal;
    mCurrentWriteAccess      = other.mCurrentWriteAccess;
    mCurrentReadAccess       = other.mCurrentReadAccess;
    mCurrentWriteStages      = other.mCurrentWriteStages;
    mCurrentReadStages       = other.mCurrentReadStages;
    if (other.mCurrentWriteEvent.valid())
    {
        mCurrentWriteEvent = std::move(other.mCurrentWriteEvent);
    }
    if (!other.mCurrentReadEvents.empty())
    {
        mCurrentReadEvents = std::move(other.mCurrentReadEvents);
    }
    mTransformFeedbackWriteHeuristicBits = std::move(other.mTransformFeedbackWriteHeuristicBits);
    mSerial                  = other.mSerial;
    mClientBuffer            = std::move(other.mClientBuffer);

    return *this;
}

angle::Result BufferHelper::init(ErrorContext *context,
                                 const VkBufferCreateInfo &requestedCreateInfo,
                                 VkMemoryPropertyFlags memoryPropertyFlags)
{
    Renderer *renderer         = context->getRenderer();
    const Allocator &allocator = renderer->getAllocator();

    initializeBarrierTracker(context);

    VkBufferCreateInfo modifiedCreateInfo;
    const VkBufferCreateInfo *createInfo = &requestedCreateInfo;

    if (renderer->getFeatures().padBuffersToMaxVertexAttribStride.enabled)
    {
        const VkDeviceSize maxVertexAttribStride = renderer->getMaxVertexAttribStride();
        ASSERT(maxVertexAttribStride);
        modifiedCreateInfo = requestedCreateInfo;
        modifiedCreateInfo.size += maxVertexAttribStride;
        createInfo = &modifiedCreateInfo;
    }

    VkMemoryPropertyFlags requiredFlags =
        (memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    VkMemoryPropertyFlags preferredFlags =
        (memoryPropertyFlags & (~VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

    bool persistentlyMapped = renderer->getFeatures().persistentlyMappedBuffers.enabled;

    // Check that the allocation is not too large.
    uint32_t memoryTypeIndex = kInvalidMemoryTypeIndex;
    ANGLE_VK_TRY(context, allocator.findMemoryTypeIndexForBufferInfo(
                              *createInfo, requiredFlags, preferredFlags, persistentlyMapped,
                              &memoryTypeIndex));

    VkDeviceSize heapSize =
        renderer->getMemoryProperties().getHeapSizeForMemoryType(memoryTypeIndex);

    ANGLE_VK_CHECK(context, createInfo->size <= heapSize, VK_ERROR_OUT_OF_DEVICE_MEMORY);

    VkMemoryPropertyFlags memoryPropertyFlagsOut;
    allocator.getMemoryTypeProperties(memoryTypeIndex, &memoryPropertyFlagsOut);
    // Allocate buffer object
    DeviceScoped<Buffer> buffer(renderer->getDevice());
    ANGLE_VK_TRY(context, buffer.get().init(context->getDevice(), *createInfo));

    DeviceScoped<DeviceMemory> deviceMemory(renderer->getDevice());
    VkDeviceSize sizeOut;
    uint32_t bufferMemoryTypeIndex;
    ANGLE_VK_TRY(context,
                 AllocateBufferMemory(context, MemoryAllocationType::Buffer, memoryPropertyFlagsOut,
                                      &memoryPropertyFlagsOut, nullptr, &buffer.get(),
                                      &bufferMemoryTypeIndex, &deviceMemory.get(), &sizeOut));
    ASSERT(sizeOut >= createInfo->size);

    mSuballocation.initWithEntireBuffer(context, buffer.get(), MemoryAllocationType::Buffer,
                                        bufferMemoryTypeIndex, deviceMemory.get(),
                                        memoryPropertyFlagsOut, requestedCreateInfo.size, sizeOut);
    if (isHostVisible())
    {
        uint8_t *ptrOut;
        ANGLE_TRY(map(context, &ptrOut));
    }

    if (renderer->getFeatures().allocateNonZeroMemory.enabled)
    {
        ANGLE_TRY(initializeNonZeroMemory(context, createInfo->usage, createInfo->size));
    }

    return angle::Result::Continue;
}

angle::Result BufferHelper::initExternal(ErrorContext *context,
                                         VkMemoryPropertyFlags memoryProperties,
                                         const VkBufferCreateInfo &requestedCreateInfo,
                                         GLeglClientBufferEXT clientBuffer)
{
    ASSERT(IsAndroid());

    Renderer *renderer = context->getRenderer();

    initializeBarrierTracker(context);

    VkBufferCreateInfo modifiedCreateInfo             = requestedCreateInfo;
    VkExternalMemoryBufferCreateInfo externCreateInfo = {};
    externCreateInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO;
    externCreateInfo.handleTypes =
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    externCreateInfo.pNext   = nullptr;
    modifiedCreateInfo.pNext = &externCreateInfo;

    DeviceScoped<Buffer> buffer(renderer->getDevice());
    ANGLE_VK_TRY(context, buffer.get().init(renderer->getDevice(), modifiedCreateInfo));

    DeviceScoped<DeviceMemory> deviceMemory(renderer->getDevice());
    VkMemoryPropertyFlags memoryPropertyFlagsOut;
    VkDeviceSize allocatedSize = 0;
    uint32_t memoryTypeIndex;
    ANGLE_TRY(InitAndroidExternalMemory(context, clientBuffer, memoryProperties, &buffer.get(),
                                        &memoryPropertyFlagsOut, &memoryTypeIndex,
                                        &deviceMemory.get(), &allocatedSize));
    mClientBuffer = clientBuffer;

    mSuballocation.initWithEntireBuffer(context, buffer.get(), MemoryAllocationType::BufferExternal,
                                        memoryTypeIndex, deviceMemory.get(), memoryPropertyFlagsOut,
                                        requestedCreateInfo.size, allocatedSize);
    if (isHostVisible())
    {
        uint8_t *ptrOut;
        ANGLE_TRY(map(context, &ptrOut));
    }
    return angle::Result::Continue;
}

VkResult BufferHelper::initSuballocation(Context *context,
                                         uint32_t memoryTypeIndex,
                                         size_t size,
                                         size_t alignment,
                                         BufferUsageType usageType,
                                         BufferPool *pool)
{
    ASSERT(pool != nullptr);
    Renderer *renderer = context->getRenderer();

    // We should reset these in case the BufferHelper object has been released and called
    // initSuballocation again.
    initializeBarrierTracker(context);

    if (renderer->getFeatures().padBuffersToMaxVertexAttribStride.enabled)
    {
        const VkDeviceSize maxVertexAttribStride = renderer->getMaxVertexAttribStride();
        ASSERT(maxVertexAttribStride);
        size += maxVertexAttribStride;
    }

    VK_RESULT_TRY(pool->allocateBuffer(context, size, alignment, &mSuballocation));

    context->getPerfCounters().bufferSuballocationCalls++;

    return VK_SUCCESS;
}

void BufferHelper::initializeBarrierTracker(ErrorContext *context)
{
    Renderer *renderer       = context->getRenderer();
    mCurrentDeviceQueueIndex = context->getDeviceQueueIndex();
    mIsReleasedToExternal    = false;
    mCurrentWriteEvent.release(renderer);
    mCurrentReadEvents.release(renderer);
    mSerial                  = renderer->getResourceSerialFactory().generateBufferSerial();
    mCurrentWriteAccess      = 0;
    mCurrentReadAccess       = 0;
    mCurrentWriteStages      = 0;
    mCurrentReadStages       = 0;
}

angle::Result BufferHelper::initializeNonZeroMemory(ErrorContext *context,
                                                    VkBufferUsageFlags usage,
                                                    VkDeviceSize size)
{
    Renderer *renderer = context->getRenderer();

    // This memory can't be mapped, so the buffer must be marked as a transfer destination so we
    // can use a staging resource to initialize it to a non-zero value. If the memory is
    // mappable we do the initialization in AllocateBufferMemory.
    if (!isHostVisible() && (usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) != 0)
    {
        ASSERT((usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) != 0);
        // Staging buffer memory is non-zero-initialized in 'init'.
        StagingBuffer stagingBuffer;
        ANGLE_TRY(stagingBuffer.init(context, size, StagingUsage::Both));

        // Queue a DMA copy.
        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset    = 0;
        copyRegion.dstOffset    = getOffset();
        copyRegion.size         = size;

        ScopedPrimaryCommandBuffer scopedCommandBuffer(renderer->getDevice());
        ANGLE_TRY(renderer->getCommandBufferOneOff(context, ProtectionType::Unprotected,
                                                   &scopedCommandBuffer));
        PrimaryCommandBuffer &commandBuffer = scopedCommandBuffer.get();

        commandBuffer.copyBuffer(stagingBuffer.getBuffer(), getBuffer(), 1, &copyRegion);

        ANGLE_VK_TRY(context, commandBuffer.end());

        QueueSerial queueSerial;
        ANGLE_TRY(renderer->queueSubmitOneOff(
            context, std::move(scopedCommandBuffer), ProtectionType::Unprotected,
            egl::ContextPriority::Medium, VK_NULL_HANDLE, 0, &queueSerial));

        stagingBuffer.collectGarbage(renderer, queueSerial);
        // Update both ResourceUse objects, since mReadOnlyUse tracks when the buffer can be
        // destroyed, and mReadWriteUse tracks when the write has completed.
        setWriteQueueSerial(queueSerial);
    }
    else if (isHostVisible())
    {
        // Can map the memory.
        // Pick an arbitrary value to initialize non-zero memory for sanitization.
        constexpr int kNonZeroInitValue = 55;
        uint8_t *mapPointer             = mSuballocation.getMappedMemory();
        memset(mapPointer, kNonZeroInitValue, static_cast<size_t>(getSize()));
        if (!isCoherent())
        {
            mSuballocation.flush(renderer->getDevice());
        }
    }

    return angle::Result::Continue;
}

const Buffer &BufferHelper::getBufferForVertexArray(ContextVk *contextVk,
                                                    VkDeviceSize actualDataSize,
                                                    VkDeviceSize *offsetOut)
{
    ASSERT(mSuballocation.valid());
    ASSERT(actualDataSize <= mSuballocation.getSize());

    if (!contextVk->hasRobustAccess() || !mSuballocation.isSuballocated() ||
        actualDataSize == mSuballocation.getSize())
    {
        *offsetOut = mSuballocation.getOffset();
        return mSuballocation.getBuffer();
    }

    if (!mBufferWithUserSize.valid())
    {
        // Allocate buffer that is backed by sub-range of the memory for vertex array usage. This is
        // only needed when robust resource init is enabled so that vulkan driver will know the
        // exact size of the vertex buffer it is supposedly to use and prevent out of bound access.
        VkBufferCreateInfo createInfo    = {};
        createInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.flags                 = 0;
        createInfo.size                  = actualDataSize;
        createInfo.usage                 = kVertexBufferUsageFlags | kIndexBufferUsageFlags;
        createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;
        mBufferWithUserSize.init(contextVk->getDevice(), createInfo);

        VkMemoryRequirements memoryRequirements;
        mBufferWithUserSize.getMemoryRequirements(contextVk->getDevice(), &memoryRequirements);
        ASSERT(contextVk->getRenderer()->isMockICDEnabled() ||
               mSuballocation.getSize() >= memoryRequirements.size);
        ASSERT(!contextVk->getRenderer()->isMockICDEnabled() ||
               mSuballocation.getOffset() % memoryRequirements.alignment == 0);

        mBufferWithUserSize.bindMemory(contextVk->getDevice(), mSuballocation.getDeviceMemory(),
                                       mSuballocation.getOffset());
    }
    *offsetOut = 0;
    return mBufferWithUserSize;
}

bool BufferHelper::onBufferUserSizeChange(Renderer *renderer)
{
    // Buffer's user size and allocation size may be different due to alignment requirement. In
    // normal usage we just use the actual allocation size and it is good enough. But when
    // robustResourceInit is enabled, mBufferWithUserSize is created to mjatch the exact user
    // size. Thus when user size changes, we must clear and recreate this mBufferWithUserSize.
    if (mBufferWithUserSize.valid())
    {
        BufferSuballocation unusedSuballocation;
        renderer->collectSuballocationGarbage(mUse, std::move(unusedSuballocation),
                                              std::move(mBufferWithUserSize));
        mSerial = renderer->getResourceSerialFactory().generateBufferSerial();
        return true;
    }
    return false;
}

void BufferHelper::destroy(Renderer *renderer)
{
    mCurrentWriteEvent.release(renderer);
    mCurrentReadEvents.release(renderer);
    mDescriptorSetCacheManager.destroyKeys(renderer);
    unmap(renderer);
    mBufferWithUserSize.destroy(renderer->getDevice());
    mSuballocation.destroy(renderer);
    if (mClientBuffer != nullptr)
    {
        ReleaseAndroidExternalMemory(renderer, mClientBuffer);
        mClientBuffer = nullptr;
    }
}

void BufferHelper::release(Renderer *renderer)
{
    mCurrentWriteEvent.release(renderer);
    mCurrentReadEvents.release(renderer);
    releaseImpl(renderer);
}

void BufferHelper::release(Context *context)
{
    mCurrentWriteEvent.release(context);
    mCurrentReadEvents.release(context);
    releaseImpl(context->getRenderer());
}

void BufferHelper::releaseImpl(Renderer *renderer)
{
    ASSERT(mDescriptorSetCacheManager.empty());
    unmap(renderer);

    if (mSuballocation.valid())
    {
        if (!mSuballocation.isSuballocated())
        {
            // Destroy cacheKeys now to avoid getting into situation that having to destroy
            // descriptorSet from garbage collection thread.
            mSuballocation.getBufferBlock()->releaseAllCachedDescriptorSetCacheKeys(renderer);
        }
        renderer->collectSuballocationGarbage(mUse, std::move(mSuballocation),
                                              std::move(mBufferWithUserSize));
    }
    mUse.reset();
    mWriteUse.reset();
    ASSERT(!mBufferWithUserSize.valid());

    if (mClientBuffer != nullptr)
    {
        ReleaseAndroidExternalMemory(renderer, mClientBuffer);
        mClientBuffer = nullptr;
    }
}

void BufferHelper::releaseBufferAndDescriptorSetCache(Context *context)
{
    Renderer *renderer = context->getRenderer();
    if (renderer->hasResourceUseFinished(getResourceUse()))
    {
        mDescriptorSetCacheManager.destroyKeys(renderer);
    }
    else
    {
        mDescriptorSetCacheManager.releaseKeys(renderer);
    }

    release(context);
}

angle::Result BufferHelper::map(ErrorContext *context, uint8_t **ptrOut)
{
    if (!mSuballocation.isMapped())
    {
        ANGLE_VK_TRY(context, mSuballocation.map(context));
    }
    *ptrOut = mSuballocation.getMappedMemory();
    return angle::Result::Continue;
}

angle::Result BufferHelper::mapWithOffset(ErrorContext *context, uint8_t **ptrOut, size_t offset)
{
    uint8_t *mapBufPointer;
    ANGLE_TRY(map(context, &mapBufPointer));
    *ptrOut = mapBufPointer + offset;
    return angle::Result::Continue;
}

angle::Result BufferHelper::flush(Renderer *renderer, VkDeviceSize offset, VkDeviceSize size)
{
    mSuballocation.flush(renderer->getDevice());
    return angle::Result::Continue;
}
angle::Result BufferHelper::flush(Renderer *renderer)
{
    return flush(renderer, 0, getSize());
}

angle::Result BufferHelper::invalidate(Renderer *renderer, VkDeviceSize offset, VkDeviceSize size)
{
    mSuballocation.invalidate(renderer->getDevice());
    return angle::Result::Continue;
}
angle::Result BufferHelper::invalidate(Renderer *renderer)
{
    return invalidate(renderer, 0, getSize());
}

void BufferHelper::changeQueueFamily(uint32_t srcQueueFamilyIndex,
                                     uint32_t dstQueueFamilyIndex,
                                     OutsideRenderPassCommandBuffer *commandBuffer)
{
    VkBufferMemoryBarrier bufferMemoryBarrier = {};
    bufferMemoryBarrier.sType                 = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    bufferMemoryBarrier.srcAccessMask         = 0;
    bufferMemoryBarrier.dstAccessMask         = 0;
    bufferMemoryBarrier.srcQueueFamilyIndex   = srcQueueFamilyIndex;
    bufferMemoryBarrier.dstQueueFamilyIndex   = dstQueueFamilyIndex;
    bufferMemoryBarrier.buffer                = getBuffer().getHandle();
    bufferMemoryBarrier.offset                = getOffset();
    bufferMemoryBarrier.size                  = getSize();

    commandBuffer->bufferBarrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, &bufferMemoryBarrier);
}

void BufferHelper::acquireFromExternal(DeviceQueueIndex externalQueueFamilyIndex,
                                       DeviceQueueIndex newDeviceQueueIndex,
                                       OutsideRenderPassCommandBuffer *commandBuffer)
{
    changeQueueFamily(externalQueueFamilyIndex.familyIndex(), newDeviceQueueIndex.familyIndex(),
                      commandBuffer);
    mCurrentDeviceQueueIndex = newDeviceQueueIndex;
    mIsReleasedToExternal    = false;
}

void BufferHelper::releaseToExternal(DeviceQueueIndex externalQueueIndex,
                                     OutsideRenderPassCommandBuffer *commandBuffer)
{
    if (mCurrentDeviceQueueIndex.familyIndex() != externalQueueIndex.familyIndex())
    {
        changeQueueFamily(mCurrentDeviceQueueIndex.familyIndex(), externalQueueIndex.familyIndex(),
                          commandBuffer);
        mCurrentDeviceQueueIndex = kInvalidDeviceQueueIndex;
    }
    mIsReleasedToExternal = true;
}

void BufferHelper::recordReadBarrier(Context *context,
                                     VkAccessFlags readAccessType,
                                     VkPipelineStageFlags readPipelineStageFlags,
                                     PipelineStage stageIndex,
                                     PipelineBarrierArray *pipelineBarriers,
                                     EventBarrierArray *eventBarriers,
                                     RefCountedEventCollector *eventCollector)
{
    // If the type of read already tracked by mCurrentReadEvents, it means we must already inserted
    // the barrier when mCurrentReadEvents is set. No new barrier is needed.
    EventStage eventStage = kBufferMemoryBarrierData[stageIndex].eventStage;
    if (mCurrentReadEvents.hasEventAndAccess(eventStage, readAccessType))
    {
        ASSERT((context->getRenderer()->getPipelineStageMask(eventStage) &
                readPipelineStageFlags) == readPipelineStageFlags);
        ASSERT((mCurrentReadEvents.getAccessFlags(eventStage) & readAccessType) == readAccessType);
        return;
    }

    // If the type of read already tracked by mCurrentReadAccess, it means we must already inserted
    // the barrier when mCurrentReadAccess is set. No new barrier is needed.
    if ((mCurrentReadAccess & readAccessType) == readAccessType &&
        (mCurrentReadStages & readPipelineStageFlags) == readPipelineStageFlags)
    {
        return;
    }

    // Barrier against prior write VkEvent.
    if (mCurrentWriteEvent.valid())
    {
        eventBarriers->addEventMemoryBarrier(context->getRenderer(), mCurrentWriteEvent.getEvent(),
                                             mCurrentWriteEvent.getAccessFlags(),
                                             readPipelineStageFlags, readAccessType);
    }

    // Barrier against prior access that not tracked by VkEvent using pipelineBarrier.
    if (mCurrentWriteAccess != 0)
    {
        pipelineBarriers->mergeMemoryBarrier(stageIndex, mCurrentWriteStages,
                                             readPipelineStageFlags, mCurrentWriteAccess,
                                             readAccessType);
    }
}

void BufferHelper::recordReadEvent(Context *context,
                                   VkAccessFlags readAccessType,
                                   VkPipelineStageFlags readPipelineStageFlags,
                                   PipelineStage readStage,
                                   const QueueSerial &queueSerial,
                                   EventStage eventStage,
                                   RefCountedEventArray *refCountedEventArray)
{
    bool useVkEvent = false;
    if (context->getFeatures().useVkEventForBufferBarrier.enabled &&
        eventStage != EventStage::InvalidEnum)
    {
        // VkCmdSetEvent can remove the unnecessary GPU pipeline bubble that comes from false
        // dependency between fragment and vertex/transfer/compute stages. But it also comes with
        // higher overhead. In order to strike the balance, right now we only track it with VkEvent
        // if it ever written by transform feedback.
        useVkEvent = mTransformFeedbackWriteHeuristicBits.any();
    }

    if (useVkEvent && refCountedEventArray->initEventAtStage(context, eventStage))
    {
        // Replace the mCurrentReadEvents so that it tracks the current read so that we can
        // waitEvent later.
        mCurrentReadEvents.replaceEventAtStage(
            context, eventStage, refCountedEventArray->getEvent(eventStage), readAccessType);
    }
    else
    {
        // Accumulate new read usage to be used in pipelineBarrier.
        mCurrentReadAccess |= readAccessType;
        mCurrentReadStages |= readPipelineStageFlags;
    }

    if (getResourceUse() >= queueSerial)
    {
        // We should not run into situation that RP is writing to it while we are reading it here
        ASSERT(!(getWriteResourceUse() >= queueSerial));
        // A buffer could have read accessed by both renderPassCommands and
        // outsideRenderPassCommands and there is no need to endRP or flush. In this case, the
        // renderPassCommands' read will override the outsideRenderPassCommands' read, since its
        // queueSerial must be greater than outsideRP.
    }
    else
    {
        setQueueSerial(queueSerial);
    }
}

void BufferHelper::recordWriteBarrier(Context *context,
                                      VkAccessFlags writeAccessType,
                                      VkPipelineStageFlags writeStage,
                                      PipelineStage stageIndex,
                                      const QueueSerial &queueSerial,
                                      PipelineBarrierArray *pipelineBarriers,
                                      EventBarrierArray *eventBarriers,
                                      RefCountedEventCollector *eventCollector)
{
    Renderer *renderer = context->getRenderer();

    // Barrier against prior read VkEvents.
    if (!mCurrentReadEvents.empty())
    {
        // If we already have a event in the same command buffer, fall back to pipeline. Otherwise
        // you may run into wait an event that has not been set. This may be can be removed once we
        // fix https://issuetracker.google.com/392968868
        if (usedByCommandBuffer(queueSerial))
        {
            for (EventStage eventStage : mCurrentReadEvents.getBitMask())
            {
                mCurrentReadStages |= renderer->getPipelineStageMask(eventStage);
                mCurrentReadAccess |= mCurrentReadEvents.getAccessFlags(eventStage);
            }
        }
        else
        {
            for (EventStage eventStage : mCurrentReadEvents.getBitMask())
            {
                const RefCountedEvent &waitEvent = mCurrentReadEvents.getEvent(eventStage);
                const VkAccessFlags srcAccess    = mCurrentReadEvents.getAccessFlags(eventStage);
                eventBarriers->addEventMemoryBarrier(renderer, waitEvent, srcAccess, writeStage,
                                                     writeAccessType);
            }
        }
        // Garbage collect the event, which tracks GPU completion automatically.
        mCurrentReadEvents.releaseToEventCollector(eventCollector);
    }

    // Barrier against prior write VkEvent.
    if (mCurrentWriteEvent.valid())
    {
        const VkPipelineStageFlags srcStageFlags =
            renderer->getPipelineStageMask(mCurrentWriteEvent.getEventStage());

        // If we already have a write event in the same command buffer, fall back to pipeline
        // barrier. Using VkEvent to track multiple writes either requires tracking multiple write
        // events or has to replace existing event with another event that tracks more pipeline
        // stage bits. Both are a bit complex. Without evidence showing we are hitting performance
        // issue in real world situation, this will just use pipeline barriers to track extra stages
        // that not captured by mCurrentWriteEvent.
        if (writtenByCommandBuffer(queueSerial))
        {
            mCurrentWriteStages |= srcStageFlags;
            mCurrentWriteAccess |= mCurrentWriteEvent.getAccessFlags();
        }
        else
        {
            eventBarriers->addEventMemoryBarrier(
                context->getRenderer(), mCurrentWriteEvent.getEvent(),
                mCurrentWriteEvent.getAccessFlags(), writeStage, writeAccessType);
        }
        // Garbage collect the event, which tracks GPU completion automatically.
        mCurrentWriteEvent.releaseToEventCollector(eventCollector);
    }

    // We don't need to check mCurrentReadStages here since if it is not zero,
    // mCurrentReadAccess must not be zero as well. stage is finer grain than accessType.
    ASSERT((!mCurrentReadStages && !mCurrentReadAccess) ||
           (mCurrentReadStages && mCurrentReadAccess));

    // Barrier against prior access that not tracked by VkEvent using pipelineBarrier.
    if (mCurrentReadAccess != 0 || mCurrentWriteAccess != 0)
    {
        // If there are more pipeline stage bits not captured by eventBarrier, use pipelineBarrier.
        VkPipelineStageFlags srcStageMask = mCurrentWriteStages | mCurrentReadStages;
        if (srcStageMask)
        {
            pipelineBarriers->mergeMemoryBarrier(stageIndex, srcStageMask, writeStage,
                                                 mCurrentWriteAccess, writeAccessType);
        }

        mCurrentReadStages  = 0;
        mCurrentReadAccess  = 0;
        mCurrentWriteStages = 0;
        mCurrentWriteAccess = 0;
    }
}

void BufferHelper::recordWriteEvent(Context *context,
                                    VkAccessFlags writeAccessType,
                                    VkPipelineStageFlags writePipelineStageFlags,
                                    const QueueSerial &writeQueueSerial,
                                    PipelineStage writeStage,
                                    RefCountedEventArray *refCountedEventArray)
{
    EventStage eventStage = kBufferMemoryBarrierData[writeStage].eventStage;
    bool useVkEvent       = false;

    if (context->getFeatures().useVkEventForBufferBarrier.enabled &&
        eventStage != EventStage::InvalidEnum)
    {
        ASSERT(mCurrentReadEvents.empty());
        updatePipelineStageWriteHistory(writeStage);

        // VkCmdSetEvent can remove the unnecessary GPU pipeline bubble that comes from false
        // dependency between fragment and vertex/transfer/compute stages. But it also comes with
        // higher overhead. In order to strike the balance, right now we only track it with VkEvent
        // if it ever written by transform feedback.
        useVkEvent = mTransformFeedbackWriteHeuristicBits.any();

        // We only track one write event. In case of multiple writes like write from different
        // shader stages in the same render pass, only the first write is tracked by event,
        // additional writes will still be tracked by pipelineBarriers.
        if (mCurrentWriteEvent.valid())
        {
            useVkEvent = false;
        }
    }

    if (useVkEvent && refCountedEventArray->initEventAtStage(context, eventStage))
    {
        // Copy the event to mCurrentEvent so that we can wait for it in future. This will add extra
        // refcount to the underlying VkEvent.
        mCurrentWriteEvent.setEventAndAccessFlags(refCountedEventArray->getEvent(eventStage),
                                                  writeAccessType);
    }
    else
    {
        // Reset usages on the new write to be used by pipelineBarrier later.
        mCurrentWriteAccess = writeAccessType;
        mCurrentWriteStages = writePipelineStageFlags;
    }

    setWriteQueueSerial(writeQueueSerial);
}

void BufferHelper::fillWithColor(const angle::Color<uint8_t> &color,
                                 const gl::InternalFormat &internalFormat)
{
    uint32_t count =
        static_cast<uint32_t>(getSize()) / static_cast<uint32_t>(internalFormat.pixelBytes);
    void *buffer = static_cast<void *>(getMappedMemory());

    switch (internalFormat.internalFormat)
    {
        case GL_RGB565:
        {
            uint16_t pixelColor =
                ((color.blue & 0xF8) << 11) | ((color.green & 0xFC) << 5) | (color.red & 0xF8);
            uint16_t *pixelPtr = static_cast<uint16_t *>(buffer);
            std::fill_n<uint16_t *, uint32_t, uint16_t>(pixelPtr, count, pixelColor);
        }
        break;
        case GL_RGBA8:
        {
            uint32_t pixelColor =
                (color.alpha << 24) | (color.blue << 16) | (color.green << 8) | (color.red);
            uint32_t *pixelPtr = static_cast<uint32_t *>(buffer);
            std::fill_n<uint32_t *, uint32_t, uint32_t>(pixelPtr, count, pixelColor);
        }
        break;
        case GL_BGR565_ANGLEX:
        {
            uint16_t pixelColor =
                ((color.red & 0xF8) << 11) | ((color.green & 0xFC) << 5) | (color.blue & 0xF8);
            uint16_t *pixelPtr = static_cast<uint16_t *>(buffer);
            std::fill_n<uint16_t *, uint32_t, uint16_t>(pixelPtr, count, pixelColor);
        }
        break;
        case GL_BGRA8_EXT:
        {
            uint32_t pixelColor =
                (color.alpha << 24) | (color.red << 16) | (color.green << 8) | (color.blue);
            uint32_t *pixelPtr = static_cast<uint32_t *>(buffer);
            std::fill_n<uint32_t *, uint32_t, uint32_t>(pixelPtr, count, pixelColor);
        }
        break;
        default:
            UNREACHABLE();  // Unsupported format
    }
}

void BufferHelper::fillWithPattern(const void *pattern,
                                   size_t patternSize,
                                   size_t offset,
                                   size_t size)
{
    ASSERT(offset + size <= getSize());
    ASSERT((size % patternSize) == 0);
    ASSERT((offset % patternSize) == 0);

    uint8_t *buffer = getMappedMemory() + offset;
    std::memcpy(buffer, pattern, patternSize);
    size_t remaining = size - patternSize;
    while (remaining > patternSize)
    {
        std::memcpy(buffer + patternSize, buffer, patternSize);
        remaining -= patternSize;
        patternSize *= 2;
    }
    std::memcpy(buffer + patternSize, buffer, remaining);
    return;
}

// Used for ImageHelper non-zero memory allocation when useVmaForImageSuballocation is disabled.
angle::Result InitMappableDeviceMemory(ErrorContext *context,
                                       DeviceMemory *deviceMemory,
                                       VkDeviceSize size,
                                       int value,
                                       VkMemoryPropertyFlags memoryPropertyFlags)
{
    ASSERT(!context->getFeatures().useVmaForImageSuballocation.enabled);
    VkDevice device = context->getDevice();

    uint8_t *mapPointer;
    ANGLE_VK_TRY(context, deviceMemory->map(device, 0, VK_WHOLE_SIZE, 0, &mapPointer));
    memset(mapPointer, value, static_cast<size_t>(size));

    // if the memory type is not host coherent, we perform an explicit flush.
    if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
    {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType               = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory              = deviceMemory->getHandle();
        mappedRange.size                = VK_WHOLE_SIZE;
        ANGLE_VK_TRY(context, vkFlushMappedMemoryRanges(device, 1, &mappedRange));
    }

    deviceMemory->unmap(device);

    return angle::Result::Continue;
}

// ImageHelper implementation.
ImageHelper::ImageHelper()
{
    resetCachedProperties();
}

ImageHelper::~ImageHelper()
{
    ASSERT(!valid());
    ASSERT(!mAcquireNextImageSemaphore.valid());
}

void ImageHelper::resetCachedProperties()
{
    mImageType                   = VK_IMAGE_TYPE_2D;
    mTilingMode                  = VK_IMAGE_TILING_OPTIMAL;
    mCreateFlags                 = kVkImageCreateFlagsNone;
    mUsage                       = 0;
    mExtents                     = {};
    mRotatedAspectRatio          = false;
    mIntendedFormatID            = angle::FormatID::NONE;
    mActualFormatID              = angle::FormatID::NONE;
    mSamples                     = 1;
    mImageSerial                 = kInvalidImageSerial;
    mCurrentLayout               = ImageLayout::Undefined;
    mCurrentDeviceQueueIndex     = kInvalidDeviceQueueIndex;
    mIsReleasedToExternal        = false;
    mIsForeignImage              = false;
    mLastNonShaderReadOnlyLayout = ImageLayout::Undefined;
    mCurrentShaderReadStageMask  = 0;
    mFirstAllocatedLevel         = gl::LevelIndex(0);
    mLayerCount                  = 0;
    mLevelCount                  = 0;
    mTotalStagedBufferUpdateSize = 0;
    mAllocationSize              = 0;
    mMemoryAllocationType        = MemoryAllocationType::InvalidEnum;
    mMemoryTypeIndex             = kInvalidMemoryTypeIndex;
    std::fill(mViewFormats.begin(), mViewFormats.begin() + mViewFormats.max_size(),
              VK_FORMAT_UNDEFINED);
    mYcbcrConversionDesc.reset();
    mCurrentSingleClearValue.reset();
    mRenderPassUsageFlags.reset();

    setEntireContentUndefined();
}

void ImageHelper::setEntireContentDefined()
{
    for (LevelContentDefinedMask &levelContentDefined : mContentDefined)
    {
        levelContentDefined.set();
    }
    for (LevelContentDefinedMask &levelContentDefined : mStencilContentDefined)
    {
        levelContentDefined.set();
    }
}

void ImageHelper::setEntireContentUndefined()
{
    for (LevelContentDefinedMask &levelContentDefined : mContentDefined)
    {
        levelContentDefined.reset();
    }
    for (LevelContentDefinedMask &levelContentDefined : mStencilContentDefined)
    {
        levelContentDefined.reset();
    }

    // Note: this function is typically called during init/release, but also when importing an image
    // from Vulkan, so unlike invalidateSubresourceContentImpl, it doesn't attempt to make sure
    // emulated formats have a clear staged.
}

void ImageHelper::setContentDefined(LevelIndex levelStart,
                                    uint32_t levelCount,
                                    uint32_t layerStart,
                                    uint32_t layerCount,
                                    VkImageAspectFlags aspectFlags)
{
    // Mark the range as defined.  Layers above 8 are discarded, and are always assumed to have
    // defined contents.
    if (layerStart >= kMaxContentDefinedLayerCount)
    {
        return;
    }

    uint8_t layerRangeBits =
        GetContentDefinedLayerRangeBits(layerStart, layerCount, kMaxContentDefinedLayerCount);

    for (uint32_t levelOffset = 0; levelOffset < levelCount; ++levelOffset)
    {
        LevelIndex level = levelStart + levelOffset;

        if ((aspectFlags & ~VK_IMAGE_ASPECT_STENCIL_BIT) != 0)
        {
            getLevelContentDefined(level) |= layerRangeBits;
        }
        if ((aspectFlags & VK_IMAGE_ASPECT_STENCIL_BIT) != 0)
        {
            getLevelStencilContentDefined(level) |= layerRangeBits;
        }
    }
}

ImageHelper::LevelContentDefinedMask &ImageHelper::getLevelContentDefined(LevelIndex level)
{
    return mContentDefined[level.get()];
}

ImageHelper::LevelContentDefinedMask &ImageHelper::getLevelStencilContentDefined(LevelIndex level)
{
    return mStencilContentDefined[level.get()];
}

const ImageHelper::LevelContentDefinedMask &ImageHelper::getLevelContentDefined(
    LevelIndex level) const
{
    return mContentDefined[level.get()];
}

const ImageHelper::LevelContentDefinedMask &ImageHelper::getLevelStencilContentDefined(
    LevelIndex level) const
{
    return mStencilContentDefined[level.get()];
}

YcbcrConversionDesc ImageHelper::deriveConversionDesc(ErrorContext *context,
                                                      angle::FormatID actualFormatID,
                                                      angle::FormatID intendedFormatID)
{
    YcbcrConversionDesc conversionDesc{};
    const angle::Format &actualFormat = angle::Format::Get(actualFormatID);

    if (actualFormat.isYUV)
    {
        // Build a suitable conversionDesc; the image is not external but may be YUV
        // if app is using ANGLE's YUV internalformat extensions.
        Renderer *renderer = context->getRenderer();

        // The Vulkan spec states: The potential format features of the sampler YCBCR conversion
        // must support VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT or
        // VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT
        constexpr VkFormatFeatureFlags kChromaSubSampleFeatureBits =
            VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT |
            VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT |
            VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT;

        VkFormatFeatureFlags supportedFeatureBits =
            renderer->getImageFormatFeatureBits(actualFormatID, kChromaSubSampleFeatureBits);

        VkChromaLocation supportedLocation =
            (supportedFeatureBits & VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT) != 0
                ? VK_CHROMA_LOCATION_COSITED_EVEN
                : VK_CHROMA_LOCATION_MIDPOINT;
        vk::YcbcrLinearFilterSupport linearFilterSupported =
            (supportedFeatureBits &
             VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT) != 0
                ? vk::YcbcrLinearFilterSupport::Supported
                : vk::YcbcrLinearFilterSupport::Unsupported;

        VkSamplerYcbcrModelConversion conversionModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601;
        VkSamplerYcbcrRange colorRange                = VK_SAMPLER_YCBCR_RANGE_ITU_NARROW;
        VkFilter chromaFilter                         = kDefaultYCbCrChromaFilter;
        VkComponentMapping components                 = {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
        };

        conversionDesc.update(renderer, 0, conversionModel, colorRange, supportedLocation,
                              supportedLocation, chromaFilter, components, intendedFormatID,
                              linearFilterSupported);
    }

    return conversionDesc;
}

angle::Result ImageHelper::init(ErrorContext *context,
                                gl::TextureType textureType,
                                const VkExtent3D &extents,
                                const Format &format,
                                GLint samples,
                                VkImageUsageFlags usage,
                                gl::LevelIndex firstLevel,
                                uint32_t mipLevels,
                                uint32_t layerCount,
                                bool isRobustResourceInitEnabled,
                                bool hasProtectedContent)
{
    return initExternal(context, textureType, extents, format.getIntendedFormatID(),
                        format.getActualRenderableImageFormatID(), samples, usage,
                        kVkImageCreateFlagsNone, ImageLayout::Undefined, nullptr, firstLevel,
                        mipLevels, layerCount, isRobustResourceInitEnabled, hasProtectedContent,
                        deriveConversionDesc(context, format.getActualRenderableImageFormatID(),
                                             format.getIntendedFormatID()),
                        nullptr);
}

angle::Result ImageHelper::initFromCreateInfo(ErrorContext *context,
                                              const VkImageCreateInfo &requestedCreateInfo,
                                              VkMemoryPropertyFlags memoryPropertyFlags)
{
    ASSERT(!valid());
    ASSERT(!IsAnySubresourceContentDefined(mContentDefined));
    ASSERT(!IsAnySubresourceContentDefined(mStencilContentDefined));

    mImageType          = requestedCreateInfo.imageType;
    mExtents            = requestedCreateInfo.extent;
    mRotatedAspectRatio = false;
    mSamples            = std::max((int)requestedCreateInfo.samples, 1);
    mImageSerial        = context->getRenderer()->getResourceSerialFactory().generateImageSerial();
    mLayerCount         = requestedCreateInfo.arrayLayers;
    mLevelCount         = requestedCreateInfo.mipLevels;
    mUsage              = requestedCreateInfo.usage;

    // Validate that mLayerCount is compatible with the image type
    ASSERT(requestedCreateInfo.imageType != VK_IMAGE_TYPE_3D || mLayerCount == 1);
    ASSERT(requestedCreateInfo.imageType != VK_IMAGE_TYPE_2D || mExtents.depth == 1);

    mCurrentLayout = ImageLayout::Undefined;

    ANGLE_VK_TRY(context, mImage.init(context->getDevice(), requestedCreateInfo));

    mVkImageCreateInfo               = requestedCreateInfo;
    mVkImageCreateInfo.pNext         = nullptr;
    mVkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    MemoryProperties memoryProperties = {};

    ANGLE_TRY(initMemoryAndNonZeroFillIfNeeded(context, false, memoryProperties,
                                               memoryPropertyFlags,
                                               vk::MemoryAllocationType::StagingImage));
    return angle::Result::Continue;
}

angle::Result ImageHelper::copyToBufferOneOff(ErrorContext *context,
                                              BufferHelper *stagingBuffer,
                                              VkBufferImageCopy copyRegion)
{
    Renderer *renderer = context->getRenderer();
    ScopedPrimaryCommandBuffer scopedCommandBuffer(renderer->getDevice());
    ANGLE_TRY(renderer->getCommandBufferOneOff(context, ProtectionType::Unprotected,
                                               &scopedCommandBuffer));
    PrimaryCommandBuffer &commandBuffer = scopedCommandBuffer.get();

    VkSemaphore acquireNextImageSemaphore;
    recordBarrierOneOffImpl(renderer, getAspectFlags(), ImageLayout::TransferDst,
                            renderer->getQueueFamilyIndex(), &commandBuffer,
                            &acquireNextImageSemaphore);
    commandBuffer.copyBufferToImage(stagingBuffer->getBuffer().getHandle(), getImage(),
                                    getCurrentLayout(renderer), 1, &copyRegion);
    ANGLE_VK_TRY(context, commandBuffer.end());

    QueueSerial submitQueueSerial;
    ANGLE_TRY(renderer->queueSubmitOneOff(
        context, std::move(scopedCommandBuffer), ProtectionType::Unprotected,
        egl::ContextPriority::Medium, acquireNextImageSemaphore,
        kSwapchainAcquireImageWaitStageFlags, &submitQueueSerial));

    return renderer->finishQueueSerial(context, submitQueueSerial);
}

angle::Result ImageHelper::initMSAASwapchain(ErrorContext *context,
                                             gl::TextureType textureType,
                                             const VkExtent3D &extents,
                                             bool rotatedAspectRatio,
                                             const Format &format,
                                             GLint samples,
                                             VkImageUsageFlags usage,
                                             gl::LevelIndex firstLevel,
                                             uint32_t mipLevels,
                                             uint32_t layerCount,
                                             bool isRobustResourceInitEnabled,
                                             bool hasProtectedContent)
{
    ANGLE_TRY(initExternal(context, textureType, extents, format.getIntendedFormatID(),
                           format.getActualRenderableImageFormatID(), samples, usage,
                           kVkImageCreateFlagsNone, ImageLayout::Undefined, nullptr, firstLevel,
                           mipLevels, layerCount, isRobustResourceInitEnabled, hasProtectedContent,
                           YcbcrConversionDesc{}, nullptr));
    if (rotatedAspectRatio)
    {
        std::swap(mExtents.width, mExtents.height);
    }
    mRotatedAspectRatio = rotatedAspectRatio;
    return angle::Result::Continue;
}

angle::Result ImageHelper::initExternal(ErrorContext *context,
                                        gl::TextureType textureType,
                                        const VkExtent3D &extents,
                                        angle::FormatID intendedFormatID,
                                        angle::FormatID actualFormatID,
                                        GLint samples,
                                        VkImageUsageFlags usage,
                                        VkImageCreateFlags additionalCreateFlags,
                                        ImageLayout initialLayout,
                                        const void *externalImageCreateInfo,
                                        gl::LevelIndex firstLevel,
                                        uint32_t mipLevels,
                                        uint32_t layerCount,
                                        bool isRobustResourceInitEnabled,
                                        bool hasProtectedContent,
                                        YcbcrConversionDesc conversionDesc,
                                        const void *compressionControl)
{
    ASSERT(!valid());
    ASSERT(!IsAnySubresourceContentDefined(mContentDefined));
    ASSERT(!IsAnySubresourceContentDefined(mStencilContentDefined));

    Renderer *renderer = context->getRenderer();

    mImageType           = gl_vk::GetImageType(textureType);
    mExtents             = extents;
    mRotatedAspectRatio  = false;
    mIntendedFormatID    = intendedFormatID;
    mActualFormatID      = actualFormatID;
    mSamples             = std::max(samples, 1);
    mImageSerial         = renderer->getResourceSerialFactory().generateImageSerial();
    mFirstAllocatedLevel = firstLevel;
    mLevelCount          = mipLevels;
    mLayerCount          = layerCount;
    mCreateFlags =
        vk::GetMinimalImageCreateFlags(renderer, textureType, usage) | additionalCreateFlags;
    mUsage = usage;

    // Validate that mLayerCount is compatible with the texture type
    ASSERT(textureType != gl::TextureType::_3D || mLayerCount == 1);
    ASSERT(textureType != gl::TextureType::_2DArray || mExtents.depth == 1);
    ASSERT(textureType != gl::TextureType::External || mLayerCount == 1);
    ASSERT(textureType != gl::TextureType::Rectangle || mLayerCount == 1);
    ASSERT(textureType != gl::TextureType::CubeMap || mLayerCount == gl::kCubeFaceCount);
    ASSERT(textureType != gl::TextureType::CubeMapArray || mLayerCount % gl::kCubeFaceCount == 0);

    // If externalImageCreateInfo is provided, use that directly.  Otherwise derive the necessary
    // pNext chain.
    const void *imageCreateInfoPNext = externalImageCreateInfo;
    VkImageFormatListCreateInfoKHR imageFormatListInfoStorage;
    ImageListFormats imageListFormatsStorage;

    if (externalImageCreateInfo == nullptr)
    {
        imageCreateInfoPNext = DeriveCreateInfoPNext(
            context, mUsage, actualFormatID, compressionControl, &imageFormatListInfoStorage,
            &imageListFormatsStorage, &mCreateFlags);
    }
    else
    {
        // Derive the tiling for external images.
        deriveExternalImageTiling(externalImageCreateInfo);
    }

    mYcbcrConversionDesc = conversionDesc;

    const angle::Format &actualFormat   = angle::Format::Get(actualFormatID);
    const angle::Format &intendedFormat = angle::Format::Get(intendedFormatID);
    VkFormat actualVkFormat             = GetVkFormatFromFormatID(renderer, actualFormatID);

    ANGLE_TRACE_EVENT_INSTANT("gpu.angle.texture_metrics", "ImageHelper::initExternal",
                              "intended_format", intendedFormat.glInternalFormat, "actual_format",
                              actualFormat.glInternalFormat, "width", extents.width, "height",
                              extents.height);

    if (actualFormat.isYUV)
    {
        ASSERT(mYcbcrConversionDesc.valid());

        // The Vulkan spec states: If the pNext chain includes a VkExternalFormatANDROID structure
        // whose externalFormat member is not 0, flags must not include
        // VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT
        if (!IsYUVExternalFormat(actualFormatID))
        {
            // The Vulkan spec states: If sampler is used and the VkFormat of the image is a
            // multi-planar format, the image must have been created with
            // VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT
            mCreateFlags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
        }
    }

    if (hasProtectedContent)
    {
        mCreateFlags |= VK_IMAGE_CREATE_PROTECTED_BIT;
    }

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext             = imageCreateInfoPNext;
    imageInfo.flags             = mCreateFlags;
    imageInfo.imageType         = mImageType;
    imageInfo.format            = actualVkFormat;
    imageInfo.extent            = mExtents;
    imageInfo.mipLevels         = mLevelCount;
    imageInfo.arrayLayers       = mLayerCount;
    imageInfo.samples =
        gl_vk::GetSamples(mSamples, context->getFeatures().limitSampleCountTo2.enabled);
    imageInfo.tiling                = mTilingMode;
    imageInfo.usage                 = mUsage;
    imageInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0;
    imageInfo.pQueueFamilyIndices   = nullptr;
    imageInfo.initialLayout         = ConvertImageLayoutToVkImageLayout(renderer, initialLayout);

    mCurrentLayout               = initialLayout;
    mCurrentDeviceQueueIndex     = kInvalidDeviceQueueIndex;
    mIsReleasedToExternal        = false;
    mIsForeignImage              = false;
    mLastNonShaderReadOnlyLayout = ImageLayout::Undefined;
    mCurrentShaderReadStageMask  = 0;

    ANGLE_VK_TRY(context, mImage.init(context->getDevice(), imageInfo));

    // Find the image formats in pNext chain in imageInfo.
    deriveImageViewFormatFromCreateInfoPNext(imageInfo, mViewFormats);

    mVkImageCreateInfo               = imageInfo;
    mVkImageCreateInfo.pNext         = nullptr;
    mVkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    stageClearIfEmulatedFormat(isRobustResourceInitEnabled, externalImageCreateInfo != nullptr);

    // Consider the contents defined for any image that has the PREINITIALIZED layout, or is
    // imported from external.
    if (initialLayout != ImageLayout::Undefined || externalImageCreateInfo != nullptr)
    {
        setEntireContentDefined();
    }

    return angle::Result::Continue;
}

// static
const void *ImageHelper::DeriveCreateInfoPNext(
    ErrorContext *context,
    VkImageUsageFlags usage,
    angle::FormatID actualFormatID,
    const void *pNext,
    VkImageFormatListCreateInfoKHR *imageFormatListInfoStorage,
    std::array<VkFormat, kImageListFormatCount> *imageListFormatsStorage,
    VkImageCreateFlags *createFlagsOut)
{
    // With the introduction of sRGB related GLES extensions any sample/render target could be
    // respecified causing it to be interpreted in a different colorspace.  Create the VkImage
    // accordingly.
    Renderer *renderer                = context->getRenderer();
    const angle::Format &actualFormat = angle::Format::Get(actualFormatID);
    angle::FormatID additionalFormat =
        actualFormat.isSRGB ? ConvertToLinear(actualFormatID) : ConvertToSRGB(actualFormatID);
    (*imageListFormatsStorage)[0] = vk::GetVkFormatFromFormatID(renderer, actualFormatID);
    (*imageListFormatsStorage)[1] = vk::GetVkFormatFromFormatID(renderer, additionalFormat);

    // Don't add the format list if the storage bit is enabled for the image; framebuffer
    // compression is already disabled in that case, and GL allows many formats to alias
    // the original format for storage images (more than ANGLE provides in the format list).
    if (renderer->getFeatures().supportsImageFormatList.enabled &&
        renderer->haveSameFormatFeatureBits(actualFormatID, additionalFormat) &&
        (usage & VK_IMAGE_USAGE_STORAGE_BIT) == 0)
    {
        // Add VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT to VkImage create flag
        *createFlagsOut |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

        // There is just 1 additional format we might use to create a VkImageView for this
        // VkImage
        imageFormatListInfoStorage->sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO_KHR;
        imageFormatListInfoStorage->pNext = pNext;
        imageFormatListInfoStorage->viewFormatCount = kImageListFormatCount;
        imageFormatListInfoStorage->pViewFormats    = imageListFormatsStorage->data();

        pNext = imageFormatListInfoStorage;
    }

    return pNext;
}

// static
bool ImageHelper::FormatSupportsUsage(Renderer *renderer,
                                      VkFormat format,
                                      VkImageType imageType,
                                      VkImageTiling tilingMode,
                                      VkImageUsageFlags usageFlags,
                                      VkImageCreateFlags createFlags,
                                      void *formatInfoPNext,
                                      void *propertiesPNext,
                                      const FormatSupportCheck formatSupportCheck)
{
    VkPhysicalDeviceImageFormatInfo2 imageFormatInfo = {};
    imageFormatInfo.sType  = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
    imageFormatInfo.pNext  = formatInfoPNext;
    imageFormatInfo.format = format;
    imageFormatInfo.type   = imageType;
    imageFormatInfo.tiling = tilingMode;
    imageFormatInfo.usage  = usageFlags;
    imageFormatInfo.flags  = createFlags;

    VkImageFormatProperties2 imageFormatProperties2 = {};
    imageFormatProperties2.sType                    = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
    imageFormatProperties2.pNext                    = propertiesPNext;

    VkResult result = vkGetPhysicalDeviceImageFormatProperties2(
        renderer->getPhysicalDevice(), &imageFormatInfo, &imageFormatProperties2);

    if (formatSupportCheck == FormatSupportCheck::RequireMultisampling)
    {
        // Some drivers return success but sampleCounts == 1 which means no MSRTT
        return result == VK_SUCCESS &&
               imageFormatProperties2.imageFormatProperties.sampleCounts > 1;
    }
    return result == VK_SUCCESS;
}

void ImageHelper::setImageFormatsFromActualFormat(VkFormat actualFormat,
                                                  ImageFormats &imageFormatsOut)
{
    imageFormatsOut.push_back(actualFormat);
}

void ImageHelper::deriveImageViewFormatFromCreateInfoPNext(VkImageCreateInfo &imageInfo,
                                                           ImageFormats &formatOut)
{
    const VkBaseInStructure *pNextChain =
        reinterpret_cast<const VkBaseInStructure *>(imageInfo.pNext);
    while (pNextChain != nullptr &&
           pNextChain->sType != VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO_KHR)
    {
        pNextChain = pNextChain->pNext;
    }

    // Clear formatOut in case it has leftovers from previous VkImage in the case of releaseImage
    // followed by initExternal.
    std::fill(formatOut.begin(), formatOut.begin() + formatOut.max_size(), VK_FORMAT_UNDEFINED);
    if (pNextChain != nullptr)
    {
        const VkImageFormatListCreateInfoKHR *imageFormatCreateInfo =
            reinterpret_cast<const VkImageFormatListCreateInfoKHR *>(pNextChain);

        for (uint32_t i = 0; i < imageFormatCreateInfo->viewFormatCount; i++)
        {
            formatOut.push_back(*(imageFormatCreateInfo->pViewFormats + i));
        }
    }
    else
    {
        setImageFormatsFromActualFormat(imageInfo.format, formatOut);
    }
}

void ImageHelper::deriveExternalImageTiling(const void *createInfoChain)
{
    const VkBaseInStructure *chain = reinterpret_cast<const VkBaseInStructure *>(createInfoChain);
    while (chain != nullptr)
    {
        if (chain->sType == VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT ||
            chain->sType == VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT)
        {
            mTilingMode = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
            return;
        }

        chain = reinterpret_cast<const VkBaseInStructure *>(chain->pNext);
    }
}

void ImageHelper::releaseImage(Renderer *renderer)
{
    // mDeviceMemory and mVmaAllocation should not be valid at the same time.
    ASSERT(!mDeviceMemory.valid() || !mVmaAllocation.valid());
    if (mDeviceMemory.valid())
    {
        renderer->onMemoryDealloc(mMemoryAllocationType, mAllocationSize, mMemoryTypeIndex,
                                  mDeviceMemory.getHandle());
    }
    if (mVmaAllocation.valid())
    {
        renderer->onMemoryDealloc(mMemoryAllocationType, mAllocationSize, mMemoryTypeIndex,
                                  mVmaAllocation.getHandle());
    }
    mCurrentEvent.release(renderer);
    mLastNonShaderReadOnlyEvent.release(renderer);
    renderer->collectGarbage(mUse, &mImage, &mDeviceMemory, &mVmaAllocation);
    mViewFormats.clear();
    mUse.reset();
    mImageSerial          = kInvalidImageSerial;
    mMemoryAllocationType = MemoryAllocationType::InvalidEnum;
    setEntireContentUndefined();
}

void ImageHelper::releaseImageFromShareContexts(Renderer *renderer,
                                                ContextVk *contextVk,
                                                UniqueSerial imageSiblingSerial)
{
    finalizeImageLayoutInShareContexts(renderer, contextVk, imageSiblingSerial);
    contextVk->addToPendingImageGarbage(mUse, mAllocationSize);
    releaseImage(renderer);
}

void ImageHelper::finalizeImageLayoutInShareContexts(Renderer *renderer,
                                                     ContextVk *contextVk,
                                                     UniqueSerial imageSiblingSerial)
{
    if (contextVk && mImageSerial.valid())
    {
        for (auto context : contextVk->getShareGroup()->getContexts())
        {
            vk::GetImpl(context.second)->finalizeImageLayout(this, imageSiblingSerial);
        }
    }
}

void ImageHelper::releaseStagedUpdates(Renderer *renderer)
{
    ASSERT(validateSubresourceUpdateRefCountsConsistent());

    // Remove updates that never made it to the texture.
    for (std::vector<SubresourceUpdate> &levelUpdates : mSubresourceUpdates)
    {
        for (SubresourceUpdate &update : levelUpdates)
        {
            update.release(renderer);
        }
    }

    ASSERT(validateSubresourceUpdateRefCountsConsistent());

    mSubresourceUpdates.clear();
    mTotalStagedBufferUpdateSize = 0;
    mCurrentSingleClearValue.reset();
}

void ImageHelper::resetImageWeakReference()
{
    mImage.reset();
    mImageSerial        = kInvalidImageSerial;
    mRotatedAspectRatio = false;
    // Caller must ensure ANI semaphores are properly waited or released.
    ASSERT(!mAcquireNextImageSemaphore.valid());
}

angle::Result ImageHelper::initializeNonZeroMemory(ErrorContext *context,
                                                   bool hasProtectedContent,
                                                   VkMemoryPropertyFlags flags,
                                                   VkDeviceSize size)
{
    // If available, memory mapping should be used.
    Renderer *renderer = context->getRenderer();

    if ((flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
    {
        // Wipe memory to an invalid value when the 'allocateNonZeroMemory' feature is enabled. The
        // invalid values ensures our testing doesn't assume zero-initialized memory.
        constexpr int kNonZeroInitValue = 0x3F;
        if (renderer->getFeatures().useVmaForImageSuballocation.enabled)
        {
            ANGLE_VK_TRY(context,
                         renderer->getImageMemorySuballocator().mapMemoryAndInitWithNonZeroValue(
                             renderer, &mVmaAllocation, size, kNonZeroInitValue, flags));
        }
        else
        {
            ANGLE_TRY(vk::InitMappableDeviceMemory(context, &mDeviceMemory, size, kNonZeroInitValue,
                                                   flags));
        }

        return angle::Result::Continue;
    }

    // If mapping the memory is unavailable, a staging resource is used.
    const angle::Format &angleFormat = getActualFormat();
    bool isCompressedFormat          = angleFormat.isBlock;

    if (angleFormat.isYUV)
    {
        // VUID-vkCmdClearColorImage-image-01545
        // vkCmdClearColorImage(): format must not be one of the formats requiring sampler YCBCR
        // conversion for VK_IMAGE_ASPECT_COLOR_BIT image views
        return angle::Result::Continue;
    }

    // Since we are going to do a one off out of order submission, there shouldn't any pending
    // setEvent.
    ASSERT(!mCurrentEvent.valid());

    ScopedPrimaryCommandBuffer scopedCommandBuffer(renderer->getDevice());
    auto protectionType = ConvertProtectionBoolToType(hasProtectedContent);
    ANGLE_TRY(renderer->getCommandBufferOneOff(context, protectionType, &scopedCommandBuffer));
    PrimaryCommandBuffer &commandBuffer = scopedCommandBuffer.get();

    // Queue a DMA copy.
    VkSemaphore acquireNextImageSemaphore;
    recordBarrierOneOffImpl(renderer, getAspectFlags(), ImageLayout::TransferDst,
                            context->getDeviceQueueIndex(), &commandBuffer,
                            &acquireNextImageSemaphore);
    // SwapChain image should not come here
    ASSERT(acquireNextImageSemaphore == VK_NULL_HANDLE);

    StagingBuffer stagingBuffer;

    if (isCompressedFormat)
    {
        // If format is compressed, set its contents through buffer copies.

        // The staging buffer memory is non-zero-initialized in 'init'.
        ANGLE_TRY(stagingBuffer.init(context, size, StagingUsage::Write));

        for (LevelIndex level(0); level < LevelIndex(mLevelCount); ++level)
        {
            VkBufferImageCopy copyRegion = {};

            gl_vk::GetExtent(getLevelExtents(level), &copyRegion.imageExtent);
            copyRegion.imageSubresource.aspectMask = getAspectFlags();
            copyRegion.imageSubresource.layerCount = mLayerCount;

            // If image has depth and stencil, copy to each individually per Vulkan spec.
            bool hasBothDepthAndStencil = isCombinedDepthStencilFormat();
            if (hasBothDepthAndStencil)
            {
                copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            }

            commandBuffer.copyBufferToImage(stagingBuffer.getBuffer().getHandle(), mImage,
                                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

            if (hasBothDepthAndStencil)
            {
                copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;

                commandBuffer.copyBufferToImage(stagingBuffer.getBuffer().getHandle(), mImage,
                                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                                                &copyRegion);
            }
        }
    }
    else
    {
        // Otherwise issue clear commands.
        VkImageSubresourceRange subresource = {};
        subresource.aspectMask              = getAspectFlags();
        subresource.baseMipLevel            = 0;
        subresource.levelCount              = mLevelCount;
        subresource.baseArrayLayer          = 0;
        subresource.layerCount              = mLayerCount;

        // Arbitrary value to initialize the memory with.  Note: the given uint value, reinterpreted
        // as float is about 0.7.
        constexpr uint32_t kInitValue   = 0x3F345678;
        constexpr float kInitValueFloat = 0.12345f;

        if ((subresource.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) != 0)
        {
            VkClearColorValue clearValue;
            clearValue.uint32[0] = kInitValue;
            clearValue.uint32[1] = kInitValue;
            clearValue.uint32[2] = kInitValue;
            clearValue.uint32[3] = kInitValue;

            commandBuffer.clearColorImage(mImage, getCurrentLayout(renderer), clearValue, 1,
                                          &subresource);
        }
        else
        {
            VkClearDepthStencilValue clearValue;
            clearValue.depth   = kInitValueFloat;
            clearValue.stencil = kInitValue;

            commandBuffer.clearDepthStencilImage(mImage, getCurrentLayout(renderer), clearValue, 1,
                                                 &subresource);
        }
    }

    ANGLE_VK_TRY(context, commandBuffer.end());

    QueueSerial queueSerial;
    ANGLE_TRY(renderer->queueSubmitOneOff(context, std::move(scopedCommandBuffer), protectionType,
                                          egl::ContextPriority::Medium, VK_NULL_HANDLE, 0,
                                          &queueSerial));

    if (isCompressedFormat)
    {
        stagingBuffer.collectGarbage(renderer, queueSerial);
    }
    setQueueSerial(queueSerial);
    ASSERT(!mIsForeignImage);

    return angle::Result::Continue;
}

VkResult ImageHelper::initMemory(ErrorContext *context,
                                 const MemoryProperties &memoryProperties,
                                 VkMemoryPropertyFlags flags,
                                 VkMemoryPropertyFlags excludedFlags,
                                 const VkMemoryRequirements *memoryRequirements,
                                 const bool allocateDedicatedMemory,
                                 MemoryAllocationType allocationType,
                                 VkMemoryPropertyFlags *flagsOut,
                                 VkDeviceSize *sizeOut)
{
    mMemoryAllocationType = allocationType;

    // To allocate memory here, if possible, we use the image memory suballocator which uses VMA.
    ASSERT(excludedFlags < VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM);
    Renderer *renderer = context->getRenderer();
    if (renderer->getFeatures().useVmaForImageSuballocation.enabled)
    {
        // While it may be preferable to allocate the image on the device, it should also be
        // possible to allocate on other memory types if the device is out of memory.
        VkMemoryPropertyFlags requiredFlags  = flags & (~excludedFlags);
        VkMemoryPropertyFlags preferredFlags = flags;
        VK_RESULT_TRY(renderer->getImageMemorySuballocator().allocateAndBindMemory(
            context, &mImage, &mVkImageCreateInfo, requiredFlags, preferredFlags,
            memoryRequirements, allocateDedicatedMemory, mMemoryAllocationType, &mVmaAllocation,
            flagsOut, &mMemoryTypeIndex, &mAllocationSize));
    }
    else
    {
        VK_RESULT_TRY(AllocateImageMemory(context, mMemoryAllocationType, flags, flagsOut, nullptr,
                                          &mImage, &mMemoryTypeIndex, &mDeviceMemory,
                                          &mAllocationSize));
    }

    mCurrentDeviceQueueIndex = context->getDeviceQueueIndex();
    mIsReleasedToExternal    = false;
    mIsForeignImage          = false;
    *sizeOut                 = mAllocationSize;

    return VK_SUCCESS;
}

angle::Result ImageHelper::initMemoryAndNonZeroFillIfNeeded(
    ErrorContext *context,
    bool hasProtectedContent,
    const MemoryProperties &memoryProperties,
    VkMemoryPropertyFlags flags,
    MemoryAllocationType allocationType)
{
    Renderer *renderer = context->getRenderer();
    VkMemoryPropertyFlags outputFlags;
    VkDeviceSize outputSize;

    if (hasProtectedContent)
    {
        flags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
    }

    // Get memory requirements for the allocation.
    VkMemoryRequirements memoryRequirements;
    mImage.getMemoryRequirements(renderer->getDevice(), &memoryRequirements);
    bool allocateDedicatedMemory =
        renderer->getImageMemorySuballocator().needsDedicatedMemory(memoryRequirements.size);

    ANGLE_VK_TRY(context,
                 initMemory(context, memoryProperties, flags, 0, &memoryRequirements,
                            allocateDedicatedMemory, allocationType, &outputFlags, &outputSize));

    // Memory can only be non-zero initialized if the TRANSFER_DST usage is set.  This is normally
    // the case, but not with |initImplicitMultisampledRenderToTexture| which creates a
    // lazy-allocated transient image.
    if (renderer->getFeatures().allocateNonZeroMemory.enabled &&
        (mUsage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) != 0)
    {
        ANGLE_TRY(initializeNonZeroMemory(context, hasProtectedContent, outputFlags, outputSize));
    }
    return angle::Result::Continue;
}

angle::Result ImageHelper::initExternalMemory(ErrorContext *context,
                                              const MemoryProperties &memoryProperties,
                                              const VkMemoryRequirements &memoryRequirements,
                                              uint32_t extraAllocationInfoCount,
                                              const void **extraAllocationInfo,
                                              DeviceQueueIndex currentDeviceQueueIndex,
                                              VkMemoryPropertyFlags flags)
{
    // Vulkan allows up to 4 memory planes.
    constexpr size_t kMaxMemoryPlanes                                     = 4;
    constexpr VkImageAspectFlagBits kMemoryPlaneAspects[kMaxMemoryPlanes] = {
        VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT,
        VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT,
        VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT,
        VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT,
    };
    ASSERT(extraAllocationInfoCount <= kMaxMemoryPlanes);

    VkBindImagePlaneMemoryInfoKHR bindImagePlaneMemoryInfo = {};
    bindImagePlaneMemoryInfo.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO;

    const VkBindImagePlaneMemoryInfoKHR *bindImagePlaneMemoryInfoPtr =
        extraAllocationInfoCount == 1 ? nullptr : &bindImagePlaneMemoryInfo;

    mAllocationSize       = memoryRequirements.size;
    mMemoryAllocationType = MemoryAllocationType::ImageExternal;

    for (uint32_t memoryPlane = 0; memoryPlane < extraAllocationInfoCount; ++memoryPlane)
    {
        bindImagePlaneMemoryInfo.planeAspect = kMemoryPlaneAspects[memoryPlane];

        ANGLE_VK_TRY(context, AllocateImageMemoryWithRequirements(
                                  context, mMemoryAllocationType, flags, memoryRequirements,
                                  extraAllocationInfo[memoryPlane], bindImagePlaneMemoryInfoPtr,
                                  &mImage, &mMemoryTypeIndex, &mDeviceMemory));
    }
    mCurrentDeviceQueueIndex = currentDeviceQueueIndex;
    mIsReleasedToExternal    = false;
    mIsForeignImage          = currentDeviceQueueIndex == kForeignDeviceQueueIndex;

    return angle::Result::Continue;
}

angle::Result ImageHelper::initLayerImageView(ErrorContext *context,
                                              gl::TextureType textureType,
                                              VkImageAspectFlags aspectMask,
                                              const gl::SwizzleState &swizzleMap,
                                              ImageView *imageViewOut,
                                              LevelIndex baseMipLevelVk,
                                              uint32_t levelCount,
                                              uint32_t baseArrayLayer,
                                              uint32_t layerCount) const
{
    return initLayerImageViewImpl(context, textureType, aspectMask, swizzleMap, imageViewOut,
                                  baseMipLevelVk, levelCount, baseArrayLayer, layerCount,
                                  GetVkFormatFromFormatID(context->getRenderer(), mActualFormatID),
                                  kDefaultImageViewUsageFlags, gl::YuvSamplingMode::Default);
}

angle::Result ImageHelper::initLayerImageViewWithUsage(ErrorContext *context,
                                                       gl::TextureType textureType,
                                                       VkImageAspectFlags aspectMask,
                                                       const gl::SwizzleState &swizzleMap,
                                                       ImageView *imageViewOut,
                                                       LevelIndex baseMipLevelVk,
                                                       uint32_t levelCount,
                                                       uint32_t baseArrayLayer,
                                                       uint32_t layerCount,
                                                       VkImageUsageFlags imageUsageFlags) const
{
    return initLayerImageViewImpl(context, textureType, aspectMask, swizzleMap, imageViewOut,
                                  baseMipLevelVk, levelCount, baseArrayLayer, layerCount,
                                  GetVkFormatFromFormatID(context->getRenderer(), mActualFormatID),
                                  imageUsageFlags, gl::YuvSamplingMode::Default);
}

angle::Result ImageHelper::initLayerImageViewWithYuvModeOverride(
    ErrorContext *context,
    gl::TextureType textureType,
    VkImageAspectFlags aspectMask,
    const gl::SwizzleState &swizzleMap,
    ImageView *imageViewOut,
    LevelIndex baseMipLevelVk,
    uint32_t levelCount,
    uint32_t baseArrayLayer,
    uint32_t layerCount,
    gl::YuvSamplingMode yuvSamplingMode,
    VkImageUsageFlags imageUsageFlags) const
{
    return initLayerImageViewImpl(context, textureType, aspectMask, swizzleMap, imageViewOut,
                                  baseMipLevelVk, levelCount, baseArrayLayer, layerCount,
                                  GetVkFormatFromFormatID(context->getRenderer(), mActualFormatID),
                                  imageUsageFlags, yuvSamplingMode);
}

angle::Result ImageHelper::initLayerImageViewImpl(ErrorContext *context,
                                                  gl::TextureType textureType,
                                                  VkImageAspectFlags aspectMask,
                                                  const gl::SwizzleState &swizzleMap,
                                                  ImageView *imageViewOut,
                                                  LevelIndex baseMipLevelVk,
                                                  uint32_t levelCount,
                                                  uint32_t baseArrayLayer,
                                                  uint32_t layerCount,
                                                  VkFormat imageFormat,
                                                  VkImageUsageFlags usageFlags,
                                                  gl::YuvSamplingMode yuvSamplingMode) const
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.flags                 = 0;
    viewInfo.image                 = mImage.getHandle();
    viewInfo.viewType              = gl_vk::GetImageViewType(textureType);
    viewInfo.format                = imageFormat;

    if (swizzleMap.swizzleRequired() && !mYcbcrConversionDesc.valid())
    {
        viewInfo.components.r = gl_vk::GetSwizzle(swizzleMap.swizzleRed);
        viewInfo.components.g = gl_vk::GetSwizzle(swizzleMap.swizzleGreen);
        viewInfo.components.b = gl_vk::GetSwizzle(swizzleMap.swizzleBlue);
        viewInfo.components.a = gl_vk::GetSwizzle(swizzleMap.swizzleAlpha);
    }
    else
    {
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    }
    viewInfo.subresourceRange.aspectMask     = aspectMask;
    viewInfo.subresourceRange.baseMipLevel   = baseMipLevelVk.get();
    viewInfo.subresourceRange.levelCount     = levelCount;
    viewInfo.subresourceRange.baseArrayLayer = baseArrayLayer;
    viewInfo.subresourceRange.layerCount     = layerCount;

    VkImageViewUsageCreateInfo imageViewUsageCreateInfo = {};
    if (usageFlags)
    {
        imageViewUsageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO;
        imageViewUsageCreateInfo.usage = usageFlags;

        viewInfo.pNext = &imageViewUsageCreateInfo;
    }

    VkSamplerYcbcrConversionInfo yuvConversionInfo = {};

    auto conversionDesc =
        yuvSamplingMode == gl::YuvSamplingMode::Y2Y ? getY2YConversionDesc() : mYcbcrConversionDesc;

    if (conversionDesc.valid())
    {
        ASSERT((context->getFeatures().supportsYUVSamplerConversion.enabled));
        yuvConversionInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO;
        yuvConversionInfo.pNext = nullptr;
        ANGLE_TRY(context->getRenderer()->getYuvConversionCache().getSamplerYcbcrConversion(
            context, conversionDesc, &yuvConversionInfo.conversion));
        AddToPNextChain(&viewInfo, &yuvConversionInfo);

        // VUID-VkImageViewCreateInfo-image-02399
        // If image has an external format, format must be VK_FORMAT_UNDEFINED
        if (conversionDesc.getExternalFormat() != 0)
        {
            viewInfo.format = VK_FORMAT_UNDEFINED;
        }
    }
    ANGLE_VK_TRY(context, imageViewOut->init(context->getDevice(), viewInfo));
    return angle::Result::Continue;
}

angle::Result ImageHelper::initReinterpretedLayerImageView(ErrorContext *context,
                                                           gl::TextureType textureType,
                                                           VkImageAspectFlags aspectMask,
                                                           const gl::SwizzleState &swizzleMap,
                                                           ImageView *imageViewOut,
                                                           LevelIndex baseMipLevelVk,
                                                           uint32_t levelCount,
                                                           uint32_t baseArrayLayer,
                                                           uint32_t layerCount,
                                                           VkImageUsageFlags imageUsageFlags,
                                                           angle::FormatID imageViewFormat) const
{
    VkImageUsageFlags usageFlags =
        imageUsageFlags & GetMaximalImageUsageFlags(context->getRenderer(), imageViewFormat);

    return initLayerImageViewImpl(
        context, textureType, aspectMask, swizzleMap, imageViewOut, baseMipLevelVk, levelCount,
        baseArrayLayer, layerCount,
        vk::GetVkFormatFromFormatID(context->getRenderer(), imageViewFormat), usageFlags,
        gl::YuvSamplingMode::Default);
}

void ImageHelper::destroy(Renderer *renderer)
{
    VkDevice device = renderer->getDevice();

    // mDeviceMemory and mVmaAllocation should not be valid at the same time.
    ASSERT(!mDeviceMemory.valid() || !mVmaAllocation.valid());
    if (mDeviceMemory.valid())
    {
        renderer->onMemoryDealloc(mMemoryAllocationType, mAllocationSize, mMemoryTypeIndex,
                                  mDeviceMemory.getHandle());
    }
    if (mVmaAllocation.valid())
    {
        renderer->onMemoryDealloc(mMemoryAllocationType, mAllocationSize, mMemoryTypeIndex,
                                  mVmaAllocation.getHandle());
    }

    mCurrentEvent.release(renderer);
    mLastNonShaderReadOnlyEvent.release(renderer);
    mImage.destroy(device);
    mDeviceMemory.destroy(device);
    mVmaAllocation.destroy(renderer->getAllocator());
    mCurrentLayout        = ImageLayout::Undefined;
    mImageType            = VK_IMAGE_TYPE_2D;
    mLayerCount           = 0;
    mLevelCount           = 0;
    mMemoryAllocationType = MemoryAllocationType::InvalidEnum;

    setEntireContentUndefined();
}

void ImageHelper::init2DWeakReference(ErrorContext *context,
                                      VkImage handle,
                                      const gl::Extents &glExtents,
                                      bool rotatedAspectRatio,
                                      angle::FormatID intendedFormatID,
                                      angle::FormatID actualFormatID,
                                      VkImageCreateFlags createFlags,
                                      VkImageUsageFlags usage,
                                      GLint samples,
                                      bool isRobustResourceInitEnabled)
{
    ASSERT(!valid());
    ASSERT(!IsAnySubresourceContentDefined(mContentDefined));
    ASSERT(!IsAnySubresourceContentDefined(mStencilContentDefined));
    vk::Renderer *renderer = context->getRenderer();

    gl_vk::GetExtent(glExtents, &mExtents);
    mRotatedAspectRatio      = rotatedAspectRatio;
    mIntendedFormatID        = intendedFormatID;
    mActualFormatID          = actualFormatID;
    mCreateFlags             = createFlags;
    mUsage                   = usage;
    mSamples                 = std::max(samples, 1);
    mImageSerial             = renderer->getResourceSerialFactory().generateImageSerial();
    mCurrentDeviceQueueIndex = context->getDeviceQueueIndex();
    mIsReleasedToExternal    = false;
    mIsForeignImage          = false;
    mCurrentLayout           = ImageLayout::Undefined;
    mLayerCount              = 1;
    mLevelCount              = 1;

    // The view formats and usage flags are used for imageless framebuffers. Here, the former is set
    // similar to deriveImageViewFormatFromCreateInfoPNext() when there is no pNext from a
    // VkImageCreateInfo object.
    setImageFormatsFromActualFormat(GetVkFormatFromFormatID(renderer, actualFormatID),
                                    mViewFormats);

    mImage.setHandle(handle);

    stageClearIfEmulatedFormat(isRobustResourceInitEnabled, false);
}

angle::Result ImageHelper::init2DStaging(ErrorContext *context,
                                         bool hasProtectedContent,
                                         const MemoryProperties &memoryProperties,
                                         const gl::Extents &glExtents,
                                         angle::FormatID intendedFormatID,
                                         angle::FormatID actualFormatID,
                                         VkImageUsageFlags usage,
                                         uint32_t layerCount)
{
    gl_vk::GetExtent(glExtents, &mExtents);

    return initStaging(context, hasProtectedContent, memoryProperties, VK_IMAGE_TYPE_2D, mExtents,
                       intendedFormatID, actualFormatID, 1, usage, 1, layerCount);
}

angle::Result ImageHelper::initStaging(ErrorContext *context,
                                       bool hasProtectedContent,
                                       const MemoryProperties &memoryProperties,
                                       VkImageType imageType,
                                       const VkExtent3D &extents,
                                       angle::FormatID intendedFormatID,
                                       angle::FormatID actualFormatID,
                                       GLint samples,
                                       VkImageUsageFlags usage,
                                       uint32_t mipLevels,
                                       uint32_t layerCount)
{
    ASSERT(!valid());
    ASSERT(!IsAnySubresourceContentDefined(mContentDefined));
    ASSERT(!IsAnySubresourceContentDefined(mStencilContentDefined));
    vk::Renderer *renderer = context->getRenderer();

    mImageType          = imageType;
    mExtents            = extents;
    mRotatedAspectRatio = false;
    mIntendedFormatID   = intendedFormatID;
    mActualFormatID     = actualFormatID;
    mSamples            = std::max(samples, 1);
    mImageSerial        = renderer->getResourceSerialFactory().generateImageSerial();
    mLayerCount         = layerCount;
    mLevelCount         = mipLevels;
    mUsage              = usage;

    // Validate that mLayerCount is compatible with the image type
    ASSERT(imageType != VK_IMAGE_TYPE_3D || mLayerCount == 1);
    ASSERT(imageType != VK_IMAGE_TYPE_2D || mExtents.depth == 1);

    mCurrentLayout = ImageLayout::Undefined;

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.flags             = hasProtectedContent ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;
    imageInfo.imageType         = mImageType;
    imageInfo.format            = GetVkFormatFromFormatID(renderer, actualFormatID);
    imageInfo.extent            = mExtents;
    imageInfo.mipLevels         = mLevelCount;
    imageInfo.arrayLayers       = mLayerCount;
    imageInfo.samples =
        gl_vk::GetSamples(mSamples, context->getFeatures().limitSampleCountTo2.enabled);
    imageInfo.tiling                = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage                 = usage;
    imageInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0;
    imageInfo.pQueueFamilyIndices   = nullptr;
    imageInfo.initialLayout         = getCurrentLayout(context->getRenderer());

    ANGLE_VK_TRY(context, mImage.init(context->getDevice(), imageInfo));

    mVkImageCreateInfo               = imageInfo;
    mVkImageCreateInfo.pNext         = nullptr;
    mVkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Allocate and bind device-local memory.
    VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (hasProtectedContent)
    {
        memoryPropertyFlags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
    }

    ANGLE_TRY(initMemoryAndNonZeroFillIfNeeded(context, hasProtectedContent, memoryProperties,
                                               memoryPropertyFlags,
                                               vk::MemoryAllocationType::StagingImage));
    return angle::Result::Continue;
}

angle::Result ImageHelper::initImplicitMultisampledRenderToTexture(
    ErrorContext *context,
    bool hasProtectedContent,
    const MemoryProperties &memoryProperties,
    gl::TextureType textureType,
    GLint samples,
    const ImageHelper &resolveImage,
    const VkExtent3D &multisampleImageExtents,
    bool isRobustResourceInitEnabled)
{
    ASSERT(!valid());
    ASSERT(samples > 1);
    ASSERT(!IsAnySubresourceContentDefined(mContentDefined));
    ASSERT(!IsAnySubresourceContentDefined(mStencilContentDefined));

    // The image is used as either color or depth/stencil attachment.  Additionally, its memory is
    // lazily allocated as the contents are discarded at the end of the renderpass and with tiling
    // GPUs no actual backing memory is required.
    //
    // Note that the Vulkan image is created with or without VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
    // based on whether the memory that will be used to create the image would have
    // VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT.  TRANSIENT is provided if there is any memory that
    // supports LAZILY_ALLOCATED.  However, based on actual image requirements, such a memory may
    // not be suitable for the image.  We don't support such a case, which will result in the
    // |initMemory| call below failing.
    const bool hasLazilyAllocatedMemory = memoryProperties.hasLazilyAllocatedMemory();

    const VkImageUsageFlags kLazyFlags =
        hasLazilyAllocatedMemory ? VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT : 0;
    constexpr VkImageUsageFlags kColorFlags =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

    // Request input attachment flag iff supportsShaderFramebufferFetchDepthStencil is enabled.
    const VkImageUsageFlags depthStencilFlags =
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
        ((context->getFeatures().supportsShaderFramebufferFetchDepthStencil.enabled)
             ? VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
             : 0);

    const VkImageUsageFlags kMultisampledUsageFlags =
        kLazyFlags |
        (resolveImage.getAspectFlags() == VK_IMAGE_ASPECT_COLOR_BIT ? kColorFlags
                                                                    : depthStencilFlags);
    const VkImageCreateFlags kMultisampledCreateFlags =
        hasProtectedContent ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;

    // Multisampled images have only 1 level
    constexpr uint32_t kLevelCount = 1;

    ANGLE_TRY(initExternal(context, textureType, multisampleImageExtents,
                           resolveImage.getIntendedFormatID(), resolveImage.getActualFormatID(),
                           samples, kMultisampledUsageFlags, kMultisampledCreateFlags,
                           ImageLayout::Undefined, nullptr, resolveImage.getFirstAllocatedLevel(),
                           kLevelCount, resolveImage.getLayerCount(), isRobustResourceInitEnabled,
                           hasProtectedContent, YcbcrConversionDesc{}, nullptr));

    // Remove the emulated format clear from the multisampled image if any.  There is one already
    // staged on the resolve image if needed.
    removeStagedUpdates(context, getFirstAllocatedLevel(), getLastAllocatedLevel());

    const VkMemoryPropertyFlags kMultisampledMemoryFlags =
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
        (hasLazilyAllocatedMemory ? VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT : 0) |
        (hasProtectedContent ? VK_MEMORY_PROPERTY_PROTECTED_BIT : 0);

    // If this ever fails, it can be retried without the LAZILY_ALLOCATED flag (which will probably
    // still fail), but ideally that means GL_EXT_multisampled_render_to_texture should not be
    // advertised on this platform in the first place.
    ANGLE_TRY(initMemoryAndNonZeroFillIfNeeded(
        context, hasProtectedContent, memoryProperties, kMultisampledMemoryFlags,
        vk::MemoryAllocationType::ImplicitMultisampledRenderToTextureImage));
    return angle::Result::Continue;
}

VkImageAspectFlags ImageHelper::getAspectFlags() const
{
    return GetFormatAspectFlags(angle::Format::Get(mActualFormatID));
}

bool ImageHelper::isCombinedDepthStencilFormat() const
{
    return (getAspectFlags() & kDepthStencilAspects) == kDepthStencilAspects;
}

void ImageHelper::setCurrentImageLayout(Renderer *renderer, ImageLayout newLayout)
{
    // Once you transition to ImageLayout::SharedPresent, you never transition out of it.
    if (mCurrentLayout == ImageLayout::SharedPresent)
    {
        return;
    }

    const ImageMemoryBarrierData &transitionFrom =
        renderer->getImageMemoryBarrierData(mCurrentLayout);
    const ImageMemoryBarrierData &transitionTo = renderer->getImageMemoryBarrierData(newLayout);
    mLastNonShaderReadOnlyLayout =
        !IsShaderReadOnlyLayout(transitionFrom) ? mCurrentLayout : mLastNonShaderReadOnlyLayout;
    // Force the use of BarrierType::Pipeline in the next barrierImpl call
    mLastNonShaderReadOnlyEvent.release(renderer);
    mCurrentShaderReadStageMask =
        IsShaderReadOnlyLayout(transitionTo) ? transitionTo.dstStageMask : 0;
    mCurrentLayout = newLayout;
}

VkImageLayout ImageHelper::getCurrentLayout(Renderer *renderer) const
{
    return ConvertImageLayoutToVkImageLayout(renderer, mCurrentLayout);
}

gl::Extents ImageHelper::getLevelExtents(LevelIndex levelVk) const
{
    // Level 0 should be the size of the extents, after that every time you increase a level
    // you shrink the extents by half.
    uint32_t width  = std::max(mExtents.width >> levelVk.get(), 1u);
    uint32_t height = std::max(mExtents.height >> levelVk.get(), 1u);
    uint32_t depth  = std::max(mExtents.depth >> levelVk.get(), 1u);

    return gl::Extents(width, height, depth);
}

gl::Extents ImageHelper::getLevelExtents2D(LevelIndex levelVk) const
{
    gl::Extents extents = getLevelExtents(levelVk);
    extents.depth       = 1;
    return extents;
}

const VkExtent3D ImageHelper::getRotatedExtents() const
{
    VkExtent3D extents = mExtents;
    if (mRotatedAspectRatio)
    {
        std::swap(extents.width, extents.height);
    }
    return extents;
}

gl::Extents ImageHelper::getRotatedLevelExtents2D(LevelIndex levelVk) const
{
    gl::Extents extents = getLevelExtents2D(levelVk);
    if (mRotatedAspectRatio)
    {
        std::swap(extents.width, extents.height);
    }
    return extents;
}

bool ImageHelper::isDepthOrStencil() const
{
    return getActualFormat().hasDepthOrStencilBits();
}

void ImageHelper::setRenderPassUsageFlag(RenderPassUsage flag)
{
    mRenderPassUsageFlags.set(flag);
}

void ImageHelper::clearRenderPassUsageFlag(RenderPassUsage flag)
{
    mRenderPassUsageFlags.reset(flag);
}

void ImageHelper::resetRenderPassUsageFlags()
{
    mRenderPassUsageFlags.reset();
}

bool ImageHelper::hasRenderPassUsageFlag(RenderPassUsage flag) const
{
    return mRenderPassUsageFlags.test(flag);
}

bool ImageHelper::usedByCurrentRenderPassAsAttachmentAndSampler(
    RenderPassUsage textureSamplerUsage) const
{
    return mRenderPassUsageFlags[RenderPassUsage::RenderTargetAttachment] &&
           mRenderPassUsageFlags[textureSamplerUsage];
}

bool ImageHelper::isReadBarrierNecessary(Renderer *renderer, ImageLayout newLayout) const
{
    // If transitioning to a different layout, we need always need a barrier.
    if (mCurrentLayout != newLayout)
    {
        return true;
    }

    // RAR (read-after-read) is not a hazard and doesn't require a barrier.
    //
    // RAW (read-after-write) hazards always require a memory barrier.  This can only happen if the
    // layout (same as new layout) is writable which in turn is only possible if the image is
    // simultaneously bound for shader write (i.e. the layout is GENERAL or SHARED_PRESENT).
    const ImageMemoryBarrierData &layoutData = renderer->getImageMemoryBarrierData(mCurrentLayout);
    return HasResourceWriteAccess(layoutData.type);
}

bool ImageHelper::isReadSubresourceBarrierNecessary(ImageLayout newLayout,
                                                    gl::LevelIndex levelStart,
                                                    uint32_t levelCount,
                                                    uint32_t layerStart,
                                                    uint32_t layerCount) const
{
    // In case an image has both read and write permissions, the written subresources since the last
    // barrier should be checked to avoid RAW and WAR hazards. However, if a layout change is
    // necessary regardless, there is no need to check the written subresources.
    if (mCurrentLayout != newLayout)
    {
        return true;
    }

    ImageLayerWriteMask layerMask = GetImageLayerWriteMask(layerStart, layerCount);
    for (uint32_t levelOffset = 0; levelOffset < levelCount; levelOffset++)
    {
        uint32_t level = levelStart.get() + levelOffset;
        if (areLevelSubresourcesWrittenWithinMaskRange(level, layerMask))
        {
            return true;
        }
    }

    return false;
}

bool ImageHelper::isWriteBarrierNecessary(ImageLayout newLayout,
                                          gl::LevelIndex levelStart,
                                          uint32_t levelCount,
                                          uint32_t layerStart,
                                          uint32_t layerCount) const
{
    // If transitioning to a different layout, we need always need a barrier.
    if (mCurrentLayout != newLayout)
    {
        return true;
    }

    if (layerCount >= kMaxParallelLayerWrites)
    {
        return true;
    }

    // If we are writing to the same parts of the image (level/layer), we need a barrier. Otherwise,
    // it can be done in parallel.
    ImageLayerWriteMask layerMask = GetImageLayerWriteMask(layerStart, layerCount);
    for (uint32_t levelOffset = 0; levelOffset < levelCount; levelOffset++)
    {
        uint32_t level = levelStart.get() + levelOffset;
        if (areLevelSubresourcesWrittenWithinMaskRange(level, layerMask))
        {
            return true;
        }
    }

    return false;
}

void ImageHelper::changeLayoutAndQueue(Context *context,
                                       VkImageAspectFlags aspectMask,
                                       ImageLayout newLayout,
                                       DeviceQueueIndex newDeviceQueueIndex,
                                       OutsideRenderPassCommandBuffer *commandBuffer)
{
    ASSERT(!mIsForeignImage);

    ASSERT(isQueueFamilyChangeNeccesary(newDeviceQueueIndex));
    VkSemaphore acquireNextImageSemaphore;
    // recordBarrierImpl should detect there is queue switch and fall back to pipelineBarrier
    // properly.
    recordBarrierImpl(context, aspectMask, newLayout, newDeviceQueueIndex, nullptr, commandBuffer,
                      &acquireNextImageSemaphore);
    // SwapChain image should not get here.
    ASSERT(acquireNextImageSemaphore == VK_NULL_HANDLE);
}

void ImageHelper::acquireFromExternal(Context *context,
                                      DeviceQueueIndex externalQueueIndex,
                                      DeviceQueueIndex newDeviceQueueIndex,
                                      ImageLayout currentLayout,
                                      OutsideRenderPassCommandBuffer *commandBuffer)
{
    // The image must be newly allocated or have been released to the external
    // queue. If this is not the case, it's an application bug, so ASSERT might
    // eventually need to change to a warning.
    ASSERT(mCurrentLayout == ImageLayout::ExternalPreInitialized ||
           mCurrentDeviceQueueIndex.familyIndex() == externalQueueIndex.familyIndex());

    mCurrentLayout           = currentLayout;
    mCurrentDeviceQueueIndex = externalQueueIndex;
    mIsReleasedToExternal    = false;

    // Only change the layout and queue if the layout is anything by Undefined.  If it is undefined,
    // leave it to transition out as the image is used later.
    if (currentLayout != ImageLayout::Undefined)
    {
        changeLayoutAndQueue(context, getAspectFlags(), mCurrentLayout, newDeviceQueueIndex,
                             commandBuffer);
    }

    // It is unknown how the external has modified the image, so assume every subresource has
    // defined content.  That is unless the layout is Undefined.
    if (currentLayout == ImageLayout::Undefined)
    {
        setEntireContentUndefined();
    }
    else
    {
        setEntireContentDefined();
    }
}

void ImageHelper::releaseToExternal(Context *context,
                                    DeviceQueueIndex externalQueueIndex,
                                    ImageLayout desiredLayout,
                                    OutsideRenderPassCommandBuffer *commandBuffer)
{
    ASSERT(!mIsReleasedToExternal);

    // A layout change is unnecessary if the image that was previously acquired was never used by
    // GL!
    if (mCurrentDeviceQueueIndex.familyIndex() != externalQueueIndex.familyIndex() ||
        mCurrentLayout != desiredLayout)
    {
        changeLayoutAndQueue(context, getAspectFlags(), desiredLayout, externalQueueIndex,
                             commandBuffer);
    }

    mIsReleasedToExternal = true;
}

VkImageMemoryBarrier ImageHelper::releaseToForeign(Renderer *renderer)
{
    ASSERT(mIsForeignImage);

    const ImageMemoryBarrierData barrierData = renderer->getImageMemoryBarrierData(mCurrentLayout);

    VkImageMemoryBarrier barrier        = {};
    barrier.sType                       = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcAccessMask               = barrierData.srcAccessMask;
    barrier.dstAccessMask               = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
    barrier.oldLayout                   = barrierData.layout;
    barrier.newLayout                   = VK_IMAGE_LAYOUT_GENERAL;
    barrier.srcQueueFamilyIndex         = renderer->getQueueFamilyIndex();
    barrier.dstQueueFamilyIndex         = VK_QUEUE_FAMILY_FOREIGN_EXT;
    barrier.image                       = mImage.getHandle();
    barrier.subresourceRange.aspectMask = getAspectFlags();
    barrier.subresourceRange.levelCount = mLevelCount;
    barrier.subresourceRange.layerCount = mLayerCount;

    mCurrentLayout               = ImageLayout::ForeignAccess;
    mCurrentDeviceQueueIndex     = kForeignDeviceQueueIndex;
    mLastNonShaderReadOnlyLayout = ImageLayout::Undefined;
    mCurrentShaderReadStageMask  = 0;

    return barrier;
}

LevelIndex ImageHelper::toVkLevel(gl::LevelIndex levelIndexGL) const
{
    return gl_vk::GetLevelIndex(levelIndexGL, mFirstAllocatedLevel);
}

gl::LevelIndex ImageHelper::toGLLevel(LevelIndex levelIndexVk) const
{
    return vk_gl::GetLevelIndex(levelIndexVk, mFirstAllocatedLevel);
}

ANGLE_INLINE void ImageHelper::initImageMemoryBarrierStruct(
    Renderer *renderer,
    VkImageAspectFlags aspectMask,
    ImageLayout newLayout,
    uint32_t newQueueFamilyIndex,
    VkImageMemoryBarrier *imageMemoryBarrier) const
{
    ASSERT(mCurrentDeviceQueueIndex.familyIndex() != QueueFamily::kInvalidIndex);
    ASSERT(newQueueFamilyIndex != QueueFamily::kInvalidIndex);

    const ImageMemoryBarrierData &transitionFrom =
        renderer->getImageMemoryBarrierData(mCurrentLayout);
    const ImageMemoryBarrierData &transitionTo = renderer->getImageMemoryBarrierData(newLayout);

    imageMemoryBarrier->sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier->srcAccessMask = transitionFrom.srcAccessMask;
    imageMemoryBarrier->dstAccessMask = transitionTo.dstAccessMask;
    imageMemoryBarrier->oldLayout     = ConvertImageLayoutToVkImageLayout(renderer, mCurrentLayout);
    imageMemoryBarrier->newLayout     = ConvertImageLayoutToVkImageLayout(renderer, newLayout);
    imageMemoryBarrier->srcQueueFamilyIndex = mCurrentDeviceQueueIndex.familyIndex();
    imageMemoryBarrier->dstQueueFamilyIndex = newQueueFamilyIndex;
    imageMemoryBarrier->image               = mImage.getHandle();

    // Transition the whole resource.
    imageMemoryBarrier->subresourceRange.aspectMask     = aspectMask;
    imageMemoryBarrier->subresourceRange.baseMipLevel   = 0;
    imageMemoryBarrier->subresourceRange.levelCount     = mLevelCount;
    imageMemoryBarrier->subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier->subresourceRange.layerCount     = mLayerCount;
}

// Generalized to accept both "primary" and "secondary" command buffers.
template <typename CommandBufferT>
void ImageHelper::barrierImpl(Renderer *renderer,
                              VkImageAspectFlags aspectMask,
                              ImageLayout newLayout,
                              DeviceQueueIndex newDeviceQueueIndex,
                              RefCountedEventCollector *eventCollector,
                              CommandBufferT *commandBuffer,
                              VkSemaphore *acquireNextImageSemaphoreOut)
{
    // Release the ANI semaphore to caller to add to the command submission.
    ASSERT(acquireNextImageSemaphoreOut != nullptr || !mAcquireNextImageSemaphore.valid());
    if (acquireNextImageSemaphoreOut != nullptr)
    {
        *acquireNextImageSemaphoreOut = mAcquireNextImageSemaphore.release();
    }

    if (mCurrentLayout == ImageLayout::SharedPresent)
    {
        const ImageMemoryBarrierData &transition =
            renderer->getImageMemoryBarrierData(mCurrentLayout);
        VkMemoryBarrier memoryBarrier = {};
        memoryBarrier.sType           = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.srcAccessMask   = transition.srcAccessMask;
        memoryBarrier.dstAccessMask   = transition.dstAccessMask;

        commandBuffer->memoryBarrier(transition.srcStageMask, transition.dstStageMask,
                                     memoryBarrier);
        return;
    }

    // Make sure we never transition out of SharedPresent
    ASSERT(mCurrentLayout != ImageLayout::SharedPresent || newLayout == ImageLayout::SharedPresent);

    const ImageMemoryBarrierData &transitionFrom =
        renderer->getImageMemoryBarrierData(mCurrentLayout);
    const ImageMemoryBarrierData &transitionTo = renderer->getImageMemoryBarrierData(newLayout);

    VkImageMemoryBarrier imageMemoryBarrier = {};
    initImageMemoryBarrierStruct(renderer, aspectMask, newLayout, newDeviceQueueIndex.familyIndex(),
                                 &imageMemoryBarrier);

    VkPipelineStageFlags dstStageMask = transitionTo.dstStageMask;

    // Fallback to pipelineBarrier if there is no event tracking image.
    // VkCmdWaitEvent requires the srcQueueFamilyIndex and dstQueueFamilyIndex members of any
    // element of pBufferMemoryBarriers or pImageMemoryBarriers must be equal
    // (VUID-vkCmdWaitEvents-srcQueueFamilyIndex-02803).
    BarrierType barrierType =
        mCurrentEvent.valid() && mCurrentDeviceQueueIndex == newDeviceQueueIndex
            ? BarrierType::Event
            : BarrierType::Pipeline;

    if (barrierType == BarrierType::Event)
    {
        // If there is an event, we use the waitEvent to do layout change. Once we have waited, the
        // event gets garbage collected (which is GPU completion tracked) to avoid waited again in
        // future. We always use DstStageMask since that is what setEvent used and
        // VUID-vkCmdWaitEvents-srcStageMask-01158 requires they must match.
        VkPipelineStageFlags srcStageMask =
            renderer->getPipelineStageMask(mCurrentEvent.getEventStage());
        commandBuffer->imageWaitEvent(mCurrentEvent.getEvent().getHandle(), srcStageMask,
                                      dstStageMask, imageMemoryBarrier);
        eventCollector->emplace_back(std::move(mCurrentEvent));
    }
    else
    {
        // There might be other shaderRead operations there other than the current layout.
        VkPipelineStageFlags srcStageMask = transitionFrom.srcStageMask;
        if (mCurrentShaderReadStageMask)
        {
            srcStageMask |= mCurrentShaderReadStageMask;
            mCurrentShaderReadStageMask  = 0;
            mLastNonShaderReadOnlyLayout = ImageLayout::Undefined;
        }
        commandBuffer->imageBarrier(srcStageMask, dstStageMask, imageMemoryBarrier);
    }

    mCurrentLayout           = newLayout;
    mCurrentDeviceQueueIndex = newDeviceQueueIndex;
    resetSubresourcesWrittenSinceBarrier();
}

template <typename CommandBufferT>
void ImageHelper::recordBarrierImpl(Context *context,
                                    VkImageAspectFlags aspectMask,
                                    ImageLayout newLayout,
                                    DeviceQueueIndex newDeviceQueueIndex,
                                    RefCountedEventCollector *eventCollector,
                                    CommandBufferT *commandBuffer,
                                    VkSemaphore *acquireNextImageSemaphoreOut)
{
    Renderer *renderer = context->getRenderer();
    // mCurrentEvent must be invalid if useVkEventForImageBarrieris disabled.
    ASSERT(renderer->getFeatures().useVkEventForImageBarrier.enabled || !mCurrentEvent.valid());

    if (mCurrentLayout == ImageLayout::SharedPresent)
    {
        // For now we always use pipelineBarrier for singlebuffer mode. We could use event here in
        // future.
        mCurrentEvent.release(context);
    }

    // The image has transitioned out of the FOREIGN queue.  Remember it so it can be transitioned
    // back on submission.
    if (mCurrentDeviceQueueIndex == kForeignDeviceQueueIndex)
    {
        context->onForeignImageUse(this);
    }

    barrierImpl(renderer, aspectMask, newLayout, newDeviceQueueIndex, eventCollector, commandBuffer,
                acquireNextImageSemaphoreOut);

    // We must release the event so that new event will be created and added. If we did not add new
    // event, because mCurrentEvent have been released, next barrier will automatically fallback to
    // pipelineBarrier. Otherwise if we keep mCurrentEvent here we may accidentally end up waiting
    // for an old event which creates sync hazard.
    mCurrentEvent.release(context);
}

void ImageHelper::recordBarrierOneOffImpl(Renderer *renderer,
                                          VkImageAspectFlags aspectMask,
                                          ImageLayout newLayout,
                                          DeviceQueueIndex newDeviceQueueIndex,
                                          PrimaryCommandBuffer *commandBuffer,
                                          VkSemaphore *acquireNextImageSemaphoreOut)
{
    // Release the event here to force pipelineBarrier.
    mCurrentEvent.release(renderer);
    ASSERT(mCurrentDeviceQueueIndex != kForeignDeviceQueueIndex);

    barrierImpl(renderer, aspectMask, newLayout, newDeviceQueueIndex, nullptr, commandBuffer,
                acquireNextImageSemaphoreOut);
}

void ImageHelper::setSubresourcesWrittenSinceBarrier(gl::LevelIndex levelStart,
                                                     uint32_t levelCount,
                                                     uint32_t layerStart,
                                                     uint32_t layerCount)
{
    for (uint32_t levelOffset = 0; levelOffset < levelCount; levelOffset++)
    {
        uint32_t level = levelStart.get() + levelOffset;
        if (layerCount >= kMaxParallelLayerWrites)
        {
            mSubresourcesWrittenSinceBarrier[level].set();
        }
        else
        {
            ImageLayerWriteMask layerMask = GetImageLayerWriteMask(layerStart, layerCount);
            mSubresourcesWrittenSinceBarrier[level] |= layerMask;
        }
    }
}

void ImageHelper::resetSubresourcesWrittenSinceBarrier()
{
    for (auto &layerWriteMask : mSubresourcesWrittenSinceBarrier)
    {
        layerWriteMask.reset();
    }
}

void ImageHelper::recordWriteBarrier(Context *context,
                                     VkImageAspectFlags aspectMask,
                                     ImageLayout newLayout,
                                     gl::LevelIndex levelStart,
                                     uint32_t levelCount,
                                     uint32_t layerStart,
                                     uint32_t layerCount,
                                     OutsideRenderPassCommandBufferHelper *commands)
{
    if (isWriteBarrierNecessary(newLayout, levelStart, levelCount, layerStart, layerCount))
    {
        ASSERT(!mCurrentEvent.valid() || !commands->hasSetEventPendingFlush(mCurrentEvent));
        VkSemaphore acquireNextImageSemaphore;
        recordBarrierImpl(context, aspectMask, newLayout, context->getDeviceQueueIndex(),
                          commands->getRefCountedEventCollector(), &commands->getCommandBuffer(),
                          &acquireNextImageSemaphore);

        if (acquireNextImageSemaphore != VK_NULL_HANDLE)
        {
            commands->setAcquireNextImageSemaphore(acquireNextImageSemaphore);
        }
    }

    setSubresourcesWrittenSinceBarrier(levelStart, levelCount, layerStart, layerCount);
}

void ImageHelper::recordReadSubresourceBarrier(Context *context,
                                               VkImageAspectFlags aspectMask,
                                               ImageLayout newLayout,
                                               gl::LevelIndex levelStart,
                                               uint32_t levelCount,
                                               uint32_t layerStart,
                                               uint32_t layerCount,
                                               OutsideRenderPassCommandBufferHelper *commands)
{
    // This barrier is used for an image with both read/write permissions, including during mipmap
    // generation and self-copy.
    if (isReadSubresourceBarrierNecessary(newLayout, levelStart, levelCount, layerStart,
                                          layerCount))
    {
        ASSERT(!mCurrentEvent.valid() || !commands->hasSetEventPendingFlush(mCurrentEvent));
        VkSemaphore acquireNextImageSemaphore;
        recordBarrierImpl(context, aspectMask, newLayout, context->getDeviceQueueIndex(),
                          commands->getRefCountedEventCollector(), &commands->getCommandBuffer(),
                          &acquireNextImageSemaphore);

        if (acquireNextImageSemaphore != VK_NULL_HANDLE)
        {
            commands->setAcquireNextImageSemaphore(acquireNextImageSemaphore);
        }
    }

    // Levels/layers being read from are also registered to avoid RAW and WAR hazards.
    setSubresourcesWrittenSinceBarrier(levelStart, levelCount, layerStart, layerCount);
}

void ImageHelper::recordReadBarrier(Context *context,
                                    VkImageAspectFlags aspectMask,
                                    ImageLayout newLayout,
                                    OutsideRenderPassCommandBufferHelper *commands)
{
    if (!isReadBarrierNecessary(context->getRenderer(), newLayout))
    {
        return;
    }

    ASSERT(!mCurrentEvent.valid() || !commands->hasSetEventPendingFlush(mCurrentEvent));
    VkSemaphore acquireNextImageSemaphore;
    recordBarrierImpl(context, aspectMask, newLayout, context->getDeviceQueueIndex(),
                      commands->getRefCountedEventCollector(), &commands->getCommandBuffer(),
                      &acquireNextImageSemaphore);

    if (acquireNextImageSemaphore != VK_NULL_HANDLE)
    {
        commands->setAcquireNextImageSemaphore(acquireNextImageSemaphore);
    }
}

void ImageHelper::updateLayoutAndBarrier(Context *context,
                                         VkImageAspectFlags aspectMask,
                                         ImageLayout newLayout,
                                         BarrierType barrierType,
                                         const QueueSerial &queueSerial,
                                         PipelineBarrierArray *pipelineBarriers,
                                         EventBarrierArray *eventBarriers,
                                         RefCountedEventCollector *eventCollector,
                                         VkSemaphore *semaphoreOut)
{
    Renderer *renderer = context->getRenderer();
    ASSERT(queueSerial.valid());
    ASSERT(!mBarrierQueueSerial.valid() ||
           mBarrierQueueSerial.getIndex() != queueSerial.getIndex() ||
           mBarrierQueueSerial.getSerial() <= queueSerial.getSerial());
    ASSERT(renderer->getImageMemoryBarrierData(newLayout).barrierIndex !=
           PipelineStage::InvalidEnum);
    // mCurrentEvent must be invalid if useVkEventForImageBarrieris disabled.
    ASSERT(renderer->getFeatures().useVkEventForImageBarrier.enabled || !mCurrentEvent.valid());

    const bool hasQueueChange = mCurrentDeviceQueueIndex != context->getDeviceQueueIndex();
    if (hasQueueChange)
    {
        // Fallback to pipelineBarrier if the VkQueue has changed.
        barrierType              = BarrierType::Pipeline;
        if (mCurrentDeviceQueueIndex == kForeignDeviceQueueIndex)
        {
            context->onForeignImageUse(this);
        }
    }
    else if (!mCurrentEvent.valid())
    {
        // Fallback to pipelineBarrier if there is no event tracking image.
        barrierType = BarrierType::Pipeline;
    }

    // Once you transition to ImageLayout::SharedPresent, you never transition out of it.
    if (mCurrentLayout == ImageLayout::SharedPresent)
    {
        newLayout = ImageLayout::SharedPresent;
    }

    if (newLayout == mCurrentLayout && !hasQueueChange)
    {
        if (mBarrierQueueSerial == queueSerial)
        {
            ASSERT(!mAcquireNextImageSemaphore.valid());
            // If there is no layout change and the previous layout change happened in the same
            // render pass, then early out do nothing. This can happen when the same image is
            // attached to the multiple attachments of the framebuffer.
            return;
        }

        const ImageMemoryBarrierData &layoutData =
            renderer->getImageMemoryBarrierData(mCurrentLayout);
        // RAR is not a hazard and doesn't require a barrier, especially as the image layout hasn't
        // changed.  The following asserts that such a barrier is not attempted.
        ASSERT(HasResourceWriteAccess(layoutData.type));

        // No layout change, only memory barrier is required
        if (barrierType == BarrierType::Event)
        {
            eventBarriers->addEventMemoryBarrier(renderer, mCurrentEvent, layoutData.dstAccessMask,
                                                 layoutData.dstStageMask, layoutData.dstAccessMask);
            // Garbage collect the event, which tracks GPU completion automatically.
            eventCollector->emplace_back(std::move(mCurrentEvent));
        }
        else
        {
            pipelineBarriers->mergeMemoryBarrier(layoutData.barrierIndex, layoutData.dstStageMask,
                                                 layoutData.dstStageMask, layoutData.srcAccessMask,
                                                 layoutData.dstAccessMask);

            // Release it. No need to garbage collect since we did not use the event here. ALl
            // previous use of event should garbage tracked already.
            mCurrentEvent.release(context);
        }
        mBarrierQueueSerial = queueSerial;
    }
    else
    {
        const ImageMemoryBarrierData &transitionFrom =
            renderer->getImageMemoryBarrierData(mCurrentLayout);
        const ImageMemoryBarrierData &transitionTo = renderer->getImageMemoryBarrierData(newLayout);
        VkPipelineStageFlags srcStageMask          = transitionFrom.srcStageMask;
        VkPipelineStageFlags dstStageMask          = transitionTo.dstStageMask;

        if (transitionFrom.layout == transitionTo.layout && IsShaderReadOnlyLayout(transitionTo) &&
            mBarrierQueueSerial == queueSerial && !hasQueueChange)
        {
            // If we are switching between different shader stage reads of the same render pass,
            // then there is no actual layout change or access type change. We only need a barrier
            // if we are making a read that is from a new stage. Also note that we do barrier
            // against previous non-shaderRead layout. We do not barrier between one shaderRead and
            // another shaderRead.
            bool isNewReadStage = (mCurrentShaderReadStageMask & dstStageMask) != dstStageMask;
            if (!isNewReadStage)
            {
                ASSERT(!mAcquireNextImageSemaphore.valid());
                return;
            }

            ASSERT(!mLastNonShaderReadOnlyEvent.valid() ||
                   mLastNonShaderReadOnlyEvent.getEventStage() ==
                       GetImageLayoutEventStage(mLastNonShaderReadOnlyLayout));
            if (!mLastNonShaderReadOnlyEvent.valid())
            {
                barrierType = BarrierType::Pipeline;
            }

            if (barrierType == BarrierType::Event)
            {
                // If we already inserted a barrier in the same renderPass, we has to add
                // the new stage mask to the existing VkCmdWaitEvent call, otherwise VVL will
                // complain.
                eventBarriers->addAdditionalStageAccess(mLastNonShaderReadOnlyEvent, dstStageMask,
                                                        transitionTo.dstAccessMask);
                eventCollector->emplace_back(mLastNonShaderReadOnlyEvent);
            }
            else
            {
                const ImageMemoryBarrierData &layoutData =
                    renderer->getImageMemoryBarrierData(mLastNonShaderReadOnlyLayout);
                pipelineBarriers->mergeMemoryBarrier(
                    transitionTo.barrierIndex, layoutData.srcStageMask, dstStageMask,
                    layoutData.srcAccessMask, transitionTo.dstAccessMask);
            }

            mBarrierQueueSerial = queueSerial;
            // Accumulate new read stage.
            mCurrentShaderReadStageMask |= dstStageMask;

            // Since we used pipelineBarrier, release the event now to avoid wait for the
            // event again.
            if (mCurrentEvent.valid())
            {
                eventCollector->emplace_back(std::move(mCurrentEvent));
            }
        }
        else
        {
            VkImageMemoryBarrier imageMemoryBarrier = {};
            initImageMemoryBarrierStruct(renderer, aspectMask, newLayout,
                                         context->getDeviceQueueIndex().familyIndex(),
                                         &imageMemoryBarrier);

            if (transitionFrom.layout == transitionTo.layout &&
                IsShaderReadOnlyLayout(transitionTo))
            {
                // If we are transiting within shaderReadOnly layout, i.e. reading from different
                // shader stages, VkEvent can't handle this right now. In order for VkEvent to
                // handle this properly we have to wait for the previous shaderReadOnly layout
                // transition event and add a new memoryBarrier. But we may have lost that event
                // already if it has been used in a new render pass (because we have to update the
                // event even if there is no barrier needed). To workaround this issue we fall back
                // to pipelineBarrier for now.
                barrierType = BarrierType::Pipeline;
            }
            else if (mBarrierQueueSerial == queueSerial)
            {
                // If we already inserted a barrier in this render pass, force to use
                // pipelineBarrier. Otherwise we will end up inserting a VkCmdWaitEvent that has not
                // been set (See https://issuetracker.google.com/333419317 for example).
                barrierType = BarrierType::Pipeline;
            }

            // if we transition from shaderReadOnly, we must add in stashed shader stage masks since
            // there might be outstanding shader reads from stages other than current layout. We do
            // not insert barrier between one shaderRead to another shaderRead
            if (mCurrentShaderReadStageMask)
            {
                if ((mCurrentShaderReadStageMask & srcStageMask) != mCurrentShaderReadStageMask)
                {
                    // mCurrentShaderReadStageMask has more bits than srcStageMask. This means it
                    // has been used by more than one shader stage in the same render pass. These
                    // two usages are tracked by two different ImageLayout, even though underline
                    // VkImageLayout is the same. This means two different RefCountedEvents since
                    // each RefCountedEvent is associated with one ImageLayout. When we transit out
                    // of this layout, we must wait for all reads to finish. But Right now
                    // ImageHelper only keep track of the last read. To workaround this problem we
                    // use pipelineBarrier in this case.
                    barrierType = BarrierType::Pipeline;
                    srcStageMask |= mCurrentShaderReadStageMask;
                }
                mCurrentShaderReadStageMask  = 0;
                mLastNonShaderReadOnlyLayout = ImageLayout::Undefined;
                if (mLastNonShaderReadOnlyEvent.valid())
                {
                    mLastNonShaderReadOnlyEvent.release(context);
                }
            }

            // If we are transition into shaderRead layout, remember the last
            // non-shaderRead layout here.
            const bool isShaderReadOnly = IsShaderReadOnlyLayout(transitionTo);
            if (isShaderReadOnly)
            {
                mLastNonShaderReadOnlyEvent.release(context);
                mLastNonShaderReadOnlyLayout = mCurrentLayout;
                mCurrentShaderReadStageMask  = dstStageMask;
            }

            if (barrierType == BarrierType::Event)
            {
                eventBarriers->addEventImageBarrier(renderer, mCurrentEvent, dstStageMask,
                                                    imageMemoryBarrier);
                if (isShaderReadOnly)
                {
                    mLastNonShaderReadOnlyEvent = mCurrentEvent;
                }
                eventCollector->emplace_back(std::move(mCurrentEvent));
            }
            else
            {
                pipelineBarriers->mergeImageBarrier(transitionTo.barrierIndex, srcStageMask,
                                                    dstStageMask, imageMemoryBarrier);
                mCurrentEvent.release(context);
            }

            mBarrierQueueSerial = queueSerial;
        }
        mCurrentLayout = newLayout;
    }

    mCurrentDeviceQueueIndex = context->getDeviceQueueIndex();

    *semaphoreOut = mAcquireNextImageSemaphore.release();
    // We must release the event so that new event will be created and added. If we did not add new
    // event, because mCurrentEvent have been released, next barrier will automatically fallback to
    // pipelineBarrier. Otherwise if we keep mCurrentEvent here we may accidentally end up waiting
    // for an old event which creates sync hazard.
    ASSERT(!mCurrentEvent.valid());
}

void ImageHelper::setCurrentRefCountedEvent(Context *context,
                                            RefCountedEventArray *refCountedEventArray)
{
    ASSERT(context->getFeatures().useVkEventForImageBarrier.enabled);

    // If there is already an event, release it first.
    mCurrentEvent.release(context);

    // VkCmdSetEvent can remove the unnecessary GPU pipeline bubble that comes from false dependency
    // between fragment and vertex/transfer/compute stages. But it also comes with higher overhead.
    // In order to strike the balance, we exclude the images that are only used by one group of
    // pipeline stages in the past N references, where N is the heuristic window that we keep track
    // of. Use of VkEvent will not be beneficial if it is only accessed by one group of stages since
    // execution within the group is expected to be non-overlap.
    if (mPipelineStageAccessHeuristic == kPipelineStageAccessFragmentOnly ||
        mPipelineStageAccessHeuristic == kPipelineStageAccessPreFragmentOnly ||
        mPipelineStageAccessHeuristic == kPipelineStageAccessComputeOnly)
    {
        return;
    }

    // Create the event if we have not yet so. Otherwise just use the already created event. This
    // means all images used in the same render pass that has the same layout will be tracked by the
    // same event.
    EventStage eventStage = GetImageLayoutEventStage(mCurrentLayout);
    if (!refCountedEventArray->getEvent(eventStage).valid() &&
        !refCountedEventArray->initEventAtStage(context, eventStage))
    {
        // If VkEvent creation fail, we fallback to pipelineBarrier
        return;
    }

    // Copy the event to mCurrentEvent so that we can wait for it in future. This will add extra
    // refcount to the underlying VkEvent.
    mCurrentEvent = refCountedEventArray->getEvent(eventStage);
}

void ImageHelper::updatePipelineStageAccessHistory()
{
    const ImageMemoryBarrierData &barrierData = kImageMemoryBarrierData[mCurrentLayout];
    mPipelineStageAccessHeuristic.onAccess(barrierData.pipelineStageGroup);
}

void ImageHelper::clearColor(Renderer *renderer,
                             const VkClearColorValue &color,
                             LevelIndex baseMipLevelVk,
                             uint32_t levelCount,
                             uint32_t baseArrayLayer,
                             uint32_t layerCount,
                             OutsideRenderPassCommandBuffer *commandBuffer)
{
    ASSERT(valid());

    ASSERT(mCurrentLayout == ImageLayout::TransferDst ||
           mCurrentLayout == ImageLayout::SharedPresent);

    VkImageSubresourceRange range = {};
    range.aspectMask              = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel            = baseMipLevelVk.get();
    range.levelCount              = levelCount;
    range.baseArrayLayer          = baseArrayLayer;
    range.layerCount              = layerCount;

    if (mImageType == VK_IMAGE_TYPE_3D)
    {
        ASSERT(baseArrayLayer == 0);
        ASSERT(layerCount == 1 ||
               layerCount == static_cast<uint32_t>(getLevelExtents(baseMipLevelVk).depth));
        range.layerCount = 1;
    }

    commandBuffer->clearColorImage(mImage, getCurrentLayout(renderer), color, 1, &range);
}

void ImageHelper::clearDepthStencil(Renderer *renderer,
                                    VkImageAspectFlags clearAspectFlags,
                                    const VkClearDepthStencilValue &depthStencil,
                                    LevelIndex baseMipLevelVk,
                                    uint32_t levelCount,
                                    uint32_t baseArrayLayer,
                                    uint32_t layerCount,
                                    OutsideRenderPassCommandBuffer *commandBuffer)
{
    ASSERT(valid());

    ASSERT(mCurrentLayout == ImageLayout::TransferDst);

    VkImageSubresourceRange range = {};
    range.aspectMask              = clearAspectFlags;
    range.baseMipLevel            = baseMipLevelVk.get();
    range.levelCount              = levelCount;
    range.baseArrayLayer          = baseArrayLayer;
    range.layerCount              = layerCount;

    if (mImageType == VK_IMAGE_TYPE_3D)
    {
        ASSERT(baseArrayLayer == 0);
        ASSERT(layerCount == 1 ||
               layerCount == static_cast<uint32_t>(getLevelExtents(baseMipLevelVk).depth));
        range.layerCount = 1;
    }

    commandBuffer->clearDepthStencilImage(mImage, getCurrentLayout(renderer), depthStencil, 1,
                                          &range);
}

void ImageHelper::clear(Renderer *renderer,
                        VkImageAspectFlags aspectFlags,
                        const VkClearValue &value,
                        LevelIndex mipLevel,
                        uint32_t baseArrayLayer,
                        uint32_t layerCount,
                        OutsideRenderPassCommandBuffer *commandBuffer)
{
    const angle::Format &angleFormat = getActualFormat();
    bool isDepthStencil              = angleFormat.hasDepthOrStencilBits();

    if (isDepthStencil)
    {
        clearDepthStencil(renderer, aspectFlags, value.depthStencil, mipLevel, 1, baseArrayLayer,
                          layerCount, commandBuffer);
    }
    else
    {
        ASSERT(!angleFormat.isBlock);

        clearColor(renderer, value.color, mipLevel, 1, baseArrayLayer, layerCount, commandBuffer);
    }
}

angle::Result ImageHelper::clearEmulatedChannels(ContextVk *contextVk,
                                                 VkColorComponentFlags colorMaskFlags,
                                                 const VkClearValue &value,
                                                 LevelIndex mipLevel,
                                                 uint32_t baseArrayLayer,
                                                 uint32_t layerCount)
{
    const gl::Extents levelExtents = getLevelExtents(mipLevel);

    if (levelExtents.depth > 1)
    {
        // Currently not implemented for 3D textures
        UNIMPLEMENTED();
        return angle::Result::Continue;
    }

    UtilsVk::ClearImageParameters params = {};
    params.clearArea                     = {0, 0, levelExtents.width, levelExtents.height};
    params.dstMip                        = mipLevel;
    params.colorMaskFlags                = colorMaskFlags;
    params.colorClearValue               = value.color;

    for (uint32_t layerIndex = 0; layerIndex < layerCount; ++layerIndex)
    {
        params.dstLayer = baseArrayLayer + layerIndex;

        ANGLE_TRY(contextVk->getUtils().clearImage(contextVk, this, params));
    }

    return angle::Result::Continue;
}

// static
void ImageHelper::Copy(Renderer *renderer,
                       ImageHelper *srcImage,
                       ImageHelper *dstImage,
                       const gl::Offset &srcOffset,
                       const gl::Offset &dstOffset,
                       const gl::Extents &copySize,
                       const VkImageSubresourceLayers &srcSubresource,
                       const VkImageSubresourceLayers &dstSubresource,
                       OutsideRenderPassCommandBuffer *commandBuffer)
{
    ASSERT(commandBuffer->valid() && srcImage->valid() && dstImage->valid());

    ASSERT(srcImage->getCurrentLayout(renderer) == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    ASSERT(dstImage->getCurrentLayout(renderer) == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageCopy region    = {};
    region.srcSubresource = srcSubresource;
    region.srcOffset.x    = srcOffset.x;
    region.srcOffset.y    = srcOffset.y;
    region.srcOffset.z    = srcOffset.z;
    region.dstSubresource = dstSubresource;
    region.dstOffset.x    = dstOffset.x;
    region.dstOffset.y    = dstOffset.y;
    region.dstOffset.z    = dstOffset.z;
    region.extent.width   = copySize.width;
    region.extent.height  = copySize.height;
    region.extent.depth   = copySize.depth;

    commandBuffer->copyImage(srcImage->getImage(), srcImage->getCurrentLayout(renderer),
                             dstImage->getImage(), dstImage->getCurrentLayout(renderer), 1,
                             &region);
}

// static
angle::Result ImageHelper::CopyImageSubData(const gl::Context *context,
                                            ImageHelper *srcImage,
                                            GLint srcLevel,
                                            GLint srcX,
                                            GLint srcY,
                                            GLint srcZ,
                                            ImageHelper *dstImage,
                                            GLint dstLevel,
                                            GLint dstX,
                                            GLint dstY,
                                            GLint dstZ,
                                            GLsizei srcWidth,
                                            GLsizei srcHeight,
                                            GLsizei srcDepth)
{
    ContextVk *contextVk = GetImpl(context);
    Renderer *renderer   = contextVk->getRenderer();

    VkImageTiling srcTilingMode  = srcImage->getTilingMode();
    VkImageTiling destTilingMode = dstImage->getTilingMode();

    const gl::LevelIndex srcLevelGL = gl::LevelIndex(srcLevel);
    const gl::LevelIndex dstLevelGL = gl::LevelIndex(dstLevel);

    if (CanCopyWithTransferForCopyImage(renderer, srcImage, srcTilingMode, dstImage,
                                        destTilingMode))
    {
        bool isSrc3D                         = srcImage->getType() == VK_IMAGE_TYPE_3D;
        bool isDst3D                         = dstImage->getType() == VK_IMAGE_TYPE_3D;
        const VkImageAspectFlags aspectFlags = srcImage->getAspectFlags();

        ASSERT(srcImage->getAspectFlags() == dstImage->getAspectFlags());

        VkImageCopy region = {};

        region.srcSubresource.aspectMask     = aspectFlags;
        region.srcSubresource.mipLevel       = srcImage->toVkLevel(srcLevelGL).get();
        region.srcSubresource.baseArrayLayer = isSrc3D ? 0 : srcZ;
        region.srcSubresource.layerCount     = isSrc3D ? 1 : srcDepth;

        region.dstSubresource.aspectMask     = aspectFlags;
        region.dstSubresource.mipLevel       = dstImage->toVkLevel(dstLevelGL).get();
        region.dstSubresource.baseArrayLayer = isDst3D ? 0 : dstZ;
        region.dstSubresource.layerCount     = isDst3D ? 1 : srcDepth;

        region.srcOffset.x   = srcX;
        region.srcOffset.y   = srcY;
        region.srcOffset.z   = isSrc3D ? srcZ : 0;
        region.dstOffset.x   = dstX;
        region.dstOffset.y   = dstY;
        region.dstOffset.z   = isDst3D ? dstZ : 0;
        region.extent.width  = srcWidth;
        region.extent.height = srcHeight;
        region.extent.depth  = (isSrc3D || isDst3D) ? srcDepth : 1;

        CommandBufferAccess access;
        if (srcImage == dstImage)
        {
            access.onImageSelfCopy(srcLevelGL, 1, region.srcSubresource.baseArrayLayer,
                                   region.srcSubresource.layerCount, dstLevelGL, 1,
                                   region.dstSubresource.baseArrayLayer,
                                   region.dstSubresource.layerCount, aspectFlags, srcImage);
        }
        else
        {
            access.onImageTransferRead(aspectFlags, srcImage);
            access.onImageTransferWrite(dstLevelGL, 1, region.dstSubresource.baseArrayLayer,
                                        region.dstSubresource.layerCount, aspectFlags, dstImage);
        }

        OutsideRenderPassCommandBuffer *commandBuffer;
        ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

        ASSERT(srcImage->valid() && dstImage->valid());
        ASSERT(srcImage->getCurrentLayout(renderer) == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL ||
               srcImage->getCurrentLayout(renderer) == VK_IMAGE_LAYOUT_GENERAL);
        ASSERT(dstImage->getCurrentLayout(renderer) == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ||
               dstImage->getCurrentLayout(renderer) == VK_IMAGE_LAYOUT_GENERAL);

        commandBuffer->copyImage(srcImage->getImage(), srcImage->getCurrentLayout(renderer),
                                 dstImage->getImage(), dstImage->getCurrentLayout(renderer), 1,
                                 &region);
    }
    else if (!srcImage->getIntendedFormat().isBlock && !dstImage->getIntendedFormat().isBlock)
    {
        // The source and destination image formats may be using a fallback in the case of RGB
        // images.  A compute shader is used in such a case to perform the copy.
        UtilsVk &utilsVk = contextVk->getUtils();

        UtilsVk::CopyImageBitsParameters params;
        params.srcOffset[0]   = srcX;
        params.srcOffset[1]   = srcY;
        params.srcOffset[2]   = srcZ;
        params.srcLevel       = srcLevelGL;
        params.dstOffset[0]   = dstX;
        params.dstOffset[1]   = dstY;
        params.dstOffset[2]   = dstZ;
        params.dstLevel       = dstLevelGL;
        params.copyExtents[0] = srcWidth;
        params.copyExtents[1] = srcHeight;
        params.copyExtents[2] = srcDepth;

        ANGLE_TRY(utilsVk.copyImageBits(contextVk, dstImage, srcImage, params));
    }
    else
    {
        // No support for emulated compressed formats.
        UNIMPLEMENTED();
        ANGLE_VK_CHECK(contextVk, false, VK_ERROR_FEATURE_NOT_PRESENT);
    }

    return angle::Result::Continue;
}

angle::Result ImageHelper::generateMipmapsWithBlit(ContextVk *contextVk,
                                                   LevelIndex baseLevel,
                                                   LevelIndex maxLevel)
{
    Renderer *renderer = contextVk->getRenderer();

    CommandBufferAccess access;
    gl::LevelIndex baseLevelGL = toGLLevel(baseLevel);
    access.onImageTransferWrite(baseLevelGL + 1, maxLevel.get(), 0, mLayerCount,
                                VK_IMAGE_ASPECT_COLOR_BIT, this);

    OutsideRenderPassCommandBuffer *commandBuffer;
    ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

    // We are able to use blitImage since the image format we are using supports it.
    int32_t mipWidth  = mExtents.width;
    int32_t mipHeight = mExtents.height;
    int32_t mipDepth  = mExtents.depth;

    // Manually manage the image memory barrier because it uses a lot more parameters than our
    // usual one.
    VkImageMemoryBarrier barrier            = {};
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image                           = mImage.getHandle();
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = mLayerCount;
    barrier.subresourceRange.levelCount     = 1;

    const VkFilter filter =
        gl_vk::GetFilter(CalculateGenerateMipmapFilter(contextVk, getActualFormatID()));

    for (LevelIndex mipLevel(1); mipLevel <= LevelIndex(mLevelCount); ++mipLevel)
    {
        int32_t nextMipWidth  = std::max<int32_t>(1, mipWidth >> 1);
        int32_t nextMipHeight = std::max<int32_t>(1, mipHeight >> 1);
        int32_t nextMipDepth  = std::max<int32_t>(1, mipDepth >> 1);

        if (mipLevel > baseLevel && mipLevel <= maxLevel)
        {
            barrier.subresourceRange.baseMipLevel = mipLevel.get() - 1;
            barrier.oldLayout                     = getCurrentLayout(renderer);
            barrier.newLayout                     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask                 = VK_ACCESS_TRANSFER_READ_BIT;

            // We can do it for all layers at once.
            commandBuffer->imageBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
                                        VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);
            VkImageBlit blit                   = {};
            blit.srcOffsets[0]                 = {0, 0, 0};
            blit.srcOffsets[1]                 = {mipWidth, mipHeight, mipDepth};
            blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel       = mipLevel.get() - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount     = mLayerCount;
            blit.dstOffsets[0]                 = {0, 0, 0};
            blit.dstOffsets[1]                 = {nextMipWidth, nextMipHeight, nextMipDepth};
            blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel       = mipLevel.get();
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount     = mLayerCount;

            commandBuffer->blitImage(mImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mImage,
                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, filter);
        }
        mipWidth  = nextMipWidth;
        mipHeight = nextMipHeight;
        mipDepth  = nextMipDepth;
    }

    // Transition all mip level to the same layout so we can declare our whole image layout to one
    // ImageLayout. FragmentShaderReadOnly is picked here since this is the most reasonable usage
    // after glGenerateMipmap call.
    barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    if (baseLevel.get() > 0)
    {
        // [0:baseLevel-1] from TRANSFER_DST to SHADER_READ
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount   = baseLevel.get();
        commandBuffer->imageBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);
    }
    // [maxLevel:mLevelCount-1] from TRANSFER_DST to SHADER_READ
    ASSERT(mLevelCount > maxLevel.get());
    barrier.subresourceRange.baseMipLevel = maxLevel.get();
    barrier.subresourceRange.levelCount   = mLevelCount - maxLevel.get();
    commandBuffer->imageBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
                                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);
    // [baseLevel:maxLevel-1] from TRANSFER_SRC to SHADER_READ
    barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.subresourceRange.baseMipLevel = baseLevel.get();
    barrier.subresourceRange.levelCount   = maxLevel.get() - baseLevel.get();
    commandBuffer->imageBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
                                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);

    // This is just changing the internal state of the image helper so that the next call
    // to changeLayout will use this layout as the "oldLayout" argument.
    // mLastNonShaderReadOnlyLayout is used to ensure previous write are made visible to reads,
    // since the only write here is transfer, hence mLastNonShaderReadOnlyLayout is set to
    // ImageLayout::TransferDst.
    setCurrentImageLayout(renderer, ImageLayout::FragmentShaderReadOnly);

    contextVk->trackImageWithOutsideRenderPassEvent(this);

    return angle::Result::Continue;
}

void ImageHelper::resolve(ImageHelper *dst,
                          const VkImageResolve &region,
                          OutsideRenderPassCommandBuffer *commandBuffer)
{
    ASSERT(mCurrentLayout == ImageLayout::TransferSrc ||
           mCurrentLayout == ImageLayout::SharedPresent);
    commandBuffer->resolveImage(getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->getImage(),
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void ImageHelper::removeSingleSubresourceStagedUpdates(ContextVk *contextVk,
                                                       gl::LevelIndex levelIndexGL,
                                                       uint32_t layerIndex,
                                                       uint32_t layerCount)
{
    mCurrentSingleClearValue.reset();

    // Find any staged updates for this index and remove them from the pending list.
    std::vector<SubresourceUpdate> *levelUpdates = getLevelUpdates(levelIndexGL);
    if (levelUpdates == nullptr)
    {
        return;
    }

    for (size_t index = 0; index < levelUpdates->size();)
    {
        auto update = levelUpdates->begin() + index;
        if (update->matchesLayerRange(layerIndex, layerCount))
        {
            // Update total staging buffer size
            mTotalStagedBufferUpdateSize -= update->updateSource == UpdateSource::Buffer
                                                ? update->data.buffer.bufferHelper->getSize()
                                                : 0;
            update->release(contextVk->getRenderer());
            levelUpdates->erase(update);
        }
        else
        {
            index++;
        }
    }
}

void ImageHelper::removeSingleStagedClearAfterInvalidate(gl::LevelIndex levelIndexGL,
                                                         uint32_t layerIndex,
                                                         uint32_t layerCount)
{
    // When this function is called, it's expected that there may be at most one
    // ClearAfterInvalidate update pending to this subresource, and that's a color clear due to
    // emulated channels after invalidate.  This function removes that update.

    std::vector<SubresourceUpdate> *levelUpdates = getLevelUpdates(levelIndexGL);
    if (levelUpdates == nullptr)
    {
        return;
    }

    for (size_t index = 0; index < levelUpdates->size(); ++index)
    {
        auto update = levelUpdates->begin() + index;
        if (update->updateSource == UpdateSource::ClearAfterInvalidate &&
            update->matchesLayerRange(layerIndex, layerCount))
        {
            // It's a clear, so doesn't need to be released.
            levelUpdates->erase(update);
            // There's only one such clear possible.
            return;
        }
    }
}

void ImageHelper::removeStagedUpdates(ErrorContext *context,
                                      gl::LevelIndex levelGLStart,
                                      gl::LevelIndex levelGLEnd)
{
    ASSERT(validateSubresourceUpdateRefCountsConsistent());

    // Remove all updates to levels [start, end].
    for (gl::LevelIndex level = levelGLStart; level <= levelGLEnd; ++level)
    {
        std::vector<SubresourceUpdate> *levelUpdates = getLevelUpdates(level);
        if (levelUpdates == nullptr)
        {
            ASSERT(static_cast<size_t>(level.get()) >= mSubresourceUpdates.size());
            return;
        }

        for (SubresourceUpdate &update : *levelUpdates)
        {
            // Update total staging buffer size
            mTotalStagedBufferUpdateSize -= update.updateSource == UpdateSource::Buffer
                                                ? update.data.buffer.bufferHelper->getSize()
                                                : 0;
            update.release(context->getRenderer());
        }

        levelUpdates->clear();
    }

    ASSERT(validateSubresourceUpdateRefCountsConsistent());
}

angle::Result ImageHelper::stageSubresourceUpdateImpl(ContextVk *contextVk,
                                                      const gl::ImageIndex &index,
                                                      const gl::Extents &glExtents,
                                                      const gl::Offset &offset,
                                                      const gl::InternalFormat &formatInfo,
                                                      const gl::PixelUnpackState &unpack,
                                                      GLenum type,
                                                      const uint8_t *pixels,
                                                      const Format &vkFormat,
                                                      ImageAccess access,
                                                      const GLuint inputRowPitch,
                                                      const GLuint inputDepthPitch,
                                                      const GLuint inputSkipBytes,
                                                      ApplyImageUpdate applyUpdate,
                                                      bool *updateAppliedImmediatelyOut)
{
    *updateAppliedImmediatelyOut = false;

    const angle::Format &storageFormat = vkFormat.getActualImageFormat(access);

    size_t outputRowPitch;
    size_t outputDepthPitch;
    size_t stencilAllocationSize = 0;
    uint32_t bufferRowLength;
    uint32_t bufferImageHeight;
    size_t allocationSize;

    LoadImageFunctionInfo loadFunctionInfo = vkFormat.getTextureLoadFunction(access, type);
    LoadImageFunction stencilLoadFunction  = nullptr;

    bool useComputeTransCoding = false;
    if (storageFormat.isBlock)
    {
        const gl::InternalFormat &storageFormatInfo = vkFormat.getInternalFormatInfo(type);
        GLuint rowPitch;
        GLuint depthPitch;
        GLuint totalSize;

        ANGLE_VK_CHECK_MATH(contextVk, storageFormatInfo.computeCompressedImageRowPitch(
                                           glExtents.width, &rowPitch));
        ANGLE_VK_CHECK_MATH(contextVk, storageFormatInfo.computeCompressedImageDepthPitch(
                                           glExtents.height, rowPitch, &depthPitch));

        ANGLE_VK_CHECK_MATH(contextVk,
                            storageFormatInfo.computeCompressedImageSize(glExtents, &totalSize));

        outputRowPitch   = rowPitch;
        outputDepthPitch = depthPitch;
        allocationSize   = totalSize;

        ANGLE_VK_CHECK_MATH(
            contextVk, storageFormatInfo.computeBufferRowLength(glExtents.width, &bufferRowLength));
        ANGLE_VK_CHECK_MATH(contextVk, storageFormatInfo.computeBufferImageHeight(
                                           glExtents.height, &bufferImageHeight));

        if (contextVk->getFeatures().supportsComputeTranscodeEtcToBc.enabled &&
            IsETCFormat(vkFormat.getIntendedFormatID()) && IsBCFormat(storageFormat.id))
        {
            useComputeTransCoding =
                shouldUseComputeForTransCoding(vk::LevelIndex(index.getLevelIndex()));
            if (!useComputeTransCoding)
            {
                loadFunctionInfo = GetEtcToBcTransCodingFunc(vkFormat.getIntendedFormatID());
            }
        }
    }
    else
    {
        ASSERT(storageFormat.pixelBytes != 0);
        const bool stencilOnly = formatInfo.sizedInternalFormat == GL_STENCIL_INDEX8;

        if (!stencilOnly && storageFormat.id == angle::FormatID::D24_UNORM_S8_UINT)
        {
            switch (type)
            {
                case GL_UNSIGNED_INT_24_8:
                    stencilLoadFunction = angle::LoadX24S8ToS8;
                    break;
                case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
                    stencilLoadFunction = angle::LoadX32S8ToS8;
                    break;
            }
        }
        if (!stencilOnly && storageFormat.id == angle::FormatID::D32_FLOAT_S8X24_UINT)
        {
            // If depth is D32FLOAT_S8, we must pack D32F tightly (no stencil) for CopyBufferToImage
            outputRowPitch = sizeof(float) * glExtents.width;

            // The generic load functions don't handle tightly packing D32FS8 to D32F & S8 so call
            // special case load functions.
            switch (type)
            {
                case GL_UNSIGNED_INT:
                    loadFunctionInfo.loadFunction = angle::LoadD32ToD32F;
                    stencilLoadFunction           = nullptr;
                    break;
                case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
                    loadFunctionInfo.loadFunction = angle::LoadD32FS8X24ToD32F;
                    stencilLoadFunction           = angle::LoadX32S8ToS8;
                    break;
                case GL_UNSIGNED_INT_24_8:
                    loadFunctionInfo.loadFunction = angle::LoadD24S8ToD32F;
                    stencilLoadFunction           = angle::LoadX24S8ToS8;
                    break;
                default:
                    UNREACHABLE();
            }
        }
        else if (!stencilOnly)
        {
            outputRowPitch = storageFormat.pixelBytes * glExtents.width;
        }
        else
        {
            // Some Vulkan implementations do not support S8_UINT, so stencil-only data is
            // uploaded using one of combined depth-stencil formats there. Since the uploaded
            // stencil data must be tightly packed, the actual storage format should be ignored
            // with regards to its load function and output row pitch.
            loadFunctionInfo.loadFunction = angle::LoadToNative<GLubyte, 1>;
            outputRowPitch                = glExtents.width;
        }
        outputDepthPitch = outputRowPitch * glExtents.height;

        bufferRowLength   = glExtents.width;
        bufferImageHeight = glExtents.height;

        allocationSize = outputDepthPitch * glExtents.depth;

        // Note: because the LoadImageFunctionInfo functions are limited to copying a single
        // component, we have to special case packed depth/stencil use and send the stencil as a
        // separate chunk.
        if (storageFormat.hasDepthAndStencilBits() && formatInfo.depthBits > 0 &&
            formatInfo.stencilBits > 0)
        {
            // Note: Stencil is always one byte
            stencilAllocationSize = glExtents.width * glExtents.height * glExtents.depth;
            allocationSize += stencilAllocationSize;
        }
    }

    const uint8_t *source = pixels + static_cast<ptrdiff_t>(inputSkipBytes);

    // If possible, copy the buffer to the image directly on the host, to avoid having to use a temp
    // image (and do a double copy).
    if (applyUpdate != ApplyImageUpdate::Defer && !loadFunctionInfo.requiresConversion &&
        inputRowPitch == outputRowPitch && inputDepthPitch == outputDepthPitch)
    {
        bool copied = false;
        ANGLE_TRY(updateSubresourceOnHost(contextVk, applyUpdate, index, glExtents, offset, source,
                                          bufferRowLength, bufferImageHeight, &copied));
        if (copied)
        {
            *updateAppliedImmediatelyOut = true;
            return angle::Result::Continue;
        }
    }

    std::unique_ptr<RefCounted<BufferHelper>> stagingBuffer =
        std::make_unique<RefCounted<BufferHelper>>();
    BufferHelper *currentBuffer = &stagingBuffer->get();

    uint8_t *stagingPointer;
    VkDeviceSize stagingOffset;
    ANGLE_TRY(contextVk->initBufferForImageCopy(currentBuffer, allocationSize,
                                                MemoryCoherency::CachedNonCoherent,
                                                storageFormat.id, &stagingOffset, &stagingPointer));

    loadFunctionInfo.loadFunction(
        contextVk->getImageLoadContext(), glExtents.width, glExtents.height, glExtents.depth,
        source, inputRowPitch, inputDepthPitch, stagingPointer, outputRowPitch, outputDepthPitch);

    // YUV formats need special handling.
    if (storageFormat.isYUV)
    {
        gl::YuvFormatInfo yuvInfo(formatInfo.internalFormat, glExtents);

        constexpr VkImageAspectFlagBits kPlaneAspectFlags[3] = {
            VK_IMAGE_ASPECT_PLANE_0_BIT, VK_IMAGE_ASPECT_PLANE_1_BIT, VK_IMAGE_ASPECT_PLANE_2_BIT};

        // We only support mip level 0 and layerCount of 1 for YUV formats.
        ASSERT(index.getLevelIndex() == 0);
        ASSERT(index.getLayerCount() == 1);

        for (uint32_t plane = 0; plane < yuvInfo.planeCount; plane++)
        {
            VkBufferImageCopy copy           = {};
            copy.bufferOffset                = stagingOffset + yuvInfo.planeOffset[plane];
            copy.bufferRowLength             = 0;
            copy.bufferImageHeight           = 0;
            copy.imageSubresource.mipLevel   = 0;
            copy.imageSubresource.layerCount = 1;
            gl_vk::GetOffset(offset, &copy.imageOffset);
            gl_vk::GetExtent(yuvInfo.planeExtent[plane], &copy.imageExtent);
            copy.imageSubresource.baseArrayLayer = 0;
            copy.imageSubresource.aspectMask     = kPlaneAspectFlags[plane];
            appendSubresourceUpdate(
                gl::LevelIndex(0),
                SubresourceUpdate(stagingBuffer.get(), currentBuffer, copy, storageFormat.id));
        }

        stagingBuffer.release();
        return angle::Result::Continue;
    }

    VkBufferImageCopy copy         = {};
    VkImageAspectFlags aspectFlags = GetFormatAspectFlags(storageFormat);

    copy.bufferOffset      = stagingOffset;
    copy.bufferRowLength   = bufferRowLength;
    copy.bufferImageHeight = bufferImageHeight;

    gl::LevelIndex updateLevelGL(index.getLevelIndex());
    copy.imageSubresource.mipLevel   = updateLevelGL.get();
    copy.imageSubresource.layerCount = index.getLayerCount();

    gl_vk::GetOffset(offset, &copy.imageOffset);
    gl_vk::GetExtent(glExtents, &copy.imageExtent);

    if (gl::IsArrayTextureType(index.getType()))
    {
        copy.imageSubresource.baseArrayLayer = offset.z;
        copy.imageOffset.z                   = 0;
        copy.imageExtent.depth               = 1;
    }
    else
    {
        copy.imageSubresource.baseArrayLayer = index.hasLayer() ? index.getLayerIndex() : 0;
    }

    if (stencilAllocationSize > 0)
    {
        // Note: Stencil is always one byte
        ASSERT((aspectFlags & VK_IMAGE_ASPECT_STENCIL_BIT) != 0);

        // Skip over depth data.
        stagingPointer += outputDepthPitch * glExtents.depth;
        stagingOffset += outputDepthPitch * glExtents.depth;

        // recompute pitch for stencil data
        outputRowPitch   = glExtents.width;
        outputDepthPitch = outputRowPitch * glExtents.height;

        ASSERT(stencilLoadFunction != nullptr);
        stencilLoadFunction(contextVk->getImageLoadContext(), glExtents.width, glExtents.height,
                            glExtents.depth, source, inputRowPitch, inputDepthPitch, stagingPointer,
                            outputRowPitch, outputDepthPitch);

        VkBufferImageCopy stencilCopy = {};

        stencilCopy.bufferOffset                    = stagingOffset;
        stencilCopy.bufferRowLength                 = bufferRowLength;
        stencilCopy.bufferImageHeight               = bufferImageHeight;
        stencilCopy.imageSubresource.mipLevel       = copy.imageSubresource.mipLevel;
        stencilCopy.imageSubresource.baseArrayLayer = copy.imageSubresource.baseArrayLayer;
        stencilCopy.imageSubresource.layerCount     = copy.imageSubresource.layerCount;
        stencilCopy.imageOffset                     = copy.imageOffset;
        stencilCopy.imageExtent                     = copy.imageExtent;
        stencilCopy.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_STENCIL_BIT;
        appendSubresourceUpdate(updateLevelGL, SubresourceUpdate(stagingBuffer.get(), currentBuffer,
                                                                 stencilCopy, storageFormat.id));

        aspectFlags &= ~VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    if (HasBothDepthAndStencilAspects(aspectFlags))
    {
        // We still have both depth and stencil aspect bits set. That means we have a destination
        // buffer that is packed depth stencil and that the application is only loading one aspect.
        // Figure out which aspect the user is touching and remove the unused aspect bit.
        if (formatInfo.stencilBits > 0)
        {
            aspectFlags &= ~VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        else
        {
            aspectFlags &= ~VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }

    if (aspectFlags)
    {
        copy.imageSubresource.aspectMask = aspectFlags;
        appendSubresourceUpdate(
            updateLevelGL, SubresourceUpdate(stagingBuffer.get(), currentBuffer, copy,
                                             useComputeTransCoding ? vkFormat.getIntendedFormatID()
                                                                   : storageFormat.id));
        pruneSupersededUpdatesForLevel(contextVk, updateLevelGL, PruneReason::MemoryOptimization);
    }

    stagingBuffer.release();
    return angle::Result::Continue;
}

angle::Result ImageHelper::updateSubresourceOnHost(ErrorContext *context,
                                                   ApplyImageUpdate applyUpdate,
                                                   const gl::ImageIndex &index,
                                                   const gl::Extents &glExtents,
                                                   const gl::Offset &offset,
                                                   const uint8_t *source,
                                                   const GLuint memoryRowLength,
                                                   const GLuint memoryImageHeight,
                                                   bool *copiedOut)
{
    // If the image is not set up for host copy, it can't be done.
    if (!valid() || (mUsage & VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT) == 0)
    {
        return angle::Result::Continue;
    }

    Renderer *renderer = context->getRenderer();
    const VkPhysicalDeviceHostImageCopyPropertiesEXT &hostImageCopyProperties =
        renderer->getPhysicalDeviceHostImageCopyProperties();

    // The image should be unused by the GPU.
    if (!renderer->hasResourceUseFinished(getResourceUse()))
    {
        ANGLE_TRY(renderer->checkCompletedCommandsAndCleanup(context));
        if (!renderer->hasResourceUseFinished(getResourceUse()))
        {
            return angle::Result::Continue;
        }
    }

    // The image should not have any pending updates to this subresource.
    //
    // TODO: if there are any pending updates, see if they can be pruned given the incoming update.
    // This would most likely be the case where a clear is automatically staged for robustness or
    // other reasons, which would now be superseded by the data upload. http://anglebug.com/42266771
    const gl::LevelIndex updateLevelGL(index.getLevelIndex());
    const uint32_t layerIndex = index.hasLayer() ? index.getLayerIndex() : 0;
    const uint32_t layerCount = index.getLayerCount();
    if (hasStagedUpdatesForSubresource(updateLevelGL, layerIndex, layerCount))
    {
        return angle::Result::Continue;
    }

    // The image should be in a layout this is copiable.  If UNDEFINED, it can be transitioned to a
    // layout that is copyable.
    const VkImageAspectFlags aspectMask = getAspectFlags();
    if (mCurrentLayout == ImageLayout::Undefined)
    {
        VkHostImageLayoutTransitionInfoEXT transition = {};
        transition.sType     = VK_STRUCTURE_TYPE_HOST_IMAGE_LAYOUT_TRANSITION_INFO_EXT;
        transition.image     = mImage.getHandle();
        transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        // The GENERAL layout is always guaranteed to be in
        // VkPhysicalDeviceHostImageCopyPropertiesEXT::pCopyDstLayouts
        transition.newLayout                       = VK_IMAGE_LAYOUT_GENERAL;
        transition.subresourceRange.aspectMask     = aspectMask;
        transition.subresourceRange.baseMipLevel   = 0;
        transition.subresourceRange.levelCount     = mLevelCount;
        transition.subresourceRange.baseArrayLayer = 0;
        transition.subresourceRange.layerCount     = mLayerCount;

        ANGLE_VK_TRY(context, vkTransitionImageLayoutEXT(renderer->getDevice(), 1, &transition));
        mCurrentLayout = ImageLayout::HostCopy;
    }
    else if (mCurrentLayout != ImageLayout::HostCopy &&
             !IsAnyLayout(getCurrentLayout(renderer), hostImageCopyProperties.pCopyDstLayouts,
                          hostImageCopyProperties.copyDstLayoutCount))
    {
        return angle::Result::Continue;
    }

    const bool isArray            = gl::IsArrayTextureType(index.getType());
    const uint32_t baseArrayLayer = isArray ? offset.z : layerIndex;

    onWrite(updateLevelGL, 1, baseArrayLayer, layerCount, aspectMask);
    *copiedOut = true;

    // Perform the copy without holding the lock.  This is important for applications that perform
    // the copy on a separate thread, and doing all the work while holding the lock effectively
    // destroys all parallelism.  Note that the texture may not be used by the other thread without
    // appropriate synchronization (such as through glFenceSync), and because the copy is happening
    // in this call (just without holding the lock), the sync function won't be called until the
    // copy is done.
    auto doCopy = [context, image = mImage.getHandle(), source, memoryRowLength, memoryImageHeight,
                   aspectMask, levelVk = toVkLevel(updateLevelGL), isArray, baseArrayLayer,
                   layerCount, offset, glExtents,
                   layout = getCurrentLayout(renderer)](void *resultOut) {
        ANGLE_TRACE_EVENT0("gpu.angle", "Upload image data on host");
        ANGLE_UNUSED_VARIABLE(resultOut);

        VkMemoryToImageCopyEXT copyRegion          = {};
        copyRegion.sType                           = VK_STRUCTURE_TYPE_MEMORY_TO_IMAGE_COPY_EXT;
        copyRegion.pHostPointer                    = source;
        copyRegion.memoryRowLength                 = memoryRowLength;
        copyRegion.memoryImageHeight               = memoryImageHeight;
        copyRegion.imageSubresource.aspectMask     = aspectMask;
        copyRegion.imageSubresource.mipLevel       = levelVk.get();
        copyRegion.imageSubresource.baseArrayLayer = baseArrayLayer;
        copyRegion.imageSubresource.layerCount     = layerCount;
        gl_vk::GetOffset(offset, &copyRegion.imageOffset);
        gl_vk::GetExtent(glExtents, &copyRegion.imageExtent);

        if (isArray)
        {
            copyRegion.imageOffset.z     = 0;
            copyRegion.imageExtent.depth = 1;
        }

        VkCopyMemoryToImageInfoEXT copyInfo = {};
        copyInfo.sType                      = VK_STRUCTURE_TYPE_COPY_MEMORY_TO_IMAGE_INFO_EXT;
        copyInfo.dstImage                   = image;
        copyInfo.dstImageLayout             = layout;
        copyInfo.regionCount                = 1;
        copyInfo.pRegions                   = &copyRegion;

        VkResult result = vkCopyMemoryToImageEXT(context->getDevice(), &copyInfo);
        if (result != VK_SUCCESS)
        {
            context->handleError(result, __FILE__, ANGLE_FUNCTION, __LINE__);
        }
    };

    switch (applyUpdate)
    {
        // If possible, perform the copy in an unlocked tail call.  Then the other threads of the
        // application are free to draw.
        case ApplyImageUpdate::ImmediatelyInUnlockedTailCall:
            egl::Display::GetCurrentThreadUnlockedTailCall()->add(doCopy);
            break;

        // In some cases, the copy cannot be delayed.  For example because the contents are
        // immediately needed (such as when the generate mipmap hint is set), or because unlocked
        // tail calls are not allowed (this is the case with incomplete textures which are lazily
        // created at draw, but unlocked tail calls are avoided on draw calls due to overhead).
        case ApplyImageUpdate::Immediately:
            doCopy(nullptr);
            break;

        default:
            UNREACHABLE();
            doCopy(nullptr);
    }

    return angle::Result::Continue;
}

angle::Result ImageHelper::reformatStagedBufferUpdates(ContextVk *contextVk,
                                                       angle::FormatID srcFormatID,
                                                       angle::FormatID dstFormatID)
{
    const angle::Format &srcFormat = angle::Format::Get(srcFormatID);
    const angle::Format &dstFormat = angle::Format::Get(dstFormatID);
    const gl::InternalFormat &dstFormatInfo =
        gl::GetSizedInternalFormatInfo(dstFormat.glInternalFormat);

    for (std::vector<SubresourceUpdate> &levelUpdates : mSubresourceUpdates)
    {
        for (SubresourceUpdate &update : levelUpdates)
        {
            // Right now whenever we stage update from a source image, the formats always match.
            ASSERT(valid() || update.updateSource != UpdateSource::Image ||
                   update.data.image.formatID == srcFormatID);

            if (update.updateSource == UpdateSource::Buffer &&
                update.data.buffer.formatID == srcFormatID)
            {
                const VkBufferImageCopy &copy = update.data.buffer.copyRegion;

                // Source and dst data are tightly packed
                GLuint srcDataRowPitch = copy.imageExtent.width * srcFormat.pixelBytes;
                GLuint dstDataRowPitch = copy.imageExtent.width * dstFormat.pixelBytes;

                GLuint srcDataDepthPitch = srcDataRowPitch * copy.imageExtent.height;
                GLuint dstDataDepthPitch = dstDataRowPitch * copy.imageExtent.height;

                // Retrieve source buffer
                vk::BufferHelper *srcBuffer = update.data.buffer.bufferHelper;
                ASSERT(srcBuffer->isMapped());
                // The bufferOffset is relative to the buffer block. We have to use the buffer
                // block's memory pointer to get the source data pointer.
                uint8_t *srcData = srcBuffer->getBlockMemory() + copy.bufferOffset;

                // Allocate memory with dstFormat
                std::unique_ptr<RefCounted<BufferHelper>> stagingBuffer =
                    std::make_unique<RefCounted<BufferHelper>>();
                BufferHelper *dstBuffer = &stagingBuffer->get();

                uint8_t *dstData;
                VkDeviceSize dstBufferOffset;
                size_t dstBufferSize = dstDataDepthPitch * copy.imageExtent.depth;
                ANGLE_TRY(contextVk->initBufferForImageCopy(
                    dstBuffer, dstBufferSize, MemoryCoherency::CachedNonCoherent, dstFormatID,
                    &dstBufferOffset, &dstData));

                rx::PixelReadFunction pixelReadFunction   = srcFormat.pixelReadFunction;
                rx::PixelWriteFunction pixelWriteFunction = dstFormat.pixelWriteFunction;

                CopyImageCHROMIUM(srcData, srcDataRowPitch, srcFormat.pixelBytes, srcDataDepthPitch,
                                  pixelReadFunction, dstData, dstDataRowPitch, dstFormat.pixelBytes,
                                  dstDataDepthPitch, pixelWriteFunction, dstFormatInfo.format,
                                  dstFormatInfo.componentType, copy.imageExtent.width,
                                  copy.imageExtent.height, copy.imageExtent.depth, false, false,
                                  false);

                // Replace srcBuffer with dstBuffer
                update.data.buffer.bufferHelper            = dstBuffer;
                update.data.buffer.formatID                = dstFormatID;
                update.data.buffer.copyRegion.bufferOffset = dstBufferOffset;

                // Update total staging buffer size
                mTotalStagedBufferUpdateSize -= srcBuffer->getSize();
                mTotalStagedBufferUpdateSize += dstBuffer->getSize();

                // Let update structure owns the staging buffer
                if (update.refCounted.buffer)
                {
                    update.refCounted.buffer->releaseRef();
                    if (!update.refCounted.buffer->isReferenced())
                    {
                        update.refCounted.buffer->get().release(contextVk);
                        SafeDelete(update.refCounted.buffer);
                    }
                }
                update.refCounted.buffer = stagingBuffer.release();
                update.refCounted.buffer->addRef();
            }
        }
    }

    return angle::Result::Continue;
}

angle::Result ImageHelper::calculateBufferInfo(ContextVk *contextVk,
                                               const gl::Extents &glExtents,
                                               const gl::InternalFormat &formatInfo,
                                               const gl::PixelUnpackState &unpack,
                                               GLenum type,
                                               bool is3D,
                                               GLuint *inputRowPitch,
                                               GLuint *inputDepthPitch,
                                               GLuint *inputSkipBytes)
{
    // YUV formats need special handling.
    if (gl::IsYuvFormat(formatInfo.internalFormat))
    {
        gl::YuvFormatInfo yuvInfo(formatInfo.internalFormat, glExtents);

        // row pitch = Y plane row pitch
        *inputRowPitch = yuvInfo.planePitch[0];
        // depth pitch = Y plane size + chroma plane size
        *inputDepthPitch = yuvInfo.planeSize[0] + yuvInfo.planeSize[1] + yuvInfo.planeSize[2];
        *inputSkipBytes  = 0;

        return angle::Result::Continue;
    }

    ANGLE_VK_CHECK_MATH(contextVk,
                        formatInfo.computeRowPitch(type, glExtents.width, unpack.alignment,
                                                   unpack.rowLength, inputRowPitch));

    ANGLE_VK_CHECK_MATH(contextVk,
                        formatInfo.computeDepthPitch(glExtents.height, unpack.imageHeight,
                                                     *inputRowPitch, inputDepthPitch));

    ANGLE_VK_CHECK_MATH(
        contextVk, formatInfo.computeSkipBytes(type, *inputRowPitch, *inputDepthPitch, unpack, is3D,
                                               inputSkipBytes));

    return angle::Result::Continue;
}

void ImageHelper::onRenderPassAttach(const QueueSerial &queueSerial)
{
    setQueueSerial(queueSerial);
    // updatePipelineStageAccessHistory uses mCurrentLayout which we dont know yet (deferred until
    // endRenderPass time). So update it directly since we know attachment will be accessed by
    // fragment and attachment stages.
    mPipelineStageAccessHeuristic.onAccess(PipelineStageGroup::FragmentOnly);
}

void ImageHelper::onWrite(gl::LevelIndex levelStart,
                          uint32_t levelCount,
                          uint32_t layerStart,
                          uint32_t layerCount,
                          VkImageAspectFlags aspectFlags)
{
    mCurrentSingleClearValue.reset();

    // Mark contents of the given subresource as defined.
    setContentDefined(toVkLevel(levelStart), levelCount, layerStart, layerCount, aspectFlags);
}

bool ImageHelper::hasSubresourceDefinedContent(gl::LevelIndex level,
                                               uint32_t layerIndex,
                                               uint32_t layerCount) const
{
    if (layerIndex >= kMaxContentDefinedLayerCount)
    {
        return true;
    }

    uint8_t layerRangeBits =
        GetContentDefinedLayerRangeBits(layerIndex, layerCount, kMaxContentDefinedLayerCount);
    return (getLevelContentDefined(toVkLevel(level)) & LevelContentDefinedMask(layerRangeBits))
        .any();
}

bool ImageHelper::hasSubresourceDefinedStencilContent(gl::LevelIndex level,
                                                      uint32_t layerIndex,
                                                      uint32_t layerCount) const
{
    if (layerIndex >= kMaxContentDefinedLayerCount)
    {
        return true;
    }

    uint8_t layerRangeBits =
        GetContentDefinedLayerRangeBits(layerIndex, layerCount, kMaxContentDefinedLayerCount);
    return (getLevelStencilContentDefined(toVkLevel(level)) &
            LevelContentDefinedMask(layerRangeBits))
        .any();
}

void ImageHelper::invalidateSubresourceContent(ContextVk *contextVk,
                                               gl::LevelIndex level,
                                               uint32_t layerIndex,
                                               uint32_t layerCount,
                                               bool *preferToKeepContentsDefinedOut)
{
    invalidateSubresourceContentImpl(
        contextVk, level, layerIndex, layerCount,
        static_cast<VkImageAspectFlagBits>(getAspectFlags() & ~VK_IMAGE_ASPECT_STENCIL_BIT),
        &getLevelContentDefined(toVkLevel(level)), preferToKeepContentsDefinedOut);
}

void ImageHelper::invalidateSubresourceStencilContent(ContextVk *contextVk,
                                                      gl::LevelIndex level,
                                                      uint32_t layerIndex,
                                                      uint32_t layerCount,
                                                      bool *preferToKeepContentsDefinedOut)
{
    invalidateSubresourceContentImpl(
        contextVk, level, layerIndex, layerCount, VK_IMAGE_ASPECT_STENCIL_BIT,
        &getLevelStencilContentDefined(toVkLevel(level)), preferToKeepContentsDefinedOut);
}

void ImageHelper::invalidateSubresourceContentImpl(ContextVk *contextVk,
                                                   gl::LevelIndex level,
                                                   uint32_t layerIndex,
                                                   uint32_t layerCount,
                                                   VkImageAspectFlagBits aspect,
                                                   LevelContentDefinedMask *contentDefinedMask,
                                                   bool *preferToKeepContentsDefinedOut)
{
    // If the aspect being invalidated doesn't exist, skip invalidation altogether.
    if ((getAspectFlags() & aspect) == 0)
    {
        if (preferToKeepContentsDefinedOut)
        {
            // Let the caller know that this invalidate request was ignored.
            *preferToKeepContentsDefinedOut = true;
        }
        return;
    }

    // If the color format is emulated and has extra channels, those channels need to stay cleared.
    // On some devices, it's cheaper to skip invalidating the framebuffer attachment, while on
    // others it's cheaper to invalidate but then re-clear the image.
    //
    // For depth/stencil formats, each channel is separately invalidated, so the invalidate is
    // simply skipped for the emulated channel on all devices.
    const bool hasEmulatedChannels = hasEmulatedImageChannels();
    bool skip                      = false;
    switch (aspect)
    {
        case VK_IMAGE_ASPECT_DEPTH_BIT:
            skip = hasEmulatedDepthChannel();
            break;
        case VK_IMAGE_ASPECT_STENCIL_BIT:
            skip = hasEmulatedStencilChannel();
            break;
        case VK_IMAGE_ASPECT_COLOR_BIT:
            skip = hasEmulatedChannels &&
                   contextVk->getFeatures().preferSkippingInvalidateForEmulatedFormats.enabled;
            break;
        default:
            UNREACHABLE();
            skip = true;
    }

    if (preferToKeepContentsDefinedOut)
    {
        *preferToKeepContentsDefinedOut = skip;
    }
    if (skip)
    {
        return;
    }

    if (layerIndex >= kMaxContentDefinedLayerCount)
    {
        const char *aspectName = "color";
        if (aspect == VK_IMAGE_ASPECT_DEPTH_BIT)
        {
            aspectName = "depth";
        }
        else if (aspect == VK_IMAGE_ASPECT_STENCIL_BIT)
        {
            aspectName = "stencil";
        }
        ANGLE_VK_PERF_WARNING(
            contextVk, GL_DEBUG_SEVERITY_LOW,
            "glInvalidateFramebuffer (%s) ineffective on attachments with layer >= 8", aspectName);
        return;
    }

    uint8_t layerRangeBits =
        GetContentDefinedLayerRangeBits(layerIndex, layerCount, kMaxContentDefinedLayerCount);
    *contentDefinedMask &= static_cast<uint8_t>(~layerRangeBits);

    // If there are emulated channels, stage a clear to make sure those channels continue to contain
    // valid values.
    if (hasEmulatedChannels && aspect == VK_IMAGE_ASPECT_COLOR_BIT)
    {
        VkClearValue clearValue;
        clearValue.color = kEmulatedInitColorValue;

        prependSubresourceUpdate(
            level, SubresourceUpdate(aspect, clearValue, level, layerIndex, layerCount));
        mSubresourceUpdates[level.get()].front().updateSource = UpdateSource::ClearAfterInvalidate;
    }
}

void ImageHelper::restoreSubresourceContent(gl::LevelIndex level,
                                            uint32_t layerIndex,
                                            uint32_t layerCount)
{
    restoreSubresourceContentImpl(
        level, layerIndex, layerCount,
        static_cast<VkImageAspectFlagBits>(getAspectFlags() & ~VK_IMAGE_ASPECT_STENCIL_BIT),
        &getLevelContentDefined(toVkLevel(level)));
}

void ImageHelper::restoreSubresourceStencilContent(gl::LevelIndex level,
                                                   uint32_t layerIndex,
                                                   uint32_t layerCount)
{
    restoreSubresourceContentImpl(level, layerIndex, layerCount, VK_IMAGE_ASPECT_STENCIL_BIT,
                                  &getLevelStencilContentDefined(toVkLevel(level)));
}

void ImageHelper::restoreSubresourceContentImpl(gl::LevelIndex level,
                                                uint32_t layerIndex,
                                                uint32_t layerCount,
                                                VkImageAspectFlagBits aspect,
                                                LevelContentDefinedMask *contentDefinedMask)
{
    if (layerIndex >= kMaxContentDefinedLayerCount)
    {
        return;
    }

    uint8_t layerRangeBits =
        GetContentDefinedLayerRangeBits(layerIndex, layerCount, kMaxContentDefinedLayerCount);

    switch (aspect)
    {
        case VK_IMAGE_ASPECT_DEPTH_BIT:
            // Emulated depth channel should never have been marked invalid, so it can retain its
            // cleared value.
            ASSERT(!hasEmulatedDepthChannel() ||
                   (contentDefinedMask->bits() & layerRangeBits) == layerRangeBits);
            break;
        case VK_IMAGE_ASPECT_STENCIL_BIT:
            // Emulated stencil channel should never have been marked invalid, so it can retain its
            // cleared value.
            ASSERT(!hasEmulatedStencilChannel() ||
                   (contentDefinedMask->bits() & layerRangeBits) == layerRangeBits);
            break;
        case VK_IMAGE_ASPECT_COLOR_BIT:
        {
            // This function is called on attachments during a render pass when it's determined that
            // they should no longer be considered invalidated.  For an attachment with emulated
            // format that has extra channels, invalidateSubresourceContentImpl may have proactively
            // inserted a clear so that the extra channels continue to have defined values.
            // |FramebufferVk::invalidateImpl| closes the render pass right away however in that
            // case, so it should be impossible for the contents of such formats to need to be
            // restored.
            const bool hasClearAfterInvalidateUpdate =
                getLevelUpdates(level) != nullptr && getLevelUpdates(level)->size() != 0 &&
                getLevelUpdates(level)->at(0).updateSource == UpdateSource::ClearAfterInvalidate;
            ASSERT(!hasEmulatedImageChannels() || !hasClearAfterInvalidateUpdate);

            break;
        }
        default:
            UNREACHABLE();
            break;
    }

    // Additionally, as the resource has been rewritten to in the render pass, its no longer cleared
    // to the cached value.
    mCurrentSingleClearValue.reset();

    *contentDefinedMask |= layerRangeBits;
}

angle::Result ImageHelper::stagePartialClear(ContextVk *contextVk,
                                             const gl::Box &clearArea,
                                             const ClearTextureMode clearMode,
                                             gl::TextureType textureType,
                                             uint32_t levelIndex,
                                             uint32_t layerIndex,
                                             uint32_t layerCount,
                                             GLenum type,
                                             const gl::InternalFormat &formatInfo,
                                             const Format &vkFormat,
                                             ImageAccess access,
                                             const uint8_t *data)
{
    // If the input data pointer is null, the texture is filled with zeros.
    const angle::Format &intendedFormat = vkFormat.getIntendedFormat();
    const angle::Format &actualFormat   = vkFormat.getActualImageFormat(access);
    auto intendedPixelSize              = static_cast<uint32_t>(intendedFormat.pixelBytes);
    auto actualPixelSize                = static_cast<uint32_t>(actualFormat.pixelBytes);

    uint8_t intendedData[16] = {0};
    if (data != nullptr)
    {
        memcpy(intendedData, data, intendedPixelSize);
    }

    // The appropriate loading function is used to take the original value as a single pixel and
    // convert it into the format actually used for this image.
    std::vector<uint8_t> actualData(actualPixelSize, 0);
    LoadImageFunctionInfo loadFunctionInfo = vkFormat.getTextureLoadFunction(access, type);

    bool stencilOnly = formatInfo.sizedInternalFormat == GL_STENCIL_INDEX8;
    if (stencilOnly)
    {
        // Some Vulkan implementations do not support S8_UINT, so stencil-only data is
        // uploaded using one of combined depth-stencil formats there. Since the uploaded
        // stencil data must be tightly packed, the actual storage format should be ignored
        // with regards to its load function and output row pitch.
        loadFunctionInfo.loadFunction = angle::LoadToNative<GLubyte, 1>;
    }

    loadFunctionInfo.loadFunction(contextVk->getImageLoadContext(), 1, 1, 1, intendedData, 1, 1,
                                  actualData.data(), 1, 1);

    // VkClearValue is used for renderable images.
    VkClearValue clearValue = {};
    if (formatInfo.isDepthOrStencil())
    {
        GetVkClearDepthStencilValueFromBytes(intendedData, intendedFormat, &clearValue);
    }
    else
    {
        GetVkClearColorValueFromBytes(actualData.data(), actualFormat, &clearValue);
    }

    // Stage a ClearPartial update.
    VkImageAspectFlags aspectFlags = 0;
    if (!formatInfo.isDepthOrStencil())
    {
        aspectFlags |= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    else
    {
        aspectFlags |= formatInfo.depthBits > 0 ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
        aspectFlags |= formatInfo.stencilBits > 0 ? VK_IMAGE_ASPECT_STENCIL_BIT : 0;
    }

    if (clearMode == ClearTextureMode::FullClear)
    {
        bool useLayerAsDepth = textureType == gl::TextureType::CubeMap ||
                               textureType == gl::TextureType::CubeMapArray ||
                               textureType == gl::TextureType::_2DArray ||
                               textureType == gl::TextureType::_2DMultisampleArray;
        const gl::ImageIndex index = gl::ImageIndex::MakeFromType(
            textureType, levelIndex, 0, useLayerAsDepth ? clearArea.depth : 1);

        appendSubresourceUpdate(gl::LevelIndex(levelIndex),
                                SubresourceUpdate(aspectFlags, clearValue, index));
    }
    else
    {
        appendSubresourceUpdate(gl::LevelIndex(levelIndex),
                                SubresourceUpdate(aspectFlags, clearValue, textureType, levelIndex,
                                                  layerIndex, layerCount, clearArea));
    }
    return angle::Result::Continue;
}

angle::Result ImageHelper::stageSubresourceUpdate(ContextVk *contextVk,
                                                  const gl::ImageIndex &index,
                                                  const gl::Extents &glExtents,
                                                  const gl::Offset &offset,
                                                  const gl::InternalFormat &formatInfo,
                                                  const gl::PixelUnpackState &unpack,
                                                  GLenum type,
                                                  const uint8_t *pixels,
                                                  const Format &vkFormat,
                                                  ImageAccess access,
                                                  ApplyImageUpdate applyUpdate,
                                                  bool *updateAppliedImmediatelyOut)
{
    GLuint inputRowPitch   = 0;
    GLuint inputDepthPitch = 0;
    GLuint inputSkipBytes  = 0;
    ANGLE_TRY(calculateBufferInfo(contextVk, glExtents, formatInfo, unpack, type, index.usesTex3D(),
                                  &inputRowPitch, &inputDepthPitch, &inputSkipBytes));

    ANGLE_TRY(stageSubresourceUpdateImpl(
        contextVk, index, glExtents, offset, formatInfo, unpack, type, pixels, vkFormat, access,
        inputRowPitch, inputDepthPitch, inputSkipBytes, applyUpdate, updateAppliedImmediatelyOut));

    return angle::Result::Continue;
}

angle::Result ImageHelper::stageSubresourceUpdateAndGetData(ContextVk *contextVk,
                                                            size_t allocationSize,
                                                            const gl::ImageIndex &imageIndex,
                                                            const gl::Extents &glExtents,
                                                            const gl::Offset &offset,
                                                            uint8_t **dstData,
                                                            angle::FormatID formatID)
{
    std::unique_ptr<RefCounted<BufferHelper>> stagingBuffer =
        std::make_unique<RefCounted<BufferHelper>>();
    BufferHelper *currentBuffer = &stagingBuffer->get();

    VkDeviceSize stagingOffset;
    ANGLE_TRY(contextVk->initBufferForImageCopy(currentBuffer, allocationSize,
                                                MemoryCoherency::CachedNonCoherent, formatID,
                                                &stagingOffset, dstData));

    gl::LevelIndex updateLevelGL(imageIndex.getLevelIndex());

    VkBufferImageCopy copy               = {};
    copy.bufferOffset                    = stagingOffset;
    copy.bufferRowLength                 = glExtents.width;
    copy.bufferImageHeight               = glExtents.height;
    copy.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    copy.imageSubresource.mipLevel       = updateLevelGL.get();
    copy.imageSubresource.baseArrayLayer = imageIndex.hasLayer() ? imageIndex.getLayerIndex() : 0;
    copy.imageSubresource.layerCount     = imageIndex.getLayerCount();

    // Note: Only support color now
    ASSERT((mActualFormatID == angle::FormatID::NONE) ||
           (getAspectFlags() == VK_IMAGE_ASPECT_COLOR_BIT));

    gl_vk::GetOffset(offset, &copy.imageOffset);
    gl_vk::GetExtent(glExtents, &copy.imageExtent);

    appendSubresourceUpdate(
        updateLevelGL, SubresourceUpdate(stagingBuffer.release(), currentBuffer, copy, formatID));
    return angle::Result::Continue;
}

angle::Result ImageHelper::stageSubresourceUpdateFromFramebuffer(
    const gl::Context *context,
    const gl::ImageIndex &index,
    const gl::Rectangle &sourceArea,
    const gl::Offset &dstOffset,
    const gl::Extents &dstExtent,
    const gl::InternalFormat &formatInfo,
    ImageAccess access,
    FramebufferVk *framebufferVk)
{
    ContextVk *contextVk = GetImpl(context);

    // If the extents and offset is outside the source image, we need to clip.
    gl::Rectangle clippedRectangle;
    const gl::Extents readExtents = framebufferVk->getReadImageExtents();
    if (!ClipRectangle(sourceArea, gl::Rectangle(0, 0, readExtents.width, readExtents.height),
                       &clippedRectangle))
    {
        // Empty source area, nothing to do.
        return angle::Result::Continue;
    }

    bool isViewportFlipEnabled = contextVk->isViewportFlipEnabledForDrawFBO();
    if (isViewportFlipEnabled)
    {
        clippedRectangle.y = readExtents.height - clippedRectangle.y - clippedRectangle.height;
    }

    // 1- obtain a buffer handle to copy to
    Renderer *renderer = contextVk->getRenderer();

    const Format &vkFormat             = renderer->getFormat(formatInfo.sizedInternalFormat);
    const angle::Format &storageFormat = vkFormat.getActualImageFormat(access);
    LoadImageFunctionInfo loadFunction = vkFormat.getTextureLoadFunction(access, formatInfo.type);

    size_t outputRowPitch   = storageFormat.pixelBytes * clippedRectangle.width;
    size_t outputDepthPitch = outputRowPitch * clippedRectangle.height;

    std::unique_ptr<RefCounted<BufferHelper>> stagingBuffer =
        std::make_unique<RefCounted<BufferHelper>>();
    BufferHelper *currentBuffer = &stagingBuffer->get();

    uint8_t *stagingPointer;
    VkDeviceSize stagingOffset;

    // The destination is only one layer deep.
    size_t allocationSize = outputDepthPitch;
    ANGLE_TRY(contextVk->initBufferForImageCopy(currentBuffer, allocationSize,
                                                MemoryCoherency::CachedNonCoherent,
                                                storageFormat.id, &stagingOffset, &stagingPointer));

    const angle::Format &copyFormat =
        GetFormatFromFormatType(formatInfo.internalFormat, formatInfo.type);
    PackPixelsParams params(clippedRectangle, copyFormat, static_cast<GLuint>(outputRowPitch),
                            isViewportFlipEnabled, nullptr, 0);

    RenderTargetVk *readRenderTarget = framebufferVk->getColorReadRenderTarget();

    // 2- copy the source image region to the pixel buffer using a cpu readback
    if (loadFunction.requiresConversion)
    {
        // When a conversion is required, we need to use the loadFunction to read from a temporary
        // buffer instead so its an even slower path.
        size_t bufferSize =
            storageFormat.pixelBytes * clippedRectangle.width * clippedRectangle.height;
        angle::MemoryBuffer *memoryBuffer = nullptr;
        ANGLE_VK_CHECK_ALLOC(contextVk, context->getScratchBuffer(bufferSize, &memoryBuffer));

        // Read into the scratch buffer
        ANGLE_TRY(framebufferVk->readPixelsImpl(contextVk, clippedRectangle, params,
                                                VK_IMAGE_ASPECT_COLOR_BIT, readRenderTarget,
                                                memoryBuffer->data()));

        // Load from scratch buffer to our pixel buffer
        loadFunction.loadFunction(contextVk->getImageLoadContext(), clippedRectangle.width,
                                  clippedRectangle.height, 1, memoryBuffer->data(), outputRowPitch,
                                  0, stagingPointer, outputRowPitch, 0);
    }
    else
    {
        // We read directly from the framebuffer into our pixel buffer.
        ANGLE_TRY(framebufferVk->readPixelsImpl(contextVk, clippedRectangle, params,
                                                VK_IMAGE_ASPECT_COLOR_BIT, readRenderTarget,
                                                stagingPointer));
    }

    gl::LevelIndex updateLevelGL(index.getLevelIndex());

    // 3- enqueue the destination image subresource update
    VkBufferImageCopy copyToImage               = {};
    copyToImage.bufferOffset                    = static_cast<VkDeviceSize>(stagingOffset);
    copyToImage.bufferRowLength                 = 0;  // Tightly packed data can be specified as 0.
    copyToImage.bufferImageHeight               = clippedRectangle.height;
    copyToImage.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    copyToImage.imageSubresource.mipLevel       = updateLevelGL.get();
    copyToImage.imageSubresource.baseArrayLayer = index.hasLayer() ? index.getLayerIndex() : 0;
    copyToImage.imageSubresource.layerCount     = index.getLayerCount();
    gl_vk::GetOffset(dstOffset, &copyToImage.imageOffset);
    gl_vk::GetExtent(dstExtent, &copyToImage.imageExtent);

    // 3- enqueue the destination image subresource update
    appendSubresourceUpdate(updateLevelGL, SubresourceUpdate(stagingBuffer.release(), currentBuffer,
                                                             copyToImage, storageFormat.id));

    return angle::Result::Continue;
}

void ImageHelper::stageSubresourceUpdateFromImage(RefCounted<ImageHelper> *image,
                                                  const gl::ImageIndex &index,
                                                  LevelIndex srcMipLevel,
                                                  const gl::Offset &destOffset,
                                                  const gl::Extents &glExtents,
                                                  const VkImageType imageType)
{
    gl::LevelIndex updateLevelGL(index.getLevelIndex());
    VkImageAspectFlags imageAspectFlags = vk::GetFormatAspectFlags(image->get().getActualFormat());

    VkImageCopy copyToImage               = {};
    copyToImage.srcSubresource.aspectMask = imageAspectFlags;
    copyToImage.srcSubresource.mipLevel   = srcMipLevel.get();
    copyToImage.srcSubresource.layerCount = index.getLayerCount();
    copyToImage.dstSubresource.aspectMask = imageAspectFlags;
    copyToImage.dstSubresource.mipLevel   = updateLevelGL.get();

    if (imageType == VK_IMAGE_TYPE_3D)
    {
        // These values must be set explicitly to follow the Vulkan spec:
        // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkImageCopy.html
        // If either of the calling command's srcImage or dstImage parameters are of VkImageType
        // VK_IMAGE_TYPE_3D, the baseArrayLayer and layerCount members of the corresponding
        // subresource must be 0 and 1, respectively
        copyToImage.dstSubresource.baseArrayLayer = 0;
        copyToImage.dstSubresource.layerCount     = 1;
        // Preserve the assumption that destOffset.z == "dstSubresource.baseArrayLayer"
        ASSERT(destOffset.z == (index.hasLayer() ? index.getLayerIndex() : 0));
    }
    else
    {
        copyToImage.dstSubresource.baseArrayLayer = index.hasLayer() ? index.getLayerIndex() : 0;
        copyToImage.dstSubresource.layerCount     = index.getLayerCount();
    }

    gl_vk::GetOffset(destOffset, &copyToImage.dstOffset);
    gl_vk::GetExtent(glExtents, &copyToImage.extent);

    appendSubresourceUpdate(
        updateLevelGL, SubresourceUpdate(image, copyToImage, image->get().getActualFormatID()));
}

void ImageHelper::stageSubresourceUpdatesFromAllImageLevels(RefCounted<ImageHelper> *image,
                                                            gl::LevelIndex baseLevel)
{
    for (LevelIndex levelVk(0); levelVk < LevelIndex(image->get().getLevelCount()); ++levelVk)
    {
        const gl::LevelIndex levelGL = vk_gl::GetLevelIndex(levelVk, baseLevel);
        const gl::ImageIndex index =
            gl::ImageIndex::Make2DArrayRange(levelGL.get(), 0, image->get().getLayerCount());

        stageSubresourceUpdateFromImage(image, index, levelVk, gl::kOffsetZero,
                                        image->get().getLevelExtents(levelVk),
                                        image->get().getType());
    }
}

void ImageHelper::stageClear(const gl::ImageIndex &index,
                             VkImageAspectFlags aspectFlags,
                             const VkClearValue &clearValue)
{
    gl::LevelIndex updateLevelGL(index.getLevelIndex());
    appendSubresourceUpdate(updateLevelGL, SubresourceUpdate(aspectFlags, clearValue, index));
}

void ImageHelper::stageRobustResourceClear(const gl::ImageIndex &index)
{
    const VkImageAspectFlags aspectFlags = getAspectFlags();

    ASSERT(mActualFormatID != angle::FormatID::NONE);
    VkClearValue clearValue = GetRobustResourceClearValue(getIntendedFormat(), getActualFormat());

    gl::LevelIndex updateLevelGL(index.getLevelIndex());
    appendSubresourceUpdate(updateLevelGL, SubresourceUpdate(aspectFlags, clearValue, index));
}

angle::Result ImageHelper::stageResourceClearWithFormat(ContextVk *contextVk,
                                                        const gl::ImageIndex &index,
                                                        const gl::Extents &glExtents,
                                                        const angle::Format &intendedFormat,
                                                        const angle::Format &imageFormat,
                                                        const VkClearValue &clearValue)
{
    // Robust clears must only be staged if we do not have any prior data for this subresource.
    ASSERT(!hasStagedUpdatesForSubresource(gl::LevelIndex(index.getLevelIndex()),
                                           index.getLayerIndex(), index.getLayerCount()));

    const VkImageAspectFlags aspectFlags = GetFormatAspectFlags(imageFormat);

    gl::LevelIndex updateLevelGL(index.getLevelIndex());

    if (imageFormat.isBlock)
    {
        // This only supports doing an initial clear to 0, not clearing to a specific encoded RGBA
        // value
        ASSERT((clearValue.color.int32[0] == 0) && (clearValue.color.int32[1] == 0) &&
               (clearValue.color.int32[2] == 0) && (clearValue.color.int32[3] == 0));

        const gl::InternalFormat &formatInfo =
            gl::GetSizedInternalFormatInfo(imageFormat.glInternalFormat);
        GLuint totalSize;
        ANGLE_VK_CHECK_MATH(contextVk,
                            formatInfo.computeCompressedImageSize(glExtents, &totalSize));

        std::unique_ptr<RefCounted<BufferHelper>> stagingBuffer =
            std::make_unique<RefCounted<BufferHelper>>();
        BufferHelper *currentBuffer = &stagingBuffer->get();

        uint8_t *stagingPointer;
        VkDeviceSize stagingOffset;
        ANGLE_TRY(contextVk->initBufferForImageCopy(
            currentBuffer, totalSize, MemoryCoherency::CachedNonCoherent, imageFormat.id,
            &stagingOffset, &stagingPointer));
        memset(stagingPointer, 0, totalSize);

        VkBufferImageCopy copyRegion               = {};
        copyRegion.bufferOffset                    = stagingOffset;
        copyRegion.imageExtent.width               = glExtents.width;
        copyRegion.imageExtent.height              = glExtents.height;
        copyRegion.imageExtent.depth               = glExtents.depth;
        copyRegion.imageSubresource.mipLevel       = updateLevelGL.get();
        copyRegion.imageSubresource.aspectMask     = aspectFlags;
        copyRegion.imageSubresource.baseArrayLayer = index.hasLayer() ? index.getLayerIndex() : 0;
        copyRegion.imageSubresource.layerCount     = index.getLayerCount();

        // The update structure owns the staging buffer.
        appendSubresourceUpdate(
            updateLevelGL,
            SubresourceUpdate(stagingBuffer.release(), currentBuffer, copyRegion, imageFormat.id));
    }
    else
    {
        appendSubresourceUpdate(updateLevelGL, SubresourceUpdate(aspectFlags, clearValue, index));
    }

    return angle::Result::Continue;
}

angle::Result ImageHelper::stageRobustResourceClearWithFormat(ContextVk *contextVk,
                                                              const gl::ImageIndex &index,
                                                              const gl::Extents &glExtents,
                                                              const angle::Format &intendedFormat,
                                                              const angle::Format &imageFormat)
{
    VkClearValue clearValue          = GetRobustResourceClearValue(intendedFormat, imageFormat);
    gl::ImageIndex fullResourceIndex = index;
    gl::Extents fullResourceExtents  = glExtents;

    if (gl::IsArrayTextureType(index.getType()))
    {
        // For 2Darray textures gl::Extents::depth is the layer count.
        fullResourceIndex = gl::ImageIndex::MakeFromType(
            index.getType(), index.getLevelIndex(), gl::ImageIndex::kEntireLevel, glExtents.depth);
        // Vulkan requires depth of 1 for 2Darray textures.
        fullResourceExtents.depth = 1;
    }

    return stageResourceClearWithFormat(contextVk, fullResourceIndex, fullResourceExtents,
                                        intendedFormat, imageFormat, clearValue);
}

void ImageHelper::stageClearIfEmulatedFormat(bool isRobustResourceInitEnabled, bool isExternalImage)
{
    // Skip staging extra clears if robust resource init is enabled.
    if (!hasEmulatedImageChannels() || isRobustResourceInitEnabled)
    {
        return;
    }

    VkClearValue clearValue = {};
    if (getIntendedFormat().hasDepthOrStencilBits())
    {
        clearValue.depthStencil = kRobustInitDepthStencilValue;
    }
    else
    {
        clearValue.color = kEmulatedInitColorValue;
    }

    const VkImageAspectFlags aspectFlags = getAspectFlags();

    // If the image has an emulated channel and robust resource init is not enabled, always clear
    // it. These channels will be masked out in future writes, and shouldn't contain uninitialized
    // values.
    //
    // For external images, we cannot clear the image entirely, as it may contain data in the
    // non-emulated channels.  For depth/stencil images, clear is already per aspect, but for color
    // images we would need to take a special path where we only clear the emulated channels.

    // Block images are not cleared, since no emulated channels are present if decoded.
    if (isExternalImage && getIntendedFormat().isBlock)
    {
        return;
    }

    const bool clearOnlyEmulatedChannels =
        isExternalImage && !getIntendedFormat().hasDepthOrStencilBits();
    const VkColorComponentFlags colorMaskFlags =
        clearOnlyEmulatedChannels ? getEmulatedChannelsMask() : 0;

    for (LevelIndex level(0); level < LevelIndex(mLevelCount); ++level)
    {
        gl::LevelIndex updateLevelGL = toGLLevel(level);
        gl::ImageIndex index =
            gl::ImageIndex::Make2DArrayRange(updateLevelGL.get(), 0, mLayerCount);

        if (clearOnlyEmulatedChannels)
        {
            prependSubresourceUpdate(updateLevelGL,
                                     SubresourceUpdate(colorMaskFlags, clearValue.color, index));
        }
        else
        {
            prependSubresourceUpdate(updateLevelGL,
                                     SubresourceUpdate(aspectFlags, clearValue, index));
        }
    }
}

bool ImageHelper::verifyEmulatedClearsAreBeforeOtherUpdates(
    const std::vector<SubresourceUpdate> &updates)
{
    bool isIteratingEmulatedClears = true;

    for (const SubresourceUpdate &update : updates)
    {
        // If anything other than ClearEmulatedChannelsOnly is visited, there cannot be any
        // ClearEmulatedChannelsOnly updates after that.
        if (update.updateSource != UpdateSource::ClearEmulatedChannelsOnly)
        {
            isIteratingEmulatedClears = false;
        }
        else if (!isIteratingEmulatedClears)
        {
            // If ClearEmulatedChannelsOnly is visited after another update, that's an error.
            return false;
        }
    }

    // Additionally, verify that emulated clear is not applied multiple times.
    if (updates.size() >= 2 && updates[1].updateSource == UpdateSource::ClearEmulatedChannelsOnly)
    {
        return false;
    }

    return true;
}

void ImageHelper::stageSelfAsSubresourceUpdates(
    ContextVk *contextVk,
    uint32_t levelCount,
    gl::TextureType textureType,
    const gl::CubeFaceArray<gl::TexLevelMask> &skipLevels)

{
    // Nothing to do if every level must be skipped
    const gl::TexLevelMask levelsMask(angle::BitMask<uint32_t>(levelCount)
                                      << mFirstAllocatedLevel.get());
    const gl::TexLevelMask skipLevelsAllFaces = AggregateSkipLevels(skipLevels);

    if ((~skipLevelsAllFaces & levelsMask).none())
    {
        return;
    }

    // Because we are cloning this object to another object, we must finalize the layout if it is
    // being used by current renderpass as attachment. Otherwise we are copying the incorrect layout
    // since it is determined at endRenderPass time.
    contextVk->finalizeImageLayout(this, {});

    std::unique_ptr<RefCounted<ImageHelper>> prevImage =
        std::make_unique<RefCounted<ImageHelper>>();

    // Move the necessary information for staged update to work, and keep the rest as part of this
    // object.

    // Usage info
    prevImage->get().Resource::operator=(std::move(*this));

    // Vulkan objects
    prevImage->get().mImage         = std::move(mImage);
    prevImage->get().mDeviceMemory  = std::move(mDeviceMemory);
    prevImage->get().mVmaAllocation = std::move(mVmaAllocation);

    // Barrier information.  Note: mLevelCount is set to levelCount so that only the necessary
    // levels are transitioned when flushing the update.
    prevImage->get().mIntendedFormatID            = mIntendedFormatID;
    prevImage->get().mActualFormatID              = mActualFormatID;
    prevImage->get().mCurrentLayout               = mCurrentLayout;
    prevImage->get().mCurrentDeviceQueueIndex     = mCurrentDeviceQueueIndex;
    prevImage->get().mLastNonShaderReadOnlyLayout = mLastNonShaderReadOnlyLayout;
    prevImage->get().mCurrentShaderReadStageMask  = mCurrentShaderReadStageMask;
    prevImage->get().mLevelCount                  = levelCount;
    prevImage->get().mLayerCount                  = mLayerCount;
    prevImage->get().mImageSerial                 = mImageSerial;
    prevImage->get().mAllocationSize              = mAllocationSize;
    prevImage->get().mMemoryAllocationType        = mMemoryAllocationType;
    prevImage->get().mMemoryTypeIndex             = mMemoryTypeIndex;

    // Reset information for current (invalid) image.
    mCurrentLayout               = ImageLayout::Undefined;
    mCurrentDeviceQueueIndex     = kInvalidDeviceQueueIndex;
    mIsReleasedToExternal        = false;
    mIsForeignImage              = false;
    mLastNonShaderReadOnlyLayout = ImageLayout::Undefined;
    mCurrentShaderReadStageMask  = 0;
    mImageSerial                 = kInvalidImageSerial;
    mMemoryAllocationType        = MemoryAllocationType::InvalidEnum;

    setEntireContentUndefined();

    // Stage updates from the previous image.
    for (LevelIndex levelVk(0); levelVk < LevelIndex(levelCount); ++levelVk)
    {
        gl::LevelIndex levelGL = toGLLevel(levelVk);
        if (!skipLevelsAllFaces.test(levelGL.get()))
        {
            const gl::ImageIndex index =
                gl::ImageIndex::Make2DArrayRange(levelGL.get(), 0, mLayerCount);

            stageSubresourceUpdateFromImage(prevImage.get(), index, levelVk, gl::kOffsetZero,
                                            getLevelExtents(levelVk), mImageType);
        }
        else if (textureType == gl::TextureType::CubeMap)
        {
            for (uint32_t face = 0; face < gl::kCubeFaceCount; ++face)
            {
                if (!skipLevels[face][levelGL.get()])
                {
                    const gl::ImageIndex index =
                        gl::ImageIndex::Make2DArrayRange(levelGL.get(), face, 1);

                    stageSubresourceUpdateFromImage(prevImage.get(), index, levelVk,
                                                    gl::kOffsetZero, getLevelExtents(levelVk),
                                                    mImageType);
                }
            }
        }
    }

    ASSERT(levelCount > 0);
    prevImage.release();
}

angle::Result ImageHelper::flushSingleSubresourceStagedUpdates(ContextVk *contextVk,
                                                               gl::LevelIndex levelGL,
                                                               uint32_t layer,
                                                               uint32_t layerCount,
                                                               ClearValuesArray *deferredClears,
                                                               uint32_t deferredClearIndex)
{
    std::vector<SubresourceUpdate> *levelUpdates = getLevelUpdates(levelGL);
    if (levelUpdates == nullptr || levelUpdates->empty())
    {
        return angle::Result::Continue;
    }

    // Handle deferred clears. Search the updates list for a matching clear index.
    if (deferredClears)
    {
        Optional<size_t> foundClear;

        for (size_t updateIndex = 0; updateIndex < levelUpdates->size(); ++updateIndex)
        {
            SubresourceUpdate &update = (*levelUpdates)[updateIndex];

            if (update.intersectsLayerRange(layer, layerCount))
            {
                // On any data update or the clear does not match exact layer range, we'll need to
                // do a full upload.
                const bool isClear = IsClearOfAllChannels(update.updateSource);
                if (isClear && update.matchesLayerRange(layer, layerCount))
                {
                    foundClear = updateIndex;
                }
                else
                {
                    foundClear.reset();
                    break;
                }
            }
        }

        // If we have a valid index we defer the clear using the clear reference.
        if (foundClear.valid())
        {
            size_t foundIndex         = foundClear.value();
            const ClearUpdate &update = (*levelUpdates)[foundIndex].data.clear;

            // Note that this set command handles combined or separate depth/stencil clears.
            deferredClears->store(deferredClearIndex, update.aspectFlags, update.value);

            // Do not call onWrite as it removes mCurrentSingleClearValue, but instead call
            // setContentDefined directly.
            setContentDefined(toVkLevel(levelGL), 1, layer, layerCount, update.aspectFlags);

            // We process the updates again to erase any clears for this level.
            removeSingleSubresourceStagedUpdates(contextVk, levelGL, layer, layerCount);
            return angle::Result::Continue;
        }

        // Otherwise we proceed with a normal update.
    }

    return flushStagedUpdates(contextVk, levelGL, levelGL + 1, layer, layer + layerCount, {});
}

angle::Result ImageHelper::flushStagedClearEmulatedChannelsUpdates(ContextVk *contextVk,
                                                                   gl::LevelIndex levelGLStart,
                                                                   gl::LevelIndex levelGLLimit,
                                                                   bool *otherUpdatesToFlushOut)
{
    *otherUpdatesToFlushOut = false;
    for (gl::LevelIndex updateMipLevelGL = levelGLStart; updateMipLevelGL < levelGLLimit;
         ++updateMipLevelGL)
    {
        // It is expected that the checked mip levels in this loop do not surpass the size of
        // mSubresourceUpdates.
        std::vector<SubresourceUpdate> *levelUpdates = getLevelUpdates(updateMipLevelGL);
        ASSERT(levelUpdates != nullptr);

        // The levels with no updates should be skipped.
        if (levelUpdates->empty())
        {
            continue;
        }

        // Since ClearEmulatedChannelsOnly is expected in the beginning and there cannot be more
        // than one such update type, we can process the first update and move on if there is
        // another update type in the list.
        ASSERT(verifyEmulatedClearsAreBeforeOtherUpdates(*levelUpdates));
        std::vector<SubresourceUpdate>::iterator update = (*levelUpdates).begin();

        if (update->updateSource != UpdateSource::ClearEmulatedChannelsOnly)
        {
            *otherUpdatesToFlushOut = true;
            continue;
        }

        // If found, ClearEmulatedChannelsOnly should be flushed before the others and removed from
        // the update list.
        ASSERT(update->updateSource == UpdateSource::ClearEmulatedChannelsOnly);
        uint32_t updateBaseLayer, updateLayerCount;
        update->getDestSubresource(mLayerCount, &updateBaseLayer, &updateLayerCount);

        const LevelIndex updateMipLevelVk = toVkLevel(updateMipLevelGL);
        update->data.clear.levelIndex     = updateMipLevelVk.get();
        ANGLE_TRY(clearEmulatedChannels(contextVk, update->data.clear.colorMaskFlags,
                                        update->data.clear.value, updateMipLevelVk, updateBaseLayer,
                                        updateLayerCount));
        // Do not call onWrite. Even though some channels of the image are cleared, don't consider
        // the contents defined. Also, since clearing emulated channels is a one-time thing that's
        // superseded by Clears, |mCurrentSingleClearValue| is irrelevant and can't have a value.
        ASSERT(!mCurrentSingleClearValue.valid());

        levelUpdates->erase(update);
        if (!levelUpdates->empty())
        {
            ASSERT(levelUpdates->begin()->updateSource != UpdateSource::ClearEmulatedChannelsOnly);
            *otherUpdatesToFlushOut = true;
        }
    }

    return angle::Result::Continue;
}

angle::Result ImageHelper::flushStagedUpdatesImpl(ContextVk *contextVk,
                                                  gl::LevelIndex levelGLStart,
                                                  gl::LevelIndex levelGLEnd,
                                                  uint32_t layerStart,
                                                  uint32_t layerEnd,
                                                  const gl::TexLevelMask &skipLevelsAllFaces)
{
    Renderer *renderer = contextVk->getRenderer();

    const angle::FormatID &actualformat   = getActualFormatID();
    const angle::FormatID &intendedFormat = getIntendedFormatID();

    const VkImageAspectFlags aspectFlags = GetFormatAspectFlags(getActualFormat());

    // Start in TransferDst.  Don't yet mark any subresource as having defined contents; that is
    // done with fine granularity as updates are applied.  This is achieved by specifying a layer
    // that is outside the tracking range. Under some circumstances, ComputeWrite is also required.
    // This need not be applied if the only updates are ClearEmulatedChannels.
    CommandBufferAccess transferAccess;
    OutsideRenderPassCommandBufferHelper *commandBuffer = nullptr;
    bool transCoding = renderer->getFeatures().supportsComputeTranscodeEtcToBc.enabled &&
                       IsETCFormat(intendedFormat) && IsBCFormat(actualformat);

    if (transCoding)
    {
        transferAccess.onImageTransferDstAndComputeWrite(
            levelGLStart, 1, kMaxContentDefinedLayerCount, 0, aspectFlags, this);
    }
    else
    {
        transferAccess.onImageTransferWrite(levelGLStart, 1, kMaxContentDefinedLayerCount, 0,
                                            aspectFlags, this);
    }
    ANGLE_TRY(contextVk->getOutsideRenderPassCommandBufferHelper(transferAccess, &commandBuffer));

    // Flush the staged updates in each mip level.
    for (gl::LevelIndex updateMipLevelGL = levelGLStart; updateMipLevelGL < levelGLEnd;
         ++updateMipLevelGL)
    {
        // If updates to this level are specifically asked to be skipped, skip
        // them. This can happen when recreating an image that has been partially incompatibly
        // redefined, in which case only updates to the levels that haven't been redefined
        // should be flushed.
        if (skipLevelsAllFaces.test(updateMipLevelGL.get()))
        {
            continue;
        }

        // It is expected that the checked mip levels in this loop do not surpass the size of
        // mSubresourceUpdates.
        std::vector<SubresourceUpdate> *levelUpdates = getLevelUpdates(updateMipLevelGL);
        std::vector<SubresourceUpdate> updatesToKeep;
        ASSERT(levelUpdates != nullptr);

        // Because updates may have overlapping layer ranges, we must first figure out the actual
        // layer ranges that will be flushed. The updatesToKeep list must compare against this
        // adjusted layer range. Otherwise you may end up keeping the update even though it is
        // overlapped with the update that gets flushed, and then content gets overwritten when
        // updatesToKeep gets flushed out.
        uint32_t adjustedLayerStart = layerStart, adjustedLayerEnd = layerEnd;
        if (levelUpdates->size() > 1)
        {
            adjustLayerRange(*levelUpdates, &adjustedLayerStart, &adjustedLayerEnd);
        }

        for (SubresourceUpdate &update : *levelUpdates)
        {
            ASSERT(IsClearOfAllChannels(update.updateSource) ||
                   (update.updateSource == UpdateSource::ClearPartial) ||
                   (update.updateSource == UpdateSource::Buffer &&
                    update.data.buffer.bufferHelper != nullptr) ||
                   (update.updateSource == UpdateSource::Image &&
                    update.refCounted.image != nullptr && update.refCounted.image->isReferenced() &&
                    update.refCounted.image->get().valid()));

            uint32_t updateBaseLayer, updateLayerCount;
            update.getDestSubresource(mLayerCount, &updateBaseLayer, &updateLayerCount);

            // If the update layers don't intersect the requested layers, skip the update.
            const bool areUpdateLayersOutsideRange =
                updateBaseLayer + updateLayerCount <= adjustedLayerStart ||
                updateBaseLayer >= adjustedLayerEnd;
            if (areUpdateLayersOutsideRange)
            {
                updatesToKeep.emplace_back(std::move(update));
                continue;
            }

            const LevelIndex updateMipLevelVk = toVkLevel(updateMipLevelGL);

            // It seems we haven't fully support glCopyImageSubData
            // when compressed format emulated by uncompressed format.
            // make assumption that there is no data source come from image.
            ASSERT(!transCoding || (transCoding && update.updateSource == UpdateSource::Buffer));
            // The updates were holding gl::LevelIndex values so that they would not need
            // modification when the base level of the texture changes.  Now that the update is
            // about to take effect, we need to change miplevel to LevelIndex.
            switch (update.updateSource)
            {
                case UpdateSource::Clear:
                case UpdateSource::ClearAfterInvalidate:
                {
                    update.data.clear.levelIndex = updateMipLevelVk.get();
                    break;
                }
                case UpdateSource::ClearPartial:
                {
                    update.data.clearPartial.levelIndex = updateMipLevelVk.get();
                    break;
                }
                case UpdateSource::Buffer:
                {
                    if (!transCoding && !isDataFormatMatchForCopy(update.data.buffer.formatID))
                    {
                        // TODO: http://anglebug.com/42264884, we should handle this in higher level
                        // code. If we have incompatible updates, skip but keep it.
                        updatesToKeep.emplace_back(std::move(update));
                        continue;
                    }
                    update.data.buffer.copyRegion.imageSubresource.mipLevel =
                        updateMipLevelVk.get();
                    break;
                }
                case UpdateSource::Image:
                {
                    if (!isDataFormatMatchForCopy(update.data.image.formatID))
                    {
                        // If we have incompatible updates, skip but keep it.
                        updatesToKeep.emplace_back(std::move(update));
                        continue;
                    }
                    update.data.image.copyRegion.dstSubresource.mipLevel = updateMipLevelVk.get();
                    break;
                }
                default:
                {
                    UNREACHABLE();
                    break;
                }
            }

            // When a barrier is necessary when uploading updates to a level, we could instead move
            // to the next level and continue uploads in parallel.  Once all levels need a barrier,
            // a single barrier can be issued and we could continue with the rest of the updates
            // from the first level. In case of multiple layer updates within the same level, a
            // barrier might be needed if there are multiple updates in the same parts of the image.
            ImageLayout barrierLayout =
                transCoding ? ImageLayout::TransferDstAndComputeWrite : ImageLayout::TransferDst;
            if (updateLayerCount >= kMaxParallelLayerWrites)
            {
                // If there are more subresources than bits we can track, always insert a barrier.
                recordWriteBarrier(contextVk, aspectFlags, barrierLayout, updateMipLevelGL, 1,
                                   updateBaseLayer, updateLayerCount, commandBuffer);
                mSubresourcesWrittenSinceBarrier[updateMipLevelGL.get()].set();
            }
            else
            {
                ImageLayerWriteMask subresourceHash =
                    GetImageLayerWriteMask(updateBaseLayer, updateLayerCount);

                if (areLevelSubresourcesWrittenWithinMaskRange(updateMipLevelGL.get(),
                                                               subresourceHash))
                {
                    // If there's overlap in subresource upload, issue a barrier.
                    recordWriteBarrier(contextVk, aspectFlags, barrierLayout, updateMipLevelGL, 1,
                                       updateBaseLayer, updateLayerCount, commandBuffer);
                    mSubresourcesWrittenSinceBarrier[updateMipLevelGL.get()].reset();
                }
                mSubresourcesWrittenSinceBarrier[updateMipLevelGL.get()] |= subresourceHash;
            }

            // Add the necessary commands to the outside command buffer.
            switch (update.updateSource)
            {
                case UpdateSource::Clear:
                case UpdateSource::ClearAfterInvalidate:
                {
                    clear(renderer, update.data.clear.aspectFlags, update.data.clear.value,
                          updateMipLevelVk, updateBaseLayer, updateLayerCount,
                          &commandBuffer->getCommandBuffer());
                    contextVk->getPerfCounters().fullImageClears++;
                    // Remember the latest operation is a clear call.
                    mCurrentSingleClearValue = update.data.clear;

                    // Do not call onWrite as it removes mCurrentSingleClearValue, but instead call
                    // setContentDefined directly.
                    setContentDefined(updateMipLevelVk, 1, updateBaseLayer, updateLayerCount,
                                      update.data.clear.aspectFlags);
                    break;
                }
                case UpdateSource::ClearPartial:
                {
                    ClearPartialUpdate &clearPartialUpdate = update.data.clearPartial;
                    gl::Box clearArea =
                        gl::Box(clearPartialUpdate.offset, clearPartialUpdate.extent);

                    // clearTexture() uses LOAD_OP_CLEAR in a render pass to clear the texture. If
                    // the texture has the depth dimension or multiple layers, the clear will be
                    // performed layer by layer. In case of the former, the z-dimension will be used
                    // as the layer index.
                    UtilsVk::ClearTextureParameters params = {};
                    params.aspectFlags                     = clearPartialUpdate.aspectFlags;
                    params.level                           = updateMipLevelVk;
                    params.clearArea                       = clearArea;
                    params.clearValue                      = clearPartialUpdate.clearValue;

                    bool shouldUseDepthAsLayer =
                        clearPartialUpdate.textureType == gl::TextureType::_3D;
                    uint32_t clearBaseLayer =
                        shouldUseDepthAsLayer ? clearArea.z : clearPartialUpdate.layerIndex;
                    uint32_t clearLayerCount =
                        shouldUseDepthAsLayer ? clearArea.depth : clearPartialUpdate.layerCount;

                    for (uint32_t layerIndex = clearBaseLayer;
                         layerIndex < clearBaseLayer + clearLayerCount; ++layerIndex)
                    {
                        params.layer = layerIndex;
                        ANGLE_TRY(contextVk->getUtils().clearTexture(contextVk, this, params));
                    }

                    // Queue serial index becomes invalid after starting render pass for the op
                    // above. Therefore, the outside command buffer should be re-acquired.
                    ANGLE_TRY(
                        contextVk->getOutsideRenderPassCommandBufferHelper({}, &commandBuffer));
                    setContentDefined(updateMipLevelVk, 1, updateBaseLayer, updateLayerCount,
                                      clearPartialUpdate.aspectFlags);
                    break;
                }
                case UpdateSource::Buffer:
                {
                    BufferUpdate &bufferUpdate = update.data.buffer;

                    BufferHelper *currentBuffer = bufferUpdate.bufferHelper;
                    ASSERT(currentBuffer && currentBuffer->valid());
                    ANGLE_TRY(currentBuffer->flush(renderer));

                    CommandBufferAccess bufferAccess;
                    VkBufferImageCopy *copyRegion = &update.data.buffer.copyRegion;

                    if (transCoding && update.data.buffer.formatID != actualformat)
                    {
                        bufferAccess.onBufferComputeShaderRead(currentBuffer);
                        ANGLE_TRY(contextVk->getOutsideRenderPassCommandBufferHelper(
                            bufferAccess, &commandBuffer));
                        ANGLE_TRY(contextVk->getUtils().transCodeEtcToBc(contextVk, currentBuffer,
                                                                         this, copyRegion));
                    }
                    else
                    {
                        bufferAccess.onBufferTransferRead(currentBuffer);
                        ANGLE_TRY(contextVk->getOutsideRenderPassCommandBufferHelper(
                            bufferAccess, &commandBuffer));
                        commandBuffer->getCommandBuffer().copyBufferToImage(
                            currentBuffer->getBuffer().getHandle(), mImage,
                            getCurrentLayout(renderer), 1, copyRegion);
                    }
                    bool commandBufferWasFlushed = false;
                    ANGLE_TRY(contextVk->onCopyUpdate(currentBuffer->getSize(),
                                                      &commandBufferWasFlushed));
                    onWrite(updateMipLevelGL, 1, updateBaseLayer, updateLayerCount,
                            copyRegion->imageSubresource.aspectMask);

                    // Update total staging buffer size.
                    mTotalStagedBufferUpdateSize -= bufferUpdate.bufferHelper->getSize();

                    if (commandBufferWasFlushed)
                    {
                        ANGLE_TRY(
                            contextVk->getOutsideRenderPassCommandBufferHelper({}, &commandBuffer));
                    }
                    break;
                }
                case UpdateSource::Image:
                {
                    CommandBufferAccess imageAccess;
                    imageAccess.onImageTransferRead(aspectFlags, &update.refCounted.image->get());
                    ANGLE_TRY(contextVk->getOutsideRenderPassCommandBufferHelper(imageAccess,
                                                                                 &commandBuffer));

                    VkImageCopy *copyRegion = &update.data.image.copyRegion;
                    commandBuffer->getCommandBuffer().copyImage(
                        update.refCounted.image->get().getImage(),
                        update.refCounted.image->get().getCurrentLayout(renderer), mImage,
                        getCurrentLayout(renderer), 1, copyRegion);
                    onWrite(updateMipLevelGL, 1, updateBaseLayer, updateLayerCount,
                            copyRegion->dstSubresource.aspectMask);
                    break;
                }
                default:
                {
                    UNREACHABLE();
                    break;
                }
            }

            update.release(renderer);
        }

        // Only remove the updates that were actually applied to the image.
        *levelUpdates = std::move(updatesToKeep);
    }

    // After applying the updates, the image serial should match the current queue serial of the
    // outside command buffer.
    if (mUse.getSerials()[commandBuffer->getQueueSerial().getIndex()] !=
        commandBuffer->getQueueSerial().getSerial())
    {
        // There has been a submission after the retainImage() call. Update the queue serial again.
        setQueueSerial(commandBuffer->getQueueSerial());
    }

    return angle::Result::Continue;
}

angle::Result ImageHelper::flushStagedUpdates(ContextVk *contextVk,
                                              gl::LevelIndex levelGLStart,
                                              gl::LevelIndex levelGLEnd,
                                              uint32_t layerStart,
                                              uint32_t layerEnd,
                                              const gl::CubeFaceArray<gl::TexLevelMask> &skipLevels)
{
    Renderer *renderer = contextVk->getRenderer();

    if (!hasStagedUpdatesInLevels(levelGLStart, levelGLEnd))
    {
        return angle::Result::Continue;
    }

    const gl::TexLevelMask skipLevelsAllFaces = AggregateSkipLevels(skipLevels);
    removeSupersededUpdates(contextVk, skipLevelsAllFaces);

    // If a clear is requested and we know it was previously cleared with the same value, we drop
    // the clear.
    if (mCurrentSingleClearValue.valid())
    {
        std::vector<SubresourceUpdate> *levelUpdates =
            getLevelUpdates(gl::LevelIndex(mCurrentSingleClearValue.value().levelIndex));
        if (levelUpdates && levelUpdates->size() == 1)
        {
            SubresourceUpdate &update = (*levelUpdates)[0];
            if (IsClearOfAllChannels(update.updateSource) &&
                mCurrentSingleClearValue.value() == update.data.clear)
            {
                ASSERT(levelGLStart + 1 == levelGLEnd);
                setContentDefined(toVkLevel(levelGLStart), 1, layerStart, layerEnd - layerStart,
                                  update.data.clear.aspectFlags);
                ANGLE_VK_PERF_WARNING(contextVk, GL_DEBUG_SEVERITY_LOW,
                                      "Repeated Clear on framebuffer attachment dropped");
                update.release(renderer);
                levelUpdates->clear();
                return angle::Result::Continue;
            }
        }
    }

    ASSERT(validateSubresourceUpdateRefCountsConsistent());

    // Process the clear emulated channels from the updates first. They are expected to be at the
    // beginning of the level updates.
    bool otherUpdatesToFlushOut = false;
    clipLevelToUpdateListUpperLimit(&levelGLEnd);
    ANGLE_TRY(flushStagedClearEmulatedChannelsUpdates(contextVk, levelGLStart, levelGLEnd,
                                                      &otherUpdatesToFlushOut));

    // If updates remain after processing ClearEmulatedChannelsOnly updates, we should acquire the
    // outside command buffer and apply the necessary barriers. Otherwise, this function can return
    // early, skipping the next loop.
    if (otherUpdatesToFlushOut)
    {
        ANGLE_TRY(flushStagedUpdatesImpl(contextVk, levelGLStart, levelGLEnd, layerStart, layerEnd,
                                         skipLevelsAllFaces));
    }

    // Compact mSubresourceUpdates, then check if there are any updates left.
    size_t compactSize;
    for (compactSize = mSubresourceUpdates.size(); compactSize > 0; --compactSize)
    {
        if (!mSubresourceUpdates[compactSize - 1].empty())
        {
            break;
        }
    }
    mSubresourceUpdates.resize(compactSize);

    ASSERT(validateSubresourceUpdateRefCountsConsistent());

    // If no updates left, release the staging buffers to save memory.
    if (mSubresourceUpdates.empty())
    {
        ASSERT(mTotalStagedBufferUpdateSize == 0);
        onStateChange(angle::SubjectMessage::InitializationComplete);
    }

    return angle::Result::Continue;
}

angle::Result ImageHelper::flushAllStagedUpdates(ContextVk *contextVk)
{
    return flushStagedUpdates(contextVk, mFirstAllocatedLevel, mFirstAllocatedLevel + mLevelCount,
                              0, mLayerCount, {});
}

bool ImageHelper::hasStagedUpdatesForSubresource(gl::LevelIndex levelGL,
                                                 uint32_t layer,
                                                 uint32_t layerCount) const
{
    // Check to see if any updates are staged for the given level and layer

    const std::vector<SubresourceUpdate> *levelUpdates = getLevelUpdates(levelGL);
    if (levelUpdates == nullptr || levelUpdates->empty())
    {
        return false;
    }

    for (const SubresourceUpdate &update : *levelUpdates)
    {
        uint32_t updateBaseLayer, updateLayerCount;
        update.getDestSubresource(mLayerCount, &updateBaseLayer, &updateLayerCount);

        const uint32_t updateLayerEnd = updateBaseLayer + updateLayerCount;
        const uint32_t layerEnd       = layer + layerCount;

        if ((layer >= updateBaseLayer && layer < updateLayerEnd) ||
            (layerEnd > updateBaseLayer && layerEnd <= updateLayerEnd))
        {
            // The layers intersect with the update range
            return true;
        }
    }

    return false;
}

bool ImageHelper::removeStagedClearUpdatesAndReturnColor(gl::LevelIndex levelGL,
                                                         const VkClearColorValue **color)
{
    std::vector<SubresourceUpdate> *levelUpdates = getLevelUpdates(levelGL);
    if (levelUpdates == nullptr || levelUpdates->empty())
    {
        return false;
    }

    bool result = false;

    for (size_t index = 0; index < levelUpdates->size();)
    {
        auto update = levelUpdates->begin() + index;
        if (IsClearOfAllChannels(update->updateSource))
        {
            if (color != nullptr)
            {
                *color = &update->data.clear.value.color;
            }
            levelUpdates->erase(update);
            result = true;
        }
    }

    return result;
}

void ImageHelper::adjustLayerRange(const std::vector<SubresourceUpdate> &levelUpdates,
                                   uint32_t *layerStart,
                                   uint32_t *layerEnd)
{
    for (const SubresourceUpdate &update : levelUpdates)
    {
        uint32_t updateBaseLayer, updateLayerCount;
        update.getDestSubresource(mLayerCount, &updateBaseLayer, &updateLayerCount);
        uint32_t updateLayerEnd = updateBaseLayer + updateLayerCount;

        // In some cases, the update has the bigger layer range than the request. If the update
        // layers intersect the requested layers, then expand the layer range to the maximum from
        // the update and from the request.
        const bool areUpdateLayersWithinRange =
            updateBaseLayer < *layerEnd && updateLayerEnd > *layerStart;
        if (areUpdateLayersWithinRange)
        {
            *layerStart = std::min(*layerStart, updateBaseLayer);
            *layerEnd   = std::max(*layerEnd, updateLayerEnd);
        }
    }
}

gl::LevelIndex ImageHelper::getLastAllocatedLevel() const
{
    return mFirstAllocatedLevel + mLevelCount - 1;
}

bool ImageHelper::hasStagedUpdatesInAllocatedLevels() const
{
    return hasStagedUpdatesInLevels(mFirstAllocatedLevel, getLastAllocatedLevel() + 1);
}

bool ImageHelper::hasStagedUpdatesInLevels(gl::LevelIndex levelStart, gl::LevelIndex levelEnd) const
{
    for (gl::LevelIndex level = levelStart; level < levelEnd; ++level)
    {
        const std::vector<SubresourceUpdate> *levelUpdates = getLevelUpdates(level);
        if (levelUpdates == nullptr)
        {
            ASSERT(static_cast<size_t>(level.get()) >= mSubresourceUpdates.size());
            return false;
        }

        if (!levelUpdates->empty())
        {
            return true;
        }
    }
    return false;
}

bool ImageHelper::hasStagedImageUpdatesWithMismatchedFormat(gl::LevelIndex levelStart,
                                                            gl::LevelIndex levelEnd,
                                                            angle::FormatID formatID) const
{
    for (gl::LevelIndex level = levelStart; level < levelEnd; ++level)
    {
        const std::vector<SubresourceUpdate> *levelUpdates = getLevelUpdates(level);
        if (levelUpdates == nullptr)
        {
            continue;
        }

        for (const SubresourceUpdate &update : *levelUpdates)
        {
            if (update.updateSource == UpdateSource::Image &&
                update.data.image.formatID != formatID)
            {
                return true;
            }
        }
    }
    return false;
}

bool ImageHelper::hasBufferSourcedStagedUpdatesInAllLevels() const
{
    for (gl::LevelIndex level = mFirstAllocatedLevel; level <= getLastAllocatedLevel(); ++level)
    {
        const std::vector<SubresourceUpdate> *levelUpdates = getLevelUpdates(level);
        if (levelUpdates == nullptr || levelUpdates->empty())
        {
            return false;
        }

        bool hasUpdateSourceWithBufferOrPartialClear = false;
        for (const SubresourceUpdate &update : *levelUpdates)
        {
            if (update.updateSource == UpdateSource::Buffer ||
                update.updateSource == UpdateSource::ClearPartial)
            {
                hasUpdateSourceWithBufferOrPartialClear = true;
                break;
            }
        }
        if (!hasUpdateSourceWithBufferOrPartialClear)
        {
            return false;
        }
    }
    return true;
}

bool ImageHelper::validateSubresourceUpdateBufferRefConsistent(
    RefCounted<BufferHelper> *buffer) const
{
    if (buffer == nullptr)
    {
        return true;
    }

    uint32_t refs = 0;

    for (const std::vector<SubresourceUpdate> &levelUpdates : mSubresourceUpdates)
    {
        for (const SubresourceUpdate &update : levelUpdates)
        {
            if (update.updateSource == UpdateSource::Buffer && update.refCounted.buffer == buffer)
            {
                ++refs;
            }
        }
    }

    return buffer->isRefCountAsExpected(refs);
}

bool ImageHelper::validateSubresourceUpdateImageRefConsistent(RefCounted<ImageHelper> *image) const
{
    if (image == nullptr)
    {
        return true;
    }

    uint32_t refs = 0;

    for (const std::vector<SubresourceUpdate> &levelUpdates : mSubresourceUpdates)
    {
        for (const SubresourceUpdate &update : levelUpdates)
        {
            if (update.updateSource == UpdateSource::Image && update.refCounted.image == image)
            {
                ++refs;
            }
        }
    }

    return image->isRefCountAsExpected(refs);
}

bool ImageHelper::validateSubresourceUpdateRefCountsConsistent() const
{
    for (const std::vector<SubresourceUpdate> &levelUpdates : mSubresourceUpdates)
    {
        for (const SubresourceUpdate &update : levelUpdates)
        {
            if (update.updateSource == UpdateSource::Image)
            {
                if (!validateSubresourceUpdateImageRefConsistent(update.refCounted.image))
                {
                    return false;
                }
            }
            else if (update.updateSource == UpdateSource::Buffer)
            {
                if (!validateSubresourceUpdateBufferRefConsistent(update.refCounted.buffer))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

void ImageHelper::pruneSupersededUpdatesForLevel(ContextVk *contextVk,
                                                 const gl::LevelIndex level,
                                                 const PruneReason reason)
{
    constexpr VkDeviceSize kSubresourceUpdateSizeBeforePruning = 16 * 1024 * 1024;  // 16 MB
    constexpr int kUpdateCountThreshold                        = 1024;
    std::vector<ImageHelper::SubresourceUpdate> *levelUpdates  = getLevelUpdates(level);

    // If we are below pruning threshold, nothing to do.
    const int updateCount      = static_cast<int>(levelUpdates->size());
    const bool withinThreshold = updateCount < kUpdateCountThreshold &&
                                 mTotalStagedBufferUpdateSize < kSubresourceUpdateSizeBeforePruning;
    if (updateCount == 1 || (reason == PruneReason::MemoryOptimization && withinThreshold))
    {
        return;
    }

    // Start from the most recent update and define a boundingBox that covers the region to be
    // updated. Walk through all earlier updates and if its update region is contained within the
    // boundingBox, mark it as superseded, otherwise reset the boundingBox and continue.
    //
    // Color, depth and stencil are the only types supported for now. The boundingBox for color and
    // depth types is at index 0 and index 1 has the boundingBox for stencil type.
    VkDeviceSize supersededUpdateSize  = 0;
    std::array<gl::Box, 2> boundingBox = {gl::Box(gl::kOffsetZero, gl::Extents())};

    auto canDropUpdate = [this, contextVk, level, &supersededUpdateSize,
                          &boundingBox](SubresourceUpdate &update) {
        VkDeviceSize updateSize       = 0;
        VkImageAspectFlags aspectMask = update.getDestAspectFlags();
        gl::Box currentUpdateBox(gl::kOffsetZero, gl::Extents());

        const bool isColor =
            (aspectMask & (VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_PLANE_0_BIT |
                           VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT)) != 0;
        const bool isDepth   = (aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0;
        const bool isStencil = (aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0;
        ASSERT(isColor || isDepth || isStencil);
        int aspectIndex = (isColor || isDepth) ? 0 : 1;

        if (update.updateSource == UpdateSource::Buffer)
        {
            currentUpdateBox = gl::Box(update.data.buffer.copyRegion.imageOffset,
                                       update.data.buffer.copyRegion.imageExtent);
            updateSize       = update.data.buffer.bufferHelper->getSize();
        }
        else if (update.updateSource == UpdateSource::Image)
        {
            currentUpdateBox = gl::Box(update.data.image.copyRegion.dstOffset,
                                       update.data.image.copyRegion.extent);
        }
        else if (update.updateSource == UpdateSource::ClearPartial)
        {
            currentUpdateBox = gl::Box(
                update.data.clearPartial.offset.x, update.data.clearPartial.offset.z,
                update.data.clearPartial.offset.z, update.data.clearPartial.extent.width,
                update.data.clearPartial.extent.height, update.data.clearPartial.extent.depth);
        }
        else
        {
            ASSERT(IsClear(update.updateSource));
            currentUpdateBox = gl::Box(gl::kOffsetZero, getLevelExtents(toVkLevel(level)));
        }

        // Account for updates to layered images
        uint32_t layerIndex = 0;
        uint32_t layerCount = 0;
        update.getDestSubresource(mLayerCount, &layerIndex, &layerCount);
        if (layerIndex > 0 || layerCount > 1)
        {
            currentUpdateBox.z     = layerIndex;
            currentUpdateBox.depth = layerCount;
        }

        // Check if current update region is superseded by the accumulated update region
        if (boundingBox[aspectIndex].contains(currentUpdateBox))
        {
            ANGLE_VK_PERF_WARNING(contextVk, GL_DEBUG_SEVERITY_LOW,
                                  "Dropped texture update that is superseded by a more recent one");

            // Release the superseded update
            update.release(contextVk->getRenderer());

            // Update pruning size
            supersededUpdateSize += updateSize;

            return true;
        }
        else
        {
            // Extend boundingBox to best accommodate current update's box.
            boundingBox[aspectIndex].extend(currentUpdateBox);
            // If the volume of the current update box is larger than the extended boundingBox
            // use that as the new boundingBox instead.
            if (currentUpdateBox.volume() > boundingBox[aspectIndex].volume())
            {
                boundingBox[aspectIndex] = currentUpdateBox;
            }
            return false;
        }
    };

    levelUpdates->erase(
        levelUpdates->begin(),
        std::remove_if(levelUpdates->rbegin(), levelUpdates->rend(), canDropUpdate).base());

    // Update total staging buffer size
    mTotalStagedBufferUpdateSize -= supersededUpdateSize;
}

void ImageHelper::removeSupersededUpdates(ContextVk *contextVk,
                                          const gl::TexLevelMask skipLevelsAllFaces)
{
    ASSERT(validateSubresourceUpdateRefCountsConsistent());

    for (LevelIndex levelVk(0); levelVk < LevelIndex(mLevelCount); ++levelVk)
    {
        gl::LevelIndex levelGL                       = toGLLevel(levelVk);
        std::vector<SubresourceUpdate> *levelUpdates = getLevelUpdates(levelGL);
        if (levelUpdates == nullptr || levelUpdates->size() == 0 ||
            skipLevelsAllFaces.test(levelGL.get()))
        {
            // There are no valid updates to process, continue.
            continue;
        }

        // ClearEmulatedChannelsOnly updates can only be in the beginning of the list of updates.
        // They don't entirely clear the image, so they cannot supersede any update.
        ASSERT(verifyEmulatedClearsAreBeforeOtherUpdates(*levelUpdates));

        pruneSupersededUpdatesForLevel(contextVk, levelGL, PruneReason::MinimizeWorkBeforeFlush);
    }

    ASSERT(validateSubresourceUpdateRefCountsConsistent());
}

angle::Result ImageHelper::copyImageDataToBuffer(ContextVk *contextVk,
                                                 gl::LevelIndex sourceLevelGL,
                                                 uint32_t layerCount,
                                                 uint32_t baseLayer,
                                                 const gl::Box &sourceArea,
                                                 BufferHelper *dstBuffer,
                                                 uint8_t **outDataPtr)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "ImageHelper::copyImageDataToBuffer");
    Renderer *renderer = contextVk->getRenderer();

    const angle::Format &imageFormat = getActualFormat();

    // As noted in the OpenGL ES 3.2 specs, table 8.13, CopyTexImage cannot
    // be used for depth textures. There is no way for the image or buffer
    // used in this function to be of some combined depth and stencil format.
    ASSERT(getAspectFlags() == VK_IMAGE_ASPECT_COLOR_BIT);

    uint32_t pixelBytes = imageFormat.pixelBytes;
    size_t bufferSize =
        sourceArea.width * sourceArea.height * sourceArea.depth * pixelBytes * layerCount;

    const VkImageAspectFlags aspectFlags = getAspectFlags();

    // Allocate staging buffer prefer coherent
    ASSERT(dstBuffer != nullptr && !dstBuffer->valid());
    VkDeviceSize dstOffset;
    ANGLE_TRY(contextVk->initBufferForImageCopy(dstBuffer, bufferSize,
                                                MemoryCoherency::CachedPreferCoherent,
                                                imageFormat.id, &dstOffset, outDataPtr));
    ANGLE_TRY(dstBuffer->flush(contextVk->getRenderer()));

    VkBuffer bufferHandle = dstBuffer->getBuffer().getHandle();

    LevelIndex sourceLevelVk = toVkLevel(sourceLevelGL);

    VkBufferImageCopy regions = {};
    uint32_t regionCount      = 1;
    // Default to non-combined DS case
    regions.bufferOffset                    = dstOffset;
    regions.bufferRowLength                 = 0;
    regions.bufferImageHeight               = 0;
    regions.imageExtent.width               = sourceArea.width;
    regions.imageExtent.height              = sourceArea.height;
    regions.imageExtent.depth               = sourceArea.depth;
    regions.imageOffset.x                   = sourceArea.x;
    regions.imageOffset.y                   = sourceArea.y;
    regions.imageOffset.z                   = sourceArea.z;
    regions.imageSubresource.aspectMask     = aspectFlags;
    regions.imageSubresource.baseArrayLayer = baseLayer;
    regions.imageSubresource.layerCount     = layerCount;
    regions.imageSubresource.mipLevel       = sourceLevelVk.get();

    CommandBufferAccess access;
    access.onBufferTransferWrite(dstBuffer);
    access.onImageTransferRead(aspectFlags, this);

    OutsideRenderPassCommandBuffer *commandBuffer;
    ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

    commandBuffer->copyImageToBuffer(mImage, getCurrentLayout(renderer), bufferHandle, regionCount,
                                     &regions);

    return angle::Result::Continue;
}

angle::Result ImageHelper::copySurfaceImageToBuffer(DisplayVk *displayVk,
                                                    gl::LevelIndex sourceLevelGL,
                                                    uint32_t layerCount,
                                                    uint32_t baseLayer,
                                                    const gl::Box &sourceArea,
                                                    vk::BufferHelper *bufferHelper)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "ImageHelper::copySurfaceImageToBuffer");

    Renderer *renderer = displayVk->getRenderer();

    VkBufferImageCopy region               = {};
    region.bufferOffset                    = 0;
    region.bufferRowLength                 = 0;
    region.bufferImageHeight               = 0;
    region.imageExtent.width               = sourceArea.width;
    region.imageExtent.height              = sourceArea.height;
    region.imageExtent.depth               = sourceArea.depth;
    region.imageOffset.x                   = sourceArea.x;
    region.imageOffset.y                   = sourceArea.y;
    region.imageOffset.z                   = sourceArea.z;
    region.imageSubresource.aspectMask     = getAspectFlags();
    region.imageSubresource.baseArrayLayer = baseLayer;
    region.imageSubresource.layerCount     = layerCount;
    region.imageSubresource.mipLevel       = toVkLevel(sourceLevelGL).get();

    ScopedPrimaryCommandBuffer scopedCommandBuffer(renderer->getDevice());
    ANGLE_TRY(renderer->getCommandBufferOneOff(displayVk, ProtectionType::Unprotected,
                                               &scopedCommandBuffer));
    PrimaryCommandBuffer &primaryCommandBuffer = scopedCommandBuffer.get();

    VkSemaphore acquireNextImageSemaphore;
    recordBarrierOneOffImpl(renderer, getAspectFlags(), ImageLayout::TransferSrc,
                            displayVk->getDeviceQueueIndex(), &primaryCommandBuffer,
                            &acquireNextImageSemaphore);
    primaryCommandBuffer.copyImageToBuffer(mImage, getCurrentLayout(renderer),
                                           bufferHelper->getBuffer().getHandle(), 1, &region);

    ANGLE_VK_TRY(displayVk, primaryCommandBuffer.end());

    QueueSerial submitQueueSerial;
    ANGLE_TRY(renderer->queueSubmitOneOff(
        displayVk, std::move(scopedCommandBuffer), ProtectionType::Unprotected,
        egl::ContextPriority::Medium, acquireNextImageSemaphore,
        kSwapchainAcquireImageWaitStageFlags, &submitQueueSerial));

    return renderer->finishQueueSerial(displayVk, submitQueueSerial);
}

angle::Result ImageHelper::copyBufferToSurfaceImage(DisplayVk *displayVk,
                                                    gl::LevelIndex sourceLevelGL,
                                                    uint32_t layerCount,
                                                    uint32_t baseLayer,
                                                    const gl::Box &sourceArea,
                                                    vk::BufferHelper *bufferHelper)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "ImageHelper::copyBufferToSurfaceImage");

    Renderer *renderer = displayVk->getRenderer();

    VkBufferImageCopy region               = {};
    region.bufferOffset                    = 0;
    region.bufferRowLength                 = 0;
    region.bufferImageHeight               = 0;
    region.imageExtent.width               = sourceArea.width;
    region.imageExtent.height              = sourceArea.height;
    region.imageExtent.depth               = sourceArea.depth;
    region.imageOffset.x                   = sourceArea.x;
    region.imageOffset.y                   = sourceArea.y;
    region.imageOffset.z                   = sourceArea.z;
    region.imageSubresource.aspectMask     = getAspectFlags();
    region.imageSubresource.baseArrayLayer = baseLayer;
    region.imageSubresource.layerCount     = layerCount;
    region.imageSubresource.mipLevel       = toVkLevel(sourceLevelGL).get();

    ScopedPrimaryCommandBuffer scopedCommandBuffer(renderer->getDevice());
    ANGLE_TRY(renderer->getCommandBufferOneOff(displayVk, ProtectionType::Unprotected,
                                               &scopedCommandBuffer));
    PrimaryCommandBuffer &commandBuffer = scopedCommandBuffer.get();

    VkSemaphore acquireNextImageSemaphore;
    recordBarrierOneOffImpl(renderer, getAspectFlags(), ImageLayout::TransferDst,
                            displayVk->getDeviceQueueIndex(), &commandBuffer,
                            &acquireNextImageSemaphore);
    commandBuffer.copyBufferToImage(bufferHelper->getBuffer().getHandle(), mImage,
                                    getCurrentLayout(renderer), 1, &region);

    ANGLE_VK_TRY(displayVk, commandBuffer.end());

    QueueSerial submitQueueSerial;
    ANGLE_TRY(renderer->queueSubmitOneOff(
        displayVk, std::move(scopedCommandBuffer), ProtectionType::Unprotected,
        egl::ContextPriority::Medium, acquireNextImageSemaphore,
        kSwapchainAcquireImageWaitStageFlags, &submitQueueSerial));

    return renderer->finishQueueSerial(displayVk, submitQueueSerial);
}

// static
angle::Result ImageHelper::GetReadPixelsParams(ContextVk *contextVk,
                                               const gl::PixelPackState &packState,
                                               gl::Buffer *packBuffer,
                                               GLenum format,
                                               GLenum type,
                                               const gl::Rectangle &area,
                                               const gl::Rectangle &clippedArea,
                                               PackPixelsParams *paramsOut,
                                               GLuint *skipBytesOut)
{
    const gl::InternalFormat &sizedFormatInfo = gl::GetInternalFormatInfo(format, type);

    GLuint outputPitch = 0;
    ANGLE_VK_CHECK_MATH(contextVk,
                        sizedFormatInfo.computeRowPitch(type, area.width, packState.alignment,
                                                        packState.rowLength, &outputPitch));
    ANGLE_VK_CHECK_MATH(contextVk, sizedFormatInfo.computeSkipBytes(type, outputPitch, 0, packState,
                                                                    false, skipBytesOut));

    ANGLE_TRY(GetPackPixelsParams(sizedFormatInfo, outputPitch, packState, packBuffer, area,
                                  clippedArea, paramsOut, skipBytesOut));
    return angle::Result::Continue;
}

angle::Result ImageHelper::readPixelsForGetImage(ContextVk *contextVk,
                                                 const gl::PixelPackState &packState,
                                                 gl::Buffer *packBuffer,
                                                 gl::LevelIndex levelGL,
                                                 uint32_t layer,
                                                 uint32_t layerCount,
                                                 GLenum format,
                                                 GLenum type,
                                                 void *pixels)
{
    const angle::Format &angleFormat = GetFormatFromFormatType(format, type);

    VkImageAspectFlagBits aspectFlags = {};
    if (angleFormat.redBits > 0 || angleFormat.blueBits > 0 || angleFormat.greenBits > 0 ||
        angleFormat.alphaBits > 0 || angleFormat.luminanceBits > 0)
    {
        aspectFlags = static_cast<VkImageAspectFlagBits>(aspectFlags | VK_IMAGE_ASPECT_COLOR_BIT);
    }
    else
    {
        if (angleFormat.depthBits > 0)
        {
            aspectFlags =
                static_cast<VkImageAspectFlagBits>(aspectFlags | VK_IMAGE_ASPECT_DEPTH_BIT);
        }
        if (angleFormat.stencilBits > 0)
        {
            aspectFlags =
                static_cast<VkImageAspectFlagBits>(aspectFlags | VK_IMAGE_ASPECT_STENCIL_BIT);
        }
    }

    ASSERT(aspectFlags != 0);

    PackPixelsParams params;
    GLuint outputSkipBytes = 0;

    const LevelIndex levelVk     = toVkLevel(levelGL);
    const gl::Extents mipExtents = getLevelExtents(levelVk);
    gl::Rectangle area(0, 0, mipExtents.width, mipExtents.height);

    ANGLE_TRY(GetReadPixelsParams(contextVk, packState, packBuffer, format, type, area, area,
                                  &params, &outputSkipBytes));

    if (mExtents.depth > 1 || layerCount > 1)
    {
        ASSERT(layer == 0);
        ASSERT(layerCount == 1 || mipExtents.depth == 1);

        uint32_t lastLayer = std::max(static_cast<uint32_t>(mipExtents.depth), layerCount);

        // Depth > 1 means this is a 3D texture and we need to copy all layers
        for (uint32_t mipLayer = 0; mipLayer < lastLayer; mipLayer++)
        {
            ANGLE_TRY(readPixels(contextVk, area, params, aspectFlags, levelGL, mipLayer,
                                 static_cast<uint8_t *>(pixels) + outputSkipBytes));

            outputSkipBytes += mipExtents.width * mipExtents.height *
                               gl::GetInternalFormatInfo(format, type).pixelBytes;
        }
    }
    else
    {
        ANGLE_TRY(readPixels(contextVk, area, params, aspectFlags, levelGL, layer,
                             static_cast<uint8_t *>(pixels) + outputSkipBytes));
    }

    return angle::Result::Continue;
}

angle::Result ImageHelper::readPixelsForCompressedGetImage(ContextVk *contextVk,
                                                           const gl::PixelPackState &packState,
                                                           gl::Buffer *packBuffer,
                                                           gl::LevelIndex levelGL,
                                                           uint32_t layer,
                                                           uint32_t layerCount,
                                                           void *pixels)
{
    PackPixelsParams params;
    GLuint outputSkipBytes = 0;

    const LevelIndex levelVk = toVkLevel(levelGL);
    gl::Extents mipExtents   = getLevelExtents(levelVk);
    gl::Rectangle area(0, 0, mipExtents.width, mipExtents.height);

    VkImageAspectFlagBits aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

    const angle::Format *readFormat = &getActualFormat();

    // TODO(anglebug.com/42264702): Implement encoding for emuluated compression formats
    ANGLE_VK_CHECK(contextVk, readFormat->isBlock, VK_ERROR_FORMAT_NOT_SUPPORTED);

    if (mExtents.depth > 1 || layerCount > 1)
    {
        ASSERT(layer == 0);
        ASSERT(layerCount == 1 || mipExtents.depth == 1);

        uint32_t lastLayer = std::max(static_cast<uint32_t>(mipExtents.depth), layerCount);

        const vk::Format &vkFormat = contextVk->getRenderer()->getFormat(readFormat->id);
        const gl::InternalFormat &storageFormatInfo =
            vkFormat.getInternalFormatInfo(readFormat->componentType);

        // Calculate size for one layer
        mipExtents.depth = 1;
        GLuint layerSize;
        ANGLE_VK_CHECK_MATH(contextVk,
                            storageFormatInfo.computeCompressedImageSize(mipExtents, &layerSize));

        // Depth > 1 means this is a 3D texture and we need to copy all layers
        for (uint32_t mipLayer = 0; mipLayer < lastLayer; mipLayer++)
        {
            ANGLE_TRY(readPixels(contextVk, area, params, aspectFlags, levelGL, mipLayer,
                                 static_cast<uint8_t *>(pixels) + outputSkipBytes));
            outputSkipBytes += layerSize;
        }
    }
    else
    {
        ANGLE_TRY(readPixels(contextVk, area, params, aspectFlags, levelGL, layer,
                             static_cast<uint8_t *>(pixels) + outputSkipBytes));
    }

    return angle::Result::Continue;
}

angle::Result ImageHelper::readPixelsWithCompute(ContextVk *contextVk,
                                                 ImageHelper *src,
                                                 const PackPixelsParams &packPixelsParams,
                                                 const VkOffset3D &srcOffset,
                                                 const VkExtent3D &srcExtent,
                                                 ptrdiff_t pixelsOffset,
                                                 const VkImageSubresourceLayers &srcSubresource)
{
    ASSERT(srcOffset.z == 0 || srcSubresource.baseArrayLayer == 0);

    UtilsVk::CopyImageToBufferParameters params = {};
    params.srcOffset[0]                         = srcOffset.x;
    params.srcOffset[1]                         = srcOffset.y;
    params.srcLayer        = std::max<uint32_t>(srcOffset.z, srcSubresource.baseArrayLayer);
    params.srcMip          = LevelIndex(srcSubresource.mipLevel);
    params.size[0]         = srcExtent.width;
    params.size[1]         = srcExtent.height;
    params.outputOffset    = packPixelsParams.offset + pixelsOffset;
    params.outputPitch     = packPixelsParams.outputPitch;
    params.reverseRowOrder = packPixelsParams.reverseRowOrder;
    params.outputFormat    = packPixelsParams.destFormat;

    BufferHelper &packBuffer = GetImpl(packPixelsParams.packBuffer)->getBuffer();

    return contextVk->getUtils().copyImageToBuffer(contextVk, &packBuffer, src, params);
}

bool ImageHelper::canCopyWithTransformForReadPixels(const PackPixelsParams &packPixelsParams,
                                                    const VkExtent3D &srcExtent,
                                                    const angle::Format *readFormat,
                                                    ptrdiff_t pixelsOffset)
{
    ASSERT(mActualFormatID != angle::FormatID::NONE && mIntendedFormatID != angle::FormatID::NONE);

    // Only allow copies to PBOs with identical format.
    const bool isSameFormatCopy = *readFormat == *packPixelsParams.destFormat;

    // Disallow any transformation.
    const bool needsTransformation =
        packPixelsParams.rotation != SurfaceRotation::Identity || packPixelsParams.reverseRowOrder;

    // Disallow copies when the output pitch cannot be correctly specified in Vulkan.
    const bool isPitchMultipleOfTexelSize =
        packPixelsParams.outputPitch % readFormat->pixelBytes == 0;

    // Disallow copies when PBO offset violates Vulkan bufferOffset alignment requirements.
    const BufferHelper &packBuffer = GetImpl(packPixelsParams.packBuffer)->getBuffer();
    const VkDeviceSize offset = packBuffer.getOffset() + packPixelsParams.offset + pixelsOffset;
    const bool isOffsetMultipleOfTexelSize = offset % readFormat->pixelBytes == 0;

    // Disallow copies when PBO row length is smaller than the source area width.
    const bool isRowLengthEnough =
        packPixelsParams.outputPitch >= srcExtent.width * readFormat->pixelBytes;

    // Don't allow copies from emulated formats for simplicity.
    return !hasEmulatedImageFormat() && isSameFormatCopy && !needsTransformation &&
           isPitchMultipleOfTexelSize && isOffsetMultipleOfTexelSize && isRowLengthEnough;
}

bool ImageHelper::canCopyWithComputeForReadPixels(const PackPixelsParams &packPixelsParams,
                                                  const VkExtent3D &srcExtent,
                                                  const angle::Format *readFormat,
                                                  ptrdiff_t pixelsOffset)
{
    ASSERT(mActualFormatID != angle::FormatID::NONE && mIntendedFormatID != angle::FormatID::NONE);
    const angle::Format *writeFormat = packPixelsParams.destFormat;

    // For now, only float formats are supported with 4-byte 4-channel normalized pixels for output.
    const bool isFloat =
        !readFormat->isSint() && !readFormat->isUint() && !readFormat->hasDepthOrStencilBits();
    const bool isFourByteOutput   = writeFormat->pixelBytes == 4 && writeFormat->channelCount == 4;
    const bool isNormalizedOutput = writeFormat->isUnorm() || writeFormat->isSnorm();

    // Disallow rotation.
    const bool needsTransformation = packPixelsParams.rotation != SurfaceRotation::Identity;

    // Disallow copies when the output pitch cannot be correctly specified in Vulkan.
    const bool isPitchMultipleOfTexelSize =
        packPixelsParams.outputPitch % readFormat->pixelBytes == 0;

    // Disallow copies when the output offset is not aligned to uint32_t
    const bool isOffsetMultipleOfUint =
        (packPixelsParams.offset + pixelsOffset) % readFormat->pixelBytes == 0;

    // Disallow copies when PBO row length is smaller than the source area width.
    const bool isRowLengthEnough =
        packPixelsParams.outputPitch >= srcExtent.width * readFormat->pixelBytes;

    return isFloat && isFourByteOutput && isNormalizedOutput && !needsTransformation &&
           isPitchMultipleOfTexelSize && isOffsetMultipleOfUint && isRowLengthEnough;
}

angle::Result ImageHelper::readPixels(ContextVk *contextVk,
                                      const gl::Rectangle &area,
                                      const PackPixelsParams &packPixelsParams,
                                      VkImageAspectFlagBits copyAspectFlags,
                                      gl::LevelIndex levelGL,
                                      uint32_t layer,
                                      void *pixels)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "ImageHelper::readPixels");

    const angle::Format &readFormat = getActualFormat();

    if (readFormat.depthBits == 0)
    {
        copyAspectFlags =
            static_cast<VkImageAspectFlagBits>(copyAspectFlags & ~VK_IMAGE_ASPECT_DEPTH_BIT);
    }
    if (readFormat.stencilBits == 0)
    {
        copyAspectFlags =
            static_cast<VkImageAspectFlagBits>(copyAspectFlags & ~VK_IMAGE_ASPECT_STENCIL_BIT);
    }

    if (copyAspectFlags == IMAGE_ASPECT_DEPTH_STENCIL)
    {
        const angle::Format &depthFormat =
            GetDepthStencilImageToBufferFormat(readFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
        const angle::Format &stencilFormat =
            GetDepthStencilImageToBufferFormat(readFormat, VK_IMAGE_ASPECT_STENCIL_BIT);

        int depthOffset   = 0;
        int stencilOffset = 0;
        switch (readFormat.id)
        {
            case angle::FormatID::D24_UNORM_S8_UINT:
                depthOffset   = 1;
                stencilOffset = 0;
                break;

            case angle::FormatID::D32_FLOAT_S8X24_UINT:
                depthOffset   = 0;
                stencilOffset = 4;
                break;

            default:
                UNREACHABLE();
        }

        ASSERT(depthOffset > 0 || stencilOffset > 0);
        ASSERT(depthOffset + depthFormat.depthBits / 8 <= readFormat.pixelBytes);
        ASSERT(stencilOffset + stencilFormat.stencilBits / 8 <= readFormat.pixelBytes);

        // Read the depth values, tightly-packed
        angle::MemoryBuffer depthBuffer;
        ANGLE_VK_CHECK_ALLOC(contextVk,
                             depthBuffer.resize(depthFormat.pixelBytes * area.width * area.height));
        ANGLE_TRY(
            readPixelsImpl(contextVk, area,
                           PackPixelsParams(area, depthFormat, depthFormat.pixelBytes * area.width,
                                            false, nullptr, 0),
                           VK_IMAGE_ASPECT_DEPTH_BIT, levelGL, layer, depthBuffer.data()));

        // Read the stencil values, tightly-packed
        angle::MemoryBuffer stencilBuffer;
        ANGLE_VK_CHECK_ALLOC(
            contextVk, stencilBuffer.resize(stencilFormat.pixelBytes * area.width * area.height));
        ANGLE_TRY(readPixelsImpl(
            contextVk, area,
            PackPixelsParams(area, stencilFormat, stencilFormat.pixelBytes * area.width, false,
                             nullptr, 0),
            VK_IMAGE_ASPECT_STENCIL_BIT, levelGL, layer, stencilBuffer.data()));

        // Interleave them together
        angle::MemoryBuffer readPixelBuffer;
        ANGLE_VK_CHECK_ALLOC(
            contextVk, readPixelBuffer.resize(readFormat.pixelBytes * area.width * area.height));
        readPixelBuffer.fill(0);
        for (int i = 0; i < area.width * area.height; i++)
        {
            uint8_t *readPixel = readPixelBuffer.data() + i * readFormat.pixelBytes;
            memcpy(readPixel + depthOffset, depthBuffer.data() + i * depthFormat.pixelBytes,
                   depthFormat.depthBits / 8);
            memcpy(readPixel + stencilOffset, stencilBuffer.data() + i * stencilFormat.pixelBytes,
                   stencilFormat.stencilBits / 8);
        }

        // Pack the interleaved depth and stencil into user-provided
        // destination, per user's pack pixels params

        // The compressed format path in packReadPixelBuffer isn't applicable
        // to our case, let's make extra sure we won't hit it
        ASSERT(!readFormat.isBlock);
        return packReadPixelBuffer(contextVk, area, packPixelsParams, readFormat, readFormat,
                                   readPixelBuffer.data(), levelGL, pixels);
    }

    return readPixelsImpl(contextVk, area, packPixelsParams, copyAspectFlags, levelGL, layer,
                          pixels);
}

angle::Result ImageHelper::readPixelsImpl(ContextVk *contextVk,
                                          const gl::Rectangle &area,
                                          const PackPixelsParams &packPixelsParams,
                                          VkImageAspectFlagBits copyAspectFlags,
                                          gl::LevelIndex levelGL,
                                          uint32_t layer,
                                          void *pixels)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "ImageHelper::readPixelsImpl");

    Renderer *renderer = contextVk->getRenderer();

    bool isExternalFormat = getExternalFormat() != 0;
    ASSERT(!isExternalFormat || (mActualFormatID >= angle::FormatID::EXTERNAL0 &&
                                 mActualFormatID <= angle::FormatID::EXTERNAL7));

    // If the source image is multisampled, we need to resolve it into a temporary image before
    // performing a readback.
    bool isMultisampled = mSamples > 1;
    RendererScoped<ImageHelper> resolvedImage(contextVk->getRenderer());

    ImageHelper *src = this;

    ASSERT(!hasStagedUpdatesForSubresource(levelGL, layer, 1));

    if (isMultisampled)
    {
        ANGLE_TRY(resolvedImage.get().init2DStaging(
            contextVk, contextVk->getState().hasProtectedContent(), renderer->getMemoryProperties(),
            gl::Extents(area.width, area.height, 1), mIntendedFormatID, mActualFormatID,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT,
            1));
    }
    else if (isExternalFormat)
    {
        ANGLE_TRY(resolvedImage.get().init2DStaging(
            contextVk, contextVk->getState().hasProtectedContent(), renderer->getMemoryProperties(),
            gl::Extents(area.width, area.height, 1), angle::FormatID::R8G8B8A8_UNORM,
            angle::FormatID::R8G8B8A8_UNORM,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            1));
    }

    VkImageAspectFlags layoutChangeAspectFlags = src->getAspectFlags();

    const angle::Format *rgbaFormat = &angle::Format::Get(angle::FormatID::R8G8B8A8_UNORM);
    const angle::Format *readFormat = isExternalFormat ? rgbaFormat : &getActualFormat();
    const vk::Format &vkFormat      = contextVk->getRenderer()->getFormat(readFormat->id);
    const gl::InternalFormat &storageFormatInfo =
        vkFormat.getInternalFormatInfo(readFormat->componentType);

    if (copyAspectFlags != VK_IMAGE_ASPECT_COLOR_BIT)
    {
        readFormat = &GetDepthStencilImageToBufferFormat(*readFormat, copyAspectFlags);
    }

    VkOffset3D srcOffset = {area.x, area.y, 0};

    VkImageSubresourceLayers srcSubresource = {};
    srcSubresource.aspectMask               = copyAspectFlags;
    srcSubresource.mipLevel                 = toVkLevel(levelGL).get();
    srcSubresource.baseArrayLayer           = layer;
    srcSubresource.layerCount               = 1;

    VkExtent3D srcExtent = {static_cast<uint32_t>(area.width), static_cast<uint32_t>(area.height),
                            1};

    if (mExtents.depth > 1)
    {
        // Depth > 1 means this is a 3D texture and we need special handling
        srcOffset.z                   = layer;
        srcSubresource.baseArrayLayer = 0;
    }

    if (isExternalFormat)
    {
        CommandBufferAccess access;
        access.onImageTransferRead(layoutChangeAspectFlags, this);
        OutsideRenderPassCommandBuffer *commandBuffer;
        ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

        // Create some temp views because copyImage works in terms of them
        gl::TextureType textureType = Get2DTextureType(1, resolvedImage.get().getSamples());

        // Surely we have a view of this already!
        vk::ImageView srcView;
        ANGLE_TRY(src->initLayerImageView(contextVk, textureType, VK_IMAGE_ASPECT_COLOR_BIT,
                                          gl::SwizzleState(), &srcView, vk::LevelIndex(0), 1, 0,
                                          mLayerCount));
        vk::ImageView stagingView;
        ANGLE_TRY(resolvedImage.get().initLayerImageView(
            contextVk, textureType, VK_IMAGE_ASPECT_COLOR_BIT, gl::SwizzleState(), &stagingView,
            vk::LevelIndex(0), 1, 0, mLayerCount));

        UtilsVk::CopyImageParameters params = {};
        params.srcOffset[0]                 = srcOffset.x;
        params.srcOffset[1]                 = srcOffset.y;
        params.srcExtents[0]                = srcExtent.width;
        params.srcExtents[1]                = srcExtent.height;
        params.srcHeight                    = srcExtent.height;
        ANGLE_TRY(contextVk->getUtils().copyImage(contextVk, &resolvedImage.get(), &stagingView,
                                                  src, &srcView, params));

        CommandBufferAccess readAccess;
        readAccess.onImageTransferRead(layoutChangeAspectFlags, &resolvedImage.get());
        ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(readAccess, &commandBuffer));

        // Make the resolved image the target of buffer copy
        src                           = &resolvedImage.get();
        srcOffset                     = {0, 0, 0};
        srcSubresource.baseArrayLayer = 0;
        srcSubresource.layerCount     = 1;
        srcSubresource.mipLevel       = 0;

        // Mark our temp views as garbage immediately
        contextVk->addGarbage(&srcView);
        contextVk->addGarbage(&stagingView);
    }

    if (isMultisampled)
    {
        CommandBufferAccess access;
        access.onImageTransferRead(layoutChangeAspectFlags, this);
        access.onImageTransferWrite(gl::LevelIndex(0), 1, 0, 1, layoutChangeAspectFlags,
                                    &resolvedImage.get());

        OutsideRenderPassCommandBuffer *commandBuffer;
        ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

        // Note: resolve only works on color images (not depth/stencil).
        ASSERT(copyAspectFlags == VK_IMAGE_ASPECT_COLOR_BIT);

        VkImageResolve resolveRegion                = {};
        resolveRegion.srcSubresource                = srcSubresource;
        resolveRegion.srcOffset                     = srcOffset;
        resolveRegion.dstSubresource.aspectMask     = copyAspectFlags;
        resolveRegion.dstSubresource.mipLevel       = 0;
        resolveRegion.dstSubresource.baseArrayLayer = 0;
        resolveRegion.dstSubresource.layerCount     = 1;
        resolveRegion.dstOffset                     = {};
        resolveRegion.extent                        = srcExtent;

        resolve(&resolvedImage.get(), resolveRegion, commandBuffer);

        // Make the resolved image the target of buffer copy.
        src                           = &resolvedImage.get();
        srcOffset                     = {0, 0, 0};
        srcSubresource.baseArrayLayer = 0;
        srcSubresource.layerCount     = 1;
        srcSubresource.mipLevel       = 0;
    }

    // If PBO and if possible, copy directly on the GPU.
    if (packPixelsParams.packBuffer)
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "ImageHelper::readPixelsImpl - PBO");

        const ptrdiff_t pixelsOffset = reinterpret_cast<ptrdiff_t>(pixels);
        if (canCopyWithTransformForReadPixels(packPixelsParams, srcExtent, readFormat,
                                              pixelsOffset))
        {
            BufferHelper &packBuffer      = GetImpl(packPixelsParams.packBuffer)->getBuffer();
            VkDeviceSize packBufferOffset = packBuffer.getOffset();

            CommandBufferAccess copyAccess;
            copyAccess.onBufferTransferWrite(&packBuffer);
            copyAccess.onImageTransferRead(layoutChangeAspectFlags, src);

            OutsideRenderPassCommandBuffer *copyCommandBuffer;
            ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(copyAccess, &copyCommandBuffer));

            ASSERT(packPixelsParams.outputPitch % readFormat->pixelBytes == 0);

            VkBufferImageCopy region = {};
            region.bufferImageHeight = srcExtent.height;
            region.bufferOffset      = packBufferOffset + packPixelsParams.offset + pixelsOffset;
            region.bufferRowLength   = packPixelsParams.outputPitch / readFormat->pixelBytes;
            region.imageExtent       = srcExtent;
            region.imageOffset       = srcOffset;
            region.imageSubresource  = srcSubresource;

            copyCommandBuffer->copyImageToBuffer(src->getImage(), src->getCurrentLayout(renderer),
                                                 packBuffer.getBuffer().getHandle(), 1, &region);
            return angle::Result::Continue;
        }
        if (canCopyWithComputeForReadPixels(packPixelsParams, srcExtent, readFormat, pixelsOffset))
        {
            ANGLE_TRY(readPixelsWithCompute(contextVk, src, packPixelsParams, srcOffset, srcExtent,
                                            pixelsOffset, srcSubresource));
            return angle::Result::Continue;
        }
    }

    ANGLE_TRACE_EVENT0("gpu.angle", "ImageHelper::readPixelsImpl - CPU Readback");

    RendererScoped<vk::BufferHelper> readBuffer(renderer);
    vk::BufferHelper *stagingBuffer = &readBuffer.get();

    uint8_t *readPixelBuffer   = nullptr;
    VkDeviceSize stagingOffset = 0;
    size_t allocationSize      = readFormat->pixelBytes * area.width * area.height;

    ANGLE_TRY(contextVk->initBufferForImageCopy(stagingBuffer, allocationSize,
                                                MemoryCoherency::CachedPreferCoherent,
                                                readFormat->id, &stagingOffset, &readPixelBuffer));
    ANGLE_TRY(stagingBuffer->flush(renderer));
    VkBuffer bufferHandle = stagingBuffer->getBuffer().getHandle();

    VkBufferImageCopy region = {};
    region.bufferImageHeight = srcExtent.height;
    region.bufferOffset      = stagingOffset;
    region.bufferRowLength   = srcExtent.width;
    region.imageExtent       = srcExtent;
    region.imageOffset       = srcOffset;
    region.imageSubresource  = srcSubresource;

    // For compressed textures, vkCmdCopyImageToBuffer requires
    // a region that is a multiple of the block size.
    if (readFormat->isBlock)
    {
        region.bufferRowLength =
            roundUp(region.bufferRowLength, storageFormatInfo.compressedBlockWidth);
        region.bufferImageHeight =
            roundUp(region.bufferImageHeight, storageFormatInfo.compressedBlockHeight);
    }

    CommandBufferAccess readbackAccess;
    readbackAccess.onBufferTransferWrite(stagingBuffer);
    readbackAccess.onImageTransferRead(layoutChangeAspectFlags, src);

    OutsideRenderPassCommandBuffer *readbackCommandBuffer;
    ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(readbackAccess, &readbackCommandBuffer));

    readbackCommandBuffer->copyImageToBuffer(src->getImage(), src->getCurrentLayout(renderer),
                                             bufferHandle, 1, &region);

    ANGLE_VK_PERF_WARNING(contextVk, GL_DEBUG_SEVERITY_HIGH, "GPU stall due to ReadPixels");

    // Triggers a full finish.
    ANGLE_TRY(contextVk->finishImpl(RenderPassClosureReason::GLReadPixels));
    // invalidate must be called after wait for finish.
    ANGLE_TRY(stagingBuffer->invalidate(renderer));

    return packReadPixelBuffer(contextVk, area, packPixelsParams, getActualFormat(), *readFormat,
                               readPixelBuffer, levelGL, pixels);
}

angle::Result ImageHelper::packReadPixelBuffer(ContextVk *contextVk,
                                               const gl::Rectangle &area,
                                               const PackPixelsParams &packPixelsParams,
                                               const angle::Format &readFormat,
                                               const angle::Format &aspectFormat,
                                               const uint8_t *readPixelBuffer,
                                               gl::LevelIndex levelGL,
                                               void *pixels)
{
    const vk::Format &vkFormat = contextVk->getRenderer()->getFormat(readFormat.id);
    const gl::InternalFormat &storageFormatInfo =
        vkFormat.getInternalFormatInfo(readFormat.componentType);

    if (readFormat.isBlock)
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "ImageHelper::packReadPixelBuffer - Block");

        ASSERT(readFormat == aspectFormat);

        const LevelIndex levelVk = toVkLevel(levelGL);
        gl::Extents levelExtents = getLevelExtents(levelVk);

        // Calculate size of one layer
        levelExtents.depth = 1;
        GLuint layerSize;
        ANGLE_VK_CHECK_MATH(contextVk,
                            storageFormatInfo.computeCompressedImageSize(levelExtents, &layerSize));
        memcpy(pixels, readPixelBuffer, layerSize);
    }
    else if (packPixelsParams.packBuffer)
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "ImageHelper::packReadPixelBuffer - PBO");

        // Must map the PBO in order to read its contents (and then unmap it later)
        BufferVk *packBufferVk = GetImpl(packPixelsParams.packBuffer);
        void *mapPtr           = nullptr;
        ANGLE_TRY(packBufferVk->mapImpl(contextVk, GL_MAP_WRITE_BIT, &mapPtr));
        uint8_t *dst = static_cast<uint8_t *>(mapPtr) + reinterpret_cast<ptrdiff_t>(pixels);
        PackPixels(packPixelsParams, aspectFormat, area.width * aspectFormat.pixelBytes,
                   readPixelBuffer, dst);
        ANGLE_TRY(packBufferVk->unmapImpl(contextVk));
    }
    else
    {
        PackPixels(packPixelsParams, aspectFormat, area.width * aspectFormat.pixelBytes,
                   readPixelBuffer, static_cast<uint8_t *>(pixels));
    }

    return angle::Result::Continue;
}

// ImageHelper::SubresourceUpdate implementation
ImageHelper::SubresourceUpdate::SubresourceUpdate() : updateSource(UpdateSource::Buffer)
{
    data.buffer.bufferHelper = nullptr;
    refCounted.buffer        = nullptr;
}

ImageHelper::SubresourceUpdate::SubresourceUpdate(const VkImageAspectFlags aspectFlags,
                                                  const VkClearValue &clearValue,
                                                  const gl::TextureType textureType,
                                                  const uint32_t levelIndex,
                                                  const uint32_t layerIndex,
                                                  const uint32_t layerCount,
                                                  const gl::Box &clearArea)
    : updateSource(UpdateSource::ClearPartial)
{
    data.clearPartial.aspectFlags = aspectFlags;
    data.clearPartial.levelIndex  = levelIndex;
    data.clearPartial.textureType = textureType;
    data.clearPartial.layerIndex  = layerIndex;
    data.clearPartial.layerCount  = layerCount;
    data.clearPartial.offset      = {clearArea.x, clearArea.y, clearArea.z};
    data.clearPartial.extent      = {static_cast<uint32_t>(clearArea.width),
                                     static_cast<uint32_t>(clearArea.height),
                                     static_cast<uint32_t>(clearArea.depth)};
    data.clearPartial.clearValue  = clearValue;
}

ImageHelper::SubresourceUpdate::~SubresourceUpdate() {}

ImageHelper::SubresourceUpdate::SubresourceUpdate(RefCounted<BufferHelper> *bufferIn,
                                                  BufferHelper *bufferHelperIn,
                                                  const VkBufferImageCopy &copyRegionIn,
                                                  angle::FormatID formatID)
    : updateSource(UpdateSource::Buffer)
{
    refCounted.buffer = bufferIn;
    if (refCounted.buffer != nullptr)
    {
        refCounted.buffer->addRef();
    }
    data.buffer.bufferHelper = bufferHelperIn;
    data.buffer.copyRegion   = copyRegionIn;
    data.buffer.formatID     = formatID;
}

ImageHelper::SubresourceUpdate::SubresourceUpdate(RefCounted<ImageHelper> *imageIn,
                                                  const VkImageCopy &copyRegionIn,
                                                  angle::FormatID formatID)
    : updateSource(UpdateSource::Image)
{
    refCounted.image = imageIn;
    refCounted.image->addRef();
    data.image.copyRegion = copyRegionIn;
    data.image.formatID   = formatID;
}

ImageHelper::SubresourceUpdate::SubresourceUpdate(VkImageAspectFlags aspectFlags,
                                                  const VkClearValue &clearValue,
                                                  const gl::ImageIndex &imageIndex)
    : SubresourceUpdate(
          aspectFlags,
          clearValue,
          gl::LevelIndex(imageIndex.getLevelIndex()),
          imageIndex.hasLayer() ? imageIndex.getLayerIndex() : 0,
          imageIndex.hasLayer() ? imageIndex.getLayerCount() : VK_REMAINING_ARRAY_LAYERS)
{}

ImageHelper::SubresourceUpdate::SubresourceUpdate(VkImageAspectFlags aspectFlags,
                                                  const VkClearValue &clearValue,
                                                  gl::LevelIndex level,
                                                  uint32_t layerIndex,
                                                  uint32_t layerCount)
    : updateSource(UpdateSource::Clear)
{
    refCounted.image          = nullptr;
    data.clear.aspectFlags    = aspectFlags;
    data.clear.value          = clearValue;
    data.clear.levelIndex     = level.get();
    data.clear.layerIndex     = layerIndex;
    data.clear.layerCount     = layerCount;
    data.clear.colorMaskFlags = 0;
}

ImageHelper::SubresourceUpdate::SubresourceUpdate(VkColorComponentFlags colorMaskFlags,
                                                  const VkClearColorValue &clearValue,
                                                  const gl::ImageIndex &imageIndex)
    : updateSource(UpdateSource::ClearEmulatedChannelsOnly)
{
    refCounted.image       = nullptr;
    data.clear.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    data.clear.value.color = clearValue;
    data.clear.levelIndex  = imageIndex.getLevelIndex();
    data.clear.layerIndex  = imageIndex.hasLayer() ? imageIndex.getLayerIndex() : 0;
    data.clear.layerCount =
        imageIndex.hasLayer() ? imageIndex.getLayerCount() : VK_REMAINING_ARRAY_LAYERS;
    data.clear.colorMaskFlags = colorMaskFlags;
}

ImageHelper::SubresourceUpdate::SubresourceUpdate(SubresourceUpdate &&other)
    : updateSource(other.updateSource)
{
    switch (updateSource)
    {
        case UpdateSource::Clear:
        case UpdateSource::ClearEmulatedChannelsOnly:
        case UpdateSource::ClearAfterInvalidate:
            data.clear        = other.data.clear;
            refCounted.buffer = nullptr;
            break;
        case UpdateSource::ClearPartial:
            data.clearPartial = other.data.clearPartial;
            break;
        case UpdateSource::Buffer:
            data.buffer             = other.data.buffer;
            refCounted.buffer       = other.refCounted.buffer;
            other.refCounted.buffer = nullptr;
            break;
        case UpdateSource::Image:
            data.image             = other.data.image;
            refCounted.image       = other.refCounted.image;
            other.refCounted.image = nullptr;
            break;
        default:
            UNREACHABLE();
    }
}

ImageHelper::SubresourceUpdate &ImageHelper::SubresourceUpdate::operator=(SubresourceUpdate &&other)
{
    // Given that the update is a union of three structs, we can't use std::swap on the fields.  For
    // example, |this| may be an Image update and |other| may be a Buffer update.
    // The following could work:
    //
    // SubresourceUpdate oldThis;
    // Set oldThis to this->field based on updateSource
    // Set this->otherField to other.otherField based on other.updateSource
    // Set other.field to oldThis->field based on updateSource
    // std::Swap(updateSource, other.updateSource);
    //
    // It's much simpler to just swap the memory instead.

    SubresourceUpdate oldThis;
    memcpy(&oldThis, this, sizeof(*this));
    memcpy(this, &other, sizeof(*this));
    memcpy(&other, &oldThis, sizeof(*this));

    return *this;
}

void ImageHelper::SubresourceUpdate::release(Renderer *renderer)
{
    if (updateSource == UpdateSource::Image)
    {
        refCounted.image->releaseRef();

        if (!refCounted.image->isReferenced())
        {
            // Staging images won't be used in render pass attachments.
            refCounted.image->get().releaseImage(renderer);
            refCounted.image->get().releaseStagedUpdates(renderer);
            SafeDelete(refCounted.image);
        }

        refCounted.image = nullptr;
    }
    else if (updateSource == UpdateSource::Buffer && refCounted.buffer != nullptr)
    {
        refCounted.buffer->releaseRef();

        if (!refCounted.buffer->isReferenced())
        {
            refCounted.buffer->get().release(renderer);
            SafeDelete(refCounted.buffer);
        }

        refCounted.buffer = nullptr;
    }
}

bool ImageHelper::SubresourceUpdate::matchesLayerRange(uint32_t layerIndex,
                                                       uint32_t layerCount) const
{
    uint32_t updateBaseLayer, updateLayerCount;
    getDestSubresource(gl::ImageIndex::kEntireLevel, &updateBaseLayer, &updateLayerCount);

    return updateBaseLayer == layerIndex &&
           (updateLayerCount == layerCount || updateLayerCount == VK_REMAINING_ARRAY_LAYERS);
}

bool ImageHelper::SubresourceUpdate::intersectsLayerRange(uint32_t layerIndex,
                                                          uint32_t layerCount) const
{
    uint32_t updateBaseLayer, updateLayerCount;
    getDestSubresource(gl::ImageIndex::kEntireLevel, &updateBaseLayer, &updateLayerCount);
    uint32_t updateLayerEnd = updateBaseLayer + updateLayerCount;

    return updateBaseLayer < (layerIndex + layerCount) && updateLayerEnd > layerIndex;
}

void ImageHelper::SubresourceUpdate::getDestSubresource(uint32_t imageLayerCount,
                                                        uint32_t *baseLayerOut,
                                                        uint32_t *layerCountOut) const
{
    if (IsClear(updateSource))
    {
        *baseLayerOut  = data.clear.layerIndex;
        *layerCountOut = data.clear.layerCount;

        if (*layerCountOut == static_cast<uint32_t>(gl::ImageIndex::kEntireLevel))
        {
            *layerCountOut = imageLayerCount;
        }
    }
    else if (updateSource == UpdateSource::ClearPartial)
    {
        *baseLayerOut  = data.clearPartial.layerIndex;
        *layerCountOut = data.clearPartial.layerCount;

        if (*layerCountOut == static_cast<uint32_t>(gl::ImageIndex::kEntireLevel))
        {
            *layerCountOut = imageLayerCount;
        }
    }
    else
    {
        const VkImageSubresourceLayers &dstSubresource =
            updateSource == UpdateSource::Buffer ? data.buffer.copyRegion.imageSubresource
                                                 : data.image.copyRegion.dstSubresource;
        *baseLayerOut  = dstSubresource.baseArrayLayer;
        *layerCountOut = dstSubresource.layerCount;

        ASSERT(*layerCountOut != static_cast<uint32_t>(gl::ImageIndex::kEntireLevel));
    }
}

VkImageAspectFlags ImageHelper::SubresourceUpdate::getDestAspectFlags() const
{
    if (IsClear(updateSource))
    {
        return data.clear.aspectFlags;
    }
    else if (updateSource == UpdateSource::ClearPartial)
    {
        return data.clearPartial.aspectFlags;
    }
    else if (updateSource == UpdateSource::Buffer)
    {
        return data.buffer.copyRegion.imageSubresource.aspectMask;
    }
    else
    {
        ASSERT(updateSource == UpdateSource::Image);
        return data.image.copyRegion.dstSubresource.aspectMask;
    }
}

size_t ImageHelper::getLevelUpdateCount(gl::LevelIndex level) const
{
    return static_cast<size_t>(level.get()) < mSubresourceUpdates.size()
               ? mSubresourceUpdates[level.get()].size()
               : 0;
}

void ImageHelper::clipLevelToUpdateListUpperLimit(gl::LevelIndex *level) const
{
    gl::LevelIndex levelLimit(static_cast<int>(mSubresourceUpdates.size()));
    *level = std::min(*level, levelLimit);
}

std::vector<ImageHelper::SubresourceUpdate> *ImageHelper::getLevelUpdates(gl::LevelIndex level)
{
    return static_cast<size_t>(level.get()) < mSubresourceUpdates.size()
               ? &mSubresourceUpdates[level.get()]
               : nullptr;
}

const std::vector<ImageHelper::SubresourceUpdate> *ImageHelper::getLevelUpdates(
    gl::LevelIndex level) const
{
    return static_cast<size_t>(level.get()) < mSubresourceUpdates.size()
               ? &mSubresourceUpdates[level.get()]
               : nullptr;
}

void ImageHelper::appendSubresourceUpdate(gl::LevelIndex level, SubresourceUpdate &&update)
{
    if (mSubresourceUpdates.size() <= static_cast<size_t>(level.get()))
    {
        mSubresourceUpdates.resize(level.get() + 1);
    }
    // Update total staging buffer size
    mTotalStagedBufferUpdateSize += update.updateSource == UpdateSource::Buffer
                                        ? update.data.buffer.bufferHelper->getSize()
                                        : 0;
    mSubresourceUpdates[level.get()].emplace_back(std::move(update));
    onStateChange(angle::SubjectMessage::SubjectChanged);
}

void ImageHelper::prependSubresourceUpdate(gl::LevelIndex level, SubresourceUpdate &&update)
{
    if (mSubresourceUpdates.size() <= static_cast<size_t>(level.get()))
    {
        mSubresourceUpdates.resize(level.get() + 1);
    }

    // Update total staging buffer size
    mTotalStagedBufferUpdateSize += update.updateSource == UpdateSource::Buffer
                                        ? update.data.buffer.bufferHelper->getSize()
                                        : 0;
    mSubresourceUpdates[level.get()].insert(mSubresourceUpdates[level.get()].begin(),
                                            std::move(update));
    onStateChange(angle::SubjectMessage::SubjectChanged);
}

bool ImageHelper::hasEmulatedImageChannels() const
{
    const angle::Format &angleFmt   = getIntendedFormat();
    const angle::Format &textureFmt = getActualFormat();

    // Block formats may be decoded and emulated with a non-block format.
    if (angleFmt.isBlock)
    {
        return !textureFmt.isBlock;
    }

    // The red channel is never emulated.
    ASSERT((angleFmt.redBits != 0 || angleFmt.luminanceBits != 0 || angleFmt.alphaBits != 0) ==
           (textureFmt.redBits != 0));

    return (angleFmt.alphaBits == 0 && textureFmt.alphaBits > 0) ||
           (angleFmt.blueBits == 0 && textureFmt.blueBits > 0) ||
           (angleFmt.greenBits == 0 && textureFmt.greenBits > 0) ||
           (angleFmt.depthBits == 0 && textureFmt.depthBits > 0) ||
           (angleFmt.stencilBits == 0 && textureFmt.stencilBits > 0);
}

bool ImageHelper::hasEmulatedDepthChannel() const
{
    return getIntendedFormat().depthBits == 0 && getActualFormat().depthBits > 0;
}

bool ImageHelper::hasEmulatedStencilChannel() const
{
    return getIntendedFormat().stencilBits == 0 && getActualFormat().stencilBits > 0;
}

bool ImageHelper::hasInefficientlyEmulatedImageFormat() const
{
    if (hasEmulatedImageFormat())
    {
        // ETC2 compression is compatible with ETC1
        return !(mIntendedFormatID == angle::FormatID::ETC1_R8G8B8_UNORM_BLOCK &&
                 mActualFormatID == angle::FormatID::ETC2_R8G8B8_UNORM_BLOCK);
    }
    return false;
}

VkColorComponentFlags ImageHelper::getEmulatedChannelsMask() const
{
    const angle::Format &angleFmt   = getIntendedFormat();
    const angle::Format &textureFmt = getActualFormat();

    ASSERT(!angleFmt.hasDepthOrStencilBits());

    VkColorComponentFlags emulatedChannelsMask = 0;

    if (angleFmt.alphaBits == 0 && textureFmt.alphaBits > 0)
    {
        emulatedChannelsMask |= VK_COLOR_COMPONENT_A_BIT;
    }
    if (angleFmt.blueBits == 0 && textureFmt.blueBits > 0)
    {
        emulatedChannelsMask |= VK_COLOR_COMPONENT_B_BIT;
    }
    if (angleFmt.greenBits == 0 && textureFmt.greenBits > 0)
    {
        emulatedChannelsMask |= VK_COLOR_COMPONENT_G_BIT;
    }

    // The red channel is never emulated.
    ASSERT((angleFmt.redBits != 0 || angleFmt.luminanceBits != 0 || angleFmt.alphaBits != 0) ==
           (textureFmt.redBits != 0));

    return emulatedChannelsMask;
}

bool ImageHelper::getCompressionFixedRate(VkImageCompressionControlEXT *compressionInfo,
                                          VkImageCompressionFixedRateFlagsEXT *compressionRates,
                                          GLenum glCompressionRate) const
{
    bool rtn = true;
    ASSERT(compressionInfo->sType == VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_CONTROL_EXT);
    compressionInfo->compressionControlPlaneCount = 1;

    if (glCompressionRate == GL_SURFACE_COMPRESSION_FIXED_RATE_NONE_EXT)
    {
        compressionInfo->flags = VK_IMAGE_COMPRESSION_DISABLED_EXT;
    }
    else if (glCompressionRate == GL_SURFACE_COMPRESSION_FIXED_RATE_DEFAULT_EXT)
    {
        compressionInfo->flags = VK_IMAGE_COMPRESSION_FIXED_RATE_DEFAULT_EXT;
    }
    else if (glCompressionRate >= GL_SURFACE_COMPRESSION_FIXED_RATE_1BPC_EXT &&
             glCompressionRate <= GL_SURFACE_COMPRESSION_FIXED_RATE_12BPC_EXT)
    {
        int offset             = glCompressionRate - GL_SURFACE_COMPRESSION_FIXED_RATE_1BPC_EXT;
        compressionInfo->flags = VK_IMAGE_COMPRESSION_FIXED_RATE_EXPLICIT_EXT;
        *compressionRates      = 1u << offset;
        compressionInfo->pFixedRateFlags = compressionRates;
    }
    else
    {
        // Invalid value
        rtn = false;
    }

    return rtn;
}

LayerMode GetLayerMode(const vk::ImageHelper &image, uint32_t layerCount)
{
    const uint32_t imageLayerCount = GetImageLayerCountForView(image);
    const bool allLayers           = layerCount == imageLayerCount;

    ASSERT(allLayers || (layerCount > 0 && layerCount <= gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS));
    return allLayers ? LayerMode::All : static_cast<LayerMode>(layerCount);
}

ComputePipelineOptions GetComputePipelineOptions(vk::PipelineRobustness robustness,
                                                 vk::PipelineProtectedAccess protectedAccess)
{
    vk::ComputePipelineOptions pipelineOptions = {};

    if (robustness == vk::PipelineRobustness::Robust)
    {
        pipelineOptions.robustness = 1;
    }
    if (protectedAccess == vk::PipelineProtectedAccess::Protected)
    {
        pipelineOptions.protectedAccess = 1;
    }

    return pipelineOptions;
}

// ImageViewHelper implementation.
ImageViewHelper::ImageViewHelper()
    : mCurrentBaseMaxLevelHash(0),
      mIsCopyImageViewShared(false),
      mReadColorspace(ImageViewColorspace::Invalid),
      mWriteColorspace(ImageViewColorspace::Invalid)
{}

ImageViewHelper::ImageViewHelper(ImageViewHelper &&other)
{
    std::swap(mCurrentBaseMaxLevelHash, other.mCurrentBaseMaxLevelHash);
    std::swap(mReadColorspace, other.mReadColorspace);
    std::swap(mWriteColorspace, other.mWriteColorspace);
    std::swap(mColorspaceState, other.mColorspaceState);

    std::swap(mPerLevelRangeLinearReadImageViews, other.mPerLevelRangeLinearReadImageViews);
    std::swap(mPerLevelRangeSRGBReadImageViews, other.mPerLevelRangeSRGBReadImageViews);
    std::swap(mPerLevelRangeLinearCopyImageViews, other.mPerLevelRangeLinearCopyImageViews);
    std::swap(mPerLevelRangeSRGBCopyImageViews, other.mPerLevelRangeSRGBCopyImageViews);
    std::swap(mIsCopyImageViewShared, other.mIsCopyImageViewShared);
    std::swap(mPerLevelRangeStencilReadImageViews, other.mPerLevelRangeStencilReadImageViews);
    std::swap(mPerLevelRangeSamplerExternal2DY2YEXTImageViews,
              other.mPerLevelRangeSamplerExternal2DY2YEXTImageViews);

    std::swap(mLayerLevelDrawImageViews, other.mLayerLevelDrawImageViews);
    std::swap(mLayerLevelDrawImageViewsLinear, other.mLayerLevelDrawImageViewsLinear);
    std::swap(mSubresourceDrawImageViews, other.mSubresourceDrawImageViews);

    std::swap(mLayerLevelDepthOnlyImageViews, other.mLayerLevelDepthOnlyImageViews);
    std::swap(mLayerLevelStencilOnlyImageViews, other.mLayerLevelStencilOnlyImageViews);
    std::swap(mSubresourceDepthOnlyImageViews, other.mSubresourceDepthOnlyImageViews);
    std::swap(mSubresourceStencilOnlyImageViews, other.mSubresourceStencilOnlyImageViews);

    std::swap(mLevelStorageImageViews, other.mLevelStorageImageViews);
    std::swap(mLayerLevelStorageImageViews, other.mLayerLevelStorageImageViews);
    std::swap(mFragmentShadingRateImageView, other.mFragmentShadingRateImageView);
    std::swap(mImageViewSerial, other.mImageViewSerial);
}

ImageViewHelper::~ImageViewHelper() {}

void ImageViewHelper::init(Renderer *renderer)
{
    if (!mImageViewSerial.valid())
    {
        mImageViewSerial = renderer->getResourceSerialFactory().generateImageOrBufferViewSerial();
    }
}

void ImageViewHelper::release(Renderer *renderer, const ResourceUse &use)
{
    mCurrentBaseMaxLevelHash = 0;
    mReadColorspace          = ImageViewColorspace::Invalid;
    mWriteColorspace         = ImageViewColorspace::Invalid;
    // Clear shared flag
    mIsCopyImageViewShared = false;
    mColorspaceState.reset();

    std::vector<vk::GarbageObject> garbage;
    // Release the read views
    ReleaseImageViews(&mPerLevelRangeLinearReadImageViews, &garbage);
    ReleaseImageViews(&mPerLevelRangeSRGBReadImageViews, &garbage);
    ReleaseImageViews(&mPerLevelRangeLinearCopyImageViews, &garbage);
    ReleaseImageViews(&mPerLevelRangeSRGBCopyImageViews, &garbage);
    ReleaseImageViews(&mPerLevelRangeStencilReadImageViews, &garbage);
    ReleaseImageViews(&mPerLevelRangeSamplerExternal2DY2YEXTImageViews, &garbage);

    // Release the draw views
    ReleaseLayerLevelImageViews(&mLayerLevelDrawImageViews, &garbage);
    ReleaseLayerLevelImageViews(&mLayerLevelDrawImageViewsLinear, &garbage);
    ReleaseSubresourceImageViews(&mSubresourceDrawImageViews, &garbage);

    // Release the depth-xor-stencil input views
    ReleaseLayerLevelImageViews(&mLayerLevelDepthOnlyImageViews, &garbage);
    ReleaseLayerLevelImageViews(&mLayerLevelStencilOnlyImageViews, &garbage);
    ReleaseSubresourceImageViews(&mSubresourceDepthOnlyImageViews, &garbage);
    ReleaseSubresourceImageViews(&mSubresourceStencilOnlyImageViews, &garbage);

    // Release the storage views
    ReleaseImageViews(&mLevelStorageImageViews, &garbage);
    ReleaseLayerLevelImageViews(&mLayerLevelStorageImageViews, &garbage);

    // Release fragment shading rate view
    if (mFragmentShadingRateImageView.valid())
    {
        garbage.emplace_back(GetGarbage(&mFragmentShadingRateImageView));
    }

    if (!garbage.empty())
    {
        renderer->collectGarbage(use, std::move(garbage));
    }

    // Update image view serial.
    mImageViewSerial = renderer->getResourceSerialFactory().generateImageOrBufferViewSerial();
}

bool ImageViewHelper::isImageViewGarbageEmpty() const
{
    return mPerLevelRangeLinearReadImageViews.empty() &&
           mPerLevelRangeLinearCopyImageViews.empty() && mPerLevelRangeSRGBReadImageViews.empty() &&
           mPerLevelRangeSRGBCopyImageViews.empty() &&
           mPerLevelRangeStencilReadImageViews.empty() &&
           mPerLevelRangeSamplerExternal2DY2YEXTImageViews.empty() &&
           mLayerLevelDrawImageViews.empty() && mLayerLevelDrawImageViewsLinear.empty() &&
           mSubresourceDrawImageViews.empty() && mLayerLevelDepthOnlyImageViews.empty() &&
           mLayerLevelStencilOnlyImageViews.empty() && mSubresourceDepthOnlyImageViews.empty() &&
           mSubresourceStencilOnlyImageViews.empty() && mLayerLevelStorageImageViews.empty();
}

void ImageViewHelper::destroy(VkDevice device)
{
    mCurrentBaseMaxLevelHash = 0;
    mReadColorspace          = ImageViewColorspace::Invalid;
    mWriteColorspace         = ImageViewColorspace::Invalid;
    mColorspaceState.reset();

    // Release the read views
    DestroyImageViews(&mPerLevelRangeLinearReadImageViews, device);
    DestroyImageViews(&mPerLevelRangeSRGBReadImageViews, device);
    DestroyImageViews(&mPerLevelRangeLinearCopyImageViews, device);
    DestroyImageViews(&mPerLevelRangeSRGBCopyImageViews, device);
    DestroyImageViews(&mPerLevelRangeStencilReadImageViews, device);
    DestroyImageViews(&mPerLevelRangeSamplerExternal2DY2YEXTImageViews, device);

    // Release the draw views
    DestroyLayerLevelImageViews(&mLayerLevelDrawImageViews, device);
    DestroyLayerLevelImageViews(&mLayerLevelDrawImageViewsLinear, device);
    DestroySubresourceImageViews(&mSubresourceDrawImageViews, device);

    // Release the depth-xor-stencil input views
    DestroyLayerLevelImageViews(&mLayerLevelDepthOnlyImageViews, device);
    DestroyLayerLevelImageViews(&mLayerLevelStencilOnlyImageViews, device);
    DestroySubresourceImageViews(&mSubresourceDepthOnlyImageViews, device);
    DestroySubresourceImageViews(&mSubresourceStencilOnlyImageViews, device);

    // Release the storage views
    DestroyImageViews(&mLevelStorageImageViews, device);
    DestroyLayerLevelImageViews(&mLayerLevelStorageImageViews, device);

    // Destroy fragment shading rate view
    mFragmentShadingRateImageView.destroy(device);

    mImageViewSerial = kInvalidImageOrBufferViewSerial;
}

angle::Result ImageViewHelper::initReadViews(ContextVk *contextVk,
                                             gl::TextureType viewType,
                                             const ImageHelper &image,
                                             const gl::SwizzleState &formatSwizzle,
                                             const gl::SwizzleState &readSwizzle,
                                             LevelIndex baseLevel,
                                             uint32_t levelCount,
                                             uint32_t baseLayer,
                                             uint32_t layerCount,
                                             bool requiresSRGBViews,
                                             VkImageUsageFlags imageUsageFlags)
{
    ASSERT(levelCount > 0);

    const uint32_t maxLevel = levelCount - 1;
    ASSERT(maxLevel < 16);
    ASSERT(baseLevel.get() < 16);
    mCurrentBaseMaxLevelHash = static_cast<uint8_t>(baseLevel.get() << 4 | maxLevel);
    updateColorspace(image);

    if (mCurrentBaseMaxLevelHash >= mPerLevelRangeLinearReadImageViews.size())
    {
        const uint32_t maxViewCount = mCurrentBaseMaxLevelHash + 1;

        mPerLevelRangeLinearReadImageViews.resize(maxViewCount);
        mPerLevelRangeSRGBReadImageViews.resize(maxViewCount);
        mPerLevelRangeLinearCopyImageViews.resize(maxViewCount);
        mPerLevelRangeSRGBCopyImageViews.resize(maxViewCount);
        mPerLevelRangeStencilReadImageViews.resize(maxViewCount);
        mPerLevelRangeSamplerExternal2DY2YEXTImageViews.resize(maxViewCount);
    }

    // Determine if we already have ImageViews for the new max level
    if (getReadImageView().valid())
    {
        return angle::Result::Continue;
    }

    // Since we don't have a readImageView, we must create ImageViews for the new max level
    if (requiresSRGBViews)
    {
        // Initialize image views for both linear and srgb colorspaces
        ANGLE_TRY(initLinearAndSrgbReadViewsImpl(contextVk, viewType, image, formatSwizzle,
                                                 readSwizzle, baseLevel, levelCount, baseLayer,
                                                 layerCount, imageUsageFlags));
    }
    else
    {
        // Initialize image view for image's format's colorspace
        ANGLE_TRY(initReadViewsImpl(contextVk, viewType, image, formatSwizzle, readSwizzle,
                                    baseLevel, levelCount, baseLayer, layerCount, imageUsageFlags));
    }

    return angle::Result::Continue;
}

angle::Result ImageViewHelper::initReadViewsImpl(ContextVk *contextVk,
                                                 gl::TextureType viewType,
                                                 const ImageHelper &image,
                                                 const gl::SwizzleState &formatSwizzle,
                                                 const gl::SwizzleState &readSwizzle,
                                                 LevelIndex baseLevel,
                                                 uint32_t levelCount,
                                                 uint32_t baseLayer,
                                                 uint32_t layerCount,
                                                 VkImageUsageFlags imageUsageFlags)
{
    ASSERT(mImageViewSerial.valid());
    ASSERT(mReadColorspace != ImageViewColorspace::Invalid);

    const VkImageAspectFlags aspectFlags = GetFormatAspectFlags(image.getIntendedFormat());

    if (HasBothDepthAndStencilAspects(aspectFlags))
    {
        ANGLE_TRY(image.initLayerImageViewWithUsage(
            contextVk, viewType, VK_IMAGE_ASPECT_DEPTH_BIT, readSwizzle, &getReadImageView(),
            baseLevel, levelCount, baseLayer, layerCount, imageUsageFlags));
        ANGLE_TRY(image.initLayerImageViewWithUsage(
            contextVk, viewType, VK_IMAGE_ASPECT_STENCIL_BIT, readSwizzle,
            &mPerLevelRangeStencilReadImageViews[mCurrentBaseMaxLevelHash], baseLevel, levelCount,
            baseLayer, layerCount, imageUsageFlags));
    }
    else
    {
        ANGLE_TRY(image.initLayerImageViewWithUsage(contextVk, viewType, aspectFlags, readSwizzle,
                                                    &getReadImageView(), baseLevel, levelCount,
                                                    baseLayer, layerCount, imageUsageFlags));

        if (image.getActualFormat().isYUV)
        {
            ANGLE_TRY(image.initLayerImageViewWithYuvModeOverride(
                contextVk, viewType, aspectFlags, readSwizzle,
                &getSamplerExternal2DY2YEXTImageView(), baseLevel, levelCount, baseLayer,
                layerCount, gl::YuvSamplingMode::Y2Y, imageUsageFlags));
        }
    }

    gl::TextureType fetchType = viewType;
    if (viewType == gl::TextureType::CubeMap || viewType == gl::TextureType::_2DArray ||
        viewType == gl::TextureType::_2DMultisampleArray)
    {
        fetchType = Get2DTextureType(layerCount, image.getSamples());
    }

    if (!image.getActualFormat().isBlock)
    {
        if (fetchType != viewType || readSwizzle != formatSwizzle ||
            HasBothDepthAndStencilAspects(aspectFlags))
        {
            ANGLE_TRY(image.initLayerImageViewWithUsage(
                contextVk, fetchType, aspectFlags, formatSwizzle, &getCopyImageViewStorage(),
                baseLevel, levelCount, baseLayer, layerCount, imageUsageFlags));
        }
        else
        {
            mIsCopyImageViewShared = true;
        }
    }
    return angle::Result::Continue;
}

angle::Result ImageViewHelper::initLinearAndSrgbReadViewsImpl(ContextVk *contextVk,
                                                              gl::TextureType viewType,
                                                              const ImageHelper &image,
                                                              const gl::SwizzleState &formatSwizzle,
                                                              const gl::SwizzleState &readSwizzle,
                                                              LevelIndex baseLevel,
                                                              uint32_t levelCount,
                                                              uint32_t baseLayer,
                                                              uint32_t layerCount,
                                                              VkImageUsageFlags imageUsageFlags)
{
    ASSERT(mReadColorspace != ImageViewColorspace::Invalid);

    // When we select the linear/srgb counterpart formats, we must first make sure they're
    // actually supported by the ICD. If they are not supported by the ICD, then we treat that as if
    // there is no counterpart format.
    const bool imageFormatIsSrgb      = image.getActualFormat().isSRGB;
    const angle::FormatID imageFormat = image.getActualFormatID();
    angle::FormatID srgbFormat = imageFormatIsSrgb ? imageFormat : ConvertToSRGB(imageFormat);
    if (srgbFormat != angle::FormatID::NONE &&
        !HasNonRenderableTextureFormatSupport(contextVk->getRenderer(), srgbFormat))
    {
        srgbFormat = angle::FormatID::NONE;
    }

    angle::FormatID linearFormat = !imageFormatIsSrgb ? imageFormat : ConvertToLinear(imageFormat);
    ASSERT(linearFormat != angle::FormatID::NONE);

    const VkImageAspectFlags aspectFlags = GetFormatAspectFlags(image.getIntendedFormat());

    if (HasBothDepthAndStencilAspects(aspectFlags))
    {
        ANGLE_TRY(image.initReinterpretedLayerImageView(
            contextVk, viewType, VK_IMAGE_ASPECT_DEPTH_BIT, readSwizzle,
            &mPerLevelRangeLinearReadImageViews[mCurrentBaseMaxLevelHash], baseLevel, levelCount,
            baseLayer, layerCount, imageUsageFlags, linearFormat));

        ANGLE_TRY(image.initReinterpretedLayerImageView(
            contextVk, viewType, VK_IMAGE_ASPECT_STENCIL_BIT, readSwizzle,
            &mPerLevelRangeStencilReadImageViews[mCurrentBaseMaxLevelHash], baseLevel, levelCount,
            baseLayer, layerCount, imageUsageFlags, linearFormat));
    }
    else
    {
        if (!mPerLevelRangeLinearReadImageViews[mCurrentBaseMaxLevelHash].valid())
        {
            ANGLE_TRY(image.initReinterpretedLayerImageView(
                contextVk, viewType, aspectFlags, readSwizzle,
                &mPerLevelRangeLinearReadImageViews[mCurrentBaseMaxLevelHash], baseLevel,
                levelCount, baseLayer, layerCount, imageUsageFlags, linearFormat));
        }

        if (srgbFormat != angle::FormatID::NONE &&
            !mPerLevelRangeSRGBReadImageViews[mCurrentBaseMaxLevelHash].valid())
        {
            ANGLE_TRY(image.initReinterpretedLayerImageView(
                contextVk, viewType, aspectFlags, readSwizzle,
                &mPerLevelRangeSRGBReadImageViews[mCurrentBaseMaxLevelHash], baseLevel, levelCount,
                baseLayer, layerCount, imageUsageFlags, srgbFormat));
        }

        if (image.getActualFormat().isYUV)
        {
            ANGLE_TRY(image.initLayerImageViewWithYuvModeOverride(
                contextVk, viewType, aspectFlags, readSwizzle,
                &getSamplerExternal2DY2YEXTImageView(), baseLevel, levelCount, baseLayer,
                layerCount, gl::YuvSamplingMode::Y2Y, imageUsageFlags));
        }
    }

    gl::TextureType fetchType = viewType;

    if (viewType == gl::TextureType::CubeMap || viewType == gl::TextureType::_2DArray ||
        viewType == gl::TextureType::_2DMultisampleArray)
    {
        fetchType = Get2DTextureType(layerCount, image.getSamples());
    }

    if (!image.getActualFormat().isBlock)
    {
        if (fetchType != viewType || formatSwizzle != readSwizzle ||
            HasBothDepthAndStencilAspects(aspectFlags))
        {
            if (!mPerLevelRangeLinearCopyImageViews[mCurrentBaseMaxLevelHash].valid())
            {
                ANGLE_TRY(image.initReinterpretedLayerImageView(
                    contextVk, fetchType, aspectFlags, formatSwizzle,
                    &mPerLevelRangeLinearCopyImageViews[mCurrentBaseMaxLevelHash], baseLevel,
                    levelCount, baseLayer, layerCount, imageUsageFlags, linearFormat));
            }
            if (srgbFormat != angle::FormatID::NONE &&
                !mPerLevelRangeSRGBCopyImageViews[mCurrentBaseMaxLevelHash].valid())
            {
                ANGLE_TRY(image.initReinterpretedLayerImageView(
                    contextVk, fetchType, aspectFlags, formatSwizzle,
                    &mPerLevelRangeSRGBCopyImageViews[mCurrentBaseMaxLevelHash], baseLevel,
                    levelCount, baseLayer, layerCount, imageUsageFlags, srgbFormat));
            }
        }
        else
        {
            mIsCopyImageViewShared = true;
        }
    }

    return angle::Result::Continue;
}

angle::Result ImageViewHelper::getLevelStorageImageView(ErrorContext *context,
                                                        gl::TextureType viewType,
                                                        const ImageHelper &image,
                                                        LevelIndex levelVk,
                                                        uint32_t layer,
                                                        VkImageUsageFlags imageUsageFlags,
                                                        angle::FormatID formatID,
                                                        const ImageView **imageViewOut)
{
    ASSERT(mImageViewSerial.valid());

    ImageView *imageView =
        GetLevelImageView(&mLevelStorageImageViews, levelVk, image.getLevelCount());

    *imageViewOut = imageView;
    if (imageView->valid())
    {
        return angle::Result::Continue;
    }

    // Create the view.  Note that storage images are not affected by swizzle parameters.
    return image.initReinterpretedLayerImageView(context, viewType, image.getAspectFlags(),
                                                 gl::SwizzleState(), imageView, levelVk, 1, layer,
                                                 image.getLayerCount(), imageUsageFlags, formatID);
}

angle::Result ImageViewHelper::getLevelLayerStorageImageView(ErrorContext *contextVk,
                                                             const ImageHelper &image,
                                                             LevelIndex levelVk,
                                                             uint32_t layer,
                                                             VkImageUsageFlags imageUsageFlags,
                                                             angle::FormatID formatID,
                                                             const ImageView **imageViewOut)
{
    ASSERT(image.valid());
    ASSERT(mImageViewSerial.valid());
    ASSERT(!image.getActualFormat().isBlock);

    ImageView *imageView =
        GetLevelLayerImageView(&mLayerLevelStorageImageViews, levelVk, layer, image.getLevelCount(),
                               GetImageLayerCountForView(image));
    *imageViewOut = imageView;

    if (imageView->valid())
    {
        return angle::Result::Continue;
    }

    // Create the view.  Note that storage images are not affected by swizzle parameters.
    gl::TextureType viewType = Get2DTextureType(1, image.getSamples());
    return image.initReinterpretedLayerImageView(contextVk, viewType, image.getAspectFlags(),
                                                 gl::SwizzleState(), imageView, levelVk, 1, layer,
                                                 1, imageUsageFlags, formatID);
}

angle::Result ImageViewHelper::getLevelLayerDrawImageViewImpl(ErrorContext *context,
                                                              const ImageHelper &image,
                                                              LevelIndex levelVk,
                                                              uint32_t layer,
                                                              uint32_t layerCount,
                                                              ImageView *imageViewOut)
{
    ASSERT(imageViewOut != nullptr);

    // If we are initializing an imageview for use with EXT_srgb_write_control, we need to override
    // the format to its linear counterpart. Formats that cannot be reinterpreted are exempt from
    // this requirement.
    angle::FormatID actualFormat = image.getActualFormatID();
    angle::FormatID linearFormat = ConvertToLinear(actualFormat);
    angle::FormatID sRGBFormat   = ConvertToSRGB(actualFormat);
    if (mWriteColorspace == ImageViewColorspace::Linear && linearFormat != angle::FormatID::NONE)
    {
        actualFormat = linearFormat;
    }
    else if (mWriteColorspace == ImageViewColorspace::SRGB && sRGBFormat != angle::FormatID::NONE)
    {
        actualFormat = sRGBFormat;
    }

    // Note that these views are specifically made to be used as framebuffer attachments, and
    // therefore don't have swizzle.
    return image.initReinterpretedLayerImageView(
        context, Get2DTextureType(layerCount, image.getSamples()), image.getAspectFlags(),
        gl::SwizzleState(), imageViewOut, levelVk, 1, layer, layerCount,
        vk::ImageHelper::kDefaultImageViewUsageFlags, actualFormat);
}

angle::Result ImageViewHelper::getLevelDrawImageView(ErrorContext *context,
                                                     const ImageHelper &image,
                                                     LevelIndex levelVk,
                                                     uint32_t layer,
                                                     uint32_t layerCount,
                                                     const ImageView **imageViewOut)
{
    ASSERT(image.valid());
    ASSERT(mImageViewSerial.valid());
    ASSERT(!image.getActualFormat().isBlock);

    if (mWriteColorspace == ImageViewColorspace::Invalid)
    {
        updateColorspace(image);
    }
    ASSERT(mWriteColorspace != ImageViewColorspace::Invalid);

    ImageSubresourceRange range = MakeImageSubresourceDrawRange(image.toGLLevel(levelVk), layer,
                                                                GetLayerMode(image, layerCount),
                                                                mReadColorspace, mWriteColorspace);

    std::unique_ptr<ImageView> &view = mSubresourceDrawImageViews[range];
    if (view)
    {
        *imageViewOut = view.get();
        return angle::Result::Continue;
    }

    view          = std::make_unique<ImageView>();
    *imageViewOut = view.get();

    return getLevelLayerDrawImageViewImpl(context, image, levelVk, layer, layerCount, view.get());
}

angle::Result ImageViewHelper::getLevelLayerDrawImageView(ErrorContext *context,
                                                          const ImageHelper &image,
                                                          LevelIndex levelVk,
                                                          uint32_t layer,
                                                          const ImageView **imageViewOut)
{
    ASSERT(image.valid());
    ASSERT(mImageViewSerial.valid());
    ASSERT(!image.getActualFormat().isBlock);

    if (mWriteColorspace == ImageViewColorspace::Invalid)
    {
        updateColorspace(image);
    }
    ASSERT(mWriteColorspace != ImageViewColorspace::Invalid);

    LayerLevelImageViewVector &imageViews = (mWriteColorspace == ImageViewColorspace::Linear)
                                                ? mLayerLevelDrawImageViewsLinear
                                                : mLayerLevelDrawImageViews;

    // Lazily allocate the storage for image views
    ImageView *imageView = GetLevelLayerImageView(
        &imageViews, levelVk, layer, image.getLevelCount(), GetImageLayerCountForView(image));
    *imageViewOut = imageView;

    if (imageView->valid())
    {
        return angle::Result::Continue;
    }

    return getLevelLayerDrawImageViewImpl(context, image, levelVk, layer, 1, imageView);
}

angle::Result ImageViewHelper::getLevelDepthOrStencilImageView(ErrorContext *context,
                                                               const ImageHelper &image,
                                                               LevelIndex levelVk,
                                                               uint32_t layer,
                                                               uint32_t layerCount,
                                                               VkImageAspectFlagBits aspect,
                                                               const ImageView **imageViewOut)
{
    ASSERT(image.valid());
    ASSERT(mImageViewSerial.valid());
    ASSERT((image.getAspectFlags() & aspect) != 0);

    ImageSubresourceRange range = MakeImageSubresourceDrawRange(
        image.toGLLevel(levelVk), layer, GetLayerMode(image, layerCount),
        ImageViewColorspace::Linear, ImageViewColorspace::Linear);

    SubresourceImageViewMap &imageViews = aspect == VK_IMAGE_ASPECT_DEPTH_BIT
                                              ? mSubresourceDepthOnlyImageViews
                                              : mSubresourceStencilOnlyImageViews;

    std::unique_ptr<ImageView> &view = imageViews[range];
    if (view)
    {
        *imageViewOut = view.get();
        return angle::Result::Continue;
    }

    view          = std::make_unique<ImageView>();
    *imageViewOut = view.get();

    return getLevelLayerDepthOrStencilImageViewImpl(context, image, levelVk, layer, layerCount,
                                                    aspect, view.get());
}

angle::Result ImageViewHelper::getLevelLayerDepthOrStencilImageView(ErrorContext *context,
                                                                    const ImageHelper &image,
                                                                    LevelIndex levelVk,
                                                                    uint32_t layer,
                                                                    VkImageAspectFlagBits aspect,
                                                                    const ImageView **imageViewOut)
{
    ASSERT(image.valid());
    ASSERT(mImageViewSerial.valid());
    ASSERT((image.getAspectFlags() & aspect) != 0);

    LayerLevelImageViewVector &imageViews = aspect == VK_IMAGE_ASPECT_DEPTH_BIT
                                                ? mLayerLevelDepthOnlyImageViews
                                                : mLayerLevelStencilOnlyImageViews;

    // Lazily allocate the storage for image views
    ImageView *imageView = GetLevelLayerImageView(
        &imageViews, levelVk, layer, image.getLevelCount(), GetImageLayerCountForView(image));
    *imageViewOut = imageView;

    if (imageView->valid())
    {
        return angle::Result::Continue;
    }

    return getLevelLayerDepthOrStencilImageViewImpl(context, image, levelVk, layer, 1, aspect,
                                                    imageView);
}

angle::Result ImageViewHelper::getLevelLayerDepthOrStencilImageViewImpl(
    ErrorContext *context,
    const ImageHelper &image,
    LevelIndex levelVk,
    uint32_t layer,
    uint32_t layerCount,
    VkImageAspectFlagBits aspect,
    ImageView *imageViewOut)
{
    // Note that these views are specifically made to be used as input attachments, and
    // therefore don't have swizzle.
    return image.initReinterpretedLayerImageView(
        context, Get2DTextureType(layerCount, image.getSamples()), aspect, gl::SwizzleState(),
        imageViewOut, levelVk, 1, layer, layerCount, vk::ImageHelper::kDefaultImageViewUsageFlags,
        image.getActualFormatID());
}

angle::Result ImageViewHelper::initFragmentShadingRateView(ContextVk *contextVk, ImageHelper *image)
{
    ASSERT(image->valid());
    ASSERT(mImageViewSerial.valid());

    // Determine if we already have ImageView
    if (mFragmentShadingRateImageView.valid())
    {
        return angle::Result::Continue;
    }

    // Fragment shading rate image view always have -
    // - gl::TextureType    == gl::TextureType::_2D
    // - VkImageAspectFlags == VK_IMAGE_ASPECT_COLOR_BIT
    // - gl::SwizzleState   == gl::SwizzleState()
    // - baseMipLevelVk     == vk::LevelIndex(0)
    // - levelCount         == 1
    // - baseArrayLayer     == 0
    // - layerCount         == 1
    return image->initLayerImageViewWithUsage(
        contextVk, gl::TextureType::_2D, VK_IMAGE_ASPECT_COLOR_BIT, gl::SwizzleState(),
        &mFragmentShadingRateImageView, vk::LevelIndex(0), 1, 0, 1, image->getUsage());
}

angle::FormatID ImageViewHelper::getColorspaceOverrideFormatForWrite(angle::FormatID format) const
{
    ASSERT(mWriteColorspace != ImageViewColorspace::Invalid);

    angle::FormatID colorspaceOverrideFormat = format;
    angle::FormatID linearFormat             = ConvertToLinear(format);
    angle::FormatID sRGBFormat               = ConvertToSRGB(format);
    if (mWriteColorspace == ImageViewColorspace::Linear && linearFormat != angle::FormatID::NONE)
    {
        colorspaceOverrideFormat = linearFormat;
    }
    else if (mWriteColorspace == ImageViewColorspace::SRGB && sRGBFormat != angle::FormatID::NONE)
    {
        colorspaceOverrideFormat = sRGBFormat;
    }

    return colorspaceOverrideFormat;
}

void ImageViewHelper::updateColorspace(const ImageHelper &image) const
{
    const angle::Format &imageFormat        = image.getActualFormat();
    ImageViewColorspace imageViewColorspace = ImageViewColorspace::Invalid;
    mReadColorspace                         = ImageViewColorspace::Invalid;
    mWriteColorspace                        = ImageViewColorspace::Invalid;

    // Initialize colorspace based on image's format's colorspace
    imageViewColorspace =
        imageFormat.isSRGB ? ImageViewColorspace::SRGB : ImageViewColorspace::Linear;

    // Process EGL image colorspace override state
    if (!imageFormat.isSRGB && mColorspaceState.eglImageColorspace == egl::ImageColorspace::SRGB)
    {
        imageViewColorspace = ImageViewColorspace::SRGB;
    }
    else if (imageFormat.isSRGB &&
             mColorspaceState.eglImageColorspace == egl::ImageColorspace::Linear)
    {
        imageViewColorspace = ImageViewColorspace::Linear;
    }
    ASSERT(imageViewColorspace != ImageViewColorspace::Invalid);

    mReadColorspace  = imageViewColorspace;
    mWriteColorspace = imageViewColorspace;

    // Process srgb decode and srgb override state
    if (mReadColorspace == ImageViewColorspace::Linear)
    {
        if (mColorspaceState.srgbOverride == gl::SrgbOverride::SRGB &&
            rx::ConvertToSRGB(imageFormat.id) != angle::FormatID::NONE &&
            mColorspaceState.srgbDecode != gl::SrgbDecode::Skip)
        {
            mReadColorspace = ImageViewColorspace::SRGB;
        }
    }
    else
    {
        ASSERT(mReadColorspace == ImageViewColorspace::SRGB);

        if (mColorspaceState.srgbDecode == gl::SrgbDecode::Skip &&
            !mColorspaceState.hasStaticTexelFetchAccess)
        {
            mReadColorspace = ImageViewColorspace::Linear;
        }
    }

    // Process srgb write control state
    if (mWriteColorspace == ImageViewColorspace::SRGB &&
        mColorspaceState.srgbWriteControl == gl::SrgbWriteControlMode::Linear)
    {
        mWriteColorspace = ImageViewColorspace::Linear;
    }

    ASSERT(mReadColorspace != ImageViewColorspace::Invalid);
    ASSERT(mWriteColorspace != ImageViewColorspace::Invalid);
}

ImageOrBufferViewSubresourceSerial ImageViewHelper::getSubresourceSerial(gl::LevelIndex levelGL,
                                                                         uint32_t levelCount,
                                                                         uint32_t layer,
                                                                         LayerMode layerMode) const
{
    return getSubresourceSerialForColorspace(levelGL, levelCount, layer, layerMode,
                                             mReadColorspace);
}

ImageOrBufferViewSubresourceSerial ImageViewHelper::getSubresourceSerialForColorspace(
    gl::LevelIndex levelGL,
    uint32_t levelCount,
    uint32_t layer,
    LayerMode layerMode,
    ImageViewColorspace readColorspace) const
{
    ASSERT(mImageViewSerial.valid());

    ImageOrBufferViewSubresourceSerial serial;
    serial.viewSerial  = mImageViewSerial;
    serial.subresource = MakeImageSubresourceReadRange(levelGL, levelCount, layer, layerMode,
                                                       readColorspace, mWriteColorspace);
    return serial;
}

ImageSubresourceRange ImageViewHelper::getSubresourceDrawRange(gl::LevelIndex level,
                                                               uint32_t layer,
                                                               LayerMode layerMode) const
{
    return MakeImageSubresourceDrawRange(level, layer, layerMode, mReadColorspace,
                                         mWriteColorspace);
}

// BufferViewHelper implementation.
BufferViewHelper::BufferViewHelper() : mInitialized(false), mOffset(0), mSize(0) {}

BufferViewHelper::BufferViewHelper(BufferViewHelper &&other) : Resource(std::move(other))
{
    std::swap(mInitialized, other.mInitialized);
    std::swap(mOffset, other.mOffset);
    std::swap(mSize, other.mSize);
    std::swap(mViews, other.mViews);
    std::swap(mViewSerial, other.mViewSerial);
}

BufferViewHelper::~BufferViewHelper() {}

void BufferViewHelper::init(Renderer *renderer, VkDeviceSize offset, VkDeviceSize size)
{
    ASSERT(mViews.empty());

    mOffset = offset;
    mSize   = size;

    if (!mViewSerial.valid())
    {
        mViewSerial = renderer->getResourceSerialFactory().generateImageOrBufferViewSerial();
    }

    mInitialized = true;
}

void BufferViewHelper::release(Renderer *renderer)
{
    if (!mInitialized)
    {
        return;
    }

    GarbageObjects garbage;

    for (auto &formatAndView : mViews)
    {
        BufferView &view = formatAndView.second;
        ASSERT(view.valid());

        garbage.emplace_back(GetGarbage(&view));
    }

    if (!garbage.empty())
    {
        renderer->collectGarbage(mUse, std::move(garbage));
        // Update image view serial.
        mViewSerial = renderer->getResourceSerialFactory().generateImageOrBufferViewSerial();
    }

    mUse.reset();
    mViews.clear();
    mOffset      = 0;
    mSize        = 0;
    mInitialized = false;
}

void BufferViewHelper::release(ContextVk *contextVk)
{
    if (!mInitialized)
    {
        return;
    }

    contextVk->flushDescriptorSetUpdates();
    return release(contextVk->getRenderer());
}

void BufferViewHelper::destroy(VkDevice device)
{
    for (auto &formatAndView : mViews)
    {
        BufferView &view = formatAndView.second;
        view.destroy(device);
    }

    mViews.clear();

    mOffset = 0;
    mSize   = 0;

    mViewSerial = kInvalidImageOrBufferViewSerial;
}

angle::Result BufferViewHelper::getView(ErrorContext *context,
                                        const BufferHelper &buffer,
                                        VkDeviceSize bufferOffset,
                                        const Format &format,
                                        const BufferView **viewOut)
{
    ASSERT(format.valid());

    vk::Renderer *renderer = context->getRenderer();
    VkFormat viewVkFormat  = format.getActualBufferVkFormat(renderer, false);

    auto iter = mViews.find(viewVkFormat);
    if (iter != mViews.end())
    {
        *viewOut = &iter->second;
        return angle::Result::Continue;
    }

    // If the size is not a multiple of pixelBytes, remove the extra bytes.  The last element cannot
    // be read anyway, and this is a requirement of Vulkan (for size to be a multiple of format
    // texel block size).
    const angle::Format &bufferFormat = format.getActualBufferFormat(false);
    const GLuint pixelBytes           = bufferFormat.pixelBytes;
    VkDeviceSize size                 = mSize - mSize % pixelBytes;

    VkBufferViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType                  = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    viewCreateInfo.buffer                 = buffer.getBuffer().getHandle();
    viewCreateInfo.format                 = viewVkFormat;
    viewCreateInfo.offset                 = mOffset + bufferOffset;
    viewCreateInfo.range                  = size;

    BufferView view;
    ANGLE_VK_TRY(context, view.init(context->getDevice(), viewCreateInfo));

    // Cache the view
    auto insertIter = mViews.insert({viewVkFormat, std::move(view)});
    *viewOut        = &insertIter.first->second;
    ASSERT(insertIter.second);

    return angle::Result::Continue;
}

ImageOrBufferViewSubresourceSerial BufferViewHelper::getSerial() const
{
    ASSERT(mViewSerial.valid());

    ImageOrBufferViewSubresourceSerial serial = {};
    serial.viewSerial                         = mViewSerial;
    return serial;
}

// ShaderProgramHelper implementation.
ShaderProgramHelper::ShaderProgramHelper()  = default;
ShaderProgramHelper::~ShaderProgramHelper() = default;

bool ShaderProgramHelper::valid(const gl::ShaderType shaderType) const
{
    return mShaders[shaderType];
}

void ShaderProgramHelper::destroy(Renderer *renderer)
{
    for (ShaderModulePtr &shader : mShaders)
    {
        shader.reset();
    }
}

void ShaderProgramHelper::release(ContextVk *contextVk)
{
    for (ShaderModulePtr &shader : mShaders)
    {
        shader.reset();
    }
}

void ShaderProgramHelper::setShader(gl::ShaderType shaderType, const ShaderModulePtr &shader)
{
    // The shaders must be set once and are not expected to change.
    ASSERT(!mShaders[shaderType]);
    ASSERT(shader && shader->valid());
    mShaders[shaderType] = shader;
}

void ShaderProgramHelper::createMonolithicPipelineCreationTask(
    vk::ErrorContext *context,
    PipelineCacheAccess *pipelineCache,
    const GraphicsPipelineDesc &desc,
    const PipelineLayout &pipelineLayout,
    const SpecializationConstants &specConsts,
    PipelineHelper *pipeline) const
{
    std::shared_ptr<CreateMonolithicPipelineTask> monolithicPipelineCreationTask =
        std::make_shared<CreateMonolithicPipelineTask>(context->getRenderer(), *pipelineCache,
                                                       pipelineLayout, mShaders, specConsts, desc);

    pipeline->setMonolithicPipelineCreationTask(std::move(monolithicPipelineCreationTask));
}

angle::Result ShaderProgramHelper::getOrCreateComputePipeline(
    vk::ErrorContext *context,
    ComputePipelineCache *computePipelines,
    PipelineCacheAccess *pipelineCache,
    const PipelineLayout &pipelineLayout,
    ComputePipelineOptions pipelineOptions,
    PipelineSource source,
    PipelineHelper **pipelineOut,
    const char *shaderName,
    VkSpecializationInfo *specializationInfo) const
{
    return computePipelines->getOrCreatePipeline(context, pipelineCache, pipelineLayout,
                                                 pipelineOptions, source, pipelineOut, shaderName,
                                                 specializationInfo, mShaders);
}

// ActiveHandleCounter implementation.
ActiveHandleCounter::ActiveHandleCounter() : mActiveCounts{}, mAllocatedCounts{} {}

ActiveHandleCounter::~ActiveHandleCounter() = default;

// CommandBufferAccess implementation.
CommandBufferAccess::CommandBufferAccess()  = default;
CommandBufferAccess::~CommandBufferAccess() = default;

void CommandBufferAccess::onBufferRead(VkAccessFlags readAccessType,
                                       PipelineStage readStage,
                                       BufferHelper *buffer)
{
    ASSERT(!buffer->isReleasedToExternal());
    mReadBuffers.emplace_back(buffer, readAccessType, readStage);
}

void CommandBufferAccess::onBufferWrite(VkAccessFlags writeAccessType,
                                        PipelineStage writeStage,
                                        BufferHelper *buffer)
{
    ASSERT(!buffer->isReleasedToExternal());
    mWriteBuffers.emplace_back(buffer, writeAccessType, writeStage);
}

void CommandBufferAccess::onImageRead(VkImageAspectFlags aspectFlags,
                                      ImageLayout imageLayout,
                                      ImageHelper *image)
{
    ASSERT(!image->isReleasedToExternal());
    ASSERT(image->getImageSerial().valid());
    mReadImages.emplace_back(image, aspectFlags, imageLayout);
}

void CommandBufferAccess::onImageWrite(gl::LevelIndex levelStart,
                                       uint32_t levelCount,
                                       uint32_t layerStart,
                                       uint32_t layerCount,
                                       VkImageAspectFlags aspectFlags,
                                       ImageLayout imageLayout,
                                       ImageHelper *image)
{
    ASSERT(!image->isReleasedToExternal());
    ASSERT(image->getImageSerial().valid());
    mWriteImages.emplace_back(CommandBufferImageAccess{image, aspectFlags, imageLayout}, levelStart,
                              levelCount, layerStart, layerCount);
}

void CommandBufferAccess::onImageReadSubresources(gl::LevelIndex levelStart,
                                                  uint32_t levelCount,
                                                  uint32_t layerStart,
                                                  uint32_t layerCount,
                                                  VkImageAspectFlags aspectFlags,
                                                  ImageLayout imageLayout,
                                                  ImageHelper *image)
{
    ASSERT(!image->isReleasedToExternal());
    ASSERT(image->getImageSerial().valid());
    mReadImageSubresources.emplace_back(CommandBufferImageAccess{image, aspectFlags, imageLayout},
                                        levelStart, levelCount, layerStart, layerCount);
}

void CommandBufferAccess::onBufferExternalAcquireRelease(BufferHelper *buffer)
{
    mExternalAcquireReleaseBuffers.emplace_back(CommandBufferBufferExternalAcquireRelease{buffer});
}

void CommandBufferAccess::onResourceAccess(Resource *resource)
{
    mAccessResources.emplace_back(CommandBufferResourceAccess{resource});
}

// DescriptorMetaCache implementation.
MetaDescriptorPool::MetaDescriptorPool() = default;

MetaDescriptorPool::~MetaDescriptorPool()
{
    ASSERT(mPayload.empty());
}

void MetaDescriptorPool::destroy(Renderer *renderer)
{
    for (auto &iter : mPayload)
    {
        DynamicDescriptorPoolPointer &pool = iter.second;
        ASSERT(pool.unique());
    }
    mPayload.clear();
}

angle::Result MetaDescriptorPool::bindCachedDescriptorPool(
    ErrorContext *context,
    const DescriptorSetLayoutDesc &descriptorSetLayoutDesc,
    uint32_t descriptorCountMultiplier,
    DescriptorSetLayoutCache *descriptorSetLayoutCache,
    DynamicDescriptorPoolPointer *dynamicDescriptorPoolOut)
{
    if (descriptorSetLayoutDesc.empty())
    {
        // No need for descriptorSet pool.
        return angle::Result::Continue;
    }

    auto cacheIter = mPayload.find(descriptorSetLayoutDesc);
    if (cacheIter != mPayload.end())
    {
        *dynamicDescriptorPoolOut = cacheIter->second;
        return angle::Result::Continue;
    }

    DescriptorSetLayoutPtr descriptorSetLayout;
    ANGLE_TRY(descriptorSetLayoutCache->getDescriptorSetLayout(context, descriptorSetLayoutDesc,
                                                               &descriptorSetLayout));

    DynamicDescriptorPool newDescriptorPool;
    ANGLE_TRY(InitDynamicDescriptorPool(context, descriptorSetLayoutDesc, *descriptorSetLayout,
                                        descriptorCountMultiplier, &newDescriptorPool));

    ASSERT(newDescriptorPool.valid());
    DynamicDescriptorPoolPointer newDynamicDescriptorPoolPtr(context->getDevice(),
                                                             std::move(newDescriptorPool));
    mPayload.emplace(descriptorSetLayoutDesc, newDynamicDescriptorPoolPtr);
    *dynamicDescriptorPoolOut = std::move(newDynamicDescriptorPoolPtr);

    return angle::Result::Continue;
}

static_assert(static_cast<uint32_t>(PresentMode::ImmediateKHR) == VK_PRESENT_MODE_IMMEDIATE_KHR,
              "PresentMode must be updated");
static_assert(static_cast<uint32_t>(PresentMode::MailboxKHR) == VK_PRESENT_MODE_MAILBOX_KHR,
              "PresentMode must be updated");
static_assert(static_cast<uint32_t>(PresentMode::FifoKHR) == VK_PRESENT_MODE_FIFO_KHR,
              "PresentMode must be updated");
static_assert(static_cast<uint32_t>(PresentMode::FifoRelaxedKHR) ==
                  VK_PRESENT_MODE_FIFO_RELAXED_KHR,
              "PresentMode must be updated");
static_assert(static_cast<uint32_t>(PresentMode::SharedDemandRefreshKHR) ==
                  VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR,
              "PresentMode must be updated");
static_assert(static_cast<uint32_t>(PresentMode::SharedContinuousRefreshKHR) ==
                  VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR,
              "PresentMode must be updated");

VkPresentModeKHR ConvertPresentModeToVkPresentMode(PresentMode presentMode)
{
    return static_cast<VkPresentModeKHR>(presentMode);
}

PresentMode ConvertVkPresentModeToPresentMode(VkPresentModeKHR vkPresentMode)
{
    return static_cast<PresentMode>(vkPresentMode);
}

}  // namespace vk
}  // namespace rx
