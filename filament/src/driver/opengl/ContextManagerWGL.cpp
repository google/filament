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

#include "driver/opengl/ContextManagerWGL.h"

#include <Wingdi.h>

#include "driver/opengl/OpenGLDriver.h"

#include "Windows.h"
#include <GL/gl.h>
#include "GL/glext.h"
#include "GL/wglext.h"

#include <utils/Log.h>
#include <utils/Panic.h>

namespace {

void reportLastWindowsError() {
    LPSTR lpMessageBuffer;
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

using namespace driver;

std::unique_ptr<Driver> ContextManagerWGL::createDriver(void* const sharedGLContext) noexcept {
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

    mHWnd = CreateWindowA("STATIC", "dummy", 0, 0, 0, 1, 1, NULL, NULL, NULL, NULL);
    HDC whdc = mWhdc = GetDC(mHWnd);
    if (whdc == NULL) {
        utils::slog.e << "CreateWindowA() failed" << utils::io::endl;
        goto error;
    }

    int pixelFormat = ChoosePixelFormat(whdc, &mPfd);
    SetPixelFormat(whdc, pixelFormat, &mPfd);

    // We need a tmp context to retrieve and call wglCreateContextAttribsARB.
    HGLRC tempContext = wglCreateContext(whdc);
    if (!wglMakeCurrent(whdc, tempContext)) {
        utils::slog.e << "wglMakeCurrent() failed, whdc=" << whdc << ", tempContext=" << tempContext << utils::io::endl;
        goto error;
    }

    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribs =
            (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");
    mContext = wglCreateContextAttribs(whdc, nullptr, attribs);
    if (!mContext) {
        utils::slog.e << "wglCreateContextAttribs() failed, whdc=" << whdc << utils::io::endl;
        goto error;
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempContext);
    tempContext = NULL;

    if (!wglMakeCurrent(whdc, mContext)) {
        utils::slog.e << "wglMakeCurrent() failed, whdc=" << whdc << ", mContext=" << mContext << utils::io::endl;
        goto error;
    }

    int result = bluegl::bind();
    ASSERT_POSTCONDITION(!result, "Unable to load OpenGL entry points.");
    return OpenGLDriver::create(this, sharedGLContext);

error:
    if (tempContext) {
        wglDeleteContext(tempContext);
    }
    reportLastWindowsError();
    terminate();
    return NULL;
}

void ContextManagerWGL::terminate() noexcept {
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

ExternalContext::SwapChain* ContextManagerWGL::createSwapChain(void* nativeWindow, uint64_t& flags) noexcept {
    // on Windows, the nativeWindow maps directly to a HDC
    HDC hdc = (HDC) nativeWindow;
    if (!ASSERT_POSTCONDITION_NON_FATAL(hdc,
            "Unable to create the SwapChain (nativeWindow = %p)", nativeWindow)) {
        reportLastWindowsError();
    }

	// We have to match pixel formats across the HDC and HGLRC (mContext)
    int pixelFormat = ChoosePixelFormat(hdc, &mPfd);
    SetPixelFormat(hdc, pixelFormat, &mPfd);

    SwapChain* swapChain = (SwapChain *)hdc;
    return swapChain;
}

void ContextManagerWGL::destroySwapChain(ExternalContext::SwapChain* swapChain) noexcept {
    // make this swapChain not current (by making a dummy one current)
    wglMakeCurrent(mWhdc, mContext);
}

void ContextManagerWGL::makeCurrent(ExternalContext::SwapChain* swapChain) noexcept {
    HDC hdc = (HDC)(swapChain);
    if (hdc != NULL) {
        BOOL success = wglMakeCurrent(hdc, mContext);
        if (!ASSERT_POSTCONDITION_NON_FATAL(success, "wglMakeCurrent() failed. hdc = %p", hdc)) {
            reportLastWindowsError();
            wglMakeCurrent(0, NULL);
        }
    }
}

void ContextManagerWGL::commit(ExternalContext::SwapChain* swapChain) noexcept {
    HDC hdc = (HDC)(swapChain);
    if (hdc != NULL) {
        SwapBuffers(hdc);
    }
}

//TODO Implement WGL fences
ExternalContext::Fence* ContextManagerWGL::createFence() noexcept {
    return nullptr;
}

void ContextManagerWGL::destroyFence(Fence* fence) noexcept {
}

driver::FenceStatus ContextManagerWGL::waitFence(Fence* fence, uint64_t timeout) noexcept {
    return driver::FenceStatus::ERROR;
}

} // namespace filament
