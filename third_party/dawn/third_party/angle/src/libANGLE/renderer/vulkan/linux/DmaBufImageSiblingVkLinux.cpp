//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DmaBufImageSiblingVkLinux.cpp: Implements DmaBufImageSiblingVkLinux.

#include "libANGLE/renderer/vulkan/linux/DmaBufImageSiblingVkLinux.h"

#include "common/linux/dma_buf_utils.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

#include <fcntl.h>

namespace rx
{
namespace
{
constexpr uint32_t kMaxPlaneCount = 4;
template <typename T>
using PerPlane = std::array<T, kMaxPlaneCount>;

constexpr PerPlane<EGLenum> kFds = {EGL_DMA_BUF_PLANE0_FD_EXT, EGL_DMA_BUF_PLANE1_FD_EXT,
                                    EGL_DMA_BUF_PLANE2_FD_EXT, EGL_DMA_BUF_PLANE3_FD_EXT};

constexpr PerPlane<EGLenum> kOffsets = {
    EGL_DMA_BUF_PLANE0_OFFSET_EXT, EGL_DMA_BUF_PLANE1_OFFSET_EXT, EGL_DMA_BUF_PLANE2_OFFSET_EXT,
    EGL_DMA_BUF_PLANE3_OFFSET_EXT};

constexpr PerPlane<EGLenum> kPitches = {EGL_DMA_BUF_PLANE0_PITCH_EXT, EGL_DMA_BUF_PLANE1_PITCH_EXT,
                                        EGL_DMA_BUF_PLANE2_PITCH_EXT, EGL_DMA_BUF_PLANE3_PITCH_EXT};

constexpr PerPlane<EGLenum> kModifiersLo = {
    EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT,
    EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT, EGL_DMA_BUF_PLANE3_MODIFIER_LO_EXT};

constexpr PerPlane<EGLenum> kModifiersHi = {
    EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT, EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT,
    EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT, EGL_DMA_BUF_PLANE3_MODIFIER_HI_EXT};

constexpr VkImageUsageFlags kTransferUsage =
    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
constexpr VkImageUsageFlags kTextureUsage = VK_IMAGE_USAGE_SAMPLED_BIT;
constexpr VkImageUsageFlags kRenderUsage =
    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
constexpr VkImageUsageFlags kRenderAndInputUsage =
    kRenderUsage | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

struct AllocateInfo
{
    PerPlane<VkMemoryDedicatedAllocateInfo> allocateInfo = {};
    PerPlane<VkImportMemoryFdInfoKHR> importFdInfo       = {};

    PerPlane<const void *> allocateInfoPtr = {};
};

// Look at provided fds and count the number of planes based on that.
uint32_t GetPlaneCount(const egl::AttributeMap &attribs)
{
    // There should always be at least one plane.
    ASSERT(attribs.contains(kFds[0]));
    ASSERT(attribs.contains(kOffsets[0]));
    ASSERT(attribs.contains(kPitches[0]));

    for (uint32_t plane = 1; plane < kMaxPlaneCount; ++plane)
    {
        if (!attribs.contains(kFds[plane]))
        {
            return plane;
        }

        ASSERT(attribs.contains(kOffsets[plane]));
        ASSERT(attribs.contains(kPitches[plane]));
    }

    return kMaxPlaneCount;
}

uint64_t GetModifier(const egl::AttributeMap &attribs, EGLenum lo, EGLenum hi)
{
    if (!attribs.contains(lo))
    {
        return 0;
    }

    ASSERT(attribs.contains(hi));

    uint64_t modifier = attribs.getAsInt(hi);
    modifier          = modifier << 32 | attribs.getAsInt(lo);

    return modifier;
}

void GetModifiers(const egl::AttributeMap &attribs,
                  uint32_t planeCount,
                  PerPlane<uint64_t> *drmModifiersOut)
{
    for (uint32_t plane = 0; plane < planeCount; ++plane)
    {
        (*drmModifiersOut)[plane] = GetModifier(attribs, kModifiersLo[plane], kModifiersHi[plane]);
    }
}

bool GetFormatModifierProperties(DisplayVk *displayVk,
                                 VkFormat vkFormat,
                                 uint64_t drmModifier,
                                 VkDrmFormatModifierPropertiesEXT *modifierPropertiesOut)
{
    vk::Renderer *renderer = displayVk->getRenderer();

    // Query list of drm format modifiers compatible with VkFormat.
    VkDrmFormatModifierPropertiesListEXT formatModifierPropertiesList = {};
    formatModifierPropertiesList.sType = VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT;
    formatModifierPropertiesList.drmFormatModifierCount = 0;

    VkFormatProperties2 formatProperties = {};
    formatProperties.sType               = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
    formatProperties.pNext               = &formatModifierPropertiesList;

    vkGetPhysicalDeviceFormatProperties2(renderer->getPhysicalDevice(), vkFormat,
                                         &formatProperties);

    std::vector<VkDrmFormatModifierPropertiesEXT> formatModifierProperties(
        formatModifierPropertiesList.drmFormatModifierCount);
    formatModifierPropertiesList.pDrmFormatModifierProperties = formatModifierProperties.data();

    vkGetPhysicalDeviceFormatProperties2(renderer->getPhysicalDevice(), vkFormat,
                                         &formatProperties);

    // Find the requested DRM modifiers.
    uint32_t propertiesIndex = formatModifierPropertiesList.drmFormatModifierCount;
    for (uint32_t index = 0; index < formatModifierPropertiesList.drmFormatModifierCount; ++index)
    {
        if (formatModifierPropertiesList.pDrmFormatModifierProperties[index].drmFormatModifier ==
            drmModifier)
        {
            propertiesIndex = index;
            break;
        }
    }

    // Return the properties if found.
    if (propertiesIndex >= formatModifierPropertiesList.drmFormatModifierCount)
    {
        return false;
    }

    *modifierPropertiesOut =
        formatModifierPropertiesList.pDrmFormatModifierProperties[propertiesIndex];
    return true;
}

VkImageUsageFlags GetUsageFlags(vk::Renderer *renderer,
                                const angle::Format &format,
                                const VkDrmFormatModifierPropertiesEXT &properties,
                                bool *texturableOut,
                                bool *renderableOut)
{
    const bool isDepthStencilFormat = format.hasDepthOrStencilBits();

    // Check what format features are exposed for this modifier.
    constexpr uint32_t kTextureableRequiredBits =
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
    constexpr uint32_t kColorRenderableRequiredBits = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    constexpr uint32_t kDepthStencilRenderableRequiredBits =
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    *texturableOut =
        IsMaskFlagSet(properties.drmFormatModifierTilingFeatures, kTextureableRequiredBits);
    *renderableOut = IsMaskFlagSet(
        properties.drmFormatModifierTilingFeatures,
        isDepthStencilFormat ? kDepthStencilRenderableRequiredBits : kColorRenderableRequiredBits);

    VkImageUsageFlags usage = kTransferUsage;
    if (*texturableOut)
    {
        usage |= kTextureUsage;
    }
    if (*renderableOut)
    {
        usage |= isDepthStencilFormat ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                                      : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (*texturableOut && *renderableOut)
    {
        usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }

    return usage;
}

bool IsFormatSupported(vk::Renderer *renderer,
                       VkFormat vkFormat,
                       uint64_t drmModifier,
                       VkImageUsageFlags usageFlags,
                       VkImageCreateFlags createFlags,
                       VkImageFormatListCreateInfoKHR imageFormatListInfo,
                       VkImageFormatProperties2 *imageFormatPropertiesOut)
{
    VkPhysicalDeviceExternalImageFormatInfo externalImageFormatInfo = {};
    externalImageFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
    externalImageFormatInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
    imageFormatListInfo.pNext          = &externalImageFormatInfo;

    VkPhysicalDeviceImageFormatInfo2 imageFormatInfo = {};
    imageFormatInfo.sType  = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
    imageFormatInfo.format = vkFormat;
    imageFormatInfo.type   = VK_IMAGE_TYPE_2D;
    imageFormatInfo.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    imageFormatInfo.usage  = usageFlags;
    imageFormatInfo.flags  = createFlags;
    imageFormatInfo.pNext  = &imageFormatListInfo;

    VkPhysicalDeviceImageDrmFormatModifierInfoEXT drmFormatModifierInfo = {};
    drmFormatModifierInfo.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT;
    drmFormatModifierInfo.drmFormatModifier = drmModifier;
    drmFormatModifierInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
    externalImageFormatInfo.pNext           = &drmFormatModifierInfo;

    return vkGetPhysicalDeviceImageFormatProperties2(renderer->getPhysicalDevice(),
                                                     &imageFormatInfo, imageFormatPropertiesOut) !=
           VK_ERROR_FORMAT_NOT_SUPPORTED;
}

VkChromaLocation GetChromaLocation(const egl::AttributeMap &attribs, EGLenum hint)
{
    return attribs.getAsInt(hint, EGL_YUV_CHROMA_SITING_0_EXT) == EGL_YUV_CHROMA_SITING_0_EXT
               ? VK_CHROMA_LOCATION_COSITED_EVEN
               : VK_CHROMA_LOCATION_MIDPOINT;
}

VkSamplerYcbcrModelConversion GetYcbcrModel(const egl::AttributeMap &attribs)
{
    switch (attribs.getAsInt(EGL_YUV_COLOR_SPACE_HINT_EXT, EGL_ITU_REC601_EXT))
    {
        case EGL_ITU_REC601_EXT:
            return VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601;
        case EGL_ITU_REC709_EXT:
            return VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
        case EGL_ITU_REC2020_EXT:
            return VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020;
        default:
            UNREACHABLE();
            return VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601;
    }
}

VkSamplerYcbcrRange GetYcbcrRange(const egl::AttributeMap &attribs)
{
    return attribs.getAsInt(EGL_SAMPLE_RANGE_HINT_EXT, EGL_YUV_FULL_RANGE_EXT) ==
                   EGL_YUV_FULL_RANGE_EXT
               ? VK_SAMPLER_YCBCR_RANGE_ITU_FULL
               : VK_SAMPLER_YCBCR_RANGE_ITU_NARROW;
}

angle::Result GetAllocateInfo(const egl::AttributeMap &attribs,
                              VkImage image,
                              uint32_t planeCount,
                              const VkDrmFormatModifierPropertiesEXT &properties,
                              AllocateInfo *infoOut,
                              uint32_t *infoCountOut)
{
    // There are a number of situations:
    //
    // - If the format tilingFeatures does not have the DISJOINT bit, then allocation and bind is
    //   done as usual; the fd is used to create the allocation and vkBindImageMemory is called
    //   without any extra bind info (which would need vkBindImageMemory2).
    // - If the format tilingFeatures does have the DISJOINT bit, but all fds are identical, it's
    //   handled similarly to the non-disjoint case.
    // - Otherwise if there are N planes, there must be N allocations and N binds (one per fd).
    //   When binding, VkBindImagePlaneMemoryInfo is used to identify which plane is being bound.
    //
    constexpr uint32_t kDisjointBit = VK_FORMAT_FEATURE_DISJOINT_BIT;
    bool isDisjoint =
        planeCount > 1 && IsMaskFlagSet(properties.drmFormatModifierTilingFeatures, kDisjointBit);
    if (isDisjoint)
    {
        bool areFdsIdentical = true;
        for (uint32_t plane = 1; plane < planeCount; ++plane)
        {
            if (attribs.getAsInt(kFds[plane]) != attribs.getAsInt(kFds[0]))
            {
                areFdsIdentical = false;
                break;
            }
        }

        // Treat DISJOINT-but-identical-fds as non-disjoint.
        if (areFdsIdentical)
        {
            isDisjoint = false;
        }
    }

    // Fill in allocateInfo, importFdInfo, bindInfo and bindPlaneInfo first.
    *infoCountOut = isDisjoint ? planeCount : 1;
    for (uint32_t plane = 0; plane < *infoCountOut; ++plane)
    {
        infoOut->allocateInfo[plane].sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
        infoOut->allocateInfo[plane].pNext = &infoOut->importFdInfo[plane];
        infoOut->allocateInfo[plane].image = image;

        infoOut->importFdInfo[plane].sType      = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR;
        infoOut->importFdInfo[plane].handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;

        // Vulkan takes ownership of the FD, closed on vkFreeMemory.
        int dfd = fcntl(attribs.getAsInt(kFds[plane]), F_DUPFD_CLOEXEC, 0);
        if (dfd < 0)
        {
            ERR() << "failed to duplicate fd for dma_buf import" << std::endl;
            return angle::Result::Stop;
        }
        infoOut->importFdInfo[plane].fd = dfd;

        infoOut->allocateInfoPtr[plane] = &infoOut->allocateInfo[plane];
    }

    return angle::Result::Continue;
}
}  // anonymous namespace

DmaBufImageSiblingVkLinux::DmaBufImageSiblingVkLinux(const egl::AttributeMap &attribs)
    : mAttribs(attribs),
      mFormat(GL_NONE),
      mVkFormats(),
      mRenderable(false),
      mTextureable(false),
      mYUV(false),
      mSamples(0),
      mImage(nullptr)
{
    ASSERT(mAttribs.contains(EGL_WIDTH));
    ASSERT(mAttribs.contains(EGL_HEIGHT));
    mSize.width  = mAttribs.getAsInt(EGL_WIDTH);
    mSize.height = mAttribs.getAsInt(EGL_HEIGHT);
    mSize.depth  = 1;

    int fourCCFormat = mAttribs.getAsInt(EGL_LINUX_DRM_FOURCC_EXT);
    mFormat          = gl::Format(angle::DrmFourCCFormatToGLInternalFormat(fourCCFormat, &mYUV));
    mVkFormats       = angle::DrmFourCCFormatToVkFormats(fourCCFormat);

    mHasProtectedContent = mAttribs.getAsInt(EGL_PROTECTED_CONTENT_EXT, false);
}

DmaBufImageSiblingVkLinux::~DmaBufImageSiblingVkLinux() {}

egl::Error DmaBufImageSiblingVkLinux::initialize(const egl::Display *display)
{
    DisplayVk *displayVk = vk::GetImpl(display);
    return angle::ToEGL(initImpl(displayVk), EGL_BAD_PARAMETER);
}

VkImageUsageFlags FindSupportedUsageFlagsForFormat(
    vk::Renderer *renderer,
    VkFormat format,
    uint64_t drmModifier,
    VkImageFormatListCreateInfo imageFormatListCreateInfo,
    VkImageUsageFlags usageFlags,
    VkImageCreateFlags createFlags,
    VkImageFormatProperties2 *outImageFormatProperties)
{
    if (!IsFormatSupported(renderer, format, drmModifier, usageFlags, createFlags,
                           imageFormatListCreateInfo, outImageFormatProperties))
    {
        usageFlags &= ~kRenderAndInputUsage;
        if (!IsFormatSupported(renderer, format, drmModifier, usageFlags, createFlags,
                               imageFormatListCreateInfo, outImageFormatProperties))
        {
            usageFlags &= ~kTextureUsage;
            if (!IsFormatSupported(renderer, format, drmModifier, usageFlags, createFlags,
                                   imageFormatListCreateInfo, outImageFormatProperties))
            {
                // Can not find supported usage flags for this image.
                return 0;
            }
        }
    }

    return usageFlags;
}

bool FindSupportedFlagsForFormat(vk::Renderer *renderer,
                                 VkFormat format,
                                 uint64_t drmModifier,
                                 VkImageFormatListCreateInfo imageFormatListCreateInfo,
                                 VkImageUsageFlags *outUsageFlags,
                                 VkImageCreateFlags createFlags,
                                 VkImageFormatProperties2 *outImageFormatProperties)
{
    *outUsageFlags =
        FindSupportedUsageFlagsForFormat(renderer, format, drmModifier, imageFormatListCreateInfo,
                                         *outUsageFlags, createFlags, outImageFormatProperties);
    return *outUsageFlags != 0;
}

angle::Result DmaBufImageSiblingVkLinux::initWithFormat(DisplayVk *displayVk,
                                                        const angle::Format &format,
                                                        VkFormat vulkanFormat,
                                                        MutableFormat mutableFormat,
                                                        InitResult *initResultOut)
{
    *initResultOut         = InitResult::Success;
    vk::Renderer *renderer = displayVk->getRenderer();

    const angle::FormatID intendedFormatID    = vk::GetFormatIDFromVkFormat(vulkanFormat);
    const angle::FormatID actualImageFormatID = vk::GetFormatIDFromVkFormat(vulkanFormat);

    const uint32_t planeCount = GetPlaneCount(mAttribs);

    PerPlane<uint64_t> planeModifiers = {};
    GetModifiers(mAttribs, planeCount, &planeModifiers);

    // The EGL extension allows for each plane to have a different DRM modifier.  This is not
    // allowed in Vulkan, and all hardware past and current require the planes to have the same DRM
    // modifier.  If an application provides different modifiers for the planes, fail.
    const uint64_t plane0Modifier = planeModifiers[0];
    for (uint32_t plane = 0; plane < planeCount; ++plane)
    {
        ANGLE_VK_CHECK(displayVk, planeModifiers[plane] == plane0Modifier,
                       VK_ERROR_INCOMPATIBLE_DRIVER);
    }

    // First, check the possible features for the format and determine usage and create flags.
    VkDrmFormatModifierPropertiesEXT modifierProperties = {};
    if (!GetFormatModifierProperties(displayVk, vulkanFormat, plane0Modifier, &modifierProperties))
    {
        // Format is incompatible
        *initResultOut = InitResult::Failed;
        return angle::Result::Continue;
    }

    VkImageUsageFlags usageFlags =
        GetUsageFlags(renderer, format, modifierProperties, &mTextureable, &mRenderable);

    VkImageCreateFlags createFlags =
        vk::kVkImageCreateFlagsNone | (hasProtectedContent() ? VK_IMAGE_CREATE_PROTECTED_BIT : 0);

    // The Vulkan and EGL plane counts are expected to match.
    ANGLE_VK_CHECK(displayVk, modifierProperties.drmFormatModifierPlaneCount == planeCount,
                   VK_ERROR_INCOMPATIBLE_DRIVER);

    // Verify that such a usage is compatible with the provided modifiers, if any.  If not, try to
    // remove features until it is.
    VkExternalImageFormatProperties externalFormatProperties = {};
    externalFormatProperties.sType = VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES;

    VkImageFormatProperties2 imageFormatProperties = {};
    imageFormatProperties.sType                    = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
    imageFormatProperties.pNext                    = &externalFormatProperties;

    std::vector<VkSubresourceLayout> planes(planeCount, VkSubresourceLayout{});
    for (uint32_t plane = 0; plane < planeCount; ++plane)
    {
        planes[plane].offset   = mAttribs.getAsInt(kOffsets[plane]);
        planes[plane].rowPitch = mAttribs.getAsInt(kPitches[plane]);
    }

    VkImageDrmFormatModifierExplicitCreateInfoEXT imageDrmModifierCreateInfo = {};
    imageDrmModifierCreateInfo.sType =
        VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT;
    imageDrmModifierCreateInfo.drmFormatModifier           = plane0Modifier;
    imageDrmModifierCreateInfo.drmFormatModifierPlaneCount = planeCount;
    imageDrmModifierCreateInfo.pPlaneLayouts               = planes.data();

    VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {};
    externalMemoryImageCreateInfo.sType       = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
    externalMemoryImageCreateInfo.pNext       = &imageDrmModifierCreateInfo;
    externalMemoryImageCreateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;

    VkImageFormatListCreateInfoKHR imageFormatListCreateInfo;
    vk::ImageHelper::ImageListFormats imageListFormatsStorage;
    const void *imageCreateInfoPNext = vk::ImageHelper::DeriveCreateInfoPNext(
        displayVk, usageFlags, actualImageFormatID, &externalMemoryImageCreateInfo,
        &imageFormatListCreateInfo, &imageListFormatsStorage, &createFlags);

    if (mutableFormat == MutableFormat::NotAllowed)
    {
        createFlags &= ~VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
        // When mutable format bit is not set, viewFormatCount must be 0 or 1.
        imageFormatListCreateInfo.viewFormatCount =
            std::min(imageFormatListCreateInfo.viewFormatCount, 1u);
    }

    if (!FindSupportedFlagsForFormat(renderer, vulkanFormat, plane0Modifier,
                                     imageFormatListCreateInfo, &usageFlags, createFlags,
                                     &imageFormatProperties))
    {
        // The image is not unusable with current flags.
        *initResultOut = InitResult::Failed;
        return angle::Result::Continue;
    }
    mRenderable  = usageFlags & kRenderUsage;
    mTextureable = usageFlags & kTextureUsage;

    // Make sure image width/height/samples are within allowed range and the image is importable.
    const bool isWidthValid = static_cast<uint32_t>(mSize.width) <=
                              imageFormatProperties.imageFormatProperties.maxExtent.width;
    const bool isHeightValid = static_cast<uint32_t>(mSize.height) <=
                               imageFormatProperties.imageFormatProperties.maxExtent.height;
    const bool isSampleCountValid =
        (imageFormatProperties.imageFormatProperties.sampleCounts & VK_SAMPLE_COUNT_1_BIT) != 0;
    const bool isMemoryImportable =
        (externalFormatProperties.externalMemoryProperties.externalMemoryFeatures &
         VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT) != 0;
    ANGLE_VK_CHECK(displayVk,
                   isWidthValid && isHeightValid && isSampleCountValid && isMemoryImportable,
                   VK_ERROR_INCOMPATIBLE_DRIVER);

    // Create the image
    mImage = new vk::ImageHelper();

    VkExtent3D vkExtents;
    gl_vk::GetExtent(mSize, &vkExtents);

    constexpr bool kIsRobustInitEnabled = false;

    vk::YcbcrConversionDesc conversionDesc{};
    if (mYUV)
    {
        const VkChromaLocation xChromaOffset =
            GetChromaLocation(mAttribs, EGL_YUV_CHROMA_HORIZONTAL_SITING_HINT_EXT);
        const VkChromaLocation yChromaOffset =
            GetChromaLocation(mAttribs, EGL_YUV_CHROMA_VERTICAL_SITING_HINT_EXT);
        const VkSamplerYcbcrModelConversion model = GetYcbcrModel(mAttribs);
        const VkSamplerYcbcrRange range           = GetYcbcrRange(mAttribs);
        const VkComponentMapping components       = {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
        };

        ANGLE_VK_CHECK(displayVk, renderer->getFeatures().supportsYUVSamplerConversion.enabled,
                       VK_ERROR_FEATURE_NOT_PRESENT);

        const vk::YcbcrLinearFilterSupport linearFilterSupported =
            renderer->hasImageFormatFeatureBits(
                actualImageFormatID,
                VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT)
                ? vk::YcbcrLinearFilterSupport::Supported
                : vk::YcbcrLinearFilterSupport::Unsupported;

        // Build an appropriate conversion desc. This is not an android-style external format,
        // but requires Ycbcr sampler conversion.
        conversionDesc.update(renderer, 0, model, range, xChromaOffset, yChromaOffset,
                              vk::kDefaultYCbCrChromaFilter, components, intendedFormatID,
                              linearFilterSupported);
    }

    ANGLE_TRY(mImage->initExternal(displayVk, gl::TextureType::_2D, vkExtents, intendedFormatID,
                                   actualImageFormatID, 1, usageFlags, createFlags,
                                   vk::ImageLayout::ExternalPreInitialized, imageCreateInfoPNext,
                                   gl::LevelIndex(0), 1, 1, kIsRobustInitEnabled,
                                   hasProtectedContent(), conversionDesc, nullptr));

    VkMemoryRequirements externalMemoryRequirements;
    mImage->getImage().getMemoryRequirements(renderer->getDevice(), &externalMemoryRequirements);

    const VkMemoryPropertyFlags flags =
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
        (hasProtectedContent() ? VK_MEMORY_PROPERTY_PROTECTED_BIT : 0);

    AllocateInfo allocateInfo;
    uint32_t allocateInfoCount;
    ANGLE_TRY(GetAllocateInfo(mAttribs, mImage->getImage().getHandle(), planeCount,
                              modifierProperties, &allocateInfo, &allocateInfoCount));

    return mImage->initExternalMemory(
        displayVk, renderer->getMemoryProperties(), externalMemoryRequirements, allocateInfoCount,
        allocateInfo.allocateInfoPtr.data(), vk::kForeignDeviceQueueIndex, flags);
}

angle::Result DmaBufImageSiblingVkLinux::initImpl(DisplayVk *displayVk)
{
    vk::Renderer *renderer = displayVk->getRenderer();

    const vk::Format &vkFormat  = renderer->getFormat(mFormat.info->sizedInternalFormat);
    const angle::Format &format = vkFormat.getActualImageFormat(rx::vk::ImageAccess::SampleOnly);

    InitResult initResult;

    for (VkFormat vkFmt : mVkFormats)
    {
        // Try all formats with mutable format bit first
        ANGLE_TRY(initWithFormat(displayVk, format, vkFmt, MutableFormat::Allowed, &initResult));
        if (initResult == InitResult::Success)
        {
            return angle::Result::Continue;
        }
    }

    for (VkFormat vkFmt : mVkFormats)
    {
        // Then try without mutable format bit
        ANGLE_TRY(initWithFormat(displayVk, format, vkFmt, MutableFormat::NotAllowed, &initResult));
        if (initResult == InitResult::Success)
        {
            return angle::Result::Continue;
        }
    }

    // Failed to find any suitable format
    ANGLE_VK_UNREACHABLE(displayVk);
    return angle::Result::Stop;
}

void DmaBufImageSiblingVkLinux::onDestroy(const egl::Display *display)
{
    ASSERT(mImage == nullptr);
}

gl::Format DmaBufImageSiblingVkLinux::getFormat() const
{
    return mFormat;
}

bool DmaBufImageSiblingVkLinux::isRenderable(const gl::Context *context) const
{
    return mRenderable;
}

bool DmaBufImageSiblingVkLinux::isTexturable(const gl::Context *context) const
{
    return mTextureable;
}

bool DmaBufImageSiblingVkLinux::isYUV() const
{
    return mYUV;
}

bool DmaBufImageSiblingVkLinux::hasProtectedContent() const
{
    return mHasProtectedContent;
}

gl::Extents DmaBufImageSiblingVkLinux::getSize() const
{
    return mSize;
}

size_t DmaBufImageSiblingVkLinux::getSamples() const
{
    return mSamples;
}

// ExternalImageSiblingVk interface
vk::ImageHelper *DmaBufImageSiblingVkLinux::getImage() const
{
    return mImage;
}

void DmaBufImageSiblingVkLinux::release(vk::Renderer *renderer)
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
