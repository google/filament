//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// angletypes.h : Defines a variety of structures and enum types that are used throughout libGLESv2

#include "libANGLE/angletypes.h"
#include "libANGLE/Program.h"
#include "libANGLE/State.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/VertexAttribute.h"

#include <limits>

#define USE_SYSTEM_ZLIB
#include "compression_utils_portable.h"

namespace gl
{
namespace
{
bool IsStencilWriteMaskedOut(GLuint stencilWritemask, GLuint framebufferStencilSize)
{
    const GLuint framebufferMask = angle::BitMask<GLuint>(framebufferStencilSize);
    return (stencilWritemask & framebufferMask) == 0;
}

bool IsStencilNoOp(GLenum stencilFunc,
                   GLenum stencilFail,
                   GLenum stencilPassDepthFail,
                   GLenum stencilPassDepthPass)
{
    const bool isNeverAndKeep           = stencilFunc == GL_NEVER && stencilFail == GL_KEEP;
    const bool isAlwaysAndKeepOrAllKeep = (stencilFunc == GL_ALWAYS || stencilFail == GL_KEEP) &&
                                          stencilPassDepthFail == GL_KEEP &&
                                          stencilPassDepthPass == GL_KEEP;

    return isNeverAndKeep || isAlwaysAndKeepOrAllKeep;
}

// Calculate whether the range [outsideLow, outsideHigh] encloses the range [insideLow, insideHigh]
bool EnclosesRange(int outsideLow, int outsideHigh, int insideLow, int insideHigh)
{
    return outsideLow <= insideLow && outsideHigh >= insideHigh;
}

bool IsAdvancedBlendEquation(gl::BlendEquationType blendEquation)
{
    return blendEquation >= gl::BlendEquationType::Multiply &&
           blendEquation <= gl::BlendEquationType::HslLuminosity;
}

bool IsExtendedBlendFactor(gl::BlendFactorType blendFactor)
{
    return blendFactor >= gl::BlendFactorType::Src1Alpha &&
           blendFactor <= gl::BlendFactorType::OneMinusSrc1Alpha;
}
}  // anonymous namespace

RasterizerState::RasterizerState()
{
    memset(this, 0, sizeof(RasterizerState));

    cullFace            = false;
    cullMode            = CullFaceMode::Back;
    frontFace           = GL_CCW;
    polygonMode         = PolygonMode::Fill;
    polygonOffsetPoint  = false;
    polygonOffsetLine   = false;
    polygonOffsetFill   = false;
    polygonOffsetFactor = 0.0f;
    polygonOffsetUnits  = 0.0f;
    polygonOffsetClamp  = 0.0f;
    depthClamp          = false;
    pointDrawMode       = false;
    multiSample         = false;
    rasterizerDiscard   = false;
    dither              = true;
}

RasterizerState::RasterizerState(const RasterizerState &other)
{
    memcpy(this, &other, sizeof(RasterizerState));
}

RasterizerState &RasterizerState::operator=(const RasterizerState &other)
{
    memcpy(this, &other, sizeof(RasterizerState));
    return *this;
}

bool operator==(const RasterizerState &a, const RasterizerState &b)
{
    return memcmp(&a, &b, sizeof(RasterizerState)) == 0;
}

bool operator!=(const RasterizerState &a, const RasterizerState &b)
{
    return !(a == b);
}

BlendState::BlendState()
{
    memset(this, 0, sizeof(BlendState));

    blend              = false;
    sourceBlendRGB     = GL_ONE;
    sourceBlendAlpha   = GL_ONE;
    destBlendRGB       = GL_ZERO;
    destBlendAlpha     = GL_ZERO;
    blendEquationRGB   = GL_FUNC_ADD;
    blendEquationAlpha = GL_FUNC_ADD;
    colorMaskRed       = true;
    colorMaskGreen     = true;
    colorMaskBlue      = true;
    colorMaskAlpha     = true;
}

BlendState::BlendState(const BlendState &other)
{
    memcpy(this, &other, sizeof(BlendState));
}

bool operator==(const BlendState &a, const BlendState &b)
{
    return memcmp(&a, &b, sizeof(BlendState)) == 0;
}

bool operator!=(const BlendState &a, const BlendState &b)
{
    return !(a == b);
}

DepthStencilState::DepthStencilState()
{
    memset(this, 0, sizeof(DepthStencilState));

    depthTest                = false;
    depthFunc                = GL_LESS;
    depthMask                = true;
    stencilTest              = false;
    stencilFunc              = GL_ALWAYS;
    stencilMask              = static_cast<GLuint>(-1);
    stencilWritemask         = static_cast<GLuint>(-1);
    stencilBackFunc          = GL_ALWAYS;
    stencilBackMask          = static_cast<GLuint>(-1);
    stencilBackWritemask     = static_cast<GLuint>(-1);
    stencilFail              = GL_KEEP;
    stencilPassDepthFail     = GL_KEEP;
    stencilPassDepthPass     = GL_KEEP;
    stencilBackFail          = GL_KEEP;
    stencilBackPassDepthFail = GL_KEEP;
    stencilBackPassDepthPass = GL_KEEP;
}

DepthStencilState::DepthStencilState(const DepthStencilState &other)
{
    memcpy(this, &other, sizeof(DepthStencilState));
}

DepthStencilState &DepthStencilState::operator=(const DepthStencilState &other)
{
    memcpy(this, &other, sizeof(DepthStencilState));
    return *this;
}

bool DepthStencilState::isDepthMaskedOut() const
{
    return !depthMask;
}

bool DepthStencilState::isStencilMaskedOut(GLuint framebufferStencilSize) const
{
    return IsStencilWriteMaskedOut(stencilWritemask, framebufferStencilSize);
}

bool DepthStencilState::isStencilNoOp(GLuint framebufferStencilSize) const
{
    return isStencilMaskedOut(framebufferStencilSize) ||
           IsStencilNoOp(stencilFunc, stencilFail, stencilPassDepthFail, stencilPassDepthPass);
}

bool DepthStencilState::isStencilBackNoOp(GLuint framebufferStencilSize) const
{
    return IsStencilWriteMaskedOut(stencilBackWritemask, framebufferStencilSize) ||
           IsStencilNoOp(stencilBackFunc, stencilBackFail, stencilBackPassDepthFail,
                         stencilBackPassDepthPass);
}

bool operator==(const DepthStencilState &a, const DepthStencilState &b)
{
    return memcmp(&a, &b, sizeof(DepthStencilState)) == 0;
}

bool operator!=(const DepthStencilState &a, const DepthStencilState &b)
{
    return !(a == b);
}

SamplerState::SamplerState()
{
    memset(this, 0, sizeof(SamplerState));

    setMinFilter(GL_NEAREST_MIPMAP_LINEAR);
    setMagFilter(GL_LINEAR);
    setWrapS(GL_REPEAT);
    setWrapT(GL_REPEAT);
    setWrapR(GL_REPEAT);
    setMaxAnisotropy(1.0f);
    setMinLod(-1000.0f);
    setMaxLod(1000.0f);
    setCompareMode(GL_NONE);
    setCompareFunc(GL_LEQUAL);
    setSRGBDecode(GL_DECODE_EXT);
}

SamplerState::SamplerState(const SamplerState &other) = default;

SamplerState &SamplerState::operator=(const SamplerState &other) = default;

// static
SamplerState SamplerState::CreateDefaultForTarget(TextureType type)
{
    SamplerState state;

    // According to OES_EGL_image_external and ARB_texture_rectangle: For external textures, the
    // default min filter is GL_LINEAR and the default s and t wrap modes are GL_CLAMP_TO_EDGE.
    if (type == TextureType::External || type == TextureType::Rectangle)
    {
        state.mMinFilter = GL_LINEAR;
        state.mWrapS     = GL_CLAMP_TO_EDGE;
        state.mWrapT     = GL_CLAMP_TO_EDGE;
    }

    return state;
}

bool SamplerState::setMinFilter(GLenum minFilter)
{
    if (mMinFilter != minFilter)
    {
        mMinFilter                    = minFilter;
        mCompleteness.typed.minFilter = static_cast<uint8_t>(FromGLenum<FilterMode>(minFilter));
        return true;
    }
    return false;
}

bool SamplerState::setMagFilter(GLenum magFilter)
{
    if (mMagFilter != magFilter)
    {
        mMagFilter                    = magFilter;
        mCompleteness.typed.magFilter = static_cast<uint8_t>(FromGLenum<FilterMode>(magFilter));
        return true;
    }
    return false;
}

bool SamplerState::setWrapS(GLenum wrapS)
{
    if (mWrapS != wrapS)
    {
        mWrapS                    = wrapS;
        mCompleteness.typed.wrapS = static_cast<uint8_t>(FromGLenum<WrapMode>(wrapS));
        return true;
    }
    return false;
}

bool SamplerState::setWrapT(GLenum wrapT)
{
    if (mWrapT != wrapT)
    {
        mWrapT = wrapT;
        updateWrapTCompareMode();
        return true;
    }
    return false;
}

bool SamplerState::setWrapR(GLenum wrapR)
{
    if (mWrapR != wrapR)
    {
        mWrapR = wrapR;
        return true;
    }
    return false;
}

bool SamplerState::setMaxAnisotropy(float maxAnisotropy)
{
    if (mMaxAnisotropy != maxAnisotropy)
    {
        mMaxAnisotropy = maxAnisotropy;
        return true;
    }
    return false;
}

bool SamplerState::setMinLod(GLfloat minLod)
{
    if (mMinLod != minLod)
    {
        mMinLod = minLod;
        return true;
    }
    return false;
}

bool SamplerState::setMaxLod(GLfloat maxLod)
{
    if (mMaxLod != maxLod)
    {
        mMaxLod = maxLod;
        return true;
    }
    return false;
}

bool SamplerState::setCompareMode(GLenum compareMode)
{
    if (mCompareMode != compareMode)
    {
        mCompareMode = compareMode;
        updateWrapTCompareMode();
        return true;
    }
    return false;
}

bool SamplerState::setCompareFunc(GLenum compareFunc)
{
    if (mCompareFunc != compareFunc)
    {
        mCompareFunc = compareFunc;
        return true;
    }
    return false;
}

bool SamplerState::setSRGBDecode(GLenum sRGBDecode)
{
    if (mSRGBDecode != sRGBDecode)
    {
        mSRGBDecode = sRGBDecode;
        return true;
    }
    return false;
}

bool SamplerState::setBorderColor(const ColorGeneric &color)
{
    if (mBorderColor != color)
    {
        mBorderColor = color;
        return true;
    }
    return false;
}

void SamplerState::updateWrapTCompareMode()
{
    uint8_t wrap    = static_cast<uint8_t>(FromGLenum<WrapMode>(mWrapT));
    uint8_t compare = static_cast<uint8_t>(mCompareMode == GL_NONE ? 0x10 : 0x00);
    mCompleteness.typed.wrapTCompareMode = wrap | compare;
}

ImageUnit::ImageUnit()
    : texture(), level(0), layered(false), layer(0), access(GL_READ_ONLY), format(GL_R32UI)
{}

ImageUnit::ImageUnit(const ImageUnit &other) = default;

ImageUnit::~ImageUnit() = default;

BlendStateExt::BlendStateExt(const size_t drawBufferCount)
    : mParameterMask(FactorStorage::GetMask(drawBufferCount)),
      mSrcColor(FactorStorage::GetReplicatedValue(BlendFactorType::One, mParameterMask)),
      mDstColor(FactorStorage::GetReplicatedValue(BlendFactorType::Zero, mParameterMask)),
      mSrcAlpha(FactorStorage::GetReplicatedValue(BlendFactorType::One, mParameterMask)),
      mDstAlpha(FactorStorage::GetReplicatedValue(BlendFactorType::Zero, mParameterMask)),
      mEquationColor(EquationStorage::GetReplicatedValue(BlendEquationType::Add, mParameterMask)),
      mEquationAlpha(EquationStorage::GetReplicatedValue(BlendEquationType::Add, mParameterMask)),
      mAllColorMask(
          ColorMaskStorage::GetReplicatedValue(PackColorMask(true, true, true, true),
                                               ColorMaskStorage::GetMask(drawBufferCount))),
      mColorMask(mAllColorMask),
      mAllEnabledMask(0xFF >> (8 - drawBufferCount)),
      mDrawBufferCount(drawBufferCount)
{}

BlendStateExt::BlendStateExt(const BlendStateExt &other) = default;

BlendStateExt &BlendStateExt::operator=(const BlendStateExt &other) = default;

void BlendStateExt::setEnabled(const bool enabled)
{
    mEnabledMask = enabled ? mAllEnabledMask : DrawBufferMask::Zero();
}

void BlendStateExt::setEnabledIndexed(const size_t index, const bool enabled)
{
    ASSERT(index < mDrawBufferCount);
    mEnabledMask.set(index, enabled);
}

BlendStateExt::ColorMaskStorage::Type BlendStateExt::expandColorMaskValue(const bool red,
                                                                          const bool green,
                                                                          const bool blue,
                                                                          const bool alpha) const
{
    return BlendStateExt::ColorMaskStorage::GetReplicatedValue(
        PackColorMask(red, green, blue, alpha), mAllColorMask);
}

BlendStateExt::ColorMaskStorage::Type BlendStateExt::expandColorMaskIndexed(
    const size_t index) const
{
    return ColorMaskStorage::GetReplicatedValue(
        ColorMaskStorage::GetValueIndexed(index, mColorMask), mAllColorMask);
}

void BlendStateExt::setColorMask(const bool red,
                                 const bool green,
                                 const bool blue,
                                 const bool alpha)
{
    mColorMask = expandColorMaskValue(red, green, blue, alpha);
}

void BlendStateExt::setColorMaskIndexed(const size_t index, const uint8_t value)
{
    ASSERT(index < mDrawBufferCount);
    ASSERT(value <= 0xF);
    ColorMaskStorage::SetValueIndexed(index, value, &mColorMask);
}

void BlendStateExt::setColorMaskIndexed(const size_t index,
                                        const bool red,
                                        const bool green,
                                        const bool blue,
                                        const bool alpha)
{
    ASSERT(index < mDrawBufferCount);
    ColorMaskStorage::SetValueIndexed(index, PackColorMask(red, green, blue, alpha), &mColorMask);
}

uint8_t BlendStateExt::getColorMaskIndexed(const size_t index) const
{
    ASSERT(index < mDrawBufferCount);
    return ColorMaskStorage::GetValueIndexed(index, mColorMask);
}

void BlendStateExt::getColorMaskIndexed(const size_t index,
                                        bool *red,
                                        bool *green,
                                        bool *blue,
                                        bool *alpha) const
{
    ASSERT(index < mDrawBufferCount);
    UnpackColorMask(ColorMaskStorage::GetValueIndexed(index, mColorMask), red, green, blue, alpha);
}

DrawBufferMask BlendStateExt::compareColorMask(ColorMaskStorage::Type other) const
{
    return ColorMaskStorage::GetDiffMask(mColorMask, other);
}

BlendStateExt::EquationStorage::Type BlendStateExt::expandEquationValue(const GLenum mode) const
{
    return EquationStorage::GetReplicatedValue(FromGLenum<BlendEquationType>(mode), mParameterMask);
}

BlendStateExt::EquationStorage::Type BlendStateExt::expandEquationValue(
    const gl::BlendEquationType equation) const
{
    return EquationStorage::GetReplicatedValue(equation, mParameterMask);
}

BlendStateExt::EquationStorage::Type BlendStateExt::expandEquationColorIndexed(
    const size_t index) const
{
    return EquationStorage::GetReplicatedValue(
        EquationStorage::GetValueIndexed(index, mEquationColor), mParameterMask);
}

BlendStateExt::EquationStorage::Type BlendStateExt::expandEquationAlphaIndexed(
    const size_t index) const
{
    return EquationStorage::GetReplicatedValue(
        EquationStorage::GetValueIndexed(index, mEquationAlpha), mParameterMask);
}

void BlendStateExt::setEquations(const GLenum modeColor, const GLenum modeAlpha)
{
    const gl::BlendEquationType colorEquation = FromGLenum<BlendEquationType>(modeColor);
    const gl::BlendEquationType alphaEquation = FromGLenum<BlendEquationType>(modeAlpha);

    mEquationColor = expandEquationValue(colorEquation);
    mEquationAlpha = expandEquationValue(alphaEquation);

    // Note that advanced blend equations cannot be independently set for color and alpha, so only
    // the color equation can be checked.
    if (IsAdvancedBlendEquation(colorEquation))
    {
        mUsesAdvancedBlendEquationMask = mAllEnabledMask;
    }
    else
    {
        mUsesAdvancedBlendEquationMask.reset();
    }
}

void BlendStateExt::setEquationsIndexed(const size_t index,
                                        const GLenum modeColor,
                                        const GLenum modeAlpha)
{
    ASSERT(index < mDrawBufferCount);

    const gl::BlendEquationType colorEquation = FromGLenum<BlendEquationType>(modeColor);
    const gl::BlendEquationType alphaEquation = FromGLenum<BlendEquationType>(modeAlpha);

    EquationStorage::SetValueIndexed(index, colorEquation, &mEquationColor);
    EquationStorage::SetValueIndexed(index, alphaEquation, &mEquationAlpha);

    mUsesAdvancedBlendEquationMask.set(index, IsAdvancedBlendEquation(colorEquation));
}

void BlendStateExt::setEquationsIndexed(const size_t index,
                                        const size_t sourceIndex,
                                        const BlendStateExt &source)
{
    ASSERT(index < mDrawBufferCount);
    ASSERT(sourceIndex < source.mDrawBufferCount);

    const gl::BlendEquationType colorEquation =
        EquationStorage::GetValueIndexed(sourceIndex, source.mEquationColor);
    const gl::BlendEquationType alphaEquation =
        EquationStorage::GetValueIndexed(sourceIndex, source.mEquationAlpha);

    EquationStorage::SetValueIndexed(index, colorEquation, &mEquationColor);
    EquationStorage::SetValueIndexed(index, alphaEquation, &mEquationAlpha);

    mUsesAdvancedBlendEquationMask.set(index, IsAdvancedBlendEquation(colorEquation));
}

DrawBufferMask BlendStateExt::compareEquations(const EquationStorage::Type color,
                                               const EquationStorage::Type alpha) const
{
    return EquationStorage::GetDiffMask(mEquationColor, color) |
           EquationStorage::GetDiffMask(mEquationAlpha, alpha);
}

BlendStateExt::FactorStorage::Type BlendStateExt::expandFactorValue(const GLenum func) const
{
    return FactorStorage::GetReplicatedValue(FromGLenum<BlendFactorType>(func), mParameterMask);
}

BlendStateExt::FactorStorage::Type BlendStateExt::expandFactorValue(
    const gl::BlendFactorType func) const
{
    return FactorStorage::GetReplicatedValue(func, mParameterMask);
}

BlendStateExt::FactorStorage::Type BlendStateExt::expandSrcColorIndexed(const size_t index) const
{
    ASSERT(index < mDrawBufferCount);
    return FactorStorage::GetReplicatedValue(FactorStorage::GetValueIndexed(index, mSrcColor),
                                             mParameterMask);
}

BlendStateExt::FactorStorage::Type BlendStateExt::expandDstColorIndexed(const size_t index) const
{
    ASSERT(index < mDrawBufferCount);
    return FactorStorage::GetReplicatedValue(FactorStorage::GetValueIndexed(index, mDstColor),
                                             mParameterMask);
}

BlendStateExt::FactorStorage::Type BlendStateExt::expandSrcAlphaIndexed(const size_t index) const
{
    ASSERT(index < mDrawBufferCount);
    return FactorStorage::GetReplicatedValue(FactorStorage::GetValueIndexed(index, mSrcAlpha),
                                             mParameterMask);
}

BlendStateExt::FactorStorage::Type BlendStateExt::expandDstAlphaIndexed(const size_t index) const
{
    ASSERT(index < mDrawBufferCount);
    return FactorStorage::GetReplicatedValue(FactorStorage::GetValueIndexed(index, mDstAlpha),
                                             mParameterMask);
}

void BlendStateExt::setFactors(const GLenum srcColor,
                               const GLenum dstColor,
                               const GLenum srcAlpha,
                               const GLenum dstAlpha)
{
    const gl::BlendFactorType srcColorFactor = FromGLenum<BlendFactorType>(srcColor);
    const gl::BlendFactorType dstColorFactor = FromGLenum<BlendFactorType>(dstColor);
    const gl::BlendFactorType srcAlphaFactor = FromGLenum<BlendFactorType>(srcAlpha);
    const gl::BlendFactorType dstAlphaFactor = FromGLenum<BlendFactorType>(dstAlpha);

    mSrcColor = expandFactorValue(srcColorFactor);
    mDstColor = expandFactorValue(dstColorFactor);
    mSrcAlpha = expandFactorValue(srcAlphaFactor);
    mDstAlpha = expandFactorValue(dstAlphaFactor);

    if (IsExtendedBlendFactor(srcColorFactor) || IsExtendedBlendFactor(dstColorFactor) ||
        IsExtendedBlendFactor(srcAlphaFactor) || IsExtendedBlendFactor(dstAlphaFactor))
    {
        mUsesExtendedBlendFactorMask = mAllEnabledMask;
    }
    else
    {
        mUsesExtendedBlendFactorMask.reset();
    }
}

void BlendStateExt::setFactorsIndexed(const size_t index,
                                      const gl::BlendFactorType srcColorFactor,
                                      const gl::BlendFactorType dstColorFactor,
                                      const gl::BlendFactorType srcAlphaFactor,
                                      const gl::BlendFactorType dstAlphaFactor)
{
    ASSERT(index < mDrawBufferCount);

    FactorStorage::SetValueIndexed(index, srcColorFactor, &mSrcColor);
    FactorStorage::SetValueIndexed(index, dstColorFactor, &mDstColor);
    FactorStorage::SetValueIndexed(index, srcAlphaFactor, &mSrcAlpha);
    FactorStorage::SetValueIndexed(index, dstAlphaFactor, &mDstAlpha);

    const bool isExtended =
        IsExtendedBlendFactor(srcColorFactor) || IsExtendedBlendFactor(dstColorFactor) ||
        IsExtendedBlendFactor(srcAlphaFactor) || IsExtendedBlendFactor(dstAlphaFactor);
    mUsesExtendedBlendFactorMask.set(index, isExtended);
}

void BlendStateExt::setFactorsIndexed(const size_t index,
                                      const GLenum srcColor,
                                      const GLenum dstColor,
                                      const GLenum srcAlpha,
                                      const GLenum dstAlpha)
{
    const gl::BlendFactorType srcColorFactor = FromGLenum<BlendFactorType>(srcColor);
    const gl::BlendFactorType dstColorFactor = FromGLenum<BlendFactorType>(dstColor);
    const gl::BlendFactorType srcAlphaFactor = FromGLenum<BlendFactorType>(srcAlpha);
    const gl::BlendFactorType dstAlphaFactor = FromGLenum<BlendFactorType>(dstAlpha);

    setFactorsIndexed(index, srcColorFactor, dstColorFactor, srcAlphaFactor, dstAlphaFactor);
}

void BlendStateExt::setFactorsIndexed(const size_t index,
                                      const size_t sourceIndex,
                                      const BlendStateExt &source)
{
    ASSERT(index < mDrawBufferCount);
    ASSERT(sourceIndex < source.mDrawBufferCount);

    const gl::BlendFactorType srcColorFactor =
        FactorStorage::GetValueIndexed(sourceIndex, source.mSrcColor);
    const gl::BlendFactorType dstColorFactor =
        FactorStorage::GetValueIndexed(sourceIndex, source.mDstColor);
    const gl::BlendFactorType srcAlphaFactor =
        FactorStorage::GetValueIndexed(sourceIndex, source.mSrcAlpha);
    const gl::BlendFactorType dstAlphaFactor =
        FactorStorage::GetValueIndexed(sourceIndex, source.mDstAlpha);

    FactorStorage::SetValueIndexed(index, srcColorFactor, &mSrcColor);
    FactorStorage::SetValueIndexed(index, dstColorFactor, &mDstColor);
    FactorStorage::SetValueIndexed(index, srcAlphaFactor, &mSrcAlpha);
    FactorStorage::SetValueIndexed(index, dstAlphaFactor, &mDstAlpha);

    const bool isExtended =
        IsExtendedBlendFactor(srcColorFactor) || IsExtendedBlendFactor(dstColorFactor) ||
        IsExtendedBlendFactor(srcAlphaFactor) || IsExtendedBlendFactor(dstAlphaFactor);
    mUsesExtendedBlendFactorMask.set(index, isExtended);
}

DrawBufferMask BlendStateExt::compareFactors(const FactorStorage::Type srcColor,
                                             const FactorStorage::Type dstColor,
                                             const FactorStorage::Type srcAlpha,
                                             const FactorStorage::Type dstAlpha) const
{
    return FactorStorage::GetDiffMask(mSrcColor, srcColor) |
           FactorStorage::GetDiffMask(mDstColor, dstColor) |
           FactorStorage::GetDiffMask(mSrcAlpha, srcAlpha) |
           FactorStorage::GetDiffMask(mDstAlpha, dstAlpha);
}

static void MinMax(int a, int b, int *minimum, int *maximum)
{
    if (a < b)
    {
        *minimum = a;
        *maximum = b;
    }
    else
    {
        *minimum = b;
        *maximum = a;
    }
}

template <>
bool RectangleImpl<int>::empty() const
{
    return width == 0 && height == 0;
}

template <>
bool RectangleImpl<float>::empty() const
{
    return std::abs(width) < std::numeric_limits<float>::epsilon() &&
           std::abs(height) < std::numeric_limits<float>::epsilon();
}

bool ClipRectangle(const Rectangle &source, const Rectangle &clip, Rectangle *intersection)
{
    angle::CheckedNumeric<int> sourceX2(source.x);
    sourceX2 += source.width;
    if (!sourceX2.IsValid())
    {
        return false;
    }
    angle::CheckedNumeric<int> sourceY2(source.y);
    sourceY2 += source.height;
    if (!sourceY2.IsValid())
    {
        return false;
    }

    int minSourceX, maxSourceX, minSourceY, maxSourceY;
    MinMax(source.x, sourceX2.ValueOrDie(), &minSourceX, &maxSourceX);
    MinMax(source.y, sourceY2.ValueOrDie(), &minSourceY, &maxSourceY);

    angle::CheckedNumeric<int> clipX2(clip.x);
    clipX2 += clip.width;
    if (!clipX2.IsValid())
    {
        return false;
    }
    angle::CheckedNumeric<int> clipY2(clip.y);
    clipY2 += clip.height;
    if (!clipY2.IsValid())
    {
        return false;
    }

    int minClipX, maxClipX, minClipY, maxClipY;
    MinMax(clip.x, clipX2.ValueOrDie(), &minClipX, &maxClipX);
    MinMax(clip.y, clipY2.ValueOrDie(), &minClipY, &maxClipY);

    if (minSourceX >= maxClipX || maxSourceX <= minClipX || minSourceY >= maxClipY ||
        maxSourceY <= minClipY)
    {
        return false;
    }

    int x      = std::max(minSourceX, minClipX);
    int y      = std::max(minSourceY, minClipY);
    int width  = std::min(maxSourceX, maxClipX) - x;
    int height = std::min(maxSourceY, maxClipY) - y;

    if (intersection)
    {
        intersection->x      = x;
        intersection->y      = y;
        intersection->width  = width;
        intersection->height = height;
    }
    return width != 0 && height != 0;
}

void GetEnclosingRectangle(const Rectangle &rect1, const Rectangle &rect2, Rectangle *rectUnion)
{
    // All callers use non-flipped framebuffer-size-clipped rectangles, so both flip and overflow
    // are impossible.
    ASSERT(!rect1.isReversedX() && !rect1.isReversedY());
    ASSERT(!rect2.isReversedX() && !rect2.isReversedY());
    ASSERT((angle::CheckedNumeric<int>(rect1.x) + rect1.width).IsValid());
    ASSERT((angle::CheckedNumeric<int>(rect1.y) + rect1.height).IsValid());
    ASSERT((angle::CheckedNumeric<int>(rect2.x) + rect2.width).IsValid());
    ASSERT((angle::CheckedNumeric<int>(rect2.y) + rect2.height).IsValid());

    // This function calculates a rectangle that covers both input rectangles:
    //
    //                     +---------+
    //          rect1 -->  |         |
    //                     |     +---+-----+
    //                     |     |   |     | <-- rect2
    //                     +-----+---+     |
    //                           |         |
    //                           +---------+
    //
    //   xy0 = min(rect1.xy0, rect2.xy0)
    //                    \
    //                     +---------+-----+
    //          union -->  |         .     |
    //                     |     + . + . . +
    //                     |     .   .     |
    //                     + . . + . +     |
    //                     |     .         |
    //                     +-----+---------+
    //                                    /
    //                         xy1 = max(rect1.xy1, rect2.xy1)

    int x0 = std::min(rect1.x0(), rect2.x0());
    int y0 = std::min(rect1.y0(), rect2.y0());

    int x1 = std::max(rect1.x1(), rect2.x1());
    int y1 = std::max(rect1.y1(), rect2.y1());

    rectUnion->x      = x0;
    rectUnion->y      = y0;
    rectUnion->width  = x1 - x0;
    rectUnion->height = y1 - y0;
}

void ExtendRectangle(const Rectangle &source, const Rectangle &extend, Rectangle *extended)
{
    // All callers use non-flipped framebuffer-size-clipped rectangles, so both flip and overflow
    // are impossible.
    ASSERT(!source.isReversedX() && !source.isReversedY());
    ASSERT(!extend.isReversedX() && !extend.isReversedY());
    ASSERT((angle::CheckedNumeric<int>(source.x) + source.width).IsValid());
    ASSERT((angle::CheckedNumeric<int>(source.y) + source.height).IsValid());
    ASSERT((angle::CheckedNumeric<int>(extend.x) + extend.width).IsValid());
    ASSERT((angle::CheckedNumeric<int>(extend.y) + extend.height).IsValid());

    int x0 = source.x0();
    int x1 = source.x1();
    int y0 = source.y0();
    int y1 = source.y1();

    const int extendX0 = extend.x0();
    const int extendX1 = extend.x1();
    const int extendY0 = extend.y0();
    const int extendY1 = extend.y1();

    // For each side of the rectangle, calculate whether it can be extended by the second rectangle.
    // If so, extend it and continue for the next side with the new dimensions.

    // Left: Reduce x0 if the second rectangle's vertical edge covers the source's:
    //
    //     +--- - - -                +--- - - -
    //     |                         |
    //     |  +--------------+       +-----------------+
    //     |  |    source    |  -->  |       source    |
    //     |  +--------------+       +-----------------+
    //     |                         |
    //     +--- - - -                +--- - - -
    //
    const bool enclosesHeight = EnclosesRange(extendY0, extendY1, y0, y1);
    if (extendX0 < x0 && extendX1 >= x0 && enclosesHeight)
    {
        x0 = extendX0;
    }

    // Right: Increase x1 simiarly.
    if (extendX0 <= x1 && extendX1 > x1 && enclosesHeight)
    {
        x1 = extendX1;
    }

    // Top: Reduce y0 if the second rectangle's horizontal edge covers the source's potentially
    // extended edge.
    const bool enclosesWidth = EnclosesRange(extendX0, extendX1, x0, x1);
    if (extendY0 < y0 && extendY1 >= y0 && enclosesWidth)
    {
        y0 = extendY0;
    }

    // Right: Increase y1 simiarly.
    if (extendY0 <= y1 && extendY1 > y1 && enclosesWidth)
    {
        y1 = extendY1;
    }

    extended->x      = x0;
    extended->y      = y0;
    extended->width  = x1 - x0;
    extended->height = y1 - y0;
}

bool Box::valid() const
{
    return width != 0 && height != 0 && depth != 0;
}

bool Box::operator==(const Box &other) const
{
    return (x == other.x && y == other.y && z == other.z && width == other.width &&
            height == other.height && depth == other.depth);
}

bool Box::operator!=(const Box &other) const
{
    return !(*this == other);
}

Rectangle Box::toRect() const
{
    ASSERT(z == 0 && depth == 1);
    return Rectangle(x, y, width, height);
}

bool Box::coversSameExtent(const Extents &size) const
{
    return x == 0 && y == 0 && z == 0 && width == size.width && height == size.height &&
           depth == size.depth;
}

bool Box::contains(const Box &other) const
{
    return x <= other.x && y <= other.y && z <= other.z && x + width >= other.x + other.width &&
           y + height >= other.y + other.height && z + depth >= other.z + other.depth;
}

size_t Box::volume() const
{
    return width * height * depth;
}

void Box::extend(const Box &other)
{
    // This extends the logic of "ExtendRectangle" to 3 dimensions

    int x0 = x;
    int x1 = x + width;
    int y0 = y;
    int y1 = y + height;
    int z0 = z;
    int z1 = z + depth;

    const int otherx0 = other.x;
    const int otherx1 = other.x + other.width;
    const int othery0 = other.y;
    const int othery1 = other.y + other.height;
    const int otherz0 = other.z;
    const int otherz1 = other.z + other.depth;

    // For each side of the box, calculate whether it can be extended by the other box.
    // If so, extend it and continue to the next side with the new dimensions.

    const bool enclosesWidth  = EnclosesRange(otherx0, otherx1, x0, x1);
    const bool enclosesHeight = EnclosesRange(othery0, othery1, y0, y1);
    const bool enclosesDepth  = EnclosesRange(otherz0, otherz1, z0, z1);

    // Left: Reduce x0 if the other box's Y and Z plane encloses the source
    if (otherx0 < x0 && otherx1 >= x0 && enclosesHeight && enclosesDepth)
    {
        x0 = otherx0;
    }

    // Right: Increase x1 simiarly.
    if (otherx0 <= x1 && otherx1 > x1 && enclosesHeight && enclosesDepth)
    {
        x1 = otherx1;
    }

    // Bottom: Reduce y0 if the other box's X and Z plane encloses the source
    if (othery0 < y0 && othery1 >= y0 && enclosesWidth && enclosesDepth)
    {
        y0 = othery0;
    }

    // Top: Increase y1 simiarly.
    if (othery0 <= y1 && othery1 > y1 && enclosesWidth && enclosesDepth)
    {
        y1 = othery1;
    }

    // Front: Reduce z0 if the other box's X and Y plane encloses the source
    if (otherz0 < z0 && otherz1 >= z0 && enclosesWidth && enclosesHeight)
    {
        z0 = otherz0;
    }

    // Back: Increase z1 simiarly.
    if (otherz0 <= z1 && otherz1 > z1 && enclosesWidth && enclosesHeight)
    {
        z1 = otherz1;
    }

    // Update member var with new dimensions
    x      = x0;
    width  = x1 - x0;
    y      = y0;
    height = y1 - y0;
    z      = z0;
    depth  = z1 - z0;
}

bool ValidateComponentTypeMasks(unsigned long outputTypes,
                                unsigned long inputTypes,
                                unsigned long outputMask,
                                unsigned long inputMask)
{
    static_assert(IMPLEMENTATION_MAX_DRAW_BUFFERS <= kMaxComponentTypeMaskIndex,
                  "Output/input masks should fit into 16 bits - 1 bit per draw buffer. The "
                  "corresponding type masks should fit into 32 bits - 2 bits per draw buffer.");
    static_assert(MAX_VERTEX_ATTRIBS <= kMaxComponentTypeMaskIndex,
                  "Output/input masks should fit into 16 bits - 1 bit per attrib. The "
                  "corresponding type masks should fit into 32 bits - 2 bits per attrib.");

    // For performance reasons, draw buffer and attribute type validation is done using bit masks.
    // We store two bits representing the type split, with the low bit in the lower 16 bits of the
    // variable, and the high bit in the upper 16 bits of the variable. This is done so we can AND
    // with the elswewhere used DrawBufferMask or AttributeMask.

    // OR the masks with themselves, shifted 16 bits. This is to match our split type bits.
    outputMask |= (outputMask << kMaxComponentTypeMaskIndex);
    inputMask |= (inputMask << kMaxComponentTypeMaskIndex);

    // To validate:
    // 1. Remove any indexes that are not enabled in the input (& inputMask)
    // 2. Remove any indexes that exist in output, but not in input (& outputMask)
    // 3. Use == to verify equality
    return (outputTypes & inputMask) == ((inputTypes & outputMask) & inputMask);
}

GLsizeiptr GetBoundBufferAvailableSize(const OffsetBindingPointer<Buffer> &binding)
{
    Buffer *buffer = binding.get();
    if (buffer == nullptr)
    {
        return 0;
    }

    const GLsizeiptr bufferSize = static_cast<GLsizeiptr>(buffer->getSize());

    if (binding.getSize() == 0)
    {
        return bufferSize;
    }

    const GLintptr offset = binding.getOffset();
    const GLsizeiptr size = binding.getSize();

    ASSERT(offset >= 0 && bufferSize >= 0);

    if (bufferSize <= offset)
    {
        return 0;
    }

    return std::min(size, bufferSize - offset);
}

}  // namespace gl
   //
namespace angle
{
bool CompressBlob(const size_t cacheSize, const uint8_t *cacheData, MemoryBuffer *compressedData)
{
    uLong uncompressedSize       = static_cast<uLong>(cacheSize);
    uLong expectedCompressedSize = zlib_internal::GzipExpectedCompressedSize(uncompressedSize);
    uLong actualCompressedSize   = expectedCompressedSize;

    // Clear previous contents and reserve enough memory.
    if (!compressedData->clearAndReserve(expectedCompressedSize))
    {
        ERR() << "Failed to allocate memory for compression";
        return false;
    }

    int zResult = zlib_internal::GzipCompressHelper(compressedData->data(), &actualCompressedSize,
                                                    cacheData, uncompressedSize, nullptr, nullptr);

    if (zResult != Z_OK)
    {
        ERR() << "Failed to compress cache data: " << zResult;
        return false;
    }

    // Trim to actual size.
    ASSERT(actualCompressedSize <= expectedCompressedSize);
    compressedData->setSize(actualCompressedSize);

    return true;
}

bool DecompressBlob(const uint8_t *compressedData,
                    const size_t compressedSize,
                    size_t maxUncompressedDataSize,
                    MemoryBuffer *uncompressedData)
{
    // Call zlib function to decompress.
    uint32_t uncompressedSize =
        zlib_internal::GetGzipUncompressedSize(compressedData, compressedSize);

    if (uncompressedSize == 0)
    {
        ERR() << "Decompressed data size is zero. Wrong or corrupted data? (compressed size is: "
              << compressedSize << ")";
        return false;
    }

    if (uncompressedSize > maxUncompressedDataSize)
    {
        ERR() << "Decompressed data size is larger than the maximum supported (" << uncompressedSize
              << " vs " << maxUncompressedDataSize << ")";
        return false;
    }

    // Clear previous contents and reserve enough memory.
    if (!uncompressedData->clearAndReserve(uncompressedSize))
    {
        ERR() << "Failed to allocate memory for decompression";
        return false;
    }

    uLong destLen = uncompressedSize;
    int zResult   = zlib_internal::GzipUncompressHelper(
        uncompressedData->data(), &destLen, compressedData, static_cast<uLong>(compressedSize));

    if (zResult != Z_OK)
    {
        WARN() << "Failed to decompress data: " << zResult << "\n";
        return false;
    }

    // Trim to actual size.
    ASSERT(destLen <= uncompressedSize);
    uncompressedData->setSize(destLen);

    return true;
}

uint32_t GenerateCRC32(const uint8_t *data, size_t size)
{
    return UpdateCRC32(InitCRC32(), data, size);
}

uint32_t InitCRC32()
{
    // To get required initial value for the crc, need to pass nullptr into buf.
    return static_cast<uint32_t>(crc32_z(0u, nullptr, 0u));
}

uint32_t UpdateCRC32(uint32_t prevCrc32, const uint8_t *data, size_t size)
{
    return static_cast<uint32_t>(crc32_z(static_cast<uLong>(prevCrc32), data, size));
}

UnlockedTailCall::UnlockedTailCall() = default;

UnlockedTailCall::~UnlockedTailCall()
{
    ASSERT(mCalls.empty());
}

void UnlockedTailCall::add(CallType &&call)
{
    mCalls.push_back(std::move(call));
}

void UnlockedTailCall::runImpl(void *resultOut)
{
    if (mCalls.empty())
    {
        return;
    }
    // Clear `mCalls` before calling, because Android sometimes calls back into ANGLE through EGL
    // calls which don't expect there to be any pre-existing tail calls.
    auto calls(std::move(mCalls));
    ASSERT(mCalls.empty());
    for (CallType &call : calls)
    {
        call(resultOut);
    }
}
}  // namespace angle
