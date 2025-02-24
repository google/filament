//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "util/EGLWindow.h"

#include <cassert>
#include <iostream>
#include <vector>

#include <string.h>

#include "common/hash_containers.h"
#include "common/system_utils.h"
#include "platform/Feature.h"
#include "platform/PlatformMethods.h"
#include "util/OSWindow.h"

namespace
{
constexpr EGLint kDefaultSwapInterval = 1;
}  // anonymous namespace

// ConfigParameters implementation.
ConfigParameters::ConfigParameters()
    : redBits(-1),
      greenBits(-1),
      blueBits(-1),
      alphaBits(-1),
      depthBits(-1),
      stencilBits(-1),
      componentType(EGL_COLOR_COMPONENT_TYPE_FIXED_EXT),
      multisample(false),
      debug(false),
      noError(false),
      bindGeneratesResource(true),
      clientArraysEnabled(true),
      robustAccess(false),
      mutableRenderBuffer(false),
      samples(-1),
      resetStrategy(EGL_NO_RESET_NOTIFICATION_EXT),
      colorSpace(EGL_COLORSPACE_LINEAR),
      swapInterval(kDefaultSwapInterval)
{}

ConfigParameters::~ConfigParameters() = default;

void ConfigParameters::reset()
{
    *this = ConfigParameters();
}

// GLWindowBase implementation.
GLWindowBase::GLWindowBase(GLint glesMajorVersion, EGLint glesMinorVersion)
    : mClientMajorVersion(glesMajorVersion), mClientMinorVersion(glesMinorVersion)
{}

GLWindowBase::~GLWindowBase() = default;

// EGLWindow implementation.
EGLWindow::EGLWindow(EGLint glesMajorVersion, EGLint glesMinorVersion)
    : GLWindowBase(glesMajorVersion, glesMinorVersion),
      mConfig(0),
      mDisplay(EGL_NO_DISPLAY),
      mSurface(EGL_NO_SURFACE),
      mContext(EGL_NO_CONTEXT),
      mEGLMajorVersion(0),
      mEGLMinorVersion(0)
{
    std::fill(mFeatures.begin(), mFeatures.end(), ANGLEFeatureStatus::Unknown);
}

EGLWindow::~EGLWindow()
{
    destroyGL();
}

void EGLWindow::swap()
{
    eglSwapBuffers(mDisplay, mSurface);
}

EGLConfig EGLWindow::getConfig() const
{
    return mConfig;
}

EGLDisplay EGLWindow::getDisplay() const
{
    return mDisplay;
}

EGLSurface EGLWindow::getSurface() const
{
    return mSurface;
}

EGLContext EGLWindow::getContext() const
{
    return mContext;
}

bool EGLWindow::isContextVersion(EGLint glesMajorVersion, EGLint glesMinorVersion) const
{
    return mClientMajorVersion == glesMajorVersion && mClientMinorVersion == glesMinorVersion;
}

GLWindowResult EGLWindow::initializeGLWithResult(OSWindow *osWindow,
                                                 angle::Library *glWindowingLibrary,
                                                 angle::GLESDriverType driverType,
                                                 const EGLPlatformParameters &platformParams,
                                                 const ConfigParameters &configParams)
{
    if (!initializeDisplay(osWindow, glWindowingLibrary, driverType, platformParams))
    {
        return GLWindowResult::Error;
    }
    GLWindowResult res = initializeSurface(osWindow, glWindowingLibrary, configParams);
    if (res != GLWindowResult::NoError)
    {
        return res;
    }
    if (!initializeContext())
    {
        return GLWindowResult::Error;
    }
    return GLWindowResult::NoError;
}

bool EGLWindow::initializeGL(OSWindow *osWindow,
                             angle::Library *glWindowingLibrary,
                             angle::GLESDriverType driverType,
                             const EGLPlatformParameters &platformParams,
                             const ConfigParameters &configParams)
{
    return initializeGLWithResult(osWindow, glWindowingLibrary, driverType, platformParams,
                                  configParams) == GLWindowResult::NoError;
}

bool EGLWindow::initializeDisplay(OSWindow *osWindow,
                                  angle::Library *glWindowingLibrary,
                                  angle::GLESDriverType driverType,
                                  const EGLPlatformParameters &params)
{
    if (driverType == angle::GLESDriverType::ZinkEGL)
    {
        std::stringstream driDirStream;
        char s = angle::GetPathSeparator();
        driDirStream << angle::GetModuleDirectory() << "mesa" << s << "src" << s << "gallium" << s
                     << "targets" << s << "dri";

        std::string driDir = driDirStream.str();

        angle::SetEnvironmentVar("MESA_LOADER_DRIVER_OVERRIDE", "zink");
        angle::SetEnvironmentVar("LIBGL_DRIVERS_PATH", driDir.c_str());
    }

#if defined(ANGLE_USE_UTIL_LOADER)
    PFNEGLGETPROCADDRESSPROC getProcAddress;
    glWindowingLibrary->getAs("eglGetProcAddress", &getProcAddress);
    if (!getProcAddress)
    {
        fprintf(stderr, "Cannot load eglGetProcAddress\n");
        return false;
    }

    // Likely we will need to use a fallback to Library::getAs on non-ANGLE platforms.
    LoadUtilEGL(getProcAddress);
#endif  // defined(ANGLE_USE_UTIL_LOADER)

    // EGL_NO_DISPLAY + EGL_EXTENSIONS returns NULL before Android 10
    const char *extensionString =
        static_cast<const char *>(eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS));
    if (!extensionString)
    {
        // fallback to an empty string for strstr
        extensionString = "";
    }

    std::vector<EGLAttrib> displayAttributes;
    displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
    displayAttributes.push_back(params.renderer);
    displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE);
    displayAttributes.push_back(params.majorVersion);
    displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE);
    displayAttributes.push_back(params.minorVersion);

    if (params.deviceType != EGL_DONT_CARE)
    {
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE);
        displayAttributes.push_back(params.deviceType);
    }

    if (params.presentPath != EGL_DONT_CARE)
    {
        if (strstr(extensionString, "EGL_ANGLE_experimental_present_path") == nullptr)
        {
            destroyGL();
            return false;
        }

        displayAttributes.push_back(EGL_EXPERIMENTAL_PRESENT_PATH_ANGLE);
        displayAttributes.push_back(params.presentPath);
    }

    // Set debug layer settings if requested.
    if (params.debugLayersEnabled != EGL_DONT_CARE)
    {
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_DEBUG_LAYERS_ENABLED_ANGLE);
        displayAttributes.push_back(params.debugLayersEnabled);
    }

    if (params.platformMethods)
    {
        static_assert(sizeof(EGLAttrib) == sizeof(params.platformMethods),
                      "Unexpected pointer size");
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_PLATFORM_METHODS_ANGLEX);
        displayAttributes.push_back(reinterpret_cast<EGLAttrib>(params.platformMethods));
    }

    if (params.displayPowerPreference != EGL_DONT_CARE)
    {
        displayAttributes.push_back(EGL_POWER_PREFERENCE_ANGLE);
        displayAttributes.push_back(params.displayPowerPreference);
    }

    std::vector<const char *> enabledFeatureOverrides;
    std::vector<const char *> disabledFeatureOverrides;

    for (angle::Feature feature : params.enabledFeatureOverrides)
    {
        enabledFeatureOverrides.push_back(angle::GetFeatureName(feature));
    }
    for (angle::Feature feature : params.disabledFeatureOverrides)
    {
        disabledFeatureOverrides.push_back(angle::GetFeatureName(feature));
    }

    const bool hasFeatureControlANGLE =
        strstr(extensionString, "EGL_ANGLE_feature_control") != nullptr;

    if (!hasFeatureControlANGLE &&
        (!enabledFeatureOverrides.empty() || !disabledFeatureOverrides.empty()))
    {
        fprintf(stderr, "Missing EGL_ANGLE_feature_control.\n");
        destroyGL();
        return false;
    }

    if (!disabledFeatureOverrides.empty())
    {
        disabledFeatureOverrides.push_back(nullptr);

        displayAttributes.push_back(EGL_FEATURE_OVERRIDES_DISABLED_ANGLE);
        displayAttributes.push_back(reinterpret_cast<EGLAttrib>(disabledFeatureOverrides.data()));
    }

    if (hasFeatureControlANGLE)
    {
        // Always enable exposeNonConformantExtensionsAndVersions in ANGLE tests.
        enabledFeatureOverrides.push_back("exposeNonConformantExtensionsAndVersions");
        enabledFeatureOverrides.push_back(nullptr);

        displayAttributes.push_back(EGL_FEATURE_OVERRIDES_ENABLED_ANGLE);
        displayAttributes.push_back(reinterpret_cast<EGLAttrib>(enabledFeatureOverrides.data()));
    }

    displayAttributes.push_back(EGL_NONE);

    if (driverType == angle::GLESDriverType::SystemWGL)
        return false;

    if (IsANGLE(driverType) && strstr(extensionString, "EGL_ANGLE_platform_angle"))
    {
        mDisplay = eglGetPlatformDisplay(EGL_PLATFORM_ANGLE_ANGLE,
                                         reinterpret_cast<void *>(osWindow->getNativeDisplay()),
                                         &displayAttributes[0]);
    }
    else
    {
        mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    }

    if (mDisplay == EGL_NO_DISPLAY)
    {
        fprintf(stderr, "Failed to get display: 0x%X\n", eglGetError());
        destroyGL();
        return false;
    }

    if (eglInitialize(mDisplay, &mEGLMajorVersion, &mEGLMinorVersion) == EGL_FALSE)
    {
        fprintf(stderr, "eglInitialize failed: 0x%X\n", eglGetError());
        destroyGL();
        return false;
    }

    queryFeatures();

    mPlatform = params;
    return true;
}

GLWindowResult EGLWindow::initializeSurface(OSWindow *osWindow,
                                            angle::Library *glWindowingLibrary,
                                            const ConfigParameters &params)
{
    mConfigParams                 = params;
    const char *displayExtensions = eglQueryString(mDisplay, EGL_EXTENSIONS);

    bool hasMutableRenderBuffer =
        strstr(displayExtensions, "EGL_KHR_mutable_render_buffer") != nullptr;
    if (mConfigParams.mutableRenderBuffer && !hasMutableRenderBuffer)
    {
        fprintf(stderr, "Mising EGL_KHR_mutable_render_buffer.\n");
        destroyGL();
        return GLWindowResult::NoMutableRenderBufferSupport;
    }

    std::vector<EGLint> configAttributes = {
        EGL_SURFACE_TYPE,
        EGL_WINDOW_BIT | (params.mutableRenderBuffer ? EGL_MUTABLE_RENDER_BUFFER_BIT_KHR : 0),
        EGL_RED_SIZE,
        (mConfigParams.redBits >= 0) ? mConfigParams.redBits : EGL_DONT_CARE,
        EGL_GREEN_SIZE,
        (mConfigParams.greenBits >= 0) ? mConfigParams.greenBits : EGL_DONT_CARE,
        EGL_BLUE_SIZE,
        (mConfigParams.blueBits >= 0) ? mConfigParams.blueBits : EGL_DONT_CARE,
        EGL_ALPHA_SIZE,
        (mConfigParams.alphaBits >= 0) ? mConfigParams.alphaBits : EGL_DONT_CARE,
        EGL_DEPTH_SIZE,
        (mConfigParams.depthBits >= 0) ? mConfigParams.depthBits : EGL_DONT_CARE,
        EGL_STENCIL_SIZE,
        (mConfigParams.stencilBits >= 0) ? mConfigParams.stencilBits : EGL_DONT_CARE,
        EGL_SAMPLE_BUFFERS,
        mConfigParams.multisample ? 1 : 0,
        EGL_SAMPLES,
        (mConfigParams.samples >= 0) ? mConfigParams.samples : EGL_DONT_CARE,
    };

    // Add dynamic attributes
    bool hasPixelFormatFloat = strstr(displayExtensions, "EGL_EXT_pixel_format_float") != nullptr;
    if (!hasPixelFormatFloat && mConfigParams.componentType != EGL_COLOR_COMPONENT_TYPE_FIXED_EXT)
    {
        fprintf(stderr, "Mising EGL_EXT_pixel_format_float.\n");
        destroyGL();
        return GLWindowResult::Error;
    }
    if (hasPixelFormatFloat)
    {
        configAttributes.push_back(EGL_COLOR_COMPONENT_TYPE_EXT);
        configAttributes.push_back(mConfigParams.componentType);
    }

    // Finish the attribute list
    configAttributes.push_back(EGL_NONE);

    if (!FindEGLConfig(mDisplay, configAttributes.data(), &mConfig))
    {
        fprintf(stderr, "Could not find a suitable EGL config!\n");
        destroyGL();
        return GLWindowResult::Error;
    }

    eglGetConfigAttrib(mDisplay, mConfig, EGL_RED_SIZE, &mConfigParams.redBits);
    eglGetConfigAttrib(mDisplay, mConfig, EGL_GREEN_SIZE, &mConfigParams.greenBits);
    eglGetConfigAttrib(mDisplay, mConfig, EGL_BLUE_SIZE, &mConfigParams.blueBits);
    eglGetConfigAttrib(mDisplay, mConfig, EGL_ALPHA_SIZE, &mConfigParams.alphaBits);
    eglGetConfigAttrib(mDisplay, mConfig, EGL_DEPTH_SIZE, &mConfigParams.depthBits);
    eglGetConfigAttrib(mDisplay, mConfig, EGL_STENCIL_SIZE, &mConfigParams.stencilBits);
    eglGetConfigAttrib(mDisplay, mConfig, EGL_SAMPLES, &mConfigParams.samples);

    std::vector<EGLint> surfaceAttributes;
    if (strstr(displayExtensions, "EGL_NV_post_sub_buffer") != nullptr)
    {
        surfaceAttributes.push_back(EGL_POST_SUB_BUFFER_SUPPORTED_NV);
        surfaceAttributes.push_back(EGL_TRUE);
    }

    bool hasRobustResourceInit =
        strstr(displayExtensions, "EGL_ANGLE_robust_resource_initialization") != nullptr;
    if (hasRobustResourceInit && mConfigParams.robustResourceInit.valid())
    {
        surfaceAttributes.push_back(EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE);
        surfaceAttributes.push_back(mConfigParams.robustResourceInit.value() ? EGL_TRUE
                                                                             : EGL_FALSE);
    }

    bool hasGLColorSpace = strstr(displayExtensions, "EGL_KHR_gl_colorspace") != nullptr;
    if (!hasGLColorSpace && mConfigParams.colorSpace != EGL_COLORSPACE_LINEAR)
    {
        fprintf(stderr, "Mising EGL_KHR_gl_colorspace.\n");
        destroyGL();
        return GLWindowResult::NoColorspaceSupport;
    }
    if (hasGLColorSpace)
    {
        surfaceAttributes.push_back(EGL_GL_COLORSPACE_KHR);
        surfaceAttributes.push_back(mConfigParams.colorSpace);
    }

    bool hasCreateSurfaceSwapInterval =
        strstr(displayExtensions, "EGL_ANGLE_create_surface_swap_interval") != nullptr;
    if (hasCreateSurfaceSwapInterval && mConfigParams.swapInterval != kDefaultSwapInterval)
    {
        surfaceAttributes.push_back(EGL_SWAP_INTERVAL_ANGLE);
        surfaceAttributes.push_back(mConfigParams.swapInterval);
    }

    surfaceAttributes.push_back(EGL_NONE);

    osWindow->resetNativeWindow();

    mSurface = eglCreateWindowSurface(mDisplay, mConfig, osWindow->getNativeWindow(),
                                      &surfaceAttributes[0]);
    if (eglGetError() != EGL_SUCCESS || (mSurface == EGL_NO_SURFACE))
    {
        fprintf(stderr, "eglCreateWindowSurface failed: 0x%X\n", eglGetError());
        destroyGL();
        return GLWindowResult::Error;
    }

#if defined(ANGLE_USE_UTIL_LOADER)
    LoadUtilGLES(eglGetProcAddress);
#endif  // defined(ANGLE_USE_UTIL_LOADER)

    return GLWindowResult::NoError;
}

GLWindowContext EGLWindow::getCurrentContextGeneric()
{
    return reinterpret_cast<GLWindowContext>(mContext);
}

GLWindowContext EGLWindow::createContextGeneric(GLWindowContext share)
{
    EGLContext shareContext = reinterpret_cast<EGLContext>(share);
    return reinterpret_cast<GLWindowContext>(createContext(shareContext, nullptr));
}

EGLContext EGLWindow::createContext(EGLContext share, EGLint *extraAttributes)
{
    const char *displayExtensions = eglQueryString(mDisplay, EGL_EXTENSIONS);

    // EGL_KHR_create_context is required to request a ES3+ context.
    bool hasKHRCreateContext = strstr(displayExtensions, "EGL_KHR_create_context") != nullptr;
    if (mClientMajorVersion > 2 && !(mEGLMajorVersion > 1 || mEGLMinorVersion >= 5) &&
        !hasKHRCreateContext)
    {
        fprintf(stderr, "EGL_KHR_create_context incompatibility.\n");
        return EGL_NO_CONTEXT;
    }

    // EGL_CONTEXT_OPENGL_DEBUG is only valid as of EGL 1.5.
    bool hasDebug = mEGLMinorVersion >= 5;
    if (mConfigParams.debug && !hasDebug)
    {
        fprintf(stderr, "EGL 1.5 is required for EGL_CONTEXT_OPENGL_DEBUG.\n");
        return EGL_NO_CONTEXT;
    }

    bool hasWebGLCompatibility =
        strstr(displayExtensions, "EGL_ANGLE_create_context_webgl_compatibility") != nullptr;
    if (mConfigParams.webGLCompatibility.valid() && !hasWebGLCompatibility)
    {
        fprintf(stderr, "EGL_ANGLE_create_context_webgl_compatibility missing.\n");
        return EGL_NO_CONTEXT;
    }

    bool hasCreateContextExtensionsEnabled =
        strstr(displayExtensions, "EGL_ANGLE_create_context_extensions_enabled") != nullptr;
    if (mConfigParams.extensionsEnabled.valid() && !hasCreateContextExtensionsEnabled)
    {
        fprintf(stderr, "EGL_ANGLE_create_context_extensions_enabled missing.\n");
        return EGL_NO_CONTEXT;
    }

    bool hasRobustness = strstr(displayExtensions, "EGL_EXT_create_context_robustness") != nullptr;
    if ((mConfigParams.robustAccess ||
         mConfigParams.resetStrategy != EGL_NO_RESET_NOTIFICATION_EXT) &&
        !hasRobustness)
    {
        fprintf(stderr, "EGL_EXT_create_context_robustness missing.\n");
        return EGL_NO_CONTEXT;
    }

    bool hasBindGeneratesResource =
        strstr(displayExtensions, "EGL_CHROMIUM_create_context_bind_generates_resource") != nullptr;
    if (!mConfigParams.bindGeneratesResource && !hasBindGeneratesResource)
    {
        fprintf(stderr, "EGL_CHROMIUM_create_context_bind_generates_resource missing.\n");
        return EGL_NO_CONTEXT;
    }

    bool hasClientArraysExtension =
        strstr(displayExtensions, "EGL_ANGLE_create_context_client_arrays") != nullptr;
    if (!mConfigParams.clientArraysEnabled && !hasClientArraysExtension)
    {
        // Non-default state requested without the extension present
        fprintf(stderr, "EGL_ANGLE_create_context_client_arrays missing.\n");
        return EGL_NO_CONTEXT;
    }

    bool hasProgramCacheControlExtension =
        strstr(displayExtensions, "EGL_ANGLE_program_cache_control ") != nullptr;
    if (mConfigParams.contextProgramCacheEnabled.valid() && !hasProgramCacheControlExtension)
    {
        fprintf(stderr, "EGL_ANGLE_program_cache_control missing.\n");
        return EGL_NO_CONTEXT;
    }

    bool hasKHRCreateContextNoError =
        strstr(displayExtensions, "EGL_KHR_create_context_no_error") != nullptr;
    if (mConfigParams.noError && !hasKHRCreateContextNoError)
    {
        fprintf(stderr, "EGL_KHR_create_context_no_error missing.\n");
        return EGL_NO_CONTEXT;
    }

    eglBindAPI(EGL_OPENGL_ES_API);
    if (eglGetError() != EGL_SUCCESS)
    {
        fprintf(stderr, "Error on eglBindAPI.\n");
        return EGL_NO_CONTEXT;
    }

    std::vector<EGLint> contextAttributes;
    for (EGLint *extraAttrib = extraAttributes;
         extraAttrib != nullptr && extraAttrib[0] != EGL_NONE; extraAttrib += 2)
    {
        contextAttributes.push_back(extraAttrib[0]);
        contextAttributes.push_back(extraAttrib[1]);
    }

    if (hasKHRCreateContext)
    {
        contextAttributes.push_back(EGL_CONTEXT_MAJOR_VERSION_KHR);
        contextAttributes.push_back(mClientMajorVersion);

        contextAttributes.push_back(EGL_CONTEXT_MINOR_VERSION_KHR);
        contextAttributes.push_back(mClientMinorVersion);

        // Note that the Android loader currently doesn't handle this flag despite reporting 1.5.
        // Work around this by only using the debug bit when we request a debug context.
        if (hasDebug && mConfigParams.debug)
        {
            contextAttributes.push_back(EGL_CONTEXT_OPENGL_DEBUG);
            contextAttributes.push_back(mConfigParams.debug ? EGL_TRUE : EGL_FALSE);
        }

        // TODO (http://anglebug.com/42264345)
        // Mesa does not allow EGL_CONTEXT_OPENGL_NO_ERROR_KHR for GLES1.
        if (hasKHRCreateContextNoError && mConfigParams.noError)
        {
            contextAttributes.push_back(EGL_CONTEXT_OPENGL_NO_ERROR_KHR);
            contextAttributes.push_back(mConfigParams.noError ? EGL_TRUE : EGL_FALSE);
        }

        if (mConfigParams.webGLCompatibility.valid())
        {
            contextAttributes.push_back(EGL_CONTEXT_WEBGL_COMPATIBILITY_ANGLE);
            contextAttributes.push_back(mConfigParams.webGLCompatibility.value() ? EGL_TRUE
                                                                                 : EGL_FALSE);
        }

        if (mConfigParams.extensionsEnabled.valid())
        {
            contextAttributes.push_back(EGL_EXTENSIONS_ENABLED_ANGLE);
            contextAttributes.push_back(mConfigParams.extensionsEnabled.value() ? EGL_TRUE
                                                                                : EGL_FALSE);
        }

        if (hasRobustness)
        {
            contextAttributes.push_back(EGL_CONTEXT_OPENGL_ROBUST_ACCESS_EXT);
            contextAttributes.push_back(mConfigParams.robustAccess ? EGL_TRUE : EGL_FALSE);

            contextAttributes.push_back(EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT);
            contextAttributes.push_back(mConfigParams.resetStrategy);
        }

        if (hasBindGeneratesResource)
        {
            contextAttributes.push_back(EGL_CONTEXT_BIND_GENERATES_RESOURCE_CHROMIUM);
            contextAttributes.push_back(mConfigParams.bindGeneratesResource ? EGL_TRUE : EGL_FALSE);
        }

        if (hasClientArraysExtension)
        {
            contextAttributes.push_back(EGL_CONTEXT_CLIENT_ARRAYS_ENABLED_ANGLE);
            contextAttributes.push_back(mConfigParams.clientArraysEnabled ? EGL_TRUE : EGL_FALSE);
        }

        if (mConfigParams.contextProgramCacheEnabled.valid())
        {
            contextAttributes.push_back(EGL_CONTEXT_PROGRAM_BINARY_CACHE_ENABLED_ANGLE);
            contextAttributes.push_back(
                mConfigParams.contextProgramCacheEnabled.value() ? EGL_TRUE : EGL_FALSE);
        }

        bool hasBackwardsCompatibleContextExtension =
            strstr(displayExtensions, "EGL_ANGLE_create_context_backwards_compatible") != nullptr;
        if (hasBackwardsCompatibleContextExtension)
        {
            // Always request the exact context version that the config wants
            contextAttributes.push_back(EGL_CONTEXT_OPENGL_BACKWARDS_COMPATIBLE_ANGLE);
            contextAttributes.push_back(EGL_FALSE);
        }

        bool hasRobustResourceInit =
            strstr(displayExtensions, "EGL_ANGLE_robust_resource_initialization") != nullptr;
        if (hasRobustResourceInit && mConfigParams.robustResourceInit.valid())
        {
            contextAttributes.push_back(EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE);
            contextAttributes.push_back(mConfigParams.robustResourceInit.value() ? EGL_TRUE
                                                                                 : EGL_FALSE);
        }
    }
    contextAttributes.push_back(EGL_NONE);

    EGLContext context = eglCreateContext(mDisplay, mConfig, share, &contextAttributes[0]);
    if (context == EGL_NO_CONTEXT)
    {
        fprintf(stderr, "eglCreateContext failed: 0x%X\n", eglGetError());
        return EGL_NO_CONTEXT;
    }

    return context;
}

bool EGLWindow::initializeContext()
{
    mContext = createContext(EGL_NO_CONTEXT, nullptr);
    if (mContext == EGL_NO_CONTEXT)
    {
        destroyGL();
        return false;
    }

    if (!makeCurrent())
    {
        destroyGL();
        return false;
    }

    return true;
}

void EGLWindow::destroyGL()
{
    destroyContext();
    destroySurface();

    if (mDisplay != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglTerminate(mDisplay);
        mDisplay = EGL_NO_DISPLAY;
    }
}

void EGLWindow::destroySurface()
{
    if (mSurface != EGL_NO_SURFACE)
    {
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        assert(mDisplay != EGL_NO_DISPLAY);
        eglDestroySurface(mDisplay, mSurface);
        mSurface = EGL_NO_SURFACE;
    }
}

void EGLWindow::destroyContext()
{
    if (mContext != EGL_NO_CONTEXT)
    {
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        assert(mDisplay != EGL_NO_DISPLAY);
        eglDestroyContext(mDisplay, mContext);
        mContext = EGL_NO_CONTEXT;
    }
}

bool EGLWindow::isGLInitialized() const
{
    return mSurface != EGL_NO_SURFACE && mContext != EGL_NO_CONTEXT && mDisplay != EGL_NO_DISPLAY;
}

// Find an EGLConfig that is an exact match for the specified attributes. EGL_FALSE is returned if
// the EGLConfig is found.  This indicates that the EGLConfig is not supported.  Surface type is
// special-cased as it's possible for a config to return support for both EGL_WINDOW_BIT and
// EGL_PBUFFER_BIT even though only one of them is requested.
EGLBoolean EGLWindow::FindEGLConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *config)
{
    EGLint numConfigs = 0;
    eglGetConfigs(dpy, nullptr, 0, &numConfigs);
    std::vector<EGLConfig> allConfigs(numConfigs);
    eglGetConfigs(dpy, allConfigs.data(), static_cast<EGLint>(allConfigs.size()), &numConfigs);

    for (size_t i = 0; i < allConfigs.size(); i++)
    {
        bool matchFound = true;
        for (const EGLint *curAttrib = attrib_list; curAttrib[0] != EGL_NONE; curAttrib += 2)
        {
            if (curAttrib[1] == EGL_DONT_CARE)
            {
                continue;
            }

            EGLint actualValue = EGL_DONT_CARE;
            eglGetConfigAttrib(dpy, allConfigs[i], curAttrib[0], &actualValue);
            if ((curAttrib[0] == EGL_SURFACE_TYPE &&
                 (curAttrib[1] & actualValue) != curAttrib[1]) ||
                (curAttrib[0] != EGL_SURFACE_TYPE && curAttrib[1] != actualValue))
            {
                matchFound = false;
                break;
            }
        }

        if (matchFound)
        {
            *config = allConfigs[i];
            return EGL_TRUE;
        }
    }

    return EGL_FALSE;
}

bool EGLWindow::makeCurrentGeneric(GLWindowContext context)
{
    EGLContext eglContext = reinterpret_cast<EGLContext>(context);
    return makeCurrent(eglContext);
}

bool EGLWindow::makeCurrent()
{
    return makeCurrent(mContext);
}

EGLWindow::Image EGLWindow::createImage(GLWindowContext context,
                                        Enum target,
                                        ClientBuffer buffer,
                                        const Attrib *attrib_list)
{
    return eglCreateImage(getDisplay(), context, target, buffer, attrib_list);
}

EGLWindow::Image EGLWindow::createImageKHR(GLWindowContext context,
                                           Enum target,
                                           ClientBuffer buffer,
                                           const AttribKHR *attrib_list)
{
    return eglCreateImageKHR(getDisplay(), context, target, buffer, attrib_list);
}

EGLBoolean EGLWindow::destroyImage(Image image)
{
    return eglDestroyImage(getDisplay(), image);
}

EGLBoolean EGLWindow::destroyImageKHR(Image image)
{
    return eglDestroyImageKHR(getDisplay(), image);
}

EGLWindow::Sync EGLWindow::createSync(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list)
{
    return eglCreateSync(dpy, type, attrib_list);
}

EGLWindow::Sync EGLWindow::createSyncKHR(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list)
{
    return eglCreateSyncKHR(dpy, type, attrib_list);
}

EGLBoolean EGLWindow::destroySync(EGLDisplay dpy, Sync sync)
{
    return eglDestroySync(dpy, sync);
}

EGLBoolean EGLWindow::destroySyncKHR(EGLDisplay dpy, Sync sync)
{
    return eglDestroySyncKHR(dpy, sync);
}

EGLint EGLWindow::clientWaitSync(EGLDisplay dpy, Sync sync, EGLint flags, EGLTimeKHR timeout)
{
    return eglClientWaitSync(dpy, sync, flags, timeout);
}

EGLint EGLWindow::clientWaitSyncKHR(EGLDisplay dpy, Sync sync, EGLint flags, EGLTimeKHR timeout)
{
    return eglClientWaitSyncKHR(dpy, sync, flags, timeout);
}

EGLint EGLWindow::getEGLError()
{
    return eglGetError();
}

EGLWindow::Display EGLWindow::getCurrentDisplay()
{
    return eglGetCurrentDisplay();
}

GLWindowBase::Surface EGLWindow::createPbufferSurface(const EGLint *attrib_list)
{
    return eglCreatePbufferSurface(getDisplay(), getConfig(), attrib_list);
}

EGLBoolean EGLWindow::destroySurface(Surface surface)
{
    return eglDestroySurface(getDisplay(), surface);
}

EGLBoolean EGLWindow::bindTexImage(EGLSurface surface, EGLint buffer)
{
    return eglBindTexImage(getDisplay(), surface, buffer);
}

EGLBoolean EGLWindow::releaseTexImage(EGLSurface surface, EGLint buffer)
{
    return eglReleaseTexImage(getDisplay(), surface, buffer);
}

bool EGLWindow::makeCurrent(EGLSurface draw, EGLSurface read, EGLContext context)
{
    if ((draw && !read) || (!draw && read))
    {
        fprintf(stderr, "eglMakeCurrent: setting only one of draw and read buffer is illegal\n");
        return false;
    }

    // if the draw buffer is a nullptr and a context is given, then we use mSurface,
    // because we didn't add this the gSurfaceMap, and it is the most likely
    // case that we actually wanted the default surface here.
    // TODO: This will need additional work when we want to support capture/replay
    // with a sourfaceless context.
    //
    // If no context is given then we also don't assign a surface
    if (!draw)
    {
        draw = read = context != EGL_NO_CONTEXT ? mSurface : EGL_NO_SURFACE;
    }

    if (isGLInitialized())
    {
        if (eglMakeCurrent(mDisplay, draw, read, context) == EGL_FALSE ||
            eglGetError() != EGL_SUCCESS)
        {
            fprintf(stderr, "Error during eglMakeCurrent.\n");
            return false;
        }
    }
    return true;
}

bool EGLWindow::makeCurrent(EGLContext context)
{
    return makeCurrent(mSurface, mSurface, context);
}

bool EGLWindow::setSwapInterval(EGLint swapInterval)
{
    if (eglSwapInterval(mDisplay, swapInterval) == EGL_FALSE || eglGetError() != EGL_SUCCESS)
    {
        fprintf(stderr, "Error during eglSwapInterval.\n");
        return false;
    }

    return true;
}

bool EGLWindow::hasError() const
{
    return eglGetError() != EGL_SUCCESS;
}

angle::GenericProc EGLWindow::getProcAddress(const char *name)
{
    return eglGetProcAddress(name);
}

// static
void GLWindowBase::Delete(GLWindowBase **window)
{
    delete *window;
    *window = nullptr;
}

// static
EGLWindow *EGLWindow::New(EGLint glesMajorVersion, EGLint glesMinorVersion)
{
    return new EGLWindow(glesMajorVersion, glesMinorVersion);
}

// static
void EGLWindow::Delete(EGLWindow **window)
{
    delete *window;
    *window = nullptr;
}

void EGLWindow::queryFeatures()
{
    // EGL_NO_DISPLAY + EGL_EXTENSIONS returns NULL before Android 10
    const char *extensionString =
        static_cast<const char *>(eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS));
    const bool hasFeatureControlANGLE =
        extensionString && strstr(extensionString, "EGL_ANGLE_feature_control") != nullptr;

    if (!hasFeatureControlANGLE)
    {
        return;
    }

    angle::HashMap<std::string, angle::Feature> featureFromName;
    for (angle::Feature feature : angle::AllEnums<angle::Feature>())
    {
        featureFromName[angle::GetFeatureName(feature)] = feature;
    }

    EGLAttrib featureCount = -1;
    eglQueryDisplayAttribANGLE(mDisplay, EGL_FEATURE_COUNT_ANGLE, &featureCount);

    for (int index = 0; index < featureCount; index++)
    {
        const char *featureName   = eglQueryStringiANGLE(mDisplay, EGL_FEATURE_NAME_ANGLE, index);
        const char *featureStatus = eglQueryStringiANGLE(mDisplay, EGL_FEATURE_STATUS_ANGLE, index);
        ASSERT(featureName != nullptr);
        ASSERT(featureStatus != nullptr);

        const angle::Feature feature = featureFromName[featureName];

        const bool isEnabled  = strcmp(featureStatus, angle::kFeatureStatusEnabled) == 0;
        const bool isDisabled = strcmp(featureStatus, angle::kFeatureStatusDisabled) == 0;
        ASSERT(isEnabled || isDisabled);

        mFeatures[feature] = isEnabled ? ANGLEFeatureStatus::Enabled : ANGLEFeatureStatus::Disabled;
    }
}

bool EGLWindow::isFeatureEnabled(angle::Feature feature)
{
    return mFeatures[feature] == ANGLEFeatureStatus::Enabled;
}
