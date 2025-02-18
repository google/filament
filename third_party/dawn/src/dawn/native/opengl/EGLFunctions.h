// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_OPENGL_EGLFUNCTIONS_H_
#define SRC_DAWN_NATIVE_OPENGL_EGLFUNCTIONS_H_

#include "dawn/common/egl_platform.h"

#include "dawn/common/ityp_bitset.h"
#include "dawn/native/Error.h"

namespace dawn::native::opengl {

enum class EGLExt {
    // Promoted to EGL 1.5
    ClientExtensions,
    PlatformBase,
    CLEvent2,
    WaitSync,
    ImageBase,
    GLTexture2DImage,
    GLTexture3DImage,
    GLTextureCubemapImage,
    GLRenderBufferImage,
    CreateContext,
    CreateContextRobustness,
    GetAllProcAddresses,
    ClientGetAllProcAddresses,
    GLColorSpace,
    SurfacelessContext,

    // Other extensions,
    FenceSync,  // Not marked as promoted due to different function prototypes
    DisplayTextureShareGroup,
    ReusableSync,
    NoConfigContext,
    PixelFormatFloat,
    GLColorspace,
    NativeFenceSync,  // EGL_ANDROID_native_fence_sync

    // EGL image creation extensions
    ImageNativeBuffer,      // EGL_ANDROID_image_native_buffer
    GetNativeClientBuffer,  // EGL_ANDROID_get_native_client_buffer

    // ANGLE specific
    ANGLECreateContextBackwardsCompatible,  // EGL_ANGLE_create_context_backwards_compatible
    ANGLECreateContextExtensionsEnabled,    // EGL_ANGLE_create_context_extensions_enabled

    EnumCount,
};

// An EGL function loader that also takes care of discovering which extensions are available.
// (taking into account the ones that have been promoted to core EGL versions).
class EGLFunctions {
  public:
    MaybeError LoadClientProcs(EGLGetProcProc getProc);
    MaybeError LoadDisplayProcs(EGLDisplay display);

    uint32_t GetMajorVersion() const;
    uint32_t GetMinorVersion() const;
    bool IsAtLeastVersion(uint32_t major, uint32_t minor) const;
    bool HasExt(EGLExt extension) const;

    // EGL 1.0
    PFNEGLGETPROCADDRESSPROC GetProcAddress;

    PFNEGLCHOOSECONFIGPROC ChooseConfig;
    PFNEGLCOPYBUFFERSPROC CopyBuffers;
    PFNEGLCREATECONTEXTPROC CreateContext;
    PFNEGLCREATEPBUFFERSURFACEPROC CreatePbufferSurface;
    PFNEGLCREATEPIXMAPSURFACEPROC CreatePixmapSurface;
    PFNEGLCREATEWINDOWSURFACEPROC CreateWindowSurface;
    PFNEGLDESTROYCONTEXTPROC DestroyContext;
    PFNEGLDESTROYSURFACEPROC DestroySurface;
    PFNEGLGETCONFIGATTRIBPROC GetConfigAttrib;
    PFNEGLGETCONFIGSPROC GetConfigs;
    PFNEGLGETCURRENTDISPLAYPROC GetCurrentDisplay;
    PFNEGLGETCURRENTSURFACEPROC GetCurrentSurface;
    PFNEGLGETDISPLAYPROC GetDisplay;
    PFNEGLGETERRORPROC GetError;
    PFNEGLINITIALIZEPROC Initialize;
    PFNEGLMAKECURRENTPROC MakeCurrent;
    PFNEGLQUERYCONTEXTPROC QueryContext;
    PFNEGLQUERYSTRINGPROC QueryString;
    PFNEGLQUERYSURFACEPROC QuerySurface;
    PFNEGLSWAPBUFFERSPROC SwapBuffers;
    PFNEGLTERMINATEPROC Terminate;
    PFNEGLWAITGLPROC WaitGL;
    PFNEGLWAITNATIVEPROC WaitNative;

    // EGL 1.1
    PFNEGLBINDTEXIMAGEPROC BindTexImage;
    PFNEGLRELEASETEXIMAGEPROC ReleaseTexImage;
    PFNEGLSURFACEATTRIBPROC SurfaceAttrib;
    PFNEGLSWAPINTERVALPROC SwapInterval;

    // EGL 1.2
    PFNEGLBINDAPIPROC BindAPI;
    PFNEGLQUERYAPIPROC QueryAPI;
    PFNEGLCREATEPBUFFERFROMCLIENTBUFFERPROC CreatePbufferFromClientBuffer;
    PFNEGLRELEASETHREADPROC ReleaseThread;
    PFNEGLWAITCLIENTPROC WaitClient;

    // EGL 1.3 (no new procs)

    // EGL 1.4
    PFNEGLGETCURRENTCONTEXTPROC GetCurrentContext;

    // EGL 1.5
    PFNEGLCREATESYNCPROC CreateSync;
    PFNEGLDESTROYSYNCPROC DestroySync;
    PFNEGLCLIENTWAITSYNCPROC ClientWaitSync;
    PFNEGLGETSYNCATTRIBPROC GetSyncAttrib;
    PFNEGLCREATEIMAGEPROC CreateImage;
    PFNEGLDESTROYIMAGEPROC DestroyImage;
    PFNEGLGETPLATFORMDISPLAYPROC GetPlatformDisplay;
    PFNEGLCREATEPLATFORMWINDOWSURFACEPROC CreatePlatformWindowSurface;
    PFNEGLCREATEPLATFORMPIXMAPSURFACEPROC CreatePlatformPixmapSurface;
    PFNEGLWAITSYNCPROC WaitSync;

    // EGL_KHR_fence_sync
    // NOTE: These functions use attribute lists with EGLint but the core versions use EGLattrib.
    // They are not compatible.
    PFNEGLCREATESYNCKHRPROC CreateSyncKHR;
    PFNEGLDESTROYSYNCKHRPROC DestroySyncKHR;
    PFNEGLCLIENTWAITSYNCKHRPROC ClientWaitSyncKHR;
    PFNEGLGETSYNCATTRIBKHRPROC GetSyncAttribKHR;

    // EGL_KHR_reusable_sync
    PFNEGLSIGNALSYNCKHRPROC SignalSync;

    // EGL_ANDROID_get_native_client_buffer
    PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC GetNativeClientBuffer;

    // EGL_ANDROID_native_fence_sync
    PFNEGLDUPNATIVEFENCEFDANDROIDPROC DupNativeFenceFD;

  private:
    MaybeError LoadClientExtensions();

    uint32_t mMajorVersion = 0;
    uint32_t mMinorVersion = 0;

    ityp::bitset<EGLExt, static_cast<size_t>(EGLExt::EnumCount)> mExtensions;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_EGLFUNCTIONS_H_
