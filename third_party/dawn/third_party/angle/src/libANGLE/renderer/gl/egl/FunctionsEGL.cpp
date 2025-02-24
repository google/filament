//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FunctionsEGL.cpp: Implements the FunctionsEGL class.

#include "libANGLE/renderer/gl/egl/FunctionsEGL.h"

#include <algorithm>

#include "common/platform.h"
#include "common/string_utils.h"
#include "libANGLE/renderer/driver_utils.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/egl/functionsegl_typedefs.h"

#if defined(ANGLE_HAS_LIBDRM)
// clang-format off
#include <fcntl.h>
#include <unistd.h>
#include <xf86drm.h>
// clang-format on
#endif  // defined(ANGLE_HAS_LIBDRM)

namespace
{

template <typename T>
bool SetPtr(T *dst, void *src)
{
    if (src)
    {
        *dst = reinterpret_cast<T>(src);
        return true;
    }
    return false;
}

bool IsValidPlatformTypeForPlatformDisplayConnection(EGLAttrib platformType)
{
    switch (platformType)
    {
        case EGL_PLATFORM_SURFACELESS_MESA:
            return true;
        default:
            break;
    }
    return false;
}

}  // namespace

namespace rx
{

struct FunctionsEGL::EGLDispatchTable
{
    EGLDispatchTable()
        : bindAPIPtr(nullptr),
          chooseConfigPtr(nullptr),
          createContextPtr(nullptr),
          createPbufferSurfacePtr(nullptr),
          createWindowSurfacePtr(nullptr),
          destroyContextPtr(nullptr),
          destroySurfacePtr(nullptr),
          getConfigAttribPtr(nullptr),
          getConfigsPtr(nullptr),
          getCurrentSurfacePtr(nullptr),
          getDisplayPtr(nullptr),
          getErrorPtr(nullptr),
          initializePtr(nullptr),
          makeCurrentPtr(nullptr),
          queryStringPtr(nullptr),
          querySurfacePtr(nullptr),
          swapBuffersPtr(nullptr),
          terminatePtr(nullptr),

          bindTexImagePtr(nullptr),
          releaseTexImagePtr(nullptr),
          surfaceAttribPtr(nullptr),
          swapIntervalPtr(nullptr),

          getCurrentContextPtr(nullptr),

          createImageKHRPtr(nullptr),
          destroyImageKHRPtr(nullptr),

          createSyncKHRPtr(nullptr),
          destroySyncKHRPtr(nullptr),
          clientWaitSyncKHRPtr(nullptr),
          getSyncAttribKHRPtr(nullptr),

          waitSyncKHRPtr(nullptr),

          swapBuffersWithDamageKHRPtr(nullptr),

          presentationTimeANDROIDPtr(nullptr),

          setBlobCacheFuncsANDROIDPtr(nullptr),

          getCompositorTimingSupportedANDROIDPtr(nullptr),
          getCompositorTimingANDROIDPtr(nullptr),
          getNextFrameIdANDROIDPtr(nullptr),
          getFrameTimestampSupportedANDROIDPtr(nullptr),
          getFrameTimestampsANDROIDPtr(nullptr),

          dupNativeFenceFDANDROIDPtr(nullptr),

          queryDmaBufFormatsEXTPtr(nullptr),
          queryDmaBufModifiersEXTPtr(nullptr),

          queryDeviceAttribEXTPtr(nullptr),
          queryDeviceStringEXTPtr(nullptr),
          queryDisplayAttribEXTPtr(nullptr)
    {}

    // 1.0
    PFNEGLBINDAPIPROC bindAPIPtr;
    PFNEGLCHOOSECONFIGPROC chooseConfigPtr;
    PFNEGLCREATECONTEXTPROC createContextPtr;
    PFNEGLCREATEPBUFFERSURFACEPROC createPbufferSurfacePtr;
    PFNEGLCREATEWINDOWSURFACEPROC createWindowSurfacePtr;
    PFNEGLDESTROYCONTEXTPROC destroyContextPtr;
    PFNEGLDESTROYSURFACEPROC destroySurfacePtr;
    PFNEGLGETCONFIGATTRIBPROC getConfigAttribPtr;
    PFNEGLGETCONFIGSPROC getConfigsPtr;
    PFNEGLGETCURRENTSURFACEPROC getCurrentSurfacePtr;
    PFNEGLGETDISPLAYPROC getDisplayPtr;
    PFNEGLGETERRORPROC getErrorPtr;
    PFNEGLINITIALIZEPROC initializePtr;
    PFNEGLMAKECURRENTPROC makeCurrentPtr;
    PFNEGLQUERYSTRINGPROC queryStringPtr;
    PFNEGLQUERYSURFACEPROC querySurfacePtr;
    PFNEGLSWAPBUFFERSPROC swapBuffersPtr;
    PFNEGLTERMINATEPROC terminatePtr;

    // 1.1
    PFNEGLBINDTEXIMAGEPROC bindTexImagePtr;
    PFNEGLRELEASETEXIMAGEPROC releaseTexImagePtr;
    PFNEGLSURFACEATTRIBPROC surfaceAttribPtr;
    PFNEGLSWAPINTERVALPROC swapIntervalPtr;

    // 1.4
    PFNEGLGETCURRENTCONTEXTPROC getCurrentContextPtr;

    // EGL_KHR_image
    PFNEGLCREATEIMAGEKHRPROC createImageKHRPtr;
    PFNEGLDESTROYIMAGEKHRPROC destroyImageKHRPtr;

    // EGL_KHR_fence_sync
    PFNEGLCREATESYNCKHRPROC createSyncKHRPtr;
    PFNEGLDESTROYSYNCKHRPROC destroySyncKHRPtr;
    PFNEGLCLIENTWAITSYNCKHRPROC clientWaitSyncKHRPtr;
    PFNEGLGETSYNCATTRIBKHRPROC getSyncAttribKHRPtr;

    // EGL_KHR_wait_sync
    PFNEGLWAITSYNCKHRPROC waitSyncKHRPtr;

    // EGL_KHR_swap_buffers_with_damage
    PFNEGLSWAPBUFFERSWITHDAMAGEKHRPROC swapBuffersWithDamageKHRPtr;

    // EGL_ANDROID_presentation_time
    PFNEGLPRESENTATIONTIMEANDROIDPROC presentationTimeANDROIDPtr;

    // EGL_ANDROID_blob_cache
    PFNEGLSETBLOBCACHEFUNCSANDROIDPROC setBlobCacheFuncsANDROIDPtr;

    // EGL_ANDROID_get_frame_timestamps
    PFNEGLGETCOMPOSITORTIMINGSUPPORTEDANDROIDPROC getCompositorTimingSupportedANDROIDPtr;
    PFNEGLGETCOMPOSITORTIMINGANDROIDPROC getCompositorTimingANDROIDPtr;
    PFNEGLGETNEXTFRAMEIDANDROIDPROC getNextFrameIdANDROIDPtr;
    PFNEGLGETFRAMETIMESTAMPSUPPORTEDANDROIDPROC getFrameTimestampSupportedANDROIDPtr;
    PFNEGLGETFRAMETIMESTAMPSANDROIDPROC getFrameTimestampsANDROIDPtr;

    // EGL_ANDROID_native_fence_sync
    PFNEGLDUPNATIVEFENCEFDANDROIDPROC dupNativeFenceFDANDROIDPtr;

    // EGL_EXT_image_dma_buf_import_modifiers
    PFNEGLQUERYDMABUFFORMATSEXTPROC queryDmaBufFormatsEXTPtr;
    PFNEGLQUERYDMABUFMODIFIERSEXTPROC queryDmaBufModifiersEXTPtr;

    // EGL_EXT_device_query
    PFNEGLQUERYDEVICEATTRIBEXTPROC queryDeviceAttribEXTPtr;
    PFNEGLQUERYDEVICESTRINGEXTPROC queryDeviceStringEXTPtr;
    PFNEGLQUERYDISPLAYATTRIBEXTPROC queryDisplayAttribEXTPtr;
};

FunctionsEGL::FunctionsEGL()
    : majorVersion(0), minorVersion(0), mFnPtrs(new EGLDispatchTable()), mEGLDisplay(EGL_NO_DISPLAY)
{}

FunctionsEGL::~FunctionsEGL()
{
    SafeDelete(mFnPtrs);
}

egl::Error FunctionsEGL::initialize(EGLAttrib platformType, EGLNativeDisplayType nativeDisplay)
{
#define ANGLE_GET_PROC_OR_WARNING(MEMBER, NAME)                \
    do                                                         \
    {                                                          \
        if (!SetPtr(MEMBER, getProcAddress(#NAME)))            \
        {                                                      \
            WARN() << "Could not load EGL entry point " #NAME; \
        }                                                      \
    } while (0)
#define ANGLE_GET_PROC_OR_ERROR(MEMBER, NAME)                                           \
    do                                                                                  \
    {                                                                                   \
        if (!SetPtr(MEMBER, getProcAddress(#NAME)))                                     \
        {                                                                               \
            return egl::EglNotInitialized() << "Could not load EGL entry point " #NAME; \
        }                                                                               \
    } while (0)

    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->bindAPIPtr, eglBindAPI);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->chooseConfigPtr, eglChooseConfig);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->createContextPtr, eglCreateContext);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->createPbufferSurfacePtr, eglCreatePbufferSurface);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->createWindowSurfacePtr, eglCreateWindowSurface);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->destroyContextPtr, eglDestroyContext);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->destroySurfacePtr, eglDestroySurface);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->getConfigAttribPtr, eglGetConfigAttrib);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->getConfigsPtr, eglGetConfigs);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->getCurrentSurfacePtr, eglGetCurrentSurface);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->getDisplayPtr, eglGetDisplay);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->getErrorPtr, eglGetError);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->initializePtr, eglInitialize);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->makeCurrentPtr, eglMakeCurrent);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->queryStringPtr, eglQueryString);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->querySurfacePtr, eglQuerySurface);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->swapBuffersPtr, eglSwapBuffers);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->terminatePtr, eglTerminate);

    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->bindTexImagePtr, eglBindTexImage);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->releaseTexImagePtr, eglReleaseTexImage);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->surfaceAttribPtr, eglSurfaceAttrib);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->swapIntervalPtr, eglSwapInterval);

    // Querying EGL_EXTENSIONS string and loading it into the mExtensions
    // vector will at this point retrieve the client extensions since mEGLDisplay is still
    // EGL_NO_DISPLAY. This is desired, and mExtensions will later be reinitialized with the display
    // extensions once the display is created and initialized.
    queryExtensions();

#if defined(ANGLE_HAS_LIBDRM)
    mEGLDisplay = getPreferredDisplay(&majorVersion, &minorVersion);
#endif  // defined(ANGLE_HAS_LIBDRM)

    if (mEGLDisplay == EGL_NO_DISPLAY)
    {
        if (IsValidPlatformTypeForPlatformDisplayConnection(platformType))
        {
            mEGLDisplay = getPlatformDisplay(platformType, nativeDisplay);
        }
        else
        {
            mEGLDisplay = mFnPtrs->getDisplayPtr(nativeDisplay);
        }
    }

    if (mEGLDisplay != EGL_NO_DISPLAY &&
        mFnPtrs->initializePtr(mEGLDisplay, &majorVersion, &minorVersion) != EGL_TRUE)
    {
        mEGLDisplay = EGL_NO_DISPLAY;
    }
    if (mEGLDisplay == EGL_NO_DISPLAY)
    {
        // If no display was available, try to fallback to the first available
        // native device object's display.
        mEGLDisplay = getNativeDisplay(&majorVersion, &minorVersion);
    }
    if (mEGLDisplay == EGL_NO_DISPLAY)
    {
        return egl::EglNotInitialized() << "Failed to get system egl display";
    }
    if (majorVersion < 1 || (majorVersion == 1 && minorVersion < 4))
    {
        return egl::EglNotInitialized() << "Unsupported EGL version (require at least 1.4).";
    }
    if (mFnPtrs->bindAPIPtr(EGL_OPENGL_ES_API) != EGL_TRUE)
    {
        return egl::Error(mFnPtrs->getErrorPtr(), "Failed to bind API in system egl");
    }

    vendorString  = queryString(EGL_VENDOR);
    versionString = queryString(EGL_VERSION);

    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->getCurrentContextPtr, eglGetCurrentContext);

    if (!queryExtensions())
    {
        return egl::Error(mFnPtrs->getErrorPtr(), "Failed to query extensions in system egl");
    }

    if (hasExtension("EGL_KHR_image_base"))
    {
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->createImageKHRPtr, eglCreateImageKHR);
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->destroyImageKHRPtr, eglDestroyImageKHR);
    }
    if (hasExtension("EGL_KHR_fence_sync"))
    {
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->createSyncKHRPtr, eglCreateSyncKHR);
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->destroySyncKHRPtr, eglDestroySyncKHR);
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->clientWaitSyncKHRPtr, eglClientWaitSyncKHR);
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->getSyncAttribKHRPtr, eglGetSyncAttribKHR);
    }
    if (hasExtension("EGL_KHR_wait_sync"))
    {
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->waitSyncKHRPtr, eglWaitSyncKHR);
    }

    if (hasExtension("EGL_KHR_swap_buffers_with_damage"))
    {
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->swapBuffersWithDamageKHRPtr, eglSwapBuffersWithDamageKHR);
    }

    if (hasExtension("EGL_ANDROID_presentation_time"))
    {
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->presentationTimeANDROIDPtr, eglPresentationTimeANDROID);
    }

    if (hasExtension("EGL_ANDROID_blob_cache"))
    {
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->setBlobCacheFuncsANDROIDPtr, eglSetBlobCacheFuncsANDROID);
    }

    if (hasExtension("EGL_ANDROID_get_frame_timestamps"))
    {
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->getCompositorTimingSupportedANDROIDPtr,
                                eglGetCompositorTimingSupportedANDROID);
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->getCompositorTimingANDROIDPtr,
                                eglGetCompositorTimingANDROID);
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->getNextFrameIdANDROIDPtr, eglGetNextFrameIdANDROID);
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->getFrameTimestampSupportedANDROIDPtr,
                                eglGetFrameTimestampSupportedANDROID);
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->getFrameTimestampsANDROIDPtr,
                                eglGetFrameTimestampsANDROID);
    }

    // The native fence sync extension is a bit complicated. It's reported as present for ChromeOS,
    // but Android currently doesn't report this extension even when it's present, and older devices
    // may export a useless wrapper function. See crbug.com/775707 for details. In short, if the
    // symbol is present and we're on Android N or newer, assume that it's usable even if the
    // extension wasn't reported.
    if (hasExtension("EGL_ANDROID_native_fence_sync") || GetAndroidSDKVersion() >= 24)
    {
        // Don't error trying to load this entry point.
        if (SetPtr(&mFnPtrs->dupNativeFenceFDANDROIDPtr,
                   getProcAddress("eglDupNativeFenceFDANDROID")) &&
            !hasExtension("EGL_ANDROID_native_fence_sync"))
        {
            mExtensions.push_back("EGL_ANDROID_native_fence_sync");
        }
    }

    if (hasExtension("EGL_EXT_image_dma_buf_import_modifiers"))
    {
        // https://anglebug.com/42266130
        // Some drivers, notably older versions of ANGLE, announce this extension without
        // implementing the following functions. DisplayEGL checks for this case and disables the
        // extension.
        ANGLE_GET_PROC_OR_WARNING(&mFnPtrs->queryDmaBufFormatsEXTPtr, eglQueryDmaBufFormatsEXT);
        ANGLE_GET_PROC_OR_WARNING(&mFnPtrs->queryDmaBufModifiersEXTPtr, eglQueryDmaBufModifiersEXT);
    }

    // EGL_EXT_device_query is only advertised in extension string in the
    // no-display case, see getNativeDisplay().
    if (SetPtr(&mFnPtrs->queryDeviceAttribEXTPtr, getProcAddress("eglQueryDeviceAttribEXT")) &&
        SetPtr(&mFnPtrs->queryDeviceStringEXTPtr, getProcAddress("eglQueryDeviceStringEXT")) &&
        SetPtr(&mFnPtrs->queryDisplayAttribEXTPtr, getProcAddress("eglQueryDisplayAttribEXT")))
    {
        mExtensions.push_back("EGL_EXT_device_query");
    }

#undef ANGLE_GET_PROC_OR_ERROR

    return egl::NoError();
}

egl::Error FunctionsEGL::terminate()
{
    if (mFnPtrs->terminatePtr == nullptr || mFnPtrs->terminatePtr(mEGLDisplay) == EGL_TRUE)
    {
        mEGLDisplay = nullptr;
        return egl::NoError();
    }
    return egl::Error(mFnPtrs->getErrorPtr());
}

bool FunctionsEGL::queryExtensions()
{
    const char *extensions = queryString(EGL_EXTENSIONS);
    if (!extensions)
    {
        return false;
    }

    angle::SplitStringAlongWhitespace(extensions, &mExtensions);

    return true;
}

EGLDisplay FunctionsEGL::getPlatformDisplay(EGLAttrib platformType,
                                            EGLNativeDisplayType nativeDisplay)
{
    PFNEGLGETPLATFORMDISPLAYEXTPROC getPlatformDisplayEXTPtr;
    if (!hasExtension("EGL_EXT_platform_base") ||
        !SetPtr(&getPlatformDisplayEXTPtr, getProcAddress("eglGetPlatformDisplayEXT")))
    {
        return EGL_NO_DISPLAY;
    }

    ASSERT(IsValidPlatformTypeForPlatformDisplayConnection(platformType));
    switch (platformType)
    {
        case EGL_PLATFORM_SURFACELESS_MESA:
            if (!hasExtension("EGL_MESA_platform_surfaceless"))
                return EGL_NO_DISPLAY;
            break;
        default:
            UNREACHABLE();
            return EGL_NO_DISPLAY;
    }

    return getPlatformDisplayEXTPtr(static_cast<EGLenum>(platformType),
                                    reinterpret_cast<void *>(nativeDisplay), nullptr);
}

std::vector<EGLDeviceEXT> FunctionsEGL::queryDevices(int *major, int *minor)
{
    // Only called after confirming we have the necessary extension.
    PFNEGLQUERYDEVICESEXTPROC queryDevicesEXTPtr;
    if (!SetPtr(&queryDevicesEXTPtr, getProcAddress("eglQueryDevicesEXT")))
    {
        return {};
    }

    // Get a list of native device objects.
    const EGLint kMaxDevices = 32;
    EGLint numDevices        = 0;
    std::vector<EGLDeviceEXT> devices(kMaxDevices, EGL_NO_DEVICE_EXT);
    if (!queryDevicesEXTPtr(kMaxDevices, devices.data(), &numDevices))
    {
        return {};
    }

    devices.resize(numDevices);
    return devices;
}

EGLDisplay FunctionsEGL::getNativeDisplay(int *major, int *minor)
{
    // This fallback mechanism makes use of:
    // - EGL_EXT_device_enumeration or EGL_EXT_device_base for eglQueryDevicesEXT
    // - EGL_EXT_platform_base for eglGetPlatformDisplayEXT
    // - EGL_EXT_platform_device for EGL_PLATFORM_DEVICE_EXT
    PFNEGLGETPLATFORMDISPLAYEXTPROC getPlatformDisplayEXTPtr;
    bool hasQueryDevicesEXT =
        hasExtension("EGL_EXT_device_enumeration") || hasExtension("EGL_EXT_device_base");
    bool hasPlatformBaseEXT   = hasExtension("EGL_EXT_platform_base");
    bool hasPlatformDeviceEXT = hasExtension("EGL_EXT_platform_device");
    if (!hasQueryDevicesEXT || !hasPlatformBaseEXT || !hasPlatformDeviceEXT)
    {
        return EGL_NO_DISPLAY;
    }

    if (!SetPtr(&getPlatformDisplayEXTPtr, getProcAddress("eglGetPlatformDisplayEXT")))
    {
        return EGL_NO_DISPLAY;
    }

    std::vector<EGLDeviceEXT> devices = queryDevices(major, minor);
    if (devices.size() == 0)
    {
        return EGL_NO_DISPLAY;
    }

    // Look for the first native device that gives us a valid display.
    for (const EGLDeviceEXT device : devices)
    {
        EGLDisplay display = getPlatformDisplayEXTPtr(EGL_PLATFORM_DEVICE_EXT, device, nullptr);
        if (mFnPtrs->getErrorPtr() == EGL_SUCCESS &&
            mFnPtrs->initializePtr(display, major, minor) == EGL_TRUE)
        {
            return display;
        }
    }

    return EGL_NO_DISPLAY;
}

#if defined(ANGLE_HAS_LIBDRM)
EGLDeviceEXT FunctionsEGL::getPreferredEGLDevice(const std::vector<EGLDeviceEXT> &devices)
{
    // Only called after confirming we have the necessary extension.
    PFNEGLQUERYDEVICESTRINGEXTPROC queryDeviceStringEXTPtr;
    if (!SetPtr(&queryDeviceStringEXTPtr, getProcAddress("eglQueryDeviceStringEXT")))
    {
        return EGL_NO_DEVICE_EXT;
    }

    std::map<EGLDeviceEXT, std::string> device_drivers;
    for (const EGLDeviceEXT device : devices)
    {
        const char *filename = queryDeviceStringEXTPtr(device, EGL_DRM_DEVICE_FILE_EXT);
        if (!filename)
        {
            continue;
        }

        int fd = open(filename, O_RDWR);
        if (!fd)
        {
            continue;
        }

        drmVersionPtr version = drmGetVersion(fd);
        if (version)
        {
            const std::string driver_name(version->name, version->name_len);
            device_drivers.insert({device, driver_name});
        }

        drmFreeVersion(version);
        close(fd);
    }

    const char *preferred_drivers[3] = {"i915", "amdgpu", "virtio_gpu"};
    for (const char *preferred_driver : preferred_drivers)
    {
        for (const EGLDeviceEXT device : devices)
        {
            const auto driver = device_drivers.find(device);
            if (driver != device_drivers.end() && driver->second == preferred_driver)
            {
                return device;
            }
        }
    }

    return EGL_NO_DEVICE_EXT;
}

EGLDisplay FunctionsEGL::getPreferredDisplay(int *major, int *minor)
{
    // This  mechanism makes use of:
    // - EGL_EXT_device_enumeration or EGL_EXT_device_base for eglQueryDevicesEXT
    // - EGL_EXT_platform_base for eglGetPlatformDisplayEXT
    // - EGL_EXT_platform_device for EGL_PLATFORM_DEVICE_EXT
    // - EGL_EXT_device_query for eglQueryDeviceStringEXT
    PFNEGLGETPLATFORMDISPLAYEXTPROC getPlatformDisplayEXTPtr;
    bool hasQueryDevicesEXT =
        hasExtension("EGL_EXT_device_enumeration") || hasExtension("EGL_EXT_device_base");
    bool hasPlatformBaseEXT   = hasExtension("EGL_EXT_platform_base");
    bool hasPlatformDeviceEXT = hasExtension("EGL_EXT_platform_device");
    bool hasDeviceQueryEXT    = hasExtension("EGL_EXT_device_query");
    if (!hasQueryDevicesEXT || !hasPlatformBaseEXT || !hasPlatformDeviceEXT || !hasDeviceQueryEXT)
    {
        return EGL_NO_DISPLAY;
    }

    if (!SetPtr(&getPlatformDisplayEXTPtr, getProcAddress("eglGetPlatformDisplayEXT")))
    {
        return EGL_NO_DISPLAY;
    }

    std::vector<EGLDeviceEXT> devices = queryDevices(major, minor);
    if (devices.size() == 0)
    {
        return EGL_NO_DISPLAY;
    }

    EGLDeviceEXT device = getPreferredEGLDevice(devices);
    if (device == EGL_NO_DEVICE_EXT)
    {
        return EGL_NO_DISPLAY;
    }

    EGLDisplay display = getPlatformDisplayEXTPtr(EGL_PLATFORM_DEVICE_EXT, device, nullptr);
    if (mFnPtrs->getErrorPtr() == EGL_SUCCESS)
    {
        return display;
    }

    return EGL_NO_DISPLAY;
}
#endif  // defined(ANGLE_HAS_LIBDRM)

class FunctionsGLEGL : public FunctionsGL
{
  public:
    FunctionsGLEGL(const FunctionsEGL &egl) : mEGL(egl) {}

    ~FunctionsGLEGL() override {}

  private:
    void *loadProcAddress(const std::string &function) const override
    {
        return mEGL.getProcAddress(function.c_str());
    }

    const FunctionsEGL &mEGL;
};

FunctionsGL *FunctionsEGL::makeFunctionsGL(void) const
{
    return new FunctionsGLEGL(*this);
}

bool FunctionsEGL::hasExtension(const char *extension) const
{
    return std::find(mExtensions.begin(), mExtensions.end(), extension) != mExtensions.end();
}

bool FunctionsEGL::hasDmaBufImportModifierFunctions() const
{
    return mFnPtrs->queryDmaBufFormatsEXTPtr != nullptr &&
           mFnPtrs->queryDmaBufModifiersEXTPtr != nullptr;
}

EGLDisplay FunctionsEGL::getDisplay() const
{
    return mEGLDisplay;
}

EGLint FunctionsEGL::getError() const
{
    return mFnPtrs->getErrorPtr();
}

EGLBoolean FunctionsEGL::chooseConfig(EGLint const *attribList,
                                      EGLConfig *configs,
                                      EGLint configSize,
                                      EGLint *numConfig) const
{
    return mFnPtrs->chooseConfigPtr(mEGLDisplay, attribList, configs, configSize, numConfig);
}

EGLBoolean FunctionsEGL::getConfigs(EGLConfig *configs,
                                    EGLint config_size,
                                    EGLint *num_config) const
{
    return mFnPtrs->getConfigsPtr(mEGLDisplay, configs, config_size, num_config);
}

EGLBoolean FunctionsEGL::getConfigAttrib(EGLConfig config, EGLint attribute, EGLint *value) const
{
    return mFnPtrs->getConfigAttribPtr(mEGLDisplay, config, attribute, value);
}

EGLSurface FunctionsEGL::getCurrentSurface(EGLint readdraw) const
{
    return mFnPtrs->getCurrentSurfacePtr(readdraw);
}

EGLContext FunctionsEGL::createContext(EGLConfig config,
                                       EGLContext share_context,
                                       EGLint const *attrib_list) const
{
    return mFnPtrs->createContextPtr(mEGLDisplay, config, share_context, attrib_list);
}

EGLSurface FunctionsEGL::createPbufferSurface(EGLConfig config, const EGLint *attrib_list) const
{
    return mFnPtrs->createPbufferSurfacePtr(mEGLDisplay, config, attrib_list);
}

EGLSurface FunctionsEGL::createWindowSurface(EGLConfig config,
                                             EGLNativeWindowType win,
                                             const EGLint *attrib_list) const
{
    return mFnPtrs->createWindowSurfacePtr(mEGLDisplay, config, win, attrib_list);
}

EGLBoolean FunctionsEGL::destroyContext(EGLContext context) const
{
    return mFnPtrs->destroyContextPtr(mEGLDisplay, context);
}

EGLBoolean FunctionsEGL::destroySurface(EGLSurface surface) const
{
    return mFnPtrs->destroySurfacePtr(mEGLDisplay, surface);
}

EGLBoolean FunctionsEGL::makeCurrent(EGLSurface surface, EGLContext context) const
{
    return mFnPtrs->makeCurrentPtr(mEGLDisplay, surface, surface, context);
}

char const *FunctionsEGL::queryString(EGLint name) const
{
    return mFnPtrs->queryStringPtr(mEGLDisplay, name);
}

EGLBoolean FunctionsEGL::querySurface(EGLSurface surface, EGLint attribute, EGLint *value) const
{
    return mFnPtrs->querySurfacePtr(mEGLDisplay, surface, attribute, value);
}

EGLBoolean FunctionsEGL::swapBuffers(EGLSurface surface) const
{
    return mFnPtrs->swapBuffersPtr(mEGLDisplay, surface);
}

EGLBoolean FunctionsEGL::bindTexImage(EGLSurface surface, EGLint buffer) const
{
    return mFnPtrs->bindTexImagePtr(mEGLDisplay, surface, buffer);
}

EGLBoolean FunctionsEGL::releaseTexImage(EGLSurface surface, EGLint buffer) const
{
    return mFnPtrs->releaseTexImagePtr(mEGLDisplay, surface, buffer);
}

EGLBoolean FunctionsEGL::surfaceAttrib(EGLSurface surface, EGLint attribute, EGLint value) const
{
    return mFnPtrs->surfaceAttribPtr(mEGLDisplay, surface, attribute, value);
}

EGLBoolean FunctionsEGL::swapInterval(EGLint interval) const
{
    return mFnPtrs->swapIntervalPtr(mEGLDisplay, interval);
}

EGLContext FunctionsEGL::getCurrentContext() const
{
    return mFnPtrs->getCurrentContextPtr();
}

EGLImageKHR FunctionsEGL::createImageKHR(EGLContext context,
                                         EGLenum target,
                                         EGLClientBuffer buffer,
                                         const EGLint *attrib_list) const
{
    return mFnPtrs->createImageKHRPtr(mEGLDisplay, context, target, buffer, attrib_list);
}

EGLBoolean FunctionsEGL::destroyImageKHR(EGLImageKHR image) const
{
    return mFnPtrs->destroyImageKHRPtr(mEGLDisplay, image);
}

EGLSyncKHR FunctionsEGL::createSyncKHR(EGLenum type, const EGLint *attrib_list) const
{
    return mFnPtrs->createSyncKHRPtr(mEGLDisplay, type, attrib_list);
}

EGLBoolean FunctionsEGL::destroySyncKHR(EGLSyncKHR sync) const
{
    return mFnPtrs->destroySyncKHRPtr(mEGLDisplay, sync);
}

EGLint FunctionsEGL::clientWaitSyncKHR(EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout) const
{
    return mFnPtrs->clientWaitSyncKHRPtr(mEGLDisplay, sync, flags, timeout);
}

EGLBoolean FunctionsEGL::getSyncAttribKHR(EGLSyncKHR sync, EGLint attribute, EGLint *value) const
{
    return mFnPtrs->getSyncAttribKHRPtr(mEGLDisplay, sync, attribute, value);
}

EGLint FunctionsEGL::waitSyncKHR(EGLSyncKHR sync, EGLint flags) const
{
    return mFnPtrs->waitSyncKHRPtr(mEGLDisplay, sync, flags);
}

EGLBoolean FunctionsEGL::swapBuffersWithDamageKHR(EGLSurface surface,
                                                  const EGLint *rects,
                                                  EGLint n_rects) const
{
    return mFnPtrs->swapBuffersWithDamageKHRPtr(mEGLDisplay, surface, rects, n_rects);
}

EGLBoolean FunctionsEGL::presentationTimeANDROID(EGLSurface surface, EGLnsecsANDROID time) const
{
    return mFnPtrs->presentationTimeANDROIDPtr(mEGLDisplay, surface, time);
}

void FunctionsEGL::setBlobCacheFuncsANDROID(EGLSetBlobFuncANDROID set,
                                            EGLGetBlobFuncANDROID get) const
{
    return mFnPtrs->setBlobCacheFuncsANDROIDPtr(mEGLDisplay, set, get);
}

EGLBoolean FunctionsEGL::getCompositorTimingSupportedANDROID(EGLSurface surface, EGLint name) const
{
    return mFnPtrs->getCompositorTimingSupportedANDROIDPtr(mEGLDisplay, surface, name);
}

EGLBoolean FunctionsEGL::getCompositorTimingANDROID(EGLSurface surface,
                                                    EGLint numTimestamps,
                                                    const EGLint *names,
                                                    EGLnsecsANDROID *values) const
{
    return mFnPtrs->getCompositorTimingANDROIDPtr(mEGLDisplay, surface, numTimestamps, names,
                                                  values);
}

EGLBoolean FunctionsEGL::getNextFrameIdANDROID(EGLSurface surface, EGLuint64KHR *frameId) const
{
    return mFnPtrs->getNextFrameIdANDROIDPtr(mEGLDisplay, surface, frameId);
}

EGLBoolean FunctionsEGL::getFrameTimestampSupportedANDROID(EGLSurface surface,
                                                           EGLint timestamp) const
{
    return mFnPtrs->getFrameTimestampSupportedANDROIDPtr(mEGLDisplay, surface, timestamp);
}

EGLBoolean FunctionsEGL::getFrameTimestampsANDROID(EGLSurface surface,
                                                   EGLuint64KHR frameId,
                                                   EGLint numTimestamps,
                                                   const EGLint *timestamps,
                                                   EGLnsecsANDROID *values) const
{
    return mFnPtrs->getFrameTimestampsANDROIDPtr(mEGLDisplay, surface, frameId, numTimestamps,
                                                 timestamps, values);
}

EGLint FunctionsEGL::dupNativeFenceFDANDROID(EGLSync sync) const
{
    return mFnPtrs->dupNativeFenceFDANDROIDPtr(mEGLDisplay, sync);
}

EGLint FunctionsEGL::queryDmaBufFormatsEXT(EGLint maxFormats,
                                           EGLint *formats,
                                           EGLint *numFormats) const
{
    return mFnPtrs->queryDmaBufFormatsEXTPtr(mEGLDisplay, maxFormats, formats, numFormats);
}

EGLint FunctionsEGL::queryDmaBufModifiersEXT(EGLint format,
                                             EGLint maxModifiers,
                                             EGLuint64KHR *modifiers,
                                             EGLBoolean *externalOnly,
                                             EGLint *numModifiers) const
{
    return mFnPtrs->queryDmaBufModifiersEXTPtr(mEGLDisplay, format, maxModifiers, modifiers,
                                               externalOnly, numModifiers);
}

EGLBoolean FunctionsEGL::queryDeviceAttribEXT(EGLDeviceEXT device,
                                              EGLint attribute,
                                              EGLAttrib *value) const
{
    return mFnPtrs->queryDeviceAttribEXTPtr(device, attribute, value);
}

const char *FunctionsEGL::queryDeviceStringEXT(EGLDeviceEXT device, EGLint name) const
{
    return mFnPtrs->queryDeviceStringEXTPtr(device, name);
}

EGLBoolean FunctionsEGL::queryDisplayAttribEXT(EGLint attribute, EGLAttrib *value) const
{
    return mFnPtrs->queryDisplayAttribEXTPtr(mEGLDisplay, attribute, value);
}

}  // namespace rx
