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

#include "OpenGLDriverFactory.h"

#include <dlfcn.h>

#include <iostream>

#define LIBRARY_GLX "libGL.so.1"
#define LIBRARY_X11 "libX11.so.6"

// Function pointer types for X11 functions
typedef Display* (* X11_OPEN_DISPLAY)(const char*);
typedef Display* (* X11_CLOSE_DISPLAY)(Display*);

// Function pointer types for GLX functions
typedef void (* GLX_DESTROY_CONTEXT)(Display*, GLXContext);
typedef void (* GLX_SWAP_BUFFERS)(Display* dpy, GLXDrawable drawable);
// Stores GLX function pointers and a handle to the system's GLX library
struct GLXFunctions {
    PFNGLXCHOOSEFBCONFIGPROC chooseFbConfig;
    PFNGLXCREATECONTEXTATTRIBSARBPROC createContext;
    PFNGLXCREATEPBUFFERPROC createPbuffer;
    PFNGLXDESTROYPBUFFERPROC destroyPbuffer;
    PFNGLXMAKECONTEXTCURRENTPROC setCurrentContext;

    /* 
       When creating a shared GL context, we query the used
       GLX_FBCONFIG_ID to make sure our display framebuffer
       attributes match; otherwise making our context current
       results in a BadMatch
       https://gist.github.com/roxlu/c282d642c353ce96ef19b6359c741bcb
    */
    PFNGLXQUERYCONTEXTPROC queryContext;

    /* 
       When creating a shared GL context, we select the matching
       GLXFBConfig that is used by the shared GL context. `getFBConfigs`
       will return all the available GLXFBConfigs.
    */
    PFNGLXGETFBCONFIGSPROC getFbConfigs;

    /*
      When creating a shared GL contect, we iterate over the
      available GLXFBConfigs that are returned by `getFBConfigs`,
      we use `getFbConfigAttrib` to find the matching
      `GLX_FBCONFIG_ID`.
    */
    PFNGLXGETFBCONFIGATTRIBPROC getFbConfigAttrib;

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
            (PFNGLXGETPROCADDRESSPROC)dlsym(g_glx.library, "glXGetProcAddressARB");

    g_glx.chooseFbConfig = (PFNGLXCHOOSEFBCONFIGPROC)
            getProcAddress((const GLubyte*)"glXChooseFBConfig");
    g_glx.createContext = (PFNGLXCREATECONTEXTATTRIBSARBPROC)
            getProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
    g_glx.createPbuffer = (PFNGLXCREATEPBUFFERPROC)
            getProcAddress((const GLubyte*)"glXCreatePbuffer");
    g_glx.destroyPbuffer = (PFNGLXDESTROYPBUFFERPROC)
            getProcAddress((const GLubyte*)"glXDestroyPbuffer");
    g_glx.setCurrentContext = (PFNGLXMAKECONTEXTCURRENTPROC)
            getProcAddress((const GLubyte*)"glXMakeContextCurrent");
    g_glx.destroyContext = (GLX_DESTROY_CONTEXT)
            getProcAddress((const GLubyte*)"glXDestroyContext");
    g_glx.swapBuffers = (GLX_SWAP_BUFFERS)
            getProcAddress((const GLubyte*)"glXSwapBuffers");

    g_glx.queryContext = (PFNGLXQUERYCONTEXTPROC)
            getProcAddress((const GLubyte*)"glXQueryContext");
    g_glx.getFbConfigs = (PFNGLXGETFBCONFIGSPROC)
            getProcAddress((const GLubyte*)"glXGetFBConfigs");
    g_glx.getFbConfigAttrib = (PFNGLXGETFBCONFIGATTRIBPROC)
            getProcAddress((const GLubyte*)"glXGetFBConfigAttrib");

    g_x11.library = dlopen(LIBRARY_X11, RTLD_LOCAL | RTLD_NOW);
    if (!g_x11.library) {
        std::cerr << "Could not find library " << LIBRARY_X11 << std::endl;
        return false;
    }

    g_x11.openDisplay = (X11_OPEN_DISPLAY)dlsym(g_x11.library, "XOpenDisplay");
    g_x11.closeDisplay = (X11_CLOSE_DISPLAY)dlsym(g_x11.library, "XCloseDisplay");
    return true;
}

namespace filament {

using namespace backend;

Driver* PlatformGLX::createDriver(void* const sharedGLContext) noexcept {
    loadLibraries();
    // Get the display device
    mGLXDisplay = g_x11.openDisplay(NULL);
    if (mGLXDisplay == nullptr) {
        utils::slog.e << "Failed to open X display. (exiting)." << utils::io::endl;
        exit(1);
    }

    if (sharedGLContext != nullptr) {
        int r = -1;
        int usedFbId = -1;
        GLXContext sharedCtx = (GLXContext)((void*)sharedGLContext);

        r = g_glx.queryContext(mGLXDisplay, sharedCtx, GLX_FBCONFIG_ID, &usedFbId);
        if (r != 0) {
            utils::slog.e << "Failed to get GLX_FBCONFIG_ID from shared GL context."
                          << utils::io::endl;
            return nullptr;
        }

        int numConfigs = 0;
        GLXFBConfig* fbConfigs = g_glx.getFbConfigs(mGLXDisplay, 0, &numConfigs);

        if (fbConfigs == nullptr) {
            utils::slog.e << "Failed to get the available GLXFBConfigs." << utils::io::endl;
            return nullptr;
        }

        int fbId = 0;
        int fbIndex = -1;

        for (int i = 0; i < numConfigs; ++i) {
            r = g_glx.getFbConfigAttrib(mGLXDisplay, fbConfigs[i], GLX_FBCONFIG_ID, &fbId);
            if (r != 0) {
                utils::slog.e << "Failed to get GLX_FBCONFIG_ID for entry " << i << "."
                              << utils::io::endl;
                continue;
            }

            if (fbId == usedFbId) {
                fbIndex = i;
                break;
            }
        }

        if (fbIndex < 0) {
            utils::slog.e << "Failed to find an `GLXFBConfig` with the requested ID."
                          << utils::io::endl;
            return nullptr;
        }

        mGLXConfig = fbConfigs + fbIndex;
    } else {
        // Create a context
        static int attribs[] = {
                GLX_DOUBLEBUFFER, True,
                GLX_DEPTH_SIZE, 24,
                None
        };

        int configCount = 0;
        mGLXConfig = g_glx.chooseFbConfig(mGLXDisplay, DefaultScreen(mGLXDisplay),
                attribs, &configCount);
        if (mGLXConfig == nullptr || configCount == 0) {
            return nullptr;
        }
    }

    PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribs = (PFNGLXCREATECONTEXTATTRIBSARBPROC)
            getProcAddress((GLubyte*)"glXCreateContextAttribsARB");

    if (glXCreateContextAttribs == nullptr) {
        utils::slog.i << "Unable to retrieve function pointer for `glXCreateContextAttribs()`."
                      << utils::io::endl;
        return nullptr;
    }

    int contextAttribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
            GLX_CONTEXT_MINOR_VERSION_ARB, 1,
            GL_NONE
    };

    mGLXContext = g_glx.createContext(mGLXDisplay, mGLXConfig[0],
            (GLXContext)sharedGLContext, True, contextAttribs);

    int pbufferAttribs[] = {
            GLX_PBUFFER_WIDTH, 1,
            GLX_PBUFFER_HEIGHT, 1,
            GL_NONE
    };

    mDummySurface = g_glx.createPbuffer(mGLXDisplay, mGLXConfig[0], pbufferAttribs);
    g_glx.setCurrentContext(mGLXDisplay, mDummySurface, mDummySurface, mGLXContext);

    int result = bluegl::bind();
    ASSERT_POSTCONDITION(!result, "Unable to load OpenGL entry points.");

    return OpenGLDriverFactory::create(this, sharedGLContext);
}

void PlatformGLX::terminate() noexcept {
    g_glx.setCurrentContext(mGLXDisplay, None, None, nullptr);
    g_glx.destroyPbuffer(mGLXDisplay, mDummySurface);
    g_glx.destroyContext(mGLXDisplay, mGLXContext);
    g_x11.closeDisplay(mGLXDisplay);
    bluegl::unbind();
}

Platform::SwapChain* PlatformGLX::createSwapChain(void* nativeWindow, uint64_t& flags) noexcept {
    // Transparent swap chain is not supported
    flags &= ~backend::SWAP_CHAIN_CONFIG_TRANSPARENT;
    return (SwapChain*)nativeWindow;
}

Platform::SwapChain* PlatformGLX::createSwapChain(
        uint32_t width, uint32_t height, uint64_t& flags) noexcept {
    // Transparent swap chain is not supported
    flags &= ~backend::SWAP_CHAIN_CONFIG_TRANSPARENT;
    int pbufferAttribs[] = {
            GLX_PBUFFER_WIDTH, int(width),
            GLX_PBUFFER_HEIGHT, int(height),
            GL_NONE
    };
    GLXPbuffer sur = g_glx.createPbuffer(mGLXDisplay, mGLXConfig[0], pbufferAttribs);
    if (sur) {
        mPBuffers.push_back(sur);
    }
    return (SwapChain*)sur;
}

void PlatformGLX::destroySwapChain(Platform::SwapChain* swapChain) noexcept {
    auto it = std::find(mPBuffers.begin(), mPBuffers.end(), (GLXPbuffer)swapChain);
    if (it != mPBuffers.end()) {
        g_glx.destroyPbuffer(mGLXDisplay, (GLXPbuffer)swapChain);
        mPBuffers.erase(it);
    }
}

void PlatformGLX::makeCurrent(
        Platform::SwapChain* drawSwapChain, Platform::SwapChain* readSwapChain) noexcept {
    g_glx.setCurrentContext(mGLXDisplay,
            (GLXDrawable)drawSwapChain, (GLXDrawable)readSwapChain, mGLXContext);
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
