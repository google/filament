//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Renderer11.h: Defines a back-end specific class for the D3D11 renderer.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_RENDERER11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_RENDERER11_H_

#include "common/angleutils.h"
#include "common/mathutil.h"
#include "libANGLE/AttributeMap.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/d3d/HLSLCompiler.h"
#include "libANGLE/renderer/d3d/ProgramD3D.h"
#include "libANGLE/renderer/d3d/ProgramExecutableD3D.h"
#include "libANGLE/renderer/d3d/RenderTargetD3D.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/d3d/d3d11/DebugAnnotator11.h"
#include "libANGLE/renderer/d3d/d3d11/RenderStateCache.h"
#include "libANGLE/renderer/d3d/d3d11/ResourceManager11.h"
#include "libANGLE/renderer/d3d/d3d11/StateManager11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

namespace gl
{
class FramebufferAttachment;
class ImageIndex;
}  // namespace gl

namespace rx
{
class Blit11;
class Buffer11;
class Clear11;
class Context11;
class IndexDataManager;
struct PackPixelsParams;
class PixelTransfer11;
class RenderTarget11;
class StreamingIndexBufferInterface;
class Trim11;
class VertexDataManager;

struct Renderer11DeviceCaps
{
    Renderer11DeviceCaps();

    D3D_FEATURE_LEVEL featureLevel;
    bool supportsDXGI1_2;                         // Support for DXGI 1.2
    bool supportsClearView;                       // Support for ID3D11DeviceContext1::ClearView
    bool supportsConstantBufferOffsets;           // Support for Constant buffer offset
    bool supportsVpRtIndexWriteFromVertexShader;  // VP/RT can be selected in the Vertex Shader
                                                  // stage.
    bool supportsMultisampledDepthStencilSRVs;   // D3D feature level 10.0 no longer allows creation
                                                 // of textures with both the bind SRV and DSV flags
                                                 // when multisampled.  Textures will need to be
                                                 // resolved before reading. crbug.com/656989
    bool supportsTypedUAVLoadAdditionalFormats;  //
    // https://learn.microsoft.com/en-us/windows/win32/direct3d11/typed-unordered-access-view-loads
    bool supportsUAVLoadStoreCommonFormats;  // Do the common additional formats support load/store?
    bool supportsRasterizerOrderViews;
    bool allowES3OnFL10_0;
    UINT B5G6R5support;     // Bitfield of D3D11_FORMAT_SUPPORT values for DXGI_FORMAT_B5G6R5_UNORM
    UINT B5G6R5maxSamples;  // Maximum number of samples supported by DXGI_FORMAT_B5G6R5_UNORM
    UINT B4G4R4A4support;  // Bitfield of D3D11_FORMAT_SUPPORT values for DXGI_FORMAT_B4G4R4A4_UNORM
    UINT B4G4R4A4maxSamples;  // Maximum number of samples supported by DXGI_FORMAT_B4G4R4A4_UNORM
    UINT B5G5R5A1support;  // Bitfield of D3D11_FORMAT_SUPPORT values for DXGI_FORMAT_B5G5R5A1_UNORM
    UINT B5G5R5A1maxSamples;  // Maximum number of samples supported by DXGI_FORMAT_B5G5R5A1_UNORM
    Optional<LARGE_INTEGER> driverVersion;  // Four-part driver version number.
};

enum
{
    MAX_VERTEX_UNIFORM_VECTORS_D3D11   = 1024,
    MAX_FRAGMENT_UNIFORM_VECTORS_D3D11 = 1024
};

class Renderer11 : public RendererD3D
{
  public:
    explicit Renderer11(egl::Display *display);
    ~Renderer11() override;

    egl::Error initialize() override;
    bool resetDevice() override;

    egl::ConfigSet generateConfigs() override;
    void generateDisplayExtensions(egl::DisplayExtensions *outExtensions) const override;

    ContextImpl *createContext(const gl::State &state, gl::ErrorSet *errorSet) override;

    angle::Result flush(Context11 *context11);
    angle::Result finish(Context11 *context11);

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

    // lost device
    bool testDeviceLost() override;
    bool testDeviceResettable() override;

    DeviceIdentifier getAdapterIdentifier() const override;

    unsigned int getReservedVertexUniformVectors() const;
    unsigned int getReservedFragmentUniformVectors() const;
    gl::ShaderMap<unsigned int> getReservedShaderUniformBuffers() const;

    bool getShareHandleSupport() const;

    int getMajorShaderModel() const override;
    int getMinorShaderModel() const override;
    std::string getShaderModelSuffix() const override;

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

    TextureStorage *createTextureStorageBuffer(const gl::OffsetBindingPointer<gl::Buffer> &buffer,
                                               GLenum internalFormat,
                                               const std::string &label) override;
    TextureStorage *createTextureStorage2DMultisampleArray(GLenum internalformat,
                                                           GLsizei width,
                                                           GLsizei height,
                                                           GLsizei depth,
                                                           int levels,
                                                           int samples,
                                                           bool fixedSampleLocations,
                                                           const std::string &label) override;

    VertexBuffer *createVertexBuffer() override;
    IndexBuffer *createIndexBuffer() override;

    // Stream Creation
    StreamProducerImpl *createStreamProducerD3DTexture(egl::Stream::ConsumerType consumerType,
                                                       const egl::AttributeMap &attribs) override;

    // D3D11-renderer specific methods
    ID3D11Device *getDevice() { return mDevice.Get(); }
    ID3D11Device1 *getDevice1() { return mDevice1.Get(); }
    void *getD3DDevice() override;
    ID3D11DeviceContext *getDeviceContext() { return mDeviceContext.Get(); }
    ID3D11DeviceContext1 *getDeviceContext1IfSupported() { return mDeviceContext1.Get(); }
    IDXGIFactory *getDxgiFactory() { return mDxgiFactory.Get(); }

    angle::Result getBlendState(const gl::Context *context,
                                const d3d11::BlendStateKey &key,
                                const d3d11::BlendState **outBlendState);
    angle::Result getRasterizerState(const gl::Context *context,
                                     const gl::RasterizerState &rasterState,
                                     bool scissorEnabled,
                                     ID3D11RasterizerState **outRasterizerState);
    angle::Result getDepthStencilState(const gl::Context *context,
                                       const gl::DepthStencilState &dsState,
                                       const d3d11::DepthStencilState **outDSState);
    angle::Result getSamplerState(const gl::Context *context,
                                  const gl::SamplerState &samplerState,
                                  ID3D11SamplerState **outSamplerState);
    UINT getSampleDescQuality(GLuint supportedSamples) const;

    Blit11 *getBlitter() { return mBlit; }
    Clear11 *getClearer() { return mClear; }
    DebugAnnotatorContext11 *getDebugAnnotatorContext();

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

    angle::Result packPixels(const gl::Context *context,
                             const TextureHelper11 &textureHelper,
                             const PackPixelsParams &params,
                             uint8_t *pixelsOut);

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

    angle::Result readFromAttachment(const gl::Context *context,
                                     const gl::FramebufferAttachment &srcAttachment,
                                     const gl::Rectangle &sourceArea,
                                     GLenum format,
                                     GLenum type,
                                     GLuint outputPitch,
                                     const gl::PixelPackState &pack,
                                     uint8_t *pixels);

    angle::Result blitRenderbufferRect(const gl::Context *context,
                                       const gl::Rectangle &readRect,
                                       const gl::Rectangle &drawRect,
                                       UINT readLayer,
                                       UINT drawLayer,
                                       RenderTargetD3D *readRenderTarget,
                                       RenderTargetD3D *drawRenderTarget,
                                       GLenum filter,
                                       const gl::Rectangle *scissor,
                                       bool colorBlit,
                                       bool depthBlit,
                                       bool stencilBlit);

    bool isES3Capable() const;
    const Renderer11DeviceCaps &getRenderer11DeviceCaps() const { return mRenderer11DeviceCaps; }

    RendererClass getRendererClass() const override;
    StateManager11 *getStateManager() { return &mStateManager; }

    void onSwap();
    void onBufferCreate(const Buffer11 *created);
    void onBufferDelete(const Buffer11 *deleted);

    DeviceImpl *createEGLDevice() override;

    angle::Result drawArrays(const gl::Context *context,
                             gl::PrimitiveMode mode,
                             GLint firstVertex,
                             GLsizei vertexCount,
                             GLsizei instanceCount,
                             GLuint baseInstance,
                             bool isInstancedDraw);
    angle::Result drawElements(const gl::Context *context,
                               gl::PrimitiveMode mode,
                               GLint startVertex,
                               GLsizei indexCount,
                               gl::DrawElementsType indexType,
                               const void *indices,
                               GLsizei instanceCount,
                               GLint baseVertex,
                               GLuint baseInstance,
                               bool isInstancedDraw);
    angle::Result drawArraysIndirect(const gl::Context *context, const void *indirect);
    angle::Result drawElementsIndirect(const gl::Context *context, const void *indirect);

    // Necessary hack for default framebuffers in D3D.
    FramebufferImpl *createDefaultFramebuffer(const gl::FramebufferState &state) override;

    angle::Result getScratchMemoryBuffer(Context11 *context11,
                                         size_t requestedSize,
                                         angle::MemoryBuffer **bufferOut);

    gl::Version getMaxSupportedESVersion() const override;
    gl::Version getMaxConformantESVersion() const override;

    angle::Result dispatchCompute(const gl::Context *context,
                                  GLuint numGroupsX,
                                  GLuint numGroupsY,
                                  GLuint numGroupsZ);
    angle::Result dispatchComputeIndirect(const gl::Context *context, GLintptr indirect);

    angle::Result createStagingTexture(const gl::Context *context,
                                       ResourceType textureType,
                                       const d3d11::Format &formatSet,
                                       const gl::Extents &size,
                                       StagingAccess readAndWriteAccess,
                                       TextureHelper11 *textureOut);

    template <typename DescT, typename ResourceT>
    angle::Result allocateResource(d3d::Context *context, const DescT &desc, ResourceT *resourceOut)
    {
        return mResourceManager11.allocate(context, this, &desc, nullptr, resourceOut);
    }

    template <typename DescT, typename InitDataT, typename ResourceT>
    angle::Result allocateResource(d3d::Context *context,
                                   const DescT &desc,
                                   InitDataT *initData,
                                   ResourceT *resourceOut)
    {
        return mResourceManager11.allocate(context, this, &desc, initData, resourceOut);
    }

    template <typename InitDataT, typename ResourceT>
    angle::Result allocateResourceNoDesc(d3d::Context *context,
                                         InitDataT *initData,
                                         ResourceT *resourceOut)
    {
        return mResourceManager11.allocate(context, this, nullptr, initData, resourceOut);
    }

    template <typename DescT>
    angle::Result allocateTexture(d3d::Context *context,
                                  const DescT &desc,
                                  const d3d11::Format &format,
                                  TextureHelper11 *textureOut)
    {
        return allocateTexture(context, desc, format, nullptr, textureOut);
    }

    angle::Result allocateTexture(d3d::Context *context,
                                  const D3D11_TEXTURE2D_DESC &desc,
                                  const d3d11::Format &format,
                                  const D3D11_SUBRESOURCE_DATA *initData,
                                  TextureHelper11 *textureOut);

    angle::Result allocateTexture(d3d::Context *context,
                                  const D3D11_TEXTURE3D_DESC &desc,
                                  const d3d11::Format &format,
                                  const D3D11_SUBRESOURCE_DATA *initData,
                                  TextureHelper11 *textureOut);

    angle::Result clearRenderTarget(const gl::Context *context,
                                    RenderTargetD3D *renderTarget,
                                    const gl::ColorF &clearColorValue,
                                    const float clearDepthValue,
                                    const unsigned int clearStencilValue) override;

    bool canSelectViewInVertexShader() const override;

    angle::Result mapResource(const gl::Context *context,
                              ID3D11Resource *resource,
                              UINT subResource,
                              D3D11_MAP mapType,
                              UINT mapFlags,
                              D3D11_MAPPED_SUBRESOURCE *mappedResource);

    angle::Result getIncompleteTexture(const gl::Context *context,
                                       gl::TextureType type,
                                       gl::Texture **textureOut) override;

    void setGlobalDebugAnnotator() override;

    std::string getRendererDescription() const override;
    std::string getVendorString() const override;
    std::string getVersionString(bool includeFullVersion) const override;

  private:
    void generateCaps(gl::Caps *outCaps,
                      gl::TextureCapsMap *outTextureCaps,
                      gl::Extensions *outExtensions,
                      gl::Limitations *outLimitations,
                      ShPixelLocalStorageOptions *outPLSOptions) const override;

    void initializeFeatures(angle::FeaturesD3D *features) const override;

    void initializeFrontendFeatures(angle::FrontendFeatures *features) const override;

    angle::Result drawLineLoop(const gl::Context *context,
                               GLuint count,
                               gl::DrawElementsType type,
                               const void *indices,
                               int baseVertex,
                               int instances);
    angle::Result drawTriangleFan(const gl::Context *context,
                                  GLuint count,
                                  gl::DrawElementsType type,
                                  const void *indices,
                                  int baseVertex,
                                  int instances);

    angle::Result resolveMultisampledTexture(const gl::Context *context,
                                             RenderTarget11 *renderTarget,
                                             bool depth,
                                             bool stencil,
                                             TextureHelper11 *textureOut);

    void populateRenderer11DeviceCaps();

    void updateHistograms();

    angle::Result copyImageInternal(const gl::Context *context,
                                    const gl::Framebuffer *framebuffer,
                                    const gl::Rectangle &sourceRect,
                                    GLenum destFormat,
                                    const gl::Offset &destOffset,
                                    RenderTargetD3D *destRenderTarget);

    gl::SupportedSampleSet generateSampleSetForEGLConfig(
        const gl::TextureCaps &colorBufferFormatCaps,
        const gl::TextureCaps &depthStencilBufferFormatCaps) const;

    HRESULT callD3D11CreateDevice(PFN_D3D11_CREATE_DEVICE createDevice, bool debug);
    HRESULT callD3D11On12CreateDevice(PFN_D3D12_CREATE_DEVICE createDevice12,
                                      PFN_D3D11ON12_CREATE_DEVICE createDevice11on12,
                                      bool debug);
    egl::Error initializeDXGIAdapter();
    egl::Error initializeD3DDevice();
    egl::Error initializeDevice();
    egl::Error initializeAdapterFromDevice();
    void releaseDeviceResources();
    void release();

    d3d11::ANGLED3D11DeviceType getDeviceType() const;

    // Make sure that the raw buffer is the latest buffer.
    angle::Result markRawBufferUsage(const gl::Context *context);
    angle::Result markTypedBufferUsage(const gl::Context *context);
    angle::Result markTransformFeedbackUsage(const gl::Context *context);
    angle::Result drawWithGeometryShaderAndTransformFeedback(Context11 *context11,
                                                             gl::PrimitiveMode mode,
                                                             UINT instanceCount,
                                                             UINT vertexCount);

    HMODULE mD3d11Module;
    HMODULE mD3d12Module;
    HMODULE mDCompModule;
    std::vector<D3D_FEATURE_LEVEL> mAvailableFeatureLevels;
    D3D_DRIVER_TYPE mRequestedDriverType;
    bool mCreateDebugDevice;
    bool mCreatedWithDeviceEXT;

    HLSLCompiler mCompiler;

    RenderStateCache mStateCache;

    StateManager11 mStateManager;

    StreamingIndexBufferInterface *mLineLoopIB;
    StreamingIndexBufferInterface *mTriangleFanIB;

    // Texture copy resources
    Blit11 *mBlit;
    PixelTransfer11 *mPixelTransfer;

    // Masked clear resources
    Clear11 *mClear;

    // Perform trim for D3D resources
    Trim11 *mTrim;

    // Sync query
    d3d11::Query mSyncQuery;

    // Created objects state tracking
    std::set<const Buffer11 *> mAliveBuffers;

    double mLastHistogramUpdateTime;

    angle::ComPtr<ID3D12Device> mDevice12;
    angle::ComPtr<ID3D12CommandQueue> mCommandQueue;

    angle::ComPtr<ID3D11Device> mDevice;
    angle::ComPtr<ID3D11Device1> mDevice1;
    Renderer11DeviceCaps mRenderer11DeviceCaps;
    angle::ComPtr<ID3D11DeviceContext> mDeviceContext;
    angle::ComPtr<ID3D11DeviceContext1> mDeviceContext1;
    angle::ComPtr<ID3D11DeviceContext3> mDeviceContext3;
    angle::ComPtr<IDXGIAdapter> mDxgiAdapter;
    DXGI_ADAPTER_DESC mAdapterDescription;
    char mDescription[128];
    angle::ComPtr<IDXGIFactory> mDxgiFactory;
    angle::ComPtr<ID3D11Debug> mDebug;

    std::vector<GLuint> mScratchIndexDataBuffer;

    angle::ScratchBuffer mScratchMemoryBuffer;

    DebugAnnotatorContext11 mAnnotatorContext;

    mutable Optional<bool> mSupportsShareHandles;
    ResourceManager11 mResourceManager11;

    TextureHelper11 mCachedResolveTexture;
};

}  // namespace rx
#endif  // LIBANGLE_RENDERER_D3D_D3D11_RENDERER11_H_
