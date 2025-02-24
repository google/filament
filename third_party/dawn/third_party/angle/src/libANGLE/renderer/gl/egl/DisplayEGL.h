//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayEGL.h: Common across EGL parts of platform specific egl::Display implementations

#ifndef LIBANGLE_RENDERER_GL_EGL_DISPLAYEGL_H_
#define LIBANGLE_RENDERER_GL_EGL_DISPLAYEGL_H_

#include <map>
#include <string>
#include <vector>

#include "libANGLE/renderer/gl/DisplayGL.h"
#include "libANGLE/renderer/gl/egl/egl_utils.h"

namespace rx
{

class FunctionsEGL;
class FunctionsEGLDL;
class RendererEGL;

class DisplayEGL : public DisplayGL
{
  public:
    DisplayEGL(const egl::DisplayState &state);
    ~DisplayEGL() override;

    ImageImpl *createImage(const egl::ImageState &state,
                           const gl::Context *context,
                           EGLenum target,
                           const egl::AttributeMap &attribs) override;

    EGLSyncImpl *createSync() override;

    void setBlobCacheFuncs(EGLSetBlobFuncANDROID set, EGLGetBlobFuncANDROID get) override;

    virtual void destroyNativeContext(EGLContext context);

    egl::Error initialize(egl::Display *display) override;
    void terminate() override;

    SurfaceImpl *createWindowSurface(const egl::SurfaceState &state,
                                     EGLNativeWindowType window,
                                     const egl::AttributeMap &attribs) override;
    SurfaceImpl *createPbufferSurface(const egl::SurfaceState &state,
                                      const egl::AttributeMap &attribs) override;
    SurfaceImpl *createPbufferFromClientBuffer(const egl::SurfaceState &state,
                                               EGLenum buftype,
                                               EGLClientBuffer clientBuffer,
                                               const egl::AttributeMap &attribs) override;
    SurfaceImpl *createPixmapSurface(const egl::SurfaceState &state,
                                     NativePixmapType nativePixmap,
                                     const egl::AttributeMap &attribs) override;

    ContextImpl *createContext(const gl::State &state,
                               gl::ErrorSet *errorSet,
                               const egl::Config *configuration,
                               const gl::Context *shareContext,
                               const egl::AttributeMap &attribs) override;

    egl::ConfigSet generateConfigs() override;

    bool testDeviceLost() override;
    egl::Error restoreLostDevice(const egl::Display *display) override;

    bool isValidNativeWindow(EGLNativeWindowType window) const override;
    egl::Error validateClientBuffer(const egl::Config *configuration,
                                    EGLenum buftype,
                                    EGLClientBuffer clientBuffer,
                                    const egl::AttributeMap &attribs) const override;

    egl::Error waitClient(const gl::Context *context) override;
    egl::Error waitNative(const gl::Context *context, EGLint engine) override;

    egl::Error makeCurrent(egl::Display *display,
                           egl::Surface *drawSurface,
                           egl::Surface *readSurface,
                           gl::Context *context) override;

    gl::Version getMaxSupportedESVersion() const override;

    void initializeFrontendFeatures(angle::FrontendFeatures *features) const override;

    void populateFeatureList(angle::FeatureList *features) override;

    RendererGL *getRenderer() const override;

    egl::Error validateImageClientBuffer(const gl::Context *context,
                                         EGLenum target,
                                         EGLClientBuffer clientBuffer,
                                         const egl::AttributeMap &attribs) const override;

    ExternalImageSiblingImpl *createExternalImageSibling(const gl::Context *context,
                                                         EGLenum target,
                                                         EGLClientBuffer buffer,
                                                         const egl::AttributeMap &attribs) override;

    const FunctionsEGL *getFunctionsEGL() const;

    DeviceImpl *createDevice() override;

    bool supportsDmaBufFormat(EGLint format) const override;
    egl::Error queryDmaBufFormats(EGLint maxFormats, EGLint *formats, EGLint *numFormats) override;
    egl::Error queryDmaBufModifiers(EGLint format,
                                    EGLint maxModifiers,
                                    EGLuint64KHR *modifiers,
                                    EGLBoolean *externalOnly,
                                    EGLint *numModifiers) override;

  protected:
    virtual EGLint fixSurfaceType(EGLint surfaceType) const;

  private:
    const char *getEGLPath() const;

    egl::Error initializeContext(EGLContext shareContext,
                                 const egl::AttributeMap &eglAttributes,
                                 EGLContext *outContext) const;

    void generateExtensions(egl::DisplayExtensions *outExtensions) const override;

    egl::Error createRenderer(EGLContext shareContext,
                              bool makeNewContextCurrent,
                              bool isExternalContext,
                              std::shared_ptr<RendererEGL> *outRenderer);

    egl::Error makeCurrentSurfaceless(gl::Context *context) override;

    template <typename T>
    void getConfigAttrib(EGLConfig config, EGLint attribute, T *value) const;

    template <typename T, typename U>
    void getConfigAttribIfExtension(EGLConfig config,
                                    EGLint attribute,
                                    T *value,
                                    const char *extension,
                                    const U &defaultValue) const;

    egl::Error findConfig(egl::Display *display,
                          bool forMockPbuffer,
                          EGLConfig *outConfig,
                          std::vector<EGLint> *outConfigAttribs);

    std::shared_ptr<RendererEGL> mRenderer;
    std::map<EGLAttrib, std::weak_ptr<RendererEGL>> mVirtualizationGroups;

    FunctionsEGLDL *mEGL = nullptr;
    EGLConfig mConfig    = EGL_NO_CONFIG_KHR;
    egl::AttributeMap mDisplayAttributes;
    std::vector<EGLint> mConfigAttribList;

    struct CurrentNativeContext
    {
        EGLSurface surface = EGL_NO_SURFACE;
        EGLContext context = EGL_NO_CONTEXT;
        // True if the current context is an external context. and both surface and context will be
        // unset when an external context is current.
        bool isExternalContext = false;
    };
    angle::HashMap<uint64_t, CurrentNativeContext> mCurrentNativeContexts;

    void generateCaps(egl::Caps *outCaps) const override;

    std::map<EGLint, EGLint> mConfigIds;

    bool mHasEXTCreateContextRobustness   = false;
    bool mHasNVRobustnessVideoMemoryPurge = false;

    bool mSupportsSurfaceless      = false;
    bool mSupportsNoConfigContexts = false;

    EGLSurface mMockPbuffer = EGL_NO_SURFACE;

    // Supported DRM formats
    bool mSupportsDmaBufImportModifiers = false;
    bool mNoOpDmaBufImportModifiers     = false;
    std::vector<EGLint> mDrmFormats;
    bool mDrmFormatsInitialized = false;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_EGL_DISPLAYEGL_H_
