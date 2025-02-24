//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WindowSurfaceCGL.h: CGL implementation of egl::Surface for windows

#ifndef LIBANGLE_RENDERER_GL_CGL_WINDOWSURFACECGL_H_
#define LIBANGLE_RENDERER_GL_CGL_WINDOWSURFACECGL_H_

#include "libANGLE/renderer/gl/SurfaceGL.h"

struct _CGLContextObject;
typedef _CGLContextObject *CGLContextObj;
@class CALayer;
struct __IOSurface;
typedef __IOSurface *IOSurfaceRef;

#ifdef ANGLE_OUTSIDE_WEBKIT
// Avoid collisions with the system's WebKit.framework.
@class ANGLESwapCGLLayer;
#else
// WebKit's build process requires that every Objective-C class name has the prefix "Web".
@class WebSwapCGLLayer;
#endif

namespace rx
{

class DisplayCGL;
class FramebufferGL;
class FunctionsGL;
class RendererGL;
class StateManagerGL;

struct SharedSwapState
{
    struct SwapTexture
    {
        GLuint texture;
        unsigned int width;
        unsigned int height;
        uint64_t swapId;
    };

    SwapTexture textures[3];

    // This code path is not going to be used by Chrome so we take the liberty
    // to use pthreads directly instead of using mutexes and condition variables
    // via the Platform API.
    pthread_mutex_t mutex;
    // The following members should be accessed only when holding the mutex
    // (or doing construction / destruction)
    SwapTexture *beingRendered;
    SwapTexture *lastRendered;
    SwapTexture *beingPresented;
};

class WindowSurfaceCGL : public SurfaceGL
{
  public:
    WindowSurfaceCGL(const egl::SurfaceState &state,
                     RendererGL *renderer,
                     EGLNativeWindowType layer,
                     CGLContextObj context);
    ~WindowSurfaceCGL() override;

    egl::Error initialize(const egl::Display *display) override;
    egl::Error makeCurrent(const gl::Context *context) override;

    egl::Error swap(const gl::Context *context) override;
    egl::Error postSubBuffer(const gl::Context *context,
                             EGLint x,
                             EGLint y,
                             EGLint width,
                             EGLint height) override;
    egl::Error querySurfacePointerANGLE(EGLint attribute, void **value) override;
    egl::Error bindTexImage(const gl::Context *context,
                            gl::Texture *texture,
                            EGLint buffer) override;
    egl::Error releaseTexImage(const gl::Context *context, EGLint buffer) override;
    void setSwapInterval(const egl::Display *display, EGLint interval) override;

    EGLint getWidth() const override;
    EGLint getHeight() const override;

    EGLint isPostSubBufferSupported() const override;
    EGLint getSwapBehavior() const override;

    egl::Error attachToFramebuffer(const gl::Context *context,
                                   gl::Framebuffer *framebuffer) override;
    egl::Error detachFromFramebuffer(const gl::Context *context,
                                     gl::Framebuffer *framebuffer) override;

  private:
#ifdef ANGLE_OUTSIDE_WEBKIT
    ANGLESwapCGLLayer *mSwapLayer;
#else
    WebSwapCGLLayer *mSwapLayer;
#endif
    SharedSwapState mSwapState;
    uint64_t mCurrentSwapId;

    CALayer *mLayer;
    CGLContextObj mContext;
    const FunctionsGL *mFunctions;
    StateManagerGL *mStateManager;

    GLuint mDSRenderbuffer;
    GLuint mFramebufferID;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_CGL_WINDOWSURFACECGL_H_
