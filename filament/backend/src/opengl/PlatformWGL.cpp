/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "PlatformWGL.h"

#include <Wingdi.h>

#include "OpenGLDriverFactory.h"

#ifdef _MSC_VER
    // this variable is checked in BlueGL.h (included from "gl_headers.h" right after this), 
    // and prevents duplicate definition of OpenGL apis when building this file. 
    // However, GL_GLEXT_PROTOTYPES need to be defined in BlueGL.h when included from other files.
    #define FILAMENT_PLATFORM_WGL
#endif

#include "gl_headers.h"

#include "Windows.h"
#include <GL/gl.h>
#include "GL/glext.h"
#include "GL/wglext.h"

#include <utils/Log.h>
#include <utils/Panic.h>

namespace {

void reportLastWindowsError() {
    LPSTR lpMessageBuffer = nullptr;
    DWORD dwError = GetLastError();

    if (dwError == 0) {
        return;
    }

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        dwError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        lpMessageBuffer,
        0, nullptr
	);

    utils::slog.e << "Windows error code: " << dwError << ". " << lpMessageBuffer
            << utils::io::endl;

    LocalFree(lpMessageBuffer);
}

} // namespace

namespace filament {

using namespace backend;

struct WGLSwapChain {
    HDC hDc = NULL;
    HWND hWnd = NULL;
    bool isHeadless = false;
};

Driver* PlatformWGL::createDriver(void* const sharedGLContext) noexcept {
    mPfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
        PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
        32,                   // Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                   // Number of bits for the depthbuffer
        0,                    // Number of bits for the stencilbuffer
        0,                    // Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 1,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_PROFILE_MASK_ARB,
        0
    };

    HGLRC tempContext = NULL;

    mHWnd = CreateWindowA("STATIC", "dummy", 0, 0, 0, 1, 1, NULL, NULL, NULL, NULL);
    HDC whdc = mWhdc = GetDC(mHWnd);
    if (whdc == NULL) {
        utils::slog.e << "CreateWindowA() failed" << utils::io::endl;
        goto error;
    }

    int pixelFormat = ChoosePixelFormat(whdc, &mPfd);
    SetPixelFormat(whdc, pixelFormat, &mPfd);

    // We need a tmp context to retrieve and call wglCreateContextAttribsARB.
    tempContext = wglCreateContext(whdc);
    if (!wglMakeCurrent(whdc, tempContext)) {
        utils::slog.e << "wglMakeCurrent() failed, whdc=" << whdc << ", tempContext=" <<
                tempContext << utils::io::endl;
        goto error;
    }

    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribs =
            (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");
    mContext = wglCreateContextAttribs(whdc, (HGLRC) sharedGLContext, attribs);
    if (!mContext) {
        utils::slog.e << "wglCreateContextAttribs() failed, whdc=" << whdc << utils::io::endl;
        goto error;
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempContext);
    tempContext = NULL;

    if (!wglMakeCurrent(whdc, mContext)) {
        utils::slog.e << "wglMakeCurrent() failed, whdc=" << whdc << ", mContext=" <<
                mContext << utils::io::endl;
        goto error;
    }

    int result = bluegl::bind();
    ASSERT_POSTCONDITION(!result, "Unable to load OpenGL entry points.");
    return OpenGLDriverFactory::create(this, sharedGLContext);

error:
    if (tempContext) {
        wglDeleteContext(tempContext);
    }
    reportLastWindowsError();
    terminate();
    return NULL;
}

void PlatformWGL::terminate() noexcept {
    wglMakeCurrent(NULL, NULL);
    if (mContext) {
        wglDeleteContext(mContext);
        mContext = NULL;
    }
    if (mHWnd && mWhdc) {
        ReleaseDC(mHWnd, mWhdc);
        DestroyWindow(mHWnd);
        mHWnd = NULL;
        mWhdc = NULL;
    } else if (mHWnd) {
        DestroyWindow(mHWnd);
        mHWnd = NULL;
    }
    bluegl::unbind();
}

Platform::SwapChain* PlatformWGL::createSwapChain(void* nativeWindow, uint64_t& flags) noexcept {
    auto* swapChain = new WGLSwapChain();
    swapChain->isHeadless = false;

    // on Windows, the nativeWindow maps to a HWND
    swapChain->hWnd = (HWND) nativeWindow;
    swapChain->hDc = GetDC(swapChain->hWnd);
    if (!ASSERT_POSTCONDITION_NON_FATAL(swapChain->hDc,
            "Unable to create the SwapChain (nativeWindow = %p)", nativeWindow)) {
        reportLastWindowsError();
    }

	// We have to match pixel formats across the HDC and HGLRC (mContext)
    int pixelFormat = ChoosePixelFormat(swapChain->hDc, &mPfd);
    SetPixelFormat(swapChain->hDc, pixelFormat, &mPfd);

    return (Platform::SwapChain*) swapChain;
}

Platform::SwapChain* PlatformWGL::createSwapChain(uint32_t width, uint32_t height, uint64_t& flags) noexcept {
    auto* swapChain = new WGLSwapChain();
    swapChain->isHeadless = true;

    // WS_POPUP was chosen for the window style here after some experimentation.
    // For some reason, using other window styles resulted in corrupted pixel buffers when using
    // readPixels.
    RECT rect = {0, 0, width, height};
    AdjustWindowRect(&rect, WS_POPUP, FALSE);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    swapChain->hWnd = CreateWindowA("STATIC", "headless", WS_POPUP, 0, 0,
            width, height, NULL, NULL, NULL, NULL);
    swapChain->hDc = GetDC(swapChain->hWnd);
    int pixelFormat = ChoosePixelFormat(swapChain->hDc, &mPfd);
    SetPixelFormat(swapChain->hDc, pixelFormat, &mPfd);

    return (Platform::SwapChain*) swapChain;
}

void PlatformWGL::destroySwapChain(Platform::SwapChain* swapChain) noexcept {
    auto* wglSwapChain = (WGLSwapChain*) swapChain;

    HDC dc = wglSwapChain->hDc;
    HWND window = wglSwapChain->hWnd;
    ReleaseDC(window, dc);

    if (wglSwapChain->isHeadless) {
        DestroyWindow(window);
    }

    delete wglSwapChain;

    // make this swapChain not current (by making a dummy one current)
    wglMakeCurrent(mWhdc, mContext);
}

void PlatformWGL::makeCurrent(Platform::SwapChain* drawSwapChain,
                              Platform::SwapChain* readSwapChain) noexcept {
    ASSERT_PRECONDITION_NON_FATAL(drawSwapChain == readSwapChain,
                                  "PlatformWGL does not support distinct draw/read swap chains.");

    auto* wglSwapChain = (WGLSwapChain*) drawSwapChain;
    HDC hdc = wglSwapChain->hDc;
    if (hdc != NULL) {
        BOOL success = wglMakeCurrent(hdc, mContext);
        if (!ASSERT_POSTCONDITION_NON_FATAL(success, "wglMakeCurrent() failed. hdc = %p", hdc)) {
            reportLastWindowsError();
            wglMakeCurrent(0, NULL);
        }
    }
}

void PlatformWGL::commit(Platform::SwapChain* swapChain) noexcept {
    auto* wglSwapChain = (WGLSwapChain*) swapChain;
    HDC hdc = wglSwapChain->hDc;
    if (hdc != NULL) {
        SwapBuffers(hdc);
    }
}

//TODO Implement WGL fences
Platform::Fence* PlatformWGL::createFence() noexcept {
    return nullptr;
}

void PlatformWGL::destroyFence(Fence* fence) noexcept {
}

backend::FenceStatus PlatformWGL::waitFence(Fence* fence, uint64_t timeout) noexcept {
    return backend::FenceStatus::ERROR;
}

} // namespace filament
