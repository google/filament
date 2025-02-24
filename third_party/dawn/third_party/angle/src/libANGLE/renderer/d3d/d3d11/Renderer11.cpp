//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Renderer11.cpp: Implements a back-end specific class for the D3D11 renderer.

#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"

#include <EGL/eglext.h>
#include <sstream>

#include "anglebase/no_destructor.h"
#include "common/SimpleMutex.h"
#include "common/debug.h"
#include "common/tls.h"
#include "common/utilities.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/Program.h"
#include "libANGLE/State.h"
#include "libANGLE/Surface.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/histogram_macros.h"
#include "libANGLE/renderer/d3d/CompilerD3D.h"
#include "libANGLE/renderer/d3d/DisplayD3D.h"
#include "libANGLE/renderer/d3d/FramebufferD3D.h"
#include "libANGLE/renderer/d3d/IndexDataManager.h"
#include "libANGLE/renderer/d3d/RenderbufferD3D.h"
#include "libANGLE/renderer/d3d/ShaderD3D.h"
#include "libANGLE/renderer/d3d/SurfaceD3D.h"
#include "libANGLE/renderer/d3d/TextureD3D.h"
#include "libANGLE/renderer/d3d/VertexDataManager.h"
#include "libANGLE/renderer/d3d/d3d11/Blit11.h"
#include "libANGLE/renderer/d3d/d3d11/Buffer11.h"
#include "libANGLE/renderer/d3d/d3d11/Clear11.h"
#include "libANGLE/renderer/d3d/d3d11/Context11.h"
#include "libANGLE/renderer/d3d/d3d11/Device11.h"
#include "libANGLE/renderer/d3d/d3d11/ExternalImageSiblingImpl11.h"
#include "libANGLE/renderer/d3d/d3d11/Fence11.h"
#include "libANGLE/renderer/d3d/d3d11/Framebuffer11.h"
#include "libANGLE/renderer/d3d/d3d11/Image11.h"
#include "libANGLE/renderer/d3d/d3d11/IndexBuffer11.h"
#include "libANGLE/renderer/d3d/d3d11/PixelTransfer11.h"
#include "libANGLE/renderer/d3d/d3d11/Program11.h"
#include "libANGLE/renderer/d3d/d3d11/Query11.h"
#include "libANGLE/renderer/d3d/d3d11/RenderTarget11.h"
#include "libANGLE/renderer/d3d/d3d11/ShaderExecutable11.h"
#include "libANGLE/renderer/d3d/d3d11/StreamProducerD3DTexture.h"
#include "libANGLE/renderer/d3d/d3d11/SwapChain11.h"
#include "libANGLE/renderer/d3d/d3d11/TextureStorage11.h"
#include "libANGLE/renderer/d3d/d3d11/TransformFeedback11.h"
#include "libANGLE/renderer/d3d/d3d11/Trim11.h"
#include "libANGLE/renderer/d3d/d3d11/VertexArray11.h"
#include "libANGLE/renderer/d3d/d3d11/VertexBuffer11.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include "libANGLE/renderer/d3d/d3d11/texture_format_table.h"
#include "libANGLE/renderer/d3d/driver_utils_d3d.h"
#include "libANGLE/renderer/driver_utils.h"
#include "libANGLE/renderer/dxgi_support_table.h"
#include "libANGLE/renderer/renderer_utils.h"
#include "libANGLE/trace.h"

#ifdef ANGLE_ENABLE_WINDOWS_UWP
#    include "libANGLE/renderer/d3d/d3d11/winrt/NativeWindow11WinRT.h"
#else
#    include "libANGLE/renderer/d3d/d3d11/win32/NativeWindow11Win32.h"
#endif

#ifdef ANGLE_ENABLE_D3D11_COMPOSITOR_NATIVE_WINDOW
#    include "libANGLE/renderer/d3d/d3d11/converged/CompositorNativeWindow11.h"
#endif

// Enable ANGLE_SKIP_DXGI_1_2_CHECK if there is not a possibility of using cross-process
// HWNDs or the Windows 7 Platform Update (KB2670838) is expected to be installed.
#ifndef ANGLE_SKIP_DXGI_1_2_CHECK
#    define ANGLE_SKIP_DXGI_1_2_CHECK 0
#endif

namespace rx
{

namespace
{

enum
{
    MAX_TEXTURE_IMAGE_UNITS_VTF_SM4 = 16
};

enum ANGLEFeatureLevel
{
    ANGLE_FEATURE_LEVEL_INVALID,
    ANGLE_FEATURE_LEVEL_9_3,
    ANGLE_FEATURE_LEVEL_10_0,
    ANGLE_FEATURE_LEVEL_10_1,
    ANGLE_FEATURE_LEVEL_11_0,
    ANGLE_FEATURE_LEVEL_11_1,
    NUM_ANGLE_FEATURE_LEVELS
};

ANGLEFeatureLevel GetANGLEFeatureLevel(D3D_FEATURE_LEVEL d3dFeatureLevel)
{
    switch (d3dFeatureLevel)
    {
        case D3D_FEATURE_LEVEL_9_3:
            return ANGLE_FEATURE_LEVEL_9_3;
        case D3D_FEATURE_LEVEL_10_0:
            return ANGLE_FEATURE_LEVEL_10_0;
        case D3D_FEATURE_LEVEL_10_1:
            return ANGLE_FEATURE_LEVEL_10_1;
        case D3D_FEATURE_LEVEL_11_0:
            return ANGLE_FEATURE_LEVEL_11_0;
        case D3D_FEATURE_LEVEL_11_1:
            return ANGLE_FEATURE_LEVEL_11_1;
        default:
            return ANGLE_FEATURE_LEVEL_INVALID;
    }
}

void SetLineLoopIndices(GLuint *dest, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        dest[i] = static_cast<GLuint>(i);
    }
    dest[count] = 0;
}

template <typename T>
void CopyLineLoopIndices(const void *indices, GLuint *dest, size_t count)
{
    const T *srcPtr = static_cast<const T *>(indices);
    for (size_t i = 0; i < count; ++i)
    {
        dest[i] = static_cast<GLuint>(srcPtr[i]);
    }
    dest[count] = static_cast<GLuint>(srcPtr[0]);
}

void SetTriangleFanIndices(GLuint *destPtr, size_t numTris)
{
    for (size_t i = 0; i < numTris; i++)
    {
        destPtr[i * 3 + 0] = 0;
        destPtr[i * 3 + 1] = static_cast<GLuint>(i) + 1;
        destPtr[i * 3 + 2] = static_cast<GLuint>(i) + 2;
    }
}

void GetLineLoopIndices(const void *indices,
                        gl::DrawElementsType indexType,
                        GLuint count,
                        bool usePrimitiveRestartFixedIndex,
                        std::vector<GLuint> *bufferOut)
{
    if (indexType != gl::DrawElementsType::InvalidEnum && usePrimitiveRestartFixedIndex)
    {
        size_t indexCount = GetLineLoopWithRestartIndexCount(indexType, count,
                                                             static_cast<const uint8_t *>(indices));
        bufferOut->resize(indexCount);
        switch (indexType)
        {
            case gl::DrawElementsType::UnsignedByte:
                CopyLineLoopIndicesWithRestart<GLubyte, GLuint>(
                    count, static_cast<const uint8_t *>(indices),
                    reinterpret_cast<uint8_t *>(bufferOut->data()));
                break;
            case gl::DrawElementsType::UnsignedShort:
                CopyLineLoopIndicesWithRestart<GLushort, GLuint>(
                    count, static_cast<const uint8_t *>(indices),
                    reinterpret_cast<uint8_t *>(bufferOut->data()));
                break;
            case gl::DrawElementsType::UnsignedInt:
                CopyLineLoopIndicesWithRestart<GLuint, GLuint>(
                    count, static_cast<const uint8_t *>(indices),
                    reinterpret_cast<uint8_t *>(bufferOut->data()));
                break;
            default:
                UNREACHABLE();
                break;
        }
        return;
    }

    // For non-primitive-restart draws, the index count is static.
    bufferOut->resize(static_cast<size_t>(count) + 1);

    switch (indexType)
    {
        // Non-indexed draw
        case gl::DrawElementsType::InvalidEnum:
            SetLineLoopIndices(&(*bufferOut)[0], count);
            break;
        case gl::DrawElementsType::UnsignedByte:
            CopyLineLoopIndices<GLubyte>(indices, &(*bufferOut)[0], count);
            break;
        case gl::DrawElementsType::UnsignedShort:
            CopyLineLoopIndices<GLushort>(indices, &(*bufferOut)[0], count);
            break;
        case gl::DrawElementsType::UnsignedInt:
            CopyLineLoopIndices<GLuint>(indices, &(*bufferOut)[0], count);
            break;
        default:
            UNREACHABLE();
            break;
    }
}

template <typename T>
void CopyTriangleFanIndices(const void *indices, GLuint *destPtr, size_t numTris)
{
    const T *srcPtr = static_cast<const T *>(indices);

    for (size_t i = 0; i < numTris; i++)
    {
        destPtr[i * 3 + 0] = static_cast<GLuint>(srcPtr[0]);
        destPtr[i * 3 + 1] = static_cast<GLuint>(srcPtr[i + 1]);
        destPtr[i * 3 + 2] = static_cast<GLuint>(srcPtr[i + 2]);
    }
}

template <typename T>
void CopyTriangleFanIndicesWithRestart(const void *indices,
                                       GLuint indexCount,
                                       gl::DrawElementsType indexType,
                                       std::vector<GLuint> *bufferOut)
{
    GLuint restartIndex    = gl::GetPrimitiveRestartIndex(indexType);
    GLuint d3dRestartIndex = gl::GetPrimitiveRestartIndex(gl::DrawElementsType::UnsignedInt);
    const T *srcPtr        = static_cast<const T *>(indices);
    Optional<GLuint> vertexA;
    Optional<GLuint> vertexB;

    bufferOut->clear();

    for (size_t indexIdx = 0; indexIdx < indexCount; ++indexIdx)
    {
        GLuint value = static_cast<GLuint>(srcPtr[indexIdx]);

        if (value == restartIndex)
        {
            bufferOut->push_back(d3dRestartIndex);
            vertexA.reset();
            vertexB.reset();
        }
        else
        {
            if (!vertexA.valid())
            {
                vertexA = value;
            }
            else if (!vertexB.valid())
            {
                vertexB = value;
            }
            else
            {
                bufferOut->push_back(vertexA.value());
                bufferOut->push_back(vertexB.value());
                bufferOut->push_back(value);
                vertexB = value;
            }
        }
    }
}

void GetTriFanIndices(const void *indices,
                      gl::DrawElementsType indexType,
                      GLuint count,
                      bool usePrimitiveRestartFixedIndex,
                      std::vector<GLuint> *bufferOut)
{
    if (indexType != gl::DrawElementsType::InvalidEnum && usePrimitiveRestartFixedIndex)
    {
        switch (indexType)
        {
            case gl::DrawElementsType::UnsignedByte:
                CopyTriangleFanIndicesWithRestart<GLubyte>(indices, count, indexType, bufferOut);
                break;
            case gl::DrawElementsType::UnsignedShort:
                CopyTriangleFanIndicesWithRestart<GLushort>(indices, count, indexType, bufferOut);
                break;
            case gl::DrawElementsType::UnsignedInt:
                CopyTriangleFanIndicesWithRestart<GLuint>(indices, count, indexType, bufferOut);
                break;
            default:
                UNREACHABLE();
                break;
        }
        return;
    }

    // For non-primitive-restart draws, the index count is static.
    GLuint numTris = count - 2;
    bufferOut->resize(numTris * 3);

    switch (indexType)
    {
        // Non-indexed draw
        case gl::DrawElementsType::InvalidEnum:
            SetTriangleFanIndices(&(*bufferOut)[0], numTris);
            break;
        case gl::DrawElementsType::UnsignedByte:
            CopyTriangleFanIndices<GLubyte>(indices, &(*bufferOut)[0], numTris);
            break;
        case gl::DrawElementsType::UnsignedShort:
            CopyTriangleFanIndices<GLushort>(indices, &(*bufferOut)[0], numTris);
            break;
        case gl::DrawElementsType::UnsignedInt:
            CopyTriangleFanIndices<GLuint>(indices, &(*bufferOut)[0], numTris);
            break;
        default:
            UNREACHABLE();
            break;
    }
}

bool IsArrayRTV(ID3D11RenderTargetView *rtv)
{
    D3D11_RENDER_TARGET_VIEW_DESC desc;
    rtv->GetDesc(&desc);
    if (desc.ViewDimension == D3D11_RTV_DIMENSION_TEXTURE1DARRAY &&
        desc.Texture1DArray.ArraySize > 1)
        return true;
    if (desc.ViewDimension == D3D11_RTV_DIMENSION_TEXTURE2DARRAY &&
        desc.Texture2DArray.ArraySize > 1)
        return true;
    if (desc.ViewDimension == D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY &&
        desc.Texture2DMSArray.ArraySize > 1)
        return true;
    return false;
}

GLsizei GetAdjustedInstanceCount(const ProgramExecutableD3D *executable, GLsizei instanceCount)
{
    if (!executable->getExecutable()->usesMultiview())
    {
        return instanceCount;
    }
    if (instanceCount == 0)
    {
        return executable->getExecutable()->getNumViews();
    }
    return executable->getExecutable()->getNumViews() * instanceCount;
}

const uint32_t ScratchMemoryBufferLifetime = 1000;

void PopulateFormatDeviceCaps(ID3D11Device *device,
                              DXGI_FORMAT format,
                              UINT *outSupport,
                              UINT *outMaxSamples)
{
    if (FAILED(device->CheckFormatSupport(format, outSupport)))
    {
        *outSupport = 0;
    }

    *outMaxSamples = 0;
    for (UINT sampleCount = 2; sampleCount <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; sampleCount *= 2)
    {
        UINT qualityCount = 0;
        if (FAILED(device->CheckMultisampleQualityLevels(format, sampleCount, &qualityCount)) ||
            qualityCount == 0)
        {
            break;
        }

        *outMaxSamples = sampleCount;
    }
}

angle::Result GetTextureD3DResourceFromStorageOrImage(const gl::Context *context,
                                                      TextureD3D *texture,
                                                      const gl::ImageIndex &index,
                                                      const TextureHelper11 **outResource,
                                                      UINT *outSubresource)
{
    // If the storage exists, use it. Otherwise, copy directly from the images to avoid
    // allocating a new storage.
    if (texture->hasStorage())
    {
        TextureStorage *storage = nullptr;
        ANGLE_TRY(texture->getNativeTexture(context, &storage));

        TextureStorage11 *storage11 = GetAs<TextureStorage11>(storage);
        ANGLE_TRY(storage11->getResource(context, outResource));
        ANGLE_TRY(storage11->getSubresourceIndex(context, index, outSubresource));
    }
    else
    {
        ImageD3D *image  = texture->getImage(index);
        Image11 *image11 = GetAs<Image11>(image);
        ANGLE_TRY(image11->getStagingTexture(context, outResource, outSubresource));
    }

    return angle::Result::Continue;
}

}  // anonymous namespace

Renderer11DeviceCaps::Renderer11DeviceCaps() = default;

Renderer11::Renderer11(egl::Display *display)
    : RendererD3D(display),
      mCreateDebugDevice(false),
      mStateCache(),
      mStateManager(this),
      mLastHistogramUpdateTime(
          ANGLEPlatformCurrent()->monotonicallyIncreasingTime(ANGLEPlatformCurrent())),
      mDebug(nullptr),
      mScratchMemoryBuffer(ScratchMemoryBufferLifetime)
{
    mLineLoopIB    = nullptr;
    mTriangleFanIB = nullptr;

    mBlit          = nullptr;
    mPixelTransfer = nullptr;

    mClear = nullptr;

    mTrim = nullptr;

    mRenderer11DeviceCaps.supportsClearView                      = false;
    mRenderer11DeviceCaps.supportsConstantBufferOffsets          = false;
    mRenderer11DeviceCaps.supportsVpRtIndexWriteFromVertexShader = false;
    mRenderer11DeviceCaps.supportsDXGI1_2                        = false;
    mRenderer11DeviceCaps.allowES3OnFL10_0                       = false;
    mRenderer11DeviceCaps.supportsTypedUAVLoadAdditionalFormats  = false;
    mRenderer11DeviceCaps.supportsUAVLoadStoreCommonFormats      = false;
    mRenderer11DeviceCaps.supportsRasterizerOrderViews           = false;
    mRenderer11DeviceCaps.B5G6R5support                          = 0;
    mRenderer11DeviceCaps.B4G4R4A4support                        = 0;
    mRenderer11DeviceCaps.B5G5R5A1support                        = 0;

    mD3d11Module          = nullptr;
    mD3d12Module          = nullptr;
    mDCompModule          = nullptr;
    mCreatedWithDeviceEXT = false;

    ZeroMemory(&mAdapterDescription, sizeof(mAdapterDescription));

    const auto &attributes = mDisplay->getAttributeMap();

    if (mDisplay->getPlatform() == EGL_PLATFORM_ANGLE_ANGLE)
    {
        EGLint requestedMajorVersion = static_cast<EGLint>(
            attributes.get(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, EGL_DONT_CARE));
        EGLint requestedMinorVersion = static_cast<EGLint>(
            attributes.get(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, EGL_DONT_CARE));

        if (requestedMajorVersion == EGL_DONT_CARE || requestedMajorVersion >= 11)
        {
            if (requestedMinorVersion == EGL_DONT_CARE || requestedMinorVersion >= 1)
            {
                // This could potentially lead to failed context creation if done on a system
                // without the platform update which installs DXGI 1.2. Currently, for Chrome users
                // D3D11 contexts are only created if the platform update is available, so this
                // should not cause any issues.
                mAvailableFeatureLevels.push_back(D3D_FEATURE_LEVEL_11_1);
            }
            if (requestedMinorVersion == EGL_DONT_CARE || requestedMinorVersion >= 0)
            {
                mAvailableFeatureLevels.push_back(D3D_FEATURE_LEVEL_11_0);
            }
        }

        if (requestedMajorVersion == EGL_DONT_CARE || requestedMajorVersion >= 10)
        {
            if (requestedMinorVersion == EGL_DONT_CARE || requestedMinorVersion >= 1)
            {
                mAvailableFeatureLevels.push_back(D3D_FEATURE_LEVEL_10_1);
            }
            if (requestedMinorVersion == EGL_DONT_CARE || requestedMinorVersion >= 0)
            {
                mAvailableFeatureLevels.push_back(D3D_FEATURE_LEVEL_10_0);
            }
        }

        EGLint requestedDeviceType = static_cast<EGLint>(attributes.get(
            EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE));
        switch (requestedDeviceType)
        {
            case EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE:
                mRequestedDriverType = D3D_DRIVER_TYPE_HARDWARE;
                break;

            case EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_WARP_ANGLE:
                mRequestedDriverType = D3D_DRIVER_TYPE_WARP;
                break;

            case EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_REFERENCE_ANGLE:
                mRequestedDriverType = D3D_DRIVER_TYPE_REFERENCE;
                break;

            case EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE:
                mRequestedDriverType = D3D_DRIVER_TYPE_NULL;
                break;

            default:
                UNREACHABLE();
        }

        mCreateDebugDevice = ShouldUseDebugLayers(attributes);
    }
    else if (mDisplay->getPlatform() == EGL_PLATFORM_DEVICE_EXT)
    {
        ASSERT(mDisplay->getDevice() != nullptr);
        mCreatedWithDeviceEXT = true;

        // Also set EGL_PLATFORM_ANGLE_ANGLE variables, in case they're used elsewhere in ANGLE
        // mAvailableFeatureLevels defaults to empty
        mRequestedDriverType = D3D_DRIVER_TYPE_UNKNOWN;
    }

    const EGLenum presentPath = static_cast<EGLenum>(attributes.get(
        EGL_EXPERIMENTAL_PRESENT_PATH_ANGLE, EGL_EXPERIMENTAL_PRESENT_PATH_COPY_ANGLE));
    mPresentPathFastEnabled   = (presentPath == EGL_EXPERIMENTAL_PRESENT_PATH_FAST_ANGLE);
}

Renderer11::~Renderer11()
{
    release();
}

#ifndef __d3d11_1_h__
#    define D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET ((D3D11_MESSAGE_ID)3146081)
#endif

egl::Error Renderer11::initialize()
{
    HRESULT result = S_OK;

    ANGLE_TRY(initializeDXGIAdapter());
    ANGLE_TRY(initializeD3DDevice());

#if !defined(ANGLE_ENABLE_WINDOWS_UWP)
#    if !ANGLE_SKIP_DXGI_1_2_CHECK
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "Renderer11::initialize (DXGICheck)");
        // In order to create a swap chain for an HWND owned by another process, DXGI 1.2 is
        // required.
        // The easiest way to check is to query for a IDXGIDevice2.
        bool requireDXGI1_2 = false;
        HWND hwnd           = WindowFromDC(mDisplay->getNativeDisplayId());
        if (hwnd)
        {
            DWORD currentProcessId = GetCurrentProcessId();
            DWORD wndProcessId;
            GetWindowThreadProcessId(hwnd, &wndProcessId);
            requireDXGI1_2 = (currentProcessId != wndProcessId);
        }
        else
        {
            requireDXGI1_2 = true;
        }

        if (requireDXGI1_2)
        {
            angle::ComPtr<IDXGIDevice2> dxgiDevice2;
            result = mDevice.As(&dxgiDevice2);
            if (FAILED(result))
            {
                return egl::EglNotInitialized(D3D11_INIT_INCOMPATIBLE_DXGI)
                       << "DXGI 1.2 required to present to HWNDs owned by another process.";
            }
        }
    }
#    endif
#endif

    {
        ANGLE_TRACE_EVENT0("gpu.angle", "Renderer11::initialize (ComQueries)");
        // Query the DeviceContext for the DeviceContext1 and DeviceContext3 interfaces.
        // This could fail on Windows 7 without the Platform Update.
        // Don't error in this case- just don't use mDeviceContext1 or mDeviceContext3.
        mDeviceContext.As(&mDeviceContext1);
        mDeviceContext.As(&mDeviceContext3);

        angle::ComPtr<IDXGIAdapter2> dxgiAdapter2;
        mDxgiAdapter.As(&dxgiAdapter2);

        // On D3D_FEATURE_LEVEL_9_*, IDXGIAdapter::GetDesc returns "Software Adapter" for the
        // description string.
        // If DXGI1.2 is available then IDXGIAdapter2::GetDesc2 can be used to get the actual
        // hardware values.
        if (mRenderer11DeviceCaps.featureLevel <= D3D_FEATURE_LEVEL_9_3 && dxgiAdapter2 != nullptr)
        {
            DXGI_ADAPTER_DESC2 adapterDesc2 = {};
            result                          = dxgiAdapter2->GetDesc2(&adapterDesc2);
            if (SUCCEEDED(result))
            {
                // Copy the contents of the DXGI_ADAPTER_DESC2 into mAdapterDescription (a
                // DXGI_ADAPTER_DESC).
                memcpy(mAdapterDescription.Description, adapterDesc2.Description,
                       sizeof(mAdapterDescription.Description));
                mAdapterDescription.VendorId              = adapterDesc2.VendorId;
                mAdapterDescription.DeviceId              = adapterDesc2.DeviceId;
                mAdapterDescription.SubSysId              = adapterDesc2.SubSysId;
                mAdapterDescription.Revision              = adapterDesc2.Revision;
                mAdapterDescription.DedicatedVideoMemory  = adapterDesc2.DedicatedVideoMemory;
                mAdapterDescription.DedicatedSystemMemory = adapterDesc2.DedicatedSystemMemory;
                mAdapterDescription.SharedSystemMemory    = adapterDesc2.SharedSystemMemory;
                mAdapterDescription.AdapterLuid           = adapterDesc2.AdapterLuid;
            }
        }
        else
        {
            result = mDxgiAdapter->GetDesc(&mAdapterDescription);
        }

        if (FAILED(result))
        {
            return egl::EglNotInitialized(D3D11_INIT_OTHER_ERROR)
                   << "Could not read DXGI adaptor description.";
        }

        memset(mDescription, 0, sizeof(mDescription));
        wcstombs(mDescription, mAdapterDescription.Description, sizeof(mDescription) - 1);

        result = mDxgiAdapter->GetParent(IID_PPV_ARGS(&mDxgiFactory));

        if (!mDxgiFactory || FAILED(result))
        {
            return egl::EglNotInitialized(D3D11_INIT_OTHER_ERROR)
                   << "Could not create DXGI factory.";
        }
    }

    // Disable some spurious D3D11 debug warnings to prevent them from flooding the output log
    if (mCreateDebugDevice)
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "Renderer11::initialize (HideWarnings)");
        angle::ComPtr<ID3D11InfoQueue> infoQueue;
        result = mDevice.As(&infoQueue);

        if (SUCCEEDED(result))
        {
            D3D11_MESSAGE_ID hideMessages[] = {
                D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET,

                // Robust access behaviour makes out of bounds messages safe
                D3D11_MESSAGE_ID_DEVICE_DRAW_VERTEX_BUFFER_TOO_SMALL,
            };

            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs         = static_cast<unsigned int>(ArraySize(hideMessages));
            filter.DenyList.pIDList        = hideMessages;

            infoQueue->AddStorageFilterEntries(&filter);
        }
    }

#if !defined(NDEBUG)
    mDevice.As(&mDebug);
#endif

    ANGLE_TRY(initializeDevice());

    return egl::NoError();
}

HRESULT Renderer11::callD3D11CreateDevice(PFN_D3D11_CREATE_DEVICE createDevice, bool debug)
{
    // If adapter is not nullptr, the driver type must be D3D_DRIVER_TYPE_UNKNOWN or
    // D3D11CreateDevice will return E_INVALIDARG.
    return createDevice(
        mDxgiAdapter.Get(), mDxgiAdapter ? D3D_DRIVER_TYPE_UNKNOWN : mRequestedDriverType, nullptr,
        debug ? D3D11_CREATE_DEVICE_DEBUG : 0, mAvailableFeatureLevels.data(),
        static_cast<unsigned int>(mAvailableFeatureLevels.size()), D3D11_SDK_VERSION, &mDevice,
        &(mRenderer11DeviceCaps.featureLevel), &mDeviceContext);
}

egl::Error Renderer11::initializeDXGIAdapter()
{
    if (mCreatedWithDeviceEXT)
    {
        ASSERT(mRequestedDriverType == D3D_DRIVER_TYPE_UNKNOWN);

        Device11 *device11 = GetImplAs<Device11>(mDisplay->getDevice());
        ASSERT(device11 != nullptr);

        ID3D11Device *d3dDevice = device11->getDevice();
        if (FAILED(d3dDevice->GetDeviceRemovedReason()))
        {
            return egl::EglNotInitialized() << "Inputted D3D11 device has been lost.";
        }

        if (d3dDevice->GetFeatureLevel() < D3D_FEATURE_LEVEL_9_3)
        {
            return egl::EglNotInitialized()
                   << "Inputted D3D11 device must be Feature Level 9_3 or greater.";
        }

        // The Renderer11 adds a ref to the inputted D3D11 device, like D3D11CreateDevice does.
        mDevice = d3dDevice;
        mDevice->GetImmediateContext(&mDeviceContext);
        mRenderer11DeviceCaps.featureLevel = mDevice->GetFeatureLevel();

        return initializeAdapterFromDevice();
    }
    else
    {
        angle::ComPtr<IDXGIFactory1> factory;
        HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
        if (FAILED(hr))
        {
            return egl::EglNotInitialized(D3D11_INIT_OTHER_ERROR)
                   << "Could not create DXGI factory";
        }

        // If the developer requests a specific adapter, honor their request regardless of the value
        // of EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE.
        const egl::AttributeMap &attributes = mDisplay->getAttributeMap();
        // Check EGL_ANGLE_platform_angle_d3d_luid
        long high = static_cast<long>(attributes.get(EGL_PLATFORM_ANGLE_D3D_LUID_HIGH_ANGLE, 0));
        unsigned long low =
            static_cast<unsigned long>(attributes.get(EGL_PLATFORM_ANGLE_D3D_LUID_LOW_ANGLE, 0));
        // Check EGL_ANGLE_platform_angle_device_id
        if (high == 0 && low == 0)
        {
            high = static_cast<long>(attributes.get(EGL_PLATFORM_ANGLE_DEVICE_ID_HIGH_ANGLE, 0));
            low  = static_cast<unsigned long>(
                attributes.get(EGL_PLATFORM_ANGLE_DEVICE_ID_LOW_ANGLE, 0));
        }
        if (high != 0 || low != 0)
        {
            angle::ComPtr<IDXGIAdapter> temp;
            for (UINT i = 0; SUCCEEDED(factory->EnumAdapters(i, &temp)); i++)
            {
                DXGI_ADAPTER_DESC desc;
                if (SUCCEEDED(temp->GetDesc(&desc)))
                {
                    // EGL_ANGLE_platform_angle_d3d_luid
                    if (desc.AdapterLuid.HighPart == high && desc.AdapterLuid.LowPart == low)
                    {
                        mDxgiAdapter = std::move(temp);
                        break;
                    }

                    // EGL_ANGLE_platform_angle_device_id
                    // NOTE: If there are multiple GPUs with the same PCI
                    // vendor and device IDs, this will arbitrarily choose one
                    // of them. To select a specific GPU, use the LUID instead.
                    if ((high == 0 || desc.VendorId == static_cast<UINT>(high)) &&
                        (low == 0 || desc.DeviceId == static_cast<UINT>(low)))
                    {
                        mDxgiAdapter = std::move(temp);
                        break;
                    }
                }
            }
        }

        // For requested driver types besides Hardware such as Warp, Reference, or Null
        // allow D3D11CreateDevice to pick the adapter by passing it the driver type.
        if (!mDxgiAdapter && mRequestedDriverType == D3D_DRIVER_TYPE_HARDWARE)
        {
            hr = factory->EnumAdapters(0, &mDxgiAdapter);
            if (FAILED(hr))
            {
                return egl::EglNotInitialized(D3D11_INIT_OTHER_ERROR)
                       << "Could not retrieve DXGI adapter";
            }
        }
    }
    return egl::NoError();
}

egl::Error Renderer11::initializeAdapterFromDevice()
{
    ASSERT(mDevice);
    ASSERT(!mDxgiAdapter);

    angle::ComPtr<IDXGIDevice> dxgiDevice;
    HRESULT result = mDevice.As(&dxgiDevice);
    if (FAILED(result))
    {
        return egl::EglNotInitialized(D3D11_INIT_OTHER_ERROR) << "Could not query DXGI device.";
    }

    result = dxgiDevice->GetParent(IID_PPV_ARGS(&mDxgiAdapter));
    if (FAILED(result))
    {
        return egl::EglNotInitialized(D3D11_INIT_OTHER_ERROR) << "Could not retrieve DXGI adapter";
    }

    return egl::NoError();
}

HRESULT Renderer11::callD3D11On12CreateDevice(PFN_D3D12_CREATE_DEVICE createDevice12,
                                              PFN_D3D11ON12_CREATE_DEVICE createDevice11on12,
                                              bool debug)
{
    HRESULT result = S_OK;
    if (mDxgiAdapter)
    {
        // Passing nullptr into pAdapter chooses the default adapter which will be the hardware
        // adapter if it exists.
        result = createDevice12(mDxgiAdapter.Get(), mAvailableFeatureLevels[0],
                                IID_PPV_ARGS(&mDevice12));
    }
    else if (mRequestedDriverType == D3D_DRIVER_TYPE_WARP)
    {
        angle::ComPtr<IDXGIFactory4> factory;
        result = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
        if (FAILED(result))
        {
            return result;
        }

        result = factory->EnumWarpAdapter(IID_PPV_ARGS(&mDxgiAdapter));
        if (SUCCEEDED(result))
        {
            result = createDevice12(mDxgiAdapter.Get(), mAvailableFeatureLevels[0],
                                    IID_PPV_ARGS(&mDevice12));
        }
    }
    else
    {
        ASSERT(mRequestedDriverType == D3D_DRIVER_TYPE_REFERENCE ||
               mRequestedDriverType == D3D_DRIVER_TYPE_NULL);
        result = E_INVALIDARG;
    }

    if (SUCCEEDED(result))
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;
        result = mDevice12->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue));
    }

    if (SUCCEEDED(result))
    {
        result = createDevice11on12(
            mDevice12.Get(), debug ? D3D11_CREATE_DEVICE_DEBUG : 0, mAvailableFeatureLevels.data(),
            static_cast<unsigned int>(mAvailableFeatureLevels.size()),
            reinterpret_cast<IUnknown **>(mCommandQueue.GetAddressOf()), 1 /* NumQueues */,
            0 /* NodeMask */, &mDevice, &mDeviceContext, &(mRenderer11DeviceCaps.featureLevel));
    }

    return result;
}

egl::Error Renderer11::initializeD3DDevice()
{
    HRESULT result             = S_OK;
    bool createD3D11on12Device = false;

    if (!mCreatedWithDeviceEXT)
    {
#if !defined(ANGLE_ENABLE_WINDOWS_UWP)
        PFN_D3D11_CREATE_DEVICE D3D11CreateDevice         = nullptr;
        PFN_D3D12_CREATE_DEVICE D3D12CreateDevice         = nullptr;
        PFN_D3D11ON12_CREATE_DEVICE D3D11On12CreateDevice = nullptr;
        {
            ANGLE_TRACE_EVENT0("gpu.angle", "Renderer11::initialize (Load DLLs)");
            mD3d11Module = LoadLibrary(TEXT("d3d11.dll"));
            mDCompModule = LoadLibrary(TEXT("dcomp.dll"));

            // create the D3D11 device
            ASSERT(mDevice == nullptr);

            const egl::AttributeMap &attributes = mDisplay->getAttributeMap();
            createD3D11on12Device =
                attributes.get(EGL_PLATFORM_ANGLE_D3D11ON12_ANGLE, EGL_FALSE) == EGL_TRUE;

            if (createD3D11on12Device)
            {
                mD3d12Module = LoadLibrary(TEXT("d3d12.dll"));
                if (mD3d12Module == nullptr)
                {
                    return egl::EglNotInitialized(D3D11_INIT_MISSING_DEP)
                           << "Could not load D3D12 library.";
                }

                D3D12CreateDevice = reinterpret_cast<PFN_D3D12_CREATE_DEVICE>(
                    GetProcAddress(mD3d12Module, "D3D12CreateDevice"));
                if (D3D12CreateDevice == nullptr)
                {
                    return egl::EglNotInitialized(D3D11_INIT_MISSING_DEP)
                           << "Could not retrieve D3D12CreateDevice address.";
                }

                D3D11On12CreateDevice = reinterpret_cast<PFN_D3D11ON12_CREATE_DEVICE>(
                    GetProcAddress(mD3d11Module, "D3D11On12CreateDevice"));
                if (D3D11On12CreateDevice == nullptr)
                {
                    return egl::EglNotInitialized(D3D11_INIT_MISSING_DEP)
                           << "Could not retrieve D3D11On12CreateDevice address.";
                }
            }
            else
            {
                if (mD3d11Module == nullptr)
                {
                    return egl::EglNotInitialized(D3D11_INIT_MISSING_DEP)
                           << "Could not load D3D11 library.";
                }

                D3D11CreateDevice = reinterpret_cast<PFN_D3D11_CREATE_DEVICE>(
                    GetProcAddress(mD3d11Module, "D3D11CreateDevice"));

                if (D3D11CreateDevice == nullptr)
                {
                    return egl::EglNotInitialized(D3D11_INIT_MISSING_DEP)
                           << "Could not retrieve D3D11CreateDevice address.";
                }
            }
        }
#endif

        if (mCreateDebugDevice)
        {
            ANGLE_TRACE_EVENT0("gpu.angle", "D3D11CreateDevice (Debug)");
            if (createD3D11on12Device)
            {
                result = callD3D11On12CreateDevice(D3D12CreateDevice, D3D11On12CreateDevice, true);
            }
            else
            {
                result = callD3D11CreateDevice(D3D11CreateDevice, true);
            }

            if (result == E_INVALIDARG && mAvailableFeatureLevels.size() > 1u &&
                mAvailableFeatureLevels[0] == D3D_FEATURE_LEVEL_11_1)
            {
                // On older Windows platforms, D3D11.1 is not supported which returns E_INVALIDARG.
                // Try again without passing D3D_FEATURE_LEVEL_11_1 in case we have other feature
                // levels to fall back on.
                mAvailableFeatureLevels.erase(mAvailableFeatureLevels.begin());
                if (createD3D11on12Device)
                {
                    result =
                        callD3D11On12CreateDevice(D3D12CreateDevice, D3D11On12CreateDevice, true);
                }
                else
                {
                    result = callD3D11CreateDevice(D3D11CreateDevice, true);
                }
            }

            if (!mDevice || FAILED(result))
            {
                WARN() << "Failed creating Debug D3D11 device - falling back to release runtime.";
            }
        }

        if (!mDevice || FAILED(result))
        {
            ANGLE_TRACE_EVENT0("gpu.angle", "D3D11CreateDevice");
            if (createD3D11on12Device)
            {
                result = callD3D11On12CreateDevice(D3D12CreateDevice, D3D11On12CreateDevice, false);
            }
            else
            {
                result = callD3D11CreateDevice(D3D11CreateDevice, false);
            }

            if (result == E_INVALIDARG && mAvailableFeatureLevels.size() > 1u &&
                mAvailableFeatureLevels[0] == D3D_FEATURE_LEVEL_11_1)
            {
                // On older Windows platforms, D3D11.1 is not supported which returns E_INVALIDARG.
                // Try again without passing D3D_FEATURE_LEVEL_11_1 in case we have other feature
                // levels to fall back on.
                mAvailableFeatureLevels.erase(mAvailableFeatureLevels.begin());
                if (createD3D11on12Device)
                {
                    result =
                        callD3D11On12CreateDevice(D3D12CreateDevice, D3D11On12CreateDevice, false);
                }
                else
                {
                    result = callD3D11CreateDevice(D3D11CreateDevice, false);
                }
            }

            // Cleanup done by destructor
            if (!mDevice || FAILED(result))
            {
                ANGLE_HISTOGRAM_SPARSE_SLOWLY("GPU.ANGLE.D3D11CreateDeviceError",
                                              static_cast<int>(result));
                return egl::EglNotInitialized(D3D11_INIT_CREATEDEVICE_ERROR)
                       << "Could not create D3D11 device.";
            }
        }

        if (!mDxgiAdapter)
        {
            // If the D3D11CreateDevice was asked to create the adapter via mRequestedDriverType,
            // fill in the adapter here.
            ANGLE_TRY(initializeAdapterFromDevice());
        }
    }

    mResourceManager11.setAllocationsInitialized(mCreateDebugDevice);

    d3d11::SetDebugName(mDeviceContext, "DeviceContext", nullptr);

    mAnnotatorContext.initialize(mDeviceContext.Get());

    mDevice.As(&mDevice1);

    return egl::NoError();
}

void Renderer11::setGlobalDebugAnnotator()
{
    static angle::base::NoDestructor<angle::SimpleMutex> gMutex;
    static angle::base::NoDestructor<DebugAnnotator11> gGlobalAnnotator;

    std::lock_guard<angle::SimpleMutex> lg(*gMutex);
    gl::InitializeDebugAnnotations(gGlobalAnnotator.get());
}

// do any one-time device initialization
// NOTE: this is also needed after a device lost/reset
// to reset the scene status and ensure the default states are reset.
egl::Error Renderer11::initializeDevice()
{
    ANGLE_TRACE_EVENT0("gpu.angle", "Renderer11::initializeDevice");

    populateRenderer11DeviceCaps();

    mStateCache.clear();

    ASSERT(!mBlit);
    mBlit = new Blit11(this);

    ASSERT(!mClear);
    mClear = new Clear11(this);

    const auto &attributes = mDisplay->getAttributeMap();
    // If automatic trim is enabled, DXGIDevice3::Trim( ) is called for the application
    // automatically when an application is suspended by the OS. This feature is currently
    // only supported for Windows Store applications.
    EGLint enableAutoTrim = static_cast<EGLint>(
        attributes.get(EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_FALSE));

    if (enableAutoTrim == EGL_TRUE)
    {
        ASSERT(!mTrim);
        mTrim = new Trim11(this);
    }

    ASSERT(!mPixelTransfer);
    mPixelTransfer = new PixelTransfer11(this);

    // Gather stats on DXGI and D3D feature level
    ANGLE_HISTOGRAM_BOOLEAN("GPU.ANGLE.SupportsDXGI1_2", mRenderer11DeviceCaps.supportsDXGI1_2);

    ANGLEFeatureLevel angleFeatureLevel = GetANGLEFeatureLevel(mRenderer11DeviceCaps.featureLevel);

    // We don't actually request a 11_1 device, because of complications with the platform
    // update. Instead we check if the mDeviceContext1 pointer cast succeeded.
    // Note: we should support D3D11_0 always, but we aren't guaranteed to be at FL11_0
    // because the app can specify a lower version (such as 9_3) on Display creation.
    if (mDeviceContext1 != nullptr)
    {
        angleFeatureLevel = ANGLE_FEATURE_LEVEL_11_1;
    }

    ANGLE_HISTOGRAM_ENUMERATION("GPU.ANGLE.D3D11FeatureLevel", angleFeatureLevel,
                                NUM_ANGLE_FEATURE_LEVELS);

    return egl::NoError();
}

void Renderer11::populateRenderer11DeviceCaps()
{
    HRESULT hr = S_OK;

    LARGE_INTEGER version;
    hr = mDxgiAdapter->CheckInterfaceSupport(__uuidof(IDXGIDevice), &version);
    if (FAILED(hr))
    {
        mRenderer11DeviceCaps.driverVersion.reset();
        ERR() << "Error querying driver version from DXGI Adapter.";
    }
    else
    {
        mRenderer11DeviceCaps.driverVersion = version;
    }

    if (mDeviceContext1)
    {
        D3D11_FEATURE_DATA_D3D11_OPTIONS d3d11Options;
        HRESULT result = mDevice->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS, &d3d11Options,
                                                      sizeof(D3D11_FEATURE_DATA_D3D11_OPTIONS));
        if (SUCCEEDED(result))
        {
            mRenderer11DeviceCaps.supportsClearView = (d3d11Options.ClearView != FALSE);
            mRenderer11DeviceCaps.supportsConstantBufferOffsets =
                (d3d11Options.ConstantBufferOffsetting != FALSE);
        }
    }

    if (mDeviceContext3)
    {
        D3D11_FEATURE_DATA_D3D11_OPTIONS3 d3d11Options3;
        HRESULT result = mDevice->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS3, &d3d11Options3,
                                                      sizeof(D3D11_FEATURE_DATA_D3D11_OPTIONS3));
        if (SUCCEEDED(result))
        {
            mRenderer11DeviceCaps.supportsVpRtIndexWriteFromVertexShader =
                (d3d11Options3.VPAndRTArrayIndexFromAnyShaderFeedingRasterizer == TRUE);
        }
        D3D11_FEATURE_DATA_D3D11_OPTIONS2 d3d11Options2;
        result = mDevice->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS2, &d3d11Options2,
                                              sizeof(D3D11_FEATURE_DATA_D3D11_OPTIONS2));
        if (SUCCEEDED(result))
        {
            mRenderer11DeviceCaps.supportsTypedUAVLoadAdditionalFormats =
                d3d11Options2.TypedUAVLoadAdditionalFormats;
            // If ROVs are disabled for testing, also disable typed UAV loads to ensure we test the
            // bare bones codepath of typeless UAV.
            if (!getFeatures().disableRasterizerOrderViews.enabled)
            {
                if (mRenderer11DeviceCaps.supportsTypedUAVLoadAdditionalFormats)
                {
                    // TypedUAVLoadAdditionalFormats is true. Now check if we can both load and
                    // store the common additional formats. The common formats are supported in a
                    // set, so we only need to check one:
                    // https://learn.microsoft.com/en-us/windows/win32/direct3d11/typed-unordered-access-view-loads.
                    D3D11_FEATURE_DATA_FORMAT_SUPPORT2 d3d11Format2{};
                    d3d11Format2.InFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
                    result = mDevice->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT2,
                                                          &d3d11Format2, sizeof(d3d11Format2));
                    if (SUCCEEDED(result))
                    {
                        constexpr UINT loadStoreFlags = D3D11_FORMAT_SUPPORT2_UAV_TYPED_LOAD |
                                                        D3D11_FORMAT_SUPPORT2_UAV_TYPED_STORE;
                        mRenderer11DeviceCaps.supportsUAVLoadStoreCommonFormats =
                            (d3d11Format2.OutFormatSupport2 & loadStoreFlags) == loadStoreFlags;
                    }
                }
                mRenderer11DeviceCaps.supportsRasterizerOrderViews = d3d11Options2.ROVsSupported;
            }
        }
    }

    mRenderer11DeviceCaps.supportsMultisampledDepthStencilSRVs =
        mRenderer11DeviceCaps.featureLevel > D3D_FEATURE_LEVEL_10_0;

    if (getFeatures().disableB5G6R5Support.enabled)
    {
        mRenderer11DeviceCaps.B5G6R5support    = 0;
        mRenderer11DeviceCaps.B5G6R5maxSamples = 0;
    }
    else
    {
        PopulateFormatDeviceCaps(mDevice.Get(), DXGI_FORMAT_B5G6R5_UNORM,
                                 &mRenderer11DeviceCaps.B5G6R5support,
                                 &mRenderer11DeviceCaps.B5G6R5maxSamples);
    }

    if (getFeatures().allowES3OnFL100.enabled)
    {
        mRenderer11DeviceCaps.allowES3OnFL10_0 = true;
    }

    PopulateFormatDeviceCaps(mDevice.Get(), DXGI_FORMAT_B4G4R4A4_UNORM,
                             &mRenderer11DeviceCaps.B4G4R4A4support,
                             &mRenderer11DeviceCaps.B4G4R4A4maxSamples);
    PopulateFormatDeviceCaps(mDevice.Get(), DXGI_FORMAT_B5G5R5A1_UNORM,
                             &mRenderer11DeviceCaps.B5G5R5A1support,
                             &mRenderer11DeviceCaps.B5G5R5A1maxSamples);

    angle::ComPtr<IDXGIAdapter2> dxgiAdapter2;
    mDxgiAdapter.As(&dxgiAdapter2);
    mRenderer11DeviceCaps.supportsDXGI1_2 = (dxgiAdapter2 != nullptr);
}

gl::SupportedSampleSet Renderer11::generateSampleSetForEGLConfig(
    const gl::TextureCaps &colorBufferFormatCaps,
    const gl::TextureCaps &depthStencilBufferFormatCaps) const
{
    gl::SupportedSampleSet sampleCounts;

    // Generate a new set from the set intersection of sample counts between the color and depth
    // format caps.
    std::set_intersection(colorBufferFormatCaps.sampleCounts.begin(),
                          colorBufferFormatCaps.sampleCounts.end(),
                          depthStencilBufferFormatCaps.sampleCounts.begin(),
                          depthStencilBufferFormatCaps.sampleCounts.end(),
                          std::inserter(sampleCounts, sampleCounts.begin()));

    // Format of GL_NONE results in no supported sample counts.
    // Add back the color sample counts to the supported sample set.
    if (depthStencilBufferFormatCaps.sampleCounts.empty())
    {
        sampleCounts = colorBufferFormatCaps.sampleCounts;
    }
    else if (colorBufferFormatCaps.sampleCounts.empty())
    {
        // Likewise, add back the depth sample counts to the supported sample set.
        sampleCounts = depthStencilBufferFormatCaps.sampleCounts;
    }

    // Always support 0 samples
    sampleCounts.insert(0);

    return sampleCounts;
}

egl::ConfigSet Renderer11::generateConfigs()
{
    std::vector<GLenum> colorBufferFormats;

    // 32-bit supported formats
    colorBufferFormats.push_back(GL_BGRA8_EXT);
    colorBufferFormats.push_back(GL_RGBA8_OES);

    // 24-bit supported formats
    colorBufferFormats.push_back(GL_RGB8_OES);

    if (mRenderer11DeviceCaps.featureLevel >= D3D_FEATURE_LEVEL_10_0)
    {
        // Additional high bit depth formats added in D3D 10.0
        // https://msdn.microsoft.com/en-us/library/windows/desktop/bb173064.aspx
        colorBufferFormats.push_back(GL_RGBA16F);
        colorBufferFormats.push_back(GL_RGB10_A2);
    }

    if (!mPresentPathFastEnabled)
    {
        // 16-bit supported formats
        // These aren't valid D3D11 swapchain formats, so don't expose them as configs
        // if present path fast is active
        colorBufferFormats.push_back(GL_RGBA4);
        colorBufferFormats.push_back(GL_RGB5_A1);
        colorBufferFormats.push_back(GL_RGB565);
    }

    static const GLenum depthStencilBufferFormats[] = {
        GL_NONE,           GL_DEPTH24_STENCIL8_OES, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT16,
        GL_STENCIL_INDEX8,
    };

    const gl::Caps &rendererCaps                  = getNativeCaps();
    const gl::TextureCapsMap &rendererTextureCaps = getNativeTextureCaps();

    const EGLint optimalSurfaceOrientation =
        mPresentPathFastEnabled ? 0 : EGL_SURFACE_ORIENTATION_INVERT_Y_ANGLE;

    egl::ConfigSet configs;
    for (GLenum colorBufferInternalFormat : colorBufferFormats)
    {
        const gl::TextureCaps &colorBufferFormatCaps =
            rendererTextureCaps.get(colorBufferInternalFormat);
        if (!colorBufferFormatCaps.renderbuffer)
        {
            ASSERT(!colorBufferFormatCaps.textureAttachment);
            continue;
        }

        for (GLenum depthStencilBufferInternalFormat : depthStencilBufferFormats)
        {
            const gl::TextureCaps &depthStencilBufferFormatCaps =
                rendererTextureCaps.get(depthStencilBufferInternalFormat);
            if (!depthStencilBufferFormatCaps.renderbuffer &&
                depthStencilBufferInternalFormat != GL_NONE)
            {
                ASSERT(!depthStencilBufferFormatCaps.textureAttachment);
                continue;
            }

            const gl::InternalFormat &colorBufferFormatInfo =
                gl::GetSizedInternalFormatInfo(colorBufferInternalFormat);
            const gl::InternalFormat &depthStencilBufferFormatInfo =
                gl::GetSizedInternalFormatInfo(depthStencilBufferInternalFormat);
            const gl::Version &maxVersion = getMaxSupportedESVersion();

            const gl::SupportedSampleSet sampleCounts =
                generateSampleSetForEGLConfig(colorBufferFormatCaps, depthStencilBufferFormatCaps);

            for (GLuint sampleCount : sampleCounts)
            {
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
                config.bindToTextureRGB =
                    ((colorBufferFormatInfo.format == GL_RGB) && (sampleCount <= 1));
                config.bindToTextureRGBA = (((colorBufferFormatInfo.format == GL_RGBA) ||
                                             (colorBufferFormatInfo.format == GL_BGRA_EXT)) &&
                                            (sampleCount <= 1));
                config.colorBufferType   = EGL_RGB_BUFFER;
                config.configCaveat      = EGL_NONE;
                config.configID          = static_cast<EGLint>(configs.size() + 1);

                // PresentPathFast may not be conformant
                config.conformant = 0;
                if (!mPresentPathFastEnabled)
                {
                    // Can only support a conformant ES2 with feature level greater than 10.0.
                    if (mRenderer11DeviceCaps.featureLevel >= D3D_FEATURE_LEVEL_10_0)
                    {
                        config.conformant |= EGL_OPENGL_ES2_BIT;
                    }

                    // We can only support conformant ES3 on FL 10.1+
                    if (maxVersion.major >= 3)
                    {
                        config.conformant |= EGL_OPENGL_ES3_BIT_KHR;
                    }
                }

                config.depthSize         = depthStencilBufferFormatInfo.depthBits;
                config.level             = 0;
                config.matchNativePixmap = EGL_NONE;
                config.maxPBufferWidth   = rendererCaps.max2DTextureSize;
                config.maxPBufferHeight  = rendererCaps.max2DTextureSize;
                config.maxPBufferPixels =
                    rendererCaps.max2DTextureSize * rendererCaps.max2DTextureSize;
                config.maxSwapInterval  = 4;
                config.minSwapInterval  = 0;
                config.nativeRenderable = EGL_FALSE;
                config.nativeVisualID   = 0;
                config.nativeVisualType = EGL_NONE;

                // Can't support ES3 at all without feature level 10.1
                config.renderableType = EGL_OPENGL_ES2_BIT;
                if (maxVersion.major >= 3)
                {
                    config.renderableType |= EGL_OPENGL_ES3_BIT_KHR;
                }

                config.sampleBuffers = (sampleCount == 0) ? 0 : 1;
                config.samples       = sampleCount;
                config.stencilSize   = depthStencilBufferFormatInfo.stencilBits;
                config.surfaceType =
                    EGL_PBUFFER_BIT | EGL_WINDOW_BIT | EGL_SWAP_BEHAVIOR_PRESERVED_BIT;
                config.transparentType       = EGL_NONE;
                config.transparentRedValue   = 0;
                config.transparentGreenValue = 0;
                config.transparentBlueValue  = 0;
                config.optimalOrientation    = optimalSurfaceOrientation;
                config.colorComponentType    = gl_egl::GLComponentTypeToEGLColorComponentType(
                    colorBufferFormatInfo.componentType);

                configs.add(config);
            }
        }
    }

    ASSERT(configs.size() > 0);
    return configs;
}

void Renderer11::generateDisplayExtensions(egl::DisplayExtensions *outExtensions) const
{
    outExtensions->createContextRobustness = true;

    if (getShareHandleSupport())
    {
        outExtensions->d3dShareHandleClientBuffer     = true;
        outExtensions->surfaceD3DTexture2DShareHandle = true;
    }
    outExtensions->d3dTextureClientBuffer = true;
    outExtensions->imageD3D11Texture      = true;

    outExtensions->keyedMutex          = true;
    outExtensions->querySurfacePointer = true;
    outExtensions->windowFixedSize     = true;

    // If present path fast is active then the surface orientation extension isn't supported
    outExtensions->surfaceOrientation = !mPresentPathFastEnabled;

    // D3D11 does not support present with dirty rectangles until DXGI 1.2.
    outExtensions->postSubBuffer = mRenderer11DeviceCaps.supportsDXGI1_2;

    outExtensions->image                 = true;
    outExtensions->imageBase             = true;
    outExtensions->glTexture2DImage      = true;
    outExtensions->glTextureCubemapImage = true;
    outExtensions->glRenderbufferImage   = true;

    outExtensions->stream                     = true;
    outExtensions->streamConsumerGLTexture    = true;
    outExtensions->streamConsumerGLTextureYUV = true;
    outExtensions->streamProducerD3DTexture   = true;

    outExtensions->noConfigContext   = true;
    outExtensions->directComposition = !!mDCompModule;

    // Contexts are virtualized so textures and semaphores can be shared globally
    outExtensions->displayTextureShareGroup   = true;
    outExtensions->displaySemaphoreShareGroup = true;

    // syncControlCHROMIUM requires direct composition.
    outExtensions->syncControlCHROMIUM = outExtensions->directComposition;

    // D3D11 can be used without a swap chain
    outExtensions->surfacelessContext = true;

    // All D3D feature levels support robust resource init
    outExtensions->robustResourceInitializationANGLE = true;

#ifdef ANGLE_ENABLE_D3D11_COMPOSITOR_NATIVE_WINDOW
    // Compositor Native Window capabilies require WinVer >= 1803
    if (CompositorNativeWindow11::IsSupportedWinRelease())
    {
        outExtensions->windowsUIComposition = true;
    }
#endif
}

angle::Result Renderer11::flush(Context11 *context11)
{
    mDeviceContext->Flush();
    return angle::Result::Continue;
}

angle::Result Renderer11::finish(Context11 *context11)
{
    if (!mSyncQuery.valid())
    {
        D3D11_QUERY_DESC queryDesc;
        queryDesc.Query     = D3D11_QUERY_EVENT;
        queryDesc.MiscFlags = 0;

        ANGLE_TRY(allocateResource(context11, queryDesc, &mSyncQuery));
    }

    mDeviceContext->End(mSyncQuery.get());

    HRESULT result       = S_OK;
    unsigned int attempt = 0;
    do
    {
        unsigned int flushFrequency = 100;
        UINT flags = (attempt % flushFrequency == 0) ? 0 : D3D11_ASYNC_GETDATA_DONOTFLUSH;
        attempt++;

        result = mDeviceContext->GetData(mSyncQuery.get(), nullptr, 0, flags);
        ANGLE_TRY_HR(context11, result, "Failed to get event query data");

        if (result == S_FALSE)
        {
            // Keep polling, but allow other threads to do something useful first
            std::this_thread::yield();
        }

        // Attempt is incremented before checking if we should test for device loss so that device
        // loss is not checked on the first iteration
        bool checkDeviceLost = (attempt % kPollingD3DDeviceLostCheckFrequency) == 0;
        if (checkDeviceLost && testDeviceLost())
        {
            mDisplay->notifyDeviceLost();
            ANGLE_CHECK(context11, false, "Device was lost while waiting for sync.",
                        GL_OUT_OF_MEMORY);
        }
    } while (result == S_FALSE);

    return angle::Result::Continue;
}

bool Renderer11::isValidNativeWindow(EGLNativeWindowType window) const
{
#if defined(ANGLE_ENABLE_WINDOWS_UWP)
    if (NativeWindow11WinRT::IsValidNativeWindow(window))
    {
        return true;
    }
#else
    if (NativeWindow11Win32::IsValidNativeWindow(window))
    {
        return true;
    }
#endif

#ifdef ANGLE_ENABLE_D3D11_COMPOSITOR_NATIVE_WINDOW
    static_assert(sizeof(ABI::Windows::UI::Composition::SpriteVisual *) == sizeof(HWND),
                  "Pointer size must match Window Handle size");
    if (CompositorNativeWindow11::IsValidNativeWindow(window))
    {
        return true;
    }
#endif

    return false;
}

NativeWindowD3D *Renderer11::createNativeWindow(EGLNativeWindowType window,
                                                const egl::Config *config,
                                                const egl::AttributeMap &attribs) const
{
#if defined(ANGLE_ENABLE_WINDOWS_UWP)
    if (window == nullptr || NativeWindow11WinRT::IsValidNativeWindow(window))
    {
        return new NativeWindow11WinRT(window, config->alphaSize > 0);
    }
#else
    if (window == nullptr || NativeWindow11Win32::IsValidNativeWindow(window))
    {
        return new NativeWindow11Win32(
            window, config->alphaSize > 0,
            attribs.get(EGL_DIRECT_COMPOSITION_ANGLE, EGL_FALSE) == EGL_TRUE);
    }
#endif

#ifdef ANGLE_ENABLE_D3D11_COMPOSITOR_NATIVE_WINDOW
    if (CompositorNativeWindow11::IsValidNativeWindow(window))
    {
        return new CompositorNativeWindow11(window, config->alphaSize > 0);
    }
#endif

    UNREACHABLE();
    return nullptr;
}

egl::Error Renderer11::getD3DTextureInfo(const egl::Config *configuration,
                                         IUnknown *texture,
                                         const egl::AttributeMap &attribs,
                                         EGLint *width,
                                         EGLint *height,
                                         GLsizei *samples,
                                         gl::Format *glFormat,
                                         const angle::Format **angleFormat,
                                         UINT *arraySlice) const
{
    angle::ComPtr<ID3D11Texture2D> d3dTexture;
    texture->QueryInterface(IID_PPV_ARGS(&d3dTexture));
    if (d3dTexture == nullptr)
    {
        return egl::EglBadParameter() << "client buffer is not a ID3D11Texture2D";
    }

    angle::ComPtr<ID3D11Device> textureDevice;
    d3dTexture->GetDevice(&textureDevice);
    if (textureDevice != mDevice)
    {
        return egl::EglBadParameter() << "Texture's device does not match.";
    }

    D3D11_TEXTURE2D_DESC desc = {};
    d3dTexture->GetDesc(&desc);

    EGLint imageWidth  = static_cast<EGLint>(desc.Width);
    EGLint imageHeight = static_cast<EGLint>(desc.Height);

    GLsizei sampleCount = static_cast<GLsizei>(desc.SampleDesc.Count);
    if (configuration && (configuration->samples != sampleCount))
    {
        // Both the texture and EGL config sample count may not be the same when multi-sampling
        // is disabled. The EGL sample count can be 0 but a D3D texture is always 1. Therefore,
        // we must only check for a invalid match when the EGL config is non-zero or the texture is
        // not one.
        if (configuration->samples != 0 || sampleCount != 1)
        {
            return egl::EglBadParameter() << "Texture's sample count does not match.";
        }
    }

    const angle::Format *textureAngleFormat = nullptr;
    GLenum sizedInternalFormat              = GL_NONE;

    // From table egl.restrictions in EGL_ANGLE_d3d_texture_client_buffer.
    if (desc.Format == DXGI_FORMAT_NV12 || desc.Format == DXGI_FORMAT_P010 ||
        desc.Format == DXGI_FORMAT_P016)
    {
        if (!attribs.contains(EGL_D3D11_TEXTURE_PLANE_ANGLE))
        {
            return egl::EglBadParameter()
                   << "EGL_D3D11_TEXTURE_PLANE_ANGLE must be specified for YUV textures.";
        }

        EGLint plane = attribs.getAsInt(EGL_D3D11_TEXTURE_PLANE_ANGLE);

        // P010 and P016 have the same memory layout, SRV/RTV format, etc.
        const bool isNV12 = (desc.Format == DXGI_FORMAT_NV12);
        if (plane == 0)
        {
            textureAngleFormat = isNV12 ? &angle::Format::Get(angle::FormatID::R8_UNORM)
                                        : &angle::Format::Get(angle::FormatID::R16_UNORM);
        }
        else if (plane == 1)
        {
            textureAngleFormat = isNV12 ? &angle::Format::Get(angle::FormatID::R8G8_UNORM)
                                        : &angle::Format::Get(angle::FormatID::R16G16_UNORM);
            imageWidth /= 2;
            imageHeight /= 2;
        }
        else
        {
            return egl::EglBadParameter() << "Invalid client buffer texture plane: " << plane;
        }

        ASSERT(textureAngleFormat);
        sizedInternalFormat = textureAngleFormat->glInternalFormat;
    }
    else
    {
        switch (desc.Format)
        {
            case DXGI_FORMAT_R8G8B8A8_UNORM:
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            case DXGI_FORMAT_R8G8B8A8_TYPELESS:
            case DXGI_FORMAT_B8G8R8A8_UNORM:
            case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            case DXGI_FORMAT_B8G8R8A8_TYPELESS:
            case DXGI_FORMAT_R16G16B16A16_FLOAT:
            case DXGI_FORMAT_R32G32B32A32_FLOAT:
            case DXGI_FORMAT_R10G10B10A2_UNORM:
            case DXGI_FORMAT_R8_UNORM:
            case DXGI_FORMAT_R8G8_UNORM:
            case DXGI_FORMAT_R16_UNORM:
            case DXGI_FORMAT_R16G16_UNORM:
                break;

            default:
                return egl::EglBadParameter()
                       << "Invalid client buffer texture format: " << desc.Format;
        }

        textureAngleFormat = &d3d11_angle::GetFormat(desc.Format);
        ASSERT(textureAngleFormat);

        sizedInternalFormat = textureAngleFormat->glInternalFormat;

        if (attribs.contains(EGL_TEXTURE_INTERNAL_FORMAT_ANGLE))
        {
            const GLenum internalFormat =
                static_cast<GLenum>(attribs.get(EGL_TEXTURE_INTERNAL_FORMAT_ANGLE));
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
                    break;
                default:
                    return egl::EglBadParameter()
                           << "Invalid client buffer texture internal format: " << std::hex
                           << internalFormat;
            }

            const GLenum type = gl::GetSizedInternalFormatInfo(sizedInternalFormat).type;

            const auto format = gl::Format(internalFormat, type);
            if (!format.valid())
            {
                return egl::EglBadParameter()
                       << "Invalid client buffer texture internal format: " << std::hex
                       << internalFormat;
            }

            sizedInternalFormat = format.info->sizedInternalFormat;
        }
    }

    UINT textureArraySlice =
        static_cast<UINT>(attribs.getAsInt(EGL_D3D11_TEXTURE_ARRAY_SLICE_ANGLE, 0));
    if (textureArraySlice >= desc.ArraySize)
    {
        return egl::EglBadParameter()
               << "Invalid client buffer texture array slice: " << textureArraySlice;
    }

    if (width)
    {
        *width = imageWidth;
    }
    if (height)
    {
        *height = imageHeight;
    }

    if (samples)
    {
        // EGL samples 0 corresponds to D3D11 sample count 1.
        *samples = sampleCount != 1 ? sampleCount : 0;
    }

    if (glFormat)
    {
        *glFormat = gl::Format(sizedInternalFormat);
    }

    if (angleFormat)
    {
        *angleFormat = textureAngleFormat;
    }

    if (arraySlice)
    {
        *arraySlice = textureArraySlice;
    }

    return egl::NoError();
}

egl::Error Renderer11::validateShareHandle(const egl::Config *config,
                                           HANDLE shareHandle,
                                           const egl::AttributeMap &attribs) const
{
    if (shareHandle == nullptr)
    {
        return egl::EglBadParameter() << "NULL share handle.";
    }

    angle::ComPtr<ID3D11Resource> tempResource11;
    HRESULT result = mDevice->OpenSharedResource(shareHandle, IID_PPV_ARGS(&tempResource11));
    if (FAILED(result) && mDevice1)
    {
        result = mDevice1->OpenSharedResource1(shareHandle, IID_PPV_ARGS(&tempResource11));
    }

    if (FAILED(result))
    {
        return egl::EglBadParameter() << "Failed to open share handle, " << gl::FmtHR(result);
    }

    angle::ComPtr<ID3D11Texture2D> texture2D;
    tempResource11.As(&texture2D);
    if (texture2D == nullptr)
    {
        return egl::EglBadParameter()
               << "Failed to query ID3D11Texture2D object from share handle.";
    }

    D3D11_TEXTURE2D_DESC desc = {};
    texture2D->GetDesc(&desc);

    EGLint width  = attribs.getAsInt(EGL_WIDTH, 0);
    EGLint height = attribs.getAsInt(EGL_HEIGHT, 0);
    ASSERT(width != 0 && height != 0);

    const d3d11::Format &backbufferFormatInfo =
        d3d11::Format::Get(config->renderTargetFormat, getRenderer11DeviceCaps());

    if (desc.Width != static_cast<UINT>(width) || desc.Height != static_cast<UINT>(height) ||
        desc.Format != backbufferFormatInfo.texFormat || desc.MipLevels != 1 || desc.ArraySize != 1)
    {
        return egl::EglBadParameter() << "Invalid texture parameters in share handle texture.";
    }

    return egl::NoError();
}

SwapChainD3D *Renderer11::createSwapChain(NativeWindowD3D *nativeWindow,
                                          HANDLE shareHandle,
                                          IUnknown *d3dTexture,
                                          GLenum backBufferFormat,
                                          GLenum depthBufferFormat,
                                          EGLint orientation,
                                          EGLint samples)
{
    return new SwapChain11(this, GetAs<NativeWindow11>(nativeWindow), shareHandle, d3dTexture,
                           backBufferFormat, depthBufferFormat, orientation, samples);
}

void *Renderer11::getD3DDevice()
{
    return mDevice.Get();
}

angle::Result Renderer11::drawWithGeometryShaderAndTransformFeedback(Context11 *context11,
                                                                     gl::PrimitiveMode mode,
                                                                     UINT instanceCount,
                                                                     UINT vertexCount)
{
    const gl::State &glState            = context11->getState();
    ProgramExecutableD3D *executableD3D = mStateManager.getProgramExecutableD3D();

    // Since we use a geometry if-and-only-if we rewrite vertex streams, transform feedback
    // won't get the correct output. To work around this, draw with *only* the stream out
    // first (no pixel shader) to feed the stream out buffers and then draw again with the
    // geometry shader + pixel shader to rasterize the primitives.
    mStateManager.setPixelShader(nullptr);

    if (instanceCount > 0)
    {
        mDeviceContext->DrawInstanced(vertexCount, instanceCount, 0, 0);
    }
    else
    {
        mDeviceContext->Draw(vertexCount, 0);
    }

    rx::ShaderExecutableD3D *pixelExe = nullptr;
    ANGLE_TRY(executableD3D->getPixelExecutableForCachedOutputLayout(
        context11, context11->getRenderer(), &pixelExe, nullptr));

    // Skip the draw call if rasterizer discard is enabled (or no fragment shader).
    if (!pixelExe || glState.getRasterizerState().rasterizerDiscard)
    {
        return angle::Result::Continue;
    }

    mStateManager.setPixelShader(&GetAs<ShaderExecutable11>(pixelExe)->getPixelShader());

    // Retrieve the geometry shader.
    rx::ShaderExecutableD3D *geometryExe = nullptr;
    ANGLE_TRY(executableD3D->getGeometryExecutableForPrimitiveType(
        context11, context11->getRenderer(), glState.getCaps(), glState.getProvokingVertex(), mode,
        &geometryExe, nullptr));

    mStateManager.setGeometryShader(&GetAs<ShaderExecutable11>(geometryExe)->getGeometryShader());

    if (instanceCount > 0)
    {
        mDeviceContext->DrawInstanced(vertexCount, instanceCount, 0, 0);
    }
    else
    {
        mDeviceContext->Draw(vertexCount, 0);
    }

    return angle::Result::Continue;
}

angle::Result Renderer11::drawArrays(const gl::Context *context,
                                     gl::PrimitiveMode mode,
                                     GLint firstVertex,
                                     GLsizei vertexCount,
                                     GLsizei instanceCount,
                                     GLuint baseInstance,
                                     bool isInstancedDraw)
{
    if (mStateManager.getCullEverything())
    {
        return angle::Result::Continue;
    }

    Context11 *context11 = GetImplAs<Context11>(context);

    ANGLE_TRY(markRawBufferUsage(context));

    ProgramExecutableD3D *executableD3D = mStateManager.getProgramExecutableD3D();
    GLsizei adjustedInstanceCount       = GetAdjustedInstanceCount(executableD3D, instanceCount);

    // Note: vertex indexes can be arbitrarily large.
    UINT clampedVertexCount = gl::GetClampedVertexCount<UINT>(vertexCount);

    const auto &glState = context->getState();
    if (glState.getCurrentTransformFeedback() && glState.isTransformFeedbackActiveUnpaused())
    {
        ANGLE_TRY(markTransformFeedbackUsage(context));

        if (executableD3D->usesGeometryShader(context11->getRenderer(),
                                              glState.getProvokingVertex(), mode))
        {
            return drawWithGeometryShaderAndTransformFeedback(
                context11, mode, adjustedInstanceCount, clampedVertexCount);
        }
    }

    switch (mode)
    {
        case gl::PrimitiveMode::LineLoop:
            return drawLineLoop(context, clampedVertexCount, gl::DrawElementsType::InvalidEnum,
                                nullptr, 0, adjustedInstanceCount);
        case gl::PrimitiveMode::TriangleFan:
            return drawTriangleFan(context, clampedVertexCount, gl::DrawElementsType::InvalidEnum,
                                   nullptr, 0, adjustedInstanceCount);
        default:
            break;
    }

    // "Normal" draw case.
    if (!isInstancedDraw && adjustedInstanceCount == 0)
    {
        mDeviceContext->Draw(clampedVertexCount, 0);
    }
    else
    {
        mDeviceContext->DrawInstanced(clampedVertexCount, adjustedInstanceCount, 0, baseInstance);
    }
    return angle::Result::Continue;
}

angle::Result Renderer11::drawElements(const gl::Context *context,
                                       gl::PrimitiveMode mode,
                                       GLint startVertex,
                                       GLsizei indexCount,
                                       gl::DrawElementsType indexType,
                                       const void *indices,
                                       GLsizei instanceCount,
                                       GLint baseVertex,
                                       GLuint baseInstance,
                                       bool isInstancedDraw)
{
    if (mStateManager.getCullEverything())
    {
        return angle::Result::Continue;
    }

    ANGLE_TRY(markRawBufferUsage(context));

    // Transform feedback is not allowed for DrawElements, this error should have been caught at the
    // API validation layer.
    const gl::State &glState = context->getState();
    ASSERT(!glState.isTransformFeedbackActiveUnpaused());

    // If this draw call is coming from an indirect call, offset by the indirect call's base vertex.
    GLint baseVertexAdjusted = baseVertex - startVertex;

    const ProgramExecutableD3D *executableD3D = mStateManager.getProgramExecutableD3D();
    GLsizei adjustedInstanceCount = GetAdjustedInstanceCount(executableD3D, instanceCount);

    if (mode == gl::PrimitiveMode::LineLoop)
    {
        return drawLineLoop(context, indexCount, indexType, indices, baseVertexAdjusted,
                            adjustedInstanceCount);
    }

    if (mode == gl::PrimitiveMode::TriangleFan)
    {
        return drawTriangleFan(context, indexCount, indexType, indices, baseVertexAdjusted,
                               adjustedInstanceCount);
    }

    if (!isInstancedDraw && adjustedInstanceCount == 0)
    {
        mDeviceContext->DrawIndexed(indexCount, 0, baseVertexAdjusted);
    }
    else
    {
        mDeviceContext->DrawIndexedInstanced(indexCount, adjustedInstanceCount, 0,
                                             baseVertexAdjusted, baseInstance);
    }
    return angle::Result::Continue;
}

angle::Result Renderer11::drawArraysIndirect(const gl::Context *context, const void *indirect)
{
    if (mStateManager.getCullEverything())
    {
        return angle::Result::Continue;
    }

    ANGLE_TRY(markRawBufferUsage(context));

    const gl::State &glState = context->getState();
    ASSERT(!glState.isTransformFeedbackActiveUnpaused());

    gl::Buffer *drawIndirectBuffer = glState.getTargetBuffer(gl::BufferBinding::DrawIndirect);
    ASSERT(drawIndirectBuffer);
    Buffer11 *storage = GetImplAs<Buffer11>(drawIndirectBuffer);

    uintptr_t offset = reinterpret_cast<uintptr_t>(indirect);

    ID3D11Buffer *buffer = nullptr;
    ANGLE_TRY(storage->getBuffer(context, BUFFER_USAGE_INDIRECT, &buffer));
    mDeviceContext->DrawInstancedIndirect(buffer, static_cast<unsigned int>(offset));
    return angle::Result::Continue;
}

angle::Result Renderer11::drawElementsIndirect(const gl::Context *context, const void *indirect)
{
    if (mStateManager.getCullEverything())
    {
        return angle::Result::Continue;
    }

    ANGLE_TRY(markRawBufferUsage(context));

    const gl::State &glState = context->getState();
    ASSERT(!glState.isTransformFeedbackActiveUnpaused());

    gl::Buffer *drawIndirectBuffer = glState.getTargetBuffer(gl::BufferBinding::DrawIndirect);
    ASSERT(drawIndirectBuffer);
    Buffer11 *storage = GetImplAs<Buffer11>(drawIndirectBuffer);
    uintptr_t offset  = reinterpret_cast<uintptr_t>(indirect);

    ID3D11Buffer *buffer = nullptr;
    ANGLE_TRY(storage->getBuffer(context, BUFFER_USAGE_INDIRECT, &buffer));
    mDeviceContext->DrawIndexedInstancedIndirect(buffer, static_cast<unsigned int>(offset));
    return angle::Result::Continue;
}

angle::Result Renderer11::drawLineLoop(const gl::Context *context,
                                       GLuint count,
                                       gl::DrawElementsType type,
                                       const void *indexPointer,
                                       int baseVertex,
                                       int instances)
{
    const gl::State &glState       = context->getState();
    gl::VertexArray *vao           = glState.getVertexArray();
    gl::Buffer *elementArrayBuffer = vao->getElementArrayBuffer();

    const void *indices = indexPointer;

    // Get the raw indices for an indexed draw
    if (type != gl::DrawElementsType::InvalidEnum && elementArrayBuffer)
    {
        BufferD3D *storage = GetImplAs<BufferD3D>(elementArrayBuffer);
        intptr_t offset    = reinterpret_cast<intptr_t>(indices);

        const uint8_t *bufferData = nullptr;
        ANGLE_TRY(storage->getData(context, &bufferData));

        indices = bufferData + offset;
    }

    if (!mLineLoopIB)
    {
        mLineLoopIB = new StreamingIndexBufferInterface(this);
        ANGLE_TRY(mLineLoopIB->reserveBufferSpace(context, INITIAL_INDEX_BUFFER_SIZE,
                                                  gl::DrawElementsType::UnsignedInt));
    }

    // Checked by Renderer11::applyPrimitiveType
    bool indexCheck = static_cast<unsigned int>(count) + 1 >
                      (std::numeric_limits<unsigned int>::max() / sizeof(unsigned int));
    ANGLE_CHECK(GetImplAs<Context11>(context), !indexCheck,
                "Failed to create a 32-bit looping index buffer for "
                "GL_LINE_LOOP, too many indices required.",
                GL_OUT_OF_MEMORY);

    GetLineLoopIndices(indices, type, static_cast<GLuint>(count),
                       glState.isPrimitiveRestartEnabled(), &mScratchIndexDataBuffer);

    unsigned int spaceNeeded =
        static_cast<unsigned int>(sizeof(GLuint) * mScratchIndexDataBuffer.size());
    ANGLE_TRY(
        mLineLoopIB->reserveBufferSpace(context, spaceNeeded, gl::DrawElementsType::UnsignedInt));

    void *mappedMemory = nullptr;
    unsigned int offset;
    ANGLE_TRY(mLineLoopIB->mapBuffer(context, spaceNeeded, &mappedMemory, &offset));

    // Copy over the converted index data.
    memcpy(mappedMemory, &mScratchIndexDataBuffer[0],
           sizeof(GLuint) * mScratchIndexDataBuffer.size());

    ANGLE_TRY(mLineLoopIB->unmapBuffer(context));

    IndexBuffer11 *indexBuffer          = GetAs<IndexBuffer11>(mLineLoopIB->getIndexBuffer());
    const d3d11::Buffer &d3dIndexBuffer = indexBuffer->getBuffer();
    DXGI_FORMAT indexFormat             = indexBuffer->getIndexFormat();

    mStateManager.setIndexBuffer(d3dIndexBuffer.get(), indexFormat, offset);

    UINT indexCount = static_cast<UINT>(mScratchIndexDataBuffer.size());

    if (instances > 0)
    {
        mDeviceContext->DrawIndexedInstanced(indexCount, instances, 0, baseVertex, 0);
    }
    else
    {
        mDeviceContext->DrawIndexed(indexCount, 0, baseVertex);
    }

    return angle::Result::Continue;
}

angle::Result Renderer11::drawTriangleFan(const gl::Context *context,
                                          GLuint count,
                                          gl::DrawElementsType type,
                                          const void *indices,
                                          int baseVertex,
                                          int instances)
{
    const gl::State &glState       = context->getState();
    gl::VertexArray *vao           = glState.getVertexArray();
    gl::Buffer *elementArrayBuffer = vao->getElementArrayBuffer();

    const void *indexPointer = indices;

    // Get the raw indices for an indexed draw
    if (type != gl::DrawElementsType::InvalidEnum && elementArrayBuffer)
    {
        BufferD3D *storage = GetImplAs<BufferD3D>(elementArrayBuffer);
        intptr_t offset    = reinterpret_cast<intptr_t>(indices);

        const uint8_t *bufferData = nullptr;
        ANGLE_TRY(storage->getData(context, &bufferData));

        indexPointer = bufferData + offset;
    }

    if (!mTriangleFanIB)
    {
        mTriangleFanIB = new StreamingIndexBufferInterface(this);
        ANGLE_TRY(mTriangleFanIB->reserveBufferSpace(context, INITIAL_INDEX_BUFFER_SIZE,
                                                     gl::DrawElementsType::UnsignedInt));
    }

    // Checked by Renderer11::applyPrimitiveType
    ASSERT(count >= 3);

    const GLuint numTris = count - 2;

    bool indexCheck =
        (numTris > std::numeric_limits<unsigned int>::max() / (sizeof(unsigned int) * 3));
    ANGLE_CHECK(GetImplAs<Context11>(context), !indexCheck,
                "Failed to create a scratch index buffer for GL_TRIANGLE_FAN, "
                "too many indices required.",
                GL_OUT_OF_MEMORY);

    GetTriFanIndices(indexPointer, type, count, glState.isPrimitiveRestartEnabled(),
                     &mScratchIndexDataBuffer);

    const unsigned int spaceNeeded =
        static_cast<unsigned int>(mScratchIndexDataBuffer.size() * sizeof(unsigned int));
    ANGLE_TRY(mTriangleFanIB->reserveBufferSpace(context, spaceNeeded,
                                                 gl::DrawElementsType::UnsignedInt));

    void *mappedMemory = nullptr;
    unsigned int offset;
    ANGLE_TRY(mTriangleFanIB->mapBuffer(context, spaceNeeded, &mappedMemory, &offset));

    memcpy(mappedMemory, &mScratchIndexDataBuffer[0], spaceNeeded);

    ANGLE_TRY(mTriangleFanIB->unmapBuffer(context));

    IndexBuffer11 *indexBuffer          = GetAs<IndexBuffer11>(mTriangleFanIB->getIndexBuffer());
    const d3d11::Buffer &d3dIndexBuffer = indexBuffer->getBuffer();
    DXGI_FORMAT indexFormat             = indexBuffer->getIndexFormat();

    mStateManager.setIndexBuffer(d3dIndexBuffer.get(), indexFormat, offset);

    UINT indexCount = static_cast<UINT>(mScratchIndexDataBuffer.size());

    if (instances > 0)
    {
        mDeviceContext->DrawIndexedInstanced(indexCount, instances, 0, baseVertex, 0);
    }
    else
    {
        mDeviceContext->DrawIndexed(indexCount, 0, baseVertex);
    }

    return angle::Result::Continue;
}

void Renderer11::releaseDeviceResources()
{
    mStateManager.deinitialize();
    mStateCache.clear();

    SafeDelete(mLineLoopIB);
    SafeDelete(mTriangleFanIB);
    SafeDelete(mBlit);
    SafeDelete(mClear);
    SafeDelete(mTrim);
    SafeDelete(mPixelTransfer);

    mSyncQuery.reset();

    mCachedResolveTexture.reset();
}

// set notify to true to broadcast a message to all contexts of the device loss
bool Renderer11::testDeviceLost()
{
    if (!mDevice)
    {
        return true;
    }

    // GetRemovedReason is used to test if the device is removed
    HRESULT result = mDevice->GetDeviceRemovedReason();
    bool isLost    = FAILED(result);

    if (isLost)
    {
        ERR() << "The D3D11 device was removed, " << gl::FmtHR(result);
    }

    return isLost;
}

bool Renderer11::testDeviceResettable()
{
    // determine if the device is resettable by creating a mock device
    PFN_D3D11_CREATE_DEVICE D3D11CreateDevice =
        (PFN_D3D11_CREATE_DEVICE)GetProcAddress(mD3d11Module, "D3D11CreateDevice");

    if (D3D11CreateDevice == nullptr)
    {
        return false;
    }

    angle::ComPtr<ID3D11Device> mockDevice;
    D3D_FEATURE_LEVEL mockFeatureLevel;
    angle::ComPtr<ID3D11DeviceContext> mockContext;
    UINT flags = (mCreateDebugDevice ? D3D11_CREATE_DEVICE_DEBUG : 0);

    ASSERT(mRequestedDriverType != D3D_DRIVER_TYPE_UNKNOWN);
    HRESULT result = D3D11CreateDevice(
        nullptr, mRequestedDriverType, nullptr, flags, mAvailableFeatureLevels.data(),
        static_cast<unsigned int>(mAvailableFeatureLevels.size()), D3D11_SDK_VERSION, &mockDevice,
        &mockFeatureLevel, &mockContext);

    if (!mDevice || FAILED(result))
    {
        return false;
    }

    return true;
}

void Renderer11::release()
{
    mScratchMemoryBuffer.clear();

    mAnnotatorContext.release();
    gl::UninitializeDebugAnnotations();

    releaseDeviceResources();

    mDxgiFactory.Reset();
    mDxgiAdapter.Reset();

    mDeviceContext3.Reset();
    mDeviceContext1.Reset();

    if (mDeviceContext)
    {
        mDeviceContext->ClearState();
        mDeviceContext->Flush();
        mDeviceContext.Reset();
    }

    mDevice.Reset();
    mDevice1.Reset();
    mDebug.Reset();

    if (mD3d11Module)
    {
        FreeLibrary(mD3d11Module);
        mD3d11Module = nullptr;
    }

    if (mDCompModule)
    {
        FreeLibrary(mDCompModule);
        mDCompModule = nullptr;
    }

    mDevice12.Reset();
    mCommandQueue.Reset();

    if (mD3d12Module)
    {
        FreeLibrary(mD3d12Module);
        mD3d12Module = nullptr;
    }

    mCompiler.release();

    mSupportsShareHandles.reset();
}

bool Renderer11::resetDevice()
{
    // recreate everything
    release();
    egl::Error result = initialize();

    if (result.isError())
    {
        ERR() << "Could not reinitialize D3D11 device: " << result;
        return false;
    }

    return true;
}

std::string Renderer11::getRendererDescription() const
{
    std::ostringstream rendererString;

    rendererString << mDescription;
    rendererString << " (" << gl::FmtHex(mAdapterDescription.DeviceId) << ")";
    rendererString << " Direct3D11";
    if (mD3d12Module)
        rendererString << "on12";

    rendererString << " vs_" << getMajorShaderModel() << "_" << getMinorShaderModel()
                   << getShaderModelSuffix();
    rendererString << " ps_" << getMajorShaderModel() << "_" << getMinorShaderModel()
                   << getShaderModelSuffix();

    return rendererString.str();
}

DeviceIdentifier Renderer11::getAdapterIdentifier() const
{
    // Don't use the AdapterLuid here, since that doesn't persist across reboot.
    DeviceIdentifier deviceIdentifier = {};
    deviceIdentifier.VendorId         = mAdapterDescription.VendorId;
    deviceIdentifier.DeviceId         = mAdapterDescription.DeviceId;
    deviceIdentifier.SubSysId         = mAdapterDescription.SubSysId;
    deviceIdentifier.Revision         = mAdapterDescription.Revision;
    deviceIdentifier.FeatureLevel     = static_cast<UINT>(mRenderer11DeviceCaps.featureLevel);

    return deviceIdentifier;
}

unsigned int Renderer11::getReservedVertexUniformVectors() const
{
    // Driver uniforms are stored in a separate constant buffer
    return d3d11_gl::GetReservedVertexUniformVectors(mRenderer11DeviceCaps.featureLevel);
}

unsigned int Renderer11::getReservedFragmentUniformVectors() const
{
    // Driver uniforms are stored in a separate constant buffer
    return d3d11_gl::GetReservedFragmentUniformVectors(mRenderer11DeviceCaps.featureLevel);
}

gl::ShaderMap<unsigned int> Renderer11::getReservedShaderUniformBuffers() const
{
    gl::ShaderMap<unsigned int> shaderReservedUniformBuffers = {};

    // we reserve one buffer for the application uniforms, and one for driver uniforms
    shaderReservedUniformBuffers[gl::ShaderType::Vertex]   = 2;
    shaderReservedUniformBuffers[gl::ShaderType::Fragment] = 2;

    return shaderReservedUniformBuffers;
}

d3d11::ANGLED3D11DeviceType Renderer11::getDeviceType() const
{
    if (mCreatedWithDeviceEXT)
    {
        return d3d11::GetDeviceType(mDevice.Get());
    }

    if ((mRequestedDriverType == D3D_DRIVER_TYPE_SOFTWARE) ||
        (mRequestedDriverType == D3D_DRIVER_TYPE_REFERENCE) ||
        (mRequestedDriverType == D3D_DRIVER_TYPE_NULL))
    {
        return d3d11::ANGLE_D3D11_DEVICE_TYPE_SOFTWARE_REF_OR_NULL;
    }

    if (mRequestedDriverType == D3D_DRIVER_TYPE_WARP)
    {
        return d3d11::ANGLE_D3D11_DEVICE_TYPE_WARP;
    }

    return d3d11::ANGLE_D3D11_DEVICE_TYPE_HARDWARE;
}

bool Renderer11::getShareHandleSupport() const
{
    if (mSupportsShareHandles.valid())
    {
        return mSupportsShareHandles.value();
    }

    // We only currently support share handles with BGRA surfaces, because
    // chrome needs BGRA. Once chrome fixes this, we should always support them.
    if (!getNativeExtensions().textureFormatBGRA8888EXT)
    {
        mSupportsShareHandles = false;
        return false;
    }

    // PIX doesn't seem to support using share handles, so disable them.
    if (mAnnotatorContext.getStatus())
    {
        mSupportsShareHandles = false;
        return false;
    }

    // Also disable share handles on Feature Level 9_3, since it doesn't support share handles on
    // RGBA8 textures/swapchains.
    if (mRenderer11DeviceCaps.featureLevel <= D3D_FEATURE_LEVEL_9_3)
    {
        mSupportsShareHandles = false;
        return false;
    }

    // Find out which type of D3D11 device the Renderer11 is using
    d3d11::ANGLED3D11DeviceType deviceType = getDeviceType();
    if (deviceType == d3d11::ANGLE_D3D11_DEVICE_TYPE_UNKNOWN)
    {
        mSupportsShareHandles = false;
        return false;
    }

    if (deviceType == d3d11::ANGLE_D3D11_DEVICE_TYPE_SOFTWARE_REF_OR_NULL)
    {
        // Software/Reference/NULL devices don't support share handles
        mSupportsShareHandles = false;
        return false;
    }

    if (deviceType == d3d11::ANGLE_D3D11_DEVICE_TYPE_WARP)
    {
        if (!IsWindows8OrLater())
        {
            // WARP on Windows 7 doesn't support shared handles
            mSupportsShareHandles = false;
            return false;
        }

        // WARP on Windows 8.0+ supports shared handles when shared with another WARP device
        // TODO: allow applications to query for HARDWARE or WARP-specific share handles,
        //       to prevent them trying to use a WARP share handle with an a HW device (or
        //       vice-versa)
        //       e.g. by creating EGL_D3D11_[HARDWARE/WARP]_DEVICE_SHARE_HANDLE_ANGLE
        mSupportsShareHandles = true;
        return true;
    }

    ASSERT(mCreatedWithDeviceEXT || mRequestedDriverType == D3D_DRIVER_TYPE_HARDWARE);
    mSupportsShareHandles = true;
    return true;
}

int Renderer11::getMajorShaderModel() const
{
    switch (mRenderer11DeviceCaps.featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_SHADER_MAJOR_VERSION;  // 5
        case D3D_FEATURE_LEVEL_10_1:
            return D3D10_1_SHADER_MAJOR_VERSION;  // 4
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_SHADER_MAJOR_VERSION;  // 4
        case D3D_FEATURE_LEVEL_9_3:
            return D3D10_SHADER_MAJOR_VERSION;  // 4
        default:
            UNREACHABLE();
            return 0;
    }
}

int Renderer11::getMinorShaderModel() const
{
    switch (mRenderer11DeviceCaps.featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_SHADER_MINOR_VERSION;  // 0
        case D3D_FEATURE_LEVEL_10_1:
            return D3D10_1_SHADER_MINOR_VERSION;  // 1
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_SHADER_MINOR_VERSION;  // 0
        case D3D_FEATURE_LEVEL_9_3:
            return D3D10_SHADER_MINOR_VERSION;  // 0
        default:
            UNREACHABLE();
            return 0;
    }
}

std::string Renderer11::getShaderModelSuffix() const
{
    switch (mRenderer11DeviceCaps.featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return "";
        case D3D_FEATURE_LEVEL_10_1:
            return "";
        case D3D_FEATURE_LEVEL_10_0:
            return "";
        case D3D_FEATURE_LEVEL_9_3:
            return "_level_9_3";
        default:
            UNREACHABLE();
            return "";
    }
}

angle::Result Renderer11::copyImageInternal(const gl::Context *context,
                                            const gl::Framebuffer *framebuffer,
                                            const gl::Rectangle &sourceRect,
                                            GLenum destFormat,
                                            const gl::Offset &destOffset,
                                            RenderTargetD3D *destRenderTarget)
{
    const gl::FramebufferAttachment *colorAttachment = framebuffer->getReadColorAttachment();
    ASSERT(colorAttachment);

    RenderTarget11 *sourceRenderTarget = nullptr;
    ANGLE_TRY(colorAttachment->getRenderTarget(context, 0, &sourceRenderTarget));
    ASSERT(sourceRenderTarget);

    const d3d11::RenderTargetView &dest =
        GetAs<RenderTarget11>(destRenderTarget)->getRenderTargetView();
    ASSERT(dest.valid());

    gl::Box sourceArea(sourceRect.x, sourceRect.y, 0, sourceRect.width, sourceRect.height, 1);
    gl::Extents sourceSize(sourceRenderTarget->getWidth(), sourceRenderTarget->getHeight(), 1);

    const bool invertSource = UsePresentPathFast(this, colorAttachment);
    if (invertSource)
    {
        sourceArea.y      = sourceSize.height - sourceRect.y;
        sourceArea.height = -sourceArea.height;
    }

    gl::Box destArea(destOffset.x, destOffset.y, 0, sourceRect.width, sourceRect.height, 1);
    gl::Extents destSize(destRenderTarget->getWidth(), destRenderTarget->getHeight(), 1);

    // Use nearest filtering because source and destination are the same size for the direct copy.
    // Convert to the unsized format before calling copyTexture.
    GLenum sourceFormat = colorAttachment->getFormat().info->format;
    if (sourceRenderTarget->getTexture().is2D() && sourceRenderTarget->isMultisampled())
    {
        TextureHelper11 tex;
        ANGLE_TRY(resolveMultisampledTexture(context, sourceRenderTarget,
                                             colorAttachment->getDepthSize() > 0,
                                             colorAttachment->getStencilSize() > 0, &tex));

        D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
        viewDesc.Format                    = sourceRenderTarget->getFormatSet().srvFormat;
        viewDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
        viewDesc.Texture2D.MipLevels       = 1;
        viewDesc.Texture2D.MostDetailedMip = 0;

        d3d11::SharedSRV readSRV;
        ANGLE_TRY(allocateResource(GetImplAs<Context11>(context), viewDesc, tex.get(), &readSRV));
        ASSERT(readSRV.valid());

        ANGLE_TRY(mBlit->copyTexture(context, readSRV, sourceArea, sourceSize, sourceFormat, dest,
                                     destArea, destSize, nullptr, gl::GetUnsizedFormat(destFormat),
                                     GL_NONE, GL_NEAREST, false, false, false));

        return angle::Result::Continue;
    }

    ASSERT(!sourceRenderTarget->isMultisampled());

    const d3d11::SharedSRV *source;
    ANGLE_TRY(sourceRenderTarget->getBlitShaderResourceView(context, &source));
    ASSERT(source->valid());

    ANGLE_TRY(mBlit->copyTexture(context, *source, sourceArea, sourceSize, sourceFormat, dest,
                                 destArea, destSize, nullptr, gl::GetUnsizedFormat(destFormat),
                                 GL_NONE, GL_NEAREST, false, false, false));

    return angle::Result::Continue;
}

angle::Result Renderer11::copyImage2D(const gl::Context *context,
                                      const gl::Framebuffer *framebuffer,
                                      const gl::Rectangle &sourceRect,
                                      GLenum destFormat,
                                      const gl::Offset &destOffset,
                                      TextureStorage *storage,
                                      GLint level)
{
    TextureStorage11_2D *storage11 = GetAs<TextureStorage11_2D>(storage);
    ASSERT(storage11);

    gl::ImageIndex index              = gl::ImageIndex::Make2D(level);
    RenderTargetD3D *destRenderTarget = nullptr;
    ANGLE_TRY(storage11->getRenderTarget(context, index, storage11->getRenderToTextureSamples(),
                                         &destRenderTarget));
    ASSERT(destRenderTarget);

    ANGLE_TRY(copyImageInternal(context, framebuffer, sourceRect, destFormat, destOffset,
                                destRenderTarget));

    storage11->markLevelDirty(level);

    return angle::Result::Continue;
}

angle::Result Renderer11::copyImageCube(const gl::Context *context,
                                        const gl::Framebuffer *framebuffer,
                                        const gl::Rectangle &sourceRect,
                                        GLenum destFormat,
                                        const gl::Offset &destOffset,
                                        TextureStorage *storage,
                                        gl::TextureTarget target,
                                        GLint level)
{
    TextureStorage11_Cube *storage11 = GetAs<TextureStorage11_Cube>(storage);
    ASSERT(storage11);

    gl::ImageIndex index              = gl::ImageIndex::MakeCubeMapFace(target, level);
    RenderTargetD3D *destRenderTarget = nullptr;
    ANGLE_TRY(storage11->getRenderTarget(context, index, storage11->getRenderToTextureSamples(),
                                         &destRenderTarget));
    ASSERT(destRenderTarget);

    ANGLE_TRY(copyImageInternal(context, framebuffer, sourceRect, destFormat, destOffset,
                                destRenderTarget));

    storage11->markLevelDirty(level);

    return angle::Result::Continue;
}

angle::Result Renderer11::copyImage3D(const gl::Context *context,
                                      const gl::Framebuffer *framebuffer,
                                      const gl::Rectangle &sourceRect,
                                      GLenum destFormat,
                                      const gl::Offset &destOffset,
                                      TextureStorage *storage,
                                      GLint level)
{
    TextureStorage11_3D *storage11 = GetAs<TextureStorage11_3D>(storage);
    ASSERT(storage11);

    gl::ImageIndex index              = gl::ImageIndex::Make3D(level, destOffset.z);
    RenderTargetD3D *destRenderTarget = nullptr;
    ANGLE_TRY(storage11->getRenderTarget(context, index, storage11->getRenderToTextureSamples(),
                                         &destRenderTarget));
    ASSERT(destRenderTarget);

    ANGLE_TRY(copyImageInternal(context, framebuffer, sourceRect, destFormat, destOffset,
                                destRenderTarget));

    storage11->markLevelDirty(level);

    return angle::Result::Continue;
}

angle::Result Renderer11::copyImage2DArray(const gl::Context *context,
                                           const gl::Framebuffer *framebuffer,
                                           const gl::Rectangle &sourceRect,
                                           GLenum destFormat,
                                           const gl::Offset &destOffset,
                                           TextureStorage *storage,
                                           GLint level)
{
    TextureStorage11_2DArray *storage11 = GetAs<TextureStorage11_2DArray>(storage);
    ASSERT(storage11);

    gl::ImageIndex index              = gl::ImageIndex::Make2DArray(level, destOffset.z);
    RenderTargetD3D *destRenderTarget = nullptr;
    ANGLE_TRY(storage11->getRenderTarget(context, index, storage11->getRenderToTextureSamples(),
                                         &destRenderTarget));
    ASSERT(destRenderTarget);

    ANGLE_TRY(copyImageInternal(context, framebuffer, sourceRect, destFormat, destOffset,
                                destRenderTarget));
    storage11->markLevelDirty(level);

    return angle::Result::Continue;
}

angle::Result Renderer11::copyTexture(const gl::Context *context,
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

    TextureD3D *sourceD3D                = GetImplAs<TextureD3D>(source);
    const gl::ImageDesc &sourceImageDesc = source->getTextureState().getImageDesc(
        NonCubeTextureTypeToTarget(source->getType()), sourceLevel);

    TextureStorage11 *destStorage11 = GetAs<TextureStorage11>(storage);
    ASSERT(destStorage11);

    // Check for fast path where a CopySubresourceRegion can be used.
    if (unpackPremultiplyAlpha == unpackUnmultiplyAlpha && !unpackFlipY &&
        sourceImageDesc.format.info->sizedInternalFormat ==
            destStorage11->getFormatSet().internalFormat)
    {
        const TextureHelper11 *destResource = nullptr;
        ANGLE_TRY(destStorage11->getResource(context, &destResource));

        if (srcTarget == gl::TextureTarget::_2D || srcTarget == gl::TextureTarget::_3D)
        {
            gl::ImageIndex sourceIndex = gl::ImageIndex::MakeFromTarget(srcTarget, sourceLevel, 1);
            const TextureHelper11 *sourceResource = nullptr;
            UINT sourceSubresource                = 0;
            ANGLE_TRY(GetTextureD3DResourceFromStorageOrImage(context, sourceD3D, sourceIndex,
                                                              &sourceResource, &sourceSubresource));

            gl::ImageIndex destIndex = gl::ImageIndex::MakeFromTarget(destTarget, destLevel, 1);

            UINT destSubresource = 0;
            ANGLE_TRY(destStorage11->getSubresourceIndex(context, destIndex, &destSubresource));

            D3D11_BOX d3dBox{static_cast<UINT>(sourceBox.x),
                             static_cast<UINT>(sourceBox.y),
                             static_cast<UINT>(sourceBox.z),
                             static_cast<UINT>(sourceBox.x + sourceBox.width),
                             static_cast<UINT>(sourceBox.y + sourceBox.height),
                             static_cast<UINT>(sourceBox.z + sourceBox.depth)};

            mDeviceContext->CopySubresourceRegion(
                destResource->get(), destSubresource, destOffset.x, destOffset.y, destOffset.z,
                sourceResource->get(), sourceSubresource, &d3dBox);
        }
        else if (srcTarget == gl::TextureTarget::_2DArray)
        {
            D3D11_BOX d3dBox{static_cast<UINT>(sourceBox.x),
                             static_cast<UINT>(sourceBox.y),
                             0,
                             static_cast<UINT>(sourceBox.x + sourceBox.width),
                             static_cast<UINT>(sourceBox.y + sourceBox.height),
                             1u};

            for (int i = 0; i < sourceBox.depth; i++)
            {
                gl::ImageIndex sourceIndex =
                    gl::ImageIndex::Make2DArray(sourceLevel, i + sourceBox.z);
                const TextureHelper11 *sourceResource = nullptr;
                UINT sourceSubresource                = 0;
                ANGLE_TRY(GetTextureD3DResourceFromStorageOrImage(
                    context, sourceD3D, sourceIndex, &sourceResource, &sourceSubresource));

                gl::ImageIndex dIndex = gl::ImageIndex::Make2DArray(destLevel, i + destOffset.z);
                UINT destSubresource  = 0;
                ANGLE_TRY(destStorage11->getSubresourceIndex(context, dIndex, &destSubresource));

                mDeviceContext->CopySubresourceRegion(
                    destResource->get(), destSubresource, destOffset.x, destOffset.y, 0,
                    sourceResource->get(), sourceSubresource, &d3dBox);
            }
        }
        else
        {
            UNREACHABLE();
        }
    }
    else
    {
        TextureStorage *sourceStorage = nullptr;
        ANGLE_TRY(sourceD3D->getNativeTexture(context, &sourceStorage));

        TextureStorage11 *sourceStorage11 = GetAs<TextureStorage11>(sourceStorage);
        ASSERT(sourceStorage11);

        const d3d11::SharedSRV *sourceSRV = nullptr;
        ANGLE_TRY(
            sourceStorage11->getSRVLevels(context, sourceLevel, sourceLevel, true, &sourceSRV));

        gl::ImageIndex destIndex;
        if (destTarget == gl::TextureTarget::_2D || destTarget == gl::TextureTarget::_3D ||
            gl::IsCubeMapFaceTarget(destTarget))
        {
            destIndex = gl::ImageIndex::MakeFromTarget(destTarget, destLevel, 1);
        }
        else if (destTarget == gl::TextureTarget::_2DArray)
        {
            destIndex = gl::ImageIndex::Make2DArrayRange(destLevel, 0, sourceImageDesc.size.depth);
        }
        else
        {
            UNREACHABLE();
        }

        RenderTargetD3D *destRenderTargetD3D = nullptr;
        ANGLE_TRY(destStorage11->getRenderTarget(
            context, destIndex, destStorage11->getRenderToTextureSamples(), &destRenderTargetD3D));

        RenderTarget11 *destRenderTarget11 = GetAs<RenderTarget11>(destRenderTargetD3D);

        const d3d11::RenderTargetView &destRTV = destRenderTarget11->getRenderTargetView();
        ASSERT(destRTV.valid());

        gl::Box sourceArea(sourceBox.x, sourceBox.y, sourceBox.z, sourceBox.width, sourceBox.height,
                           sourceBox.depth);

        if (unpackFlipY)
        {
            sourceArea.y += sourceArea.height;
            sourceArea.height = -sourceArea.height;
        }

        gl::Box destArea(destOffset.x, destOffset.y, destOffset.z, sourceBox.width,
                         sourceBox.height, sourceBox.depth);

        gl::Extents destSize(destRenderTarget11->getWidth(), destRenderTarget11->getHeight(),
                             sourceBox.depth);

        // Use nearest filtering because source and destination are the same size for the direct
        // copy
        GLenum sourceFormat = source->getFormat(srcTarget, sourceLevel).info->format;
        ANGLE_TRY(mBlit->copyTexture(context, *sourceSRV, sourceArea, sourceImageDesc.size,
                                     sourceFormat, destRTV, destArea, destSize, nullptr, destFormat,
                                     destType, GL_NEAREST, false, unpackPremultiplyAlpha,
                                     unpackUnmultiplyAlpha));
    }

    destStorage11->markLevelDirty(destLevel);

    return angle::Result::Continue;
}

angle::Result Renderer11::copyCompressedTexture(const gl::Context *context,
                                                const gl::Texture *source,
                                                GLint sourceLevel,
                                                TextureStorage *storage,
                                                GLint destLevel)
{
    TextureStorage11_2D *destStorage11 = GetAs<TextureStorage11_2D>(storage);
    ASSERT(destStorage11);

    const TextureHelper11 *destResource = nullptr;
    ANGLE_TRY(destStorage11->getResource(context, &destResource));

    gl::ImageIndex destIndex = gl::ImageIndex::Make2D(destLevel);
    UINT destSubresource     = 0;
    ANGLE_TRY(destStorage11->getSubresourceIndex(context, destIndex, &destSubresource));

    TextureD3D *sourceD3D = GetImplAs<TextureD3D>(source);
    ASSERT(sourceD3D);

    TextureStorage *sourceStorage = nullptr;
    ANGLE_TRY(sourceD3D->getNativeTexture(context, &sourceStorage));

    TextureStorage11_2D *sourceStorage11 = GetAs<TextureStorage11_2D>(sourceStorage);
    ASSERT(sourceStorage11);

    const TextureHelper11 *sourceResource = nullptr;
    ANGLE_TRY(sourceStorage11->getResource(context, &sourceResource));

    gl::ImageIndex sourceIndex = gl::ImageIndex::Make2D(sourceLevel);
    UINT sourceSubresource     = 0;
    ANGLE_TRY(sourceStorage11->getSubresourceIndex(context, sourceIndex, &sourceSubresource));

    mDeviceContext->CopySubresourceRegion(destResource->get(), destSubresource, 0, 0, 0,
                                          sourceResource->get(), sourceSubresource, nullptr);

    return angle::Result::Continue;
}

angle::Result Renderer11::createRenderTarget(const gl::Context *context,
                                             int width,
                                             int height,
                                             GLenum format,
                                             GLsizei samples,
                                             RenderTargetD3D **outRT)
{
    const d3d11::Format &formatInfo = d3d11::Format::Get(format, mRenderer11DeviceCaps);

    const gl::TextureCaps &textureCaps = getNativeTextureCaps().get(format);
    GLuint supportedSamples            = textureCaps.getNearestSamples(samples);

    Context11 *context11 = GetImplAs<Context11>(context);

    if (width > 0 && height > 0)
    {
        // Create texture resource
        D3D11_TEXTURE2D_DESC desc;
        desc.Width              = width;
        desc.Height             = height;
        desc.MipLevels          = 1;
        desc.ArraySize          = 1;
        desc.Format             = formatInfo.texFormat;
        desc.SampleDesc.Count   = (supportedSamples == 0) ? 1 : supportedSamples;
        desc.SampleDesc.Quality = getSampleDescQuality(supportedSamples);
        desc.Usage              = D3D11_USAGE_DEFAULT;
        desc.CPUAccessFlags     = 0;
        desc.MiscFlags          = 0;

        // If a rendertarget or depthstencil format exists for this texture format,
        // we'll flag it to allow binding that way. Shader resource views are a little
        // more complicated.
        bool bindRTV = false, bindDSV = false, bindSRV = false;
        bindRTV = (formatInfo.rtvFormat != DXGI_FORMAT_UNKNOWN);
        bindDSV = (formatInfo.dsvFormat != DXGI_FORMAT_UNKNOWN);
        bindSRV = (formatInfo.srvFormat != DXGI_FORMAT_UNKNOWN);

        bool isMultisampledDepthStencil = bindDSV && desc.SampleDesc.Count > 1;
        if (isMultisampledDepthStencil &&
            !mRenderer11DeviceCaps.supportsMultisampledDepthStencilSRVs)
        {
            bindSRV = false;
        }

        desc.BindFlags = (bindRTV ? D3D11_BIND_RENDER_TARGET : 0) |
                         (bindDSV ? D3D11_BIND_DEPTH_STENCIL : 0) |
                         (bindSRV ? D3D11_BIND_SHADER_RESOURCE : 0);

        // The format must be either an RTV or a DSV
        ASSERT(bindRTV != bindDSV);

        TextureHelper11 texture;
        ANGLE_TRY(allocateTexture(context11, desc, formatInfo, &texture));
        texture.setInternalName("createRenderTarget.Texture");

        d3d11::SharedSRV srv;
        d3d11::SharedSRV blitSRV;
        if (bindSRV)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            srvDesc.Format        = formatInfo.srvFormat;
            srvDesc.ViewDimension = (supportedSamples == 0) ? D3D11_SRV_DIMENSION_TEXTURE2D
                                                            : D3D11_SRV_DIMENSION_TEXTURE2DMS;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels       = 1;

            ANGLE_TRY(allocateResource(context11, srvDesc, texture.get(), &srv));
            srv.setInternalName("createRenderTarget.SRV");

            if (formatInfo.blitSRVFormat != formatInfo.srvFormat)
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC blitSRVDesc;
                blitSRVDesc.Format                    = formatInfo.blitSRVFormat;
                blitSRVDesc.ViewDimension             = (supportedSamples == 0)
                                                            ? D3D11_SRV_DIMENSION_TEXTURE2D
                                                            : D3D11_SRV_DIMENSION_TEXTURE2DMS;
                blitSRVDesc.Texture2D.MostDetailedMip = 0;
                blitSRVDesc.Texture2D.MipLevels       = 1;

                ANGLE_TRY(allocateResource(context11, blitSRVDesc, texture.get(), &blitSRV));
                blitSRV.setInternalName("createRenderTarget.BlitSRV");
            }
            else
            {
                blitSRV = srv.makeCopy();
            }
        }

        if (bindDSV)
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
            dsvDesc.Format             = formatInfo.dsvFormat;
            dsvDesc.ViewDimension      = (supportedSamples == 0) ? D3D11_DSV_DIMENSION_TEXTURE2D
                                                                 : D3D11_DSV_DIMENSION_TEXTURE2DMS;
            dsvDesc.Texture2D.MipSlice = 0;
            dsvDesc.Flags              = 0;

            d3d11::DepthStencilView dsv;
            ANGLE_TRY(allocateResource(context11, dsvDesc, texture.get(), &dsv));
            dsv.setInternalName("createRenderTarget.DSV");

            *outRT = new TextureRenderTarget11(std::move(dsv), texture, srv, format, formatInfo,
                                               width, height, 1, supportedSamples);
        }
        else if (bindRTV)
        {
            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
            rtvDesc.Format             = formatInfo.rtvFormat;
            rtvDesc.ViewDimension      = (supportedSamples == 0) ? D3D11_RTV_DIMENSION_TEXTURE2D
                                                                 : D3D11_RTV_DIMENSION_TEXTURE2DMS;
            rtvDesc.Texture2D.MipSlice = 0;

            d3d11::RenderTargetView rtv;
            ANGLE_TRY(allocateResource(context11, rtvDesc, texture.get(), &rtv));
            rtv.setInternalName("createRenderTarget.RTV");

            if (formatInfo.dataInitializerFunction != nullptr)
            {
                const float clearValues[4] = {0.0f, 0.0f, 0.0f, 1.0f};
                mDeviceContext->ClearRenderTargetView(rtv.get(), clearValues);
            }

            *outRT = new TextureRenderTarget11(std::move(rtv), texture, srv, blitSRV, format,
                                               formatInfo, width, height, 1, supportedSamples);
        }
        else
        {
            UNREACHABLE();
        }
    }
    else
    {
        *outRT = new TextureRenderTarget11(d3d11::RenderTargetView(), TextureHelper11(),
                                           d3d11::SharedSRV(), d3d11::SharedSRV(), format,
                                           d3d11::Format::Get(GL_NONE, mRenderer11DeviceCaps),
                                           width, height, 1, supportedSamples);
    }

    return angle::Result::Continue;
}

angle::Result Renderer11::createRenderTargetCopy(const gl::Context *context,
                                                 RenderTargetD3D *source,
                                                 RenderTargetD3D **outRT)
{
    ASSERT(source != nullptr);

    RenderTargetD3D *newRT = nullptr;
    ANGLE_TRY(createRenderTarget(context, source->getWidth(), source->getHeight(),
                                 source->getInternalFormat(), source->getSamples(), &newRT));

    RenderTarget11 *source11 = GetAs<RenderTarget11>(source);
    RenderTarget11 *dest11   = GetAs<RenderTarget11>(newRT);

    mDeviceContext->CopySubresourceRegion(dest11->getTexture().get(), dest11->getSubresourceIndex(),
                                          0, 0, 0, source11->getTexture().get(),
                                          source11->getSubresourceIndex(), nullptr);
    *outRT = newRT;
    return angle::Result::Continue;
}

angle::Result Renderer11::loadExecutable(d3d::Context *context,
                                         const uint8_t *function,
                                         size_t length,
                                         gl::ShaderType type,
                                         const std::vector<D3DVarying> &streamOutVaryings,
                                         bool separatedOutputBuffers,
                                         ShaderExecutableD3D **outExecutable)
{
    ShaderData shaderData(function, length);

    switch (type)
    {
        case gl::ShaderType::Vertex:
        {
            d3d11::VertexShader vertexShader;
            d3d11::GeometryShader streamOutShader;
            ANGLE_TRY(allocateResource(context, shaderData, &vertexShader));

            if (!streamOutVaryings.empty())
            {
                std::vector<D3D11_SO_DECLARATION_ENTRY> soDeclaration;
                soDeclaration.reserve(streamOutVaryings.size());

                for (const auto &streamOutVarying : streamOutVaryings)
                {
                    D3D11_SO_DECLARATION_ENTRY entry = {};
                    entry.Stream                     = 0;
                    entry.SemanticName               = streamOutVarying.semanticName.c_str();
                    entry.SemanticIndex              = streamOutVarying.semanticIndex;
                    entry.StartComponent             = 0;
                    entry.ComponentCount = static_cast<BYTE>(streamOutVarying.componentCount);
                    entry.OutputSlot     = static_cast<BYTE>(
                        (separatedOutputBuffers ? streamOutVarying.outputSlot : 0));
                    soDeclaration.push_back(entry);
                }

                ANGLE_TRY(allocateResource(context, shaderData, &soDeclaration, &streamOutShader));
            }

            *outExecutable = new ShaderExecutable11(function, length, std::move(vertexShader),
                                                    std::move(streamOutShader));
        }
        break;
        case gl::ShaderType::Fragment:
        {
            d3d11::PixelShader pixelShader;
            ANGLE_TRY(allocateResource(context, shaderData, &pixelShader));
            *outExecutable = new ShaderExecutable11(function, length, std::move(pixelShader));
        }
        break;
        case gl::ShaderType::Geometry:
        {
            d3d11::GeometryShader geometryShader;
            ANGLE_TRY(allocateResource(context, shaderData, &geometryShader));
            *outExecutable = new ShaderExecutable11(function, length, std::move(geometryShader));
        }
        break;
        case gl::ShaderType::Compute:
        {
            d3d11::ComputeShader computeShader;
            ANGLE_TRY(allocateResource(context, shaderData, &computeShader));
            *outExecutable = new ShaderExecutable11(function, length, std::move(computeShader));
        }
        break;
        default:
            ANGLE_HR_UNREACHABLE(context);
    }

    return angle::Result::Continue;
}

angle::Result Renderer11::compileToExecutable(d3d::Context *context,
                                              gl::InfoLog &infoLog,
                                              const std::string &shaderHLSL,
                                              gl::ShaderType type,
                                              const std::vector<D3DVarying> &streamOutVaryings,
                                              bool separatedOutputBuffers,
                                              const CompilerWorkaroundsD3D &workarounds,
                                              ShaderExecutableD3D **outExectuable)
{
    std::stringstream profileStream;

    switch (type)
    {
        case gl::ShaderType::Vertex:
            profileStream << "vs";
            break;
        case gl::ShaderType::Fragment:
            profileStream << "ps";
            break;
        case gl::ShaderType::Geometry:
            profileStream << "gs";
            break;
        case gl::ShaderType::Compute:
            profileStream << "cs";
            break;
        default:
            ANGLE_HR_UNREACHABLE(context);
    }

    profileStream << "_" << getMajorShaderModel() << "_" << getMinorShaderModel()
                  << getShaderModelSuffix();
    std::string profile = profileStream.str();

    UINT flags = D3DCOMPILE_OPTIMIZATION_LEVEL2;

#if defined(ANGLE_ENABLE_DEBUG_TRACE)
#    ifndef NDEBUG
    flags = D3DCOMPILE_SKIP_OPTIMIZATION;
#    endif  // NDEBUG
    flags |= D3DCOMPILE_DEBUG;
#endif  // defined(ANGLE_ENABLE_DEBUG_TRACE)

    if (workarounds.enableIEEEStrictness)
        flags |= D3DCOMPILE_IEEE_STRICTNESS;

    // Sometimes D3DCompile will fail with the default compilation flags for complicated shaders
    // when it would otherwise pass with alternative options.
    // Try the default flags first and if compilation fails, try some alternatives.
    std::vector<CompileConfig> configs;
    configs.push_back(CompileConfig(flags, "default"));
    configs.push_back(CompileConfig(flags | D3DCOMPILE_SKIP_VALIDATION, "skip validation"));
    configs.push_back(CompileConfig(flags | D3DCOMPILE_SKIP_OPTIMIZATION, "skip optimization"));

    if (getMajorShaderModel() == 4 && getShaderModelSuffix() != "")
    {
        // Some shaders might cause a "blob content mismatch between level9 and d3d10 shader".
        // e.g. dEQP-GLES2.functional.shaders.struct.local.loop_nested_struct_array_*.
        // Using the [unroll] directive works around this, as does this D3DCompile flag.
        configs.push_back(
            CompileConfig(flags | D3DCOMPILE_AVOID_FLOW_CONTROL, "avoid flow control"));
    }

    D3D_SHADER_MACRO loopMacros[] = {{"ANGLE_ENABLE_LOOP_FLATTEN", "1"}, {0, 0}};

    angle::ComPtr<ID3DBlob> binary;
    std::string debugInfo;
    ANGLE_TRY(mCompiler.compileToBinary(context, infoLog, shaderHLSL, profile, configs, loopMacros,
                                        &binary, &debugInfo));

    // It's possible that binary is NULL if the compiler failed in all configurations.  Set the
    // executable to NULL and return GL_NO_ERROR to signify that there was a link error but the
    // internal state is still OK.
    if (!binary)
    {
        *outExectuable = nullptr;
        return angle::Result::Continue;
    }

    angle::Result error = loadExecutable(
        context, static_cast<const uint8_t *>(binary->GetBufferPointer()), binary->GetBufferSize(),
        type, streamOutVaryings, separatedOutputBuffers, outExectuable);

    if (error == angle::Result::Stop)
    {
        return error;
    }

    if (!debugInfo.empty())
    {
        (*outExectuable)->appendDebugInfo(debugInfo);
    }

    return angle::Result::Continue;
}

angle::Result Renderer11::ensureHLSLCompilerInitialized(d3d::Context *context)
{
    return mCompiler.ensureInitialized(context);
}

UniformStorageD3D *Renderer11::createUniformStorage(size_t storageSize)
{
    return new UniformStorage11(storageSize);
}

VertexBuffer *Renderer11::createVertexBuffer()
{
    return new VertexBuffer11(this);
}

IndexBuffer *Renderer11::createIndexBuffer()
{
    return new IndexBuffer11(this);
}

StreamProducerImpl *Renderer11::createStreamProducerD3DTexture(
    egl::Stream::ConsumerType consumerType,
    const egl::AttributeMap &attribs)
{
    return new StreamProducerD3DTexture(this);
}

bool Renderer11::supportsFastCopyBufferToTexture(GLenum internalFormat) const
{
    ASSERT(getNativeExtensions().pixelBufferObjectNV);

    const gl::InternalFormat &internalFormatInfo = gl::GetSizedInternalFormatInfo(internalFormat);
    const d3d11::Format &d3d11FormatInfo =
        d3d11::Format::Get(internalFormat, mRenderer11DeviceCaps);

    // sRGB formats do not work with D3D11 buffer SRVs
    if (internalFormatInfo.colorEncoding == GL_SRGB)
    {
        return false;
    }

    // We cannot support direct copies to non-color-renderable formats
    if (d3d11FormatInfo.rtvFormat == DXGI_FORMAT_UNKNOWN)
    {
        return false;
    }

    // We skip all 3-channel formats since sometimes format support is missing
    if (internalFormatInfo.componentCount == 3)
    {
        return false;
    }

    // We don't support formats which we can't represent without conversion
    if (d3d11FormatInfo.format().glInternalFormat != internalFormat)
    {
        return false;
    }

    // Buffer SRV creation for this format was not working on Windows 10.
    if (d3d11FormatInfo.texFormat == DXGI_FORMAT_B5G5R5A1_UNORM)
    {
        return false;
    }

    // This format is not supported as a buffer SRV.
    if (d3d11FormatInfo.texFormat == DXGI_FORMAT_A8_UNORM)
    {
        return false;
    }

    return true;
}

angle::Result Renderer11::fastCopyBufferToTexture(const gl::Context *context,
                                                  const gl::PixelUnpackState &unpack,
                                                  gl::Buffer *unpackBuffer,
                                                  unsigned int offset,
                                                  RenderTargetD3D *destRenderTarget,
                                                  GLenum destinationFormat,
                                                  GLenum sourcePixelsType,
                                                  const gl::Box &destArea)
{
    ASSERT(supportsFastCopyBufferToTexture(destinationFormat));
    return mPixelTransfer->copyBufferToTexture(context, unpack, unpackBuffer, offset,
                                               destRenderTarget, destinationFormat,
                                               sourcePixelsType, destArea);
}

ImageD3D *Renderer11::createImage()
{
    return new Image11(this);
}

ExternalImageSiblingImpl *Renderer11::createExternalImageSibling(const gl::Context *context,
                                                                 EGLenum target,
                                                                 EGLClientBuffer buffer,
                                                                 const egl::AttributeMap &attribs)
{
    switch (target)
    {
        case EGL_D3D11_TEXTURE_ANGLE:
            return new ExternalImageSiblingImpl11(this, buffer, attribs);

        default:
            UNREACHABLE();
            return nullptr;
    }
}

angle::Result Renderer11::generateMipmap(const gl::Context *context, ImageD3D *dest, ImageD3D *src)
{
    Image11 *dest11 = GetAs<Image11>(dest);
    Image11 *src11  = GetAs<Image11>(src);
    return Image11::GenerateMipmap(context, dest11, src11, mRenderer11DeviceCaps);
}

angle::Result Renderer11::generateMipmapUsingD3D(const gl::Context *context,
                                                 TextureStorage *storage,
                                                 const gl::TextureState &textureState)
{
    TextureStorage11 *storage11 = GetAs<TextureStorage11>(storage);

    ASSERT(storage11->isRenderTarget());
    ASSERT(storage11->supportsNativeMipmapFunction());

    const d3d11::SharedSRV *srv = nullptr;
    ANGLE_TRY(storage11->getSRVLevels(context, textureState.getEffectiveBaseLevel(),
                                      textureState.getEffectiveMaxLevel(), false, &srv));

    mDeviceContext->GenerateMips(srv->get());

    return angle::Result::Continue;
}

angle::Result Renderer11::copyImage(const gl::Context *context,
                                    ImageD3D *dest,
                                    ImageD3D *source,
                                    const gl::Box &sourceBox,
                                    const gl::Offset &destOffset,
                                    bool unpackFlipY,
                                    bool unpackPremultiplyAlpha,
                                    bool unpackUnmultiplyAlpha)
{
    Image11 *dest11 = GetAs<Image11>(dest);
    Image11 *src11  = GetAs<Image11>(source);
    return Image11::CopyImage(context, dest11, src11, sourceBox, destOffset, unpackFlipY,
                              unpackPremultiplyAlpha, unpackUnmultiplyAlpha, mRenderer11DeviceCaps);
}

TextureStorage *Renderer11::createTextureStorage2D(SwapChainD3D *swapChain,
                                                   const std::string &label)
{
    SwapChain11 *swapChain11 = GetAs<SwapChain11>(swapChain);
    return new TextureStorage11_2D(this, swapChain11, label);
}

TextureStorage *Renderer11::createTextureStorageEGLImage(EGLImageD3D *eglImage,
                                                         RenderTargetD3D *renderTargetD3D,
                                                         const std::string &label)
{
    return new TextureStorage11_EGLImage(this, eglImage, GetAs<RenderTarget11>(renderTargetD3D),
                                         label);
}

TextureStorage *Renderer11::createTextureStorageExternal(
    egl::Stream *stream,
    const egl::Stream::GLTextureDescription &desc,
    const std::string &label)
{
    return new TextureStorage11_External(this, stream, desc, label);
}

TextureStorage *Renderer11::createTextureStorage2D(GLenum internalformat,
                                                   BindFlags bindFlags,
                                                   GLsizei width,
                                                   GLsizei height,
                                                   int levels,
                                                   const std::string &label,
                                                   bool hintLevelZeroOnly)
{
    return new TextureStorage11_2D(this, internalformat, bindFlags, width, height, levels, label,
                                   hintLevelZeroOnly);
}

TextureStorage *Renderer11::createTextureStorageCube(GLenum internalformat,
                                                     BindFlags bindFlags,
                                                     int size,
                                                     int levels,
                                                     bool hintLevelZeroOnly,
                                                     const std::string &label)
{
    return new TextureStorage11_Cube(this, internalformat, bindFlags, size, levels,
                                     hintLevelZeroOnly, label);
}

TextureStorage *Renderer11::createTextureStorage3D(GLenum internalformat,
                                                   BindFlags bindFlags,
                                                   GLsizei width,
                                                   GLsizei height,
                                                   GLsizei depth,
                                                   int levels,
                                                   const std::string &label)
{
    return new TextureStorage11_3D(this, internalformat, bindFlags, width, height, depth, levels,
                                   label);
}

TextureStorage *Renderer11::createTextureStorage2DArray(GLenum internalformat,
                                                        BindFlags bindFlags,
                                                        GLsizei width,
                                                        GLsizei height,
                                                        GLsizei depth,
                                                        int levels,
                                                        const std::string &label)
{
    return new TextureStorage11_2DArray(this, internalformat, bindFlags, width, height, depth,
                                        levels, label);
}

TextureStorage *Renderer11::createTextureStorage2DMultisample(GLenum internalformat,
                                                              GLsizei width,
                                                              GLsizei height,
                                                              int levels,
                                                              int samples,
                                                              bool fixedSampleLocations,
                                                              const std::string &label)
{
    return new TextureStorage11_2DMultisample(this, internalformat, width, height, levels, samples,
                                              fixedSampleLocations, label);
}

TextureStorage *Renderer11::createTextureStorageBuffer(
    const gl::OffsetBindingPointer<gl::Buffer> &buffer,
    GLenum internalFormat,
    const std::string &label)
{
    return new TextureStorage11_Buffer(this, buffer, internalFormat, label);
}

TextureStorage *Renderer11::createTextureStorage2DMultisampleArray(GLenum internalformat,
                                                                   GLsizei width,
                                                                   GLsizei height,
                                                                   GLsizei depth,
                                                                   int levels,
                                                                   int samples,
                                                                   bool fixedSampleLocations,
                                                                   const std::string &label)
{
    return new TextureStorage11_2DMultisampleArray(this, internalformat, width, height, depth,
                                                   levels, samples, fixedSampleLocations, label);
}

angle::Result Renderer11::readFromAttachment(const gl::Context *context,
                                             const gl::FramebufferAttachment &srcAttachment,
                                             const gl::Rectangle &sourceArea,
                                             GLenum format,
                                             GLenum type,
                                             GLuint outputPitch,
                                             const gl::PixelPackState &pack,
                                             uint8_t *pixelsOut)
{
    ASSERT(sourceArea.width >= 0);
    ASSERT(sourceArea.height >= 0);

    const bool invertTexture = UsePresentPathFast(this, &srcAttachment);

    RenderTarget11 *rt11 = nullptr;
    ANGLE_TRY(srcAttachment.getRenderTarget(context, 0, &rt11));
    ASSERT(rt11->getTexture().valid());

    const TextureHelper11 &textureHelper = rt11->getTexture();
    unsigned int sourceSubResource       = rt11->getSubresourceIndex();

    const gl::Extents &texSize = textureHelper.getExtents();

    gl::Rectangle actualArea = sourceArea;
    bool reverseRowOrder     = pack.reverseRowOrder;
    if (invertTexture)
    {
        actualArea.y    = texSize.height - actualArea.y - actualArea.height;
        reverseRowOrder = !reverseRowOrder;
    }

    // Clamp read region to the defined texture boundaries, preventing out of bounds reads
    // and reads of uninitialized data.
    gl::Rectangle safeArea;
    safeArea.x = gl::clamp(actualArea.x, 0, texSize.width);
    safeArea.y = gl::clamp(actualArea.y, 0, texSize.height);
    safeArea.width =
        gl::clamp(actualArea.width + std::min(actualArea.x, 0), 0, texSize.width - safeArea.x);
    safeArea.height =
        gl::clamp(actualArea.height + std::min(actualArea.y, 0), 0, texSize.height - safeArea.y);

    ASSERT(safeArea.x >= 0 && safeArea.y >= 0);
    ASSERT(safeArea.x + safeArea.width <= texSize.width);
    ASSERT(safeArea.y + safeArea.height <= texSize.height);

    if (safeArea.width == 0 || safeArea.height == 0)
    {
        // no work to do
        return angle::Result::Continue;
    }

    gl::Extents safeSize(safeArea.width, safeArea.height, 1);

    // Intermediate texture used for copy for multiplanar formats or resolving multisampled
    // textures.
    TextureHelper11 intermediateTextureHelper;

    // "srcTexture" usually points to the source texture.
    // For 2D multisampled textures, it points to the multisampled resolve texture.
    const TextureHelper11 *srcTexture = &textureHelper;

    if (textureHelper.is2D())
    {
        // For multiplanar d3d11 textures, perform a copy before reading.
        if (d3d11::IsSupportedMultiplanarFormat(textureHelper.getFormat()))
        {
            D3D11_TEXTURE2D_DESC planeDesc;
            planeDesc.Width              = static_cast<UINT>(safeSize.width);
            planeDesc.Height             = static_cast<UINT>(safeSize.height);
            planeDesc.MipLevels          = 1;
            planeDesc.ArraySize          = 1;
            planeDesc.Format             = textureHelper.getFormatSet().srvFormat;
            planeDesc.SampleDesc.Count   = 1;
            planeDesc.SampleDesc.Quality = 0;
            planeDesc.Usage              = D3D11_USAGE_DEFAULT;
            planeDesc.BindFlags          = D3D11_BIND_RENDER_TARGET;
            planeDesc.CPUAccessFlags     = 0;
            planeDesc.MiscFlags          = 0;

            GLenum internalFormat = textureHelper.getFormatSet().internalFormat;
            ANGLE_TRY(allocateTexture(GetImplAs<Context11>(context), planeDesc,
                                      d3d11::Format::Get(internalFormat, mRenderer11DeviceCaps),
                                      &intermediateTextureHelper));
            intermediateTextureHelper.setInternalName(
                "readFromAttachment::intermediateTextureHelper");

            Context11 *context11 = GetImplAs<Context11>(context);
            d3d11::RenderTargetView rtv;
            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
            rtvDesc.Format             = textureHelper.getFormatSet().rtvFormat;
            rtvDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice = 0;

            ANGLE_TRY(allocateResource(context11, rtvDesc, intermediateTextureHelper.get(), &rtv));
            rtv.setInternalName("readFromAttachment.RTV");

            d3d11::SharedSRV srv;
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            srvDesc.Format                    = textureHelper.getFormatSet().srvFormat;
            srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels       = 1;

            ANGLE_TRY(allocateResource(context11, srvDesc, textureHelper.get(), &srv));
            srv.setInternalName("readFromAttachment.SRV");

            gl::Box srcGlBox(safeArea.x, safeArea.y, 0, safeArea.width, safeArea.height, 1);
            gl::Box destGlBox(0, 0, 0, safeSize.width, safeSize.height, 1);

            // Perform a copy to planeTexture as we cannot read directly from NV12 d3d11 textures.
            ANGLE_TRY(mBlit->copyTexture(
                context, srv, srcGlBox, safeSize, internalFormat, rtv, destGlBox, safeSize, nullptr,
                gl::GetUnsizedFormat(internalFormat), GL_NONE, GL_NEAREST, false, false, false));

            // Update safeArea based on the destination.
            safeArea.x      = destGlBox.x;
            safeArea.y      = destGlBox.y;
            safeArea.width  = destGlBox.width;
            safeArea.height = destGlBox.height;

            sourceSubResource = 0;
            srcTexture        = &intermediateTextureHelper;
        }
        else
        {
            if (textureHelper.getSampleCount() > 1)
            {
                D3D11_TEXTURE2D_DESC resolveDesc;
                resolveDesc.Width              = static_cast<UINT>(texSize.width);
                resolveDesc.Height             = static_cast<UINT>(texSize.height);
                resolveDesc.MipLevels          = 1;
                resolveDesc.ArraySize          = 1;
                resolveDesc.Format             = textureHelper.getFormat();
                resolveDesc.SampleDesc.Count   = 1;
                resolveDesc.SampleDesc.Quality = 0;
                resolveDesc.Usage              = D3D11_USAGE_DEFAULT;
                resolveDesc.BindFlags          = 0;
                resolveDesc.CPUAccessFlags     = 0;
                resolveDesc.MiscFlags          = 0;

                ANGLE_TRY(allocateTexture(GetImplAs<Context11>(context), resolveDesc,
                                          textureHelper.getFormatSet(),
                                          &intermediateTextureHelper));
                intermediateTextureHelper.setInternalName(
                    "readFromAttachment::intermediateTextureHelper");

                mDeviceContext->ResolveSubresource(intermediateTextureHelper.get(), 0,
                                                   textureHelper.get(), sourceSubResource,
                                                   textureHelper.getFormat());

                sourceSubResource = 0;
                srcTexture        = &intermediateTextureHelper;
            }
        }
    }

    D3D11_BOX srcBox;
    srcBox.left   = static_cast<UINT>(safeArea.x);
    srcBox.right  = static_cast<UINT>(safeArea.x + safeArea.width);
    srcBox.top    = static_cast<UINT>(safeArea.y);
    srcBox.bottom = static_cast<UINT>(safeArea.y + safeArea.height);

    // Select the correct layer from a 3D attachment
    srcBox.front = 0;
    if (textureHelper.is3D())
    {
        srcBox.front = static_cast<UINT>(srcAttachment.layer());
    }
    srcBox.back = srcBox.front + 1;

    TextureHelper11 stagingHelper;
    ANGLE_TRY(createStagingTexture(context, textureHelper.getTextureType(),
                                   srcTexture->getFormatSet(), safeSize, StagingAccess::READ,
                                   &stagingHelper));
    stagingHelper.setInternalName("readFromAttachment::stagingHelper");

    mDeviceContext->CopySubresourceRegion(stagingHelper.get(), 0, 0, 0, 0, srcTexture->get(),
                                          sourceSubResource, &srcBox);

    const angle::Format &angleFormat = GetFormatFromFormatType(format, type);
    gl::Buffer *packBuffer = context->getState().getTargetBuffer(gl::BufferBinding::PixelPack);

    PackPixelsParams packParams(safeArea, angleFormat, outputPitch, reverseRowOrder, packBuffer, 0);
    return packPixels(context, stagingHelper, packParams, pixelsOut);
}

angle::Result Renderer11::packPixels(const gl::Context *context,
                                     const TextureHelper11 &textureHelper,
                                     const PackPixelsParams &params,
                                     uint8_t *pixelsOut)
{
    ID3D11Resource *readResource = textureHelper.get();

    D3D11_MAPPED_SUBRESOURCE mapping;
    ANGLE_TRY(mapResource(context, readResource, 0, D3D11_MAP_READ, 0, &mapping));

    uint8_t *source = static_cast<uint8_t *>(mapping.pData);
    int inputPitch  = static_cast<int>(mapping.RowPitch);

    const auto &formatInfo = textureHelper.getFormatSet();
    ASSERT(formatInfo.format().glInternalFormat != GL_NONE);

    PackPixels(params, formatInfo.format(), inputPitch, source, pixelsOut);

    mDeviceContext->Unmap(readResource, 0);

    return angle::Result::Continue;
}

angle::Result Renderer11::blitRenderbufferRect(const gl::Context *context,
                                               const gl::Rectangle &readRectIn,
                                               const gl::Rectangle &drawRectIn,
                                               UINT readLayer,
                                               UINT drawLayer,
                                               RenderTargetD3D *readRenderTarget,
                                               RenderTargetD3D *drawRenderTarget,
                                               GLenum filter,
                                               const gl::Rectangle *scissor,
                                               bool colorBlit,
                                               bool depthBlit,
                                               bool stencilBlit)
{
    // Since blitRenderbufferRect is called for each render buffer that needs to be blitted,
    // it should never be the case that both color and depth/stencil need to be blitted at
    // at the same time.
    ASSERT(colorBlit != (depthBlit || stencilBlit));

    RenderTarget11 *drawRenderTarget11 = GetAs<RenderTarget11>(drawRenderTarget);
    ASSERT(drawRenderTarget11);

    const TextureHelper11 &drawTexture = drawRenderTarget11->getTexture();
    unsigned int drawSubresource       = drawRenderTarget11->getSubresourceIndex();

    RenderTarget11 *readRenderTarget11 = GetAs<RenderTarget11>(readRenderTarget);
    ASSERT(readRenderTarget11);

    TextureHelper11 readTexture;
    unsigned int readSubresource = 0;
    d3d11::SharedSRV readSRV;

    if (readRenderTarget->isMultisampled())
    {
        ANGLE_TRY(resolveMultisampledTexture(context, readRenderTarget11, depthBlit, stencilBlit,
                                             &readTexture));

        if (!stencilBlit)
        {
            const auto &readFormatSet = readTexture.getFormatSet();

            D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
            viewDesc.Format                    = readFormatSet.srvFormat;
            viewDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
            viewDesc.Texture2D.MipLevels       = 1;
            viewDesc.Texture2D.MostDetailedMip = 0;

            ANGLE_TRY(allocateResource(GetImplAs<Context11>(context), viewDesc, readTexture.get(),
                                       &readSRV));
        }
    }
    else
    {
        ASSERT(readRenderTarget11);
        readTexture     = readRenderTarget11->getTexture();
        readSubresource = readRenderTarget11->getSubresourceIndex();
        const d3d11::SharedSRV *blitSRV;
        ANGLE_TRY(readRenderTarget11->getBlitShaderResourceView(context, &blitSRV));
        readSRV = blitSRV->makeCopy();
        if (!readSRV.valid())
        {
            ASSERT(depthBlit || stencilBlit);
            const d3d11::SharedSRV *srv;
            ANGLE_TRY(readRenderTarget11->getShaderResourceView(context, &srv));
            readSRV = srv->makeCopy();
        }
        ASSERT(readSRV.valid());
    }

    // Stencil blits don't use shaders.
    ASSERT(readSRV.valid() || stencilBlit);

    const gl::Extents readSize(readRenderTarget->getWidth(), readRenderTarget->getHeight(), 1);
    const gl::Extents drawSize(drawRenderTarget->getWidth(), drawRenderTarget->getHeight(), 1);

    // From the spec:
    // "The actual region taken from the read framebuffer is limited to the intersection of the
    // source buffers being transferred, which may include the color buffer selected by the read
    // buffer, the depth buffer, and / or the stencil buffer depending on mask."
    // This means negative x and y are out of bounds, and not to be read from. We handle this here
    // by internally scaling the read and draw rectangles.

    // Remove reversal from readRect to simplify further operations.
    gl::Rectangle readRect = readRectIn;
    gl::Rectangle drawRect = drawRectIn;
    if (readRect.isReversedX())
    {
        readRect.x     = readRect.x + readRect.width;
        readRect.width = -readRect.width;
        drawRect.x     = drawRect.x + drawRect.width;
        drawRect.width = -drawRect.width;
    }
    if (readRect.isReversedY())
    {
        readRect.y      = readRect.y + readRect.height;
        readRect.height = -readRect.height;
        drawRect.y      = drawRect.y + drawRect.height;
        drawRect.height = -drawRect.height;
    }

    gl::Rectangle readBounds(0, 0, readSize.width, readSize.height);
    gl::Rectangle inBoundsReadRect;
    if (!gl::ClipRectangle(readRect, readBounds, &inBoundsReadRect))
    {
        return angle::Result::Continue;
    }

    {
        // Calculate the drawRect that corresponds to inBoundsReadRect.
        auto readToDrawX = [&drawRect, &readRect](int readOffset) {
            double readToDrawScale =
                static_cast<double>(drawRect.width) / static_cast<double>(readRect.width);
            return static_cast<int>(
                round(static_cast<double>(readOffset - readRect.x) * readToDrawScale) + drawRect.x);
        };
        auto readToDrawY = [&drawRect, &readRect](int readOffset) {
            double readToDrawScale =
                static_cast<double>(drawRect.height) / static_cast<double>(readRect.height);
            return static_cast<int>(
                round(static_cast<double>(readOffset - readRect.y) * readToDrawScale) + drawRect.y);
        };

        gl::Rectangle drawRectMatchingInBoundsReadRect;
        drawRectMatchingInBoundsReadRect.x = readToDrawX(inBoundsReadRect.x);
        drawRectMatchingInBoundsReadRect.y = readToDrawY(inBoundsReadRect.y);
        drawRectMatchingInBoundsReadRect.width =
            readToDrawX(inBoundsReadRect.x1()) - drawRectMatchingInBoundsReadRect.x;
        drawRectMatchingInBoundsReadRect.height =
            readToDrawY(inBoundsReadRect.y1()) - drawRectMatchingInBoundsReadRect.y;
        drawRect = drawRectMatchingInBoundsReadRect;
        readRect = inBoundsReadRect;
    }

    bool scissorNeeded = false;
    if (scissor)
    {
        gl::Rectangle scissoredDrawRect;
        if (!gl::ClipRectangle(drawRect, *scissor, &scissoredDrawRect))
        {
            return angle::Result::Continue;
        }
        scissorNeeded = scissoredDrawRect != drawRect;
    }

    const auto &destFormatInfo =
        gl::GetSizedInternalFormatInfo(drawRenderTarget->getInternalFormat());
    const auto &srcFormatInfo =
        gl::GetSizedInternalFormatInfo(readRenderTarget->getInternalFormat());
    const auto &formatSet    = drawRenderTarget11->getFormatSet();
    const auto &nativeFormat = formatSet.format();

    // Some blits require masking off emulated texture channels. eg: from RGBA8 to RGB8, we
    // emulate RGB8 with RGBA8, so we need to mask off the alpha channel when we copy.

    gl::Color<bool> colorMask;
    colorMask.red =
        (srcFormatInfo.redBits > 0) && (destFormatInfo.redBits == 0) && (nativeFormat.redBits > 0);
    colorMask.green = (srcFormatInfo.greenBits > 0) && (destFormatInfo.greenBits == 0) &&
                      (nativeFormat.greenBits > 0);
    colorMask.blue = (srcFormatInfo.blueBits > 0) && (destFormatInfo.blueBits == 0) &&
                     (nativeFormat.blueBits > 0);
    colorMask.alpha = (srcFormatInfo.alphaBits > 0) && (destFormatInfo.alphaBits == 0) &&
                      (nativeFormat.alphaBits > 0);

    // We only currently support masking off the alpha channel.
    bool colorMaskingNeeded = colorMask.alpha;
    ASSERT(!colorMask.red && !colorMask.green && !colorMask.blue);

    bool wholeBufferCopy = !scissorNeeded && !colorMaskingNeeded && readRect.x == 0 &&
                           readRect.width == readSize.width && readRect.y == 0 &&
                           readRect.height == readSize.height && drawRect.x == 0 &&
                           drawRect.width == drawSize.width && drawRect.y == 0 &&
                           drawRect.height == drawSize.height;

    bool stretchRequired = readRect.width != drawRect.width || readRect.height != drawRect.height;

    ASSERT(!readRect.isReversedX() && !readRect.isReversedY());
    bool reversalRequired = drawRect.isReversedX() || drawRect.isReversedY();

    bool outOfBounds = readRect.x < 0 || readRect.x + readRect.width > readSize.width ||
                       readRect.y < 0 || readRect.y + readRect.height > readSize.height ||
                       drawRect.x < 0 || drawRect.x + drawRect.width > drawSize.width ||
                       drawRect.y < 0 || drawRect.y + drawRect.height > drawSize.height;

    bool partialDSBlit =
        (nativeFormat.depthBits > 0 && depthBlit) != (nativeFormat.stencilBits > 0 && stencilBlit);

    if (drawRenderTarget->getSamples() == readRenderTarget->getSamples() &&
        readRenderTarget11->getFormatSet().formatID ==
            drawRenderTarget11->getFormatSet().formatID &&
        !stretchRequired && !outOfBounds && !reversalRequired && !partialDSBlit &&
        !colorMaskingNeeded && (!(depthBlit || stencilBlit) || wholeBufferCopy))
    {
        UINT dstX = drawRect.x;
        UINT dstY = drawRect.y;
        UINT dstZ = drawLayer;

        D3D11_BOX readBox;
        readBox.left   = readRect.x;
        readBox.right  = readRect.x + readRect.width;
        readBox.top    = readRect.y;
        readBox.bottom = readRect.y + readRect.height;
        readBox.front  = readLayer;
        readBox.back   = readLayer + 1;

        if (scissorNeeded)
        {
            // drawRect is guaranteed to have positive width and height because stretchRequired is
            // false.
            ASSERT(drawRect.width >= 0 || drawRect.height >= 0);

            if (drawRect.x < scissor->x)
            {
                dstX = scissor->x;
                readBox.left += (scissor->x - drawRect.x);
            }
            if (drawRect.y < scissor->y)
            {
                dstY = scissor->y;
                readBox.top += (scissor->y - drawRect.y);
            }
            if (drawRect.x + drawRect.width > scissor->x + scissor->width)
            {
                readBox.right -= ((drawRect.x + drawRect.width) - (scissor->x + scissor->width));
            }
            if (drawRect.y + drawRect.height > scissor->y + scissor->height)
            {
                readBox.bottom -= ((drawRect.y + drawRect.height) - (scissor->y + scissor->height));
            }
        }

        // D3D11 needs depth-stencil CopySubresourceRegions to have a NULL pSrcBox
        // We also require complete framebuffer copies for depth-stencil blit.
        D3D11_BOX *pSrcBox = wholeBufferCopy && readLayer == 0 ? nullptr : &readBox;

        mDeviceContext->CopySubresourceRegion(drawTexture.get(), drawSubresource, dstX, dstY, dstZ,
                                              readTexture.get(), readSubresource, pSrcBox);
    }
    else
    {
        gl::Box readArea(readRect.x, readRect.y, 0, readRect.width, readRect.height, 1);
        gl::Box drawArea(drawRect.x, drawRect.y, 0, drawRect.width, drawRect.height, 1);

        if (depthBlit && stencilBlit)
        {
            ANGLE_TRY(mBlit->copyDepthStencil(context, readTexture, readSubresource, readArea,
                                              readSize, drawTexture, drawSubresource, drawArea,
                                              drawSize, scissor));
        }
        else if (depthBlit)
        {
            const d3d11::DepthStencilView &drawDSV = drawRenderTarget11->getDepthStencilView();
            ASSERT(readSRV.valid());
            ANGLE_TRY(mBlit->copyDepth(context, readSRV, readArea, readSize, drawDSV, drawArea,
                                       drawSize, scissor));
        }
        else if (stencilBlit)
        {
            ANGLE_TRY(mBlit->copyStencil(context, readTexture, readSubresource, readArea, readSize,
                                         drawTexture, drawSubresource, drawArea, drawSize,
                                         scissor));
        }
        else
        {
            const d3d11::RenderTargetView &drawRTV = drawRenderTarget11->getRenderTargetView();

            // We don't currently support masking off any other channel than alpha
            bool maskOffAlpha = colorMaskingNeeded && colorMask.alpha;
            ASSERT(readSRV.valid());
            ANGLE_TRY(mBlit->copyTexture(context, readSRV, readArea, readSize, srcFormatInfo.format,
                                         drawRTV, drawArea, drawSize, scissor,
                                         destFormatInfo.format, GL_NONE, filter, maskOffAlpha,
                                         false, false));
        }
    }

    return angle::Result::Continue;
}

bool Renderer11::isES3Capable() const
{
    return (d3d11_gl::GetMaximumClientVersion(mRenderer11DeviceCaps).major > 2);
}

RendererClass Renderer11::getRendererClass() const
{
    return RENDERER_D3D11;
}

void Renderer11::onSwap()
{
    // Send histogram updates every half hour
    const double kHistogramUpdateInterval = 30 * 60;

    auto *platform                   = ANGLEPlatformCurrent();
    const double currentTime         = platform->monotonicallyIncreasingTime(platform);
    const double timeSinceLastUpdate = currentTime - mLastHistogramUpdateTime;

    if (timeSinceLastUpdate > kHistogramUpdateInterval)
    {
        updateHistograms();
        mLastHistogramUpdateTime = currentTime;
    }
}

void Renderer11::updateHistograms()
{
    // Update the buffer CPU memory histogram
    {
        size_t sizeSum = 0;
        for (const Buffer11 *buffer : mAliveBuffers)
        {
            sizeSum += buffer->getTotalCPUBufferMemoryBytes();
        }
        const int kOneMegaByte = 1024 * 1024;
        ANGLE_HISTOGRAM_MEMORY_MB("GPU.ANGLE.Buffer11CPUMemoryMB",
                                  static_cast<int>(sizeSum) / kOneMegaByte);
    }
}

void Renderer11::onBufferCreate(const Buffer11 *created)
{
    mAliveBuffers.insert(created);
}

void Renderer11::onBufferDelete(const Buffer11 *deleted)
{
    mAliveBuffers.erase(deleted);
}

angle::Result Renderer11::resolveMultisampledTexture(const gl::Context *context,
                                                     RenderTarget11 *renderTarget,
                                                     bool depth,
                                                     bool stencil,
                                                     TextureHelper11 *textureOut)
{
    if (depth && !stencil)
    {
        return mBlit->resolveDepth(context, renderTarget, textureOut);
    }

    if (stencil)
    {
        return mBlit->resolveStencil(context, renderTarget, depth, textureOut);
    }

    const auto &formatSet = renderTarget->getFormatSet();

    ASSERT(renderTarget->isMultisampled());
    const d3d11::SharedSRV *sourceSRV;
    ANGLE_TRY(renderTarget->getShaderResourceView(context, &sourceSRV));
    D3D11_SHADER_RESOURCE_VIEW_DESC sourceSRVDesc;
    sourceSRV->get()->GetDesc(&sourceSRVDesc);
    ASSERT(sourceSRVDesc.ViewDimension == D3D_SRV_DIMENSION_TEXTURE2DMS ||
           sourceSRVDesc.ViewDimension == D3D_SRV_DIMENSION_TEXTURE2DMSARRAY);

    if (!mCachedResolveTexture.valid() ||
        mCachedResolveTexture.getExtents().width != renderTarget->getWidth() ||
        mCachedResolveTexture.getExtents().height != renderTarget->getHeight() ||
        mCachedResolveTexture.getFormat() != formatSet.texFormat)
    {
        D3D11_TEXTURE2D_DESC resolveDesc;
        resolveDesc.Width              = renderTarget->getWidth();
        resolveDesc.Height             = renderTarget->getHeight();
        resolveDesc.MipLevels          = 1;
        resolveDesc.ArraySize          = 1;
        resolveDesc.Format             = formatSet.texFormat;
        resolveDesc.SampleDesc.Count   = 1;
        resolveDesc.SampleDesc.Quality = 0;
        resolveDesc.Usage              = D3D11_USAGE_DEFAULT;
        resolveDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
        resolveDesc.CPUAccessFlags     = 0;
        resolveDesc.MiscFlags          = 0;

        ANGLE_TRY(allocateTexture(GetImplAs<Context11>(context), resolveDesc, formatSet,
                                  &mCachedResolveTexture));
    }

    mDeviceContext->ResolveSubresource(mCachedResolveTexture.get(), 0,
                                       renderTarget->getTexture().get(),
                                       renderTarget->getSubresourceIndex(), formatSet.texFormat);
    *textureOut = mCachedResolveTexture;
    return angle::Result::Continue;
}

bool Renderer11::getLUID(LUID *adapterLuid) const
{
    adapterLuid->HighPart = 0;
    adapterLuid->LowPart  = 0;

    if (!mDxgiAdapter)
    {
        return false;
    }

    DXGI_ADAPTER_DESC adapterDesc;
    if (FAILED(mDxgiAdapter->GetDesc(&adapterDesc)))
    {
        return false;
    }

    *adapterLuid = adapterDesc.AdapterLuid;
    return true;
}

VertexConversionType Renderer11::getVertexConversionType(angle::FormatID vertexFormatID) const
{
    return d3d11::GetVertexFormatInfo(vertexFormatID, mRenderer11DeviceCaps.featureLevel)
        .conversionType;
}

GLenum Renderer11::getVertexComponentType(angle::FormatID vertexFormatID) const
{
    const auto &format =
        d3d11::GetVertexFormatInfo(vertexFormatID, mRenderer11DeviceCaps.featureLevel);
    return d3d11::GetComponentType(format.nativeFormat);
}

angle::Result Renderer11::getVertexSpaceRequired(const gl::Context *context,
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

    unsigned int elementCount  = 0;
    const unsigned int divisor = binding.getDivisor();
    if (instances == 0 || divisor == 0)
    {
        // This could be a clipped cast.
        elementCount = gl::clampCast<unsigned int>(count);
    }
    else
    {
        // Round up to divisor, if possible
        elementCount =
            UnsignedCeilDivide(static_cast<unsigned int>(instances + baseInstance), divisor);
    }

    ASSERT(elementCount > 0);

    const D3D_FEATURE_LEVEL featureLevel = mRenderer11DeviceCaps.featureLevel;
    const d3d11::VertexFormat &vertexFormatInfo =
        d3d11::GetVertexFormatInfo(attrib.format->id, featureLevel);
    const d3d11::DXGIFormatSize &dxgiFormatInfo =
        d3d11::GetDXGIFormatSizeInfo(vertexFormatInfo.nativeFormat);
    unsigned int elementSize = dxgiFormatInfo.pixelBytes;
    bool check = (elementSize > std::numeric_limits<unsigned int>::max() / elementCount);
    ANGLE_CHECK(GetImplAs<Context11>(context), !check,
                "New vertex buffer size would result in an overflow.", GL_OUT_OF_MEMORY);

    *bytesRequiredOut = elementSize * elementCount;
    return angle::Result::Continue;
}

void Renderer11::generateCaps(gl::Caps *outCaps,
                              gl::TextureCapsMap *outTextureCaps,
                              gl::Extensions *outExtensions,
                              gl::Limitations *outLimitations,
                              ShPixelLocalStorageOptions *outPLSOptions) const
{
    d3d11_gl::GenerateCaps(mDevice.Get(), mDeviceContext.Get(), mRenderer11DeviceCaps,
                           getFeatures(), mDescription, outCaps, outTextureCaps, outExtensions,
                           outLimitations, outPLSOptions);
}

void Renderer11::initializeFeatures(angle::FeaturesD3D *features) const
{
    ApplyFeatureOverrides(features, mDisplay->getState().featureOverrides);
    if (!mDisplay->getState().featureOverrides.allDisabled)
    {
        d3d11::InitializeFeatures(mRenderer11DeviceCaps, mAdapterDescription, features);
    }
}

void Renderer11::initializeFrontendFeatures(angle::FrontendFeatures *features) const
{
    ApplyFeatureOverrides(features, mDisplay->getState().featureOverrides);
    if (!mDisplay->getState().featureOverrides.allDisabled)
    {
        d3d11::InitializeFrontendFeatures(mAdapterDescription, features);
    }
}

DeviceImpl *Renderer11::createEGLDevice()
{
    return new Device11(mDevice.Get());
}

ContextImpl *Renderer11::createContext(const gl::State &state, gl::ErrorSet *errorSet)
{
    return new Context11(state, errorSet, this);
}

FramebufferImpl *Renderer11::createDefaultFramebuffer(const gl::FramebufferState &state)
{
    return new Framebuffer11(state, this);
}

angle::Result Renderer11::getScratchMemoryBuffer(Context11 *context11,
                                                 size_t requestedSize,
                                                 angle::MemoryBuffer **bufferOut)
{
    ANGLE_CHECK_GL_ALLOC(context11, mScratchMemoryBuffer.get(requestedSize, bufferOut));
    return angle::Result::Continue;
}

gl::Version Renderer11::getMaxSupportedESVersion() const
{
    return d3d11_gl::GetMaximumClientVersion(mRenderer11DeviceCaps);
}

gl::Version Renderer11::getMaxConformantESVersion() const
{
    // 3.1 support is in progress.
    return std::min(getMaxSupportedESVersion(), gl::Version(3, 0));
}

DebugAnnotatorContext11 *Renderer11::getDebugAnnotatorContext()
{
    return &mAnnotatorContext;
}

angle::Result Renderer11::dispatchCompute(const gl::Context *context,
                                          GLuint numGroupsX,
                                          GLuint numGroupsY,
                                          GLuint numGroupsZ)
{
    const gl::State &glState                = context->getState();
    const gl::ProgramExecutable *executable = glState.getProgramExecutable();
    if (executable->getShaderStorageBlocks().size() > 0 ||
        executable->getAtomicCounterBuffers().size() > 0)
    {
        ANGLE_TRY(markRawBufferUsage(context));
    }
    ANGLE_TRY(markTypedBufferUsage(context));
    ANGLE_TRY(mStateManager.updateStateForCompute(context, numGroupsX, numGroupsY, numGroupsZ));
    mDeviceContext->Dispatch(numGroupsX, numGroupsY, numGroupsZ);

    return angle::Result::Continue;
}
angle::Result Renderer11::dispatchComputeIndirect(const gl::Context *context, GLintptr indirect)
{
    const auto &glState                     = context->getState();
    const gl::ProgramExecutable *executable = glState.getProgramExecutable();
    if (executable->getShaderStorageBlocks().size() > 0 ||
        executable->getAtomicCounterBuffers().size() > 0)
    {
        ANGLE_TRY(markRawBufferUsage(context));
    }

    auto *dispatchIndirectBuffer = glState.getTargetBuffer(gl::BufferBinding::DispatchIndirect);
    ASSERT(dispatchIndirectBuffer);

    Buffer11 *storage         = GetImplAs<Buffer11>(dispatchIndirectBuffer);
    const uint8_t *bufferData = nullptr;
    // TODO(jie.a.chen@intel.com): num_groups_x,y,z have to be written into the driver constant
    // buffer for the built-in variable gl_NumWorkGroups. There is an opportunity for optimization
    // to use GPU->GPU copy instead.
    // http://anglebug.com/42261508
    ANGLE_TRY(storage->getData(context, &bufferData));
    const GLuint *groups = reinterpret_cast<const GLuint *>(bufferData + indirect);
    ANGLE_TRY(mStateManager.updateStateForCompute(context, groups[0], groups[1], groups[2]));

    ID3D11Buffer *buffer = nullptr;
    ANGLE_TRY(storage->getBuffer(context, BUFFER_USAGE_INDIRECT, &buffer));

    mDeviceContext->DispatchIndirect(buffer, static_cast<UINT>(indirect));
    return angle::Result::Continue;
}

angle::Result Renderer11::createStagingTexture(const gl::Context *context,
                                               ResourceType textureType,
                                               const d3d11::Format &formatSet,
                                               const gl::Extents &size,
                                               StagingAccess readAndWriteAccess,
                                               TextureHelper11 *textureOut)
{
    Context11 *context11 = GetImplAs<Context11>(context);

    if (textureType == ResourceType::Texture2D)
    {
        D3D11_TEXTURE2D_DESC stagingDesc;
        stagingDesc.Width              = size.width;
        stagingDesc.Height             = size.height;
        stagingDesc.MipLevels          = 1;
        stagingDesc.ArraySize          = 1;
        stagingDesc.Format             = formatSet.texFormat;
        stagingDesc.SampleDesc.Count   = 1;
        stagingDesc.SampleDesc.Quality = 0;
        stagingDesc.Usage              = D3D11_USAGE_STAGING;
        stagingDesc.BindFlags          = 0;
        stagingDesc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ;
        stagingDesc.MiscFlags          = 0;

        if (readAndWriteAccess == StagingAccess::READ_WRITE)
        {
            stagingDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
        }

        ANGLE_TRY(allocateTexture(context11, stagingDesc, formatSet, textureOut));
        return angle::Result::Continue;
    }
    ASSERT(textureType == ResourceType::Texture3D);

    D3D11_TEXTURE3D_DESC stagingDesc;
    stagingDesc.Width          = size.width;
    stagingDesc.Height         = size.height;
    stagingDesc.Depth          = 1;
    stagingDesc.MipLevels      = 1;
    stagingDesc.Format         = formatSet.texFormat;
    stagingDesc.Usage          = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags      = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.MiscFlags      = 0;

    ANGLE_TRY(allocateTexture(context11, stagingDesc, formatSet, textureOut));
    return angle::Result::Continue;
}

angle::Result Renderer11::allocateTexture(d3d::Context *context,
                                          const D3D11_TEXTURE2D_DESC &desc,
                                          const d3d11::Format &format,
                                          const D3D11_SUBRESOURCE_DATA *initData,
                                          TextureHelper11 *textureOut)
{
    d3d11::Texture2D texture;
    ANGLE_TRY(mResourceManager11.allocate(context, this, &desc, initData, &texture));
    textureOut->init(std::move(texture), desc, format);
    return angle::Result::Continue;
}

angle::Result Renderer11::allocateTexture(d3d::Context *context,
                                          const D3D11_TEXTURE3D_DESC &desc,
                                          const d3d11::Format &format,
                                          const D3D11_SUBRESOURCE_DATA *initData,
                                          TextureHelper11 *textureOut)
{
    d3d11::Texture3D texture;
    ANGLE_TRY(mResourceManager11.allocate(context, this, &desc, initData, &texture));
    textureOut->init(std::move(texture), desc, format);
    return angle::Result::Continue;
}

angle::Result Renderer11::getBlendState(const gl::Context *context,
                                        const d3d11::BlendStateKey &key,
                                        const d3d11::BlendState **outBlendState)
{
    return mStateCache.getBlendState(context, this, key, outBlendState);
}

angle::Result Renderer11::getRasterizerState(const gl::Context *context,
                                             const gl::RasterizerState &rasterState,
                                             bool scissorEnabled,
                                             ID3D11RasterizerState **outRasterizerState)
{
    return mStateCache.getRasterizerState(context, this, rasterState, scissorEnabled,
                                          outRasterizerState);
}

angle::Result Renderer11::getDepthStencilState(const gl::Context *context,
                                               const gl::DepthStencilState &dsState,
                                               const d3d11::DepthStencilState **outDSState)
{
    return mStateCache.getDepthStencilState(context, this, dsState, outDSState);
}

angle::Result Renderer11::getSamplerState(const gl::Context *context,
                                          const gl::SamplerState &samplerState,
                                          ID3D11SamplerState **outSamplerState)
{
    return mStateCache.getSamplerState(context, this, samplerState, outSamplerState);
}

UINT Renderer11::getSampleDescQuality(GLuint supportedSamples) const
{
    // Per the documentation on
    // https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_standard_multisample_quality_levels
    // applications can only request the standard multisample pattern on
    // feature levels 10_1 and above.
    if (supportedSamples > 0 && mDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_10_1)
    {
        return D3D11_STANDARD_MULTISAMPLE_PATTERN;
    }
    return 0;
}

angle::Result Renderer11::clearRenderTarget(const gl::Context *context,
                                            RenderTargetD3D *renderTarget,
                                            const gl::ColorF &clearColorValue,
                                            const float clearDepthValue,
                                            const unsigned int clearStencilValue)
{
    RenderTarget11 *rt11 = GetAs<RenderTarget11>(renderTarget);

    if (rt11->getFormatSet().dsvFormat != DXGI_FORMAT_UNKNOWN)
    {
        ASSERT(rt11->getDepthStencilView().valid());

        const auto &format    = rt11->getFormatSet();
        const UINT clearFlags = (format.format().depthBits > 0 ? D3D11_CLEAR_DEPTH : 0) |
                                (format.format().stencilBits ? D3D11_CLEAR_STENCIL : 0);
        mDeviceContext->ClearDepthStencilView(rt11->getDepthStencilView().get(), clearFlags,
                                              clearDepthValue,
                                              static_cast<UINT8>(clearStencilValue));
        return angle::Result::Continue;
    }

    ASSERT(rt11->getRenderTargetView().valid());
    ID3D11RenderTargetView *rtv = rt11->getRenderTargetView().get();

    // There are complications with some types of RTV and FL 9_3 with ClearRenderTargetView.
    // See https://msdn.microsoft.com/en-us/library/windows/desktop/ff476388(v=vs.85).aspx
    ASSERT(mRenderer11DeviceCaps.featureLevel > D3D_FEATURE_LEVEL_9_3 || !IsArrayRTV(rtv));

    const auto &d3d11Format = rt11->getFormatSet();
    const auto &glFormat    = gl::GetSizedInternalFormatInfo(renderTarget->getInternalFormat());

    gl::ColorF safeClearColor = clearColorValue;

    if (d3d11Format.format().alphaBits > 0 && glFormat.alphaBits == 0)
    {
        safeClearColor.alpha = 1.0f;
    }

    mDeviceContext->ClearRenderTargetView(rtv, &safeClearColor.red);
    return angle::Result::Continue;
}

bool Renderer11::canSelectViewInVertexShader() const
{
    return !getFeatures().selectViewInGeometryShader.enabled &&
           getRenderer11DeviceCaps().supportsVpRtIndexWriteFromVertexShader;
}

angle::Result Renderer11::mapResource(const gl::Context *context,
                                      ID3D11Resource *resource,
                                      UINT subResource,
                                      D3D11_MAP mapType,
                                      UINT mapFlags,
                                      D3D11_MAPPED_SUBRESOURCE *mappedResource)
{
    HRESULT hr = mDeviceContext->Map(resource, subResource, mapType, mapFlags, mappedResource);
    ANGLE_TRY_HR(GetImplAs<Context11>(context), hr, "Failed to map D3D11 resource.");
    return angle::Result::Continue;
}

angle::Result Renderer11::markTypedBufferUsage(const gl::Context *context)
{
    const gl::State &glState = context->getState();
    ProgramExecutableD3D *executableD3D =
        GetImplAs<ProgramExecutableD3D>(glState.getProgramExecutable());
    gl::RangeUI imageRange = executableD3D->getUsedImageRange(gl::ShaderType::Compute, false);
    for (unsigned int imageIndex = imageRange.low(); imageIndex < imageRange.high(); imageIndex++)
    {
        GLint imageUnitIndex = executableD3D->getImageMapping(gl::ShaderType::Compute, imageIndex,
                                                              false, context->getCaps());
        ASSERT(imageUnitIndex != -1);
        const gl::ImageUnit &imageUnit = glState.getImageUnit(imageUnitIndex);
        if (imageUnit.texture.get()->getType() == gl::TextureType::Buffer)
        {
            Buffer11 *buffer11 = GetImplAs<Buffer11>(imageUnit.texture.get()->getBuffer().get());
            ANGLE_TRY(buffer11->markTypedBufferUsage(context));
        }
    }
    return angle::Result::Continue;
}

angle::Result Renderer11::markRawBufferUsage(const gl::Context *context)
{
    const gl::State &glState                = context->getState();
    const gl::ProgramExecutable *executable = glState.getProgramExecutable();
    for (size_t blockIndex = 0; blockIndex < executable->getShaderStorageBlocks().size();
         blockIndex++)
    {
        GLuint binding = executable->getShaderStorageBlockBinding(static_cast<GLuint>(blockIndex));
        const auto &shaderStorageBuffer = glState.getIndexedShaderStorageBuffer(binding);
        if (shaderStorageBuffer.get() != nullptr)
        {
            Buffer11 *bufferStorage = GetImplAs<Buffer11>(shaderStorageBuffer.get());
            ANGLE_TRY(bufferStorage->markRawBufferUsage(context));
        }
    }

    const std::vector<gl::AtomicCounterBuffer> &atomicCounterBuffers =
        executable->getAtomicCounterBuffers();
    for (size_t index = 0; index < atomicCounterBuffers.size(); ++index)
    {
        const GLuint binding = executable->getAtomicCounterBufferBinding(index);
        const auto &buffer   = glState.getIndexedAtomicCounterBuffer(binding);

        if (buffer.get() != nullptr)
        {
            Buffer11 *bufferStorage = GetImplAs<Buffer11>(buffer.get());
            ANGLE_TRY(bufferStorage->markRawBufferUsage(context));
        }
    }
    return angle::Result::Continue;
}

angle::Result Renderer11::markTransformFeedbackUsage(const gl::Context *context)
{
    const gl::State &glState                       = context->getState();
    const gl::TransformFeedback *transformFeedback = glState.getCurrentTransformFeedback();
    for (size_t i = 0; i < transformFeedback->getIndexedBufferCount(); i++)
    {
        const gl::OffsetBindingPointer<gl::Buffer> &binding =
            transformFeedback->getIndexedBuffer(i);
        if (binding.get() != nullptr)
        {
            BufferD3D *bufferD3D = GetImplAs<BufferD3D>(binding.get());
            ANGLE_TRY(bufferD3D->markTransformFeedbackUsage(context));
        }
    }

    return angle::Result::Continue;
}

angle::Result Renderer11::getIncompleteTexture(const gl::Context *context,
                                               gl::TextureType type,
                                               gl::Texture **textureOut)
{
    return GetImplAs<Context11>(context)->getIncompleteTexture(context, type, textureOut);
}

std::string Renderer11::getVendorString() const
{
    return GetVendorString(mAdapterDescription.VendorId);
}

std::string Renderer11::getVersionString(bool includeFullVersion) const
{
    std::ostringstream versionString;
    versionString << "D3D11";
    if (includeFullVersion && mRenderer11DeviceCaps.driverVersion.valid())
    {
        versionString << "-" << GetDriverVersionString(mRenderer11DeviceCaps.driverVersion.value());
    }
    return versionString.str();
}

RendererD3D *CreateRenderer11(egl::Display *display)
{
    return new Renderer11(display);
}

}  // namespace rx
