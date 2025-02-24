//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManager11.h: Defines a class for caching D3D11 state

#ifndef LIBANGLE_RENDERER_D3D11_STATEMANAGER11_H_
#define LIBANGLE_RENDERER_D3D11_STATEMANAGER11_H_

#include <array>

#include "libANGLE/State.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/d3d/IndexDataManager.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/d3d/d3d11/InputLayoutCache.h"
#include "libANGLE/renderer/d3d/d3d11/Query11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

namespace rx
{
class Buffer11;
class DisplayD3D;
class Framebuffer11;
struct RenderTargetDesc;
struct Renderer11DeviceCaps;
class VertexArray11;

class ShaderConstants11 : angle::NonCopyable
{
  public:
    ShaderConstants11();
    ~ShaderConstants11();

    void init(const gl::Caps &caps);
    size_t getRequiredBufferSize(gl::ShaderType shaderType) const;
    void markDirty();

    void setComputeWorkGroups(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ);
    void onViewportChange(const gl::Rectangle &glViewport,
                          const D3D11_VIEWPORT &dxViewport,
                          const gl::Offset &glFragCoordOffset,
                          bool is9_3,
                          bool presentPathFast);
    bool onFirstVertexChange(GLint firstVertex);
    void onImageLayerChange(gl::ShaderType shaderType, unsigned int imageIndex, int layer);
    void onSamplerChange(gl::ShaderType shaderType,
                         unsigned int samplerIndex,
                         const gl::Texture &texture,
                         const gl::SamplerState &samplerState);
    bool onImageChange(gl::ShaderType shaderType,
                       unsigned int imageIndex,
                       const gl::ImageUnit &imageUnit);
    void onClipOriginChange(bool lowerLeft);
    bool onClipDepthModeChange(bool zeroToOne);
    bool onClipDistancesEnabledChange(const uint32_t value);
    bool onMultisamplingChange(bool multisampling);

    angle::Result updateBuffer(const gl::Context *context,
                               Renderer11 *renderer,
                               gl::ShaderType shaderType,
                               const ProgramExecutableD3D &executableD3D,
                               const d3d11::Buffer &driverConstantBuffer);

  private:
    struct Vertex
    {
        Vertex()
            : depthRange{.0f},
              viewAdjust{.0f},
              viewCoords{.0f},
              viewScale{.0f},
              clipControlOrigin{-1.0f},
              clipControlZeroToOne{.0f},
              firstVertex{0},
              clipDistancesEnabled{0},
              padding{.0f}
        {}

        float depthRange[4];
        float viewAdjust[4];
        float viewCoords[4];
        float viewScale[2];

        // EXT_clip_control
        // Multiplied with Y coordinate: -1.0 for GL_LOWER_LEFT_EXT, 1.0f for GL_UPPER_LEFT_EXT
        float clipControlOrigin;
        // 0.0 for GL_NEGATIVE_ONE_TO_ONE_EXT, 1.0 for GL_ZERO_TO_ONE_EXT
        float clipControlZeroToOne;

        uint32_t firstVertex;

        uint32_t clipDistancesEnabled;

        // Added here to manually pad the struct to 16 byte boundary
        float padding[2];
    };
    static_assert(sizeof(Vertex) % 16u == 0,
                  "D3D11 constant buffers must be multiples of 16 bytes");

    struct Pixel
    {
        Pixel()
            : depthRange{.0f},
              viewCoords{.0f},
              depthFront{.0f},
              misc{0},
              fragCoordOffset{.0f},
              viewScale{.0f}
        {}

        float depthRange[4];
        float viewCoords[4];
        float depthFront[3];
        uint32_t misc;
        float fragCoordOffset[2];
        float viewScale[2];
    };
    // Packing information for pixel driver uniform's misc field:
    // - 1 bit for whether multisampled rendering is used
    // - 31 bits unused
    static constexpr uint32_t kPixelMiscMultisamplingMask = 0x1;
    static_assert(sizeof(Pixel) % 16u == 0, "D3D11 constant buffers must be multiples of 16 bytes");

    struct Compute
    {
        Compute() : numWorkGroups{0u}, padding(0u) {}
        unsigned int numWorkGroups[3];
        unsigned int padding;  // This just pads the struct to 16 bytes
    };

    struct SamplerMetadata
    {
        SamplerMetadata() : baseLevel(0), wrapModes(0), padding{0}, intBorderColor{0} {}

        int baseLevel;
        int wrapModes;
        int padding[2];  // This just pads the struct to 32 bytes
        int intBorderColor[4];
    };

    static_assert(sizeof(SamplerMetadata) == 32u,
                  "Sampler metadata struct must be two 4-vec --> 32 bytes.");

    struct ImageMetadata
    {
        ImageMetadata() : layer(0), level(0), padding{0} {}

        int layer;
        unsigned int level;
        int padding[2];  // This just pads the struct to 16 bytes
    };
    static_assert(sizeof(ImageMetadata) == 16u,
                  "Image metadata struct must be one 4-vec --> 16 bytes.");

    static size_t GetShaderConstantsStructSize(gl::ShaderType shaderType);

    // Return true if dirty.
    bool updateSamplerMetadata(SamplerMetadata *data,
                               const gl::Texture &texture,
                               const gl::SamplerState &samplerState);

    // Return true if dirty.
    bool updateImageMetadata(ImageMetadata *data, const gl::ImageUnit &imageUnit);

    Vertex mVertex;
    Pixel mPixel;
    Compute mCompute;
    gl::ShaderBitSet mShaderConstantsDirty;

    gl::ShaderMap<std::vector<SamplerMetadata>> mShaderSamplerMetadata;
    gl::ShaderMap<int> mNumActiveShaderSamplers;
    gl::ShaderMap<std::vector<ImageMetadata>> mShaderReadonlyImageMetadata;
    gl::ShaderMap<int> mNumActiveShaderReadonlyImages;
    gl::ShaderMap<std::vector<ImageMetadata>> mShaderImageMetadata;
    gl::ShaderMap<int> mNumActiveShaderImages;
};

class StateManager11 final : angle::NonCopyable
{
  public:
    StateManager11(Renderer11 *renderer);
    ~StateManager11();

    void deinitialize();

    void syncState(const gl::Context *context,
                   const gl::state::DirtyBits &dirtyBits,
                   const gl::state::ExtendedDirtyBits &extendedDirtyBits,
                   gl::Command command);

    angle::Result updateStateForCompute(const gl::Context *context,
                                        GLuint numGroupsX,
                                        GLuint numGroupsY,
                                        GLuint numGroupsZ);

    void updateStencilSizeIfChanged(bool depthStencilInitialized, unsigned int stencilSize);

    // These invalidations methods are called externally.

    // Called from TextureStorage11.
    void invalidateBoundViews();

    // Called from VertexArray11::updateVertexAttribStorage.
    void invalidateCurrentValueAttrib(size_t attribIndex);

    // Checks are done on a framebuffer state change to trigger other state changes.
    // The Context is allowed to be nullptr for these methods, when called in EGL init code.
    void invalidateRenderTarget();

    // Called by instanced point sprite emulation.
    void invalidateVertexBuffer();

    // Called by Framebuffer11::syncState for the default sized viewport.
    void invalidateViewport(const gl::Context *context);

    // Called by TextureStorage11::markLevelDirty.
    void invalidateSwizzles();

    // Called by the Framebuffer11 and VertexArray11.
    void invalidateShaders();

    // Called by the Program on Uniform Buffer change. Also called internally.
    void invalidateProgramUniformBuffers();

    // Called by TransformFeedback11.
    void invalidateTransformFeedback();

    // Called by VertexArray11.
    void invalidateInputLayout();

    // Called by VertexArray11 element array buffer sync.
    void invalidateIndexBuffer();

    // Called by TextureStorage11. Also called internally.
    void invalidateTexturesAndSamplers();

    void setRenderTarget(ID3D11RenderTargetView *rtv, ID3D11DepthStencilView *dsv);
    void setRenderTargets(ID3D11RenderTargetView **rtvs, UINT numRtvs, ID3D11DepthStencilView *dsv);

    void onBeginQuery(Query11 *query);
    void onDeleteQueryObject(Query11 *query);
    angle::Result onMakeCurrent(const gl::Context *context);

    void setInputLayout(const d3d11::InputLayout *inputLayout);

    void setSingleVertexBuffer(const d3d11::Buffer *buffer, UINT stride, UINT offset);

    angle::Result updateState(const gl::Context *context,
                              gl::PrimitiveMode mode,
                              GLint firstVertex,
                              GLsizei vertexOrIndexCount,
                              gl::DrawElementsType indexTypeOrInvalid,
                              const void *indices,
                              GLsizei instanceCount,
                              GLint baseVertex,
                              GLuint baseInstance,
                              bool promoteDynamic);

    void setShaderResourceShared(gl::ShaderType shaderType,
                                 UINT resourceSlot,
                                 const d3d11::SharedSRV *srv);
    void setShaderResource(gl::ShaderType shaderType,
                           UINT resourceSlot,
                           const d3d11::ShaderResourceView *srv);
    void setPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY primitiveTopology);

    void setDrawShaders(const d3d11::VertexShader *vertexShader,
                        const d3d11::GeometryShader *geometryShader,
                        const d3d11::PixelShader *pixelShader);
    void setVertexShader(const d3d11::VertexShader *shader);
    void setGeometryShader(const d3d11::GeometryShader *shader);
    void setPixelShader(const d3d11::PixelShader *shader);
    void setComputeShader(const d3d11::ComputeShader *shader);
    void setVertexConstantBuffer(unsigned int slot, const d3d11::Buffer *buffer);
    void setPixelConstantBuffer(unsigned int slot, const d3d11::Buffer *buffer);
    void setDepthStencilState(const d3d11::DepthStencilState *depthStencilState, UINT stencilRef);
    void setSimpleBlendState(const d3d11::BlendState *blendState);
    void setRasterizerState(const d3d11::RasterizerState *rasterizerState);
    void setSimpleViewport(const gl::Extents &viewportExtents);
    void setSimpleViewport(int width, int height);
    void setSimplePixelTextureAndSampler(const d3d11::SharedSRV &srv,
                                         const d3d11::SamplerState &samplerState);
    void setSimpleScissorRect(const gl::Rectangle &glRect);
    void setScissorRectD3D(const D3D11_RECT &d3dRect);

    void setIndexBuffer(ID3D11Buffer *buffer, DXGI_FORMAT indexFormat, unsigned int offset);

    angle::Result updateVertexOffsetsForPointSpritesEmulation(const gl::Context *context,
                                                              GLint startVertex,
                                                              GLsizei emulatedInstanceId);

    // TODO(jmadill): Should be private.
    angle::Result applyComputeUniforms(const gl::Context *context,
                                       ProgramExecutableD3D *executableD3D);

    // Only used in testing.
    InputLayoutCache *getInputLayoutCache() { return &mInputLayoutCache; }

    bool getCullEverything() const { return mCullEverything; }
    VertexDataManager *getVertexDataManager() { return &mVertexDataManager; }

    ProgramExecutableD3D *getProgramExecutableD3D() const { return mExecutableD3D; }

  private:
    angle::Result ensureInitialized(const gl::Context *context);

    template <typename SRVType>
    void setShaderResourceInternal(gl::ShaderType shaderType,
                                   UINT resourceSlot,
                                   const SRVType *srv);

    struct UAVList
    {
        UAVList(size_t size) : data(size) {}
        std::vector<ID3D11UnorderedAccessView *> data;
        int highestUsed = -1;
    };

    template <typename UAVType>
    void setUnorderedAccessViewInternal(UINT resourceSlot, const UAVType *uav, UAVList *uavList);

    void unsetConflictingView(gl::PipelineType pipeline, ID3D11View *view, bool isRenderTarget);
    void unsetConflictingSRVs(gl::PipelineType pipeline,
                              gl::ShaderType shaderType,
                              uintptr_t resource,
                              const gl::ImageIndex *index,
                              bool isRenderTarget);
    void unsetConflictingUAVs(gl::PipelineType pipeline,
                              gl::ShaderType shaderType,
                              uintptr_t resource,
                              const gl::ImageIndex *index);

    template <typename CacheType>
    void unsetConflictingRTVs(uintptr_t resource, CacheType &viewCache);

    void unsetConflictingRTVs(uintptr_t resource);

    void unsetConflictingAttachmentResources(const gl::FramebufferAttachment &attachment,
                                             ID3D11Resource *resource);

    angle::Result syncBlendState(const gl::Context *context,
                                 const gl::BlendStateExt &blendStateExt,
                                 const gl::ColorF &blendColor,
                                 unsigned int sampleMask,
                                 bool sampleAlphaToCoverage,
                                 bool emulateConstantAlpha);

    angle::Result syncDepthStencilState(const gl::Context *context);

    angle::Result syncRasterizerState(const gl::Context *context, gl::PrimitiveMode mode);

    void syncScissorRectangle(const gl::Context *context);

    void syncViewport(const gl::Context *context);

    void checkPresentPath(const gl::Context *context);

    angle::Result syncFramebuffer(const gl::Context *context);
    angle::Result syncProgram(const gl::Context *context, gl::PrimitiveMode drawMode);
    angle::Result syncProgramForCompute(const gl::Context *context);

    angle::Result syncTextures(const gl::Context *context);
    angle::Result applyTexturesForSRVs(const gl::Context *context, gl::ShaderType shaderType);
    angle::Result applyTexturesForUAVs(const gl::Context *context, gl::ShaderType shaderType);
    angle::Result syncTexturesForCompute(const gl::Context *context);

    angle::Result setSamplerState(const gl::Context *context,
                                  gl::ShaderType type,
                                  int index,
                                  gl::Texture *texture,
                                  const gl::SamplerState &sampler);
    angle::Result setTextureForSampler(const gl::Context *context,
                                       gl::ShaderType type,
                                       int index,
                                       gl::Texture *texture,
                                       const gl::SamplerState &sampler);
    angle::Result setImageState(const gl::Context *context,
                                gl::ShaderType type,
                                int index,
                                const gl::ImageUnit &imageUnit);
    angle::Result setTextureForImage(const gl::Context *context,
                                     gl::ShaderType type,
                                     int index,
                                     const gl::ImageUnit &imageUnit);
    angle::Result getUAVsForRWImages(const gl::Context *context,
                                     gl::ShaderType shaderType,
                                     UAVList *uavList);
    angle::Result getUAVForRWImage(const gl::Context *context,
                                   gl::ShaderType type,
                                   int index,
                                   const gl::ImageUnit &imageUnit,
                                   UAVList *uavList);

    angle::Result syncCurrentValueAttribs(
        const gl::Context *context,
        const std::vector<gl::VertexAttribCurrentValueData> &currentValues);

    angle::Result generateSwizzle(const gl::Context *context, gl::Texture *texture);
    angle::Result generateSwizzlesForShader(const gl::Context *context, gl::ShaderType type);
    angle::Result generateSwizzles(const gl::Context *context);

    angle::Result applyDriverUniforms(const gl::Context *context);
    angle::Result applyDriverUniformsForShader(const gl::Context *context,
                                               gl::ShaderType shaderType);
    angle::Result applyUniforms(const gl::Context *context);
    angle::Result applyUniformsForShader(const gl::Context *context, gl::ShaderType shaderType);

    angle::Result getUAVsForShaderStorageBuffers(const gl::Context *context,
                                                 gl::ShaderType shaderType,
                                                 UAVList *uavList);

    angle::Result syncUniformBuffers(const gl::Context *context);
    angle::Result syncUniformBuffersForShader(const gl::Context *context,
                                              gl::ShaderType shaderType);
    angle::Result getUAVsForAtomicCounterBuffers(const gl::Context *context,
                                                 gl::ShaderType shaderType,
                                                 UAVList *uavList);
    angle::Result getUAVsForShader(const gl::Context *context,
                                   gl::ShaderType shaderType,
                                   UAVList *uavList);
    angle::Result syncUAVsForGraphics(const gl::Context *context);
    angle::Result syncUAVsForCompute(const gl::Context *context);
    angle::Result syncTransformFeedbackBuffers(const gl::Context *context);

    // These are currently only called internally.
    void invalidateDriverUniforms();
    void invalidateProgramUniforms();
    void invalidateConstantBuffer(unsigned int slot);
    void invalidateProgramAtomicCounterBuffers();
    void invalidateProgramShaderStorageBuffers();
    void invalidateImageBindings();

    // Called by the Framebuffer11 directly.
    void processFramebufferInvalidation(const gl::Context *context);

    bool syncIndexBuffer(ID3D11Buffer *buffer, DXGI_FORMAT indexFormat, unsigned int offset);
    angle::Result syncVertexBuffersAndInputLayout(const gl::Context *context,
                                                  gl::PrimitiveMode mode,
                                                  GLint firstVertex,
                                                  GLsizei vertexOrIndexCount,
                                                  gl::DrawElementsType indexTypeOrInvalid,
                                                  GLsizei instanceCount);

    bool setInputLayoutInternal(const d3d11::InputLayout *inputLayout);

    angle::Result applyVertexBuffers(const gl::Context *context,
                                     gl::PrimitiveMode mode,
                                     gl::DrawElementsType indexTypeOrInvalid,
                                     GLint firstVertex);
    // TODO(jmadill): Migrate to d3d11::Buffer.
    bool queueVertexBufferChange(size_t bufferIndex,
                                 ID3D11Buffer *buffer,
                                 UINT stride,
                                 UINT offset);
    void applyVertexBufferChanges();
    bool setPrimitiveTopologyInternal(D3D11_PRIMITIVE_TOPOLOGY primitiveTopology);
    void syncPrimitiveTopology(const gl::State &glState, gl::PrimitiveMode currentDrawMode);

    // Not handled by an internal dirty bit because it isn't synced on drawArrays calls.
    angle::Result applyIndexBuffer(const gl::Context *context,
                                   GLsizei indexCount,
                                   gl::DrawElementsType indexType,
                                   const void *indices);

    enum DirtyBitType
    {
        DIRTY_BIT_RENDER_TARGET,
        DIRTY_BIT_VIEWPORT_STATE,
        DIRTY_BIT_SCISSOR_STATE,
        DIRTY_BIT_RASTERIZER_STATE,
        DIRTY_BIT_BLEND_STATE,
        DIRTY_BIT_DEPTH_STENCIL_STATE,
        // DIRTY_BIT_SHADERS and DIRTY_BIT_TEXTURE_AND_SAMPLER_STATE should be dealt before
        // DIRTY_BIT_PROGRAM_UNIFORM_BUFFERS for update image layers.
        DIRTY_BIT_SHADERS,
        // DIRTY_BIT_GRAPHICS_SRV_STATE and DIRTY_BIT_COMPUTE_SRV_STATE should be lower
        // bits than DIRTY_BIT_TEXTURE_AND_SAMPLER_STATE.
        DIRTY_BIT_GRAPHICS_SRV_STATE,
        DIRTY_BIT_GRAPHICS_UAV_STATE,
        DIRTY_BIT_COMPUTE_SRV_STATE,
        DIRTY_BIT_COMPUTE_UAV_STATE,
        DIRTY_BIT_TEXTURE_AND_SAMPLER_STATE,
        DIRTY_BIT_PROGRAM_UNIFORMS,
        DIRTY_BIT_DRIVER_UNIFORMS,
        DIRTY_BIT_PROGRAM_UNIFORM_BUFFERS,
        DIRTY_BIT_CURRENT_VALUE_ATTRIBS,
        DIRTY_BIT_TRANSFORM_FEEDBACK,
        DIRTY_BIT_VERTEX_BUFFERS_AND_INPUT_LAYOUT,
        DIRTY_BIT_PRIMITIVE_TOPOLOGY,
        DIRTY_BIT_INVALID,
        DIRTY_BIT_MAX = DIRTY_BIT_INVALID,
    };

    using DirtyBits = angle::BitSet<DIRTY_BIT_MAX>;

    Renderer11 *mRenderer;

    // Internal dirty bits.
    DirtyBits mInternalDirtyBits;
    DirtyBits mGraphicsDirtyBitsMask;
    DirtyBits mComputeDirtyBitsMask;

    bool mCurSampleAlphaToCoverage;

    // Blend State
    gl::BlendStateExt mCurBlendStateExt;
    gl::ColorF mCurBlendColor;
    unsigned int mCurSampleMask;

    // Currently applied depth stencil state
    gl::DepthStencilState mCurDepthStencilState;
    int mCurStencilRef;
    int mCurStencilBackRef;
    unsigned int mCurStencilSize;
    Optional<bool> mCurDisableDepth;
    Optional<bool> mCurDisableStencil;

    // Currently applied rasterizer state
    gl::RasterizerState mCurRasterState;

    // Currently applied scissor rectangle state
    bool mCurScissorEnabled;
    gl::Rectangle mCurScissorRect;

    // Currently applied viewport state
    gl::Rectangle mCurViewport;
    float mCurNear;
    float mCurFar;

    // Currently applied offset to viewport and scissor
    gl::Offset mCurViewportOffset;
    gl::Offset mCurScissorOffset;

    // Things needed in viewport state
    ShaderConstants11 mShaderConstants;

    // Render target variables
    gl::Extents mViewportBounds;
    bool mRenderTargetIsDirty;

    // EGL_ANGLE_experimental_present_path variables
    bool mCurPresentPathFastEnabled;
    int mCurPresentPathFastColorBufferHeight;

    // Queries that are currently active in this state
    std::set<Query11 *> mCurrentQueries;

    // Currently applied textures
    template <typename DescType>
    struct ViewRecord
    {
        uintptr_t view;
        uintptr_t resource;
        DescType desc;
    };

    // A cache of current Views that also tracks the highest 'used' (non-NULL) View.
    // We might want to investigate a more robust approach that is also fast when there's
    // a large gap between used Views (e.g. if View 0 and 7 are non-NULL, this approach will
    // waste time on Views 1-6.)
    template <typename ViewType, typename DescType>
    class ViewCache : angle::NonCopyable
    {
      public:
        ViewCache();
        ~ViewCache();

        void initialize(size_t size) { mCurrentViews.resize(size); }

        size_t size() const { return mCurrentViews.size(); }
        size_t highestUsed() const { return mHighestUsedView; }

        const ViewRecord<DescType> &operator[](size_t index) const { return mCurrentViews[index]; }
        void clear();
        void update(size_t resourceIndex, ViewType *view);

      private:
        std::vector<ViewRecord<DescType>> mCurrentViews;
        size_t mHighestUsedView;
    };

    using SRVCache = ViewCache<ID3D11ShaderResourceView, D3D11_SHADER_RESOURCE_VIEW_DESC>;
    using UAVCache = ViewCache<ID3D11UnorderedAccessView, D3D11_UNORDERED_ACCESS_VIEW_DESC>;
    using RTVCache = ViewCache<ID3D11RenderTargetView, D3D11_RENDER_TARGET_VIEW_DESC>;
    using DSVCache = ViewCache<ID3D11DepthStencilView, D3D11_DEPTH_STENCIL_VIEW_DESC>;
    gl::ShaderMap<SRVCache> mCurShaderSRVs;
    UAVCache mCurComputeUAVs;
    RTVCache mCurRTVs;
    DSVCache mCurrentDSV;

    SRVCache *getSRVCache(gl::ShaderType shaderType);

    // A block of NULL pointers, cached so we don't re-allocate every draw call
    std::vector<ID3D11ShaderResourceView *> mNullSRVs;
    std::vector<ID3D11UnorderedAccessView *> mNullUAVs;

    // Current translations of "Current-Value" data - owned by Context, not VertexArray.
    gl::AttributesMask mDirtyCurrentValueAttribs;
    std::vector<TranslatedAttribute> mCurrentValueAttribs;

    // Current applied input layout.
    ResourceSerial mCurrentInputLayout;

    // Current applied vertex states.
    // TODO(jmadill): Figure out how to use ResourceSerial here.
    gl::AttribArray<ID3D11Buffer *> mCurrentVertexBuffers;
    gl::AttribArray<UINT> mCurrentVertexStrides;
    gl::AttribArray<UINT> mCurrentVertexOffsets;
    gl::RangeUI mDirtyVertexBufferRange;

    // Currently applied primitive topology
    D3D11_PRIMITIVE_TOPOLOGY mCurrentPrimitiveTopology;
    gl::PrimitiveMode mLastAppliedDrawMode;
    bool mCullEverything;

    // Currently applied shaders
    gl::ShaderMap<ResourceSerial> mAppliedShaders;

    // Currently applied sampler states
    gl::ShaderMap<std::vector<bool>> mForceSetShaderSamplerStates;
    gl::ShaderMap<std::vector<gl::SamplerState>> mCurShaderSamplerStates;

    // Special dirty bit for swizzles. Since they use internal shaders, must be done in a pre-pass.
    bool mDirtySwizzles;

    // Currently applied index buffer
    ID3D11Buffer *mAppliedIB;
    DXGI_FORMAT mAppliedIBFormat;
    unsigned int mAppliedIBOffset;
    bool mIndexBufferIsDirty;

    // Vertex, index and input layouts
    VertexDataManager mVertexDataManager;
    IndexDataManager mIndexDataManager;
    InputLayoutCache mInputLayoutCache;
    std::vector<const TranslatedAttribute *> mCurrentAttributes;
    Optional<GLint> mLastFirstVertex;

    bool mIsMultiviewEnabled;

    bool mIndependentBlendStates;

    // Driver Constants.
    gl::ShaderMap<d3d11::Buffer> mShaderDriverConstantBuffers;

    ResourceSerial mCurrentComputeConstantBuffer;
    ResourceSerial mCurrentGeometryConstantBuffer;

    template <typename T>
    using VertexConstantBufferArray =
        std::array<T, gl::IMPLEMENTATION_MAX_VERTEX_SHADER_UNIFORM_BUFFERS>;

    VertexConstantBufferArray<ResourceSerial> mCurrentConstantBufferVS;
    VertexConstantBufferArray<GLintptr> mCurrentConstantBufferVSOffset;
    VertexConstantBufferArray<GLsizeiptr> mCurrentConstantBufferVSSize;

    template <typename T>
    using FragmentConstantBufferArray =
        std::array<T, gl::IMPLEMENTATION_MAX_FRAGMENT_SHADER_UNIFORM_BUFFERS>;

    FragmentConstantBufferArray<ResourceSerial> mCurrentConstantBufferPS;
    FragmentConstantBufferArray<GLintptr> mCurrentConstantBufferPSOffset;
    FragmentConstantBufferArray<GLsizeiptr> mCurrentConstantBufferPSSize;

    template <typename T>
    using ComputeConstantBufferArray =
        std::array<T, gl::IMPLEMENTATION_MAX_COMPUTE_SHADER_UNIFORM_BUFFERS>;

    ComputeConstantBufferArray<ResourceSerial> mCurrentConstantBufferCS;
    ComputeConstantBufferArray<GLintptr> mCurrentConstantBufferCSOffset;
    ComputeConstantBufferArray<GLsizeiptr> mCurrentConstantBufferCSSize;

    // Currently applied transform feedback buffers
    UniqueSerial mAppliedTFSerial;

    UniqueSerial mEmptySerial;

    // These objects are cached to avoid having to query the impls.
    ProgramExecutableD3D *mExecutableD3D;
    VertexArray11 *mVertexArray11;
    Framebuffer11 *mFramebuffer11;
};

}  // namespace rx
#endif  // LIBANGLE_RENDERER_D3D11_STATEMANAGER11_H_
