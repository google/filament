//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// HardwareBufferImageSiblingVkAndroid.cpp: Implements HardwareBufferImageSiblingVkAndroid.

#include "libANGLE/renderer/vulkan/android/HardwareBufferImageSiblingVkAndroid.h"

#include "common/android_util.h"

#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/android/AHBFunctions.h"
#include "libANGLE/renderer/vulkan/android/DisplayVkAndroid.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{

namespace
{
VkImageTiling AhbDescUsageToVkImageTiling(const AHardwareBuffer_Desc &ahbDescription)
{
    // A note about the choice of OPTIMAL here.

    // When running Android on certain GPUs, there are problems creating Vulkan
    // image siblings of AHardwareBuffers because it's currently assumed that
    // the underlying driver can create linear tiling images that have input
    // attachment usage, which isn't supported on NVIDIA for example, resulting
    // in failure to create the image siblings. Yet, we don't currently take
    // advantage of linear elsewhere in ANGLE. To maintain maximum
    // compatibility on Android for such drivers, use optimal tiling for image
    // siblings.
    //
    // Note that while we have switched to optimal unconditionally in this path
    // versus linear, it's possible that previously compatible linear usages
    // might become uncompatible after switching to optimal. However, from what
    // we've seen on Samsung/NVIDIA/Intel/AMD GPUs so far, formats generally
    // have more possible usages in optimal tiling versus linear tiling:
    //
    // http://vulkan.gpuinfo.org/displayreport.php?id=10804#formats_linear
    // http://vulkan.gpuinfo.org/displayreport.php?id=10804#formats_optimal
    //
    // http://vulkan.gpuinfo.org/displayreport.php?id=10807#formats_linear
    // http://vulkan.gpuinfo.org/displayreport.php?id=10807#formats_optimal
    //
    // http://vulkan.gpuinfo.org/displayreport.php?id=10809#formats_linear
    // http://vulkan.gpuinfo.org/displayreport.php?id=10809#formats_optimal
    //
    // http://vulkan.gpuinfo.org/displayreport.php?id=10787#formats_linear
    // http://vulkan.gpuinfo.org/displayreport.php?id=10787#formats_optimal
    //
    // Also, as an aside, in terms of what's generally expected from the Vulkan
    // ICD in Android when determining AHB compatibility, if the vendor wants
    // to declare a particular combination of format/tiling/usage/etc as not
    // supported AHB-wise, it's up to the ICD vendor to zero out bits in
    // supportedHandleTypes in the vkGetPhysicalDeviceImageFormatProperties2
    // query:
    //
    // ``` *
    // [VUID-VkImageCreateInfo-pNext-00990](https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#VUID-VkImageCreateInfo-pNext-00990)
    // If the pNext chain includes a VkExternalMemoryImageCreateInfo structure,
    // its handleTypes member must only contain bits that are also in
    // VkExternalImageFormatProperties::externalMemoryProperties.compatibleHandleTypes,
    // as returned by vkGetPhysicalDeviceImageFormatProperties2 with format,
    // imageType, tiling, usage, and flags equal to those in this structure,
    // and with a VkPhysicalDeviceExternalImageFormatInfo structure included in
    // the pNext chain, with a handleType equal to any one of the handle types
    // specified in VkExternalMemoryImageCreateInfo::handleTypes ```

    return VK_IMAGE_TILING_OPTIMAL;
}

// Map AHB usage flags to VkImageUsageFlags using this table from the Vulkan spec
// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/chap11.html#memory-external-android-hardware-buffer-usage
VkImageUsageFlags AhbDescUsageToVkImageUsage(const AHardwareBuffer_Desc &ahbDescription,
                                             bool isDepthOrStencilFormat)
{
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    if ((ahbDescription.usage & AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE) != 0)
    {
        usage |= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }

    if ((ahbDescription.usage & AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER) != 0)
    {
        if (isDepthOrStencilFormat)
        {
            usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        else
        {
            usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
    }

    return usage;
}

// Map AHB usage flags to VkImageCreateFlags using this table from the Vulkan spec
// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/chap11.html#memory-external-android-hardware-buffer-usage
VkImageCreateFlags AhbDescUsageToVkImageCreateFlags(const AHardwareBuffer_Desc &ahbDescription)
{
    VkImageCreateFlags imageCreateFlags = vk::kVkImageCreateFlagsNone;

    if ((ahbDescription.usage & AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP) != 0)
    {
        imageCreateFlags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    if ((ahbDescription.usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT) != 0)
    {
        imageCreateFlags |= VK_IMAGE_CREATE_PROTECTED_BIT;
    }

    return imageCreateFlags;
}

// Deduce texture type based on AHB usage flags and layer count
gl::TextureType AhbDescUsageToTextureType(const AHardwareBuffer_Desc &ahbDescription,
                                          const uint32_t layerCount)
{
    gl::TextureType textureType = layerCount > 1 ? gl::TextureType::_2DArray : gl::TextureType::_2D;
    if ((ahbDescription.usage & AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP) != 0)
    {
        textureType = layerCount > gl::kCubeFaceCount ? gl::TextureType::CubeMapArray
                                                      : gl::TextureType::CubeMap;
    }
    return textureType;
}
// TODO(anglebug.com/42266422): remove when NDK header is updated to contain FRONT_BUFFER usage flag
constexpr uint64_t kAHardwareBufferUsageFrontBuffer = (1ULL << 32);
}  // namespace

HardwareBufferImageSiblingVkAndroid::HardwareBufferImageSiblingVkAndroid(EGLClientBuffer buffer)
    : mBuffer(buffer),
      mFormat(GL_NONE),
      mRenderable(false),
      mTextureable(false),
      mYUV(false),
      mLevelCount(0),
      mUsage(0),
      mSamples(0),
      mImage(nullptr)
{}

HardwareBufferImageSiblingVkAndroid::~HardwareBufferImageSiblingVkAndroid() {}

// Static
egl::Error HardwareBufferImageSiblingVkAndroid::ValidateHardwareBuffer(
    vk::Renderer *renderer,
    EGLClientBuffer buffer,
    const egl::AttributeMap &attribs)
{
    struct ANativeWindowBuffer *windowBuffer =
        angle::android::ClientBufferToANativeWindowBuffer(buffer);
    struct AHardwareBuffer *hardwareBuffer = nullptr;
    if (windowBuffer != nullptr)
    {
        if (!angle::android::IsValidNativeWindowBuffer(windowBuffer))
        {
            return egl::EglBadParameter()
                   << "The given buffer is not a valid native window buffer.";
        }
        hardwareBuffer = angle::android::ANativeWindowBufferToAHardwareBuffer(windowBuffer);
        if (hardwareBuffer == nullptr)
        {
            return egl::EglBadParameter()
                   << "Failed to obtain hardware buffer through given window buffer.";
        }
    }
    else
    {
        return egl::EglBadParameter()
               << "Failed to obtain Window buffer through given client buffer handler.";
    }

    VkAndroidHardwareBufferFormatPropertiesANDROID bufferFormatProperties = {};
    bufferFormatProperties.sType =
        VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
    bufferFormatProperties.pNext = nullptr;

    VkAndroidHardwareBufferPropertiesANDROID bufferProperties = {};
    bufferProperties.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    bufferProperties.pNext = &bufferFormatProperties;

    VkDevice device = renderer->getDevice();
    VkResult result =
        vkGetAndroidHardwareBufferPropertiesANDROID(device, hardwareBuffer, &bufferProperties);
    if (result != VK_SUCCESS)
    {
        return egl::EglBadParameter() << "Failed to query AHardwareBuffer properties";
    }

    int width       = 0;
    int height      = 0;
    int depth       = 0;
    int pixelFormat = 0;
    uint64_t usage  = 0;
    angle::android::GetANativeWindowBufferProperties(windowBuffer, &width, &height, &depth,
                                                     &pixelFormat, &usage);

    if (bufferFormatProperties.format == VK_FORMAT_UNDEFINED)
    {
        ASSERT(bufferFormatProperties.externalFormat != 0);
        // We must have an external format, check that it supports texture sampling
        if (!(bufferFormatProperties.formatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
        {
            return egl::EglBadParameter()
                   << "Sampling from AHardwareBuffer externalFormat 0x" << std::hex
                   << bufferFormatProperties.externalFormat << " is unsupported ";
        }
    }
    else
    {
        angle::FormatID formatID = vk::GetFormatIDFromVkFormat(bufferFormatProperties.format);
        const bool hasNecessaryFormatSupport =
            (usage & AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER) != 0
                ? HasFullTextureFormatSupport(renderer, formatID)
                : HasNonRenderableTextureFormatSupport(renderer, formatID);
        if (!hasNecessaryFormatSupport)
        {
            return egl::EglBadParameter()
                   << "AHardwareBuffer format " << bufferFormatProperties.format
                   << " does not support enough features to use as a texture.";
        }
    }

    if (attribs.getAsInt(EGL_PROTECTED_CONTENT_EXT, EGL_FALSE) == EGL_TRUE)
    {
        if ((usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT) == 0)
        {
            return egl::EglBadAccess()
                   << "EGL_PROTECTED_CONTENT_EXT attribute does not match protected state "
                      "of EGLClientBuffer.";
        }
    }

    return egl::NoError();
}

egl::Error HardwareBufferImageSiblingVkAndroid::initialize(const egl::Display *display)
{
    DisplayVk *displayVk = vk::GetImpl(display);
    return angle::ToEGL(initImpl(displayVk), EGL_BAD_PARAMETER);
}

angle::Result HardwareBufferImageSiblingVkAndroid::initImpl(DisplayVk *displayVk)
{
    const AHBFunctions &functions = static_cast<DisplayVkAndroid *>(displayVk)->getAHBFunctions();
    ANGLE_VK_CHECK(displayVk, functions.valid(), VK_ERROR_INITIALIZATION_FAILED);

    vk::Renderer *renderer = displayVk->getRenderer();

    struct ANativeWindowBuffer *windowBuffer =
        angle::android::ClientBufferToANativeWindowBuffer(mBuffer);

    int pixelFormat = 0;
    angle::android::GetANativeWindowBufferProperties(windowBuffer, &mSize.width, &mSize.height,
                                                     &mSize.depth, &pixelFormat, &mUsage);

    struct AHardwareBuffer *hardwareBuffer =
        angle::android::ANativeWindowBufferToAHardwareBuffer(windowBuffer);

    functions.acquire(hardwareBuffer);

    VkAndroidHardwareBufferPropertiesANDROID bufferProperties = {};
    bufferProperties.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    bufferProperties.pNext = nullptr;

    VkAndroidHardwareBufferFormatPropertiesANDROID bufferFormatProperties;
    bufferFormatProperties.sType =
        VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
    bufferFormatProperties.pNext = nullptr;
    vk::AddToPNextChain(&bufferProperties, &bufferFormatProperties);

    VkAndroidHardwareBufferFormatResolvePropertiesANDROID bufferFormatResolveProperties = {};
    if (renderer->getFeatures().supportsExternalFormatResolve.enabled)
    {
        bufferFormatResolveProperties.sType =
            VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_RESOLVE_PROPERTIES_ANDROID;
        bufferFormatResolveProperties.pNext = nullptr;
        vk::AddToPNextChain(&bufferFormatProperties, &bufferFormatResolveProperties);
    }

    VkDevice device = renderer->getDevice();
    ANGLE_VK_TRY(displayVk, vkGetAndroidHardwareBufferPropertiesANDROID(device, hardwareBuffer,
                                                                        &bufferProperties));

    const bool isExternal = bufferFormatProperties.format == VK_FORMAT_UNDEFINED;

    VkExternalFormatANDROID externalFormat = {};
    externalFormat.sType                   = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    externalFormat.externalFormat          = 0;

    // Use bufferFormatProperties.format directly when possible.  For RGBX, the spec requires the
    // corresponding format to be RGB, which is not _technically_ correct.  The Vulkan backend uses
    // the RGBX8_ANGLE format, so that's overriden.
    //
    // Where bufferFormatProperties.format returns UNDEFINED, NativePixelFormatToGLInternalFormat is
    // used to infer the format.
    const vk::Format *vkFormat = nullptr;
    if (pixelFormat == AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM)
    {
        vkFormat = &renderer->getFormat(GL_RGBX8_ANGLE);
    }
    else if (!isExternal)
    {
        vkFormat = &renderer->getFormat(vk::GetFormatIDFromVkFormat(bufferFormatProperties.format));
    }
    else
    {
        vkFormat =
            &renderer->getFormat(angle::android::NativePixelFormatToGLInternalFormat(pixelFormat));
    }

    const angle::Format &imageFormat = vkFormat->getActualRenderableImageFormat();
    bool isDepthOrStencilFormat      = imageFormat.hasDepthOrStencilBits();
    mFormat                          = gl::Format(vkFormat->getIntendedGLFormat());

    bool externalRenderTargetSupported =
        renderer->getFeatures().supportsExternalFormatResolve.enabled &&
        bufferFormatResolveProperties.colorAttachmentFormat != VK_FORMAT_UNDEFINED;
    // Can assume based on us getting here already. The supportsYUVSamplerConversion
    // check below should serve as a backup otherwise.
    bool externalTexturingSupported = true;

    // Query AHB description and do the following -
    // 1. Derive VkImageTiling mode based on AHB usage flags
    // 2. Map AHB usage flags to VkImageUsageFlags
    AHardwareBuffer_Desc ahbDescription;
    functions.describe(hardwareBuffer, &ahbDescription);
    VkImageTiling imageTilingMode = AhbDescUsageToVkImageTiling(ahbDescription);
    VkImageUsageFlags usage = AhbDescUsageToVkImageUsage(ahbDescription, isDepthOrStencilFormat);

    if (isExternal)
    {
        ANGLE_VK_CHECK(displayVk, bufferFormatProperties.externalFormat != 0, VK_ERROR_UNKNOWN);
        externalFormat.externalFormat = bufferFormatProperties.externalFormat;

        // VkImageCreateInfo struct: If the pNext chain includes a VkExternalFormatANDROID structure
        // whose externalFormat member is not 0, usage must not include any usages except
        // VK_IMAGE_USAGE_SAMPLED_BIT
        if (externalFormat.externalFormat != 0 && !externalRenderTargetSupported)
        {
            // Clear all other bits except sampled
            usage &= VK_IMAGE_USAGE_SAMPLED_BIT;
        }

        // If the pNext chain includes a VkExternalFormatANDROID structure whose externalFormat
        // member is not 0, tiling must be VK_IMAGE_TILING_OPTIMAL
        imageTilingMode = VK_IMAGE_TILING_OPTIMAL;
    }

    // If forceSampleUsageForAhbBackedImages feature is enabled force enable
    // VK_IMAGE_USAGE_SAMPLED_BIT
    if (renderer->getFeatures().forceSampleUsageForAhbBackedImages.enabled)
    {
        usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {};
    externalMemoryImageCreateInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
    externalMemoryImageCreateInfo.pNext = &externalFormat;
    externalMemoryImageCreateInfo.handleTypes =
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkExtent3D vkExtents;
    gl_vk::GetExtent(mSize, &vkExtents);

    // Setup level count
    mLevelCount = ((ahbDescription.usage & AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE) != 0)
                      ? static_cast<uint32_t>(log2(std::max(mSize.width, mSize.height))) + 1
                      : 1;

    // No support for rendering to external YUV AHB with multiple miplevels
    ANGLE_VK_CHECK(displayVk, (!externalRenderTargetSupported || mLevelCount == 1),
                   VK_ERROR_INITIALIZATION_FAILED);

    // Setup layer count
    const uint32_t layerCount = mSize.depth;
    vkExtents.depth           = 1;

    mImage = new vk::ImageHelper();

    // disable robust init for this external image.
    bool robustInitEnabled = false;

    mImage->setTilingMode(imageTilingMode);
    VkImageCreateFlags imageCreateFlags = AhbDescUsageToVkImageCreateFlags(ahbDescription);
    vk::YcbcrConversionDesc conversionDesc{};

    if (isExternal)
    {
        if (externalRenderTargetSupported)
        {
            angle::FormatID externalFormatID =
                renderer->getExternalFormatTable()->getOrAllocExternalFormatID(
                    bufferFormatProperties.externalFormat,
                    bufferFormatResolveProperties.colorAttachmentFormat,
                    bufferFormatProperties.formatFeatures);

            vkFormat = &renderer->getFormat(externalFormatID);
        }
        else
        {
            // If not renderable, don't burn a slot on it.
            vkFormat = &renderer->getFormat(angle::FormatID::NONE);
        }
    }

    if (isExternal || imageFormat.isYUV)
    {
        // Note from Vulkan spec: Since GL_OES_EGL_image_external does not require the same sampling
        // and conversion calculations as Vulkan does, achieving identical results between APIs may
        // not be possible on some implementations.
        ANGLE_VK_CHECK(displayVk, renderer->getFeatures().supportsYUVSamplerConversion.enabled,
                       VK_ERROR_FEATURE_NOT_PRESENT);
        ASSERT(externalFormat.pNext == nullptr);

        // This may not actually mean the format is YUV. But the rest of ANGLE makes this
        // assumption and needs this member variable.
        mYUV = true;

        vk::YcbcrLinearFilterSupport linearFilterSupported =
            (bufferFormatProperties.formatFeatures &
             VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT) != 0
                ? vk::YcbcrLinearFilterSupport::Supported
                : vk::YcbcrLinearFilterSupport::Unsupported;

        conversionDesc.update(
            renderer, isExternal ? bufferFormatProperties.externalFormat : 0,
            bufferFormatProperties.suggestedYcbcrModel, bufferFormatProperties.suggestedYcbcrRange,
            bufferFormatProperties.suggestedXChromaOffset,
            bufferFormatProperties.suggestedYChromaOffset, vk::kDefaultYCbCrChromaFilter,
            bufferFormatProperties.samplerYcbcrConversionComponents,
            isExternal ? angle::FormatID::NONE : imageFormat.id, linearFilterSupported);
    }

    const gl::TextureType textureType = AhbDescUsageToTextureType(ahbDescription, layerCount);
    const angle::FormatID actualRenderableFormatID = vkFormat->getActualRenderableImageFormatID();

    VkImageFormatListCreateInfoKHR imageFormatListInfoStorage;
    vk::ImageHelper::ImageListFormats imageListFormatsStorage;

    const void *imageCreateInfoPNext = vk::ImageHelper::DeriveCreateInfoPNext(
        displayVk, usage, actualRenderableFormatID, &externalMemoryImageCreateInfo,
        &imageFormatListInfoStorage, &imageListFormatsStorage, &imageCreateFlags);

    ANGLE_TRY(mImage->initExternal(displayVk, textureType, vkExtents,
                                   vkFormat->getIntendedFormatID(), actualRenderableFormatID, 1,
                                   usage, imageCreateFlags, vk::ImageLayout::ExternalPreInitialized,
                                   imageCreateInfoPNext, gl::LevelIndex(0), mLevelCount, layerCount,
                                   robustInitEnabled, hasProtectedContent(), conversionDesc,
                                   nullptr));

    VkImportAndroidHardwareBufferInfoANDROID importHardwareBufferInfo = {};
    importHardwareBufferInfo.sType  = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
    importHardwareBufferInfo.buffer = hardwareBuffer;

    VkMemoryDedicatedAllocateInfo dedicatedAllocInfo = {};
    dedicatedAllocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
    dedicatedAllocInfo.pNext          = &importHardwareBufferInfo;
    dedicatedAllocInfo.image          = mImage->getImage().getHandle();
    dedicatedAllocInfo.buffer         = VK_NULL_HANDLE;
    const void *dedicatedAllocInfoPtr = &dedicatedAllocInfo;

    VkMemoryRequirements externalMemoryRequirements = {};
    externalMemoryRequirements.size                 = bufferProperties.allocationSize;
    externalMemoryRequirements.alignment            = 0;
    externalMemoryRequirements.memoryTypeBits       = bufferProperties.memoryTypeBits;

    const VkMemoryPropertyFlags flags =
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
        (hasProtectedContent() ? VK_MEMORY_PROPERTY_PROTECTED_BIT : 0);

    ANGLE_TRY(mImage->initExternalMemory(displayVk, renderer->getMemoryProperties(),
                                         externalMemoryRequirements, 1, &dedicatedAllocInfoPtr,
                                         vk::kForeignDeviceQueueIndex, flags));

    if (isExternal)
    {
        // External format means that we are working with VK_FORMAT_UNDEFINED,
        // so hasImageFormatFeatureBits will assert. Set these based on
        // presence of extensions or assumption.
        mRenderable  = externalRenderTargetSupported;
        mTextureable = externalTexturingSupported;
    }
    else
    {
        constexpr uint32_t kColorRenderableRequiredBits = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
        constexpr uint32_t kDepthStencilRenderableRequiredBits =
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
        mRenderable = renderer->hasImageFormatFeatureBits(actualRenderableFormatID,
                                                          kColorRenderableRequiredBits) ||
                      renderer->hasImageFormatFeatureBits(actualRenderableFormatID,
                                                          kDepthStencilRenderableRequiredBits);
        constexpr uint32_t kTextureableRequiredBits =
            VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
        mTextureable =
            renderer->hasImageFormatFeatureBits(actualRenderableFormatID, kTextureableRequiredBits);
    }

    return angle::Result::Continue;
}

void HardwareBufferImageSiblingVkAndroid::onDestroy(const egl::Display *display)
{
    const AHBFunctions &functions = GetImplAs<DisplayVkAndroid>(display)->getAHBFunctions();
    ASSERT(functions.valid());

    functions.release(angle::android::ANativeWindowBufferToAHardwareBuffer(
        angle::android::ClientBufferToANativeWindowBuffer(mBuffer)));

    ASSERT(mImage == nullptr);
}

gl::Format HardwareBufferImageSiblingVkAndroid::getFormat() const
{
    return mFormat;
}

bool HardwareBufferImageSiblingVkAndroid::isRenderable(const gl::Context *context) const
{
    return mRenderable;
}

bool HardwareBufferImageSiblingVkAndroid::isTexturable(const gl::Context *context) const
{
    return mTextureable;
}

bool HardwareBufferImageSiblingVkAndroid::isYUV() const
{
    return mYUV;
}

bool HardwareBufferImageSiblingVkAndroid::hasFrontBufferUsage() const
{
    return (mUsage & kAHardwareBufferUsageFrontBuffer) != 0;
}

bool HardwareBufferImageSiblingVkAndroid::isCubeMap() const
{
    return (mUsage & AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP) != 0;
}

bool HardwareBufferImageSiblingVkAndroid::hasProtectedContent() const
{
    return ((mUsage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT) != 0);
}

gl::Extents HardwareBufferImageSiblingVkAndroid::getSize() const
{
    return mSize;
}

size_t HardwareBufferImageSiblingVkAndroid::getSamples() const
{
    return mSamples;
}

uint32_t HardwareBufferImageSiblingVkAndroid::getLevelCount() const
{
    return mLevelCount;
}

// ExternalImageSiblingVk interface
vk::ImageHelper *HardwareBufferImageSiblingVkAndroid::getImage() const
{
    return mImage;
}

void HardwareBufferImageSiblingVkAndroid::release(vk::Renderer *renderer)
{
    if (mImage != nullptr)
    {
        // TODO: Handle the case where the EGLImage is used in two contexts not in the same share
        // group.  https://issuetracker.google.com/169868803
        mImage->releaseImage(renderer);
        mImage->releaseStagedUpdates(renderer);
        SafeDelete(mImage);
    }
}

}  // namespace rx
