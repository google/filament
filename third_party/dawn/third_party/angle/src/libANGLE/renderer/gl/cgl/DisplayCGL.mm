//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayCGL.mm: CGL implementation of egl::Display

#import "libANGLE/renderer/gl/cgl/DisplayCGL.h"

#import <Cocoa/Cocoa.h>
#import <EGL/eglext.h>
#import <dlfcn.h>

#import "common/debug.h"
#import "common/gl/cgl/FunctionsCGL.h"
#import "common/system_utils.h"
#import "gpu_info_util/SystemInfo_internal.h"
#import "libANGLE/Display.h"
#import "libANGLE/Error.h"
#import "libANGLE/renderer/gl/RendererGL.h"
#import "libANGLE/renderer/gl/cgl/ContextCGL.h"
#import "libANGLE/renderer/gl/cgl/DeviceCGL.h"
#import "libANGLE/renderer/gl/cgl/IOSurfaceSurfaceCGL.h"
#import "libANGLE/renderer/gl/cgl/PbufferSurfaceCGL.h"
#import "libANGLE/renderer/gl/cgl/WindowSurfaceCGL.h"
#import "platform/PlatformMethods.h"

namespace
{

const char *kDefaultOpenGLDylibName =
    "/System/Library/Frameworks/OpenGL.framework/Libraries/libGL.dylib";
const char *kFallbackOpenGLDylibName = "GL";

}  // namespace

namespace rx
{

namespace
{

// Global IOKit I/O registryID that can match a GPU across process boundaries.
using IORegistryGPUID = uint64_t;

// Code from WebKit to set an OpenGL context to use a particular GPU by ID.
// https://trac.webkit.org/browser/webkit/trunk/Source/WebCore/platform/graphics/cocoa/GraphicsContextGLOpenGLCocoa.mm
// Used with permission.
static std::optional<GLint> GetVirtualScreenByRegistryID(CGLPixelFormatObj pixelFormatObj,
                                                         IORegistryGPUID gpuID)
{
    // When a process does not have access to the WindowServer (as with Chromium's GPU process
    // and WebKit's WebProcess), there is no way for OpenGL to tell which GPU is connected to a
    // display. On 10.13+, find the virtual screen that corresponds to the preferred GPU by its
    // registryID. CGLSetVirtualScreen can then be used to tell OpenGL which GPU it should be
    // using.

    GLint virtualScreenCount = 0;
    CGLError error =
        CGLDescribePixelFormat(pixelFormatObj, 0, kCGLPFAVirtualScreenCount, &virtualScreenCount);
    if (error != kCGLNoError)
    {
        NOTREACHED();
        return std::nullopt;
    }

    for (GLint virtualScreen = 0; virtualScreen < virtualScreenCount; ++virtualScreen)
    {
        GLint displayMask = 0;
        error =
            CGLDescribePixelFormat(pixelFormatObj, virtualScreen, kCGLPFADisplayMask, &displayMask);
        if (error != kCGLNoError)
        {
            NOTREACHED();
            return std::nullopt;
        }

        auto virtualScreenGPUID = angle::GetGpuIDFromOpenGLDisplayMask(displayMask);
        if (virtualScreenGPUID == gpuID)
        {
            return virtualScreen;
        }
    }
    return std::nullopt;
}

static std::optional<GLint> GetFirstAcceleratedVirtualScreen(CGLPixelFormatObj pixelFormatObj)
{
    GLint virtualScreenCount = 0;
    CGLError error =
        CGLDescribePixelFormat(pixelFormatObj, 0, kCGLPFAVirtualScreenCount, &virtualScreenCount);
    if (error != kCGLNoError)
    {
        NOTREACHED();
        return std::nullopt;
    }
    for (GLint virtualScreen = 0; virtualScreen < virtualScreenCount; ++virtualScreen)
    {
        GLint isAccelerated = 0;
        error = CGLDescribePixelFormat(pixelFormatObj, virtualScreen, kCGLPFAAccelerated,
                                       &isAccelerated);
        if (error != kCGLNoError)
        {
            NOTREACHED();
            return std::nullopt;
        }
        if (isAccelerated)
        {
            return virtualScreen;
        }
    }
    return std::nullopt;
}

}  // anonymous namespace

EnsureCGLContextIsCurrent::EnsureCGLContextIsCurrent(CGLContextObj context)
    : mOldContext(CGLGetCurrentContext()), mResetContext(mOldContext != context)
{
    if (mResetContext)
    {
        CGLSetCurrentContext(context);
    }
}

EnsureCGLContextIsCurrent::~EnsureCGLContextIsCurrent()
{
    if (mResetContext)
    {
        CGLSetCurrentContext(mOldContext);
    }
}

class FunctionsGLCGL : public FunctionsGL
{
  public:
    FunctionsGLCGL(void *dylibHandle) : mDylibHandle(dylibHandle) {}

    ~FunctionsGLCGL() override { dlclose(mDylibHandle); }

  private:
    void *loadProcAddress(const std::string &function) const override
    {
        return dlsym(mDylibHandle, function.c_str());
    }

    void *mDylibHandle;
};

DisplayCGL::DisplayCGL(const egl::DisplayState &state)
    : DisplayGL(state),
      mEGLDisplay(nullptr),
      mContext(nullptr),
      mThreadsWithCurrentContext(),
      mPixelFormat(nullptr),
      mSupportsGPUSwitching(false),
      mCurrentGPUID(0),
      mDiscreteGPUPixelFormat(nullptr),
      mDiscreteGPURefs(0),
      mLastDiscreteGPUUnrefTime(0.0)
{}

DisplayCGL::~DisplayCGL() {}

egl::Error DisplayCGL::initialize(egl::Display *display)
{
    mEGLDisplay = display;

    angle::SystemInfo info;
    // It's legal for GetSystemInfo to return false and thereby
    // contain incomplete information.
    (void)angle::GetSystemInfo(&info);

    // This code implements the effect of the
    // disableGPUSwitchingSupport workaround in FeaturesGL.
    mSupportsGPUSwitching = info.isMacSwitchable && !info.hasNVIDIAGPU();

    {
        // TODO(cwallez) investigate which pixel format we want
        std::vector<CGLPixelFormatAttribute> attribs;
        attribs.push_back(kCGLPFAOpenGLProfile);
        attribs.push_back(static_cast<CGLPixelFormatAttribute>(kCGLOGLPVersion_3_2_Core));
        attribs.push_back(kCGLPFAAllowOfflineRenderers);
        attribs.push_back(static_cast<CGLPixelFormatAttribute>(0));
        GLint nVirtualScreens = 0;
        CGLChoosePixelFormat(attribs.data(), &mPixelFormat, &nVirtualScreens);

        if (mPixelFormat == nullptr)
        {
            return egl::EglNotInitialized() << "Could not create the context's pixel format.";
        }
    }

    CGLCreateContext(mPixelFormat, nullptr, &mContext);
    if (mContext == nullptr)
    {
        return egl::EglNotInitialized() << "Could not create the CGL context.";
    }

    if (mSupportsGPUSwitching)
    {
        auto gpuIndex = info.getPreferredGPUIndex();
        if (gpuIndex)
        {
            auto gpuID         = info.gpus[*gpuIndex].systemDeviceId;
            auto virtualScreen = GetVirtualScreenByRegistryID(mPixelFormat, gpuID);
            if (virtualScreen)
            {
                CGLError error = CGLSetVirtualScreen(mContext, *virtualScreen);
                ASSERT(error == kCGLNoError);
                if (error == kCGLNoError)
                {
                    mCurrentGPUID = gpuID;
                }
            }
        }
        if (mCurrentGPUID == 0)
        {
            // Determine the currently active GPU on the system.
            mCurrentGPUID = angle::GetGpuIDFromDisplayID(kCGDirectMainDisplay);
        }
    }

    if (CGLSetCurrentContext(mContext) != kCGLNoError)
    {
        return egl::EglNotInitialized() << "Could not make the CGL context current.";
    }
    mThreadsWithCurrentContext.insert(angle::GetCurrentThreadUniqueId());

    // There is no equivalent getProcAddress in CGL so we open the dylib directly
    void *handle = dlopen(kDefaultOpenGLDylibName, RTLD_NOW);
    if (!handle)
    {
        handle = dlopen(kFallbackOpenGLDylibName, RTLD_NOW);
    }
    if (!handle)
    {
        return egl::EglNotInitialized() << "Could not open the OpenGL Framework.";
    }

    std::unique_ptr<FunctionsGL> functionsGL(new FunctionsGLCGL(handle));
    functionsGL->initialize(display->getAttributeMap());

    mRenderer.reset(new RendererGL(std::move(functionsGL), display->getAttributeMap(), this));

    const gl::Version &maxVersion = mRenderer->getMaxSupportedESVersion();
    if (maxVersion < gl::Version(2, 0))
    {
        return egl::EglNotInitialized() << "OpenGL ES 2.0 is not supportable.";
    }

    auto &attributes = display->getAttributeMap();
    mDeviceContextIsVolatile =
        attributes.get(EGL_PLATFORM_ANGLE_DEVICE_CONTEXT_VOLATILE_CGL_ANGLE, GL_FALSE);

    return DisplayGL::initialize(display);
}

void DisplayCGL::terminate()
{
    DisplayGL::terminate();

    mRenderer.reset();
    if (mPixelFormat != nullptr)
    {
        CGLDestroyPixelFormat(mPixelFormat);
        mPixelFormat = nullptr;
    }
    if (mContext != nullptr)
    {
        CGLSetCurrentContext(nullptr);
        CGLDestroyContext(mContext);
        mContext = nullptr;
        mThreadsWithCurrentContext.clear();
    }
    if (mDiscreteGPUPixelFormat != nullptr)
    {
        CGLDestroyPixelFormat(mDiscreteGPUPixelFormat);
        mDiscreteGPUPixelFormat   = nullptr;
        mLastDiscreteGPUUnrefTime = 0.0;
    }
}

egl::Error DisplayCGL::prepareForCall()
{
    if (!mContext)
    {
        return egl::EglNotInitialized() << "Context not allocated.";
    }
    auto threadId = angle::GetCurrentThreadUniqueId();
    if (mDeviceContextIsVolatile ||
        mThreadsWithCurrentContext.find(threadId) == mThreadsWithCurrentContext.end())
    {
        if (CGLSetCurrentContext(mContext) != kCGLNoError)
        {
            return egl::EglBadAlloc() << "Could not make device CGL context current.";
        }
        mThreadsWithCurrentContext.insert(threadId);
    }
    return egl::NoError();
}

egl::Error DisplayCGL::releaseThread()
{
    ASSERT(mContext);
    auto threadId = angle::GetCurrentThreadUniqueId();
    if (mThreadsWithCurrentContext.find(threadId) != mThreadsWithCurrentContext.end())
    {
        if (CGLSetCurrentContext(nullptr) != kCGLNoError)
        {
            return egl::EglBadAlloc() << "Could not release device CGL context.";
        }
        mThreadsWithCurrentContext.erase(threadId);
    }
    return egl::NoError();
}

egl::Error DisplayCGL::makeCurrent(egl::Display *display,
                                   egl::Surface *drawSurface,
                                   egl::Surface *readSurface,
                                   gl::Context *context)
{
    checkDiscreteGPUStatus();
    return DisplayGL::makeCurrent(display, drawSurface, readSurface, context);
}

SurfaceImpl *DisplayCGL::createWindowSurface(const egl::SurfaceState &state,
                                             EGLNativeWindowType window,
                                             const egl::AttributeMap &attribs)
{
    return new WindowSurfaceCGL(state, mRenderer.get(), window, mContext);
}

SurfaceImpl *DisplayCGL::createPbufferSurface(const egl::SurfaceState &state,
                                              const egl::AttributeMap &attribs)
{
    EGLint width  = static_cast<EGLint>(attribs.get(EGL_WIDTH, 0));
    EGLint height = static_cast<EGLint>(attribs.get(EGL_HEIGHT, 0));
    return new PbufferSurfaceCGL(state, mRenderer.get(), width, height);
}

SurfaceImpl *DisplayCGL::createPbufferFromClientBuffer(const egl::SurfaceState &state,
                                                       EGLenum buftype,
                                                       EGLClientBuffer clientBuffer,
                                                       const egl::AttributeMap &attribs)
{
    ASSERT(buftype == EGL_IOSURFACE_ANGLE);

    return new IOSurfaceSurfaceCGL(state, getRenderer(), mContext, clientBuffer, attribs);
}

SurfaceImpl *DisplayCGL::createPixmapSurface(const egl::SurfaceState &state,
                                             NativePixmapType nativePixmap,
                                             const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

ContextImpl *DisplayCGL::createContext(const gl::State &state,
                                       gl::ErrorSet *errorSet,
                                       const egl::Config *configuration,
                                       const gl::Context *shareContext,
                                       const egl::AttributeMap &attribs)
{
    bool usesDiscreteGPU = false;

    if (attribs.get(EGL_POWER_PREFERENCE_ANGLE, EGL_LOW_POWER_ANGLE) == EGL_HIGH_POWER_ANGLE)
    {
        // Should have been rejected by validation if not supported.
        ASSERT(mSupportsGPUSwitching);
        usesDiscreteGPU = true;
    }

    return new ContextCGL(this, state, errorSet, mRenderer, usesDiscreteGPU);
}

DeviceImpl *DisplayCGL::createDevice()
{
    return new DeviceCGL();
}

egl::ConfigSet DisplayCGL::generateConfigs()
{
    // TODO(cwallez): generate more config permutations
    egl::ConfigSet configs;

    const gl::Version &maxVersion = getMaxSupportedESVersion();
    ASSERT(maxVersion >= gl::Version(2, 0));
    bool supportsES3 = maxVersion >= gl::Version(3, 0);

    egl::Config config;

    // Native stuff
    config.nativeVisualID   = 0;
    config.nativeVisualType = 0;
    config.nativeRenderable = EGL_TRUE;

    // Buffer sizes
    config.redSize     = 8;
    config.greenSize   = 8;
    config.blueSize    = 8;
    config.alphaSize   = 8;
    config.depthSize   = 24;
    config.stencilSize = 8;

    config.colorBufferType = EGL_RGB_BUFFER;
    config.luminanceSize   = 0;
    config.alphaMaskSize   = 0;

    config.bufferSize = config.redSize + config.greenSize + config.blueSize + config.alphaSize;

    config.transparentType = EGL_NONE;

    // Pbuffer
    config.maxPBufferWidth  = 4096;
    config.maxPBufferHeight = 4096;
    config.maxPBufferPixels = 4096 * 4096;

    // Caveat
    config.configCaveat = EGL_NONE;

    // Misc
    config.sampleBuffers     = 0;
    config.samples           = 0;
    config.level             = 0;
    config.bindToTextureRGB  = EGL_FALSE;
    config.bindToTextureRGBA = EGL_FALSE;

    config.bindToTextureTarget = EGL_TEXTURE_RECTANGLE_ANGLE;

    config.surfaceType = EGL_WINDOW_BIT | EGL_PBUFFER_BIT;

    config.minSwapInterval = 1;
    config.maxSwapInterval = 1;

    config.renderTargetFormat = GL_RGBA8;
    config.depthStencilFormat = GL_DEPTH24_STENCIL8;

    config.conformant     = EGL_OPENGL_ES2_BIT | (supportsES3 ? EGL_OPENGL_ES3_BIT_KHR : 0);
    config.renderableType = config.conformant;

    config.matchNativePixmap = EGL_NONE;

    config.colorComponentType = EGL_COLOR_COMPONENT_TYPE_FIXED_EXT;

    configs.add(config);
    return configs;
}

bool DisplayCGL::testDeviceLost()
{
    // TODO(cwallez) investigate implementing this
    return false;
}

egl::Error DisplayCGL::restoreLostDevice(const egl::Display *display)
{
    UNIMPLEMENTED();
    return egl::EglBadDisplay();
}

bool DisplayCGL::isValidNativeWindow(EGLNativeWindowType window) const
{
    NSObject *layer = reinterpret_cast<NSObject *>(window);
    return [layer isKindOfClass:[CALayer class]];
}

egl::Error DisplayCGL::validateClientBuffer(const egl::Config *configuration,
                                            EGLenum buftype,
                                            EGLClientBuffer clientBuffer,
                                            const egl::AttributeMap &attribs) const
{
    ASSERT(buftype == EGL_IOSURFACE_ANGLE);

    if (!IOSurfaceSurfaceCGL::validateAttributes(clientBuffer, attribs))
    {
        return egl::EglBadAttribute();
    }

    return egl::NoError();
}

CGLContextObj DisplayCGL::getCGLContext() const
{
    return mContext;
}

CGLPixelFormatObj DisplayCGL::getCGLPixelFormat() const
{
    return mPixelFormat;
}

void DisplayCGL::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
    outExtensions->iosurfaceClientBuffer  = true;
    outExtensions->surfacelessContext     = true;
    outExtensions->waitUntilWorkScheduled = true;

    // Contexts are virtualized so textures and semaphores can be shared globally
    outExtensions->displayTextureShareGroup   = true;
    outExtensions->displaySemaphoreShareGroup = true;

    if (mSupportsGPUSwitching)
    {
        outExtensions->powerPreference = true;
    }

    DisplayGL::generateExtensions(outExtensions);
}

void DisplayCGL::generateCaps(egl::Caps *outCaps) const
{
    outCaps->textureNPOT = true;
}

egl::Error DisplayCGL::waitClient(const gl::Context *context)
{
    // TODO(cwallez) UNIMPLEMENTED()
    return egl::NoError();
}

egl::Error DisplayCGL::waitNative(const gl::Context *context, EGLint engine)
{
    // TODO(cwallez) UNIMPLEMENTED()
    return egl::NoError();
}

egl::Error DisplayCGL::waitUntilWorkScheduled()
{
    for (auto context : mState.contextMap)
    {
        context.second->flush();
    }
    return egl::NoError();
}

gl::Version DisplayCGL::getMaxSupportedESVersion() const
{
    return mRenderer->getMaxSupportedESVersion();
}

egl::Error DisplayCGL::makeCurrentSurfaceless(gl::Context *context)
{
    // We have nothing to do as mContext is always current, and that CGL is surfaceless by
    // default.
    return egl::NoError();
}

void DisplayCGL::initializeFrontendFeatures(angle::FrontendFeatures *features) const
{
    mRenderer->initializeFrontendFeatures(features);
}

void DisplayCGL::populateFeatureList(angle::FeatureList *features)
{
    mRenderer->getFeatures().populateFeatureList(features);
}

RendererGL *DisplayCGL::getRenderer() const
{
    return mRenderer.get();
}

egl::Error DisplayCGL::referenceDiscreteGPU()
{
    // Should have been rejected by validation if not supported.
    ASSERT(mSupportsGPUSwitching);
    // Create discrete pixel format if necessary.
    if (mDiscreteGPUPixelFormat)
    {
        // Clear this out if necessary.
        mLastDiscreteGPUUnrefTime = 0.0;
    }
    else
    {
        ASSERT(mLastDiscreteGPUUnrefTime == 0.0);
        CGLPixelFormatAttribute discreteAttribs[] = {static_cast<CGLPixelFormatAttribute>(0)};
        GLint numPixelFormats                     = 0;
        if (CGLChoosePixelFormat(discreteAttribs, &mDiscreteGPUPixelFormat, &numPixelFormats) !=
            kCGLNoError)
        {
            return egl::EglBadAlloc() << "Error choosing discrete pixel format.";
        }
    }
    ++mDiscreteGPURefs;

    return egl::NoError();
}

egl::Error DisplayCGL::unreferenceDiscreteGPU()
{
    // Should have been rejected by validation if not supported.
    ASSERT(mSupportsGPUSwitching);
    ASSERT(mDiscreteGPURefs > 0);
    if (--mDiscreteGPURefs == 0)
    {
        auto *platform            = ANGLEPlatformCurrent();
        mLastDiscreteGPUUnrefTime = platform->monotonicallyIncreasingTime(platform);
    }

    return egl::NoError();
}

void DisplayCGL::checkDiscreteGPUStatus()
{
    const double kDiscreteGPUTimeoutInSeconds = 10.0;

    if (mLastDiscreteGPUUnrefTime != 0.0)
    {
        ASSERT(mSupportsGPUSwitching);
        // A non-zero value implies that the timer is ticking on deleting the discrete GPU pixel
        // format.
        auto *platform = ANGLEPlatformCurrent();
        ASSERT(platform);
        double currentTime = platform->monotonicallyIncreasingTime(platform);
        if (currentTime > mLastDiscreteGPUUnrefTime + kDiscreteGPUTimeoutInSeconds)
        {
            CGLDestroyPixelFormat(mDiscreteGPUPixelFormat);
            mDiscreteGPUPixelFormat   = nullptr;
            mLastDiscreteGPUUnrefTime = 0.0;
        }
    }
}

egl::Error DisplayCGL::handleGPUSwitch()
{
    if (mSupportsGPUSwitching)
    {
        uint64_t gpuID = angle::GetGpuIDFromDisplayID(kCGDirectMainDisplay);
        if (gpuID != mCurrentGPUID)
        {
            auto virtualScreen = GetVirtualScreenByRegistryID(mPixelFormat, gpuID);
            if (!virtualScreen)
            {
                virtualScreen = GetFirstAcceleratedVirtualScreen(mPixelFormat);
            }
            if (virtualScreen)
            {
                setContextToGPU(gpuID, *virtualScreen);
            }
        }
    }

    return egl::NoError();
}

egl::Error DisplayCGL::forceGPUSwitch(EGLint gpuIDHigh, EGLint gpuIDLow)
{
    if (mSupportsGPUSwitching)
    {
        uint64_t gpuID = static_cast<uint64_t>(static_cast<uint32_t>(gpuIDHigh)) << 32 |
                         static_cast<uint32_t>(gpuIDLow);
        if (gpuID != mCurrentGPUID)
        {
            auto virtualScreen = GetVirtualScreenByRegistryID(mPixelFormat, gpuID);
            if (virtualScreen)
            {
                setContextToGPU(gpuID, *virtualScreen);
            }
        }
    }
    return egl::NoError();
}

void DisplayCGL::setContextToGPU(uint64_t gpuID, GLint virtualScreen)
{
    CGLError error = CGLSetVirtualScreen(mContext, virtualScreen);
    ASSERT(error == kCGLNoError);
    if (error == kCGLNoError)
    {
        // Performing the above operation seems to need a call to CGLSetCurrentContext to make
        // the context work properly again. Failing to do this returns null strings for
        // GL_VENDOR and GL_RENDERER.
        CGLUpdateContext(mContext);
        CGLSetCurrentContext(mContext);
        onStateChange(angle::SubjectMessage::SubjectChanged);
        mCurrentGPUID = gpuID;
        mRenderer->handleGPUSwitch();
    }
}

}  // namespace rx
