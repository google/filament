//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationES1.cpp: Validation functions for OpenGL ES 1.0 entry point parameters

#include "libANGLE/validationES1_autogen.h"

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/ErrorStrings.h"
#include "libANGLE/GLES1State.h"
#include "libANGLE/queryconversions.h"
#include "libANGLE/queryutils.h"
#include "libANGLE/validationES.h"

#define ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint)                         \
    do                                                                             \
    {                                                                              \
        if (state.getClientMajorVersion() > 1)                                     \
        {                                                                          \
            errors->validationError(entryPoint, GL_INVALID_OPERATION, kGLES1Only); \
            return false;                                                          \
        }                                                                          \
    } while (0)

#define ANGLE_VALIDATE_IS_GLES1_CONTEXT(context, entryPoint) \
    ANGLE_VALIDATE_IS_GLES1(context->getPrivateState(),      \
                            context->getMutableErrorSetForValidation(), entryPoint)

namespace gl
{
using namespace err;

bool ValidateAlphaFuncCommon(const PrivateState &state,
                             ErrorSet *errors,
                             angle::EntryPoint entryPoint,
                             AlphaTestFunc func)
{
    switch (func)
    {
        case AlphaTestFunc::AlwaysPass:
        case AlphaTestFunc::Equal:
        case AlphaTestFunc::Gequal:
        case AlphaTestFunc::Greater:
        case AlphaTestFunc::Lequal:
        case AlphaTestFunc::Less:
        case AlphaTestFunc::Never:
        case AlphaTestFunc::NotEqual:
            return true;
        default:
            errors->validationError(entryPoint, GL_INVALID_ENUM, kEnumInvalid);
            return false;
    }
}

bool ValidateClientStateCommon(const Context *context,
                               angle::EntryPoint entryPoint,
                               ClientVertexArrayType arrayType)
{
    ANGLE_VALIDATE_IS_GLES1_CONTEXT(context, entryPoint);
    switch (arrayType)
    {
        case ClientVertexArrayType::Vertex:
        case ClientVertexArrayType::Normal:
        case ClientVertexArrayType::Color:
        case ClientVertexArrayType::TextureCoord:
            return true;
        case ClientVertexArrayType::PointSize:
            if (!context->getExtensions().pointSizeArrayOES)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kPointSizeArrayExtensionNotEnabled);
                return false;
            }
            return true;
        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidClientState);
            return false;
    }
}

bool ValidateBuiltinVertexAttributeCommon(const Context *context,
                                          angle::EntryPoint entryPoint,
                                          ClientVertexArrayType arrayType,
                                          GLint size,
                                          VertexAttribType type,
                                          GLsizei stride,
                                          const void *pointer)
{
    ANGLE_VALIDATE_IS_GLES1_CONTEXT(context, entryPoint);

    if (stride < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidVertexPointerStride);
        return false;
    }

    int minSize = 1;
    int maxSize = 4;

    switch (arrayType)
    {
        case ClientVertexArrayType::Vertex:
        case ClientVertexArrayType::TextureCoord:
            minSize = 2;
            maxSize = 4;
            break;
        case ClientVertexArrayType::Normal:
            minSize = 3;
            maxSize = 3;
            break;
        case ClientVertexArrayType::Color:
            minSize = 4;
            maxSize = 4;
            break;
        case ClientVertexArrayType::PointSize:
            if (!context->getExtensions().pointSizeArrayOES)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kPointSizeArrayExtensionNotEnabled);
                return false;
            }

            minSize = 1;
            maxSize = 1;
            break;
        default:
            UNREACHABLE();
            return false;
    }

    if (size < minSize || size > maxSize)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidVertexPointerSize);
        return false;
    }

    switch (type)
    {
        case VertexAttribType::Byte:
            if (arrayType == ClientVertexArrayType::PointSize)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidVertexPointerType);
                return false;
            }
            break;
        case VertexAttribType::Short:
            if (arrayType == ClientVertexArrayType::PointSize ||
                arrayType == ClientVertexArrayType::Color)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidVertexPointerType);
                return false;
            }
            break;
        case VertexAttribType::Fixed:
        case VertexAttribType::Float:
            break;
        case VertexAttribType::UnsignedByte:
            if (arrayType != ClientVertexArrayType::Color)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidVertexPointerType);
                return false;
            }
            break;
        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidVertexPointerType);
            return false;
    }

    return true;
}

bool ValidateLightCaps(const PrivateState &state,
                       ErrorSet *errors,
                       angle::EntryPoint entryPoint,
                       GLenum light)
{
    if (light < GL_LIGHT0 || light >= GL_LIGHT0 + state.getCaps().maxLights)
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidLight);
        return false;
    }

    return true;
}

bool ValidateLightCommon(const PrivateState &state,
                         ErrorSet *errors,
                         angle::EntryPoint entryPoint,
                         GLenum light,
                         LightParameter pname,
                         const GLfloat *params)
{

    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);

    if (!ValidateLightCaps(state, errors, entryPoint, light))
    {
        return false;
    }

    switch (pname)
    {
        case LightParameter::Ambient:
        case LightParameter::Diffuse:
        case LightParameter::Specular:
        case LightParameter::Position:
        case LightParameter::SpotDirection:
            return true;
        case LightParameter::SpotExponent:
            if (params[0] < 0.0f || params[0] > 128.0f)
            {
                errors->validationError(entryPoint, GL_INVALID_VALUE, kLightParameterOutOfRange);
                return false;
            }
            return true;
        case LightParameter::SpotCutoff:
            if (params[0] == 180.0f)
            {
                return true;
            }
            if (params[0] < 0.0f || params[0] > 90.0f)
            {
                errors->validationError(entryPoint, GL_INVALID_VALUE, kLightParameterOutOfRange);
                return false;
            }
            return true;
        case LightParameter::ConstantAttenuation:
        case LightParameter::LinearAttenuation:
        case LightParameter::QuadraticAttenuation:
            if (params[0] < 0.0f)
            {
                errors->validationError(entryPoint, GL_INVALID_VALUE, kLightParameterOutOfRange);
                return false;
            }
            return true;
        default:
            errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidLightParameter);
            return false;
    }
}

bool ValidateLightSingleComponent(const PrivateState &state,
                                  ErrorSet *errors,
                                  angle::EntryPoint entryPoint,
                                  GLenum light,
                                  LightParameter pname,
                                  GLfloat param)
{
    if (!ValidateLightCommon(state, errors, entryPoint, light, pname, &param))
    {
        return false;
    }

    if (GetLightParameterCount(pname) > 1)
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidLightParameter);
        return false;
    }

    return true;
}

bool ValidateMaterialCommon(const PrivateState &state,
                            ErrorSet *errors,
                            angle::EntryPoint entryPoint,
                            GLenum face,
                            MaterialParameter pname,
                            const GLfloat *params)
{
    switch (pname)
    {
        case MaterialParameter::Ambient:
        case MaterialParameter::AmbientAndDiffuse:
        case MaterialParameter::Diffuse:
        case MaterialParameter::Specular:
        case MaterialParameter::Emission:
            return true;
        case MaterialParameter::Shininess:
            if (params[0] < 0.0f || params[0] > 128.0f)
            {
                errors->validationError(entryPoint, GL_INVALID_VALUE, kMaterialParameterOutOfRange);
                return false;
            }
            return true;
        default:
            errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidMaterialParameter);
            return false;
    }
}

bool ValidateMaterialSetting(const PrivateState &state,
                             ErrorSet *errors,
                             angle::EntryPoint entryPoint,
                             GLenum face,
                             MaterialParameter pname,
                             const GLfloat *params)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);

    if (face != GL_FRONT_AND_BACK)
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidMaterialFace);
        return false;
    }

    return ValidateMaterialCommon(state, errors, entryPoint, face, pname, params);
}

bool ValidateMaterialQuery(const PrivateState &state,
                           ErrorSet *errors,
                           angle::EntryPoint entryPoint,
                           GLenum face,
                           MaterialParameter pname)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);

    if (face != GL_FRONT && face != GL_BACK)
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidMaterialFace);
        return false;
    }

    GLfloat validateParams[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    return ValidateMaterialCommon(state, errors, entryPoint, face, pname, validateParams);
}

bool ValidateMaterialSingleComponent(const PrivateState &state,
                                     ErrorSet *errors,
                                     angle::EntryPoint entryPoint,
                                     GLenum face,
                                     MaterialParameter pname,
                                     GLfloat param)
{
    if (!ValidateMaterialSetting(state, errors, entryPoint, face, pname, &param))
    {
        return false;
    }

    if (GetMaterialParameterCount(pname) > 1)
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidMaterialParameter);
        return false;
    }

    return true;
}

bool ValidateLightModelCommon(const PrivateState &state,
                              ErrorSet *errors,
                              angle::EntryPoint entryPoint,
                              GLenum pname)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    switch (pname)
    {
        case GL_LIGHT_MODEL_AMBIENT:
        case GL_LIGHT_MODEL_TWO_SIDE:
            return true;
        default:
            errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidLightModelParameter);
            return false;
    }
}

bool ValidateLightModelSingleComponent(const PrivateState &state,
                                       ErrorSet *errors,
                                       angle::EntryPoint entryPoint,
                                       GLenum pname)
{
    if (!ValidateLightModelCommon(state, errors, entryPoint, pname))
    {
        return false;
    }

    switch (pname)
    {
        case GL_LIGHT_MODEL_TWO_SIDE:
            return true;
        default:
            errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidLightModelParameter);
            return false;
    }
}

bool ValidateClipPlaneCommon(const PrivateState &state,
                             ErrorSet *errors,
                             angle::EntryPoint entryPoint,
                             GLenum plane)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);

    if (plane < GL_CLIP_PLANE0 || plane >= GL_CLIP_PLANE0 + state.getCaps().maxClipPlanes)
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidClipPlane);
        return false;
    }

    return true;
}

bool ValidateFogCommon(const PrivateState &state,
                       ErrorSet *errors,
                       angle::EntryPoint entryPoint,
                       GLenum pname,
                       const GLfloat *params)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);

    switch (pname)
    {
        case GL_FOG_MODE:
        {
            GLenum modeParam = static_cast<GLenum>(params[0]);
            switch (modeParam)
            {
                case GL_EXP:
                case GL_EXP2:
                case GL_LINEAR:
                    return true;
                default:
                    errors->validationError(entryPoint, GL_INVALID_VALUE, kInvalidFogMode);
                    return false;
            }
        }
        case GL_FOG_START:
        case GL_FOG_END:
        case GL_FOG_COLOR:
            break;
        case GL_FOG_DENSITY:
            if (params[0] < 0.0f)
            {
                errors->validationError(entryPoint, GL_INVALID_VALUE, kInvalidFogDensity);
                return false;
            }
            break;
        default:
            errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidFogParameter);
            return false;
    }
    return true;
}

bool ValidateTexEnvCommon(const PrivateState &state,
                          ErrorSet *errors,
                          angle::EntryPoint entryPoint,
                          TextureEnvTarget target,
                          TextureEnvParameter pname,
                          const GLfloat *params)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);

    switch (target)
    {
        case TextureEnvTarget::Env:
            switch (pname)
            {
                case TextureEnvParameter::Mode:
                {
                    TextureEnvMode mode = FromGLenum<TextureEnvMode>(ConvertToGLenum(params[0]));
                    switch (mode)
                    {
                        case TextureEnvMode::Add:
                        case TextureEnvMode::Blend:
                        case TextureEnvMode::Combine:
                        case TextureEnvMode::Decal:
                        case TextureEnvMode::Modulate:
                        case TextureEnvMode::Replace:
                            break;
                        default:
                            errors->validationError(entryPoint, GL_INVALID_VALUE,
                                                    kInvalidTextureEnvMode);
                            return false;
                    }
                    break;
                }
                case TextureEnvParameter::CombineRgb:
                case TextureEnvParameter::CombineAlpha:
                {
                    TextureCombine combine = FromGLenum<TextureCombine>(ConvertToGLenum(params[0]));
                    switch (combine)
                    {
                        case TextureCombine::Add:
                        case TextureCombine::AddSigned:
                        case TextureCombine::Interpolate:
                        case TextureCombine::Modulate:
                        case TextureCombine::Replace:
                        case TextureCombine::Subtract:
                            break;
                        case TextureCombine::Dot3Rgb:
                        case TextureCombine::Dot3Rgba:
                            if (pname == TextureEnvParameter::CombineAlpha)
                            {
                                errors->validationError(entryPoint, GL_INVALID_VALUE,
                                                        kInvalidTextureCombine);
                                return false;
                            }
                            break;
                        default:
                            errors->validationError(entryPoint, GL_INVALID_VALUE,
                                                    kInvalidTextureCombine);
                            return false;
                    }
                    break;
                }
                case TextureEnvParameter::Src0Rgb:
                case TextureEnvParameter::Src1Rgb:
                case TextureEnvParameter::Src2Rgb:
                case TextureEnvParameter::Src0Alpha:
                case TextureEnvParameter::Src1Alpha:
                case TextureEnvParameter::Src2Alpha:
                {
                    TextureSrc combine = FromGLenum<TextureSrc>(ConvertToGLenum(params[0]));
                    switch (combine)
                    {
                        case TextureSrc::Constant:
                        case TextureSrc::Previous:
                        case TextureSrc::PrimaryColor:
                        case TextureSrc::Texture:
                            break;
                        default:
                            errors->validationError(entryPoint, GL_INVALID_VALUE,
                                                    kInvalidTextureCombineSrc);
                            return false;
                    }
                    break;
                }
                case TextureEnvParameter::Op0Rgb:
                case TextureEnvParameter::Op1Rgb:
                case TextureEnvParameter::Op2Rgb:
                case TextureEnvParameter::Op0Alpha:
                case TextureEnvParameter::Op1Alpha:
                case TextureEnvParameter::Op2Alpha:
                {
                    TextureOp operand = FromGLenum<TextureOp>(ConvertToGLenum(params[0]));
                    switch (operand)
                    {
                        case TextureOp::SrcAlpha:
                        case TextureOp::OneMinusSrcAlpha:
                            break;
                        case TextureOp::SrcColor:
                        case TextureOp::OneMinusSrcColor:
                            if (pname == TextureEnvParameter::Op0Alpha ||
                                pname == TextureEnvParameter::Op1Alpha ||
                                pname == TextureEnvParameter::Op2Alpha)
                            {
                                errors->validationError(entryPoint, GL_INVALID_VALUE,
                                                        kInvalidTextureCombine);
                                return false;
                            }
                            break;
                        default:
                            errors->validationError(entryPoint, GL_INVALID_VALUE,
                                                    kInvalidTextureCombineOp);
                            return false;
                    }
                    break;
                }
                case TextureEnvParameter::RgbScale:
                case TextureEnvParameter::AlphaScale:
                    if (params[0] != 1.0f && params[0] != 2.0f && params[0] != 4.0f)
                    {
                        errors->validationError(entryPoint, GL_INVALID_VALUE,
                                                kInvalidTextureEnvScale);
                        return false;
                    }
                    break;
                case TextureEnvParameter::Color:
                    break;
                default:
                    errors->validationError(entryPoint, GL_INVALID_ENUM,
                                            kInvalidTextureEnvParameter);
                    return false;
            }
            break;
        case TextureEnvTarget::PointSprite:
            if (!state.getExtensions().pointSpriteOES)
            {
                errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidTextureEnvTarget);
                return false;
            }
            switch (pname)
            {
                case TextureEnvParameter::PointCoordReplace:
                    break;
                default:
                    errors->validationError(entryPoint, GL_INVALID_ENUM,
                                            kInvalidTextureEnvParameter);
                    return false;
            }
            break;
        default:
            errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidTextureEnvTarget);
            return false;
    }
    return true;
}

bool ValidateGetTexEnvCommon(const PrivateState &state,
                             ErrorSet *errors,
                             angle::EntryPoint entryPoint,
                             TextureEnvTarget target,
                             TextureEnvParameter pname)
{
    GLfloat validateParams[4] = {};
    switch (pname)
    {
        case TextureEnvParameter::Mode:
            ConvertPackedEnum(TextureEnvMode::Add, validateParams);
            break;
        case TextureEnvParameter::CombineRgb:
        case TextureEnvParameter::CombineAlpha:
            ConvertPackedEnum(TextureCombine::Add, validateParams);
            break;
        case TextureEnvParameter::Src0Rgb:
        case TextureEnvParameter::Src1Rgb:
        case TextureEnvParameter::Src2Rgb:
        case TextureEnvParameter::Src0Alpha:
        case TextureEnvParameter::Src1Alpha:
        case TextureEnvParameter::Src2Alpha:
            ConvertPackedEnum(TextureSrc::Constant, validateParams);
            break;
        case TextureEnvParameter::Op0Rgb:
        case TextureEnvParameter::Op1Rgb:
        case TextureEnvParameter::Op2Rgb:
        case TextureEnvParameter::Op0Alpha:
        case TextureEnvParameter::Op1Alpha:
        case TextureEnvParameter::Op2Alpha:
            ConvertPackedEnum(TextureOp::SrcAlpha, validateParams);
            break;
        case TextureEnvParameter::RgbScale:
        case TextureEnvParameter::AlphaScale:
        case TextureEnvParameter::PointCoordReplace:
            validateParams[0] = 1.0f;
            break;
        default:
            break;
    }

    return ValidateTexEnvCommon(state, errors, entryPoint, target, pname, validateParams);
}

bool ValidatePointParameterCommon(const PrivateState &state,
                                  ErrorSet *errors,
                                  angle::EntryPoint entryPoint,
                                  PointParameter pname,
                                  const GLfloat *params)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);

    switch (pname)
    {
        case PointParameter::PointSizeMin:
        case PointParameter::PointSizeMax:
        case PointParameter::PointFadeThresholdSize:
        case PointParameter::PointDistanceAttenuation:
            for (unsigned int i = 0; i < GetPointParameterCount(pname); i++)
            {
                if (params[i] < 0.0f)
                {
                    errors->validationError(entryPoint, GL_INVALID_VALUE,
                                            kInvalidPointParameterValue);
                    return false;
                }
            }
            break;
        default:
            errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidPointParameter);
            return false;
    }

    return true;
}

bool ValidatePointSizeCommon(const PrivateState &state,
                             ErrorSet *errors,
                             angle::EntryPoint entryPoint,
                             GLfloat size)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);

    if (size <= 0.0f)
    {
        errors->validationError(entryPoint, GL_INVALID_VALUE, kInvalidPointSizeValue);
        return false;
    }

    return true;
}

bool ValidateDrawTexCommon(const Context *context,
                           angle::EntryPoint entryPoint,
                           float width,
                           float height)
{
    ANGLE_VALIDATE_IS_GLES1_CONTEXT(context, entryPoint);

    if (width <= 0.0f || height <= 0.0f)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNonPositiveDrawTextureDimension);
        return false;
    }

    return true;
}

}  // namespace gl

namespace gl
{

bool ValidateAlphaFunc(const PrivateState &state,
                       ErrorSet *errors,
                       angle::EntryPoint entryPoint,
                       AlphaTestFunc func,
                       GLfloat ref)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return ValidateAlphaFuncCommon(state, errors, entryPoint, func);
}

bool ValidateAlphaFuncx(const PrivateState &state,
                        ErrorSet *errors,
                        angle::EntryPoint entryPoint,
                        AlphaTestFunc func,
                        GLfixed ref)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return ValidateAlphaFuncCommon(state, errors, entryPoint, func);
}

bool ValidateClearColorx(const PrivateState &state,
                         ErrorSet *errors,
                         angle::EntryPoint entryPoint,
                         GLfixed red,
                         GLfixed green,
                         GLfixed blue,
                         GLfixed alpha)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return true;
}

bool ValidateClearDepthx(const PrivateState &state,
                         ErrorSet *errors,
                         angle::EntryPoint entryPoint,
                         GLfixed depth)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return true;
}

bool ValidateClientActiveTexture(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 GLenum texture)
{
    ANGLE_VALIDATE_IS_GLES1_CONTEXT(context, entryPoint);
    return ValidateMultitextureUnit(context->getPrivateState(),
                                    context->getMutableErrorSetForValidation(), entryPoint,
                                    texture);
}

bool ValidateClipPlanef(const PrivateState &state,
                        ErrorSet *errors,
                        angle::EntryPoint entryPoint,
                        GLenum plane,
                        const GLfloat *eqn)
{
    return ValidateClipPlaneCommon(state, errors, entryPoint, plane);
}

bool ValidateClipPlanex(const PrivateState &state,
                        ErrorSet *errors,
                        angle::EntryPoint entryPoint,
                        GLenum plane,
                        const GLfixed *equation)
{
    return ValidateClipPlaneCommon(state, errors, entryPoint, plane);
}

bool ValidateColor4f(const PrivateState &state,
                     ErrorSet *errors,
                     angle::EntryPoint entryPoint,
                     GLfloat red,
                     GLfloat green,
                     GLfloat blue,
                     GLfloat alpha)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return true;
}

bool ValidateColor4ub(const PrivateState &state,
                      ErrorSet *errors,
                      angle::EntryPoint entryPoint,
                      GLubyte red,
                      GLubyte green,
                      GLubyte blue,
                      GLubyte alpha)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return true;
}

bool ValidateColor4x(const PrivateState &state,
                     ErrorSet *errors,
                     angle::EntryPoint entryPoint,
                     GLfixed red,
                     GLfixed green,
                     GLfixed blue,
                     GLfixed alpha)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return true;
}

bool ValidateColorPointer(const Context *context,
                          angle::EntryPoint entryPoint,
                          GLint size,
                          VertexAttribType type,
                          GLsizei stride,
                          const void *pointer)
{
    return ValidateBuiltinVertexAttributeCommon(context, entryPoint, ClientVertexArrayType::Color,
                                                size, type, stride, pointer);
}

bool ValidateCullFace(const PrivateState &state,
                      ErrorSet *errors,
                      angle::EntryPoint entryPoint,
                      GLenum mode)
{
    UNIMPLEMENTED();
    return true;
}

bool ValidateDepthRangex(const PrivateState &state,
                         ErrorSet *errors,
                         angle::EntryPoint entryPoint,
                         GLfixed n,
                         GLfixed f)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    if (state.isWebGL() && n > f)
    {
        errors->validationError(entryPoint, GL_INVALID_OPERATION, kInvalidDepthRange);
        return false;
    }

    return true;
}

bool ValidateDisableClientState(const Context *context,
                                angle::EntryPoint entryPoint,
                                ClientVertexArrayType arrayType)
{
    return ValidateClientStateCommon(context, entryPoint, arrayType);
}

bool ValidateEnableClientState(const Context *context,
                               angle::EntryPoint entryPoint,
                               ClientVertexArrayType arrayType)
{
    return ValidateClientStateCommon(context, entryPoint, arrayType);
}

bool ValidateFogf(const PrivateState &state,
                  ErrorSet *errors,
                  angle::EntryPoint entryPoint,
                  GLenum pname,
                  GLfloat param)
{
    return ValidateFogCommon(state, errors, entryPoint, pname, &param);
}

bool ValidateFogfv(const PrivateState &state,
                   ErrorSet *errors,
                   angle::EntryPoint entryPoint,
                   GLenum pname,
                   const GLfloat *params)
{
    return ValidateFogCommon(state, errors, entryPoint, pname, params);
}

bool ValidateFogx(const PrivateState &state,
                  ErrorSet *errors,
                  angle::EntryPoint entryPoint,
                  GLenum pname,
                  GLfixed param)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    GLfloat asFloat =
        pname == GL_FOG_MODE ? static_cast<GLfloat>(param) : ConvertFixedToFloat(param);
    return ValidateFogCommon(state, errors, entryPoint, pname, &asFloat);
}

bool ValidateFogxv(const PrivateState &state,
                   ErrorSet *errors,
                   angle::EntryPoint entryPoint,
                   GLenum pname,
                   const GLfixed *params)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    unsigned int paramCount = GetFogParameterCount(pname);
    GLfloat paramsf[4]      = {};

    if (pname == GL_FOG_MODE)
    {
        paramsf[0] = static_cast<GLfloat>(params[0]);
    }
    else
    {
        for (unsigned int i = 0; i < paramCount; i++)
        {
            paramsf[i] = ConvertFixedToFloat(params[i]);
        }
    }

    return ValidateFogCommon(state, errors, entryPoint, pname, paramsf);
}

bool ValidateFrustumf(const PrivateState &state,
                      ErrorSet *errors,
                      angle::EntryPoint entryPoint,
                      GLfloat l,
                      GLfloat r,
                      GLfloat b,
                      GLfloat t,
                      GLfloat n,
                      GLfloat f)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    if (l == r || b == t || n == f || n <= 0.0f || f <= 0.0f)
    {
        errors->validationError(entryPoint, GL_INVALID_VALUE, kInvalidProjectionMatrix);
        return false;
    }
    return true;
}

bool ValidateFrustumx(const PrivateState &state,
                      ErrorSet *errors,
                      angle::EntryPoint entryPoint,
                      GLfixed l,
                      GLfixed r,
                      GLfixed b,
                      GLfixed t,
                      GLfixed n,
                      GLfixed f)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    if (l == r || b == t || n == f || n <= 0 || f <= 0)
    {
        errors->validationError(entryPoint, GL_INVALID_VALUE, kInvalidProjectionMatrix);
        return false;
    }
    return true;
}

bool ValidateGetBufferParameteriv(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  GLenum target,
                                  GLenum pname,
                                  const GLint *params)
{
    UNIMPLEMENTED();
    return true;
}

bool ValidateGetClipPlanef(const PrivateState &state,
                           ErrorSet *errors,
                           angle::EntryPoint entryPoint,
                           GLenum plane,
                           const GLfloat *equation)
{
    return ValidateClipPlaneCommon(state, errors, entryPoint, plane);
}

bool ValidateGetClipPlanex(const PrivateState &state,
                           ErrorSet *errors,
                           angle::EntryPoint entryPoint,
                           GLenum plane,
                           const GLfixed *equation)
{
    return ValidateClipPlaneCommon(state, errors, entryPoint, plane);
}

bool ValidateGetFixedv(const Context *context,
                       angle::EntryPoint entryPoint,
                       GLenum pname,
                       const GLfixed *params)
{
    ANGLE_VALIDATE_IS_GLES1_CONTEXT(context, entryPoint);
    GLenum nativeType;
    unsigned int numParams = 0;
    return ValidateStateQuery(context, entryPoint, pname, &nativeType, &numParams);
}

bool ValidateGetLightfv(const PrivateState &state,
                        ErrorSet *errors,
                        angle::EntryPoint entryPoint,
                        GLenum light,
                        LightParameter pname,
                        const GLfloat *params)
{
    GLfloat validateParams[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    return ValidateLightCommon(state, errors, entryPoint, light, pname, validateParams);
}

bool ValidateGetLightxv(const PrivateState &state,
                        ErrorSet *errors,
                        angle::EntryPoint entryPoint,
                        GLenum light,
                        LightParameter pname,
                        const GLfixed *params)
{
    GLfloat validateParams[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    return ValidateLightCommon(state, errors, entryPoint, light, pname, validateParams);
}

bool ValidateGetMaterialfv(const PrivateState &state,
                           ErrorSet *errors,
                           angle::EntryPoint entryPoint,
                           GLenum face,
                           MaterialParameter pname,
                           const GLfloat *params)
{
    return ValidateMaterialQuery(state, errors, entryPoint, face, pname);
}

bool ValidateGetMaterialxv(const PrivateState &state,
                           ErrorSet *errors,
                           angle::EntryPoint entryPoint,
                           GLenum face,
                           MaterialParameter pname,
                           const GLfixed *params)
{
    return ValidateMaterialQuery(state, errors, entryPoint, face, pname);
}

bool ValidateGetTexEnvfv(const PrivateState &state,
                         ErrorSet *errors,
                         angle::EntryPoint entryPoint,
                         TextureEnvTarget target,
                         TextureEnvParameter pname,
                         const GLfloat *params)
{
    return ValidateGetTexEnvCommon(state, errors, entryPoint, target, pname);
}

bool ValidateGetTexEnviv(const PrivateState &state,
                         ErrorSet *errors,
                         angle::EntryPoint entryPoint,
                         TextureEnvTarget target,
                         TextureEnvParameter pname,
                         const GLint *params)
{
    return ValidateGetTexEnvCommon(state, errors, entryPoint, target, pname);
}

bool ValidateGetTexEnvxv(const PrivateState &state,
                         ErrorSet *errors,
                         angle::EntryPoint entryPoint,
                         TextureEnvTarget target,
                         TextureEnvParameter pname,
                         const GLfixed *params)
{
    return ValidateGetTexEnvCommon(state, errors, entryPoint, target, pname);
}

bool ValidateGetTexParameterxv(const Context *context,
                               angle::EntryPoint entryPoint,
                               TextureType target,
                               GLenum pname,
                               const GLfixed *params)
{
    ANGLE_VALIDATE_IS_GLES1_CONTEXT(context, entryPoint);

    if (!ValidateGetTexParameterBase(context, entryPoint, target, pname, nullptr))
    {
        return false;
    }

    return true;
}

bool ValidateLightModelf(const PrivateState &state,
                         ErrorSet *errors,
                         angle::EntryPoint entryPoint,
                         GLenum pname,
                         GLfloat param)
{
    return ValidateLightModelSingleComponent(state, errors, entryPoint, pname);
}

bool ValidateLightModelfv(const PrivateState &state,
                          ErrorSet *errors,
                          angle::EntryPoint entryPoint,
                          GLenum pname,
                          const GLfloat *params)
{
    return ValidateLightModelCommon(state, errors, entryPoint, pname);
}

bool ValidateLightModelx(const PrivateState &state,
                         ErrorSet *errors,
                         angle::EntryPoint entryPoint,
                         GLenum pname,
                         GLfixed param)
{
    return ValidateLightModelSingleComponent(state, errors, entryPoint, pname);
}

bool ValidateLightModelxv(const PrivateState &state,
                          ErrorSet *errors,
                          angle::EntryPoint entryPoint,
                          GLenum pname,
                          const GLfixed *param)
{
    return ValidateLightModelCommon(state, errors, entryPoint, pname);
}

bool ValidateLightf(const PrivateState &state,
                    ErrorSet *errors,
                    angle::EntryPoint entryPoint,
                    GLenum light,
                    LightParameter pname,
                    GLfloat param)
{
    return ValidateLightSingleComponent(state, errors, entryPoint, light, pname, param);
}

bool ValidateLightfv(const PrivateState &state,
                     ErrorSet *errors,
                     angle::EntryPoint entryPoint,
                     GLenum light,
                     LightParameter pname,
                     const GLfloat *params)
{
    return ValidateLightCommon(state, errors, entryPoint, light, pname, params);
}

bool ValidateLightx(const PrivateState &state,
                    ErrorSet *errors,
                    angle::EntryPoint entryPoint,
                    GLenum light,
                    LightParameter pname,
                    GLfixed param)
{
    return ValidateLightSingleComponent(state, errors, entryPoint, light, pname,
                                        ConvertFixedToFloat(param));
}

bool ValidateLightxv(const PrivateState &state,
                     ErrorSet *errors,
                     angle::EntryPoint entryPoint,
                     GLenum light,
                     LightParameter pname,
                     const GLfixed *params)
{
    GLfloat paramsf[4];
    for (unsigned int i = 0; i < GetLightParameterCount(pname); i++)
    {
        paramsf[i] = ConvertFixedToFloat(params[i]);
    }

    return ValidateLightCommon(state, errors, entryPoint, light, pname, paramsf);
}

bool ValidateLineWidthx(const PrivateState &state,
                        ErrorSet *errors,
                        angle::EntryPoint entryPoint,
                        GLfixed width)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    if (width <= 0)
    {
        errors->validationError(entryPoint, GL_INVALID_VALUE, kInvalidWidth);
        return false;
    }

    return true;
}

bool ValidateLoadIdentity(const PrivateState &state, ErrorSet *errors, angle::EntryPoint entryPoint)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return true;
}

bool ValidateLoadMatrixf(const PrivateState &state,
                         ErrorSet *errors,
                         angle::EntryPoint entryPoint,
                         const GLfloat *m)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return true;
}

bool ValidateLoadMatrixx(const PrivateState &state,
                         ErrorSet *errors,
                         angle::EntryPoint entryPoint,
                         const GLfixed *m)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return true;
}

bool ValidateLogicOp(const PrivateState &state,
                     ErrorSet *errors,
                     angle::EntryPoint entryPoint,
                     LogicalOperation opcode)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return ValidateLogicOpCommon(state, errors, entryPoint, opcode);
}

bool ValidateMaterialf(const PrivateState &state,
                       ErrorSet *errors,
                       angle::EntryPoint entryPoint,
                       GLenum face,
                       MaterialParameter pname,
                       GLfloat param)
{
    return ValidateMaterialSingleComponent(state, errors, entryPoint, face, pname, param);
}

bool ValidateMaterialfv(const PrivateState &state,
                        ErrorSet *errors,
                        angle::EntryPoint entryPoint,
                        GLenum face,
                        MaterialParameter pname,
                        const GLfloat *params)
{
    return ValidateMaterialSetting(state, errors, entryPoint, face, pname, params);
}

bool ValidateMaterialx(const PrivateState &state,
                       ErrorSet *errors,
                       angle::EntryPoint entryPoint,
                       GLenum face,
                       MaterialParameter pname,
                       GLfixed param)
{
    return ValidateMaterialSingleComponent(state, errors, entryPoint, face, pname,
                                           ConvertFixedToFloat(param));
}

bool ValidateMaterialxv(const PrivateState &state,
                        ErrorSet *errors,
                        angle::EntryPoint entryPoint,
                        GLenum face,
                        MaterialParameter pname,
                        const GLfixed *params)
{
    GLfloat paramsf[4];

    for (unsigned int i = 0; i < GetMaterialParameterCount(pname); i++)
    {
        paramsf[i] = ConvertFixedToFloat(params[i]);
    }

    return ValidateMaterialSetting(state, errors, entryPoint, face, pname, paramsf);
}

bool ValidateMatrixMode(const PrivateState &state,
                        ErrorSet *errors,
                        angle::EntryPoint entryPoint,
                        MatrixType mode)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    switch (mode)
    {
        case MatrixType::Projection:
        case MatrixType::Modelview:
        case MatrixType::Texture:
            return true;
        default:
            errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidMatrixMode);
            return false;
    }
}

bool ValidateMultMatrixf(const PrivateState &state,
                         ErrorSet *errors,
                         angle::EntryPoint entryPoint,
                         const GLfloat *m)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return true;
}

bool ValidateMultMatrixx(const PrivateState &state,
                         ErrorSet *errors,
                         angle::EntryPoint entryPoint,
                         const GLfixed *m)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return true;
}

bool ValidateMultiTexCoord4f(const PrivateState &state,
                             ErrorSet *errors,
                             angle::EntryPoint entryPoint,
                             GLenum target,
                             GLfloat s,
                             GLfloat t,
                             GLfloat r,
                             GLfloat q)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return ValidateMultitextureUnit(state, errors, entryPoint, target);
}

bool ValidateMultiTexCoord4x(const PrivateState &state,
                             ErrorSet *errors,
                             angle::EntryPoint entryPoint,
                             GLenum target,
                             GLfixed s,
                             GLfixed t,
                             GLfixed r,
                             GLfixed q)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return ValidateMultitextureUnit(state, errors, entryPoint, target);
}

bool ValidateNormal3f(const PrivateState &state,
                      ErrorSet *errors,
                      angle::EntryPoint entryPoint,
                      GLfloat nx,
                      GLfloat ny,
                      GLfloat nz)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return true;
}

bool ValidateNormal3x(const PrivateState &state,
                      ErrorSet *errors,
                      angle::EntryPoint entryPoint,
                      GLfixed nx,
                      GLfixed ny,
                      GLfixed nz)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return true;
}

bool ValidateNormalPointer(const Context *context,
                           angle::EntryPoint entryPoint,
                           VertexAttribType type,
                           GLsizei stride,
                           const void *pointer)
{
    return ValidateBuiltinVertexAttributeCommon(context, entryPoint, ClientVertexArrayType::Normal,
                                                3, type, stride, pointer);
}

bool ValidateOrthof(const PrivateState &state,
                    ErrorSet *errors,
                    angle::EntryPoint entryPoint,
                    GLfloat l,
                    GLfloat r,
                    GLfloat b,
                    GLfloat t,
                    GLfloat n,
                    GLfloat f)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    // [OpenGL ES 1.1.12] section 2.10.2 page 31:
    // If l is equal to r, b is equal to t, or n is equal to f, the
    // error INVALID VALUE results.
    if (l == r || b == t || n == f)
    {
        errors->validationError(entryPoint, GL_INVALID_VALUE, kInvalidProjectionMatrix);
        return false;
    }
    return true;
}

bool ValidateOrthox(const PrivateState &state,
                    ErrorSet *errors,
                    angle::EntryPoint entryPoint,
                    GLfixed l,
                    GLfixed r,
                    GLfixed b,
                    GLfixed t,
                    GLfixed n,
                    GLfixed f)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    if (l == r || b == t || n == f)
    {
        errors->validationError(entryPoint, GL_INVALID_VALUE, kInvalidProjectionMatrix);
        return false;
    }
    return true;
}

bool ValidatePointParameterf(const PrivateState &state,
                             ErrorSet *errors,
                             angle::EntryPoint entryPoint,
                             PointParameter pname,
                             GLfloat param)
{
    unsigned int paramCount = GetPointParameterCount(pname);
    if (paramCount != 1)
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidPointParameter);
        return false;
    }

    return ValidatePointParameterCommon(state, errors, entryPoint, pname, &param);
}

bool ValidatePointParameterfv(const PrivateState &state,
                              ErrorSet *errors,
                              angle::EntryPoint entryPoint,
                              PointParameter pname,
                              const GLfloat *params)
{
    return ValidatePointParameterCommon(state, errors, entryPoint, pname, params);
}

bool ValidatePointParameterx(const PrivateState &state,
                             ErrorSet *errors,
                             angle::EntryPoint entryPoint,
                             PointParameter pname,
                             GLfixed param)
{
    unsigned int paramCount = GetPointParameterCount(pname);
    if (paramCount != 1)
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidPointParameter);
        return false;
    }

    GLfloat paramf = ConvertFixedToFloat(param);
    return ValidatePointParameterCommon(state, errors, entryPoint, pname, &paramf);
}

bool ValidatePointParameterxv(const PrivateState &state,
                              ErrorSet *errors,
                              angle::EntryPoint entryPoint,
                              PointParameter pname,
                              const GLfixed *params)
{
    GLfloat paramsf[4] = {};
    for (unsigned int i = 0; i < GetPointParameterCount(pname); i++)
    {
        paramsf[i] = ConvertFixedToFloat(params[i]);
    }
    return ValidatePointParameterCommon(state, errors, entryPoint, pname, paramsf);
}

bool ValidatePointSize(const PrivateState &state,
                       ErrorSet *errors,
                       angle::EntryPoint entryPoint,
                       GLfloat size)
{
    return ValidatePointSizeCommon(state, errors, entryPoint, size);
}

bool ValidatePointSizex(const PrivateState &state,
                        ErrorSet *errors,
                        angle::EntryPoint entryPoint,
                        GLfixed size)
{
    return ValidatePointSizeCommon(state, errors, entryPoint, ConvertFixedToFloat(size));
}

bool ValidatePolygonOffsetx(const PrivateState &state,
                            ErrorSet *errors,
                            angle::EntryPoint entryPoint,
                            GLfixed factor,
                            GLfixed units)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return true;
}

bool ValidatePopMatrix(const PrivateState &state, ErrorSet *errors, angle::EntryPoint entryPoint)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    const auto &stack = state.gles1().currentMatrixStack();
    if (stack.size() == 1)
    {
        errors->validationError(entryPoint, GL_STACK_UNDERFLOW, kMatrixStackUnderflow);
        return false;
    }
    return true;
}

bool ValidatePushMatrix(const PrivateState &state, ErrorSet *errors, angle::EntryPoint entryPoint)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    const auto &stack = state.gles1().currentMatrixStack();
    if (stack.size() == stack.max_size())
    {
        errors->validationError(entryPoint, GL_STACK_OVERFLOW, kMatrixStackOverflow);
        return false;
    }
    return true;
}

bool ValidateRotatef(const PrivateState &state,
                     ErrorSet *errors,
                     angle::EntryPoint entryPoint,
                     GLfloat angle,
                     GLfloat x,
                     GLfloat y,
                     GLfloat z)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return true;
}

bool ValidateRotatex(const PrivateState &state,
                     ErrorSet *errors,
                     angle::EntryPoint entryPoint,
                     GLfixed angle,
                     GLfixed x,
                     GLfixed y,
                     GLfixed z)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return true;
}

bool ValidateSampleCoveragex(const PrivateState &state,
                             ErrorSet *errors,
                             angle::EntryPoint entryPoint,
                             GLclampx value,
                             GLboolean invert)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return true;
}

bool ValidateScalef(const PrivateState &state,
                    ErrorSet *errors,
                    angle::EntryPoint entryPoint,
                    GLfloat x,
                    GLfloat y,
                    GLfloat z)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return true;
}

bool ValidateScalex(const PrivateState &state,
                    ErrorSet *errors,
                    angle::EntryPoint entryPoint,
                    GLfixed x,
                    GLfixed y,
                    GLfixed z)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return true;
}

bool ValidateShadeModel(const PrivateState &state,
                        ErrorSet *errors,
                        angle::EntryPoint entryPoint,
                        ShadingModel mode)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    switch (mode)
    {
        case ShadingModel::Flat:
        case ShadingModel::Smooth:
            return true;
        default:
            errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidShadingModel);
            return false;
    }
}

bool ValidateTexCoordPointer(const Context *context,
                             angle::EntryPoint entryPoint,
                             GLint size,
                             VertexAttribType type,
                             GLsizei stride,
                             const void *pointer)
{
    return ValidateBuiltinVertexAttributeCommon(
        context, entryPoint, ClientVertexArrayType::TextureCoord, size, type, stride, pointer);
}

bool ValidateTexEnvf(const PrivateState &state,
                     ErrorSet *errors,
                     angle::EntryPoint entryPoint,
                     TextureEnvTarget target,
                     TextureEnvParameter pname,
                     GLfloat param)
{
    return ValidateTexEnvCommon(state, errors, entryPoint, target, pname, &param);
}

bool ValidateTexEnvfv(const PrivateState &state,
                      ErrorSet *errors,
                      angle::EntryPoint entryPoint,
                      TextureEnvTarget target,
                      TextureEnvParameter pname,
                      const GLfloat *params)
{
    return ValidateTexEnvCommon(state, errors, entryPoint, target, pname, params);
}

bool ValidateTexEnvi(const PrivateState &state,
                     ErrorSet *errors,
                     angle::EntryPoint entryPoint,
                     TextureEnvTarget target,
                     TextureEnvParameter pname,
                     GLint param)
{
    GLfloat paramf = static_cast<GLfloat>(param);
    return ValidateTexEnvCommon(state, errors, entryPoint, target, pname, &paramf);
}

bool ValidateTexEnviv(const PrivateState &state,
                      ErrorSet *errors,
                      angle::EntryPoint entryPoint,
                      TextureEnvTarget target,
                      TextureEnvParameter pname,
                      const GLint *params)
{
    GLfloat paramsf[4];
    for (unsigned int i = 0; i < GetTextureEnvParameterCount(pname); i++)
    {
        paramsf[i] = static_cast<GLfloat>(params[i]);
    }
    return ValidateTexEnvCommon(state, errors, entryPoint, target, pname, paramsf);
}

bool ValidateTexEnvx(const PrivateState &state,
                     ErrorSet *errors,
                     angle::EntryPoint entryPoint,
                     TextureEnvTarget target,
                     TextureEnvParameter pname,
                     GLfixed param)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    GLfloat paramsf[4] = {};
    ConvertTextureEnvFromFixed(pname, &param, paramsf);
    return ValidateTexEnvCommon(state, errors, entryPoint, target, pname, paramsf);
}

bool ValidateTexEnvxv(const PrivateState &state,
                      ErrorSet *errors,
                      angle::EntryPoint entryPoint,
                      TextureEnvTarget target,
                      TextureEnvParameter pname,
                      const GLfixed *params)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    GLfloat paramsf[4] = {};
    ConvertTextureEnvFromFixed(pname, params, paramsf);
    return ValidateTexEnvCommon(state, errors, entryPoint, target, pname, paramsf);
}

bool ValidateTexParameterBaseForGLfixed(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        TextureType target,
                                        GLenum pname,
                                        GLsizei bufSize,
                                        bool vectorParams,
                                        const GLfixed *params)
{
    // Convert GLfixed parameter for GL_TEXTURE_MAX_ANISOTROPY_EXT independently
    // since it compares against 1 and maxTextureAnisotropy instead of just 0
    // (other values are fine to leave unconverted since they only check positive or negative or
    // are used as enums)
    GLfloat paramValue;
    if (pname == GL_TEXTURE_MAX_ANISOTROPY_EXT)
    {
        paramValue = ConvertFixedToFloat(static_cast<GLfixed>(params[0]));
    }
    else
    {
        paramValue = static_cast<GLfloat>(params[0]);
    }
    return ValidateTexParameterBase(context, entryPoint, target, pname, bufSize, vectorParams,
                                    &paramValue);
}

bool ValidateTexParameterx(const Context *context,
                           angle::EntryPoint entryPoint,
                           TextureType target,
                           GLenum pname,
                           GLfixed param)
{
    ANGLE_VALIDATE_IS_GLES1_CONTEXT(context, entryPoint);
    return ValidateTexParameterBaseForGLfixed(context, entryPoint, target, pname, -1, false,
                                              &param);
}

bool ValidateTexParameterxv(const Context *context,
                            angle::EntryPoint entryPoint,
                            TextureType target,
                            GLenum pname,
                            const GLfixed *params)
{
    ANGLE_VALIDATE_IS_GLES1_CONTEXT(context, entryPoint);
    return ValidateTexParameterBaseForGLfixed(context, entryPoint, target, pname, -1, true, params);
}

bool ValidateTranslatef(const PrivateState &state,
                        ErrorSet *errors,
                        angle::EntryPoint entryPoint,
                        GLfloat x,
                        GLfloat y,
                        GLfloat z)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return true;
}

bool ValidateTranslatex(const PrivateState &state,
                        ErrorSet *errors,
                        angle::EntryPoint entryPoint,
                        GLfixed x,
                        GLfixed y,
                        GLfixed z)
{
    ANGLE_VALIDATE_IS_GLES1(state, errors, entryPoint);
    return true;
}

bool ValidateVertexPointer(const Context *context,
                           angle::EntryPoint entryPoint,
                           GLint size,
                           VertexAttribType type,
                           GLsizei stride,
                           const void *pointer)
{
    return ValidateBuiltinVertexAttributeCommon(context, entryPoint, ClientVertexArrayType::Vertex,
                                                size, type, stride, pointer);
}

bool ValidateDrawTexfOES(const Context *context,
                         angle::EntryPoint entryPoint,
                         GLfloat x,
                         GLfloat y,
                         GLfloat z,
                         GLfloat width,
                         GLfloat height)
{
    return ValidateDrawTexCommon(context, entryPoint, width, height);
}

bool ValidateDrawTexfvOES(const Context *context,
                          angle::EntryPoint entryPoint,
                          const GLfloat *coords)
{
    return ValidateDrawTexCommon(context, entryPoint, coords[3], coords[4]);
}

bool ValidateDrawTexiOES(const Context *context,
                         angle::EntryPoint entryPoint,
                         GLint x,
                         GLint y,
                         GLint z,
                         GLint width,
                         GLint height)
{
    return ValidateDrawTexCommon(context, entryPoint, static_cast<GLfloat>(width),
                                 static_cast<GLfloat>(height));
}

bool ValidateDrawTexivOES(const Context *context, angle::EntryPoint entryPoint, const GLint *coords)
{
    return ValidateDrawTexCommon(context, entryPoint, static_cast<GLfloat>(coords[3]),
                                 static_cast<GLfloat>(coords[4]));
}

bool ValidateDrawTexsOES(const Context *context,
                         angle::EntryPoint entryPoint,
                         GLshort x,
                         GLshort y,
                         GLshort z,
                         GLshort width,
                         GLshort height)
{
    return ValidateDrawTexCommon(context, entryPoint, static_cast<GLfloat>(width),
                                 static_cast<GLfloat>(height));
}

bool ValidateDrawTexsvOES(const Context *context,
                          angle::EntryPoint entryPoint,
                          const GLshort *coords)
{
    return ValidateDrawTexCommon(context, entryPoint, static_cast<GLfloat>(coords[3]),
                                 static_cast<GLfloat>(coords[4]));
}

bool ValidateDrawTexxOES(const Context *context,
                         angle::EntryPoint entryPoint,
                         GLfixed x,
                         GLfixed y,
                         GLfixed z,
                         GLfixed width,
                         GLfixed height)
{
    return ValidateDrawTexCommon(context, entryPoint, ConvertFixedToFloat(width),
                                 ConvertFixedToFloat(height));
}

bool ValidateDrawTexxvOES(const Context *context,
                          angle::EntryPoint entryPoint,
                          const GLfixed *coords)
{
    return ValidateDrawTexCommon(context, entryPoint, ConvertFixedToFloat(coords[3]),
                                 ConvertFixedToFloat(coords[4]));
}

bool ValidateCurrentPaletteMatrixOES(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     GLuint matrixpaletteindex)
{
    UNIMPLEMENTED();
    return true;
}

bool ValidateLoadPaletteFromModelViewMatrixOES(const Context *context, angle::EntryPoint entryPoint)
{
    UNIMPLEMENTED();
    return true;
}

bool ValidateMatrixIndexPointerOES(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   GLint size,
                                   GLenum type,
                                   GLsizei stride,
                                   const void *pointer)
{
    UNIMPLEMENTED();
    return true;
}

bool ValidateWeightPointerOES(const Context *context,
                              angle::EntryPoint entryPoint,
                              GLint size,
                              GLenum type,
                              GLsizei stride,
                              const void *pointer)
{
    UNIMPLEMENTED();
    return true;
}

bool ValidatePointSizePointerOES(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 VertexAttribType type,
                                 GLsizei stride,
                                 const void *pointer)
{
    return ValidateBuiltinVertexAttributeCommon(
        context, entryPoint, ClientVertexArrayType::PointSize, 1, type, stride, pointer);
}

bool ValidateQueryMatrixxOES(const Context *context,
                             angle::EntryPoint entryPoint,
                             const GLfixed *mantissa,
                             const GLint *exponent)
{
    UNIMPLEMENTED();
    return true;
}

bool ValidateGenFramebuffersOES(const Context *context,
                                angle::EntryPoint entryPoint,
                                GLsizei n,
                                const FramebufferID *framebuffers)
{
    if (!context->getExtensions().framebufferObjectOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateGenOrDelete(context, entryPoint, n);
}

bool ValidateDeleteFramebuffersOES(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   GLsizei n,
                                   const FramebufferID *framebuffers)
{
    if (!context->getExtensions().framebufferObjectOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateGenOrDelete(context, entryPoint, n);
}

bool ValidateGenRenderbuffersOES(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 GLsizei n,
                                 const RenderbufferID *renderbuffers)
{
    if (!context->getExtensions().framebufferObjectOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateGenOrDelete(context, entryPoint, n);
}

bool ValidateDeleteRenderbuffersOES(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    GLsizei n,
                                    const RenderbufferID *renderbuffers)
{
    if (!context->getExtensions().framebufferObjectOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateGenOrDelete(context, entryPoint, n);
}

bool ValidateBindFramebufferOES(const Context *context,
                                angle::EntryPoint entryPoint,
                                GLenum target,
                                FramebufferID framebuffer)
{
    if (!context->getExtensions().framebufferObjectOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateBindFramebufferBase(context, entryPoint, target, framebuffer);
}

bool ValidateBindRenderbufferOES(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 GLenum target,
                                 RenderbufferID renderbuffer)
{
    if (!context->getExtensions().framebufferObjectOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateBindRenderbufferBase(context, entryPoint, target, renderbuffer);
}

bool ValidateCheckFramebufferStatusOES(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       GLenum target)
{
    if (!context->getExtensions().framebufferObjectOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (!ValidFramebufferTarget(context, target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFramebufferTarget);
        return false;
    }

    return true;
}

bool ValidateFramebufferRenderbufferOES(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        GLenum target,
                                        GLenum attachment,
                                        GLenum rbtarget,
                                        RenderbufferID renderbuffer)
{
    if (!context->getExtensions().framebufferObjectOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateFramebufferRenderbufferBase(context, entryPoint, target, attachment, rbtarget,
                                               renderbuffer);
}

bool ValidateFramebufferTexture2DOES(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     GLenum target,
                                     GLenum attachment,
                                     TextureTarget textarget,
                                     TextureID texture,
                                     GLint level)
{
    if (!context->getExtensions().framebufferObjectOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (level != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidFramebufferTextureLevel);
        return false;
    }

    if (!ValidateFramebufferTextureBase(context, entryPoint, target, attachment, texture, level))
    {
        return false;
    }

    if (texture.value != 0)
    {
        Texture *tex = context->getTexture(texture);
        ASSERT(tex);

        const Caps &caps = context->getCaps();

        switch (textarget)
        {
            case TextureTarget::_2D:
            {
                if (level > log2(caps.max2DTextureSize))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevel);
                    return false;
                }
                if (tex->getType() != TextureType::_2D)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidTextureTarget);
                    return false;
                }
            }
            break;

            case TextureTarget::CubeMapNegativeX:
            case TextureTarget::CubeMapNegativeY:
            case TextureTarget::CubeMapNegativeZ:
            case TextureTarget::CubeMapPositiveX:
            case TextureTarget::CubeMapPositiveY:
            case TextureTarget::CubeMapPositiveZ:
            {
                if (!context->getExtensions().textureCubeMapOES)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
                    return false;
                }

                if (level > log2(caps.maxCubeMapTextureSize))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevel);
                    return false;
                }
                if (tex->getType() != TextureType::CubeMap)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTextureTargetMismatch);
                    return false;
                }
            }
            break;

            default:
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
                return false;
        }
    }

    return true;
}

bool ValidateGenerateMipmapOES(const Context *context,
                               angle::EntryPoint entryPoint,
                               TextureType target)
{
    if (!context->getExtensions().framebufferObjectOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateGenerateMipmapBase(context, entryPoint, target);
}

bool ValidateGetFramebufferAttachmentParameterivOES(const Context *context,
                                                    angle::EntryPoint entryPoint,
                                                    GLenum target,
                                                    GLenum attachment,
                                                    GLenum pname,
                                                    const GLint *params)
{
    if (!context->getExtensions().framebufferObjectOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateGetFramebufferAttachmentParameterivBase(context, entryPoint, target, attachment,
                                                           pname, nullptr);
}

bool ValidateGetRenderbufferParameterivOES(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           GLenum target,
                                           GLenum pname,
                                           const GLint *params)
{
    if (!context->getExtensions().framebufferObjectOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateGetRenderbufferParameterivBase(context, entryPoint, target, pname, nullptr);
}

bool ValidateIsFramebufferOES(const Context *context,
                              angle::EntryPoint entryPoint,
                              FramebufferID framebuffer)
{
    if (!context->getExtensions().framebufferObjectOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return true;
}

bool ValidateIsRenderbufferOES(const Context *context,
                               angle::EntryPoint entryPoint,
                               RenderbufferID renderbuffer)
{
    if (!context->getExtensions().framebufferObjectOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return true;
}

bool ValidateRenderbufferStorageOES(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    GLenum target,
                                    GLenum internalformat,
                                    GLsizei width,
                                    GLsizei height)
{
    if (!context->getExtensions().framebufferObjectOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateRenderbufferStorageParametersBase(context, entryPoint, target, 0, internalformat,
                                                     width, height);
}

// GL_OES_texture_cube_map

bool ValidateGetTexGenfvOES(const Context *context,
                            angle::EntryPoint entryPoint,
                            GLenum coord,
                            GLenum pname,
                            const GLfloat *params)
{
    UNIMPLEMENTED();
    return true;
}

bool ValidateGetTexGenivOES(const Context *context,
                            angle::EntryPoint entryPoint,
                            GLenum coord,
                            GLenum pname,
                            const int *params)
{
    UNIMPLEMENTED();
    return true;
}

bool ValidateGetTexGenxvOES(const Context *context,
                            angle::EntryPoint entryPoint,
                            GLenum coord,
                            GLenum pname,
                            const GLfixed *params)
{
    UNIMPLEMENTED();
    return true;
}

bool ValidateTexGenfvOES(const Context *context,
                         angle::EntryPoint entryPoint,
                         GLenum coord,
                         GLenum pname,
                         const GLfloat *params)
{
    UNIMPLEMENTED();
    return true;
}

bool ValidateTexGenivOES(const Context *context,
                         angle::EntryPoint entryPoint,
                         GLenum coord,
                         GLenum pname,
                         const GLint *param)
{
    UNIMPLEMENTED();
    return true;
}

bool ValidateTexGenxvOES(const Context *context,
                         angle::EntryPoint entryPoint,
                         GLenum coord,
                         GLenum pname,
                         const GLint *param)
{
    UNIMPLEMENTED();
    return true;
}

bool ValidateTexGenfOES(const Context *context,
                        angle::EntryPoint entryPoint,
                        GLenum coord,
                        GLenum pname,
                        GLfloat param)
{
    UNIMPLEMENTED();
    return true;
}

bool ValidateTexGeniOES(const Context *context,
                        angle::EntryPoint entryPoint,
                        GLenum coord,
                        GLenum pname,
                        GLint param)
{
    UNIMPLEMENTED();
    return true;
}

bool ValidateTexGenxOES(const Context *context,
                        angle::EntryPoint entryPoint,
                        GLenum coord,
                        GLenum pname,
                        GLfixed param)
{
    UNIMPLEMENTED();
    return true;
}

}  // namespace gl
