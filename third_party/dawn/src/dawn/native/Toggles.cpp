// Copyright 2019 The Dawn & Tint Authors
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

#include <array>

#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "dawn/native/Toggles.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/native/stream/Stream.h"

namespace dawn::native {
namespace {

struct ToggleEnumAndInfo {
    Toggle toggle;
    ToggleInfo info;
};

using ToggleEnumAndInfoList = std::array<ToggleEnumAndInfo, static_cast<size_t>(Toggle::EnumCount)>;

static constexpr ToggleEnumAndInfoList kToggleNameAndInfoList = {{
    {Toggle::EmulateStoreAndMSAAResolve,
     {"emulate_store_and_msaa_resolve",
      "Emulate storing into multisampled color attachments and doing MSAA resolve simultaneously. "
      "This workaround is enabled by default on the Metal drivers that do not support "
      "MTLStoreActionStoreAndMultisampleResolve. To support StoreOp::Store on those platforms, we "
      "should do MSAA resolve in another render pass after ending the previous one.",
      "https://crbug.com/dawn/56", ToggleStage::Device}},
    {Toggle::NonzeroClearResourcesOnCreationForTesting,
     {"nonzero_clear_resources_on_creation_for_testing",
      "Clears texture to full 1 bits as soon as they are created, but doesn't update the tracking "
      "state of the texture. This way we can test the logic of clearing textures that use recycled "
      "memory.",
      "https://crbug.com/dawn/145", ToggleStage::Device}},
    {Toggle::AlwaysResolveIntoZeroLevelAndLayer,
     {"always_resolve_into_zero_level_and_layer",
      "When the resolve target is a texture view that is created on the non-zero level or layer of "
      "a texture, we first resolve into a temporarily 2D texture with only one mipmap level and "
      "one array layer, and copy the result of MSAA resolve into the true resolve target. This "
      "workaround is enabled by default on the Metal drivers that have bugs when setting non-zero "
      "resolveLevel or resolveSlice. It is also enabled by default on Qualcomm Vulkan drivers, "
      "which have similar bugs.",
      "https://crbug.com/dawn/56", ToggleStage::Device}},
    {Toggle::LazyClearResourceOnFirstUse,
     {"lazy_clear_resource_on_first_use",
      "Clears resource to zero on first usage. This initializes the resource so that no dirty bits "
      "from recycled memory is present in the new resource.",
      "https://crbug.com/dawn/145", ToggleStage::Device}},
    {Toggle::DisableLazyClearForMappedAtCreationBuffer,
     {"disable_lazy_clear_for_mapped_at_creation_buffer",
      "Disable clearing buffers to zero for buffers which are mapped at creation.",
      "https://crbug.com/dawn/145", ToggleStage::Device}},
    {Toggle::TurnOffVsync,
     {"turn_off_vsync",
      "Turn off vsync when rendering. In order to do performance test or run perf tests, turn off "
      "vsync so that the fps can exeed 60.",
      "https://crbug.com/dawn/237", ToggleStage::Device}},
    {Toggle::UseTemporaryBufferInCompressedTextureToTextureCopy,
     {"use_temporary_buffer_in_texture_to_texture_copy",
      "Split texture-to-texture copy into two copies: copy from source texture into a temporary "
      "buffer, and copy from the temporary buffer into the destination texture when copying "
      "between compressed textures that don't have block-aligned sizes. This workaround is enabled "
      "by default on all Vulkan drivers to solve an issue in the Vulkan SPEC about the "
      "texture-to-texture copies with compressed formats. See #1005 "
      "(https://github.com/KhronosGroup/Vulkan-Docs/issues/1005) for more details.",
      "https://crbug.com/dawn/42", ToggleStage::Device}},
    {Toggle::UseD3D12ResourceHeapTier2,
     {"use_d3d12_resource_heap_tier2",
      "Enable support for resource heap tier 2. Resource heap tier 2 allows mixing of texture and "
      "buffers in the same heap. This allows better heap re-use and reduces fragmentation.",
      "https://crbug.com/dawn/27", ToggleStage::Device}},
    {Toggle::UseD3D12RenderPass,
     {"use_d3d12_render_pass",
      "Use the D3D12 render pass API introduced in Windows build 1809 by default. On versions of "
      "Windows prior to build 1809, or when this toggle is turned off, Dawn will emulate a render "
      "pass.",
      "https://crbug.com/dawn/36", ToggleStage::Device}},
    {Toggle::UseD3D12ResidencyManagement,
     {"use_d3d12_residency_management",
      "Enable residency management. This allows page-in and page-out of resource heaps in GPU "
      "memory. This component improves overcommitted performance by keeping the most recently used "
      "resources local to the GPU. Turning this component off can cause allocation failures when "
      "application memory exceeds physical device memory.",
      "https://crbug.com/dawn/193", ToggleStage::Device}},
    {Toggle::DisableResourceSuballocation,
     {"disable_resource_suballocation",
      "Force the backends to not perform resource suballocation. This may expose allocation "
      "patterns which would otherwise only occur with large or specific types of resources.",
      "https://crbug.com/1313172", ToggleStage::Device}},
    {Toggle::SkipValidation,
     {"skip_validation", "Skip expensive validation of Dawn commands.",
      "https://crbug.com/dawn/271", ToggleStage::Device}},
    {Toggle::VulkanUseD32S8,
     {"vulkan_use_d32s8",
      "Vulkan mandates support of either D32_FLOAT_S8 or D24_UNORM_S8. When available the backend "
      "will use D32S8 (toggle to on) but setting the toggle to off will make it use the D24S8 "
      "format when possible.",
      "https://crbug.com/dawn/286", ToggleStage::Device}},
    {Toggle::VulkanUseS8,
     {"vulkan_use_s8",
      "Vulkan has a pure stencil8 format but it is not universally available. When this toggle is "
      "on, the backend will use S8 for the stencil8 format, otherwise it will fallback to D32S8 or "
      "D24S8.",
      "https://crbug.com/dawn/666", ToggleStage::Device}},
    {Toggle::MetalDisableSamplerCompare,
     {"metal_disable_sampler_compare",
      "Disables the use of sampler compare on Metal. This is unsupported before A9 processors.",
      "https://crbug.com/dawn/342", ToggleStage::Device}},
    {Toggle::MetalUseSharedModeForCounterSampleBuffer,
     {"metal_use_shared_mode_for_counter_sample_buffer",
      "The query set on Metal need to create MTLCounterSampleBuffer which storage mode must be "
      "either MTLStorageModeShared or MTLStorageModePrivate. But the private mode does not work "
      "properly on Intel platforms. The workaround is use shared mode instead.",
      "https://crbug.com/dawn/434", ToggleStage::Device}},
    {Toggle::DisableBaseVertex,
     {"disable_base_vertex",
      "Disables the use of non-zero base vertex which is unsupported on some platforms.",
      "https://crbug.com/dawn/343", ToggleStage::Device}},
    {Toggle::DisableBaseInstance,
     {"disable_base_instance",
      "Disables the use of non-zero base instance which is unsupported on some platforms.",
      "https://crbug.com/dawn/343", ToggleStage::Device}},
    {Toggle::DisableIndexedDrawBuffers,
     {"disable_indexed_draw_buffers",
      "Disables the use of indexed draw buffer state which is unsupported on some platforms.",
      "https://crbug.com/dawn/582", ToggleStage::Device}},
    {Toggle::DisableSampleVariables,
     {"disable_sample_variables",
      "Disables gl_SampleMask and related functionality which is unsupported on some platforms.",
      "https://crbug.com/dawn/673", ToggleStage::Device}},
    {Toggle::UseD3D12SmallShaderVisibleHeapForTesting,
     {"use_d3d12_small_shader_visible_heap",
      "Enable use of a small D3D12 shader visible heap, instead of using a large one by default. "
      "This setting is used to test bindgroup encoding.",
      "https://crbug.com/dawn/155", ToggleStage::Device}},
    {Toggle::UseDXC,
     {"use_dxc",
      "Use DXC instead of FXC for compiling HLSL when both dxcompiler.dll and dxil.dll is "
      "available.",
      "https://crbug.com/dawn/402", ToggleStage::Adapter}},
    {Toggle::DisableRobustness,
     {"disable_robustness", "Disable robust buffer access", "https://crbug.com/dawn/480",
      ToggleStage::Device}},
    {Toggle::MetalEnableVertexPulling,
     {"metal_enable_vertex_pulling", "Uses vertex pulling to protect out-of-bounds reads on Metal",
      "https://crbug.com/dawn/480", ToggleStage::Device}},
    {Toggle::AllowUnsafeAPIs,
     {"allow_unsafe_apis",
      "Suppresses validation errors on API entry points or parameter combinations that aren't "
      "considered secure yet.",
      "http://crbug.com/1138528", ToggleStage::Instance}},
    {Toggle::FlushBeforeClientWaitSync,
     {"flush_before_client_wait_sync",
      "Call glFlush before glClientWaitSync to work around bugs in the latter",
      "https://crbug.com/dawn/633", ToggleStage::Device}},
    {Toggle::UseTempBufferInSmallFormatTextureToTextureCopyFromGreaterToLessMipLevel,
     {"use_temp_buffer_in_small_format_texture_to_texture_copy_from_greater_to_less_mip_level",
      "Split texture-to-texture copy into two copies: copy from source texture into a temporary "
      "buffer, and copy from the temporary buffer into the destination texture under specific "
      "situations. This workaround is by default enabled on some Intel GPUs which have a driver "
      "bug in the execution of CopyTextureRegion() when we copy with the formats whose texel "
      "block sizes are less than 4 bytes from a greater mip level to a smaller mip level on D3D12 "
      "backends.",
      "https://crbug.com/1161355", ToggleStage::Device}},
    {Toggle::EmitHLSLDebugSymbols,
     {"emit_hlsl_debug_symbols",
      "Sets the D3DCOMPILE_SKIP_OPTIMIZATION and D3DCOMPILE_DEBUG compilation flags when compiling "
      "HLSL code. Enables better shader debugging with external graphics debugging tools.",
      "https://crbug.com/dawn/776", ToggleStage::Device}},
    {Toggle::DisallowSpirv,
     {"disallow_spirv",
      "Disallow usage of SPIR-V completely so that only WGSL is used for shader modules. This is "
      "useful to prevent a Chromium renderer process from successfully sending SPIR-V code to be "
      "compiled in the GPU process.",
      "https://crbug.com/1214923", ToggleStage::Device}},
    {Toggle::DumpShaders,
     {"dump_shaders",
      "Dump shaders for debugging purposes. Dumped shaders will be log via EmitLog, thus printed "
      "in Chrome console or consumed by user-defined callback function.",
      "https://crbug.com/dawn/792", ToggleStage::Device}},
    {Toggle::DisableWorkgroupInit,
     {"disable_workgroup_init",
      "Disables the workgroup memory zero-initialization for compute shaders.",
      "https://crbug.com/tint/1003", ToggleStage::Device}},
    {Toggle::DisableDemoteToHelper,
     {"disable_demote_to_helper",
      "Disables the conversion of discard to demote to helper thread in the IR transform",
      "https://crbug.com/42250787", ToggleStage::Device}},
    {Toggle::VulkanUseDemoteToHelperInvocationExtension,
     {"vulkan_use_demote_to_helper_invocation_extension",
      "Sets the use of the vulkan demote to helper extension", "https://crbug.com/42250787",
      ToggleStage::Device}},
    {Toggle::DisableSymbolRenaming,
     {"disable_symbol_renaming", "Disables the WGSL symbol renaming so that names are preserved.",
      "https://crbug.com/dawn/1016", ToggleStage::Device}},
    {Toggle::UseUserDefinedLabelsInBackend,
     {"use_user_defined_labels_in_backend",
      "Enables setting labels on backend-specific APIs that label objects. The labels used will be "
      "those of the corresponding frontend objects if non-empty and default labels otherwise. "
      "Defaults to false. NOTE: On Vulkan, backend labels are currently always set (with default "
      "labels if this toggle is not set). The reason is that Dawn currently uses backend "
      "object labels on Vulkan to map errors back to the device with which the backend objects "
      "included in the error are associated.",
      "https://crbug.com/dawn/840", ToggleStage::Device}},
    {Toggle::UsePlaceholderFragmentInVertexOnlyPipeline,
     {"use_placeholder_fragment_in_vertex_only_pipeline",
      "Use a placeholder empty fragment shader in vertex only render pipeline. This toggle must be "
      "enabled for OpenGL ES backend, the Vulkan Backend, and serves as a workaround by default "
      "enabled on some Metal "
      "devices with Intel GPU to ensure the depth result is correct.",
      "https://crbug.com/dawn/136", ToggleStage::Device}},
    {Toggle::FxcOptimizations,
     {"fxc_optimizations",
      "Enable optimizations when compiling with FXC. Disabled by default because FXC miscompiles "
      "in many cases when optimizations are enabled.",
      "https://crbug.com/dawn/1203", ToggleStage::Device}},
    {Toggle::RecordDetailedTimingInTraceEvents,
     {"record_detailed_timing_in_trace_events",
      "Record detailed timing information in trace events at certain point. Currently the timing "
      "information is recorded right before calling ExecuteCommandLists on a D3D12 command queue, "
      "and the information includes system time, CPU timestamp, GPU timestamp, and their "
      "frequency.",
      "https://crbug.com/dawn/1264", ToggleStage::Device}},
    {Toggle::DisableTimestampQueryConversion,
     {"disable_timestamp_query_conversion",
      "Resolve timestamp queries into ticks instead of nanoseconds.", "https://crbug.com/dawn/1305",
      ToggleStage::Device}},
    {Toggle::TimestampQuantization,
     {"timestamp_quantization",
      "Enable timestamp queries quantization to reduce the precision of timers that can be created "
      "with timestamp queries.",
      "https://crbug.com/dawn/1800", ToggleStage::Device}},
    {Toggle::ClearBufferBeforeResolveQueries,
     {"clear_buffer_before_resolve_queries",
      "clear destination buffer to zero before resolving queries. This toggle is enabled on Intel "
      "Gen12 GPUs due to driver issue.",
      "https://crbug.com/dawn/1823", ToggleStage::Device}},
    {Toggle::VulkanUseZeroInitializeWorkgroupMemoryExtension,
     {"use_vulkan_zero_initialize_workgroup_memory_extension",
      "Initialize workgroup memory with OpConstantNull on Vulkan when the Vulkan extension "
      "VK_KHR_zero_initialize_workgroup_memory is supported.",
      "https://crbug.com/dawn/1302", ToggleStage::Device}},
    {Toggle::MetalRenderR8RG8UnormSmallMipToTempTexture,
     {"metal_render_r8_rg8_unorm_small_mip_to_temp_texture",
      "Metal Intel devices have issues with r8unorm and rg8unorm textures where rendering to small "
      "mips (level >= 2) doesn't work correctly. Workaround this issue by detecting this case and "
      "rendering to a temporary texture instead (with copies before and after if needed).",
      "https://crbug.com/dawn/1071", ToggleStage::Device}},
    {Toggle::DisableBlobCache,
     {"disable_blob_cache",
      "Disables usage of the blob cache (backed by the platform cache if set/passed). Prevents any "
      "persistent caching capabilities, i.e. pipeline caching.",
      "https://crbug.com/dawn/549", ToggleStage::Device}},
    {Toggle::D3D12ForceClearCopyableDepthStencilTextureOnCreation,
     {"d3d12_force_clear_copyable_depth_stencil_texture_on_creation",
      "Always clearing copyable depth stencil textures when creating them instead of skipping the "
      "initialization when the entire subresource is the copy destination as a workaround on Intel "
      "D3D12 drivers.",
      "https://crbug.com/dawn/1487", ToggleStage::Device}},
    {Toggle::D3D12DontSetClearValueOnDepthTextureCreation,
     {"d3d12_dont_set_clear_value_on_depth_texture_creation",
      "Don't set D3D12_CLEAR_VALUE when creating depth textures with CreatePlacedResource() or "
      "CreateCommittedResource() as a workaround on Intel Gen12 D3D12 drivers.",
      "https://crbug.com/dawn/1487", ToggleStage::Device}},
    {Toggle::D3D12AlwaysUseTypelessFormatsForCastableTexture,
     {"d3d12_always_use_typeless_formats_for_castable_texture",
      "Always use the typeless DXGI format when we create a texture with valid viewFormat. This "
      "Toggle is enabled by default on the D3D12 platforms where CastingFullyTypedFormatSupported "
      "is false.",
      "https://crbug.com/dawn/1276", ToggleStage::Device}},
    {Toggle::D3D12AllocateExtraMemoryFor2DArrayColorTexture,
     {"d3d12_allocate_extra_memory_for_2d_array_color_texture",
      "Memory allocation for 2D array color texture may be smaller than it should be on D3D12 on "
      "some Intel devices. So texture access can be out-of-bound, which may cause critical "
      "security issue. We can workaround this security issue via allocating extra memory and "
      "limiting its access in itself.",
      "https://crbug.com/dawn/949", ToggleStage::Device}},
    {Toggle::D3D12UseTempBufferInDepthStencilTextureAndBufferCopyWithNonZeroBufferOffset,
     {"d3d12_use_temp_buffer_in_depth_stencil_texture_and_buffer_copy_with_non_zero_buffer_offset",
      "Split buffer-texture copy into two copies: do first copy with a temporary buffer at offset "
      "0, then copy from the temporary buffer to the destination. Now this toggle must be enabled "
      "on the D3D12 platforms where programmable MSAA is not supported.",
      "https://crbug.com/dawn/727", ToggleStage::Device}},
    {Toggle::D3D12UseTempBufferInTextureToTextureCopyBetweenDifferentDimensions,
     {"d3d12_use_temp_buffer_in_texture_to_texture_copy_between_different_dimensions",
      "Use an intermediate temporary buffer when copying between textures of different dimensions. "
      "Force-enabled on D3D12 when the driver does not have the "
      "TextureCopyBetweenDimensionsSupported feature.",
      "https://crbug.com/dawn/1216", ToggleStage::Device}},
    {Toggle::ApplyClearBigIntegerColorValueWithDraw,
     {"apply_clear_big_integer_color_value_with_draw",
      "Apply the clear value of the color attachment with a draw call when load op is 'clear'. "
      "This toggle is enabled by default on D3D12 backends when we set large integer values "
      "(> 2^24 or < -2^24 for signed integer formats) as the clear value of a color attachment "
      "with 32-bit integer or unsigned integer formats because D3D12 APIs only support using "
      "float numbers as clear values, while a float number cannot always precisely represent an "
      "integer that is greater than 2^24 or smaller than -2^24). This toggle is also enabled on "
      "Intel GPUs on Metal backend due to a driver issue on Intel Metal driver.",
      "https://crbug.com/dawn/537", ToggleStage::Device}},
    {Toggle::MetalUseMockBlitEncoderForWriteTimestamp,
     {"metal_use_mock_blit_encoder_for_write_timestamp",
      "Add mock blit command to blit encoder when encoding writeTimestamp as workaround on Metal."
      "This toggle is enabled by default on Metal backend where GPU counters cannot be stored to"
      "sampleBufferAttachments on empty blit encoder.",
      "https://crbug.com/dawn/1473", ToggleStage::Device}},
    {Toggle::MetalDisableTimestampPeriodEstimation,
     {"metal_disable_timestamp_period_estimation",
      "Calling sampleTimestamps:gpuTimestamp: from MTLDevice to estimate timestamp period leads to "
      "GPU overheating on some specific Intel GPUs due to driver issue, such as Intel Iris "
      "Plus Graphics 655. Enable this workaround to skip timestamp period estimation and use a "
      "default value instead on the specific GPUs.",
      "https://crbug.com/342701242", ToggleStage::Device}},
    {Toggle::VulkanSplitCommandBufferOnComputePassAfterRenderPass,
     {"vulkan_split_command_buffer_on_compute_pass_after_render_pass",
      "Splits any command buffer where a compute pass is recorded after a render pass. This "
      "toggle is enabled by default on Qualcomm GPUs, which have been observed experiencing a "
      "driver crash in this situation.",
      "https://crbug.com/dawn/1564", ToggleStage::Device}},
    {Toggle::DisableSubAllocationFor2DTextureWithCopyDstOrRenderAttachment,
     {"disable_sub_allocation_for_2d_texture_with_copy_dst_or_render_attachment",
      "Disable resource sub-allocation for the 2D texture with CopyDst or RenderAttachment usage. "
      "Due to driver issues, this toggle is enabled by default on D3D12 backends using Intel "
      "Gen9.5 or Gen11 GPUs, on Vulkan backends using Intel Gen12 GPUs, and D3D12 backends using "
      "AMD GPUs.",
      "https://crbug.com/1237175", ToggleStage::Device}},
    {Toggle::MetalUseCombinedDepthStencilFormatForStencil8,
     {"metal_use_combined_depth_stencil_format_for_stencil8",
      "Use a combined depth stencil format instead of stencil8. Works around an issue where the "
      "stencil8 format alone does not work correctly. This toggle also causes depth stencil "
      "attachments using a stencil8 format to also set the depth attachment in the Metal render "
      "pass. This works around another issue where Metal fails to set the stencil attachment "
      "correctly for a combined depth stencil format if the depth attachment is not also set.",
      "https://crbug.com/dawn/1389", ToggleStage::Device}},
    {Toggle::MetalUseBothDepthAndStencilAttachmentsForCombinedDepthStencilFormats,
     {"metal_use_both_depth_and_stencil_attachments_for_combined_depth_stencil_formats",
      "In Metal, depth and stencil attachments are set separately. Setting just one without the "
      "other does not work correctly for combined depth stencil formats on some Metal drivers. "
      "This workarounds ensures that both are set. This situation arises during lazy clears, or "
      "for stencil8 formats if metal_use_combined_depth_stencil_format_for_stencil8 is also "
      "enabled.",
      "https://crbug.com/dawn/1389", ToggleStage::Device}},
    {Toggle::MetalKeepMultisubresourceDepthStencilTexturesInitialized,
     {"metal_keep_multisubresource_depth_stencil_textures_initialized",
      "Some platforms have bugs where the wrong depth stencil subresource is read/written. To "
      "avoid reads of uninitialized data, ensure that depth stencil textures with more than one "
      "subresource are completely initialized, and StoreOp::Discard is always translated as a "
      "Store.",
      "https://crbug.com/dawn/838", ToggleStage::Device}},
    {Toggle::MetalFillEmptyOcclusionQueriesWithZero,
     {"metal_fill_empty_occlusion_queries_with_zero",
      "Apple GPUs leave stale results in the visibility result buffer instead of writing zero if "
      "an occlusion query is empty. Workaround this by explicitly filling it with zero if there "
      "are no draw calls.",
      "https://crbug.com/dawn/1707", ToggleStage::Device}},
    {Toggle::UseBlitForBufferToDepthTextureCopy,
     {"use_blit_for_buffer_to_depth_texture_copy",
      "Use a blit instead of a copy command to copy buffer data to the depth aspect of a "
      "texture. Works around an issue where depth writes by copy commands are not visible "
      "to a render or compute pass.",
      "https://crbug.com/dawn/1389", ToggleStage::Device}},
    {Toggle::UseBlitForBufferToStencilTextureCopy,
     {"use_blit_for_buffer_to_stencil_texture_copy",
      "Use a blit instead of a copy command to copy buffer data to the stencil aspect of a "
      "texture. Works around an issue where stencil writes by copy commands are not visible "
      "to a render or compute pass.",
      "https://crbug.com/dawn/1389", ToggleStage::Device}},
    {Toggle::UseBlitForStencilTextureWrite,
     {"use_blit_for_stencil_texture_write",
      "Use a blit instead of a write texture command to upload data to the stencil aspect of a "
      "texture. Works around for OpenGLES when glTexSubImage doesn't support GL_STENCIL_INDEX, "
      "and when the texture format is depth-stencil-combined.",
      "https://crbug.com/dawn/2391", ToggleStage::Device}},
    {Toggle::UseBlitForDepthTextureToTextureCopyToNonzeroSubresource,
     {"use_blit_for_depth_texture_to_texture_copy_to_nonzero_subresource",
      "Use a blit to copy from a depth texture to the nonzero subresource of a depth texture. "
      "Works around an issue where nonzero layers are not written.",
      "https://crbug.com/dawn/1083", ToggleStage::Device}},
    {Toggle::UseBlitForDepth16UnormTextureToBufferCopy,
     {"use_blit_for_depth16unorm_texture_to_buffer_copy",
      "Use a blit instead of a copy command to copy depth aspect of a texture to a buffer."
      "Workaround for OpenGL and OpenGLES.",
      "https://crbug.com/dawn/1782", ToggleStage::Device}},
    {Toggle::UseBlitForDepth32FloatTextureToBufferCopy,
     {"use_blit_for_depth32float_texture_to_buffer_copy",
      "Use a blit instead of a copy command to copy depth aspect of a texture to a buffer."
      "Workaround for OpenGLES.",
      "https://crbug.com/dawn/1782", ToggleStage::Device}},
    {Toggle::UseBlitForStencilTextureToBufferCopy,
     {"use_blit_for_stencil_texture_to_buffer_copy",
      "Use a blit instead of a copy command to copy stencil aspect of a texture to a buffer."
      "Workaround for OpenGLES.",
      "https://crbug.com/dawn/1782", ToggleStage::Device}},
    {Toggle::UseBlitForSnormTextureToBufferCopy,
     {"use_blit_for_snorm_texture_to_buffer_copy",
      "Use a blit instead of a copy command to copy snorm texture to a buffer."
      "Workaround for OpenGLES.",
      "https://crbug.com/dawn/1781", ToggleStage::Device}},
    {Toggle::UseBlitForBGRA8UnormTextureToBufferCopy,
     {"use_blit_for_bgra8unorm_texture_to_buffer_copy",
      "Use a blit instead of a copy command to copy bgra8unorm texture to a buffer."
      "Workaround for OpenGLES.",
      "https://crbug.com/dawn/1393", ToggleStage::Device}},
    {Toggle::UseBlitForRGB9E5UfloatTextureCopy,
     {"use_blit_for_rgb9e5ufloat_texture_copy",
      "Use a blit instead of a copy command to copy rgb9e5ufloat texture to a texture or a buffer."
      "Workaround for OpenGLES.",
      "https://crbug.com/dawn/2079", ToggleStage::Device}},
    {Toggle::UseBlitForRG11B10UfloatTextureCopy,
     {"use_blit_for_rgb9e5ufloat_texture_copy",
      "Use a blit instead of a copy command to copy rg11b10ufloat texture to a texture or a buffer."
      "Workaround for OpenGLES.",
      "https://crbug.com/381214487", ToggleStage::Device}},
    {Toggle::UseBlitForFloat16TextureCopy,
     {"use_blit_for_float_16_texture_copy",
      "Use a blit instead of a copy command to copy float16 texture to a texture or a buffer."
      "Workaround for OpenGLES.",
      "https://crbug.com/381214487", ToggleStage::Device}},
    {Toggle::UseBlitForFloat32TextureCopy,
     {"use_blit_for_float_32_texture_copy",
      "Use a blit instead of a copy command to copy float32 texture to a texture or a buffer."
      "Workaround for OpenGLES.",
      "https://crbug.com/381214487", ToggleStage::Device}},
    {Toggle::UseBlitForT2B,
     {"use_blit_for_t2b",
      "Use a compute based blit instead of a copy command to copy texture with supported format to "
      "a buffer.",
      "https://crbug.com/dawn/348654098", ToggleStage::Device}},
    {Toggle::UseT2B2TForSRGBTextureCopy,
     {"use_t2b2t_for_srgb_texture_copy",
      "Use T2B and B2T copies to emulate a T2T copy between sRGB and non-sRGB textures."
      "Workaround for OpenGLES.",
      "https://crbug.com/dawn/2362", ToggleStage::Device}},
    {Toggle::D3D12ReplaceAddWithMinusWhenDstFactorIsZeroAndSrcFactorIsDstAlpha,
     {"d3d12_replace_add_with_minus_when_dst_factor_is_zero_and_src_factor_is_dst_alpha",
      "Replace the blending operation 'Add' with 'Minus' when dstBlendFactor is 'Zero' and "
      "srcBlendFactor is 'DstAlpha'. Works around an Intel D3D12 driver issue about alpha "
      "blending.",
      "https://crbug.com/dawn/1579", ToggleStage::Device}},
    {Toggle::D3D12PolyfillReflectVec2F32,
     {"d3d12_polyfill_reflect_vec2_f32",
      "Polyfill the reflect builtin for vec2<f32> for D3D12. This toggle is enabled by default on "
      "D3D12 backends using FXC on Intel GPUs due to a driver issue on Intel D3D12 driver.",
      "https://crbug.com/tint/1798", ToggleStage::Device}},
    {Toggle::VulkanClearGen12TextureWithCCSAmbiguateOnCreation,
     {"vulkan_clear_gen12_texture_with_ccs_ambiguate_on_creation",
      "Clears some R8-like textures to full 0 bits as soon as they are created. This Toggle is "
      "enabled on Intel Gen12 GPUs due to a mesa driver issue.",
      "https://crbug.com/chromium/1361662", ToggleStage::Device}},
    {Toggle::D3D12UseRootSignatureVersion1_1,
     {"d3d12_use_root_signature_version_1_1",
      "Use D3D12 Root Signature Version 1.1 to make additional guarantees about the descriptors in "
      "a descriptor heap and the data pointed to by the descriptors so that the drivers can make "
      "better optimizations on them.",
      "https://crbug.com/tint/1890", ToggleStage::Device}},
    {Toggle::VulkanUseImageRobustAccess2,
     {"vulkan_use_image_robust_access_2",
      "Disable Tint robustness transform on textures when VK_EXT_robustness2 is supported and "
      "robustImageAccess2 == VK_TRUE.",
      "https://crbug.com/tint/1890", ToggleStage::Device}},
    {Toggle::VulkanUseBufferRobustAccess2,
     {"vulkan_use_buffer_robust_access_2",
      "Disable index clamping on the runtime-sized arrays on buffers in Tint robustness transform "
      "when VK_EXT_robustness2 is supported and robustBufferAccess2 == VK_TRUE.",
      "https://crbug.com/tint/1890", ToggleStage::Device}},
    {Toggle::D3D12Use64KBAlignedMSAATexture,
     {"d3d12_use_64kb_alignment_msaa_texture",
      "Create MSAA textures with 64KB (D3D12_SMALL_MSAA_RESOURCE_PLACEMENT_ALIGNMENT) alignment.",
      "https://crbug.com/dawn/282", ToggleStage::Device}},
    {Toggle::ResolveMultipleAttachmentInSeparatePasses,
     {"resolve_multiple_attachments_in_separate_passes",
      "When multiple MSAA attachments are used in a render pass, splits any resolve steps into a "
      "separate render pass per resolve target. "
      "This workaround is enabled by default on ARM Mali drivers.",
      "https://crbug.com/dawn/1550", ToggleStage::Device}},
    {Toggle::D3D12CreateNotZeroedHeap,
     {"d3d12_create_not_zeroed_heap",
      "Create D3D12 heap with D3D12_HEAP_FLAG_CREATE_NOT_ZEROED when it is supported. It is safe "
      "because in Dawn we always clear the resources manually when needed.",
      "https://crbug.com/dawn/484", ToggleStage::Device}},
    {Toggle::D3D12DontUseNotZeroedHeapFlagOnTexturesAsCommitedResources,
     {"d3d12_dont_use_not_zeroed_heap_flag_on_textures_as_commited_resources",
      "Don't set the heap flag D3D12_HEAP_FLAG_CREATE_NOT_ZEROED on the D3D12 textures created "
      "with CreateCommittedResource() as a workaround of some driver issues on Intel Gen9 and "
      "Gen11 GPUs.",
      "https://crbug.com/dawn/484", ToggleStage::Device}},
    {Toggle::UseTintIR,
     {"use_tint_ir", "Enable the use of the Tint IR for backend codegen.",
      "https://crbug.com/tint/1718", ToggleStage::Device}},
    {Toggle::D3DDisableIEEEStrictness,
     {"d3d_disable_ieee_strictness",
      "Disable IEEE strictness when compiling shaders. It is otherwise enabled by default to "
      "workaround issues where FXC can miscompile code that depends on special float values (NaN, "
      "INF, etc).",
      "https://crbug.com/tint/976", ToggleStage::Device}},
    {Toggle::PolyFillPacked4x8DotProduct,
     {"polyfill_packed_4x8_dot_product",
      "Always use the polyfill version of dot4I8Packed() and dot4U8Packed().",
      "https://crbug.com/tint/1497", ToggleStage::Device}},
    {Toggle::PolyfillPackUnpack4x8Norm,
     {"polyfill_pack_unpack_4x8_norm",
      "Always use the polyfill version of pack4x8snorm, pack4x8unorm, unpack4x8snorm, "
      "unpack4x8unorm.",
      "https://crbug.com/379551588", ToggleStage::Device}},
    {Toggle::D3D12PolyFillPackUnpack4x8,
     {"d3d12_polyfill_pack_unpack_4x8",
      "Always use the polyfill version of pack4xI8(), pack4xU8(), pack4xI8Clamp(), unpack4xI8() "
      "and unpack4xU8() on D3D12 backends. Note that these functions are always polyfilled on all "
      "other backends right now.",
      "https://crbug.com/tint/1497", ToggleStage::Device}},
    {Toggle::ExposeWGSLTestingFeatures,
     {"expose_wgsl_testing_features",
      "Make the Instance expose the ChromiumTesting* features for testing of "
      "wgslLanguageFeatures functionality.",
      "https://crbug.com/dawn/2260", ToggleStage::Instance}},
    {Toggle::ExposeWGSLExperimentalFeatures,
     {"expose_wgsl_experimental_features",
      "Make the Instance expose the experimental features but not the unsage ones, so that safe "
      "experimental features can be used without the need for allow_unsafe_apis",
      "https://crbug.com/dawn/2260", ToggleStage::Instance}},
    {Toggle::DisablePolyfillsOnIntegerDivisonAndModulo,
     {"disable_polyfills_on_integer_div_and_mod",
      "Disable the Tint polyfills on integer division and modulo.", "https://crbug.com/tint/2128",
      ToggleStage::Device}},
    {Toggle::EnableImmediateErrorHandling,
     {"enable_immediate_error_handling",
      "Have the uncaptured error callback invoked immediately when an error occurs, rather than "
      "waiting for the next Tick. This enables using the stack trace in which the uncaptured error "
      "occured when breaking into the uncaptured error callback.",
      "https://crbug.com/dawn/1789", ToggleStage::Device}},
    {Toggle::VulkanUseStorageInputOutput16,
     {"vulkan_use_storage_input_output_16",
      "Use the StorageInputOutput16 SPIR-V capability for f16 shader IO types when the device "
      "supports it.",
      "https://crbug.com/tint/2161", ToggleStage::Device}},
    {Toggle::D3D12DontUseShaderModel66OrHigher,
     {"d3d12_dont_use_shader_model_66_or_higher",
      "Only use shader model 6.5 or less for D3D12 backend, to workaround issues on some Intel "
      "devices.",
      "https://crbug.com/dawn/2470", ToggleStage::Adapter}},
    {Toggle::UsePackedDepth24UnormStencil8Format,
     {"use_packed_depth24_unorm_stencil8_format",
      "Use a packed depth24_unorm_stencil8 format like DXGI_FORMAT_D24_UNORM_STENCIL8_UINT on D3D "
      "for wgpu::TextureFormat::Depth24PlusStencil8.",
      "https://crbug.com/341254292", ToggleStage::Device}},
    {Toggle::D3D12ForceStencilComponentReplicateSwizzle,
     {"d3d12_force_stencil_component_replicate_swizzle",
      "Force a replicate swizzle for the stencil component i.e. (ssss) instead of (s001) to "
      "workaround issues on certain Nvidia drivers on D3D12 with depth24_unorm_stencil8 format.",
      "https://crbug.com/341254292", ToggleStage::Device}},
    {Toggle::D3D12ExpandShaderResourceStateTransitionsToCopySource,
     {"d3d12_expand_shader_resource_state_transitions_to_copy_source",
      "When transitioning to any shader resource states PIXEL or NON_PIXEL_SHADER_RESOURCE include "
      "COPY_SOURCE too on Nvidia since the shader resource states seem to miss flushing all caches "
      "and layout transitions causing rendering corruption.",
      "https://crbug.com/356905061", ToggleStage::Device}},
    {Toggle::GLDepthBiasModifier,
     {"gl_depth_bias_modifier",
      "Empirically some GL drivers select n+1 when a depth value lies between 2^n and 2^(n+1), "
      "while the WebGPU CTS is expecting n. Scale the depth bias value by multiple 0.5 on certain "
      "backends to achieve conformant result.",
      "https://crbug.com/42241017", ToggleStage::Device}},
    {Toggle::GLForceES31AndNoExtensions,
     {"gl_force_es_31_and_no_extensions",
      "Force EGLContext creation with the minimal ES version and no extensions required by the "
      "Compat spec, which for OpenGLES is version 3.1 with no extensions. This toggle is used by "
      "end2end testing.",
      "crbug.com/382084196", ToggleStage::Adapter}},
    {Toggle::VulkanMonolithicPipelineCache,
     {"vulkan_monolithic_pipeline_cache",
      "Use a monolithic VkPipelineCache per device. The embedder is responsible for calling "
      "PerformIdleTasks() on the device to serialize VkPipelineCache to BlobCache if needed.",
      "crbug.com/370343334", ToggleStage::Device}},
    {Toggle::MetalSerializeTimestampGenerationAndResolution,
     {"metal_serialize_timestamp_generation_and_resolution",
      "Newer Apple GPUs can race on query set resolution with timestamp writing from earlier "
      "compute passes. This can be worked around by signaling and waiting for a shared event in "
      "between timestamp generation and resolution.",
      "crbug.com/372698905", ToggleStage::Device}},
    {Toggle::D3D12RelaxMinSubgroupSizeTo8,
     {"d3d12_relax_min_subgroup_size_to_8",
      "Relax the adapters and devices' subgroupMinSize to the minimium of D3D12 reported "
      "minWaveLaneCount and 8. Some D3D12 drivers is possible to run fragment shader with wave "
      "count 8 while reporting minWaveLaneCount 16.",
      "https://crbug.com/381969450", ToggleStage::Adapter}},
    {Toggle::D3D12RelaxBufferTextureCopyPitchAndOffsetAlignment,
     {"d3d12_relax_buffer_texture_copy_pitch_and_offset_alignment",
      "Don't require the alignments of D3D12_TEXTURE_DATA_PITCH_ALIGNMENT (256) for row pitch "
      "and D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT (512) for offset in buffer-texture copies.",
      "https://crbug.com/381000081", ToggleStage::Device}},
    {Toggle::UseVulkanMemoryModel,
     {"use_vulkan_memory_model", "Use the Vulkan Memory Model if available.",
      "https://crbug.com/392606604", ToggleStage::Device}},
    {Toggle::NoWorkaroundSampleMaskBecomesZeroForAllButLastColorTarget,
     {"no_workaround_sample_mask_becomes_zero_for_all_but_last_color_target",
      "MacOS 12.0+ Intel has a bug where the sample mask is only applied for the last color "
      "target. If there are multiple color targets, all but the last one will use a sample mask "
      "of zero.",
      "https://crbug.com/dawn/1462", ToggleStage::Device}},
    {Toggle::NoWorkaroundIndirectBaseVertexNotApplied,
     {"no_workaround_indirect_base_vertex_not_applied",
      "MacOS Intel < Gen9 has a bug where indirect base vertex is not applied for "
      "drawIndexedIndirect. Draws are done as if it is always zero.",
      "https://crbug.com/dawn/966", ToggleStage::Device}},
    {Toggle::NoWorkaroundDstAlphaAsSrcBlendFactorForBothColorAndAlphaDoesNotWork,
     {"no_workaround_dst_alpha_as_src_blend_factor_for_both_color_and_alpha_does_not_work",
      "Using D3D12_BLEND_DEST_ALPHA as source blend factor for both color and alpha blending "
      "doesn't work correctly on the D3D12 backend using Intel Gen9 or Gen9.5 GPUs.",
      "https://crbug.com/dawn/1579", ToggleStage::Device}},
    {Toggle::ClearColorWithDraw,
     {"clear_color_with_draw",
      "Use Draw instead of ClearRenderTargetView() to clear color attachments. On D3D11, "
      "ClearRenderTargetView() does not always clear texture correctly.",
      "https://crbug.com/chromium/329702368", ToggleStage::Device}},
    {Toggle::VulkanSkipDraw,
     {"vulkan_skip_draw",
      "Some chrome tests run with swiftshader, they don't care about the pixel output. This toggle "
      "allows skipping expensive draw operations for them.",
      "https://crbug.com/chromium/331688266", ToggleStage::Device}},
    {Toggle::D3D11UseUnmonitoredFence,
     {"d3d11_use_unmonitored_fence", "Use d3d11 unmonitored fence.",
      "https://crbug.com/chromium/335553337", ToggleStage::Device}},
    {Toggle::IgnoreImportedAHardwareBufferVulkanImageSize,
     {"ignore_imported_ahardwarebuffer_vulkan_image_size",
      "Don't validate the required VkImage size against the size of the AHardwareBuffer on import. "
      "Some drivers report the wrong size.",
      "https://crbug.com/333424893", ToggleStage::Device}},
    // Comment to separate the }} so it is clearer what to copy-paste to add a toggle.
}};
}  // anonymous namespace

void TogglesSet::Set(Toggle toggle, bool enabled) {
    DAWN_ASSERT(toggle != Toggle::InvalidEnum);
    const size_t toggleIndex = static_cast<size_t>(toggle);
    bitset.set(toggleIndex, enabled);
}

bool TogglesSet::Has(Toggle toggle) const {
    DAWN_ASSERT(toggle != Toggle::InvalidEnum);
    const size_t toggleIndex = static_cast<size_t>(toggle);
    return bitset.test(toggleIndex);
}

size_t TogglesSet::Count() const {
    return bitset.count();
}

TogglesSet::Iterator TogglesSet::Iterate() const {
    return IterateBitSet(bitset);
}

TogglesState::TogglesState(ToggleStage stage) : mStage(stage) {}

TogglesState TogglesState::CreateFromTogglesDescriptor(const DawnTogglesDescriptor* togglesDesc,
                                                       ToggleStage requiredStage) {
    TogglesState togglesState(requiredStage);

    if (togglesDesc == nullptr) {
        return togglesState;
    }

    TogglesInfo togglesInfo;
    for (uint32_t i = 0; i < togglesDesc->enabledToggleCount; ++i) {
        Toggle toggle = togglesInfo.ToggleNameToEnum(togglesDesc->enabledToggles[i]);
        if (toggle != Toggle::InvalidEnum) {
            const ToggleInfo* toggleInfo = togglesInfo.GetToggleInfo(toggle);
            // Accept the required toggles of current and earlier stage to allow override
            // inheritance.
            if (toggleInfo->stage <= requiredStage) {
                togglesState.mTogglesSet.Set(toggle, true);
                togglesState.mEnabledToggles.Set(toggle, true);
            }
        }
    }
    for (uint32_t i = 0; i < togglesDesc->disabledToggleCount; ++i) {
        Toggle toggle = togglesInfo.ToggleNameToEnum(togglesDesc->disabledToggles[i]);
        if (toggle != Toggle::InvalidEnum) {
            const ToggleInfo* toggleInfo = togglesInfo.GetToggleInfo(toggle);
            // Accept the required toggles of current and earlier stage to allow override
            // inheritance.
            if (toggleInfo->stage <= requiredStage) {
                togglesState.mTogglesSet.Set(toggle, true);
                togglesState.mEnabledToggles.Set(toggle, false);
            }
        }
    }

    return togglesState;
}

TogglesState& TogglesState::InheritFrom(const TogglesState& inheritedToggles) {
    DAWN_ASSERT(inheritedToggles.GetStage() < mStage);

    // Do inheritance. All toggles that are force-set in the inherited toggles states would
    // be force-set in the result toggles state, and all toggles that are set in the inherited
    // toggles states and not required in current toggles state would be set in the result toggles
    // state.
    for (uint32_t i : inheritedToggles.mTogglesSet.Iterate()) {
        const Toggle& toggle = static_cast<Toggle>(i);
        DAWN_ASSERT(TogglesInfo::GetToggleInfo(toggle)->stage < mStage);
        bool isEnabled = inheritedToggles.mEnabledToggles.Has(toggle);
        bool isForced = inheritedToggles.mForcedToggles.Has(toggle);
        // Only inherit a toggle if it is not set by user requirement or is forced in earlier stage.
        // In this way we allow user requirement override the inheritance if not forced.
        if (!mTogglesSet.Has(toggle) || isForced) {
            mTogglesSet.Set(toggle, true);
            mEnabledToggles.Set(toggle, isEnabled);
            mForcedToggles.Set(toggle, isForced);
        }
    }

    return *this;
}

// Set a toggle to given state, if the toggle has not been already set. Do nothing otherwise.
void TogglesState::Default(Toggle toggle, bool enabled) {
    DAWN_ASSERT(toggle != Toggle::InvalidEnum);
    DAWN_ASSERT(TogglesInfo::GetToggleInfo(toggle)->stage == mStage);
    if (IsSet(toggle)) {
        return;
    }
    mTogglesSet.Set(toggle, true);
    mEnabledToggles.Set(toggle, enabled);
}

void TogglesState::ForceSet(Toggle toggle, bool enabled) {
    DAWN_ASSERT(toggle != Toggle::InvalidEnum);
    DAWN_ASSERT(TogglesInfo::GetToggleInfo(toggle)->stage == mStage);
    // Make sure that each toggle is force-set at most once.
    DAWN_ASSERT(!mForcedToggles.Has(toggle));
    if (mTogglesSet.Has(toggle) && mEnabledToggles.Has(toggle) != enabled) {
        dawn::WarningLog() << "Forcing toggle \"" << ToggleEnumToName(toggle) << "\" to " << enabled
                           << " when it was " << !enabled;
    }
    mTogglesSet.Set(toggle, true);
    mEnabledToggles.Set(toggle, enabled);
    mForcedToggles.Set(toggle, true);
}

TogglesState& TogglesState::SetForTesting(Toggle toggle, bool enabled, bool forced) {
    DAWN_ASSERT(toggle != Toggle::InvalidEnum);
    mTogglesSet.Set(toggle, true);
    mEnabledToggles.Set(toggle, enabled);
    mForcedToggles.Set(toggle, forced);

    return *this;
}

bool TogglesState::IsSet(Toggle toggle) const {
    // Ensure that the toggle never used earlier than its stage.
    DAWN_ASSERT(TogglesInfo::GetToggleInfo(toggle)->stage <= mStage);
    return mTogglesSet.Has(toggle);
}

// Return true if the toggle is provided in enable list, and false otherwise.
bool TogglesState::IsEnabled(Toggle toggle) const {
    // Ensure that the toggle never used earlier than its stage.
    DAWN_ASSERT(TogglesInfo::GetToggleInfo(toggle)->stage <= mStage);
    return mEnabledToggles.Has(toggle);
}

ToggleStage TogglesState::GetStage() const {
    return mStage;
}

std::vector<const char*> TogglesState::GetEnabledToggleNames() const {
    std::vector<const char*> enabledTogglesName(mEnabledToggles.Count());

    uint32_t index = 0;
    for (uint32_t i : mEnabledToggles.Iterate()) {
        const Toggle& toggle = static_cast<Toggle>(i);
        // All enabled toggles must be provided.
        DAWN_ASSERT(mTogglesSet.Has(toggle));
        const char* toggleName = ToggleEnumToName(toggle);
        enabledTogglesName[index] = toggleName;
        ++index;
    }

    return enabledTogglesName;
}

std::vector<const char*> TogglesState::GetDisabledToggleNames() const {
    std::vector<const char*> enabledTogglesName(mTogglesSet.Count() - mEnabledToggles.Count());

    uint32_t index = 0;
    for (uint32_t i : mTogglesSet.Iterate()) {
        const Toggle& toggle = static_cast<Toggle>(i);
        // Disabled toggles are those provided but not enabled.
        if (!mEnabledToggles.Has(toggle)) {
            const char* toggleName = ToggleEnumToName(toggle);
            enabledTogglesName[index] = toggleName;
            ++index;
        }
    }

    return enabledTogglesName;
}

// Allowing TogglesState to be used in cache key.
void StreamIn(stream::Sink* s, const TogglesState& togglesState) {
    StreamIn(s, togglesState.mEnabledToggles.bitset);
}

const char* ToggleEnumToName(Toggle toggle) {
    DAWN_ASSERT(toggle != Toggle::InvalidEnum);

    const ToggleEnumAndInfo& toggleNameAndInfo =
        kToggleNameAndInfoList[static_cast<size_t>(toggle)];
    DAWN_ASSERT(toggleNameAndInfo.toggle == toggle);
    return toggleNameAndInfo.info.name;
}

// static
std::vector<const ToggleInfo*> TogglesInfo::AllToggleInfos() {
    std::vector<const ToggleInfo*> infos;
    infos.reserve(kToggleNameAndInfoList.size());

    for (const auto& entry : kToggleNameAndInfoList) {
        infos.push_back(&(entry.info));
    }
    return infos;
}

TogglesInfo::TogglesInfo() = default;

TogglesInfo::~TogglesInfo() = default;

const ToggleInfo* TogglesInfo::GetToggleInfo(const char* toggleName) {
    DAWN_ASSERT(toggleName);

    EnsureToggleNameToEnumMapInitialized();

    const auto& iter = mToggleNameToEnumMap.find(toggleName);
    if (iter != mToggleNameToEnumMap.cend()) {
        return &kToggleNameAndInfoList[static_cast<size_t>(iter->second)].info;
    }
    return nullptr;
}

const ToggleInfo* TogglesInfo::GetToggleInfo(Toggle toggle) {
    DAWN_ASSERT(toggle != Toggle::InvalidEnum);

    return &kToggleNameAndInfoList[static_cast<size_t>(toggle)].info;
}

Toggle TogglesInfo::ToggleNameToEnum(const char* toggleName) {
    DAWN_ASSERT(toggleName);

    EnsureToggleNameToEnumMapInitialized();

    const auto& iter = mToggleNameToEnumMap.find(toggleName);
    if (iter != mToggleNameToEnumMap.cend()) {
        return kToggleNameAndInfoList[static_cast<size_t>(iter->second)].toggle;
    }
    return Toggle::InvalidEnum;
}

void TogglesInfo::EnsureToggleNameToEnumMapInitialized() {
    if (mToggleNameToEnumMapInitialized) {
        return;
    }

    for (size_t index = 0; index < kToggleNameAndInfoList.size(); ++index) {
        const ToggleEnumAndInfo& toggleNameAndInfo = kToggleNameAndInfoList[index];
        DAWN_ASSERT(index == static_cast<size_t>(toggleNameAndInfo.toggle));
        mToggleNameToEnumMap[toggleNameAndInfo.info.name] = toggleNameAndInfo.toggle;
    }

    mToggleNameToEnumMapInitialized = true;
}

}  // namespace dawn::native
