//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayMtl.mm: Metal implementation of DisplayImpl

#include "libANGLE/renderer/metal/DisplayMtl.h"
#include <sys/param.h>

#include "common/apple_platform_utils.h"
#include "common/system_utils.h"
#include "gpu_info_util/SystemInfo.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/driver_utils.h"
#include "libANGLE/renderer/metal/CompilerMtl.h"
#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/DeviceMtl.h"
#include "libANGLE/renderer/metal/IOSurfaceSurfaceMtl.h"
#include "libANGLE/renderer/metal/ImageMtl.h"
#include "libANGLE/renderer/metal/SurfaceMtl.h"
#include "libANGLE/renderer/metal/SyncMtl.h"
#include "libANGLE/renderer/metal/mtl_common.h"
#include "libANGLE/trace.h"
#include "mtl_command_buffer.h"
#include "platform/PlatformMethods.h"

#if ANGLE_METAL_XCODE_BUILDS_SHADERS
#    include "libANGLE/renderer/metal/shaders/mtl_internal_shaders_metallib.h"
#elif ANGLE_METAL_HAS_PREBUILT_INTERNAL_SHADERS
#    include "mtl_internal_shaders_metallib.h"
#else
#    include "libANGLE/renderer/metal/shaders/mtl_internal_shaders_src_autogen.h"
#endif

#include "EGL/eglext.h"

namespace rx
{

static EGLint GetDepthSize(GLint internalformat)
{
    switch (internalformat)
    {
        case GL_STENCIL_INDEX8:
            return 0;
        case GL_DEPTH_COMPONENT16:
            return 16;
        case GL_DEPTH_COMPONENT24:
            return 24;
        case GL_DEPTH_COMPONENT32_OES:
            return 32;
        case GL_DEPTH_COMPONENT32F:
            return 32;
        case GL_DEPTH24_STENCIL8:
            return 24;
        case GL_DEPTH32F_STENCIL8:
            return 32;
        default:
            //    UNREACHABLE(internalformat);
            return 0;
    }
}

static EGLint GetStencilSize(GLint internalformat)
{
    switch (internalformat)
    {
        case GL_STENCIL_INDEX8:
            return 8;
        case GL_DEPTH_COMPONENT16:
            return 0;
        case GL_DEPTH_COMPONENT24:
            return 0;
        case GL_DEPTH_COMPONENT32_OES:
            return 0;
        case GL_DEPTH_COMPONENT32F:
            return 0;
        case GL_DEPTH24_STENCIL8:
            return 8;
        case GL_DEPTH32F_STENCIL8:
            return 8;
        default:
            //    UNREACHABLE(internalformat);
            return 0;
    }
}

bool IsMetalDisplayAvailable()
{
    return angle::IsMetalRendererAvailable();
}

DisplayImpl *CreateMetalDisplay(const egl::DisplayState &state)
{
    return new DisplayMtl(state);
}

DisplayMtl::DisplayMtl(const egl::DisplayState &state)
    : DisplayImpl(state), mDisplay(nullptr), mStateCache(mFeatures)
{}

DisplayMtl::~DisplayMtl() {}

egl::Error DisplayMtl::initialize(egl::Display *display)
{
    ASSERT(IsMetalDisplayAvailable());

    angle::Result result = initializeImpl(display);
    if (result != angle::Result::Continue)
    {
        terminate();
        return egl::EglNotInitialized();
    }
    return egl::NoError();
}

angle::Result DisplayMtl::initializeImpl(egl::Display *display)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        mDisplay = display;

        mMetalDevice = getMetalDeviceMatchingAttribute(display->getAttributeMap());
        // If we can't create a device, fail initialization.
        if (!mMetalDevice.get())
        {
            return angle::Result::Stop;
        }

        mMetalDeviceVendorId = mtl::GetDeviceVendorId(mMetalDevice);

        mCapsInitialized = false;

        initializeFeatures();

        if (mFeatures.requireGpuFamily2.enabled && !supportsEitherGPUFamily(1, 2))
        {
            ANGLE_MTL_LOG("Could not initialize: Metal device does not support Mac GPU family 2.");
            return angle::Result::Stop;
        }

        if (mFeatures.disableMetalOnNvidia.enabled && isNVIDIA())
        {
            ANGLE_MTL_LOG("Could not initialize: Metal not supported on NVIDIA GPUs.");
            return angle::Result::Stop;
        }

        mCmdQueue = mtl::adoptObjCPtr([mMetalDevice newCommandQueue]);

        ANGLE_TRY(mFormatTable.initialize(this));
        ANGLE_TRY(initializeShaderLibrary());

        mUtils = std::make_unique<mtl::RenderUtils>();

        return angle::Result::Continue;
    }
}

void DisplayMtl::terminate()
{
    mUtils = nullptr;
    mCmdQueue.reset();
    mDefaultShaders = nil;
    mMetalDevice    = nil;
    mSharedEventListener = nil;
    mCapsInitialized = false;

    mMetalDeviceVendorId = 0;
    mComputedAMDBronze   = false;
    mIsAMDBronze         = false;
}

bool DisplayMtl::testDeviceLost()
{
    return false;
}

egl::Error DisplayMtl::restoreLostDevice(const egl::Display *display)
{
    return egl::NoError();
}

std::string DisplayMtl::getRendererDescription()
{
    ANGLE_MTL_OBJC_SCOPE
    {
        std::string desc = "ANGLE Metal Renderer";

        if (mMetalDevice)
        {
            desc += ": ";
            desc += mMetalDevice.get().name.UTF8String;
        }

        return desc;
    }
}

std::string DisplayMtl::getVendorString()
{
    return GetVendorString(mMetalDeviceVendorId);
}

std::string DisplayMtl::getVersionString(bool includeFullVersion)
{
    if (!includeFullVersion)
    {
        // For WebGL contexts it's inappropriate to include any
        // additional version information, but Chrome requires
        // something to be present here.
        return "Unspecified Version";
    }

    ANGLE_MTL_OBJC_SCOPE
    {
        NSProcessInfo *procInfo = [NSProcessInfo processInfo];
        return procInfo.operatingSystemVersionString.UTF8String;
    }
}

DeviceImpl *DisplayMtl::createDevice()
{
    return new DeviceMtl();
}

mtl::AutoObjCPtr<id<MTLDevice>> DisplayMtl::getMetalDeviceMatchingAttribute(
    const egl::AttributeMap &attribs)
{
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
    auto deviceList = mtl::adoptObjCPtr(MTLCopyAllDevices());

    EGLAttrib high = attribs.get(EGL_PLATFORM_ANGLE_DEVICE_ID_HIGH_ANGLE, 0);
    EGLAttrib low  = attribs.get(EGL_PLATFORM_ANGLE_DEVICE_ID_LOW_ANGLE, 0);
    uint64_t deviceId =
        angle::GetSystemDeviceIdFromParts(static_cast<uint32_t>(high), static_cast<uint32_t>(low));
    // Check EGL_ANGLE_platform_angle_device_id to see if a device was specified.
    if (deviceId != 0)
    {
        for (id<MTLDevice> device in deviceList.get())
        {
            if ([device registryID] == deviceId)
            {
                return device;
            }
        }
    }

    auto externalGPUs =
        mtl::adoptObjCPtr<NSMutableArray<id<MTLDevice>>>([[NSMutableArray alloc] init]);
    auto integratedGPUs =
        mtl::adoptObjCPtr<NSMutableArray<id<MTLDevice>>>([[NSMutableArray alloc] init]);
    auto discreteGPUs =
        mtl::adoptObjCPtr<NSMutableArray<id<MTLDevice>>>([[NSMutableArray alloc] init]);
    for (id<MTLDevice> device in deviceList.get())
    {
        if (device.removable)
        {
            [externalGPUs addObject:device];
        }
        else if (device.lowPower)
        {
            [integratedGPUs addObject:device];
        }
        else
        {
            [discreteGPUs addObject:device];
        }
    }
    // TODO(kpiddington: External GPU support. Do we prefer high power / low bandwidth for general
    // WebGL applications?
    //      Can we support hot-swapping in GPU's?
    if (attribs.get(EGL_POWER_PREFERENCE_ANGLE, 0) == EGL_HIGH_POWER_ANGLE)
    {
        // Search for a discrete GPU first.
        for (id<MTLDevice> device in discreteGPUs.get())
        {
            if (![device isHeadless])
                return device;
        }
    }
    else if (attribs.get(EGL_POWER_PREFERENCE_ANGLE, 0) == EGL_LOW_POWER_ANGLE)
    {
        // If we've selected a low power device, look through integrated devices.
        for (id<MTLDevice> device in integratedGPUs.get())
        {
            if (![device isHeadless])
                return device;
        }
    }

    const std::string preferredDeviceString = angle::GetPreferredDeviceString();
    if (!preferredDeviceString.empty())
    {
        for (id<MTLDevice> device in deviceList.get())
        {
            if ([device.name.lowercaseString
                    containsString:[NSString stringWithUTF8String:preferredDeviceString.c_str()]])
            {
                NSLog(@"Using Metal Device: %@", [device name]);
                return device;
            }
        }
    }

#endif
    // If we can't find anything, or are on a platform that doesn't support power options, create a
    // default device.
    return mtl::adoptObjCPtr(MTLCreateSystemDefaultDevice());
}

egl::Error DisplayMtl::waitClient(const gl::Context *context)
{
    auto contextMtl      = GetImplAs<ContextMtl>(context);
    angle::Result result = contextMtl->finishCommandBuffer();

    if (result != angle::Result::Continue)
    {
        return egl::EglBadAccess();
    }
    return egl::NoError();
}

egl::Error DisplayMtl::waitNative(const gl::Context *context, EGLint engine)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

egl::Error DisplayMtl::waitUntilWorkScheduled()
{
    for (auto context : mState.contextMap)
    {
        auto contextMtl = GetImplAs<ContextMtl>(context.second);
        contextMtl->flushCommandBuffer(mtl::WaitUntilScheduled);
    }
    return egl::NoError();
}

SurfaceImpl *DisplayMtl::createWindowSurface(const egl::SurfaceState &state,
                                             EGLNativeWindowType window,
                                             const egl::AttributeMap &attribs)
{
    return new WindowSurfaceMtl(this, state, window, attribs);
}

SurfaceImpl *DisplayMtl::createPbufferSurface(const egl::SurfaceState &state,
                                              const egl::AttributeMap &attribs)
{
    return new PBufferSurfaceMtl(this, state, attribs);
}

SurfaceImpl *DisplayMtl::createPbufferFromClientBuffer(const egl::SurfaceState &state,
                                                       EGLenum buftype,
                                                       EGLClientBuffer clientBuffer,
                                                       const egl::AttributeMap &attribs)
{
    switch (buftype)
    {
        case EGL_IOSURFACE_ANGLE:
            return new IOSurfaceSurfaceMtl(this, state, clientBuffer, attribs);
        default:
            UNREACHABLE();
    }
    return nullptr;
}

SurfaceImpl *DisplayMtl::createPixmapSurface(const egl::SurfaceState &state,
                                             NativePixmapType nativePixmap,
                                             const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return static_cast<SurfaceImpl *>(0);
}

ImageImpl *DisplayMtl::createImage(const egl::ImageState &state,
                                   const gl::Context *context,
                                   EGLenum target,
                                   const egl::AttributeMap &attribs)
{
    return new ImageMtl(state, context);
}

rx::ContextImpl *DisplayMtl::createContext(const gl::State &state,
                                           gl::ErrorSet *errorSet,
                                           const egl::Config *configuration,
                                           const gl::Context *shareContext,
                                           const egl::AttributeMap &attribs)
{
    return new ContextMtl(state, errorSet, attribs, this);
}

StreamProducerImpl *DisplayMtl::createStreamProducerD3DTexture(
    egl::Stream::ConsumerType consumerType,
    const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

ShareGroupImpl *DisplayMtl::createShareGroup(const egl::ShareGroupState &state)
{
    return new ShareGroupMtl(state);
}

ExternalImageSiblingImpl *DisplayMtl::createExternalImageSibling(const gl::Context *context,
                                                                 EGLenum target,
                                                                 EGLClientBuffer buffer,
                                                                 const egl::AttributeMap &attribs)
{
    switch (target)
    {
        case EGL_METAL_TEXTURE_ANGLE:
            return new TextureImageSiblingMtl(buffer, attribs);

        default:
            UNREACHABLE();
            return nullptr;
    }
}

gl::Version DisplayMtl::getMaxSupportedESVersion() const
{
#if TARGET_OS_SIMULATOR
    // Simulator should be able to support ES3, despite not supporting iOS GPU
    // Family 3 in its entirety.
    // FIXME: None of the feature conditions are checked for simulator support.
    return gl::Version(3, 0);
#else
    if (supportsEitherGPUFamily(3, 1))
    {
        return mtl::kMaxSupportedGLVersion;
    }
    return gl::Version(2, 0);
#endif
}

gl::Version DisplayMtl::getMaxConformantESVersion() const
{
    return std::min(getMaxSupportedESVersion(), gl::Version(3, 0));
}

EGLSyncImpl *DisplayMtl::createSync()
{
    return new EGLSyncMtl();
}

egl::Error DisplayMtl::makeCurrent(egl::Display *display,
                                   egl::Surface *drawSurface,
                                   egl::Surface *readSurface,
                                   gl::Context *context)
{
    if (!context)
    {
        return egl::NoError();
    }

    return egl::NoError();
}

void DisplayMtl::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
    outExtensions->iosurfaceClientBuffer      = true;
    outExtensions->surfacelessContext         = true;
    outExtensions->noConfigContext            = true;
    outExtensions->displayTextureShareGroup   = true;
    outExtensions->displaySemaphoreShareGroup = true;
    outExtensions->mtlTextureClientBuffer     = true;
    outExtensions->waitUntilWorkScheduled     = true;

    if (mFeatures.hasEvents.enabled)
    {
        // MTLSharedEvent is only available since Metal 2.1
        outExtensions->fenceSync = true;
        outExtensions->waitSync  = true;
    }

    // Note that robust resource initialization is not yet implemented. We only expose
    // this extension so that ANGLE can be initialized in Chrome. WebGL will fail to use
    // this extension (anglebug.com/42263503)
    outExtensions->robustResourceInitializationANGLE = true;

    // EGL_KHR_image
    outExtensions->image     = true;
    outExtensions->imageBase = true;

    // EGL_ANGLE_metal_create_context_ownership_identity
    outExtensions->metalCreateContextOwnershipIdentityANGLE = true;

    // EGL_ANGLE_metal_sync_shared_event
    outExtensions->mtlSyncSharedEventANGLE = true;
}

void DisplayMtl::generateCaps(egl::Caps *outCaps) const
{
    outCaps->textureNPOT = true;
}

void DisplayMtl::initializeFrontendFeatures(angle::FrontendFeatures *features) const
{
    // The Metal backend's handling of compile and link is thread-safe
    ANGLE_FEATURE_CONDITION(features, compileJobIsThreadSafe, true);
    ANGLE_FEATURE_CONDITION(features, linkJobIsThreadSafe, true);
}

void DisplayMtl::populateFeatureList(angle::FeatureList *features)
{
    mFeatures.populateFeatureList(features);
}

EGLenum DisplayMtl::EGLDrawingBufferTextureTarget()
{
    // TODO(anglebug.com/42264909): Apple's implementation conditionalized this on
    // MacCatalyst and whether it was running on ARM64 or X64, preferring
    // EGL_TEXTURE_RECTANGLE_ANGLE. Metal can bind IOSurfaces to regular 2D
    // textures, and rectangular textures don't work in the SPIR-V Metal
    // backend, so for the time being use EGL_TEXTURE_2D on all platforms.
    return EGL_TEXTURE_2D;
}

egl::ConfigSet DisplayMtl::generateConfigs()
{
    // NOTE(hqle): generate more config permutations
    egl::ConfigSet configs;

    const gl::Version &maxVersion = getMaxSupportedESVersion();
    ASSERT(maxVersion >= gl::Version(2, 0));
    bool supportsES3 = maxVersion >= gl::Version(3, 0);

    egl::Config config;

    // Native stuff
    config.nativeVisualID   = 0;
    config.nativeVisualType = 0;
    config.nativeRenderable = EGL_TRUE;

    config.colorBufferType = EGL_RGB_BUFFER;
    config.luminanceSize   = 0;
    config.alphaMaskSize   = 0;

    config.transparentType = EGL_NONE;

    // Pbuffer
    config.bindToTextureTarget = EGLDrawingBufferTextureTarget();
    config.maxPBufferWidth     = 4096;
    config.maxPBufferHeight    = 4096;
    config.maxPBufferPixels    = 4096 * 4096;

    // Caveat
    config.configCaveat = EGL_NONE;

    // Misc
    config.sampleBuffers     = 0;
    config.samples           = 0;
    config.level             = 0;
    config.bindToTextureRGB  = EGL_FALSE;
    config.bindToTextureRGBA = EGL_TRUE;

    config.surfaceType = EGL_WINDOW_BIT | EGL_PBUFFER_BIT;

#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
    config.minSwapInterval = 0;
    config.maxSwapInterval = 1;
#else
    config.minSwapInterval = 1;
    config.maxSwapInterval = 1;
#endif

    config.renderTargetFormat = GL_RGBA8;

    config.conformant     = EGL_OPENGL_ES2_BIT | (supportsES3 ? EGL_OPENGL_ES3_BIT_KHR : 0);
    config.renderableType = config.conformant;

    config.matchNativePixmap = EGL_NONE;

    config.colorComponentType = EGL_COLOR_COMPONENT_TYPE_FIXED_EXT;

    constexpr int samplesSupported[] = {0, 4};

    for (int samples : samplesSupported)
    {
        config.samples       = samples;
        config.sampleBuffers = (samples == 0) ? 0 : 1;

        // Buffer sizes
        config.redSize    = 8;
        config.greenSize  = 8;
        config.blueSize   = 8;
        config.alphaSize  = 8;
        config.bufferSize = config.redSize + config.greenSize + config.blueSize + config.alphaSize;

        // With DS
        config.depthSize          = 24;
        config.stencilSize        = 8;
        config.depthStencilFormat = GL_DEPTH24_STENCIL8;

        configs.add(config);

        // With D
        config.depthSize          = 24;
        config.stencilSize        = 0;
        config.depthStencilFormat = GL_DEPTH_COMPONENT24;
        configs.add(config);

        // With S
        config.depthSize          = 0;
        config.stencilSize        = 8;
        config.depthStencilFormat = GL_STENCIL_INDEX8;
        configs.add(config);

        // No DS
        config.depthSize          = 0;
        config.stencilSize        = 0;
        config.depthStencilFormat = GL_NONE;
        configs.add(config);

        // Tests like dEQP-GLES2.functional.depth_range.* assume EGL_DEPTH_SIZE is properly set even
        // if renderConfig attributes are set to glu::RenderConfig::DONT_CARE
        config.depthSize   = GetDepthSize(config.depthStencilFormat);
        config.stencilSize = GetStencilSize(config.depthStencilFormat);
        configs.add(config);
    }

    return configs;
}

bool DisplayMtl::isValidNativeWindow(EGLNativeWindowType window) const
{
    ANGLE_MTL_OBJC_SCOPE
    {
        NSObject *layer = (__bridge NSObject *)(window);
        return [layer isKindOfClass:[CALayer class]];
    }
}

egl::Error DisplayMtl::validateClientBuffer(const egl::Config *configuration,
                                            EGLenum buftype,
                                            EGLClientBuffer clientBuffer,
                                            const egl::AttributeMap &attribs) const
{
    switch (buftype)
    {
        case EGL_IOSURFACE_ANGLE:
            if (!IOSurfaceSurfaceMtl::ValidateAttributes(clientBuffer, attribs))
            {
                return egl::EglBadAttribute();
            }
            break;
        default:
            UNREACHABLE();
            return egl::EglBadAttribute();
    }
    return egl::NoError();
}

egl::Error DisplayMtl::validateImageClientBuffer(const gl::Context *context,
                                                 EGLenum target,
                                                 EGLClientBuffer clientBuffer,
                                                 const egl::AttributeMap &attribs) const
{
    switch (target)
    {
        case EGL_METAL_TEXTURE_ANGLE:
            return TextureImageSiblingMtl::ValidateClientBuffer(this, clientBuffer, attribs);
        default:
            UNREACHABLE();
            return egl::EglBadAttribute();
    }
}

gl::Caps DisplayMtl::getNativeCaps() const
{
    ensureCapsInitialized();
    return mNativeCaps;
}
const gl::TextureCapsMap &DisplayMtl::getNativeTextureCaps() const
{
    ensureCapsInitialized();
    return mNativeTextureCaps;
}
const gl::Extensions &DisplayMtl::getNativeExtensions() const
{
    ensureCapsInitialized();
    return mNativeExtensions;
}
const gl::Limitations &DisplayMtl::getNativeLimitations() const
{
    ensureCapsInitialized();
    return mNativeLimitations;
}
const ShPixelLocalStorageOptions &DisplayMtl::getNativePixelLocalStorageOptions() const
{
    ensureCapsInitialized();
    return mNativePLSOptions;
}

void DisplayMtl::ensureCapsInitialized() const
{
    if (mCapsInitialized)
    {
        return;
    }

    mCapsInitialized = true;

    // Reset
    mNativeCaps = gl::Caps();

    // Fill extension and texture caps
    initializeExtensions();
    initializeTextureCaps();

    // https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf
    mNativeCaps.maxElementIndex  = std::numeric_limits<GLuint>::max() - 1;
    mNativeCaps.max3DTextureSize = 2048;
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
    mNativeCaps.max2DTextureSize = 16384;
    // On macOS exclude [[position]] from maxVaryingVectors.
    mNativeCaps.maxVaryingVectors         = 31 - 1;
    mNativeCaps.maxVertexOutputComponents = mNativeCaps.maxFragmentInputComponents = 124 - 4;
#elif TARGET_OS_SIMULATOR
    mNativeCaps.max2DTextureSize          = 8192;
    mNativeCaps.maxVaryingVectors         = 31 - 1;
    mNativeCaps.maxVertexOutputComponents = mNativeCaps.maxFragmentInputComponents = 124 - 4;
#else
    if (supportsAppleGPUFamily(3))
    {
        mNativeCaps.max2DTextureSize          = 16384;
        mNativeCaps.maxVertexOutputComponents = mNativeCaps.maxFragmentInputComponents = 124;
        mNativeCaps.maxVaryingVectors = mNativeCaps.maxVertexOutputComponents / 4;
    }
    else
    {
        mNativeCaps.max2DTextureSize          = 8192;
        mNativeCaps.maxVertexOutputComponents = mNativeCaps.maxFragmentInputComponents = 60;
        mNativeCaps.maxVaryingVectors = mNativeCaps.maxVertexOutputComponents / 4;
    }
#endif

    mNativeCaps.maxArrayTextureLayers = 2048;
    mNativeCaps.maxLODBias            = std::log2(mNativeCaps.max2DTextureSize) + 1;
    mNativeCaps.maxCubeMapTextureSize = mNativeCaps.max2DTextureSize;
    mNativeCaps.maxRenderbufferSize   = mNativeCaps.max2DTextureSize;
    mNativeCaps.minAliasedPointSize   = 1;
    // NOTE(hqle): Metal has some problems drawing big point size even though
    // Metal-Feature-Set-Tables.pdf says that max supported point size is 511. We limit it to 64
    // for now. http://anglebug.com/42263403

    // NOTE(kpiddington): This seems to be fixed in macOS Monterey
    if (@available(macOS 12.0, *))
    {
        mNativeCaps.maxAliasedPointSize = 511;
    }
    else
    {
        mNativeCaps.maxAliasedPointSize = 64;
    }
    mNativeCaps.minAliasedLineWidth = 1.0f;
    mNativeCaps.maxAliasedLineWidth = 1.0f;

    if (supportsEitherGPUFamily(2, 1) && !mFeatures.limitMaxDrawBuffersForTesting.enabled)
    {
        mNativeCaps.maxDrawBuffers      = mtl::kMaxRenderTargets;
        mNativeCaps.maxColorAttachments = mtl::kMaxRenderTargets;
    }
    else
    {
        mNativeCaps.maxDrawBuffers      = mtl::kMaxRenderTargetsOlderGPUFamilies;
        mNativeCaps.maxColorAttachments = mtl::kMaxRenderTargetsOlderGPUFamilies;
    }
    ASSERT(static_cast<uint32_t>(mNativeCaps.maxDrawBuffers) <= mtl::kMaxRenderTargets);
    ASSERT(static_cast<uint32_t>(mNativeCaps.maxColorAttachments) <= mtl::kMaxRenderTargets);

    mNativeCaps.maxFramebufferWidth  = mNativeCaps.max2DTextureSize;
    mNativeCaps.maxFramebufferHeight = mNativeCaps.max2DTextureSize;
    mNativeCaps.maxViewportWidth     = mNativeCaps.max2DTextureSize;
    mNativeCaps.maxViewportHeight    = mNativeCaps.max2DTextureSize;

    bool isCatalyst = TARGET_OS_MACCATALYST;

    mMaxColorTargetBits = mtl::kMaxColorTargetBitsApple1To3;
    if (supportsMacGPUFamily(1) || isCatalyst)
    {
        mMaxColorTargetBits = mtl::kMaxColorTargetBitsMacAndCatalyst;
    }
    else if (supportsAppleGPUFamily(4))
    {
        mMaxColorTargetBits = mtl::kMaxColorTargetBitsApple4Plus;
    }

    if (mFeatures.limitMaxColorTargetBitsForTesting.enabled)
    {
        // Set so we have enough for RGBA8 on every attachment
        // but not enough for RGBA32UI.
        mMaxColorTargetBits = mNativeCaps.maxColorAttachments * 32;
    }

    // MSAA
    mNativeCaps.maxSamples             = mFormatTable.getMaxSamples();
    mNativeCaps.maxSampleMaskWords     = 1;
    mNativeCaps.maxColorTextureSamples = mNativeCaps.maxSamples;
    mNativeCaps.maxDepthTextureSamples = mNativeCaps.maxSamples;
    mNativeCaps.maxIntegerSamples      = mNativeCaps.maxSamples;

    mNativeCaps.maxVertexAttributes           = mtl::kMaxVertexAttribs;
    mNativeCaps.maxVertexAttribBindings       = mtl::kMaxVertexAttribs;
    mNativeCaps.maxVertexAttribRelativeOffset = std::numeric_limits<GLint>::max();
    mNativeCaps.maxVertexAttribStride         = std::numeric_limits<GLint>::max();

    // glGet() use signed integer as parameter so we have to use GLint's max here, not GLuint.
    mNativeCaps.maxElementsIndices  = std::numeric_limits<GLint>::max();
    mNativeCaps.maxElementsVertices = std::numeric_limits<GLint>::max();

    // Looks like all floats are IEEE according to the docs here:
    mNativeCaps.vertexHighpFloat.setIEEEFloat();
    mNativeCaps.vertexMediumpFloat.setIEEEFloat();
    mNativeCaps.vertexLowpFloat.setIEEEFloat();
    mNativeCaps.fragmentHighpFloat.setIEEEFloat();
    mNativeCaps.fragmentMediumpFloat.setIEEEFloat();
    mNativeCaps.fragmentLowpFloat.setIEEEFloat();

    mNativeCaps.vertexHighpInt.setTwosComplementInt(32);
    mNativeCaps.vertexMediumpInt.setTwosComplementInt(32);
    mNativeCaps.vertexLowpInt.setTwosComplementInt(32);
    mNativeCaps.fragmentHighpInt.setTwosComplementInt(32);
    mNativeCaps.fragmentMediumpInt.setTwosComplementInt(32);
    mNativeCaps.fragmentLowpInt.setTwosComplementInt(32);

    GLuint maxDefaultUniformVectors = mtl::kDefaultUniformsMaxSize / (sizeof(GLfloat) * 4);

    const GLuint maxDefaultUniformComponents = maxDefaultUniformVectors * 4;

    // Uniforms are implemented using a uniform buffer, so the max number of uniforms we can
    // support is the max buffer range divided by the size of a single uniform (4X float).
    mNativeCaps.maxVertexUniformVectors                              = maxDefaultUniformVectors;
    mNativeCaps.maxShaderUniformComponents[gl::ShaderType::Vertex]   = maxDefaultUniformComponents;
    mNativeCaps.maxFragmentUniformVectors                            = maxDefaultUniformVectors;
    mNativeCaps.maxShaderUniformComponents[gl::ShaderType::Fragment] = maxDefaultUniformComponents;

    mNativeCaps.maxShaderUniformBlocks[gl::ShaderType::Vertex]   = mtl::kMaxShaderUBOs;
    mNativeCaps.maxShaderUniformBlocks[gl::ShaderType::Fragment] = mtl::kMaxShaderUBOs;
    mNativeCaps.maxCombinedUniformBlocks                         = mtl::kMaxGLUBOBindings;

    // Note that we currently implement textures as combined image+samplers, so the limit is
    // the minimum of supported samplers and sampled images.
    mNativeCaps.maxCombinedTextureImageUnits                         = mtl::kMaxGLSamplerBindings;
    mNativeCaps.maxShaderTextureImageUnits[gl::ShaderType::Fragment] = mtl::kMaxShaderSamplers;
    mNativeCaps.maxShaderTextureImageUnits[gl::ShaderType::Vertex]   = mtl::kMaxShaderSamplers;

    // No info from Metal given, use default GLES3 spec values:
    mNativeCaps.minProgramTexelOffset = -8;
    mNativeCaps.maxProgramTexelOffset = 7;

    // NOTE(hqle): support storage buffer.
    const uint32_t maxPerStageStorageBuffers                     = 0;
    mNativeCaps.maxShaderStorageBlocks[gl::ShaderType::Vertex]   = maxPerStageStorageBuffers;
    mNativeCaps.maxShaderStorageBlocks[gl::ShaderType::Fragment] = maxPerStageStorageBuffers;
    mNativeCaps.maxCombinedShaderStorageBlocks                   = maxPerStageStorageBuffers;

    // Fill in additional limits for UBOs and SSBOs.
    mNativeCaps.maxUniformBufferBindings = mNativeCaps.maxCombinedUniformBlocks;
    mNativeCaps.maxUniformBlockSize      = mtl::kMaxUBOSize;  // Default according to GLES 3.0 spec.
    if (supportsAppleGPUFamily(1))
    {
        mNativeCaps.uniformBufferOffsetAlignment =
            16;  // on Apple based GPU's We can ignore data types when setting constant buffer
                 // alignment at 16.
    }
    else
    {
        mNativeCaps.uniformBufferOffsetAlignment =
            256;  // constant buffers on all other GPUs must be aligned to 256.
    }

    mNativeCaps.maxShaderStorageBufferBindings     = 0;
    mNativeCaps.maxShaderStorageBlockSize          = 0;
    mNativeCaps.shaderStorageBufferOffsetAlignment = 0;

    // UBO plus default uniform limits
    const uint32_t maxCombinedUniformComponents =
        maxDefaultUniformComponents + mtl::kMaxUBOSize * mtl::kMaxShaderUBOs / 4;
    for (gl::ShaderType shaderType : gl::kAllGraphicsShaderTypes)
    {
        mNativeCaps.maxCombinedShaderUniformComponents[shaderType] = maxCombinedUniformComponents;
    }

    mNativeCaps.maxCombinedShaderOutputResources = 0;

    mNativeCaps.maxTransformFeedbackInterleavedComponents =
        gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS;
    mNativeCaps.maxTransformFeedbackSeparateAttributes =
        gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS;
    mNativeCaps.maxTransformFeedbackSeparateComponents =
        gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS;

    // GL_OES_get_program_binary
    mNativeCaps.programBinaryFormats.push_back(GL_PROGRAM_BINARY_ANGLE);

    // GL_APPLE_clip_distance / GL_ANGLE_clip_cull_distance
    mNativeCaps.maxClipDistances = 8;

    // Metal doesn't support GL_TEXTURE_COMPARE_MODE=GL_NONE for shadow samplers
    mNativeLimitations.noShadowSamplerCompareModeNone = true;

    // Apple platforms require PVRTC1 textures to be squares.
    mNativeLimitations.squarePvrtc1 = true;
}

void DisplayMtl::initializeExtensions() const
{
    // Reset
    mNativeExtensions = gl::Extensions();

    // Enable this for simple buffer readback testing, but some functionality is missing.
    // NOTE(hqle): Support full mapBufferRangeEXT extension.
    mNativeExtensions.mapbufferOES                  = true;
    mNativeExtensions.mapBufferRangeEXT             = true;
    mNativeExtensions.textureStorageEXT             = true;
    mNativeExtensions.clipControlEXT                = true;
    mNativeExtensions.drawBuffersEXT                = true;
    mNativeExtensions.drawBuffersIndexedEXT         = true;
    mNativeExtensions.drawBuffersIndexedOES         = true;
    mNativeExtensions.fboRenderMipmapOES            = true;
    mNativeExtensions.fragDepthEXT                  = true;
    mNativeExtensions.conservativeDepthEXT          = true;
    mNativeExtensions.framebufferBlitANGLE          = true;
    mNativeExtensions.framebufferBlitNV             = true;
    mNativeExtensions.framebufferMultisampleANGLE   = true;
    mNativeExtensions.polygonModeANGLE              = true;
    mNativeExtensions.polygonOffsetClampEXT         = true;
    mNativeExtensions.stencilTexturingANGLE         = true;
    mNativeExtensions.copyTextureCHROMIUM           = true;
    mNativeExtensions.copyCompressedTextureCHROMIUM = false;
    mNativeExtensions.textureMirrorClampToEdgeEXT   = true;
    mNativeExtensions.depthClampEXT                 = true;

    // EXT_debug_marker is not implemented yet, but the entry points must be exposed for the
    // Metal backend to be used in Chrome (http://anglebug.com/42263519)
    mNativeExtensions.debugMarkerEXT = true;

    mNativeExtensions.robustnessEXT               = true;
    mNativeExtensions.robustnessKHR               = true;
    mNativeExtensions.textureBorderClampOES       = false;  // not implemented yet
    mNativeExtensions.multiDrawIndirectEXT        = true;
    mNativeExtensions.translatedShaderSourceANGLE = true;
    mNativeExtensions.discardFramebufferEXT       = true;
    // TODO(anglebug.com/42264909): Apple's implementation exposed
    // mNativeExtensions.textureRectangle = true here and
    // EGL_TEXTURE_RECTANGLE_ANGLE as the eglBindTexImage texture target on
    // macOS. This no longer seems necessary as IOSurfaces can be bound to
    // regular 2D textures with Metal, and causes other problems such as
    // breaking the SPIR-V Metal compiler.

    mNativeExtensions.multisampledRenderToTextureEXT =
        (supportsAppleGPUFamily(1) ||
         mFeatures.enableMultisampledRenderToTextureOnNonTilers.enabled) &&
        mFeatures.hasShaderStencilOutput.enabled && mFeatures.hasDepthAutoResolve.enabled &&
        mFeatures.hasStencilAutoResolve.enabled;

    // Enable EXT_blend_minmax
    mNativeExtensions.blendMinmaxEXT = true;

    mNativeExtensions.EGLImageOES         = true;
    mNativeExtensions.EGLImageExternalOES = false;
    // NOTE(hqle): Support GL_OES_EGL_image_external_essl3.
    mNativeExtensions.EGLImageExternalEssl3OES = false;

    mNativeExtensions.memoryObjectEXT   = false;
    mNativeExtensions.memoryObjectFdEXT = false;

    mNativeExtensions.semaphoreEXT   = false;
    mNativeExtensions.semaphoreFdEXT = false;

    mNativeExtensions.instancedArraysANGLE = true;
    mNativeExtensions.instancedArraysEXT   = mNativeExtensions.instancedArraysANGLE;

    mNativeExtensions.robustBufferAccessBehaviorKHR = false;

    mNativeExtensions.EGLSyncOES = false;

    mNativeExtensions.occlusionQueryBooleanEXT = true;

    mNativeExtensions.disjointTimerQueryEXT = true;
    mNativeCaps.queryCounterBitsTimeElapsed = 64;
    mNativeCaps.queryCounterBitsTimestamp   = 0;

    mNativeExtensions.textureFilterAnisotropicEXT = true;
    mNativeCaps.maxTextureAnisotropy              = 16;

    mNativeExtensions.textureNpotOES = true;

    mNativeExtensions.texture3DOES = true;

    mNativeExtensions.textureShadowLodEXT = true;

    if ([mMetalDevice areProgrammableSamplePositionsSupported])
    {
        mNativeExtensions.textureMultisampleANGLE = true;
    }

    mNativeExtensions.sampleVariablesOES = true;

    if ([mMetalDevice supportsPullModelInterpolation])
    {
        mNativeExtensions.shaderMultisampleInterpolationOES = true;
        mNativeCaps.subPixelInterpolationOffsetBits         = 4;
        if (supportsAppleGPUFamily(1))
        {
            mNativeCaps.minInterpolationOffset = -0.5f;
            mNativeCaps.maxInterpolationOffset = +0.5f;
        }
        else
        {
            // On non-Apple GPUs, the actual range is usually
            // [-0.5, +0.4375] but due to framebuffer Y-flip
            // the effective range for the Y direction will be
            // [-0.4375, +0.5] when the default FBO is bound.
            mNativeCaps.minInterpolationOffset = -0.4375f;  // -0.5 + (2 ^ -4)
            mNativeCaps.maxInterpolationOffset = +0.4375f;  // +0.5 - (2 ^ -4)
        }
    }

    mNativeExtensions.shaderNoperspectiveInterpolationNV = true;

    mNativeExtensions.shaderTextureLodEXT = true;

    mNativeExtensions.standardDerivativesOES = true;

    mNativeExtensions.elementIndexUintOES = true;

    // GL_OES_get_program_binary
    mNativeExtensions.getProgramBinaryOES = true;

    // GL_APPLE_clip_distance
    mNativeExtensions.clipDistanceAPPLE = true;

    // GL_ANGLE_clip_cull_distance
    mNativeExtensions.clipCullDistanceANGLE = true;

    // GL_NV_pixel_buffer_object
    mNativeExtensions.pixelBufferObjectNV = true;

    mNativeExtensions.packReverseRowOrderANGLE = true;

    if (mFeatures.hasEvents.enabled)
    {
        // MTLSharedEvent is only available since Metal 2.1

        // GL_NV_fence
        mNativeExtensions.fenceNV = true;

        // GL_OES_EGL_sync
        mNativeExtensions.EGLSyncOES = true;

        // GL_ARB_sync
        mNativeExtensions.syncARB = true;
    }

    // GL_KHR_parallel_shader_compile
    mNativeExtensions.parallelShaderCompileKHR = true;

    mNativeExtensions.baseInstanceEXT             = mFeatures.hasBaseVertexInstancedDraw.enabled;
    mNativeExtensions.baseVertexBaseInstanceANGLE = mFeatures.hasBaseVertexInstancedDraw.enabled;
    mNativeExtensions.baseVertexBaseInstanceShaderBuiltinANGLE =
        mFeatures.hasBaseVertexInstancedDraw.enabled;

    // Metal uses the opposite provoking vertex as GLES so emulation is required to use the GLES
    // behaviour. Allow users to change the provoking vertex for improved performance.
    mNativeExtensions.provokingVertexANGLE = true;

    // GL_EXT_blend_func_extended
    mNativeExtensions.blendFuncExtendedEXT = true;
    mNativeCaps.maxDualSourceDrawBuffers   = 1;

    // GL_ANGLE_shader_pixel_local_storage.
    if (!mFeatures.disableProgrammableBlending.enabled && supportsAppleGPUFamily(1))
    {
        // Programmable blending is supported on all Apple GPU families, and is always coherent.
        mNativePLSOptions.type = ShPixelLocalStorageType::FramebufferFetch;

        // Raster order groups are NOT required to make framebuffer fetch coherent, however, they
        // may improve performance by allowing finer grained synchronization (e.g., by assigning
        // attachments to different raster order groups when they don't depend on each other).
        bool rasterOrderGroupsSupported =
            !mFeatures.disableRasterOrderGroups.enabled && supportsAppleGPUFamily(4);
        mNativePLSOptions.fragmentSyncType =
            rasterOrderGroupsSupported ? ShFragmentSynchronizationType::RasterOrderGroups_Metal
                                       : ShFragmentSynchronizationType::Automatic;

        mNativeExtensions.shaderPixelLocalStorageANGLE         = true;
        mNativeExtensions.shaderPixelLocalStorageCoherentANGLE = true;
    }
    else
    {
        MTLReadWriteTextureTier readWriteTextureTier = [mMetalDevice readWriteTextureSupport];
        if (readWriteTextureTier != MTLReadWriteTextureTierNone)
        {
            mNativePLSOptions.type = ShPixelLocalStorageType::ImageLoadStore;

            // Raster order groups are required to make PLS coherent when using read_write textures.
            bool rasterOrderGroupsSupported = !mFeatures.disableRasterOrderGroups.enabled &&
                                              [mMetalDevice areRasterOrderGroupsSupported];
            mNativePLSOptions.fragmentSyncType =
                rasterOrderGroupsSupported ? ShFragmentSynchronizationType::RasterOrderGroups_Metal
                                           : ShFragmentSynchronizationType::NotSupported;

            mNativePLSOptions.supportsNativeRGBA8ImageFormats =
                !mFeatures.disableRWTextureTier2Support.enabled &&
                readWriteTextureTier == MTLReadWriteTextureTier2;

            if (rasterOrderGroupsSupported && isAMD())
            {
                // anglebug.com/42266263 -- [[raster_order_group()]] does not work for read_write
                // textures on AMD when the render pass doesn't have a color attachment on slot 0.
                // To work around this we attach one of the PLS textures to GL_COLOR_ATTACHMENT0, if
                // there isn't one already.
                mNativePLSOptions.renderPassNeedsAMDRasterOrderGroupsWorkaround = true;
            }

            mNativeExtensions.shaderPixelLocalStorageANGLE         = true;
            mNativeExtensions.shaderPixelLocalStorageCoherentANGLE = rasterOrderGroupsSupported;

            // Set up PLS caps here because the higher level context won't have enough info to set
            // them up itself. Shader images and other ES3.1 caps aren't fully exposed yet.
            static_assert(mtl::kMaxShaderImages >=
                          gl::IMPLEMENTATION_MAX_PIXEL_LOCAL_STORAGE_PLANES);
            mNativeCaps.maxPixelLocalStoragePlanes =
                gl::IMPLEMENTATION_MAX_PIXEL_LOCAL_STORAGE_PLANES;
            mNativeCaps.maxColorAttachmentsWithActivePixelLocalStorage =
                gl::IMPLEMENTATION_MAX_DRAW_BUFFERS;
            mNativeCaps.maxCombinedDrawBuffersAndPixelLocalStoragePlanes =
                gl::IMPLEMENTATION_MAX_PIXEL_LOCAL_STORAGE_PLANES +
                gl::IMPLEMENTATION_MAX_DRAW_BUFFERS;
            mNativeCaps.maxShaderImageUniforms[gl::ShaderType::Fragment] =
                gl::IMPLEMENTATION_MAX_PIXEL_LOCAL_STORAGE_PLANES;
            mNativeCaps.maxImageUnits = gl::IMPLEMENTATION_MAX_PIXEL_LOCAL_STORAGE_PLANES;
        }
    }
    // "The GPUs in Apple3 through Apple8 families only support memory barriers for compute command
    // encoders, and for vertex-to-vertex and vertex-to-fragment stages of render command encoders."
    mHasFragmentMemoryBarriers = !supportsAppleGPUFamily(3);
}

void DisplayMtl::initializeTextureCaps() const
{
    mNativeTextureCaps.clear();

    mFormatTable.generateTextureCaps(this, &mNativeTextureCaps);

    // Re-verify texture extensions.
    mNativeExtensions.setTextureExtensionSupport(mNativeTextureCaps);

    // When ETC2/EAC formats are natively supported, enable ANGLE-specific extension string to
    // expose them to WebGL. In other case, mark potentially-available ETC1 extension as
    // emulated.
    if (supportsAppleGPUFamily(1) && gl::DetermineCompressedTextureETCSupport(mNativeTextureCaps))
    {
        mNativeExtensions.compressedTextureEtcANGLE = true;
    }
    else
    {
        mNativeLimitations.emulatedEtc1 = true;
    }

    // Enable EXT_compressed_ETC1_RGB8_sub_texture if ETC1 is supported.
    mNativeExtensions.compressedETC1RGB8SubTextureEXT =
        mNativeExtensions.compressedETC1RGB8TextureOES;

    // Enable ASTC sliced 3D, requires MTLGPUFamilyApple3
    if (supportsAppleGPUFamily(3) && mNativeExtensions.textureCompressionAstcLdrKHR)
    {
        mNativeExtensions.textureCompressionAstcSliced3dKHR = true;
    }

    // Enable ASTC HDR, requires MTLGPUFamilyApple6
    if (supportsAppleGPUFamily(6) && mNativeExtensions.textureCompressionAstcLdrKHR)
    {
        mNativeExtensions.textureCompressionAstcHdrKHR = true;
    }

    // Disable all depth buffer and stencil buffer readback extensions until we need them
    mNativeExtensions.readDepthNV         = false;
    mNativeExtensions.readStencilNV       = false;
    mNativeExtensions.depthBufferFloat2NV = false;
}

void DisplayMtl::initializeLimitations()
{
    mNativeLimitations.noVertexAttributeAliasing = true;
}

void DisplayMtl::initializeFeatures()
{
    bool isOSX       = TARGET_OS_OSX;
    bool isCatalyst  = TARGET_OS_MACCATALYST;
    bool isSimulator = TARGET_OS_SIMULATOR;
    bool isARM       = ANGLE_APPLE_IS_ARM;

    ApplyFeatureOverrides(&mFeatures, getState().featureOverrides);
    if (mState.featureOverrides.allDisabled)
    {
        return;
    }

    ANGLE_FEATURE_CONDITION((&mFeatures), allowGenMultipleMipsPerPass, true);
    ANGLE_FEATURE_CONDITION((&mFeatures), forceBufferGPUStorage, false);
    ANGLE_FEATURE_CONDITION((&mFeatures), hasExplicitMemBarrier, (isOSX || isCatalyst) && !isARM);
    ANGLE_FEATURE_CONDITION((&mFeatures), hasDepthAutoResolve, supportsEitherGPUFamily(3, 2));
    ANGLE_FEATURE_CONDITION((&mFeatures), hasStencilAutoResolve, supportsEitherGPUFamily(5, 2));
    ANGLE_FEATURE_CONDITION((&mFeatures), allowMultisampleStoreAndResolve,
                            supportsEitherGPUFamily(3, 1));

    // Apple2 does not support comparison functions in MTLSamplerState
    ANGLE_FEATURE_CONDITION((&mFeatures), allowRuntimeSamplerCompareMode,
                            supportsEitherGPUFamily(3, 1));

    // AMD does not support sample_compare with gradients
    ANGLE_FEATURE_CONDITION((&mFeatures), allowSamplerCompareGradient, !isAMD());

    // http://anglebug.com/40644746
    // Stencil blit shader is not compiled on Intel & NVIDIA, need investigation.
    ANGLE_FEATURE_CONDITION((&mFeatures), hasShaderStencilOutput, !isIntel() && !isNVIDIA());

    ANGLE_FEATURE_CONDITION((&mFeatures), hasTextureSwizzle,
                            supportsEitherGPUFamily(3, 2) && !isSimulator);

    ANGLE_FEATURE_CONDITION((&mFeatures), avoidStencilTextureSwizzle, isIntel());

    // http://crbug.com/1136673
    // Fence sync is flaky on Nvidia
    ANGLE_FEATURE_CONDITION((&mFeatures), hasEvents, !isNVIDIA());

    ANGLE_FEATURE_CONDITION((&mFeatures), hasCheapRenderPass, (isOSX || isCatalyst) && !isARM);

    // http://anglebug.com/42263788
    // D24S8 is unreliable on AMD.
    ANGLE_FEATURE_CONDITION((&mFeatures), forceD24S8AsUnsupported, isAMD());

    // Base Vertex drawing is only supported since GPU family 3.
    ANGLE_FEATURE_CONDITION((&mFeatures), hasBaseVertexInstancedDraw,
                            isOSX || isCatalyst || supportsAppleGPUFamily(3));

    ANGLE_FEATURE_CONDITION((&mFeatures), hasNonUniformDispatch,
                            isOSX || isCatalyst || supportsAppleGPUFamily(4));

    ANGLE_FEATURE_CONDITION((&mFeatures), allowSeparateDepthStencilBuffers,
                            supportsAppleGPUFamily(1) && !isSimulator);
    ANGLE_FEATURE_CONDITION((&mFeatures), emulateTransformFeedback, true);

    ANGLE_FEATURE_CONDITION((&mFeatures), intelExplicitBoolCastWorkaround,
                            isIntel() && GetMacOSVersion() < OSVersion(11, 0, 0));
    ANGLE_FEATURE_CONDITION((&mFeatures), intelDisableFastMath,
                            isIntel() && GetMacOSVersion() < OSVersion(12, 0, 0));

    ANGLE_FEATURE_CONDITION((&mFeatures), emulateAlphaToCoverage,
                            isSimulator || !supportsAppleGPUFamily(1));

    ANGLE_FEATURE_CONDITION((&mFeatures), writeHelperSampleMask, supportsAppleGPUFamily(1));

    ANGLE_FEATURE_CONDITION((&mFeatures), multisampleColorFormatShaderReadWorkaround, isAMD());
    ANGLE_FEATURE_CONDITION((&mFeatures), copyIOSurfaceToNonIOSurfaceForReadOptimization,
                            isIntel() || isAMD());
    ANGLE_FEATURE_CONDITION((&mFeatures), copyTextureToBufferForReadOptimization, isAMD());

    ANGLE_FEATURE_CONDITION((&mFeatures), forceNonCSBaseMipmapGeneration, isIntel());

    ANGLE_FEATURE_CONDITION((&mFeatures), preemptivelyStartProvokingVertexCommandBuffer, isAMD());

    ANGLE_FEATURE_CONDITION((&mFeatures), alwaysUseStagedBufferUpdates, isAMD());
    ANGLE_FEATURE_CONDITION((&mFeatures), alwaysUseManagedStorageModeForBuffers, isAMD());

    ANGLE_FEATURE_CONDITION((&mFeatures), alwaysUseSharedStorageModeForBuffers, isIntel());
    ANGLE_FEATURE_CONDITION((&mFeatures), useShadowBuffersWhenAppropriate, isIntel());

    // At least one of these must not be set.
    ASSERT(!mFeatures.alwaysUseManagedStorageModeForBuffers.enabled ||
           !mFeatures.alwaysUseSharedStorageModeForBuffers.enabled);

    ANGLE_FEATURE_CONDITION((&mFeatures), uploadDataToIosurfacesWithStagingBuffers, isAMD());

    // Render passes can be rendered without attachments on Apple4 , mac2 hardware.
    ANGLE_FEATURE_CONDITION(&(mFeatures), allowRenderpassWithoutAttachment,
                            supportsEitherGPUFamily(4, 2));

    ANGLE_FEATURE_CONDITION((&mFeatures), enableInMemoryMtlLibraryCache, true);
    ANGLE_FEATURE_CONDITION((&mFeatures), enableParallelMtlLibraryCompilation, true);

    // Uploading texture data via staging buffers improves performance on all tested systems.
    // http://anglebug.com/40644905: Disabled on intel due to some texture formats uploading
    // incorrectly with staging buffers
    ANGLE_FEATURE_CONDITION(&mFeatures, alwaysPreferStagedTextureUploads, true);
    ANGLE_FEATURE_CONDITION(&mFeatures, disableStagedInitializationOfPackedTextureFormats,
                            isIntel() || isAMD());

    ANGLE_FEATURE_CONDITION((&mFeatures), generateShareableShaders, true);

    // http://anglebug.com/42266609: NVIDIA GPUs are unsupported due to scarcity of the hardware.
    ANGLE_FEATURE_CONDITION((&mFeatures), disableMetalOnNvidia, true);

    // The AMDMTLBronzeDriver seems to have bugs flushing vertex data to the GPU during some kinds
    // of buffer uploads which require a flush to work around.
    ANGLE_FEATURE_CONDITION((&mFeatures), flushAfterStreamVertexData, isAMDBronzeDriver());

    // TODO(anglebug.com/40096869): GPUs that don't support Mac GPU family 2 or greater are
    // unsupported by the Metal backend.
    ANGLE_FEATURE_CONDITION((&mFeatures), requireGpuFamily2, true);

    // http://anglebug.com/42266744: Rescope global variables which are only used in one function to
    // be function local. Disabled on AMD FirePro devices: http://anglebug.com/40096898
    ANGLE_FEATURE_CONDITION((&mFeatures), rescopeGlobalVariables, !isAMDFireProDevice());

    // Apple-specific pre-transform for explicit cubemap derivatives
    ANGLE_FEATURE_CONDITION((&mFeatures), preTransformTextureCubeGradDerivatives,
                            supportsAppleGPUFamily(1));

    // Apple-specific cubemap derivative transformation is not compatible with
    // the textureGrad shadow sampler emulation. The latter is used only on AMD.
    ASSERT(!mFeatures.preTransformTextureCubeGradDerivatives.enabled ||
           mFeatures.allowSamplerCompareGradient.enabled);

    // Metal compiler optimizations may remove infinite loops causing crashes later in shader
    // execution. http://crbug.com/1513738
    // Disabled on Mac11 due to test failures. http://crbug.com/1522730
    ANGLE_FEATURE_CONDITION((&mFeatures), injectAsmStatementIntoLoopBodies,
                            !isOSX || GetMacOSVersion() >= OSVersion(12, 0, 0));
}

angle::Result DisplayMtl::initializeShaderLibrary()
{
    mtl::AutoObjCPtr<NSError *> err = nil;
#if ANGLE_METAL_XCODE_BUILDS_SHADERS || ANGLE_METAL_HAS_PREBUILT_INTERNAL_SHADERS
    mDefaultShaders = mtl::CreateShaderLibraryFromStaticBinary(getMetalDevice(), gDefaultMetallib,
                                                               std::size(gDefaultMetallib), &err);
#else
    const bool disableFastMath = false;
    const bool usesInvariance  = true;
    mDefaultShaders            = mtl::CreateShaderLibrary(getMetalDevice(), gDefaultMetallibSrc, {},
                                                          disableFastMath, usesInvariance, &err);
#endif

    if (err)
    {
        ERR() << "Internal error: " << err.get().localizedDescription.UTF8String;
        return angle::Result::Stop;
    }

    return angle::Result::Continue;
}

id<MTLLibrary> DisplayMtl::getDefaultShadersLib()
{
    return mDefaultShaders;
}

bool DisplayMtl::supportsAppleGPUFamily(uint8_t iOSFamily) const
{
    return mtl::SupportsAppleGPUFamily(getMetalDevice(), iOSFamily);
}

bool DisplayMtl::supportsMacGPUFamily(uint8_t macFamily) const
{
    return mtl::SupportsMacGPUFamily(getMetalDevice(), macFamily);
}

bool DisplayMtl::supportsEitherGPUFamily(uint8_t iOSFamily, uint8_t macFamily) const
{
    return supportsAppleGPUFamily(iOSFamily) || supportsMacGPUFamily(macFamily);
}

bool DisplayMtl::supports32BitFloatFiltering() const
{
#if !TARGET_OS_WATCH
    return [mMetalDevice supports32BitFloatFiltering];
#else
    return false;
#endif
}

bool DisplayMtl::supportsBCTextureCompression() const
{
    if (@available(macCatalyst 16.4, iOS 16.4, *))
    {
        return [mMetalDevice supportsBCTextureCompression];
    }
#if TARGET_OS_MACCATALYST
    return true;  // Always true on old Catalyst
#else
    return false;  // Always false on old iOS
#endif
}

bool DisplayMtl::supportsDepth24Stencil8PixelFormat() const
{
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
    return [mMetalDevice isDepth24Stencil8PixelFormatSupported];
#else
    return false;
#endif
}
bool DisplayMtl::isAMD() const
{
    return angle::IsAMD(mMetalDeviceVendorId);
}
bool DisplayMtl::isAMDBronzeDriver() const
{
    if (!isAMD())
    {
        return false;
    }

    if (mComputedAMDBronze)
    {
        return mIsAMDBronze;
    }

    // All devices known to be covered by AMDMTlBronzeDriver.
    //
    // Note that we can not compare substrings because some devices
    // (AMD Radeon Pro 560) are substrings of ones supported by a
    // later driver (AMD Radeon Pro 5600M).
    NSString *kMTLBronzeDeviceNames[22] = {
        @"FirePro D300",    @"FirePro D500",    @"FirePro D700",   @"Radeon R9 M290",
        @"Radeon R9 M290X", @"Radeon R9 M370X", @"Radeon R9 M380", @"Radeon R9 M390",
        @"Radeon R9 M395",  @"Radeon Pro 450",  @"Radeon Pro 455", @"Radeon Pro 460",
        @"Radeon Pro 555",  @"Radeon Pro 555X", @"Radeon Pro 560", @"Radeon Pro 560X",
        @"Radeon Pro 570",  @"Radeon Pro 570X", @"Radeon Pro 575", @"Radeon Pro 575X",
        @"Radeon Pro 580",  @"Radeon Pro 580X"};

    for (size_t i = 0; i < ArraySize(kMTLBronzeDeviceNames); ++i)
    {
        if ([[mMetalDevice name] hasSuffix:kMTLBronzeDeviceNames[i]])
        {
            mIsAMDBronze = true;
            break;
        }
    }

    mComputedAMDBronze = true;
    return mIsAMDBronze;
}

bool DisplayMtl::isAMDFireProDevice() const
{
    if (!isAMD())
    {
        return false;
    }

    return [[mMetalDevice name] containsString:@"FirePro"];
}

bool DisplayMtl::isIntel() const
{
    return angle::IsIntel(mMetalDeviceVendorId);
}

bool DisplayMtl::isNVIDIA() const
{
    return angle::IsNVIDIA(mMetalDeviceVendorId);
}

bool DisplayMtl::isSimulator() const
{
    return TARGET_OS_SIMULATOR;
}

mtl::AutoObjCPtr<MTLSharedEventListener *> DisplayMtl::getOrCreateSharedEventListener()
{
    if (!mSharedEventListener)
    {
        ANGLE_MTL_OBJC_SCOPE
        {
            mSharedEventListener = mtl::adoptObjCPtr([[MTLSharedEventListener alloc] init]);
            ASSERT(mSharedEventListener);  // Failure here most probably means a sandbox issue.
        }
    }
    return mSharedEventListener;
}

}  // namespace rx
