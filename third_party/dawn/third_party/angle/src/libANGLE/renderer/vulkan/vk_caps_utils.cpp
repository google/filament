//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_utils:
//    Helper functions for the Vulkan Caps.
//

#include "libANGLE/renderer/vulkan/vk_caps_utils.h"

#include <type_traits>

#include "common/system_utils.h"
#include "common/utilities.h"
#include "libANGLE/Caps.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/driver_utils.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/vk_cache_utils.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"
#include "vk_format_utils.h"

namespace
{
constexpr unsigned int kComponentsPerVector         = 4;
constexpr bool kEnableLogMissingExtensionsForGLES32 = false;
}  // anonymous namespace

namespace rx
{

namespace vk
{
namespace
{
// Checks to see if each format can be reinterpreted to an equivalent format in a different
// colorspace. If all supported formats can be reinterpreted, it returns true. Formats which are not
// supported at all are ignored and not counted as failures.
bool FormatReinterpretationSupported(const std::vector<GLenum> &optionalSizedFormats,
                                     const Renderer *renderer,
                                     bool checkLinearColorspace)
{
    for (GLenum glFormat : optionalSizedFormats)
    {
        const gl::TextureCaps &baseCaps = renderer->getNativeTextureCaps().get(glFormat);
        if (baseCaps.texturable && baseCaps.filterable)
        {
            const Format &vkFormat = renderer->getFormat(glFormat);
            // For capability query, we use the renderable format since that is what we are capable
            // of when we fallback.
            angle::FormatID imageFormatID = vkFormat.getActualRenderableImageFormatID();

            angle::FormatID reinterpretedFormatID = checkLinearColorspace
                                                        ? ConvertToLinear(imageFormatID)
                                                        : ConvertToSRGB(imageFormatID);

            const Format &reinterpretedVkFormat = renderer->getFormat(reinterpretedFormatID);

            if (reinterpretedVkFormat.getActualRenderableImageFormatID() != reinterpretedFormatID)
            {
                return false;
            }

            if (!renderer->haveSameFormatFeatureBits(imageFormatID, reinterpretedFormatID))
            {
                return false;
            }
        }
    }

    return true;
}

bool GetTextureSRGBDecodeSupport(const Renderer *renderer)
{
    static constexpr bool kLinearColorspace = true;

    // GL_SRGB and GL_SRGB_ALPHA unsized formats are also required by the spec, but the only valid
    // type for them is GL_UNSIGNED_BYTE, so they are fully included in the sized formats listed
    // here
    std::vector<GLenum> optionalSizedSRGBFormats = {
        GL_SRGB8,
        GL_SRGB8_ALPHA8_EXT,
        GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,
        GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,
        GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,
        GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,
    };

    if (!FormatReinterpretationSupported(optionalSizedSRGBFormats, renderer, kLinearColorspace))
    {
        return false;
    }

    return true;
}

bool GetTextureSRGBOverrideSupport(const Renderer *renderer,
                                   const gl::Extensions &supportedExtensions)
{
    static constexpr bool kNonLinearColorspace = false;

    // If the given linear format is supported, we also need to support its corresponding nonlinear
    // format. If the given linear format is NOT supported, we don't care about its corresponding
    // nonlinear format.
    std::vector<GLenum> optionalLinearFormats     = {GL_RGB8,
                                                     GL_RGBA8,
                                                     GL_COMPRESSED_RGB8_ETC2,
                                                     GL_COMPRESSED_RGBA8_ETC2_EAC,
                                                     GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,
                                                     GL_COMPRESSED_RGBA_ASTC_4x4,
                                                     GL_COMPRESSED_RGBA_ASTC_5x4,
                                                     GL_COMPRESSED_RGBA_ASTC_5x5,
                                                     GL_COMPRESSED_RGBA_ASTC_6x5,
                                                     GL_COMPRESSED_RGBA_ASTC_6x6,
                                                     GL_COMPRESSED_RGBA_ASTC_8x5,
                                                     GL_COMPRESSED_RGBA_ASTC_8x6,
                                                     GL_COMPRESSED_RGBA_ASTC_8x8,
                                                     GL_COMPRESSED_RGBA_ASTC_10x5,
                                                     GL_COMPRESSED_RGBA_ASTC_10x6,
                                                     GL_COMPRESSED_RGBA_ASTC_10x8,
                                                     GL_COMPRESSED_RGBA_ASTC_10x10,
                                                     GL_COMPRESSED_RGBA_ASTC_12x10,
                                                     GL_COMPRESSED_RGBA_ASTC_12x12};
    std::vector<GLenum> optionalS3TCLinearFormats = {
        GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
        GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT};
    std::vector<GLenum> optionalR8LinearFormats   = {GL_R8};
    std::vector<GLenum> optionalRG8LinearFormats  = {GL_RG8};
    std::vector<GLenum> optionalBPTCLinearFormats = {GL_COMPRESSED_RGBA_BPTC_UNORM_EXT};

    if (!FormatReinterpretationSupported(optionalLinearFormats, renderer, kNonLinearColorspace))
    {
        return false;
    }

    if (supportedExtensions.textureCompressionS3tcSrgbEXT)
    {
        if (!FormatReinterpretationSupported(optionalS3TCLinearFormats, renderer,
                                             kNonLinearColorspace))
        {
            return false;
        }
    }

    if (supportedExtensions.textureSRGBR8EXT)
    {
        if (!FormatReinterpretationSupported(optionalR8LinearFormats, renderer,
                                             kNonLinearColorspace))
        {
            return false;
        }
    }

    if (supportedExtensions.textureSRGBRG8EXT)
    {
        if (!FormatReinterpretationSupported(optionalRG8LinearFormats, renderer,
                                             kNonLinearColorspace))
        {
            return false;
        }
    }

    if (supportedExtensions.textureCompressionBptcEXT)
    {
        if (!FormatReinterpretationSupported(optionalBPTCLinearFormats, renderer,
                                             kNonLinearColorspace))
        {
            return false;
        }
    }

    return true;
}

bool CanSupportYuvInternalFormat(const Renderer *renderer)
{
    // The following formats are not mandatory in Vulkan, even when VK_KHR_sampler_ycbcr_conversion
    // is supported. GL_ANGLE_yuv_internal_format requires support for sampling only the
    // 8-bit 2-plane YUV format (VK_FORMAT_G8_B8R8_2PLANE_420_UNORM), if the ICD supports that we
    // can expose the extension.
    //
    // Various test cases need multiple YUV formats. It would be preferrable to have support for the
    // 3 plane 8 bit YUV format (VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM) as well.

    const Format &twoPlane8bitYuvFormat = renderer->getFormat(GL_G8_B8R8_2PLANE_420_UNORM_ANGLE);
    bool twoPlane8bitYuvFormatSupported = renderer->hasImageFormatFeatureBits(
        twoPlane8bitYuvFormat.getActualImageFormatID(vk::ImageAccess::SampleOnly),
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);

    const Format &threePlane8bitYuvFormat = renderer->getFormat(GL_G8_B8_R8_3PLANE_420_UNORM_ANGLE);
    bool threePlane8bitYuvFormatSupported = renderer->hasImageFormatFeatureBits(
        threePlane8bitYuvFormat.getActualImageFormatID(vk::ImageAccess::SampleOnly),
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);

    return twoPlane8bitYuvFormatSupported && threePlane8bitYuvFormatSupported;
}

uint32_t GetTimestampValidBits(const std::vector<VkQueueFamilyProperties> &queueFamilyProperties,
                               uint32_t queueFamilyIndex)
{
    ASSERT(!queueFamilyProperties.empty());

    if (queueFamilyIndex < queueFamilyProperties.size())
    {
        // If a queue family is already selected (which is only currently the case if there is only
        // one family), get the timestamp valid bits from that queue.
        return queueFamilyProperties[queueFamilyIndex].timestampValidBits;
    }

    // If a queue family is not already selected, we cannot know which queue family will end up
    // being used until a surface is used.  Take the minimum valid bits from all queues as a safe
    // measure.
    uint32_t timestampValidBits = queueFamilyProperties[0].timestampValidBits;
    for (const VkQueueFamilyProperties &properties : queueFamilyProperties)
    {
        timestampValidBits = std::min(timestampValidBits, properties.timestampValidBits);
    }
    return timestampValidBits;
}

bool CanSupportGPUShader5(const VkPhysicalDeviceFeatures &features)
{
    // We use the following Vulkan features to implement EXT_gpu_shader5 and OES_gpu_shader5:
    // - shaderImageGatherExtended: textureGatherOffset with non-constant offset and
    //   textureGatherOffsets family of functions.
    // - shaderSampledImageArrayDynamicIndexing and shaderUniformBufferArrayDynamicIndexing:
    //   dynamically uniform indices for samplers and uniform buffers.
    return features.shaderImageGatherExtended && features.shaderSampledImageArrayDynamicIndexing &&
           features.shaderUniformBufferArrayDynamicIndexing;
}

ANGLE_INLINE std::vector<bool> GetRequiredGLES32ExtensionList(
    const gl::Extensions &nativeExtensions)
{
    // From the GLES 3.2 spec: Almost all features of [ANDROID_extension_pack_es31a], incorporating
    // by reference all of the following features - with the exception of the sRGB decode features
    // of EXT_texture_sRGB_decode.

    // The extension debugKHR (also required for the Android extension pack) is a frontend feature
    // and is unconditionally enabled as a supported feature (in generateSupportedExtensions()).
    // Therefore, it is not included here.
    return {
        // From ANDROID_extension_pack_es31a
        nativeExtensions.textureCompressionAstcLdrKHR,
        nativeExtensions.blendEquationAdvancedKHR,
        nativeExtensions.sampleShadingOES,
        nativeExtensions.sampleVariablesOES,
        nativeExtensions.shaderImageAtomicOES,
        nativeExtensions.shaderMultisampleInterpolationOES,
        nativeExtensions.textureStencil8OES,
        nativeExtensions.textureStorageMultisample2dArrayOES,
        nativeExtensions.copyImageEXT,
        nativeExtensions.drawBuffersIndexedEXT,
        nativeExtensions.geometryShaderEXT,
        nativeExtensions.gpuShader5EXT,
        nativeExtensions.primitiveBoundingBoxEXT,
        nativeExtensions.shaderIoBlocksEXT,
        nativeExtensions.tessellationShaderEXT,
        nativeExtensions.textureBorderClampEXT,
        nativeExtensions.textureBufferEXT,
        nativeExtensions.textureCubeMapArrayEXT,

        // Other extensions
        nativeExtensions.drawElementsBaseVertexOES,
        nativeExtensions.colorBufferFloatEXT,
        nativeExtensions.robustnessKHR,
    };
}

void LogMissingExtensionsForGLES32(const gl::Extensions &nativeExtensions)
{
    if (!kEnableLogMissingExtensionsForGLES32)
    {
        return;
    }
    std::vector<bool> requiredExtensions = GetRequiredGLES32ExtensionList(nativeExtensions);

    constexpr const char *kRequiredExtensionNames[] = {
        // From ANDROID_extension_pack_es31a
        "textureCompressionAstcLdrKHR",
        "blendEquationAdvancedKHR",
        "sampleShadingOES",
        "sampleVariablesOES",
        "shaderImageAtomicOES",
        "shaderMultisampleInterpolationOES",
        "textureStencil8OES",
        "textureStorageMultisample2dArrayOES",
        "copyImageEXT",
        "drawBuffersIndexedEXT",
        "geometryShaderEXT",
        "gpuShader5EXT",
        "primitiveBoundingBoxEXT",
        "shaderIoBlocksEXT",
        "tessellationShaderEXT",
        "textureBorderClampEXT",
        "textureBufferEXT",
        "textureCubeMapArrayEXT",

        // Other extensions
        "drawElementsBaseVertexOES",
        "colorBufferFloatEXT",
        "robustnessKHR",
    };
    ASSERT(std::end(kRequiredExtensionNames) - std::begin(kRequiredExtensionNames) ==
           requiredExtensions.size());

    for (uint32_t index = 0; index < requiredExtensions.size(); index++)
    {
        if (!requiredExtensions[index])
        {
            INFO() << "The following extension is required for GLES 3.2: "
                   << kRequiredExtensionNames[index];
        }
    }
}

}  // namespace

void Renderer::ensureCapsInitialized() const
{
    if (mCapsInitialized)
    {
        return;
    }
    mCapsInitialized = true;

    const VkPhysicalDeviceLimits &limitsVk = mPhysicalDeviceProperties.limits;

    mNativeExtensions.setTextureExtensionSupport(mNativeTextureCaps);

    // Enable GL_EXT_buffer_storage
    mNativeExtensions.bufferStorageEXT = true;

    // When ETC2/EAC formats are natively supported, enable ANGLE-specific extension string to
    // expose them to WebGL. In other case, mark potentially-available ETC1 extension as emulated.
    if ((mPhysicalDeviceFeatures.textureCompressionETC2 == VK_TRUE) &&
        gl::DetermineCompressedTextureETCSupport(mNativeTextureCaps))
    {
        mNativeExtensions.compressedTextureEtcANGLE = true;
    }
    else
    {
        mNativeLimitations.emulatedEtc1 = true;
    }

    // When ASTC formats are not natively supported
    // mark potentially-available ASTC extension as emulated.
    if (mPhysicalDeviceFeatures.textureCompressionASTC_LDR == VK_FALSE)
    {
        mNativeLimitations.emulatedAstc = true;
    }

    // Vulkan doesn't support ASTC 3D block textures, which are required by
    // GL_OES_texture_compression_astc.
    mNativeExtensions.textureCompressionAstcOES = false;
    // Enable KHR_texture_compression_astc_sliced_3d
    mNativeExtensions.textureCompressionAstcSliced3dKHR =
        mNativeExtensions.textureCompressionAstcLdrKHR &&
        getFeatures().supportsAstcSliced3d.enabled;

    // Enable KHR_texture_compression_astc_hdr
    mNativeExtensions.textureCompressionAstcHdrKHR =
        mNativeExtensions.textureCompressionAstcLdrKHR &&
        getFeatures().supportsTextureCompressionAstcHdr.enabled;

    // Enable EXT_compressed_ETC1_RGB8_sub_texture
    mNativeExtensions.compressedETC1RGB8SubTextureEXT =
        mNativeExtensions.compressedETC1RGB8TextureOES;

    // Enable this for simple buffer readback testing, but some functionality is missing.
    // TODO(jmadill): Support full mapBufferRangeEXT extension.
    mNativeExtensions.mapbufferOES                = true;
    mNativeExtensions.mapBufferRangeEXT           = true;
    mNativeExtensions.textureStorageEXT           = true;
    mNativeExtensions.drawBuffersEXT              = true;
    mNativeExtensions.fragDepthEXT                = true;
    mNativeExtensions.conservativeDepthEXT        = true;
    mNativeExtensions.framebufferBlitANGLE        = true;
    mNativeExtensions.framebufferBlitNV           = true;
    mNativeExtensions.framebufferMultisampleANGLE = true;
    mNativeExtensions.textureMultisampleANGLE     = true;
    mNativeExtensions.multisampledRenderToTextureEXT =
        getFeatures().enableMultisampledRenderToTexture.enabled;
    mNativeExtensions.multisampledRenderToTexture2EXT =
        getFeatures().enableMultisampledRenderToTexture.enabled;
    mNativeExtensions.textureStorageMultisample2dArrayOES =
        (limitsVk.standardSampleLocations == VK_TRUE);
    mNativeExtensions.copyTextureCHROMIUM           = true;
    mNativeExtensions.copyTexture3dANGLE            = true;
    mNativeExtensions.copyCompressedTextureCHROMIUM = true;
    mNativeExtensions.debugMarkerEXT                = true;
    mNativeExtensions.robustnessEXT                 = true;
    mNativeExtensions.robustnessKHR                 = true;
    mNativeExtensions.translatedShaderSourceANGLE   = true;
    mNativeExtensions.discardFramebufferEXT         = true;
    mNativeExtensions.stencilTexturingANGLE         = true;
    mNativeExtensions.packReverseRowOrderANGLE      = true;
    mNativeExtensions.textureBorderClampOES = getFeatures().supportsCustomBorderColor.enabled;
    mNativeExtensions.textureBorderClampEXT = getFeatures().supportsCustomBorderColor.enabled;
    mNativeExtensions.polygonModeNV         = mPhysicalDeviceFeatures.fillModeNonSolid == VK_TRUE;
    mNativeExtensions.polygonModeANGLE      = mPhysicalDeviceFeatures.fillModeNonSolid == VK_TRUE;
    mNativeExtensions.polygonOffsetClampEXT = mPhysicalDeviceFeatures.depthBiasClamp == VK_TRUE;
    mNativeExtensions.depthClampEXT         = mPhysicalDeviceFeatures.depthClamp == VK_TRUE;
    // Enable EXT_texture_type_2_10_10_10_REV
    mNativeExtensions.textureType2101010REVEXT = true;

    // Enable EXT_texture_mirror_clamp_to_edge
    mNativeExtensions.textureMirrorClampToEdgeEXT =
        getFeatures().supportsSamplerMirrorClampToEdge.enabled;

    // Enable EXT_texture_shadow_lod
    mNativeExtensions.textureShadowLodEXT = true;

    // Enable EXT_multi_draw_indirect
    mNativeExtensions.multiDrawIndirectEXT = true;
    mNativeLimitations.multidrawEmulated   = false;

    // Enable EXT_base_instance
    mNativeExtensions.baseInstanceEXT = true;
    mNativeLimitations.baseInstanceEmulated = false;

    // Enable ANGLE_base_vertex_base_instance
    mNativeExtensions.baseVertexBaseInstanceANGLE              = true;
    mNativeExtensions.baseVertexBaseInstanceShaderBuiltinANGLE = true;

    // Enable OES/EXT_draw_elements_base_vertex
    mNativeExtensions.drawElementsBaseVertexOES = true;
    mNativeExtensions.drawElementsBaseVertexEXT = true;

    // Enable EXT_blend_minmax
    mNativeExtensions.blendMinmaxEXT = true;

    // Enable OES/EXT_draw_buffers_indexed
    mNativeExtensions.drawBuffersIndexedOES = mPhysicalDeviceFeatures.independentBlend == VK_TRUE;
    mNativeExtensions.drawBuffersIndexedEXT = mNativeExtensions.drawBuffersIndexedOES;

    mNativeExtensions.EGLImageOES                  = true;
    mNativeExtensions.EGLImageExternalOES          = true;
    mNativeExtensions.EGLImageExternalWrapModesEXT = true;
    mNativeExtensions.EGLImageExternalEssl3OES     = true;
    mNativeExtensions.EGLImageArrayEXT             = true;
    mNativeExtensions.EGLImageStorageEXT           = true;
    mNativeExtensions.memoryObjectEXT              = true;
    mNativeExtensions.memoryObjectFdEXT            = getFeatures().supportsExternalMemoryFd.enabled;
    mNativeExtensions.memoryObjectFlagsANGLE       = true;
    mNativeExtensions.memoryObjectFuchsiaANGLE =
        getFeatures().supportsExternalMemoryFuchsia.enabled;

    mNativeExtensions.semaphoreEXT   = true;
    mNativeExtensions.semaphoreFdEXT = getFeatures().supportsExternalSemaphoreFd.enabled;
    mNativeExtensions.semaphoreFuchsiaANGLE =
        getFeatures().supportsExternalSemaphoreFuchsia.enabled;

    mNativeExtensions.vertexHalfFloatOES = true;

    // Enabled in HW if VK_EXT_vertex_attribute_divisor available, otherwise emulated
    mNativeExtensions.instancedArraysANGLE = true;
    mNativeExtensions.instancedArraysEXT   = true;

    // Only expose robust buffer access if the physical device supports it.
    mNativeExtensions.robustBufferAccessBehaviorKHR =
        (mPhysicalDeviceFeatures.robustBufferAccess == VK_TRUE);

    mNativeExtensions.EGLSyncOES = true;

    mNativeExtensions.vertexType1010102OES = true;

    // Occlusion queries are natively supported in Vulkan.  ANGLE only issues this query inside a
    // render pass, so there is no dependency to `inheritedQueries`.
    mNativeExtensions.occlusionQueryBooleanEXT = true;

    // From the Vulkan specs:
    // > The number of valid bits in a timestamp value is determined by the
    // > VkQueueFamilyProperties::timestampValidBits property of the queue on which the timestamp is
    // > written. Timestamps are supported on any queue which reports a non-zero value for
    // > timestampValidBits via vkGetPhysicalDeviceQueueFamilyProperties.
    //
    // This query is applicable to render passes, but the `inheritedQueries` feature may not be
    // present.  The extension is not exposed in that case.
    // We use secondary command buffers almost everywhere and they require a feature to be
    // able to execute in the presence of queries.  As a result, we won't support timestamp queries
    // unless that feature is available.
    if (vk::OutsideRenderPassCommandBuffer::SupportsQueries(mPhysicalDeviceFeatures) &&
        vk::RenderPassCommandBuffer::SupportsQueries(mPhysicalDeviceFeatures))
    {
        const uint32_t timestampValidBits =
            vk::GetTimestampValidBits(mQueueFamilyProperties, mCurrentQueueFamilyIndex);

        mNativeExtensions.disjointTimerQueryEXT = timestampValidBits > 0;
        mNativeCaps.queryCounterBitsTimeElapsed = timestampValidBits;
        mNativeCaps.queryCounterBitsTimestamp   = timestampValidBits;
    }

    mNativeExtensions.textureFilterAnisotropicEXT =
        mPhysicalDeviceFeatures.samplerAnisotropy && limitsVk.maxSamplerAnisotropy > 1.0f;
    mNativeCaps.maxTextureAnisotropy =
        mNativeExtensions.textureFilterAnisotropicEXT ? limitsVk.maxSamplerAnisotropy : 0.0f;

    // Vulkan natively supports non power-of-two textures
    mNativeExtensions.textureNpotOES = true;

    mNativeExtensions.texture3DOES = true;

    // Vulkan natively supports standard derivatives
    mNativeExtensions.standardDerivativesOES = true;

    // Vulkan natively supports texture LOD
    mNativeExtensions.shaderTextureLodEXT = true;

    // Vulkan natively supports noperspective interpolation
    mNativeExtensions.shaderNoperspectiveInterpolationNV = true;

    // Vulkan natively supports 32-bit indices, entry in kIndexTypeMap
    mNativeExtensions.elementIndexUintOES = true;

    mNativeExtensions.fboRenderMipmapOES = true;

    // We support getting image data for Textures and Renderbuffers.
    mNativeExtensions.getImageANGLE = true;

    // Implemented in the translator
    mNativeExtensions.shaderNonConstantGlobalInitializersEXT = true;

    // Implemented in the front end. Enable SSO if not explicitly disabled.
    mNativeExtensions.separateShaderObjectsEXT =
        !getFeatures().disableSeparateShaderObjects.enabled;

    // Vulkan has no restrictions of the format of cubemaps, so if the proper formats are supported,
    // creating a cube of any of these formats should be implicitly supported.
    mNativeExtensions.depthTextureCubeMapOES =
        mNativeExtensions.depthTextureOES && mNativeExtensions.packedDepthStencilOES;

    // Vulkan natively supports format reinterpretation, but we still require support for all
    // formats we may reinterpret to
    mNativeExtensions.textureFormatSRGBOverrideEXT =
        vk::GetTextureSRGBOverrideSupport(this, mNativeExtensions);
    mNativeExtensions.textureSRGBDecodeEXT = vk::GetTextureSRGBDecodeSupport(this);

    // EXT_srgb_write_control requires image_format_list
    mNativeExtensions.sRGBWriteControlEXT = getFeatures().supportsImageFormatList.enabled;

    // Vulkan natively supports io interface block.
    mNativeExtensions.shaderIoBlocksOES = true;
    mNativeExtensions.shaderIoBlocksEXT = true;

    bool gpuShader5Support          = vk::CanSupportGPUShader5(mPhysicalDeviceFeatures);
    mNativeExtensions.gpuShader5EXT = gpuShader5Support;
    mNativeExtensions.gpuShader5OES = gpuShader5Support;

    // Only expose texture cubemap array if the physical device supports it.
    mNativeExtensions.textureCubeMapArrayOES = getFeatures().supportsImageCubeArray.enabled;
    mNativeExtensions.textureCubeMapArrayEXT = mNativeExtensions.textureCubeMapArrayOES;

    mNativeExtensions.shadowSamplersEXT = true;

    // Enable EXT_external_buffer on Android. External buffers are implemented using Android
    // hardware buffer (struct AHardwareBuffer).
    mNativeExtensions.externalBufferEXT = IsAndroid() && GetAndroidSDKVersion() >= 26;

    // From the Vulkan specs:
    // sampleRateShading specifies whether Sample Shading and multisample interpolation are
    // supported. If this feature is not enabled, the sampleShadingEnable member of the
    // VkPipelineMultisampleStateCreateInfo structure must be set to VK_FALSE and the
    // minSampleShading member is ignored. This also specifies whether shader modules can declare
    // the SampleRateShading capability
    bool supportSampleRateShading      = mPhysicalDeviceFeatures.sampleRateShading == VK_TRUE;
    mNativeExtensions.sampleShadingOES = supportSampleRateShading;

    // From the SPIR-V spec at 3.21. BuiltIn, SampleId and SamplePosition needs
    // SampleRateShading. https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html
    // To replace non-constant index to constant 0 index, this extension assumes that ANGLE only
    // supports the number of samples less than or equal to 32.
    constexpr unsigned int kNotSupportedSampleCounts = VK_SAMPLE_COUNT_64_BIT;
    mNativeExtensions.sampleVariablesOES =
        supportSampleRateShading && vk_gl::GetMaxSampleCount(kNotSupportedSampleCounts) == 0;

    // EXT_multisample_compatibility is necessary for GLES1 conformance so calls like
    // glDisable(GL_MULTISAMPLE) don't fail.  This is not actually implemented in Vulkan.  However,
    // no CTS tests actually test this extension.  GL_SAMPLE_ALPHA_TO_ONE requires the Vulkan
    // alphaToOne feature.
    mNativeExtensions.multisampleCompatibilityEXT =
        mPhysicalDeviceFeatures.alphaToOne ||
        mFeatures.exposeNonConformantExtensionsAndVersions.enabled;

    // GL_KHR_blend_equation_advanced.  According to the spec, only color attachment zero can be
    // used with advanced blend:
    //
    // > Advanced blending equations are supported only when rendering to a single
    // > color buffer using fragment color zero.
    //
    // Vulkan requires advancedBlendMaxColorAttachments to be at least one, so we can support
    // advanced blend as long as the Vulkan extension is supported.  Otherwise, the extension is
    // emulated where possible.
    // GL_EXT_blend_minmax is required for this extension, which is always enabled (hence omitted).
    mNativeExtensions.blendEquationAdvancedKHR = mFeatures.supportsBlendOperationAdvanced.enabled ||
                                                 mFeatures.emulateAdvancedBlendEquations.enabled;

    mNativeExtensions.blendEquationAdvancedCoherentKHR =
        mFeatures.supportsBlendOperationAdvancedCoherent.enabled ||
        (mFeatures.emulateAdvancedBlendEquations.enabled && mIsColorFramebufferFetchCoherent);

    // Enable EXT_unpack_subimage
    mNativeExtensions.unpackSubimageEXT = true;

    // Enable NV_pack_subimage
    mNativeExtensions.packSubimageNV = true;

    mNativeCaps.minInterpolationOffset          = limitsVk.minInterpolationOffset;
    mNativeCaps.maxInterpolationOffset          = limitsVk.maxInterpolationOffset;
    mNativeCaps.subPixelInterpolationOffsetBits = limitsVk.subPixelInterpolationOffsetBits;

    // Enable GL_ANGLE_robust_fragment_shader_output
    mNativeExtensions.robustFragmentShaderOutputANGLE = true;

    // From the Vulkan spec:
    //
    // > The values minInterpolationOffset and maxInterpolationOffset describe the closed interval
    // > of supported interpolation offsets : [ minInterpolationOffset, maxInterpolationOffset ].
    // > The ULP is determined by subPixelInterpolationOffsetBits. If
    // > subPixelInterpolationOffsetBits is 4, this provides increments of(1 / 2^4) = 0.0625, and
    // > thus the range of supported interpolation offsets would be[-0.5, 0.4375]
    //
    // OES_shader_multisample_interpolation requires a maximum value of -0.5 for
    // MIN_FRAGMENT_INTERPOLATION_OFFSET_OES and minimum 0.5 for
    // MAX_FRAGMENT_INTERPOLATION_OFFSET_OES.  Vulkan has an identical limit for
    // minInterpolationOffset, but its limit for maxInterpolationOffset is 0.5-(1/ULP).
    // OES_shader_multisample_interpolation is therefore only supported if
    // maxInterpolationOffset is at least 0.5.
    //
    // The GL spec is not as precise as Vulkan's in this regard and that the requirements really
    // meant to match.  This is rectified in the GL spec.
    // https://gitlab.khronos.org/opengl/API/-/issues/149
    mNativeExtensions.shaderMultisampleInterpolationOES = mNativeExtensions.sampleVariablesOES;

    // Always enable ANGLE_rgbx_internal_format to expose GL_RGBX8_ANGLE except for Samsung.
    mNativeExtensions.rgbxInternalFormatANGLE = mFeatures.supportsAngleRgbxInternalFormat.enabled;

    // https://vulkan.lunarg.com/doc/view/1.0.30.0/linux/vkspec.chunked/ch31s02.html
    mNativeCaps.maxElementIndex  = std::numeric_limits<GLuint>::max() - 1;
    mNativeCaps.max3DTextureSize = rx::LimitToInt(limitsVk.maxImageDimension3D);
    mNativeCaps.max2DTextureSize =
        std::min(limitsVk.maxFramebufferWidth, limitsVk.maxImageDimension2D);
    mNativeCaps.maxArrayTextureLayers = rx::LimitToInt(limitsVk.maxImageArrayLayers);
    mNativeCaps.maxLODBias            = limitsVk.maxSamplerLodBias;
    mNativeCaps.maxCubeMapTextureSize = rx::LimitToInt(limitsVk.maxImageDimensionCube);
    mNativeCaps.maxRenderbufferSize =
        std::min({limitsVk.maxImageDimension2D, limitsVk.maxFramebufferWidth,
                  limitsVk.maxFramebufferHeight});
    mNativeCaps.minAliasedPointSize = std::max(1.0f, limitsVk.pointSizeRange[0]);
    mNativeCaps.maxAliasedPointSize = limitsVk.pointSizeRange[1];

    // Line width ranges and granularity
    if (mPhysicalDeviceFeatures.wideLines && mFeatures.bresenhamLineRasterization.enabled)
    {
        mNativeCaps.minAliasedLineWidth = std::max(1.0f, limitsVk.lineWidthRange[0]);
        mNativeCaps.maxAliasedLineWidth = limitsVk.lineWidthRange[1];
    }
    else
    {
        mNativeCaps.minAliasedLineWidth = 1.0f;
        mNativeCaps.maxAliasedLineWidth = 1.0f;
    }
    mNativeCaps.minMultisampleLineWidth = mNativeCaps.minAliasedLineWidth;
    mNativeCaps.maxMultisampleLineWidth = mNativeCaps.maxAliasedLineWidth;
    mNativeCaps.lineWidthGranularity    = limitsVk.lineWidthGranularity;

    mNativeCaps.maxDrawBuffers =
        std::min(limitsVk.maxColorAttachments, limitsVk.maxFragmentOutputAttachments);
    mNativeCaps.maxFramebufferWidth  = rx::LimitToInt(limitsVk.maxFramebufferWidth);
    mNativeCaps.maxFramebufferHeight = rx::LimitToInt(limitsVk.maxFramebufferHeight);
    mNativeCaps.maxColorAttachments  = rx::LimitToInt(limitsVk.maxColorAttachments);
    mNativeCaps.maxViewportWidth     = rx::LimitToInt(limitsVk.maxViewportDimensions[0]);
    mNativeCaps.maxViewportHeight    = rx::LimitToInt(limitsVk.maxViewportDimensions[1]);
    mNativeCaps.maxSampleMaskWords   = rx::LimitToInt(limitsVk.maxSampleMaskWords);
    mNativeCaps.maxColorTextureSamples =
        vk_gl::GetMaxSampleCount(limitsVk.sampledImageColorSampleCounts);
    mNativeCaps.maxDepthTextureSamples =
        vk_gl::GetMaxSampleCount(limitsVk.sampledImageDepthSampleCounts);
    mNativeCaps.maxIntegerSamples =
        vk_gl::GetMaxSampleCount(limitsVk.sampledImageIntegerSampleCounts);

    mNativeCaps.maxVertexAttributes     = rx::LimitToInt(limitsVk.maxVertexInputAttributes);
    mNativeCaps.maxVertexAttribBindings = rx::LimitToInt(limitsVk.maxVertexInputBindings);
    // Offset and stride are stored as uint16_t in PackedAttribDesc.
    mNativeCaps.maxVertexAttribRelativeOffset =
        std::min((1u << kAttributeOffsetMaxBits) - 1, limitsVk.maxVertexInputAttributeOffset);
    mNativeCaps.maxVertexAttribStride =
        std::min(static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()),
                 limitsVk.maxVertexInputBindingStride);

    mNativeCaps.maxElementsIndices  = std::numeric_limits<GLint>::max();
    mNativeCaps.maxElementsVertices = std::numeric_limits<GLint>::max();

    // Looks like all floats are IEEE according to the docs here:
    // https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/html/vkspec.html#spirvenv-precision-operation
    mNativeCaps.vertexHighpFloat.setIEEEFloat();
    mNativeCaps.vertexMediumpFloat.setIEEEHalfFloat();
    mNativeCaps.vertexLowpFloat.setIEEEHalfFloat();
    mNativeCaps.fragmentHighpFloat.setIEEEFloat();
    mNativeCaps.fragmentMediumpFloat.setIEEEHalfFloat();
    mNativeCaps.fragmentLowpFloat.setIEEEHalfFloat();

    // Vulkan doesn't provide such information.  We provide the spec-required minimum here.
    mNativeCaps.vertexHighpInt.setTwosComplementInt(32);
    mNativeCaps.vertexMediumpInt.setTwosComplementInt(16);
    mNativeCaps.vertexLowpInt.setTwosComplementInt(16);
    mNativeCaps.fragmentHighpInt.setTwosComplementInt(32);
    mNativeCaps.fragmentMediumpInt.setTwosComplementInt(16);
    mNativeCaps.fragmentLowpInt.setTwosComplementInt(16);

    // Compute shader limits.
    mNativeCaps.maxComputeWorkGroupCount[0] = rx::LimitToInt(limitsVk.maxComputeWorkGroupCount[0]);
    mNativeCaps.maxComputeWorkGroupCount[1] = rx::LimitToInt(limitsVk.maxComputeWorkGroupCount[1]);
    mNativeCaps.maxComputeWorkGroupCount[2] = rx::LimitToInt(limitsVk.maxComputeWorkGroupCount[2]);
    mNativeCaps.maxComputeWorkGroupSize[0]  = rx::LimitToInt(limitsVk.maxComputeWorkGroupSize[0]);
    mNativeCaps.maxComputeWorkGroupSize[1]  = rx::LimitToInt(limitsVk.maxComputeWorkGroupSize[1]);
    mNativeCaps.maxComputeWorkGroupSize[2]  = rx::LimitToInt(limitsVk.maxComputeWorkGroupSize[2]);
    mNativeCaps.maxComputeWorkGroupInvocations =
        rx::LimitToInt(limitsVk.maxComputeWorkGroupInvocations);
    mNativeCaps.maxComputeSharedMemorySize = rx::LimitToInt(limitsVk.maxComputeSharedMemorySize);

    GLuint maxUniformBlockSize = limitsVk.maxUniformBufferRange;

    // Clamp the maxUniformBlockSize to 64KB (majority of devices support up to this size
    // currently), on AMD the maxUniformBufferRange is near uint32_t max.
    maxUniformBlockSize = std::min(0x10000u, maxUniformBlockSize);

    const GLuint maxUniformVectors = maxUniformBlockSize / (sizeof(GLfloat) * kComponentsPerVector);
    const GLuint maxUniformComponents = maxUniformVectors * kComponentsPerVector;

    // Uniforms are implemented using a uniform buffer, so the max number of uniforms we can
    // support is the max buffer range divided by the size of a single uniform (4X float).
    mNativeCaps.maxVertexUniformVectors   = maxUniformVectors;
    mNativeCaps.maxFragmentUniformVectors = maxUniformVectors;
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        mNativeCaps.maxShaderUniformComponents[shaderType] = maxUniformComponents;
    }
    mNativeCaps.maxUniformLocations = maxUniformVectors;

    const int32_t maxPerStageUniformBuffers = rx::LimitToInt(
        limitsVk.maxPerStageDescriptorUniformBuffers - kReservedPerStageDefaultUniformBindingCount);
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        mNativeCaps.maxShaderUniformBlocks[shaderType] = maxPerStageUniformBuffers;
    }

    // Reserved uniform buffer count depends on number of stages.  Vertex and fragment shaders are
    // always supported.  The limit needs to be adjusted based on whether geometry and tessellation
    // is supported.
    int32_t maxCombinedUniformBuffers = rx::LimitToInt(limitsVk.maxDescriptorSetUniformBuffers) -
                                        2 * kReservedPerStageDefaultUniformBindingCount;

    mNativeCaps.maxUniformBlockSize = maxUniformBlockSize;
    mNativeCaps.uniformBufferOffsetAlignment =
        static_cast<GLint>(limitsVk.minUniformBufferOffsetAlignment);

    // Note that Vulkan currently implements textures as combined image+samplers, so the limit is
    // the minimum of supported samplers and sampled images.
    const uint32_t maxPerStageTextures = std::min(limitsVk.maxPerStageDescriptorSamplers,
                                                  limitsVk.maxPerStageDescriptorSampledImages);
    const uint32_t maxCombinedTextures =
        std::min(limitsVk.maxDescriptorSetSamplers, limitsVk.maxDescriptorSetSampledImages);
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        mNativeCaps.maxShaderTextureImageUnits[shaderType] = rx::LimitToInt(maxPerStageTextures);
    }
    mNativeCaps.maxCombinedTextureImageUnits = rx::LimitToInt(maxCombinedTextures);

    uint32_t maxPerStageStorageBuffers    = limitsVk.maxPerStageDescriptorStorageBuffers;
    uint32_t maxVertexStageStorageBuffers = maxPerStageStorageBuffers;
    uint32_t maxCombinedStorageBuffers    = limitsVk.maxDescriptorSetStorageBuffers;

    // A number of storage buffer slots are used in the vertex shader to emulate transform feedback.
    // Note that Vulkan requires maxPerStageDescriptorStorageBuffers to be at least 4 (i.e. the same
    // as gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS).
    // TODO(syoussefi): This should be conditioned to transform feedback extension not being
    // present.  http://anglebug.com/42261882.
    // TODO(syoussefi): If geometry shader is supported, emulation will be done at that stage, and
    // so the reserved storage buffers should be accounted in that stage.
    // http://anglebug.com/42262271
    static_assert(
        gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS == 4,
        "Limit to ES2.0 if supported SSBO count < supporting transform feedback buffer count");
    if (mPhysicalDeviceFeatures.vertexPipelineStoresAndAtomics)
    {
        ASSERT(maxVertexStageStorageBuffers >= gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS);
        maxVertexStageStorageBuffers -= gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS;
        maxCombinedStorageBuffers -= gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS;

        // Cap the per-stage limit of the other stages to the combined limit, in case the combined
        // limit is now lower than that.
        maxPerStageStorageBuffers = std::min(maxPerStageStorageBuffers, maxCombinedStorageBuffers);
    }

    // Reserve up to IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFERS storage buffers in the fragment and
    // compute stages for atomic counters.  This is only possible if the number of per-stage storage
    // buffers is greater than 4, which is the required GLES minimum for compute.
    //
    // For each stage, we'll either not support atomic counter buffers, or support exactly
    // IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFERS.  This is due to restrictions in the shader
    // translator where we can't know how many atomic counter buffers we would really need after
    // linking so we can't create a packed buffer array.
    //
    // For the vertex stage, we could support atomic counters without storage buffers, but that's
    // likely not very useful, so we use the same limit (4 + MAX_ATOMIC_COUNTER_BUFFERS) for the
    // vertex stage to determine if we would want to add support for atomic counter buffers.
    constexpr uint32_t kMinimumStorageBuffersForAtomicCounterBufferSupport =
        gl::limits::kMinimumComputeStorageBuffers +
        gl::IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS;
    uint32_t maxVertexStageAtomicCounterBuffers = 0;
    uint32_t maxPerStageAtomicCounterBuffers    = 0;
    uint32_t maxCombinedAtomicCounterBuffers    = 0;

    if (maxPerStageStorageBuffers >= kMinimumStorageBuffersForAtomicCounterBufferSupport)
    {
        maxPerStageAtomicCounterBuffers = gl::IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS;
        maxCombinedAtomicCounterBuffers = gl::IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS;
    }

    if (maxVertexStageStorageBuffers >= kMinimumStorageBuffersForAtomicCounterBufferSupport)
    {
        maxVertexStageAtomicCounterBuffers = gl::IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS;
    }

    maxVertexStageStorageBuffers -= maxVertexStageAtomicCounterBuffers;
    maxPerStageStorageBuffers -= maxPerStageAtomicCounterBuffers;
    maxCombinedStorageBuffers -= maxCombinedAtomicCounterBuffers;

    mNativeCaps.maxShaderStorageBlocks[gl::ShaderType::Vertex] =
        mPhysicalDeviceFeatures.vertexPipelineStoresAndAtomics
            ? rx::LimitToInt(maxVertexStageStorageBuffers)
            : 0;
    mNativeCaps.maxShaderStorageBlocks[gl::ShaderType::Fragment] =
        mPhysicalDeviceFeatures.fragmentStoresAndAtomics ? rx::LimitToInt(maxPerStageStorageBuffers)
                                                         : 0;
    mNativeCaps.maxShaderStorageBlocks[gl::ShaderType::Compute] =
        rx::LimitToInt(maxPerStageStorageBuffers);
    mNativeCaps.maxCombinedShaderStorageBlocks = rx::LimitToInt(maxCombinedStorageBuffers);

    // Emulated as storage buffers, atomic counter buffers have the same size limit.  However, the
    // limit is a signed integer and values above int max will end up as a negative size.  The
    // storage buffer size is just capped to int unconditionally.
    uint32_t maxStorageBufferRange = rx::LimitToInt(limitsVk.maxStorageBufferRange);
    if (mFeatures.limitMaxStorageBufferSize.enabled)
    {
        constexpr uint32_t kStorageBufferLimit = 256 * 1024 * 1024;
        maxStorageBufferRange = std::min(maxStorageBufferRange, kStorageBufferLimit);
    }

    mNativeCaps.maxShaderStorageBufferBindings = rx::LimitToInt(maxCombinedStorageBuffers);
    mNativeCaps.maxShaderStorageBlockSize      = maxStorageBufferRange;
    mNativeCaps.shaderStorageBufferOffsetAlignment =
        rx::LimitToInt(static_cast<uint32_t>(limitsVk.minStorageBufferOffsetAlignment));

    mNativeCaps.maxShaderAtomicCounterBuffers[gl::ShaderType::Vertex] =
        mPhysicalDeviceFeatures.vertexPipelineStoresAndAtomics
            ? rx::LimitToInt(maxVertexStageAtomicCounterBuffers)
            : 0;
    mNativeCaps.maxShaderAtomicCounterBuffers[gl::ShaderType::Fragment] =
        mPhysicalDeviceFeatures.fragmentStoresAndAtomics
            ? rx::LimitToInt(maxPerStageAtomicCounterBuffers)
            : 0;
    mNativeCaps.maxShaderAtomicCounterBuffers[gl::ShaderType::Compute] =
        rx::LimitToInt(maxPerStageAtomicCounterBuffers);
    mNativeCaps.maxCombinedAtomicCounterBuffers = rx::LimitToInt(maxCombinedAtomicCounterBuffers);

    mNativeCaps.maxAtomicCounterBufferBindings = rx::LimitToInt(maxCombinedAtomicCounterBuffers);
    mNativeCaps.maxAtomicCounterBufferSize     = maxStorageBufferRange;

    // There is no particular limit to how many atomic counters there can be, other than the size of
    // a storage buffer.  We nevertheless limit this to something reasonable (4096 arbitrarily).
    const int32_t maxAtomicCounters =
        std::min<int32_t>(4096, maxStorageBufferRange / sizeof(uint32_t));
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        mNativeCaps.maxShaderAtomicCounters[shaderType] = maxAtomicCounters;
    }

    // Set maxShaderAtomicCounters to zero if atomic is not supported.
    if (!mPhysicalDeviceFeatures.vertexPipelineStoresAndAtomics)
    {
        mNativeCaps.maxShaderAtomicCounters[gl::ShaderType::Vertex]         = 0;
        mNativeCaps.maxShaderAtomicCounters[gl::ShaderType::Geometry]       = 0;
        mNativeCaps.maxShaderAtomicCounters[gl::ShaderType::TessControl]    = 0;
        mNativeCaps.maxShaderAtomicCounters[gl::ShaderType::TessEvaluation] = 0;
    }
    if (!mPhysicalDeviceFeatures.fragmentStoresAndAtomics)
    {
        mNativeCaps.maxShaderAtomicCounters[gl::ShaderType::Fragment] = 0;
    }

    mNativeCaps.maxCombinedAtomicCounters = maxAtomicCounters;

    // GL Images correspond to Vulkan Storage Images.
    const int32_t maxPerStageImages = rx::LimitToInt(limitsVk.maxPerStageDescriptorStorageImages);
    const int32_t maxCombinedImages = rx::LimitToInt(limitsVk.maxDescriptorSetStorageImages);
    const int32_t maxVertexPipelineImages =
        mPhysicalDeviceFeatures.vertexPipelineStoresAndAtomics ? maxPerStageImages : 0;

    mNativeCaps.maxShaderImageUniforms[gl::ShaderType::Vertex]         = maxVertexPipelineImages;
    mNativeCaps.maxShaderImageUniforms[gl::ShaderType::TessControl]    = maxVertexPipelineImages;
    mNativeCaps.maxShaderImageUniforms[gl::ShaderType::TessEvaluation] = maxVertexPipelineImages;
    mNativeCaps.maxShaderImageUniforms[gl::ShaderType::Geometry]       = maxVertexPipelineImages;
    mNativeCaps.maxShaderImageUniforms[gl::ShaderType::Fragment] =
        mPhysicalDeviceFeatures.fragmentStoresAndAtomics ? maxPerStageImages : 0;
    mNativeCaps.maxShaderImageUniforms[gl::ShaderType::Compute] = maxPerStageImages;

    mNativeCaps.maxCombinedImageUniforms = maxCombinedImages;
    mNativeCaps.maxImageUnits            = maxCombinedImages;

    mNativeCaps.minProgramTexelOffset         = limitsVk.minTexelOffset;
    mNativeCaps.maxProgramTexelOffset         = limitsVk.maxTexelOffset;
    mNativeCaps.minProgramTextureGatherOffset = limitsVk.minTexelGatherOffset;
    mNativeCaps.maxProgramTextureGatherOffset = limitsVk.maxTexelGatherOffset;

    // There is no additional limit to the combined number of components.  We can have up to a
    // maximum number of uniform buffers, each having the maximum number of components.  Note that
    // this limit includes both components in and out of uniform buffers.
    //
    // This value is limited to INT_MAX to avoid overflow when queried from glGetIntegerv().
    const uint64_t maxCombinedUniformComponents =
        std::min<uint64_t>(static_cast<uint64_t>(maxPerStageUniformBuffers +
                                                 kReservedPerStageDefaultUniformBindingCount) *
                               maxUniformComponents,
                           std::numeric_limits<GLint>::max());
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        mNativeCaps.maxCombinedShaderUniformComponents[shaderType] = maxCombinedUniformComponents;
    }

    // Total number of resources available to the user are as many as Vulkan allows minus everything
    // that ANGLE uses internally.  That is, one dynamic uniform buffer used per stage for default
    // uniforms.  Additionally, Vulkan uses up to IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS + 1
    // buffers for transform feedback (Note: +1 is for the "counter" buffer of
    // VK_EXT_transform_feedback).
    constexpr uint32_t kReservedPerStageUniformBufferCount = 1;
    constexpr uint32_t kReservedPerStageBindingCount =
        kReservedPerStageUniformBufferCount + gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS + 1;

    // Note: maxPerStageResources is required to be at least the sum of per stage UBOs, SSBOs etc
    // which total a minimum of 44 resources, so no underflow is possible here.  Limit the total
    // number of resources reported by Vulkan to 2 billion though to avoid seeing negative numbers
    // in applications that take the value as signed int (including dEQP).
    const uint32_t maxPerStageResources = limitsVk.maxPerStageResources;
    mNativeCaps.maxCombinedShaderOutputResources =
        rx::LimitToInt(maxPerStageResources - kReservedPerStageBindingCount);

    // Reserve 1 extra varying for transform feedback capture of gl_Position.
    constexpr GLint kReservedVaryingComponentsForTransformFeedbackExtension = 4;

    GLint reservedVaryingComponentCount = 0;

    if (getFeatures().supportsTransformFeedbackExtension.enabled)
    {
        reservedVaryingComponentCount += kReservedVaryingComponentsForTransformFeedbackExtension;
    }

    // The max varying vectors should not include gl_Position.
    // The gles2.0 section 2.10 states that "gl_Position is not a varying variable and does
    // not count against this limit.", but the Vulkan spec has no such mention in its Built-in
    // vars section. It is implicit that we need to actually reserve it for Vulkan in that case.
    //
    // Note that this exception for gl_Position does not apply to MAX_VERTEX_OUTPUT_COMPONENTS and
    // similar limits.
    //
    // Note also that the reserved components are for transform feedback capture only, so they don't
    // apply to the _input_ component limit.
    const GLint reservedVaryingVectorCount = reservedVaryingComponentCount / 4 + 1;

    const GLint maxVaryingCount =
        std::min(limitsVk.maxVertexOutputComponents, limitsVk.maxFragmentInputComponents);
    mNativeCaps.maxVaryingVectors =
        rx::LimitToInt((maxVaryingCount / kComponentsPerVector) - reservedVaryingVectorCount);
    mNativeCaps.maxVertexOutputComponents =
        rx::LimitToInt(limitsVk.maxVertexOutputComponents) - reservedVaryingComponentCount;
    mNativeCaps.maxFragmentInputComponents = rx::LimitToInt(limitsVk.maxFragmentInputComponents);

    mNativeCaps.maxTransformFeedbackInterleavedComponents =
        gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS;
    mNativeCaps.maxTransformFeedbackSeparateAttributes =
        gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS;
    mNativeCaps.maxTransformFeedbackSeparateComponents =
        gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS;

    mNativeCaps.minProgramTexelOffset = limitsVk.minTexelOffset;
    mNativeCaps.maxProgramTexelOffset = rx::LimitToInt(limitsVk.maxTexelOffset);

    const uint32_t sampleCounts =
        limitsVk.framebufferColorSampleCounts & limitsVk.framebufferDepthSampleCounts &
        limitsVk.framebufferStencilSampleCounts & vk_gl::kSupportedSampleCounts;

    mNativeCaps.maxSamples            = rx::LimitToInt(vk_gl::GetMaxSampleCount(sampleCounts));
    mNativeCaps.maxFramebufferSamples = mNativeCaps.maxSamples;

    mNativeCaps.subPixelBits = limitsVk.subPixelPrecisionBits;

    if (getFeatures().supportsShaderFramebufferFetch.enabled)
    {
        mNativeExtensions.shaderFramebufferFetchEXT = true;
        mNativeExtensions.shaderFramebufferFetchARM = true;
        // ANGLE correctly maps gl_LastFragColorARM to input attachment 0 and has no problem with
        // MRT.
        mNativeCaps.fragmentShaderFramebufferFetchMRT = true;
    }

    if (getFeatures().supportsShaderFramebufferFetchNonCoherent.enabled)
    {
        mNativeExtensions.shaderFramebufferFetchNonCoherentEXT = true;
    }

    // Enable Program Binary extension.
    mNativeExtensions.getProgramBinaryOES = true;
    mNativeCaps.programBinaryFormats.push_back(GL_PROGRAM_BINARY_ANGLE);

    // Enable Shader Binary extension.
    mNativeCaps.shaderBinaryFormats.push_back(GL_SHADER_BINARY_ANGLE);

    // Enable GL_NV_pixel_buffer_object extension.
    mNativeExtensions.pixelBufferObjectNV = true;

    // Enable GL_NV_fence extension.
    mNativeExtensions.fenceNV = true;

    // Enable GL_EXT_copy_image
    mNativeExtensions.copyImageEXT = true;
    mNativeExtensions.copyImageOES = true;

    // GL_EXT_clip_control
    mNativeExtensions.clipControlEXT = true;

    // GL_ANGLE_read_only_depth_stencil_feedback_loops
    mNativeExtensions.readOnlyDepthStencilFeedbackLoopsANGLE = true;

    // Enable GL_EXT_texture_buffer and OES variant.  Nearly all formats required for this extension
    // are also required to have the UNIFORM_TEXEL_BUFFER feature bit in Vulkan, except for
    // R32G32B32_SFLOAT/UINT/SINT which are optional.  For many formats, the STORAGE_TEXEL_BUFFER
    // feature is optional though.  This extension is exposed only if the formats specified in
    // EXT_texture_buffer support the necessary feature bits.
    //
    //  glTexBuffer page 187 table 8.18.
    //  glBindImageTexture page 216 table 8.24.
    //  https://www.khronos.org/registry/OpenGL/specs/es/3.2/es_spec_3.2.pdf.
    //  https://www.khronos.org/registry/vulkan/specs/1.0-extensions/html/chap43.html#features-required-format-support
    //  required image and texture access for texture buffer formats are
    //                         texture access                image access
    //    8-bit components, all required by vulkan.
    //
    //    GL_R8                        Y                           N
    //    GL_R8I                       Y                           N
    //    GL_R8UI                      Y                           N
    //    GL_RG8                       Y                           N
    //    GL_RG8I                      Y                           N
    //    GL_RG8UI                     Y                           N
    //    GL_RGBA8                     Y                           Y
    //    GL_RGBA8I                    Y                           Y
    //    GL_RGBA8UI                   Y                           Y
    //    GL_RGBA8_SNORM               N                           Y
    //
    //    16-bit components,  all required by vulkan.
    //
    //    GL_R16F                      Y                           N
    //    GL_R16I                      Y                           N
    //    GL_R16UI                     Y                           N
    //    GL_RG16F                     Y                           N
    //    GL_RG16I                     Y                           N
    //    GL_RG16UI                    Y                           N
    //    GL_RGBA16F                   Y                           Y
    //    GL_RGBA16I                   Y                           Y
    //    GL_RGBA16UI                  Y                           Y
    //
    //    32-bit components, except RGB32 all others required by vulkan.
    //                       RGB32 is emulated by ANGLE
    //
    //    GL_R32F                      Y                           Y
    //    GL_R32I                      Y                           Y
    //    GL_R32UI                     Y                           Y
    //    GL_RG32F                     Y                           N
    //    GL_RG32I                     Y                           N
    //    GL_RG32UI                    Y                           N
    //    GL_RGB32F                    Y                           N
    //    GL_RGB32I                    Y                           N
    //    GL_RGB32UI                   Y                           N
    //    GL_RGBA32F                   Y                           Y
    //    GL_RGBA32I                   Y                           Y
    //    GL_RGBA32UI                  Y                           Y
    mNativeExtensions.textureBufferOES = true;
    mNativeExtensions.textureBufferEXT = true;
    mNativeCaps.maxTextureBufferSize   = rx::LimitToInt(limitsVk.maxTexelBufferElements);
    mNativeCaps.textureBufferOffsetAlignment =
        rx::LimitToInt(limitsVk.minTexelBufferOffsetAlignment);

    // From the GL_EXT_texture_norm16 spec: Accepted by the <internalFormat> parameter of
    // TexImage2D,TexImage3D, TexStorage2D, TexStorage3D and TexStorage2DMultisample,
    // TexStorage3DMultisampleOES, TexBufferEXT, TexBufferRangeEXT, TextureViewEXT,
    // RenderbufferStorage and RenderbufferStorageMultisample:
    //   - R16_EXT
    //   - RG16_EXT
    //   - RGBA16_EXT
    bool norm16FormatsSupportedForBufferTexture =
        hasBufferFormatFeatureBits(angle::FormatID::R16_UNORM,
                                   VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT) &&
        hasBufferFormatFeatureBits(angle::FormatID::R16G16_UNORM,
                                   VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT) &&
        hasBufferFormatFeatureBits(angle::FormatID::R16G16B16A16_UNORM,
                                   VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT);

    if (!norm16FormatsSupportedForBufferTexture)
    {
        mNativeExtensions.textureNorm16EXT = false;

        // With textureNorm16EXT disabled, renderSnormEXT will skip checking support for the 16-bit
        // normalized formats.
        mNativeExtensions.renderSnormEXT =
            DetermineRenderSnormSupport(mNativeTextureCaps, mNativeExtensions.textureNorm16EXT);
    }

    // Atomic image operations in the vertex and fragment shaders require the
    // vertexPipelineStoresAndAtomics and fragmentStoresAndAtomics Vulkan features respectively.
    // If either of these features is not present, the number of image uniforms for that stage is
    // advertised as zero, so image atomic operations support can be agnostic of shader stages.
    //
    // GL_OES_shader_image_atomic requires that image atomic functions have support for r32i and
    // r32ui formats.  These formats have mandatory support for STORAGE_IMAGE_ATOMIC and
    // STORAGE_TEXEL_BUFFER_ATOMIC features in Vulkan.  Additionally, it requires that
    // imageAtomicExchange supports r32f, which is emulated in ANGLE transforming the shader to
    // expect r32ui instead.
    mNativeExtensions.shaderImageAtomicOES = true;

    // Tessellation shaders are required for ES 3.2.
    if (mPhysicalDeviceFeatures.tessellationShader)
    {
        constexpr uint32_t kReservedTessellationDefaultUniformBindingCount = 2;

        bool tessellationShaderEnabled =
            mFeatures.supportsTransformFeedbackExtension.enabled &&
            (mFeatures.supportsPrimitivesGeneratedQuery.enabled ||
             mFeatures.exposeNonConformantExtensionsAndVersions.enabled);
        mNativeExtensions.tessellationShaderEXT = tessellationShaderEnabled;
        mNativeExtensions.tessellationShaderOES = tessellationShaderEnabled;
        mNativeCaps.maxPatchVertices            = rx::LimitToInt(limitsVk.maxTessellationPatchSize);
        mNativeCaps.maxTessPatchComponents =
            rx::LimitToInt(limitsVk.maxTessellationControlPerPatchOutputComponents);
        mNativeCaps.maxTessGenLevel = rx::LimitToInt(limitsVk.maxTessellationGenerationLevel);

        mNativeCaps.maxTessControlInputComponents =
            rx::LimitToInt(limitsVk.maxTessellationControlPerVertexInputComponents);
        mNativeCaps.maxTessControlOutputComponents =
            rx::LimitToInt(limitsVk.maxTessellationControlPerVertexOutputComponents);
        mNativeCaps.maxTessControlTotalOutputComponents =
            rx::LimitToInt(limitsVk.maxTessellationControlTotalOutputComponents);
        mNativeCaps.maxTessEvaluationInputComponents =
            rx::LimitToInt(limitsVk.maxTessellationEvaluationInputComponents);
        mNativeCaps.maxTessEvaluationOutputComponents =
            rx::LimitToInt(limitsVk.maxTessellationEvaluationOutputComponents) -
            reservedVaryingComponentCount;

        // There is 1 default uniform binding used per tessellation stages.
        mNativeCaps.maxCombinedUniformBlocks = rx::LimitToInt(
            mNativeCaps.maxCombinedUniformBlocks + kReservedTessellationDefaultUniformBindingCount);
        mNativeCaps.maxUniformBufferBindings = rx::LimitToInt(
            mNativeCaps.maxUniformBufferBindings + kReservedTessellationDefaultUniformBindingCount);

        if (mPhysicalDeviceFeatures.vertexPipelineStoresAndAtomics)
        {
            mNativeCaps.maxShaderStorageBlocks[gl::ShaderType::TessControl] =
                mNativeCaps.maxCombinedShaderOutputResources;
            mNativeCaps.maxShaderAtomicCounterBuffers[gl::ShaderType::TessControl] =
                maxCombinedAtomicCounterBuffers;

            mNativeCaps.maxShaderStorageBlocks[gl::ShaderType::TessEvaluation] =
                mNativeCaps.maxCombinedShaderOutputResources;
            mNativeCaps.maxShaderAtomicCounterBuffers[gl::ShaderType::TessEvaluation] =
                maxCombinedAtomicCounterBuffers;
        }

        mNativeCaps.primitiveRestartForPatchesSupported =
            mPrimitiveTopologyListRestartFeatures.primitiveTopologyPatchListRestart == VK_TRUE;

        // Reserve a uniform buffer binding for each tessellation stage
        if (tessellationShaderEnabled)
        {
            maxCombinedUniformBuffers -= 2 * kReservedPerStageDefaultUniformBindingCount;
        }
    }

    // Geometry shaders are required for ES 3.2.
    if (mPhysicalDeviceFeatures.geometryShader)
    {
        bool geometryShaderEnabled = mFeatures.supportsTransformFeedbackExtension.enabled &&
                                     (mFeatures.supportsPrimitivesGeneratedQuery.enabled ||
                                      mFeatures.exposeNonConformantExtensionsAndVersions.enabled);
        mNativeExtensions.geometryShaderEXT = geometryShaderEnabled;
        mNativeExtensions.geometryShaderOES = geometryShaderEnabled;
        mNativeCaps.maxFramebufferLayers    = rx::LimitToInt(limitsVk.maxFramebufferLayers);

        // Use "undefined" which means APP would have to set gl_Layer identically.
        mNativeCaps.layerProvokingVertex = GL_UNDEFINED_VERTEX_EXT;

        mNativeCaps.maxGeometryInputComponents =
            rx::LimitToInt(limitsVk.maxGeometryInputComponents);
        mNativeCaps.maxGeometryOutputComponents =
            rx::LimitToInt(limitsVk.maxGeometryOutputComponents) - reservedVaryingComponentCount;
        mNativeCaps.maxGeometryOutputVertices = rx::LimitToInt(limitsVk.maxGeometryOutputVertices);
        mNativeCaps.maxGeometryTotalOutputComponents =
            rx::LimitToInt(limitsVk.maxGeometryTotalOutputComponents);
        if (mPhysicalDeviceFeatures.vertexPipelineStoresAndAtomics)
        {
            mNativeCaps.maxShaderStorageBlocks[gl::ShaderType::Geometry] =
                mNativeCaps.maxCombinedShaderOutputResources;
            mNativeCaps.maxShaderAtomicCounterBuffers[gl::ShaderType::Geometry] =
                maxCombinedAtomicCounterBuffers;
        }
        mNativeCaps.maxGeometryShaderInvocations =
            rx::LimitToInt(limitsVk.maxGeometryShaderInvocations);

        // Cap maxGeometryInputComponents by maxVertexOutputComponents and
        // maxTessellationEvaluationOutputComponents; there can't be more inputs than there are
        // outputs in the previous stage.
        mNativeCaps.maxGeometryInputComponents =
            std::min(mNativeCaps.maxGeometryInputComponents,
                     std::min(mNativeCaps.maxVertexOutputComponents,
                              mNativeCaps.maxTessEvaluationOutputComponents));

        // Reserve a uniform buffer binding for the geometry stage
        if (geometryShaderEnabled)
        {
            maxCombinedUniformBuffers -= kReservedPerStageDefaultUniformBindingCount;
        }
    }

    mNativeCaps.maxCombinedUniformBlocks = maxCombinedUniformBuffers;
    mNativeCaps.maxUniformBufferBindings = maxCombinedUniformBuffers;

    // GL_APPLE_clip_distance / GL_EXT_clip_cull_distance / GL_ANGLE_clip_cull_distance
    // From the EXT_clip_cull_distance extension spec:
    //
    // > Modify Section 7.2, "Built-In Constants" (p. 126)
    // >
    // > const mediump int gl_MaxClipDistances = 8;
    // > const mediump int gl_MaxCullDistances = 8;
    // > const mediump int gl_MaxCombinedClipAndCullDistances = 8;
    constexpr uint32_t kMaxClipDistancePerSpec                = 8;
    constexpr uint32_t kMaxCullDistancePerSpec                = 8;
    constexpr uint32_t kMaxCombinedClipAndCullDistancePerSpec = 8;

    // TODO: http://anglebug.com/42264006
    // After implementing EXT_geometry_shader, EXT_clip_cull_distance should be additionally
    // implemented to support the geometry shader. Until then, EXT_clip_cull_distance is enabled
    // only in the experimental cases.
    if (mPhysicalDeviceFeatures.shaderClipDistance &&
        limitsVk.maxClipDistances >= kMaxClipDistancePerSpec)
    {
        // Do not enable GL_APPLE_clip_distance for Samsung devices.
        mNativeExtensions.clipDistanceAPPLE     = mFeatures.supportsAppleClipDistance.enabled;
        mNativeExtensions.clipCullDistanceANGLE = true;
        mNativeCaps.maxClipDistances            = limitsVk.maxClipDistances;

        if (mPhysicalDeviceFeatures.shaderCullDistance &&
            limitsVk.maxCullDistances >= kMaxCullDistancePerSpec &&
            limitsVk.maxCombinedClipAndCullDistances >= kMaxCombinedClipAndCullDistancePerSpec)
        {
            mNativeExtensions.clipCullDistanceEXT       = true;
            mNativeCaps.maxCullDistances                = limitsVk.maxCullDistances;
            mNativeCaps.maxCombinedClipAndCullDistances = limitsVk.maxCombinedClipAndCullDistances;
        }
    }

    // GL_EXT_blend_func_extended
    mNativeExtensions.blendFuncExtendedEXT = mPhysicalDeviceFeatures.dualSrcBlend == VK_TRUE;
    mNativeCaps.maxDualSourceDrawBuffers   = rx::LimitToInt(limitsVk.maxFragmentDualSrcAttachments);

    // GL_ANGLE_relaxed_vertex_attribute_type
    mNativeExtensions.relaxedVertexAttributeTypeANGLE = true;

    // GL_OVR_multiview*.  Bresenham line emulation does not work with multiview.  There's no
    // limitation in Vulkan to restrict an application to multiview 1.
    mNativeExtensions.multiviewOVR =
        mFeatures.supportsMultiview.enabled && mFeatures.bresenhamLineRasterization.enabled;
    mNativeExtensions.multiview2OVR = mNativeExtensions.multiviewOVR;
    // Max views affects the number of Vulkan queries per GL query in render pass, and
    // SecondaryCommandBuffer's ResetQueryPoolParams would like this to have an upper limit (of
    // 255).
    mNativeCaps.maxViews = std::min(mMultiviewProperties.maxMultiviewViewCount, 8u);

    // GL_ANGLE_yuv_internal_format
    mNativeExtensions.yuvInternalFormatANGLE =
        getFeatures().supportsYUVSamplerConversion.enabled && vk::CanSupportYuvInternalFormat(this);

    // GL_EXT_primitive_bounding_box
    mNativeExtensions.primitiveBoundingBoxEXT = true;

    // GL_OES_primitive_bounding_box
    mNativeExtensions.primitiveBoundingBoxOES = true;

    // GL_EXT_protected_textures
    mNativeExtensions.protectedTexturesEXT = mFeatures.supportsProtectedMemory.enabled;

    // GL_ANGLE_vulkan_image
    mNativeExtensions.vulkanImageANGLE = true;

    // GL_ANGLE_texture_usage
    mNativeExtensions.textureUsageANGLE = true;

    // GL_KHR_parallel_shader_compile
    mNativeExtensions.parallelShaderCompileKHR = mFeatures.enableParallelCompileAndLink.enabled;

    // GL_NV_read_depth, GL_NV_read_depth_stencil, GL_NV_read_stencil
    mNativeExtensions.readDepthNV        = true;
    mNativeExtensions.readDepthStencilNV = true;
    mNativeExtensions.readStencilNV      = true;

    // GL_EXT_clear_texture
    mNativeExtensions.clearTextureEXT = true;

    // GL_QCOM_shading_rate
    mNativeExtensions.shadingRateQCOM = mFeatures.supportsFragmentShadingRate.enabled;

    // GL_QCOM_framebuffer_foveated
    mNativeExtensions.framebufferFoveatedQCOM = mFeatures.supportsFoveatedRendering.enabled;
    // GL_QCOM_texture_foveated
    mNativeExtensions.textureFoveatedQCOM = mFeatures.supportsFoveatedRendering.enabled;

    // GL_ANGLE_shader_pixel_local_storage
    mNativeExtensions.shaderPixelLocalStorageANGLE = true;
    if (getFeatures().supportsShaderFramebufferFetch.enabled && mIsColorFramebufferFetchCoherent)
    {
        mNativeExtensions.shaderPixelLocalStorageCoherentANGLE = true;
        mNativePLSOptions.type             = ShPixelLocalStorageType::FramebufferFetch;
        mNativePLSOptions.fragmentSyncType = ShFragmentSynchronizationType::Automatic;
    }
    else if (getFeatures().supportsFragmentShaderPixelInterlock.enabled)
    {
        // Use shader images with VK_EXT_fragment_shader_interlock, instead of attachments, if
        // they're our only option to be coherent.
        mNativeExtensions.shaderPixelLocalStorageCoherentANGLE = true;
        mNativePLSOptions.type = ShPixelLocalStorageType::ImageLoadStore;
        // GL_ARB_fragment_shader_interlock compiles to SPV_EXT_fragment_shader_interlock.
        mNativePLSOptions.fragmentSyncType =
            ShFragmentSynchronizationType::FragmentShaderInterlock_ARB_GL;
        mNativePLSOptions.supportsNativeRGBA8ImageFormats = true;
    }
    else
    {
        mNativePLSOptions.type = ShPixelLocalStorageType::FramebufferFetch;
        ASSERT(mNativePLSOptions.fragmentSyncType == ShFragmentSynchronizationType::NotSupported);
    }

    // If framebuffer fetch is to be enabled/used, cap maxColorAttachments/maxDrawBuffers to
    // maxPerStageDescriptorInputAttachments.  Note that 4 is the minimum required value for
    // maxColorAttachments and maxDrawBuffers in GL, and also happens to be the minimum required
    // value for maxPerStageDescriptorInputAttachments in Vulkan.  This means that capping the color
    // attachment count to maxPerStageDescriptorInputAttachments can never lead to an invalid value.
    const bool hasMRTFramebufferFetch =
        mNativeExtensions.shaderFramebufferFetchEXT ||
        mNativeExtensions.shaderFramebufferFetchNonCoherentEXT ||
        mNativePLSOptions.type == ShPixelLocalStorageType::FramebufferFetch;
    if (hasMRTFramebufferFetch)
    {
        mNativeCaps.maxColorAttachments = std::min<uint32_t>(
            mNativeCaps.maxColorAttachments, limitsVk.maxPerStageDescriptorInputAttachments);
        mNativeCaps.maxDrawBuffers = std::min<uint32_t>(
            mNativeCaps.maxDrawBuffers, limitsVk.maxPerStageDescriptorInputAttachments);

        // Make sure no more than the allowed input attachments bindings are used by descriptor set
        // layouts.  This number matches the number of color attachments because of framebuffer
        // fetch, and that limit is later capped to IMPLEMENTATION_MAX_DRAW_BUFFERS in Context.cpp.
        mMaxColorInputAttachmentCount = std::min<uint32_t>(mNativeCaps.maxColorAttachments,
                                                           gl::IMPLEMENTATION_MAX_DRAW_BUFFERS);
    }
    else if (mFeatures.emulateAdvancedBlendEquations.enabled)
    {
        // ANGLE may also use framebuffer fetch to emulate KHR_blend_equation_advanced, which needs
        // a single input attachment.
        mMaxColorInputAttachmentCount = 1;
    }
    else
    {
        // mMaxColorInputAttachmentCount is left as 0 to catch bugs if a future user of framebuffer
        // fetch functionality does not update the logic in this if/else chain.
    }

    // Enable the ARM_shader_framebuffer_fetch_depth_stencil extension only if the number of input
    // descriptor exceeds the color attachment count by at least 2 (for depth and stencil), or if
    // the number of color attachments can be reduced to accomodate for the 2 depth/stencil images.
    if (mFeatures.supportsShaderFramebufferFetchDepthStencil.enabled)
    {
        const uint32_t maxColorAttachmentsWithDepthStencilInput = std::min<uint32_t>(
            mNativeCaps.maxColorAttachments, limitsVk.maxPerStageDescriptorInputAttachments - 2);
        const uint32_t maxDrawBuffersWithDepthStencilInput = std::min<uint32_t>(
            mNativeCaps.maxDrawBuffers, limitsVk.maxPerStageDescriptorInputAttachments - 2);

        // As long as the minimum required color attachments (4) is satisfied, the extension can be
        // exposed.
        if (maxColorAttachmentsWithDepthStencilInput >= 4 &&
            maxDrawBuffersWithDepthStencilInput >= 4)
        {
            mNativeExtensions.shaderFramebufferFetchDepthStencilARM = true;
            mNativeCaps.maxColorAttachments = maxColorAttachmentsWithDepthStencilInput;
            mNativeCaps.maxDrawBuffers      = maxDrawBuffersWithDepthStencilInput;
            mMaxColorInputAttachmentCount =
                std::min<uint32_t>(mMaxColorInputAttachmentCount, mNativeCaps.maxColorAttachments);
        }
    }

    mNativeExtensions.logicOpANGLE = mPhysicalDeviceFeatures.logicOp == VK_TRUE;

    mNativeExtensions.YUVTargetEXT = mFeatures.supportsExternalFormatResolve.enabled;

    mNativeExtensions.textureStorageCompressionEXT =
        mFeatures.supportsImageCompressionControl.enabled;
    mNativeExtensions.EGLImageStorageCompressionEXT =
        mFeatures.supportsImageCompressionControl.enabled;

    // Log any missing extensions required for GLES 3.2.
    LogMissingExtensionsForGLES32(mNativeExtensions);
}

bool CanSupportGLES32(const gl::Extensions &nativeExtensions)
{
    std::vector<bool> requiredExtensions = GetRequiredGLES32ExtensionList(nativeExtensions);
    for (uint32_t index = 0; index < requiredExtensions.size(); index++)
    {
        if (!requiredExtensions[index])
        {
            return false;
        }
    }

    return true;
}

bool CanSupportTransformFeedbackExtension(
    const VkPhysicalDeviceTransformFeedbackFeaturesEXT &xfbFeatures)
{
    return xfbFeatures.transformFeedback == VK_TRUE;
}

bool CanSupportTransformFeedbackEmulation(const VkPhysicalDeviceFeatures &features)
{
    return features.vertexPipelineStoresAndAtomics == VK_TRUE;
}

}  // namespace vk

namespace egl_vk
{

namespace
{

EGLint ComputeMaximumPBufferPixels(const VkPhysicalDeviceProperties &physicalDeviceProperties)
{
    // EGLints are signed 32-bit integers, it's fairly easy to overflow them, especially since
    // Vulkan's minimum guaranteed VkImageFormatProperties::maxResourceSize is 2^31 bytes.
    constexpr uint64_t kMaxValueForEGLint =
        static_cast<uint64_t>(std::numeric_limits<EGLint>::max());

    // TODO(geofflang): Compute the maximum size of a pbuffer by using the maxResourceSize result
    // from vkGetPhysicalDeviceImageFormatProperties for both the color and depth stencil format and
    // the exact image creation parameters that would be used to create the pbuffer. Because it is
    // always safe to return out-of-memory errors on pbuffer allocation, it's fine to simply return
    // the number of pixels in a max width by max height pbuffer for now.
    // http://anglebug.com/42261335

    // Storing the result of squaring a 32-bit unsigned int in a 64-bit unsigned int is safe.
    static_assert(std::is_same<decltype(physicalDeviceProperties.limits.maxImageDimension2D),
                               uint32_t>::value,
                  "physicalDeviceProperties.limits.maxImageDimension2D expected to be a uint32_t.");
    const uint64_t maxDimensionsSquared =
        static_cast<uint64_t>(physicalDeviceProperties.limits.maxImageDimension2D) *
        static_cast<uint64_t>(physicalDeviceProperties.limits.maxImageDimension2D);

    return static_cast<EGLint>(std::min(maxDimensionsSquared, kMaxValueForEGLint));
}

EGLint GetMatchFormat(GLenum internalFormat)
{
    // Lock Surface match format
    switch (internalFormat)
    {
        case GL_RGBA8:
            return EGL_FORMAT_RGBA_8888_KHR;
        case GL_BGRA8_EXT:
            return EGL_FORMAT_RGBA_8888_EXACT_KHR;
        case GL_RGB565:
            return EGL_FORMAT_RGB_565_EXACT_KHR;
        default:
            return EGL_NONE;
    }
}

// Generates a basic config for a combination of color format, depth stencil format and sample
// count.
egl::Config GenerateDefaultConfig(DisplayVk *display,
                                  const gl::InternalFormat &colorFormat,
                                  const gl::InternalFormat &depthStencilFormat,
                                  EGLint sampleCount)
{
    const vk::Renderer *renderer = display->getRenderer();

    const VkPhysicalDeviceProperties &physicalDeviceProperties =
        renderer->getPhysicalDeviceProperties();
    gl::Version maxSupportedESVersion = renderer->getMaxSupportedESVersion();

    // ES3 features are required to emulate ES1
    EGLint es1Support = (maxSupportedESVersion.major >= 3 ? EGL_OPENGL_ES_BIT : 0);
    EGLint es2Support = (maxSupportedESVersion.major >= 2 ? EGL_OPENGL_ES2_BIT : 0);
    EGLint es3Support = (maxSupportedESVersion.major >= 3 ? EGL_OPENGL_ES3_BIT : 0);

    egl::Config config;

    config.renderTargetFormat = colorFormat.internalFormat;
    config.depthStencilFormat = depthStencilFormat.internalFormat;
    config.bufferSize         = colorFormat.getEGLConfigBufferSize();
    config.redSize            = colorFormat.redBits;
    config.greenSize          = colorFormat.greenBits;
    config.blueSize           = colorFormat.blueBits;
    config.alphaSize          = colorFormat.alphaBits;
    config.alphaMaskSize      = 0;
    config.bindToTextureRGB   = colorFormat.format == GL_RGB;
    config.bindToTextureRGBA  = colorFormat.format == GL_RGBA || colorFormat.format == GL_BGRA_EXT;
    config.colorBufferType    = EGL_RGB_BUFFER;
    config.configCaveat       = GetConfigCaveat(colorFormat.internalFormat);
    config.conformant         = es1Support | es2Support | es3Support;
    config.depthSize          = depthStencilFormat.depthBits;
    config.stencilSize        = depthStencilFormat.stencilBits;
    config.level              = 0;
    config.matchNativePixmap  = EGL_NONE;
    config.maxPBufferWidth    = physicalDeviceProperties.limits.maxImageDimension2D;
    config.maxPBufferHeight   = physicalDeviceProperties.limits.maxImageDimension2D;
    config.maxPBufferPixels   = ComputeMaximumPBufferPixels(physicalDeviceProperties);
    config.maxSwapInterval    = 1;
    config.minSwapInterval    = 0;
    config.nativeRenderable   = EGL_TRUE;
    config.nativeVisualID     = static_cast<EGLint>(GetNativeVisualID(colorFormat));
    config.nativeVisualType   = EGL_NONE;
    config.renderableType     = es1Support | es2Support | es3Support;
    config.sampleBuffers      = (sampleCount > 0) ? 1 : 0;
    config.samples            = sampleCount;
    config.surfaceType        = EGL_WINDOW_BIT | EGL_PBUFFER_BIT;
    if (display->getExtensions().mutableRenderBufferKHR)
    {
        config.surfaceType |= EGL_MUTABLE_RENDER_BUFFER_BIT_KHR;
    }
    // Vulkan surfaces use a different origin than OpenGL, always prefer to be flipped vertically if
    // possible.
    config.optimalOrientation    = EGL_SURFACE_ORIENTATION_INVERT_Y_ANGLE;
    config.transparentType       = EGL_NONE;
    config.transparentRedValue   = 0;
    config.transparentGreenValue = 0;
    config.transparentBlueValue  = 0;
    config.colorComponentType =
        gl_egl::GLComponentTypeToEGLColorComponentType(colorFormat.componentType);
    // LockSurface matching
    config.matchFormat = GetMatchFormat(colorFormat.internalFormat);
    if (config.matchFormat != EGL_NONE)
    {
        config.surfaceType |= EGL_LOCK_SURFACE_BIT_KHR;
    }

    // Vulkan always supports off-screen rendering.  Check the config with display to see if it can
    // also have window support.  If not, the following call should automatically remove
    // EGL_WINDOW_BIT.
    display->checkConfigSupport(&config);

    return config;
}

}  // anonymous namespace

egl::ConfigSet GenerateConfigs(const GLenum *colorFormats,
                               size_t colorFormatsCount,
                               const GLenum *depthStencilFormats,
                               size_t depthStencilFormatCount,
                               DisplayVk *display)
{
    ASSERT(colorFormatsCount > 0);
    ASSERT(display != nullptr);

    gl::SupportedSampleSet colorSampleCounts;
    gl::SupportedSampleSet depthStencilSampleCounts;
    gl::SupportedSampleSet sampleCounts;

    const VkPhysicalDeviceLimits &limits =
        display->getRenderer()->getPhysicalDeviceProperties().limits;
    const uint32_t depthStencilSampleCountsLimit = limits.framebufferDepthSampleCounts &
                                                   limits.framebufferStencilSampleCounts &
                                                   vk_gl::kSupportedSampleCounts;

    vk_gl::AddSampleCounts(limits.framebufferColorSampleCounts & vk_gl::kSupportedSampleCounts,
                           &colorSampleCounts);
    vk_gl::AddSampleCounts(depthStencilSampleCountsLimit, &depthStencilSampleCounts);

    // Always support 0 samples
    colorSampleCounts.insert(0);
    depthStencilSampleCounts.insert(0);

    std::set_intersection(colorSampleCounts.begin(), colorSampleCounts.end(),
                          depthStencilSampleCounts.begin(), depthStencilSampleCounts.end(),
                          std::inserter(sampleCounts, sampleCounts.begin()));

    egl::ConfigSet configSet;

    for (size_t colorFormatIdx = 0; colorFormatIdx < colorFormatsCount; colorFormatIdx++)
    {
        const gl::InternalFormat &colorFormatInfo =
            gl::GetSizedInternalFormatInfo(colorFormats[colorFormatIdx]);
        ASSERT(colorFormatInfo.sized);

        for (size_t depthStencilFormatIdx = 0; depthStencilFormatIdx < depthStencilFormatCount;
             depthStencilFormatIdx++)
        {
            const gl::InternalFormat &depthStencilFormatInfo =
                gl::GetSizedInternalFormatInfo(depthStencilFormats[depthStencilFormatIdx]);
            ASSERT(depthStencilFormats[depthStencilFormatIdx] == GL_NONE ||
                   depthStencilFormatInfo.sized);

            const gl::SupportedSampleSet *configSampleCounts = &sampleCounts;
            // If there is no depth/stencil buffer, use the color samples set.
            if (depthStencilFormats[depthStencilFormatIdx] == GL_NONE)
            {
                configSampleCounts = &colorSampleCounts;
            }
            // If there is no color buffer, use the depth/stencil samples set.
            else if (colorFormats[colorFormatIdx] == GL_NONE)
            {
                configSampleCounts = &depthStencilSampleCounts;
            }

            for (EGLint sampleCount : *configSampleCounts)
            {
                egl::Config config = GenerateDefaultConfig(display, colorFormatInfo,
                                                           depthStencilFormatInfo, sampleCount);
                configSet.add(config);
            }
        }
    }

    return configSet;
}

}  // namespace egl_vk

}  // namespace rx
