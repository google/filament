//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManager9.h: Defines a class for caching D3D9 state

#ifndef LIBANGLE_RENDERER_D3D9_STATEMANAGER9_H_
#define LIBANGLE_RENDERER_D3D9_STATEMANAGER9_H_

#include "libANGLE/State.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"

namespace rx
{

class Renderer9;

struct dx_VertexConstants9
{
    float depthRange[4];
    float viewAdjust[4];
    float viewCoords[4];
};

struct dx_PixelConstants9
{
    float depthRange[4];
    float viewCoords[4];
    float depthFront[4];
};

class StateManager9 final : angle::NonCopyable
{
  public:
    StateManager9(Renderer9 *renderer9);
    ~StateManager9();

    void initialize();

    void syncState(const gl::State &state,
                   const gl::state::DirtyBits &dirtyBits,
                   const gl::state::ExtendedDirtyBits &extendedDirtyBits);

    void setBlendDepthRasterStates(const gl::State &glState, unsigned int sampleMask);
    void setScissorState(const gl::Rectangle &scissor, bool enabled);
    void setViewportState(const gl::Rectangle &viewport,
                          float zNear,
                          float zFar,
                          gl::PrimitiveMode drawMode,
                          GLenum frontFace,
                          bool ignoreViewport);

    void setShaderConstants();

    void forceSetBlendState();
    void forceSetRasterState();
    void forceSetDepthStencilState();
    void forceSetScissorState();
    void forceSetViewportState();
    void forceSetDXUniformsState();

    void updateDepthSizeIfChanged(bool depthStencilInitialized, unsigned int depthSize);
    void updateStencilSizeIfChanged(bool depthStencilInitialized, unsigned int stencilSize);

    void setRenderTargetBounds(size_t width, size_t height);

    int getRenderTargetWidth() const { return mRenderTargetBounds.width; }
    int getRenderTargetHeight() const { return mRenderTargetBounds.height; }

    void setAllDirtyBits() { mDirtyBits.set(); }
    void resetDirtyBits() { mDirtyBits.reset(); }

  private:
    // Blend state functions
    void setBlendEnabled(bool enabled);
    void setBlendColor(const gl::BlendState &blendState, const gl::ColorF &blendColor);
    void setBlendFuncsEquations(const gl::BlendState &blendState);
    void setColorMask(const gl::Framebuffer *framebuffer,
                      bool red,
                      bool blue,
                      bool green,
                      bool alpha);
    void setSampleAlphaToCoverage(bool enabled);
    void setDither(bool dither);
    void setSampleMask(unsigned int sampleMask);

    // Current raster state functions
    void setCullMode(bool cullFace, gl::CullFaceMode cullMode, GLenum frontFace);
    void setDepthBias(bool polygonOffsetFill,
                      GLfloat polygonOffsetFactor,
                      GLfloat polygonOffsetUnits);

    // Depth stencil state functions
    void setStencilOpsFront(GLenum stencilFail,
                            GLenum stencilPassDepthFail,
                            GLenum stencilPassDepthPass,
                            bool frontFaceCCW);
    void setStencilOpsBack(GLenum stencilBackFail,
                           GLenum stencilBackPassDepthFail,
                           GLenum stencilBackPassDepthPass,
                           bool frontFaceCCW);
    void setStencilBackWriteMask(GLuint stencilBackWriteMask, bool frontFaceCCW);
    void setDepthFunc(bool depthTest, GLenum depthFunc);
    void setStencilTestEnabled(bool enabled);
    void setDepthMask(bool depthMask);
    void setStencilFuncsFront(GLenum stencilFunc,
                              GLuint stencilMask,
                              GLint stencilRef,
                              bool frontFaceCCW,
                              unsigned int maxStencil);
    void setStencilFuncsBack(GLenum stencilBackFunc,
                             GLuint stencilBackMask,
                             GLint stencilBackRef,
                             bool frontFaceCCW,
                             unsigned int maxStencil);
    void setStencilWriteMask(GLuint stencilWriteMask, bool frontFaceCCW);

    void setScissorEnabled(bool scissorEnabled);
    void setScissorRect(const gl::Rectangle &scissor, bool enabled);

    enum DirtyBitType
    {
        // Blend dirty bits
        DIRTY_BIT_BLEND_ENABLED,
        DIRTY_BIT_BLEND_COLOR,
        DIRTY_BIT_BLEND_FUNCS_EQUATIONS,
        DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE,
        DIRTY_BIT_COLOR_MASK,
        DIRTY_BIT_DITHER,
        DIRTY_BIT_SAMPLE_MASK,

        // Rasterizer dirty bits
        DIRTY_BIT_CULL_MODE,
        DIRTY_BIT_DEPTH_BIAS,

        // Depth stencil dirty bits
        DIRTY_BIT_STENCIL_DEPTH_MASK,
        DIRTY_BIT_STENCIL_DEPTH_FUNC,
        DIRTY_BIT_STENCIL_TEST_ENABLED,
        DIRTY_BIT_STENCIL_FUNCS_FRONT,
        DIRTY_BIT_STENCIL_FUNCS_BACK,
        DIRTY_BIT_STENCIL_WRITEMASK_FRONT,
        DIRTY_BIT_STENCIL_WRITEMASK_BACK,
        DIRTY_BIT_STENCIL_OPS_FRONT,
        DIRTY_BIT_STENCIL_OPS_BACK,

        // Scissor dirty bits
        DIRTY_BIT_SCISSOR_ENABLED,
        DIRTY_BIT_SCISSOR_RECT,

        // Viewport dirty bits
        DIRTY_BIT_VIEWPORT,

        DIRTY_BIT_MAX
    };

    using DirtyBits = angle::BitSet<DIRTY_BIT_MAX>;

    bool mUsingZeroColorMaskWorkaround;

    bool mCurSampleAlphaToCoverage;

    // Currently applied blend state
    gl::BlendState mCurBlendState;
    gl::ColorF mCurBlendColor;
    unsigned int mCurSampleMask;
    DirtyBits mBlendStateDirtyBits;

    // Currently applied raster state
    gl::RasterizerState mCurRasterState;
    unsigned int mCurDepthSize;
    DirtyBits mRasterizerStateDirtyBits;

    // Currently applied depth stencil state
    gl::DepthStencilState mCurDepthStencilState;
    int mCurStencilRef;
    int mCurStencilBackRef;
    bool mCurFrontFaceCCW;
    unsigned int mCurStencilSize;
    DirtyBits mDepthStencilStateDirtyBits;

    // Currently applied scissor states
    gl::Rectangle mCurScissorRect;
    bool mCurScissorEnabled;
    gl::Extents mRenderTargetBounds;
    DirtyBits mScissorStateDirtyBits;

    // Currently applied viewport states
    bool mForceSetViewport;
    gl::Rectangle mCurViewport;
    float mCurNear;
    float mCurFar;
    float mCurDepthFront;
    bool mCurIgnoreViewport;

    dx_VertexConstants9 mVertexConstants;
    dx_PixelConstants9 mPixelConstants;
    bool mDxUniformsDirty;

    // FIXME: Unsupported by D3D9
    static const D3DRENDERSTATETYPE D3DRS_CCW_STENCILREF       = D3DRS_STENCILREF;
    static const D3DRENDERSTATETYPE D3DRS_CCW_STENCILMASK      = D3DRS_STENCILMASK;
    static const D3DRENDERSTATETYPE D3DRS_CCW_STENCILWRITEMASK = D3DRS_STENCILWRITEMASK;

    Renderer9 *mRenderer9;
    DirtyBits mDirtyBits;
};

}  // namespace rx
#endif  // LIBANGLE_RENDERER_D3D9_STATEMANAGER9_H_
