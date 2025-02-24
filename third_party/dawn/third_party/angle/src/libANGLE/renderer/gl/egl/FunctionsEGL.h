//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FunctionsEGL.h: Defines the FunctionsEGL class to load functions and data from EGL

#ifndef LIBANGLE_RENDERER_GL_CROS_FUNCTIONSEGL_H_
#define LIBANGLE_RENDERER_GL_CROS_FUNCTIONSEGL_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <string>
#include <vector>

#include "libANGLE/Error.h"

namespace rx
{

class FunctionsGL;

class FunctionsEGL
{
  public:
    FunctionsEGL();
    virtual ~FunctionsEGL();

    int majorVersion;
    int minorVersion;
    std::string vendorString;
    std::string versionString;

    egl::Error initialize(EGLAttrib platformType, EGLNativeDisplayType nativeDisplay);
    egl::Error terminate();

    virtual void *getProcAddress(const char *name) const = 0;

    FunctionsGL *makeFunctionsGL() const;
    bool hasExtension(const char *extension) const;
    bool hasDmaBufImportModifierFunctions() const;
    EGLDisplay getDisplay() const;
    EGLint getError() const;

    EGLBoolean chooseConfig(EGLint const *attrib_list,
                            EGLConfig *configs,
                            EGLint config_size,
                            EGLint *num_config) const;
    EGLBoolean getConfigs(EGLConfig *configs, EGLint config_size, EGLint *num_config) const;
    EGLBoolean getConfigAttrib(EGLConfig config, EGLint attribute, EGLint *value) const;
    EGLSurface getCurrentSurface(EGLint readdraw) const;
    EGLContext createContext(EGLConfig config,
                             EGLContext share_context,
                             EGLint const *attrib_list) const;
    EGLSurface createPbufferSurface(EGLConfig config, const EGLint *attrib_list) const;
    EGLSurface createWindowSurface(EGLConfig config,
                                   EGLNativeWindowType win,
                                   const EGLint *attrib_list) const;
    EGLBoolean destroyContext(EGLContext context) const;
    EGLBoolean destroySurface(EGLSurface surface) const;
    EGLBoolean makeCurrent(EGLSurface surface, EGLContext context) const;
    const char *queryString(EGLint name) const;
    EGLBoolean querySurface(EGLSurface surface, EGLint attribute, EGLint *value) const;
    EGLBoolean swapBuffers(EGLSurface surface) const;

    EGLBoolean bindTexImage(EGLSurface surface, EGLint buffer) const;
    EGLBoolean releaseTexImage(EGLSurface surface, EGLint buffer) const;
    EGLBoolean surfaceAttrib(EGLSurface surface, EGLint attribute, EGLint value) const;
    EGLBoolean swapInterval(EGLint interval) const;

    EGLContext getCurrentContext() const;

    EGLImageKHR createImageKHR(EGLContext context,
                               EGLenum target,
                               EGLClientBuffer buffer,
                               const EGLint *attrib_list) const;
    EGLBoolean destroyImageKHR(EGLImageKHR image) const;

    EGLSyncKHR createSyncKHR(EGLenum type, const EGLint *attrib_list) const;
    EGLBoolean destroySyncKHR(EGLSyncKHR sync) const;
    EGLint clientWaitSyncKHR(EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout) const;
    EGLBoolean getSyncAttribKHR(EGLSyncKHR sync, EGLint attribute, EGLint *value) const;

    EGLint waitSyncKHR(EGLSyncKHR sync, EGLint flags) const;

    EGLBoolean swapBuffersWithDamageKHR(EGLSurface surface,
                                        const EGLint *rects,
                                        EGLint n_rects) const;

    EGLBoolean presentationTimeANDROID(EGLSurface surface, EGLnsecsANDROID time) const;

    void setBlobCacheFuncsANDROID(EGLSetBlobFuncANDROID set, EGLGetBlobFuncANDROID get) const;

    EGLBoolean getCompositorTimingSupportedANDROID(EGLSurface surface, EGLint name) const;
    EGLBoolean getCompositorTimingANDROID(EGLSurface surface,
                                          EGLint numTimestamps,
                                          const EGLint *names,
                                          EGLnsecsANDROID *values) const;
    EGLBoolean getNextFrameIdANDROID(EGLSurface surface, EGLuint64KHR *frameId) const;
    EGLBoolean getFrameTimestampSupportedANDROID(EGLSurface surface, EGLint timestamp) const;
    EGLBoolean getFrameTimestampsANDROID(EGLSurface surface,
                                         EGLuint64KHR frameId,
                                         EGLint numTimestamps,
                                         const EGLint *timestamps,
                                         EGLnsecsANDROID *values) const;

    EGLint dupNativeFenceFDANDROID(EGLSync sync) const;

    EGLint queryDmaBufFormatsEXT(EGLint maxFormats, EGLint *formats, EGLint *numFormats) const;

    EGLint queryDmaBufModifiersEXT(EGLint format,
                                   EGLint maxModifiers,
                                   EGLuint64KHR *modifiers,
                                   EGLBoolean *externalOnly,
                                   EGLint *numModifiers) const;

    EGLBoolean queryDeviceAttribEXT(EGLDisplay dpy, EGLint attribute, EGLAttrib *value) const;
    const char *queryDeviceStringEXT(EGLDeviceEXT device, EGLint name) const;
    EGLBoolean queryDisplayAttribEXT(EGLint attribute, EGLAttrib *value) const;

  private:
    // So as to isolate from angle we do not include angleutils.h and cannot
    // use angle::NonCopyable so we replicated it here instead.
    FunctionsEGL(const FunctionsEGL &)   = delete;
    void operator=(const FunctionsEGL &) = delete;

    // Helper mechanism for creating a display for the desired platform type.
    EGLDisplay getPlatformDisplay(EGLAttrib platformType, EGLNativeDisplayType nativeDisplay);

    // Helper method to populate EGL extensions. Returns true if extensions are found.
    bool queryExtensions();

    // Helper method to query egl for devices.
    std::vector<EGLDeviceEXT> queryDevices(int *major, int *minor);

    // Fallback mechanism for creating a display from a native device object.
    EGLDisplay getNativeDisplay(int *major, int *minor);

#if defined(ANGLE_HAS_LIBDRM)
    // Helper method to select preferred EGL device from queried devices.
    EGLDeviceEXT getPreferredEGLDevice(const std::vector<EGLDeviceEXT> &devices);

    // Helper method to get preferred display.
    EGLDisplay getPreferredDisplay(int *major, int *minor);
#endif  // defined(ANGLE_HAS_LIBDRM)

    struct EGLDispatchTable;
    EGLDispatchTable *mFnPtrs;
    EGLDisplay mEGLDisplay;
    std::vector<std::string> mExtensions;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_CROS_FUNCTIONSEGL_H_
