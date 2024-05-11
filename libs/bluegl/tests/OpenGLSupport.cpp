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

#include "OpenGLSupport.hpp"

#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <mutex>

#ifdef __APPLE__
#include <dlfcn.h>
#include <OpenGL/OpenGL.h>

#define LIBRARY_CGL "/System/Library/Frameworks/OpenGL.framework/OpenGL"
#elif defined(__CYGWIN__) || defined(WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/glxext.h>

#define LIBRARY_GLX "libGL.so.1"
#define LIBRARY_X11 "libX11.so.6"
#endif

namespace bluegl {
namespace gl {

// This mutex protect g_library_refcount below.
static std::mutex g_library_mutex;

#ifdef __APPLE__
static uint32_t g_library_refcount = 0;

// Function pointer types for CGL functions
typedef CGLError (*CGL_CHOOSE_PIXEL_FORMAT_PROC)(const CGLPixelFormatAttribute*, CGLPixelFormatObj*, GLint*);
typedef CGLError (*CGL_DESTROY_PIXEL_FORMAT_PROC)(CGLPixelFormatObj);
typedef CGLError (*CGL_CREATE_CONTEXT_PROC)(CGLPixelFormatObj, CGLContextObj, CGLContextObj*);
typedef CGLError (*CGL_SET_CURRENT_CONTEXT_PROC)(CGLContextObj);
typedef CGLError (*CGL_DESTROY_CONTEXT_PROC)(CGLContextObj);

// Stores CGL function pointers and a handle to the system's OpenGL library
struct CGLFunctions {
    CGL_CHOOSE_PIXEL_FORMAT_PROC choosePixelFormat;
    CGL_DESTROY_PIXEL_FORMAT_PROC destroyPixelFormat;
    CGL_CREATE_CONTEXT_PROC createContext;
    CGL_SET_CURRENT_CONTEXT_PROC setCurrentContext;
    CGL_DESTROY_CONTEXT_PROC destroyContext;
    void* library;
} g_cgl;

bool loadLibraries() {
    std::lock_guard<std::mutex> lock(g_library_mutex);
    g_library_refcount++;

    if (g_library_refcount == 1) {
        g_cgl.library = dlopen(LIBRARY_CGL, RTLD_GLOBAL);
        if (!g_cgl.library) {
            std::cerr << "Could not find library " << LIBRARY_CGL << std::endl;
            return false;
        }

        g_cgl.choosePixelFormat = (CGL_CHOOSE_PIXEL_FORMAT_PROC)
                dlsym(g_cgl.library, "CGLChoosePixelFormat");
        g_cgl.destroyPixelFormat = (CGL_DESTROY_PIXEL_FORMAT_PROC)
                dlsym(g_cgl.library, "CGLDestroyPixelFormat");
        g_cgl.createContext = (CGL_CREATE_CONTEXT_PROC)
                dlsym(g_cgl.library, "CGLCreateContext");
        g_cgl.setCurrentContext = (CGL_SET_CURRENT_CONTEXT_PROC)
                dlsym(g_cgl.library, "CGLSetCurrentContext");
        g_cgl.destroyContext = (CGL_DESTROY_CONTEXT_PROC)
                dlsym(g_cgl.library, "CGLDestroyContext");
    }

    return true;
}

void unloadLibraries() {
    std::lock_guard<std::mutex> lock(g_library_mutex);
    uint32_t refcount = g_library_refcount;
    if (refcount > 0) {
        g_library_refcount--;
    }

    if (refcount == 1) {
        dlclose(g_cgl.library);
        memset(&g_cgl, 0, sizeof(g_cgl));
    }
}

OpenGLContext createOpenGLContext() {
    if (!loadLibraries()) return nullptr;

    // The attributes don't really matter here but we choose a configuration
    // that would make sense if we were to perform actual rendering
    CGLPixelFormatAttribute attributes[] = {
        kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) kCGLOGLPVersion_GL4_Core,
        kCGLPFAColorSize,     (CGLPixelFormatAttribute) 24,
        kCGLPFAAlphaSize,     (CGLPixelFormatAttribute)  8,
        kCGLPFAAccelerated,   (CGLPixelFormatAttribute)  0
    };

    GLint pixelFormatCount;
    CGLPixelFormatObj pixelFormat;
    (*g_cgl.choosePixelFormat)(attributes, &pixelFormat, &pixelFormatCount);

    OpenGLContext context;
    (*g_cgl.createContext)(pixelFormat, nullptr, (CGLContextObj*) &context);

    (*g_cgl.destroyPixelFormat)(pixelFormat);

    return context;
}

void setCurrentOpenGLContext(OpenGLContext context) {
    (*g_cgl.setCurrentContext)((CGLContextObj) context);
}

void destroyOpenGLContext(OpenGLContext context) {
    (*g_cgl.setCurrentContext)(nullptr);
    (*g_cgl.destroyContext)((CGLContextObj) context);

    unloadLibraries();
}

#elif defined(__CYGWIN__) || defined(WIN32)

#include "../include/GL/glcorearb.h"
#include "GL/wglext.h"

struct wglLocalContext {
    wglLocalContext(HWND ohwnd, HDC owhdc, HGLRC ocontext) :
            hwnd(ohwnd), whdc(owhdc), context(ocontext) {
    };
    HWND hwnd;
    HDC whdc;
    HGLRC context;
};

bool loadLibraries() {
    return true;
}

bool unloadLibraries() {
    return true;
}

OpenGLContext createOpenGLContext() {
    if (!loadLibraries()) return nullptr;

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

    int attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 1,
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_PROFILE_MASK_ARB,
            0
    };

    HWND hwnd = CreateWindowA("STATIC", "dummy", 0, 0, 0, 1, 1, NULL, NULL, NULL, NULL);
    HDC whdc = GetDC(hwnd);

    int pixelFormat = ChoosePixelFormat(whdc, &pfd);
    SetPixelFormat(whdc, pixelFormat, &pfd);

    // We need a tmp context to retrieve and call wglCreateContextAttribsARB.
    HGLRC tempContext = wglCreateContext(whdc);
    wglMakeCurrent(whdc, tempContext);

    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribs =
            (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");
    HGLRC context = wglCreateContextAttribs(whdc, nullptr, attribs);

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempContext);
    wglMakeCurrent(whdc, context);

    return new wglLocalContext(hwnd, whdc, context);
}

void setCurrentOpenGLContext(OpenGLContext context) {
    wglLocalContext* wContext = static_cast<wglLocalContext*>(context);
    HDC hdc = GetDC(wContext->hwnd);
    wglMakeCurrent(hdc, wContext->context);
}

void destroyOpenGLContext(OpenGLContext context) {
    wglLocalContext* wContext = static_cast<wglLocalContext*>(context);
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(wContext->context);
    ReleaseDC(wContext->hwnd, wContext->whdc);
    DestroyWindow(wContext->hwnd);
    delete static_cast<wglLocalContext*>(context);
    unloadLibraries();
}

#else

// Function pointer types for X11 functions
typedef Display* (*X11_OPEN_DISPLAY)(const char*);
typedef Display* (*X11_CLOSE_DISPLAY)(Display*);
typedef int      (*X11_FREE)(void*);
typedef int      (*X11_SYNC)(Display*, Bool);

// Function pointer types for GLX functions
typedef void (*GLX_DESTROY_CONTEXT)(Display*, GLXContext);

// Stores GLX function pointers and a handle to the system's GLX library
struct GLXFunctions {
    PFNGLXCHOOSEFBCONFIGPROC chooseFbConfig;
    PFNGLXCREATECONTEXTATTRIBSARBPROC createContext;
    PFNGLXCREATEPBUFFERPROC createPbuffer;
    PFNGLXDESTROYPBUFFERPROC destroyPbuffer;
    PFNGLXMAKECONTEXTCURRENTPROC setCurrentContext;
    GLX_DESTROY_CONTEXT destroyContext;
    void* library;
} g_glx;

// Stores X11 function pointers and a handle to the system's X11 library
struct X11Functions {
    X11_OPEN_DISPLAY openDisplay;
    X11_CLOSE_DISPLAY closeDisplay;
    X11_FREE free;
    X11_SYNC sync;
    void* library;
} g_x11;

/*
 * A GLXLocalContext is returned by createOpenGLContext() and is
 * expected by destroyOpenGLContext(). It contains system types
 * required to properly make a context current and perform cleanup
 * when we're done.
 */
struct GLXLocalContext {
    Display* display;
    GLXContext context;
    GLXPbuffer buffer;
};

static uint32_t g_library_refcount = 0;

bool loadLibraries() {
    std::lock_guard<std::mutex> lock(g_library_mutex);
    g_library_refcount++;

    if (g_library_refcount == 1) {
        g_glx.library = dlopen(LIBRARY_GLX, RTLD_LOCAL | RTLD_NOW);
        if (!g_glx.library) {
            std::cerr << "Could not find library " << LIBRARY_GLX << std::endl;
            return false;
        }

        PFNGLXGETPROCADDRESSPROC getProcAddress =
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

        g_x11.library = dlopen(LIBRARY_X11, RTLD_LOCAL | RTLD_NOW);
        if (!g_x11.library) {
            std::cerr << "Could not find library " << LIBRARY_X11 << std::endl;
            return false;
        }

        g_x11.openDisplay  = (X11_OPEN_DISPLAY)  dlsym(g_x11.library, "XOpenDisplay");
        g_x11.closeDisplay = (X11_CLOSE_DISPLAY) dlsym(g_x11.library, "XCloseDisplay");
        g_x11.free         = (X11_FREE)          dlsym(g_x11.library, "XFree");
        g_x11.sync         = (X11_SYNC)          dlsym(g_x11.library, "XSync");
    }
    return true;
}

void unloadLibraries() {
    std::lock_guard<std::mutex> lock(g_library_mutex);
    uint32_t refcount = g_library_refcount;
    if (refcount > 0) {
        g_library_refcount--;
    }

    if (refcount == 1) {
        dlclose(g_glx.library);
        memset(&g_glx, 0, sizeof(g_glx));

        dlclose(g_x11.library);
        memset(&g_x11, 0, sizeof(g_x11));
    }
}

OpenGLContext createOpenGLContext() {
    const char* gl_indirect = getenv("LIBGL_ALWAYS_INDIRECT");
    if (gl_indirect != nullptr) {
        std::cerr << "The environment variable LIBGL_ALWAYS_INDIRECT is set.\n"
                  << "This variable must be unset for this test to run properly."
                  << std::endl;
        return nullptr;
    }

    if (!loadLibraries()) return nullptr;

    // Open the default display, it doesn't matter which one since we
    // are not creating a window but only a pbuffer
    Display* display = (*g_x11.openDisplay)(nullptr);
    if (display == nullptr) return nullptr;

    // Any frame buffer configuration will do, we only care about the
    // context, not the frame buffer itself
    static int attribs[] = { None };
    int config_count = 0;
    GLXFBConfig* configs = (*g_glx.chooseFbConfig)(display, DefaultScreen(display),
            attribs, &config_count);
    if (configs == nullptr || config_count == 0) return nullptr;

    // We'll only need OpenGL 2.1 for testing purposes. This ensures
    // tests can be run in a virtual machine
    int context_attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 2,
        GLX_CONTEXT_MINOR_VERSION_ARB, 1,
        None
    };
    GLXContext context = (*g_glx.createContext)(display, configs[0],
            nullptr, True, context_attribs);

    // A 1x1 pbuffer is required to be able to do a make current later on
    int pbuffer_attribs[] = {
        GLX_PBUFFER_WIDTH,  1,
        GLX_PBUFFER_HEIGHT, 1,
        None
    };
    GLXPbuffer pbuffer = (*g_glx.createPbuffer)(display, configs[0], pbuffer_attribs);

    (*g_x11.free)(configs);
    (*g_x11.sync)(display, False);

    GLXLocalContext* local_context = new GLXLocalContext();
    local_context->display = display;
    local_context->context = context;
    local_context->buffer = pbuffer;

    return local_context;
}

void setCurrentOpenGLContext(OpenGLContext context) {
    GLXLocalContext* local_context = (GLXLocalContext*) context;
    (*g_glx.setCurrentContext)(local_context->display,
            local_context->buffer, local_context->buffer, local_context->context);
}

void destroyOpenGLContext(OpenGLContext context) {
    GLXLocalContext* local_context = (GLXLocalContext*) context;
    (*g_glx.setCurrentContext)(local_context->display, None, None, nullptr);

    (*g_glx.destroyPbuffer)(local_context->display, local_context->buffer);
    (*g_glx.destroyContext)(local_context->display, local_context->context);

    (*g_x11.closeDisplay)(local_context->display);

    delete local_context;
    unloadLibraries();
}

#endif

}; // namespace gl
}; // namespace bluegl
