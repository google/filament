//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Renderer9.h: Defines a back-end specific class for the D3D9 renderer.

#ifndef LIBANGLE_RENDERER_D3D_D3D9_RENDERER9_H_
#define LIBANGLE_RENDERER_D3D_D3D9_RENDERER9_H_

#include "common/angleutils.h"
#include "common/mathutil.h"
#include "libANGLE/renderer/d3d/HLSLCompiler.h"
#include "libANGLE/renderer/d3d/RenderTargetD3D.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/d3d/d3d9/DebugAnnotator9.h"
#include "libANGLE/renderer/d3d/d3d9/ShaderCache.h"
#include "libANGLE/renderer/d3d/d3d9/StateManager9.h"
#include "libANGLE/renderer/d3d/d3d9/VertexDeclarationCache.h"
#include "libANGLE/renderer/driver_utils.h"

namespace gl
{
class FramebufferAttachment;
}

namespace egl
{
class AttributeMap;
}

namespace rx
{
class Blit9;
class Context9;
class IndexDataManager;
class ProgramD3D;
class ProgramExecutableD3D;
class RenderTarget9;
class StreamingIndexBufferInterface;
class StaticIndexBufferInterface;
class VertexDataManager;
struct ClearParameters;
struct D3DUniform;
struct TranslatedAttribute;

class Renderer9 : public RendererD3D
{
  public:
    explicit Renderer9(egl::Display *display);
    ~Renderer9() override;

    egl::Error initialize() override;
    bool resetDevice() override;

    egl::ConfigSet generateConfigs() override;
    void generateDisplayExtensions(egl::DisplayExtensions *outExtensions) const override;

    void startScene();
    void endScene();

    angle::Result flush(const gl::Context *context);
    angle::Result finish(const gl::Context *context);

    bool isValidNativeWindow(EGLNativeWindowType window) const override;
    NativeWindowD3D *createNativeWindow(EGLNativeWindowType window,
                                        const egl::Config *config,
                                        const egl::AttributeMap &attribs) const override;

    SwapChainD3D *createSwapChain(NativeWindowD3D *nativeWindow,
                                  HANDLE shareHandle,
                                  IUnknown *d3dTexture,
                                  GLenum backBufferFormat,
                                  GLenum depthBufferFormat,
                                  EGLint orientation,
                                  EGLint samples) override;
    egl::Error getD3DTextureInfo(const egl::Config *configuration,
                                 IUnknown *d3dTexture,
                                 const egl::AttributeMap &attribs,
                                 EGLint *width,
                                 EGLint *height,
                                 GLsizei *samples,
                                 gl::Format *glFormat,
                                 const angle::Format **angleFormat,
                                 UINT *arraySlice) const override;
    egl::Error validateShareHandle(const egl::Config *config,
                                   HANDLE shareHandle,
                                   const egl::AttributeMap &attribs) const override;

    ContextImpl *createContext(const gl::State &state, gl::ErrorSet *errorSet) override;

    angle::Result allocateEventQuery(const gl::Context *context, IDirect3DQuery9 **outQuery);
    void freeEventQuery(IDirect3DQuery9 *query);

    // resource creation
    angle::Result createVertexShader(d3d::Context *context,
                                     const DWORD *function,
                                     size_t length,
                                     IDirect3DVertexShader9 **outShader);
    angle::Result createPixelShader(d3d::Context *context,
                                    const DWORD *function,
                                    size_t length,
                                    IDirect3DPixelShader9 **outShader);
    HRESULT createVertexBuffer(UINT Length, DWORD Usage, IDirect3DVertexBuffer9 **ppVertexBuffer);
    HRESULT createIndexBuffer(UINT Length,
                              DWORD Usage,
                              D3DFORMAT Format,
                              IDirect3DIndexBuffer9 **ppIndexBuffer);
    angle::Result setSamplerState(const gl::Context *context,
                                  gl::ShaderType type,
                                  int index,
                                  gl::Texture *texture,
                                  const gl::SamplerState &sampler);
    angle::Result setTexture(const gl::Context *context,
                             gl::ShaderType type,
                             int index,
                             gl::Texture *texture);

    angle::Result updateState(const gl::Context *context, gl::PrimitiveMode drawMode);

    void setScissorRectangle(const gl::Rectangle &scissor, bool enabled);
    void setViewport(const gl::Rectangle &viewport,
                     float zNear,
                     float zFar,
                     gl::PrimitiveMode drawMode,
                     GLenum frontFace,
                     bool ignoreViewport);

    angle::Result applyRenderTarget(const gl::Context *context,
                                    const RenderTarget9 *colorRenderTarget,
                                    const RenderTarget9 *depthStencilRenderTarget);
    void applyUniforms(ProgramExecutableD3D *executableD3D);
    bool applyPrimitiveType(gl::PrimitiveMode primitiveType,
                            GLsizei elementCount,
                            bool usesPointSize);
    angle::Result applyVertexBuffer(const gl::Context *context,
                                    gl::PrimitiveMode mode,
                                    GLint first,
                                    GLsizei count,
                                    GLsizei instances,
                                    TranslatedIndexData *indexInfo);
    angle::Result applyIndexBuffer(const gl::Context *context,
                                   const void *indices,
                                   GLsizei count,
                                   gl::PrimitiveMode mode,
                                   gl::DrawElementsType type,
                                   TranslatedIndexData *indexInfo);

    void clear(const ClearParameters &clearParams,
               const RenderTarget9 *colorRenderTarget,
               const RenderTarget9 *depthStencilRenderTarget);

    void markAllStateDirty();

    // lost device
    bool testDeviceLost() override;
    bool testDeviceResettable() override;

    VendorID getVendorId() const;
    DeviceIdentifier getAdapterIdentifier() const override;

    IDirect3DDevice9 *getDevice() { return mDevice; }
    void *getD3DDevice() override;

    unsigned int getReservedVertexUniformVectors() const;
    unsigned int getReservedFragmentUniformVectors() const;

    bool getShareHandleSupport() const;

    int getMajorShaderModel() const override;
    int getMinorShaderModel() const override;
    std::string getShaderModelSuffix() const override;

    DWORD getCapsDeclTypes() const;

    // Pixel operations
    angle::Result copyImage2D(const gl::Context *context,
                              const gl::Framebuffer *framebuffer,
                              const gl::Rectangle &sourceRect,
                              GLenum destFormat,
                              const gl::Offset &destOffset,
                              TextureStorage *storage,
                              GLint level) override;
    angle::Result copyImageCube(const gl::Context *context,
                                const gl::Framebuffer *framebuffer,
                                const gl::Rectangle &sourceRect,
                                GLenum destFormat,
                                const gl::Offset &destOffset,
                                TextureStorage *storage,
                                gl::TextureTarget target,
                                GLint level) override;
    angle::Result copyImage3D(const gl::Context *context,
                              const gl::Framebuffer *framebuffer,
                              const gl::Rectangle &sourceRect,
                              GLenum destFormat,
                              const gl::Offset &destOffset,
                              TextureStorage *storage,
                              GLint level) override;
    angle::Result copyImage2DArray(const gl::Context *context,
                                   const gl::Framebuffer *framebuffer,
                                   const gl::Rectangle &sourceRect,
                                   GLenum destFormat,
                                   const gl::Offset &destOffset,
                                   TextureStorage *storage,
                                   GLint level) override;

    angle::Result copyTexture(const gl::Context *context,
                              const gl::Texture *source,
                              GLint sourceLevel,
                              gl::TextureTarget srcTarget,
                              const gl::Box &sourceBox,
                              GLenum destFormat,
                              GLenum destType,
                              const gl::Offset &destOffset,
                              TextureStorage *storage,
                              gl::TextureTarget destTarget,
                              GLint destLevel,
                              bool unpackFlipY,
                              bool unpackPremultiplyAlpha,
                              bool unpackUnmultiplyAlpha) override;
    angle::Result copyCompressedTexture(const gl::Context *context,
                                        const gl::Texture *source,
                                        GLint sourceLevel,
                                        TextureStorage *storage,
                                        GLint destLevel) override;

    // RenderTarget creation
    angle::Result createRenderTarget(const gl::Context *context,
                                     int width,
                                     int height,
                                     GLenum format,
                                     GLsizei samples,
                                     RenderTargetD3D **outRT) override;
    angle::Result createRenderTargetCopy(const gl::Context *context,
                                         RenderTargetD3D *source,
                                         RenderTargetD3D **outRT) override;

    // Shader operations
    angle::Result loadExecutable(d3d::Context *context,
                                 const uint8_t *function,
                                 size_t length,
                                 gl::ShaderType type,
                                 const std::vector<D3DVarying> &streamOutVaryings,
                                 bool separatedOutputBuffers,
                                 ShaderExecutableD3D **outExecutable) override;
    angle::Result compileToExecutable(d3d::Context *context,
                                      gl::InfoLog &infoLog,
                                      const std::string &shaderHLSL,
                                      gl::ShaderType type,
                                      const std::vector<D3DVarying> &streamOutVaryings,
                                      bool separatedOutputBuffers,
                                      const CompilerWorkaroundsD3D &workarounds,
                                      ShaderExecutableD3D **outExectuable) override;
    angle::Result ensureHLSLCompilerInitialized(d3d::Context *context) override;

    UniformStorageD3D *createUniformStorage(size_t storageSize) override;

    // Image operations
    ImageD3D *createImage() override;
    ExternalImageSiblingImpl *createExternalImageSibling(const gl::Context *context,
                                                         EGLenum target,
                                                         EGLClientBuffer buffer,
                                                         const egl::AttributeMap &attribs) override;
    angle::Result generateMipmap(const gl::Context *context,
                                 ImageD3D *dest,
                                 ImageD3D *source) override;
    angle::Result generateMipmapUsingD3D(const gl::Context *context,
                                         TextureStorage *storage,
                                         const gl::TextureState &textureState) override;
    angle::Result copyImage(const gl::Context *context,
                            ImageD3D *dest,
                            ImageD3D *source,
                            const gl::Box &sourceBox,
                            const gl::Offset &destOffset,
                            bool unpackFlipY,
                            bool unpackPremultiplyAlpha,
                            bool unpackUnmultiplyAlpha) override;
    TextureStorage *createTextureStorage2D(SwapChainD3D *swapChain,
                                           const std::string &label) override;
    TextureStorage *createTextureStorageEGLImage(EGLImageD3D *eglImage,
                                                 RenderTargetD3D *renderTargetD3D,
                                                 const std::string &label) override;

    TextureStorage *createTextureStorageBuffer(const gl::OffsetBindingPointer<gl::Buffer> &buffer,
                                               GLenum internalFormat,
                                               const std::string &label) override;

    TextureStorage *createTextureStorageExternal(egl::Stream *stream,
                                                 const egl::Stream::GLTextureDescription &desc,
                                                 const std::string &label) override;

    TextureStorage *createTextureStorage2D(GLenum internalformat,
                                           BindFlags bindFlags,
                                           GLsizei width,
                                           GLsizei height,
                                           int levels,
                                           const std::string &label,
                                           bool hintLevelZeroOnly) override;
    TextureStorage *createTextureStorageCube(GLenum internalformat,
                                             BindFlags bindFlags,
                                             int size,
                                             int levels,
                                             bool hintLevelZeroOnly,
                                             const std::string &label) override;
    TextureStorage *createTextureStorage3D(GLenum internalformat,
                                           BindFlags bindFlags,
                                           GLsizei width,
                                           GLsizei height,
                                           GLsizei depth,
                                           int levels,
                                           const std::string &label) override;
    TextureStorage *createTextureStorage2DArray(GLenum internalformat,
                                                BindFlags bindFlags,
                                                GLsizei width,
                                                GLsizei height,
                                                GLsizei depth,
                                                int levels,
                                                const std::string &label) override;

    TextureStorage *createTextureStorage2DMultisample(GLenum internalformat,
                                                      GLsizei width,
                                                      GLsizei height,
                                                      int levels,
                                                      int samples,
                                                      bool fixedSampleLocations,
                                                      const std::string &label) override;
    TextureStorage *createTextureStorage2DMultisampleArray(GLenum internalformat,
                                                           GLsizei width,
                                                           GLsizei height,
                                                           GLsizei depth,
                                                           int levels,
                                                           int samples,
                                                           bool fixedSampleLocations,
                                                           const std::string &label) override;

    // Buffer creation
    VertexBuffer *createVertexBuffer() override;
    IndexBuffer *createIndexBuffer() override;

    // Stream Creation
    StreamProducerImpl *createStreamProducerD3DTexture(egl::Stream::ConsumerType consumerType,
                                                       const egl::AttributeMap &attribs) override;

    // Buffer-to-texture and Texture-to-buffer copies
    bool supportsFastCopyBufferToTexture(GLenum internalFormat) const override;
    angle::Result fastCopyBufferToTexture(const gl::Context *context,
                                          const gl::PixelUnpackState &unpack,
                                          gl::Buffer *unpackBuffer,
                                          unsigned int offset,
                                          RenderTargetD3D *destRenderTarget,
                                          GLenum destinationFormat,
                                          GLenum sourcePixelsType,
                                          const gl::Box &destArea) override;

    // D3D9-renderer specific methods
    angle::Result boxFilter(Context9 *context9, IDirect3DSurface9 *source, IDirect3DSurface9 *dest);

    D3DPOOL getTexturePool(DWORD usage) const;

    bool getLUID(LUID *adapterLuid) const override;
    VertexConversionType getVertexConversionType(angle::FormatID vertexFormatID) const override;
    GLenum getVertexComponentType(angle::FormatID vertexFormatID) const override;

    // Warning: you should ensure binding really matches attrib.bindingIndex before using this
    // function.
    angle::Result getVertexSpaceRequired(const gl::Context *context,
                                         const gl::VertexAttribute &attrib,
                                         const gl::VertexBinding &binding,
                                         size_t count,
                                         GLsizei instances,
                                         GLuint baseInstance,
                                         unsigned int *bytesRequiredOut) const override;

    angle::Result copyToRenderTarget(const gl::Context *context,
                                     IDirect3DSurface9 *dest,
                                     IDirect3DSurface9 *source,
                                     bool fromManaged);

    RendererClass getRendererClass() const override;

    D3DDEVTYPE getD3D9DeviceType() const { return mDeviceType; }

    DeviceImpl *createEGLDevice() override;

    StateManager9 *getStateManager() { return &mStateManager; }

    angle::Result genericDrawArrays(const gl::Context *context,
                                    gl::PrimitiveMode mode,
                                    GLint first,
                                    GLsizei count,
                                    GLsizei instances);

    angle::Result genericDrawElements(const gl::Context *context,
                                      gl::PrimitiveMode mode,
                                      GLsizei count,
                                      gl::DrawElementsType type,
                                      const void *indices,
                                      GLsizei instances);

    // Necessary hack for default framebuffers in D3D.
    FramebufferImpl *createDefaultFramebuffer(const gl::FramebufferState &state) override;

    DebugAnnotator9 *getAnnotator() { return &mAnnotator; }

    gl::Version getMaxSupportedESVersion() const override;
    gl::Version getMaxConformantESVersion() const override;

    angle::Result clearRenderTarget(const gl::Context *context,
                                    RenderTargetD3D *renderTarget,
                                    const gl::ColorF &clearColorValue,
                                    const float clearDepthValue,
                                    const unsigned int clearStencilValue) override;

    bool canSelectViewInVertexShader() const override;

    angle::Result getIncompleteTexture(const gl::Context *context,
                                       gl::TextureType type,
                                       gl::Texture **textureOut) override;

    angle::Result ensureVertexDataManagerInitialized(const gl::Context *context);

    void setGlobalDebugAnnotator() override;

    std::string getRendererDescription() const override;
    std::string getVendorString() const override;
    std::string getVersionString(bool includeFullVersion) const override;

  private:
    angle::Result drawArraysImpl(const gl::Context *context,
                                 gl::PrimitiveMode mode,
                                 GLint startVertex,
                                 GLsizei count,
                                 GLsizei instances);
    angle::Result drawElementsImpl(const gl::Context *context,
                                   gl::PrimitiveMode mode,
                                   GLsizei count,
                                   gl::DrawElementsType type,
                                   const void *indices,
                                   GLsizei instances);

    angle::Result applyShaders(const gl::Context *context, gl::PrimitiveMode drawMode);

    angle::Result applyTextures(const gl::Context *context);
    angle::Result applyTextures(const gl::Context *context, gl::ShaderType shaderType);

    void generateCaps(gl::Caps *outCaps,
                      gl::TextureCapsMap *outTextureCaps,
                      gl::Extensions *outExtensions,
                      gl::Limitations *outLimitations,
                      ShPixelLocalStorageOptions *outPLSOptions) const override;

    void initializeFeatures(angle::FeaturesD3D *features) const override;

    void initializeFrontendFeatures(angle::FrontendFeatures *features) const override;

    angle::Result setBlendDepthRasterStates(const gl::Context *context, gl::PrimitiveMode drawMode);

    void release();

    void applyUniformnfv(const D3DUniform *targetUniform, const GLfloat *v);
    void applyUniformniv(const D3DUniform *targetUniform, const GLint *v);
    void applyUniformnbv(const D3DUniform *targetUniform, const GLint *v);

    angle::Result drawLineLoop(const gl::Context *context,
                               GLsizei count,
                               gl::DrawElementsType type,
                               const void *indices,
                               int minIndex,
                               gl::Buffer *elementArrayBuffer);
    angle::Result drawIndexedPoints(const gl::Context *context,
                                    GLsizei count,
                                    gl::DrawElementsType type,
                                    const void *indices,
                                    int minIndex,
                                    gl::Buffer *elementArrayBuffer);

    angle::Result getCountingIB(const gl::Context *context,
                                size_t count,
                                StaticIndexBufferInterface **outIB);

    angle::Result getNullColorRenderTarget(const gl::Context *context,
                                           const RenderTarget9 *depthRenderTarget,
                                           const RenderTarget9 **outColorRenderTarget);

    D3DPOOL getBufferPool(DWORD usage) const;

    HMODULE mD3d9Module;

    egl::Error initializeDevice();
    D3DPRESENT_PARAMETERS getDefaultPresentParameters();
    void releaseDeviceResources();

    HRESULT getDeviceStatusCode();
    bool isRemovedDeviceResettable() const;
    bool resetRemovedDevice();

    UINT mAdapter;
    D3DDEVTYPE mDeviceType;
    IDirect3D9 *mD3d9;      // Always valid after successful initialization.
    IDirect3D9Ex *mD3d9Ex;  // Might be null if D3D9Ex is not supported.
    IDirect3DDevice9 *mDevice;
    IDirect3DDevice9Ex *mDeviceEx;  // Might be null if D3D9Ex is not supported.

    HLSLCompiler mCompiler;

    Blit9 *mBlit;

    HWND mDeviceWindow;

    D3DCAPS9 mDeviceCaps;
    D3DADAPTER_IDENTIFIER9 mAdapterIdentifier;

    D3DPRIMITIVETYPE mPrimitiveType;
    int mPrimitiveCount;
    GLsizei mRepeatDraw;

    bool mSceneStarted;

    bool mVertexTextureSupport;

    // current render target states
    unsigned int mAppliedRenderTargetSerial;
    unsigned int mAppliedDepthStencilSerial;
    bool mDepthStencilInitialized;
    bool mRenderTargetDescInitialized;

    IDirect3DStateBlock9 *mMaskedClearSavedState;

    StateManager9 mStateManager;

    // Currently applied sampler states
    struct CurSamplerState
    {
        CurSamplerState();

        bool forceSet;
        size_t baseLevel;
        gl::SamplerState samplerState;
    };
    std::vector<CurSamplerState> mCurVertexSamplerStates;
    std::vector<CurSamplerState> mCurPixelSamplerStates;

    // Currently applied textures
    std::vector<uintptr_t> mCurVertexTextures;
    std::vector<uintptr_t> mCurPixelTextures;

    unsigned int mAppliedIBSerial;
    IDirect3DVertexShader9 *mAppliedVertexShader;
    IDirect3DPixelShader9 *mAppliedPixelShader;
    unsigned int mAppliedProgramSerial;

    // A pool of event queries that are currently unused.
    std::vector<IDirect3DQuery9 *> mEventQueryPool;
    VertexShaderCache mVertexShaderCache;
    PixelShaderCache mPixelShaderCache;

    VertexDataManager *mVertexDataManager;
    VertexDeclarationCache mVertexDeclarationCache;

    IndexDataManager *mIndexDataManager;
    StreamingIndexBufferInterface *mLineLoopIB;
    StaticIndexBufferInterface *mCountingIB;

    enum
    {
        NUM_NULL_COLORBUFFER_CACHE_ENTRIES = 12
    };
    struct NullRenderTargetCacheEntry
    {
        UINT lruCount;
        int width;
        int height;
        RenderTarget9 *renderTarget;
    };

    std::array<NullRenderTargetCacheEntry, NUM_NULL_COLORBUFFER_CACHE_ENTRIES>
        mNullRenderTargetCache;
    UINT mMaxNullColorbufferLRU;

    std::vector<TranslatedAttribute> mTranslatedAttribCache;

    DebugAnnotator9 mAnnotator;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D9_RENDERER9_H_
