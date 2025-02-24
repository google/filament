//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// queryutils.cpp: Utilities for querying values from GL objects

#include "libANGLE/queryutils.h"

#include <algorithm>

#include "common/utilities.h"

#include "libANGLE/Buffer.h"
#include "libANGLE/Config.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/EGLSync.h"
#include "libANGLE/Fence.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/GLES1State.h"
#include "libANGLE/MemoryObject.h"
#include "libANGLE/Program.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/Sampler.h"
#include "libANGLE/Shader.h"
#include "libANGLE/Surface.h"
#include "libANGLE/Texture.h"
#include "libANGLE/Uniform.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/queryconversions.h"

namespace gl
{

namespace
{

template <bool isPureInteger>
ColorGeneric ConvertToColor(const GLfloat *params)
{
    if (isPureInteger)
    {
        UNREACHABLE();
        return ColorGeneric(ColorI());
    }
    else
    {
        return ColorGeneric(ColorF::fromData(params));
    }
}

template <bool isPureInteger>
ColorGeneric ConvertToColor(const GLint *params)
{
    if (isPureInteger)
    {
        return ColorGeneric(ColorI(params[0], params[1], params[2], params[3]));
    }
    else
    {
        return ColorGeneric(ColorF(normalizedToFloat(params[0]), normalizedToFloat(params[1]),
                                   normalizedToFloat(params[2]), normalizedToFloat(params[3])));
    }
}

template <bool isPureInteger>
ColorGeneric ConvertToColor(const GLuint *params)
{
    if (isPureInteger)
    {
        return ColorGeneric(ColorUI(params[0], params[1], params[2], params[3]));
    }
    else
    {
        UNREACHABLE();
        return ColorGeneric(ColorF());
    }
}

template <bool isPureInteger>
void ConvertFromColor(const ColorGeneric &color, GLfloat *outParams)
{
    if (isPureInteger)
    {
        UNREACHABLE();
    }
    else
    {
        color.colorF.writeData(outParams);
    }
}

template <bool isPureInteger>
void ConvertFromColor(const ColorGeneric &color, GLint *outParams)
{
    if (isPureInteger)
    {
        outParams[0] = color.colorI.red;
        outParams[1] = color.colorI.green;
        outParams[2] = color.colorI.blue;
        outParams[3] = color.colorI.alpha;
    }
    else
    {
        outParams[0] = floatToNormalized<GLint>(color.colorF.red);
        outParams[1] = floatToNormalized<GLint>(color.colorF.green);
        outParams[2] = floatToNormalized<GLint>(color.colorF.blue);
        outParams[3] = floatToNormalized<GLint>(color.colorF.alpha);
    }
}

template <bool isPureInteger>
void ConvertFromColor(const ColorGeneric &color, GLuint *outParams)
{
    if (isPureInteger)
    {
        constexpr unsigned int kMinValue = 0;

        outParams[0] = std::max(color.colorUI.red, kMinValue);
        outParams[1] = std::max(color.colorUI.green, kMinValue);
        outParams[2] = std::max(color.colorUI.blue, kMinValue);
        outParams[3] = std::max(color.colorUI.alpha, kMinValue);
    }
    else
    {
        UNREACHABLE();
    }
}

template <typename ParamType>
void QueryTexLevelParameterBase(const Texture *texture,
                                TextureTarget target,
                                GLint level,
                                GLenum pname,
                                ParamType *params)
{
    ASSERT(texture != nullptr);
    const InternalFormat *info = texture->getTextureState().getImageDesc(target, level).format.info;

    switch (pname)
    {
        case GL_TEXTURE_RED_TYPE:
            *params = CastFromGLintStateValue<ParamType>(
                pname, info->redBits ? info->componentType : GL_NONE);
            break;
        case GL_TEXTURE_GREEN_TYPE:
            *params = CastFromGLintStateValue<ParamType>(
                pname, info->greenBits ? info->componentType : GL_NONE);
            break;
        case GL_TEXTURE_BLUE_TYPE:
            *params = CastFromGLintStateValue<ParamType>(
                pname, info->blueBits ? info->componentType : GL_NONE);
            break;
        case GL_TEXTURE_ALPHA_TYPE:
            *params = CastFromGLintStateValue<ParamType>(
                pname, info->alphaBits ? info->componentType : GL_NONE);
            break;
        case GL_TEXTURE_DEPTH_TYPE:
            *params = CastFromGLintStateValue<ParamType>(
                pname, info->depthBits ? info->componentType : GL_NONE);
            break;
        case GL_TEXTURE_RED_SIZE:
            *params = CastFromGLintStateValue<ParamType>(pname, info->redBits);
            break;
        case GL_TEXTURE_GREEN_SIZE:
            *params = CastFromGLintStateValue<ParamType>(pname, info->greenBits);
            break;
        case GL_TEXTURE_BLUE_SIZE:
            *params = CastFromGLintStateValue<ParamType>(pname, info->blueBits);
            break;
        case GL_TEXTURE_ALPHA_SIZE:
            *params = CastFromGLintStateValue<ParamType>(pname, info->alphaBits);
            break;
        case GL_TEXTURE_DEPTH_SIZE:
            *params = CastFromGLintStateValue<ParamType>(pname, info->depthBits);
            break;
        case GL_TEXTURE_STENCIL_SIZE:
            *params = CastFromGLintStateValue<ParamType>(pname, info->stencilBits);
            break;
        case GL_TEXTURE_SHARED_SIZE:
            *params = CastFromGLintStateValue<ParamType>(pname, info->sharedBits);
            break;
        case GL_TEXTURE_INTERNAL_FORMAT:
            *params = CastFromGLintStateValue<ParamType>(
                pname, info->internalFormat ? info->internalFormat : GL_RGBA);
            break;
        case GL_TEXTURE_WIDTH:
            *params = CastFromGLintStateValue<ParamType>(
                pname, static_cast<uint32_t>(texture->getWidth(target, level)));
            break;
        case GL_TEXTURE_HEIGHT:
            *params = CastFromGLintStateValue<ParamType>(
                pname, static_cast<uint32_t>(texture->getHeight(target, level)));
            break;
        case GL_TEXTURE_DEPTH:
            *params = CastFromGLintStateValue<ParamType>(
                pname, static_cast<uint32_t>(texture->getDepth(target, level)));
            break;
        case GL_TEXTURE_SAMPLES:
            *params = CastFromStateValue<ParamType>(pname, texture->getSamples(target, level));
            break;
        case GL_TEXTURE_FIXED_SAMPLE_LOCATIONS:
            *params = CastFromStateValue<ParamType>(
                pname, static_cast<GLint>(texture->getFixedSampleLocations(target, level)));
            break;
        case GL_TEXTURE_COMPRESSED:
            *params = CastFromStateValue<ParamType>(pname, static_cast<GLint>(info->compressed));
            break;
        case GL_MEMORY_SIZE_ANGLE:
            *params =
                CastFromStateValue<ParamType>(pname, texture->getLevelMemorySize(target, level));
            break;
        case GL_RESOURCE_INITIALIZED_ANGLE:
            *params = CastFromGLintStateValue<ParamType>(
                pname, texture->initState(GL_NONE, ImageIndex::MakeFromTarget(target, level)) ==
                           InitState::Initialized);
            break;
        case GL_TEXTURE_BUFFER_DATA_STORE_BINDING:
            *params = CastFromStateValue<ParamType>(
                pname, static_cast<GLint>(texture->getBuffer().id().value));
            break;
        case GL_TEXTURE_BUFFER_OFFSET:
            *params = CastFromStateValue<ParamType>(
                pname, static_cast<GLint>(texture->getBuffer().getOffset()));
            break;
        case GL_TEXTURE_BUFFER_SIZE:
            *params = CastFromStateValue<ParamType>(
                pname, static_cast<GLint>(GetBoundBufferAvailableSize(texture->getBuffer())));
            break;
        default:
            UNREACHABLE();
            break;
    }
}

// This function is needed to handle fixed_point data.
// It can be used when some pname need special conversion from int/float/bool to fixed_point.
template <bool isGLfixed, typename QueryT, typename ParamType>
QueryT CastFromSpecialValue(GLenum pname, const ParamType param)
{
    if (isGLfixed)
    {
        return static_cast<QueryT>(ConvertFloatToFixed(CastFromStateValue<GLfloat>(pname, param)));
    }
    else
    {
        return CastFromStateValue<QueryT>(pname, param);
    }
}

template <bool isPureInteger, bool isGLfixed, typename ParamType>
void QueryTexParameterBase(const Context *context,
                           const Texture *texture,
                           GLenum pname,
                           ParamType *params)
{
    ASSERT(texture != nullptr);

    switch (pname)
    {
        case GL_TEXTURE_MAG_FILTER:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getMagFilter());
            break;
        case GL_TEXTURE_MIN_FILTER:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getMinFilter());
            break;
        case GL_TEXTURE_WRAP_S:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getWrapS());
            break;
        case GL_TEXTURE_WRAP_T:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getWrapT());
            break;
        case GL_TEXTURE_WRAP_R:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getWrapR());
            break;
        case GL_TEXTURE_IMMUTABLE_FORMAT:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getImmutableFormat());
            break;
        case GL_TEXTURE_IMMUTABLE_LEVELS:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getImmutableLevels());
            break;
        case GL_TEXTURE_USAGE_ANGLE:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getUsage());
            break;
        case GL_TEXTURE_MAX_ANISOTROPY_EXT:
            *params =
                CastFromSpecialValue<isGLfixed, ParamType>(pname, texture->getMaxAnisotropy());
            break;
        case GL_TEXTURE_SWIZZLE_R:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getSwizzleRed());
            break;
        case GL_TEXTURE_SWIZZLE_G:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getSwizzleGreen());
            break;
        case GL_TEXTURE_SWIZZLE_B:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getSwizzleBlue());
            break;
        case GL_TEXTURE_SWIZZLE_A:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getSwizzleAlpha());
            break;
        case GL_TEXTURE_BASE_LEVEL:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getBaseLevel());
            break;
        case GL_TEXTURE_MAX_LEVEL:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getMaxLevel());
            break;
        case GL_TEXTURE_MIN_LOD:
            *params = CastFromSpecialValue<isGLfixed, ParamType>(pname, texture->getMinLod());
            break;
        case GL_TEXTURE_MAX_LOD:
            *params = CastFromSpecialValue<isGLfixed, ParamType>(pname, texture->getMaxLod());
            break;
        case GL_TEXTURE_COMPARE_MODE:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getCompareMode());
            break;
        case GL_TEXTURE_COMPARE_FUNC:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getCompareFunc());
            break;
        case GL_TEXTURE_SRGB_DECODE_EXT:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getSRGBDecode());
            break;
        case GL_TEXTURE_FORMAT_SRGB_OVERRIDE_EXT:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getSRGBOverride());
            break;
        case GL_DEPTH_STENCIL_TEXTURE_MODE:
            *params =
                CastFromGLintStateValue<ParamType>(pname, texture->getDepthStencilTextureMode());
            break;
        case GL_TEXTURE_CROP_RECT_OES:
        {
            const gl::Rectangle &crop = texture->getCrop();
            params[0]                 = CastFromSpecialValue<isGLfixed, ParamType>(pname, crop.x);
            params[1]                 = CastFromSpecialValue<isGLfixed, ParamType>(pname, crop.y);
            params[2] = CastFromSpecialValue<isGLfixed, ParamType>(pname, crop.width);
            params[3] = CastFromSpecialValue<isGLfixed, ParamType>(pname, crop.height);
            break;
        }
        case GL_GENERATE_MIPMAP:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getGenerateMipmapHint());
            break;
        case GL_MEMORY_SIZE_ANGLE:
            *params = CastFromSpecialValue<isGLfixed, ParamType>(pname, texture->getMemorySize());
            break;
        case GL_TEXTURE_BORDER_COLOR:
            ConvertFromColor<isPureInteger>(texture->getBorderColor(), params);
            break;
        case GL_TEXTURE_NATIVE_ID_ANGLE:
            *params = CastFromSpecialValue<isGLfixed, ParamType>(pname, texture->getNativeID());
            break;
        case GL_IMPLEMENTATION_COLOR_READ_FORMAT:
            *params = CastFromGLintStateValue<ParamType>(
                pname, texture->getImplementationColorReadFormat(context));
            break;
        case GL_IMPLEMENTATION_COLOR_READ_TYPE:
            *params = CastFromGLintStateValue<ParamType>(
                pname, texture->getImplementationColorReadType(context));
            break;
        case GL_IMAGE_FORMAT_COMPATIBILITY_TYPE:
            *params =
                CastFromGLintStateValue<ParamType>(pname, GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE);
            break;
        case GL_RESOURCE_INITIALIZED_ANGLE:
            *params = CastFromGLintStateValue<ParamType>(
                pname, texture->initState() == InitState::Initialized);
            break;
        case GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES:
            *params = CastFromGLintStateValue<ParamType>(
                pname, texture->getRequiredTextureImageUnits(context));
            break;
        case GL_TEXTURE_PROTECTED_EXT:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->hasProtectedContent());
            break;
        case GL_TEXTURE_TILING_EXT:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getTilingMode());
            break;
        case GL_TEXTURE_FOVEATED_FEATURE_BITS_QCOM:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getFoveatedFeatureBits());
            break;
        case GL_TEXTURE_FOVEATED_FEATURE_QUERY_QCOM:
            *params =
                CastFromGLintStateValue<ParamType>(pname, texture->getSupportedFoveationFeatures());
            break;
        case GL_TEXTURE_FOVEATED_MIN_PIXEL_DENSITY_QCOM:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getMinPixelDensity());
            break;
        case GL_TEXTURE_FOVEATED_NUM_FOCAL_POINTS_QUERY_QCOM:
            *params = CastFromGLintStateValue<ParamType>(pname, texture->getNumFocalPoints());
            break;
        case GL_SURFACE_COMPRESSION_EXT:
            *params = CastFromGLintStateValue<ParamType>(pname,
                                                         texture->getImageCompressionRate(context));
            break;
        default:
            UNREACHABLE();
            break;
    }
}

// this function is needed to handle OES_FIXED_POINT.
// Some pname values can take in GLfixed values and may need to be converted
template <bool isGLfixed, typename ReturnType, typename ParamType>
ReturnType ConvertTexParam(GLenum pname, const ParamType param)
{
    if (isGLfixed)
    {
        return CastQueryValueTo<ReturnType>(pname,
                                            ConvertFixedToFloat(static_cast<GLfixed>(param)));
    }
    else
    {
        return CastQueryValueTo<ReturnType>(pname, param);
    }
}

template <bool isPureInteger, bool isGLfixed, typename ParamType>
void SetTexParameterBase(Context *context, Texture *texture, GLenum pname, const ParamType *params)
{
    ASSERT(texture != nullptr);

    switch (pname)
    {
        case GL_TEXTURE_WRAP_S:
            texture->setWrapS(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_WRAP_T:
            texture->setWrapT(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_WRAP_R:
            texture->setWrapR(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_MIN_FILTER:
            texture->setMinFilter(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_MAG_FILTER:
            texture->setMagFilter(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_USAGE_ANGLE:
            texture->setUsage(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_MAX_ANISOTROPY_EXT:
            texture->setMaxAnisotropy(context,
                                      ConvertTexParam<isGLfixed, GLfloat>(pname, params[0]));
            break;
        case GL_TEXTURE_COMPARE_MODE:
            texture->setCompareMode(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_COMPARE_FUNC:
            texture->setCompareFunc(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_SWIZZLE_R:
            texture->setSwizzleRed(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_SWIZZLE_G:
            texture->setSwizzleGreen(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_SWIZZLE_B:
            texture->setSwizzleBlue(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_SWIZZLE_A:
            texture->setSwizzleAlpha(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_BASE_LEVEL:
        {
            (void)(texture->setBaseLevel(
                context, clampCast<GLuint>(CastQueryValueTo<GLint>(pname, params[0]))));
            break;
        }
        case GL_TEXTURE_MAX_LEVEL:
            texture->setMaxLevel(context,
                                 clampCast<GLuint>(CastQueryValueTo<GLint>(pname, params[0])));
            break;
        case GL_TEXTURE_MIN_LOD:
            texture->setMinLod(context, CastQueryValueTo<GLfloat>(pname, params[0]));
            break;
        case GL_TEXTURE_MAX_LOD:
            texture->setMaxLod(context, CastQueryValueTo<GLfloat>(pname, params[0]));
            break;
        case GL_DEPTH_STENCIL_TEXTURE_MODE:
            texture->setDepthStencilTextureMode(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_SRGB_DECODE_EXT:
            texture->setSRGBDecode(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_FORMAT_SRGB_OVERRIDE_EXT:
            texture->setSRGBOverride(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_CROP_RECT_OES:
            texture->setCrop(gl::Rectangle(ConvertTexParam<isGLfixed, GLint>(pname, params[0]),
                                           ConvertTexParam<isGLfixed, GLint>(pname, params[1]),
                                           ConvertTexParam<isGLfixed, GLint>(pname, params[2]),
                                           ConvertTexParam<isGLfixed, GLint>(pname, params[3])));
            break;
        case GL_GENERATE_MIPMAP:
            texture->setGenerateMipmapHint(ConvertToGLenum(params[0]));
            break;
        case GL_TEXTURE_BORDER_COLOR:
            texture->setBorderColor(context, ConvertToColor<isPureInteger>(params));
            break;
        case GL_RESOURCE_INITIALIZED_ANGLE:
            texture->setInitState(ConvertToBool(params[0]) ? InitState::Initialized
                                                           : InitState::MayNeedInit);
            break;
        case GL_TEXTURE_PROTECTED_EXT:
            texture->setProtectedContent(context, (params[0] == GL_TRUE));
            break;
        case GL_RENDERABILITY_VALIDATION_ANGLE:
            texture->setRenderabilityValidation(context, (params[0] == GL_TRUE));
            break;
        case GL_TEXTURE_TILING_EXT:
            texture->setTilingMode(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_FOVEATED_FEATURE_BITS_QCOM:
            texture->setFoveatedFeatureBits(ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_FOVEATED_MIN_PIXEL_DENSITY_QCOM:
            texture->setMinPixelDensity(ConvertToGLfloat(params[0]));
            break;
        default:
            UNREACHABLE();
            break;
    }
}

template <bool isPureInteger, typename ParamType>
void QuerySamplerParameterBase(const Sampler *sampler, GLenum pname, ParamType *params)
{
    switch (pname)
    {
        case GL_TEXTURE_MIN_FILTER:
            *params = CastFromGLintStateValue<ParamType>(pname, sampler->getMinFilter());
            break;
        case GL_TEXTURE_MAG_FILTER:
            *params = CastFromGLintStateValue<ParamType>(pname, sampler->getMagFilter());
            break;
        case GL_TEXTURE_WRAP_S:
            *params = CastFromGLintStateValue<ParamType>(pname, sampler->getWrapS());
            break;
        case GL_TEXTURE_WRAP_T:
            *params = CastFromGLintStateValue<ParamType>(pname, sampler->getWrapT());
            break;
        case GL_TEXTURE_WRAP_R:
            *params = CastFromGLintStateValue<ParamType>(pname, sampler->getWrapR());
            break;
        case GL_TEXTURE_MAX_ANISOTROPY_EXT:
            *params = CastFromStateValue<ParamType>(pname, sampler->getMaxAnisotropy());
            break;
        case GL_TEXTURE_MIN_LOD:
            *params = CastFromStateValue<ParamType>(pname, sampler->getMinLod());
            break;
        case GL_TEXTURE_MAX_LOD:
            *params = CastFromStateValue<ParamType>(pname, sampler->getMaxLod());
            break;
        case GL_TEXTURE_COMPARE_MODE:
            *params = CastFromGLintStateValue<ParamType>(pname, sampler->getCompareMode());
            break;
        case GL_TEXTURE_COMPARE_FUNC:
            *params = CastFromGLintStateValue<ParamType>(pname, sampler->getCompareFunc());
            break;
        case GL_TEXTURE_SRGB_DECODE_EXT:
            *params = CastFromGLintStateValue<ParamType>(pname, sampler->getSRGBDecode());
            break;
        case GL_TEXTURE_BORDER_COLOR:
            ConvertFromColor<isPureInteger>(sampler->getBorderColor(), params);
            break;
        default:
            UNREACHABLE();
            break;
    }
}

template <bool isPureInteger, typename ParamType>
void SetSamplerParameterBase(Context *context,
                             Sampler *sampler,
                             GLenum pname,
                             const ParamType *params)
{
    switch (pname)
    {
        case GL_TEXTURE_WRAP_S:
            sampler->setWrapS(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_WRAP_T:
            sampler->setWrapT(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_WRAP_R:
            sampler->setWrapR(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_MIN_FILTER:
            sampler->setMinFilter(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_MAG_FILTER:
            sampler->setMagFilter(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_MAX_ANISOTROPY_EXT:
            sampler->setMaxAnisotropy(context, CastQueryValueTo<GLfloat>(pname, params[0]));
            break;
        case GL_TEXTURE_COMPARE_MODE:
            sampler->setCompareMode(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_COMPARE_FUNC:
            sampler->setCompareFunc(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_MIN_LOD:
            sampler->setMinLod(context, CastQueryValueTo<GLfloat>(pname, params[0]));
            break;
        case GL_TEXTURE_MAX_LOD:
            sampler->setMaxLod(context, CastQueryValueTo<GLfloat>(pname, params[0]));
            break;
        case GL_TEXTURE_SRGB_DECODE_EXT:
            sampler->setSRGBDecode(context, ConvertToGLenum(pname, params[0]));
            break;
        case GL_TEXTURE_BORDER_COLOR:
            sampler->setBorderColor(context, ConvertToColor<isPureInteger>(params));
            break;
        default:
            UNREACHABLE();
            break;
    }

    sampler->onStateChange(angle::SubjectMessage::ContentsChanged);
}

// Warning: you should ensure binding really matches attrib.bindingIndex before using this function.
template <typename ParamType, typename CurrentDataType, size_t CurrentValueCount>
void QueryVertexAttribBase(const VertexAttribute &attrib,
                           const VertexBinding &binding,
                           const CurrentDataType (&currentValueData)[CurrentValueCount],
                           GLenum pname,
                           ParamType *params)
{
    switch (pname)
    {
        case GL_CURRENT_VERTEX_ATTRIB:
            for (size_t i = 0; i < CurrentValueCount; ++i)
            {
                params[i] = CastFromStateValue<ParamType>(pname, currentValueData[i]);
            }
            break;
        case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
            *params = CastFromStateValue<ParamType>(pname, static_cast<GLint>(attrib.enabled));
            break;
        case GL_VERTEX_ATTRIB_ARRAY_SIZE:
            *params = CastFromGLintStateValue<ParamType>(pname, attrib.format->channelCount);
            break;
        case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
            *params = CastFromGLintStateValue<ParamType>(pname, attrib.vertexAttribArrayStride);
            break;
        case GL_VERTEX_ATTRIB_ARRAY_TYPE:
            *params = CastFromGLintStateValue<ParamType>(
                pname, gl::ToGLenum(attrib.format->vertexAttribType));
            break;
        case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
            *params =
                CastFromStateValue<ParamType>(pname, static_cast<GLint>(attrib.format->isNorm()));
            break;
        case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
            *params = CastFromGLintStateValue<ParamType>(pname, binding.getBuffer().id().value);
            break;
        case GL_VERTEX_ATTRIB_ARRAY_DIVISOR:
            *params = CastFromStateValue<ParamType>(pname, binding.getDivisor());
            break;
        case GL_VERTEX_ATTRIB_ARRAY_INTEGER:
            *params = CastFromGLintStateValue<ParamType>(pname, attrib.format->isPureInt());
            break;
        case GL_VERTEX_ATTRIB_BINDING:
            *params = CastFromGLintStateValue<ParamType>(pname, attrib.bindingIndex);
            break;
        case GL_VERTEX_ATTRIB_RELATIVE_OFFSET:
            *params = CastFromGLintStateValue<ParamType>(pname, attrib.relativeOffset);
            break;
        default:
            UNREACHABLE();
            break;
    }
}

template <typename ParamType>
void QueryBufferParameterBase(const Buffer *buffer, GLenum pname, ParamType *params)
{
    ASSERT(buffer != nullptr);

    switch (pname)
    {
        case GL_BUFFER_USAGE:
            *params = CastFromGLintStateValue<ParamType>(pname, ToGLenum(buffer->getUsage()));
            break;
        case GL_BUFFER_SIZE:
            *params = CastFromStateValue<ParamType>(pname, buffer->getSize());
            break;
        case GL_BUFFER_ACCESS_FLAGS:
            *params = CastFromGLintStateValue<ParamType>(pname, buffer->getAccessFlags());
            break;
        case GL_BUFFER_ACCESS_OES:
            *params = CastFromGLintStateValue<ParamType>(pname, buffer->getAccess());
            break;
        case GL_BUFFER_MAPPED:
            *params = CastFromStateValue<ParamType>(pname, buffer->isMapped());
            break;
        case GL_BUFFER_MAP_OFFSET:
            *params = CastFromStateValue<ParamType>(pname, buffer->getMapOffset());
            break;
        case GL_BUFFER_MAP_LENGTH:
            *params = CastFromStateValue<ParamType>(pname, buffer->getMapLength());
            break;
        case GL_MEMORY_SIZE_ANGLE:
            *params = CastFromStateValue<ParamType>(pname, buffer->getMemorySize());
            break;
        case GL_BUFFER_IMMUTABLE_STORAGE_EXT:
            *params = CastFromStateValue<ParamType>(pname, buffer->isImmutable());
            break;
        case GL_BUFFER_STORAGE_FLAGS_EXT:
            *params = CastFromGLintStateValue<ParamType>(pname, buffer->getStorageExtUsageFlags());
            break;
        case GL_RESOURCE_INITIALIZED_ANGLE:
            *params = CastFromStateValue<ParamType>(
                pname, ConvertToGLBoolean(buffer->initState() == InitState::Initialized));
            break;
        default:
            UNREACHABLE();
            break;
    }
}

template <typename T>
GLint GetCommonVariableProperty(const T &var, GLenum prop)
{
    switch (prop)
    {
        case GL_TYPE:
            return clampCast<GLint>(var.pod.type);

        case GL_ARRAY_SIZE:
            // Queryable variables are guaranteed not to be arrays of arrays or arrays of structs,
            // see GLES 3.1 spec section 7.3.1.1 page 77.
            return clampCast<GLint>(var.getBasicTypeElementCount());

        case GL_NAME_LENGTH:
            // ES31 spec p84: This counts the terminating null char.
            return clampCast<GLint>(var.name.size() + 1u);

        default:
            UNREACHABLE();
            return GL_INVALID_VALUE;
    }
}

GLint GetInputResourceProperty(const Program *program, GLuint index, GLenum prop)
{
    const ProgramExecutable &executable = program->getExecutable();
    const ProgramInput &variable        = executable.getInputResource(index);

    switch (prop)
    {
        case GL_TYPE:
            return clampCast<GLint>(variable.getType());
        case GL_ARRAY_SIZE:
            return clampCast<GLint>(variable.getBasicTypeElementCount());

        case GL_NAME_LENGTH:
            return clampCast<GLint>(executable.getInputResourceName(index).size() + 1u);

        case GL_LOCATION:
            return variable.isBuiltIn() ? GL_INVALID_INDEX : variable.getLocation();

        // The query is targeted at the set of active input variables used by the first shader stage
        // of program. If program contains multiple shader stages then input variables from any
        // stage other than the first will not be enumerated. Since we found the variable to get
        // this far, we know it exists in the first attached shader stage.
        case GL_REFERENCED_BY_VERTEX_SHADER:
            return executable.getFirstLinkedShaderStageType() == ShaderType::Vertex;
        case GL_REFERENCED_BY_FRAGMENT_SHADER:
            return executable.getFirstLinkedShaderStageType() == ShaderType::Fragment;
        case GL_REFERENCED_BY_COMPUTE_SHADER:
            return executable.getFirstLinkedShaderStageType() == ShaderType::Compute;
        case GL_REFERENCED_BY_GEOMETRY_SHADER_EXT:
            return executable.getFirstLinkedShaderStageType() == ShaderType::Geometry;
        case GL_REFERENCED_BY_TESS_CONTROL_SHADER_EXT:
            return executable.getFirstLinkedShaderStageType() == ShaderType::TessControl;
        case GL_REFERENCED_BY_TESS_EVALUATION_SHADER_EXT:
            return executable.getFirstLinkedShaderStageType() == ShaderType::TessEvaluation;
        case GL_IS_PER_PATCH_EXT:
            return variable.isPatch();

        default:
            UNREACHABLE();
            return GL_INVALID_VALUE;
    }
}

GLint GetOutputResourceProperty(const Program *program, GLuint index, const GLenum prop)
{
    const ProgramExecutable &executable = program->getExecutable();
    const ProgramOutput &outputVariable = executable.getOutputResource(index);

    switch (prop)
    {
        case GL_TYPE:
            return clampCast<GLint>(outputVariable.pod.type);
        case GL_ARRAY_SIZE:
            return clampCast<GLint>(outputVariable.pod.basicTypeElementCount);

        case GL_NAME_LENGTH:
            return clampCast<GLint>(executable.getOutputResourceName(index).size() + 1u);

        case GL_LOCATION:
            return outputVariable.pod.location;

        case GL_LOCATION_INDEX_EXT:
            // EXT_blend_func_extended
            if (executable.getLastLinkedShaderStageType() == gl::ShaderType::Fragment)
            {
                return executable.getFragDataIndex(outputVariable.name);
            }
            return GL_INVALID_INDEX;

        // The set of active user-defined outputs from the final shader stage in this program. If
        // the final stage is a Fragment Shader, then this represents the fragment outputs that get
        // written to individual color buffers. If the program only contains a Compute Shader, then
        // there are no user-defined outputs.
        case GL_REFERENCED_BY_VERTEX_SHADER:
            return executable.getLastLinkedShaderStageType() == ShaderType::Vertex;
        case GL_REFERENCED_BY_FRAGMENT_SHADER:
            return executable.getLastLinkedShaderStageType() == ShaderType::Fragment;
        case GL_REFERENCED_BY_COMPUTE_SHADER:
            return executable.getLastLinkedShaderStageType() == ShaderType::Compute;
        case GL_REFERENCED_BY_GEOMETRY_SHADER_EXT:
            return executable.getLastLinkedShaderStageType() == ShaderType::Geometry;
        case GL_REFERENCED_BY_TESS_CONTROL_SHADER_EXT:
            return executable.getLastLinkedShaderStageType() == ShaderType::TessControl;
        case GL_REFERENCED_BY_TESS_EVALUATION_SHADER_EXT:
            return executable.getLastLinkedShaderStageType() == ShaderType::TessEvaluation;
        case GL_IS_PER_PATCH_EXT:
            return outputVariable.pod.isPatch;

        default:
            UNREACHABLE();
            return GL_INVALID_VALUE;
    }
}

GLint GetTransformFeedbackVaryingResourceProperty(const Program *program,
                                                  GLuint index,
                                                  const GLenum prop)
{
    const ProgramExecutable &executable = program->getExecutable();
    const TransformFeedbackVarying &tfVariable =
        executable.getTransformFeedbackVaryingResource(index);
    switch (prop)
    {
        case GL_TYPE:
            return clampCast<GLint>(tfVariable.type);

        case GL_ARRAY_SIZE:
            return clampCast<GLint>(tfVariable.size());

        case GL_NAME_LENGTH:
            return clampCast<GLint>(tfVariable.nameWithArrayIndex().size() + 1);

        default:
            UNREACHABLE();
            return GL_INVALID_VALUE;
    }
}

GLint QueryProgramInterfaceActiveResources(const Program *program, GLenum programInterface)
{
    const ProgramExecutable &executable = program->getExecutable();
    switch (programInterface)
    {
        case GL_PROGRAM_INPUT:
            return clampCast<GLint>(executable.getProgramInputs().size());

        case GL_PROGRAM_OUTPUT:
            return clampCast<GLint>(executable.getOutputVariables().size());

        case GL_UNIFORM:
            return clampCast<GLint>(executable.getUniforms().size());

        case GL_UNIFORM_BLOCK:
            return clampCast<GLint>(executable.getUniformBlocks().size());

        case GL_ATOMIC_COUNTER_BUFFER:
            return clampCast<GLint>(executable.getAtomicCounterBuffers().size());

        case GL_BUFFER_VARIABLE:
            return clampCast<GLint>(executable.getBufferVariables().size());

        case GL_SHADER_STORAGE_BLOCK:
            return clampCast<GLint>(executable.getShaderStorageBlocks().size());

        case GL_TRANSFORM_FEEDBACK_VARYING:
            return clampCast<GLint>(executable.getLinkedTransformFeedbackVaryings().size());

        default:
            UNREACHABLE();
            return 0;
    }
}

template <typename T, typename M>
GLint FindMaxSize(const std::vector<T> &resources, M member)
{
    GLint max = 0;
    for (const T &resource : resources)
    {
        max = std::max(max, clampCast<GLint>((resource.*member).size()));
    }
    return max;
}

GLint FindMaxNameLength(const std::vector<std::string> &names)
{
    GLint max = 0;
    for (const std::string &name : names)
    {
        max = std::max(max, clampCast<GLint>(name.size()));
    }
    return max;
}

GLint QueryProgramInterfaceMaxNameLength(const Program *program, GLenum programInterface)
{
    const ProgramExecutable &executable = program->getExecutable();

    GLint maxNameLength = 0;
    switch (programInterface)
    {
        case GL_PROGRAM_INPUT:
            maxNameLength = executable.getInputResourceMaxNameSize();
            break;

        case GL_PROGRAM_OUTPUT:
            maxNameLength = executable.getOutputResourceMaxNameSize();
            break;

        case GL_UNIFORM:
            maxNameLength = FindMaxNameLength(executable.getUniformNames());
            break;

        case GL_UNIFORM_BLOCK:
            return executable.getActiveUniformBlockMaxNameLength();

        case GL_BUFFER_VARIABLE:
            maxNameLength = FindMaxSize(executable.getBufferVariables(), &BufferVariable::name);
            break;

        case GL_SHADER_STORAGE_BLOCK:
            return executable.getActiveShaderStorageBlockMaxNameLength();

        case GL_TRANSFORM_FEEDBACK_VARYING:
            return clampCast<GLint>(executable.getTransformFeedbackVaryingMaxLength());

        default:
            UNREACHABLE();
            return 0;
    }
    // This length includes an extra character for the null terminator.
    return (maxNameLength == 0 ? 0 : maxNameLength + 1);
}

GLint QueryProgramInterfaceMaxNumActiveVariables(const Program *program, GLenum programInterface)
{
    const ProgramExecutable &executable = program->getExecutable();

    switch (programInterface)
    {
        case GL_UNIFORM_BLOCK:
            return FindMaxSize(executable.getUniformBlocks(), &InterfaceBlock::memberIndexes);
        case GL_ATOMIC_COUNTER_BUFFER:
            return FindMaxSize(executable.getAtomicCounterBuffers(),
                               &AtomicCounterBuffer::memberIndexes);

        case GL_SHADER_STORAGE_BLOCK:
            return FindMaxSize(executable.getShaderStorageBlocks(), &InterfaceBlock::memberIndexes);

        default:
            UNREACHABLE();
            return 0;
    }
}

GLenum GetUniformPropertyEnum(GLenum prop)
{
    switch (prop)
    {
        case GL_UNIFORM_TYPE:
            return GL_TYPE;
        case GL_UNIFORM_SIZE:
            return GL_ARRAY_SIZE;
        case GL_UNIFORM_NAME_LENGTH:
            return GL_NAME_LENGTH;
        case GL_UNIFORM_BLOCK_INDEX:
            return GL_BLOCK_INDEX;
        case GL_UNIFORM_OFFSET:
            return GL_OFFSET;
        case GL_UNIFORM_ARRAY_STRIDE:
            return GL_ARRAY_STRIDE;
        case GL_UNIFORM_MATRIX_STRIDE:
            return GL_MATRIX_STRIDE;
        case GL_UNIFORM_IS_ROW_MAJOR:
            return GL_IS_ROW_MAJOR;

        default:
            return prop;
    }
}

GLenum GetUniformBlockPropertyEnum(GLenum prop)
{
    switch (prop)
    {
        case GL_UNIFORM_BLOCK_BINDING:
            return GL_BUFFER_BINDING;

        case GL_UNIFORM_BLOCK_DATA_SIZE:
            return GL_BUFFER_DATA_SIZE;

        case GL_UNIFORM_BLOCK_NAME_LENGTH:
            return GL_NAME_LENGTH;

        case GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS:
            return GL_NUM_ACTIVE_VARIABLES;

        case GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES:
            return GL_ACTIVE_VARIABLES;

        case GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER:
            return GL_REFERENCED_BY_VERTEX_SHADER;

        case GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER:
            return GL_REFERENCED_BY_FRAGMENT_SHADER;

        default:
            return prop;
    }
}

template <typename ShaderVariableT>
void GetShaderVariableBufferResourceProperty(const ShaderVariableT &buffer,
                                             GLenum pname,
                                             GLint *params,
                                             GLsizei bufSize,
                                             GLsizei *outputPosition)

{
    switch (pname)
    {
        case GL_BUFFER_DATA_SIZE:
            params[(*outputPosition)++] = clampCast<GLint>(buffer.pod.dataSize);
            break;
        case GL_NUM_ACTIVE_VARIABLES:
            params[(*outputPosition)++] = buffer.numActiveVariables();
            break;
        case GL_ACTIVE_VARIABLES:
            for (size_t memberIndex = 0;
                 memberIndex < buffer.memberIndexes.size() && *outputPosition < bufSize;
                 ++memberIndex)
            {
                params[(*outputPosition)++] = clampCast<GLint>(buffer.memberIndexes[memberIndex]);
            }
            break;
        case GL_REFERENCED_BY_VERTEX_SHADER:
            params[(*outputPosition)++] = static_cast<GLint>(buffer.isActive(ShaderType::Vertex));
            break;
        case GL_REFERENCED_BY_FRAGMENT_SHADER:
            params[(*outputPosition)++] = static_cast<GLint>(buffer.isActive(ShaderType::Fragment));
            break;
        case GL_REFERENCED_BY_COMPUTE_SHADER:
            params[(*outputPosition)++] = static_cast<GLint>(buffer.isActive(ShaderType::Compute));
            break;
        case GL_REFERENCED_BY_GEOMETRY_SHADER_EXT:
            params[(*outputPosition)++] = static_cast<GLint>(buffer.isActive(ShaderType::Geometry));
            break;
        case GL_REFERENCED_BY_TESS_CONTROL_SHADER_EXT:
            params[(*outputPosition)++] =
                static_cast<GLint>(buffer.isActive(ShaderType::TessControl));
            break;
        case GL_REFERENCED_BY_TESS_EVALUATION_SHADER_EXT:
            params[(*outputPosition)++] =
                static_cast<GLint>(buffer.isActive(ShaderType::TessEvaluation));
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void GetInterfaceBlockResourceProperty(const InterfaceBlock &block,
                                       GLenum pname,
                                       GLint *params,
                                       GLsizei bufSize,
                                       GLsizei *outputPosition)
{
    if (pname == GL_NAME_LENGTH)
    {
        params[(*outputPosition)++] = clampCast<GLint>(block.nameWithArrayIndex().size() + 1);
        return;
    }
    GetShaderVariableBufferResourceProperty(block, pname, params, bufSize, outputPosition);
}

void GetUniformBlockResourceProperty(const Program *program,
                                     GLuint blockIndex,
                                     GLenum pname,
                                     GLint *params,
                                     GLsizei bufSize,
                                     GLsizei *outputPosition)

{
    ASSERT(*outputPosition < bufSize);

    if (pname == GL_BUFFER_BINDING)
    {
        params[(*outputPosition)++] = program->getExecutable().getUniformBlockBinding(blockIndex);
        return;
    }

    const auto &block = program->getExecutable().getUniformBlockByIndex(blockIndex);
    GetInterfaceBlockResourceProperty(block, pname, params, bufSize, outputPosition);
}

void GetShaderStorageBlockResourceProperty(const Program *program,
                                           GLuint blockIndex,
                                           GLenum pname,
                                           GLint *params,
                                           GLsizei bufSize,
                                           GLsizei *outputPosition)

{
    ASSERT(*outputPosition < bufSize);

    if (pname == GL_BUFFER_BINDING)
    {
        params[(*outputPosition)++] =
            program->getExecutable().getShaderStorageBlockBinding(blockIndex);
        return;
    }

    const auto &block = program->getExecutable().getShaderStorageBlockByIndex(blockIndex);
    GetInterfaceBlockResourceProperty(block, pname, params, bufSize, outputPosition);
}

void GetAtomicCounterBufferResourceProperty(const Program *program,
                                            GLuint index,
                                            GLenum pname,
                                            GLint *params,
                                            GLsizei bufSize,
                                            GLsizei *outputPosition)

{
    ASSERT(*outputPosition < bufSize);

    if (pname == GL_BUFFER_BINDING)
    {
        params[(*outputPosition)++] = program->getExecutable().getAtomicCounterBufferBinding(index);
        return;
    }

    const auto &buffer = program->getExecutable().getAtomicCounterBuffers()[index];
    GetShaderVariableBufferResourceProperty(buffer, pname, params, bufSize, outputPosition);
}

bool IsTextureEnvEnumParameter(TextureEnvParameter pname)
{
    switch (pname)
    {
        case TextureEnvParameter::Mode:
        case TextureEnvParameter::CombineRgb:
        case TextureEnvParameter::CombineAlpha:
        case TextureEnvParameter::Src0Rgb:
        case TextureEnvParameter::Src1Rgb:
        case TextureEnvParameter::Src2Rgb:
        case TextureEnvParameter::Src0Alpha:
        case TextureEnvParameter::Src1Alpha:
        case TextureEnvParameter::Src2Alpha:
        case TextureEnvParameter::Op0Rgb:
        case TextureEnvParameter::Op1Rgb:
        case TextureEnvParameter::Op2Rgb:
        case TextureEnvParameter::Op0Alpha:
        case TextureEnvParameter::Op1Alpha:
        case TextureEnvParameter::Op2Alpha:
        case TextureEnvParameter::PointCoordReplace:
            return true;
        default:
            return false;
    }
}

void GetShaderProgramId(ProgramPipeline *programPipeline, ShaderType shaderType, GLint *params)
{
    ASSERT(params);

    *params = 0;
    if (programPipeline)
    {
        const Program *program = programPipeline->getShaderProgram(shaderType);
        if (program)
        {
            *params = program->id().value;
        }
    }
}

}  // namespace

void QueryFramebufferAttachmentParameteriv(const Context *context,
                                           const Framebuffer *framebuffer,
                                           GLenum attachment,
                                           GLenum pname,
                                           GLint *params)
{
    ASSERT(framebuffer);

    const FramebufferAttachment *attachmentObject = framebuffer->getAttachment(context, attachment);

    if (attachmentObject == nullptr)
    {
        // ES 2.0.25 spec pg 127 states that if the value of FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE
        // is NONE, then querying any other pname will generate INVALID_ENUM.

        // ES 3.0.2 spec pg 235 states that if the attachment type is none,
        // GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME will return zero and be an
        // INVALID_OPERATION for all other pnames

        switch (pname)
        {
            case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
                *params = GL_NONE;
                break;

            case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
                *params = 0;
                break;

            default:
                UNREACHABLE();
                break;
        }

        return;
    }

    switch (pname)
    {
        case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
            *params = attachmentObject->type();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
            *params = attachmentObject->id();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
            *params = attachmentObject->mipLevel();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
        {
            TextureTarget face = attachmentObject->cubeMapFace();
            if (face != TextureTarget::InvalidEnum)
            {
                *params = ToGLenum(attachmentObject->cubeMapFace());
            }
            else
            {
                // This happens when the attachment isn't a texture cube map face
                *params = GL_NONE;
            }
        }
        break;

        case GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE:
            *params = attachmentObject->getRedSize();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE:
            *params = attachmentObject->getGreenSize();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE:
            *params = attachmentObject->getBlueSize();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE:
            *params = attachmentObject->getAlphaSize();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE:
            *params = attachmentObject->getDepthSize();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE:
            *params = attachmentObject->getStencilSize();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE:
            *params = attachmentObject->getComponentType();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING:
            *params = attachmentObject->getColorEncoding();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER:
            *params = attachmentObject->layer();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_NUM_VIEWS_OVR:
            *params = attachmentObject->getNumViews();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_BASE_VIEW_INDEX_OVR:
            *params = attachmentObject->getBaseViewIndex();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_LAYERED_EXT:
            *params = attachmentObject->isLayered();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SAMPLES_EXT:
            if (attachmentObject->type() == GL_TEXTURE)
            {
                *params = attachmentObject->getSamples();
            }
            else
            {
                *params = 0;
            }
            break;

        default:
            UNREACHABLE();
            break;
    }
}

void QueryBufferParameteriv(const Buffer *buffer, GLenum pname, GLint *params)
{
    QueryBufferParameterBase(buffer, pname, params);
}

void QueryBufferParameteri64v(const Buffer *buffer, GLenum pname, GLint64 *params)
{
    QueryBufferParameterBase(buffer, pname, params);
}

void QueryBufferPointerv(const Buffer *buffer, GLenum pname, void **params)
{
    switch (pname)
    {
        case GL_BUFFER_MAP_POINTER:
            *params = buffer->getMapPointer();
            break;

        default:
            UNREACHABLE();
            break;
    }
}

void QueryProgramiv(Context *context, Program *program, GLenum pname, GLint *params)
{
    ASSERT(program != nullptr || pname == GL_COMPLETION_STATUS_KHR);

    switch (pname)
    {
        case GL_DELETE_STATUS:
            *params = program->isFlaggedForDeletion();
            return;
        case GL_LINK_STATUS:
            *params = program->isLinked();
            return;
        case GL_COMPLETION_STATUS_KHR:
            if (context->isContextLost())
            {
                *params = GL_TRUE;
            }
            else
            {
                *params = program->isLinking() ? GL_FALSE : GL_TRUE;
            }
            return;
        case GL_VALIDATE_STATUS:
            *params = program->isValidated();
            return;
        case GL_INFO_LOG_LENGTH:
            *params = program->getInfoLogLength();
            return;
        case GL_ATTACHED_SHADERS:
            *params = program->getAttachedShadersCount();
            return;
        case GL_ACTIVE_ATTRIBUTES:
            *params = static_cast<GLint>(program->getExecutable().getProgramInputs().size());
            return;
        case GL_ACTIVE_ATTRIBUTE_MAX_LENGTH:
            *params = program->getExecutable().getActiveAttributeMaxLength();
            return;
        case GL_ACTIVE_UNIFORMS:
            *params = static_cast<GLint>(program->getExecutable().getUniforms().size());
            return;
        case GL_ACTIVE_UNIFORM_MAX_LENGTH:
            *params = program->getExecutable().getActiveUniformMaxLength();
            return;
        case GL_PROGRAM_BINARY_READY_ANGLE:
            *params = program->isBinaryReady(context);
            return;
        case GL_PROGRAM_BINARY_LENGTH_OES:
            *params = context->getCaps().programBinaryFormats.empty()
                          ? 0
                          : program->getBinaryLength(context);
            return;
        case GL_ACTIVE_UNIFORM_BLOCKS:
            *params = static_cast<GLint>(program->getExecutable().getUniformBlocks().size());
            return;
        case GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH:
            *params = program->getExecutable().getActiveUniformBlockMaxNameLength();
            break;
        case GL_TRANSFORM_FEEDBACK_BUFFER_MODE:
            *params = program->getExecutable().getTransformFeedbackBufferMode();
            break;
        case GL_TRANSFORM_FEEDBACK_VARYINGS:
            *params = clampCast<GLint>(
                program->getExecutable().getLinkedTransformFeedbackVaryings().size());
            break;
        case GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH:
            *params = program->getExecutable().getTransformFeedbackVaryingMaxLength();
            break;
        case GL_PROGRAM_BINARY_RETRIEVABLE_HINT:
            *params = program->getBinaryRetrievableHint();
            break;
        case GL_PROGRAM_SEPARABLE:
            // From es31cSeparateShaderObjsTests.cpp:
            // ProgramParameteri PROGRAM_SEPARABLE
            // NOTE: The query for PROGRAM_SEPARABLE must query latched
            //       state. In other words, the state of the binary after
            //       it was linked. So in the tests below, the queries
            //       should return the default state GL_FALSE since the
            //       program has no linked binary.
            *params = program->isSeparable() && program->isLinked();
            break;
        case GL_COMPUTE_WORK_GROUP_SIZE:
        {
            const sh::WorkGroupSize &localSize =
                program->getExecutable().getComputeShaderLocalSize();
            params[0] = localSize[0];
            params[1] = localSize[1];
            params[2] = localSize[2];
        }
        break;
        case GL_ACTIVE_ATOMIC_COUNTER_BUFFERS:
            *params = static_cast<GLint>(program->getExecutable().getAtomicCounterBuffers().size());
            break;
        case GL_GEOMETRY_LINKED_INPUT_TYPE_EXT:
            *params = ToGLenum(program->getExecutable().getGeometryShaderInputPrimitiveType());
            break;
        case GL_GEOMETRY_LINKED_OUTPUT_TYPE_EXT:
            *params = ToGLenum(program->getExecutable().getGeometryShaderOutputPrimitiveType());
            break;
        case GL_GEOMETRY_LINKED_VERTICES_OUT_EXT:
            *params = program->getExecutable().getGeometryShaderMaxVertices();
            break;
        case GL_GEOMETRY_SHADER_INVOCATIONS_EXT:
            *params = program->getExecutable().getGeometryShaderInvocations();
            break;
        case GL_TESS_CONTROL_OUTPUT_VERTICES_EXT:
            *params = program->getExecutable().getTessControlShaderVertices();
            break;
        case GL_TESS_GEN_MODE_EXT:
            *params = program->getExecutable().getTessGenMode();
            break;
        case GL_TESS_GEN_SPACING_EXT:
            *params = program->getExecutable().getTessGenSpacing()
                          ? program->getExecutable().getTessGenSpacing()
                          : GL_EQUAL;
            break;
        case GL_TESS_GEN_VERTEX_ORDER:
            *params = program->getExecutable().getTessGenVertexOrder()
                          ? program->getExecutable().getTessGenVertexOrder()
                          : GL_CCW;
            break;
        case GL_TESS_GEN_POINT_MODE_EXT:
            *params = program->getExecutable().getTessGenPointMode() ? GL_TRUE : GL_FALSE;
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void QueryRenderbufferiv(const Context *context,
                         const Renderbuffer *renderbuffer,
                         GLenum pname,
                         GLint *params)
{
    ASSERT(renderbuffer != nullptr);

    switch (pname)
    {
        case GL_RENDERBUFFER_WIDTH:
            *params = renderbuffer->getWidth();
            break;
        case GL_RENDERBUFFER_HEIGHT:
            *params = renderbuffer->getHeight();
            break;
        case GL_RENDERBUFFER_INTERNAL_FORMAT:
            // Special case the WebGL 1 DEPTH_STENCIL format.
            if (context->isWebGL1() &&
                renderbuffer->getFormat().info->internalFormat == GL_DEPTH24_STENCIL8)
            {
                *params = GL_DEPTH_STENCIL;
            }
            else
            {
                *params = renderbuffer->getFormat().info->internalFormat;
            }
            break;
        case GL_RENDERBUFFER_RED_SIZE:
            *params = renderbuffer->getRedSize();
            break;
        case GL_RENDERBUFFER_GREEN_SIZE:
            *params = renderbuffer->getGreenSize();
            break;
        case GL_RENDERBUFFER_BLUE_SIZE:
            *params = renderbuffer->getBlueSize();
            break;
        case GL_RENDERBUFFER_ALPHA_SIZE:
            *params = renderbuffer->getAlphaSize();
            break;
        case GL_RENDERBUFFER_DEPTH_SIZE:
            *params = renderbuffer->getDepthSize();
            break;
        case GL_RENDERBUFFER_STENCIL_SIZE:
            *params = renderbuffer->getStencilSize();
            break;
        case GL_RENDERBUFFER_SAMPLES_ANGLE:
            *params = renderbuffer->getState().getSamples();
            break;
        case GL_MEMORY_SIZE_ANGLE:
            *params = renderbuffer->getMemorySize();
            break;
        case GL_IMPLEMENTATION_COLOR_READ_FORMAT:
            *params = static_cast<GLint>(renderbuffer->getImplementationColorReadFormat(context));
            break;
        case GL_IMPLEMENTATION_COLOR_READ_TYPE:
            *params = static_cast<GLint>(renderbuffer->getImplementationColorReadType(context));
            break;
        case GL_RESOURCE_INITIALIZED_ANGLE:
            *params = (renderbuffer->initState(GL_NONE, ImageIndex()) == InitState::Initialized);
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void QueryShaderiv(const Context *context, Shader *shader, GLenum pname, GLint *params)
{
    ASSERT(shader != nullptr || pname == GL_COMPLETION_STATUS_KHR);

    switch (pname)
    {
        case GL_SHADER_TYPE:
            *params = static_cast<GLint>(ToGLenum(shader->getType()));
            return;
        case GL_DELETE_STATUS:
            *params = shader->isFlaggedForDeletion();
            return;
        case GL_COMPILE_STATUS:
            *params = shader->isCompiled(context) ? GL_TRUE : GL_FALSE;
            return;
        case GL_COMPLETION_STATUS_KHR:
            if (context->isContextLost())
            {
                *params = GL_TRUE;
            }
            else
            {
                *params = shader->isCompleted() ? GL_TRUE : GL_FALSE;
            }
            return;
        case GL_INFO_LOG_LENGTH:
            *params = shader->getInfoLogLength(context);
            return;
        case GL_SHADER_SOURCE_LENGTH:
            *params = shader->getSourceLength();
            return;
        case GL_TRANSLATED_SHADER_SOURCE_LENGTH_ANGLE:
            *params = shader->getTranslatedSourceWithDebugInfoLength(context);
            return;
        default:
            UNREACHABLE();
            break;
    }
}

void QueryTexLevelParameterfv(const Texture *texture,
                              TextureTarget target,
                              GLint level,
                              GLenum pname,
                              GLfloat *params)
{
    QueryTexLevelParameterBase(texture, target, level, pname, params);
}

void QueryTexLevelParameteriv(const Texture *texture,
                              TextureTarget target,
                              GLint level,
                              GLenum pname,
                              GLint *params)
{
    QueryTexLevelParameterBase(texture, target, level, pname, params);
}

void QueryTexParameterfv(const Context *context,
                         const Texture *texture,
                         GLenum pname,
                         GLfloat *params)
{
    QueryTexParameterBase<false, false>(context, texture, pname, params);
}

void QueryTexParameterxv(const Context *context,
                         const Texture *texture,
                         GLenum pname,
                         GLfixed *params)
{
    QueryTexParameterBase<false, true>(context, texture, pname, params);
}

void QueryTexParameteriv(const Context *context,
                         const Texture *texture,
                         GLenum pname,
                         GLint *params)
{
    QueryTexParameterBase<false, false>(context, texture, pname, params);
}

void QueryTexParameterIiv(const Context *context,
                          const Texture *texture,
                          GLenum pname,
                          GLint *params)
{
    QueryTexParameterBase<true, false>(context, texture, pname, params);
}

void QueryTexParameterIuiv(const Context *context,
                           const Texture *texture,
                           GLenum pname,
                           GLuint *params)
{
    QueryTexParameterBase<true, false>(context, texture, pname, params);
}

void QuerySamplerParameterfv(const Sampler *sampler, GLenum pname, GLfloat *params)
{
    QuerySamplerParameterBase<false>(sampler, pname, params);
}

void QuerySamplerParameteriv(const Sampler *sampler, GLenum pname, GLint *params)
{
    QuerySamplerParameterBase<false>(sampler, pname, params);
}

void QuerySamplerParameterIiv(const Sampler *sampler, GLenum pname, GLint *params)
{
    QuerySamplerParameterBase<true>(sampler, pname, params);
}

void QuerySamplerParameterIuiv(const Sampler *sampler, GLenum pname, GLuint *params)
{
    QuerySamplerParameterBase<true>(sampler, pname, params);
}

void QueryVertexAttribfv(const VertexAttribute &attrib,
                         const VertexBinding &binding,
                         const VertexAttribCurrentValueData &currentValueData,
                         GLenum pname,
                         GLfloat *params)
{
    QueryVertexAttribBase(attrib, binding, currentValueData.Values.FloatValues, pname, params);
}

void QueryVertexAttribiv(const VertexAttribute &attrib,
                         const VertexBinding &binding,
                         const VertexAttribCurrentValueData &currentValueData,
                         GLenum pname,
                         GLint *params)
{
    QueryVertexAttribBase(attrib, binding, currentValueData.Values.FloatValues, pname, params);
}

void QueryVertexAttribPointerv(const VertexAttribute &attrib, GLenum pname, void **pointer)
{
    switch (pname)
    {
        case GL_VERTEX_ATTRIB_ARRAY_POINTER:
            *pointer = const_cast<void *>(attrib.pointer);
            break;

        default:
            UNREACHABLE();
            break;
    }
}

void QueryVertexAttribIiv(const VertexAttribute &attrib,
                          const VertexBinding &binding,
                          const VertexAttribCurrentValueData &currentValueData,
                          GLenum pname,
                          GLint *params)
{
    QueryVertexAttribBase(attrib, binding, currentValueData.Values.IntValues, pname, params);
}

void QueryVertexAttribIuiv(const VertexAttribute &attrib,
                           const VertexBinding &binding,
                           const VertexAttribCurrentValueData &currentValueData,
                           GLenum pname,
                           GLuint *params)
{
    QueryVertexAttribBase(attrib, binding, currentValueData.Values.UnsignedIntValues, pname,
                          params);
}

void QueryActiveUniformBlockiv(const Program *program,
                               UniformBlockIndex uniformBlockIndex,
                               GLenum pname,
                               GLint *params)
{
    GLenum prop = GetUniformBlockPropertyEnum(pname);
    QueryProgramResourceiv(program, GL_UNIFORM_BLOCK, uniformBlockIndex, 1, &prop,
                           std::numeric_limits<GLsizei>::max(), nullptr, params);
}

void QueryInternalFormativ(const Context *context,
                           const Texture *texture,
                           GLenum internalformat,
                           const TextureCaps &format,
                           GLenum pname,
                           GLsizei bufSize,
                           GLint *params)
{
    switch (pname)
    {
        case GL_NUM_SAMPLE_COUNTS:
            if (bufSize != 0)
            {
                *params = clampCast<GLint>(format.sampleCounts.size());
            }
            break;

        case GL_SAMPLES:
        {
            size_t returnCount   = std::min<size_t>(bufSize, format.sampleCounts.size());
            auto sampleReverseIt = format.sampleCounts.rbegin();
            for (size_t sampleIndex = 0; sampleIndex < returnCount; ++sampleIndex)
            {
                params[sampleIndex] = *sampleReverseIt++;
            }
        }
        break;

        case GL_NUM_SURFACE_COMPRESSION_FIXED_RATES_EXT:
            if (texture != nullptr)
            {
                *params = texture->getFormatSupportedCompressionRates(context, internalformat,
                                                                      bufSize, nullptr);
            }
            break;

        case GL_SURFACE_COMPRESSION_EXT:
            if (texture != nullptr)
            {
                texture->getFormatSupportedCompressionRates(context, internalformat, bufSize,
                                                            params);
            }
            break;

        default:
            UNREACHABLE();
            break;
    }
}

void QueryFramebufferParameteriv(const Framebuffer *framebuffer, GLenum pname, GLint *params)
{
    ASSERT(framebuffer);

    switch (pname)
    {
        case GL_FRAMEBUFFER_DEFAULT_WIDTH:
            *params = framebuffer->getDefaultWidth();
            break;
        case GL_FRAMEBUFFER_DEFAULT_HEIGHT:
            *params = framebuffer->getDefaultHeight();
            break;
        case GL_FRAMEBUFFER_DEFAULT_SAMPLES:
            *params = framebuffer->getDefaultSamples();
            break;
        case GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS:
            *params = ConvertToGLBoolean(framebuffer->getDefaultFixedSampleLocations());
            break;
        case GL_FRAMEBUFFER_DEFAULT_LAYERS_EXT:
            *params = framebuffer->getDefaultLayers();
            break;
        case GL_FRAMEBUFFER_FLIP_Y_MESA:
            *params = ConvertToGLBoolean(framebuffer->getFlipY());
            break;
        default:
            UNREACHABLE();
            break;
    }
}

angle::Result QuerySynciv(const Context *context,
                          const Sync *sync,
                          GLenum pname,
                          GLsizei bufSize,
                          GLsizei *length,
                          GLint *values)
{
    ASSERT(sync != nullptr || pname == GL_SYNC_STATUS);

    // All queries return one value, exit early if the buffer can't fit anything.
    if (bufSize < 1)
    {
        if (length != nullptr)
        {
            *length = 0;
        }
        return angle::Result::Continue;
    }

    switch (pname)
    {
        case GL_OBJECT_TYPE:
            *values = clampCast<GLint>(GL_SYNC_FENCE);
            break;
        case GL_SYNC_CONDITION:
            *values = clampCast<GLint>(sync->getCondition());
            break;
        case GL_SYNC_FLAGS:
            *values = clampCast<GLint>(sync->getFlags());
            break;
        case GL_SYNC_STATUS:
            if (context->isContextLost())
            {
                *values = GL_SIGNALED;
            }
            else
            {
                ANGLE_TRY(sync->getStatus(context, values));
            }
            break;

        default:
            UNREACHABLE();
            break;
    }

    if (length != nullptr)
    {
        *length = 1;
    }

    return angle::Result::Continue;
}

void SetTexParameterx(Context *context, Texture *texture, GLenum pname, GLfixed param)
{
    SetTexParameterBase<false, true>(context, texture, pname, &param);
}

void SetTexParameterxv(Context *context, Texture *texture, GLenum pname, const GLfixed *params)
{
    SetTexParameterBase<false, true>(context, texture, pname, params);
}

void SetTexParameterf(Context *context, Texture *texture, GLenum pname, GLfloat param)
{
    SetTexParameterBase<false, false>(context, texture, pname, &param);
}

void SetTexParameterfv(Context *context, Texture *texture, GLenum pname, const GLfloat *params)
{
    SetTexParameterBase<false, false>(context, texture, pname, params);
}

void SetTexParameteri(Context *context, Texture *texture, GLenum pname, GLint param)
{
    SetTexParameterBase<false, false>(context, texture, pname, &param);
}

void SetTexParameteriv(Context *context, Texture *texture, GLenum pname, const GLint *params)
{
    SetTexParameterBase<false, false>(context, texture, pname, params);
}

void SetTexParameterIiv(Context *context, Texture *texture, GLenum pname, const GLint *params)
{
    SetTexParameterBase<true, false>(context, texture, pname, params);
}

void SetTexParameterIuiv(Context *context, Texture *texture, GLenum pname, const GLuint *params)
{
    SetTexParameterBase<true, false>(context, texture, pname, params);
}

void SetSamplerParameterf(Context *context, Sampler *sampler, GLenum pname, GLfloat param)
{
    SetSamplerParameterBase<false>(context, sampler, pname, &param);
}

void SetSamplerParameterfv(Context *context, Sampler *sampler, GLenum pname, const GLfloat *params)
{
    SetSamplerParameterBase<false>(context, sampler, pname, params);
}

void SetSamplerParameteri(Context *context, Sampler *sampler, GLenum pname, GLint param)
{
    SetSamplerParameterBase<false>(context, sampler, pname, &param);
}

void SetSamplerParameteriv(Context *context, Sampler *sampler, GLenum pname, const GLint *params)
{
    SetSamplerParameterBase<false>(context, sampler, pname, params);
}

void SetSamplerParameterIiv(Context *context, Sampler *sampler, GLenum pname, const GLint *params)
{
    SetSamplerParameterBase<true>(context, sampler, pname, params);
}

void SetSamplerParameterIuiv(Context *context, Sampler *sampler, GLenum pname, const GLuint *params)
{
    SetSamplerParameterBase<true>(context, sampler, pname, params);
}

void SetFramebufferParameteri(const Context *context,
                              Framebuffer *framebuffer,
                              GLenum pname,
                              GLint param)
{
    ASSERT(framebuffer);

    switch (pname)
    {
        case GL_FRAMEBUFFER_DEFAULT_WIDTH:
            framebuffer->setDefaultWidth(context, param);
            break;
        case GL_FRAMEBUFFER_DEFAULT_HEIGHT:
            framebuffer->setDefaultHeight(context, param);
            break;
        case GL_FRAMEBUFFER_DEFAULT_SAMPLES:
            framebuffer->setDefaultSamples(context, param);
            break;
        case GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS:
            framebuffer->setDefaultFixedSampleLocations(context, ConvertToBool(param));
            break;
        case GL_FRAMEBUFFER_DEFAULT_LAYERS_EXT:
            framebuffer->setDefaultLayers(param);
            break;
        case GL_FRAMEBUFFER_FLIP_Y_MESA:
            framebuffer->setFlipY(ConvertToBool(param));
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void SetProgramParameteri(const Context *context, Program *program, GLenum pname, GLint value)
{
    ASSERT(program);

    switch (pname)
    {
        case GL_PROGRAM_BINARY_RETRIEVABLE_HINT:
            program->setBinaryRetrievableHint(ConvertToBool(value));
            break;
        case GL_PROGRAM_SEPARABLE:
            program->setSeparable(context, ConvertToBool(value));
            break;
        default:
            UNREACHABLE();
            break;
    }
}

GLint GetUniformResourceProperty(const Program *program, GLuint index, const GLenum prop)
{
    const ProgramExecutable &executable = program->getExecutable();
    const LinkedUniform &uniform        = executable.getUniformByIndex(index);

    GLenum resourceProp = GetUniformPropertyEnum(prop);
    switch (resourceProp)
    {
        case GL_TYPE:
            return clampCast<GLint>(uniform.getType());

        case GL_ARRAY_SIZE:
            return clampCast<GLint>(uniform.getBasicTypeElementCount());

        case GL_NAME_LENGTH:
            return clampCast<GLint>(executable.getUniformNameByIndex(index).size() + 1u);

        case GL_LOCATION:
            return executable.getUniformLocation(executable.getUniformNameByIndex(index)).value;

        case GL_BLOCK_INDEX:
            return (uniform.isAtomicCounter() ? -1 : uniform.getBufferIndex());

        case GL_OFFSET:
            return uniform.pod.flagBits.isBlock ? uniform.pod.blockOffset : -1;

        case GL_ARRAY_STRIDE:
            return uniform.pod.flagBits.isBlock ? uniform.pod.blockArrayStride : -1;

        case GL_MATRIX_STRIDE:
            return uniform.pod.flagBits.isBlock ? uniform.pod.blockMatrixStride : -1;

        case GL_IS_ROW_MAJOR:
            return uniform.pod.flagBits.blockIsRowMajorMatrix ? 1 : 0;

        case GL_REFERENCED_BY_VERTEX_SHADER:
            return uniform.isActive(ShaderType::Vertex);

        case GL_REFERENCED_BY_FRAGMENT_SHADER:
            return uniform.isActive(ShaderType::Fragment);

        case GL_REFERENCED_BY_COMPUTE_SHADER:
            return uniform.isActive(ShaderType::Compute);

        case GL_REFERENCED_BY_GEOMETRY_SHADER_EXT:
            return uniform.isActive(ShaderType::Geometry);

        case GL_REFERENCED_BY_TESS_CONTROL_SHADER_EXT:
            return uniform.isActive(ShaderType::TessControl);

        case GL_REFERENCED_BY_TESS_EVALUATION_SHADER_EXT:
            return uniform.isActive(ShaderType::TessEvaluation);

        case GL_ATOMIC_COUNTER_BUFFER_INDEX:
            return (uniform.isAtomicCounter() ? uniform.getBufferIndex() : -1);

        default:
            UNREACHABLE();
            return 0;
    }
}

GLint GetBufferVariableResourceProperty(const Program *program, GLuint index, const GLenum prop)
{
    const ProgramExecutable &executable  = program->getExecutable();
    const BufferVariable &bufferVariable = executable.getBufferVariableByIndex(index);

    switch (prop)
    {
        case GL_TYPE:
        case GL_ARRAY_SIZE:
        case GL_NAME_LENGTH:
            return GetCommonVariableProperty(bufferVariable, prop);

        case GL_BLOCK_INDEX:
            return bufferVariable.pod.bufferIndex;

        case GL_OFFSET:
            return bufferVariable.pod.blockInfo.offset;

        case GL_ARRAY_STRIDE:
            return bufferVariable.pod.blockInfo.arrayStride;

        case GL_MATRIX_STRIDE:
            return bufferVariable.pod.blockInfo.matrixStride;

        case GL_IS_ROW_MAJOR:
            return static_cast<GLint>(bufferVariable.pod.blockInfo.isRowMajorMatrix);

        case GL_REFERENCED_BY_VERTEX_SHADER:
            return bufferVariable.isActive(ShaderType::Vertex);

        case GL_REFERENCED_BY_FRAGMENT_SHADER:
            return bufferVariable.isActive(ShaderType::Fragment);

        case GL_REFERENCED_BY_COMPUTE_SHADER:
            return bufferVariable.isActive(ShaderType::Compute);

        case GL_REFERENCED_BY_GEOMETRY_SHADER_EXT:
            return bufferVariable.isActive(ShaderType::Geometry);

        case GL_REFERENCED_BY_TESS_CONTROL_SHADER_EXT:
            return bufferVariable.isActive(ShaderType::TessControl);

        case GL_REFERENCED_BY_TESS_EVALUATION_SHADER_EXT:
            return bufferVariable.isActive(ShaderType::TessEvaluation);

        case GL_TOP_LEVEL_ARRAY_SIZE:
            return bufferVariable.pod.topLevelArraySize;

        case GL_TOP_LEVEL_ARRAY_STRIDE:
            return bufferVariable.pod.blockInfo.topLevelArrayStride;

        default:
            UNREACHABLE();
            return 0;
    }
}

GLuint QueryProgramResourceIndex(const Program *program,
                                 GLenum programInterface,
                                 const GLchar *name)
{
    const ProgramExecutable &executable = program->getExecutable();

    switch (programInterface)
    {
        case GL_PROGRAM_INPUT:
            return executable.getInputResourceIndex(name);

        case GL_PROGRAM_OUTPUT:
            return executable.getOutputResourceIndex(name);

        case GL_UNIFORM:
            return executable.getUniformIndexFromName(name);

        case GL_BUFFER_VARIABLE:
            return executable.getBufferVariableIndexFromName(name);

        case GL_SHADER_STORAGE_BLOCK:
            return executable.getShaderStorageBlockIndex(name);

        case GL_UNIFORM_BLOCK:
            return executable.getUniformBlockIndex(name);

        case GL_TRANSFORM_FEEDBACK_VARYING:
            return executable.getTransformFeedbackVaryingResourceIndex(name);

        default:
            UNREACHABLE();
            return GL_INVALID_INDEX;
    }
}

void QueryProgramResourceName(const Context *context,
                              const Program *program,
                              GLenum programInterface,
                              GLuint index,
                              GLsizei bufSize,
                              GLsizei *length,
                              GLchar *name)
{
    const ProgramExecutable &executable = program->getExecutable();

    switch (programInterface)
    {
        case GL_PROGRAM_INPUT:
            executable.getInputResourceName(index, bufSize, length, name);
            break;

        case GL_PROGRAM_OUTPUT:
            executable.getOutputResourceName(index, bufSize, length, name);
            break;

        case GL_UNIFORM:
            executable.getUniformResourceName(index, bufSize, length, name);
            break;

        case GL_BUFFER_VARIABLE:
            executable.getBufferVariableResourceName(index, bufSize, length, name);
            break;

        case GL_SHADER_STORAGE_BLOCK:
            executable.getActiveShaderStorageBlockName(index, bufSize, length, name);
            break;

        case GL_UNIFORM_BLOCK:
            executable.getActiveUniformBlockName(context, {index}, bufSize, length, name);
            break;

        case GL_TRANSFORM_FEEDBACK_VARYING:
            executable.getTransformFeedbackVarying(index, bufSize, length, nullptr, nullptr, name);
            break;

        default:
            UNREACHABLE();
    }
}

GLint QueryProgramResourceLocation(const Program *program,
                                   GLenum programInterface,
                                   const GLchar *name)
{
    const ProgramExecutable &executable = program->getExecutable();

    switch (programInterface)
    {
        case GL_PROGRAM_INPUT:
            return executable.getInputResourceLocation(name);

        case GL_PROGRAM_OUTPUT:
            return executable.getOutputResourceLocation(name);

        case GL_UNIFORM:
            return executable.getUniformLocation(name).value;

        default:
            UNREACHABLE();
            return -1;
    }
}

void QueryProgramResourceiv(const Program *program,
                            GLenum programInterface,
                            UniformBlockIndex index,
                            GLsizei propCount,
                            const GLenum *props,
                            GLsizei bufSize,
                            GLsizei *length,
                            GLint *params)
{
    if (!program->isLinked())
    {
        return;
    }

    if (length != nullptr)
    {
        *length = 0;
    }

    if (bufSize == 0)
    {
        // No room to write the results
        return;
    }

    GLsizei pos = 0;
    for (GLsizei i = 0; i < propCount; i++)
    {
        switch (programInterface)
        {
            case GL_PROGRAM_INPUT:
                params[i] = GetInputResourceProperty(program, index.value, props[i]);
                ++pos;
                break;

            case GL_PROGRAM_OUTPUT:
                params[i] = GetOutputResourceProperty(program, index.value, props[i]);
                ++pos;
                break;

            case GL_UNIFORM:
                params[i] = GetUniformResourceProperty(program, index.value, props[i]);
                ++pos;
                break;

            case GL_BUFFER_VARIABLE:
                params[i] = GetBufferVariableResourceProperty(program, index.value, props[i]);
                ++pos;
                break;

            case GL_UNIFORM_BLOCK:
                GetUniformBlockResourceProperty(program, index.value, props[i], params, bufSize,
                                                &pos);
                break;

            case GL_SHADER_STORAGE_BLOCK:
                GetShaderStorageBlockResourceProperty(program, index.value, props[i], params,
                                                      bufSize, &pos);
                break;

            case GL_ATOMIC_COUNTER_BUFFER:
                GetAtomicCounterBufferResourceProperty(program, index.value, props[i], params,
                                                       bufSize, &pos);
                break;

            case GL_TRANSFORM_FEEDBACK_VARYING:
                params[i] =
                    GetTransformFeedbackVaryingResourceProperty(program, index.value, props[i]);
                ++pos;
                break;

            default:
                UNREACHABLE();
                params[i] = GL_INVALID_VALUE;
        }
        if (pos == bufSize)
        {
            // Most properties return one value, but GL_ACTIVE_VARIABLES returns an array of values.
            // This checks not to break buffer bounds for such case.
            break;
        }
    }

    if (length != nullptr)
    {
        *length = pos;
    }
}

void QueryProgramInterfaceiv(const Program *program,
                             GLenum programInterface,
                             GLenum pname,
                             GLint *params)
{
    switch (pname)
    {
        case GL_ACTIVE_RESOURCES:
            *params = QueryProgramInterfaceActiveResources(program, programInterface);
            break;

        case GL_MAX_NAME_LENGTH:
            *params = QueryProgramInterfaceMaxNameLength(program, programInterface);
            break;

        case GL_MAX_NUM_ACTIVE_VARIABLES:
            *params = QueryProgramInterfaceMaxNumActiveVariables(program, programInterface);
            break;

        default:
            UNREACHABLE();
    }
}

angle::Result SetMemoryObjectParameteriv(const Context *context,
                                         MemoryObject *memoryObject,
                                         GLenum pname,
                                         const GLint *params)
{
    switch (pname)
    {
        case GL_DEDICATED_MEMORY_OBJECT_EXT:
            ANGLE_TRY(memoryObject->setDedicatedMemory(context, ConvertToBool(params[0])));
            break;

        case GL_PROTECTED_MEMORY_OBJECT_EXT:
            ANGLE_TRY(memoryObject->setProtectedMemory(context, ConvertToBool(params[0])));
            break;

        default:
            UNREACHABLE();
    }

    return angle::Result::Continue;
}

void QueryMemoryObjectParameteriv(const MemoryObject *memoryObject, GLenum pname, GLint *params)
{
    switch (pname)
    {
        case GL_DEDICATED_MEMORY_OBJECT_EXT:
            *params = memoryObject->isDedicatedMemory();
            break;

        case GL_PROTECTED_MEMORY_OBJECT_EXT:
            *params = memoryObject->isProtectedMemory();
            break;

        default:
            UNREACHABLE();
    }
}

ClientVertexArrayType ParamToVertexArrayType(GLenum param)
{
    switch (param)
    {
        case GL_VERTEX_ARRAY:
        case GL_VERTEX_ARRAY_BUFFER_BINDING:
        case GL_VERTEX_ARRAY_STRIDE:
        case GL_VERTEX_ARRAY_SIZE:
        case GL_VERTEX_ARRAY_TYPE:
        case GL_VERTEX_ARRAY_POINTER:
            return ClientVertexArrayType::Vertex;
        case GL_NORMAL_ARRAY:
        case GL_NORMAL_ARRAY_BUFFER_BINDING:
        case GL_NORMAL_ARRAY_STRIDE:
        case GL_NORMAL_ARRAY_TYPE:
        case GL_NORMAL_ARRAY_POINTER:
            return ClientVertexArrayType::Normal;
        case GL_COLOR_ARRAY:
        case GL_COLOR_ARRAY_BUFFER_BINDING:
        case GL_COLOR_ARRAY_STRIDE:
        case GL_COLOR_ARRAY_SIZE:
        case GL_COLOR_ARRAY_TYPE:
        case GL_COLOR_ARRAY_POINTER:
            return ClientVertexArrayType::Color;
        case GL_POINT_SIZE_ARRAY_OES:
        case GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES:
        case GL_POINT_SIZE_ARRAY_STRIDE_OES:
        case GL_POINT_SIZE_ARRAY_TYPE_OES:
        case GL_POINT_SIZE_ARRAY_POINTER_OES:
            return ClientVertexArrayType::PointSize;
        case GL_TEXTURE_COORD_ARRAY:
        case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING:
        case GL_TEXTURE_COORD_ARRAY_STRIDE:
        case GL_TEXTURE_COORD_ARRAY_SIZE:
        case GL_TEXTURE_COORD_ARRAY_TYPE:
        case GL_TEXTURE_COORD_ARRAY_POINTER:
            return ClientVertexArrayType::TextureCoord;
        default:
            UNREACHABLE();
            return ClientVertexArrayType::InvalidEnum;
    }
}

void SetLightModelParameters(GLES1State *state, GLenum pname, const GLfloat *params)
{
    LightModelParameters &lightModel = state->lightModelParameters();

    switch (pname)
    {
        case GL_LIGHT_MODEL_AMBIENT:
            lightModel.color = ColorF::fromData(params);
            break;
        case GL_LIGHT_MODEL_TWO_SIDE:
            lightModel.twoSided = *params == 1.0f ? true : false;
            break;
        default:
            break;
    }
}

void GetLightModelParameters(const GLES1State *state, GLenum pname, GLfloat *params)
{
    const LightModelParameters &lightModel = state->lightModelParameters();

    switch (pname)
    {
        case GL_LIGHT_MODEL_TWO_SIDE:
            *params = lightModel.twoSided ? 1.0f : 0.0f;
            break;
        case GL_LIGHT_MODEL_AMBIENT:
            lightModel.color.writeData(params);
            break;
        default:
            break;
    }
}

bool IsLightModelTwoSided(const GLES1State *state)
{
    return state->lightModelParameters().twoSided;
}

void SetLightParameters(GLES1State *state,
                        GLenum light,
                        LightParameter pname,
                        const GLfloat *params)
{
    uint32_t lightIndex = light - GL_LIGHT0;

    LightParameters &lightParams = state->lightParameters(lightIndex);

    switch (pname)
    {
        case LightParameter::Ambient:
            lightParams.ambient = ColorF::fromData(params);
            break;
        case LightParameter::Diffuse:
            lightParams.diffuse = ColorF::fromData(params);
            break;
        case LightParameter::Specular:
            lightParams.specular = ColorF::fromData(params);
            break;
        case LightParameter::Position:
        {
            angle::Mat4 mv = state->getModelviewMatrix();
            angle::Vector4 transformedPos =
                mv.product(angle::Vector4(params[0], params[1], params[2], params[3]));
            lightParams.position[0] = transformedPos[0];
            lightParams.position[1] = transformedPos[1];
            lightParams.position[2] = transformedPos[2];
            lightParams.position[3] = transformedPos[3];
        }
        break;
        case LightParameter::SpotDirection:
        {
            angle::Mat4 mv = state->getModelviewMatrix();
            angle::Vector4 transformedPos =
                mv.product(angle::Vector4(params[0], params[1], params[2], 0.0f));
            lightParams.direction[0] = transformedPos[0];
            lightParams.direction[1] = transformedPos[1];
            lightParams.direction[2] = transformedPos[2];
        }
        break;
        case LightParameter::SpotExponent:
            lightParams.spotlightExponent = *params;
            break;
        case LightParameter::SpotCutoff:
            lightParams.spotlightCutoffAngle = *params;
            break;
        case LightParameter::ConstantAttenuation:
            lightParams.attenuationConst = *params;
            break;
        case LightParameter::LinearAttenuation:
            lightParams.attenuationLinear = *params;
            break;
        case LightParameter::QuadraticAttenuation:
            lightParams.attenuationQuadratic = *params;
            break;
        default:
            return;
    }
}

void GetLightParameters(const GLES1State *state,
                        GLenum light,
                        LightParameter pname,
                        GLfloat *params)
{
    uint32_t lightIndex                = light - GL_LIGHT0;
    const LightParameters &lightParams = state->lightParameters(lightIndex);

    switch (pname)
    {
        case LightParameter::Ambient:
            lightParams.ambient.writeData(params);
            break;
        case LightParameter::Diffuse:
            lightParams.diffuse.writeData(params);
            break;
        case LightParameter::Specular:
            lightParams.specular.writeData(params);
            break;
        case LightParameter::Position:
            memcpy(params, lightParams.position.data(), 4 * sizeof(GLfloat));
            break;
        case LightParameter::SpotDirection:
            memcpy(params, lightParams.direction.data(), 3 * sizeof(GLfloat));
            break;
        case LightParameter::SpotExponent:
            *params = lightParams.spotlightExponent;
            break;
        case LightParameter::SpotCutoff:
            *params = lightParams.spotlightCutoffAngle;
            break;
        case LightParameter::ConstantAttenuation:
            *params = lightParams.attenuationConst;
            break;
        case LightParameter::LinearAttenuation:
            *params = lightParams.attenuationLinear;
            break;
        case LightParameter::QuadraticAttenuation:
            *params = lightParams.attenuationQuadratic;
            break;
        default:
            break;
    }
}

void SetMaterialParameters(GLES1State *state,
                           GLenum face,
                           MaterialParameter pname,
                           const GLfloat *params)
{
    // Note: Ambient and diffuse colors are inherited from glColor when COLOR_MATERIAL is enabled,
    // and can only be modified by this function if that is disabled:
    //
    // > the replaced values remain until changed by either sending a new color or by setting a
    // > new material value when COLOR_MATERIAL is not currently enabled, to override that
    // particular value.

    MaterialParameters &material = state->materialParameters();
    switch (pname)
    {
        case MaterialParameter::Ambient:
            if (!state->isColorMaterialEnabled())
            {
                material.ambient = ColorF::fromData(params);
            }
            break;
        case MaterialParameter::Diffuse:
            if (!state->isColorMaterialEnabled())
            {
                material.diffuse = ColorF::fromData(params);
            }
            break;
        case MaterialParameter::AmbientAndDiffuse:
            if (!state->isColorMaterialEnabled())
            {
                material.ambient = ColorF::fromData(params);
                material.diffuse = ColorF::fromData(params);
            }
            break;
        case MaterialParameter::Specular:
            material.specular = ColorF::fromData(params);
            break;
        case MaterialParameter::Emission:
            material.emissive = ColorF::fromData(params);
            break;
        case MaterialParameter::Shininess:
            material.specularExponent = *params;
            break;
        default:
            return;
    }
}

void GetMaterialParameters(const GLES1State *state,
                           GLenum face,
                           MaterialParameter pname,
                           GLfloat *params)
{
    const ColorF &currentColor         = state->getCurrentColor();
    const MaterialParameters &material = state->materialParameters();
    const bool colorMaterialEnabled    = state->isColorMaterialEnabled();

    switch (pname)
    {
        case MaterialParameter::Ambient:
            if (colorMaterialEnabled)
            {
                currentColor.writeData(params);
            }
            else
            {
                material.ambient.writeData(params);
            }
            break;
        case MaterialParameter::Diffuse:
            if (colorMaterialEnabled)
            {
                currentColor.writeData(params);
            }
            else
            {
                material.diffuse.writeData(params);
            }
            break;
        case MaterialParameter::Specular:
            material.specular.writeData(params);
            break;
        case MaterialParameter::Emission:
            material.emissive.writeData(params);
            break;
        case MaterialParameter::Shininess:
            *params = material.specularExponent;
            break;
        default:
            return;
    }
}

unsigned int GetLightModelParameterCount(GLenum pname)
{
    switch (pname)
    {
        case GL_LIGHT_MODEL_AMBIENT:
            return 4;
        case GL_LIGHT_MODEL_TWO_SIDE:
            return 1;
        default:
            UNREACHABLE();
            return 0;
    }
}

unsigned int GetLightParameterCount(LightParameter pname)
{
    switch (pname)
    {
        case LightParameter::Ambient:
        case LightParameter::Diffuse:
        case LightParameter::AmbientAndDiffuse:
        case LightParameter::Specular:
        case LightParameter::Position:
            return 4;
        case LightParameter::SpotDirection:
            return 3;
        case LightParameter::SpotExponent:
        case LightParameter::SpotCutoff:
        case LightParameter::ConstantAttenuation:
        case LightParameter::LinearAttenuation:
        case LightParameter::QuadraticAttenuation:
            return 1;
        default:
            UNREACHABLE();
            return 0;
    }
}

unsigned int GetMaterialParameterCount(MaterialParameter pname)
{
    switch (pname)
    {
        case MaterialParameter::Ambient:
        case MaterialParameter::Diffuse:
        case MaterialParameter::AmbientAndDiffuse:
        case MaterialParameter::Specular:
        case MaterialParameter::Emission:
            return 4;
        case MaterialParameter::Shininess:
            return 1;
        default:
            UNREACHABLE();
            return 0;
    }
}

void SetFogParameters(GLES1State *state, GLenum pname, const GLfloat *params)
{
    FogParameters &fog = state->fogParameters();
    switch (pname)
    {
        case GL_FOG_MODE:
            fog.mode = FromGLenum<FogMode>(static_cast<GLenum>(params[0]));
            break;
        case GL_FOG_DENSITY:
            fog.density = params[0];
            break;
        case GL_FOG_START:
            fog.start = params[0];
            break;
        case GL_FOG_END:
            fog.end = params[0];
            break;
        case GL_FOG_COLOR:
            fog.color = ColorF::fromData(params);
            break;
        default:
            return;
    }
}

void GetFogParameters(const GLES1State *state, GLenum pname, GLfloat *params)
{
    const FogParameters &fog = state->fogParameters();
    switch (pname)
    {
        case GL_FOG_MODE:
            params[0] = static_cast<GLfloat>(ToGLenum(fog.mode));
            break;
        case GL_FOG_DENSITY:
            params[0] = fog.density;
            break;
        case GL_FOG_START:
            params[0] = fog.start;
            break;
        case GL_FOG_END:
            params[0] = fog.end;
            break;
        case GL_FOG_COLOR:
            fog.color.writeData(params);
            break;
        default:
            return;
    }
}

unsigned int GetFogParameterCount(GLenum pname)
{
    switch (pname)
    {
        case GL_FOG_MODE:
        case GL_FOG_DENSITY:
        case GL_FOG_START:
        case GL_FOG_END:
            return 1;
        case GL_FOG_COLOR:
            return 4;
        default:
            return 0;
    }
}

unsigned int GetTextureEnvParameterCount(TextureEnvParameter pname)
{
    switch (pname)
    {
        case TextureEnvParameter::Mode:
        case TextureEnvParameter::CombineRgb:
        case TextureEnvParameter::CombineAlpha:
        case TextureEnvParameter::Src0Rgb:
        case TextureEnvParameter::Src1Rgb:
        case TextureEnvParameter::Src2Rgb:
        case TextureEnvParameter::Src0Alpha:
        case TextureEnvParameter::Src1Alpha:
        case TextureEnvParameter::Src2Alpha:
        case TextureEnvParameter::Op0Rgb:
        case TextureEnvParameter::Op1Rgb:
        case TextureEnvParameter::Op2Rgb:
        case TextureEnvParameter::Op0Alpha:
        case TextureEnvParameter::Op1Alpha:
        case TextureEnvParameter::Op2Alpha:
        case TextureEnvParameter::RgbScale:
        case TextureEnvParameter::AlphaScale:
        case TextureEnvParameter::PointCoordReplace:
            return 1;
        case TextureEnvParameter::Color:
            return 4;
        default:
            return 0;
    }
}

void ConvertTextureEnvFromInt(TextureEnvParameter pname, const GLint *input, GLfloat *output)
{
    if (IsTextureEnvEnumParameter(pname))
    {
        ConvertGLenumValue(input[0], output);
        return;
    }

    switch (pname)
    {
        case TextureEnvParameter::RgbScale:
        case TextureEnvParameter::AlphaScale:
            output[0] = static_cast<GLfloat>(input[0]);
            break;
        case TextureEnvParameter::Color:
            for (int i = 0; i < 4; i++)
            {
                output[i] = input[i] / 255.0f;
            }
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void ConvertTextureEnvFromFixed(TextureEnvParameter pname, const GLfixed *input, GLfloat *output)
{
    if (IsTextureEnvEnumParameter(pname))
    {
        ConvertGLenumValue(input[0], output);
        return;
    }

    switch (pname)
    {
        case TextureEnvParameter::RgbScale:
        case TextureEnvParameter::AlphaScale:
            output[0] = ConvertFixedToFloat(input[0]);
            break;
        case TextureEnvParameter::Color:
            for (int i = 0; i < 4; i++)
            {
                output[i] = ConvertFixedToFloat(input[i]);
            }
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void ConvertTextureEnvToInt(TextureEnvParameter pname, const GLfloat *input, GLint *output)
{
    if (IsTextureEnvEnumParameter(pname))
    {
        ConvertGLenumValue(input[0], output);
        return;
    }

    switch (pname)
    {
        case TextureEnvParameter::RgbScale:
        case TextureEnvParameter::AlphaScale:
            output[0] = static_cast<GLint>(input[0]);
            break;
        case TextureEnvParameter::Color:
            for (int i = 0; i < 4; i++)
            {
                output[i] = static_cast<GLint>(input[i] * 255.0f);
            }
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void ConvertTextureEnvToFixed(TextureEnvParameter pname, const GLfloat *input, GLfixed *output)
{
    if (IsTextureEnvEnumParameter(pname))
    {
        ConvertGLenumValue(input[0], output);
        return;
    }

    switch (pname)
    {
        case TextureEnvParameter::RgbScale:
        case TextureEnvParameter::AlphaScale:
            output[0] = ConvertFloatToFixed(input[0]);
            break;
        case TextureEnvParameter::Color:
            for (int i = 0; i < 4; i++)
            {
                output[i] = ConvertFloatToFixed(input[i]);
            }
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void SetTextureEnv(unsigned int unit,
                   GLES1State *state,
                   TextureEnvTarget target,
                   TextureEnvParameter pname,
                   const GLfloat *params)
{
    TextureEnvironmentParameters &env = state->textureEnvironment(unit);
    GLenum asEnum                     = ConvertToGLenum(params[0]);

    switch (target)
    {
        case TextureEnvTarget::Env:
            switch (pname)
            {
                case TextureEnvParameter::Mode:
                    env.mode = FromGLenum<TextureEnvMode>(asEnum);
                    break;
                case TextureEnvParameter::CombineRgb:
                    env.combineRgb = FromGLenum<TextureCombine>(asEnum);
                    break;
                case TextureEnvParameter::CombineAlpha:
                    env.combineAlpha = FromGLenum<TextureCombine>(asEnum);
                    break;
                case TextureEnvParameter::Src0Rgb:
                    env.src0Rgb = FromGLenum<TextureSrc>(asEnum);
                    break;
                case TextureEnvParameter::Src1Rgb:
                    env.src1Rgb = FromGLenum<TextureSrc>(asEnum);
                    break;
                case TextureEnvParameter::Src2Rgb:
                    env.src2Rgb = FromGLenum<TextureSrc>(asEnum);
                    break;
                case TextureEnvParameter::Src0Alpha:
                    env.src0Alpha = FromGLenum<TextureSrc>(asEnum);
                    break;
                case TextureEnvParameter::Src1Alpha:
                    env.src1Alpha = FromGLenum<TextureSrc>(asEnum);
                    break;
                case TextureEnvParameter::Src2Alpha:
                    env.src2Alpha = FromGLenum<TextureSrc>(asEnum);
                    break;
                case TextureEnvParameter::Op0Rgb:
                    env.op0Rgb = FromGLenum<TextureOp>(asEnum);
                    break;
                case TextureEnvParameter::Op1Rgb:
                    env.op1Rgb = FromGLenum<TextureOp>(asEnum);
                    break;
                case TextureEnvParameter::Op2Rgb:
                    env.op2Rgb = FromGLenum<TextureOp>(asEnum);
                    break;
                case TextureEnvParameter::Op0Alpha:
                    env.op0Alpha = FromGLenum<TextureOp>(asEnum);
                    break;
                case TextureEnvParameter::Op1Alpha:
                    env.op1Alpha = FromGLenum<TextureOp>(asEnum);
                    break;
                case TextureEnvParameter::Op2Alpha:
                    env.op2Alpha = FromGLenum<TextureOp>(asEnum);
                    break;
                case TextureEnvParameter::Color:
                    env.color = ColorF::fromData(params);
                    break;
                case TextureEnvParameter::RgbScale:
                    env.rgbScale = params[0];
                    break;
                case TextureEnvParameter::AlphaScale:
                    env.alphaScale = params[0];
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
            break;
        case TextureEnvTarget::PointSprite:
            switch (pname)
            {
                case TextureEnvParameter::PointCoordReplace:
                    env.pointSpriteCoordReplace = static_cast<bool>(params[0]);
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void GetTextureEnv(unsigned int unit,
                   const GLES1State *state,
                   TextureEnvTarget target,
                   TextureEnvParameter pname,
                   GLfloat *params)
{
    const TextureEnvironmentParameters &env = state->textureEnvironment(unit);

    switch (target)
    {
        case TextureEnvTarget::Env:
            switch (pname)
            {
                case TextureEnvParameter::Mode:
                    ConvertPackedEnum(env.mode, params);
                    break;
                case TextureEnvParameter::CombineRgb:
                    ConvertPackedEnum(env.combineRgb, params);
                    break;
                case TextureEnvParameter::CombineAlpha:
                    ConvertPackedEnum(env.combineAlpha, params);
                    break;
                case TextureEnvParameter::Src0Rgb:
                    ConvertPackedEnum(env.src0Rgb, params);
                    break;
                case TextureEnvParameter::Src1Rgb:
                    ConvertPackedEnum(env.src1Rgb, params);
                    break;
                case TextureEnvParameter::Src2Rgb:
                    ConvertPackedEnum(env.src2Rgb, params);
                    break;
                case TextureEnvParameter::Src0Alpha:
                    ConvertPackedEnum(env.src0Alpha, params);
                    break;
                case TextureEnvParameter::Src1Alpha:
                    ConvertPackedEnum(env.src1Alpha, params);
                    break;
                case TextureEnvParameter::Src2Alpha:
                    ConvertPackedEnum(env.src2Alpha, params);
                    break;
                case TextureEnvParameter::Op0Rgb:
                    ConvertPackedEnum(env.op0Rgb, params);
                    break;
                case TextureEnvParameter::Op1Rgb:
                    ConvertPackedEnum(env.op1Rgb, params);
                    break;
                case TextureEnvParameter::Op2Rgb:
                    ConvertPackedEnum(env.op2Rgb, params);
                    break;
                case TextureEnvParameter::Op0Alpha:
                    ConvertPackedEnum(env.op0Alpha, params);
                    break;
                case TextureEnvParameter::Op1Alpha:
                    ConvertPackedEnum(env.op1Alpha, params);
                    break;
                case TextureEnvParameter::Op2Alpha:
                    ConvertPackedEnum(env.op2Alpha, params);
                    break;
                case TextureEnvParameter::Color:
                    env.color.writeData(params);
                    break;
                case TextureEnvParameter::RgbScale:
                    *params = env.rgbScale;
                    break;
                case TextureEnvParameter::AlphaScale:
                    *params = env.alphaScale;
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
            break;
        case TextureEnvTarget::PointSprite:
            switch (pname)
            {
                case TextureEnvParameter::PointCoordReplace:
                    *params = static_cast<GLfloat>(env.pointSpriteCoordReplace);
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
            break;
        default:
            UNREACHABLE();
            break;
    }
}

unsigned int GetPointParameterCount(PointParameter pname)
{
    switch (pname)
    {
        case PointParameter::PointSizeMin:
        case PointParameter::PointSizeMax:
        case PointParameter::PointFadeThresholdSize:
            return 1;
        case PointParameter::PointDistanceAttenuation:
            return 3;
        default:
            return 0;
    }
}

void SetPointParameter(GLES1State *state, PointParameter pname, const GLfloat *params)
{

    PointParameters &pointParams = state->pointParameters();

    switch (pname)
    {
        case PointParameter::PointSizeMin:
            pointParams.pointSizeMin = params[0];
            break;
        case PointParameter::PointSizeMax:
            pointParams.pointSizeMax = params[0];
            break;
        case PointParameter::PointFadeThresholdSize:
            pointParams.pointFadeThresholdSize = params[0];
            break;
        case PointParameter::PointDistanceAttenuation:
            for (unsigned int i = 0; i < 3; i++)
            {
                pointParams.pointDistanceAttenuation[i] = params[i];
            }
            break;
        default:
            UNREACHABLE();
    }
}

void GetPointParameter(const GLES1State *state, PointParameter pname, GLfloat *params)
{
    const PointParameters &pointParams = state->pointParameters();

    switch (pname)
    {
        case PointParameter::PointSizeMin:
            params[0] = pointParams.pointSizeMin;
            break;
        case PointParameter::PointSizeMax:
            params[0] = pointParams.pointSizeMax;
            break;
        case PointParameter::PointFadeThresholdSize:
            params[0] = pointParams.pointFadeThresholdSize;
            break;
        case PointParameter::PointDistanceAttenuation:
            for (unsigned int i = 0; i < 3; i++)
            {
                params[i] = pointParams.pointDistanceAttenuation[i];
            }
            break;
        default:
            UNREACHABLE();
    }
}

void SetPointSize(GLES1State *state, GLfloat size)
{
    PointParameters &params = state->pointParameters();
    params.pointSize        = size;
}

void GetPointSize(const GLES1State *state, GLfloat *sizeOut)
{
    const PointParameters &params = state->pointParameters();
    *sizeOut                      = params.pointSize;
}

unsigned int GetTexParameterCount(GLenum pname)
{
    switch (pname)
    {
        case GL_TEXTURE_CROP_RECT_OES:
        case GL_TEXTURE_BORDER_COLOR:
            return 4;
        case GL_TEXTURE_MAG_FILTER:
        case GL_TEXTURE_MIN_FILTER:
        case GL_TEXTURE_WRAP_S:
        case GL_TEXTURE_WRAP_T:
        case GL_TEXTURE_USAGE_ANGLE:
        case GL_TEXTURE_MAX_ANISOTROPY_EXT:
        case GL_TEXTURE_IMMUTABLE_FORMAT:
        case GL_TEXTURE_WRAP_R:
        case GL_TEXTURE_IMMUTABLE_LEVELS:
        case GL_TEXTURE_SWIZZLE_R:
        case GL_TEXTURE_SWIZZLE_G:
        case GL_TEXTURE_SWIZZLE_B:
        case GL_TEXTURE_SWIZZLE_A:
        case GL_TEXTURE_BASE_LEVEL:
        case GL_TEXTURE_MAX_LEVEL:
        case GL_TEXTURE_MIN_LOD:
        case GL_TEXTURE_MAX_LOD:
        case GL_TEXTURE_COMPARE_MODE:
        case GL_TEXTURE_COMPARE_FUNC:
        case GL_TEXTURE_SRGB_DECODE_EXT:
        case GL_DEPTH_STENCIL_TEXTURE_MODE:
        case GL_TEXTURE_NATIVE_ID_ANGLE:
        case GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES:
        case GL_RENDERABILITY_VALIDATION_ANGLE:
            return 1;
        default:
            return 0;
    }
}

bool GetQueryParameterInfo(const State &glState,
                           GLenum pname,
                           GLenum *type,
                           unsigned int *numParams)
{
    const Caps &caps             = glState.getCaps();
    const Extensions &extensions = glState.getExtensions();
    GLint clientMajorVersion     = glState.getClientMajorVersion();

    // Please note: the query type returned for DEPTH_CLEAR_VALUE in this implementation
    // is FLOAT rather than INT, as would be suggested by the GL ES 2.0 spec. This is due
    // to the fact that it is stored internally as a float, and so would require conversion
    // if returned from Context::getIntegerv. Since this conversion is already implemented
    // in the case that one calls glGetIntegerv to retrieve a float-typed state variable, we
    // place DEPTH_CLEAR_VALUE with the floats. This should make no difference to the calling
    // application.
    switch (pname)
    {
        case GL_COMPRESSED_TEXTURE_FORMATS:
        {
            *type      = GL_INT;
            *numParams = static_cast<unsigned int>(caps.compressedTextureFormats.size());
            return true;
        }
        case GL_SHADER_BINARY_FORMATS:
        {
            *type      = GL_INT;
            *numParams = static_cast<unsigned int>(caps.shaderBinaryFormats.size());
            return true;
        }

        case GL_MAX_VERTEX_ATTRIBS:
        case GL_MAX_VERTEX_UNIFORM_VECTORS:
        case GL_MAX_VARYING_VECTORS:
        case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
        case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
        case GL_MAX_TEXTURE_IMAGE_UNITS:
        case GL_MAX_FRAGMENT_UNIFORM_VECTORS:
        case GL_MAX_RENDERBUFFER_SIZE:
        case GL_NUM_SHADER_BINARY_FORMATS:
        case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
        case GL_ARRAY_BUFFER_BINDING:
        case GL_FRAMEBUFFER_BINDING:  // GL_FRAMEBUFFER_BINDING now equivalent to
                                      // GL_DRAW_FRAMEBUFFER_BINDING
        case GL_RENDERBUFFER_BINDING:
        case GL_CURRENT_PROGRAM:
        case GL_PACK_ALIGNMENT:
        case GL_UNPACK_ALIGNMENT:
        case GL_GENERATE_MIPMAP_HINT:
        case GL_RED_BITS:
        case GL_GREEN_BITS:
        case GL_BLUE_BITS:
        case GL_ALPHA_BITS:
        case GL_DEPTH_BITS:
        case GL_STENCIL_BITS:
        case GL_ELEMENT_ARRAY_BUFFER_BINDING:
        case GL_CULL_FACE_MODE:
        case GL_FRONT_FACE:
        case GL_ACTIVE_TEXTURE:
        case GL_STENCIL_FUNC:
        case GL_STENCIL_VALUE_MASK:
        case GL_STENCIL_REF:
        case GL_STENCIL_FAIL:
        case GL_STENCIL_PASS_DEPTH_FAIL:
        case GL_STENCIL_PASS_DEPTH_PASS:
        case GL_STENCIL_BACK_FUNC:
        case GL_STENCIL_BACK_VALUE_MASK:
        case GL_STENCIL_BACK_REF:
        case GL_STENCIL_BACK_FAIL:
        case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
        case GL_STENCIL_BACK_PASS_DEPTH_PASS:
        case GL_DEPTH_FUNC:
        case GL_BLEND_SRC_RGB:
        case GL_BLEND_SRC_ALPHA:
        case GL_BLEND_DST_RGB:
        case GL_BLEND_DST_ALPHA:
        case GL_BLEND_EQUATION_RGB:
        case GL_BLEND_EQUATION_ALPHA:
        case GL_STENCIL_WRITEMASK:
        case GL_STENCIL_BACK_WRITEMASK:
        case GL_STENCIL_CLEAR_VALUE:
        case GL_SUBPIXEL_BITS:
        case GL_MAX_TEXTURE_SIZE:
        case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
        case GL_SAMPLE_BUFFERS:
        case GL_SAMPLES:
        case GL_IMPLEMENTATION_COLOR_READ_TYPE:
        case GL_IMPLEMENTATION_COLOR_READ_FORMAT:
        case GL_TEXTURE_BINDING_2D:
        case GL_TEXTURE_BINDING_CUBE_MAP:
        case GL_RESET_NOTIFICATION_STRATEGY_EXT:
        case GL_QUERY_COUNTER_BITS_EXT:
        {
            *type      = GL_INT;
            *numParams = 1;
            return true;
        }
        case GL_PACK_REVERSE_ROW_ORDER_ANGLE:
        {
            if (!extensions.packReverseRowOrderANGLE)
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
        }
        case GL_MAX_RECTANGLE_TEXTURE_SIZE_ANGLE:
        case GL_TEXTURE_BINDING_RECTANGLE_ANGLE:
        {
            if (!extensions.textureRectangleANGLE)
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
        }
        case GL_MAX_DRAW_BUFFERS_EXT:
        case GL_MAX_COLOR_ATTACHMENTS_EXT:
        {
            if ((clientMajorVersion < 3) && !extensions.drawBuffersEXT)
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
        }
        case GL_BLEND_ADVANCED_COHERENT_KHR:
        {
            if (clientMajorVersion < 2 || !extensions.blendEquationAdvancedCoherentKHR)
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
        }
        case GL_MAX_VIEWPORT_DIMS:
        {
            *type      = GL_INT;
            *numParams = 2;
            return true;
        }
        case GL_VIEWPORT:
        case GL_SCISSOR_BOX:
        {
            *type      = GL_INT;
            *numParams = 4;
            return true;
        }
        case GL_SHADER_COMPILER:
        case GL_SAMPLE_COVERAGE_INVERT:
        case GL_DEPTH_WRITEMASK:
        case GL_CULL_FACE:                 // CULL_FACE through DITHER are natural to IsEnabled,
        case GL_POLYGON_OFFSET_FILL:       // but can be retrieved through the Get{Type}v queries.
        case GL_SAMPLE_ALPHA_TO_COVERAGE:  // For this purpose, they are treated here as
                                           // bool-natural
        case GL_SAMPLE_COVERAGE:
        case GL_SCISSOR_TEST:
        case GL_STENCIL_TEST:
        case GL_DEPTH_TEST:
        case GL_BLEND:
        case GL_DITHER:
        case GL_CONTEXT_ROBUST_ACCESS_EXT:
        {
            *type      = GL_BOOL;
            *numParams = 1;
            return true;
        }
        case GL_POLYGON_OFFSET_POINT_NV:
        {
            if (!extensions.polygonModeNV)
            {
                return false;
            }
            *type      = GL_BOOL;
            *numParams = 1;
            return true;
        }
        case GL_POLYGON_OFFSET_LINE_NV:  // = GL_POLYGON_OFFSET_LINE_ANGLE
        {
            if (!extensions.polygonModeAny())
            {
                return false;
            }
            *type      = GL_BOOL;
            *numParams = 1;
            return true;
        }
        case GL_DEPTH_CLAMP_EXT:
        {
            if (!extensions.depthClampEXT)
            {
                return false;
            }
            *type      = GL_BOOL;
            *numParams = 1;
            return true;
        }
        case GL_COLOR_LOGIC_OP:
        {
            if (clientMajorVersion == 1)
            {
                // Handle logicOp in GLES1 through GLES1 state management.
                break;
            }

            if (!extensions.logicOpANGLE)
            {
                return false;
            }
            *type      = GL_BOOL;
            *numParams = 1;
            return true;
        }
        case GL_COLOR_WRITEMASK:
        {
            *type      = GL_BOOL;
            *numParams = 4;
            return true;
        }
        case GL_POLYGON_OFFSET_FACTOR:
        case GL_POLYGON_OFFSET_UNITS:
        case GL_SAMPLE_COVERAGE_VALUE:
        case GL_DEPTH_CLEAR_VALUE:
        case GL_MULTISAMPLE_LINE_WIDTH_GRANULARITY:
        case GL_LINE_WIDTH:
        {
            *type      = GL_FLOAT;
            *numParams = 1;
            return true;
        }
        case GL_POLYGON_OFFSET_CLAMP_EXT:
            if (!extensions.polygonOffsetClampEXT)
            {
                return false;
            }
            *type      = GL_FLOAT;
            *numParams = 1;
            return true;
        case GL_ALIASED_LINE_WIDTH_RANGE:
        case GL_MULTISAMPLE_LINE_WIDTH_RANGE:
        case GL_ALIASED_POINT_SIZE_RANGE:
        case GL_DEPTH_RANGE:
        {
            *type      = GL_FLOAT;
            *numParams = 2;
            return true;
        }
        case GL_COLOR_CLEAR_VALUE:
        case GL_BLEND_COLOR:
        {
            *type      = GL_FLOAT;
            *numParams = 4;
            return true;
        }
        case GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT:
            if (!extensions.textureFilterAnisotropicEXT)
            {
                return false;
            }
            *type      = GL_FLOAT;
            *numParams = 1;
            return true;
        case GL_TIMESTAMP_EXT:
            if (!extensions.disjointTimerQueryEXT)
            {
                return false;
            }
            *type      = GL_INT_64_ANGLEX;
            *numParams = 1;
            return true;
        case GL_GPU_DISJOINT_EXT:
            if (!extensions.disjointTimerQueryEXT)
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
        case GL_COVERAGE_MODULATION_CHROMIUM:
            if (!extensions.framebufferMixedSamplesCHROMIUM)
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
        case GL_TEXTURE_BINDING_EXTERNAL_OES:
            if (!extensions.EGLStreamConsumerExternalNV && !extensions.EGLImageExternalOES)
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
        case GL_MAX_CLIP_DISTANCES_EXT:  // case GL_MAX_CLIP_PLANES
        case GL_CLIP_DISTANCE0_EXT:
        case GL_CLIP_DISTANCE1_EXT:
        case GL_CLIP_DISTANCE2_EXT:
        case GL_CLIP_DISTANCE3_EXT:
        case GL_CLIP_DISTANCE4_EXT:
        case GL_CLIP_DISTANCE5_EXT:
        case GL_CLIP_DISTANCE6_EXT:
        case GL_CLIP_DISTANCE7_EXT:
            if (clientMajorVersion < 2)
            {
                break;
            }
            if (!extensions.clipDistanceAPPLE && !extensions.clipCullDistanceAny())
            {
                // NOTE(hqle): if client version is 1. GL_MAX_CLIP_DISTANCES_EXT is equal
                // to GL_MAX_CLIP_PLANES which is a valid enum.
                return false;
            }
            *type      = (pname == GL_MAX_CLIP_DISTANCES_EXT) ? GL_INT : GL_BOOL;
            *numParams = 1;
            return true;
        case GL_MAX_CULL_DISTANCES_EXT:
        case GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES_EXT:
            if (!extensions.clipCullDistanceAny())
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
        case GL_CLIP_ORIGIN_EXT:
        case GL_CLIP_DEPTH_MODE_EXT:
            if (!extensions.clipControlEXT)
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
        case GL_POLYGON_MODE_NV:  // = GL_POLYGON_MODE_ANGLE
        {
            if (!extensions.polygonModeAny())
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
        }
        case GL_PRIMITIVE_BOUNDING_BOX:
            if (!extensions.primitiveBoundingBoxAny())
            {
                return false;
            }
            *type      = GL_FLOAT;
            *numParams = 8;
            return true;
        case GL_SHADING_RATE_QCOM:
            if (!extensions.shadingRateQCOM)
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
    }

    if (glState.getClientVersion() >= Version(3, 2))
    {
        switch (pname)
        {
            case GL_CONTEXT_FLAGS:
            {
                *type      = GL_INT;
                *numParams = 1;
                return true;
            }
        }
    }

    if (extensions.debugKHR)
    {
        switch (pname)
        {
            case GL_DEBUG_LOGGED_MESSAGES:
            case GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH:
            case GL_DEBUG_GROUP_STACK_DEPTH:
            case GL_MAX_DEBUG_MESSAGE_LENGTH:
            case GL_MAX_DEBUG_LOGGED_MESSAGES:
            case GL_MAX_DEBUG_GROUP_STACK_DEPTH:
            case GL_MAX_LABEL_LENGTH:
                *type      = GL_INT;
                *numParams = 1;
                return true;

            case GL_DEBUG_OUTPUT_SYNCHRONOUS:
            case GL_DEBUG_OUTPUT:
                *type      = GL_BOOL;
                *numParams = 1;
                return true;
        }
    }

    if (extensions.multisampleCompatibilityEXT)
    {
        switch (pname)
        {
            case GL_MULTISAMPLE_EXT:
            case GL_SAMPLE_ALPHA_TO_ONE_EXT:
                *type      = GL_BOOL;
                *numParams = 1;
                return true;
        }
    }

    if (extensions.bindGeneratesResourceCHROMIUM)
    {
        switch (pname)
        {
            case GL_BIND_GENERATES_RESOURCE_CHROMIUM:
                *type      = GL_BOOL;
                *numParams = 1;
                return true;
        }
    }

    if (extensions.clientArraysANGLE)
    {
        switch (pname)
        {
            case GL_CLIENT_ARRAYS_ANGLE:
                *type      = GL_BOOL;
                *numParams = 1;
                return true;
        }
    }

    if (extensions.sRGBWriteControlEXT)
    {
        switch (pname)
        {
            case GL_FRAMEBUFFER_SRGB_EXT:
                *type      = GL_BOOL;
                *numParams = 1;
                return true;
        }
    }

    if (extensions.robustResourceInitializationANGLE &&
        pname == GL_ROBUST_RESOURCE_INITIALIZATION_ANGLE)
    {
        *type      = GL_BOOL;
        *numParams = 1;
        return true;
    }

    if (extensions.programCacheControlANGLE && pname == GL_PROGRAM_CACHE_ENABLED_ANGLE)
    {
        *type      = GL_BOOL;
        *numParams = 1;
        return true;
    }

    if (extensions.parallelShaderCompileKHR && pname == GL_MAX_SHADER_COMPILER_THREADS_KHR)
    {
        *type      = GL_INT;
        *numParams = 1;
        return true;
    }

    if (extensions.blendFuncExtendedEXT && pname == GL_MAX_DUAL_SOURCE_DRAW_BUFFERS_EXT)
    {
        *type      = GL_INT;
        *numParams = 1;
        return true;
    }

    if (extensions.robustFragmentShaderOutputANGLE &&
        pname == GL_ROBUST_FRAGMENT_SHADER_OUTPUT_ANGLE)
    {
        *type      = GL_BOOL;
        *numParams = 1;
        return true;
    }

    // Check for ES3.0+ parameter names which are also exposed as ES2 extensions
    switch (pname)
    {
        // GL_DRAW_FRAMEBUFFER_BINDING equivalent to GL_FRAMEBUFFER_BINDING
        case GL_READ_FRAMEBUFFER_BINDING:
            if ((clientMajorVersion < 3) && !extensions.framebufferBlitAny())
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;

        case GL_NUM_PROGRAM_BINARY_FORMATS_OES:
            if ((clientMajorVersion < 3) && !extensions.getProgramBinaryOES)
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;

        case GL_PROGRAM_BINARY_FORMATS_OES:
            if ((clientMajorVersion < 3) && !extensions.getProgramBinaryOES)
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = static_cast<unsigned int>(caps.programBinaryFormats.size());
            return true;

        case GL_PACK_ROW_LENGTH:
        case GL_PACK_SKIP_ROWS:
        case GL_PACK_SKIP_PIXELS:
            if ((clientMajorVersion < 3) && !extensions.packSubimageNV)
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
        case GL_UNPACK_ROW_LENGTH:
        case GL_UNPACK_SKIP_ROWS:
        case GL_UNPACK_SKIP_PIXELS:
            if ((clientMajorVersion < 3) && !extensions.unpackSubimageEXT)
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
        case GL_VERTEX_ARRAY_BINDING:
            if ((clientMajorVersion < 3) && !extensions.vertexArrayObjectOES)
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
        case GL_PIXEL_PACK_BUFFER_BINDING:
        case GL_PIXEL_UNPACK_BUFFER_BINDING:
            if ((clientMajorVersion < 3) && !extensions.pixelBufferObjectNV)
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
        case GL_MAX_SAMPLES:
            static_assert(GL_MAX_SAMPLES_ANGLE == GL_MAX_SAMPLES,
                          "GL_MAX_SAMPLES_ANGLE not equal to GL_MAX_SAMPLES");
            if ((clientMajorVersion < 3) && !(extensions.framebufferMultisampleANGLE ||
                                              extensions.multisampledRenderToTextureEXT))
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
        case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
            if ((clientMajorVersion < 3) && !extensions.standardDerivativesOES)
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
        case GL_TEXTURE_BINDING_3D:
            if ((clientMajorVersion < 3) && !extensions.texture3DOES)
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
        case GL_MAX_3D_TEXTURE_SIZE:
            if ((clientMajorVersion < 3) && !extensions.texture3DOES)
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
    }

    if (pname >= GL_DRAW_BUFFER0_EXT && pname <= GL_DRAW_BUFFER15_EXT)
    {
        if ((glState.getClientVersion() < Version(3, 0)) && !extensions.drawBuffersEXT)
        {
            return false;
        }
        *type      = GL_INT;
        *numParams = 1;
        return true;
    }

    if ((extensions.multiview2OVR || extensions.multiviewOVR) && pname == GL_MAX_VIEWS_OVR)
    {
        *type      = GL_INT;
        *numParams = 1;
        return true;
    }

    if (extensions.provokingVertexANGLE && pname == GL_PROVOKING_VERTEX_ANGLE)
    {
        *type      = GL_INT;
        *numParams = 1;
        return true;
    }

    if (extensions.shaderFramebufferFetchARM &&
        (pname == GL_FETCH_PER_SAMPLE_ARM || pname == GL_FRAGMENT_SHADER_FRAMEBUFFER_FETCH_MRT_ARM))
    {
        *type      = GL_BOOL;
        *numParams = 1;
        return true;
    }

    if (glState.getClientVersion() < Version(2, 0))
    {
        switch (pname)
        {
            case GL_ALPHA_TEST_FUNC:
            case GL_CLIENT_ACTIVE_TEXTURE:
            case GL_MATRIX_MODE:
            case GL_MAX_TEXTURE_UNITS:
            case GL_MAX_MODELVIEW_STACK_DEPTH:
            case GL_MAX_PROJECTION_STACK_DEPTH:
            case GL_MAX_TEXTURE_STACK_DEPTH:
            case GL_MAX_LIGHTS:
            case GL_MAX_CLIP_PLANES:
            case GL_VERTEX_ARRAY_STRIDE:
            case GL_NORMAL_ARRAY_STRIDE:
            case GL_COLOR_ARRAY_STRIDE:
            case GL_TEXTURE_COORD_ARRAY_STRIDE:
            case GL_VERTEX_ARRAY_SIZE:
            case GL_COLOR_ARRAY_SIZE:
            case GL_TEXTURE_COORD_ARRAY_SIZE:
            case GL_VERTEX_ARRAY_TYPE:
            case GL_NORMAL_ARRAY_TYPE:
            case GL_COLOR_ARRAY_TYPE:
            case GL_TEXTURE_COORD_ARRAY_TYPE:
            case GL_VERTEX_ARRAY_BUFFER_BINDING:
            case GL_NORMAL_ARRAY_BUFFER_BINDING:
            case GL_COLOR_ARRAY_BUFFER_BINDING:
            case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING:
            case GL_POINT_SIZE_ARRAY_STRIDE_OES:
            case GL_POINT_SIZE_ARRAY_TYPE_OES:
            case GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES:
            case GL_SHADE_MODEL:
            case GL_MODELVIEW_STACK_DEPTH:
            case GL_PROJECTION_STACK_DEPTH:
            case GL_TEXTURE_STACK_DEPTH:
            case GL_LOGIC_OP_MODE:
            case GL_BLEND_SRC:
            case GL_BLEND_DST:
            case GL_PERSPECTIVE_CORRECTION_HINT:
            case GL_POINT_SMOOTH_HINT:
            case GL_LINE_SMOOTH_HINT:
            case GL_FOG_HINT:
                *type      = GL_INT;
                *numParams = 1;
                return true;
            case GL_ALPHA_TEST_REF:
            case GL_FOG_DENSITY:
            case GL_FOG_START:
            case GL_FOG_END:
            case GL_FOG_MODE:
            case GL_POINT_SIZE:
            case GL_POINT_SIZE_MIN:
            case GL_POINT_SIZE_MAX:
            case GL_POINT_FADE_THRESHOLD_SIZE:
                *type      = GL_FLOAT;
                *numParams = 1;
                return true;
            case GL_SMOOTH_POINT_SIZE_RANGE:
            case GL_SMOOTH_LINE_WIDTH_RANGE:
                *type      = GL_FLOAT;
                *numParams = 2;
                return true;
            case GL_CURRENT_COLOR:
            case GL_CURRENT_TEXTURE_COORDS:
            case GL_LIGHT_MODEL_AMBIENT:
            case GL_FOG_COLOR:
                *type      = GL_FLOAT;
                *numParams = 4;
                return true;
            case GL_CURRENT_NORMAL:
            case GL_POINT_DISTANCE_ATTENUATION:
                *type      = GL_FLOAT;
                *numParams = 3;
                return true;
            case GL_MODELVIEW_MATRIX:
            case GL_PROJECTION_MATRIX:
            case GL_TEXTURE_MATRIX:
                *type      = GL_FLOAT;
                *numParams = 16;
                return true;
            case GL_ALPHA_TEST:
            case GL_CLIP_PLANE0:
            case GL_CLIP_PLANE1:
            case GL_CLIP_PLANE2:
            case GL_CLIP_PLANE3:
            case GL_CLIP_PLANE4:
            case GL_CLIP_PLANE5:
            case GL_COLOR_ARRAY:
            case GL_COLOR_LOGIC_OP:
            case GL_COLOR_MATERIAL:
            case GL_FOG:
            case GL_LIGHT_MODEL_TWO_SIDE:
            case GL_LIGHT0:
            case GL_LIGHT1:
            case GL_LIGHT2:
            case GL_LIGHT3:
            case GL_LIGHT4:
            case GL_LIGHT5:
            case GL_LIGHT6:
            case GL_LIGHT7:
            case GL_LIGHTING:
            case GL_LINE_SMOOTH:
            case GL_NORMAL_ARRAY:
            case GL_NORMALIZE:
            case GL_POINT_SIZE_ARRAY_OES:
            case GL_POINT_SMOOTH:
            case GL_POINT_SPRITE_OES:
            case GL_RESCALE_NORMAL:
            case GL_TEXTURE_2D:
            case GL_TEXTURE_CUBE_MAP:
            case GL_TEXTURE_COORD_ARRAY:
            case GL_VERTEX_ARRAY:
                *type      = GL_BOOL;
                *numParams = 1;
                return true;
        }
    }

    if (glState.getClientVersion() < Version(3, 0))
    {
        return false;
    }

    // Check for ES3.0+ parameter names
    switch (pname)
    {
        case GL_MAX_UNIFORM_BUFFER_BINDINGS:
        case GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT:
        case GL_UNIFORM_BUFFER_BINDING:
        case GL_TRANSFORM_FEEDBACK_BINDING:
        case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
        case GL_COPY_READ_BUFFER_BINDING:
        case GL_COPY_WRITE_BUFFER_BINDING:
        case GL_SAMPLER_BINDING:
        case GL_READ_BUFFER:
        case GL_TEXTURE_BINDING_3D:
        case GL_TEXTURE_BINDING_2D_ARRAY:
        case GL_MAX_ARRAY_TEXTURE_LAYERS:
        case GL_MAX_VERTEX_UNIFORM_BLOCKS:
        case GL_MAX_FRAGMENT_UNIFORM_BLOCKS:
        case GL_MAX_COMBINED_UNIFORM_BLOCKS:
        case GL_MAX_VERTEX_OUTPUT_COMPONENTS:
        case GL_MAX_FRAGMENT_INPUT_COMPONENTS:
        case GL_MAX_VARYING_COMPONENTS:
        case GL_MAX_VERTEX_UNIFORM_COMPONENTS:
        case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:
        case GL_MIN_PROGRAM_TEXEL_OFFSET:
        case GL_MAX_PROGRAM_TEXEL_OFFSET:
        case GL_NUM_EXTENSIONS:
        case GL_MAJOR_VERSION:
        case GL_MINOR_VERSION:
        case GL_MAX_ELEMENTS_INDICES:
        case GL_MAX_ELEMENTS_VERTICES:
        case GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS:
        case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS:
        case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS:
        case GL_UNPACK_IMAGE_HEIGHT:
        case GL_UNPACK_SKIP_IMAGES:
        {
            *type      = GL_INT;
            *numParams = 1;
            return true;
        }

        case GL_MAX_ELEMENT_INDEX:
        case GL_MAX_UNIFORM_BLOCK_SIZE:
        case GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS:
        case GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS:
        case GL_MAX_SERVER_WAIT_TIMEOUT:
        {
            *type      = GL_INT_64_ANGLEX;
            *numParams = 1;
            return true;
        }

        case GL_TRANSFORM_FEEDBACK_ACTIVE:
        case GL_TRANSFORM_FEEDBACK_PAUSED:
        case GL_PRIMITIVE_RESTART_FIXED_INDEX:
        case GL_RASTERIZER_DISCARD:
        {
            *type      = GL_BOOL;
            *numParams = 1;
            return true;
        }

        case GL_MAX_TEXTURE_LOD_BIAS:
        {
            *type      = GL_FLOAT;
            *numParams = 1;
            return true;
        }
    }

    if (extensions.shaderMultisampleInterpolationOES)
    {
        switch (pname)
        {
            case GL_MIN_FRAGMENT_INTERPOLATION_OFFSET_OES:
            case GL_MAX_FRAGMENT_INTERPOLATION_OFFSET_OES:
            {
                *type      = GL_FLOAT;
                *numParams = 1;
                return true;
            }
            case GL_FRAGMENT_INTERPOLATION_OFFSET_BITS_OES:
            {
                *type      = GL_INT;
                *numParams = 1;
                return true;
            }
        }
    }

    if (extensions.requestExtensionANGLE)
    {
        switch (pname)
        {
            case GL_NUM_REQUESTABLE_EXTENSIONS_ANGLE:
                *type      = GL_INT;
                *numParams = 1;
                return true;
        }
    }

    if (glState.getClientVersion() >= Version(3, 1) || extensions.textureMultisampleANGLE)
    {
        static_assert(GL_SAMPLE_MASK_ANGLE == GL_SAMPLE_MASK);
        static_assert(GL_MAX_SAMPLE_MASK_WORDS_ANGLE == GL_MAX_SAMPLE_MASK_WORDS);
        static_assert(GL_MAX_COLOR_TEXTURE_SAMPLES_ANGLE == GL_MAX_COLOR_TEXTURE_SAMPLES);
        static_assert(GL_MAX_DEPTH_TEXTURE_SAMPLES_ANGLE == GL_MAX_DEPTH_TEXTURE_SAMPLES);
        static_assert(GL_MAX_INTEGER_SAMPLES_ANGLE == GL_MAX_INTEGER_SAMPLES);
        static_assert(GL_TEXTURE_BINDING_2D_MULTISAMPLE_ANGLE == GL_TEXTURE_BINDING_2D_MULTISAMPLE);

        switch (pname)
        {
            case GL_SAMPLE_MASK:
                *type      = GL_BOOL;
                *numParams = 1;
                return true;
            case GL_MAX_SAMPLE_MASK_WORDS:
            case GL_MAX_COLOR_TEXTURE_SAMPLES:
            case GL_MAX_DEPTH_TEXTURE_SAMPLES:
            case GL_MAX_INTEGER_SAMPLES:
            case GL_TEXTURE_BINDING_2D_MULTISAMPLE:
                *type      = GL_INT;
                *numParams = 1;
                return true;
        }
    }

    if (extensions.textureCubeMapArrayAny())
    {
        switch (pname)
        {
            case GL_TEXTURE_BINDING_CUBE_MAP_ARRAY:
                *type      = GL_INT;
                *numParams = 1;
                return true;
        }
    }

    if (extensions.textureBufferAny())
    {
        switch (pname)
        {
            case GL_TEXTURE_BUFFER_BINDING:
            case GL_TEXTURE_BINDING_BUFFER:
            case GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT:
            case GL_MAX_TEXTURE_BUFFER_SIZE:
                *type      = GL_INT;
                *numParams = 1;
                return true;
        }
    }

    if (extensions.shaderPixelLocalStorageANGLE)
    {
        switch (pname)
        {
            case GL_MAX_PIXEL_LOCAL_STORAGE_PLANES_ANGLE:
            case GL_MAX_COLOR_ATTACHMENTS_WITH_ACTIVE_PIXEL_LOCAL_STORAGE_ANGLE:
            case GL_MAX_COMBINED_DRAW_BUFFERS_AND_PIXEL_LOCAL_STORAGE_PLANES_ANGLE:
            case GL_PIXEL_LOCAL_STORAGE_ACTIVE_PLANES_ANGLE:
                *type      = GL_INT;
                *numParams = 1;
                return true;
        }
    }

    if (glState.getClientVersion() >= Version(3, 2) ||
        extensions.textureStorageMultisample2dArrayOES)
    {
        switch (pname)
        {
            case GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY:
                *type      = GL_INT;
                *numParams = 1;
                return true;
        }
    }

    if (glState.getClientVersion() < Version(3, 1))
    {
        return false;
    }

    // Check for ES3.1+ parameter names
    switch (pname)
    {
        case GL_ATOMIC_COUNTER_BUFFER_BINDING:
        case GL_DRAW_INDIRECT_BUFFER_BINDING:
        case GL_DISPATCH_INDIRECT_BUFFER_BINDING:
        case GL_MAX_FRAMEBUFFER_WIDTH:
        case GL_MAX_FRAMEBUFFER_HEIGHT:
        case GL_MAX_FRAMEBUFFER_SAMPLES:
        case GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET:
        case GL_MAX_VERTEX_ATTRIB_BINDINGS:
        case GL_MAX_VERTEX_ATTRIB_STRIDE:
        case GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS:
        case GL_MAX_VERTEX_ATOMIC_COUNTERS:
        case GL_MAX_VERTEX_IMAGE_UNIFORMS:
        case GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS:
        case GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS:
        case GL_MAX_FRAGMENT_ATOMIC_COUNTERS:
        case GL_MAX_FRAGMENT_IMAGE_UNIFORMS:
        case GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS:
        case GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET:
        case GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET:
        case GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS:
        case GL_MAX_COMPUTE_UNIFORM_BLOCKS:
        case GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS:
        case GL_MAX_COMPUTE_SHARED_MEMORY_SIZE:
        case GL_MAX_COMPUTE_UNIFORM_COMPONENTS:
        case GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS:
        case GL_MAX_COMPUTE_ATOMIC_COUNTERS:
        case GL_MAX_COMPUTE_IMAGE_UNIFORMS:
        case GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS:
        case GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS:
        case GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES:
        case GL_MAX_UNIFORM_LOCATIONS:
        case GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS:
        case GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE:
        case GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS:
        case GL_MAX_COMBINED_ATOMIC_COUNTERS:
        case GL_MAX_IMAGE_UNITS:
        case GL_MAX_COMBINED_IMAGE_UNIFORMS:
        case GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS:
        case GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS:
        case GL_SHADER_STORAGE_BUFFER_BINDING:
        case GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT:
        case GL_PROGRAM_PIPELINE_BINDING:
            *type      = GL_INT;
            *numParams = 1;
            return true;
        case GL_MAX_SHADER_STORAGE_BLOCK_SIZE:
            *type      = GL_INT_64_ANGLEX;
            *numParams = 1;
            return true;
        case GL_SAMPLE_SHADING:
            *type      = GL_BOOL;
            *numParams = 1;
            return true;
        case GL_MIN_SAMPLE_SHADING_VALUE:
            *type      = GL_FLOAT;
            *numParams = 1;
            return true;
    }

    if (extensions.geometryShaderAny())
    {
        switch (pname)
        {
            case GL_MAX_FRAMEBUFFER_LAYERS_EXT:
            case GL_LAYER_PROVOKING_VERTEX_EXT:
            case GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT:
            case GL_MAX_GEOMETRY_UNIFORM_BLOCKS_EXT:
            case GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS_EXT:
            case GL_MAX_GEOMETRY_INPUT_COMPONENTS_EXT:
            case GL_MAX_GEOMETRY_OUTPUT_COMPONENTS_EXT:
            case GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT:
            case GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT:
            case GL_MAX_GEOMETRY_SHADER_INVOCATIONS_EXT:
            case GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT:
            case GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS_EXT:
            case GL_MAX_GEOMETRY_ATOMIC_COUNTERS_EXT:
            case GL_MAX_GEOMETRY_IMAGE_UNIFORMS_EXT:
            case GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS_EXT:
                *type      = GL_INT;
                *numParams = 1;
                return true;
        }
    }

    if (extensions.tessellationShaderAny())
    {
        switch (pname)
        {
            case GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED:
                *type      = GL_BOOL;
                *numParams = 1;
                return true;
            case GL_PATCH_VERTICES:
            case GL_MAX_PATCH_VERTICES_EXT:
            case GL_MAX_TESS_GEN_LEVEL_EXT:
            case GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS_EXT:
            case GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS_EXT:
            case GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS_EXT:
            case GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS_EXT:
            case GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS_EXT:
            case GL_MAX_TESS_PATCH_COMPONENTS_EXT:
            case GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS_EXT:
            case GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS_EXT:
            case GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS_EXT:
            case GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS_EXT:
            case GL_MAX_TESS_CONTROL_INPUT_COMPONENTS_EXT:
            case GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS_EXT:
            case GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS_EXT:
            case GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS_EXT:
            case GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS_EXT:
            case GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS_EXT:
            case GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS_EXT:
            case GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS_EXT:
            case GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS_EXT:
            case GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS_EXT:
            case GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS_EXT:
            case GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS_EXT:
                *type      = GL_INT;
                *numParams = 1;
                return true;
        }
    }

    return false;
}

bool GetIndexedQueryParameterInfo(const State &glState,
                                  GLenum target,
                                  GLenum *type,
                                  unsigned int *numParams)
{
    const Extensions &extensions = glState.getExtensions();
    const Version &clientVersion = glState.getClientVersion();

    ASSERT(clientVersion >= ES_3_0);

    switch (target)
    {
        case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
        case GL_UNIFORM_BUFFER_BINDING:
        {
            *type      = GL_INT;
            *numParams = 1;
            return true;
        }
        case GL_TRANSFORM_FEEDBACK_BUFFER_START:
        case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
        case GL_UNIFORM_BUFFER_START:
        case GL_UNIFORM_BUFFER_SIZE:
        {
            *type      = GL_INT_64_ANGLEX;
            *numParams = 1;
            return true;
        }
    }

    if (clientVersion >= ES_3_1 || extensions.textureMultisampleANGLE)
    {
        static_assert(GL_SAMPLE_MASK_VALUE_ANGLE == GL_SAMPLE_MASK_VALUE);
        switch (target)
        {
            case GL_SAMPLE_MASK_VALUE:
            {
                *type      = GL_INT;
                *numParams = 1;
                return true;
            }
        }
    }

    if (clientVersion >= ES_3_2 || extensions.drawBuffersIndexedAny())
    {
        switch (target)
        {
            case GL_BLEND_SRC_RGB:
            case GL_BLEND_SRC_ALPHA:
            case GL_BLEND_DST_RGB:
            case GL_BLEND_DST_ALPHA:
            case GL_BLEND_EQUATION_RGB:
            case GL_BLEND_EQUATION_ALPHA:
            {
                *type      = GL_INT;
                *numParams = 1;
                return true;
            }
            case GL_COLOR_WRITEMASK:
            {
                *type      = GL_BOOL;
                *numParams = 4;
                return true;
            }
        }
    }

    if (clientVersion < ES_3_1)
    {
        return false;
    }

    switch (target)
    {
        case GL_IMAGE_BINDING_LAYERED:
        {
            *type      = GL_BOOL;
            *numParams = 1;
            return true;
        }
        case GL_MAX_COMPUTE_WORK_GROUP_COUNT:
        case GL_MAX_COMPUTE_WORK_GROUP_SIZE:
        case GL_ATOMIC_COUNTER_BUFFER_BINDING:
        case GL_SHADER_STORAGE_BUFFER_BINDING:
        case GL_VERTEX_BINDING_BUFFER:
        case GL_VERTEX_BINDING_DIVISOR:
        case GL_VERTEX_BINDING_OFFSET:
        case GL_VERTEX_BINDING_STRIDE:
        case GL_IMAGE_BINDING_NAME:
        case GL_IMAGE_BINDING_LEVEL:
        case GL_IMAGE_BINDING_LAYER:
        case GL_IMAGE_BINDING_ACCESS:
        case GL_IMAGE_BINDING_FORMAT:
        {
            *type      = GL_INT;
            *numParams = 1;
            return true;
        }
        case GL_ATOMIC_COUNTER_BUFFER_START:
        case GL_ATOMIC_COUNTER_BUFFER_SIZE:
        case GL_SHADER_STORAGE_BUFFER_START:
        case GL_SHADER_STORAGE_BUFFER_SIZE:
        {
            *type      = GL_INT_64_ANGLEX;
            *numParams = 1;
            return true;
        }
    }

    return false;
}

void QueryProgramPipelineiv(const Context *context,
                            ProgramPipeline *programPipeline,
                            GLenum pname,
                            GLint *params)
{
    if (!params)
    {
        // Can't write the result anywhere, so just return immediately.
        return;
    }

    switch (pname)
    {
        case GL_ACTIVE_PROGRAM:
        {
            // the name of the active program object of the program pipeline object is returned in
            // params
            *params = 0;
            if (programPipeline)
            {
                const Program *program = programPipeline->getActiveShaderProgram();
                if (program)
                {
                    *params = program->id().value;
                }
            }
            break;
        }

        case GL_VERTEX_SHADER:
        {
            // the name of the current program object for the vertex shader type of the program
            // pipeline object is returned in params
            GetShaderProgramId(programPipeline, ShaderType::Vertex, params);
            break;
        }

        case GL_FRAGMENT_SHADER:
        {
            // the name of the current program object for the fragment shader type of the program
            // pipeline object is returned in params
            GetShaderProgramId(programPipeline, ShaderType::Fragment, params);
            break;
        }

        case GL_TESS_CONTROL_SHADER:
        {
            // the name of the current program object for the tessellation control shader type of
            // the program pipeline object is returned in params
            GetShaderProgramId(programPipeline, ShaderType::TessControl, params);
            break;
        }

        case GL_TESS_EVALUATION_SHADER:
        {
            // the name of the current program object for the tessellation evaluation shader type of
            // the program pipeline object is returned in params
            GetShaderProgramId(programPipeline, ShaderType::TessEvaluation, params);
            break;
        }

        case GL_COMPUTE_SHADER:
        {
            // the name of the current program object for the compute shader type of the program
            // pipeline object is returned in params
            GetShaderProgramId(programPipeline, ShaderType::Compute, params);
            break;
        }

        case GL_GEOMETRY_SHADER:
        {
            // the name of the current program object for the geometry shader type of the program
            // pipeline object is returned in params
            GetShaderProgramId(programPipeline, ShaderType::Geometry, params);
            break;
        }

        case GL_INFO_LOG_LENGTH:
        {
            // the length of the info log, including the null terminator, is returned in params. If
            // there is no info log, zero is returned.
            *params = 0;
            if (programPipeline)
            {
                *params = programPipeline->getInfoLogLength();
            }
            break;
        }

        case GL_VALIDATE_STATUS:
        {
            // the validation status of pipeline, as determined by glValidateProgramPipeline, is
            // returned in params
            *params = 0;
            if (programPipeline)
            {
                *params = programPipeline->isValid();
            }
            break;
        }

        default:
            break;
    }
}

}  // namespace gl

namespace egl
{

void QueryConfigAttrib(const Config *config, EGLint attribute, EGLint *value)
{
    ASSERT(config != nullptr);
    switch (attribute)
    {
        case EGL_BUFFER_SIZE:
            *value = config->bufferSize;
            break;
        case EGL_ALPHA_SIZE:
            *value = config->alphaSize;
            break;
        case EGL_BLUE_SIZE:
            *value = config->blueSize;
            break;
        case EGL_GREEN_SIZE:
            *value = config->greenSize;
            break;
        case EGL_RED_SIZE:
            *value = config->redSize;
            break;
        case EGL_DEPTH_SIZE:
            *value = config->depthSize;
            break;
        case EGL_STENCIL_SIZE:
            *value = config->stencilSize;
            break;
        case EGL_CONFIG_CAVEAT:
            *value = config->configCaveat;
            break;
        case EGL_CONFIG_ID:
            *value = config->configID;
            break;
        case EGL_LEVEL:
            *value = config->level;
            break;
        case EGL_NATIVE_RENDERABLE:
            *value = config->nativeRenderable;
            break;
        case EGL_NATIVE_VISUAL_ID:
            *value = config->nativeVisualID;
            break;
        case EGL_NATIVE_VISUAL_TYPE:
            *value = config->nativeVisualType;
            break;
        case EGL_SAMPLES:
            *value = config->samples;
            break;
        case EGL_SAMPLE_BUFFERS:
            *value = config->sampleBuffers;
            break;
        case EGL_SURFACE_TYPE:
            *value = config->surfaceType;
            break;
        case EGL_BIND_TO_TEXTURE_TARGET_ANGLE:
            *value = config->bindToTextureTarget;
            break;
        case EGL_TRANSPARENT_TYPE:
            *value = config->transparentType;
            break;
        case EGL_TRANSPARENT_BLUE_VALUE:
            *value = config->transparentBlueValue;
            break;
        case EGL_TRANSPARENT_GREEN_VALUE:
            *value = config->transparentGreenValue;
            break;
        case EGL_TRANSPARENT_RED_VALUE:
            *value = config->transparentRedValue;
            break;
        case EGL_BIND_TO_TEXTURE_RGB:
            *value = config->bindToTextureRGB;
            break;
        case EGL_BIND_TO_TEXTURE_RGBA:
            *value = config->bindToTextureRGBA;
            break;
        case EGL_MIN_SWAP_INTERVAL:
            *value = config->minSwapInterval;
            break;
        case EGL_MAX_SWAP_INTERVAL:
            *value = config->maxSwapInterval;
            break;
        case EGL_LUMINANCE_SIZE:
            *value = config->luminanceSize;
            break;
        case EGL_ALPHA_MASK_SIZE:
            *value = config->alphaMaskSize;
            break;
        case EGL_COLOR_BUFFER_TYPE:
            *value = config->colorBufferType;
            break;
        case EGL_RENDERABLE_TYPE:
            *value = config->renderableType;
            break;
        case EGL_MATCH_NATIVE_PIXMAP:
            *value = false;
            UNIMPLEMENTED();
            break;
        case EGL_CONFORMANT:
            *value = config->conformant;
            break;
        case EGL_MAX_PBUFFER_WIDTH:
            *value = config->maxPBufferWidth;
            break;
        case EGL_MAX_PBUFFER_HEIGHT:
            *value = config->maxPBufferHeight;
            break;
        case EGL_MAX_PBUFFER_PIXELS:
            *value = config->maxPBufferPixels;
            break;
        case EGL_OPTIMAL_SURFACE_ORIENTATION_ANGLE:
            *value = config->optimalOrientation;
            break;
        case EGL_COLOR_COMPONENT_TYPE_EXT:
            *value = config->colorComponentType;
            break;
        case EGL_RECORDABLE_ANDROID:
            *value = config->recordable;
            break;
        case EGL_FRAMEBUFFER_TARGET_ANDROID:
            *value = config->framebufferTarget;
            break;
        case EGL_MATCH_FORMAT_KHR:
            *value = config->matchFormat;
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void QueryContextAttrib(const gl::Context *context, EGLint attribute, EGLint *value)
{
    switch (attribute)
    {
        case EGL_CONFIG_ID:
            if (context->getConfig() != EGL_NO_CONFIG_KHR)
            {
                *value = context->getConfig()->configID;
            }
            else
            {
                *value = 0;
            }
            break;
        case EGL_CONTEXT_CLIENT_TYPE:
            *value = EGL_OPENGL_ES_API;
            break;
        case EGL_CONTEXT_MAJOR_VERSION:
            static_assert(EGL_CONTEXT_MAJOR_VERSION == EGL_CONTEXT_CLIENT_VERSION);
            *value = context->getClientMajorVersion();
            break;
        case EGL_CONTEXT_MINOR_VERSION:
            *value = context->getClientMinorVersion();
            break;
        case EGL_RENDER_BUFFER:
            *value = context->getRenderBuffer();
            break;
        case EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE:
            *value = context->isRobustResourceInitEnabled();
            break;
        case EGL_CONTEXT_PRIORITY_LEVEL_IMG:
            *value = static_cast<EGLint>(context->getContextPriority());
            break;
        case EGL_PROTECTED_CONTENT_EXT:
            *value = context->getState().hasProtectedContent();
            break;
        case EGL_CONTEXT_MEMORY_USAGE_ANGLE:
        {
            uint64_t memory = context->getMemoryUsage();
            value[0]        = static_cast<GLint>(memory & 0xffffffff);
            value[1]        = static_cast<GLint>(memory >> 32);
        }
        break;
        default:
            UNREACHABLE();
            break;
    }
}

egl::Error QuerySurfaceAttrib(const Display *display,
                              const gl::Context *context,
                              Surface *surface,
                              EGLint attribute,
                              EGLint *value)
{
    switch (attribute)
    {
        case EGL_GL_COLORSPACE:
            *value = surface->getGLColorspace();
            break;
        case EGL_VG_ALPHA_FORMAT:
            *value = surface->getVGAlphaFormat();
            break;
        case EGL_VG_COLORSPACE:
            *value = surface->getVGColorspace();
            break;
        case EGL_CONFIG_ID:
            *value = surface->getConfig()->configID;
            break;
        case EGL_HEIGHT:
            ANGLE_TRY(surface->getUserHeight(display, value));
            break;
        case EGL_HORIZONTAL_RESOLUTION:
            *value = surface->getHorizontalResolution();
            break;
        case EGL_LARGEST_PBUFFER:
            // The EGL spec states that value is not written if the surface is not a pbuffer
            if (surface->getType() == EGL_PBUFFER_BIT)
            {
                *value = surface->getLargestPbuffer();
            }
            break;
        case EGL_MIPMAP_TEXTURE:
            // The EGL spec states that value is not written if the surface is not a pbuffer
            if (surface->getType() == EGL_PBUFFER_BIT)
            {
                *value = surface->getMipmapTexture();
            }
            break;
        case EGL_MIPMAP_LEVEL:
            // The EGL spec states that value is not written if the surface is not a pbuffer
            if (surface->getType() == EGL_PBUFFER_BIT)
            {
                *value = surface->getMipmapLevel();
            }
            break;
        case EGL_MULTISAMPLE_RESOLVE:
            *value = surface->getMultisampleResolve();
            break;
        case EGL_PIXEL_ASPECT_RATIO:
            *value = surface->getPixelAspectRatio();
            break;
        case EGL_RENDER_BUFFER:
            if (surface->getType() == EGL_WINDOW_BIT)
            {
                *value = surface->getRequestedRenderBuffer();
            }
            else
            {
                *value = surface->getRenderBuffer();
            }
            break;
        case EGL_SWAP_BEHAVIOR:
            *value = surface->getSwapBehavior();
            break;
        case EGL_TEXTURE_FORMAT:
            // The EGL spec states that value is not written if the surface is not a pbuffer
            if (surface->getType() == EGL_PBUFFER_BIT)
            {
                *value = ToEGLenum(surface->getTextureFormat());
            }
            break;
        case EGL_TEXTURE_TARGET:
            // The EGL spec states that value is not written if the surface is not a pbuffer
            if (surface->getType() == EGL_PBUFFER_BIT)
            {
                *value = surface->getTextureTarget();
            }
            break;
        case EGL_VERTICAL_RESOLUTION:
            *value = surface->getVerticalResolution();
            break;
        case EGL_WIDTH:
            ANGLE_TRY(surface->getUserWidth(display, value));
            break;
        case EGL_POST_SUB_BUFFER_SUPPORTED_NV:
            *value = surface->isPostSubBufferSupported();
            break;
        case EGL_FIXED_SIZE_ANGLE:
            *value = surface->isFixedSize();
            break;
        case EGL_SURFACE_ORIENTATION_ANGLE:
            *value = surface->getOrientation();
            break;
        case EGL_DIRECT_COMPOSITION_ANGLE:
            *value = surface->directComposition();
            break;
        case EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE:
            *value = surface->isRobustResourceInitEnabled();
            break;
        case EGL_TIMESTAMPS_ANDROID:
            *value = surface->isTimestampsEnabled();
            break;
        case EGL_BUFFER_AGE_EXT:
            ANGLE_TRY(surface->getBufferAge(context, value));
            break;
        case EGL_BITMAP_PITCH_KHR:
            *value = surface->getBitmapPitch();
            break;
        case EGL_BITMAP_ORIGIN_KHR:
            *value = surface->getBitmapOrigin();
            break;
        case EGL_BITMAP_PIXEL_RED_OFFSET_KHR:
            *value = surface->getRedOffset();
            break;
        case EGL_BITMAP_PIXEL_GREEN_OFFSET_KHR:
            *value = surface->getGreenOffset();
            break;
        case EGL_BITMAP_PIXEL_BLUE_OFFSET_KHR:
            *value = surface->getBlueOffset();
            break;
        case EGL_BITMAP_PIXEL_ALPHA_OFFSET_KHR:
            *value = surface->getAlphaOffset();
            break;
        case EGL_BITMAP_PIXEL_LUMINANCE_OFFSET_KHR:
            *value = surface->getLuminanceOffset();
            break;
        case EGL_BITMAP_PIXEL_SIZE_KHR:
            *value = surface->getBitmapPixelSize();
            break;
        case EGL_PROTECTED_CONTENT_EXT:
            *value = surface->hasProtectedContent();
            break;
        case EGL_SURFACE_COMPRESSION_EXT:
            *value = surface->getCompressionRate(display);
            break;
        default:
            UNREACHABLE();
            break;
    }
    return NoError();
}

egl::Error QuerySurfaceAttrib64KHR(const Display *display,
                                   const gl::Context *context,
                                   Surface *surface,
                                   EGLint attribute,
                                   EGLAttribKHR *value)
{
    switch (attribute)
    {
        case EGL_BITMAP_PITCH_KHR:
            *value = static_cast<EGLAttribKHR>(surface->getBitmapPitch());
            break;
        case EGL_BITMAP_ORIGIN_KHR:
            *value = static_cast<EGLAttribKHR>(surface->getBitmapOrigin());
            break;
        case EGL_BITMAP_PIXEL_RED_OFFSET_KHR:
            *value = static_cast<EGLAttribKHR>(surface->getRedOffset());
            break;
        case EGL_BITMAP_PIXEL_GREEN_OFFSET_KHR:
            *value = static_cast<EGLAttribKHR>(surface->getGreenOffset());
            break;
        case EGL_BITMAP_PIXEL_BLUE_OFFSET_KHR:
            *value = static_cast<EGLAttribKHR>(surface->getBlueOffset());
            break;
        case EGL_BITMAP_PIXEL_ALPHA_OFFSET_KHR:
            *value = static_cast<EGLAttribKHR>(surface->getAlphaOffset());
            break;
        case EGL_BITMAP_PIXEL_LUMINANCE_OFFSET_KHR:
            *value = static_cast<EGLAttribKHR>(surface->getLuminanceOffset());
            break;
        case EGL_BITMAP_PIXEL_SIZE_KHR:
            *value = static_cast<EGLAttribKHR>(surface->getBitmapPixelSize());
            break;
        case EGL_BITMAP_POINTER_KHR:
            *value = surface->getBitmapPointer();
            break;
        default:
        {
            EGLint intValue = 0;
            ANGLE_TRY(QuerySurfaceAttrib(display, context, surface, attribute, &intValue));
            *value = static_cast<EGLAttribKHR>(intValue);
        }
        break;
    }
    return NoError();
}

egl::Error SetSurfaceAttrib(Surface *surface, EGLint attribute, EGLint value)
{
    switch (attribute)
    {
        case EGL_MIPMAP_LEVEL:
            surface->setMipmapLevel(value);
            break;
        case EGL_MULTISAMPLE_RESOLVE:
            surface->setMultisampleResolve(value);
            break;
        case EGL_SWAP_BEHAVIOR:
            surface->setSwapBehavior(value);
            break;
        case EGL_WIDTH:
            surface->setFixedWidth(value);
            break;
        case EGL_HEIGHT:
            surface->setFixedHeight(value);
            break;
        case EGL_TIMESTAMPS_ANDROID:
            surface->setTimestampsEnabled(value != EGL_FALSE);
            break;
        case EGL_FRONT_BUFFER_AUTO_REFRESH_ANDROID:
            return surface->setAutoRefreshEnabled(value != EGL_FALSE);
        case EGL_RENDER_BUFFER:
            surface->setRequestedRenderBuffer(value);
            break;
        default:
            UNREACHABLE();
            break;
    }
    return NoError();
}

Error GetSyncAttrib(Display *display, SyncID sync, EGLint attribute, EGLint *value)
{
    const egl::Sync *syncObj = display->getSync(sync);
    switch (attribute)
    {
        case EGL_SYNC_TYPE_KHR:
            *value = syncObj->getType();
            return NoError();

        case EGL_SYNC_STATUS_KHR:
            return syncObj->getStatus(display, value);

        case EGL_SYNC_CONDITION_KHR:
            *value = syncObj->getCondition();
            return NoError();

        default:
            break;
    }

    UNREACHABLE();
    return NoError();
}
}  // namespace egl
