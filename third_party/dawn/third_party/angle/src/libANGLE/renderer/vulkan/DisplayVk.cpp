//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVk.cpp:
//    Implements the class methods for DisplayVk.
//

#include "libANGLE/renderer/vulkan/DisplayVk.h"

#include "common/debug.h"
#include "common/system_utils.h"
#include "libANGLE/BlobCache.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/BufferVk.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/DeviceVk.h"
#include "libANGLE/renderer/vulkan/ImageVk.h"
#include "libANGLE/renderer/vulkan/ShareGroupVk.h"
#include "libANGLE/renderer/vulkan/SurfaceVk.h"
#include "libANGLE/renderer/vulkan/SyncVk.h"
#include "libANGLE/renderer/vulkan/TextureVk.h"
#include "libANGLE/renderer/vulkan/VkImageImageSiblingVk.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{

namespace
{
// Query surface format and colorspace support.
void GetSupportedFormatColorspaces(VkPhysicalDevice physicalDevice,
                                   const angle::FeaturesVk &featuresVk,
                                   VkSurfaceKHR surface,
                                   std::vector<VkSurfaceFormat2KHR> *surfaceFormatsOut)
{
    ASSERT(surfaceFormatsOut);
    surfaceFormatsOut->clear();

    constexpr VkSurfaceFormat2KHR kSurfaceFormat2Initializer = {
        VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR,
        nullptr,
        {VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};

    if (featuresVk.supportsSurfaceCapabilities2Extension.enabled)
    {
        VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo2 = {};
        surfaceInfo2.sType          = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
        surfaceInfo2.surface        = surface;
        uint32_t surfaceFormatCount = 0;

        // Query the count first
        VkResult result = vkGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, &surfaceInfo2,
                                                                &surfaceFormatCount, nullptr);
        ASSERT(result == VK_SUCCESS);
        ASSERT(surfaceFormatCount > 0);

        // Query the VkSurfaceFormat2KHR list
        std::vector<VkSurfaceFormat2KHR> surfaceFormats2(surfaceFormatCount,
                                                         kSurfaceFormat2Initializer);
        result = vkGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, &surfaceInfo2,
                                                       &surfaceFormatCount, surfaceFormats2.data());
        ASSERT(result == VK_SUCCESS);

        *surfaceFormatsOut = std::move(surfaceFormats2);
    }
    else
    {
        uint32_t surfaceFormatCount = 0;
        // Query the count first
        VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface,
                                                               &surfaceFormatCount, nullptr);
        ASSERT(result == VK_SUCCESS);

        // Query the VkSurfaceFormatKHR list
        std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount,
                                                      surfaceFormats.data());
        ASSERT(result == VK_SUCCESS);

        // Copy over data from std::vector<VkSurfaceFormatKHR> to std::vector<VkSurfaceFormat2KHR>
        std::vector<VkSurfaceFormat2KHR> surfaceFormats2(surfaceFormatCount,
                                                         kSurfaceFormat2Initializer);
        for (size_t index = 0; index < surfaceFormatCount; index++)
        {
            surfaceFormats2[index].surfaceFormat.format = surfaceFormats[index].format;
        }

        *surfaceFormatsOut = std::move(surfaceFormats2);
    }
}

vk::UseDebugLayers ShouldLoadDebugLayers(const egl::AttributeMap &attribs)
{
    EGLAttrib debugSetting =
        attribs.get(EGL_PLATFORM_ANGLE_DEBUG_LAYERS_ENABLED_ANGLE, EGL_DONT_CARE);

#if defined(ANGLE_ENABLE_VULKAN_VALIDATION_LAYERS_BY_DEFAULT)
    const bool yes = ShouldUseDebugLayers(attribs);
#else
    const bool yes = debugSetting == EGL_TRUE;
#endif  // defined(ANGLE_ENABLE_VULKAN_VALIDATION_LAYERS_BY_DEFAULT)

    const bool ifAvailable = debugSetting == EGL_DONT_CARE;

    return yes && ifAvailable ? vk::UseDebugLayers::YesIfAvailable
           : yes              ? vk::UseDebugLayers::Yes
                              : vk::UseDebugLayers::No;
}

angle::vk::ICD ChooseICDFromAttribs(const egl::AttributeMap &attribs)
{
#if !defined(ANGLE_PLATFORM_ANDROID)
    // Mock ICD does not currently run on Android
    EGLAttrib deviceType = attribs.get(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE,
                                       EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE);

    switch (deviceType)
    {
        case EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE:
            break;
        case EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE:
            return angle::vk::ICD::Mock;
        case EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE:
            return angle::vk::ICD::SwiftShader;
        default:
            UNREACHABLE();
            break;
    }
#endif  // !defined(ANGLE_PLATFORM_ANDROID)

    return angle::vk::ICD::Default;
}

void InstallDebugAnnotator(egl::Display *display, vk::Renderer *renderer)
{
    bool installedAnnotator = false;

    // Ensure the appropriate global DebugAnnotator is used
    ASSERT(renderer);
    renderer->setGlobalDebugAnnotator(&installedAnnotator);

    if (!installedAnnotator)
    {
        std::unique_lock<angle::SimpleMutex> lock(gl::GetDebugMutex());
        display->setGlobalDebugAnnotator();
    }
}
}  // namespace

DisplayVk::DisplayVk(const egl::DisplayState &state)
    : DisplayImpl(state),
      vk::ErrorContext(new vk::Renderer()),
      mScratchBuffer(1000u),
      mSupportedColorspaceFormatsMap{}
{}

DisplayVk::~DisplayVk()
{
    delete mRenderer;
}

egl::Error DisplayVk::initialize(egl::Display *display)
{
    ASSERT(mRenderer != nullptr && display != nullptr);
    const egl::AttributeMap &attribs = display->getAttributeMap();

    const vk::UseDebugLayers useDebugLayers = ShouldLoadDebugLayers(attribs);
    const angle::vk::ICD desiredICD         = ChooseICDFromAttribs(attribs);
    const uint32_t preferredVendorId =
        static_cast<uint32_t>(attribs.get(EGL_PLATFORM_ANGLE_DEVICE_ID_HIGH_ANGLE, 0));
    const uint32_t preferredDeviceId =
        static_cast<uint32_t>(attribs.get(EGL_PLATFORM_ANGLE_DEVICE_ID_LOW_ANGLE, 0));
    const uint8_t *preferredDeviceUuid = reinterpret_cast<const uint8_t *>(
        attribs.get(EGL_PLATFORM_ANGLE_VULKAN_DEVICE_UUID_ANGLE, 0));
    const uint8_t *preferredDriverUuid = reinterpret_cast<const uint8_t *>(
        attribs.get(EGL_PLATFORM_ANGLE_VULKAN_DRIVER_UUID_ANGLE, 0));
    const VkDriverId preferredDriverId =
        static_cast<VkDriverId>(attribs.get(EGL_PLATFORM_ANGLE_VULKAN_DRIVER_ID_ANGLE, 0));

    angle::Result result = mRenderer->initialize(
        this, this, desiredICD, preferredVendorId, preferredDeviceId, preferredDeviceUuid,
        preferredDriverUuid, preferredDriverId, useDebugLayers, getWSIExtension(), getWSILayer(),
        getWindowSystem(), mState.featureOverrides);
    ANGLE_TRY(angle::ToEGL(result, EGL_NOT_INITIALIZED));

    mDeviceQueueIndex = mRenderer->getDeviceQueueIndex(egl::ContextPriority::Medium);

    InstallDebugAnnotator(display, mRenderer);

    // Query and cache supported surface format and colorspace for later use.
    initSupportedSurfaceFormatColorspaces();
    return egl::NoError();
}

void DisplayVk::terminate()
{
    mRenderer->reloadVolkIfNeeded();

    ASSERT(mRenderer);
    mRenderer->onDestroy(this);
}

egl::Error DisplayVk::makeCurrent(egl::Display *display,
                                  egl::Surface * /*drawSurface*/,
                                  egl::Surface * /*readSurface*/,
                                  gl::Context * /*context*/)
{
    InstallDebugAnnotator(display, mRenderer);
    return egl::NoError();
}

bool DisplayVk::testDeviceLost()
{
    return mRenderer->isDeviceLost();
}

egl::Error DisplayVk::restoreLostDevice(const egl::Display *display)
{
    // A vulkan device cannot be restored, the entire renderer would have to be re-created along
    // with any other EGL objects that reference it.
    return egl::EglBadDisplay();
}

std::string DisplayVk::getRendererDescription()
{
    if (mRenderer)
    {
        return mRenderer->getRendererDescription();
    }
    return std::string();
}

std::string DisplayVk::getVendorString()
{
    if (mRenderer)
    {
        return mRenderer->getVendorString();
    }
    return std::string();
}

std::string DisplayVk::getVersionString(bool includeFullVersion)
{
    if (mRenderer)
    {
        return mRenderer->getVersionString(includeFullVersion);
    }
    return std::string();
}

DeviceImpl *DisplayVk::createDevice()
{
    return new DeviceVk();
}

egl::Error DisplayVk::waitClient(const gl::Context *context)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "DisplayVk::waitClient");
    ContextVk *contextVk = vk::GetImpl(context);
    return angle::ToEGL(contextVk->finishImpl(RenderPassClosureReason::EGLWaitClient),
                        EGL_BAD_ACCESS);
}

egl::Error DisplayVk::waitNative(const gl::Context *context, EGLint engine)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "DisplayVk::waitNative");
    return angle::ResultToEGL(waitNativeImpl());
}

angle::Result DisplayVk::waitNativeImpl()
{
    return angle::Result::Continue;
}

SurfaceImpl *DisplayVk::createWindowSurface(const egl::SurfaceState &state,
                                            EGLNativeWindowType window,
                                            const egl::AttributeMap &attribs)
{
    return createWindowSurfaceVk(state, window);
}

SurfaceImpl *DisplayVk::createPbufferSurface(const egl::SurfaceState &state,
                                             const egl::AttributeMap &attribs)
{
    ASSERT(mRenderer);
    return new OffscreenSurfaceVk(state, mRenderer);
}

SurfaceImpl *DisplayVk::createPbufferFromClientBuffer(const egl::SurfaceState &state,
                                                      EGLenum buftype,
                                                      EGLClientBuffer clientBuffer,
                                                      const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return static_cast<SurfaceImpl *>(0);
}

SurfaceImpl *DisplayVk::createPixmapSurface(const egl::SurfaceState &state,
                                            NativePixmapType nativePixmap,
                                            const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return static_cast<SurfaceImpl *>(0);
}

ImageImpl *DisplayVk::createImage(const egl::ImageState &state,
                                  const gl::Context *context,
                                  EGLenum target,
                                  const egl::AttributeMap &attribs)
{
    return new ImageVk(state, context);
}

ShareGroupImpl *DisplayVk::createShareGroup(const egl::ShareGroupState &state)
{
    return new ShareGroupVk(state, mRenderer);
}

bool DisplayVk::isConfigFormatSupported(VkFormat format) const
{
    // Requires VK_GOOGLE_surfaceless_query extension to be supported.
    ASSERT(mRenderer->getFeatures().supportsSurfacelessQueryExtension.enabled);

    // A format is considered supported if it is supported in atleast 1 colorspace.
    using ColorspaceFormatSetItem =
        const std::pair<const VkColorSpaceKHR, std::unordered_set<VkFormat>>;
    for (ColorspaceFormatSetItem &colorspaceFormatSetItem : mSupportedColorspaceFormatsMap)
    {
        if (colorspaceFormatSetItem.second.count(format) > 0)
        {
            return true;
        }
    }

    return false;
}

bool DisplayVk::isSurfaceFormatColorspacePairSupported(VkSurfaceKHR surface,
                                                       VkFormat format,
                                                       VkColorSpaceKHR colorspace) const
{
    if (mSupportedColorspaceFormatsMap.size() > 0)
    {
        return mSupportedColorspaceFormatsMap.count(colorspace) > 0 &&
               mSupportedColorspaceFormatsMap.at(colorspace).count(format) > 0;
    }
    else
    {
        const angle::FeaturesVk &featuresVk = mRenderer->getFeatures();
        std::vector<VkSurfaceFormat2KHR> surfaceFormats;
        GetSupportedFormatColorspaces(mRenderer->getPhysicalDevice(), featuresVk, surface,
                                      &surfaceFormats);

        if (!featuresVk.supportsSurfaceCapabilities2Extension.enabled)
        {
            if (surfaceFormats.size() == 1u &&
                surfaceFormats[0].surfaceFormat.format == VK_FORMAT_UNDEFINED)
            {
                return true;
            }
        }

        for (const VkSurfaceFormat2KHR &surfaceFormat : surfaceFormats)
        {
            if (surfaceFormat.surfaceFormat.format == format &&
                surfaceFormat.surfaceFormat.colorSpace == colorspace)
            {
                return true;
            }
        }
    }

    return false;
}

bool DisplayVk::isColorspaceSupported(VkColorSpaceKHR colorspace) const
{
    return mSupportedColorspaceFormatsMap.count(colorspace) > 0;
}

void DisplayVk::initSupportedSurfaceFormatColorspaces()
{
    const angle::FeaturesVk &featuresVk = mRenderer->getFeatures();
    if (featuresVk.supportsSurfacelessQueryExtension.enabled &&
        featuresVk.supportsSurfaceCapabilities2Extension.enabled)
    {
        // Use the VK_GOOGLE_surfaceless_query extension to query supported surface formats and
        // colorspaces by using a VK_NULL_HANDLE for the VkSurfaceKHR handle.
        std::vector<VkSurfaceFormat2KHR> surfaceFormats;
        GetSupportedFormatColorspaces(mRenderer->getPhysicalDevice(), featuresVk, VK_NULL_HANDLE,
                                      &surfaceFormats);
        for (const VkSurfaceFormat2KHR &surfaceFormat : surfaceFormats)
        {
            // Cache supported VkFormat and VkColorSpaceKHR for later use
            VkFormat format            = surfaceFormat.surfaceFormat.format;
            VkColorSpaceKHR colorspace = surfaceFormat.surfaceFormat.colorSpace;

            ASSERT(format != VK_FORMAT_UNDEFINED);

            mSupportedColorspaceFormatsMap[colorspace].insert(format);
        }

        ASSERT(mSupportedColorspaceFormatsMap.size() > 0);
    }
    else
    {
        mSupportedColorspaceFormatsMap.clear();
    }
}

ContextImpl *DisplayVk::createContext(const gl::State &state,
                                      gl::ErrorSet *errorSet,
                                      const egl::Config *configuration,
                                      const gl::Context *shareContext,
                                      const egl::AttributeMap &attribs)
{
    return new ContextVk(state, errorSet, mRenderer);
}

StreamProducerImpl *DisplayVk::createStreamProducerD3DTexture(
    egl::Stream::ConsumerType consumerType,
    const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return static_cast<StreamProducerImpl *>(0);
}

EGLSyncImpl *DisplayVk::createSync()
{
    return new EGLSyncVk();
}

gl::Version DisplayVk::getMaxSupportedESVersion() const
{
    return mRenderer->getMaxSupportedESVersion();
}

gl::Version DisplayVk::getMaxConformantESVersion() const
{
    return mRenderer->getMaxConformantESVersion();
}

egl::Error DisplayVk::validateImageClientBuffer(const gl::Context *context,
                                                EGLenum target,
                                                EGLClientBuffer clientBuffer,
                                                const egl::AttributeMap &attribs) const
{
    switch (target)
    {
        case EGL_VULKAN_IMAGE_ANGLE:
        {
            VkImage *vkImage = reinterpret_cast<VkImage *>(clientBuffer);
            if (!vkImage || *vkImage == VK_NULL_HANDLE)
            {
                return egl::EglBadParameter() << "clientBuffer is invalid.";
            }

            GLenum internalFormat =
                static_cast<GLenum>(attribs.get(EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_NONE));
            switch (internalFormat)
            {
                case GL_RGBA:
                case GL_BGRA_EXT:
                case GL_RGB:
                case GL_RED_EXT:
                case GL_RG_EXT:
                case GL_RGB10_A2_EXT:
                case GL_R16_EXT:
                case GL_RG16_EXT:
                case GL_NONE:
                    break;
                default:
                    return egl::EglBadParameter() << "Invalid EGLImage texture internal format: 0x"
                                                  << std::hex << internalFormat;
            }

            uint64_t hi = static_cast<uint64_t>(attribs.get(EGL_VULKAN_IMAGE_CREATE_INFO_HI_ANGLE));
            uint64_t lo = static_cast<uint64_t>(attribs.get(EGL_VULKAN_IMAGE_CREATE_INFO_LO_ANGLE));
            uint64_t info = ((hi & 0xffffffff) << 32) | (lo & 0xffffffff);
            if (reinterpret_cast<const VkImageCreateInfo *>(info)->sType !=
                VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO)
            {
                return egl::EglBadParameter()
                       << "EGL_VULKAN_IMAGE_CREATE_INFO_HI_ANGLE and "
                          "EGL_VULKAN_IMAGE_CREATE_INFO_LO_ANGLE are not pointing to a "
                          "valid VkImageCreateInfo structure.";
            }

            return egl::NoError();
        }
        default:
            return DisplayImpl::validateImageClientBuffer(context, target, clientBuffer, attribs);
    }
}

ExternalImageSiblingImpl *DisplayVk::createExternalImageSibling(const gl::Context *context,
                                                                EGLenum target,
                                                                EGLClientBuffer buffer,
                                                                const egl::AttributeMap &attribs)
{
    switch (target)
    {
        case EGL_VULKAN_IMAGE_ANGLE:
            return new VkImageImageSiblingVk(buffer, attribs);
        default:
            return DisplayImpl::createExternalImageSibling(context, target, buffer, attribs);
    }
}

void DisplayVk::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
    outExtensions->createContextRobustness  = getRenderer()->getNativeExtensions().robustnessAny();
    outExtensions->surfaceOrientation       = true;
    outExtensions->displayTextureShareGroup = true;
    outExtensions->displaySemaphoreShareGroup        = true;
    outExtensions->robustResourceInitializationANGLE = true;

    // The Vulkan implementation will always say that EGL_KHR_swap_buffers_with_damage is supported.
    // When the Vulkan driver supports VK_KHR_incremental_present, it will use it.  Otherwise, it
    // will ignore the hint and do a regular swap.
    outExtensions->swapBuffersWithDamage = true;

    outExtensions->fenceSync            = true;
    outExtensions->waitSync             = true;
    outExtensions->globalFenceSyncANGLE = true;

    outExtensions->image                 = true;
    outExtensions->imageBase             = true;
    outExtensions->imagePixmap           = false;  // ANGLE does not support pixmaps
    outExtensions->glTexture2DImage      = true;
    outExtensions->glTextureCubemapImage = true;
    outExtensions->glTexture3DImage      = getFeatures().supportsSampler2dViewOf3d.enabled;
    outExtensions->glRenderbufferImage = true;
    outExtensions->imageNativeBuffer     = getFeatures().supportsAndroidHardwareBuffer.enabled;
    outExtensions->surfacelessContext = true;
    outExtensions->glColorspace       = true;
    outExtensions->imageGlColorspace =
        outExtensions->glColorspace && getFeatures().supportsImageFormatList.enabled;

#if defined(ANGLE_PLATFORM_ANDROID)
    outExtensions->getNativeClientBufferANDROID = true;
    outExtensions->framebufferTargetANDROID     = true;

    // Only expose EGL_ANDROID_front_buffer_auto_refresh on Android and when Vulkan supports
    // VK_EXT_swapchain_maintenance1 (supportsSwapchainMaintenance1 feature), since we know that
    // VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR and VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR
    // are compatible on Android (does not require swapchain recreation).
    outExtensions->frontBufferAutoRefreshANDROID =
        getFeatures().supportsSwapchainMaintenance1.enabled;
#endif  // defined(ANGLE_PLATFORM_ANDROID)

    // EGL_EXT_image_dma_buf_import is only exposed if EGL_EXT_image_dma_buf_import_modifiers can
    // also be exposed.  The Vulkan extensions that support these EGL extensions are not split in
    // the same way; both Vulkan extensions are needed for EGL_EXT_image_dma_buf_import, and with
    // both Vulkan extensions, EGL_EXT_image_dma_buf_import_modifiers is also supportable.
    outExtensions->imageDmaBufImportEXT =
        getFeatures().supportsExternalMemoryDmaBufAndModifiers.enabled;
    outExtensions->imageDmaBufImportModifiersEXT = outExtensions->imageDmaBufImportEXT;

    // Disable context priority when non-zero memory init is enabled. This enforces a queue order.
    outExtensions->contextPriority = !getFeatures().allocateNonZeroMemory.enabled;
    outExtensions->noConfigContext = true;

#if defined(ANGLE_PLATFORM_ANDROID) || defined(ANGLE_PLATFORM_LINUX)
    outExtensions->nativeFenceSyncANDROID = getFeatures().supportsAndroidNativeFenceSync.enabled;
#endif  // defined(ANGLE_PLATFORM_ANDROID) || defined(ANGLE_PLATFORM_LINUX)

#if defined(ANGLE_PLATFORM_GGP)
    outExtensions->ggpStreamDescriptor = true;
    outExtensions->swapWithFrameToken  = getFeatures().supportsGGPFrameToken.enabled;
#endif  // defined(ANGLE_PLATFORM_GGP)

    outExtensions->bufferAgeEXT = true;

    outExtensions->protectedContentEXT = (getFeatures().supportsProtectedMemory.enabled &&
                                          getFeatures().supportsSurfaceProtectedSwapchains.enabled);

    outExtensions->createSurfaceSwapIntervalANGLE = true;

    outExtensions->mutableRenderBufferKHR =
        getFeatures().supportsSharedPresentableImageExtension.enabled;

    outExtensions->vulkanImageANGLE = true;

    outExtensions->lockSurface3KHR = getFeatures().supportsLockSurfaceExtension.enabled;

    outExtensions->partialUpdateKHR = true;

    outExtensions->timestampSurfaceAttributeANGLE =
        getFeatures().supportsTimestampSurfaceAttribute.enabled;

    outExtensions->eglColorspaceAttributePassthroughANGLE =
        outExtensions->glColorspace && getFeatures().eglColorspaceAttributePassthrough.enabled;

    // If EGL_KHR_gl_colorspace extension is supported check if other colorspace extensions
    // can be supported as well.
    if (outExtensions->glColorspace)
    {
        if (isColorspaceSupported(VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT))
        {
            outExtensions->glColorspaceDisplayP3            = true;
            outExtensions->glColorspaceDisplayP3Passthrough = true;
        }

        outExtensions->glColorspaceDisplayP3Linear =
            isColorspaceSupported(VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT);
        outExtensions->glColorspaceScrgb =
            isColorspaceSupported(VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT);
        outExtensions->glColorspaceScrgbLinear =
            isColorspaceSupported(VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT);
        outExtensions->glColorspaceBt2020Linear =
            isColorspaceSupported(VK_COLOR_SPACE_BT2020_LINEAR_EXT);
        outExtensions->glColorspaceBt2020Pq =
            isColorspaceSupported(VK_COLOR_SPACE_HDR10_ST2084_EXT);
        outExtensions->glColorspaceBt2020Hlg = isColorspaceSupported(VK_COLOR_SPACE_HDR10_HLG_EXT);
    }

    outExtensions->surfaceCompressionEXT =
        getFeatures().supportsImageCompressionControlSwapchain.enabled;
}

void DisplayVk::generateCaps(egl::Caps *outCaps) const
{
    outCaps->textureNPOT = true;
    outCaps->stencil8    = getRenderer()->getNativeExtensions().textureStencil8OES;
}

const char *DisplayVk::getWSILayer() const
{
    return nullptr;
}

void DisplayVk::handleError(VkResult result,
                            const char *file,
                            const char *function,
                            unsigned int line)
{
    ASSERT(result != VK_SUCCESS);

    std::stringstream errorStream;
    errorStream << "Internal Vulkan error (" << result << "): " << VulkanResultString(result)
                << ", in " << file << ", " << function << ":" << line << ".";
    std::string errorString = errorStream.str();

    if (result == VK_ERROR_DEVICE_LOST)
    {
        WARN() << errorString;
        mRenderer->notifyDeviceLost();
    }

    // Note: the errorCode will be set later in angle::ToEGL where it's available.
    *egl::Display::GetCurrentThreadErrorScratchSpace() = egl::Error(0, 0, std::move(errorString));
}

void DisplayVk::initializeFrontendFeatures(angle::FrontendFeatures *features) const
{
    mRenderer->initializeFrontendFeatures(features);
}

void DisplayVk::populateFeatureList(angle::FeatureList *features)
{
    mRenderer->getFeatures().populateFeatureList(features);
}

// vk::GlobalOps
void DisplayVk::putBlob(const angle::BlobCacheKey &key, const angle::MemoryBuffer &value)
{
    getBlobCache()->putApplication(nullptr, key, value);
}

bool DisplayVk::getBlob(const angle::BlobCacheKey &key, angle::BlobCacheValue *valueOut)
{
    return getBlobCache()->get(nullptr, &mScratchBuffer, key, valueOut);
}

std::shared_ptr<angle::WaitableEvent> DisplayVk::postMultiThreadWorkerTask(
    const std::shared_ptr<angle::Closure> &task)
{
    return mState.multiThreadPool->postWorkerTask(task);
}

void DisplayVk::notifyDeviceLost()
{
    mState.notifyDeviceLost();
}

void DisplayVk::lockVulkanQueue()
{
    mRenderer->lockVulkanQueueForExternalAccess();
}

void DisplayVk::unlockVulkanQueue()
{
    mRenderer->unlockVulkanQueueForExternalAccess();
}

egl::Error DisplayVk::querySupportedCompressionRates(const egl::Config *configuration,
                                                     const egl::AttributeMap &attributes,
                                                     EGLint *rates,
                                                     EGLint rate_size,
                                                     EGLint *num_rates) const
{
    ASSERT(mRenderer->getFeatures().supportsImageCompressionControl.enabled);
    ASSERT(mRenderer->getFeatures().supportsImageCompressionControlSwapchain.enabled);

    if (rate_size == 0 || rates == nullptr)
    {
        *num_rates = 0;
        return egl::NoError();
    }

    const vk::Format &format = mRenderer->getFormat(configuration->renderTargetFormat);

    VkImageCompressionControlEXT compressionInfo = {};
    compressionInfo.sType                        = VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_CONTROL_EXT;
    compressionInfo.flags                        = VK_IMAGE_COMPRESSION_FIXED_RATE_DEFAULT_EXT;
    compressionInfo.compressionControlPlaneCount = 1;

    VkPhysicalDeviceImageFormatInfo2 imageFormatInfo = {};
    imageFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
    imageFormatInfo.pNext = &compressionInfo;
    imageFormatInfo.format =
        vk::GetVkFormatFromFormatID(mRenderer, format.getActualRenderableImageFormatID());
    imageFormatInfo.type   = VK_IMAGE_TYPE_2D;
    imageFormatInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageFormatInfo.usage  = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                            VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

    VkImageCompressionPropertiesEXT compressionProperties = {};
    compressionProperties.sType = VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_PROPERTIES_EXT;

    VkImageFormatProperties2 imageFormatProperties2 = {};
    imageFormatProperties2.sType                    = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
    imageFormatProperties2.pNext                    = &compressionProperties;

    VkResult result = vkGetPhysicalDeviceImageFormatProperties2(
        mRenderer->getPhysicalDevice(), &imageFormatInfo, &imageFormatProperties2);

    if (result == VK_ERROR_FORMAT_NOT_SUPPORTED)
    {
        *num_rates = 0;
        return egl::NoError();
    }
    else if (result == VK_ERROR_OUT_OF_HOST_MEMORY || result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
    {
        return egl::EglBadAlloc();
    }
    else if (result != VK_SUCCESS)
    {
        return egl::EglBadAccess();
    }

    std::vector<EGLint> eglFixedRates = vk_gl::ConvertCompressionFlagsToEGLFixedRate(
        compressionProperties.imageCompressionFixedRateFlags, static_cast<size_t>(rate_size));
    std::copy(eglFixedRates.begin(), eglFixedRates.end(), rates);
    *num_rates = static_cast<EGLint>(eglFixedRates.size());

    return egl::NoError();
}

}  // namespace rx
