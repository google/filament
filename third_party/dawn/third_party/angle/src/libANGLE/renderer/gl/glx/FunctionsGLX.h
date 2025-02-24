//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FunctionsGLX.h: Defines the FunctionsGLX class to load functions and data from GLX

#ifndef LIBANGLE_RENDERER_GL_GLX_FUNCTIONSGLX_H_
#define LIBANGLE_RENDERER_GL_GLX_FUNCTIONSGLX_H_

#include <string>
#include <vector>

#include "libANGLE/renderer/gl/glx/platform_glx.h"

namespace rx
{

class FunctionsGLX
{
  public:
    FunctionsGLX();
    ~FunctionsGLX();

    // Load data from GLX, can be called multiple times
    bool initialize(Display *xDisplay, int screen, std::string *errorString);
    void terminate();

    bool hasExtension(const char *extension) const;
    int majorVersion;
    int minorVersion;

    Display *getDisplay() const;
    int getScreen() const;

    PFNGETPROCPROC getProc;

    // GLX 1.0
    glx::Context createContext(XVisualInfo *visual, glx::Context share, bool direct) const;
    void destroyContext(glx::Context context) const;
    Bool makeCurrent(glx::Drawable drawable, glx::Context context) const;
    void swapBuffers(glx::Drawable drawable) const;
    Bool queryExtension(int *errorBase, int *event) const;
    Bool queryVersion(int *major, int *minor) const;
    glx::Context getCurrentContext() const;
    glx::Drawable getCurrentDrawable() const;
    void waitX() const;
    void waitGL() const;

    // GLX 1.1
    const char *getClientString(int name) const;
    const char *queryExtensionsString() const;

    // GLX 1.3
    glx::FBConfig *getFBConfigs(int *nElements) const;
    glx::FBConfig *chooseFBConfig(const int *attribList, int *nElements) const;
    int getFBConfigAttrib(glx::FBConfig config, int attribute, int *value) const;
    XVisualInfo *getVisualFromFBConfig(glx::FBConfig config) const;
    glx::Window createWindow(glx::FBConfig config, Window window, const int *attribList) const;
    void destroyWindow(glx::Window window) const;
    glx::Pbuffer createPbuffer(glx::FBConfig config, const int *attribList) const;
    void destroyPbuffer(glx::Pbuffer pbuffer) const;
    void queryDrawable(glx::Drawable drawable, int attribute, unsigned int *value) const;
    glx::Pixmap createPixmap(glx::FBConfig config, Pixmap pixmap, const int *attribList) const;
    void destroyPixmap(Pixmap pixmap) const;

    // GLX_ARB_create_context
    glx::Context createContextAttribsARB(glx::FBConfig config,
                                         glx::Context shareContext,
                                         Bool direct,
                                         const int *attribList) const;

    // GLX_EXT_swap_control
    void swapIntervalEXT(glx::Drawable drawable, int interval) const;

    // GLX_MESA_swap_control
    int swapIntervalMESA(int interval) const;

    // GLX_SGI_swap_control
    int swapIntervalSGI(int interval) const;

    // GLX_OML_sync_control
    bool getSyncValuesOML(glx::Drawable drawable, int64_t *ust, int64_t *msc, int64_t *sbc) const;
    bool getMscRateOML(glx::Drawable drawable, int32_t *numerator, int32_t *denominator) const;

    // GLX_EXT_texture_from_pixmap
    void bindTexImageEXT(glx::Drawable drawable, int buffer, const int *attribList) const;
    void releaseTexImageEXT(glx::Drawable drawable, int buffer) const;

  private:
    // So as to isolate GLX from angle we do not include angleutils.h and cannot
    // use angle::NonCopyable so we replicated it here instead.
    FunctionsGLX(const FunctionsGLX &) = delete;
    void operator=(const FunctionsGLX &) = delete;

    struct GLXFunctionTable;

    static void *sLibHandle;
    Display *mXDisplay;
    int mXScreen;

    GLXFunctionTable *mFnPtrs;
    std::vector<std::string> mExtensions;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_GLX_FUNCTIONSGLX_H_
