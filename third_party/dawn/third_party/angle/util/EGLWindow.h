//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef UTIL_EGLWINDOW_H_
#define UTIL_EGLWINDOW_H_

#include <stdint.h>
#include <list>
#include <memory>
#include <string>

#include "common/Optional.h"
#include "common/PackedEnums.h"
#include "common/angleutils.h"
#include "util/EGLPlatformParameters.h"
#include "util/util_export.h"
#include "util/util_gl.h"

class OSWindow;

namespace angle
{
class Library;
struct PlatformMethods;
using GenericProc = void (*)();
}  // namespace angle

struct ANGLE_UTIL_EXPORT ConfigParameters
{
    ConfigParameters();
    ~ConfigParameters();

    void reset();

    // Surface and Context parameters.
    int redBits;
    int greenBits;
    int blueBits;
    int alphaBits;
    int depthBits;
    int stencilBits;

    Optional<bool> webGLCompatibility;
    Optional<bool> robustResourceInit;

    // EGLWindow-specific.
    EGLenum componentType;
    bool multisample;
    bool debug;
    bool noError;
    Optional<bool> extensionsEnabled;
    bool bindGeneratesResource;
    bool clientArraysEnabled;
    bool robustAccess;
    bool mutableRenderBuffer;
    EGLint samples;
    Optional<bool> contextProgramCacheEnabled;
    EGLenum resetStrategy;
    EGLenum colorSpace;
    EGLint swapInterval;
};

using GLWindowContext = struct GLWindowHandleContext_T *;

enum class GLWindowResult
{
    NoError,
    NoColorspaceSupport,
    NoMutableRenderBufferSupport,
    Error,
};

class ANGLE_UTIL_EXPORT GLWindowBase : angle::NonCopyable
{
  public:
    static void Delete(GLWindowBase **window);

    using Image        = void *;
    using ClientBuffer = void *;
    using Enum         = unsigned int;
    using Attrib       = intptr_t;
    using AttribKHR    = khronos_int32_t;
    using Boolean      = unsigned int;
    using Surface      = void *;
    using Sync         = void *;
    using Display      = void *;

    // It should also be possible to set multisample and floating point framebuffers.
    EGLint getClientMajorVersion() const { return mClientMajorVersion; }
    EGLint getClientMinorVersion() const { return mClientMinorVersion; }

    virtual bool initializeGL(OSWindow *osWindow,
                              angle::Library *glWindowingLibrary,
                              angle::GLESDriverType driverType,
                              const EGLPlatformParameters &platformParams,
                              const ConfigParameters &configParams) = 0;

    virtual GLWindowResult initializeGLWithResult(OSWindow *osWindow,
                                                  angle::Library *glWindowingLibrary,
                                                  angle::GLESDriverType driverType,
                                                  const EGLPlatformParameters &platformParams,
                                                  const ConfigParameters &configParams) = 0;

    virtual bool isGLInitialized() const                        = 0;
    virtual void swap()                                         = 0;
    virtual void destroyGL()                                    = 0;
    virtual bool makeCurrent()                                  = 0;
    virtual bool hasError() const                               = 0;
    virtual bool setSwapInterval(EGLint swapInterval)           = 0;
    virtual angle::GenericProc getProcAddress(const char *name) = 0;
    // EGLContext and HGLRC (WGL) are both "handles", which are implemented as pointers.
    // Use void* here and let the underlying implementation handle interpreting the type correctly.
    virtual GLWindowContext getCurrentContextGeneric()                  = 0;
    virtual GLWindowContext createContextGeneric(GLWindowContext share) = 0;
    virtual bool makeCurrentGeneric(GLWindowContext context)            = 0;
    virtual Image createImage(GLWindowContext context,
                              Enum target,
                              ClientBuffer buffer,
                              const Attrib *attrib_list)                = 0;
    virtual Image createImageKHR(GLWindowContext context,
                                 Enum target,
                                 ClientBuffer buffer,
                                 const AttribKHR *attrib_list)          = 0;
    virtual EGLBoolean destroyImage(Image image)                        = 0;
    virtual EGLBoolean destroyImageKHR(Image image)                     = 0;

    virtual Sync createSync(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list)        = 0;
    virtual Sync createSyncKHR(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list)        = 0;
    virtual EGLBoolean destroySync(EGLDisplay dpy, Sync sync)                                  = 0;
    virtual EGLBoolean destroySyncKHR(EGLDisplay dpy, Sync sync)                               = 0;
    virtual EGLint clientWaitSync(EGLDisplay dpy, Sync sync, EGLint flags, EGLTimeKHR timeout) = 0;
    virtual EGLint clientWaitSyncKHR(EGLDisplay dpy,
                                     Sync sync,
                                     EGLint flags,
                                     EGLTimeKHR timeout)                                       = 0;

    virtual EGLint getEGLError()                                    = 0;
    virtual Display getCurrentDisplay()                             = 0;
    virtual Surface createPbufferSurface(const EGLint *attrib_list) = 0;
    virtual EGLBoolean destroySurface(Surface surface)              = 0;

    virtual EGLBoolean bindTexImage(EGLSurface surface, EGLint buffer)    = 0;
    virtual EGLBoolean releaseTexImage(EGLSurface surface, EGLint buffer) = 0;

    virtual bool makeCurrent(EGLSurface draw, EGLSurface read, EGLContext context) = 0;

    bool isMultisample() const { return mConfigParams.multisample; }
    bool isDebugEnabled() const { return mConfigParams.debug; }

    const angle::PlatformMethods *getPlatformMethods() const { return mPlatform.platformMethods; }

    const EGLPlatformParameters &getPlatform() const { return mPlatform; }
    const ConfigParameters &getConfigParams() const { return mConfigParams; }

    virtual bool isFeatureEnabled(angle::Feature feature) { return false; }

  protected:
    GLWindowBase(EGLint glesMajorVersion, EGLint glesMinorVersion);
    virtual ~GLWindowBase();

    EGLint mClientMajorVersion;
    EGLint mClientMinorVersion;
    EGLPlatformParameters mPlatform;
    ConfigParameters mConfigParams;
};

enum class ANGLEFeatureStatus
{
    Enabled,
    Disabled,
    Unknown,
};
using ANGLEFeatureArray = angle::PackedEnumMap<angle::Feature, ANGLEFeatureStatus>;

class ANGLE_UTIL_EXPORT EGLWindow : public GLWindowBase
{
  public:
    static EGLWindow *New(EGLint glesMajorVersion, EGLint glesMinorVersion);
    static void Delete(EGLWindow **window);

    static EGLBoolean FindEGLConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *config);

    EGLConfig getConfig() const;
    EGLDisplay getDisplay() const;
    EGLSurface getSurface() const;
    EGLContext getContext() const;

    bool isContextVersion(EGLint glesMajorVersion, EGLint glesMinorVersion) const;

    // Internally initializes the Display, Surface and Context.
    bool initializeGL(OSWindow *osWindow,
                      angle::Library *glWindowingLibrary,
                      angle::GLESDriverType driverType,
                      const EGLPlatformParameters &platformParams,
                      const ConfigParameters &configParams) override;

    GLWindowResult initializeGLWithResult(OSWindow *osWindow,
                                          angle::Library *glWindowingLibrary,
                                          angle::GLESDriverType driverType,
                                          const EGLPlatformParameters &platformParams,
                                          const ConfigParameters &configParams) override;

    bool isGLInitialized() const override;
    void swap() override;
    void destroyGL() override;
    bool makeCurrent() override;
    bool hasError() const override;
    bool setSwapInterval(EGLint swapInterval) override;
    angle::GenericProc getProcAddress(const char *name) override;
    // Initializes EGL resources.
    GLWindowContext getCurrentContextGeneric() override;
    GLWindowContext createContextGeneric(GLWindowContext share) override;
    bool makeCurrentGeneric(GLWindowContext context) override;

    // Only initializes the Display.
    bool initializeDisplay(OSWindow *osWindow,
                           angle::Library *glWindowingLibrary,
                           angle::GLESDriverType driverType,
                           const EGLPlatformParameters &params);

    // Only initializes the Surface.
    GLWindowResult initializeSurface(OSWindow *osWindow,
                                     angle::Library *glWindowingLibrary,
                                     const ConfigParameters &params);

    // Create an EGL context with this window's configuration
    EGLContext createContext(EGLContext share, EGLint *extraAttributes);
    // Make the EGL context current
    bool makeCurrent(EGLContext context);

    Image createImage(GLWindowContext context,
                      Enum target,
                      ClientBuffer buffer,
                      const Attrib *attrib_list) override;
    Image createImageKHR(GLWindowContext context,
                         Enum target,
                         ClientBuffer buffer,
                         const AttribKHR *attrib_list) override;

    EGLBoolean destroyImage(Image image) override;
    EGLBoolean destroyImageKHR(Image image) override;

    Sync createSync(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list) override;
    Sync createSyncKHR(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list) override;
    EGLBoolean destroySync(EGLDisplay dpy, Sync sync) override;
    EGLBoolean destroySyncKHR(EGLDisplay dpy, Sync sync) override;
    EGLint clientWaitSync(EGLDisplay dpy, Sync sync, EGLint flags, EGLTimeKHR timeout) override;
    EGLint clientWaitSyncKHR(EGLDisplay dpy, Sync sync, EGLint flags, EGLTimeKHR timeout) override;

    EGLint getEGLError() override;
    Display getCurrentDisplay() override;
    Surface createPbufferSurface(const EGLint *attrib_list) override;
    EGLBoolean destroySurface(Surface surface) override;

    EGLBoolean bindTexImage(EGLSurface surface, EGLint buffer) override;
    EGLBoolean releaseTexImage(EGLSurface surface, EGLint buffer) override;
    bool makeCurrent(EGLSurface draw, EGLSurface read, EGLContext context) override;

    // Only initializes the Context.
    bool initializeContext();

    void destroySurface();
    void destroyContext();

    bool isDisplayInitialized() const { return mDisplay != EGL_NO_DISPLAY; }

    // Get the status of features and cache them in mFeatures.
    void queryFeatures();
    // Return whether a feature is enabled.  Features that don't exist in the backend have Unknown
    // status, and are considered disabled for the purposes of this function.
    bool isFeatureEnabled(angle::Feature feature) override;

  private:
    EGLWindow(EGLint glesMajorVersion, EGLint glesMinorVersion);
    ~EGLWindow() override;

    EGLConfig mConfig;
    EGLDisplay mDisplay;
    EGLSurface mSurface;
    EGLContext mContext;

    EGLint mEGLMajorVersion;
    EGLint mEGLMinorVersion;

    ANGLEFeatureArray mFeatures;
};

#endif  // UTIL_EGLWINDOW_H_
