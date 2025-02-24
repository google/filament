//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Renderer9.cpp: Implements a back-end specific class for the D3D9 renderer.

#include "libANGLE/renderer/d3d/d3d9/Renderer9.h"

#include <EGL/eglext.h>
#include <sstream>

#include "common/utilities.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/Program.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/State.h"
#include "libANGLE/Surface.h"
#include "libANGLE/Texture.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/features.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/CompilerD3D.h"
#include "libANGLE/renderer/d3d/DisplayD3D.h"
#include "libANGLE/renderer/d3d/FramebufferD3D.h"
#include "libANGLE/renderer/d3d/IndexDataManager.h"
#include "libANGLE/renderer/d3d/ProgramD3D.h"
#include "libANGLE/renderer/d3d/ProgramExecutableD3D.h"
#include "libANGLE/renderer/d3d/RenderbufferD3D.h"
#include "libANGLE/renderer/d3d/ShaderD3D.h"
#include "libANGLE/renderer/d3d/SurfaceD3D.h"
#include "libANGLE/renderer/d3d/TextureD3D.h"
#include "libANGLE/renderer/d3d/d3d9/Blit9.h"
#include "libANGLE/renderer/d3d/d3d9/Buffer9.h"
#include "libANGLE/renderer/d3d/d3d9/Context9.h"
#include "libANGLE/renderer/d3d/d3d9/Device9.h"
#include "libANGLE/renderer/d3d/d3d9/Fence9.h"
#include "libANGLE/renderer/d3d/d3d9/Framebuffer9.h"
#include "libANGLE/renderer/d3d/d3d9/Image9.h"
#include "libANGLE/renderer/d3d/d3d9/IndexBuffer9.h"
#include "libANGLE/renderer/d3d/d3d9/NativeWindow9.h"
#include "libANGLE/renderer/d3d/d3d9/Query9.h"
#include "libANGLE/renderer/d3d/d3d9/RenderTarget9.h"
#include "libANGLE/renderer/d3d/d3d9/ShaderExecutable9.h"
#include "libANGLE/renderer/d3d/d3d9/SwapChain9.h"
#include "libANGLE/renderer/d3d/d3d9/TextureStorage9.h"
#include "libANGLE/renderer/d3d/d3d9/VertexArray9.h"
#include "libANGLE/renderer/d3d/d3d9/VertexBuffer9.h"
#include "libANGLE/renderer/d3d/d3d9/formatutils9.h"
#include "libANGLE/renderer/d3d/d3d9/renderer9_utils.h"
#include "libANGLE/renderer/d3d/driver_utils_d3d.h"
#include "libANGLE/renderer/driver_utils.h"
#include "libANGLE/trace.h"

#if !defined(ANGLE_COMPILE_OPTIMIZATION_LEVEL)
#    define ANGLE_COMPILE_OPTIMIZATION_LEVEL D3DCOMPILE_OPTIMIZATION_LEVEL3
#endif

// Enable ANGLE_SUPPORT_SHADER_MODEL_2 if you wish devices with only shader model 2.
// Such a device would not be conformant.
#ifndef ANGLE_SUPPORT_SHADER_MODEL_2
#    define ANGLE_SUPPORT_SHADER_MODEL_2 0
#endif

namespace rx
{

namespace
{
enum
{
    MAX_VERTEX_CONSTANT_VECTORS_D3D9 = 256,
    MAX_PIXEL_CONSTANT_VECTORS_SM2   = 32,
    MAX_PIXEL_CONSTANT_VECTORS_SM3   = 224,
    MAX_VARYING_VECTORS_SM2          = 8,
    MAX_VARYING_VECTORS_SM3          = 10,

    MAX_TEXTURE_IMAGE_UNITS_VTF_SM3 = 4
};

template <typename T>
static void DrawPoints(IDirect3DDevice9 *device, GLsizei count, const void *indices, int minIndex)
{
    for (int i = 0; i < count; i++)
    {
        unsigned int indexValue =
            static_cast<unsigned int>(static_cast<const T *>(indices)[i]) - minIndex;
        device->DrawPrimitive(D3DPT_POINTLIST, indexValue, 1);
    }
}

// A hard limit on buffer size. This works around a problem in the NVIDIA drivers where buffer sizes
// close to MAX_UINT would give undefined results. The limit of MAX_UINT/2 should be generous enough
// for almost any demanding application.
constexpr UINT kMaximumBufferSizeHardLimit = std::numeric_limits<UINT>::max() >> 1;
}  // anonymous namespace

Renderer9::Renderer9(egl::Display *display) : RendererD3D(display), mStateManager(this)
{
    mD3d9Module = nullptr;

    mD3d9         = nullptr;
    mD3d9Ex       = nullptr;
    mDevice       = nullptr;
    mDeviceEx     = nullptr;
    mDeviceWindow = nullptr;
    mBlit         = nullptr;

    mAdapter = D3DADAPTER_DEFAULT;

    const egl::AttributeMap &attributes = display->getAttributeMap();
    EGLint requestedDeviceType          = static_cast<EGLint>(attributes.get(
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE));
    switch (requestedDeviceType)
    {
        case EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE:
            mDeviceType = D3DDEVTYPE_HAL;
            break;

        case EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_REFERENCE_ANGLE:
            mDeviceType = D3DDEVTYPE_REF;
            break;

        case EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE:
            mDeviceType = D3DDEVTYPE_NULLREF;
            break;

        default:
            UNREACHABLE();
    }

    mMaskedClearSavedState = nullptr;

    mVertexDataManager = nullptr;
    mIndexDataManager  = nullptr;
    mLineLoopIB        = nullptr;
    mCountingIB        = nullptr;

    mMaxNullColorbufferLRU = 0;
    for (int i = 0; i < NUM_NULL_COLORBUFFER_CACHE_ENTRIES; i++)
    {
        mNullRenderTargetCache[i].lruCount     = 0;
        mNullRenderTargetCache[i].width        = 0;
        mNullRenderTargetCache[i].height       = 0;
        mNullRenderTargetCache[i].renderTarget = nullptr;
    }

    mAppliedVertexShader  = nullptr;
    mAppliedPixelShader   = nullptr;
    mAppliedProgramSerial = 0;

    gl::InitializeDebugAnnotations(&mAnnotator);
}

void Renderer9::setGlobalDebugAnnotator()
{
    gl::InitializeDebugAnnotations(&mAnnotator);
}

Renderer9::~Renderer9()
{
    if (mDevice)
    {
        // If the device is lost, reset it first to prevent leaving the driver in an unstable state
        if (testDeviceLost())
        {
            resetDevice();
        }
    }

    release();
}

void Renderer9::release()
{
    gl::UninitializeDebugAnnotations();

    mTranslatedAttribCache.clear();

    releaseDeviceResources();

    SafeRelease(mDevice);
    SafeRelease(mDeviceEx);
    SafeRelease(mD3d9);
    SafeRelease(mD3d9Ex);

    mCompiler.release();

    if (mDeviceWindow)
    {
        DestroyWindow(mDeviceWindow);
        mDeviceWindow = nullptr;
    }

    mD3d9Module = nullptr;
}

egl::Error Renderer9::initialize()
{
    ANGLE_TRACE_EVENT0("gpu.angle", "GetModuleHandle_d3d9");
    mD3d9Module = ::LoadLibrary(TEXT("d3d9.dll"));

    if (mD3d9Module == nullptr)
    {
        return egl::EglNotInitialized(D3D9_INIT_MISSING_DEP) << "No D3D9 module found.";
    }

    typedef HRESULT(WINAPI * Direct3DCreate9ExFunc)(UINT, IDirect3D9Ex **);
    Direct3DCreate9ExFunc Direct3DCreate9ExPtr =
        reinterpret_cast<Direct3DCreate9ExFunc>(GetProcAddress(mD3d9Module, "Direct3DCreate9Ex"));

    // Use Direct3D9Ex if available. Among other things, this version is less
    // inclined to report a lost context, for example when the user switches
    // desktop. Direct3D9Ex is available in Windows Vista and later if suitable drivers are
    // available.
    if (static_cast<bool>(ANGLE_D3D9EX) && Direct3DCreate9ExPtr &&
        SUCCEEDED(Direct3DCreate9ExPtr(D3D_SDK_VERSION, &mD3d9Ex)))
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "D3d9Ex_QueryInterface");
        ASSERT(mD3d9Ex);
        mD3d9Ex->QueryInterface(__uuidof(IDirect3D9), reinterpret_cast<void **>(&mD3d9));
        ASSERT(mD3d9);
    }
    else
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "Direct3DCreate9");
        mD3d9 = Direct3DCreate9(D3D_SDK_VERSION);
    }

    if (!mD3d9)
    {
        return egl::EglNotInitialized(D3D9_INIT_MISSING_DEP) << "Could not create D3D9 device.";
    }

    if (mDisplay->getNativeDisplayId() != nullptr)
    {
        //  UNIMPLEMENTED();   // FIXME: Determine which adapter index the device context
        //  corresponds to
    }

    HRESULT result;

    // Give up on getting device caps after about one second.
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "GetDeviceCaps");
        for (int i = 0; i < 10; ++i)
        {
            result = mD3d9->GetDeviceCaps(mAdapter, mDeviceType, &mDeviceCaps);
            if (SUCCEEDED(result))
            {
                break;
            }
            else if (result == D3DERR_NOTAVAILABLE)
            {
                Sleep(100);  // Give the driver some time to initialize/recover
            }
            else if (FAILED(result))  // D3DERR_OUTOFVIDEOMEMORY, E_OUTOFMEMORY,
                                      // D3DERR_INVALIDDEVICE, or another error we can't recover
                                      // from
            {
                return egl::EglNotInitialized(D3D9_INIT_OTHER_ERROR)
                       << "Failed to get device caps, " << gl::FmtHR(result);
            }
        }
    }

#if ANGLE_SUPPORT_SHADER_MODEL_2
    size_t minShaderModel = 2;
#else
    size_t minShaderModel = 3;
#endif

    if (mDeviceCaps.PixelShaderVersion < D3DPS_VERSION(minShaderModel, 0))
    {
        return egl::EglNotInitialized(D3D9_INIT_UNSUPPORTED_VERSION)
               << "Renderer does not support PS " << minShaderModel << ".0, aborting!";
    }

    // When DirectX9 is running with an older DirectX8 driver, a StretchRect from a regular texture
    // to a render target texture is not supported. This is required by
    // Texture2D::ensureRenderTarget.
    if ((mDeviceCaps.DevCaps2 & D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES) == 0)
    {
        return egl::EglNotInitialized(D3D9_INIT_UNSUPPORTED_STRETCHRECT)
               << "Renderer does not support StretctRect from textures.";
    }

    {
        ANGLE_TRACE_EVENT0("gpu.angle", "GetAdapterIdentifier");
        mD3d9->GetAdapterIdentifier(mAdapter, 0, &mAdapterIdentifier);
    }

    static const TCHAR windowName[] = TEXT("AngleHiddenWindow");
    static const TCHAR className[]  = TEXT("STATIC");

    {
        ANGLE_TRACE_EVENT0("gpu.angle", "CreateWindowEx");
        mDeviceWindow =
            CreateWindowEx(WS_EX_NOACTIVATE, className, windowName, WS_DISABLED | WS_POPUP, 0, 0, 1,
                           1, HWND_MESSAGE, nullptr, GetModuleHandle(nullptr), nullptr);
    }

    D3DPRESENT_PARAMETERS presentParameters = getDefaultPresentParameters();
    DWORD behaviorFlags =
        D3DCREATE_FPU_PRESERVE | D3DCREATE_NOWINDOWCHANGES | D3DCREATE_MULTITHREADED;

    {
        ANGLE_TRACE_EVENT0("gpu.angle", "D3d9_CreateDevice");
        result = mD3d9->CreateDevice(
            mAdapter, mDeviceType, mDeviceWindow,
            behaviorFlags | D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE,
            &presentParameters, &mDevice);

        if (FAILED(result))
        {
            ERR() << "CreateDevice1 failed: (" << gl::FmtHR(result) << ")";
        }
    }
    if (result == D3DERR_OUTOFVIDEOMEMORY || result == E_OUTOFMEMORY || result == D3DERR_DEVICELOST)
    {
        return egl::EglBadAlloc(D3D9_INIT_OUT_OF_MEMORY)
               << "CreateDevice failed: device lost or out of memory (" << gl::FmtHR(result) << ")";
    }

    if (FAILED(result))
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "D3d9_CreateDevice2");
        result = mD3d9->CreateDevice(mAdapter, mDeviceType, mDeviceWindow,
                                     behaviorFlags | D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                     &presentParameters, &mDevice);

        if (FAILED(result))
        {
            ASSERT(result == D3DERR_OUTOFVIDEOMEMORY || result == E_OUTOFMEMORY ||
                   result == D3DERR_NOTAVAILABLE || result == D3DERR_DEVICELOST);
            return egl::EglBadAlloc(D3D9_INIT_OUT_OF_MEMORY)
                   << "CreateDevice2 failed: device lost, not available, or of out of memory ("
                   << gl::FmtHR(result) << ")";
        }
    }

    if (mD3d9Ex)
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "mDevice_QueryInterface");
        result = mDevice->QueryInterface(__uuidof(IDirect3DDevice9Ex), (void **)&mDeviceEx);
        ASSERT(SUCCEEDED(result));
    }

    {
        ANGLE_TRACE_EVENT0("gpu.angle", "ShaderCache initialize");
        mVertexShaderCache.initialize(mDevice);
        mPixelShaderCache.initialize(mDevice);
    }

    D3DDISPLAYMODE currentDisplayMode;
    mD3d9->GetAdapterDisplayMode(mAdapter, &currentDisplayMode);

    // Check vertex texture support
    // Only Direct3D 10 ready devices support all the necessary vertex texture formats.
    // We test this using D3D9 by checking support for the R16F format.
    mVertexTextureSupport = mDeviceCaps.PixelShaderVersion >= D3DPS_VERSION(3, 0) &&
                            SUCCEEDED(mD3d9->CheckDeviceFormat(
                                mAdapter, mDeviceType, currentDisplayMode.Format,
                                D3DUSAGE_QUERY_VERTEXTEXTURE, D3DRTYPE_TEXTURE, D3DFMT_R16F));

    ANGLE_TRY(initializeDevice());

    return egl::NoError();
}

// do any one-time device initialization
// NOTE: this is also needed after a device lost/reset
// to reset the scene status and ensure the default states are reset.
egl::Error Renderer9::initializeDevice()
{
    // Permanent non-default states
    mDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE);
    mDevice->SetRenderState(D3DRS_LASTPIXEL, FALSE);

    if (mDeviceCaps.PixelShaderVersion >= D3DPS_VERSION(3, 0))
    {
        mDevice->SetRenderState(D3DRS_POINTSIZE_MAX, (DWORD &)mDeviceCaps.MaxPointSize);
    }
    else
    {
        mDevice->SetRenderState(D3DRS_POINTSIZE_MAX, 0x3F800000);  // 1.0f
    }

    const gl::Caps &rendererCaps = getNativeCaps();

    mCurVertexSamplerStates.resize(rendererCaps.maxShaderTextureImageUnits[gl::ShaderType::Vertex]);
    mCurPixelSamplerStates.resize(
        rendererCaps.maxShaderTextureImageUnits[gl::ShaderType::Fragment]);

    mCurVertexTextures.resize(rendererCaps.maxShaderTextureImageUnits[gl::ShaderType::Vertex]);
    mCurPixelTextures.resize(rendererCaps.maxShaderTextureImageUnits[gl::ShaderType::Fragment]);

    markAllStateDirty();

    mSceneStarted = false;

    ASSERT(!mBlit);
    mBlit = new Blit9(this);

    ASSERT(!mVertexDataManager && !mIndexDataManager);
    mIndexDataManager = new IndexDataManager(this);

    mTranslatedAttribCache.resize(getNativeCaps().maxVertexAttributes);

    mStateManager.initialize();

    return egl::NoError();
}

D3DPRESENT_PARAMETERS Renderer9::getDefaultPresentParameters()
{
    D3DPRESENT_PARAMETERS presentParameters = {};

    // The default swap chain is never actually used. Surface will create a new swap chain with the
    // proper parameters.
    presentParameters.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
    presentParameters.BackBufferCount        = 1;
    presentParameters.BackBufferFormat       = D3DFMT_UNKNOWN;
    presentParameters.BackBufferWidth        = 1;
    presentParameters.BackBufferHeight       = 1;
    presentParameters.EnableAutoDepthStencil = FALSE;
    presentParameters.Flags                  = 0;
    presentParameters.hDeviceWindow          = mDeviceWindow;
    presentParameters.MultiSampleQuality     = 0;
    presentParameters.MultiSampleType        = D3DMULTISAMPLE_NONE;
    presentParameters.PresentationInterval   = D3DPRESENT_INTERVAL_DEFAULT;
    presentParameters.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    presentParameters.Windowed               = TRUE;

    return presentParameters;
}

egl::ConfigSet Renderer9::generateConfigs()
{
    static const GLenum colorBufferFormats[] = {
        GL_BGR5_A1_ANGLEX,
        GL_BGRA8_EXT,
        GL_RGB565,

    };

    static const GLenum depthStencilBufferFormats[] = {
        GL_NONE,
        GL_DEPTH_COMPONENT32_OES,
        GL_DEPTH24_STENCIL8_OES,
        GL_DEPTH_COMPONENT24_OES,
        GL_DEPTH_COMPONENT16,
    };

    const gl::Caps &rendererCaps                  = getNativeCaps();
    const gl::TextureCapsMap &rendererTextureCaps = getNativeTextureCaps();

    D3DDISPLAYMODE currentDisplayMode;
    mD3d9->GetAdapterDisplayMode(mAdapter, &currentDisplayMode);

    // Determine the min and max swap intervals
    int minSwapInterval = 4;
    int maxSwapInterval = 0;

    if (mDeviceCaps.PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE)
    {
        minSwapInterval = std::min(minSwapInterval, 0);
        maxSwapInterval = std::max(maxSwapInterval, 0);
    }
    if (mDeviceCaps.PresentationIntervals & D3DPRESENT_INTERVAL_ONE)
    {
        minSwapInterval = std::min(minSwapInterval, 1);
        maxSwapInterval = std::max(maxSwapInterval, 1);
    }
    if (mDeviceCaps.PresentationIntervals & D3DPRESENT_INTERVAL_TWO)
    {
        minSwapInterval = std::min(minSwapInterval, 2);
        maxSwapInterval = std::max(maxSwapInterval, 2);
    }
    if (mDeviceCaps.PresentationIntervals & D3DPRESENT_INTERVAL_THREE)
    {
        minSwapInterval = std::min(minSwapInterval, 3);
        maxSwapInterval = std::max(maxSwapInterval, 3);
    }
    if (mDeviceCaps.PresentationIntervals & D3DPRESENT_INTERVAL_FOUR)
    {
        minSwapInterval = std::min(minSwapInterval, 4);
        maxSwapInterval = std::max(maxSwapInterval, 4);
    }

    egl::ConfigSet configs;
    for (size_t formatIndex = 0; formatIndex < ArraySize(colorBufferFormats); formatIndex++)
    {
        GLenum colorBufferInternalFormat = colorBufferFormats[formatIndex];
        const gl::TextureCaps &colorBufferFormatCaps =
            rendererTextureCaps.get(colorBufferInternalFormat);
        if (colorBufferFormatCaps.renderbuffer)
        {
            ASSERT(colorBufferFormatCaps.textureAttachment);
            for (size_t depthStencilIndex = 0;
                 depthStencilIndex < ArraySize(depthStencilBufferFormats); depthStencilIndex++)
            {
                GLenum depthStencilBufferInternalFormat =
                    depthStencilBufferFormats[depthStencilIndex];
                const gl::TextureCaps &depthStencilBufferFormatCaps =
                    rendererTextureCaps.get(depthStencilBufferInternalFormat);
                if (depthStencilBufferFormatCaps.renderbuffer ||
                    depthStencilBufferInternalFormat == GL_NONE)
                {
                    ASSERT(depthStencilBufferFormatCaps.textureAttachment ||
                           depthStencilBufferInternalFormat == GL_NONE);
                    const gl::InternalFormat &colorBufferFormatInfo =
                        gl::GetSizedInternalFormatInfo(colorBufferInternalFormat);
                    const gl::InternalFormat &depthStencilBufferFormatInfo =
                        gl::GetSizedInternalFormatInfo(depthStencilBufferInternalFormat);
                    const d3d9::TextureFormat &d3d9ColorBufferFormatInfo =
                        d3d9::GetTextureFormatInfo(colorBufferInternalFormat);

                    egl::Config config;
                    config.renderTargetFormat = colorBufferInternalFormat;
                    config.depthStencilFormat = depthStencilBufferInternalFormat;
                    config.bufferSize         = colorBufferFormatInfo.getEGLConfigBufferSize();
                    config.redSize            = colorBufferFormatInfo.redBits;
                    config.greenSize          = colorBufferFormatInfo.greenBits;
                    config.blueSize           = colorBufferFormatInfo.blueBits;
                    config.luminanceSize      = colorBufferFormatInfo.luminanceBits;
                    config.alphaSize          = colorBufferFormatInfo.alphaBits;
                    config.alphaMaskSize      = 0;
                    config.bindToTextureRGB   = (colorBufferFormatInfo.format == GL_RGB);
                    config.bindToTextureRGBA  = (colorBufferFormatInfo.format == GL_RGBA ||
                                                colorBufferFormatInfo.format == GL_BGRA_EXT);
                    config.colorBufferType    = EGL_RGB_BUFFER;
                    // Mark as slow if blits to the back-buffer won't be straight forward
                    config.configCaveat =
                        (currentDisplayMode.Format == d3d9ColorBufferFormatInfo.renderFormat)
                            ? EGL_NONE
                            : EGL_SLOW_CONFIG;
                    config.configID          = static_cast<EGLint>(configs.size() + 1);
                    config.conformant        = EGL_OPENGL_ES2_BIT;
                    config.depthSize         = depthStencilBufferFormatInfo.depthBits;
                    config.level             = 0;
                    config.matchNativePixmap = EGL_NONE;
                    config.maxPBufferWidth   = rendererCaps.max2DTextureSize;
                    config.maxPBufferHeight  = rendererCaps.max2DTextureSize;
                    config.maxPBufferPixels =
                        rendererCaps.max2DTextureSize * rendererCaps.max2DTextureSize;
                    config.maxSwapInterval  = maxSwapInterval;
                    config.minSwapInterval  = minSwapInterval;
                    config.nativeRenderable = EGL_FALSE;
                    config.nativeVisualID   = 0;
                    config.nativeVisualType = EGL_NONE;
                    config.renderableType   = EGL_OPENGL_ES2_BIT;
                    config.sampleBuffers    = 0;  // FIXME: enumerate multi-sampling
                    config.samples          = 0;
                    config.stencilSize      = depthStencilBufferFormatInfo.stencilBits;
                    config.surfaceType =
                        EGL_PBUFFER_BIT | EGL_WINDOW_BIT | EGL_SWAP_BEHAVIOR_PRESERVED_BIT;
                    config.transparentType       = EGL_NONE;
                    config.transparentRedValue   = 0;
                    config.transparentGreenValue = 0;
                    config.transparentBlueValue  = 0;
                    config.colorComponentType    = gl_egl::GLComponentTypeToEGLColorComponentType(
                        colorBufferFormatInfo.componentType);

                    configs.add(config);
                }
            }
        }
    }

    ASSERT(configs.size() > 0);
    return configs;
}

void Renderer9::generateDisplayExtensions(egl::DisplayExtensions *outExtensions) const
{
    outExtensions->createContextRobustness = true;

    if (getShareHandleSupport())
    {
        outExtensions->d3dShareHandleClientBuffer     = true;
        outExtensions->surfaceD3DTexture2DShareHandle = true;
    }
    outExtensions->d3dTextureClientBuffer = true;

    outExtensions->querySurfacePointer = true;
    outExtensions->windowFixedSize     = true;
    outExtensions->postSubBuffer       = true;

    outExtensions->image               = true;
    outExtensions->imageBase           = true;
    outExtensions->glTexture2DImage    = true;
    outExtensions->glRenderbufferImage = true;

    outExtensions->noConfigContext = true;

    // Contexts are virtualized so textures and semaphores can be shared globally
    outExtensions->displayTextureShareGroup   = true;
    outExtensions->displaySemaphoreShareGroup = true;

    // D3D9 can be used without an output surface
    outExtensions->surfacelessContext = true;

    outExtensions->robustResourceInitializationANGLE = true;
}

void Renderer9::startScene()
{
    if (!mSceneStarted)
    {
        long result = mDevice->BeginScene();
        if (SUCCEEDED(result))
        {
            // This is defensive checking against the device being
            // lost at unexpected times.
            mSceneStarted = true;
        }
    }
}

void Renderer9::endScene()
{
    if (mSceneStarted)
    {
        // EndScene can fail if the device was lost, for example due
        // to a TDR during a draw call.
        mDevice->EndScene();
        mSceneStarted = false;
    }
}

angle::Result Renderer9::flush(const gl::Context *context)
{
    IDirect3DQuery9 *query = nullptr;
    ANGLE_TRY(allocateEventQuery(context, &query));

    Context9 *context9 = GetImplAs<Context9>(context);

    HRESULT result = query->Issue(D3DISSUE_END);
    ANGLE_TRY_HR(context9, result, "Failed to issue event query");

    // Grab the query data once
    result = query->GetData(nullptr, 0, D3DGETDATA_FLUSH);
    freeEventQuery(query);
    ANGLE_TRY_HR(context9, result, "Failed to get event query data");

    return angle::Result::Continue;
}

angle::Result Renderer9::finish(const gl::Context *context)
{
    IDirect3DQuery9 *query = nullptr;
    ANGLE_TRY(allocateEventQuery(context, &query));

    Context9 *context9 = GetImplAs<Context9>(context);

    HRESULT result = query->Issue(D3DISSUE_END);
    ANGLE_TRY_HR(context9, result, "Failed to issue event query");

    // Grab the query data once
    result = query->GetData(nullptr, 0, D3DGETDATA_FLUSH);
    if (FAILED(result))
    {
        freeEventQuery(query);
    }
    ANGLE_TRY_HR(context9, result, "Failed to get event query data");

    // Loop until the query completes
    unsigned int attempt = 0;
    while (result == S_FALSE)
    {
        // Keep polling, but allow other threads to do something useful first
        std::this_thread::yield();

        result = query->GetData(nullptr, 0, D3DGETDATA_FLUSH);
        attempt++;

        if (result == S_FALSE)
        {
            // explicitly check for device loss
            // some drivers seem to return S_FALSE even if the device is lost
            // instead of D3DERR_DEVICELOST like they should
            bool checkDeviceLost = (attempt % kPollingD3DDeviceLostCheckFrequency) == 0;
            if (checkDeviceLost && testDeviceLost())
            {
                result = D3DERR_DEVICELOST;
            }
        }

        if (FAILED(result))
        {
            freeEventQuery(query);
        }
        ANGLE_TRY_HR(context9, result, "Failed to get event query data");
    }

    freeEventQuery(query);

    return angle::Result::Continue;
}

bool Renderer9::isValidNativeWindow(EGLNativeWindowType window) const
{
    return NativeWindow9::IsValidNativeWindow(window);
}

NativeWindowD3D *Renderer9::createNativeWindow(EGLNativeWindowType window,
                                               const egl::Config *,
                                               const egl::AttributeMap &) const
{
    return new NativeWindow9(window);
}

SwapChainD3D *Renderer9::createSwapChain(NativeWindowD3D *nativeWindow,
                                         HANDLE shareHandle,
                                         IUnknown *d3dTexture,
                                         GLenum backBufferFormat,
                                         GLenum depthBufferFormat,
                                         EGLint orientation,
                                         EGLint samples)
{
    return new SwapChain9(this, GetAs<NativeWindow9>(nativeWindow), shareHandle, d3dTexture,
                          backBufferFormat, depthBufferFormat, orientation);
}

egl::Error Renderer9::getD3DTextureInfo(const egl::Config *configuration,
                                        IUnknown *d3dTexture,
                                        const egl::AttributeMap &attribs,
                                        EGLint *width,
                                        EGLint *height,
                                        GLsizei *samples,
                                        gl::Format *glFormat,
                                        const angle::Format **angleFormat,
                                        UINT *arraySlice) const
{
    IDirect3DTexture9 *texture = nullptr;
    if (FAILED(d3dTexture->QueryInterface(&texture)))
    {
        return egl::EglBadParameter() << "Client buffer is not a IDirect3DTexture9";
    }

    IDirect3DDevice9 *textureDevice = nullptr;
    texture->GetDevice(&textureDevice);
    if (textureDevice != mDevice)
    {
        SafeRelease(texture);
        return egl::EglBadParameter() << "Texture's device does not match.";
    }
    SafeRelease(textureDevice);

    D3DSURFACE_DESC desc;
    texture->GetLevelDesc(0, &desc);
    SafeRelease(texture);

    if (width)
    {
        *width = static_cast<EGLint>(desc.Width);
    }
    if (height)
    {
        *height = static_cast<EGLint>(desc.Height);
    }

    // GetSamplesCount() returns 0 when multisampling isn't used.
    GLsizei sampleCount = d3d9_gl::GetSamplesCount(desc.MultiSampleType);
    if ((configuration && configuration->samples > 1) || sampleCount != 0)
    {
        return egl::EglBadParameter() << "Multisampling not supported for client buffer texture";
    }
    if (samples)
    {
        *samples = static_cast<EGLint>(sampleCount);
    }

    // From table egl.restrictions in EGL_ANGLE_d3d_texture_client_buffer.
    switch (desc.Format)
    {
        case D3DFMT_R8G8B8:
        case D3DFMT_A8R8G8B8:
        case D3DFMT_A16B16G16R16F:
        case D3DFMT_A32B32G32R32F:
            break;

        default:
            return egl::EglBadParameter()
                   << "Unknown client buffer texture format: " << desc.Format;
    }

    const auto &d3dFormatInfo = d3d9::GetD3DFormatInfo(desc.Format);
    ASSERT(d3dFormatInfo.info().id != angle::FormatID::NONE);

    if (glFormat)
    {
        *glFormat = gl::Format(d3dFormatInfo.info().glInternalFormat);
    }

    if (angleFormat)
    {

        *angleFormat = &d3dFormatInfo.info();
    }

    if (arraySlice)
    {
        *arraySlice = 0;
    }

    return egl::NoError();
}

egl::Error Renderer9::validateShareHandle(const egl::Config *config,
                                          HANDLE shareHandle,
                                          const egl::AttributeMap &attribs) const
{
    if (shareHandle == nullptr)
    {
        return egl::EglBadParameter() << "NULL share handle.";
    }

    EGLint width  = attribs.getAsInt(EGL_WIDTH, 0);
    EGLint height = attribs.getAsInt(EGL_HEIGHT, 0);
    ASSERT(width != 0 && height != 0);

    const d3d9::TextureFormat &backBufferd3dFormatInfo =
        d3d9::GetTextureFormatInfo(config->renderTargetFormat);

    IDirect3DTexture9 *texture = nullptr;
    HRESULT result             = mDevice->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET,
                                                        backBufferd3dFormatInfo.texFormat, D3DPOOL_DEFAULT,
                                                        &texture, &shareHandle);
    if (FAILED(result))
    {
        return egl::EglBadParameter() << "Failed to open share handle, " << gl::FmtHR(result);
    }

    DWORD levelCount = texture->GetLevelCount();

    D3DSURFACE_DESC desc;
    texture->GetLevelDesc(0, &desc);
    SafeRelease(texture);

    if (levelCount != 1 || desc.Width != static_cast<UINT>(width) ||
        desc.Height != static_cast<UINT>(height) ||
        desc.Format != backBufferd3dFormatInfo.texFormat)
    {
        return egl::EglBadParameter() << "Invalid texture parameters in share handle texture.";
    }

    return egl::NoError();
}

ContextImpl *Renderer9::createContext(const gl::State &state, gl::ErrorSet *errorSet)
{
    return new Context9(state, errorSet, this);
}

void *Renderer9::getD3DDevice()
{
    return mDevice;
}

angle::Result Renderer9::allocateEventQuery(const gl::Context *context, IDirect3DQuery9 **outQuery)
{
    if (mEventQueryPool.empty())
    {
        HRESULT result = mDevice->CreateQuery(D3DQUERYTYPE_EVENT, outQuery);
        ANGLE_TRY_HR(GetImplAs<Context9>(context), result, "Failed to allocate event query");
    }
    else
    {
        *outQuery = mEventQueryPool.back();
        mEventQueryPool.pop_back();
    }

    return angle::Result::Continue;
}

void Renderer9::freeEventQuery(IDirect3DQuery9 *query)
{
    if (mEventQueryPool.size() > 1000)
    {
        SafeRelease(query);
    }
    else
    {
        mEventQueryPool.push_back(query);
    }
}

angle::Result Renderer9::createVertexShader(d3d::Context *context,
                                            const DWORD *function,
                                            size_t length,
                                            IDirect3DVertexShader9 **outShader)
{
    return mVertexShaderCache.create(context, function, length, outShader);
}

angle::Result Renderer9::createPixelShader(d3d::Context *context,
                                           const DWORD *function,
                                           size_t length,
                                           IDirect3DPixelShader9 **outShader)
{
    return mPixelShaderCache.create(context, function, length, outShader);
}

HRESULT Renderer9::createVertexBuffer(UINT Length,
                                      DWORD Usage,
                                      IDirect3DVertexBuffer9 **ppVertexBuffer)
{
    // Force buffers to be limited to a fixed max size.
    if (Length > kMaximumBufferSizeHardLimit)
    {
        return E_OUTOFMEMORY;
    }

    D3DPOOL Pool = getBufferPool(Usage);
    return mDevice->CreateVertexBuffer(Length, Usage, 0, Pool, ppVertexBuffer, nullptr);
}

VertexBuffer *Renderer9::createVertexBuffer()
{
    return new VertexBuffer9(this);
}

HRESULT Renderer9::createIndexBuffer(UINT Length,
                                     DWORD Usage,
                                     D3DFORMAT Format,
                                     IDirect3DIndexBuffer9 **ppIndexBuffer)
{
    // Force buffers to be limited to a fixed max size.
    if (Length > kMaximumBufferSizeHardLimit)
    {
        return E_OUTOFMEMORY;
    }

    D3DPOOL Pool = getBufferPool(Usage);
    return mDevice->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, nullptr);
}

IndexBuffer *Renderer9::createIndexBuffer()
{
    return new IndexBuffer9(this);
}

StreamProducerImpl *Renderer9::createStreamProducerD3DTexture(
    egl::Stream::ConsumerType consumerType,
    const egl::AttributeMap &attribs)
{
    // Streams are not supported under D3D9
    UNREACHABLE();
    return nullptr;
}

bool Renderer9::supportsFastCopyBufferToTexture(GLenum internalFormat) const
{
    // Pixel buffer objects are not supported in D3D9, since D3D9 is ES2-only and PBOs are ES3.
    return false;
}

angle::Result Renderer9::fastCopyBufferToTexture(const gl::Context *context,
                                                 const gl::PixelUnpackState &unpack,
                                                 gl::Buffer *unpackBuffer,
                                                 unsigned int offset,
                                                 RenderTargetD3D *destRenderTarget,
                                                 GLenum destinationFormat,
                                                 GLenum sourcePixelsType,
                                                 const gl::Box &destArea)
{
    // Pixel buffer objects are not supported in D3D9, since D3D9 is ES2-only and PBOs are ES3.
    ANGLE_HR_UNREACHABLE(GetImplAs<Context9>(context));
    return angle::Result::Stop;
}

angle::Result Renderer9::setSamplerState(const gl::Context *context,
                                         gl::ShaderType type,
                                         int index,
                                         gl::Texture *texture,
                                         const gl::SamplerState &samplerState)
{
    CurSamplerState &appliedSampler = (type == gl::ShaderType::Fragment)
                                          ? mCurPixelSamplerStates[index]
                                          : mCurVertexSamplerStates[index];

    // Make sure to add the level offset for our tiny compressed texture workaround
    TextureD3D *textureD3D = GetImplAs<TextureD3D>(texture);

    TextureStorage *storage = nullptr;
    ANGLE_TRY(textureD3D->getNativeTexture(context, &storage));

    // Storage should exist, texture should be complete
    ASSERT(storage);

    DWORD baseLevel = texture->getBaseLevel() + storage->getTopLevel();

    if (appliedSampler.forceSet || appliedSampler.baseLevel != baseLevel ||
        memcmp(&samplerState, &appliedSampler, sizeof(gl::SamplerState)) != 0)
    {
        int d3dSamplerOffset = (type == gl::ShaderType::Fragment) ? 0 : D3DVERTEXTEXTURESAMPLER0;
        int d3dSampler       = index + d3dSamplerOffset;

        mDevice->SetSamplerState(d3dSampler, D3DSAMP_ADDRESSU,
                                 gl_d3d9::ConvertTextureWrap(samplerState.getWrapS()));
        mDevice->SetSamplerState(d3dSampler, D3DSAMP_ADDRESSV,
                                 gl_d3d9::ConvertTextureWrap(samplerState.getWrapT()));

        mDevice->SetSamplerState(d3dSampler, D3DSAMP_MAGFILTER,
                                 gl_d3d9::ConvertMagFilter(samplerState.getMagFilter(),
                                                           samplerState.getMaxAnisotropy()));

        D3DTEXTUREFILTERTYPE d3dMinFilter, d3dMipFilter;
        float lodBias;
        gl_d3d9::ConvertMinFilter(samplerState.getMinFilter(), &d3dMinFilter, &d3dMipFilter,
                                  &lodBias, samplerState.getMaxAnisotropy(), baseLevel);
        mDevice->SetSamplerState(d3dSampler, D3DSAMP_MINFILTER, d3dMinFilter);
        mDevice->SetSamplerState(d3dSampler, D3DSAMP_MIPFILTER, d3dMipFilter);
        mDevice->SetSamplerState(d3dSampler, D3DSAMP_MAXMIPLEVEL, baseLevel);
        mDevice->SetSamplerState(d3dSampler, D3DSAMP_MIPMAPLODBIAS, static_cast<DWORD>(lodBias));
        if (getNativeExtensions().textureFilterAnisotropicEXT)
        {
            DWORD maxAnisotropy = std::min(mDeviceCaps.MaxAnisotropy,
                                           static_cast<DWORD>(samplerState.getMaxAnisotropy()));
            mDevice->SetSamplerState(d3dSampler, D3DSAMP_MAXANISOTROPY, maxAnisotropy);
        }

        const gl::InternalFormat &info =
            gl::GetSizedInternalFormatInfo(textureD3D->getBaseLevelInternalFormat());

        mDevice->SetSamplerState(d3dSampler, D3DSAMP_SRGBTEXTURE, info.colorEncoding == GL_SRGB);

        if (samplerState.usesBorderColor())
        {
            angle::ColorGeneric borderColor = texture->getBorderColor();
            ASSERT(borderColor.type == angle::ColorGeneric::Type::Float);

            // Enforce opaque alpha for opaque formats, excluding DXT1 RGBA as it has no bits info.
            if (info.alphaBits == 0 && info.componentCount < 4)
            {
                borderColor.colorF.alpha = 1.0f;
            }

            if (info.isLUMA())
            {
                if (info.luminanceBits == 0)
                {
                    borderColor.colorF.red = 0.0f;
                }
                // Older Intel drivers use RGBA border color when sampling from D3DFMT_A8L8.
                // However, some recent Intel drivers sample alpha from green border channel
                // when using this format. Assume the old behavior because newer GPUs should
                // use D3D11 anyway.
                borderColor.colorF.green = borderColor.colorF.red;
                borderColor.colorF.blue  = borderColor.colorF.red;
            }

            D3DCOLOR d3dBorderColor;
            if (info.colorEncoding == GL_SRGB && getFeatures().borderColorSrgb.enabled)
            {
                d3dBorderColor =
                    D3DCOLOR_RGBA(gl::linearToSRGB(gl::clamp01(borderColor.colorF.red)),
                                  gl::linearToSRGB(gl::clamp01(borderColor.colorF.green)),
                                  gl::linearToSRGB(gl::clamp01(borderColor.colorF.blue)),
                                  gl::unorm<8>(borderColor.colorF.alpha));
            }
            else
            {
                d3dBorderColor = gl_d3d9::ConvertColor(borderColor.colorF);
            }

            mDevice->SetSamplerState(d3dSampler, D3DSAMP_BORDERCOLOR, d3dBorderColor);
        }
    }

    appliedSampler.forceSet     = false;
    appliedSampler.samplerState = samplerState;
    appliedSampler.baseLevel    = baseLevel;

    return angle::Result::Continue;
}

angle::Result Renderer9::setTexture(const gl::Context *context,
                                    gl::ShaderType type,
                                    int index,
                                    gl::Texture *texture)
{
    int d3dSamplerOffset = (type == gl::ShaderType::Fragment) ? 0 : D3DVERTEXTEXTURESAMPLER0;
    int d3dSampler       = index + d3dSamplerOffset;
    IDirect3DBaseTexture9 *d3dTexture = nullptr;
    bool forceSetTexture              = false;

    std::vector<uintptr_t> &appliedTextures =
        (type == gl::ShaderType::Fragment) ? mCurPixelTextures : mCurVertexTextures;

    if (texture)
    {
        TextureD3D *textureImpl = GetImplAs<TextureD3D>(texture);

        TextureStorage *texStorage = nullptr;
        ANGLE_TRY(textureImpl->getNativeTexture(context, &texStorage));

        // Texture should be complete and have a storage
        ASSERT(texStorage);

        TextureStorage9 *storage9 = GetAs<TextureStorage9>(texStorage);
        ANGLE_TRY(storage9->getBaseTexture(context, &d3dTexture));

        // If we get NULL back from getBaseTexture here, something went wrong
        // in the texture class and we're unexpectedly missing the d3d texture
        ASSERT(d3dTexture != nullptr);

        forceSetTexture = textureImpl->hasDirtyImages();
        textureImpl->resetDirty();
    }

    if (forceSetTexture || appliedTextures[index] != reinterpret_cast<uintptr_t>(d3dTexture))
    {
        mDevice->SetTexture(d3dSampler, d3dTexture);
    }

    appliedTextures[index] = reinterpret_cast<uintptr_t>(d3dTexture);

    return angle::Result::Continue;
}

angle::Result Renderer9::updateState(const gl::Context *context, gl::PrimitiveMode drawMode)
{
    const auto &glState = context->getState();

    // Applies the render target surface, depth stencil surface, viewport rectangle and
    // scissor rectangle to the renderer
    gl::Framebuffer *framebuffer = glState.getDrawFramebuffer();
    ASSERT(framebuffer && !framebuffer->hasAnyDirtyBit());

    Framebuffer9 *framebuffer9 = GetImplAs<Framebuffer9>(framebuffer);

    ANGLE_TRY(applyRenderTarget(context, framebuffer9->getCachedColorRenderTargets()[0],
                                framebuffer9->getCachedDepthStencilRenderTarget()));

    // Setting viewport state
    setViewport(glState.getViewport(), glState.getNearPlane(), glState.getFarPlane(), drawMode,
                glState.getRasterizerState().frontFace, false);

    // Setting scissors state
    setScissorRectangle(glState.getScissor(), glState.isScissorTestEnabled());

    // Setting blend, depth stencil, and rasterizer states
    // Since framebuffer->getSamples will return the original samples which may be different with
    // the sample counts that we set in render target view, here we use renderTarget->getSamples to
    // get the actual samples.
    GLsizei samples                                       = 0;
    const gl::FramebufferAttachment *firstColorAttachment = framebuffer->getFirstColorAttachment();
    if (firstColorAttachment)
    {
        ASSERT(firstColorAttachment->isAttached());
        RenderTarget9 *renderTarget = nullptr;
        ANGLE_TRY(firstColorAttachment->getRenderTarget(context, firstColorAttachment->getSamples(),
                                                        &renderTarget));
        samples = renderTarget->getSamples();

        mDevice->SetRenderState(D3DRS_SRGBWRITEENABLE,
                                renderTarget->getInternalFormat() == GL_SRGB8_ALPHA8);
    }
    gl::RasterizerState rasterizer = glState.getRasterizerState();
    rasterizer.pointDrawMode       = (drawMode == gl::PrimitiveMode::Points);
    rasterizer.multiSample         = (samples != 0);

    ANGLE_TRY(setBlendDepthRasterStates(context, drawMode));

    mStateManager.resetDirtyBits();

    return angle::Result::Continue;
}

void Renderer9::setScissorRectangle(const gl::Rectangle &scissor, bool enabled)
{
    mStateManager.setScissorState(scissor, enabled);
}

angle::Result Renderer9::setBlendDepthRasterStates(const gl::Context *context,
                                                   gl::PrimitiveMode drawMode)
{
    const auto &glState              = context->getState();
    gl::Framebuffer *drawFramebuffer = glState.getDrawFramebuffer();
    ASSERT(!drawFramebuffer->hasAnyDirtyBit());
    // Since framebuffer->getSamples will return the original samples which may be different with
    // the sample counts that we set in render target view, here we use renderTarget->getSamples to
    // get the actual samples.
    GLsizei samples = 0;
    const gl::FramebufferAttachment *firstColorAttachment =
        drawFramebuffer->getFirstColorAttachment();
    if (firstColorAttachment)
    {
        ASSERT(firstColorAttachment->isAttached());
        RenderTarget9 *renderTarget = nullptr;
        ANGLE_TRY(firstColorAttachment->getRenderTarget(context, firstColorAttachment->getSamples(),
                                                        &renderTarget));
        samples = renderTarget->getSamples();
    }
    gl::RasterizerState rasterizer = glState.getRasterizerState();
    rasterizer.pointDrawMode       = (drawMode == gl::PrimitiveMode::Points);
    rasterizer.multiSample         = (samples != 0);

    unsigned int mask = GetBlendSampleMask(glState, samples);
    mStateManager.setBlendDepthRasterStates(glState, mask);
    return angle::Result::Continue;
}

void Renderer9::setViewport(const gl::Rectangle &viewport,
                            float zNear,
                            float zFar,
                            gl::PrimitiveMode drawMode,
                            GLenum frontFace,
                            bool ignoreViewport)
{
    mStateManager.setViewportState(viewport, zNear, zFar, drawMode, frontFace, ignoreViewport);
}

bool Renderer9::applyPrimitiveType(gl::PrimitiveMode mode, GLsizei count, bool usesPointSize)
{
    switch (mode)
    {
        case gl::PrimitiveMode::Points:
            mPrimitiveType  = D3DPT_POINTLIST;
            mPrimitiveCount = count;
            break;
        case gl::PrimitiveMode::Lines:
            mPrimitiveType  = D3DPT_LINELIST;
            mPrimitiveCount = count / 2;
            break;
        case gl::PrimitiveMode::LineLoop:
            mPrimitiveType = D3DPT_LINESTRIP;
            mPrimitiveCount =
                count - 1;  // D3D doesn't support line loops, so we draw the last line separately
            break;
        case gl::PrimitiveMode::LineStrip:
            mPrimitiveType  = D3DPT_LINESTRIP;
            mPrimitiveCount = count - 1;
            break;
        case gl::PrimitiveMode::Triangles:
            mPrimitiveType  = D3DPT_TRIANGLELIST;
            mPrimitiveCount = count / 3;
            break;
        case gl::PrimitiveMode::TriangleStrip:
            mPrimitiveType  = D3DPT_TRIANGLESTRIP;
            mPrimitiveCount = count - 2;
            break;
        case gl::PrimitiveMode::TriangleFan:
            mPrimitiveType  = D3DPT_TRIANGLEFAN;
            mPrimitiveCount = count - 2;
            break;
        default:
            UNREACHABLE();
            return false;
    }

    return mPrimitiveCount > 0;
}

angle::Result Renderer9::getNullColorRenderTarget(const gl::Context *context,
                                                  const RenderTarget9 *depthRenderTarget,
                                                  const RenderTarget9 **outColorRenderTarget)
{
    ASSERT(depthRenderTarget);

    const gl::Extents &size = depthRenderTarget->getExtents();

    // search cached nullcolorbuffers
    for (int i = 0; i < NUM_NULL_COLORBUFFER_CACHE_ENTRIES; i++)
    {
        if (mNullRenderTargetCache[i].renderTarget != nullptr &&
            mNullRenderTargetCache[i].width == size.width &&
            mNullRenderTargetCache[i].height == size.height)
        {
            mNullRenderTargetCache[i].lruCount = ++mMaxNullColorbufferLRU;
            *outColorRenderTarget              = mNullRenderTargetCache[i].renderTarget;
            return angle::Result::Continue;
        }
    }

    RenderTargetD3D *nullRenderTarget = nullptr;
    ANGLE_TRY(createRenderTarget(context, size.width, size.height, GL_NONE, 0, &nullRenderTarget));

    // add nullbuffer to the cache
    NullRenderTargetCacheEntry *oldest = &mNullRenderTargetCache[0];
    for (int i = 1; i < NUM_NULL_COLORBUFFER_CACHE_ENTRIES; i++)
    {
        if (mNullRenderTargetCache[i].lruCount < oldest->lruCount)
        {
            oldest = &mNullRenderTargetCache[i];
        }
    }

    SafeDelete(oldest->renderTarget);
    oldest->renderTarget = GetAs<RenderTarget9>(nullRenderTarget);
    oldest->lruCount     = ++mMaxNullColorbufferLRU;
    oldest->width        = size.width;
    oldest->height       = size.height;

    *outColorRenderTarget = oldest->renderTarget;
    return angle::Result::Continue;
}

angle::Result Renderer9::applyRenderTarget(const gl::Context *context,
                                           const RenderTarget9 *colorRenderTargetIn,
                                           const RenderTarget9 *depthStencilRenderTarget)
{
    // if there is no color attachment we must synthesize a NULL colorattachment
    // to keep the D3D runtime happy.  This should only be possible if depth texturing.
    const RenderTarget9 *colorRenderTarget = colorRenderTargetIn;
    if (colorRenderTarget == nullptr)
    {
        ANGLE_TRY(getNullColorRenderTarget(context, depthStencilRenderTarget, &colorRenderTarget));
    }
    ASSERT(colorRenderTarget != nullptr);

    size_t renderTargetWidth  = 0;
    size_t renderTargetHeight = 0;

    bool renderTargetChanged        = false;
    unsigned int renderTargetSerial = colorRenderTarget->getSerial();
    if (renderTargetSerial != mAppliedRenderTargetSerial)
    {
        // Apply the render target on the device
        IDirect3DSurface9 *renderTargetSurface = colorRenderTarget->getSurface();
        ASSERT(renderTargetSurface);

        mDevice->SetRenderTarget(0, renderTargetSurface);
        SafeRelease(renderTargetSurface);

        renderTargetWidth  = colorRenderTarget->getWidth();
        renderTargetHeight = colorRenderTarget->getHeight();

        mAppliedRenderTargetSerial = renderTargetSerial;
        renderTargetChanged        = true;
    }

    unsigned int depthStencilSerial = 0;
    if (depthStencilRenderTarget != nullptr)
    {
        depthStencilSerial = depthStencilRenderTarget->getSerial();
    }

    if (depthStencilSerial != mAppliedDepthStencilSerial || !mDepthStencilInitialized)
    {
        unsigned int depthSize   = 0;
        unsigned int stencilSize = 0;

        // Apply the depth stencil on the device
        if (depthStencilRenderTarget)
        {
            IDirect3DSurface9 *depthStencilSurface = depthStencilRenderTarget->getSurface();
            ASSERT(depthStencilSurface);

            mDevice->SetDepthStencilSurface(depthStencilSurface);
            SafeRelease(depthStencilSurface);

            const gl::InternalFormat &format =
                gl::GetSizedInternalFormatInfo(depthStencilRenderTarget->getInternalFormat());

            depthSize   = format.depthBits;
            stencilSize = format.stencilBits;
        }
        else
        {
            mDevice->SetDepthStencilSurface(nullptr);
        }

        mStateManager.updateDepthSizeIfChanged(mDepthStencilInitialized, depthSize);
        mStateManager.updateStencilSizeIfChanged(mDepthStencilInitialized, stencilSize);

        mAppliedDepthStencilSerial = depthStencilSerial;
        mDepthStencilInitialized   = true;
    }

    if (renderTargetChanged || !mRenderTargetDescInitialized)
    {
        mStateManager.forceSetBlendState();
        mStateManager.forceSetScissorState();
        mStateManager.setRenderTargetBounds(renderTargetWidth, renderTargetHeight);
        mRenderTargetDescInitialized = true;
    }

    return angle::Result::Continue;
}

angle::Result Renderer9::applyVertexBuffer(const gl::Context *context,
                                           gl::PrimitiveMode mode,
                                           GLint first,
                                           GLsizei count,
                                           GLsizei instances,
                                           TranslatedIndexData * /*indexInfo*/)
{
    const gl::State &state = context->getState();
    ANGLE_TRY(mVertexDataManager->prepareVertexData(context, first, count, &mTranslatedAttribCache,
                                                    instances));

    return mVertexDeclarationCache.applyDeclaration(context, mDevice, mTranslatedAttribCache,
                                                    state.getProgramExecutable(), first, instances,
                                                    &mRepeatDraw);
}

// Applies the indices and element array bindings to the Direct3D 9 device
angle::Result Renderer9::applyIndexBuffer(const gl::Context *context,
                                          const void *indices,
                                          GLsizei count,
                                          gl::PrimitiveMode mode,
                                          gl::DrawElementsType type,
                                          TranslatedIndexData *indexInfo)
{
    gl::VertexArray *vao           = context->getState().getVertexArray();
    gl::Buffer *elementArrayBuffer = vao->getElementArrayBuffer();

    gl::DrawElementsType dstType = gl::DrawElementsType::InvalidEnum;
    ANGLE_TRY(GetIndexTranslationDestType(context, count, type, indices, false, &dstType));

    ANGLE_TRY(mIndexDataManager->prepareIndexData(context, type, dstType, count, elementArrayBuffer,
                                                  indices, indexInfo));

    // Directly binding the storage buffer is not supported for d3d9
    ASSERT(indexInfo->storage == nullptr);

    if (indexInfo->serial != mAppliedIBSerial)
    {
        IndexBuffer9 *indexBuffer = GetAs<IndexBuffer9>(indexInfo->indexBuffer);

        mDevice->SetIndices(indexBuffer->getBuffer());
        mAppliedIBSerial = indexInfo->serial;
    }

    return angle::Result::Continue;
}

angle::Result Renderer9::drawArraysImpl(const gl::Context *context,
                                        gl::PrimitiveMode mode,
                                        GLint startVertex,
                                        GLsizei count,
                                        GLsizei instances)
{
    ASSERT(!context->getState().isTransformFeedbackActiveUnpaused());

    startScene();

    if (mode == gl::PrimitiveMode::LineLoop)
    {
        return drawLineLoop(context, count, gl::DrawElementsType::InvalidEnum, nullptr, 0, nullptr);
    }

    if (instances > 0)
    {
        StaticIndexBufferInterface *countingIB = nullptr;
        ANGLE_TRY(getCountingIB(context, count, &countingIB));

        if (mAppliedIBSerial != countingIB->getSerial())
        {
            IndexBuffer9 *indexBuffer = GetAs<IndexBuffer9>(countingIB->getIndexBuffer());

            mDevice->SetIndices(indexBuffer->getBuffer());
            mAppliedIBSerial = countingIB->getSerial();
        }

        for (int i = 0; i < mRepeatDraw; i++)
        {
            mDevice->DrawIndexedPrimitive(mPrimitiveType, 0, 0, count, 0, mPrimitiveCount);
        }

        return angle::Result::Continue;
    }

    // Regular case
    mDevice->DrawPrimitive(mPrimitiveType, 0, mPrimitiveCount);
    return angle::Result::Continue;
}

angle::Result Renderer9::drawElementsImpl(const gl::Context *context,
                                          gl::PrimitiveMode mode,
                                          GLsizei count,
                                          gl::DrawElementsType type,
                                          const void *indices,
                                          GLsizei instances)
{
    TranslatedIndexData indexInfo;

    ANGLE_TRY(applyIndexBuffer(context, indices, count, mode, type, &indexInfo));

    gl::IndexRange indexRange;
    ANGLE_TRY(context->getState().getVertexArray()->getIndexRange(context, type, count, indices,
                                                                  &indexRange));

    size_t vertexCount = indexRange.vertexCount();
    ANGLE_TRY(applyVertexBuffer(context, mode, static_cast<GLsizei>(indexRange.start),
                                static_cast<GLsizei>(vertexCount), instances, &indexInfo));

    startScene();

    int minIndex = static_cast<int>(indexRange.start);

    gl::VertexArray *vao           = context->getState().getVertexArray();
    gl::Buffer *elementArrayBuffer = vao->getElementArrayBuffer();

    if (mode == gl::PrimitiveMode::Points)
    {
        return drawIndexedPoints(context, count, type, indices, minIndex, elementArrayBuffer);
    }

    if (mode == gl::PrimitiveMode::LineLoop)
    {
        return drawLineLoop(context, count, type, indices, minIndex, elementArrayBuffer);
    }

    for (int i = 0; i < mRepeatDraw; i++)
    {
        mDevice->DrawIndexedPrimitive(mPrimitiveType, -minIndex, minIndex,
                                      static_cast<UINT>(vertexCount), indexInfo.startIndex,
                                      mPrimitiveCount);
    }
    return angle::Result::Continue;
}

angle::Result Renderer9::drawLineLoop(const gl::Context *context,
                                      GLsizei count,
                                      gl::DrawElementsType type,
                                      const void *indices,
                                      int minIndex,
                                      gl::Buffer *elementArrayBuffer)
{
    // Get the raw indices for an indexed draw
    if (type != gl::DrawElementsType::InvalidEnum && elementArrayBuffer)
    {
        BufferD3D *storage        = GetImplAs<BufferD3D>(elementArrayBuffer);
        intptr_t offset           = reinterpret_cast<intptr_t>(indices);
        const uint8_t *bufferData = nullptr;
        ANGLE_TRY(storage->getData(context, &bufferData));
        indices = bufferData + offset;
    }

    unsigned int startIndex = 0;
    Context9 *context9      = GetImplAs<Context9>(context);

    if (getNativeExtensions().elementIndexUintOES)
    {
        if (!mLineLoopIB)
        {
            mLineLoopIB = new StreamingIndexBufferInterface(this);
            ANGLE_TRY(mLineLoopIB->reserveBufferSpace(context, INITIAL_INDEX_BUFFER_SIZE,
                                                      gl::DrawElementsType::UnsignedInt));
        }

        // Checked by Renderer9::applyPrimitiveType
        ASSERT(count >= 0);

        ANGLE_CHECK(context9,
                    static_cast<unsigned int>(count) + 1 <=
                        (std::numeric_limits<unsigned int>::max() / sizeof(unsigned int)),
                    "Failed to create a 32-bit looping index buffer for "
                    "GL_LINE_LOOP, too many indices required.",
                    GL_OUT_OF_MEMORY);

        const unsigned int spaceNeeded =
            (static_cast<unsigned int>(count) + 1) * sizeof(unsigned int);
        ANGLE_TRY(mLineLoopIB->reserveBufferSpace(context, spaceNeeded,
                                                  gl::DrawElementsType::UnsignedInt));

        void *mappedMemory  = nullptr;
        unsigned int offset = 0;
        ANGLE_TRY(mLineLoopIB->mapBuffer(context, spaceNeeded, &mappedMemory, &offset));

        startIndex         = static_cast<unsigned int>(offset) / 4;
        unsigned int *data = static_cast<unsigned int *>(mappedMemory);

        switch (type)
        {
            case gl::DrawElementsType::InvalidEnum:  // Non-indexed draw
                for (int i = 0; i < count; i++)
                {
                    data[i] = i;
                }
                data[count] = 0;
                break;
            case gl::DrawElementsType::UnsignedByte:
                for (int i = 0; i < count; i++)
                {
                    data[i] = static_cast<const GLubyte *>(indices)[i];
                }
                data[count] = static_cast<const GLubyte *>(indices)[0];
                break;
            case gl::DrawElementsType::UnsignedShort:
                for (int i = 0; i < count; i++)
                {
                    data[i] = static_cast<const GLushort *>(indices)[i];
                }
                data[count] = static_cast<const GLushort *>(indices)[0];
                break;
            case gl::DrawElementsType::UnsignedInt:
                for (int i = 0; i < count; i++)
                {
                    data[i] = static_cast<const GLuint *>(indices)[i];
                }
                data[count] = static_cast<const GLuint *>(indices)[0];
                break;
            default:
                UNREACHABLE();
        }

        ANGLE_TRY(mLineLoopIB->unmapBuffer(context));
    }
    else
    {
        if (!mLineLoopIB)
        {
            mLineLoopIB = new StreamingIndexBufferInterface(this);
            ANGLE_TRY(mLineLoopIB->reserveBufferSpace(context, INITIAL_INDEX_BUFFER_SIZE,
                                                      gl::DrawElementsType::UnsignedShort));
        }

        // Checked by Renderer9::applyPrimitiveType
        ASSERT(count >= 0);

        ANGLE_CHECK(context9,
                    static_cast<unsigned int>(count) + 1 <=
                        (std::numeric_limits<unsigned short>::max() / sizeof(unsigned short)),
                    "Failed to create a 16-bit looping index buffer for "
                    "GL_LINE_LOOP, too many indices required.",
                    GL_OUT_OF_MEMORY);

        const unsigned int spaceNeeded =
            (static_cast<unsigned int>(count) + 1) * sizeof(unsigned short);
        ANGLE_TRY(mLineLoopIB->reserveBufferSpace(context, spaceNeeded,
                                                  gl::DrawElementsType::UnsignedShort));

        void *mappedMemory = nullptr;
        unsigned int offset;
        ANGLE_TRY(mLineLoopIB->mapBuffer(context, spaceNeeded, &mappedMemory, &offset));

        startIndex           = static_cast<unsigned int>(offset) / 2;
        unsigned short *data = static_cast<unsigned short *>(mappedMemory);

        switch (type)
        {
            case gl::DrawElementsType::InvalidEnum:  // Non-indexed draw
                for (int i = 0; i < count; i++)
                {
                    data[i] = static_cast<unsigned short>(i);
                }
                data[count] = 0;
                break;
            case gl::DrawElementsType::UnsignedByte:
                for (int i = 0; i < count; i++)
                {
                    data[i] = static_cast<const GLubyte *>(indices)[i];
                }
                data[count] = static_cast<const GLubyte *>(indices)[0];
                break;
            case gl::DrawElementsType::UnsignedShort:
                for (int i = 0; i < count; i++)
                {
                    data[i] = static_cast<const GLushort *>(indices)[i];
                }
                data[count] = static_cast<const GLushort *>(indices)[0];
                break;
            case gl::DrawElementsType::UnsignedInt:
                for (int i = 0; i < count; i++)
                {
                    data[i] = static_cast<unsigned short>(static_cast<const GLuint *>(indices)[i]);
                }
                data[count] = static_cast<unsigned short>(static_cast<const GLuint *>(indices)[0]);
                break;
            default:
                UNREACHABLE();
        }

        ANGLE_TRY(mLineLoopIB->unmapBuffer(context));
    }

    if (mAppliedIBSerial != mLineLoopIB->getSerial())
    {
        IndexBuffer9 *indexBuffer = GetAs<IndexBuffer9>(mLineLoopIB->getIndexBuffer());

        mDevice->SetIndices(indexBuffer->getBuffer());
        mAppliedIBSerial = mLineLoopIB->getSerial();
    }

    mDevice->DrawIndexedPrimitive(D3DPT_LINESTRIP, -minIndex, minIndex, count, startIndex, count);

    return angle::Result::Continue;
}

angle::Result Renderer9::drawIndexedPoints(const gl::Context *context,
                                           GLsizei count,
                                           gl::DrawElementsType type,
                                           const void *indices,
                                           int minIndex,
                                           gl::Buffer *elementArrayBuffer)
{
    // Drawing index point lists is unsupported in d3d9, fall back to a regular DrawPrimitive call
    // for each individual point. This call is not expected to happen often.

    if (elementArrayBuffer)
    {
        BufferD3D *storage = GetImplAs<BufferD3D>(elementArrayBuffer);
        intptr_t offset    = reinterpret_cast<intptr_t>(indices);

        const uint8_t *bufferData = nullptr;
        ANGLE_TRY(storage->getData(context, &bufferData));
        indices = bufferData + offset;
    }

    switch (type)
    {
        case gl::DrawElementsType::UnsignedByte:
            DrawPoints<GLubyte>(mDevice, count, indices, minIndex);
            return angle::Result::Continue;
        case gl::DrawElementsType::UnsignedShort:
            DrawPoints<GLushort>(mDevice, count, indices, minIndex);
            return angle::Result::Continue;
        case gl::DrawElementsType::UnsignedInt:
            DrawPoints<GLuint>(mDevice, count, indices, minIndex);
            return angle::Result::Continue;
        default:
            ANGLE_HR_UNREACHABLE(GetImplAs<Context9>(context));
    }
}

angle::Result Renderer9::getCountingIB(const gl::Context *context,
                                       size_t count,
                                       StaticIndexBufferInterface **outIB)
{
    // Update the counting index buffer if it is not large enough or has not been created yet.
    if (count <= 65536)  // 16-bit indices
    {
        const unsigned int spaceNeeded = static_cast<unsigned int>(count) * sizeof(unsigned short);

        if (!mCountingIB || mCountingIB->getBufferSize() < spaceNeeded)
        {
            SafeDelete(mCountingIB);
            mCountingIB = new StaticIndexBufferInterface(this);
            ANGLE_TRY(mCountingIB->reserveBufferSpace(context, spaceNeeded,
                                                      gl::DrawElementsType::UnsignedShort));

            void *mappedMemory = nullptr;
            ANGLE_TRY(mCountingIB->mapBuffer(context, spaceNeeded, &mappedMemory, nullptr));

            unsigned short *data = static_cast<unsigned short *>(mappedMemory);
            for (size_t i = 0; i < count; i++)
            {
                data[i] = static_cast<unsigned short>(i);
            }

            ANGLE_TRY(mCountingIB->unmapBuffer(context));
        }
    }
    else if (getNativeExtensions().elementIndexUintOES)
    {
        const unsigned int spaceNeeded = static_cast<unsigned int>(count) * sizeof(unsigned int);

        if (!mCountingIB || mCountingIB->getBufferSize() < spaceNeeded)
        {
            SafeDelete(mCountingIB);
            mCountingIB = new StaticIndexBufferInterface(this);
            ANGLE_TRY(mCountingIB->reserveBufferSpace(context, spaceNeeded,
                                                      gl::DrawElementsType::UnsignedInt));

            void *mappedMemory = nullptr;
            ANGLE_TRY(mCountingIB->mapBuffer(context, spaceNeeded, &mappedMemory, nullptr));

            unsigned int *data = static_cast<unsigned int *>(mappedMemory);
            for (unsigned int i = 0; i < count; i++)
            {
                data[i] = i;
            }

            ANGLE_TRY(mCountingIB->unmapBuffer(context));
        }
    }
    else
    {
        ANGLE_TRY_HR(GetImplAs<Context9>(context), E_OUTOFMEMORY,
                     "Could not create a counting index buffer for glDrawArraysInstanced.");
    }

    *outIB = mCountingIB;
    return angle::Result::Continue;
}

angle::Result Renderer9::applyShaders(const gl::Context *context, gl::PrimitiveMode drawMode)
{
    const gl::State &state = context->getState();
    Context9 *context9     = GetImplAs<Context9>(context);
    RendererD3D *renderer  = context9->getRenderer();

    // This method is called single-threaded.
    ANGLE_TRY(ensureHLSLCompilerInitialized(context9));

    ProgramExecutableD3D *executableD3D =
        GetImplAs<ProgramExecutableD3D>(state.getProgramExecutable());
    VertexArray9 *vao = GetImplAs<VertexArray9>(state.getVertexArray());
    executableD3D->updateCachedInputLayout(renderer, vao->getCurrentStateSerial(), state);

    ShaderExecutableD3D *vertexExe = nullptr;
    ANGLE_TRY(executableD3D->getVertexExecutableForCachedInputLayout(context9, renderer, &vertexExe,
                                                                     nullptr));

    const gl::Framebuffer *drawFramebuffer = state.getDrawFramebuffer();
    executableD3D->updateCachedOutputLayout(context, drawFramebuffer);

    ShaderExecutableD3D *pixelExe = nullptr;
    ANGLE_TRY(executableD3D->getPixelExecutableForCachedOutputLayout(context9, renderer, &pixelExe,
                                                                     nullptr));

    IDirect3DVertexShader9 *vertexShader =
        (vertexExe ? GetAs<ShaderExecutable9>(vertexExe)->getVertexShader() : nullptr);
    IDirect3DPixelShader9 *pixelShader =
        (pixelExe ? GetAs<ShaderExecutable9>(pixelExe)->getPixelShader() : nullptr);

    if (vertexShader != mAppliedVertexShader)
    {
        mDevice->SetVertexShader(vertexShader);
        mAppliedVertexShader = vertexShader;
    }

    if (pixelShader != mAppliedPixelShader)
    {
        mDevice->SetPixelShader(pixelShader);
        mAppliedPixelShader = pixelShader;
    }

    // D3D9 has a quirk where creating multiple shaders with the same content
    // can return the same shader pointer. Because GL programs store different data
    // per-program, checking the program serial guarantees we upload fresh
    // uniform data even if our shader pointers are the same.
    // https://code.google.com/p/angleproject/issues/detail?id=661
    unsigned int programSerial = executableD3D->getSerial();
    if (programSerial != mAppliedProgramSerial)
    {
        executableD3D->dirtyAllUniforms();
        mStateManager.forceSetDXUniformsState();
        mAppliedProgramSerial = programSerial;
    }

    applyUniforms(executableD3D);

    // Driver uniforms
    mStateManager.setShaderConstants();

    return angle::Result::Continue;
}

void Renderer9::applyUniforms(ProgramExecutableD3D *executableD3D)
{
    // Skip updates if we're not dirty. Note that D3D9 cannot have compute or geometry.
    if (!executableD3D->anyShaderUniformsDirty())
    {
        return;
    }

    const auto &uniformArray = executableD3D->getD3DUniforms();

    for (const D3DUniform *targetUniform : uniformArray)
    {
        // Built-in uniforms must be skipped.
        if (!targetUniform->isReferencedByShader(gl::ShaderType::Vertex) &&
            !targetUniform->isReferencedByShader(gl::ShaderType::Fragment))
            continue;

        const GLfloat *f = reinterpret_cast<const GLfloat *>(targetUniform->firstNonNullData());
        const GLint *i   = reinterpret_cast<const GLint *>(targetUniform->firstNonNullData());

        switch (targetUniform->typeInfo.type)
        {
            case GL_SAMPLER_2D:
            case GL_SAMPLER_CUBE:
            case GL_SAMPLER_EXTERNAL_OES:
            case GL_SAMPLER_VIDEO_IMAGE_WEBGL:
                break;
            case GL_BOOL:
            case GL_BOOL_VEC2:
            case GL_BOOL_VEC3:
            case GL_BOOL_VEC4:
                applyUniformnbv(targetUniform, i);
                break;
            case GL_FLOAT:
            case GL_FLOAT_VEC2:
            case GL_FLOAT_VEC3:
            case GL_FLOAT_VEC4:
            case GL_FLOAT_MAT2:
            case GL_FLOAT_MAT3:
            case GL_FLOAT_MAT4:
                applyUniformnfv(targetUniform, f);
                break;
            case GL_INT:
            case GL_INT_VEC2:
            case GL_INT_VEC3:
            case GL_INT_VEC4:
                applyUniformniv(targetUniform, i);
                break;
            default:
                UNREACHABLE();
        }
    }

    executableD3D->markUniformsClean();
}

void Renderer9::applyUniformnfv(const D3DUniform *targetUniform, const GLfloat *v)
{
    if (targetUniform->isReferencedByShader(gl::ShaderType::Fragment))
    {
        mDevice->SetPixelShaderConstantF(
            targetUniform->mShaderRegisterIndexes[gl::ShaderType::Fragment], v,
            targetUniform->registerCount);
    }

    if (targetUniform->isReferencedByShader(gl::ShaderType::Vertex))
    {
        mDevice->SetVertexShaderConstantF(
            targetUniform->mShaderRegisterIndexes[gl::ShaderType::Vertex], v,
            targetUniform->registerCount);
    }
}

void Renderer9::applyUniformniv(const D3DUniform *targetUniform, const GLint *v)
{
    ASSERT(targetUniform->registerCount <= MAX_VERTEX_CONSTANT_VECTORS_D3D9);
    GLfloat vector[MAX_VERTEX_CONSTANT_VECTORS_D3D9][4];

    for (unsigned int i = 0; i < targetUniform->registerCount; i++)
    {
        vector[i][0] = (GLfloat)v[4 * i + 0];
        vector[i][1] = (GLfloat)v[4 * i + 1];
        vector[i][2] = (GLfloat)v[4 * i + 2];
        vector[i][3] = (GLfloat)v[4 * i + 3];
    }

    applyUniformnfv(targetUniform, (GLfloat *)vector);
}

void Renderer9::applyUniformnbv(const D3DUniform *targetUniform, const GLint *v)
{
    ASSERT(targetUniform->registerCount <= MAX_VERTEX_CONSTANT_VECTORS_D3D9);
    GLfloat vector[MAX_VERTEX_CONSTANT_VECTORS_D3D9][4];

    for (unsigned int i = 0; i < targetUniform->registerCount; i++)
    {
        vector[i][0] = (v[4 * i + 0] == GL_FALSE) ? 0.0f : 1.0f;
        vector[i][1] = (v[4 * i + 1] == GL_FALSE) ? 0.0f : 1.0f;
        vector[i][2] = (v[4 * i + 2] == GL_FALSE) ? 0.0f : 1.0f;
        vector[i][3] = (v[4 * i + 3] == GL_FALSE) ? 0.0f : 1.0f;
    }

    applyUniformnfv(targetUniform, (GLfloat *)vector);
}

void Renderer9::clear(const ClearParameters &clearParams,
                      const RenderTarget9 *colorRenderTarget,
                      const RenderTarget9 *depthStencilRenderTarget)
{
    // Clearing buffers with non-float values is not supported by Renderer9 and ES 2.0
    ASSERT(clearParams.colorType == GL_FLOAT);

    // Clearing individual buffers other than buffer zero is not supported by Renderer9 and ES 2.0
    bool clearColor = clearParams.clearColor[0];
    for (unsigned int i = 0; i < clearParams.clearColor.size(); i++)
    {
        ASSERT(clearParams.clearColor[i] == clearColor);
    }

    float depth   = gl::clamp01(clearParams.depthValue);
    DWORD stencil = clearParams.stencilValue & 0x000000FF;

    unsigned int stencilUnmasked = 0x0;
    if (clearParams.clearStencil && depthStencilRenderTarget)
    {
        const gl::InternalFormat &depthStencilFormat =
            gl::GetSizedInternalFormatInfo(depthStencilRenderTarget->getInternalFormat());
        if (depthStencilFormat.stencilBits > 0)
        {
            const d3d9::D3DFormat &d3dFormatInfo =
                d3d9::GetD3DFormatInfo(depthStencilRenderTarget->getD3DFormat());
            stencilUnmasked = (0x1 << d3dFormatInfo.stencilBits) - 1;
        }
    }

    const bool needMaskedStencilClear =
        clearParams.clearStencil &&
        (clearParams.stencilWriteMask & stencilUnmasked) != stencilUnmasked;

    bool needMaskedColorClear = false;
    D3DCOLOR color            = D3DCOLOR_ARGB(255, 0, 0, 0);
    if (clearColor)
    {
        ASSERT(colorRenderTarget != nullptr);

        const gl::InternalFormat &formatInfo =
            gl::GetSizedInternalFormatInfo(colorRenderTarget->getInternalFormat());
        const d3d9::D3DFormat &d3dFormatInfo =
            d3d9::GetD3DFormatInfo(colorRenderTarget->getD3DFormat());

        color =
            D3DCOLOR_ARGB(gl::unorm<8>((formatInfo.alphaBits == 0 && d3dFormatInfo.alphaBits > 0)
                                           ? 1.0f
                                           : clearParams.colorF.alpha),
                          gl::unorm<8>((formatInfo.redBits == 0 && d3dFormatInfo.redBits > 0)
                                           ? 0.0f
                                           : clearParams.colorF.red),
                          gl::unorm<8>((formatInfo.greenBits == 0 && d3dFormatInfo.greenBits > 0)
                                           ? 0.0f
                                           : clearParams.colorF.green),
                          gl::unorm<8>((formatInfo.blueBits == 0 && d3dFormatInfo.blueBits > 0)
                                           ? 0.0f
                                           : clearParams.colorF.blue));

        const uint8_t colorMask =
            gl::BlendStateExt::ColorMaskStorage::GetValueIndexed(0, clearParams.colorMask);
        bool r, g, b, a;
        gl::BlendStateExt::UnpackColorMask(colorMask, &r, &g, &b, &a);
        if ((formatInfo.redBits > 0 && !r) || (formatInfo.greenBits > 0 && !g) ||
            (formatInfo.blueBits > 0 && !b) || (formatInfo.alphaBits > 0 && !a))
        {
            needMaskedColorClear = true;
        }
    }

    if (needMaskedColorClear || needMaskedStencilClear)
    {
        // State which is altered in all paths from this point to the clear call is saved.
        // State which is altered in only some paths will be flagged dirty in the case that
        //  that path is taken.
        HRESULT hr;
        if (mMaskedClearSavedState == nullptr)
        {
            hr = mDevice->BeginStateBlock();
            ASSERT(SUCCEEDED(hr) || hr == D3DERR_OUTOFVIDEOMEMORY || hr == E_OUTOFMEMORY);

            mDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
            mDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
            mDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
            mDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
            mDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
            mDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
            mDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
            mDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
            mDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
            mDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
            mDevice->SetPixelShader(nullptr);
            mDevice->SetVertexShader(nullptr);
            mDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
            mDevice->SetStreamSource(0, nullptr, 0, 0);
            mDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
            mDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            mDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
            mDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            mDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
            mDevice->SetRenderState(D3DRS_TEXTUREFACTOR, color);
            mDevice->SetRenderState(D3DRS_MULTISAMPLEMASK, 0xFFFFFFFF);

            for (int i = 0; i < gl::MAX_VERTEX_ATTRIBS; i++)
            {
                mDevice->SetStreamSourceFreq(i, 1);
            }

            hr = mDevice->EndStateBlock(&mMaskedClearSavedState);
            ASSERT(SUCCEEDED(hr) || hr == D3DERR_OUTOFVIDEOMEMORY || hr == E_OUTOFMEMORY);
        }

        ASSERT(mMaskedClearSavedState != nullptr);

        if (mMaskedClearSavedState != nullptr)
        {
            hr = mMaskedClearSavedState->Capture();
            ASSERT(SUCCEEDED(hr));
        }

        mDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        mDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
        mDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        mDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        mDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
        mDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        mDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        mDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);

        if (clearColor)
        {
            // clearParams.colorMask follows the same packing scheme as
            // D3DCOLORWRITEENABLE_RED/GREEN/BLUE/ALPHA
            mDevice->SetRenderState(
                D3DRS_COLORWRITEENABLE,
                gl::BlendStateExt::ColorMaskStorage::GetValueIndexed(0, clearParams.colorMask));
        }
        else
        {
            mDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
        }

        if (stencilUnmasked != 0x0 && clearParams.clearStencil)
        {
            mDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
            mDevice->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, FALSE);
            mDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
            mDevice->SetRenderState(D3DRS_STENCILREF, stencil);
            mDevice->SetRenderState(D3DRS_STENCILWRITEMASK, clearParams.stencilWriteMask);
            mDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_REPLACE);
            mDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_REPLACE);
            mDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
        }
        else
        {
            mDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
        }

        mDevice->SetPixelShader(nullptr);
        mDevice->SetVertexShader(nullptr);
        mDevice->SetFVF(D3DFVF_XYZRHW);
        mDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
        mDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        mDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
        mDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        mDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
        mDevice->SetRenderState(D3DRS_TEXTUREFACTOR, color);
        mDevice->SetRenderState(D3DRS_MULTISAMPLEMASK, 0xFFFFFFFF);

        for (int i = 0; i < gl::MAX_VERTEX_ATTRIBS; i++)
        {
            mDevice->SetStreamSourceFreq(i, 1);
        }

        int renderTargetWidth  = mStateManager.getRenderTargetWidth();
        int renderTargetHeight = mStateManager.getRenderTargetHeight();

        float quad[4][4];  // A quadrilateral covering the target, aligned to match the edges
        quad[0][0] = -0.5f;
        quad[0][1] = renderTargetHeight - 0.5f;
        quad[0][2] = 0.0f;
        quad[0][3] = 1.0f;

        quad[1][0] = renderTargetWidth - 0.5f;
        quad[1][1] = renderTargetHeight - 0.5f;
        quad[1][2] = 0.0f;
        quad[1][3] = 1.0f;

        quad[2][0] = -0.5f;
        quad[2][1] = -0.5f;
        quad[2][2] = 0.0f;
        quad[2][3] = 1.0f;

        quad[3][0] = renderTargetWidth - 0.5f;
        quad[3][1] = -0.5f;
        quad[3][2] = 0.0f;
        quad[3][3] = 1.0f;

        startScene();
        mDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(float[4]));

        if (clearParams.clearDepth)
        {
            mDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
            mDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
            mDevice->Clear(0, nullptr, D3DCLEAR_ZBUFFER, color, depth, stencil);
        }

        if (mMaskedClearSavedState != nullptr)
        {
            mMaskedClearSavedState->Apply();
        }
    }
    else if (clearColor || clearParams.clearDepth || clearParams.clearStencil)
    {
        DWORD dxClearFlags = 0;
        if (clearColor)
        {
            dxClearFlags |= D3DCLEAR_TARGET;
        }
        if (clearParams.clearDepth)
        {
            dxClearFlags |= D3DCLEAR_ZBUFFER;
        }
        if (clearParams.clearStencil)
        {
            dxClearFlags |= D3DCLEAR_STENCIL;
        }

        mDevice->Clear(0, nullptr, dxClearFlags, color, depth, stencil);
    }
}

void Renderer9::markAllStateDirty()
{
    mAppliedRenderTargetSerial   = 0;
    mAppliedDepthStencilSerial   = 0;
    mDepthStencilInitialized     = false;
    mRenderTargetDescInitialized = false;

    mStateManager.forceSetRasterState();
    mStateManager.forceSetDepthStencilState();
    mStateManager.forceSetBlendState();
    mStateManager.forceSetScissorState();
    mStateManager.forceSetViewportState();

    ASSERT(mCurVertexSamplerStates.size() == mCurVertexTextures.size());
    for (unsigned int i = 0; i < mCurVertexTextures.size(); i++)
    {
        mCurVertexSamplerStates[i].forceSet = true;
        mCurVertexTextures[i]               = angle::DirtyPointer;
    }

    ASSERT(mCurPixelSamplerStates.size() == mCurPixelTextures.size());
    for (unsigned int i = 0; i < mCurPixelSamplerStates.size(); i++)
    {
        mCurPixelSamplerStates[i].forceSet = true;
        mCurPixelTextures[i]               = angle::DirtyPointer;
    }

    mAppliedIBSerial      = 0;
    mAppliedVertexShader  = nullptr;
    mAppliedPixelShader   = nullptr;
    mAppliedProgramSerial = 0;
    mStateManager.forceSetDXUniformsState();

    mVertexDeclarationCache.markStateDirty();
}

void Renderer9::releaseDeviceResources()
{
    for (size_t i = 0; i < mEventQueryPool.size(); i++)
    {
        SafeRelease(mEventQueryPool[i]);
    }
    mEventQueryPool.clear();

    SafeRelease(mMaskedClearSavedState);

    mVertexShaderCache.clear();
    mPixelShaderCache.clear();

    SafeDelete(mBlit);
    SafeDelete(mVertexDataManager);
    SafeDelete(mIndexDataManager);
    SafeDelete(mLineLoopIB);
    SafeDelete(mCountingIB);

    for (int i = 0; i < NUM_NULL_COLORBUFFER_CACHE_ENTRIES; i++)
    {
        SafeDelete(mNullRenderTargetCache[i].renderTarget);
    }
}

// set notify to true to broadcast a message to all contexts of the device loss
bool Renderer9::testDeviceLost()
{
    HRESULT status = getDeviceStatusCode();
    return FAILED(status);
}

HRESULT Renderer9::getDeviceStatusCode()
{
    HRESULT status = D3D_OK;

    if (mDeviceEx)
    {
        status = mDeviceEx->CheckDeviceState(nullptr);
    }
    else if (mDevice)
    {
        status = mDevice->TestCooperativeLevel();
    }

    return status;
}

bool Renderer9::testDeviceResettable()
{
    // On D3D9Ex, DEVICELOST represents a hung device that needs to be restarted
    // DEVICEREMOVED indicates the device has been stopped and must be recreated
    switch (getDeviceStatusCode())
    {
        case D3DERR_DEVICENOTRESET:
        case D3DERR_DEVICEHUNG:
            return true;
        case D3DERR_DEVICELOST:
            return (mDeviceEx != nullptr);
        case D3DERR_DEVICEREMOVED:
            ASSERT(mDeviceEx != nullptr);
            return isRemovedDeviceResettable();
        default:
            return false;
    }
}

bool Renderer9::resetDevice()
{
    releaseDeviceResources();

    D3DPRESENT_PARAMETERS presentParameters = getDefaultPresentParameters();

    HRESULT result     = D3D_OK;
    bool lost          = testDeviceLost();
    bool removedDevice = (getDeviceStatusCode() == D3DERR_DEVICEREMOVED);

    // Device Removed is a feature which is only present with D3D9Ex
    ASSERT(mDeviceEx != nullptr || !removedDevice);

    for (int attempts = 3; lost && attempts > 0; attempts--)
    {
        if (removedDevice)
        {
            // Device removed, which may trigger on driver reinstallation,
            // may cause a longer wait other reset attempts before the
            // system is ready to handle creating a new device.
            Sleep(800);
            lost = !resetRemovedDevice();
        }
        else if (mDeviceEx)
        {
            Sleep(500);  // Give the graphics driver some CPU time
            result = mDeviceEx->ResetEx(&presentParameters, nullptr);
            lost   = testDeviceLost();
        }
        else
        {
            result = mDevice->TestCooperativeLevel();
            while (result == D3DERR_DEVICELOST)
            {
                Sleep(100);  // Give the graphics driver some CPU time
                result = mDevice->TestCooperativeLevel();
            }

            if (result == D3DERR_DEVICENOTRESET)
            {
                result = mDevice->Reset(&presentParameters);
            }
            lost = testDeviceLost();
        }
    }

    if (FAILED(result))
    {
        ERR() << "Reset/ResetEx failed multiple times, " << gl::FmtHR(result);
        return false;
    }

    if (removedDevice && lost)
    {
        ERR() << "Device lost reset failed multiple times";
        return false;
    }

    // If the device was removed, we already finished re-initialization in resetRemovedDevice
    if (!removedDevice)
    {
        // reset device defaults
        if (initializeDevice().isError())
        {
            return false;
        }
    }

    return true;
}

bool Renderer9::isRemovedDeviceResettable() const
{
    bool success = false;

#if ANGLE_D3D9EX
    IDirect3D9Ex *d3d9Ex = nullptr;
    typedef HRESULT(WINAPI * Direct3DCreate9ExFunc)(UINT, IDirect3D9Ex **);
    Direct3DCreate9ExFunc Direct3DCreate9ExPtr =
        reinterpret_cast<Direct3DCreate9ExFunc>(GetProcAddress(mD3d9Module, "Direct3DCreate9Ex"));

    if (Direct3DCreate9ExPtr && SUCCEEDED(Direct3DCreate9ExPtr(D3D_SDK_VERSION, &d3d9Ex)))
    {
        D3DCAPS9 deviceCaps;
        HRESULT result = d3d9Ex->GetDeviceCaps(mAdapter, mDeviceType, &deviceCaps);
        success        = SUCCEEDED(result);
    }

    SafeRelease(d3d9Ex);
#else
    UNREACHABLE();
#endif

    return success;
}

bool Renderer9::resetRemovedDevice()
{
    // From http://msdn.microsoft.com/en-us/library/windows/desktop/bb172554(v=vs.85).aspx:
    // The hardware adapter has been removed. Application must destroy the device, do enumeration of
    // adapters and create another Direct3D device. If application continues rendering without
    // calling Reset, the rendering calls will succeed. Applies to Direct3D 9Ex only.
    release();
    return !initialize().isError();
}

VendorID Renderer9::getVendorId() const
{
    return static_cast<VendorID>(mAdapterIdentifier.VendorId);
}

std::string Renderer9::getRendererDescription() const
{
    std::ostringstream rendererString;

    rendererString << mAdapterIdentifier.Description;
    if (getShareHandleSupport())
    {
        rendererString << " Direct3D9Ex";
    }
    else
    {
        rendererString << " Direct3D9";
    }

    rendererString << " vs_" << D3DSHADER_VERSION_MAJOR(mDeviceCaps.VertexShaderVersion) << "_"
                   << D3DSHADER_VERSION_MINOR(mDeviceCaps.VertexShaderVersion);
    rendererString << " ps_" << D3DSHADER_VERSION_MAJOR(mDeviceCaps.PixelShaderVersion) << "_"
                   << D3DSHADER_VERSION_MINOR(mDeviceCaps.PixelShaderVersion);

    return rendererString.str();
}

DeviceIdentifier Renderer9::getAdapterIdentifier() const
{
    DeviceIdentifier deviceIdentifier = {};
    deviceIdentifier.VendorId         = static_cast<UINT>(mAdapterIdentifier.VendorId);
    deviceIdentifier.DeviceId         = static_cast<UINT>(mAdapterIdentifier.DeviceId);
    deviceIdentifier.SubSysId         = static_cast<UINT>(mAdapterIdentifier.SubSysId);
    deviceIdentifier.Revision         = static_cast<UINT>(mAdapterIdentifier.Revision);
    deviceIdentifier.FeatureLevel     = 0;

    return deviceIdentifier;
}

unsigned int Renderer9::getReservedVertexUniformVectors() const
{
    return d3d9_gl::GetReservedVertexUniformVectors();
}

unsigned int Renderer9::getReservedFragmentUniformVectors() const
{
    return d3d9_gl::GetReservedFragmentUniformVectors();
}

bool Renderer9::getShareHandleSupport() const
{
    // PIX doesn't seem to support using share handles, so disable them.
    return (mD3d9Ex != nullptr) && !gl::DebugAnnotationsActive(/*context=*/nullptr);
}

int Renderer9::getMajorShaderModel() const
{
    return D3DSHADER_VERSION_MAJOR(mDeviceCaps.PixelShaderVersion);
}

int Renderer9::getMinorShaderModel() const
{
    return D3DSHADER_VERSION_MINOR(mDeviceCaps.PixelShaderVersion);
}

std::string Renderer9::getShaderModelSuffix() const
{
    return "";
}

DWORD Renderer9::getCapsDeclTypes() const
{
    return mDeviceCaps.DeclTypes;
}

D3DPOOL Renderer9::getBufferPool(DWORD usage) const
{
    if (mD3d9Ex != nullptr)
    {
        return D3DPOOL_DEFAULT;
    }
    else
    {
        if (!(usage & D3DUSAGE_DYNAMIC))
        {
            return D3DPOOL_MANAGED;
        }
    }

    return D3DPOOL_DEFAULT;
}

angle::Result Renderer9::copyImage2D(const gl::Context *context,
                                     const gl::Framebuffer *framebuffer,
                                     const gl::Rectangle &sourceRect,
                                     GLenum destFormat,
                                     const gl::Offset &destOffset,
                                     TextureStorage *storage,
                                     GLint level)
{
    RECT rect;
    rect.left   = sourceRect.x;
    rect.top    = sourceRect.y;
    rect.right  = sourceRect.x + sourceRect.width;
    rect.bottom = sourceRect.y + sourceRect.height;

    return mBlit->copy2D(context, framebuffer, rect, destFormat, destOffset, storage, level);
}

angle::Result Renderer9::copyImageCube(const gl::Context *context,
                                       const gl::Framebuffer *framebuffer,
                                       const gl::Rectangle &sourceRect,
                                       GLenum destFormat,
                                       const gl::Offset &destOffset,
                                       TextureStorage *storage,
                                       gl::TextureTarget target,
                                       GLint level)
{
    RECT rect;
    rect.left   = sourceRect.x;
    rect.top    = sourceRect.y;
    rect.right  = sourceRect.x + sourceRect.width;
    rect.bottom = sourceRect.y + sourceRect.height;

    return mBlit->copyCube(context, framebuffer, rect, destFormat, destOffset, storage, target,
                           level);
}

angle::Result Renderer9::copyImage3D(const gl::Context *context,
                                     const gl::Framebuffer *framebuffer,
                                     const gl::Rectangle &sourceRect,
                                     GLenum destFormat,
                                     const gl::Offset &destOffset,
                                     TextureStorage *storage,
                                     GLint level)
{
    // 3D textures are not available in the D3D9 backend.
    ANGLE_HR_UNREACHABLE(GetImplAs<Context9>(context));
    return angle::Result::Stop;
}

angle::Result Renderer9::copyImage2DArray(const gl::Context *context,
                                          const gl::Framebuffer *framebuffer,
                                          const gl::Rectangle &sourceRect,
                                          GLenum destFormat,
                                          const gl::Offset &destOffset,
                                          TextureStorage *storage,
                                          GLint level)
{
    // 2D array textures are not available in the D3D9 backend.
    ANGLE_HR_UNREACHABLE(GetImplAs<Context9>(context));
    return angle::Result::Stop;
}

angle::Result Renderer9::copyTexture(const gl::Context *context,
                                     const gl::Texture *source,
                                     GLint sourceLevel,
                                     gl::TextureTarget srcTarget,
                                     const gl::Box &sourceBox,
                                     GLenum destFormat,
                                     GLenum destType,
                                     const gl::Offset &destOffset,
                                     TextureStorage *storage,
                                     gl::TextureTarget destTarget,
                                     GLint destLevel,
                                     bool unpackFlipY,
                                     bool unpackPremultiplyAlpha,
                                     bool unpackUnmultiplyAlpha)
{
    RECT rect;
    rect.left   = sourceBox.x;
    rect.top    = sourceBox.y;
    rect.right  = sourceBox.x + sourceBox.width;
    rect.bottom = sourceBox.y + sourceBox.height;

    return mBlit->copyTexture(context, source, sourceLevel, rect, destFormat, destOffset, storage,
                              destTarget, destLevel, unpackFlipY, unpackPremultiplyAlpha,
                              unpackUnmultiplyAlpha);
}

angle::Result Renderer9::copyCompressedTexture(const gl::Context *context,
                                               const gl::Texture *source,
                                               GLint sourceLevel,
                                               TextureStorage *storage,
                                               GLint destLevel)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context9>(context));
    return angle::Result::Stop;
}

angle::Result Renderer9::createRenderTarget(const gl::Context *context,
                                            int width,
                                            int height,
                                            GLenum format,
                                            GLsizei samples,
                                            RenderTargetD3D **outRT)
{
    const d3d9::TextureFormat &d3d9FormatInfo = d3d9::GetTextureFormatInfo(format);

    const gl::TextureCaps &textureCaps = getNativeTextureCaps().get(format);
    GLuint supportedSamples            = textureCaps.getNearestSamples(samples);

    IDirect3DTexture9 *texture      = nullptr;
    IDirect3DSurface9 *renderTarget = nullptr;
    if (width > 0 && height > 0)
    {
        bool requiresInitialization = false;
        HRESULT result              = D3DERR_INVALIDCALL;

        const gl::InternalFormat &formatInfo = gl::GetSizedInternalFormatInfo(format);
        if (formatInfo.depthBits > 0 || formatInfo.stencilBits > 0)
        {
            result = mDevice->CreateDepthStencilSurface(
                width, height, d3d9FormatInfo.renderFormat,
                gl_d3d9::GetMultisampleType(supportedSamples), 0, FALSE, &renderTarget, nullptr);
        }
        else
        {
            requiresInitialization = (d3d9FormatInfo.dataInitializerFunction != nullptr);
            if (supportedSamples > 0)
            {
                result = mDevice->CreateRenderTarget(width, height, d3d9FormatInfo.renderFormat,
                                                     gl_d3d9::GetMultisampleType(supportedSamples),
                                                     0, FALSE, &renderTarget, nullptr);
            }
            else
            {
                result = mDevice->CreateTexture(
                    width, height, 1, D3DUSAGE_RENDERTARGET, d3d9FormatInfo.texFormat,
                    getTexturePool(D3DUSAGE_RENDERTARGET), &texture, nullptr);
                if (!FAILED(result))
                {
                    result = texture->GetSurfaceLevel(0, &renderTarget);
                }
            }
        }

        ANGLE_TRY_HR(GetImplAs<Context9>(context), result, "Failed to create render target");

        if (requiresInitialization)
        {
            // This format requires that the data be initialized before the render target can be
            // used Unfortunately this requires a Get call on the d3d device but it is far better
            // than having to mark the render target as lockable and copy data to the gpu.
            IDirect3DSurface9 *prevRenderTarget = nullptr;
            mDevice->GetRenderTarget(0, &prevRenderTarget);
            mDevice->SetRenderTarget(0, renderTarget);
            mDevice->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_RGBA(0, 0, 0, 255), 0.0f, 0);
            mDevice->SetRenderTarget(0, prevRenderTarget);
        }
    }

    *outRT = new TextureRenderTarget9(texture, 0, renderTarget, format, width, height, 1,
                                      supportedSamples);
    return angle::Result::Continue;
}

angle::Result Renderer9::createRenderTargetCopy(const gl::Context *context,
                                                RenderTargetD3D *source,
                                                RenderTargetD3D **outRT)
{
    ASSERT(source != nullptr);

    RenderTargetD3D *newRT = nullptr;
    ANGLE_TRY(createRenderTarget(context, source->getWidth(), source->getHeight(),
                                 source->getInternalFormat(), source->getSamples(), &newRT));

    RenderTarget9 *source9 = GetAs<RenderTarget9>(source);
    RenderTarget9 *dest9   = GetAs<RenderTarget9>(newRT);

    HRESULT result = mDevice->StretchRect(source9->getSurface(), nullptr, dest9->getSurface(),
                                          nullptr, D3DTEXF_NONE);
    ANGLE_TRY_HR(GetImplAs<Context9>(context), result, "Failed to copy render target");

    *outRT = newRT;
    return angle::Result::Continue;
}

angle::Result Renderer9::loadExecutable(d3d::Context *context,
                                        const uint8_t *function,
                                        size_t length,
                                        gl::ShaderType type,
                                        const std::vector<D3DVarying> &streamOutVaryings,
                                        bool separatedOutputBuffers,
                                        ShaderExecutableD3D **outExecutable)
{
    // Transform feedback is not supported in ES2 or D3D9
    ASSERT(streamOutVaryings.empty());

    switch (type)
    {
        case gl::ShaderType::Vertex:
        {
            IDirect3DVertexShader9 *vshader = nullptr;
            ANGLE_TRY(createVertexShader(context, (DWORD *)function, length, &vshader));
            *outExecutable = new ShaderExecutable9(function, length, vshader);
        }
        break;
        case gl::ShaderType::Fragment:
        {
            IDirect3DPixelShader9 *pshader = nullptr;
            ANGLE_TRY(createPixelShader(context, (DWORD *)function, length, &pshader));
            *outExecutable = new ShaderExecutable9(function, length, pshader);
        }
        break;
        default:
            ANGLE_HR_UNREACHABLE(context);
    }

    return angle::Result::Continue;
}

angle::Result Renderer9::compileToExecutable(d3d::Context *context,
                                             gl::InfoLog &infoLog,
                                             const std::string &shaderHLSL,
                                             gl::ShaderType type,
                                             const std::vector<D3DVarying> &streamOutVaryings,
                                             bool separatedOutputBuffers,
                                             const CompilerWorkaroundsD3D &workarounds,
                                             ShaderExecutableD3D **outExectuable)
{
    // Transform feedback is not supported in ES2 or D3D9
    ASSERT(streamOutVaryings.empty());

    std::stringstream profileStream;

    switch (type)
    {
        case gl::ShaderType::Vertex:
            profileStream << "vs";
            break;
        case gl::ShaderType::Fragment:
            profileStream << "ps";
            break;
        default:
            ANGLE_HR_UNREACHABLE(context);
    }

    profileStream << "_" << ((getMajorShaderModel() >= 3) ? 3 : 2);
    profileStream << "_" << "0";

    std::string profile = profileStream.str();

    UINT flags = ANGLE_COMPILE_OPTIMIZATION_LEVEL;

    if (workarounds.skipOptimization)
    {
        flags = D3DCOMPILE_SKIP_OPTIMIZATION;
    }
    else if (workarounds.useMaxOptimization)
    {
        flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
    }

    if (gl::DebugAnnotationsActive(/*context=*/nullptr))
    {
#ifndef NDEBUG
        flags = D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        flags |= D3DCOMPILE_DEBUG;
    }

    // Sometimes D3DCompile will fail with the default compilation flags for complicated shaders
    // when it would otherwise pass with alternative options. Try the default flags first and if
    // compilation fails, try some alternatives.
    std::vector<CompileConfig> configs;
    configs.push_back(CompileConfig(flags, "default"));
    configs.push_back(CompileConfig(flags | D3DCOMPILE_AVOID_FLOW_CONTROL, "avoid flow control"));
    configs.push_back(CompileConfig(flags | D3DCOMPILE_PREFER_FLOW_CONTROL, "prefer flow control"));

    ID3DBlob *binary = nullptr;
    std::string debugInfo;
    angle::Result error = mCompiler.compileToBinary(context, infoLog, shaderHLSL, profile, configs,
                                                    nullptr, &binary, &debugInfo);
    ANGLE_TRY(error);

    // It's possible that binary is NULL if the compiler failed in all configurations.  Set the
    // executable to NULL and return GL_NO_ERROR to signify that there was a link error but the
    // internal state is still OK.
    if (!binary)
    {
        *outExectuable = nullptr;
        return angle::Result::Continue;
    }

    error = loadExecutable(context, reinterpret_cast<const uint8_t *>(binary->GetBufferPointer()),
                           binary->GetBufferSize(), type, streamOutVaryings, separatedOutputBuffers,
                           outExectuable);

    SafeRelease(binary);
    ANGLE_TRY(error);

    if (!debugInfo.empty())
    {
        (*outExectuable)->appendDebugInfo(debugInfo);
    }

    return angle::Result::Continue;
}

angle::Result Renderer9::ensureHLSLCompilerInitialized(d3d::Context *context)
{
    return mCompiler.ensureInitialized(context);
}

UniformStorageD3D *Renderer9::createUniformStorage(size_t storageSize)
{
    return new UniformStorageD3D(storageSize);
}

angle::Result Renderer9::boxFilter(Context9 *context9,
                                   IDirect3DSurface9 *source,
                                   IDirect3DSurface9 *dest)
{
    return mBlit->boxFilter(context9, source, dest);
}

D3DPOOL Renderer9::getTexturePool(DWORD usage) const
{
    if (mD3d9Ex != nullptr)
    {
        return D3DPOOL_DEFAULT;
    }
    else
    {
        if (!(usage & (D3DUSAGE_DEPTHSTENCIL | D3DUSAGE_RENDERTARGET)))
        {
            return D3DPOOL_MANAGED;
        }
    }

    return D3DPOOL_DEFAULT;
}

angle::Result Renderer9::copyToRenderTarget(const gl::Context *context,
                                            IDirect3DSurface9 *dest,
                                            IDirect3DSurface9 *source,
                                            bool fromManaged)
{
    ASSERT(source && dest);

    Context9 *context9 = GetImplAs<Context9>(context);

    HRESULT result = D3DERR_OUTOFVIDEOMEMORY;

    if (fromManaged)
    {
        D3DSURFACE_DESC desc;
        source->GetDesc(&desc);

        IDirect3DSurface9 *surf = 0;
        result = mDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format,
                                                      D3DPOOL_SYSTEMMEM, &surf, nullptr);

        if (SUCCEEDED(result))
        {
            ANGLE_TRY(Image9::CopyLockableSurfaces(context9, surf, source));
            result = mDevice->UpdateSurface(surf, nullptr, dest, nullptr);
            SafeRelease(surf);
        }
    }
    else
    {
        endScene();
        result = mDevice->StretchRect(source, nullptr, dest, nullptr, D3DTEXF_NONE);
    }

    ANGLE_TRY_HR(context9, result, "Failed to blit internal texture");
    return angle::Result::Continue;
}

RendererClass Renderer9::getRendererClass() const
{
    return RENDERER_D3D9;
}

ImageD3D *Renderer9::createImage()
{
    return new Image9(this);
}

ExternalImageSiblingImpl *Renderer9::createExternalImageSibling(const gl::Context *context,
                                                                EGLenum target,
                                                                EGLClientBuffer buffer,
                                                                const egl::AttributeMap &attribs)
{
    UNREACHABLE();
    return nullptr;
}

angle::Result Renderer9::generateMipmap(const gl::Context *context, ImageD3D *dest, ImageD3D *src)
{
    Image9 *src9 = GetAs<Image9>(src);
    Image9 *dst9 = GetAs<Image9>(dest);
    return Image9::GenerateMipmap(GetImplAs<Context9>(context), dst9, src9);
}

angle::Result Renderer9::generateMipmapUsingD3D(const gl::Context *context,
                                                TextureStorage *storage,
                                                const gl::TextureState &textureState)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context9>(context));
    return angle::Result::Stop;
}

angle::Result Renderer9::copyImage(const gl::Context *context,
                                   ImageD3D *dest,
                                   ImageD3D *source,
                                   const gl::Box &sourceBox,
                                   const gl::Offset &destOffset,
                                   bool unpackFlipY,
                                   bool unpackPremultiplyAlpha,
                                   bool unpackUnmultiplyAlpha)
{
    Image9 *dest9 = GetAs<Image9>(dest);
    Image9 *src9  = GetAs<Image9>(source);
    return Image9::CopyImage(context, dest9, src9, sourceBox.toRect(), destOffset, unpackFlipY,
                             unpackPremultiplyAlpha, unpackUnmultiplyAlpha);
}

TextureStorage *Renderer9::createTextureStorage2D(SwapChainD3D *swapChain, const std::string &label)
{
    SwapChain9 *swapChain9 = GetAs<SwapChain9>(swapChain);
    return new TextureStorage9_2D(this, swapChain9, label);
}

TextureStorage *Renderer9::createTextureStorageEGLImage(EGLImageD3D *eglImage,
                                                        RenderTargetD3D *renderTargetD3D,
                                                        const std::string &label)
{
    return new TextureStorage9_EGLImage(this, eglImage, GetAs<RenderTarget9>(renderTargetD3D),
                                        label);
}

TextureStorage *Renderer9::createTextureStorageBuffer(
    const gl::OffsetBindingPointer<gl::Buffer> &buffer,
    GLenum internalFormat,
    const std::string &label)
{
    UNREACHABLE();
    return nullptr;
}

TextureStorage *Renderer9::createTextureStorageExternal(
    egl::Stream *stream,
    const egl::Stream::GLTextureDescription &desc,
    const std::string &label)
{
    UNIMPLEMENTED();
    return nullptr;
}

TextureStorage *Renderer9::createTextureStorage2D(GLenum internalformat,
                                                  BindFlags bindFlags,
                                                  GLsizei width,
                                                  GLsizei height,
                                                  int levels,
                                                  const std::string &label,
                                                  bool hintLevelZeroOnly)
{
    return new TextureStorage9_2D(this, internalformat, bindFlags.renderTarget, width, height,
                                  levels, label);
}

TextureStorage *Renderer9::createTextureStorageCube(GLenum internalformat,
                                                    BindFlags bindFlags,
                                                    int size,
                                                    int levels,
                                                    bool hintLevelZeroOnly,
                                                    const std::string &label)
{
    return new TextureStorage9_Cube(this, internalformat, bindFlags.renderTarget, size, levels,
                                    hintLevelZeroOnly, label);
}

TextureStorage *Renderer9::createTextureStorage3D(GLenum internalformat,
                                                  BindFlags bindFlags,
                                                  GLsizei width,
                                                  GLsizei height,
                                                  GLsizei depth,
                                                  int levels,
                                                  const std::string &label)
{
    // 3D textures are not supported by the D3D9 backend.
    UNREACHABLE();

    return nullptr;
}

TextureStorage *Renderer9::createTextureStorage2DArray(GLenum internalformat,
                                                       BindFlags bindFlags,
                                                       GLsizei width,
                                                       GLsizei height,
                                                       GLsizei depth,
                                                       int levels,
                                                       const std::string &label)
{
    // 2D array textures are not supported by the D3D9 backend.
    UNREACHABLE();

    return nullptr;
}

TextureStorage *Renderer9::createTextureStorage2DMultisample(GLenum internalformat,
                                                             GLsizei width,
                                                             GLsizei height,
                                                             int levels,
                                                             int samples,
                                                             bool fixedSampleLocations,
                                                             const std::string &label)
{
    // 2D multisampled textures are not supported by the D3D9 backend.
    UNREACHABLE();

    return nullptr;
}

TextureStorage *Renderer9::createTextureStorage2DMultisampleArray(GLenum internalformat,
                                                                  GLsizei width,
                                                                  GLsizei height,
                                                                  GLsizei depth,
                                                                  int levels,
                                                                  int samples,
                                                                  bool fixedSampleLocations,
                                                                  const std::string &label)
{
    // 2D multisampled textures are not supported by the D3D9 backend.
    UNREACHABLE();

    return nullptr;
}

bool Renderer9::getLUID(LUID *adapterLuid) const
{
    adapterLuid->HighPart = 0;
    adapterLuid->LowPart  = 0;

    if (mD3d9Ex)
    {
        mD3d9Ex->GetAdapterLUID(mAdapter, adapterLuid);
        return true;
    }

    return false;
}

VertexConversionType Renderer9::getVertexConversionType(angle::FormatID vertexFormatID) const
{
    return d3d9::GetVertexFormatInfo(getCapsDeclTypes(), vertexFormatID).conversionType;
}

GLenum Renderer9::getVertexComponentType(angle::FormatID vertexFormatID) const
{
    return d3d9::GetVertexFormatInfo(getCapsDeclTypes(), vertexFormatID).componentType;
}

angle::Result Renderer9::getVertexSpaceRequired(const gl::Context *context,
                                                const gl::VertexAttribute &attrib,
                                                const gl::VertexBinding &binding,
                                                size_t count,
                                                GLsizei instances,
                                                GLuint baseInstance,
                                                unsigned int *bytesRequiredOut) const
{
    if (!attrib.enabled)
    {
        *bytesRequiredOut = 16u;
        return angle::Result::Continue;
    }

    angle::FormatID vertexFormatID = gl::GetVertexFormatID(attrib, gl::VertexAttribType::Float);
    const d3d9::VertexFormat &d3d9VertexInfo =
        d3d9::GetVertexFormatInfo(getCapsDeclTypes(), vertexFormatID);

    unsigned int elementCount  = 0;
    const unsigned int divisor = binding.getDivisor();
    if (instances == 0 || divisor == 0)
    {
        elementCount = static_cast<unsigned int>(count);
    }
    else
    {
        // Round up to divisor, if possible
        elementCount = UnsignedCeilDivide(static_cast<unsigned int>(instances), divisor);
    }

    bool check = (d3d9VertexInfo.outputElementSize >
                  std::numeric_limits<unsigned int>::max() / elementCount);
    ANGLE_CHECK(GetImplAs<Context9>(context), !check,
                "New vertex buffer size would result in an overflow.", GL_OUT_OF_MEMORY);

    *bytesRequiredOut = static_cast<unsigned int>(d3d9VertexInfo.outputElementSize) * elementCount;
    return angle::Result::Continue;
}

void Renderer9::generateCaps(gl::Caps *outCaps,
                             gl::TextureCapsMap *outTextureCaps,
                             gl::Extensions *outExtensions,
                             gl::Limitations *outLimitations,
                             ShPixelLocalStorageOptions *outPLSOptions) const
{
    d3d9_gl::GenerateCaps(mD3d9, mDevice, mDeviceType, mAdapter, outCaps, outTextureCaps,
                          outExtensions, outLimitations);
}

void Renderer9::initializeFeatures(angle::FeaturesD3D *features) const
{
    ApplyFeatureOverrides(features, mDisplay->getState().featureOverrides);
    if (!mDisplay->getState().featureOverrides.allDisabled)
    {
        d3d9::InitializeFeatures(features, mAdapterIdentifier.VendorId);
    }
}

void Renderer9::initializeFrontendFeatures(angle::FrontendFeatures *features) const
{
    ApplyFeatureOverrides(features, mDisplay->getState().featureOverrides);
    if (!mDisplay->getState().featureOverrides.allDisabled)
    {
        d3d9::InitializeFrontendFeatures(features, mAdapterIdentifier.VendorId);
    }
}

DeviceImpl *Renderer9::createEGLDevice()
{
    return new Device9(mDevice);
}

Renderer9::CurSamplerState::CurSamplerState()
    : forceSet(true), baseLevel(std::numeric_limits<size_t>::max()), samplerState()
{}

angle::Result Renderer9::genericDrawElements(const gl::Context *context,
                                             gl::PrimitiveMode mode,
                                             GLsizei count,
                                             gl::DrawElementsType type,
                                             const void *indices,
                                             GLsizei instances)
{
    const gl::State &state = context->getState();
    ProgramExecutableD3D *executableD3D =
        GetImplAs<ProgramExecutableD3D>(state.getProgramExecutable());
    ASSERT(executableD3D != nullptr);
    bool usesPointSize = executableD3D->usesPointSize();

    if (executableD3D->isSamplerMappingDirty())
    {
        executableD3D->updateSamplerMapping();
    }

    if (!applyPrimitiveType(mode, count, usesPointSize))
    {
        return angle::Result::Continue;
    }

    ANGLE_TRY(updateState(context, mode));
    ANGLE_TRY(applyTextures(context));
    ANGLE_TRY(applyShaders(context, mode));

    if (!skipDraw(state, mode))
    {
        ANGLE_TRY(drawElementsImpl(context, mode, count, type, indices, instances));
    }

    return angle::Result::Continue;
}

angle::Result Renderer9::genericDrawArrays(const gl::Context *context,
                                           gl::PrimitiveMode mode,
                                           GLint first,
                                           GLsizei count,
                                           GLsizei instances)
{
    const gl::State &state = context->getState();
    ProgramExecutableD3D *executableD3D =
        GetImplAs<ProgramExecutableD3D>(state.getProgramExecutable());
    ASSERT(executableD3D != nullptr);
    bool usesPointSize = executableD3D->usesPointSize();

    if (executableD3D->isSamplerMappingDirty())
    {
        executableD3D->updateSamplerMapping();
    }

    if (!applyPrimitiveType(mode, count, usesPointSize))
    {
        return angle::Result::Continue;
    }

    ANGLE_TRY(updateState(context, mode));
    ANGLE_TRY(applyVertexBuffer(context, mode, first, count, instances, nullptr));
    ANGLE_TRY(applyTextures(context));
    ANGLE_TRY(applyShaders(context, mode));

    if (!skipDraw(context->getState(), mode))
    {
        ANGLE_TRY(drawArraysImpl(context, mode, first, count, instances));
    }

    return angle::Result::Continue;
}

FramebufferImpl *Renderer9::createDefaultFramebuffer(const gl::FramebufferState &state)
{
    return new Framebuffer9(state, this);
}

gl::Version Renderer9::getMaxSupportedESVersion() const
{
    return gl::Version(2, 0);
}

gl::Version Renderer9::getMaxConformantESVersion() const
{
    return gl::Version(2, 0);
}

angle::Result Renderer9::clearRenderTarget(const gl::Context *context,
                                           RenderTargetD3D *renderTarget,
                                           const gl::ColorF &clearColorValue,
                                           const float clearDepthValue,
                                           const unsigned int clearStencilValue)
{
    D3DCOLOR color =
        D3DCOLOR_ARGB(gl::unorm<8>(clearColorValue.alpha), gl::unorm<8>(clearColorValue.red),
                      gl::unorm<8>(clearColorValue.green), gl::unorm<8>(clearColorValue.blue));
    float depth   = clearDepthValue;
    DWORD stencil = clearStencilValue & 0x000000FF;

    unsigned int renderTargetSerial        = renderTarget->getSerial();
    RenderTarget9 *renderTarget9           = GetAs<RenderTarget9>(renderTarget);
    IDirect3DSurface9 *renderTargetSurface = renderTarget9->getSurface();
    ASSERT(renderTargetSurface);

    DWORD dxClearFlags = 0;

    const gl::InternalFormat &internalFormatInfo =
        gl::GetSizedInternalFormatInfo(renderTarget->getInternalFormat());
    if (internalFormatInfo.depthBits > 0 || internalFormatInfo.stencilBits > 0)
    {
        dxClearFlags = D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL;
        if (mAppliedDepthStencilSerial != renderTargetSerial)
        {
            mDevice->SetDepthStencilSurface(renderTargetSurface);
        }
    }
    else
    {
        dxClearFlags = D3DCLEAR_TARGET;
        if (mAppliedRenderTargetSerial != renderTargetSerial)
        {
            mDevice->SetRenderTarget(0, renderTargetSurface);
        }
    }
    SafeRelease(renderTargetSurface);

    D3DVIEWPORT9 viewport;
    viewport.X      = 0;
    viewport.Y      = 0;
    viewport.Width  = renderTarget->getWidth();
    viewport.Height = renderTarget->getHeight();
    mDevice->SetViewport(&viewport);

    mDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

    mDevice->Clear(0, nullptr, dxClearFlags, color, depth, stencil);

    markAllStateDirty();

    return angle::Result::Continue;
}

bool Renderer9::canSelectViewInVertexShader() const
{
    return false;
}

// For each Direct3D sampler of either the pixel or vertex stage,
// looks up the corresponding OpenGL texture image unit and texture type,
// and sets the texture and its addressing/filtering state (or NULL when inactive).
// Sampler mapping needs to be up-to-date on the program object before this is called.
angle::Result Renderer9::applyTextures(const gl::Context *context, gl::ShaderType shaderType)
{
    const auto &glState = context->getState();
    const auto &caps    = context->getCaps();
    ProgramExecutableD3D *executableD3D =
        GetImplAs<ProgramExecutableD3D>(glState.getProgramExecutable());

    ASSERT(!executableD3D->isSamplerMappingDirty());

    // TODO(jmadill): Use the Program's sampler bindings.
    const gl::ActiveTexturesCache &activeTextures = glState.getActiveTexturesCache();

    const gl::RangeUI samplerRange = executableD3D->getUsedSamplerRange(shaderType);
    for (unsigned int samplerIndex = samplerRange.low(); samplerIndex < samplerRange.high();
         samplerIndex++)
    {
        GLint textureUnit = executableD3D->getSamplerMapping(shaderType, samplerIndex, caps);
        ASSERT(textureUnit != -1);
        gl::Texture *texture = activeTextures[textureUnit];

        // A nullptr texture indicates incomplete.
        if (texture)
        {
            gl::Sampler *samplerObject = glState.getSampler(textureUnit);

            const gl::SamplerState &samplerState =
                samplerObject ? samplerObject->getSamplerState() : texture->getSamplerState();

            ANGLE_TRY(setSamplerState(context, shaderType, samplerIndex, texture, samplerState));
            ANGLE_TRY(setTexture(context, shaderType, samplerIndex, texture));
        }
        else
        {
            gl::TextureType textureType =
                executableD3D->getSamplerTextureType(shaderType, samplerIndex);

            // Texture is not sampler complete or it is in use by the framebuffer.  Bind the
            // incomplete texture.
            gl::Texture *incompleteTexture = nullptr;
            ANGLE_TRY(getIncompleteTexture(context, textureType, &incompleteTexture));
            ANGLE_TRY(setSamplerState(context, shaderType, samplerIndex, incompleteTexture,
                                      incompleteTexture->getSamplerState()));
            ANGLE_TRY(setTexture(context, shaderType, samplerIndex, incompleteTexture));
        }
    }

    // Set all the remaining textures to NULL
    int samplerCount = (shaderType == gl::ShaderType::Fragment)
                           ? caps.maxShaderTextureImageUnits[gl::ShaderType::Fragment]
                           : caps.maxShaderTextureImageUnits[gl::ShaderType::Vertex];

    // TODO(jmadill): faster way?
    for (int samplerIndex = samplerRange.high(); samplerIndex < samplerCount; samplerIndex++)
    {
        ANGLE_TRY(setTexture(context, shaderType, samplerIndex, nullptr));
    }

    return angle::Result::Continue;
}

angle::Result Renderer9::applyTextures(const gl::Context *context)
{
    ANGLE_TRY(applyTextures(context, gl::ShaderType::Vertex));
    ANGLE_TRY(applyTextures(context, gl::ShaderType::Fragment));
    return angle::Result::Continue;
}

angle::Result Renderer9::getIncompleteTexture(const gl::Context *context,
                                              gl::TextureType type,
                                              gl::Texture **textureOut)
{
    return GetImplAs<Context9>(context)->getIncompleteTexture(context, type, textureOut);
}

angle::Result Renderer9::ensureVertexDataManagerInitialized(const gl::Context *context)
{
    if (!mVertexDataManager)
    {
        mVertexDataManager = new VertexDataManager(this);
        ANGLE_TRY(mVertexDataManager->initialize(context));
    }

    return angle::Result::Continue;
}

std::string Renderer9::getVendorString() const
{
    return GetVendorString(getVendorId());
}

std::string Renderer9::getVersionString(bool includeFullVersion) const
{
    std::ostringstream versionString;
    std::string driverName(mAdapterIdentifier.Driver);
    if (!driverName.empty())
    {
        versionString << mAdapterIdentifier.Driver;
    }
    else
    {
        versionString << "D3D9";
    }

    if (includeFullVersion)
    {
        versionString << " -";
        versionString << GetDriverVersionString(mAdapterIdentifier.DriverVersion);
    }

    return versionString.str();
}

RendererD3D *CreateRenderer9(egl::Display *display)
{
    return new Renderer9(display);
}

}  // namespace rx
