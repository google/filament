/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "PlatformGLX.h"

#include <utils/Log.h>
#include <utils/Panic.h>

#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/glxext.h>

#include "OpenGLDriver.h"

#include <dlfcn.h>

#include <iostream>

#define LIBRARY_GLX "libGL.so.1"
#define LIBRARY_X11 "libX11.so.6"

// Function pointer types for X11 functions
typedef Display* (*X11_OPEN_DISPLAY)(const char*);
typedef Display* (*X11_CLOSE_DISPLAY)(Display*);

// Function pointer types for GLX functions
typedef void (*GLX_DESTROY_CONTEXT)(Display*, GLXContext);
typedef void (*GLX_SWAP_BUFFERS)(Display *dpy, GLXDrawable drawable);
// Stores GLX function pointers and a handle to the system's GLX library
struct GLXFunctions {
    PFNGLXCHOOSEFBCONFIGPROC chooseFbConfig;
    PFNGLXCREATECONTEXTATTRIBSARBPROC createContext;
    PFNGLXCREATEPBUFFERPROC createPbuffer;
    PFNGLXDESTROYPBUFFERPROC destroyPbuffer;
    PFNGLXMAKECONTEXTCURRENTPROC setCurrentContext;
    GLX_DESTROY_CONTEXT destroyContext;
    GLX_SWAP_BUFFERS swapBuffers;
    void* library;
} g_glx;

// Stores X11 function pointers and a handle to the system's X11 library
struct X11Functions {
    X11_OPEN_DISPLAY openDisplay;
    X11_CLOSE_DISPLAY closeDisplay;
    void* library;
} g_x11;

static PFNGLXGETPROCADDRESSPROC getProcAddress;

static bool loadLibraries() {
    g_glx.library = dlopen(LIBRARY_GLX, RTLD_LOCAL | RTLD_NOW);
    if (!g_glx.library) {
        std::cerr << "Could not find library " << LIBRARY_GLX << std::endl;
        return false;
    }

    getProcAddress =
            (PFNGLXGETPROCADDRESSPROC) dlsym(g_glx.library, "glXGetProcAddressARB");

    g_glx.chooseFbConfig = (PFNGLXCHOOSEFBCONFIGPROC)
            getProcAddress((const GLubyte*) "glXChooseFBConfig");
    g_glx.createContext = (PFNGLXCREATECONTEXTATTRIBSARBPROC)
            getProcAddress((const GLubyte*) "glXCreateContextAttribsARB");
    g_glx.createPbuffer = (PFNGLXCREATEPBUFFERPROC)
            getProcAddress((const GLubyte*) "glXCreatePbuffer");
    g_glx.destroyPbuffer = (PFNGLXDESTROYPBUFFERPROC)
            getProcAddress((const GLubyte*) "glXDestroyPbuffer");
    g_glx.setCurrentContext = (PFNGLXMAKECONTEXTCURRENTPROC)
            getProcAddress((const GLubyte*) "glXMakeContextCurrent");
    g_glx.destroyContext = (GLX_DESTROY_CONTEXT)
            getProcAddress((const GLubyte*) "glXDestroyContext");
    g_glx.swapBuffers = (GLX_SWAP_BUFFERS)
            getProcAddress((const GLubyte*) "glXSwapBuffers");


    g_x11.library = dlopen(LIBRARY_X11, RTLD_LOCAL | RTLD_NOW);
    if (!g_x11.library) {
        std::cerr << "Could not find library " << LIBRARY_X11 << std::endl;
        return false;
    }

    g_x11.openDisplay  = (X11_OPEN_DISPLAY)  dlsym(g_x11.library, "XOpenDisplay");
    g_x11.closeDisplay = (X11_CLOSE_DISPLAY) dlsym(g_x11.library, "XCloseDisplay");
    return true;
}

namespace filament {

using namespace backend;

Driver* PlatformGLX::createDriver(void* const sharedGLContext) noexcept {
    loadLibraries();
    // Get the display device
    mGLXDisplay = g_x11.openDisplay(NULL);
    if (!mGLXDisplay) {
        printf("Failed to open X display\n");
        exit(1);
    }

    // Create a context
    static int attribs[] = { GLX_DOUBLEBUFFER, True, None };

    int config_count = 0;
    mGLXConfig = g_glx.chooseFbConfig(mGLXDisplay, DefaultScreen(mGLXDisplay),
            attribs, &config_count);
    if (mGLXConfig == nullptr || config_count == 0) {
        return nullptr;
    }

    PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribs = (PFNGLXCREATECONTEXTATTRIBSARBPROC)
            getProcAddress((GLubyte*) "glXCreateContextAttribsARB");

    if (glXCreateContextAttribs == NULL) {
        utils::slog.i << "Unable to retrieve function pointer" << utils::io::endl;
        return nullptr;
    }

    int context_attribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
            GLX_CONTEXT_MINOR_VERSION_ARB, 1,
            GL_NONE
    };
    mGLXContext = g_glx.createContext(mGLXDisplay, mGLXConfig[0],
            (GLXContext) sharedGLContext, True, context_attribs);

    int pbufferAttribs[] = {
            GLX_PBUFFER_WIDTH,  1,
            GLX_PBUFFER_HEIGHT, 1,
            GL_NONE
    };

    mDummySurface = g_glx.createPbuffer(mGLXDisplay, mGLXConfig[0], pbufferAttribs);
    g_glx.setCurrentContext(mGLXDisplay, mDummySurface, mDummySurface, mGLXContext);

    int result = bluegl::bind();
    ASSERT_POSTCONDITION(!result, "Unable to load OpenGL entry points.");

    return OpenGLDriver::create(this, sharedGLContext);
}

void PlatformGLX::terminate() noexcept {
    g_glx.setCurrentContext(mGLXDisplay, None, None, nullptr);
    g_glx.destroyPbuffer(mGLXDisplay, mDummySurface);
    g_glx.destroyContext(mGLXDisplay, mGLXContext);
    g_x11.closeDisplay(mGLXDisplay);
    bluegl::unbind();
}

Platform::SwapChain* PlatformGLX::createSwapChain(
        void* nativeWindow, uint64_t& flags) noexcept {

    // Transparent swap chain is not supported
    flags &= ~backend::SWAP_CHAIN_CONFIG_TRANSPARENT;
    return (SwapChain*) nativeWindow;
}

void PlatformGLX::destroySwapChain(Platform::SwapChain* /*swapChain*/) noexcept {
}

void PlatformGLX::makeCurrent(
        Platform::SwapChain* drawSwapChain, Platform::SwapChain* readSwapChain) noexcept {
    g_glx.setCurrentContext(mGLXDisplay,
            (GLXDrawable) drawSwapChain, (GLXDrawable) readSwapChain, mGLXContext);
}

void PlatformGLX::commit(Platform::SwapChain* swapChain) noexcept {
    g_glx.swapBuffers(mGLXDisplay, (GLXDrawable)swapChain);
}

// TODO Implement GLX fences
Platform::Fence* PlatformGLX::createFence() noexcept {
    Fence* f = new Fence();
    return f;
}

void PlatformGLX::destroyFence(Fence* fence) noexcept {
    delete fence;
}

backend::FenceStatus PlatformGLX::waitFence(Fence* fence, uint64_t timeout) noexcept {
    return backend::FenceStatus::CONDITION_SATISFIED;
}

} // namespace filament
