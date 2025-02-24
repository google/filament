//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManager9.cpp: Defines a class for caching D3D9 state
#include "libANGLE/renderer/d3d/d3d9/StateManager9.h"

#include "common/bitset_utils.h"
#include "common/utilities.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/d3d9/Framebuffer9.h"
#include "libANGLE/renderer/d3d/d3d9/Renderer9.h"
#include "libANGLE/renderer/d3d/d3d9/renderer9_utils.h"

namespace rx
{

StateManager9::StateManager9(Renderer9 *renderer9)
    : mUsingZeroColorMaskWorkaround(false),
      mCurSampleAlphaToCoverage(false),
      mCurBlendState(),
      mCurBlendColor(0, 0, 0, 0),
      mCurSampleMask(0),
      mCurRasterState(),
      mCurDepthSize(0),
      mCurDepthStencilState(),
      mCurStencilRef(0),
      mCurStencilBackRef(0),
      mCurFrontFaceCCW(0),
      mCurStencilSize(0),
      mCurScissorRect(),
      mCurScissorEnabled(false),
      mCurViewport(),
      mCurNear(0.0f),
      mCurFar(0.0f),
      mCurDepthFront(0.0f),
      mCurIgnoreViewport(false),
      mRenderer9(renderer9),
      mDirtyBits()
{
    mBlendStateDirtyBits.set(DIRTY_BIT_BLEND_ENABLED);
    mBlendStateDirtyBits.set(DIRTY_BIT_BLEND_COLOR);
    mBlendStateDirtyBits.set(DIRTY_BIT_BLEND_FUNCS_EQUATIONS);
    mBlendStateDirtyBits.set(DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE);
    mBlendStateDirtyBits.set(DIRTY_BIT_COLOR_MASK);
    mBlendStateDirtyBits.set(DIRTY_BIT_DITHER);
    mBlendStateDirtyBits.set(DIRTY_BIT_SAMPLE_MASK);

    mRasterizerStateDirtyBits.set(DIRTY_BIT_CULL_MODE);
    mRasterizerStateDirtyBits.set(DIRTY_BIT_DEPTH_BIAS);

    mDepthStencilStateDirtyBits.set(DIRTY_BIT_STENCIL_DEPTH_MASK);
    mDepthStencilStateDirtyBits.set(DIRTY_BIT_STENCIL_DEPTH_FUNC);
    mDepthStencilStateDirtyBits.set(DIRTY_BIT_STENCIL_TEST_ENABLED);
    mDepthStencilStateDirtyBits.set(DIRTY_BIT_STENCIL_FUNCS_FRONT);
    mDepthStencilStateDirtyBits.set(DIRTY_BIT_STENCIL_FUNCS_BACK);
    mDepthStencilStateDirtyBits.set(DIRTY_BIT_STENCIL_WRITEMASK_FRONT);
    mDepthStencilStateDirtyBits.set(DIRTY_BIT_STENCIL_WRITEMASK_BACK);
    mDepthStencilStateDirtyBits.set(DIRTY_BIT_STENCIL_OPS_FRONT);
    mDepthStencilStateDirtyBits.set(DIRTY_BIT_STENCIL_OPS_BACK);

    mScissorStateDirtyBits.set(DIRTY_BIT_SCISSOR_ENABLED);
    mScissorStateDirtyBits.set(DIRTY_BIT_SCISSOR_RECT);
}

StateManager9::~StateManager9() {}

void StateManager9::initialize()
{
    mUsingZeroColorMaskWorkaround = IsAMD(mRenderer9->getVendorId());
}

void StateManager9::forceSetBlendState()
{
    mDirtyBits |= mBlendStateDirtyBits;
}

void StateManager9::forceSetRasterState()
{
    mDirtyBits |= mRasterizerStateDirtyBits;
}

void StateManager9::forceSetDepthStencilState()
{
    mDirtyBits |= mDepthStencilStateDirtyBits;
}

void StateManager9::forceSetScissorState()
{
    mDirtyBits |= mScissorStateDirtyBits;
}

void StateManager9::forceSetViewportState()
{
    mForceSetViewport = true;
}

void StateManager9::forceSetDXUniformsState()
{
    mDxUniformsDirty = true;
}

void StateManager9::updateStencilSizeIfChanged(bool depthStencilInitialized,
                                               unsigned int stencilSize)
{
    if (!depthStencilInitialized || stencilSize != mCurStencilSize)
    {
        mCurStencilSize = stencilSize;
        forceSetDepthStencilState();
    }
}

void StateManager9::syncState(const gl::State &state,
                              const gl::state::DirtyBits &dirtyBits,
                              const gl::state::ExtendedDirtyBits &extendedDirtyBits)
{
    if (!dirtyBits.any())
    {
        return;
    }

    for (auto dirtyBit : dirtyBits)
    {
        switch (dirtyBit)
        {
            case gl::state::DIRTY_BIT_BLEND_ENABLED:
                if (state.getBlendState().blend != mCurBlendState.blend)
                {
                    mDirtyBits.set(DIRTY_BIT_BLEND_ENABLED);
                    // BlendColor and funcs and equations has to be set if blend is enabled
                    mDirtyBits.set(DIRTY_BIT_BLEND_COLOR);
                    mDirtyBits.set(DIRTY_BIT_BLEND_FUNCS_EQUATIONS);

                    // The color mask may have to be updated if the blend state changes
                    if (mUsingZeroColorMaskWorkaround)
                    {
                        mDirtyBits.set(DIRTY_BIT_COLOR_MASK);
                    }
                }
                break;
            case gl::state::DIRTY_BIT_BLEND_FUNCS:
            {
                const gl::BlendState &blendState = state.getBlendState();
                if (blendState.sourceBlendRGB != mCurBlendState.sourceBlendRGB ||
                    blendState.destBlendRGB != mCurBlendState.destBlendRGB ||
                    blendState.sourceBlendAlpha != mCurBlendState.sourceBlendAlpha ||
                    blendState.destBlendAlpha != mCurBlendState.destBlendAlpha)
                {
                    mDirtyBits.set(DIRTY_BIT_BLEND_FUNCS_EQUATIONS);
                    // BlendColor depends on the values of blend funcs
                    mDirtyBits.set(DIRTY_BIT_BLEND_COLOR);

                    // The color mask may have to be updated if the blend funcs change
                    if (mUsingZeroColorMaskWorkaround)
                    {
                        mDirtyBits.set(DIRTY_BIT_COLOR_MASK);
                    }
                }
                break;
            }
            case gl::state::DIRTY_BIT_BLEND_EQUATIONS:
            {
                const gl::BlendState &blendState = state.getBlendState();
                if (blendState.blendEquationRGB != mCurBlendState.blendEquationRGB ||
                    blendState.blendEquationAlpha != mCurBlendState.blendEquationAlpha)
                {
                    mDirtyBits.set(DIRTY_BIT_BLEND_FUNCS_EQUATIONS);

                    // The color mask may have to be updated if the blend funcs change
                    if (mUsingZeroColorMaskWorkaround)
                    {
                        mDirtyBits.set(DIRTY_BIT_COLOR_MASK);
                    }
                }
                break;
            }
            case gl::state::DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE_ENABLED:
                if (state.isSampleAlphaToCoverageEnabled() != mCurSampleAlphaToCoverage)
                {
                    mDirtyBits.set(DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE);
                }
                break;
            case gl::state::DIRTY_BIT_COLOR_MASK:
            {
                const gl::BlendState &blendState = state.getBlendState();
                if (blendState.colorMaskRed != mCurBlendState.colorMaskRed ||
                    blendState.colorMaskGreen != mCurBlendState.colorMaskGreen ||
                    blendState.colorMaskBlue != mCurBlendState.colorMaskBlue ||
                    blendState.colorMaskAlpha != mCurBlendState.colorMaskAlpha)
                {
                    mDirtyBits.set(DIRTY_BIT_COLOR_MASK);

                    // The color mask can cause the blend state to get out of sync when using the
                    // zero color mask workaround
                    if (mUsingZeroColorMaskWorkaround)
                    {
                        mDirtyBits.set(DIRTY_BIT_BLEND_ENABLED);
                        mDirtyBits.set(DIRTY_BIT_BLEND_FUNCS_EQUATIONS);
                    }
                }
                break;
            }
            case gl::state::DIRTY_BIT_DITHER_ENABLED:
                if (state.getRasterizerState().dither != mCurRasterState.dither)
                {
                    mDirtyBits.set(DIRTY_BIT_DITHER);
                }
                break;
            case gl::state::DIRTY_BIT_BLEND_COLOR:
                if (state.getBlendColor() != mCurBlendColor)
                {
                    mDirtyBits.set(DIRTY_BIT_BLEND_COLOR);
                }
                break;
            case gl::state::DIRTY_BIT_CULL_FACE_ENABLED:
                if (state.getRasterizerState().cullFace != mCurRasterState.cullFace)
                {
                    mDirtyBits.set(DIRTY_BIT_CULL_MODE);
                }
                break;
            case gl::state::DIRTY_BIT_CULL_FACE:
                if (state.getRasterizerState().cullMode != mCurRasterState.cullMode)
                {
                    mDirtyBits.set(DIRTY_BIT_CULL_MODE);
                }
                break;
            case gl::state::DIRTY_BIT_FRONT_FACE:
                if (state.getRasterizerState().frontFace != mCurRasterState.frontFace)
                {
                    mDirtyBits.set(DIRTY_BIT_CULL_MODE);

                    // Viewport state depends on rasterizer.frontface
                    mDirtyBits.set(DIRTY_BIT_VIEWPORT);
                }
                break;
            case gl::state::DIRTY_BIT_POLYGON_OFFSET_FILL_ENABLED:
                if (state.getRasterizerState().polygonOffsetFill !=
                    mCurRasterState.polygonOffsetFill)
                {
                    mDirtyBits.set(DIRTY_BIT_DEPTH_BIAS);
                }
                break;
            case gl::state::DIRTY_BIT_POLYGON_OFFSET:
            {
                const gl::RasterizerState &rasterizerState = state.getRasterizerState();
                if (rasterizerState.polygonOffsetFactor != mCurRasterState.polygonOffsetFactor ||
                    rasterizerState.polygonOffsetUnits != mCurRasterState.polygonOffsetUnits)
                {
                    mDirtyBits.set(DIRTY_BIT_DEPTH_BIAS);
                }
                break;
            }
            // Depth and stencil redundant state changes are guarded in the
            // frontend so for related cases here just set the dirty bit.
            case gl::state::DIRTY_BIT_DEPTH_MASK:
                if (state.getDepthStencilState().depthMask != mCurDepthStencilState.depthMask)
                {
                    mDirtyBits.set(DIRTY_BIT_STENCIL_DEPTH_MASK);
                }
                break;
            case gl::state::DIRTY_BIT_DEPTH_TEST_ENABLED:
                mDirtyBits.set(DIRTY_BIT_STENCIL_DEPTH_FUNC);
                break;
            case gl::state::DIRTY_BIT_DEPTH_FUNC:
                mDirtyBits.set(DIRTY_BIT_STENCIL_DEPTH_FUNC);
                break;
            case gl::state::DIRTY_BIT_STENCIL_TEST_ENABLED:
                mDirtyBits.set(DIRTY_BIT_STENCIL_TEST_ENABLED);
                // If we enable the stencil test, all of these must be set
                mDirtyBits.set(DIRTY_BIT_STENCIL_WRITEMASK_BACK);
                mDirtyBits.set(DIRTY_BIT_STENCIL_WRITEMASK_FRONT);
                mDirtyBits.set(DIRTY_BIT_STENCIL_FUNCS_FRONT);
                mDirtyBits.set(DIRTY_BIT_STENCIL_FUNCS_BACK);
                mDirtyBits.set(DIRTY_BIT_STENCIL_OPS_FRONT);
                mDirtyBits.set(DIRTY_BIT_STENCIL_OPS_BACK);
                break;
            case gl::state::DIRTY_BIT_STENCIL_FUNCS_FRONT:
                mDirtyBits.set(DIRTY_BIT_STENCIL_FUNCS_FRONT);
                break;
            case gl::state::DIRTY_BIT_STENCIL_FUNCS_BACK:
                mDirtyBits.set(DIRTY_BIT_STENCIL_FUNCS_BACK);
                break;
            case gl::state::DIRTY_BIT_STENCIL_WRITEMASK_FRONT:
                mDirtyBits.set(DIRTY_BIT_STENCIL_WRITEMASK_FRONT);
                break;
            case gl::state::DIRTY_BIT_STENCIL_WRITEMASK_BACK:
                mDirtyBits.set(DIRTY_BIT_STENCIL_WRITEMASK_BACK);
                break;
            case gl::state::DIRTY_BIT_STENCIL_OPS_FRONT:
                mDirtyBits.set(DIRTY_BIT_STENCIL_OPS_FRONT);
                break;
            case gl::state::DIRTY_BIT_STENCIL_OPS_BACK:
                mDirtyBits.set(DIRTY_BIT_STENCIL_OPS_BACK);
                break;
            case gl::state::DIRTY_BIT_SCISSOR_TEST_ENABLED:
                if (state.isScissorTestEnabled() != mCurScissorEnabled)
                {
                    mDirtyBits.set(DIRTY_BIT_SCISSOR_ENABLED);
                    // If scissor is enabled, we have to set the scissor rect
                    mDirtyBits.set(DIRTY_BIT_SCISSOR_RECT);
                }
                break;
            case gl::state::DIRTY_BIT_SCISSOR:
                if (state.getScissor() != mCurScissorRect)
                {
                    mDirtyBits.set(DIRTY_BIT_SCISSOR_RECT);
                }
                break;
            case gl::state::DIRTY_BIT_DEPTH_RANGE:
                mDirtyBits.set(DIRTY_BIT_VIEWPORT);
                break;
            case gl::state::DIRTY_BIT_VIEWPORT:
                if (state.getViewport() != mCurViewport)
                {
                    mDirtyBits.set(DIRTY_BIT_VIEWPORT);
                }
                break;
            default:
                break;
        }
    }
}

void StateManager9::setBlendDepthRasterStates(const gl::State &glState, unsigned int sampleMask)
{
    const gl::Framebuffer *framebuffer = glState.getDrawFramebuffer();

    const gl::BlendState &blendState       = glState.getBlendState();
    const gl::ColorF &blendColor           = glState.getBlendColor();
    const gl::RasterizerState &rasterState = glState.getRasterizerState();

    const auto &depthStencilState = glState.getDepthStencilState();
    bool frontFaceCCW             = (glState.getRasterizerState().frontFace == GL_CCW);
    unsigned int maxStencil       = (1 << mCurStencilSize) - 1;

    // All the depth stencil states depends on the front face ccw variable
    if (frontFaceCCW != mCurFrontFaceCCW)
    {
        forceSetDepthStencilState();
        mCurFrontFaceCCW = frontFaceCCW;
    }

    for (auto dirtyBit : mDirtyBits)
    {
        switch (dirtyBit)
        {
            case DIRTY_BIT_BLEND_ENABLED:
                setBlendEnabled(blendState.blend);
                break;
            case DIRTY_BIT_BLEND_COLOR:
                setBlendColor(blendState, blendColor);
                break;
            case DIRTY_BIT_BLEND_FUNCS_EQUATIONS:
                setBlendFuncsEquations(blendState);
                break;
            case DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE:
                setSampleAlphaToCoverage(glState.isSampleAlphaToCoverageEnabled());
                break;
            case DIRTY_BIT_COLOR_MASK:
                setColorMask(framebuffer, blendState.colorMaskRed, blendState.colorMaskBlue,
                             blendState.colorMaskGreen, blendState.colorMaskAlpha);
                break;
            case DIRTY_BIT_DITHER:
                setDither(rasterState.dither);
                break;
            case DIRTY_BIT_CULL_MODE:
                setCullMode(rasterState.cullFace, rasterState.cullMode, rasterState.frontFace);
                break;
            case DIRTY_BIT_DEPTH_BIAS:
                setDepthBias(rasterState.polygonOffsetFill, rasterState.polygonOffsetFactor,
                             rasterState.polygonOffsetUnits);
                break;
            case DIRTY_BIT_STENCIL_DEPTH_MASK:
                setDepthMask(depthStencilState.depthMask);
                break;
            case DIRTY_BIT_STENCIL_DEPTH_FUNC:
                setDepthFunc(depthStencilState.depthTest, depthStencilState.depthFunc);
                break;
            case DIRTY_BIT_STENCIL_TEST_ENABLED:
                setStencilTestEnabled(depthStencilState.stencilTest);
                break;
            case DIRTY_BIT_STENCIL_FUNCS_FRONT:
                setStencilFuncsFront(depthStencilState.stencilFunc, depthStencilState.stencilMask,
                                     glState.getStencilRef(), frontFaceCCW, maxStencil);
                break;
            case DIRTY_BIT_STENCIL_FUNCS_BACK:
                setStencilFuncsBack(depthStencilState.stencilBackFunc,
                                    depthStencilState.stencilBackMask, glState.getStencilBackRef(),
                                    frontFaceCCW, maxStencil);
                break;
            case DIRTY_BIT_STENCIL_WRITEMASK_FRONT:
                setStencilWriteMask(depthStencilState.stencilWritemask, frontFaceCCW);
                break;
            case DIRTY_BIT_STENCIL_WRITEMASK_BACK:
                setStencilBackWriteMask(depthStencilState.stencilBackWritemask, frontFaceCCW);
                break;
            case DIRTY_BIT_STENCIL_OPS_FRONT:
                setStencilOpsFront(depthStencilState.stencilFail,
                                   depthStencilState.stencilPassDepthFail,
                                   depthStencilState.stencilPassDepthPass, frontFaceCCW);
                break;
            case DIRTY_BIT_STENCIL_OPS_BACK:
                setStencilOpsBack(depthStencilState.stencilBackFail,
                                  depthStencilState.stencilBackPassDepthFail,
                                  depthStencilState.stencilBackPassDepthPass, frontFaceCCW);
                break;
            default:
                break;
        }
    }

    if (sampleMask != mCurSampleMask)
    {
        setSampleMask(sampleMask);
    }
}

void StateManager9::setViewportState(const gl::Rectangle &viewport,
                                     float zNear,
                                     float zFar,
                                     gl::PrimitiveMode drawMode,
                                     GLenum frontFace,
                                     bool ignoreViewport)
{
    if (!mDirtyBits.test(DIRTY_BIT_VIEWPORT) && mCurIgnoreViewport == ignoreViewport)
        return;

    gl::Rectangle actualViewport = viewport;
    float actualZNear            = gl::clamp01(zNear);
    float actualZFar             = gl::clamp01(zFar);

    if (ignoreViewport)
    {
        actualViewport.x      = 0;
        actualViewport.y      = 0;
        actualViewport.width  = static_cast<int>(mRenderTargetBounds.width);
        actualViewport.height = static_cast<int>(mRenderTargetBounds.height);
        actualZNear           = 0.0f;
        actualZFar            = 1.0f;
    }

    D3DVIEWPORT9 dxViewport;
    dxViewport.X = gl::clamp(actualViewport.x, 0, static_cast<int>(mRenderTargetBounds.width));
    dxViewport.Y = gl::clamp(actualViewport.y, 0, static_cast<int>(mRenderTargetBounds.height));
    dxViewport.Width =
        gl::clamp(actualViewport.width, 0,
                  static_cast<int>(mRenderTargetBounds.width) - static_cast<int>(dxViewport.X));
    dxViewport.Height =
        gl::clamp(actualViewport.height, 0,
                  static_cast<int>(mRenderTargetBounds.height) - static_cast<int>(dxViewport.Y));
    dxViewport.MinZ = actualZNear;
    dxViewport.MaxZ = actualZFar;

    float depthFront = !gl::IsTriangleMode(drawMode) ? 0.0f : (frontFace == GL_CCW ? 1.0f : -1.0f);

    mRenderer9->getDevice()->SetViewport(&dxViewport);

    mCurViewport       = actualViewport;
    mCurNear           = actualZNear;
    mCurFar            = actualZFar;
    mCurDepthFront     = depthFront;
    mCurIgnoreViewport = ignoreViewport;

    // Setting shader constants
    dx_VertexConstants9 vc = {};
    dx_PixelConstants9 pc  = {};

    vc.viewAdjust[0] =
        static_cast<float>((actualViewport.width - static_cast<int>(dxViewport.Width)) +
                           2 * (actualViewport.x - static_cast<int>(dxViewport.X)) - 1) /
        dxViewport.Width;
    vc.viewAdjust[1] =
        static_cast<float>((actualViewport.height - static_cast<int>(dxViewport.Height)) +
                           2 * (actualViewport.y - static_cast<int>(dxViewport.Y)) - 1) /
        dxViewport.Height;
    vc.viewAdjust[2] = static_cast<float>(actualViewport.width) / dxViewport.Width;
    vc.viewAdjust[3] = static_cast<float>(actualViewport.height) / dxViewport.Height;

    pc.viewCoords[0] = actualViewport.width * 0.5f;
    pc.viewCoords[1] = actualViewport.height * 0.5f;
    pc.viewCoords[2] = actualViewport.x + (actualViewport.width * 0.5f);
    pc.viewCoords[3] = actualViewport.y + (actualViewport.height * 0.5f);

    pc.depthFront[0] = (actualZFar - actualZNear) * 0.5f;
    pc.depthFront[1] = (actualZNear + actualZFar) * 0.5f;
    pc.depthFront[2] = depthFront;

    vc.depthRange[0] = actualZNear;
    vc.depthRange[1] = actualZFar;
    vc.depthRange[2] = actualZFar - actualZNear;

    pc.depthRange[0] = actualZNear;
    pc.depthRange[1] = actualZFar;
    pc.depthRange[2] = actualZFar - actualZNear;

    if (memcmp(&vc, &mVertexConstants, sizeof(dx_VertexConstants9)) != 0)
    {
        mVertexConstants = vc;
        mDxUniformsDirty = true;
    }

    if (memcmp(&pc, &mPixelConstants, sizeof(dx_PixelConstants9)) != 0)
    {
        mPixelConstants  = pc;
        mDxUniformsDirty = true;
    }

    mForceSetViewport = false;
}

void StateManager9::setShaderConstants()
{
    if (!mDxUniformsDirty)
        return;

    IDirect3DDevice9 *device = mRenderer9->getDevice();
    device->SetVertexShaderConstantF(0, reinterpret_cast<float *>(&mVertexConstants),
                                     sizeof(dx_VertexConstants9) / sizeof(float[4]));
    device->SetPixelShaderConstantF(0, reinterpret_cast<float *>(&mPixelConstants),
                                    sizeof(dx_PixelConstants9) / sizeof(float[4]));
    mDxUniformsDirty = false;
}

// This is separate from the main state loop because other functions
// outside call only setScissorState to update scissor state
void StateManager9::setScissorState(const gl::Rectangle &scissor, bool enabled)
{
    if (mDirtyBits.test(DIRTY_BIT_SCISSOR_ENABLED))
        setScissorEnabled(enabled);

    if (mDirtyBits.test(DIRTY_BIT_SCISSOR_RECT))
        setScissorRect(scissor, enabled);
}

void StateManager9::setRenderTargetBounds(size_t width, size_t height)
{
    mRenderTargetBounds.width  = (int)width;
    mRenderTargetBounds.height = (int)height;
    forceSetViewportState();
}

void StateManager9::setScissorEnabled(bool scissorEnabled)
{
    mRenderer9->getDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, scissorEnabled ? TRUE : FALSE);
    mCurScissorEnabled = scissorEnabled;
}

void StateManager9::setScissorRect(const gl::Rectangle &scissor, bool enabled)
{
    if (!enabled)
        return;

    RECT rect;
    rect.left = gl::clamp(scissor.x, 0, static_cast<int>(mRenderTargetBounds.width));
    rect.top  = gl::clamp(scissor.y, 0, static_cast<int>(mRenderTargetBounds.height));
    rect.right =
        gl::clamp(scissor.x + scissor.width, 0, static_cast<int>(mRenderTargetBounds.width));
    rect.bottom =
        gl::clamp(scissor.y + scissor.height, 0, static_cast<int>(mRenderTargetBounds.height));
    mRenderer9->getDevice()->SetScissorRect(&rect);
}

void StateManager9::setDepthFunc(bool depthTest, GLenum depthFunc)
{
    if (depthTest)
    {
        IDirect3DDevice9 *device = mRenderer9->getDevice();
        device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
        device->SetRenderState(D3DRS_ZFUNC, gl_d3d9::ConvertComparison(depthFunc));
    }
    else
    {
        mRenderer9->getDevice()->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
    }

    mCurDepthStencilState.depthTest = depthTest;
    mCurDepthStencilState.depthFunc = depthFunc;
}

void StateManager9::setStencilOpsFront(GLenum stencilFail,
                                       GLenum stencilPassDepthFail,
                                       GLenum stencilPassDepthPass,
                                       bool frontFaceCCW)
{
    // TODO(dianx) It may be slightly more efficient todo these and other similar areas
    // with separate dirty bits.
    IDirect3DDevice9 *device = mRenderer9->getDevice();
    device->SetRenderState(frontFaceCCW ? D3DRS_STENCILFAIL : D3DRS_CCW_STENCILFAIL,
                           gl_d3d9::ConvertStencilOp(stencilFail));
    device->SetRenderState(frontFaceCCW ? D3DRS_STENCILZFAIL : D3DRS_CCW_STENCILZFAIL,
                           gl_d3d9::ConvertStencilOp(stencilPassDepthFail));
    device->SetRenderState(frontFaceCCW ? D3DRS_STENCILPASS : D3DRS_CCW_STENCILPASS,
                           gl_d3d9::ConvertStencilOp(stencilPassDepthPass));

    mCurDepthStencilState.stencilFail          = stencilFail;
    mCurDepthStencilState.stencilPassDepthFail = stencilPassDepthFail;
    mCurDepthStencilState.stencilPassDepthPass = stencilPassDepthPass;
}

void StateManager9::setStencilOpsBack(GLenum stencilBackFail,
                                      GLenum stencilBackPassDepthFail,
                                      GLenum stencilBackPassDepthPass,
                                      bool frontFaceCCW)
{
    IDirect3DDevice9 *device = mRenderer9->getDevice();
    device->SetRenderState(!frontFaceCCW ? D3DRS_STENCILFAIL : D3DRS_CCW_STENCILFAIL,
                           gl_d3d9::ConvertStencilOp(stencilBackFail));
    device->SetRenderState(!frontFaceCCW ? D3DRS_STENCILZFAIL : D3DRS_CCW_STENCILZFAIL,
                           gl_d3d9::ConvertStencilOp(stencilBackPassDepthFail));
    device->SetRenderState(!frontFaceCCW ? D3DRS_STENCILPASS : D3DRS_CCW_STENCILPASS,
                           gl_d3d9::ConvertStencilOp(stencilBackPassDepthPass));

    mCurDepthStencilState.stencilBackFail          = stencilBackFail;
    mCurDepthStencilState.stencilBackPassDepthFail = stencilBackPassDepthFail;
    mCurDepthStencilState.stencilBackPassDepthPass = stencilBackPassDepthPass;
}

void StateManager9::setStencilBackWriteMask(GLuint stencilBackWriteMask, bool frontFaceCCW)
{
    mRenderer9->getDevice()->SetRenderState(
        !frontFaceCCW ? D3DRS_STENCILWRITEMASK : D3DRS_CCW_STENCILWRITEMASK, stencilBackWriteMask);

    mCurDepthStencilState.stencilBackWritemask = stencilBackWriteMask;
}

void StateManager9::setStencilFuncsBack(GLenum stencilBackFunc,
                                        GLuint stencilBackMask,
                                        GLint stencilBackRef,
                                        bool frontFaceCCW,
                                        unsigned int maxStencil)
{
    IDirect3DDevice9 *device = mRenderer9->getDevice();
    device->SetRenderState(!frontFaceCCW ? D3DRS_STENCILFUNC : D3DRS_CCW_STENCILFUNC,
                           gl_d3d9::ConvertComparison(stencilBackFunc));
    device->SetRenderState(!frontFaceCCW ? D3DRS_STENCILREF : D3DRS_CCW_STENCILREF,
                           (stencilBackRef < (int)maxStencil) ? stencilBackRef : maxStencil);
    device->SetRenderState(!frontFaceCCW ? D3DRS_STENCILMASK : D3DRS_CCW_STENCILMASK,
                           stencilBackMask);

    mCurDepthStencilState.stencilBackFunc = stencilBackFunc;
    mCurStencilBackRef                    = stencilBackRef;
    mCurDepthStencilState.stencilBackMask = stencilBackMask;
}

void StateManager9::setStencilWriteMask(GLuint stencilWriteMask, bool frontFaceCCW)
{
    mRenderer9->getDevice()->SetRenderState(
        frontFaceCCW ? D3DRS_STENCILWRITEMASK : D3DRS_CCW_STENCILWRITEMASK, stencilWriteMask);
    mCurDepthStencilState.stencilWritemask = stencilWriteMask;
}

void StateManager9::setStencilFuncsFront(GLenum stencilFunc,
                                         GLuint stencilMask,
                                         GLint stencilRef,
                                         bool frontFaceCCW,
                                         unsigned int maxStencil)
{
    IDirect3DDevice9 *device = mRenderer9->getDevice();
    device->SetRenderState(frontFaceCCW ? D3DRS_STENCILFUNC : D3DRS_CCW_STENCILFUNC,
                           gl_d3d9::ConvertComparison(stencilFunc));
    device->SetRenderState(frontFaceCCW ? D3DRS_STENCILREF : D3DRS_CCW_STENCILREF,
                           (stencilRef < static_cast<int>(maxStencil)) ? stencilRef : maxStencil);
    device->SetRenderState(frontFaceCCW ? D3DRS_STENCILMASK : D3DRS_CCW_STENCILMASK, stencilMask);

    mCurDepthStencilState.stencilFunc = stencilFunc;
    mCurStencilRef                    = stencilRef;
    mCurDepthStencilState.stencilMask = stencilMask;
}
void StateManager9::setStencilTestEnabled(bool stencilTestEnabled)
{
    if (stencilTestEnabled && mCurStencilSize > 0)
    {
        mRenderer9->getDevice()->SetRenderState(D3DRS_STENCILENABLE, TRUE);
        mRenderer9->getDevice()->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, TRUE);
    }
    else
    {
        mRenderer9->getDevice()->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    }

    mCurDepthStencilState.stencilTest = stencilTestEnabled;
}

void StateManager9::setDepthMask(bool depthMask)
{
    mRenderer9->getDevice()->SetRenderState(D3DRS_ZWRITEENABLE, depthMask ? TRUE : FALSE);
    mCurDepthStencilState.depthMask = depthMask;
}

// TODO(dianx) one bit for sampleAlphaToCoverage
void StateManager9::setSampleAlphaToCoverage(bool enabled)
{
    if (enabled)
    {
        // D3D9 support for alpha-to-coverage is vendor-specific.
        UNIMPLEMENTED();
    }
}

void StateManager9::setBlendColor(const gl::BlendState &blendState, const gl::ColorF &blendColor)
{
    if (!blendState.blend)
        return;

    if (blendState.sourceBlendRGB != GL_CONSTANT_ALPHA &&
        blendState.sourceBlendRGB != GL_ONE_MINUS_CONSTANT_ALPHA &&
        blendState.destBlendRGB != GL_CONSTANT_ALPHA &&
        blendState.destBlendRGB != GL_ONE_MINUS_CONSTANT_ALPHA)
    {
        mRenderer9->getDevice()->SetRenderState(D3DRS_BLENDFACTOR,
                                                gl_d3d9::ConvertColor(blendColor));
    }
    else
    {
        mRenderer9->getDevice()->SetRenderState(
            D3DRS_BLENDFACTOR,
            D3DCOLOR_RGBA(gl::unorm<8>(blendColor.alpha), gl::unorm<8>(blendColor.alpha),
                          gl::unorm<8>(blendColor.alpha), gl::unorm<8>(blendColor.alpha)));
    }
    mCurBlendColor = blendColor;
}

void StateManager9::setBlendFuncsEquations(const gl::BlendState &blendState)
{
    if (!blendState.blend)
        return;

    IDirect3DDevice9 *device = mRenderer9->getDevice();

    device->SetRenderState(D3DRS_SRCBLEND, gl_d3d9::ConvertBlendFunc(blendState.sourceBlendRGB));
    device->SetRenderState(D3DRS_DESTBLEND, gl_d3d9::ConvertBlendFunc(blendState.destBlendRGB));
    device->SetRenderState(D3DRS_BLENDOP, gl_d3d9::ConvertBlendOp(blendState.blendEquationRGB));

    if (blendState.sourceBlendRGB != blendState.sourceBlendAlpha ||
        blendState.destBlendRGB != blendState.destBlendAlpha ||
        blendState.blendEquationRGB != blendState.blendEquationAlpha)
    {
        device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);

        device->SetRenderState(D3DRS_SRCBLENDALPHA,
                               gl_d3d9::ConvertBlendFunc(blendState.sourceBlendAlpha));
        device->SetRenderState(D3DRS_DESTBLENDALPHA,
                               gl_d3d9::ConvertBlendFunc(blendState.destBlendAlpha));
        device->SetRenderState(D3DRS_BLENDOPALPHA,
                               gl_d3d9::ConvertBlendOp(blendState.blendEquationAlpha));
    }
    else
    {
        device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
    }

    mCurBlendState.sourceBlendRGB     = blendState.sourceBlendRGB;
    mCurBlendState.destBlendRGB       = blendState.destBlendRGB;
    mCurBlendState.blendEquationRGB   = blendState.blendEquationRGB;
    mCurBlendState.blendEquationAlpha = blendState.blendEquationAlpha;
}

void StateManager9::setBlendEnabled(bool enabled)
{
    mRenderer9->getDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, enabled ? TRUE : FALSE);
    mCurBlendState.blend = enabled;
}

void StateManager9::setDither(bool dither)
{
    mRenderer9->getDevice()->SetRenderState(D3DRS_DITHERENABLE, dither ? TRUE : FALSE);
    mCurRasterState.dither = dither;
}

// TODO(dianx) one bit for color mask
void StateManager9::setColorMask(const gl::Framebuffer *framebuffer,
                                 bool red,
                                 bool blue,
                                 bool green,
                                 bool alpha)
{
    // Set the color mask

    const auto *attachment = framebuffer->getFirstColorAttachment();
    const auto &format     = attachment ? attachment->getFormat() : gl::Format::Invalid();

    DWORD colorMask = gl_d3d9::ConvertColorMask(
        format.info->redBits > 0 && red, format.info->greenBits > 0 && green,
        format.info->blueBits > 0 && blue, format.info->alphaBits > 0 && alpha);

    // Apparently some ATI cards have a bug where a draw with a zero color write mask can cause
    // later draws to have incorrect results. Instead, set a nonzero color write mask but modify the
    // blend state so that no drawing is done.
    // http://anglebug.com/42260632
    if (colorMask == 0 && mUsingZeroColorMaskWorkaround)
    {
        IDirect3DDevice9 *device = mRenderer9->getDevice();
        // Enable green channel, but set blending so nothing will be drawn.
        device->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_GREEN);

        device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

        device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
        device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

        mCurBlendState.colorMaskRed   = false;
        mCurBlendState.colorMaskGreen = true;
        mCurBlendState.colorMaskBlue  = false;
        mCurBlendState.colorMaskAlpha = false;

        mCurBlendState.blend              = true;
        mCurBlendState.sourceBlendRGB     = GL_ZERO;
        mCurBlendState.sourceBlendAlpha   = GL_ZERO;
        mCurBlendState.destBlendRGB       = GL_ONE;
        mCurBlendState.destBlendAlpha     = GL_ONE;
        mCurBlendState.blendEquationRGB   = GL_FUNC_ADD;
        mCurBlendState.blendEquationAlpha = GL_FUNC_ADD;
    }
    else
    {
        mRenderer9->getDevice()->SetRenderState(D3DRS_COLORWRITEENABLE, colorMask);

        mCurBlendState.colorMaskRed   = red;
        mCurBlendState.colorMaskGreen = green;
        mCurBlendState.colorMaskBlue  = blue;
        mCurBlendState.colorMaskAlpha = alpha;
    }
}

void StateManager9::setSampleMask(unsigned int sampleMask)
{
    IDirect3DDevice9 *device = mRenderer9->getDevice();
    // Set the multisample mask
    device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
    device->SetRenderState(D3DRS_MULTISAMPLEMASK, static_cast<DWORD>(sampleMask));

    mCurSampleMask = sampleMask;
}

void StateManager9::setCullMode(bool cullFace, gl::CullFaceMode cullMode, GLenum frontFace)
{
    if (cullFace)
    {
        mRenderer9->getDevice()->SetRenderState(D3DRS_CULLMODE,
                                                gl_d3d9::ConvertCullMode(cullMode, frontFace));
    }
    else
    {
        mRenderer9->getDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    }

    mCurRasterState.cullFace  = cullFace;
    mCurRasterState.cullMode  = cullMode;
    mCurRasterState.frontFace = frontFace;
}

void StateManager9::setDepthBias(bool polygonOffsetFill,
                                 GLfloat polygonOffsetFactor,
                                 GLfloat polygonOffsetUnits)
{
    if (polygonOffsetFill)
    {
        if (mCurDepthSize > 0)
        {
            IDirect3DDevice9 *device = mRenderer9->getDevice();
            device->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, *(DWORD *)&polygonOffsetFactor);

            float depthBias = ldexp(polygonOffsetUnits, -static_cast<int>(mCurDepthSize));
            device->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD *)&depthBias);
        }
    }
    else
    {
        IDirect3DDevice9 *device = mRenderer9->getDevice();
        device->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, 0);
        device->SetRenderState(D3DRS_DEPTHBIAS, 0);
    }

    mCurRasterState.polygonOffsetFill   = polygonOffsetFill;
    mCurRasterState.polygonOffsetFactor = polygonOffsetFactor;
    mCurRasterState.polygonOffsetUnits  = polygonOffsetUnits;
}

void StateManager9::updateDepthSizeIfChanged(bool depthStencilInitialized, unsigned int depthSize)
{
    if (!depthStencilInitialized || depthSize != mCurDepthSize)
    {
        mCurDepthSize = depthSize;
        forceSetRasterState();
    }
}
}  // namespace rx
