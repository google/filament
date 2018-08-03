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

#include <utils/Panic.h>

namespace filament {

using namespace driver;

std::unique_ptr<Driver> ContextManagerWGL::createDriver(void* const sharedGLContext) noexcept {
    PIXELFORMATDESCRIPTOR pfd = {
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

    HWND hWnd= CreateWindowA("STATIC", "dummy", 0, 0, 0, 1, 1, NULL, NULL, NULL, NULL);
    HDC whdc = GetDC(hWnd);

    int pixelFormat = ChoosePixelFormat(whdc, &pfd);
    SetPixelFormat(whdc, pixelFormat, &pfd);

    int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 1,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_PROFILE_MASK_ARB  ,
        0
    };

    // We need a tmp context to retrieve and call wglCreateContextAttribsARB.
    HGLRC tempContext = wglCreateContext(whdc);
    wglMakeCurrent(whdc, tempContext);

    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribs =
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    mContext = wglCreateContextAttribs(whdc, nullptr, attribs);

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempContext);
    wglMakeCurrent(whdc, mContext);

    int result = bluegl::bind();
    ASSERT_POSTCONDITION(!result, "Unable to load OpenGL entry points.");
    return OpenGLDriver::create(this, sharedGLContext);
}

void ContextManagerWGL::terminate() noexcept {
    bluegl::unbind();
}

ExternalContext::SwapChain* ContextManagerWGL::createSwapChain(void* nativeWindow, uint64_t& flags) noexcept {
    return (SwapChain*) nativeWindow;
}

void ContextManagerWGL::destroySwapChain(ExternalContext::SwapChain* swapChain) noexcept {
}

void ContextManagerWGL::makeCurrent(ExternalContext::SwapChain* swapChain) noexcept {
    HDC hdc = (HDC)(swapChain);
    wglMakeCurrent(hdc, mContext);
}

void ContextManagerWGL::commit(ExternalContext::SwapChain* swapChain) noexcept {
    HDC hdc = (HDC)(swapChain);
    SwapBuffers(hdc);
}

//TODO Implement WGL fences
ExternalContext::Fence* ContextManagerWGL::createFence() noexcept {
    Fence* f = new Fence();
    return f;
}

void ContextManagerWGL::destroyFence(Fence* fence) noexcept {
    delete fence;
}

driver::FenceStatus ContextManagerWGL::waitFence(Fence* fence, uint64_t timeout) noexcept {
    return driver::FenceStatus::CONDITION_SATISFIED;
}

} // namespace filament
