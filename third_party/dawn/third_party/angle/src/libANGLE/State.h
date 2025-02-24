//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// State.h: Defines the State class, encapsulating raw GL state

#ifndef LIBANGLE_STATE_H_
#define LIBANGLE_STATE_H_

#include <bitset>
#include <memory>

#include "common/Color.h"
#include "common/angleutils.h"
#include "common/bitset_utils.h"
#include "libANGLE/ContextMutex.h"
#include "libANGLE/Debug.h"
#include "libANGLE/GLES1State.h"
#include "libANGLE/Overlay.h"
#include "libANGLE/Program.h"
#include "libANGLE/ProgramExecutable.h"
#include "libANGLE/ProgramPipeline.h"
#include "libANGLE/RefCountObject.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/Sampler.h"
#include "libANGLE/Texture.h"
#include "libANGLE/TransformFeedback.h"
#include "libANGLE/Version.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/angletypes.h"

namespace egl
{
class ShareGroup;
}  // namespace egl

namespace gl
{
class BufferManager;
struct Caps;
class Context;
class FramebufferManager;
class MemoryObjectManager;
class ProgramPipelineManager;
class Query;
class RenderbufferManager;
class SamplerManager;
class SemaphoreManager;
class ShaderProgramManager;
class SyncManager;
class TextureManager;
class VertexArray;

static constexpr Version ES_1_0 = Version(1, 0);
static constexpr Version ES_1_1 = Version(1, 1);
static constexpr Version ES_2_0 = Version(2, 0);
static constexpr Version ES_3_0 = Version(3, 0);
static constexpr Version ES_3_1 = Version(3, 1);
static constexpr Version ES_3_2 = Version(3, 2);

template <typename T>
using BufferBindingMap     = angle::PackedEnumMap<BufferBinding, T>;
using BoundBufferMap       = BufferBindingMap<BindingPointer<Buffer>>;
using TextureBindingVector = std::vector<BindingPointer<Texture>>;
using TextureBindingMap    = angle::PackedEnumMap<TextureType, TextureBindingVector>;
using ActiveQueryMap       = angle::PackedEnumMap<QueryType, BindingPointer<Query>>;

class ActiveTexturesCache final : angle::NonCopyable
{
  public:
    ActiveTexturesCache();
    ~ActiveTexturesCache();

    Texture *operator[](size_t textureIndex) const { return mTextures[textureIndex]; }

    void clear();
    void set(size_t textureIndex, Texture *texture);
    void reset(size_t textureIndex);
    bool empty() const;
    size_t size() const { return mTextures.size(); }

  private:
    ActiveTextureArray<Texture *> mTextures;
};

namespace state
{
enum DirtyBitType
{
    // Note: process draw framebuffer binding first, so that other dirty bits whose effect
    // depend on the current draw framebuffer are not processed while the old framebuffer is
    // still bound.
    DIRTY_BIT_DRAW_FRAMEBUFFER_BINDING,
    DIRTY_BIT_READ_FRAMEBUFFER_BINDING,
    DIRTY_BIT_SCISSOR_TEST_ENABLED,
    DIRTY_BIT_SCISSOR,
    DIRTY_BIT_VIEWPORT,
    DIRTY_BIT_DEPTH_RANGE,
    DIRTY_BIT_BLEND_ENABLED,
    DIRTY_BIT_BLEND_COLOR,
    DIRTY_BIT_BLEND_FUNCS,
    DIRTY_BIT_BLEND_EQUATIONS,
    DIRTY_BIT_COLOR_MASK,
    DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE_ENABLED,
    DIRTY_BIT_SAMPLE_COVERAGE_ENABLED,
    DIRTY_BIT_SAMPLE_COVERAGE,
    DIRTY_BIT_SAMPLE_MASK_ENABLED,
    DIRTY_BIT_SAMPLE_MASK,
    DIRTY_BIT_DEPTH_TEST_ENABLED,
    DIRTY_BIT_DEPTH_FUNC,
    DIRTY_BIT_DEPTH_MASK,
    DIRTY_BIT_STENCIL_TEST_ENABLED,
    DIRTY_BIT_STENCIL_FUNCS_FRONT,
    DIRTY_BIT_STENCIL_FUNCS_BACK,
    DIRTY_BIT_STENCIL_OPS_FRONT,
    DIRTY_BIT_STENCIL_OPS_BACK,
    DIRTY_BIT_STENCIL_WRITEMASK_FRONT,
    DIRTY_BIT_STENCIL_WRITEMASK_BACK,
    DIRTY_BIT_CULL_FACE_ENABLED,
    DIRTY_BIT_CULL_FACE,
    DIRTY_BIT_FRONT_FACE,
    DIRTY_BIT_POLYGON_OFFSET_FILL_ENABLED,
    DIRTY_BIT_POLYGON_OFFSET,
    DIRTY_BIT_RASTERIZER_DISCARD_ENABLED,
    DIRTY_BIT_LINE_WIDTH,
    DIRTY_BIT_PRIMITIVE_RESTART_ENABLED,
    DIRTY_BIT_CLEAR_COLOR,
    DIRTY_BIT_CLEAR_DEPTH,
    DIRTY_BIT_CLEAR_STENCIL,
    DIRTY_BIT_UNPACK_STATE,
    DIRTY_BIT_UNPACK_BUFFER_BINDING,
    DIRTY_BIT_PACK_STATE,
    DIRTY_BIT_PACK_BUFFER_BINDING,
    DIRTY_BIT_DITHER_ENABLED,
    DIRTY_BIT_RENDERBUFFER_BINDING,
    DIRTY_BIT_VERTEX_ARRAY_BINDING,
    DIRTY_BIT_DRAW_INDIRECT_BUFFER_BINDING,
    DIRTY_BIT_DISPATCH_INDIRECT_BUFFER_BINDING,
    // Note: Fine-grained dirty bits for each index could be an optimization.
    DIRTY_BIT_PROGRAM_BINDING,  // Must be before DIRTY_BIT_PROGRAM_EXECUTABLE
    DIRTY_BIT_PROGRAM_EXECUTABLE,
    // Note: Fine-grained dirty bits for each texture/sampler could be an optimization.
    DIRTY_BIT_SAMPLER_BINDINGS,
    DIRTY_BIT_TEXTURE_BINDINGS,
    DIRTY_BIT_IMAGE_BINDINGS,
    DIRTY_BIT_TRANSFORM_FEEDBACK_BINDING,
    DIRTY_BIT_UNIFORM_BUFFER_BINDINGS,
    DIRTY_BIT_SHADER_STORAGE_BUFFER_BINDING,
    DIRTY_BIT_ATOMIC_COUNTER_BUFFER_BINDING,
    DIRTY_BIT_MULTISAMPLING,
    DIRTY_BIT_SAMPLE_ALPHA_TO_ONE,
    DIRTY_BIT_COVERAGE_MODULATION,                  // CHROMIUM_framebuffer_mixed_samples
    DIRTY_BIT_FRAMEBUFFER_SRGB_WRITE_CONTROL_MODE,  // GL_EXT_sRGB_write_control
    DIRTY_BIT_CURRENT_VALUES,
    DIRTY_BIT_PROVOKING_VERTEX,
    DIRTY_BIT_SAMPLE_SHADING,
    DIRTY_BIT_PATCH_VERTICES,
    DIRTY_BIT_EXTENDED,  // clip distances, mipmap generation hint, derivative hint,
                         // EXT_clip_control, EXT_depth_clamp

    DIRTY_BIT_INVALID,
    DIRTY_BIT_MAX = DIRTY_BIT_INVALID,
};
static_assert(DIRTY_BIT_MAX <= 64, "State dirty bits must be capped at 64");
using DirtyBits = angle::BitSet<DIRTY_BIT_MAX>;

enum ExtendedDirtyBitType
{
    EXTENDED_DIRTY_BIT_CLIP_CONTROL,                  // EXT_clip_control
    EXTENDED_DIRTY_BIT_CLIP_DISTANCES,                // clip distances
    EXTENDED_DIRTY_BIT_DEPTH_CLAMP_ENABLED,           // EXT_depth_clamp
    EXTENDED_DIRTY_BIT_MIPMAP_GENERATION_HINT,        // mipmap generation hint
    EXTENDED_DIRTY_BIT_POLYGON_MODE,                  // NV_polygon_mode
    EXTENDED_DIRTY_BIT_POLYGON_OFFSET_POINT_ENABLED,  // NV_polygon_mode
    EXTENDED_DIRTY_BIT_POLYGON_OFFSET_LINE_ENABLED,   // NV_polygon_mode
    EXTENDED_DIRTY_BIT_SHADER_DERIVATIVE_HINT,        // shader derivative hint
    EXTENDED_DIRTY_BIT_SHADING_RATE,                  // QCOM_shading_rate
    EXTENDED_DIRTY_BIT_LOGIC_OP_ENABLED,              // ANGLE_logic_op
    EXTENDED_DIRTY_BIT_LOGIC_OP,                      // ANGLE_logic_op
    EXTENDED_DIRTY_BIT_BLEND_ADVANCED_COHERENT,       // KHR_blend_operation_advanced_coherent

    EXTENDED_DIRTY_BIT_INVALID,
    EXTENDED_DIRTY_BIT_MAX = EXTENDED_DIRTY_BIT_INVALID,
};
static_assert(EXTENDED_DIRTY_BIT_MAX <= 32, "State extended dirty bits must be capped at 32");
using ExtendedDirtyBits = angle::BitSet32<EXTENDED_DIRTY_BIT_MAX>;

// TODO(jmadill): Consider storing dirty objects in a list instead of by binding.
enum DirtyObjectType
{
    DIRTY_OBJECT_ACTIVE_TEXTURES,  // Top-level dirty bit. Also see mDirtyActiveTextures.
    DIRTY_OBJECT_TEXTURES_INIT,
    DIRTY_OBJECT_IMAGES_INIT,
    DIRTY_OBJECT_READ_ATTACHMENTS,
    DIRTY_OBJECT_DRAW_ATTACHMENTS,
    DIRTY_OBJECT_READ_FRAMEBUFFER,
    DIRTY_OBJECT_DRAW_FRAMEBUFFER,
    DIRTY_OBJECT_VERTEX_ARRAY,
    DIRTY_OBJECT_TEXTURES,  // Top-level dirty bit. Also see mDirtyTextures.
    DIRTY_OBJECT_IMAGES,    // Top-level dirty bit. Also see mDirtyImages.
    DIRTY_OBJECT_SAMPLERS,  // Top-level dirty bit. Also see mDirtySamplers.
    DIRTY_OBJECT_PROGRAM_PIPELINE_OBJECT,

    DIRTY_OBJECT_INVALID,
    DIRTY_OBJECT_MAX = DIRTY_OBJECT_INVALID,
};
using DirtyObjects = angle::BitSet<DIRTY_OBJECT_MAX>;

}  // namespace state

// This class represents the portion of the GL context's state that is purely private to the
// context. Manipulating this state does not affect the other contexts in any way, nor do operations
// in other contexts affect this.
//
// Note that "currently bound X" states do not belong here because unbinding most objects could lead
// to object destruction which in turn may trigger a notification to an observer that may affect
// another context.
class PrivateState : angle::NonCopyable
{
  public:
    PrivateState(const Version &clientVersion,
                 bool debug,
                 bool bindGeneratesResourceCHROMIUM,
                 bool clientArraysEnabled,
                 bool robustResourceInit,
                 bool programBinaryCacheEnabled,
                 bool isExternal);
    ~PrivateState();

    void initialize(Context *context);
    void initializeForCapture(const Context *context);

    void reset();

    const Version &getClientVersion() const { return mClientVersion; }
    GLint getClientMajorVersion() const { return mClientVersion.major; }
    GLint getClientMinorVersion() const { return mClientVersion.minor; }

    bool isWebGL() const { return getExtensions().webglCompatibilityANGLE; }
    bool isWebGL1() const { return isWebGL() && getClientVersion().major == 2; }
    bool isGLES1() const { return getClientVersion() < ES_2_0; }

    const Caps &getCaps() const { return mCaps; }
    const TextureCapsMap &getTextureCaps() const { return mTextureCaps; }
    const Extensions &getExtensions() const { return mExtensions; }
    const Limitations &getLimitations() const { return mLimitations; }

    bool isExternal() const { return mIsExternal; }

    Caps *getMutableCaps() { return &mCaps; }
    TextureCapsMap *getMutableTextureCaps() { return &mTextureCaps; }
    Extensions *getMutableExtensions() { return &mExtensions; }
    Limitations *getMutableLimitations() { return &mLimitations; }

    // State chunk getters
    const RasterizerState &getRasterizerState() const { return mRasterizer; }
    const BlendState &getBlendState() const { return mBlendState; }
    const BlendStateExt &getBlendStateExt() const { return mBlendStateExt; }
    const DepthStencilState &getDepthStencilState() const { return mDepthStencil; }

    // Clear values
    void setColorClearValue(float red, float green, float blue, float alpha);
    void setDepthClearValue(float depth);
    void setStencilClearValue(int stencil);

    const ColorF &getColorClearValue() const { return mColorClearValue; }
    float getDepthClearValue() const { return mDepthClearValue; }
    int getStencilClearValue() const { return mStencilClearValue; }

    // Write mask manipulation
    void setColorMask(bool red, bool green, bool blue, bool alpha);
    void setColorMaskIndexed(bool red, bool green, bool blue, bool alpha, GLuint index);
    void setDepthMask(bool mask);

    // Discard toggle & query
    bool isRasterizerDiscardEnabled() const { return mRasterizer.rasterizerDiscard; }
    void setRasterizerDiscard(bool enabled);

    // Primitive restart
    bool isPrimitiveRestartEnabled() const { return mPrimitiveRestart; }
    void setPrimitiveRestart(bool enabled);

    // Face culling state manipulation
    bool isCullFaceEnabled() const { return mRasterizer.cullFace; }
    void setCullFace(bool enabled);
    void setCullMode(CullFaceMode mode);
    void setFrontFace(GLenum front);

    // EXT_depth_clamp
    bool isDepthClampEnabled() const { return mRasterizer.depthClamp; }
    void setDepthClamp(bool enabled);

    // Depth test state manipulation
    bool isDepthTestEnabled() const { return mDepthStencil.depthTest; }
    bool isDepthWriteEnabled() const { return mDepthStencil.depthTest && mDepthStencil.depthMask; }
    void setDepthTest(bool enabled);
    void setDepthFunc(GLenum depthFunc);
    void setDepthRange(float zNear, float zFar);
    float getNearPlane() const { return mNearZ; }
    float getFarPlane() const { return mFarZ; }

    // EXT_clip_control
    void setClipControl(ClipOrigin origin, ClipDepthMode depth);
    ClipOrigin getClipOrigin() const { return mClipOrigin; }
    ClipDepthMode getClipDepthMode() const { return mClipDepthMode; }
    bool isClipDepthModeZeroToOne() const { return mClipDepthMode == ClipDepthMode::ZeroToOne; }

    // Blend state manipulation
    bool isBlendEnabled() const { return mBlendStateExt.getEnabledMask().test(0); }
    bool isBlendEnabledIndexed(GLuint index) const
    {
        ASSERT(static_cast<size_t>(index) < mBlendStateExt.getDrawBufferCount());
        return mBlendStateExt.getEnabledMask().test(index);
    }
    DrawBufferMask getBlendEnabledDrawBufferMask() const { return mBlendStateExt.getEnabledMask(); }
    void setBlend(bool enabled);
    void setBlendIndexed(bool enabled, GLuint index);
    void setBlendFactors(GLenum sourceRGB, GLenum destRGB, GLenum sourceAlpha, GLenum destAlpha);
    void setBlendFactorsIndexed(GLenum sourceRGB,
                                GLenum destRGB,
                                GLenum sourceAlpha,
                                GLenum destAlpha,
                                GLuint index);
    void setBlendColor(float red, float green, float blue, float alpha);
    void setBlendEquation(GLenum rgbEquation, GLenum alphaEquation);
    void setBlendEquationIndexed(GLenum rgbEquation, GLenum alphaEquation, GLuint index);
    const ColorF &getBlendColor() const { return mBlendColor; }

    // Stencil state maniupulation
    bool isStencilTestEnabled() const { return mDepthStencil.stencilTest; }
    bool isStencilWriteEnabled(GLuint framebufferStencilSize) const
    {
        return mDepthStencil.stencilTest &&
               !(mDepthStencil.isStencilNoOp(framebufferStencilSize) &&
                 mDepthStencil.isStencilBackNoOp(framebufferStencilSize));
    }
    void setStencilTest(bool enabled);
    void setStencilParams(GLenum stencilFunc, GLint stencilRef, GLuint stencilMask);
    void setStencilBackParams(GLenum stencilBackFunc, GLint stencilBackRef, GLuint stencilBackMask);
    void setStencilWritemask(GLuint stencilWritemask);
    void setStencilBackWritemask(GLuint stencilBackWritemask);
    void setStencilOperations(GLenum stencilFail,
                              GLenum stencilPassDepthFail,
                              GLenum stencilPassDepthPass);
    void setStencilBackOperations(GLenum stencilBackFail,
                                  GLenum stencilBackPassDepthFail,
                                  GLenum stencilBackPassDepthPass);
    GLint getStencilRef() const { return mStencilRef; }
    GLint getStencilBackRef() const { return mStencilBackRef; }

    PolygonMode getPolygonMode() const { return mRasterizer.polygonMode; }
    void setPolygonMode(PolygonMode mode);

    // Depth bias/polygon offset state manipulation
    bool isPolygonOffsetPointEnabled() const { return mRasterizer.polygonOffsetPoint; }
    bool isPolygonOffsetLineEnabled() const { return mRasterizer.polygonOffsetLine; }
    bool isPolygonOffsetFillEnabled() const { return mRasterizer.polygonOffsetFill; }
    bool isPolygonOffsetEnabled() const { return mRasterizer.isPolygonOffsetEnabled(); }
    void setPolygonOffsetPoint(bool enabled);
    void setPolygonOffsetLine(bool enabled);
    void setPolygonOffsetFill(bool enabled);
    void setPolygonOffsetParams(GLfloat factor, GLfloat units, GLfloat clamp);

    // Multisample coverage state manipulation
    bool isSampleAlphaToCoverageEnabled() const { return mSampleAlphaToCoverage; }
    void setSampleAlphaToCoverage(bool enabled);
    bool isSampleCoverageEnabled() const { return mSampleCoverage; }
    void setSampleCoverage(bool enabled);
    void setSampleCoverageParams(GLclampf value, bool invert);
    GLclampf getSampleCoverageValue() const { return mSampleCoverageValue; }
    bool getSampleCoverageInvert() const { return mSampleCoverageInvert; }

    // Multisample mask state manipulation.
    bool isSampleMaskEnabled() const { return mSampleMask; }
    void setSampleMaskEnabled(bool enabled);
    void setSampleMaskParams(GLuint maskNumber, GLbitfield mask);
    GLbitfield getSampleMaskWord(GLuint maskNumber) const
    {
        ASSERT(maskNumber < mMaxSampleMaskWords);
        return mSampleMaskValues[maskNumber];
    }
    SampleMaskArray<GLbitfield> getSampleMaskValues() const { return mSampleMaskValues; }
    GLuint getMaxSampleMaskWords() const { return mMaxSampleMaskWords; }

    // Multisampling/alpha to one manipulation.
    void setSampleAlphaToOne(bool enabled);
    bool isSampleAlphaToOneEnabled() const { return mSampleAlphaToOne; }
    void setMultisampling(bool enabled);
    bool isMultisamplingEnabled() const { return mMultiSampling; }

    void setSampleShading(bool enabled);
    bool isSampleShadingEnabled() const { return mIsSampleShadingEnabled; }
    void setMinSampleShading(float value);
    float getMinSampleShading() const { return mMinSampleShading; }

    // Scissor test state toggle & query
    bool isScissorTestEnabled() const { return mScissorTest; }
    void setScissorTest(bool enabled);
    void setScissorParams(GLint x, GLint y, GLsizei width, GLsizei height);
    const Rectangle &getScissor() const { return mScissor; }

    // Dither state toggle & query
    bool isDitherEnabled() const { return mRasterizer.dither; }
    void setDither(bool enabled);

    // GL_KHR_blend_equation_advanced_coherent
    void setBlendAdvancedCoherent(bool enabled);
    bool isBlendAdvancedCoherentEnabled() const { return mBlendAdvancedCoherent; }

    // GL_CHROMIUM_bind_generates_resource
    bool isBindGeneratesResourceEnabled() const { return mBindGeneratesResource; }

    // GL_ANGLE_client_arrays
    bool areClientArraysEnabled() const { return mClientArraysEnabled; }

    // GL_ANGLE_robust_resource_initialization
    bool isRobustResourceInitEnabled() const { return mRobustResourceInit; }

    // GL_ANGLE_program_cache_control
    bool isProgramBinaryCacheEnabled() const { return mProgramBinaryCacheEnabled; }

    // Viewport state setter/getter
    void setViewportParams(GLint x, GLint y, GLsizei width, GLsizei height);
    const Rectangle &getViewport() const { return mViewport; }

    // QCOM_shading_rate helpers
    void setShadingRate(GLenum rate);
    ShadingRate getShadingRate() const { return mShadingRate; }

    // Pixel pack state manipulation
    void setPackAlignment(GLint alignment);
    GLint getPackAlignment() const { return mPack.alignment; }
    void setPackReverseRowOrder(bool reverseRowOrder);
    bool getPackReverseRowOrder() const { return mPack.reverseRowOrder; }
    void setPackRowLength(GLint rowLength);
    GLint getPackRowLength() const { return mPack.rowLength; }
    void setPackSkipRows(GLint skipRows);
    GLint getPackSkipRows() const { return mPack.skipRows; }
    void setPackSkipPixels(GLint skipPixels);
    GLint getPackSkipPixels() const { return mPack.skipPixels; }
    const PixelPackState &getPackState() const { return mPack; }
    PixelPackState &getPackState() { return mPack; }

    // Pixel unpack state manipulation
    void setUnpackAlignment(GLint alignment);
    GLint getUnpackAlignment() const { return mUnpack.alignment; }
    void setUnpackRowLength(GLint rowLength);
    GLint getUnpackRowLength() const { return mUnpack.rowLength; }
    void setUnpackImageHeight(GLint imageHeight);
    GLint getUnpackImageHeight() const { return mUnpack.imageHeight; }
    void setUnpackSkipImages(GLint skipImages);
    GLint getUnpackSkipImages() const { return mUnpack.skipImages; }
    void setUnpackSkipRows(GLint skipRows);
    GLint getUnpackSkipRows() const { return mUnpack.skipRows; }
    void setUnpackSkipPixels(GLint skipPixels);
    GLint getUnpackSkipPixels() const { return mUnpack.skipPixels; }
    const PixelUnpackState &getUnpackState() const { return mUnpack; }
    PixelUnpackState &getUnpackState() { return mUnpack; }

    // CHROMIUM_framebuffer_mixed_samples coverage modulation
    void setCoverageModulation(GLenum components);
    GLenum getCoverageModulation() const { return mCoverageModulation; }

    // GL_EXT_sRGB_write_control
    void setFramebufferSRGB(bool sRGB);
    bool getFramebufferSRGB() const { return mFramebufferSRGB; }

    // GL_EXT_tessellation_shader
    void setPatchVertices(GLuint value);
    GLuint getPatchVertices() const { return mPatchVertices; }

    // GL_ANGLE_shader_pixel_local_storage
    void setPixelLocalStorageActivePlanes(GLsizei n);
    GLsizei getPixelLocalStorageActivePlanes() const { return mPixelLocalStorageActivePlanes; }
    // While pixel local storage is active, some draw buffers may be reserved for internal use by
    // PLS and blocked from the client. All draw buffers at or beyond 'firstActivePLSDrawBuffer' are
    // overridden.
    bool hasActivelyOverriddenPLSDrawBuffers(GLint *firstActivePLSDrawBuffer) const;
    bool isActivelyOverriddenPLSDrawBuffer(GLint drawbuffer) const;

    // Line width state setter
    void setLineWidth(GLfloat width);
    float getLineWidth() const { return mLineWidth; }

    void setActiveSampler(unsigned int active);
    unsigned int getActiveSampler() const { return static_cast<unsigned int>(mActiveSampler); }

    // Hint setters
    void setGenerateMipmapHint(GLenum hint);
    GLenum getGenerateMipmapHint() const { return mGenerateMipmapHint; }
    GLenum getFragmentShaderDerivativeHint() const { return mFragmentShaderDerivativeHint; }
    void setFragmentShaderDerivativeHint(GLenum hint);

    ProvokingVertexConvention getProvokingVertex() const { return mProvokingVertex; }
    void setProvokingVertex(ProvokingVertexConvention val)
    {
        mDirtyBits.set(state::DIRTY_BIT_PROVOKING_VERTEX);
        mProvokingVertex = val;
    }

    const VertexAttribCurrentValueData &getVertexAttribCurrentValue(size_t attribNum) const
    {
        ASSERT(attribNum < mVertexAttribCurrentValues.size());
        return mVertexAttribCurrentValues[attribNum];
    }
    const std::vector<VertexAttribCurrentValueData> &getVertexAttribCurrentValues() const
    {
        return mVertexAttribCurrentValues;
    }
    // This actually clears the current value dirty bits.
    // TODO(jmadill): Pass mutable dirty bits into Impl.
    AttributesMask getAndResetDirtyCurrentValues() const;
    ComponentTypeMask getCurrentValuesTypeMask() const { return mCurrentValuesTypeMask; }

    const ClipDistanceEnableBits &getEnabledClipDistances() const { return mClipDistancesEnabled; }
    void setClipDistanceEnable(int idx, bool enable);

    bool noSimultaneousConstantColorAndAlphaBlendFunc() const
    {
        return mNoSimultaneousConstantColorAndAlphaBlendFunc;
    }

    GLfloat getBoundingBoxMinX() const { return mBoundingBoxMinX; }
    GLfloat getBoundingBoxMinY() const { return mBoundingBoxMinY; }
    GLfloat getBoundingBoxMinZ() const { return mBoundingBoxMinZ; }
    GLfloat getBoundingBoxMinW() const { return mBoundingBoxMinW; }
    GLfloat getBoundingBoxMaxX() const { return mBoundingBoxMaxX; }
    GLfloat getBoundingBoxMaxY() const { return mBoundingBoxMaxY; }
    GLfloat getBoundingBoxMaxZ() const { return mBoundingBoxMaxZ; }
    GLfloat getBoundingBoxMaxW() const { return mBoundingBoxMaxW; }
    void setBoundingBox(GLfloat minX,
                        GLfloat minY,
                        GLfloat minZ,
                        GLfloat minW,
                        GLfloat maxX,
                        GLfloat maxY,
                        GLfloat maxZ,
                        GLfloat maxW);

    bool isTextureRectangleEnabled() const { return mTextureRectangleEnabled; }

    DrawBufferMask getBlendFuncConstantAlphaDrawBuffers() const
    {
        return mBlendFuncConstantAlphaDrawBuffers;
    }

    DrawBufferMask getBlendFuncConstantColorDrawBuffers() const
    {
        return mBlendFuncConstantColorDrawBuffers;
    }

    void setLogicOpEnabled(bool enabled);
    bool isLogicOpEnabled() const { return mLogicOpEnabled; }

    void setLogicOp(LogicalOperation opcode);
    LogicalOperation getLogicOp() const { return mLogicOp; }

    // Vertex attrib manipulation
    void setVertexAttribf(GLuint index, const GLfloat values[4]);
    void setVertexAttribu(GLuint index, const GLuint values[4]);
    void setVertexAttribi(GLuint index, const GLint values[4]);

    // QCOM_tiled_rendering
    void setTiledRendering(bool tiledRendering) { mTiledRendering = tiledRendering; }
    bool isTiledRendering() const { return mTiledRendering; }

    // Debug state
    const Debug &getDebug() const { return mDebug; }
    Debug &getDebug() { return mDebug; }

    // GL_ANGLE_blob_cache
    const BlobCacheCallbacks &getBlobCacheCallbacks() const { return mBlobCacheCallbacks; }
    BlobCacheCallbacks &getBlobCacheCallbacks() { return mBlobCacheCallbacks; }

    // Generic state toggle & query
    void setEnableFeature(GLenum feature, bool enabled);
    void setEnableFeatureIndexed(GLenum feature, bool enabled, GLuint index);
    bool getEnableFeature(GLenum feature) const;
    bool getEnableFeatureIndexed(GLenum feature, GLuint index) const;

    // State query functions
    void getBooleanv(GLenum pname, GLboolean *params) const;
    void getFloatv(GLenum pname, GLfloat *params) const;
    void getIntegerv(GLenum pname, GLint *params) const;
    void getIntegeri_v(GLenum target, GLuint index, GLint *data) const;
    void getBooleani_v(GLenum target, GLuint index, GLboolean *data) const;

    GLES1State *getMutableGLES1State() { return &mGLES1State; }
    const GLES1State &gles1() const { return mGLES1State; }

    const state::DirtyBits &getDirtyBits() const { return mDirtyBits; }
    void clearDirtyBits() { mDirtyBits.reset(); }
    void clearDirtyBits(const state::DirtyBits &bitset) { mDirtyBits &= ~bitset; }
    void setAllDirtyBits()
    {
        mDirtyBits.set();
        mExtendedDirtyBits.set();
        mDirtyCurrentValues = mAllAttribsMask;
    }

    const state::ExtendedDirtyBits &getExtendedDirtyBits() const { return mExtendedDirtyBits; }
    void clearExtendedDirtyBits() { mExtendedDirtyBits.reset(); }
    void clearExtendedDirtyBits(const state::ExtendedDirtyBits &bitset)
    {
        mExtendedDirtyBits &= ~bitset;
    }

    const state::DirtyObjects &getDirtyObjects() const { return mDirtyObjects; }
    void clearDirtyObjects() { mDirtyObjects.reset(); }

    void setPerfMonitorActive(bool active) { mIsPerfMonitorActive = active; }
    bool isPerfMonitorActive() const { return mIsPerfMonitorActive; }

  private:
    bool hasConstantColor(GLenum sourceRGB, GLenum destRGB) const;
    bool hasConstantAlpha(GLenum sourceRGB, GLenum destRGB) const;

    const Version mClientVersion;

    // Caps to use for validation
    Caps mCaps;
    TextureCapsMap mTextureCaps;
    Extensions mExtensions;
    Limitations mLimitations;
    const bool mIsExternal;

    ColorF mColorClearValue;
    GLfloat mDepthClearValue;
    int mStencilClearValue;

    RasterizerState mRasterizer;
    bool mScissorTest;
    Rectangle mScissor;

    bool mNoUnclampedBlendColor;

    BlendState mBlendState;  // Buffer zero blend state legacy struct
    BlendStateExt mBlendStateExt;
    ColorF mBlendColor;
    bool mSampleAlphaToCoverage;
    bool mSampleCoverage;
    GLfloat mSampleCoverageValue;
    bool mSampleCoverageInvert;
    bool mSampleMask;
    GLuint mMaxSampleMaskWords;
    SampleMaskArray<GLbitfield> mSampleMaskValues;
    bool mIsSampleShadingEnabled;
    float mMinSampleShading;

    DepthStencilState mDepthStencil;
    GLint mStencilRef;
    GLint mStencilBackRef;

    GLfloat mLineWidth;

    GLenum mGenerateMipmapHint;
    GLenum mFragmentShaderDerivativeHint;

    Rectangle mViewport;
    float mNearZ;
    float mFarZ;

    ClipOrigin mClipOrigin;
    ClipDepthMode mClipDepthMode;

    // GL_ANGLE_provoking_vertex
    ProvokingVertexConvention mProvokingVertex;

    using VertexAttribVector = std::vector<VertexAttribCurrentValueData>;
    VertexAttribVector mVertexAttribCurrentValues;  // From glVertexAttrib
    ComponentTypeMask mCurrentValuesTypeMask;

    // Mask of all attributes that are available to this context: [0, maxVertexAttributes)
    AttributesMask mAllAttribsMask;

    // Texture and sampler bindings
    GLint mActiveSampler;  // Active texture unit selector - GL_TEXTURE0

    PixelUnpackState mUnpack;
    PixelPackState mPack;

    bool mPrimitiveRestart;

    bool mMultiSampling;
    bool mSampleAlphaToOne;

    // GL_KHR_blend_equation_advanced_coherent
    bool mBlendAdvancedCoherent;

    GLenum mCoverageModulation;

    // GL_EXT_sRGB_write_control
    bool mFramebufferSRGB;

    // GL_ANGLE_webgl_compatibility
    bool mTextureRectangleEnabled;

    // GL_ANGLE_logic_op
    bool mLogicOpEnabled;
    LogicalOperation mLogicOp;

    // GL_APPLE_clip_distance / GL_EXT_clip_cull_distance / GL_ANGLE_clip_cull_distance
    ClipDistanceEnableBits mClipDistancesEnabled;

    // GL_EXT_tessellation_shader
    GLuint mPatchVertices;

    // GL_ANGLE_shader_pixel_local_storage
    GLsizei mPixelLocalStorageActivePlanes;

    // GLES1 emulation: state specific to GLES1
    GLES1State mGLES1State;

    // OES_draw_buffers_indexed
    DrawBufferMask mBlendFuncConstantAlphaDrawBuffers;
    DrawBufferMask mBlendFuncConstantColorDrawBuffers;
    bool mNoSimultaneousConstantColorAndAlphaBlendFunc;
    // Whether the indexed variants of setBlend* have been called.  If so, the call to the
    // non-indexed variants are not no-oped.
    bool mSetBlendIndexedInvoked;
    bool mSetBlendFactorsIndexedInvoked;
    bool mSetBlendEquationsIndexedInvoked;

    // GL_EXT_primitive_bounding_box
    GLfloat mBoundingBoxMinX;
    GLfloat mBoundingBoxMinY;
    GLfloat mBoundingBoxMinZ;
    GLfloat mBoundingBoxMinW;
    GLfloat mBoundingBoxMaxX;
    GLfloat mBoundingBoxMaxY;
    GLfloat mBoundingBoxMaxZ;
    GLfloat mBoundingBoxMaxW;

    // QCOM_shading_rate
    bool mShadingRatePreserveAspectRatio;
    ShadingRate mShadingRate;

    // GL_ARM_shader_framebuffer_fetch
    bool mFetchPerSample;

    // Whether perf monitoring is enabled through GL_AMD_performance_monitor.
    bool mIsPerfMonitorActive;

    // QCOM_tiled_rendering
    bool mTiledRendering;

    const bool mBindGeneratesResource;
    const bool mClientArraysEnabled;
    const bool mRobustResourceInit;
    const bool mProgramBinaryCacheEnabled;

    Debug mDebug;

    // ANGLE_blob_cache
    BlobCacheCallbacks mBlobCacheCallbacks;

    state::DirtyBits mDirtyBits;
    state::ExtendedDirtyBits mExtendedDirtyBits;
    state::DirtyObjects mDirtyObjects;
    mutable AttributesMask mDirtyCurrentValues;
};

// This class represents all of the GL context's state.
class State : angle::NonCopyable
{
  public:
    State(const State *shareContextState,
          egl::ShareGroup *shareGroup,
          TextureManager *shareTextures,
          SemaphoreManager *shareSemaphores,
          egl::ContextMutex *contextMutex,
          const OverlayType *overlay,
          const Version &clientVersion,
          bool debug,
          bool bindGeneratesResourceCHROMIUM,
          bool clientArraysEnabled,
          bool robustResourceInit,
          bool programBinaryCacheEnabled,
          EGLenum contextPriority,
          bool hasRobustAccess,
          bool hasProtectedContent,
          bool isExternal);
    ~State();

    void initialize(Context *context);
    void reset(const Context *context);

    // Getters
    ContextID getContextID() const { return mID; }
    EGLenum getContextPriority() const { return mContextPriority; }
    bool hasRobustAccess() const { return mHasRobustAccess; }
    bool hasProtectedContent() const { return mHasProtectedContent; }
    bool isDebugContext() const { return mIsDebugContext; }
    GLint getClientMajorVersion() const { return mPrivateState.getClientMajorVersion(); }
    GLint getClientMinorVersion() const { return mPrivateState.getClientMinorVersion(); }
    const Version &getClientVersion() const { return mPrivateState.getClientVersion(); }
    egl::ShareGroup *getShareGroup() const { return mShareGroup; }

    bool isWebGL() const { return mPrivateState.isWebGL(); }
    bool isWebGL1() const { return mPrivateState.isWebGL1(); }
    bool isGLES1() const { return mPrivateState.isGLES1(); }

    const Caps &getCaps() const { return mPrivateState.getCaps(); }
    const TextureCapsMap &getTextureCaps() const { return mPrivateState.getTextureCaps(); }
    const Extensions &getExtensions() const { return mPrivateState.getExtensions(); }
    const Limitations &getLimitations() const { return mPrivateState.getLimitations(); }

    bool isExternal() const { return mPrivateState.isExternal(); }

    Caps *getMutableCaps() { return mPrivateState.getMutableCaps(); }
    TextureCapsMap *getMutableTextureCaps() { return mPrivateState.getMutableTextureCaps(); }
    Extensions *getMutableExtensions() { return mPrivateState.getMutableExtensions(); }
    Limitations *getMutableLimitations() { return mPrivateState.getMutableLimitations(); }

    const TextureCaps &getTextureCap(GLenum internalFormat) const
    {
        return getTextureCaps().get(internalFormat);
    }

    bool allActiveDrawBufferChannelsMasked() const;
    bool anyActiveDrawBufferChannelMasked() const;

    // Texture binding & active texture unit manipulation
    void setSamplerTexture(const Context *context, TextureType type, Texture *texture);
    Texture *getTargetTexture(TextureType type) const
    {
        return getSamplerTexture(getActiveSampler(), type);
    }

    Texture *getSamplerTexture(unsigned int sampler, TextureType type) const
    {
        ASSERT(sampler < mSamplerTextures[type].size());
        return mSamplerTextures[type][sampler].get();
    }

    TextureID getSamplerTextureId(unsigned int sampler, TextureType type) const;
    void detachTexture(Context *context, const TextureMap &zeroTextures, TextureID texture);
    void initializeZeroTextures(const Context *context, const TextureMap &zeroTextures);

    void invalidateTextureBindings(TextureType type);

    // Sampler object binding manipulation
    void setSamplerBinding(const Context *context, GLuint textureUnit, Sampler *sampler);
    SamplerID getSamplerId(GLuint textureUnit) const
    {
        ASSERT(textureUnit < mSamplers.size());
        return mSamplers[textureUnit].id();
    }

    Sampler *getSampler(GLuint textureUnit) const { return mSamplers[textureUnit].get(); }

    const SamplerBindingVector &getSamplers() const { return mSamplers; }

    void detachSampler(const Context *context, SamplerID sampler);

    // Renderbuffer binding manipulation
    void setRenderbufferBinding(const Context *context, Renderbuffer *renderbuffer);
    RenderbufferID getRenderbufferId() const { return mRenderbuffer.id(); }
    Renderbuffer *getCurrentRenderbuffer() const { return mRenderbuffer.get(); }
    void detachRenderbuffer(Context *context, RenderbufferID renderbuffer);

    // Framebuffer binding manipulation
    void setReadFramebufferBinding(Framebuffer *framebuffer);
    void setDrawFramebufferBinding(Framebuffer *framebuffer);
    Framebuffer *getTargetFramebuffer(GLenum target) const;
    Framebuffer *getReadFramebuffer() const { return mReadFramebuffer; }
    Framebuffer *getDrawFramebuffer() const { return mDrawFramebuffer; }
    Framebuffer *getDefaultFramebuffer() const;

    bool removeReadFramebufferBinding(FramebufferID framebuffer);
    bool removeDrawFramebufferBinding(FramebufferID framebuffer);

    // Vertex array object binding manipulation
    void setVertexArrayBinding(const Context *context, VertexArray *vertexArray);
    bool removeVertexArrayBinding(const Context *context, VertexArrayID vertexArray);
    VertexArrayID getVertexArrayId() const;

    VertexArray *getVertexArray() const
    {
        ASSERT(mVertexArray != nullptr);
        return mVertexArray;
    }

    // If both a Program and a ProgramPipeline are bound, the Program will
    // always override the ProgramPipeline.
    ProgramExecutable *getProgramExecutable() const { return mExecutable.get(); }
    void ensureNoPendingLink(const Context *context) const
    {
        if (mProgram)
        {
            mProgram->resolveLink(context);
        }
        else if (mProgramPipeline.get())
        {
            mProgramPipeline->resolveLink(context);
        }
    }
    ProgramExecutable *getLinkedProgramExecutable(const Context *context) const
    {
        ensureNoPendingLink(context);
        return mExecutable.get();
    }

    // Program binding manipulation
    angle::Result setProgram(const Context *context, Program *newProgram);

    Program *getProgram() const
    {
        ASSERT(!mProgram || !mProgram->isLinking());
        return mProgram;
    }

    Program *getLinkedProgram(const Context *context) const
    {
        if (mProgram)
        {
            mProgram->resolveLink(context);
        }
        return mProgram;
    }

    ProgramPipeline *getProgramPipeline() const { return mProgramPipeline.get(); }

    ProgramPipeline *getLinkedProgramPipeline(const Context *context) const
    {
        if (mProgramPipeline.get())
        {
            mProgramPipeline->resolveLink(context);
        }
        return mProgramPipeline.get();
    }

    // Transform feedback object (not buffer) binding manipulation
    void setTransformFeedbackBinding(const Context *context, TransformFeedback *transformFeedback);
    TransformFeedback *getCurrentTransformFeedback() const { return mTransformFeedback.get(); }

    ANGLE_INLINE bool isTransformFeedbackActive() const
    {
        TransformFeedback *curTransformFeedback = mTransformFeedback.get();
        return curTransformFeedback && curTransformFeedback->isActive();
    }
    ANGLE_INLINE bool isTransformFeedbackActiveUnpaused() const
    {
        TransformFeedback *curTransformFeedback = mTransformFeedback.get();
        return curTransformFeedback && curTransformFeedback->isActive() &&
               !curTransformFeedback->isPaused();
    }

    bool removeTransformFeedbackBinding(const Context *context,
                                        TransformFeedbackID transformFeedback);

    // Query binding manipulation
    bool isQueryActive(QueryType type) const;
    bool isQueryActive(Query *query) const;
    void setActiveQuery(const Context *context, QueryType type, Query *query);
    QueryID getActiveQueryId(QueryType type) const;
    Query *getActiveQuery(QueryType type) const;

    // Program Pipeline binding manipulation
    angle::Result setProgramPipelineBinding(const Context *context, ProgramPipeline *pipeline);
    void detachProgramPipeline(const Context *context, ProgramPipelineID pipeline);

    //// Typed buffer binding point manipulation ////
    ANGLE_INLINE void setBufferBinding(const Context *context, BufferBinding target, Buffer *buffer)
    {
        (this->*(kBufferSetters[target]))(context, buffer);
    }

    ANGLE_INLINE Buffer *getTargetBuffer(BufferBinding target) const
    {
        switch (target)
        {
            case BufferBinding::ElementArray:
                return getVertexArray()->getElementArrayBuffer();
            default:
                return mBoundBuffers[target].get();
        }
    }

    ANGLE_INLINE Buffer *getArrayBuffer() const { return getTargetBuffer(BufferBinding::Array); }

    angle::Result setIndexedBufferBinding(const Context *context,
                                          BufferBinding target,
                                          GLuint index,
                                          Buffer *buffer,
                                          GLintptr offset,
                                          GLsizeiptr size);

    size_t getAtomicCounterBufferCount() const { return mAtomicCounterBuffers.size(); }

    ANGLE_INLINE bool hasValidAtomicCounterBuffer() const
    {
        return mBoundAtomicCounterBuffersMask.any();
    }

    const OffsetBindingPointer<Buffer> &getIndexedUniformBuffer(size_t index) const;
    const OffsetBindingPointer<Buffer> &getIndexedAtomicCounterBuffer(size_t index) const;
    const OffsetBindingPointer<Buffer> &getIndexedShaderStorageBuffer(size_t index) const;

    const angle::BitSet<gl::IMPLEMENTATION_MAX_UNIFORM_BUFFER_BINDINGS> &getUniformBuffersMask()
        const
    {
        return mBoundUniformBuffersMask;
    }
    const angle::BitSet<gl::IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS> &
    getAtomicCounterBuffersMask() const
    {
        return mBoundAtomicCounterBuffersMask;
    }
    const angle::BitSet<gl::IMPLEMENTATION_MAX_SHADER_STORAGE_BUFFER_BINDINGS> &
    getShaderStorageBuffersMask() const
    {
        return mBoundShaderStorageBuffersMask;
    }

    // Detach a buffer from all bindings
    angle::Result detachBuffer(Context *context, const Buffer *buffer);

    // Vertex attrib manipulation
    void setEnableVertexAttribArray(unsigned int attribNum, bool enabled);

    ANGLE_INLINE void setVertexAttribPointer(const Context *context,
                                             unsigned int attribNum,
                                             Buffer *boundBuffer,
                                             GLint size,
                                             VertexAttribType type,
                                             bool normalized,
                                             GLsizei stride,
                                             const void *pointer)
    {
        mVertexArray->setVertexAttribPointer(context, attribNum, boundBuffer, size, type,
                                             normalized, stride, pointer);
        mDirtyObjects.set(state::DIRTY_OBJECT_VERTEX_ARRAY);
    }

    ANGLE_INLINE void setVertexAttribIPointer(const Context *context,
                                              unsigned int attribNum,
                                              Buffer *boundBuffer,
                                              GLint size,
                                              VertexAttribType type,
                                              GLsizei stride,
                                              const void *pointer)
    {
        mVertexArray->setVertexAttribIPointer(context, attribNum, boundBuffer, size, type, stride,
                                              pointer);
        mDirtyObjects.set(state::DIRTY_OBJECT_VERTEX_ARRAY);
    }

    void setVertexAttribDivisor(const Context *context, GLuint index, GLuint divisor);
    const void *getVertexAttribPointer(unsigned int attribNum) const;

    void bindVertexBuffer(const Context *context,
                          GLuint bindingIndex,
                          Buffer *boundBuffer,
                          GLintptr offset,
                          GLsizei stride);
    void setVertexAttribFormat(GLuint attribIndex,
                               GLint size,
                               VertexAttribType type,
                               bool normalized,
                               bool pureInteger,
                               GLuint relativeOffset);

    void setVertexAttribBinding(const Context *context, GLuint attribIndex, GLuint bindingIndex)
    {
        mVertexArray->setVertexAttribBinding(context, attribIndex, bindingIndex);
        mDirtyObjects.set(state::DIRTY_OBJECT_VERTEX_ARRAY);
    }

    void setVertexBindingDivisor(const Context *context, GLuint bindingIndex, GLuint divisor);

    // State query functions
    void getBooleanv(GLenum pname, GLboolean *params) const;
    void getFloatv(GLenum pname, GLfloat *params) const { mPrivateState.getFloatv(pname, params); }
    angle::Result getIntegerv(const Context *context, GLenum pname, GLint *params) const;
    void getPointerv(const Context *context, GLenum pname, void **params) const;
    void getIntegeri_v(const Context *context, GLenum target, GLuint index, GLint *data) const;
    void getInteger64i_v(GLenum target, GLuint index, GLint64 *data) const;
    void getBooleani_v(GLenum target, GLuint index, GLboolean *data) const;

    bool isDrawFramebufferBindingDirty() const
    {
        return mDirtyBits.test(state::DIRTY_BIT_DRAW_FRAMEBUFFER_BINDING);
    }

    // Sets the dirty bit for the program executable.
    angle::Result installProgramExecutable(const Context *context);
    // Sets the dirty bit for the program pipeline executable.
    angle::Result installProgramPipelineExecutable(const Context *context);

    const state::DirtyBits getDirtyBits() const
    {
        return mDirtyBits | mPrivateState.getDirtyBits();
    }
    void clearDirtyBits()
    {
        mDirtyBits.reset();
        mPrivateState.clearDirtyBits();
    }
    void clearDirtyBits(const state::DirtyBits &bitset)
    {
        mDirtyBits &= ~bitset;
        mPrivateState.clearDirtyBits(bitset);
    }
    void setAllDirtyBits()
    {
        mDirtyBits.set();
        mExtendedDirtyBits.set();
        mPrivateState.setAllDirtyBits();
    }

    const state::ExtendedDirtyBits getExtendedDirtyBits() const
    {
        return mExtendedDirtyBits | mPrivateState.getExtendedDirtyBits();
    }
    void clearExtendedDirtyBits()
    {
        mExtendedDirtyBits.reset();
        mPrivateState.clearExtendedDirtyBits();
    }
    void clearExtendedDirtyBits(const state::ExtendedDirtyBits &bitset)
    {
        mExtendedDirtyBits &= ~bitset;
        mPrivateState.clearExtendedDirtyBits(bitset);
    }

    void clearDirtyObjects()
    {
        mDirtyObjects.reset();
        mPrivateState.clearDirtyObjects();
    }
    void setAllDirtyObjects() { mDirtyObjects.set(); }
    angle::Result syncDirtyObjects(const Context *context,
                                   const state::DirtyObjects &bitset,
                                   Command command);
    angle::Result syncDirtyObject(const Context *context, GLenum target);
    void setObjectDirty(GLenum target);
    void setTextureDirty(size_t textureUnitIndex);
    void setSamplerDirty(size_t samplerIndex);

    ANGLE_INLINE void setReadFramebufferDirty()
    {
        mDirtyObjects.set(state::DIRTY_OBJECT_READ_FRAMEBUFFER);
        mDirtyObjects.set(state::DIRTY_OBJECT_READ_ATTACHMENTS);
    }

    ANGLE_INLINE void setDrawFramebufferDirty()
    {
        mDirtyObjects.set(state::DIRTY_OBJECT_DRAW_FRAMEBUFFER);
        mDirtyObjects.set(state::DIRTY_OBJECT_DRAW_ATTACHMENTS);
    }

    void setImageUnit(const Context *context,
                      size_t unit,
                      Texture *texture,
                      GLint level,
                      GLboolean layered,
                      GLint layer,
                      GLenum access,
                      GLenum format);

    const ImageUnit &getImageUnit(size_t unit) const { return mImageUnits[unit]; }
    const ActiveTexturesCache &getActiveTexturesCache() const { return mActiveTexturesCache; }

    // "onActiveTextureChange" is called when a texture binding changes.
    void onActiveTextureChange(const Context *context, size_t textureUnit);

    // "onActiveTextureStateChange" is called when the Texture changed but the binding did not.
    void onActiveTextureStateChange(const Context *context, size_t textureUnit);

    void onImageStateChange(const Context *context, size_t unit);

    void onUniformBufferStateChange(size_t uniformBufferIndex);
    void onAtomicCounterBufferStateChange(size_t atomicCounterBufferIndex);
    void onShaderStorageBufferStateChange(size_t shaderStorageBufferIndex);

    bool isCurrentTransformFeedback(const TransformFeedback *tf) const
    {
        return tf == mTransformFeedback.get();
    }
    bool isCurrentVertexArray(const VertexArray *va) const { return va == mVertexArray; }

    // Helpers for setting bound buffers. They should all have the same signature.
    // Not meant to be called externally. Used for local helpers in State.cpp.
    template <BufferBinding Target>
    void setGenericBufferBindingWithBit(const Context *context, Buffer *buffer);

    template <BufferBinding Target>
    void setGenericBufferBinding(const Context *context, Buffer *buffer);

    using BufferBindingSetter = void (State::*)(const Context *, Buffer *);

    ANGLE_INLINE bool validateSamplerFormats() const
    {
        return (!mExecutable || !(mTexturesIncompatibleWithSamplers.intersects(
                                    mExecutable->getActiveSamplersMask())));
    }

    ANGLE_INLINE void setReadFramebufferBindingDirty()
    {
        mDirtyBits.set(state::DIRTY_BIT_READ_FRAMEBUFFER_BINDING);
    }

    ANGLE_INLINE void setDrawFramebufferBindingDirty()
    {
        mDirtyBits.set(state::DIRTY_BIT_DRAW_FRAMEBUFFER_BINDING);
    }

    const OverlayType *getOverlay() const { return mOverlay; }

    // Not for general use.
    const BufferManager &getBufferManagerForCapture() const { return *mBufferManager; }
    const BoundBufferMap &getBoundBuffersForCapture() const { return mBoundBuffers; }
    const TextureManager &getTextureManagerForCapture() const { return *mTextureManager; }
    const TextureBindingMap &getBoundTexturesForCapture() const { return mSamplerTextures; }
    const RenderbufferManager &getRenderbufferManagerForCapture() const
    {
        return *mRenderbufferManager;
    }
    const FramebufferManager &getFramebufferManagerForCapture() const
    {
        return *mFramebufferManager;
    }
    const ShaderProgramManager &getShaderProgramManagerForCapture() const
    {
        return *mShaderProgramManager;
    }
    const SyncManager &getSyncManagerForCapture() const { return *mSyncManager; }
    const SamplerManager &getSamplerManagerForCapture() const { return *mSamplerManager; }
    const ProgramPipelineManager *getProgramPipelineManagerForCapture() const
    {
        return mProgramPipelineManager;
    }
    const SamplerBindingVector &getSamplerBindingsForCapture() const { return mSamplers; }
    const ActiveQueryMap &getActiveQueriesForCapture() const { return mActiveQueries; }
    void initializeForCapture(const Context *context);

    bool hasConstantAlphaBlendFunc() const
    {
        return (getBlendFuncConstantAlphaDrawBuffers() & getBlendStateExt().getEnabledMask()).any();
    }

    bool hasSimultaneousConstantColorAndAlphaBlendFunc() const
    {
        return (getBlendFuncConstantColorDrawBuffers() & getBlendStateExt().getEnabledMask())
                   .any() &&
               hasConstantAlphaBlendFunc();
    }

    const BufferVector &getOffsetBindingPointerUniformBuffers() const { return mUniformBuffers; }

    const BufferVector &getOffsetBindingPointerAtomicCounterBuffers() const
    {
        return mAtomicCounterBuffers;
    }

    const BufferVector &getOffsetBindingPointerShaderStorageBuffers() const
    {
        return mShaderStorageBuffers;
    }

    ActiveTextureMask getTexturesIncompatibleWithSamplers() const
    {
        return mTexturesIncompatibleWithSamplers;
    }

    const std::vector<ImageUnit> &getImageUnits() const { return mImageUnits; }

    bool hasDisplayTextureShareGroup() const { return mDisplayTextureShareGroup; }

    // GL_KHR_parallel_shader_compile
    void setMaxShaderCompilerThreads(GLuint count);
    GLuint getMaxShaderCompilerThreads() const { return mMaxShaderCompilerThreads; }

    // Convenience functions that forward to context-private state.
    const RasterizerState &getRasterizerState() const { return mPrivateState.getRasterizerState(); }
    const BlendState &getBlendState() const { return mPrivateState.getBlendState(); }
    const BlendStateExt &getBlendStateExt() const { return mPrivateState.getBlendStateExt(); }
    const DepthStencilState &getDepthStencilState() const
    {
        return mPrivateState.getDepthStencilState();
    }
    const ColorF &getColorClearValue() const { return mPrivateState.getColorClearValue(); }
    float getDepthClearValue() const { return mPrivateState.getDepthClearValue(); }
    int getStencilClearValue() const { return mPrivateState.getStencilClearValue(); }
    bool isRasterizerDiscardEnabled() const { return mPrivateState.isRasterizerDiscardEnabled(); }
    bool isPrimitiveRestartEnabled() const { return mPrivateState.isPrimitiveRestartEnabled(); }
    bool isCullFaceEnabled() const { return mPrivateState.isCullFaceEnabled(); }
    bool isDepthClampEnabled() const { return mPrivateState.isDepthClampEnabled(); }
    bool isDepthTestEnabled() const { return mPrivateState.isDepthTestEnabled(); }
    bool isDepthWriteEnabled() const { return mPrivateState.isDepthWriteEnabled(); }
    float getNearPlane() const { return mPrivateState.getNearPlane(); }
    float getFarPlane() const { return mPrivateState.getFarPlane(); }
    ClipOrigin getClipOrigin() const { return mPrivateState.getClipOrigin(); }
    ClipDepthMode getClipDepthMode() const { return mPrivateState.getClipDepthMode(); }
    bool isClipDepthModeZeroToOne() const { return mPrivateState.isClipDepthModeZeroToOne(); }
    bool isBlendEnabled() const { return mPrivateState.isBlendEnabled(); }
    bool isBlendEnabledIndexed(GLuint index) const
    {
        return mPrivateState.isBlendEnabledIndexed(index);
    }
    DrawBufferMask getBlendEnabledDrawBufferMask() const
    {
        return mPrivateState.getBlendEnabledDrawBufferMask();
    }
    const ColorF &getBlendColor() const { return mPrivateState.getBlendColor(); }
    bool isStencilTestEnabled() const { return mPrivateState.isStencilTestEnabled(); }
    bool isBlendAdvancedCoherentEnabled() const
    {
        return mPrivateState.isBlendAdvancedCoherentEnabled();
    }
    bool isStencilWriteEnabled(GLuint framebufferStencilSize) const
    {
        return mPrivateState.isStencilWriteEnabled(framebufferStencilSize);
    }
    GLint getStencilRef() const { return mPrivateState.getStencilRef(); }
    GLint getStencilBackRef() const { return mPrivateState.getStencilBackRef(); }
    PolygonMode getPolygonMode() const { return mPrivateState.getPolygonMode(); }
    bool isPolygonOffsetPointEnabled() const { return mPrivateState.isPolygonOffsetPointEnabled(); }
    bool isPolygonOffsetLineEnabled() const { return mPrivateState.isPolygonOffsetLineEnabled(); }
    bool isPolygonOffsetFillEnabled() const { return mPrivateState.isPolygonOffsetFillEnabled(); }
    bool isPolygonOffsetEnabled() const { return mPrivateState.isPolygonOffsetEnabled(); }
    bool isSampleAlphaToCoverageEnabled() const
    {
        return mPrivateState.isSampleAlphaToCoverageEnabled();
    }
    bool isSampleCoverageEnabled() const { return mPrivateState.isSampleCoverageEnabled(); }
    GLclampf getSampleCoverageValue() const { return mPrivateState.getSampleCoverageValue(); }
    bool getSampleCoverageInvert() const { return mPrivateState.getSampleCoverageInvert(); }
    bool isSampleMaskEnabled() const { return mPrivateState.isSampleMaskEnabled(); }
    GLbitfield getSampleMaskWord(GLuint maskNumber) const
    {
        return mPrivateState.getSampleMaskWord(maskNumber);
    }
    SampleMaskArray<GLbitfield> getSampleMaskValues() const
    {
        return mPrivateState.getSampleMaskValues();
    }
    GLuint getMaxSampleMaskWords() const { return mPrivateState.getMaxSampleMaskWords(); }
    bool isSampleAlphaToOneEnabled() const { return mPrivateState.isSampleAlphaToOneEnabled(); }
    bool isMultisamplingEnabled() const { return mPrivateState.isMultisamplingEnabled(); }
    bool isSampleShadingEnabled() const { return mPrivateState.isSampleShadingEnabled(); }
    float getMinSampleShading() const { return mPrivateState.getMinSampleShading(); }
    bool isScissorTestEnabled() const { return mPrivateState.isScissorTestEnabled(); }
    const Rectangle &getScissor() const { return mPrivateState.getScissor(); }
    bool isDitherEnabled() const { return mPrivateState.isDitherEnabled(); }
    bool isBindGeneratesResourceEnabled() const
    {
        return mPrivateState.isBindGeneratesResourceEnabled();
    }
    bool areClientArraysEnabled() const { return mPrivateState.areClientArraysEnabled(); }
    bool isRobustResourceInitEnabled() const { return mPrivateState.isRobustResourceInitEnabled(); }
    bool isProgramBinaryCacheEnabled() const { return mPrivateState.isProgramBinaryCacheEnabled(); }
    const Rectangle &getViewport() const { return mPrivateState.getViewport(); }
    ShadingRate getShadingRate() const { return mPrivateState.getShadingRate(); }
    GLint getPackAlignment() const { return mPrivateState.getPackAlignment(); }
    bool getPackReverseRowOrder() const { return mPrivateState.getPackReverseRowOrder(); }
    GLint getPackRowLength() const { return mPrivateState.getPackRowLength(); }
    GLint getPackSkipRows() const { return mPrivateState.getPackSkipRows(); }
    GLint getPackSkipPixels() const { return mPrivateState.getPackSkipPixels(); }
    const PixelPackState &getPackState() const { return mPrivateState.getPackState(); }
    PixelPackState &getPackState() { return mPrivateState.getPackState(); }
    GLint getUnpackAlignment() const { return mPrivateState.getUnpackAlignment(); }
    GLint getUnpackRowLength() const { return mPrivateState.getUnpackRowLength(); }
    GLint getUnpackImageHeight() const { return mPrivateState.getUnpackImageHeight(); }
    GLint getUnpackSkipImages() const { return mPrivateState.getUnpackSkipImages(); }
    GLint getUnpackSkipRows() const { return mPrivateState.getUnpackSkipRows(); }
    GLint getUnpackSkipPixels() const { return mPrivateState.getUnpackSkipPixels(); }
    const PixelUnpackState &getUnpackState() const { return mPrivateState.getUnpackState(); }
    PixelUnpackState &getUnpackState() { return mPrivateState.getUnpackState(); }
    GLenum getCoverageModulation() const { return mPrivateState.getCoverageModulation(); }
    bool getFramebufferSRGB() const { return mPrivateState.getFramebufferSRGB(); }
    GLuint getPatchVertices() const { return mPrivateState.getPatchVertices(); }
    void setPixelLocalStorageActivePlanes(GLsizei n)
    {
        mPrivateState.setPixelLocalStorageActivePlanes(n);
    }
    GLsizei getPixelLocalStorageActivePlanes() const
    {
        return mPrivateState.getPixelLocalStorageActivePlanes();
    }
    float getLineWidth() const { return mPrivateState.getLineWidth(); }
    unsigned int getActiveSampler() const { return mPrivateState.getActiveSampler(); }
    GLenum getGenerateMipmapHint() const { return mPrivateState.getGenerateMipmapHint(); }
    GLenum getFragmentShaderDerivativeHint() const
    {
        return mPrivateState.getFragmentShaderDerivativeHint();
    }
    ProvokingVertexConvention getProvokingVertex() const
    {
        return mPrivateState.getProvokingVertex();
    }
    const VertexAttribCurrentValueData &getVertexAttribCurrentValue(size_t attribNum) const
    {
        return mPrivateState.getVertexAttribCurrentValue(attribNum);
    }
    const std::vector<VertexAttribCurrentValueData> &getVertexAttribCurrentValues() const
    {
        return mPrivateState.getVertexAttribCurrentValues();
    }
    AttributesMask getAndResetDirtyCurrentValues() const
    {
        return mPrivateState.getAndResetDirtyCurrentValues();
    }
    ComponentTypeMask getCurrentValuesTypeMask() const
    {
        return mPrivateState.getCurrentValuesTypeMask();
    }
    const ClipDistanceEnableBits &getEnabledClipDistances() const
    {
        return mPrivateState.getEnabledClipDistances();
    }
    bool noSimultaneousConstantColorAndAlphaBlendFunc() const
    {
        return mPrivateState.noSimultaneousConstantColorAndAlphaBlendFunc();
    }
    GLfloat getBoundingBoxMinX() const { return mPrivateState.getBoundingBoxMinX(); }
    GLfloat getBoundingBoxMinY() const { return mPrivateState.getBoundingBoxMinY(); }
    GLfloat getBoundingBoxMinZ() const { return mPrivateState.getBoundingBoxMinZ(); }
    GLfloat getBoundingBoxMinW() const { return mPrivateState.getBoundingBoxMinW(); }
    GLfloat getBoundingBoxMaxX() const { return mPrivateState.getBoundingBoxMaxX(); }
    GLfloat getBoundingBoxMaxY() const { return mPrivateState.getBoundingBoxMaxY(); }
    GLfloat getBoundingBoxMaxZ() const { return mPrivateState.getBoundingBoxMaxZ(); }
    GLfloat getBoundingBoxMaxW() const { return mPrivateState.getBoundingBoxMaxW(); }
    bool isTextureRectangleEnabled() const { return mPrivateState.isTextureRectangleEnabled(); }
    DrawBufferMask getBlendFuncConstantAlphaDrawBuffers() const
    {
        return mPrivateState.getBlendFuncConstantAlphaDrawBuffers();
    }
    DrawBufferMask getBlendFuncConstantColorDrawBuffers() const
    {
        return mPrivateState.getBlendFuncConstantColorDrawBuffers();
    }
    bool isLogicOpEnabled() const { return mPrivateState.isLogicOpEnabled(); }
    LogicalOperation getLogicOp() const { return mPrivateState.getLogicOp(); }
    bool isPerfMonitorActive() const { return mPrivateState.isPerfMonitorActive(); }
    const Debug &getDebug() const { return mPrivateState.getDebug(); }
    Debug &getDebug() { return mPrivateState.getDebug(); }
    const BlobCacheCallbacks &getBlobCacheCallbacks() const
    {
        return mPrivateState.getBlobCacheCallbacks();
    }
    BlobCacheCallbacks &getBlobCacheCallbacks() { return mPrivateState.getBlobCacheCallbacks(); }
    bool getEnableFeature(GLenum feature) const { return mPrivateState.getEnableFeature(feature); }
    bool getEnableFeatureIndexed(GLenum feature, GLuint index) const
    {
        return mPrivateState.getEnableFeatureIndexed(feature, index);
    }
    ProgramUniformBlockMask getAndResetDirtyUniformBlocks() const
    {
        ProgramUniformBlockMask dirtyBits = mDirtyUniformBlocks;
        mDirtyUniformBlocks.reset();
        return dirtyBits;
    }
    const PrivateState &privateState() const { return mPrivateState; }
    const GLES1State &gles1() const { return mPrivateState.gles1(); }

    // Used by the capture/replay tool to create state.
    PrivateState *getMutablePrivateStateForCapture() { return &mPrivateState; }

  private:
    friend class Context;

    // Used only by the entry points to set private state without holding the share group lock.
    PrivateState *getMutablePrivateState() { return &mPrivateState; }
    GLES1State *getMutableGLES1State() { return mPrivateState.getMutableGLES1State(); }

    angle::Result installProgramPipelineExecutableIfNotAlready(const Context *context);
    angle::Result onExecutableChange(const Context *context);

    void unsetActiveTextures(const ActiveTextureMask &textureMask);
    void setActiveTextureDirty(size_t textureIndex, Texture *texture);
    void updateTextureBinding(const Context *context, size_t textureIndex, Texture *texture);
    void updateActiveTextureStateOnSync(const Context *context,
                                        size_t textureIndex,
                                        const Sampler *sampler,
                                        Texture *texture);
    Texture *getTextureForActiveSampler(TextureType type, size_t index);

    // Functions to synchronize dirty states
    angle::Result syncActiveTextures(const Context *context, Command command);
    angle::Result syncTexturesInit(const Context *context, Command command);
    angle::Result syncImagesInit(const Context *context, Command command);
    angle::Result syncReadAttachments(const Context *context, Command command);
    angle::Result syncDrawAttachments(const Context *context, Command command);
    angle::Result syncReadFramebuffer(const Context *context, Command command);
    angle::Result syncDrawFramebuffer(const Context *context, Command command);
    angle::Result syncVertexArray(const Context *context, Command command);
    angle::Result syncTextures(const Context *context, Command command);
    angle::Result syncImages(const Context *context, Command command);
    angle::Result syncSamplers(const Context *context, Command command);
    angle::Result syncProgramPipelineObject(const Context *context, Command command);

    using DirtyObjectHandler = angle::Result (State::*)(const Context *context, Command command);
    using DirtyObjectHandlerArray = std::array<DirtyObjectHandler, state::DIRTY_OBJECT_MAX>;

    static constexpr DirtyObjectHandlerArray MakeDirtyObjectHandlers()
    {
        // Work around C++'s lack of array element support in designated initializers
        // This function cannot be a lambda due to MSVC C++17 limitations b/330910097#comment5
        DirtyObjectHandlerArray handlers{};

        handlers[state::DIRTY_OBJECT_ACTIVE_TEXTURES]         = &State::syncActiveTextures;
        handlers[state::DIRTY_OBJECT_TEXTURES_INIT]           = &State::syncTexturesInit;
        handlers[state::DIRTY_OBJECT_IMAGES_INIT]             = &State::syncImagesInit;
        handlers[state::DIRTY_OBJECT_READ_ATTACHMENTS]        = &State::syncReadAttachments;
        handlers[state::DIRTY_OBJECT_DRAW_ATTACHMENTS]        = &State::syncDrawAttachments;
        handlers[state::DIRTY_OBJECT_READ_FRAMEBUFFER]        = &State::syncReadFramebuffer;
        handlers[state::DIRTY_OBJECT_DRAW_FRAMEBUFFER]        = &State::syncDrawFramebuffer;
        handlers[state::DIRTY_OBJECT_VERTEX_ARRAY]            = &State::syncVertexArray;
        handlers[state::DIRTY_OBJECT_TEXTURES]                = &State::syncTextures;
        handlers[state::DIRTY_OBJECT_IMAGES]                  = &State::syncImages;
        handlers[state::DIRTY_OBJECT_SAMPLERS]                = &State::syncSamplers;
        handlers[state::DIRTY_OBJECT_PROGRAM_PIPELINE_OBJECT] = &State::syncProgramPipelineObject;

        // If a handler is missing, reset everything for ease of static_assert
        for (auto handler : handlers)
        {
            if (handler == nullptr)
            {
                return DirtyObjectHandlerArray();
            }
        }

        return handlers;
    }

    angle::Result dirtyObjectHandler(size_t dirtyObject, const Context *context, Command command)
    {
        static constexpr DirtyObjectHandlerArray handlers = MakeDirtyObjectHandlers();
        static_assert(handlers[0] != nullptr, "MakeDirtyObjectHandlers missing a handler");

        return (this->*handlers[dirtyObject])(context, command);
    }

    // Robust init must happen before Framebuffer init for the Vulkan back-end.
    static_assert(state::DIRTY_OBJECT_ACTIVE_TEXTURES < state::DIRTY_OBJECT_TEXTURES_INIT,
                  "init order");
    static_assert(state::DIRTY_OBJECT_TEXTURES_INIT < state::DIRTY_OBJECT_DRAW_FRAMEBUFFER,
                  "init order");
    static_assert(state::DIRTY_OBJECT_IMAGES_INIT < state::DIRTY_OBJECT_DRAW_FRAMEBUFFER,
                  "init order");
    static_assert(state::DIRTY_OBJECT_DRAW_ATTACHMENTS < state::DIRTY_OBJECT_DRAW_FRAMEBUFFER,
                  "init order");
    static_assert(state::DIRTY_OBJECT_READ_ATTACHMENTS < state::DIRTY_OBJECT_READ_FRAMEBUFFER,
                  "init order");

    // Dispatch table for buffer update functions.
    static const angle::PackedEnumMap<BufferBinding, BufferBindingSetter> kBufferSetters;

    ContextID mID;

    EGLenum mContextPriority;
    bool mHasRobustAccess;
    bool mHasProtectedContent;
    bool mIsDebugContext;

    egl::ShareGroup *mShareGroup;
    mutable egl::ContextMutex mContextMutex;

    // Resource managers.
    BufferManager *mBufferManager;
    ShaderProgramManager *mShaderProgramManager;
    TextureManager *mTextureManager;
    RenderbufferManager *mRenderbufferManager;
    SamplerManager *mSamplerManager;
    SyncManager *mSyncManager;
    FramebufferManager *mFramebufferManager;
    ProgramPipelineManager *mProgramPipelineManager;
    MemoryObjectManager *mMemoryObjectManager;
    SemaphoreManager *mSemaphoreManager;

    Framebuffer *mReadFramebuffer;
    Framebuffer *mDrawFramebuffer;
    BindingPointer<Renderbuffer> mRenderbuffer;
    Program *mProgram;
    BindingPointer<ProgramPipeline> mProgramPipeline;
    // The _installed_ executable.  Note that this may be different from the program's (or the
    // program pipeline's) executable, as they may have been unsuccessfully relinked.
    SharedProgramExecutable mExecutable;

    VertexArray *mVertexArray;

    TextureBindingMap mSamplerTextures;

    // Active Textures Cache
    // ---------------------
    // The active textures cache gives ANGLE components access to a complete array of textures
    // on a draw call. gl::State implements angle::Observer and watches gl::Texture for state
    // changes via the onSubjectStateChange method above. We update the cache before draws.
    // See Observer.h and the design doc linked there for more info on Subject/Observer events.
    //
    // On state change events (re-binding textures, samplers, programs etc) we clear the cache
    // and flag dirty bits. nullptr indicates unbound or incomplete.
    ActiveTexturesCache mActiveTexturesCache;
    std::vector<angle::ObserverBinding> mCompleteTextureBindings;

    ActiveTextureMask mTexturesIncompatibleWithSamplers;

    SamplerBindingVector mSamplers;

    // It would be nice to merge the image and observer binding. Same for textures.
    std::vector<ImageUnit> mImageUnits;

    ActiveQueryMap mActiveQueries;

    // Stores the currently bound buffer for each binding point. It has an entry for the element
    // array buffer but it should not be used. Instead this bind point is owned by the current
    // vertex array object.
    BoundBufferMap mBoundBuffers;

    BufferVector mUniformBuffers;
    BufferVector mAtomicCounterBuffers;
    BufferVector mShaderStorageBuffers;

    angle::BitSet<gl::IMPLEMENTATION_MAX_UNIFORM_BUFFER_BINDINGS> mBoundUniformBuffersMask;
    angle::BitSet<gl::IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS>
        mBoundAtomicCounterBuffersMask;
    angle::BitSet<gl::IMPLEMENTATION_MAX_SHADER_STORAGE_BUFFER_BINDINGS>
        mBoundShaderStorageBuffersMask;

    BindingPointer<TransformFeedback> mTransformFeedback;

    bool mDisplayTextureShareGroup;

    // GL_KHR_parallel_shader_compile
    GLuint mMaxShaderCompilerThreads;

    // The Overlay object, used by the backend to render the overlay.
    const OverlayType *mOverlay;

    state::DirtyBits mDirtyBits;
    state::ExtendedDirtyBits mExtendedDirtyBits;
    state::DirtyObjects mDirtyObjects;
    ActiveTextureMask mDirtyActiveTextures;
    ActiveTextureMask mDirtyTextures;
    ActiveTextureMask mDirtySamplers;
    ImageUnitMask mDirtyImages;
    // Tracks uniform blocks that need reprocessing, for example because their mapped bindings have
    // changed, or buffers in their mapped bindings have changed.  This is in State because every
    // context needs to react to such changes.
    mutable ProgramUniformBlockMask mDirtyUniformBlocks;

    PrivateState mPrivateState;
};

ANGLE_INLINE angle::Result State::syncDirtyObjects(const Context *context,
                                                   const state::DirtyObjects &bitset,
                                                   Command command)
{
    // Accumulate any dirty objects that might have been set due to context-private state changes.
    mDirtyObjects |= mPrivateState.getDirtyObjects();
    mPrivateState.clearDirtyObjects();

    const state::DirtyObjects &dirtyObjects = mDirtyObjects & bitset;

    for (size_t dirtyObject : dirtyObjects)
    {
        ANGLE_TRY(dirtyObjectHandler(dirtyObject, context, command));
    }

    mDirtyObjects &= ~dirtyObjects;
    return angle::Result::Continue;
}

}  // namespace gl

#endif  // LIBANGLE_STATE_H_
