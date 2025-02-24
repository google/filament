//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayEGL.cpp: Common across EGL parts of platform specific egl::Display implementations

#include "libANGLE/renderer/gl/egl/DisplayEGL.h"

#include "common/debug.h"
#include "common/system_utils.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/gl/ContextGL.h"
#include "libANGLE/renderer/gl/RendererGL.h"
#include "libANGLE/renderer/gl/egl/ContextEGL.h"
#include "libANGLE/renderer/gl/egl/DeviceEGL.h"
#include "libANGLE/renderer/gl/egl/DmaBufImageSiblingEGL.h"
#include "libANGLE/renderer/gl/egl/FunctionsEGLDL.h"
#include "libANGLE/renderer/gl/egl/ImageEGL.h"
#include "libANGLE/renderer/gl/egl/PbufferSurfaceEGL.h"
#include "libANGLE/renderer/gl/egl/RendererEGL.h"
#include "libANGLE/renderer/gl/egl/SyncEGL.h"
#include "libANGLE/renderer/gl/egl/WindowSurfaceEGL.h"
#include "libANGLE/renderer/gl/renderergl_utils.h"

namespace
{

rx::RobustnessVideoMemoryPurgeStatus GetRobustnessVideoMemoryPurge(const egl::AttributeMap &attribs)
{
    return static_cast<rx::RobustnessVideoMemoryPurgeStatus>(
        attribs.get(EGL_GENERATE_RESET_ON_VIDEO_MEMORY_PURGE_NV, GL_FALSE));
}

std::vector<EGLint> RenderableTypesFromPlatformAttrib(const rx::FunctionsEGL *egl,
                                                      const EGLAttrib platformAttrib)
{
    std::vector<EGLint> renderableTypes;
    switch (platformAttrib)
    {
        case EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE:
        {
            static_assert(EGL_OPENGL_ES3_BIT == EGL_OPENGL_ES3_BIT_KHR,
                          "Extension define must match core");

            gl::Version eglVersion(egl->majorVersion, egl->minorVersion);
            if (eglVersion >= gl::Version(1, 5) || egl->hasExtension("EGL_KHR_create_context"))
            {
                renderableTypes.push_back(EGL_OPENGL_ES3_BIT);
            }
            renderableTypes.push_back(EGL_OPENGL_ES2_BIT);
        }
        break;

        default:
            break;
    }
    return renderableTypes;
}

template <typename... Rest>
egl::AttributeMap MergeAttributeMaps(const egl::AttributeMap &a)
{
    return a;
}

template <typename... Rest>
egl::AttributeMap MergeAttributeMaps(const egl::AttributeMap &a, Rest... rest)
{
    egl::AttributeMap result(a);
    for (const auto &attrib : MergeAttributeMaps(rest...))
    {
        ASSERT(!result.contains(attrib.first));
        result.insert(attrib.first, attrib.second);
    }
    return result;
}
}  // namespace

namespace rx
{

DisplayEGL::DisplayEGL(const egl::DisplayState &state) : DisplayGL(state) {}

DisplayEGL::~DisplayEGL() {}

ImageImpl *DisplayEGL::createImage(const egl::ImageState &state,
                                   const gl::Context *context,
                                   EGLenum target,
                                   const egl::AttributeMap &attribs)
{
    return new ImageEGL(state, context, target, attribs, mEGL);
}

EGLSyncImpl *DisplayEGL::createSync()
{
    return new SyncEGL(mEGL);
}

const char *DisplayEGL::getEGLPath() const
{
#if defined(ANGLE_PLATFORM_ANDROID)
#    if defined(__LP64__)
    return "/system/lib64/libEGL.so";
#    else
    return "/system/lib/libEGL.so";
#    endif
#else
    return "libEGL.so.1";
#endif
}

egl::Error DisplayEGL::initializeContext(EGLContext shareContext,
                                         const egl::AttributeMap &eglAttributes,
                                         EGLContext *outContext) const
{
    gl::Version eglVersion(mEGL->majorVersion, mEGL->minorVersion);

    EGLint requestedMajor =
        eglAttributes.getAsInt(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, EGL_DONT_CARE);
    EGLint requestedMinor =
        eglAttributes.getAsInt(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, EGL_DONT_CARE);
    bool initializeRequested = requestedMajor != EGL_DONT_CARE && requestedMinor != EGL_DONT_CARE;

    static_assert(EGL_CONTEXT_MAJOR_VERSION == EGL_CONTEXT_MAJOR_VERSION_KHR,
                  "Major Version define should match");
    static_assert(EGL_CONTEXT_MINOR_VERSION == EGL_CONTEXT_MINOR_VERSION_KHR,
                  "Minor Version define should match");

    std::vector<egl::AttributeMap> contextAttribLists;
    if (eglVersion >= gl::Version(1, 5) || mEGL->hasExtension("EGL_KHR_create_context"))
    {
        if (initializeRequested)
        {
            egl::AttributeMap requestedVersionAttribs;
            requestedVersionAttribs.insert(EGL_CONTEXT_MAJOR_VERSION, requestedMajor);
            requestedVersionAttribs.insert(EGL_CONTEXT_MINOR_VERSION, requestedMinor);

            contextAttribLists.push_back(std::move(requestedVersionAttribs));
        }
        else
        {
            // clang-format off
            const gl::Version esVersionsFrom2_0[] = {
                gl::Version(3, 2),
                gl::Version(3, 1),
                gl::Version(3, 0),
                gl::Version(2, 0),
            };
            // clang-format on

            for (const auto &version : esVersionsFrom2_0)
            {
                egl::AttributeMap versionAttribs;
                versionAttribs.insert(EGL_CONTEXT_MAJOR_VERSION,
                                      static_cast<EGLint>(version.major));
                versionAttribs.insert(EGL_CONTEXT_MINOR_VERSION,
                                      static_cast<EGLint>(version.minor));

                contextAttribLists.push_back(std::move(versionAttribs));
            }
        }
    }
    else
    {
        if (initializeRequested && (requestedMajor != 2 || requestedMinor != 0))
        {
            return egl::EglBadAttribute() << "Unsupported requested context version";
        }

        egl::AttributeMap fallbackAttribs;
        fallbackAttribs.insert(EGL_CONTEXT_CLIENT_VERSION, 2);

        contextAttribLists.push_back(std::move(fallbackAttribs));
    }

    for (const egl::AttributeMap &attribs : contextAttribLists)
    {
        // If robustness is supported, try to create a context with robustness enabled. If it fails,
        // fall back to creating a context without the robustness parameters. We've seen devices
        // that expose the robustness extensions but fail to create robust contexts.
        if (mHasEXTCreateContextRobustness)
        {
            egl::AttributeMap attribsWithRobustness(attribs);

            attribsWithRobustness.insert(EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY,
                                         EGL_LOSE_CONTEXT_ON_RESET);
            if (mHasNVRobustnessVideoMemoryPurge)
            {
                attribsWithRobustness.insert(EGL_GENERATE_RESET_ON_VIDEO_MEMORY_PURGE_NV, GL_TRUE);
            }

            native_egl::AttributeVector attribVector = attribsWithRobustness.toIntVector();
            EGLContext context = mEGL->createContext(mConfig, shareContext, attribVector.data());
            if (context != EGL_NO_CONTEXT)
            {
                *outContext = context;
                return egl::NoError();
            }

            INFO() << "EGL_EXT_create_context_robustness available but robust context creation "
                      "failed.";
        }

        native_egl::AttributeVector attribVector = attribs.toIntVector();
        EGLContext context = mEGL->createContext(mConfig, shareContext, attribVector.data());
        if (context != EGL_NO_CONTEXT)
        {
            *outContext = context;
            return egl::NoError();
        }
    }

    return egl::Error(mEGL->getError(), "eglCreateContext failed");
}

egl::Error DisplayEGL::findConfig(egl::Display *display,
                                  bool forMockPbuffer,
                                  EGLConfig *outConfig,
                                  std::vector<EGLint> *outConfigAttribs)
{
    const EGLAttrib platformAttrib      = mDisplayAttributes.get(EGL_PLATFORM_ANGLE_TYPE_ANGLE,
                                                                 EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE);
    std::vector<EGLint> renderableTypes = RenderableTypesFromPlatformAttrib(mEGL, platformAttrib);
    if (renderableTypes.empty())
    {
        return egl::EglNotInitialized() << "No available renderable types.";
    }

    EGLint surfaceType = EGL_DONT_CARE;
    if (forMockPbuffer)
    {
        surfaceType = EGL_PBUFFER_BIT;
    }
    else if (!mSupportsSurfaceless)
    {
        surfaceType = EGL_WINDOW_BIT | EGL_PBUFFER_BIT;
    }

    egl::AttributeMap rootAttribs;
    rootAttribs.insert(EGL_SURFACE_TYPE, surfaceType);

    egl::AttributeMap rgba888Attribs;
    rgba888Attribs.insert(EGL_RED_SIZE, 8);
    rgba888Attribs.insert(EGL_GREEN_SIZE, 8);
    rgba888Attribs.insert(EGL_BLUE_SIZE, 8);
    rgba888Attribs.insert(EGL_ALPHA_SIZE, 8);

    egl::AttributeMap rgb565Attribs;
    rgb565Attribs.insert(EGL_RED_SIZE, 5);
    rgb565Attribs.insert(EGL_GREEN_SIZE, 6);
    rgb565Attribs.insert(EGL_BLUE_SIZE, 5);
    rgb565Attribs.insert(EGL_BUFFER_SIZE, 16);

    egl::AttributeMap ds248Attribs;
    ds248Attribs.insert(EGL_DEPTH_SIZE, 24);
    ds248Attribs.insert(EGL_STENCIL_SIZE, 8);

    egl::AttributeMap d16Attribs;
    ds248Attribs.insert(EGL_DEPTH_SIZE, 16);

    egl::AttributeMap attributePermutations[] = {
        // First try RGBA8 + any depth/stencil
        MergeAttributeMaps(rootAttribs, rgba888Attribs, ds248Attribs),
        MergeAttributeMaps(rootAttribs, rgba888Attribs, d16Attribs),
        // Fall back to RGB565 + any depth/stencil
        MergeAttributeMaps(rootAttribs, rgb565Attribs, ds248Attribs),
        MergeAttributeMaps(rootAttribs, rgb565Attribs, d16Attribs),
        // Accept no depth stencil if that's all there is
        MergeAttributeMaps(rootAttribs, rgba888Attribs),
        MergeAttributeMaps(rootAttribs, rgb565Attribs),
    };

    for (const egl::AttributeMap &attributePermutation : attributePermutations)
    {
        for (EGLint renderableType : renderableTypes)
        {
            egl::AttributeMap configAttribs = attributePermutation;
            configAttribs.insert(EGL_RENDERABLE_TYPE, renderableType);

            std::vector<EGLint> attribVector = configAttribs.toIntVector();

            EGLConfig config = EGL_NO_CONFIG_KHR;
            EGLint numConfig = 0;
            if (mEGL->chooseConfig(attribVector.data(), &config, 1, &numConfig) == EGL_TRUE &&
                numConfig > 0)
            {
                *outConfig = config;
                if (outConfigAttribs)
                {
                    *outConfigAttribs = configAttribs.toIntVector();
                }
                return egl::NoError();
            }
        }
    }

    return egl::EglNotInitialized()
           << "Failed to find a usable config. Last error: " << egl::Error(mEGL->getError());
}

egl::Error DisplayEGL::initialize(egl::Display *display)
{
    mDisplayAttributes = display->getAttributeMap();
    mEGL               = new FunctionsEGLDL();

    void *eglHandle =
        reinterpret_cast<void *>(mDisplayAttributes.get(EGL_PLATFORM_ANGLE_EGL_HANDLE_ANGLE, 0));
    EGLAttrib platformType =
        mDisplayAttributes.get(EGL_PLATFORM_ANGLE_NATIVE_PLATFORM_TYPE_ANGLE, 0);
    ANGLE_TRY(
        mEGL->initialize(platformType, display->getNativeDisplayId(), getEGLPath(), eglHandle));

    gl::Version eglVersion(mEGL->majorVersion, mEGL->minorVersion);
    if (eglVersion < gl::Version(1, 4))
    {
        return egl::EglNotInitialized() << "EGL >= 1.4 is required";
    }

    // https://anglebug.com/42266130
    // TODO: turn this into a feature so we can communicate that this is disabled on purpose.
    mSupportsDmaBufImportModifiers = mEGL->hasExtension("EGL_EXT_image_dma_buf_import_modifiers");

    bool isKevin = mEGL->vendorString.find("ARM") != std::string::npos &&
                   mEGL->versionString.find("r26p0-01rel0") != std::string::npos;
    mNoOpDmaBufImportModifiers = isKevin || !mEGL->hasDmaBufImportModifierFunctions();

    mHasEXTCreateContextRobustness   = mEGL->hasExtension("EGL_EXT_create_context_robustness");
    mHasNVRobustnessVideoMemoryPurge = mEGL->hasExtension("EGL_NV_robustness_video_memory_purge");
    mSupportsNoConfigContexts        = mEGL->hasExtension("EGL_KHR_no_config_context") ||
                                mEGL->hasExtension("EGL_KHR_no_config_context");
    mSupportsSurfaceless = mEGL->hasExtension("EGL_KHR_surfaceless_context");

    if (!mSupportsNoConfigContexts)
    {
        ANGLE_TRY(findConfig(display, false, &mConfig, &mConfigAttribList));
    }

    // A mock pbuffer is only needed if surfaceless contexts are not supported.
    if (!mSupportsSurfaceless)
    {
        EGLConfig pbufferConfig;
        ANGLE_TRY(findConfig(display, true, &pbufferConfig, nullptr));

        constexpr const int mockPbufferAttribs[] = {
            EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE,
        };

        mMockPbuffer = mEGL->createPbufferSurface(pbufferConfig, mockPbufferAttribs);
        if (mMockPbuffer == EGL_NO_SURFACE)
        {
            return egl::EglNotInitialized()
                   << "eglCreatePbufferSurface failed with " << egl::Error(mEGL->getError());
        }
    }

    ANGLE_TRY(createRenderer(EGL_NO_CONTEXT, true, false, &mRenderer));

    const gl::Version &maxVersion = mRenderer->getMaxSupportedESVersion();
    if (maxVersion < gl::Version(2, 0))
    {
        return egl::EglNotInitialized() << "OpenGL ES 2.0 is not supportable.";
    }

    ANGLE_TRY(DisplayGL::initialize(display));

    INFO() << "ANGLE DisplayEGL initialized: " << getRendererDescription();

    return egl::NoError();
}

void DisplayEGL::terminate()
{
    DisplayGL::terminate();

    EGLBoolean success = mEGL->makeCurrent(EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (success == EGL_FALSE)
    {
        ERR() << "eglMakeCurrent error " << egl::Error(mEGL->getError());
    }

    if (mMockPbuffer != EGL_NO_SURFACE)
    {
        success      = mEGL->destroySurface(mMockPbuffer);
        mMockPbuffer = EGL_NO_SURFACE;
        if (success == EGL_FALSE)
        {
            ERR() << "eglDestroySurface error " << egl::Error(mEGL->getError());
        }
    }

    mRenderer.reset();
    mVirtualizationGroups.clear();

    mCurrentNativeContexts.clear();

    egl::Error result = mEGL->terminate();
    if (result.isError())
    {
        ERR() << "eglTerminate error " << result;
    }

    SafeDelete(mEGL);
}

SurfaceImpl *DisplayEGL::createWindowSurface(const egl::SurfaceState &state,
                                             EGLNativeWindowType window,
                                             const egl::AttributeMap &attribs)
{
    EGLConfig config;
    EGLint numConfig;
    EGLBoolean success;

    const EGLint configAttribList[] = {EGL_CONFIG_ID, mConfigIds[state.config->configID], EGL_NONE};
    success                         = mEGL->chooseConfig(configAttribList, &config, 1, &numConfig);
    ASSERT(success && numConfig == 1);

    return new WindowSurfaceEGL(state, mEGL, config, window);
}

SurfaceImpl *DisplayEGL::createPbufferSurface(const egl::SurfaceState &state,
                                              const egl::AttributeMap &attribs)
{
    EGLConfig config;
    EGLint numConfig;
    EGLBoolean success;

    const EGLint configAttribList[] = {EGL_CONFIG_ID, mConfigIds[state.config->configID], EGL_NONE};
    success                         = mEGL->chooseConfig(configAttribList, &config, 1, &numConfig);
    ASSERT(success && numConfig == 1);

    return new PbufferSurfaceEGL(state, mEGL, config);
}

class ExternalSurfaceEGL : public SurfaceEGL
{
  public:
    ExternalSurfaceEGL(const egl::SurfaceState &state,
                       const FunctionsEGL *egl,
                       EGLConfig config,
                       EGLint width,
                       EGLint height)
        : SurfaceEGL(state, egl, config), mWidth(width), mHeight(height)
    {}
    ~ExternalSurfaceEGL() override = default;

    egl::Error initialize(const egl::Display *display) override { return egl::NoError(); }
    EGLint getSwapBehavior() const override { return EGL_BUFFER_DESTROYED; }
    EGLint getWidth() const override { return mWidth; }
    EGLint getHeight() const override { return mHeight; }
    bool isExternal() const override { return true; }

  private:
    const EGLint mWidth;
    const EGLint mHeight;
};

SurfaceImpl *DisplayEGL::createPbufferFromClientBuffer(const egl::SurfaceState &state,
                                                       EGLenum buftype,
                                                       EGLClientBuffer clientBuffer,
                                                       const egl::AttributeMap &attribs)
{
    switch (buftype)
    {
        case EGL_EXTERNAL_SURFACE_ANGLE:
            return new ExternalSurfaceEGL(state, mEGL, EGL_NO_CONFIG_KHR,
                                          attribs.getAsInt(EGL_WIDTH, 0),
                                          attribs.getAsInt(EGL_HEIGHT, 0));

        default:
            return DisplayGL::createPbufferFromClientBuffer(state, buftype, clientBuffer, attribs);
    }
}

SurfaceImpl *DisplayEGL::createPixmapSurface(const egl::SurfaceState &state,
                                             NativePixmapType nativePixmap,
                                             const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

ContextImpl *DisplayEGL::createContext(const gl::State &state,
                                       gl::ErrorSet *errorSet,
                                       const egl::Config *configuration,
                                       const gl::Context *shareContext,
                                       const egl::AttributeMap &attribs)
{
    bool usingExternalContext = attribs.get(EGL_EXTERNAL_CONTEXT_ANGLE, EGL_FALSE) == EGL_TRUE;
    EGLAttrib virtualizationGroup =
        attribs.get(EGL_CONTEXT_VIRTUALIZATION_GROUP_ANGLE, EGL_DONT_CARE);
    bool globalTextureShareGroup =
        attribs.get(EGL_DISPLAY_TEXTURE_SHARE_GROUP_ANGLE, EGL_FALSE) == EGL_TRUE;

    std::shared_ptr<RendererEGL> renderer = mRenderer;
    if (usingExternalContext)
    {
        ASSERT(!shareContext);
        egl::Error error = createRenderer(EGL_NO_CONTEXT, false, true, &renderer);
        if (error.isError())
        {
            ERR() << "Failed to create a shared renderer: " << error.getMessage();
            return nullptr;
        }
    }
    else if (virtualizationGroup != EGL_DONT_CARE)
    {
        renderer = mVirtualizationGroups[virtualizationGroup].lock();
        if (!renderer)
        {
            // If the user requested a dispaly-level texture share group, all contexts must be in
            // the same share group. Otherwise honor the user's share group request.
            EGLContext nativeShareContext = EGL_NO_CONTEXT;
            if (globalTextureShareGroup)
            {
                nativeShareContext = mRenderer->getContext();
            }
            else if (shareContext)
            {
                ContextEGL *shareContextEGL = GetImplAs<ContextEGL>(shareContext);
                nativeShareContext          = shareContextEGL->getContext();
            }

            // Create a new renderer for this context.
            egl::Error error = createRenderer(nativeShareContext, false, false, &renderer);
            if (error.isError())
            {
                ERR() << "Failed to create a shared renderer: " << error.getMessage();
                return nullptr;
            }

            mVirtualizationGroups[virtualizationGroup] = renderer;
        }
    }
    ASSERT(renderer);

    RobustnessVideoMemoryPurgeStatus robustnessVideoMemoryPurgeStatus =
        GetRobustnessVideoMemoryPurge(attribs);
    return new ContextEGL(state, errorSet, renderer, robustnessVideoMemoryPurgeStatus);
}

template <typename T>
void DisplayEGL::getConfigAttrib(EGLConfig config, EGLint attribute, T *value) const
{
    EGLint tmp         = *value;
    EGLBoolean success = mEGL->getConfigAttrib(config, attribute, &tmp);
    ASSERT(success == EGL_TRUE);
    *value = tmp;
}

template <typename T, typename U>
void DisplayEGL::getConfigAttribIfExtension(EGLConfig config,
                                            EGLint attribute,
                                            T *value,
                                            const char *extension,
                                            const U &defaultValue) const
{
    if (mEGL->hasExtension(extension))
    {
        getConfigAttrib(config, attribute, value);
    }
    else
    {
        *value = static_cast<T>(defaultValue);
    }
}

egl::ConfigSet DisplayEGL::generateConfigs()
{
    egl::ConfigSet configSet;
    mConfigIds.clear();

    std::vector<EGLConfig> configs;
    if (mSupportsNoConfigContexts)
    {
        // Gather all configs
        EGLint numConfigs;
        EGLBoolean success = mEGL->getConfigs(nullptr, 0, &numConfigs);
        ASSERT(success == EGL_TRUE && numConfigs > 0);

        configs.resize(numConfigs);
        EGLint numConfigs2;
        success = mEGL->getConfigs(configs.data(), numConfigs, &numConfigs2);
        ASSERT(success == EGL_TRUE && numConfigs2 == numConfigs);
    }
    else
    {
        // Choose configs that match the attribute list of the config used for the context
        EGLint numConfigs;
        EGLBoolean success = mEGL->chooseConfig(mConfigAttribList.data(), nullptr, 0, &numConfigs);
        ASSERT(success == EGL_TRUE && numConfigs > 0);

        configs.resize(numConfigs);
        EGLint numConfigs2;
        success =
            mEGL->chooseConfig(mConfigAttribList.data(), configs.data(), numConfigs, &numConfigs2);
        ASSERT(success == EGL_TRUE && numConfigs2 == numConfigs);
    }

    for (size_t i = 0; i < configs.size(); i++)
    {
        egl::Config config;

        getConfigAttrib(configs[i], EGL_BUFFER_SIZE, &config.bufferSize);
        getConfigAttrib(configs[i], EGL_RED_SIZE, &config.redSize);
        getConfigAttrib(configs[i], EGL_GREEN_SIZE, &config.greenSize);
        getConfigAttrib(configs[i], EGL_BLUE_SIZE, &config.blueSize);
        getConfigAttrib(configs[i], EGL_LUMINANCE_SIZE, &config.luminanceSize);
        getConfigAttrib(configs[i], EGL_ALPHA_SIZE, &config.alphaSize);
        getConfigAttrib(configs[i], EGL_ALPHA_MASK_SIZE, &config.alphaMaskSize);
        getConfigAttrib(configs[i], EGL_BIND_TO_TEXTURE_RGB, &config.bindToTextureRGB);
        getConfigAttrib(configs[i], EGL_BIND_TO_TEXTURE_RGBA, &config.bindToTextureRGBA);
        getConfigAttrib(configs[i], EGL_COLOR_BUFFER_TYPE, &config.colorBufferType);
        getConfigAttrib(configs[i], EGL_CONFIG_CAVEAT, &config.configCaveat);
        getConfigAttrib(configs[i], EGL_CONFIG_ID, &config.configID);
        getConfigAttrib(configs[i], EGL_CONFORMANT, &config.conformant);
        getConfigAttrib(configs[i], EGL_DEPTH_SIZE, &config.depthSize);
        getConfigAttrib(configs[i], EGL_LEVEL, &config.level);
        getConfigAttrib(configs[i], EGL_MAX_PBUFFER_WIDTH, &config.maxPBufferWidth);
        getConfigAttrib(configs[i], EGL_MAX_PBUFFER_HEIGHT, &config.maxPBufferHeight);
        getConfigAttrib(configs[i], EGL_MAX_PBUFFER_PIXELS, &config.maxPBufferPixels);
        getConfigAttrib(configs[i], EGL_MAX_SWAP_INTERVAL, &config.maxSwapInterval);
        getConfigAttrib(configs[i], EGL_MIN_SWAP_INTERVAL, &config.minSwapInterval);
        getConfigAttrib(configs[i], EGL_NATIVE_RENDERABLE, &config.nativeRenderable);
        getConfigAttrib(configs[i], EGL_NATIVE_VISUAL_ID, &config.nativeVisualID);
        getConfigAttrib(configs[i], EGL_NATIVE_VISUAL_TYPE, &config.nativeVisualType);
        getConfigAttrib(configs[i], EGL_RENDERABLE_TYPE, &config.renderableType);
        getConfigAttrib(configs[i], EGL_SAMPLE_BUFFERS, &config.sampleBuffers);
        getConfigAttrib(configs[i], EGL_SAMPLES, &config.samples);
        getConfigAttrib(configs[i], EGL_STENCIL_SIZE, &config.stencilSize);
        getConfigAttrib(configs[i], EGL_SURFACE_TYPE, &config.surfaceType);
        getConfigAttrib(configs[i], EGL_TRANSPARENT_TYPE, &config.transparentType);
        getConfigAttrib(configs[i], EGL_TRANSPARENT_RED_VALUE, &config.transparentRedValue);
        getConfigAttrib(configs[i], EGL_TRANSPARENT_GREEN_VALUE, &config.transparentGreenValue);
        getConfigAttrib(configs[i], EGL_TRANSPARENT_BLUE_VALUE, &config.transparentBlueValue);
        getConfigAttribIfExtension(configs[i], EGL_COLOR_COMPONENT_TYPE_EXT,
                                   &config.colorComponentType, "EGL_EXT_pixel_format_float",
                                   EGL_COLOR_COMPONENT_TYPE_FIXED_EXT);

        config.surfaceType = fixSurfaceType(config.surfaceType);

        if (config.colorBufferType == EGL_RGB_BUFFER)
        {
            config.renderTargetFormat = gl::GetConfigColorBufferFormat(&config);
            if (config.renderTargetFormat == GL_NONE)
            {
                ERR() << "RGBA(" << config.redSize << "," << config.greenSize << ","
                      << config.blueSize << "," << config.alphaSize << ") not handled";
                continue;
            }
        }
        else if (config.colorBufferType == EGL_LUMINANCE_BUFFER ||
                 config.colorBufferType == EGL_YUV_BUFFER_EXT)
        {
            // YUV and luminance EGL configs are not exposed, the frontened has not implemented
            // them.
            continue;
        }
        else
        {
            WARN() << "Unknown EGL color buffer type " << gl::FmtHex(config.colorBufferType)
                   << ", skipping.";
            continue;
        }

        config.depthStencilFormat = gl::GetConfigDepthStencilBufferFormat(&config);

        config.matchNativePixmap  = EGL_NONE;
        config.optimalOrientation = 0;

        int internalId         = configSet.add(config);
        mConfigIds[internalId] = config.configID;
    }

    return configSet;
}

bool DisplayEGL::testDeviceLost()
{
    return false;
}

egl::Error DisplayEGL::restoreLostDevice(const egl::Display *display)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

bool DisplayEGL::isValidNativeWindow(EGLNativeWindowType window) const
{
    return true;
}

egl::Error DisplayEGL::validateClientBuffer(const egl::Config *configuration,
                                            EGLenum buftype,
                                            EGLClientBuffer clientBuffer,
                                            const egl::AttributeMap &attribs) const
{
    switch (buftype)
    {
        case EGL_EXTERNAL_SURFACE_ANGLE:
            ASSERT(clientBuffer == nullptr);
            return egl::NoError();

        default:
            return DisplayGL::validateClientBuffer(configuration, buftype, clientBuffer, attribs);
    }
}

egl::Error DisplayEGL::waitClient(const gl::Context *context)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

egl::Error DisplayEGL::waitNative(const gl::Context *context, EGLint engine)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

egl::Error DisplayEGL::makeCurrent(egl::Display *display,
                                   egl::Surface *drawSurface,
                                   egl::Surface *readSurface,
                                   gl::Context *context)
{
    CurrentNativeContext &currentContext =
        mCurrentNativeContexts[angle::GetCurrentThreadUniqueId()];

    EGLSurface newSurface = EGL_NO_SURFACE;
    if (drawSurface)
    {
        SurfaceEGL *drawSurfaceEGL = GetImplAs<SurfaceEGL>(drawSurface);
        newSurface                 = drawSurfaceEGL->getSurface();
    }

    EGLContext newContext = EGL_NO_CONTEXT;
    if (context)
    {
        ContextEGL *contextEGL = GetImplAs<ContextEGL>(context);
        newContext             = contextEGL->getContext();
    }

    if (currentContext.isExternalContext || (context && context->isExternal()))
    {
        ASSERT(currentContext.surface == EGL_NO_SURFACE);
        if (!currentContext.isExternalContext)
        {
            // Switch to an ANGLE external context.
            ASSERT(context);
            ASSERT(currentContext.context == EGL_NO_CONTEXT);
            currentContext.context           = newContext;
            currentContext.isExternalContext = true;

            // We only support using external surface with external context.
            ASSERT(GetImplAs<SurfaceEGL>(drawSurface)->isExternal());
            ASSERT(GetImplAs<SurfaceEGL>(drawSurface)->getSurface() == EGL_NO_SURFACE);
        }
        else if (context)
        {
            // Switch surface but not context.
            ASSERT(currentContext.context == newContext);
            ASSERT(newSurface == EGL_NO_SURFACE);
            ASSERT(newContext != EGL_NO_CONTEXT);
            // We only support using external surface with external context.
            ASSERT(GetImplAs<SurfaceEGL>(drawSurface)->isExternal());
            ASSERT(GetImplAs<SurfaceEGL>(drawSurface)->getSurface() == EGL_NO_SURFACE);
        }
        else
        {
            // Release the ANGLE external context.
            ASSERT(newSurface == EGL_NO_SURFACE);
            ASSERT(newContext == EGL_NO_CONTEXT);
            ASSERT(currentContext.context != EGL_NO_CONTEXT);
            currentContext.context           = EGL_NO_CONTEXT;
            currentContext.isExternalContext = false;
        }

        // Do not need to call eglMakeCurrent(), since we don't support switching EGLSurface for
        // external context.
        return DisplayGL::makeCurrent(display, drawSurface, readSurface, context);
    }

    if (newSurface != currentContext.surface || newContext != currentContext.context)
    {
        if (mEGL->makeCurrent(newSurface, newContext) == EGL_FALSE)
        {
            return egl::Error(mEGL->getError(), "eglMakeCurrent failed");
        }
        currentContext.surface = newSurface;
        currentContext.context = newContext;
    }

    return DisplayGL::makeCurrent(display, drawSurface, readSurface, context);
}

gl::Version DisplayEGL::getMaxSupportedESVersion() const
{
    return mRenderer->getMaxSupportedESVersion();
}

void DisplayEGL::destroyNativeContext(EGLContext context)
{
    // If this context is current, remove it from the tracking of current contexts to make sure we
    // don't try to make it current again.
    for (auto &currentContext : mCurrentNativeContexts)
    {
        if (currentContext.second.context == context)
        {
            currentContext.second.surface = EGL_NO_SURFACE;
            currentContext.second.context = EGL_NO_CONTEXT;
        }
    }

    mEGL->destroyContext(context);
}

void DisplayEGL::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
    gl::Version eglVersion(mEGL->majorVersion, mEGL->minorVersion);

    outExtensions->createContextRobustness =
        mEGL->hasExtension("EGL_EXT_create_context_robustness");

    outExtensions->postSubBuffer    = false;  // Since SurfaceEGL::postSubBuffer is not implemented
    outExtensions->presentationTime = mEGL->hasExtension("EGL_ANDROID_presentation_time");

    // Contexts are virtualized so textures and semaphores can be shared globally
    outExtensions->displayTextureShareGroup   = true;
    outExtensions->displaySemaphoreShareGroup = true;

    // We will fallback to regular swap if swapBuffersWithDamage isn't
    // supported, so indicate support here to keep validation happy.
    outExtensions->swapBuffersWithDamage = true;

    outExtensions->image     = mEGL->hasExtension("EGL_KHR_image");
    outExtensions->imageBase = mEGL->hasExtension("EGL_KHR_image_base");
    // Pixmaps are not supported in ANGLE's EGL implementation.
    // outExtensions->imagePixmap = mEGL->hasExtension("EGL_KHR_image_pixmap");
    outExtensions->glTexture2DImage      = mEGL->hasExtension("EGL_KHR_gl_texture_2D_image");
    outExtensions->glTextureCubemapImage = mEGL->hasExtension("EGL_KHR_gl_texture_cubemap_image");
    outExtensions->glTexture3DImage      = mEGL->hasExtension("EGL_KHR_gl_texture_3D_image");
    outExtensions->glRenderbufferImage   = mEGL->hasExtension("EGL_KHR_gl_renderbuffer_image");
    outExtensions->pixelFormatFloat      = mEGL->hasExtension("EGL_EXT_pixel_format_float");

    outExtensions->glColorspace = mEGL->hasExtension("EGL_KHR_gl_colorspace");
    if (outExtensions->glColorspace)
    {
        outExtensions->glColorspaceDisplayP3Linear =
            mEGL->hasExtension("EGL_EXT_gl_colorspace_display_p3_linear");
        outExtensions->glColorspaceDisplayP3 =
            mEGL->hasExtension("EGL_EXT_gl_colorspace_display_p3");
        outExtensions->glColorspaceScrgb = mEGL->hasExtension("EGL_EXT_gl_colorspace_scrgb");
        outExtensions->glColorspaceScrgbLinear =
            mEGL->hasExtension("EGL_EXT_gl_colorspace_scrgb_linear");
        outExtensions->glColorspaceDisplayP3Passthrough =
            mEGL->hasExtension("EGL_EXT_gl_colorspace_display_p3_passthrough");
        outExtensions->imageGlColorspace = mEGL->hasExtension("EGL_EXT_image_gl_colorspace");
    }

    outExtensions->imageNativeBuffer = mEGL->hasExtension("EGL_ANDROID_image_native_buffer");

    outExtensions->getFrameTimestamps = mEGL->hasExtension("EGL_ANDROID_get_frame_timestamps");

    outExtensions->fenceSync =
        eglVersion >= gl::Version(1, 5) || mEGL->hasExtension("EGL_KHR_fence_sync");
    outExtensions->waitSync =
        eglVersion >= gl::Version(1, 5) || mEGL->hasExtension("EGL_KHR_wait_sync");

    outExtensions->getNativeClientBufferANDROID =
        mEGL->hasExtension("EGL_ANDROID_get_native_client_buffer");

    outExtensions->createNativeClientBufferANDROID =
        mEGL->hasExtension("EGL_ANDROID_create_native_client_buffer");

    outExtensions->nativeFenceSyncANDROID = mEGL->hasExtension("EGL_ANDROID_native_fence_sync");

    outExtensions->noConfigContext = mSupportsNoConfigContexts;

    outExtensions->surfacelessContext = mEGL->hasExtension("EGL_KHR_surfaceless_context");

    outExtensions->framebufferTargetANDROID = mEGL->hasExtension("EGL_ANDROID_framebuffer_target");

    outExtensions->imageDmaBufImportEXT = mEGL->hasExtension("EGL_EXT_image_dma_buf_import");

    outExtensions->imageDmaBufImportModifiersEXT = mSupportsDmaBufImportModifiers;

    outExtensions->robustnessVideoMemoryPurgeNV = mHasNVRobustnessVideoMemoryPurge;

    outExtensions->bufferAgeEXT = mEGL->hasExtension("EGL_EXT_buffer_age");

    // Surfaceless can be support if the native driver supports it or we know that we are running on
    // a single thread (mVirtualizedContexts == true)
    outExtensions->surfacelessContext = mSupportsSurfaceless;

    outExtensions->externalContextAndSurface = true;

    outExtensions->contextVirtualizationANGLE = true;

    DisplayGL::generateExtensions(outExtensions);
}

void DisplayEGL::generateCaps(egl::Caps *outCaps) const
{
    outCaps->textureNPOT = true;  // Since we request GLES >= 2
}

void DisplayEGL::setBlobCacheFuncs(EGLSetBlobFuncANDROID set, EGLGetBlobFuncANDROID get)
{
    if (mEGL->hasExtension("EGL_ANDROID_blob_cache"))
    {
        mEGL->setBlobCacheFuncsANDROID(set, get);
    }
}

egl::Error DisplayEGL::makeCurrentSurfaceless(gl::Context *context)
{
    // Nothing to do because EGL always uses the same context and the previous surface can be left
    // current.
    return egl::NoError();
}

egl::Error DisplayEGL::createRenderer(EGLContext shareContext,
                                      bool makeNewContextCurrent,
                                      bool isExternalContext,
                                      std::shared_ptr<RendererEGL> *outRenderer)
{
    EGLContext context = EGL_NO_CONTEXT;

    // If isExternalContext is true, the external context is current, so we don't need to make the
    // mMockPbuffer current.
    if (isExternalContext)
    {
        ASSERT(shareContext == EGL_NO_CONTEXT);
        ASSERT(!makeNewContextCurrent);
        // TODO(penghuang): Should we consider creating a share context to avoid querying and
        // restoring GL context state? http://anglebug.com/42264046
        context = mEGL->getCurrentContext();
        ASSERT(context != EGL_NO_CONTEXT);
    }
    else
    {
        ANGLE_TRY(initializeContext(shareContext, mDisplayAttributes, &context));
        if (mEGL->makeCurrent(mMockPbuffer, context) == EGL_FALSE)
        {
            return egl::EglNotInitialized()
                   << "eglMakeCurrent failed with " << egl::Error(mEGL->getError());
        }
    }

    std::unique_ptr<FunctionsGL> functionsGL(mEGL->makeFunctionsGL());
    functionsGL->initialize(mDisplayAttributes);

    outRenderer->reset(new RendererEGL(std::move(functionsGL), mDisplayAttributes, this, context,
                                       isExternalContext));

    CurrentNativeContext &currentContext =
        mCurrentNativeContexts[angle::GetCurrentThreadUniqueId()];
    if (makeNewContextCurrent)
    {
        currentContext.surface = mMockPbuffer;
        currentContext.context = context;
    }
    else if (!isExternalContext)
    {
        // Reset the current context back to the previous state
        if (mEGL->makeCurrent(currentContext.surface, currentContext.context) == EGL_FALSE)
        {
            return egl::EglNotInitialized()
                   << "eglMakeCurrent failed with " << egl::Error(mEGL->getError());
        }
    }

    return egl::NoError();
}

void DisplayEGL::initializeFrontendFeatures(angle::FrontendFeatures *features) const
{
    mRenderer->initializeFrontendFeatures(features);
}

void DisplayEGL::populateFeatureList(angle::FeatureList *features)
{
    mRenderer->getFeatures().populateFeatureList(features);
}

RendererGL *DisplayEGL::getRenderer() const
{
    return mRenderer.get();
}

egl::Error DisplayEGL::validateImageClientBuffer(const gl::Context *context,
                                                 EGLenum target,
                                                 EGLClientBuffer clientBuffer,
                                                 const egl::AttributeMap &attribs) const
{
    switch (target)
    {
        case EGL_LINUX_DMA_BUF_EXT:
            return egl::NoError();

        default:
            return DisplayGL::validateImageClientBuffer(context, target, clientBuffer, attribs);
    }
}

ExternalImageSiblingImpl *DisplayEGL::createExternalImageSibling(const gl::Context *context,
                                                                 EGLenum target,
                                                                 EGLClientBuffer buffer,
                                                                 const egl::AttributeMap &attribs)
{
    switch (target)
    {
        case EGL_LINUX_DMA_BUF_EXT:
            ASSERT(context == nullptr);
            ASSERT(buffer == nullptr);
            return new DmaBufImageSiblingEGL(attribs);

        default:
            return DisplayGL::createExternalImageSibling(context, target, buffer, attribs);
    }
}

EGLint DisplayEGL::fixSurfaceType(EGLint surfaceType) const
{
    // Pixmaps are not supported on EGL, make sure the config doesn't expose them.
    return surfaceType & ~EGL_PIXMAP_BIT;
}

const FunctionsEGL *DisplayEGL::getFunctionsEGL() const
{
    return mEGL;
}

DeviceImpl *DisplayEGL::createDevice()
{
    return new DeviceEGL(this);
}

bool DisplayEGL::supportsDmaBufFormat(EGLint format) const
{
    return std::find(std::begin(mDrmFormats), std::end(mDrmFormats), format) !=
           std::end(mDrmFormats);
}

egl::Error DisplayEGL::queryDmaBufFormats(EGLint maxFormats, EGLint *formats, EGLint *numFormats)

{
    if (!mDrmFormatsInitialized)
    {
        if (!mNoOpDmaBufImportModifiers)
        {
            EGLint numFormatsInit = 0;
            if (mEGL->queryDmaBufFormatsEXT(0, nullptr, &numFormatsInit) && numFormatsInit > 0)
            {
                mDrmFormats.resize(numFormatsInit);
                if (!mEGL->queryDmaBufFormatsEXT(numFormatsInit, mDrmFormats.data(),
                                                 &numFormatsInit))
                {
                    mDrmFormats.resize(0);
                }
            }
        }
        mDrmFormatsInitialized = true;
    }

    EGLint formatsSize = static_cast<EGLint>(mDrmFormats.size());
    *numFormats        = formatsSize;
    if (maxFormats > 0)
    {
        // Do not copy data beyond the limits of the vector
        maxFormats = std::min(maxFormats, formatsSize);
        std::memcpy(formats, mDrmFormats.data(), maxFormats * sizeof(EGLint));
    }

    return egl::NoError();
}

egl::Error DisplayEGL::queryDmaBufModifiers(EGLint drmFormat,
                                            EGLint maxModifiers,
                                            EGLuint64KHR *modifiers,
                                            EGLBoolean *externalOnly,
                                            EGLint *numModifiers)

{
    *numModifiers = 0;
    if (!mNoOpDmaBufImportModifiers)
    {
        if (!mEGL->queryDmaBufModifiersEXT(drmFormat, maxModifiers, modifiers, externalOnly,
                                           numModifiers))
        {
            return egl::Error(mEGL->getError(), "eglQueryDmaBufModifiersEXT failed");
        }
    }

    return egl::NoError();
}

}  // namespace rx
