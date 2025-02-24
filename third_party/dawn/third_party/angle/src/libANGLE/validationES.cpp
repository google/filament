//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationES.h: Validation functions for generic OpenGL ES entry point parameters

#include "libANGLE/validationES.h"

#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/ErrorStrings.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/Image.h"
#include "libANGLE/PixelLocalStorage.h"
#include "libANGLE/Program.h"
#include "libANGLE/Query.h"
#include "libANGLE/Texture.h"
#include "libANGLE/TransformFeedback.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/queryconversions.h"
#include "libANGLE/queryutils.h"
#include "libANGLE/validationES2.h"
#include "libANGLE/validationES3.h"

#include "common/mathutil.h"
#include "common/utilities.h"

using namespace angle;

namespace gl
{
using namespace err;

namespace
{
bool CompressedTextureFormatRequiresExactSize(GLenum internalFormat)
{
    // List of compressed format that require that the texture size is smaller than or a multiple of
    // the compressed block size.
    switch (internalFormat)
    {
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE:
        case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
        case GL_ETC1_RGB8_LOSSY_DECODE_ANGLE:
        case GL_COMPRESSED_RGB8_LOSSY_DECODE_ETC2_ANGLE:
        case GL_COMPRESSED_SRGB8_LOSSY_DECODE_ETC2_ANGLE:
        case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE:
        case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE:
        case GL_COMPRESSED_RGBA8_LOSSY_DECODE_ETC2_EAC_ANGLE:
        case GL_COMPRESSED_SRGB8_ALPHA8_LOSSY_DECODE_ETC2_EAC_ANGLE:
        case GL_COMPRESSED_RED_RGTC1_EXT:
        case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:
        case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
        case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:
        case GL_COMPRESSED_RGBA_BPTC_UNORM_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT:
        case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT:
        case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT:
            return true;

        default:
            return false;
    }
}

bool DifferenceCanOverflow(GLint a, GLint b)
{
    CheckedNumeric<GLint> checkedA(a);
    checkedA -= b;
    // Use negation to make sure that the difference can't overflow regardless of the order.
    checkedA = -checkedA;
    return !checkedA.IsValid();
}

bool ValidReadPixelsTypeEnum(const Context *context, GLenum type)
{
    switch (type)
    {
        // Types referenced in Table 3.4 of the ES 2.0.25 spec
        case GL_UNSIGNED_BYTE:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_5_6_5:
            return context->getClientVersion() >= ES_2_0;

        // Types referenced in Table 3.2 of the ES 3.0.5 spec (Except depth stencil)
        case GL_BYTE:
        case GL_INT:
        case GL_SHORT:
        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_INT_10F_11F_11F_REV:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
        case GL_UNSIGNED_INT_5_9_9_9_REV:
        case GL_UNSIGNED_SHORT:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT:
            return context->getClientVersion() >= ES_3_0;

        case GL_FLOAT:
            return context->getClientVersion() >= ES_3_0 ||
                   context->getExtensions().textureFloatOES ||
                   context->getExtensions().colorBufferHalfFloatEXT;

        case GL_HALF_FLOAT:
            return context->getClientVersion() >= ES_3_0 ||
                   context->getExtensions().textureHalfFloatOES;

        case GL_HALF_FLOAT_OES:
            return context->getExtensions().colorBufferHalfFloatEXT;

        default:
            return false;
    }
}

bool ValidReadPixelsFormatEnum(const Context *context, GLenum format)
{
    switch (format)
    {
        // Formats referenced in Table 3.4 of the ES 2.0.25 spec (Except luminance)
        case GL_RGBA:
        case GL_RGB:
        case GL_ALPHA:
            return context->getClientVersion() >= ES_2_0;

        // Formats referenced in Table 3.2 of the ES 3.0.5 spec
        case GL_RG:
        case GL_RED:
        case GL_RGBA_INTEGER:
        case GL_RGB_INTEGER:
        case GL_RG_INTEGER:
        case GL_RED_INTEGER:
            return context->getClientVersion() >= ES_3_0;

        case GL_SRGB_ALPHA_EXT:
        case GL_SRGB_EXT:
            return context->getExtensions().sRGBEXT;

        case GL_BGRA_EXT:
            return context->getExtensions().readFormatBgraEXT;

        case GL_RGBX8_ANGLE:
            return context->getExtensions().rgbxInternalFormatANGLE;

        default:
            return false;
    }
}

bool ValidReadPixelsUnsignedNormalizedDepthType(const Context *context,
                                                const gl::InternalFormat *info,
                                                GLenum type)
{
    bool supportsReadDepthNV = context->getExtensions().readDepthNV && (info->depthBits > 0);
    switch (type)
    {
        case GL_UNSIGNED_SHORT:
        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_INT_24_8:
            return supportsReadDepthNV;
        default:
            return false;
    }
}

bool ValidReadPixelsFloatDepthType(const Context *context,
                                   const gl::InternalFormat *info,
                                   GLenum type)
{
    return context->getExtensions().readDepthNV && (type == GL_FLOAT) &&
           context->getExtensions().depthBufferFloat2NV;
}

bool ValidReadPixelsFormatType(const Context *context,
                               const gl::InternalFormat *info,
                               GLenum format,
                               GLenum type)
{
    switch (info->componentType)
    {
        case GL_UNSIGNED_NORMALIZED:
            // TODO(geofflang): Don't accept BGRA here.  Some chrome internals appear to try to use
            // ReadPixels with BGRA even if the extension is not present
            switch (format)
            {
                case GL_RGBA:
                    return type == GL_UNSIGNED_BYTE ||
                           (context->getExtensions().textureNorm16EXT &&
                            type == GL_UNSIGNED_SHORT && info->type == GL_UNSIGNED_SHORT);
                case GL_BGRA_EXT:
                    return context->getExtensions().readFormatBgraEXT && (type == GL_UNSIGNED_BYTE);
                case GL_STENCIL_INDEX_OES:
                    return context->getExtensions().readStencilNV && (type == GL_UNSIGNED_BYTE);
                case GL_DEPTH_COMPONENT:
                    return ValidReadPixelsUnsignedNormalizedDepthType(context, info, type);
                case GL_DEPTH_STENCIL_OES:
                    return context->getExtensions().readDepthStencilNV &&
                           (type == GL_UNSIGNED_INT_24_8_OES) && info->stencilBits > 0;
                case GL_RGBX8_ANGLE:
                    return context->getExtensions().rgbxInternalFormatANGLE &&
                           (type == GL_UNSIGNED_BYTE);
                default:
                    return false;
            }
        case GL_SIGNED_NORMALIZED:
            ASSERT(context->getExtensions().renderSnormEXT);
            ASSERT(info->type == GL_BYTE ||
                   (context->getExtensions().textureNorm16EXT && info->type == GL_SHORT));
            // Type conversions are not allowed for signed normalized color buffers
            return format == GL_RGBA && type == info->type;

        case GL_INT:
            return (format == GL_RGBA_INTEGER && type == GL_INT);

        case GL_UNSIGNED_INT:
            return (format == GL_RGBA_INTEGER && type == GL_UNSIGNED_INT);

        case GL_FLOAT:
            switch (format)
            {
                case GL_RGBA:
                    return (type == GL_FLOAT);
                case GL_DEPTH_COMPONENT:
                    return ValidReadPixelsFloatDepthType(context, info, type);
                case GL_DEPTH_STENCIL_OES:
                    return context->getExtensions().readDepthStencilNV &&
                           type == GL_FLOAT_32_UNSIGNED_INT_24_8_REV && info->stencilBits > 0;
                default:
                    return false;
            }
        default:
            UNREACHABLE();
            return false;
    }
}

template <typename ParamType>
bool ValidateTextureWrapModeValue(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  const ParamType *params,
                                  bool restrictedWrapModes)
{
    switch (ConvertToGLenum(params[0]))
    {
        case GL_CLAMP_TO_EDGE:
            break;

        case GL_CLAMP_TO_BORDER:
            if (!context->getExtensions().textureBorderClampAny() &&
                context->getClientVersion() < ES_3_2)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kExtensionNotEnabled);
                return false;
            }
            if (restrictedWrapModes)
            {
                // OES_EGL_image_external and ANGLE_texture_rectangle specify this error.
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidWrapModeTexture);
                return false;
            }
            break;

        case GL_REPEAT:
        case GL_MIRRORED_REPEAT:
            if (restrictedWrapModes)
            {
                // OES_EGL_image_external and ANGLE_texture_rectangle specify this error.
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidWrapModeTexture);
                return false;
            }
            break;

        case GL_MIRROR_CLAMP_TO_EDGE_EXT:
            if (!context->getExtensions().textureMirrorClampToEdgeEXT)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kExtensionNotEnabled);
                return false;
            }
            if (restrictedWrapModes)
            {
                // OES_EGL_image_external and ANGLE_texture_rectangle specify this error.
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidWrapModeTexture);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureWrap);
            return false;
    }

    return true;
}

template <typename ParamType>
bool ValidateTextureMinFilterValue(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   const ParamType *params,
                                   bool restrictedMinFilter)
{
    switch (ConvertToGLenum(params[0]))
    {
        case GL_NEAREST:
        case GL_LINEAR:
            break;

        case GL_NEAREST_MIPMAP_NEAREST:
        case GL_LINEAR_MIPMAP_NEAREST:
        case GL_NEAREST_MIPMAP_LINEAR:
        case GL_LINEAR_MIPMAP_LINEAR:
            if (restrictedMinFilter)
            {
                // OES_EGL_image_external specifies this error.
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFilterTexture);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureFilterParam);
            return false;
    }

    return true;
}

template <typename ParamType>
bool ValidateTextureMagFilterValue(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   const ParamType *params)
{
    switch (ConvertToGLenum(params[0]))
    {
        case GL_NEAREST:
        case GL_LINEAR:
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureFilterParam);
            return false;
    }

    return true;
}

template <typename ParamType>
bool ValidateTextureCompareModeValue(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     const ParamType *params)
{
    // Acceptable mode parameters from GLES 3.0.2 spec, table 3.17
    switch (ConvertToGLenum(params[0]))
    {
        case GL_NONE:
        case GL_COMPARE_REF_TO_TEXTURE:
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kUnknownParameter);
            return false;
    }

    return true;
}

template <typename ParamType>
bool ValidateTextureCompareFuncValue(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     const ParamType *params)
{
    // Acceptable function parameters from GLES 3.0.2 spec, table 3.17
    switch (ConvertToGLenum(params[0]))
    {
        case GL_LEQUAL:
        case GL_GEQUAL:
        case GL_LESS:
        case GL_GREATER:
        case GL_EQUAL:
        case GL_NOTEQUAL:
        case GL_ALWAYS:
        case GL_NEVER:
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kUnknownParameter);
            return false;
    }

    return true;
}

template <typename ParamType>
bool ValidateTextureSRGBDecodeValue(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    const ParamType *params)
{
    if (!context->getExtensions().textureSRGBDecodeEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kExtensionNotEnabled);
        return false;
    }

    switch (ConvertToGLenum(params[0]))
    {
        case GL_DECODE_EXT:
        case GL_SKIP_DECODE_EXT:
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kUnknownParameter);
            return false;
    }

    return true;
}

template <typename ParamType>
bool ValidateTextureSRGBOverrideValue(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      const ParamType *params)
{
    if (!context->getExtensions().textureFormatSRGBOverrideEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kExtensionNotEnabled);
        return false;
    }

    switch (ConvertToGLenum(params[0]))
    {
        case GL_SRGB:
        case GL_NONE:
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kUnknownParameter);
            return false;
    }

    return true;
}

bool ValidateTextureMaxAnisotropyExtensionEnabled(const Context *context,
                                                  angle::EntryPoint entryPoint)
{
    if (!context->getExtensions().textureFilterAnisotropicEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kExtensionNotEnabled);
        return false;
    }

    return true;
}

bool ValidateTextureMaxAnisotropyValue(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       GLfloat paramValue)
{
    if (!ValidateTextureMaxAnisotropyExtensionEnabled(context, entryPoint))
    {
        return false;
    }

    GLfloat largest = context->getCaps().maxTextureAnisotropy;

    if (paramValue < 1 || paramValue > largest)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kOutsideOfBounds);
        return false;
    }

    return true;
}

bool ValidateFragmentShaderColorBufferMaskMatch(const Context *context)
{
    const auto &glState                 = context->getState();
    const ProgramExecutable *executable = context->getState().getLinkedProgramExecutable(context);
    const Framebuffer *framebuffer      = glState.getDrawFramebuffer();

    const auto &blendStateExt = glState.getBlendStateExt();
    auto drawBufferMask = framebuffer->getDrawBufferMask() & blendStateExt.compareColorMask(0);
    auto dualSourceBlendingMask = drawBufferMask & blendStateExt.getEnabledMask() &
                                  blendStateExt.getUsesExtendedBlendFactorMask();
    auto fragmentOutputMask          = executable->getActiveOutputVariablesMask();
    auto fragmentSecondaryOutputMask = executable->getActiveSecondaryOutputVariablesMask();

    return drawBufferMask == (drawBufferMask & fragmentOutputMask) &&
           dualSourceBlendingMask == (dualSourceBlendingMask & fragmentSecondaryOutputMask);
}

bool ValidateFragmentShaderColorBufferTypeMatch(const Context *context)
{
    const ProgramExecutable *executable = context->getState().getLinkedProgramExecutable(context);
    const Framebuffer *framebuffer      = context->getState().getDrawFramebuffer();

    return ValidateComponentTypeMasks(executable->getFragmentOutputsTypeMask().to_ulong(),
                                      framebuffer->getDrawBufferTypeMask().to_ulong(),
                                      executable->getActiveOutputVariablesMask().to_ulong(),
                                      framebuffer->getDrawBufferMask().to_ulong());
}

bool ValidateVertexShaderAttributeTypeMatch(const Context *context)
{
    const auto &glState                 = context->getState();
    const ProgramExecutable *executable = context->getState().getLinkedProgramExecutable(context);
    const VertexArray *vao              = context->getState().getVertexArray();

    if (executable == nullptr)
    {
        return false;
    }

    unsigned long stateCurrentValuesTypeBits = glState.getCurrentValuesTypeMask().to_ulong();
    unsigned long vaoAttribTypeBits          = vao->getAttributesTypeMask().to_ulong();
    unsigned long vaoAttribEnabledMask       = vao->getAttributesMask().to_ulong();

    vaoAttribEnabledMask |= vaoAttribEnabledMask << kMaxComponentTypeMaskIndex;
    vaoAttribTypeBits = (vaoAttribEnabledMask & vaoAttribTypeBits);
    vaoAttribTypeBits |= (~vaoAttribEnabledMask & stateCurrentValuesTypeBits);

    return ValidateComponentTypeMasks(executable->getAttributesTypeMask().to_ulong(),
                                      vaoAttribTypeBits, executable->getAttributesMask().to_ulong(),
                                      0xFFFF);
}

bool IsCompatibleDrawModeWithGeometryShader(PrimitiveMode drawMode,
                                            PrimitiveMode geometryShaderInputPrimitiveType)
{
    // [EXT_geometry_shader] Section 11.1gs.1, Geometry Shader Input Primitives
    switch (drawMode)
    {
        case PrimitiveMode::Points:
            return geometryShaderInputPrimitiveType == PrimitiveMode::Points;
        case PrimitiveMode::Lines:
        case PrimitiveMode::LineStrip:
        case PrimitiveMode::LineLoop:
            return geometryShaderInputPrimitiveType == PrimitiveMode::Lines;
        case PrimitiveMode::LinesAdjacency:
        case PrimitiveMode::LineStripAdjacency:
            return geometryShaderInputPrimitiveType == PrimitiveMode::LinesAdjacency;
        case PrimitiveMode::Triangles:
        case PrimitiveMode::TriangleFan:
        case PrimitiveMode::TriangleStrip:
            return geometryShaderInputPrimitiveType == PrimitiveMode::Triangles;
        case PrimitiveMode::TrianglesAdjacency:
        case PrimitiveMode::TriangleStripAdjacency:
            return geometryShaderInputPrimitiveType == PrimitiveMode::TrianglesAdjacency;
        default:
            UNREACHABLE();
            return false;
    }
}

// GLES1 texture parameters are a small subset of the others
bool IsValidGLES1TextureParameter(GLenum pname)
{
    switch (pname)
    {
        case GL_TEXTURE_MAG_FILTER:
        case GL_TEXTURE_MIN_FILTER:
        case GL_TEXTURE_WRAP_S:
        case GL_TEXTURE_WRAP_T:
        case GL_TEXTURE_WRAP_R:
        case GL_GENERATE_MIPMAP:
        case GL_TEXTURE_CROP_RECT_OES:
            return true;
        default:
            return false;
    }
}

unsigned int GetSamplerParameterCount(GLenum pname)
{
    return pname == GL_TEXTURE_BORDER_COLOR ? 4 : 1;
}

const char *ValidateProgramDrawAdvancedBlendState(const Context *context,
                                                  const ProgramExecutable &executable)
{
    const State &state                                 = context->getState();
    const BlendEquationBitSet &supportedBlendEquations = executable.getAdvancedBlendEquations();
    const DrawBufferMask &enabledDrawBufferMask        = state.getBlendStateExt().getEnabledMask();

    // Zero (default) means everything is BlendEquationType::Add, so check can be skipped
    if (state.getBlendStateExt().getEquationColorBits() != 0)
    {
        for (size_t blendEnabledBufferIndex : enabledDrawBufferMask)
        {
            const gl::BlendEquationType enabledBlendEquation =
                state.getBlendStateExt().getEquationColorIndexed(blendEnabledBufferIndex);

            if (enabledBlendEquation < gl::BlendEquationType::Multiply ||
                enabledBlendEquation > gl::BlendEquationType::HslLuminosity)
            {
                continue;
            }

            if (!supportedBlendEquations.test(enabledBlendEquation))
            {
                return gl::err::kBlendEquationNotEnabled;
            }
        }
    }

    return nullptr;
}

ANGLE_INLINE GLenum ShPixelLocalStorageFormatToGLenum(ShPixelLocalStorageFormat format)
{
    switch (format)
    {
        case ShPixelLocalStorageFormat::NotPLS:
            return GL_NONE;
        case ShPixelLocalStorageFormat::RGBA8:
            return GL_RGBA8;
        case ShPixelLocalStorageFormat::RGBA8I:
            return GL_RGBA8I;
        case ShPixelLocalStorageFormat::RGBA8UI:
            return GL_RGBA8UI;
        case ShPixelLocalStorageFormat::R32UI:
            return GL_R32UI;
        case ShPixelLocalStorageFormat::R32F:
            return GL_R32F;
    }
    UNREACHABLE();
    return GL_NONE;
}

ANGLE_INLINE const char *ValidateProgramDrawStates(const Context *context,
                                                   const Extensions &extensions,
                                                   const ProgramExecutable &executable)
{
    const State &state = context->getState();
    if (extensions.multiviewOVR || extensions.multiview2OVR)
    {
        const int programNumViews     = executable.usesMultiview() ? executable.getNumViews() : 1;
        Framebuffer *framebuffer      = state.getDrawFramebuffer();
        const int framebufferNumViews = framebuffer->getNumViews();

        if (framebufferNumViews != programNumViews)
        {
            return gl::err::kMultiviewMismatch;
        }

        if (state.isTransformFeedbackActiveUnpaused() && framebufferNumViews > 1)
        {
            return gl::err::kMultiviewTransformFeedback;
        }

        if (extensions.disjointTimerQueryEXT && framebufferNumViews > 1 &&
            state.isQueryActive(QueryType::TimeElapsed))
        {
            return gl::err::kMultiviewTimerQuery;
        }
    }

    // Uniform buffer validation
    for (size_t uniformBlockIndex = 0; uniformBlockIndex < executable.getUniformBlocks().size();
         uniformBlockIndex++)
    {
        const InterfaceBlock &uniformBlock = executable.getUniformBlockByIndex(uniformBlockIndex);
        GLuint blockBinding                = executable.getUniformBlockBinding(uniformBlockIndex);
        const OffsetBindingPointer<Buffer> &uniformBuffer =
            state.getIndexedUniformBuffer(blockBinding);

        if (uniformBuffer.get() == nullptr && context->isWebGL())
        {
            // undefined behaviour
            return gl::err::kUniformBufferUnbound;
        }

        size_t uniformBufferSize = GetBoundBufferAvailableSize(uniformBuffer);
        if (uniformBufferSize < uniformBlock.pod.dataSize &&
            (context->isWebGL() || context->isBufferAccessValidationEnabled()))
        {
            // undefined behaviour
            return gl::err::kUniformBufferTooSmall;
        }

        if (uniformBuffer->hasWebGLXFBBindingConflict(context->isWebGL()))
        {
            return gl::err::kUniformBufferBoundForTransformFeedback;
        }
    }

    // ANGLE_shader_pixel_local_storage validation.
    if (extensions.shaderPixelLocalStorageANGLE)
    {
        const Framebuffer *framebuffer = state.getDrawFramebuffer();
        const PixelLocalStorage *pls   = framebuffer->peekPixelLocalStorage();
        const auto &shaderPLSFormats   = executable.getPixelLocalStorageFormats();
        size_t activePLSCount          = context->getState().getPixelLocalStorageActivePlanes();

        if (shaderPLSFormats.size() > activePLSCount)
        {
            // INVALID_OPERATION is generated if a draw is issued with a fragment shader that has a
            // pixel local uniform bound to an inactive pixel local storage plane.
            return gl::err::kPLSDrawProgramPlanesInactive;
        }

        if (shaderPLSFormats.size() < activePLSCount)
        {
            // INVALID_OPERATION is generated if a draw is issued with a fragment shader that does
            // _not_ have a pixel local uniform bound to an _active_ pixel local storage plane
            // (i.e., the fragment shader must declare uniforms bound to every single active pixel
            // local storage plane).
            return gl::err::kPLSDrawProgramActivePlanesUnused;
        }

        for (size_t i = 0; i < activePLSCount; ++i)
        {
            const auto &plsPlane = pls->getPlane(static_cast<GLint>(i));
            ASSERT(plsPlane.isActive());
            if (shaderPLSFormats[i] == ShPixelLocalStorageFormat::NotPLS)
            {
                // INVALID_OPERATION is generated if a draw is issued with a fragment shader that
                // does _not_ have a pixel local uniform bound to an _active_ pixel local storage
                // plane (i.e., the fragment shader must declare uniforms bound to every single
                // active pixel local storage plane).
                return gl::err::kPLSDrawProgramActivePlanesUnused;
            }

            if (ShPixelLocalStorageFormatToGLenum(shaderPLSFormats[i]) !=
                plsPlane.getInternalformat())
            {
                // INVALID_OPERATION is generated if a draw is issued with a fragment shader that
                // has a pixel local storage uniform whose format layout qualifier does not
                // identically match the internalformat of its associated pixel local storage plane
                // on the current draw framebuffer, as enumerated in Table X.3.
                return gl::err::kPLSDrawProgramFormatMismatch;
            }
        }
    }

    // Enabled blend equation validation
    const char *errorString = nullptr;

    if (extensions.blendEquationAdvancedKHR)
    {
        errorString = ValidateProgramDrawAdvancedBlendState(context, executable);
    }

    return errorString;
}
}  // anonymous namespace

void SetRobustLengthParam(const GLsizei *length, GLsizei value)
{
    if (length)
    {
        // Currently we modify robust length parameters in the validation layer. We should be only
        // doing this in the Context instead.
        // TODO(http://anglebug.com/42263032): Remove when possible.
        *const_cast<GLsizei *>(length) = value;
    }
}

bool ValidTextureTarget(const Context *context, TextureType type)
{
    switch (type)
    {
        case TextureType::_2D:
        case TextureType::CubeMap:
            return true;

        case TextureType::Rectangle:
            return context->getExtensions().textureRectangleANGLE;

        case TextureType::_3D:
            return context->getClientVersion() >= ES_3_0 || context->getExtensions().texture3DOES;

        case TextureType::_2DArray:
            return context->getClientVersion() >= ES_3_0;

        case TextureType::_2DMultisample:
            return context->getClientVersion() >= ES_3_1 ||
                   context->getExtensions().textureMultisampleANGLE;

        case TextureType::_2DMultisampleArray:
            return context->getClientVersion() >= ES_3_2 ||
                   context->getExtensions().textureStorageMultisample2dArrayOES;

        case TextureType::CubeMapArray:
            return context->getClientVersion() >= ES_3_2 ||
                   context->getExtensions().textureCubeMapArrayAny();

        case TextureType::VideoImage:
            return context->getExtensions().videoTextureWEBGL;

        case TextureType::Buffer:
            return context->getClientVersion() >= ES_3_2 ||
                   context->getExtensions().textureBufferAny();

        default:
            return false;
    }
}

bool ValidTexture2DTarget(const Context *context, TextureType type)
{
    switch (type)
    {
        case TextureType::_2D:
        case TextureType::CubeMap:
            return true;

        case TextureType::Rectangle:
            return context->getExtensions().textureRectangleANGLE;

        default:
            return false;
    }
}

bool ValidTexture3DTarget(const Context *context, TextureType target)
{
    switch (target)
    {
        case TextureType::_3D:
        case TextureType::_2DArray:
            return (context->getClientMajorVersion() >= 3);

        case TextureType::CubeMapArray:
            return (context->getClientVersion() >= Version(3, 2) ||
                    context->getExtensions().textureCubeMapArrayAny());

        default:
            return false;
    }
}

// Most texture GL calls are not compatible with external textures, so we have a separate validation
// function for use in the GL calls that do
bool ValidTextureExternalTarget(const Context *context, TextureType target)
{
    return (target == TextureType::External) &&
           (context->getExtensions().EGLImageExternalOES ||
            context->getExtensions().EGLStreamConsumerExternalNV);
}

bool ValidTextureExternalTarget(const Context *context, TextureTarget target)
{
    return (target == TextureTarget::External) &&
           ValidTextureExternalTarget(context, TextureType::External);
}

// This function differs from ValidTextureTarget in that the target must be
// usable as the destination of a 2D operation-- so a cube face is valid, but
// GL_TEXTURE_CUBE_MAP is not.
// Note: duplicate of IsInternalTextureTarget
bool ValidTexture2DDestinationTarget(const Context *context, TextureTarget target)
{
    switch (target)
    {
        case TextureTarget::_2D:
        case TextureTarget::CubeMapNegativeX:
        case TextureTarget::CubeMapNegativeY:
        case TextureTarget::CubeMapNegativeZ:
        case TextureTarget::CubeMapPositiveX:
        case TextureTarget::CubeMapPositiveY:
        case TextureTarget::CubeMapPositiveZ:
            return true;
        case TextureTarget::Rectangle:
            return context->getExtensions().textureRectangleANGLE;
        case TextureTarget::VideoImage:
            return context->getExtensions().videoTextureWEBGL;
        default:
            return false;
    }
}

bool ValidateTransformFeedbackPrimitiveMode(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            PrimitiveMode transformFeedbackPrimitiveMode,
                                            PrimitiveMode renderPrimitiveMode)
{
    ASSERT(context);

    if ((!context->getExtensions().geometryShaderAny() ||
         !context->getExtensions().tessellationShaderAny()) &&
        context->getClientVersion() < ES_3_2)
    {
        // It is an invalid operation to call DrawArrays or DrawArraysInstanced with a draw mode
        // that does not match the current transform feedback object's draw mode (if transform
        // feedback is active), (3.0.2, section 2.14, pg 86)
        return transformFeedbackPrimitiveMode == renderPrimitiveMode;
    }

    const ProgramExecutable *executable = context->getState().getLinkedProgramExecutable(context);
    ASSERT(executable);
    if (executable->hasLinkedShaderStage(ShaderType::Geometry))
    {
        // If geometry shader is active, transform feedback mode must match what is output from this
        // stage.
        renderPrimitiveMode = executable->getGeometryShaderOutputPrimitiveType();
    }
    else if (executable->hasLinkedShaderStage(ShaderType::TessEvaluation))
    {
        // Similarly with tessellation shaders, but only if no geometry shader is present.  With
        // tessellation shaders, only triangles are possibly output.
        return transformFeedbackPrimitiveMode == PrimitiveMode::Triangles &&
               executable->getTessGenMode() == GL_TRIANGLES;
    }

    // [GL_EXT_geometry_shader] Table 12.1gs
    switch (renderPrimitiveMode)
    {
        case PrimitiveMode::Points:
            return transformFeedbackPrimitiveMode == PrimitiveMode::Points;
        case PrimitiveMode::Lines:
        case PrimitiveMode::LineStrip:
        case PrimitiveMode::LineLoop:
            return transformFeedbackPrimitiveMode == PrimitiveMode::Lines;
        case PrimitiveMode::Triangles:
        case PrimitiveMode::TriangleFan:
        case PrimitiveMode::TriangleStrip:
            return transformFeedbackPrimitiveMode == PrimitiveMode::Triangles;
        case PrimitiveMode::Patches:
            return transformFeedbackPrimitiveMode == PrimitiveMode::Patches;
        default:
            UNREACHABLE();
            return false;
    }
}

bool ValidateDrawElementsInstancedBase(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       PrimitiveMode mode,
                                       GLsizei count,
                                       DrawElementsType type,
                                       const void *indices,
                                       GLsizei primcount,
                                       GLuint baseinstance)
{
    if (primcount <= 0)
    {
        if (primcount < 0)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativePrimcount);
            return false;
        }

        // Early exit.
        return ValidateDrawElementsCommon(context, entryPoint, mode, count, type, indices,
                                          primcount);
    }

    if (!ValidateDrawElementsCommon(context, entryPoint, mode, count, type, indices, primcount))
    {
        return false;
    }

    if (count == 0)
    {
        // Early exit.
        return true;
    }

    return ValidateDrawInstancedAttribs(context, entryPoint, primcount, baseinstance);
}

bool ValidateDrawArraysInstancedBase(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     PrimitiveMode mode,
                                     GLint first,
                                     GLsizei count,
                                     GLsizei primcount,
                                     GLuint baseinstance)
{
    if (primcount <= 0)
    {
        if (primcount < 0)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativePrimcount);
            return false;
        }

        // Early exit.
        return ValidateDrawArraysCommon(context, entryPoint, mode, first, count, primcount);
    }

    if (!ValidateDrawArraysCommon(context, entryPoint, mode, first, count, primcount))
    {
        return false;
    }

    if (count == 0)
    {
        // Early exit.
        return true;
    }

    return ValidateDrawInstancedAttribs(context, entryPoint, primcount, baseinstance);
}

bool ValidateDrawInstancedANGLE(const Context *context, angle::EntryPoint entryPoint)
{
    // Verify there is at least one active attribute with a divisor of zero
    const State &state                  = context->getState();
    const ProgramExecutable *executable = state.getLinkedProgramExecutable(context);

    if (!executable)
    {
        // No executable means there is no Program/PPO bound, which is undefined behavior, but isn't
        // an error.
        context->getState().getDebug().insertMessage(
            GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, 0, GL_DEBUG_SEVERITY_HIGH,
            std::string("Attempting to draw without a program"), gl::LOG_WARN, entryPoint);
        return true;
    }

    const auto &attribs  = state.getVertexArray()->getVertexAttributes();
    const auto &bindings = state.getVertexArray()->getVertexBindings();
    for (size_t attributeIndex = 0; attributeIndex < attribs.size(); attributeIndex++)
    {
        const VertexAttribute &attrib = attribs[attributeIndex];
        const VertexBinding &binding  = bindings[attrib.bindingIndex];
        if (executable->isAttribLocationActive(attributeIndex) && binding.getDivisor() == 0)
        {
            return true;
        }
    }

    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNoZeroDivisor);
    return false;
}

bool ValidTexture3DDestinationTarget(const Context *context, TextureTarget target)
{
    switch (target)
    {
        case TextureTarget::_3D:
            return true;
        case TextureTarget::_2DArray:
            return context->getClientVersion() >= Version(3, 0);
        case TextureTarget::CubeMapArray:
            return (context->getClientVersion() >= Version(3, 2) ||
                    context->getExtensions().textureCubeMapArrayAny());
        default:
            return false;
    }
}

bool ValidTexLevelDestinationTarget(const Context *context, TextureType type)
{
    switch (type)
    {
        case TextureType::_2D:
        case TextureType::CubeMap:
            return true;
        case TextureType::_2DArray:
            return context->getClientVersion() >= ES_3_0;
        case TextureType::_2DMultisample:
            return context->getClientVersion() >= ES_3_1 ||
                   context->getExtensions().textureMultisampleANGLE;
        case TextureType::_2DMultisampleArray:
            return context->getClientVersion() >= ES_3_2 ||
                   context->getExtensions().textureStorageMultisample2dArrayOES;
        case TextureType::_3D:
            return context->getClientVersion() >= ES_3_0 || context->getExtensions().texture3DOES;
        case TextureType::CubeMapArray:
            return context->getClientVersion() >= ES_3_2 ||
                   context->getExtensions().textureCubeMapArrayAny();
        case TextureType::Rectangle:
            return context->getExtensions().textureRectangleANGLE;
        case TextureType::Buffer:
            return context->getClientVersion() >= ES_3_2 ||
                   context->getExtensions().textureBufferAny();
        default:
            return false;
    }
}

bool ValidFramebufferTarget(const Context *context, GLenum target)
{
    static_assert(GL_DRAW_FRAMEBUFFER_ANGLE == GL_DRAW_FRAMEBUFFER &&
                      GL_READ_FRAMEBUFFER_ANGLE == GL_READ_FRAMEBUFFER,
                  "ANGLE framebuffer enums must equal the ES3 framebuffer enums.");

    switch (target)
    {
        case GL_FRAMEBUFFER:
            return true;

        case GL_READ_FRAMEBUFFER:
        case GL_DRAW_FRAMEBUFFER:
            return (context->getExtensions().framebufferBlitAny() ||
                    context->getClientMajorVersion() >= 3);

        default:
            return false;
    }
}

bool ValidMipLevel(const Context *context, TextureType type, GLint level)
{
    const auto &caps = context->getCaps();
    int maxDimension = 0;
    switch (type)
    {
        case TextureType::_2D:
        case TextureType::_2DArray:
            maxDimension = caps.max2DTextureSize;
            break;

        case TextureType::CubeMap:
        case TextureType::CubeMapArray:
            maxDimension = caps.maxCubeMapTextureSize;
            break;

        case TextureType::External:
        case TextureType::Rectangle:
        case TextureType::VideoImage:
        case TextureType::Buffer:
        case TextureType::_2DMultisample:
        case TextureType::_2DMultisampleArray:
            return level == 0;

        case TextureType::_3D:
            maxDimension = caps.max3DTextureSize;
            break;

        default:
            UNREACHABLE();
    }

    return level <= log2(maxDimension) && level >= 0;
}

bool ValidImageSizeParameters(const Context *context,
                              angle::EntryPoint entryPoint,
                              TextureType target,
                              GLint level,
                              GLsizei width,
                              GLsizei height,
                              GLsizei depth,
                              bool isSubImage)
{
    if (width < 0 || height < 0 || depth < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeSize);
        return false;
    }
    // TexSubImage parameters can be NPOT without textureNPOT extension,
    // as long as the destination texture is POT.
    bool hasNPOTSupport =
        context->getExtensions().textureNpotOES || context->getClientVersion() >= Version(3, 0);
    if (!isSubImage && !hasNPOTSupport &&
        (level != 0 && (!isPow2(width) || !isPow2(height) || !isPow2(depth))))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kTextureNotPow2);
        return false;
    }

    if (!ValidMipLevel(context, target, level))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevel);
        return false;
    }

    return true;
}

bool ValidCompressedBaseLevel(GLsizei size, GLuint blockSize, GLint level)
{
    // Already checked in ValidMipLevel.
    ASSERT(level < 32);
    // This function is used only for 4x4 BC formats.
    ASSERT(blockSize == 4);
    // Use the constant value to avoid division.
    return ((size << level) % 4) == 0;
}

bool ValidCompressedImageSize(const Context *context,
                              GLenum internalFormat,
                              GLint level,
                              GLsizei width,
                              GLsizei height,
                              GLsizei depth)
{
    if (width < 0 || height < 0)
    {
        return false;
    }

    const InternalFormat &formatInfo = GetSizedInternalFormatInfo(internalFormat);

    if (!formatInfo.compressed && !formatInfo.paletted)
    {
        return false;
    }

    // A texture format can not be both block-compressed and paletted
    ASSERT(!(formatInfo.compressed && formatInfo.paletted));

    if (formatInfo.compressed)
    {
        // Only PVRTC1 requires dimensions to be powers of two
        if (IsPVRTC1Format(internalFormat))
        {
            if (!isPow2(width) || !isPow2(height))
            {
                return false;
            }

            if (context->getLimitations().squarePvrtc1)
            {
                if (width != height)
                {
                    return false;
                }
            }
        }

        if (CompressedTextureFormatRequiresExactSize(internalFormat))
        {
            // In WebGL compatibility mode and D3D, enforce that the base level implied
            // by the compressed texture's mip level would conform to the block
            // size.
            if (context->isWebGL() ||
                context->getLimitations().compressedBaseMipLevelMultipleOfFour)
            {
                // This check is performed only for BC formats.
                ASSERT(formatInfo.compressedBlockDepth == 1);
                if (!ValidCompressedBaseLevel(width, formatInfo.compressedBlockWidth, level) ||
                    !ValidCompressedBaseLevel(height, formatInfo.compressedBlockHeight, level))
                {
                    return false;
                }
            }
            // non-WebGL and non-D3D check is not necessary for the following formats
            // From EXT_texture_compression_s3tc specification:
            // If the width or height is not a multiple of four, there will be 4x4 blocks at the
            // edge of the image that contain "extra" texels that are not part of the image. From
            // EXT_texture_compression_bptc & EXT_texture_compression_rgtc specification: If an
            // RGTC/BPTC image has a width or height that is not a multiple of four, the data
            // corresponding to texels outside the image are irrelevant and undefined.
        }
    }

    if (formatInfo.paletted)
    {
        // TODO(http://anglebug.com/42266155): multi-level paletted images
        if (level != 0)
        {
            return false;
        }

        if (!isPow2(width) || !isPow2(height))
        {
            return false;
        }
    }

    return true;
}

bool ValidCompressedSubImageSize(const Context *context,
                                 GLenum internalFormat,
                                 GLint xoffset,
                                 GLint yoffset,
                                 GLint zoffset,
                                 GLsizei width,
                                 GLsizei height,
                                 GLsizei depth,
                                 size_t textureWidth,
                                 size_t textureHeight,
                                 size_t textureDepth)
{
    // Passing non-compressed internal format to sub-image compressed entry points generates
    // INVALID_OPERATION, so check it here.
    const InternalFormat &formatInfo = GetSizedInternalFormatInfo(internalFormat);
    if (!formatInfo.compressed)
    {
        return false;
    }

    // Negative dimensions already checked in ValidImageSizeParameters called by
    // ValidateES2TexImageParametersBase or ValidateES3TexImageParametersBase.
    ASSERT(width >= 0 && height >= 0 && depth >= 0);

    // Negative and overflowed offsets already checked in ValidateES2TexImageParametersBase or
    // ValidateES3TexImageParametersBase.
    ASSERT(xoffset >= 0 && yoffset >= 0 && zoffset >= 0);
    ASSERT(std::numeric_limits<GLsizei>::max() - xoffset >= width &&
           std::numeric_limits<GLsizei>::max() - yoffset >= height &&
           std::numeric_limits<GLsizei>::max() - zoffset >= depth);

    // Ensure that format's block dimensions are set.
    ASSERT(formatInfo.compressedBlockWidth > 0 && formatInfo.compressedBlockHeight > 0 &&
           formatInfo.compressedBlockDepth > 0);

    // Check if the whole image is being replaced. For 2D texture blocks, zoffset and depth do not
    // affect whether the replaced region fills the entire image.
    if ((xoffset == 0 && static_cast<size_t>(width) == textureWidth) &&
        (yoffset == 0 && static_cast<size_t>(height) == textureHeight) &&
        ((zoffset == 0 && static_cast<size_t>(depth) == textureDepth) ||
         formatInfo.compressedBlockDepth == 1))
    {
        // All compressed formats support whole image replacement, early pass.
        return true;
    }

    // The replaced region does not match the image size. Fail if the format does not support
    // partial updates.
    if (CompressedFormatRequiresWholeImage(internalFormat))
    {
        return false;
    }

    // The format supports partial updates. Check that the origin of the replaced region is aligned
    // to block boundaries.
    if (xoffset % formatInfo.compressedBlockWidth != 0 ||
        yoffset % formatInfo.compressedBlockHeight != 0 ||
        zoffset % formatInfo.compressedBlockDepth != 0)
    {
        return false;
    }

    // The replaced region dimensions must either be multiples of the block dimensions or exactly
    // reach the image boundaries.
    return (static_cast<size_t>(xoffset + width) == textureWidth ||
            width % formatInfo.compressedBlockWidth == 0) &&
           (static_cast<size_t>(yoffset + height) == textureHeight ||
            height % formatInfo.compressedBlockHeight == 0) &&
           (static_cast<size_t>(zoffset + depth) == textureDepth ||
            depth % formatInfo.compressedBlockDepth == 0);
}

bool ValidImageDataSize(const Context *context,
                        angle::EntryPoint entryPoint,
                        TextureType texType,
                        GLsizei width,
                        GLsizei height,
                        GLsizei depth,
                        GLenum format,
                        GLenum type,
                        const void *pixels,
                        GLsizei imageSize)
{
    Buffer *pixelUnpackBuffer = context->getState().getTargetBuffer(BufferBinding::PixelUnpack);
    if (pixelUnpackBuffer == nullptr && imageSize < 0)
    {
        // Checks are not required
        return true;
    }

    // ...the data would be unpacked from the buffer object such that the memory reads required
    // would exceed the data store size.
    const InternalFormat &formatInfo = GetInternalFormatInfo(format, type);
    if (formatInfo.internalFormat == GL_NONE)
    {
        UNREACHABLE();
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInternalErrorFormatNotFound);
        return false;
    }
    const Extents size(width, height, depth);
    const auto &unpack = context->getState().getUnpackState();

    bool targetIs3D = texType == TextureType::_3D || texType == TextureType::_2DArray;
    GLuint endByte  = 0;
    if (!formatInfo.computePackUnpackEndByte(type, size, unpack, targetIs3D, &endByte))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kIntegerOverflow);
        return false;
    }

    if (pixelUnpackBuffer)
    {
        CheckedNumeric<size_t> checkedEndByte(endByte);
        CheckedNumeric<size_t> checkedOffset(reinterpret_cast<size_t>(pixels));
        checkedEndByte += checkedOffset;

        if (!checkedEndByte.IsValid() ||
            (checkedEndByte.ValueOrDie() > static_cast<size_t>(pixelUnpackBuffer->getSize())))
        {
            // Overflow past the end of the buffer
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kIntegerOverflow);
            return false;
        }
        if (pixelUnpackBuffer->hasWebGLXFBBindingConflict(context->isWebGL()))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                   kPixelUnpackBufferBoundForTransformFeedback);
            return false;
        }
    }
    else
    {
        ASSERT(imageSize >= 0);
        if (pixels == nullptr && imageSize != 0)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kImageSizeMustBeZero);
            return false;
        }

        if (pixels != nullptr && endByte > static_cast<GLuint>(imageSize))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kImageSizeTooSmall);
            return false;
        }
    }

    return true;
}

bool ValidQueryType(const Context *context, QueryType queryType)
{
    switch (queryType)
    {
        case QueryType::AnySamples:
        case QueryType::AnySamplesConservative:
            return context->getClientMajorVersion() >= 3 ||
                   context->getExtensions().occlusionQueryBooleanEXT;
        case QueryType::TransformFeedbackPrimitivesWritten:
            return (context->getClientMajorVersion() >= 3);
        case QueryType::TimeElapsed:
            return context->getExtensions().disjointTimerQueryEXT;
        case QueryType::CommandsCompleted:
            return context->getExtensions().syncQueryCHROMIUM;
        case QueryType::PrimitivesGenerated:
            return context->getClientVersion() >= ES_3_2 ||
                   context->getExtensions().geometryShaderAny();
        default:
            return false;
    }
}

bool ValidateWebGLVertexAttribPointer(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      VertexAttribType type,
                                      GLboolean normalized,
                                      GLsizei stride,
                                      const void *ptr,
                                      bool pureInteger)
{
    ASSERT(context->isWebGL());
    // WebGL 1.0 [Section 6.11] Vertex Attribute Data Stride
    // The WebGL API supports vertex attribute data strides up to 255 bytes. A call to
    // vertexAttribPointer will generate an INVALID_VALUE error if the value for the stride
    // parameter exceeds 255.
    constexpr GLsizei kMaxWebGLStride = 255;
    if (stride > kMaxWebGLStride)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kStrideExceedsWebGLLimit);
        return false;
    }

    // WebGL 1.0 [Section 6.4] Buffer Offset and Stride Requirements
    // The offset arguments to drawElements and vertexAttribPointer, and the stride argument to
    // vertexAttribPointer, must be a multiple of the size of the data type passed to the call,
    // or an INVALID_OPERATION error is generated.
    angle::FormatID internalType = GetVertexFormatID(type, normalized, 1, pureInteger);
    size_t typeSize              = GetVertexFormatSize(internalType);

    ASSERT(isPow2(typeSize) && typeSize > 0);
    size_t sizeMask = (typeSize - 1);
    if ((reinterpret_cast<intptr_t>(ptr) & sizeMask) != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kOffsetMustBeMultipleOfType);
        return false;
    }

    if ((stride & sizeMask) != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kStrideMustBeMultipleOfType);
        return false;
    }

    return true;
}

Program *GetValidProgramNoResolve(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID id)
{
    // ES3 spec (section 2.11.1) -- "Commands that accept shader or program object names will
    // generate the error INVALID_VALUE if the provided name is not the name of either a shader
    // or program object and INVALID_OPERATION if the provided name identifies an object
    // that is not the expected type."

    Program *validProgram = context->getProgramNoResolveLink(id);

    if (!validProgram)
    {
        if (context->getShaderNoResolveCompile(id))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExpectedProgramName);
        }
        else
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidProgramName);
        }
    }

    return validProgram;
}

Program *GetValidProgram(const Context *context, angle::EntryPoint entryPoint, ShaderProgramID id)
{
    Program *program = GetValidProgramNoResolve(context, entryPoint, id);
    if (program)
    {
        program->resolveLink(context);
    }
    return program;
}

Shader *GetValidShader(const Context *context, angle::EntryPoint entryPoint, ShaderProgramID id)
{
    // See ValidProgram for spec details.

    Shader *validShader = context->getShaderNoResolveCompile(id);

    if (!validShader)
    {
        if (context->getProgramNoResolveLink(id))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExpectedShaderName);
        }
        else
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidShaderName);
        }
    }

    return validShader;
}

bool ValidateAttachmentTarget(const Context *context,
                              angle::EntryPoint entryPoint,
                              GLenum attachment)
{
    if (attachment >= GL_COLOR_ATTACHMENT1_EXT && attachment <= GL_COLOR_ATTACHMENT15_EXT)
    {
        if (context->getClientMajorVersion() < 3 && !context->getExtensions().drawBuffersEXT)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidAttachment);
            return false;
        }

        // Color attachment 0 is validated below because it is always valid
        const int colorAttachment = (attachment - GL_COLOR_ATTACHMENT0_EXT);
        if (colorAttachment >= context->getCaps().maxColorAttachments)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidAttachment);
            return false;
        }
    }
    else
    {
        switch (attachment)
        {
            case GL_COLOR_ATTACHMENT0:
            case GL_DEPTH_ATTACHMENT:
            case GL_STENCIL_ATTACHMENT:
                break;

            case GL_DEPTH_STENCIL_ATTACHMENT:
                if (!context->isWebGL() && context->getClientMajorVersion() < 3)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidAttachment);
                    return false;
                }
                break;

            default:
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidAttachment);
                return false;
        }
    }

    return true;
}

bool ValidateRenderbufferStorageParametersBase(const Context *context,
                                               angle::EntryPoint entryPoint,
                                               GLenum target,
                                               GLsizei samples,
                                               GLenum internalformat,
                                               GLsizei width,
                                               GLsizei height)
{
    switch (target)
    {
        case GL_RENDERBUFFER:
            break;
        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidRenderbufferTarget);
            return false;
    }

    if (width < 0 || height < 0 || samples < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidRenderbufferWidthHeight);
        return false;
    }

    // Hack for the special WebGL 1 "DEPTH_STENCIL" internal format.
    GLenum convertedInternalFormat = context->getConvertedRenderbufferFormat(internalformat);

    const TextureCaps &formatCaps = context->getTextureCaps().get(convertedInternalFormat);
    if (!formatCaps.renderbuffer)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidRenderbufferInternalFormat);
        return false;
    }

    // ANGLE_framebuffer_multisample does not explicitly state that the internal format must be
    // sized but it does state that the format must be in the ES2.0 spec table 4.5 which contains
    // only sized internal formats.
    const InternalFormat &formatInfo = GetSizedInternalFormatInfo(convertedInternalFormat);
    if (formatInfo.internalFormat == GL_NONE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidRenderbufferInternalFormat);
        return false;
    }

    if (std::max(width, height) > context->getCaps().maxRenderbufferSize)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kResourceMaxRenderbufferSize);
        return false;
    }

    RenderbufferID id = context->getState().getRenderbufferId();
    if (id.value == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidRenderbufferTarget);
        return false;
    }

    return true;
}

bool ValidateBlitFramebufferParameters(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       GLint srcX0,
                                       GLint srcY0,
                                       GLint srcX1,
                                       GLint srcY1,
                                       GLint dstX0,
                                       GLint dstY0,
                                       GLint dstX1,
                                       GLint dstY1,
                                       GLbitfield mask,
                                       GLenum filter)
{
    switch (filter)
    {
        case GL_NEAREST:
            break;
        case GL_LINEAR:
            break;
        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kBlitInvalidFilter);
            return false;
    }

    if ((mask & ~(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)) != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kBlitInvalidMask);
        return false;
    }

    // ES3.0 spec, section 4.3.2 states that linear filtering is only available for the
    // color buffer, leaving only nearest being unfiltered from above
    if ((mask & ~GL_COLOR_BUFFER_BIT) != 0 && filter != GL_NEAREST)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBlitOnlyNearestForNonColor);
        return false;
    }

    const auto &glState          = context->getState();
    Framebuffer *readFramebuffer = glState.getReadFramebuffer();
    Framebuffer *drawFramebuffer = glState.getDrawFramebuffer();

    if (!readFramebuffer || !drawFramebuffer)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION, kBlitFramebufferMissing);
        return false;
    }

    if (!ValidateFramebufferComplete(context, entryPoint, readFramebuffer))
    {
        return false;
    }

    if (!ValidateFramebufferComplete(context, entryPoint, drawFramebuffer))
    {
        return false;
    }

    // The QCOM_framebuffer_foveated spec:
    if (drawFramebuffer->isFoveationEnabled())
    {
        // INVALID_OPERATION is generated by any API call which causes a framebuffer
        // attachment to be written to if the framebuffer attachments have changed for
        // a foveated fbo.
        if (drawFramebuffer->hasAnyAttachmentChanged())
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kFramebufferFoveationAttachmentChanged);
            return false;
        }
    }

    // EXT_YUV_target disallows blitting to or from a YUV framebuffer
    if ((mask & GL_COLOR_BUFFER_BIT) != 0 &&
        (readFramebuffer->hasYUVAttachment() || drawFramebuffer->hasYUVAttachment()))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBlitYUVFramebuffer);
        return false;
    }

    // The draw and read framebuffers can only match if:
    // - They are the default framebuffer AND
    // - The read/draw surfaces are different
    if ((readFramebuffer->id() == drawFramebuffer->id()) &&
        ((drawFramebuffer->id() != Framebuffer::kDefaultDrawFramebufferHandle) ||
         (context->getCurrentDrawSurface() == context->getCurrentReadSurface())))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBlitFeedbackLoop);
        return false;
    }

    // Not allow blitting to MS buffers, therefore if renderToTextureSamples exist,
    // consider it MS. checkReadBufferResourceSamples = false
    if (!ValidateFramebufferNotMultisampled(context, entryPoint, drawFramebuffer, false))
    {
        return false;
    }

    // This validation is specified in the WebGL 2.0 spec and not in the GLES 3.0.5 spec, but we
    // always run it in order to avoid triggering driver bugs.
    if (DifferenceCanOverflow(srcX0, srcX1) || DifferenceCanOverflow(srcY0, srcY1) ||
        DifferenceCanOverflow(dstX0, dstX1) || DifferenceCanOverflow(dstY0, dstY1))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kBlitDimensionsOutOfRange);
        return false;
    }

    bool sameBounds = srcX0 == dstX0 && srcY0 == dstY0 && srcX1 == dstX1 && srcY1 == dstY1;

    if (mask & GL_COLOR_BUFFER_BIT)
    {
        const FramebufferAttachment *readColorBuffer = readFramebuffer->getReadColorAttachment();
        const Extensions &extensions                 = context->getExtensions();

        if (readColorBuffer)
        {
            const Format &readFormat = readColorBuffer->getFormat();

            for (size_t drawbufferIdx = 0;
                 drawbufferIdx < drawFramebuffer->getDrawbufferStateCount(); ++drawbufferIdx)
            {
                const FramebufferAttachment *attachment =
                    drawFramebuffer->getDrawBuffer(drawbufferIdx);
                if (attachment)
                {
                    const Format &drawFormat = attachment->getFormat();

                    // The GL ES 3.0.2 spec (pg 193) states that:
                    // 1) If the read buffer is fixed point format, the draw buffer must be as well
                    // 2) If the read buffer is an unsigned integer format, the draw buffer must be
                    // as well
                    // 3) If the read buffer is a signed integer format, the draw buffer must be as
                    // well
                    // Changes with EXT_color_buffer_float:
                    // Case 1) is changed to fixed point OR floating point
                    GLenum readComponentType = readFormat.info->componentType;
                    GLenum drawComponentType = drawFormat.info->componentType;
                    bool readFixedPoint      = (readComponentType == GL_UNSIGNED_NORMALIZED ||
                                           readComponentType == GL_SIGNED_NORMALIZED);
                    bool drawFixedPoint      = (drawComponentType == GL_UNSIGNED_NORMALIZED ||
                                           drawComponentType == GL_SIGNED_NORMALIZED);

                    if (extensions.colorBufferFloatEXT)
                    {
                        bool readFixedOrFloat = (readFixedPoint || readComponentType == GL_FLOAT);
                        bool drawFixedOrFloat = (drawFixedPoint || drawComponentType == GL_FLOAT);

                        if (readFixedOrFloat != drawFixedOrFloat)
                        {
                            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                                   kBlitTypeMismatchFixedOrFloat);
                            return false;
                        }
                    }
                    else if (readFixedPoint != drawFixedPoint)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBlitTypeMismatchFixedPoint);
                        return false;
                    }

                    if (readComponentType == GL_UNSIGNED_INT &&
                        drawComponentType != GL_UNSIGNED_INT)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                               kBlitTypeMismatchUnsignedInteger);
                        return false;
                    }

                    if (readComponentType == GL_INT && drawComponentType != GL_INT)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                               kBlitTypeMismatchSignedInteger);
                        return false;
                    }

                    if (readColorBuffer->getResourceSamples() > 0 &&
                        (!Format::EquivalentForBlit(readFormat, drawFormat) || !sameBounds))
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                               kBlitMultisampledFormatOrBoundsMismatch);
                        return false;
                    }

                    if (context->isWebGL() && *readColorBuffer == *attachment)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBlitSameImageColor);
                        return false;
                    }
                }
            }

            if (readFormat.info->isInt() && filter == GL_LINEAR)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBlitIntegerWithLinearFilter);
                return false;
            }
        }
        // In OpenGL ES, blits to/from missing attachments are silently ignored.  In WebGL 2.0, this
        // is defined to be an error.
        else if (context->isWebGL() && drawFramebuffer->hasEnabledDrawBuffer())
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBlitMissingColor);
            return false;
        }
    }

    GLenum masks[]       = {GL_DEPTH_BUFFER_BIT, GL_STENCIL_BUFFER_BIT};
    GLenum attachments[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    for (size_t i = 0; i < 2; i++)
    {
        if (mask & masks[i])
        {
            const FramebufferAttachment *readBuffer =
                readFramebuffer->getAttachment(context, attachments[i]);
            const FramebufferAttachment *drawBuffer =
                drawFramebuffer->getAttachment(context, attachments[i]);

            if (readBuffer && drawBuffer)
            {
                if (!Format::EquivalentForBlit(readBuffer->getFormat(), drawBuffer->getFormat()))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBlitDepthOrStencilFormatMismatch);
                    return false;
                }

                if (readBuffer->getResourceSamples() > 0 && !sameBounds)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBlitMultisampledBoundsMismatch);
                    return false;
                }

                if (context->isWebGL() && *readBuffer == *drawBuffer)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBlitSameImageDepthOrStencil);
                    return false;
                }
            }
            // WebGL 2.0 BlitFramebuffer when blitting from a missing attachment
            else if (context->isWebGL() && drawBuffer)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBlitMissingDepthOrStencil);
                return false;
            }
        }
    }

    // OVR_multiview2:
    // Calling BlitFramebuffer will result in an INVALID_FRAMEBUFFER_OPERATION error if the
    // current draw framebuffer isMultiview() or the number of
    // views in the current read framebuffer is more than one.
    if (readFramebuffer->readDisallowedByMultiview())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION, kBlitFromMultiview);
        return false;
    }
    if (drawFramebuffer->isMultiview())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION, kBlitToMultiview);
        return false;
    }

    return true;
}

bool ValidateBindFramebufferBase(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 GLenum target,
                                 FramebufferID framebuffer)
{
    if (!ValidFramebufferTarget(context, target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFramebufferTarget);
        return false;
    }

    if (!context->getState().isBindGeneratesResourceEnabled() &&
        !context->isFramebufferGenerated(framebuffer))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kObjectNotGenerated);
        return false;
    }

    return true;
}

bool ValidateBindRenderbufferBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  GLenum target,
                                  RenderbufferID renderbuffer)
{
    if (target != GL_RENDERBUFFER)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidRenderbufferTarget);
        return false;
    }

    if (!context->getState().isBindGeneratesResourceEnabled() &&
        !context->isRenderbufferGenerated(renderbuffer))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kObjectNotGenerated);
        return false;
    }

    return true;
}

bool ValidateFramebufferParameteriBase(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       GLenum target,
                                       GLenum pname,
                                       GLint param)
{
    if (!ValidFramebufferTarget(context, target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFramebufferTarget);
        return false;
    }

    switch (pname)
    {
        case GL_FRAMEBUFFER_DEFAULT_WIDTH:
        {
            GLint maxWidth = context->getCaps().maxFramebufferWidth;
            if (param < 0 || param > maxWidth)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsFramebufferWidth);
                return false;
            }
            break;
        }
        case GL_FRAMEBUFFER_DEFAULT_HEIGHT:
        {
            GLint maxHeight = context->getCaps().maxFramebufferHeight;
            if (param < 0 || param > maxHeight)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsFramebufferHeight);
                return false;
            }
            break;
        }
        case GL_FRAMEBUFFER_DEFAULT_SAMPLES:
        {
            GLint maxSamples = context->getCaps().maxFramebufferSamples;
            if (param < 0 || param > maxSamples)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsFramebufferSamples);
                return false;
            }
            break;
        }
        case GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS:
        {
            break;
        }
        case GL_FRAMEBUFFER_DEFAULT_LAYERS_EXT:
        {
            if (!context->getExtensions().geometryShaderAny() &&
                context->getClientVersion() < ES_3_2)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kGeometryShaderExtensionNotEnabled);
                return false;
            }
            GLint maxLayers = context->getCaps().maxFramebufferLayers;
            if (param < 0 || param > maxLayers)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidFramebufferLayer);
                return false;
            }
            break;
        }
        case GL_FRAMEBUFFER_FLIP_Y_MESA:
        {
            if (!context->getExtensions().framebufferFlipYMESA)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidPname);
                return false;
            }
            break;
        }
        default:
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidPname);
            return false;
        }
    }

    const Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);
    ASSERT(framebuffer);
    if (framebuffer->isDefault())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kDefaultFramebuffer);
        return false;
    }
    return true;
}

bool ValidateFramebufferRenderbufferBase(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         GLenum target,
                                         GLenum attachment,
                                         GLenum renderbuffertarget,
                                         RenderbufferID renderbuffer)
{
    if (!ValidFramebufferTarget(context, target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFramebufferTarget);
        return false;
    }

    if (renderbuffertarget != GL_RENDERBUFFER)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidRenderbufferTarget);
        return false;
    }

    Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);

    ASSERT(framebuffer);
    if (framebuffer->isDefault())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kDefaultFramebufferTarget);
        return false;
    }

    if (!ValidateAttachmentTarget(context, entryPoint, attachment))
    {
        return false;
    }

    // [OpenGL ES 2.0.25] Section 4.4.3 page 112
    // [OpenGL ES 3.0.2] Section 4.4.2 page 201
    // 'renderbuffer' must be either zero or the name of an existing renderbuffer object of
    // type 'renderbuffertarget', otherwise an INVALID_OPERATION error is generated.
    if (renderbuffer.value != 0)
    {
        if (!context->getRenderbuffer(renderbuffer))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidRenderbufferTarget);
            return false;
        }
    }

    return true;
}

bool ValidateFramebufferTextureBase(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    GLenum target,
                                    GLenum attachment,
                                    TextureID texture,
                                    GLint level)
{
    if (!ValidFramebufferTarget(context, target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFramebufferTarget);
        return false;
    }

    if (!ValidateAttachmentTarget(context, entryPoint, attachment))
    {
        return false;
    }

    if (texture.value != 0)
    {
        Texture *tex = context->getTexture(texture);

        if (tex == nullptr)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMissingTexture);
            return false;
        }

        if (level < 0)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevel);
            return false;
        }

        // GLES spec 3.1, Section 9.2.8 "Attaching Texture Images to a Framebuffer"
        // An INVALID_VALUE error is generated if texture is not zero and level is
        // not a supported texture level for textarget

        // Common criteria for not supported texture levels(other criteria are handled case by case
        // in non base functions): If texture refers to an immutable-format texture, level must be
        // greater than or equal to zero and smaller than the value of TEXTURE_IMMUTABLE_LEVELS for
        // texture.
        if (tex->getImmutableFormat() && context->getClientVersion() >= ES_3_1)
        {
            if (level >= static_cast<GLint>(tex->getImmutableLevels()))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevel);
                return false;
            }
        }

        // GLES spec 3.2, Section 9.2.8 "Attaching Texture Images to a Framebuffer"
        // An INVALID_OPERATION error is generated if <texture> is the name of a buffer texture.
        if ((context->getClientVersion() >= ES_3_2 ||
             context->getExtensions().textureBufferAny()) &&
            tex->getType() == TextureType::Buffer)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidTextureTarget);
            return false;
        }

        if (tex->getState().hasProtectedContent() != context->getState().hasProtectedContent())
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                   "Mismatch between Texture and Context Protected Content state");
            return false;
        }
    }

    const Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);
    ASSERT(framebuffer);

    if (framebuffer->isDefault())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kDefaultFramebufferTarget);
        return false;
    }

    return true;
}

bool ValidateGenerateMipmapBase(const Context *context,
                                angle::EntryPoint entryPoint,
                                TextureType target)
{
    if (!ValidTextureTarget(context, target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
        return false;
    }

    Texture *texture = context->getTextureByType(target);

    if (texture == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTextureNotBound);
        return false;
    }

    const GLuint effectiveBaseLevel = texture->getTextureState().getEffectiveBaseLevel();

    // This error isn't spelled out in the spec in a very explicit way, but we interpret the spec so
    // that out-of-range base level has a non-color-renderable / non-texture-filterable format.
    if (effectiveBaseLevel >= IMPLEMENTATION_MAX_TEXTURE_LEVELS)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBaseLevelOutOfRange);
        return false;
    }

    TextureTarget baseTarget = (target == TextureType::CubeMap)
                                   ? TextureTarget::CubeMapPositiveX
                                   : NonCubeTextureTypeToTarget(target);
    const auto &format       = *(texture->getFormat(baseTarget, effectiveBaseLevel).info);
    if (format.sizedInternalFormat == GL_NONE || format.compressed || format.depthBits > 0 ||
        format.stencilBits > 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kGenerateMipmapNotAllowed);
        return false;
    }

    // GenerateMipmap accepts formats that are unsized or both color renderable and filterable.
    bool formatUnsized = !format.sized;
    bool formatColorRenderableAndFilterable =
        format.filterSupport(context->getClientVersion(), context->getExtensions()) &&
        format.textureAttachmentSupport(context->getClientVersion(), context->getExtensions());
    if (!formatUnsized && !formatColorRenderableAndFilterable)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kGenerateMipmapNotAllowed);
        return false;
    }

    // GL_EXT_sRGB adds an unsized SRGB (no alpha) format which has explicitly disabled mipmap
    // generation
    if (format.colorEncoding == GL_SRGB && format.format == GL_RGB)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kGenerateMipmapNotAllowed);
        return false;
    }

    // According to the OpenGL extension spec EXT_sRGB.txt, EXT_SRGB is based on ES 2.0 and
    // generateMipmap is not allowed if texture format is SRGB_EXT or SRGB_ALPHA_EXT.
    if (context->getClientVersion() < Version(3, 0) && format.colorEncoding == GL_SRGB)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kGenerateMipmapNotAllowed);
        return false;
    }

    // Non-power of 2 ES2 check
    if (context->getClientVersion() < Version(3, 0) && !context->getExtensions().textureNpotOES &&
        (!isPow2(static_cast<int>(texture->getWidth(baseTarget, 0))) ||
         !isPow2(static_cast<int>(texture->getHeight(baseTarget, 0)))))
    {
        ASSERT(target == TextureType::_2D || target == TextureType::Rectangle ||
               target == TextureType::CubeMap);
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTextureNotPow2);
        return false;
    }

    // Cube completeness check
    if (target == TextureType::CubeMap && !texture->getTextureState().isCubeComplete())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kCubemapIncomplete);
        return false;
    }

    if (context->isWebGL() && (texture->getWidth(baseTarget, effectiveBaseLevel) == 0 ||
                               texture->getHeight(baseTarget, effectiveBaseLevel) == 0))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kGenerateMipmapZeroSize);
        return false;
    }

    return true;
}

bool ValidateReadPixelsRobustANGLE(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   GLint x,
                                   GLint y,
                                   GLsizei width,
                                   GLsizei height,
                                   GLenum format,
                                   GLenum type,
                                   GLsizei bufSize,
                                   const GLsizei *length,
                                   const GLsizei *columns,
                                   const GLsizei *rows,
                                   const void *pixels)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei writeLength  = 0;
    GLsizei writeColumns = 0;
    GLsizei writeRows    = 0;

    if (!ValidateReadPixelsBase(context, entryPoint, x, y, width, height, format, type, bufSize,
                                &writeLength, &writeColumns, &writeRows, pixels))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, writeLength))
    {
        return false;
    }

    SetRobustLengthParam(length, writeLength);
    SetRobustLengthParam(columns, writeColumns);
    SetRobustLengthParam(rows, writeRows);

    return true;
}

bool ValidateReadnPixelsEXT(const Context *context,
                            angle::EntryPoint entryPoint,
                            GLint x,
                            GLint y,
                            GLsizei width,
                            GLsizei height,
                            GLenum format,
                            GLenum type,
                            GLsizei bufSize,
                            const void *pixels)
{
    if (bufSize < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeBufSize);
        return false;
    }

    return ValidateReadPixelsBase(context, entryPoint, x, y, width, height, format, type, bufSize,
                                  nullptr, nullptr, nullptr, pixels);
}

bool ValidateReadnPixelsRobustANGLE(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    GLint x,
                                    GLint y,
                                    GLsizei width,
                                    GLsizei height,
                                    GLenum format,
                                    GLenum type,
                                    GLsizei bufSize,
                                    const GLsizei *length,
                                    const GLsizei *columns,
                                    const GLsizei *rows,
                                    const void *data)
{
    GLsizei writeLength  = 0;
    GLsizei writeColumns = 0;
    GLsizei writeRows    = 0;

    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    if (!ValidateReadPixelsBase(context, entryPoint, x, y, width, height, format, type, bufSize,
                                &writeLength, &writeColumns, &writeRows, data))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, writeLength))
    {
        return false;
    }

    SetRobustLengthParam(length, writeLength);
    SetRobustLengthParam(columns, writeColumns);
    SetRobustLengthParam(rows, writeRows);

    return true;
}

bool ValidateGenQueriesEXT(const Context *context,
                           angle::EntryPoint entryPoint,
                           GLsizei n,
                           const QueryID *ids)
{
    if (!context->getExtensions().occlusionQueryBooleanEXT &&
        !context->getExtensions().disjointTimerQueryEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kQueryExtensionNotEnabled);
        return false;
    }

    return ValidateGenOrDelete(context, entryPoint, n);
}

bool ValidateDeleteQueriesEXT(const Context *context,
                              angle::EntryPoint entryPoint,
                              GLsizei n,
                              const QueryID *ids)
{
    if (!context->getExtensions().occlusionQueryBooleanEXT &&
        !context->getExtensions().disjointTimerQueryEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kQueryExtensionNotEnabled);
        return false;
    }

    return ValidateGenOrDelete(context, entryPoint, n);
}

bool ValidateIsQueryEXT(const Context *context, angle::EntryPoint entryPoint, QueryID id)
{
    if (!context->getExtensions().occlusionQueryBooleanEXT &&
        !context->getExtensions().disjointTimerQueryEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kQueryExtensionNotEnabled);
        return false;
    }

    return true;
}

bool ValidateBeginQueryBase(const Context *context,
                            angle::EntryPoint entryPoint,
                            QueryType target,
                            QueryID id)
{
    if (!ValidQueryType(context, target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidQueryType);
        return false;
    }

    if (id.value == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidQueryId);
        return false;
    }

    // From EXT_occlusion_query_boolean: If BeginQueryEXT is called with an <id>
    // of zero, if the active query object name for <target> is non-zero (for the
    // targets ANY_SAMPLES_PASSED_EXT and ANY_SAMPLES_PASSED_CONSERVATIVE_EXT, if
    // the active query for either target is non-zero), if <id> is the name of an
    // existing query object whose type does not match <target>, or if <id> is the
    // active query object name for any query type, the error INVALID_OPERATION is
    // generated.

    // Ensure no other queries are active
    // NOTE: If other queries than occlusion are supported, we will need to check
    // separately that:
    //    a) The query ID passed is not the current active query for any target/type
    //    b) There are no active queries for the requested target (and in the case
    //       of GL_ANY_SAMPLES_PASSED_EXT and GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT,
    //       no query may be active for either if glBeginQuery targets either.

    if (context->getState().isQueryActive(target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kOtherQueryActive);
        return false;
    }

    // check that name was obtained with glGenQueries
    if (!context->isQueryGenerated(id))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidQueryId);
        return false;
    }

    // Check for type mismatch. If query is not yet started we're good to go.
    Query *queryObject = context->getQuery(id);
    if (queryObject && queryObject->getType() != target)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kQueryTargetMismatch);
        return false;
    }

    return true;
}

bool ValidateBeginQueryEXT(const Context *context,
                           angle::EntryPoint entryPoint,
                           QueryType target,
                           QueryID id)
{
    if (!context->getExtensions().occlusionQueryBooleanEXT &&
        !context->getExtensions().disjointTimerQueryEXT &&
        !context->getExtensions().syncQueryCHROMIUM)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kQueryExtensionNotEnabled);
        return false;
    }

    return ValidateBeginQueryBase(context, entryPoint, target, id);
}

bool ValidateEndQueryBase(const Context *context, angle::EntryPoint entryPoint, QueryType target)
{
    if (!ValidQueryType(context, target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidQueryType);
        return false;
    }

    const Query *queryObject = context->getState().getActiveQuery(target);

    if (queryObject == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kQueryInactive);
        return false;
    }

    return true;
}

bool ValidateEndQueryEXT(const Context *context, angle::EntryPoint entryPoint, QueryType target)
{
    if (!context->getExtensions().occlusionQueryBooleanEXT &&
        !context->getExtensions().disjointTimerQueryEXT &&
        !context->getExtensions().syncQueryCHROMIUM)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kQueryExtensionNotEnabled);
        return false;
    }

    return ValidateEndQueryBase(context, entryPoint, target);
}

bool ValidateQueryCounterEXT(const Context *context,
                             angle::EntryPoint entryPoint,
                             QueryID id,
                             QueryType target)
{
    if (!context->getExtensions().disjointTimerQueryEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (target != QueryType::Timestamp)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidQueryTarget);
        return false;
    }

    if (!context->isQueryGenerated(id))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidQueryId);
        return false;
    }

    // If query object is not started, that's fine.
    Query *queryObject = context->getQuery(id);
    if (queryObject && context->getState().isQueryActive(queryObject))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kQueryActive);
        return false;
    }

    return true;
}

bool ValidateGetQueryivBase(const Context *context,
                            angle::EntryPoint entryPoint,
                            QueryType target,
                            GLenum pname,
                            GLsizei *numParams)
{
    if (numParams)
    {
        *numParams = 0;
    }

    if (!ValidQueryType(context, target) && target != QueryType::Timestamp)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidQueryType);
        return false;
    }

    switch (pname)
    {
        case GL_CURRENT_QUERY_EXT:
            if (target == QueryType::Timestamp)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidQueryTarget);
                return false;
            }
            break;
        case GL_QUERY_COUNTER_BITS_EXT:
            if (!context->getExtensions().disjointTimerQueryEXT ||
                (target != QueryType::Timestamp && target != QueryType::TimeElapsed))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidPname);
                return false;
            }
            break;
        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidPname);
            return false;
    }

    if (numParams)
    {
        // All queries return only one value
        *numParams = 1;
    }

    return true;
}

bool ValidateGetQueryivEXT(const Context *context,
                           angle::EntryPoint entryPoint,
                           QueryType target,
                           GLenum pname,
                           const GLint *params)
{
    if (!context->getExtensions().occlusionQueryBooleanEXT &&
        !context->getExtensions().disjointTimerQueryEXT &&
        !context->getExtensions().syncQueryCHROMIUM)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateGetQueryivBase(context, entryPoint, target, pname, nullptr);
}

bool ValidateGetQueryivRobustANGLE(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   QueryType target,
                                   GLenum pname,
                                   GLsizei bufSize,
                                   const GLsizei *length,
                                   const GLint *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei numParams = 0;

    if (!ValidateGetQueryivBase(context, entryPoint, target, pname, &numParams))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, numParams))
    {
        return false;
    }

    SetRobustLengthParam(length, numParams);

    return true;
}

bool ValidateGetQueryObjectValueBase(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     QueryID id,
                                     GLenum pname,
                                     GLsizei *numParams)
{
    if (numParams)
    {
        *numParams = 1;
    }

    if (context->isContextLost())
    {
        ANGLE_VALIDATION_ERROR(GL_CONTEXT_LOST, kContextLost);

        if (pname == GL_QUERY_RESULT_AVAILABLE_EXT)
        {
            // Generate an error but still return true, the context still needs to return a
            // value in this case.
            return true;
        }
        else
        {
            return false;
        }
    }

    Query *queryObject = context->getQuery(id);

    if (!queryObject)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidQueryId);
        return false;
    }

    if (context->getState().isQueryActive(queryObject))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kQueryActive);
        return false;
    }

    switch (pname)
    {
        case GL_QUERY_RESULT_EXT:
        case GL_QUERY_RESULT_AVAILABLE_EXT:
            break;

        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
            return false;
    }

    return true;
}

bool ValidateGetQueryObjectivEXT(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 QueryID id,
                                 GLenum pname,
                                 const GLint *params)
{
    if (!context->getExtensions().disjointTimerQueryEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }
    return ValidateGetQueryObjectValueBase(context, entryPoint, id, pname, nullptr);
}

bool ValidateGetQueryObjectivRobustANGLE(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         QueryID id,
                                         GLenum pname,
                                         GLsizei bufSize,
                                         const GLsizei *length,
                                         const GLint *params)
{
    if (!context->getExtensions().disjointTimerQueryEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei numParams = 0;

    if (!ValidateGetQueryObjectValueBase(context, entryPoint, id, pname, &numParams))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, numParams))
    {
        return false;
    }

    SetRobustLengthParam(length, numParams);

    return true;
}

bool ValidateGetQueryObjectuivEXT(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  QueryID id,
                                  GLenum pname,
                                  const GLuint *params)
{
    if (!context->getExtensions().disjointTimerQueryEXT &&
        !context->getExtensions().occlusionQueryBooleanEXT &&
        !context->getExtensions().syncQueryCHROMIUM)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }
    return ValidateGetQueryObjectValueBase(context, entryPoint, id, pname, nullptr);
}

bool ValidateGetQueryObjectuivRobustANGLE(const Context *context,
                                          angle::EntryPoint entryPoint,
                                          QueryID id,
                                          GLenum pname,
                                          GLsizei bufSize,
                                          const GLsizei *length,
                                          const GLuint *params)
{
    if (!context->getExtensions().disjointTimerQueryEXT &&
        !context->getExtensions().occlusionQueryBooleanEXT &&
        !context->getExtensions().syncQueryCHROMIUM)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei numParams = 0;

    if (!ValidateGetQueryObjectValueBase(context, entryPoint, id, pname, &numParams))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, numParams))
    {
        return false;
    }

    SetRobustLengthParam(length, numParams);

    return true;
}

bool ValidateGetQueryObjecti64vEXT(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   QueryID id,
                                   GLenum pname,
                                   GLint64 *params)
{
    if (!context->getExtensions().disjointTimerQueryEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }
    return ValidateGetQueryObjectValueBase(context, entryPoint, id, pname, nullptr);
}

bool ValidateGetQueryObjecti64vRobustANGLE(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           QueryID id,
                                           GLenum pname,
                                           GLsizei bufSize,
                                           const GLsizei *length,
                                           GLint64 *params)
{
    if (!context->getExtensions().disjointTimerQueryEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei numParams = 0;

    if (!ValidateGetQueryObjectValueBase(context, entryPoint, id, pname, &numParams))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, numParams))
    {
        return false;
    }

    SetRobustLengthParam(length, numParams);

    return true;
}

bool ValidateGetQueryObjectui64vEXT(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    QueryID id,
                                    GLenum pname,
                                    GLuint64 *params)
{
    if (!context->getExtensions().disjointTimerQueryEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }
    return ValidateGetQueryObjectValueBase(context, entryPoint, id, pname, nullptr);
}

bool ValidateGetQueryObjectui64vRobustANGLE(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            QueryID id,
                                            GLenum pname,
                                            GLsizei bufSize,
                                            const GLsizei *length,
                                            GLuint64 *params)
{
    if (!context->getExtensions().disjointTimerQueryEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei numParams = 0;

    if (!ValidateGetQueryObjectValueBase(context, entryPoint, id, pname, &numParams))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, numParams))
    {
        return false;
    }

    SetRobustLengthParam(length, numParams);

    return true;
}

bool ValidateUniform1ivValue(const Context *context,
                             angle::EntryPoint entryPoint,
                             GLenum uniformType,
                             GLsizei count,
                             const GLint *value)
{
    // Value type is GL_INT, because we only get here from glUniform1i{v}.
    // It is compatible with INT or BOOL.
    // Do these cheap tests first, for a little extra speed.
    if (GL_INT == uniformType || GL_BOOL == uniformType)
    {
        return true;
    }

    if (IsSamplerType(uniformType))
    {
        // Check that the values are in range.
        const GLint max = context->getCaps().maxCombinedTextureImageUnits;
        for (GLsizei i = 0; i < count; ++i)
        {
            if (value[i] < 0 || value[i] >= max)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kSamplerUniformValueOutOfRange);
                return false;
            }
        }
        return true;
    }

    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kUniformTypeMismatch);
    return false;
}

bool ValidateUniformMatrixValue(const Context *context,
                                angle::EntryPoint entryPoint,
                                GLenum valueType,
                                GLenum uniformType)
{
    // Check that the value type is compatible with uniform type.
    if (valueType == uniformType)
    {
        return true;
    }

    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kUniformTypeMismatch);
    return false;
}

bool ValidateUniform(const Context *context,
                     angle::EntryPoint entryPoint,
                     GLenum valueType,
                     UniformLocation location,
                     GLsizei count)
{
    const LinkedUniform *uniform = nullptr;
    Program *programObject       = context->getActiveLinkedProgram();
    return ValidateUniformCommonBase(context, entryPoint, programObject, location, count,
                                     &uniform) &&
           ValidateUniformValue(context, entryPoint, valueType, uniform->getType());
}

bool ValidateUniform1iv(const Context *context,
                        angle::EntryPoint entryPoint,
                        UniformLocation location,
                        GLsizei count,
                        const GLint *value)
{
    const LinkedUniform *uniform = nullptr;
    Program *programObject       = context->getActiveLinkedProgram();
    return ValidateUniformCommonBase(context, entryPoint, programObject, location, count,
                                     &uniform) &&
           ValidateUniform1ivValue(context, entryPoint, uniform->getType(), count, value);
}

bool ValidateUniformMatrix(const Context *context,
                           angle::EntryPoint entryPoint,
                           GLenum valueType,
                           UniformLocation location,
                           GLsizei count,
                           GLboolean transpose)
{
    if (ConvertToBool(transpose) && context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kES3Required);
        return false;
    }

    const LinkedUniform *uniform = nullptr;
    Program *programObject       = context->getActiveLinkedProgram();
    return ValidateUniformCommonBase(context, entryPoint, programObject, location, count,
                                     &uniform) &&
           ValidateUniformMatrixValue(context, entryPoint, valueType, uniform->getType());
}

bool ValidateStateQuery(const Context *context,
                        angle::EntryPoint entryPoint,
                        GLenum pname,
                        GLenum *nativeType,
                        unsigned int *numParams)
{
    if (!context->getQueryParameterInfo(pname, nativeType, numParams))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidPname);
        return false;
    }

    const Caps &caps = context->getCaps();

    if (pname >= GL_DRAW_BUFFER0 && pname <= GL_DRAW_BUFFER15)
    {
        int colorAttachment = (pname - GL_DRAW_BUFFER0);

        if (colorAttachment >= caps.maxDrawBuffers)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kIndexExceedsMaxDrawBuffer);
            return false;
        }
    }

    switch (pname)
    {
        case GL_TEXTURE_BINDING_2D:
        case GL_TEXTURE_BINDING_CUBE_MAP:
        case GL_TEXTURE_BINDING_3D:
        case GL_TEXTURE_BINDING_2D_ARRAY:
        case GL_TEXTURE_BINDING_2D_MULTISAMPLE:
        case GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY:
            break;

        case GL_TEXTURE_BINDING_RECTANGLE_ANGLE:
            if (!context->getExtensions().textureRectangleANGLE)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        case GL_TEXTURE_BINDING_EXTERNAL_OES:
            if (!context->getExtensions().EGLStreamConsumerExternalNV &&
                !context->getExtensions().EGLImageExternalOES)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        case GL_TEXTURE_BUFFER_BINDING:
        case GL_TEXTURE_BINDING_BUFFER:
        case GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT:
        case GL_MAX_TEXTURE_BUFFER_SIZE:
            if (context->getClientVersion() < Version(3, 2) &&
                !context->getExtensions().textureBufferAny())
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kTextureBufferExtensionNotAvailable);
                return false;
            }
            break;

        case GL_IMPLEMENTATION_COLOR_READ_TYPE:
        case GL_IMPLEMENTATION_COLOR_READ_FORMAT:
        {
            Framebuffer *readFramebuffer = context->getState().getReadFramebuffer();
            ASSERT(readFramebuffer);

            if (!ValidateFramebufferComplete<GL_INVALID_OPERATION>(context, entryPoint,
                                                                   readFramebuffer))
            {
                return false;
            }

            if (readFramebuffer->getReadBufferState() == GL_NONE)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kReadBufferNone);
                return false;
            }

            const FramebufferAttachment *attachment = readFramebuffer->getReadColorAttachment();
            if (!attachment)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kReadBufferNotAttached);
                return false;
            }
            break;
        }

        case GL_PRIMITIVE_BOUNDING_BOX:
            if (!context->getExtensions().primitiveBoundingBoxAny())
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kExtensionNotEnabled);
                return false;
            }
            break;

        case GL_SHADING_RATE_QCOM:
            if (!context->getExtensions().shadingRateQCOM)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kExtensionNotEnabled);
                return false;
            }
            break;

        case GL_MULTISAMPLE_LINE_WIDTH_RANGE:
            if (context->getClientVersion() < Version(3, 2))
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        case GL_MULTISAMPLE_LINE_WIDTH_GRANULARITY:
            if (context->getClientVersion() < Version(3, 2))
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        case GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED:
            if (context->getClientVersion() < Version(3, 2) &&
                !context->getExtensions().tessellationShaderAny())
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        default:
            break;
    }

    // pname is valid, but there are no parameters to return
    if (*numParams == 0)
    {
        return false;
    }

    return true;
}

bool ValidateGetBooleanvRobustANGLE(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    GLenum pname,
                                    GLsizei bufSize,
                                    const GLsizei *length,
                                    const GLboolean *params)
{
    GLenum nativeType;
    unsigned int numParams = 0;

    if (!ValidateRobustStateQuery(context, entryPoint, pname, bufSize, &nativeType, &numParams))
    {
        return false;
    }

    SetRobustLengthParam(length, numParams);

    return true;
}

bool ValidateGetFloatvRobustANGLE(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  GLenum pname,
                                  GLsizei bufSize,
                                  const GLsizei *length,
                                  const GLfloat *params)
{
    GLenum nativeType;
    unsigned int numParams = 0;

    if (!ValidateRobustStateQuery(context, entryPoint, pname, bufSize, &nativeType, &numParams))
    {
        return false;
    }

    SetRobustLengthParam(length, numParams);

    return true;
}

bool ValidateGetIntegervRobustANGLE(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    GLenum pname,
                                    GLsizei bufSize,
                                    const GLsizei *length,
                                    const GLint *data)
{
    GLenum nativeType;
    unsigned int numParams = 0;

    if (!ValidateRobustStateQuery(context, entryPoint, pname, bufSize, &nativeType, &numParams))
    {
        return false;
    }

    SetRobustLengthParam(length, numParams);

    return true;
}

bool ValidateGetInteger64vRobustANGLE(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      GLenum pname,
                                      GLsizei bufSize,
                                      const GLsizei *length,
                                      GLint64 *data)
{
    GLenum nativeType;
    unsigned int numParams = 0;

    if (!ValidateRobustStateQuery(context, entryPoint, pname, bufSize, &nativeType, &numParams))
    {
        return false;
    }

    if (nativeType == GL_INT_64_ANGLEX)
    {
        CastStateValues(context, nativeType, pname, numParams, data);
        return false;
    }

    SetRobustLengthParam(length, numParams);
    return true;
}

bool ValidateRobustStateQuery(const Context *context,
                              angle::EntryPoint entryPoint,
                              GLenum pname,
                              GLsizei bufSize,
                              GLenum *nativeType,
                              unsigned int *numParams)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    if (!ValidateStateQuery(context, entryPoint, pname, nativeType, numParams))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, *numParams))
    {
        return false;
    }

    return true;
}

bool ValidateCopyImageSubDataTarget(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    GLuint name,
                                    GLenum target)
{
    // From EXT_copy_image: INVALID_ENUM is generated if either <srcTarget> or <dstTarget> is not
    // RENDERBUFFER or a valid non - proxy texture target, is TEXTURE_BUFFER, or is one of the
    // cubemap face selectors described in table 3.17, or if the target does not match the type of
    // the object. INVALID_VALUE is generated if either <srcName> or <dstName> does not correspond
    // to a valid renderbuffer or texture object according to the corresponding target parameter.
    switch (target)
    {
        case GL_RENDERBUFFER:
        {
            RenderbufferID renderbuffer = PackParam<RenderbufferID>(name);
            if (!context->isRenderbuffer(renderbuffer))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidRenderbufferName);
                return false;
            }
            break;
        }
        case GL_TEXTURE_2D:
        case GL_TEXTURE_3D:
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_CUBE_MAP:
        case GL_TEXTURE_CUBE_MAP_ARRAY_EXT:
        case GL_TEXTURE_EXTERNAL_OES:
        case GL_TEXTURE_2D_MULTISAMPLE:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES:
        {
            TextureID texture = PackParam<TextureID>(name);
            if (!context->isTexture(texture))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidTextureName);
                return false;
            }

            Texture *textureObject = context->getTexture(texture);
            if (textureObject && textureObject->getType() != PackParam<TextureType>(target))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, err::kTextureTypeMismatch);
                return false;
            }
            break;
        }
        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTarget);
            return false;
    }

    return true;
}

bool ValidateCopyImageSubDataLevel(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   GLenum target,
                                   GLint level)
{
    switch (target)
    {
        case GL_RENDERBUFFER:
        {
            if (level != 0)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevel);
                return false;
            }
            break;
        }
        case GL_TEXTURE_2D:
        case GL_TEXTURE_3D:
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_CUBE_MAP:
        case GL_TEXTURE_CUBE_MAP_ARRAY_EXT:
        case GL_TEXTURE_EXTERNAL_OES:
        case GL_TEXTURE_2D_MULTISAMPLE:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES:
        {
            if (!ValidMipLevel(context, PackParam<TextureType>(target), level))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevel);
                return false;
            }
            break;
        }
        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTarget);
            return false;
    }

    return true;
}

bool ValidateCopyImageSubDataTargetRegion(const Context *context,
                                          angle::EntryPoint entryPoint,
                                          GLuint name,
                                          GLenum target,
                                          GLint level,
                                          GLint offsetX,
                                          GLint offsetY,
                                          GLint offsetZ,
                                          GLsizei width,
                                          GLsizei height,
                                          GLsizei *samples)
{
    // INVALID_VALUE is generated if the dimensions of the either subregion exceeds the boundaries
    // of the corresponding image object.
    if (offsetX < 0 || offsetY < 0 || offsetZ < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeOffset);
        return false;
    }

    if (target == GL_RENDERBUFFER)
    {
        // INVALID_VALUE is generated if the dimensions of the either subregion exceeds the
        // boundaries of the corresponding image object
        Renderbuffer *buffer = context->getRenderbuffer(PackParam<RenderbufferID>(name));
        if ((buffer->getWidth() - offsetX < width) || (buffer->getHeight() - offsetY < height))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kSourceTextureTooSmall);
            return false;
        }
    }
    else
    {
        Texture *texture = context->getTexture(PackParam<TextureID>(name));

        // INVALID_OPERATION is generated if either object is a texture and the texture is not
        // complete
        // This will handle the texture completeness check. Note that this ignores format-based
        // compleness rules.
        if (!texture->isSamplerCompleteForCopyImage(context, nullptr))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNotTextureComplete);
            return false;
        }

        GLenum textureTargetToUse = target;
        if (target == GL_TEXTURE_CUBE_MAP)
        {
            // Use GL_TEXTURE_CUBE_MAP_POSITIVE_X to properly gather the textureWidth/textureHeight
            textureTargetToUse = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        }

        const GLsizei textureWidth = static_cast<GLsizei>(
            texture->getWidth(PackParam<TextureTarget>(textureTargetToUse), level));
        const GLsizei textureHeight = static_cast<GLsizei>(
            texture->getHeight(PackParam<TextureTarget>(textureTargetToUse), level));

        // INVALID_VALUE is generated if the dimensions of the either subregion exceeds the
        // boundaries of the corresponding image object
        if ((textureWidth - offsetX < width) || (textureHeight - offsetY < height))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kSourceTextureTooSmall);
            return false;
        }

        *samples = texture->getSamples(PackParam<TextureTarget>(textureTargetToUse), level);
        *samples = (*samples == 0) ? 1 : *samples;
    }

    return true;
}

bool ValidateCompressedRegion(const Context *context,
                              angle::EntryPoint entryPoint,
                              const InternalFormat &formatInfo,
                              GLsizei width,
                              GLsizei height)
{
    ASSERT(formatInfo.compressed);

    // INVALID_VALUE is generated if the image format is compressed and the dimensions of the
    // subregion fail to meet the alignment constraints of the format.
    if ((width % formatInfo.compressedBlockWidth != 0) ||
        (height % formatInfo.compressedBlockHeight != 0))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidCompressedRegionSize);
        return false;
    }

    return true;
}

const InternalFormat &GetTargetFormatInfo(const Context *context,
                                          angle::EntryPoint entryPoint,
                                          GLuint name,
                                          GLenum target,
                                          GLint level)
{
    static const InternalFormat defaultInternalFormat;

    switch (target)
    {
        case GL_RENDERBUFFER:
        {
            Renderbuffer *buffer = context->getRenderbuffer(PackParam<RenderbufferID>(name));
            return *buffer->getFormat().info;
        }
        case GL_TEXTURE_2D:
        case GL_TEXTURE_3D:
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_CUBE_MAP:
        case GL_TEXTURE_CUBE_MAP_ARRAY_EXT:
        case GL_TEXTURE_EXTERNAL_OES:
        case GL_TEXTURE_2D_MULTISAMPLE:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES:
        {
            Texture *texture          = context->getTexture(PackParam<TextureID>(name));
            GLenum textureTargetToUse = target;

            if (target == GL_TEXTURE_CUBE_MAP)
            {
                // Use GL_TEXTURE_CUBE_MAP_POSITIVE_X to properly gather the
                // textureWidth/textureHeight
                textureTargetToUse = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
            }
            return *texture->getFormat(PackParam<TextureTarget>(textureTargetToUse), level).info;
        }
        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTarget);
            return defaultInternalFormat;
    }
}

bool ValidateCopyMixedFormatCompatible(GLenum uncompressedFormat, GLenum compressedFormat)
{
    // Validates mixed format compatibility (uncompressed and compressed) from Table 4.X.1 of the
    // EXT_copy_image spec.
    switch (compressedFormat)
    {
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
        case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:
        case GL_COMPRESSED_RGBA_BPTC_UNORM_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT:
        case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT:
        case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT:
        case GL_COMPRESSED_RGBA8_ETC2_EAC:
        case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
        case GL_COMPRESSED_RG11_EAC:
        case GL_COMPRESSED_SIGNED_RG11_EAC:
        case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
        case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:
        case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:
        case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:
        case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:
        case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:
        case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:
        case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:
        case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:
        case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:
        case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:
        case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:
        case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:
        case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:
        case GL_COMPRESSED_RGBA_ASTC_3x3x3_OES:
        case GL_COMPRESSED_RGBA_ASTC_4x3x3_OES:
        case GL_COMPRESSED_RGBA_ASTC_4x4x3_OES:
        case GL_COMPRESSED_RGBA_ASTC_4x4x4_OES:
        case GL_COMPRESSED_RGBA_ASTC_5x4x4_OES:
        case GL_COMPRESSED_RGBA_ASTC_5x5x4_OES:
        case GL_COMPRESSED_RGBA_ASTC_5x5x5_OES:
        case GL_COMPRESSED_RGBA_ASTC_6x5x5_OES:
        case GL_COMPRESSED_RGBA_ASTC_6x6x5_OES:
        case GL_COMPRESSED_RGBA_ASTC_6x6x6_OES:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES:
        {
            switch (uncompressedFormat)
            {
                case GL_RGBA32UI:
                case GL_RGBA32I:
                case GL_RGBA32F:
                    return true;
                default:
                    return false;
            }
        }
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RED_RGTC1_EXT:
        case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:
        case GL_COMPRESSED_RGB8_ETC2:
        case GL_COMPRESSED_SRGB8_ETC2:
        case GL_COMPRESSED_R11_EAC:
        case GL_COMPRESSED_SIGNED_R11_EAC:
        case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        {
            switch (uncompressedFormat)
            {
                case GL_RGBA16UI:
                case GL_RGBA16I:
                case GL_RGBA16F:
                case GL_RG32UI:
                case GL_RG32I:
                case GL_RG32F:
                    return true;
                default:
                    return false;
            }
        }
        default:
            break;
    }

    return false;
}

bool ValidateCopyCompressedFormatCompatible(const InternalFormat &srcFormatInfo,
                                            const InternalFormat &dstFormatInfo)
{
    // Validates compressed format compatibility from Table 4.X.2 of the EXT_copy_image spec.

    ASSERT(srcFormatInfo.internalFormat != dstFormatInfo.internalFormat);

    const GLenum srcFormat = srcFormatInfo.internalFormat;
    const GLenum dstFormat = dstFormatInfo.internalFormat;

    switch (srcFormat)
    {
        case GL_COMPRESSED_RED_RGTC1_EXT:
            return (dstFormat == GL_COMPRESSED_SIGNED_RED_RGTC1_EXT);
        case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:
            return (dstFormat == GL_COMPRESSED_RED_RGTC1_EXT);
        case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
            return (dstFormat == GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT);
        case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:
            return (dstFormat == GL_COMPRESSED_RED_GREEN_RGTC2_EXT);
        case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT:
            return (dstFormat == GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT);
        case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT:
            return (dstFormat == GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT);
        case GL_COMPRESSED_R11_EAC:
            return (dstFormat == GL_COMPRESSED_SIGNED_R11_EAC);
        case GL_COMPRESSED_SIGNED_R11_EAC:
            return (dstFormat == GL_COMPRESSED_R11_EAC);
        case GL_COMPRESSED_RG11_EAC:
            return (dstFormat == GL_COMPRESSED_SIGNED_RG11_EAC);
        case GL_COMPRESSED_SIGNED_RG11_EAC:
            return (dstFormat == GL_COMPRESSED_RG11_EAC);
        default:
            break;
    }

    // Since they can't be the same format and are both compressed formats, one must be linear and
    // the other nonlinear.
    if (srcFormatInfo.colorEncoding == dstFormatInfo.colorEncoding)
    {
        return false;
    }

    const GLenum linearFormat = (srcFormatInfo.colorEncoding == GL_LINEAR) ? srcFormat : dstFormat;
    const GLenum nonLinearFormat =
        (srcFormatInfo.colorEncoding != GL_LINEAR) ? srcFormat : dstFormat;

    switch (linearFormat)
    {
        case GL_COMPRESSED_RGBA_BPTC_UNORM_EXT:
            return (nonLinearFormat == GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT);
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            return (nonLinearFormat == GL_COMPRESSED_SRGB_S3TC_DXT1_EXT);
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            return (nonLinearFormat == GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT);
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
            return (nonLinearFormat == GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT);
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            return (nonLinearFormat == GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT);
        case GL_COMPRESSED_RGB8_ETC2:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ETC2);
        case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2);
        case GL_COMPRESSED_RGBA8_ETC2_EAC:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC);
        case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR);
        case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR);
        case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR);
        case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR);
        case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR);
        case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR);
        case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR);
        case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR);
        case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR);
        case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR);
        case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR);
        case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR);
        case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR);
        case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR);
        case GL_COMPRESSED_RGBA_ASTC_3x3x3_OES:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES);
        case GL_COMPRESSED_RGBA_ASTC_4x3x3_OES:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES);
        case GL_COMPRESSED_RGBA_ASTC_4x4x3_OES:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES);
        case GL_COMPRESSED_RGBA_ASTC_4x4x4_OES:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES);
        case GL_COMPRESSED_RGBA_ASTC_5x4x4_OES:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES);
        case GL_COMPRESSED_RGBA_ASTC_5x5x4_OES:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES);
        case GL_COMPRESSED_RGBA_ASTC_5x5x5_OES:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES);
        case GL_COMPRESSED_RGBA_ASTC_6x5x5_OES:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES);
        case GL_COMPRESSED_RGBA_ASTC_6x6x5_OES:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES);
        case GL_COMPRESSED_RGBA_ASTC_6x6x6_OES:
            return (nonLinearFormat == GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES);
        default:
            break;
    }

    return false;
}

bool ValidateCopyFormatCompatible(const InternalFormat &srcFormatInfo,
                                  const InternalFormat &dstFormatInfo)
{
    // Matching source and destination formats are compatible.
    if (srcFormatInfo.internalFormat == dstFormatInfo.internalFormat)
    {
        return true;
    }

    if (srcFormatInfo.compressed != dstFormatInfo.compressed)
    {
        GLenum uncompressedFormat = (!srcFormatInfo.compressed) ? srcFormatInfo.internalFormat
                                                                : dstFormatInfo.internalFormat;
        GLenum compressedFormat   = (srcFormatInfo.compressed) ? srcFormatInfo.internalFormat
                                                               : dstFormatInfo.internalFormat;

        return ValidateCopyMixedFormatCompatible(uncompressedFormat, compressedFormat);
    }

    if (!srcFormatInfo.compressed)
    {
        // Source and destination are uncompressed formats.
        return (srcFormatInfo.pixelBytes == dstFormatInfo.pixelBytes);
    }

    return ValidateCopyCompressedFormatCompatible(srcFormatInfo, dstFormatInfo);
}

bool ValidateCopyImageSubDataBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  GLuint srcName,
                                  GLenum srcTarget,
                                  GLint srcLevel,
                                  GLint srcX,
                                  GLint srcY,
                                  GLint srcZ,
                                  GLuint dstName,
                                  GLenum dstTarget,
                                  GLint dstLevel,
                                  GLint dstX,
                                  GLint dstY,
                                  GLint dstZ,
                                  GLsizei srcWidth,
                                  GLsizei srcHeight,
                                  GLsizei srcDepth)
{
    // INVALID_VALUE is generated if the dimensions of the either subregion exceeds the boundaries
    // of the corresponding image object
    if ((srcWidth < 0) || (srcHeight < 0) || (srcDepth < 0))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeSize);
        return false;
    }

    if (!ValidateCopyImageSubDataTarget(context, entryPoint, srcName, srcTarget))
    {
        return false;
    }
    if (!ValidateCopyImageSubDataTarget(context, entryPoint, dstName, dstTarget))
    {
        return false;
    }

    if (!ValidateCopyImageSubDataLevel(context, entryPoint, srcTarget, srcLevel))
    {
        return false;
    }
    if (!ValidateCopyImageSubDataLevel(context, entryPoint, dstTarget, dstLevel))
    {
        return false;
    }

    const InternalFormat &srcFormatInfo =
        GetTargetFormatInfo(context, entryPoint, srcName, srcTarget, srcLevel);
    const InternalFormat &dstFormatInfo =
        GetTargetFormatInfo(context, entryPoint, dstName, dstTarget, dstLevel);
    GLsizei dstWidth   = srcWidth;
    GLsizei dstHeight  = srcHeight;
    GLsizei srcSamples = 1;
    GLsizei dstSamples = 1;

    if (srcFormatInfo.internalFormat == GL_NONE || dstFormatInfo.internalFormat == GL_NONE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidTextureLevel);
        return false;
    }

    // From EXT_copy_image: INVALID_OPERATION is generated if the source and destination formats
    // are not compatible, if one image is compressed and the other is uncompressed and the block
    // size of compressed image is not equal to the texel size of the compressed image.
    if (!ValidateCopyFormatCompatible(srcFormatInfo, dstFormatInfo))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kIncompatibleTextures);
        return false;
    }

    if (!ValidateCopyImageSubDataTargetRegion(context, entryPoint, srcName, srcTarget, srcLevel,
                                              srcX, srcY, srcZ, srcWidth, srcHeight, &srcSamples))
    {
        return false;
    }

    // When copying from a compressed image to an uncompressed image the image texel dimensions
    // written to the uncompressed image will be source extent divided by the compressed texel block
    // dimensions.
    if ((srcFormatInfo.compressed) && (!dstFormatInfo.compressed))
    {
        ASSERT(srcFormatInfo.compressedBlockWidth != 0);
        ASSERT(srcFormatInfo.compressedBlockHeight != 0);

        dstWidth /= srcFormatInfo.compressedBlockWidth;
        dstHeight /= srcFormatInfo.compressedBlockHeight;
    }
    // When copying from an uncompressed image to a compressed image the image texel dimensions
    // written to the compressed image will be the source extent multiplied by the compressed texel
    // block dimensions.
    else if ((!srcFormatInfo.compressed) && (dstFormatInfo.compressed))
    {
        dstWidth *= dstFormatInfo.compressedBlockWidth;
        dstHeight *= dstFormatInfo.compressedBlockHeight;
    }

    if (!ValidateCopyImageSubDataTargetRegion(context, entryPoint, dstName, dstTarget, dstLevel,
                                              dstX, dstY, dstZ, dstWidth, dstHeight, &dstSamples))
    {
        return false;
    }

    bool fillsEntireMip               = false;
    gl::Texture *dstTexture           = context->getTexture({dstName});
    gl::TextureTarget dstTargetPacked = gl::PackParam<gl::TextureTarget>(dstTarget);
    // TODO(http://anglebug.com/42264179): Some targets (e.g., GL_TEXTURE_CUBE_MAP, GL_RENDERBUFFER)
    // are unsupported when used with compressed formats due to gl::PackParam() returning
    // TextureTarget::InvalidEnum.
    if (dstTargetPacked != gl::TextureTarget::InvalidEnum)
    {
        const gl::Extents &dstExtents = dstTexture->getExtents(dstTargetPacked, dstLevel);
        fillsEntireMip = dstX == 0 && dstY == 0 && dstZ == 0 && srcWidth == dstExtents.width &&
                         srcHeight == dstExtents.height && srcDepth == dstExtents.depth;
    }

    if (srcFormatInfo.compressed && !fillsEntireMip &&
        !ValidateCompressedRegion(context, entryPoint, srcFormatInfo, srcWidth, srcHeight))
    {
        return false;
    }

    if (dstFormatInfo.compressed && !fillsEntireMip &&
        !ValidateCompressedRegion(context, entryPoint, dstFormatInfo, dstWidth, dstHeight))
    {
        return false;
    }

    // INVALID_OPERATION is generated if the source and destination number of samples do not match
    if (srcSamples != dstSamples)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kSamplesOutOfRange);
        return false;
    }

    return true;
}

bool ValidateCopyTexImageParametersBase(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        TextureTarget target,
                                        GLint level,
                                        GLenum internalformat,
                                        bool isSubImage,
                                        GLint xoffset,
                                        GLint yoffset,
                                        GLint zoffset,
                                        GLint x,
                                        GLint y,
                                        GLsizei width,
                                        GLsizei height,
                                        GLint border,
                                        Format *textureFormatOut)
{
    TextureType texType = TextureTargetToType(target);

    if (xoffset < 0 || yoffset < 0 || zoffset < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeOffset);
        return false;
    }

    if (width < 0 || height < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeSize);
        return false;
    }

    if (std::numeric_limits<GLsizei>::max() - xoffset < width ||
        std::numeric_limits<GLsizei>::max() - yoffset < height)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kOffsetOverflow);
        return false;
    }

    if (std::numeric_limits<GLint>::max() - width < x ||
        std::numeric_limits<GLint>::max() - height < y)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIntegerOverflow);
        return false;
    }

    if (border != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidBorder);
        return false;
    }

    if (!ValidMipLevel(context, texType, level))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevel);
        return false;
    }

    const State &state           = context->getState();
    Framebuffer *readFramebuffer = state.getReadFramebuffer();
    if (!ValidateFramebufferComplete(context, entryPoint, readFramebuffer))
    {
        return false;
    }

    // checkReadBufferResourceSamples = true. Treat renderToTexture textures as single sample since
    // they will be resolved before copying.
    if (!readFramebuffer->isDefault() &&
        !ValidateFramebufferNotMultisampled(context, entryPoint, readFramebuffer, true))
    {
        return false;
    }

    if (readFramebuffer->getReadBufferState() == GL_NONE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kReadBufferNone);
        return false;
    }

    // WebGL 1.0 [Section 6.26] Reading From a Missing Attachment
    // In OpenGL ES it is undefined what happens when an operation tries to read from a missing
    // attachment and WebGL defines it to be an error. We do the check unconditionally as the
    // situation is an application error that would lead to a crash in ANGLE.
    const FramebufferAttachment *source = readFramebuffer->getReadColorAttachment();
    if (source == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMissingReadAttachment);
        return false;
    }

    if (source->isYUV())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kCopyFromYUVFramebuffer);
        return false;
    }

    // OVR_multiview spec:
    // INVALID_FRAMEBUFFER_OPERATION is generated by commands that read from the
    // framebuffer such as BlitFramebuffer, ReadPixels, CopyTexImage*, and
    // CopyTexSubImage*, if the number of views in the current read framebuffer
    // is greater than 1.
    if (readFramebuffer->readDisallowedByMultiview())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION, kMultiviewReadFramebuffer);
        return false;
    }

    const Caps &caps = context->getCaps();

    GLint maxDimension = 0;
    switch (texType)
    {
        case TextureType::_2D:
            maxDimension = caps.max2DTextureSize;
            break;

        case TextureType::CubeMap:
        case TextureType::CubeMapArray:
            maxDimension = caps.maxCubeMapTextureSize;
            break;

        case TextureType::Rectangle:
            maxDimension = caps.maxRectangleTextureSize;
            break;

        case TextureType::_2DArray:
            maxDimension = caps.max2DTextureSize;
            break;

        case TextureType::_3D:
            maxDimension = caps.max3DTextureSize;
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
            return false;
    }

    Texture *texture = state.getTargetTexture(texType);
    if (!texture)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTextureNotBound);
        return false;
    }

    if (texture->getImmutableFormat() && !isSubImage)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTextureIsImmutable);
        return false;
    }

    const InternalFormat &formatInfo =
        isSubImage ? *texture->getFormat(target, level).info
                   : GetInternalFormatInfo(internalformat, GL_UNSIGNED_BYTE);

    if (formatInfo.depthBits > 0 || formatInfo.compressed)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFormat);
        return false;
    }

    if (isSubImage)
    {
        if (static_cast<size_t>(xoffset + width) > texture->getWidth(target, level) ||
            static_cast<size_t>(yoffset + height) > texture->getHeight(target, level) ||
            static_cast<size_t>(zoffset) >= texture->getDepth(target, level))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kOffsetOverflow);
            return false;
        }
    }
    else
    {
        if ((texType == TextureType::CubeMap || texType == TextureType::CubeMapArray) &&
            width != height)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kCubemapIncomplete);
            return false;
        }

        if (!formatInfo.textureSupport(context->getClientVersion(), context->getExtensions()))
        {
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, internalformat);
            return false;
        }

        int maxLevelDimension = (maxDimension >> level);
        if (static_cast<int>(width) > maxLevelDimension ||
            static_cast<int>(height) > maxLevelDimension)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kResourceMaxTextureSize);
            return false;
        }
    }

    // Do not leak the previous texture format for non-subImage case.
    if (textureFormatOut && isSubImage)
    {
        *textureFormatOut = texture->getFormat(target, level);
    }

    // Detect texture copying feedback loops for WebGL.
    if (context->isWebGL())
    {
        if (readFramebuffer->formsCopyingFeedbackLoopWith(texture->id(), level, zoffset))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kFeedbackLoop);
            return false;
        }
    }

    return true;
}

const char *ValidateProgramPipelineDrawStates(const Context *context,
                                              const Extensions &extensions,
                                              ProgramPipeline *programPipeline)
{
    for (const ShaderType shaderType : gl::AllShaderTypes())
    {
        const SharedProgramExecutable &executable =
            programPipeline->getShaderProgramExecutable(shaderType);
        if (executable)
        {
            const char *errorMsg = ValidateProgramDrawStates(context, extensions, *executable);
            if (errorMsg)
            {
                return errorMsg;
            }
        }
    }

    return nullptr;
}

const char *ValidateProgramPipelineAttachedPrograms(ProgramPipeline *programPipeline)
{
    // An INVALID_OPERATION error is generated by any command that transfers vertices to the
    // GL or launches compute work if the current set of active
    // program objects cannot be executed, for reasons including:
    // - There is no current program object specified by UseProgram, there is a current program
    //    pipeline object, and that object is empty (no executable code is installed for any stage).
    // - A program object is active for at least one, but not all of the shader
    // stages that were present when the program was linked.
    if (!programPipeline->getExecutable().getLinkedShaderStages().any())
    {
        return gl::err::kNoExecutableCodeInstalled;
    }
    for (const ShaderType shaderType : gl::AllShaderTypes())
    {
        const Program *shaderProgram = programPipeline->getShaderProgram(shaderType);
        if (shaderProgram)
        {
            const ProgramExecutable &executable = shaderProgram->getExecutable();
            for (const ShaderType programShaderType : executable.getLinkedShaderStages())
            {
                if (shaderProgram != programPipeline->getShaderProgram(programShaderType))
                {
                    return gl::err::kNotAllStagesOfSeparableProgramUsed;
                }
            }
        }
    }

    // [EXT_geometry_shader] Section 11.1.gs Geometry Shaders
    // A non-separable program object or program pipeline object that includes
    // a geometry shader must also include a vertex shader.
    // An INVALID_OPERATION error is generated by any command that transfers
    // vertices to the GL if the current program state has a geometry shader
    // but no vertex shader.
    if (!programPipeline->getShaderProgram(ShaderType::Vertex) &&
        programPipeline->getShaderProgram(ShaderType::Geometry))
    {
        return gl::err::kNoActiveGraphicsShaderStage;
    }

    return nullptr;
}

const char *ValidateDrawStates(const Context *context, GLenum *outErrorCode)
{
    // Note all errors returned from this function are INVALID_OPERATION except for the draw
    // framebuffer completeness check.
    *outErrorCode = GL_INVALID_OPERATION;

    const Extensions &extensions = context->getExtensions();
    const State &state           = context->getState();

    // WebGL buffers cannot be mapped/unmapped because the MapBufferRange, FlushMappedBufferRange,
    // and UnmapBuffer entry points are removed from the WebGL 2.0 API.
    // https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.14
    VertexArray *vertexArray = state.getVertexArray();
    ASSERT(vertexArray);

    if (!extensions.webglCompatibilityANGLE && vertexArray->hasInvalidMappedArrayBuffer())
    {
        return kBufferMapped;
    }

    // Note: these separate values are not supported in WebGL, due to D3D's limitations. See
    // Section 6.10 of the WebGL 1.0 spec.
    Framebuffer *framebuffer = state.getDrawFramebuffer();
    ASSERT(framebuffer);

    if (context->getLimitations().noSeparateStencilRefsAndMasks ||
        extensions.webglCompatibilityANGLE)
    {
        ASSERT(framebuffer);
        const FramebufferAttachment *dsAttachment =
            framebuffer->getStencilOrDepthStencilAttachment();
        const GLuint stencilBits = dsAttachment ? dsAttachment->getStencilSize() : 0;
        ASSERT(stencilBits <= 8);

        const DepthStencilState &depthStencilState = state.getDepthStencilState();
        if (depthStencilState.stencilTest && stencilBits > 0)
        {
            auto maxStencilValue = angle::BitMask<GLuint>(stencilBits);

            bool differentRefs =
                clamp(state.getStencilRef(), 0, static_cast<GLint>(maxStencilValue)) !=
                clamp(state.getStencilBackRef(), 0, static_cast<GLint>(maxStencilValue));
            bool differentWritemasks = (depthStencilState.stencilWritemask & maxStencilValue) !=
                                       (depthStencilState.stencilBackWritemask & maxStencilValue);
            bool differentMasks = (depthStencilState.stencilMask & maxStencilValue) !=
                                  (depthStencilState.stencilBackMask & maxStencilValue);

            if (differentRefs || differentWritemasks || differentMasks)
            {
                if (!extensions.webglCompatibilityANGLE)
                {
                    WARN() << "This ANGLE implementation does not support separate front/back "
                              "stencil writemasks, reference values, or stencil mask values.";
                }
                return kStencilReferenceMaskOrMismatch;
            }
        }
    }

    if (!extensions.floatBlendEXT)
    {
        const DrawBufferMask blendEnabledActiveFloat32ColorAttachmentDrawBufferMask =
            state.getBlendEnabledDrawBufferMask() &
            framebuffer->getActiveFloat32ColorAttachmentDrawBufferMask();
        if (blendEnabledActiveFloat32ColorAttachmentDrawBufferMask.any())
        {
            return kUnsupportedFloatBlending;
        }
    }

    if (extensions.renderSharedExponentQCOM)
    {
        if (!ValidateColorMasksForSharedExponentColorBuffers(state.getBlendStateExt(), framebuffer))
        {
            return kUnsupportedColorMaskForSharedExponentColorBuffer;
        }
    }

    if (context->getLimitations().noSimultaneousConstantColorAndAlphaBlendFunc ||
        extensions.webglCompatibilityANGLE)
    {
        if (state.hasSimultaneousConstantColorAndAlphaBlendFunc())
        {
            if (extensions.webglCompatibilityANGLE)
            {
                return kInvalidConstantColor;
            }

            WARN() << kConstantColorAlphaLimitation;
            return kConstantColorAlphaLimitation;
        }
    }

    const FramebufferStatus &framebufferStatus = framebuffer->checkStatus(context);
    if (!framebufferStatus.isComplete())
    {
        *outErrorCode = GL_INVALID_FRAMEBUFFER_OPERATION;
        ASSERT(framebufferStatus.reason);
        return framebufferStatus.reason;
    }

    bool framebufferIsYUV = framebuffer->hasYUVAttachment();
    if (framebufferIsYUV)
    {
        const BlendState &blendState = state.getBlendState();
        if (!blendState.colorMaskRed || !blendState.colorMaskGreen || !blendState.colorMaskBlue ||
            !blendState.colorMaskAlpha)
        {
            // When rendering into a YUV framebuffer, the color mask must have r g b and alpha set
            // to true.
            return kInvalidColorMaskForYUV;
        }

        if (blendState.blend)
        {
            // When rendering into a YUV framebuffer, blending must be disabled.
            return kInvalidBlendStateForYUV;
        }
    }
    else
    {
        if (framebuffer->hasExternalTextureAttachment())
        {
            // It is an error to render into an external texture that is not YUV.
            return kExternalTextureAttachmentNotYUV;
        }
    }

    // Advanced blend equation can only be enabled for a single render target.
    const BlendStateExt &blendStateExt = state.getBlendStateExt();
    if (blendStateExt.getUsesAdvancedBlendEquationMask().any())
    {
        const size_t drawBufferCount            = framebuffer->getDrawbufferStateCount();
        uint32_t advancedBlendRenderTargetCount = 0;

        for (size_t drawBufferIndex : blendStateExt.getUsesAdvancedBlendEquationMask())
        {
            if (drawBufferIndex < drawBufferCount &&
                framebuffer->getDrawBufferState(drawBufferIndex) != GL_NONE &&
                blendStateExt.getEnabledMask().test(drawBufferIndex) &&
                blendStateExt.getUsesAdvancedBlendEquationMask().test(drawBufferIndex))
            {
                ++advancedBlendRenderTargetCount;
            }
        }

        if (advancedBlendRenderTargetCount > 1)
        {
            return kAdvancedBlendEquationWithMRT;
        }
    }

    // Dual-source blending functions limit the number of supported draw buffers.
    if (blendStateExt.getUsesExtendedBlendFactorMask().any())
    {
        // Imply the strictest spec interpretation to pass on all OpenGL drivers:
        // dual-source blending is considered active if the blend state contains
        // any SRC1 factor no matter what.
        const size_t drawBufferCount = framebuffer->getDrawbufferStateCount();
        for (size_t drawBufferIndex = context->getCaps().maxDualSourceDrawBuffers;
             drawBufferIndex < drawBufferCount; ++drawBufferIndex)
        {
            if (framebuffer->getDrawBufferState(drawBufferIndex) != GL_NONE)
            {
                return kDualSourceBlendingDrawBuffersLimit;
            }
        }
    }

    if (context->getStateCache().hasAnyEnabledClientAttrib())
    {
        if (extensions.webglCompatibilityANGLE || !state.areClientArraysEnabled())
        {
            // [WebGL 1.0] Section 6.5 Enabled Vertex Attributes and Range Checking
            // If a vertex attribute is enabled as an array via enableVertexAttribArray but no
            // buffer is bound to that attribute via bindBuffer and vertexAttribPointer, then calls
            // to drawArrays or drawElements will generate an INVALID_OPERATION error.
            return kVertexArrayNoBuffer;
        }

        if (state.getVertexArray()->hasEnabledNullPointerClientArray())
        {
            // This is an application error that would normally result in a crash, but we catch it
            // and return an error
            return kVertexArrayNoBufferPointer;
        }
    }

    // If we are running GLES1, there is no current program.
    if (context->getClientVersion() >= Version(2, 0))
    {
        Program *program                    = state.getLinkedProgram(context);
        ProgramPipeline *programPipeline    = state.getLinkedProgramPipeline(context);
        const ProgramExecutable *executable = state.getProgramExecutable();

        bool programIsYUVOutput = false;

        if (program)
        {
            const char *errorMsg = ValidateProgramDrawStates(context, extensions, *executable);
            if (errorMsg)
            {
                return errorMsg;
            }

            programIsYUVOutput = executable->isYUVOutput();
        }
        else if (programPipeline)
        {
            const char *errorMsg = ValidateProgramPipelineAttachedPrograms(programPipeline);
            if (errorMsg)
            {
                return errorMsg;
            }

            errorMsg = ValidateProgramPipelineDrawStates(context, extensions, programPipeline);
            if (errorMsg)
            {
                return errorMsg;
            }

            if (!programPipeline->isLinked())
            {
                return kProgramPipelineLinkFailed;
            }

            programIsYUVOutput = executable->isYUVOutput();
        }

        if (executable)
        {
            if (!executable->validateSamplers(context->getCaps()))
            {
                return kTextureTypeConflict;
            }

            if (executable->hasLinkedTessellationShader())
            {
                if (!executable->hasLinkedShaderStage(ShaderType::Vertex))
                {
                    return kTessellationShaderRequiresVertexShader;
                }

                if (!executable->hasLinkedShaderStage(ShaderType::TessControl) ||
                    !executable->hasLinkedShaderStage(ShaderType::TessEvaluation))
                {
                    return kTessellationShaderRequiresBothControlAndEvaluation;
                }
            }

            if (state.isTransformFeedbackActive())
            {
                if (!ValidateProgramExecutableXFBBuffersPresent(context, executable))
                {
                    return kTransformFeedbackBufferMissing;
                }
            }
        }

        if (programIsYUVOutput != framebufferIsYUV)
        {
            // Both the program and framebuffer must match in YUV output state.
            return kYUVOutputMissmatch;
        }

        if (!state.validateSamplerFormats())
        {
            return kSamplerFormatMismatch;
        }

        // Do some additional WebGL-specific validation
        if (extensions.webglCompatibilityANGLE)
        {
            const TransformFeedback *transformFeedbackObject = state.getCurrentTransformFeedback();
            if (state.isTransformFeedbackActive() &&
                transformFeedbackObject->buffersBoundForOtherUseInWebGL())
            {
                return kTransformFeedbackBufferDoubleBound;
            }

            // Detect rendering feedback loops for WebGL.
            if (framebuffer->formsRenderingFeedbackLoopWith(context))
            {
                return kFeedbackLoop;
            }

            // Detect that the vertex shader input types match the attribute types
            if (!ValidateVertexShaderAttributeTypeMatch(context))
            {
                return kVertexShaderTypeMismatch;
            }

            if (!context->getState().getRasterizerState().rasterizerDiscard &&
                !context->getState().allActiveDrawBufferChannelsMasked())
            {
                // Detect that if there's active color buffer without fragment shader output
                if (!ValidateFragmentShaderColorBufferMaskMatch(context))
                {
                    return kDrawBufferMaskMismatch;
                }

                // Detect that the color buffer types match the fragment shader output types
                if (!ValidateFragmentShaderColorBufferTypeMatch(context))
                {
                    return kDrawBufferTypeMismatch;
                }
            }

            const VertexArray *vao = context->getState().getVertexArray();
            if (vao->hasTransformFeedbackBindingConflict(context))
            {
                return kVertexBufferBoundForTransformFeedback;
            }

            // Validate that we are rendering with a linked program.
            if (!program->isLinked())
            {
                return kProgramNotLinked;
            }
        }

        // The QCOM_framebuffer_foveated spec:
        if (framebuffer->isFoveationEnabled())
        {
            ASSERT(extensions.framebufferFoveatedQCOM);

            //   INVALID_OPERATION is generated if a rendering command is issued and the
            //   current bound program uses tessellation or geometry shaders.
            if (executable->hasLinkedShaderStage(gl::ShaderType::Geometry) ||
                executable->hasLinkedShaderStage(gl::ShaderType::TessControl) ||
                executable->hasLinkedShaderStage(gl::ShaderType::TessEvaluation))
            {
                return err::kGeometryOrTessellationShaderBoundForFoveatedDraw;
            }

            // INVALID_OPERATION is generated by any API call which causes a framebuffer
            // attachment to be written to if the framebuffer attachments have changed for
            // a foveated fbo.
            if (framebuffer->hasAnyAttachmentChanged())
            {
                return err::kFramebufferFoveationAttachmentChanged;
            }
        }
    }

    *outErrorCode = GL_NO_ERROR;
    return nullptr;
}

const char *ValidateProgramPipeline(const Context *context)
{
    const State &state = context->getState();
    // If we are running GLES1, there is no current program.
    if (context->getClientVersion() >= Version(2, 0))
    {
        ProgramPipeline *programPipeline = state.getProgramPipeline();
        if (programPipeline)
        {
            const char *errorMsg = ValidateProgramPipelineAttachedPrograms(programPipeline);
            if (errorMsg)
            {
                return errorMsg;
            }
        }
    }
    return nullptr;
}

void RecordDrawModeError(const Context *context, angle::EntryPoint entryPoint, PrimitiveMode mode)
{
    const State &state                      = context->getState();
    TransformFeedback *curTransformFeedback = state.getCurrentTransformFeedback();
    if (state.isTransformFeedbackActiveUnpaused())
    {
        if (!ValidateTransformFeedbackPrimitiveMode(context, entryPoint,
                                                    curTransformFeedback->getPrimitiveMode(), mode))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidDrawModeTransformFeedback);
            return;
        }
    }

    const Extensions &extensions = context->getExtensions();

    switch (mode)
    {
        case PrimitiveMode::Points:
        case PrimitiveMode::Lines:
        case PrimitiveMode::LineLoop:
        case PrimitiveMode::LineStrip:
        case PrimitiveMode::Triangles:
        case PrimitiveMode::TriangleStrip:
        case PrimitiveMode::TriangleFan:
            break;

        case PrimitiveMode::LinesAdjacency:
        case PrimitiveMode::LineStripAdjacency:
        case PrimitiveMode::TrianglesAdjacency:
        case PrimitiveMode::TriangleStripAdjacency:
            if (!extensions.geometryShaderAny() && context->getClientVersion() < ES_3_2)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kGeometryShaderExtensionNotEnabled);
                return;
            }
            break;

        case PrimitiveMode::Patches:
            if (!extensions.tessellationShaderAny() && context->getClientVersion() < ES_3_2)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kTessellationShaderEXTNotEnabled);
                return;
            }
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidDrawMode);
            return;
    }

    // If we are running GLES1, there is no current program.
    if (context->getClientVersion() >= Version(2, 0))
    {
        const ProgramExecutable *executable = state.getProgramExecutable();
        ASSERT(executable);

        // Do geometry shader specific validations
        if (executable->hasLinkedShaderStage(ShaderType::Geometry))
        {
            if (!IsCompatibleDrawModeWithGeometryShader(
                    mode, executable->getGeometryShaderInputPrimitiveType()))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                       kIncompatibleDrawModeAgainstGeometryShader);
                return;
            }
        }

        if (executable->hasLinkedTessellationShader() && mode != PrimitiveMode::Patches)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                   kIncompatibleDrawModeWithTessellationShader);
            return;
        }

        if (!executable->hasLinkedTessellationShader() && mode == PrimitiveMode::Patches)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                   kIncompatibleDrawModeWithoutTessellationShader);
            return;
        }
    }

    // An error should be recorded.
    UNREACHABLE();
}

bool ValidateDrawArraysInstancedANGLE(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      PrimitiveMode mode,
                                      GLint first,
                                      GLsizei count,
                                      GLsizei primcount)
{
    if (!context->getExtensions().instancedArraysANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (!ValidateDrawArraysInstancedBase(context, entryPoint, mode, first, count, primcount, 0))
    {
        return false;
    }

    return ValidateDrawInstancedANGLE(context, entryPoint);
}

bool ValidateDrawArraysInstancedEXT(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    PrimitiveMode mode,
                                    GLint first,
                                    GLsizei count,
                                    GLsizei primcount)
{
    if (!context->getExtensions().instancedArraysEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (!ValidateDrawArraysInstancedBase(context, entryPoint, mode, first, count, primcount, 0))
    {
        return false;
    }

    return true;
}

const char *ValidateDrawElementsStates(const Context *context)
{
    const State &state = context->getState();

    if (context->getStateCache().isTransformFeedbackActiveUnpaused())
    {
        // EXT_geometry_shader allows transform feedback to work with all draw commands.
        // [EXT_geometry_shader] Section 12.1, "Transform Feedback"
        if (!context->getExtensions().geometryShaderAny() && context->getClientVersion() < ES_3_2)
        {
            // It is an invalid operation to call DrawElements, DrawRangeElements or
            // DrawElementsInstanced while transform feedback is active, (3.0.2, section 2.14, pg
            // 86)
            return kUnsupportedDrawModeForTransformFeedback;
        }
    }

    const VertexArray *vao     = state.getVertexArray();
    Buffer *elementArrayBuffer = vao->getElementArrayBuffer();

    if (elementArrayBuffer)
    {
        if (elementArrayBuffer->hasWebGLXFBBindingConflict(context->isWebGL()))
        {
            return kElementArrayBufferBoundForTransformFeedback;
        }
        if (elementArrayBuffer->isMapped() &&
            (!elementArrayBuffer->isImmutable() ||
             (elementArrayBuffer->getAccessFlags() & GL_MAP_PERSISTENT_BIT_EXT) == 0))
        {
            return kBufferMapped;
        }
    }
    else
    {
        // [WebGL 1.0] Section 6.2 No Client Side Arrays
        // If an indexed draw command (drawElements) is called and no WebGLBuffer is bound to
        // the ELEMENT_ARRAY_BUFFER binding point, an INVALID_OPERATION error is generated.
        if (!context->getState().areClientArraysEnabled() || context->isWebGL())
        {
            return kMustHaveElementArrayBinding;
        }
    }

    return nullptr;
}

bool ValidateDrawElementsInstancedANGLE(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        PrimitiveMode mode,
                                        GLsizei count,
                                        DrawElementsType type,
                                        const void *indices,
                                        GLsizei primcount)
{
    if (!context->getExtensions().instancedArraysANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (!ValidateDrawElementsInstancedBase(context, entryPoint, mode, count, type, indices,
                                           primcount, 0))
    {
        return false;
    }

    return ValidateDrawInstancedANGLE(context, entryPoint);
}

bool ValidateDrawElementsInstancedEXT(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      PrimitiveMode mode,
                                      GLsizei count,
                                      DrawElementsType type,
                                      const void *indices,
                                      GLsizei primcount)
{
    if (!context->getExtensions().instancedArraysEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (!ValidateDrawElementsInstancedBase(context, entryPoint, mode, count, type, indices,
                                           primcount, 0))
    {
        return false;
    }

    return true;
}

bool ValidateGetUniformBase(const Context *context,
                            angle::EntryPoint entryPoint,
                            ShaderProgramID program,
                            UniformLocation location)
{
    if (program.value == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kProgramDoesNotExist);
        return false;
    }

    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (!programObject)
    {
        return false;
    }

    if (!programObject || !programObject->isLinked())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kProgramNotLinked);
        return false;
    }

    if (!programObject->getExecutable().isValidUniformLocation(location))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidUniformLocation);
        return false;
    }

    return true;
}

bool ValidateSizedGetUniform(const Context *context,
                             angle::EntryPoint entryPoint,
                             ShaderProgramID program,
                             UniformLocation location,
                             GLsizei bufSize,
                             GLsizei *length)
{
    if (length)
    {
        *length = 0;
    }

    if (!ValidateGetUniformBase(context, entryPoint, program, location))
    {
        return false;
    }

    if (bufSize < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNegativeBufSize);
        return false;
    }

    Program *programObject = context->getProgramResolveLink(program);
    ASSERT(programObject);

    // sized queries -- ensure the provided buffer is large enough
    const LinkedUniform &uniform = programObject->getExecutable().getUniformByLocation(location);
    size_t requiredBytes         = VariableExternalSize(uniform.getType());
    if (static_cast<size_t>(bufSize) < requiredBytes)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInsufficientBufferSize);
        return false;
    }

    if (length)
    {
        *length = VariableComponentCount(uniform.getType());
    }
    return true;
}

bool ValidateGetnUniformfvEXT(const Context *context,
                              angle::EntryPoint entryPoint,
                              ShaderProgramID program,
                              UniformLocation location,
                              GLsizei bufSize,
                              const GLfloat *params)
{
    return ValidateSizedGetUniform(context, entryPoint, program, location, bufSize, nullptr);
}

bool ValidateGetnUniformfvRobustANGLE(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      ShaderProgramID program,
                                      UniformLocation location,
                                      GLsizei bufSize,
                                      const GLsizei *length,
                                      const GLfloat *params)
{
    UNIMPLEMENTED();
    return false;
}

bool ValidateGetnUniformivEXT(const Context *context,
                              angle::EntryPoint entryPoint,
                              ShaderProgramID program,
                              UniformLocation location,
                              GLsizei bufSize,
                              const GLint *params)
{
    return ValidateSizedGetUniform(context, entryPoint, program, location, bufSize, nullptr);
}

bool ValidateGetnUniformivRobustANGLE(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      ShaderProgramID program,
                                      UniformLocation location,
                                      GLsizei bufSize,
                                      const GLsizei *length,
                                      const GLint *params)
{
    UNIMPLEMENTED();
    return false;
}

bool ValidateGetnUniformuivRobustANGLE(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       ShaderProgramID program,
                                       UniformLocation location,
                                       GLsizei bufSize,
                                       const GLsizei *length,
                                       const GLuint *params)
{
    UNIMPLEMENTED();
    return false;
}

bool ValidateGetUniformfvRobustANGLE(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     ShaderProgramID program,
                                     UniformLocation location,
                                     GLsizei bufSize,
                                     const GLsizei *length,
                                     const GLfloat *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei writeLength = 0;

    // bufSize is validated in ValidateSizedGetUniform
    if (!ValidateSizedGetUniform(context, entryPoint, program, location, bufSize, &writeLength))
    {
        return false;
    }

    SetRobustLengthParam(length, writeLength);

    return true;
}

bool ValidateGetUniformivRobustANGLE(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     ShaderProgramID program,
                                     UniformLocation location,
                                     GLsizei bufSize,
                                     const GLsizei *length,
                                     const GLint *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei writeLength = 0;

    // bufSize is validated in ValidateSizedGetUniform
    if (!ValidateSizedGetUniform(context, entryPoint, program, location, bufSize, &writeLength))
    {
        return false;
    }

    SetRobustLengthParam(length, writeLength);

    return true;
}

bool ValidateGetUniformuivRobustANGLE(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      ShaderProgramID program,
                                      UniformLocation location,
                                      GLsizei bufSize,
                                      const GLsizei *length,
                                      const GLuint *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    GLsizei writeLength = 0;

    // bufSize is validated in ValidateSizedGetUniform
    if (!ValidateSizedGetUniform(context, entryPoint, program, location, bufSize, &writeLength))
    {
        return false;
    }

    SetRobustLengthParam(length, writeLength);

    return true;
}

bool ValidateDiscardFramebufferBase(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    GLenum target,
                                    GLsizei numAttachments,
                                    const GLenum *attachments,
                                    bool defaultFramebuffer)
{
    if (numAttachments < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeAttachments);
        return false;
    }

    for (GLsizei i = 0; i < numAttachments; ++i)
    {
        if (attachments[i] >= GL_COLOR_ATTACHMENT0 && attachments[i] <= GL_COLOR_ATTACHMENT31)
        {
            if (defaultFramebuffer)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kDefaultFramebufferInvalidAttachment);
                return false;
            }

            if (attachments[i] >=
                GL_COLOR_ATTACHMENT0 + static_cast<GLuint>(context->getCaps().maxColorAttachments))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExceedsMaxColorAttachments);
                return false;
            }
        }
        else
        {
            switch (attachments[i])
            {
                case GL_DEPTH_ATTACHMENT:
                case GL_STENCIL_ATTACHMENT:
                case GL_DEPTH_STENCIL_ATTACHMENT:
                    if (defaultFramebuffer)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM,
                                               kDefaultFramebufferInvalidAttachment);
                        return false;
                    }
                    break;
                case GL_COLOR:
                case GL_DEPTH:
                case GL_STENCIL:
                    if (!defaultFramebuffer)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM,
                                               kDefaultFramebufferAttachmentOnUserFBO);
                        return false;
                    }
                    break;
                default:
                    ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidAttachment);
                    return false;
            }
        }
    }

    return true;
}

bool ValidateInsertEventMarkerEXT(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  GLsizei length,
                                  const char *marker)
{
    if (!context->getExtensions().debugMarkerEXT)
    {
        // The debug marker calls should not set error state
        // However, it seems reasonable to set an error state if the extension is not enabled
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    // Note that debug marker calls must not set error state
    if (length < 0)
    {
        return false;
    }

    if (marker == nullptr)
    {
        return false;
    }

    return true;
}

bool ValidatePushGroupMarkerEXT(const Context *context,
                                angle::EntryPoint entryPoint,
                                GLsizei length,
                                const char *marker)
{
    if (!context->getExtensions().debugMarkerEXT)
    {
        // The debug marker calls should not set error state
        // However, it seems reasonable to set an error state if the extension is not enabled
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    // Note that debug marker calls must not set error state
    if (length < 0)
    {
        return false;
    }

    if (length > 0 && marker == nullptr)
    {
        return false;
    }

    return true;
}

bool ValidateEGLImageObject(const Context *context,
                            angle::EntryPoint entryPoint,
                            TextureType type,
                            egl::ImageID imageID)
{
    ASSERT(context->getDisplay());
    if (!context->getDisplay()->isValidImage(imageID))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidEGLImage);
        return false;
    }

    egl::Image *imageObject = context->getDisplay()->getImage(imageID);
    if (imageObject->getSamples() > 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kEGLImageCannotCreate2DMultisampled);
        return false;
    }

    if (!imageObject->isTexturable(context))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kEGLImageTextureFormatNotSupported);
        return false;
    }

    // Validate source egl image and target texture are compatible
    size_t depth = static_cast<size_t>(imageObject->getExtents().depth);
    if (imageObject->isYUV() && type != TextureType::External)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                               "Image is YUV, target must be TEXTURE_EXTERNAL_OES");
        return false;
    }

    if (depth > 1 && type != TextureType::_2DArray && type != TextureType::CubeMap &&
        type != TextureType::CubeMapArray && type != TextureType::_3D)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kEGLImageTextureTargetMismatch);
        return false;
    }

    if (imageObject->isCubeMap() && type != TextureType::CubeMapArray &&
        (type != TextureType::CubeMap || depth > gl::kCubeFaceCount))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kEGLImageTextureTargetMismatch);
        return false;
    }

    if (imageObject->getLevelCount() > 1 && type == TextureType::External)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kEGLImageTextureTargetMismatch);
        return false;
    }

    // 3d EGLImages are currently not supported
    if (type == TextureType::_3D)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kEGLImageTextureTargetMismatch);
        return false;
    }

    if (imageObject->hasProtectedContent() && !context->getState().hasProtectedContent())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                               "Mismatch between Image and Context Protected Content state");
        return false;
    }

    return true;
}

bool ValidateEGLImageTargetTexture2DOES(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        TextureType type,
                                        egl::ImageID image)
{
    if (!context->getExtensions().EGLImageOES && !context->getExtensions().EGLImageExternalOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    switch (type)
    {
        case TextureType::_2D:
            if (!context->getExtensions().EGLImageOES)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, ToGLenum(type));
            }
            break;

        case TextureType::_2DArray:
            if (!context->getExtensions().EGLImageArrayEXT)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, ToGLenum(type));
            }
            break;

        case TextureType::External:
            if (!context->getExtensions().EGLImageExternalOES)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, ToGLenum(type));
            }
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
            return false;
    }

    return ValidateEGLImageObject(context, entryPoint, type, image);
}

bool ValidateEGLImageTargetRenderbufferStorageOES(const Context *context,
                                                  angle::EntryPoint entryPoint,
                                                  GLenum target,
                                                  egl::ImageID image)
{
    if (!context->getExtensions().EGLImageOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    switch (target)
    {
        case GL_RENDERBUFFER:
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidRenderbufferTarget);
            return false;
    }

    ASSERT(context->getDisplay());
    if (!context->getDisplay()->isValidImage(image))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidEGLImage);
        return false;
    }

    egl::Image *imageObject = context->getDisplay()->getImage(image);
    if (!imageObject->isRenderable(context))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kEGLImageRenderbufferFormatNotSupported);
        return false;
    }
    const auto &glState = context->getState();
    if (imageObject->hasProtectedContent() != glState.hasProtectedContent())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                               "Mismatch between Image and Context Protected Content state");
        return false;
    }

    Renderbuffer *renderbuffer = glState.getCurrentRenderbuffer();
    if (renderbuffer == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kRenderbufferNotBound);
        return false;
    }

    return true;
}

bool ValidateProgramBinaryBase(const Context *context,
                               angle::EntryPoint entryPoint,
                               ShaderProgramID program,
                               GLenum binaryFormat,
                               const void *binary,
                               GLint length)
{
    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (programObject == nullptr)
    {
        return false;
    }

    const std::vector<GLenum> &programBinaryFormats = context->getCaps().programBinaryFormats;
    if (std::find(programBinaryFormats.begin(), programBinaryFormats.end(), binaryFormat) ==
        programBinaryFormats.end())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidProgramBinaryFormat);
        return false;
    }

    if (context->hasActiveTransformFeedback(program))
    {
        // ES 3.0.4 section 2.15 page 91
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTransformFeedbackProgramBinary);
        return false;
    }

    return true;
}

bool ValidateGetProgramBinaryBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID program,
                                  GLsizei bufSize,
                                  const GLsizei *length,
                                  const GLenum *binaryFormat,
                                  const void *binary)
{
    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (programObject == nullptr)
    {
        return false;
    }

    if (!programObject->isLinked())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kProgramNotLinked);
        return false;
    }

    if (context->getCaps().programBinaryFormats.empty())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNoProgramBinaryFormats);
        return false;
    }

    return true;
}

bool ValidateDrawBuffersBase(const Context *context,
                             angle::EntryPoint entryPoint,
                             GLsizei n,
                             const GLenum *bufs)
{
    // INVALID_VALUE is generated if n is negative or greater than value of MAX_DRAW_BUFFERS
    if (n < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeCount);
        return false;
    }
    if (n > context->getCaps().maxDrawBuffers)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxDrawBuffer);
        return false;
    }

    ASSERT(context->getState().getDrawFramebuffer());
    FramebufferID frameBufferId = context->getState().getDrawFramebuffer()->id();
    GLuint maxColorAttachment   = GL_COLOR_ATTACHMENT0_EXT + context->getCaps().maxColorAttachments;

    // This should come first before the check for the default frame buffer
    // because when we switch to ES3.1+, invalid enums will return INVALID_ENUM
    // rather than INVALID_OPERATION
    for (int colorAttachment = 0; colorAttachment < n; colorAttachment++)
    {
        const GLenum attachment = GL_COLOR_ATTACHMENT0_EXT + colorAttachment;

        if (bufs[colorAttachment] != GL_NONE && bufs[colorAttachment] != GL_BACK &&
            (bufs[colorAttachment] < GL_COLOR_ATTACHMENT0 ||
             bufs[colorAttachment] > GL_COLOR_ATTACHMENT31))
        {
            // Value in bufs is not NONE, BACK, or GL_COLOR_ATTACHMENTi
            // The 3.0.4 spec says to generate GL_INVALID_OPERATION here, but this
            // was changed to GL_INVALID_ENUM in 3.1, which dEQP also expects.
            // 3.1 is still a bit ambiguous about the error, but future specs are
            // expected to clarify that GL_INVALID_ENUM is the correct error.
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidDrawBuffer);
            return false;
        }
        else if (bufs[colorAttachment] >= maxColorAttachment)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExceedsMaxColorAttachments);
            return false;
        }
        else if (bufs[colorAttachment] != GL_NONE && bufs[colorAttachment] != attachment &&
                 frameBufferId.value != 0)
        {
            // INVALID_OPERATION-GL is bound to buffer and ith argument
            // is not COLOR_ATTACHMENTi or NONE
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidDrawBufferValue);
            return false;
        }
    }

    // INVALID_OPERATION is generated if GL is bound to the default framebuffer
    // and n is not 1 or bufs is bound to value other than BACK and NONE
    if (frameBufferId.value == 0)
    {
        if (n != 1)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidDrawBufferCountForDefault);
            return false;
        }

        if (bufs[0] != GL_NONE && bufs[0] != GL_BACK)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kDefaultFramebufferInvalidDrawBuffer);
            return false;
        }
    }

    return true;
}

bool ValidateGetBufferPointervBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   BufferBinding target,
                                   GLenum pname,
                                   GLsizei *length,
                                   void *const *params)
{
    if (length)
    {
        *length = 0;
    }

    if (!context->isValidBufferBinding(target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidBufferTypes);
        return false;
    }

    switch (pname)
    {
        case GL_BUFFER_MAP_POINTER:
            break;

        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
            return false;
    }

    // GLES 3.0 section 2.10.1: "Attempts to attempts to modify or query buffer object state for a
    // target bound to zero generate an INVALID_OPERATION error."
    // GLES 3.1 section 6.6 explicitly specifies this error.
    if (context->getState().getTargetBuffer(target) == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferPointerNotAvailable);
        return false;
    }

    if (length)
    {
        *length = 1;
    }

    return true;
}

bool ValidateUnmapBufferBase(const Context *context,
                             angle::EntryPoint entryPoint,
                             BufferBinding target)
{
    if (!context->isValidBufferBinding(target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidBufferTypes);
        return false;
    }

    Buffer *buffer = context->getState().getTargetBuffer(target);

    if (buffer == nullptr || !buffer->isMapped())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferNotMapped);
        return false;
    }

    return true;
}

bool ValidateMapBufferRangeBase(const Context *context,
                                angle::EntryPoint entryPoint,
                                BufferBinding target,
                                GLintptr offset,
                                GLsizeiptr length,
                                GLbitfield access)
{
    if (!context->isValidBufferBinding(target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidBufferTypes);
        return false;
    }

    if (offset < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeOffset);
        return false;
    }

    if (length < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeLength);
        return false;
    }

    Buffer *buffer = context->getState().getTargetBuffer(target);

    if (!buffer)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferNotMappable);
        return false;
    }

    // Check for buffer overflow
    CheckedNumeric<size_t> checkedOffset(offset);
    auto checkedSize = checkedOffset + length;

    if (!checkedSize.IsValid() || checkedSize.ValueOrDie() > static_cast<size_t>(buffer->getSize()))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kMapOutOfRange);
        return false;
    }

    // Check for invalid bits in the mask
    constexpr GLbitfield kAllAccessBits =
        GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT |
        GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT;

    if (buffer->isImmutable())
    {
        // GL_EXT_buffer_storage's additions to glMapBufferRange
        constexpr GLbitfield kBufferStorageAccessBits =
            kAllAccessBits | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT;

        if ((access & ~kBufferStorageAccessBits) != 0)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidAccessBits);
            return false;
        }

        // It is invalid if any of bufferStorageMatchedAccessBits bits are included in access,
        // but the same bits are not included in the buffer's storage flags
        constexpr GLbitfield kBufferStorageMatchedAccessBits = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT |
                                                               GL_MAP_PERSISTENT_BIT_EXT |
                                                               GL_MAP_COHERENT_BIT_EXT;
        GLbitfield accessFlags = access & kBufferStorageMatchedAccessBits;
        if ((accessFlags & buffer->getStorageExtUsageFlags()) != accessFlags)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidAccessBits);
            return false;
        }
    }
    else if ((access & ~kAllAccessBits) != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidAccessBits);
        return false;
    }

    if (length == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kLengthZero);
        return false;
    }

    if (buffer->isMapped())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferAlreadyMapped);
        return false;
    }

    // Check for invalid bit combinations
    if ((access & (GL_MAP_READ_BIT | GL_MAP_WRITE_BIT)) == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidAccessBitsReadWrite);
        return false;
    }

    GLbitfield writeOnlyBits =
        GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT;

    if ((access & GL_MAP_READ_BIT) != 0 && (access & writeOnlyBits) != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidAccessBitsRead);
        return false;
    }

    if ((access & GL_MAP_WRITE_BIT) == 0 && (access & GL_MAP_FLUSH_EXPLICIT_BIT) != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidAccessBitsFlush);
        return false;
    }

    return ValidateMapBufferBase(context, entryPoint, target);
}

bool ValidateFlushMappedBufferRangeBase(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        BufferBinding target,
                                        GLintptr offset,
                                        GLsizeiptr length)
{
    if (offset < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeOffset);
        return false;
    }

    if (length < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeLength);
        return false;
    }

    if (!context->isValidBufferBinding(target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidBufferTypes);
        return false;
    }

    Buffer *buffer = context->getState().getTargetBuffer(target);

    if (buffer == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFlushZero);
        return false;
    }

    if (!buffer->isMapped() || (buffer->getAccessFlags() & GL_MAP_FLUSH_EXPLICIT_BIT) == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFlushTarget);
        return false;
    }

    // Check for buffer overflow
    CheckedNumeric<size_t> checkedOffset(offset);
    auto checkedSize = checkedOffset + length;

    if (!checkedSize.IsValid() ||
        checkedSize.ValueOrDie() > static_cast<size_t>(buffer->getMapLength()))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidFlushOutOfRange);
        return false;
    }

    return true;
}

bool ValidateGenOrDelete(const Context *context, angle::EntryPoint entryPoint, GLint n)
{
    if (n < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeCount);
        return false;
    }
    return true;
}

bool ValidateRobustEntryPoint(const Context *context, angle::EntryPoint entryPoint, GLsizei bufSize)
{
    if (!context->getExtensions().robustClientMemoryANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (bufSize < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeBufSize);
        return false;
    }

    return true;
}

bool ValidateRobustBufferSize(const Context *context,
                              angle::EntryPoint entryPoint,
                              GLsizei bufSize,
                              GLsizei numParams)
{
    if (bufSize < numParams)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInsufficientParams);
        return false;
    }

    return true;
}

bool ValidateGetFramebufferAttachmentParameterivBase(const Context *context,
                                                     angle::EntryPoint entryPoint,
                                                     GLenum target,
                                                     GLenum attachment,
                                                     GLenum pname,
                                                     GLsizei *numParams)
{
    if (!ValidFramebufferTarget(context, target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFramebufferTarget);
        return false;
    }

    int clientVersion = context->getClientMajorVersion();

    switch (pname)
    {
        case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
        case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_NUM_VIEWS_OVR:
        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_BASE_VIEW_INDEX_OVR:
            if (clientVersion < 3 ||
                !(context->getExtensions().multiviewOVR || context->getExtensions().multiview2OVR))
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SAMPLES_EXT:
            if (!context->getExtensions().multisampledRenderToTextureEXT)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING:
            if (clientVersion < 3 && !context->getExtensions().sRGBEXT)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE:
            if (clientVersion < 3 && !context->getExtensions().colorBufferHalfFloatEXT &&
                !context->getExtensions().colorBufferFloatRgbCHROMIUM &&
                !context->getExtensions().colorBufferFloatRgbaCHROMIUM)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE:
        case GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE:
        case GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE:
        case GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE:
        case GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE:
        case GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE:
        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER:
            if (clientVersion < 3)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kES3Required);
                return false;
            }
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_LAYERED_EXT:
            if (!context->getExtensions().geometryShaderAny() &&
                context->getClientVersion() < ES_3_2)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kGeometryShaderExtensionNotEnabled);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidPname);
            return false;
    }

    // Determine if the attachment is a valid enum
    switch (attachment)
    {
        case GL_BACK:
        case GL_DEPTH:
        case GL_STENCIL:
            if (clientVersion < 3)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidAttachment);
                return false;
            }
            break;

        case GL_DEPTH_STENCIL_ATTACHMENT:
            if (clientVersion < 3 && !context->isWebGL1())
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidAttachment);
                return false;
            }
            break;

        case GL_COLOR_ATTACHMENT0:
        case GL_DEPTH_ATTACHMENT:
        case GL_STENCIL_ATTACHMENT:
            break;

        default:
            if ((clientVersion < 3 && !context->getExtensions().drawBuffersEXT) ||
                attachment < GL_COLOR_ATTACHMENT0_EXT ||
                (attachment - GL_COLOR_ATTACHMENT0_EXT) >=
                    static_cast<GLuint>(context->getCaps().maxColorAttachments))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidAttachment);
                return false;
            }
            break;
    }

    const Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);
    ASSERT(framebuffer);

    if (framebuffer->isDefault())
    {
        if (clientVersion < 3)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kDefaultFramebufferTarget);
            return false;
        }

        switch (attachment)
        {
            case GL_BACK:
            case GL_DEPTH:
            case GL_STENCIL:
                break;

            default:
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidAttachment);
                return false;
        }
    }
    else
    {
        if (attachment >= GL_COLOR_ATTACHMENT0_EXT && attachment <= GL_COLOR_ATTACHMENT15_EXT)
        {
            // Valid attachment query
        }
        else
        {
            switch (attachment)
            {
                case GL_DEPTH_ATTACHMENT:
                case GL_STENCIL_ATTACHMENT:
                    break;

                case GL_DEPTH_STENCIL_ATTACHMENT:
                    if (!framebuffer->hasValidDepthStencil() && !context->isWebGL1())
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidAttachment);
                        return false;
                    }
                    break;

                default:
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidAttachment);
                    return false;
            }
        }
    }

    const FramebufferAttachment *attachmentObject = framebuffer->getAttachment(context, attachment);
    if (attachmentObject)
    {
        ASSERT(attachmentObject->type() == GL_RENDERBUFFER ||
               attachmentObject->type() == GL_TEXTURE ||
               attachmentObject->type() == GL_FRAMEBUFFER_DEFAULT);

        switch (pname)
        {
            case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
                if (attachmentObject->type() != GL_RENDERBUFFER &&
                    attachmentObject->type() != GL_TEXTURE)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kFramebufferIncompleteAttachment);
                    return false;
                }
                break;

            case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
                if (attachmentObject->type() != GL_TEXTURE)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kFramebufferIncompleteAttachment);
                    return false;
                }
                break;

            case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
                if (attachmentObject->type() != GL_TEXTURE)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kFramebufferIncompleteAttachment);
                    return false;
                }
                break;

            case GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE:
                if (attachment == GL_DEPTH_STENCIL_ATTACHMENT)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidAttachment);
                    return false;
                }
                break;

            case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER:
                if (attachmentObject->type() != GL_TEXTURE)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kFramebufferIncompleteAttachment);
                    return false;
                }
                break;

            default:
                break;
        }
    }
    else
    {
        // ES 2.0.25 spec pg 127 states that if the value of FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE
        // is NONE, then querying any other pname will generate INVALID_ENUM.

        // ES 3.0.2 spec pg 235 states that if the attachment type is none,
        // GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME will return zero and be an
        // INVALID_OPERATION for all other pnames

        switch (pname)
        {
            case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
                break;

            case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
                if (clientVersion < 3)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFramebufferAttachmentParameter);
                    return false;
                }
                break;

            default:
                if (clientVersion < 3)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFramebufferAttachmentParameter);
                    return false;
                }
                else
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                           kInvalidFramebufferAttachmentParameter);
                    return false;
                }
        }
    }

    if (numParams)
    {
        *numParams = 1;
    }

    return true;
}

bool ValidateGetFramebufferParameterivBase(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           GLenum target,
                                           GLenum pname,
                                           const GLint *params)
{
    if (!ValidFramebufferTarget(context, target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFramebufferTarget);
        return false;
    }

    switch (pname)
    {
        case GL_FRAMEBUFFER_DEFAULT_WIDTH:
        case GL_FRAMEBUFFER_DEFAULT_HEIGHT:
        case GL_FRAMEBUFFER_DEFAULT_SAMPLES:
        case GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS:
            break;
        case GL_FRAMEBUFFER_DEFAULT_LAYERS_EXT:
            if (!context->getExtensions().geometryShaderAny() &&
                context->getClientVersion() < ES_3_2)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kGeometryShaderExtensionNotEnabled);
                return false;
            }
            break;
        case GL_FRAMEBUFFER_FLIP_Y_MESA:
            if (!context->getExtensions().framebufferFlipYMESA)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidPname);
                return false;
            }
            break;
        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidPname);
            return false;
    }

    const Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);
    ASSERT(framebuffer);

    if (framebuffer->isDefault())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kDefaultFramebuffer);
        return false;
    }
    return true;
}

bool ValidateGetFramebufferAttachmentParameterivRobustANGLE(const Context *context,
                                                            angle::EntryPoint entryPoint,
                                                            GLenum target,
                                                            GLenum attachment,
                                                            GLenum pname,
                                                            GLsizei bufSize,
                                                            const GLsizei *length,
                                                            const GLint *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei numParams = 0;
    if (!ValidateGetFramebufferAttachmentParameterivBase(context, entryPoint, target, attachment,
                                                         pname, &numParams))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, numParams))
    {
        return false;
    }

    SetRobustLengthParam(length, numParams);

    return true;
}

bool ValidateGetBufferParameterivRobustANGLE(const Context *context,
                                             angle::EntryPoint entryPoint,
                                             BufferBinding target,
                                             GLenum pname,
                                             GLsizei bufSize,
                                             const GLsizei *length,
                                             const GLint *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei numParams = 0;

    if (!ValidateGetBufferParameterBase(context, entryPoint, target, pname, false, &numParams))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, numParams))
    {
        return false;
    }

    SetRobustLengthParam(length, numParams);
    return true;
}

bool ValidateGetBufferParameteri64vRobustANGLE(const Context *context,
                                               angle::EntryPoint entryPoint,
                                               BufferBinding target,
                                               GLenum pname,
                                               GLsizei bufSize,
                                               const GLsizei *length,
                                               const GLint64 *params)
{
    GLsizei numParams = 0;

    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    if (!ValidateGetBufferParameterBase(context, entryPoint, target, pname, false, &numParams))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, numParams))
    {
        return false;
    }

    SetRobustLengthParam(length, numParams);

    return true;
}

bool ValidateGetProgramivBase(const Context *context,
                              angle::EntryPoint entryPoint,
                              ShaderProgramID program,
                              GLenum pname,
                              GLsizei *numParams)
{
    // Currently, all GetProgramiv queries return 1 parameter
    if (numParams)
    {
        *numParams = 1;
    }

    if (context->isContextLost())
    {
        ANGLE_VALIDATION_ERROR(GL_CONTEXT_LOST, kContextLost);

        if (context->getExtensions().parallelShaderCompileKHR && pname == GL_COMPLETION_STATUS_KHR)
        {
            // Generate an error but still return true, the context still needs to return a
            // value in this case.
            return true;
        }
        else
        {
            return false;
        }
    }

    // Special case for GL_COMPLETION_STATUS_KHR: don't resolve the link. Otherwise resolve it now.
    Program *programObject = (pname == GL_COMPLETION_STATUS_KHR)
                                 ? GetValidProgramNoResolve(context, entryPoint, program)
                                 : GetValidProgram(context, entryPoint, program);
    if (!programObject)
    {
        return false;
    }

    switch (pname)
    {
        case GL_DELETE_STATUS:
        case GL_LINK_STATUS:
        case GL_VALIDATE_STATUS:
        case GL_INFO_LOG_LENGTH:
        case GL_ATTACHED_SHADERS:
        case GL_ACTIVE_ATTRIBUTES:
        case GL_ACTIVE_ATTRIBUTE_MAX_LENGTH:
        case GL_ACTIVE_UNIFORMS:
        case GL_ACTIVE_UNIFORM_MAX_LENGTH:
            break;

        case GL_PROGRAM_BINARY_READY_ANGLE:
            if (!context->getExtensions().programBinaryReadinessQueryANGLE)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        case GL_PROGRAM_BINARY_LENGTH:
            if (context->getClientMajorVersion() < 3 &&
                !context->getExtensions().getProgramBinaryOES)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        case GL_ACTIVE_UNIFORM_BLOCKS:
        case GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH:
        case GL_TRANSFORM_FEEDBACK_BUFFER_MODE:
        case GL_TRANSFORM_FEEDBACK_VARYINGS:
        case GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH:
        case GL_PROGRAM_BINARY_RETRIEVABLE_HINT:
            if (context->getClientMajorVersion() < 3)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kEnumRequiresGLES30);
                return false;
            }
            break;

        case GL_PROGRAM_SEPARABLE:
        case GL_ACTIVE_ATOMIC_COUNTER_BUFFERS:
            if (context->getClientVersion() < Version(3, 1))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kEnumRequiresGLES31);
                return false;
            }
            break;

        case GL_COMPUTE_WORK_GROUP_SIZE:
            if (context->getClientVersion() < Version(3, 1))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kEnumRequiresGLES31);
                return false;
            }

            // [OpenGL ES 3.1] Chapter 7.12 Page 122
            // An INVALID_OPERATION error is generated if COMPUTE_WORK_GROUP_SIZE is queried for a
            // program which has not been linked successfully, or which does not contain objects to
            // form a compute shader.
            if (!programObject->isLinked())
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kProgramNotLinked);
                return false;
            }
            if (!programObject->getExecutable().hasLinkedShaderStage(ShaderType::Compute))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNoActiveComputeShaderStage);
                return false;
            }
            break;

        case GL_GEOMETRY_LINKED_INPUT_TYPE_EXT:
        case GL_GEOMETRY_LINKED_OUTPUT_TYPE_EXT:
        case GL_GEOMETRY_LINKED_VERTICES_OUT_EXT:
        case GL_GEOMETRY_SHADER_INVOCATIONS_EXT:
            if (!context->getExtensions().geometryShaderAny() &&
                context->getClientVersion() < ES_3_2)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kGeometryShaderExtensionNotEnabled);
                return false;
            }

            // [EXT_geometry_shader] Chapter 7.12
            // An INVALID_OPERATION error is generated if GEOMETRY_LINKED_VERTICES_OUT_EXT,
            // GEOMETRY_LINKED_INPUT_TYPE_EXT, GEOMETRY_LINKED_OUTPUT_TYPE_EXT, or
            // GEOMETRY_SHADER_INVOCATIONS_EXT are queried for a program which has not been linked
            // successfully, or which does not contain objects to form a geometry shader.
            if (!programObject->isLinked())
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kProgramNotLinked);
                return false;
            }
            if (!programObject->getExecutable().hasLinkedShaderStage(ShaderType::Geometry))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNoActiveGeometryShaderStage);
                return false;
            }
            break;

        case GL_COMPLETION_STATUS_KHR:
            if (!context->getExtensions().parallelShaderCompileKHR)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
                return false;
            }
            break;
        case GL_TESS_CONTROL_OUTPUT_VERTICES_EXT:
        case GL_TESS_GEN_MODE_EXT:
        case GL_TESS_GEN_SPACING_EXT:
        case GL_TESS_GEN_VERTEX_ORDER_EXT:
        case GL_TESS_GEN_POINT_MODE_EXT:
            if (!context->getExtensions().tessellationShaderAny() &&
                context->getClientVersion() < ES_3_2)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kTessellationShaderEXTNotEnabled);
                return false;
            }
            if (!programObject->isLinked())
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kProgramNotLinked);
                return false;
            }
            break;
        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
            return false;
    }

    return true;
}

bool ValidateGetProgramivRobustANGLE(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     ShaderProgramID program,
                                     GLenum pname,
                                     GLsizei bufSize,
                                     const GLsizei *length,
                                     const GLint *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei numParams = 0;

    if (!ValidateGetProgramivBase(context, entryPoint, program, pname, &numParams))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, numParams))
    {
        return false;
    }

    SetRobustLengthParam(length, numParams);

    return true;
}

bool ValidateGetRenderbufferParameterivRobustANGLE(const Context *context,
                                                   angle::EntryPoint entryPoint,
                                                   GLenum target,
                                                   GLenum pname,
                                                   GLsizei bufSize,
                                                   const GLsizei *length,
                                                   const GLint *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei numParams = 0;

    if (!ValidateGetRenderbufferParameterivBase(context, entryPoint, target, pname, &numParams))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, numParams))
    {
        return false;
    }

    SetRobustLengthParam(length, numParams);

    return true;
}

bool ValidateGetShaderivRobustANGLE(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    ShaderProgramID shader,
                                    GLenum pname,
                                    GLsizei bufSize,
                                    const GLsizei *length,
                                    const GLint *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei numParams = 0;

    if (!ValidateGetShaderivBase(context, entryPoint, shader, pname, &numParams))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, numParams))
    {
        return false;
    }

    SetRobustLengthParam(length, numParams);

    return true;
}

bool ValidateGetTexParameterfvRobustANGLE(const Context *context,
                                          angle::EntryPoint entryPoint,
                                          TextureType target,
                                          GLenum pname,
                                          GLsizei bufSize,
                                          const GLsizei *length,
                                          const GLfloat *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei numParams = 0;

    if (!ValidateGetTexParameterBase(context, entryPoint, target, pname, &numParams))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, numParams))
    {
        return false;
    }

    SetRobustLengthParam(length, numParams);

    return true;
}

bool ValidateGetTexParameterivRobustANGLE(const Context *context,
                                          angle::EntryPoint entryPoint,
                                          TextureType target,
                                          GLenum pname,
                                          GLsizei bufSize,
                                          const GLsizei *length,
                                          const GLint *params)
{

    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }
    GLsizei numParams = 0;
    if (!ValidateGetTexParameterBase(context, entryPoint, target, pname, &numParams))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, numParams))
    {
        return false;
    }

    SetRobustLengthParam(length, numParams);
    return true;
}

bool ValidateGetTexParameterIivRobustANGLE(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           TextureType target,
                                           GLenum pname,
                                           GLsizei bufSize,
                                           const GLsizei *length,
                                           const GLint *params)
{
    UNIMPLEMENTED();
    return false;
}

bool ValidateGetTexParameterIuivRobustANGLE(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            TextureType target,
                                            GLenum pname,
                                            GLsizei bufSize,
                                            const GLsizei *length,
                                            const GLuint *params)
{
    UNIMPLEMENTED();
    return false;
}

bool ValidateTexParameterfvRobustANGLE(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       TextureType target,
                                       GLenum pname,
                                       GLsizei bufSize,
                                       const GLfloat *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    return ValidateTexParameterBase(context, entryPoint, target, pname, bufSize, true, params);
}

bool ValidateTexParameterivRobustANGLE(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       TextureType target,
                                       GLenum pname,
                                       GLsizei bufSize,
                                       const GLint *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    return ValidateTexParameterBase(context, entryPoint, target, pname, bufSize, true, params);
}

bool ValidateTexParameterIivRobustANGLE(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        TextureType target,
                                        GLenum pname,
                                        GLsizei bufSize,
                                        const GLint *params)
{
    UNIMPLEMENTED();
    return false;
}

bool ValidateTexParameterIuivRobustANGLE(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         TextureType target,
                                         GLenum pname,
                                         GLsizei bufSize,
                                         const GLuint *params)
{
    UNIMPLEMENTED();
    return false;
}

bool ValidateGetSamplerParameterfvRobustANGLE(const Context *context,
                                              angle::EntryPoint entryPoint,
                                              SamplerID sampler,
                                              GLenum pname,
                                              GLsizei bufSize,
                                              const GLsizei *length,
                                              const GLfloat *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei numParams = 0;

    if (!ValidateGetSamplerParameterBase(context, entryPoint, sampler, pname, &numParams))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, numParams))
    {
        return false;
    }

    SetRobustLengthParam(length, numParams);
    return true;
}

bool ValidateGetSamplerParameterivRobustANGLE(const Context *context,
                                              angle::EntryPoint entryPoint,
                                              SamplerID sampler,
                                              GLenum pname,
                                              GLsizei bufSize,
                                              const GLsizei *length,
                                              const GLint *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei numParams = 0;

    if (!ValidateGetSamplerParameterBase(context, entryPoint, sampler, pname, &numParams))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, numParams))
    {
        return false;
    }

    SetRobustLengthParam(length, numParams);
    return true;
}

bool ValidateGetSamplerParameterIivRobustANGLE(const Context *context,
                                               angle::EntryPoint entryPoint,
                                               SamplerID sampler,
                                               GLenum pname,
                                               GLsizei bufSize,
                                               const GLsizei *length,
                                               const GLint *params)
{
    UNIMPLEMENTED();
    return false;
}

bool ValidateGetSamplerParameterIuivRobustANGLE(const Context *context,
                                                angle::EntryPoint entryPoint,
                                                SamplerID sampler,
                                                GLenum pname,
                                                GLsizei bufSize,
                                                const GLsizei *length,
                                                const GLuint *params)
{
    UNIMPLEMENTED();
    return false;
}

bool ValidateSamplerParameterfvRobustANGLE(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           SamplerID sampler,
                                           GLenum pname,
                                           GLsizei bufSize,
                                           const GLfloat *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    return ValidateSamplerParameterBase(context, entryPoint, sampler, pname, bufSize, true, params);
}

bool ValidateSamplerParameterivRobustANGLE(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           SamplerID sampler,
                                           GLenum pname,
                                           GLsizei bufSize,
                                           const GLint *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    return ValidateSamplerParameterBase(context, entryPoint, sampler, pname, bufSize, true, params);
}

bool ValidateSamplerParameterIivRobustANGLE(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            SamplerID sampler,
                                            GLenum pname,
                                            GLsizei bufSize,
                                            const GLint *param)
{
    UNIMPLEMENTED();
    return false;
}

bool ValidateSamplerParameterIuivRobustANGLE(const Context *context,
                                             angle::EntryPoint entryPoint,
                                             SamplerID sampler,
                                             GLenum pname,
                                             GLsizei bufSize,
                                             const GLuint *param)
{
    UNIMPLEMENTED();
    return false;
}

bool ValidateGetVertexAttribfvRobustANGLE(const Context *context,
                                          angle::EntryPoint entryPoint,
                                          GLuint index,
                                          GLenum pname,
                                          GLsizei bufSize,
                                          const GLsizei *length,
                                          const GLfloat *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei writeLength = 0;

    if (!ValidateGetVertexAttribBase(context, entryPoint, index, pname, &writeLength, false, false))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, writeLength))
    {
        return false;
    }

    SetRobustLengthParam(length, writeLength);
    return true;
}

bool ValidateGetVertexAttribivRobustANGLE(const Context *context,
                                          angle::EntryPoint entryPoint,
                                          GLuint index,
                                          GLenum pname,
                                          GLsizei bufSize,
                                          const GLsizei *length,
                                          const GLint *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei writeLength = 0;

    if (!ValidateGetVertexAttribBase(context, entryPoint, index, pname, &writeLength, false, false))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, writeLength))
    {
        return false;
    }

    SetRobustLengthParam(length, writeLength);

    return true;
}

bool ValidateGetVertexAttribPointervRobustANGLE(const Context *context,
                                                angle::EntryPoint entryPoint,
                                                GLuint index,
                                                GLenum pname,
                                                GLsizei bufSize,
                                                const GLsizei *length,
                                                void *const *pointer)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei writeLength = 0;

    if (!ValidateGetVertexAttribBase(context, entryPoint, index, pname, &writeLength, true, false))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, writeLength))
    {
        return false;
    }

    SetRobustLengthParam(length, writeLength);

    return true;
}

bool ValidateGetVertexAttribIivRobustANGLE(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           GLuint index,
                                           GLenum pname,
                                           GLsizei bufSize,
                                           const GLsizei *length,
                                           const GLint *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei writeLength = 0;

    if (!ValidateGetVertexAttribBase(context, entryPoint, index, pname, &writeLength, false, true))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, writeLength))
    {
        return false;
    }

    SetRobustLengthParam(length, writeLength);

    return true;
}

bool ValidateGetVertexAttribIuivRobustANGLE(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            GLuint index,
                                            GLenum pname,
                                            GLsizei bufSize,
                                            const GLsizei *length,
                                            const GLuint *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei writeLength = 0;

    if (!ValidateGetVertexAttribBase(context, entryPoint, index, pname, &writeLength, false, true))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, writeLength))
    {
        return false;
    }

    SetRobustLengthParam(length, writeLength);

    return true;
}

bool ValidateGetActiveUniformBlockivRobustANGLE(const Context *context,
                                                angle::EntryPoint entryPoint,
                                                ShaderProgramID program,
                                                UniformBlockIndex uniformBlockIndex,
                                                GLenum pname,
                                                GLsizei bufSize,
                                                const GLsizei *length,
                                                const GLint *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei writeLength = 0;

    if (!ValidateGetActiveUniformBlockivBase(context, entryPoint, program, uniformBlockIndex, pname,
                                             &writeLength))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, writeLength))
    {
        return false;
    }

    SetRobustLengthParam(length, writeLength);

    return true;
}

bool ValidateGetInternalformativRobustANGLE(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            GLenum target,
                                            GLenum internalformat,
                                            GLenum pname,
                                            GLsizei bufSize,
                                            const GLsizei *length,
                                            const GLint *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei numParams = 0;

    if (!ValidateGetInternalFormativBase(context, entryPoint, target, internalformat, pname,
                                         bufSize, &numParams))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, numParams))
    {
        return false;
    }

    SetRobustLengthParam(length, numParams);

    return true;
}

bool ValidateRobustCompressedTexImageBase(const Context *context,
                                          angle::EntryPoint entryPoint,
                                          GLsizei imageSize,
                                          GLsizei dataSize)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, dataSize))
    {
        return false;
    }

    Buffer *pixelUnpackBuffer = context->getState().getTargetBuffer(BufferBinding::PixelUnpack);
    if (pixelUnpackBuffer == nullptr)
    {
        if (dataSize < imageSize)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kCompressedDataSizeTooSmall);
        }
    }
    return true;
}

bool ValidateGetBufferParameterBase(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    BufferBinding target,
                                    GLenum pname,
                                    bool pointerVersion,
                                    GLsizei *numParams)
{
    if (numParams)
    {
        *numParams = 0;
    }

    if (!context->isValidBufferBinding(target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidBufferTypes);
        return false;
    }

    const Buffer *buffer = context->getState().getTargetBuffer(target);
    if (!buffer)
    {
        // A null buffer means that "0" is bound to the requested buffer target
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferNotBound);
        return false;
    }

    const Extensions &extensions = context->getExtensions();

    switch (pname)
    {
        case GL_BUFFER_USAGE:
        case GL_BUFFER_SIZE:
            break;

        case GL_BUFFER_ACCESS_OES:
            if (!extensions.mapbufferOES)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        case GL_BUFFER_MAPPED:
            static_assert(GL_BUFFER_MAPPED == GL_BUFFER_MAPPED_OES, "GL enums should be equal.");
            if (context->getClientMajorVersion() < 3 && !extensions.mapbufferOES &&
                !extensions.mapBufferRangeEXT)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        case GL_BUFFER_MAP_POINTER:
            if (!pointerVersion)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidMapPointerQuery);
                return false;
            }
            break;

        case GL_BUFFER_ACCESS_FLAGS:
        case GL_BUFFER_MAP_OFFSET:
        case GL_BUFFER_MAP_LENGTH:
            if (context->getClientMajorVersion() < 3 && !extensions.mapBufferRangeEXT)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        case GL_MEMORY_SIZE_ANGLE:
            if (!context->getExtensions().memorySizeANGLE)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kExtensionNotEnabled);
                return false;
            }
            break;

        case GL_RESOURCE_INITIALIZED_ANGLE:
            if (!context->getExtensions().robustResourceInitializationANGLE)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM,
                                       kRobustResourceInitializationExtensionRequired);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
            return false;
    }

    // All buffer parameter queries return one value.
    if (numParams)
    {
        *numParams = 1;
    }

    return true;
}

bool ValidateGetRenderbufferParameterivBase(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            GLenum target,
                                            GLenum pname,
                                            GLsizei *length)
{
    if (length)
    {
        *length = 0;
    }

    if (target != GL_RENDERBUFFER)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidRenderbufferTarget);
        return false;
    }

    Renderbuffer *renderbuffer = context->getState().getCurrentRenderbuffer();
    if (renderbuffer == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kRenderbufferNotBound);
        return false;
    }

    switch (pname)
    {
        case GL_RENDERBUFFER_WIDTH:
        case GL_RENDERBUFFER_HEIGHT:
        case GL_RENDERBUFFER_INTERNAL_FORMAT:
        case GL_RENDERBUFFER_RED_SIZE:
        case GL_RENDERBUFFER_GREEN_SIZE:
        case GL_RENDERBUFFER_BLUE_SIZE:
        case GL_RENDERBUFFER_ALPHA_SIZE:
        case GL_RENDERBUFFER_DEPTH_SIZE:
        case GL_RENDERBUFFER_STENCIL_SIZE:
            break;

        case GL_RENDERBUFFER_SAMPLES_ANGLE:
            if (context->getClientMajorVersion() < 3 &&
                !context->getExtensions().framebufferMultisampleANGLE)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kExtensionNotEnabled);
                return false;
            }
            break;

        case GL_MEMORY_SIZE_ANGLE:
            if (!context->getExtensions().memorySizeANGLE)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kExtensionNotEnabled);
                return false;
            }
            break;

        case GL_IMPLEMENTATION_COLOR_READ_FORMAT:
        case GL_IMPLEMENTATION_COLOR_READ_TYPE:
            if (!context->getExtensions().getImageANGLE)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kGetImageExtensionNotEnabled);
                return false;
            }
            break;

        case GL_RESOURCE_INITIALIZED_ANGLE:
            if (!context->getExtensions().robustResourceInitializationANGLE)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM,
                                       kRobustResourceInitializationExtensionRequired);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
            return false;
    }

    if (length)
    {
        *length = 1;
    }
    return true;
}

bool ValidateGetShaderivBase(const Context *context,
                             angle::EntryPoint entryPoint,
                             ShaderProgramID shader,
                             GLenum pname,
                             GLsizei *length)
{
    if (length)
    {
        *length = 0;
    }

    if (context->isContextLost())
    {
        ANGLE_VALIDATION_ERROR(GL_CONTEXT_LOST, kContextLost);

        if (context->getExtensions().parallelShaderCompileKHR && pname == GL_COMPLETION_STATUS_KHR)
        {
            // Generate an error but still return true, the context still needs to return a
            // value in this case.
            return true;
        }
        else
        {
            return false;
        }
    }

    if (GetValidShader(context, entryPoint, shader) == nullptr)
    {
        return false;
    }

    switch (pname)
    {
        case GL_SHADER_TYPE:
        case GL_DELETE_STATUS:
        case GL_COMPILE_STATUS:
        case GL_INFO_LOG_LENGTH:
        case GL_SHADER_SOURCE_LENGTH:
            break;

        case GL_TRANSLATED_SHADER_SOURCE_LENGTH_ANGLE:
            if (!context->getExtensions().translatedShaderSourceANGLE)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kExtensionNotEnabled);
                return false;
            }
            break;

        case GL_COMPLETION_STATUS_KHR:
            if (!context->getExtensions().parallelShaderCompileKHR)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
            return false;
    }

    if (length)
    {
        *length = 1;
    }
    return true;
}

bool ValidateGetTexParameterBase(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 TextureType target,
                                 GLenum pname,
                                 GLsizei *length)
{
    if (length)
    {
        *length = 0;
    }

    if ((!ValidTextureTarget(context, target) && !ValidTextureExternalTarget(context, target)) ||
        target == TextureType::Buffer)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
        return false;
    }

    if (context->getTextureByType(target) == nullptr)
    {
        // Should only be possible for external textures
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kTextureNotBound);
        return false;
    }

    if (context->getClientMajorVersion() == 1 && !IsValidGLES1TextureParameter(pname))
    {
        ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
        return false;
    }

    switch (pname)
    {
        case GL_TEXTURE_MAG_FILTER:
        case GL_TEXTURE_MIN_FILTER:
        case GL_TEXTURE_WRAP_S:
        case GL_TEXTURE_WRAP_T:
            break;

        case GL_TEXTURE_USAGE_ANGLE:
            if (!context->getExtensions().textureUsageANGLE)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        case GL_TEXTURE_MAX_ANISOTROPY_EXT:
            if (!ValidateTextureMaxAnisotropyExtensionEnabled(context, entryPoint))
            {
                return false;
            }
            break;

        case GL_TEXTURE_IMMUTABLE_FORMAT:
            if (context->getClientMajorVersion() < 3 && !context->getExtensions().textureStorageEXT)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

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
            if (context->getClientMajorVersion() < 3)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kEnumRequiresGLES30);
                return false;
            }
            break;

        case GL_TEXTURE_COMPARE_MODE:
        case GL_TEXTURE_COMPARE_FUNC:
            if (context->getClientMajorVersion() < 3 && !context->getExtensions().shadowSamplersEXT)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        case GL_TEXTURE_SRGB_DECODE_EXT:
            if (!context->getExtensions().textureSRGBDecodeEXT)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        case GL_DEPTH_STENCIL_TEXTURE_MODE:
            if (context->getClientVersion() < ES_3_1 &&
                !context->getExtensions().stencilTexturingANGLE)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        case GL_IMAGE_FORMAT_COMPATIBILITY_TYPE:
            if (context->getClientVersion() < ES_3_1)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kEnumRequiresGLES31);
                return false;
            }
            break;

        case GL_GENERATE_MIPMAP:
        case GL_TEXTURE_CROP_RECT_OES:
            // TODO(lfy@google.com): Restrict to GL_OES_draw_texture
            // after GL_OES_draw_texture functionality implemented
            if (context->getClientMajorVersion() > 1)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kGLES1Only);
                return false;
            }
            break;

        case GL_MEMORY_SIZE_ANGLE:
            if (!context->getExtensions().memorySizeANGLE)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        case GL_TEXTURE_BORDER_COLOR:
            if (!context->getExtensions().textureBorderClampAny() &&
                context->getClientVersion() < ES_3_2)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kExtensionNotEnabled);
                return false;
            }
            break;

        case GL_TEXTURE_NATIVE_ID_ANGLE:
            if (!context->getExtensions().textureExternalUpdateANGLE)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kExtensionNotEnabled);
                return false;
            }
            break;

        case GL_IMPLEMENTATION_COLOR_READ_FORMAT:
        case GL_IMPLEMENTATION_COLOR_READ_TYPE:
            if (!context->getExtensions().getImageANGLE)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kGetImageExtensionNotEnabled);
                return false;
            }
            break;

        case GL_RESOURCE_INITIALIZED_ANGLE:
            if (!context->getExtensions().robustResourceInitializationANGLE)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM,
                                       kRobustResourceInitializationExtensionRequired);
                return false;
            }
            break;

        case GL_TEXTURE_PROTECTED_EXT:
            if (!context->getExtensions().protectedTexturesEXT)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kProtectedTexturesExtensionRequired);
                return false;
            }
            break;

        case GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES:
            break;

        case GL_TEXTURE_FOVEATED_FEATURE_QUERY_QCOM:
            if (!context->getExtensions().textureFoveatedQCOM)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kFoveatedTextureQcomExtensionRequired);
                return false;
            }
            break;

        case GL_TEXTURE_FOVEATED_NUM_FOCAL_POINTS_QUERY_QCOM:
            if (!context->getExtensions().textureFoveatedQCOM)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kFoveatedTextureQcomExtensionRequired);
                return false;
            }
            break;

        case GL_SURFACE_COMPRESSION_EXT:
            if (!context->getExtensions().textureStorageCompressionEXT)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM,
                                       kTextureStorageCompressionExtensionRequired);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
            return false;
    }

    if (length)
    {
        *length = GetTexParameterCount(pname);
    }
    return true;
}

bool ValidateGetVertexAttribBase(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 GLuint index,
                                 GLenum pname,
                                 GLsizei *length,
                                 bool pointer,
                                 bool pureIntegerEntryPoint)
{
    if (length)
    {
        *length = 0;
    }

    if (pureIntegerEntryPoint && context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (index >= static_cast<GLuint>(context->getCaps().maxVertexAttributes))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxVertexAttribute);
        return false;
    }

    if (pointer)
    {
        if (pname != GL_VERTEX_ATTRIB_ARRAY_POINTER)
        {
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
            return false;
        }
    }
    else
    {
        switch (pname)
        {
            case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
            case GL_VERTEX_ATTRIB_ARRAY_SIZE:
            case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
            case GL_VERTEX_ATTRIB_ARRAY_TYPE:
            case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
            case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
            case GL_CURRENT_VERTEX_ATTRIB:
                break;

            case GL_VERTEX_ATTRIB_ARRAY_DIVISOR:
                static_assert(
                    GL_VERTEX_ATTRIB_ARRAY_DIVISOR == GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ANGLE,
                    "ANGLE extension enums not equal to GL enums.");
                if (context->getClientMajorVersion() < 3 &&
                    !context->getExtensions().instancedArraysAny())
                {
                    ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                    return false;
                }
                break;

            case GL_VERTEX_ATTRIB_ARRAY_INTEGER:
                if (context->getClientMajorVersion() < 3)
                {
                    ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                    return false;
                }
                break;

            case GL_VERTEX_ATTRIB_BINDING:
            case GL_VERTEX_ATTRIB_RELATIVE_OFFSET:
                if (context->getClientVersion() < ES_3_1)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kEnumRequiresGLES31);
                    return false;
                }
                break;

            default:
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
        }
    }

    if (length)
    {
        if (pname == GL_CURRENT_VERTEX_ATTRIB)
        {
            *length = 4;
        }
        else
        {
            *length = 1;
        }
    }

    return true;
}

bool ValidatePixelPack(const Context *context,
                       angle::EntryPoint entryPoint,
                       GLenum format,
                       GLenum type,
                       GLint x,
                       GLint y,
                       GLsizei width,
                       GLsizei height,
                       GLsizei bufSize,
                       GLsizei *length,
                       const void *pixels)
{
    // Check for pixel pack buffer related API errors
    Buffer *pixelPackBuffer = context->getState().getTargetBuffer(BufferBinding::PixelPack);
    if (pixelPackBuffer != nullptr && pixelPackBuffer->isMapped())
    {
        // ...the buffer object's data store is currently mapped.
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferMapped);
        return false;
    }
    if (pixelPackBuffer != nullptr &&
        pixelPackBuffer->hasWebGLXFBBindingConflict(context->isWebGL()))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kPixelPackBufferBoundForTransformFeedback);
        return false;
    }

    // ..  the data would be packed to the buffer object such that the memory writes required
    // would exceed the data store size.
    const InternalFormat &formatInfo = GetInternalFormatInfo(format, type);
    const Extents size(width, height, 1);
    const auto &pack = context->getState().getPackState();

    GLuint endByte = 0;
    if (!formatInfo.computePackUnpackEndByte(type, size, pack, false, &endByte))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kIntegerOverflow);
        return false;
    }

    if (bufSize >= 0)
    {
        if (pixelPackBuffer == nullptr && static_cast<size_t>(bufSize) < endByte)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInsufficientBufferSize);
            return false;
        }
    }

    if (pixelPackBuffer != nullptr)
    {
        CheckedNumeric<size_t> checkedEndByte(endByte);
        CheckedNumeric<size_t> checkedOffset(reinterpret_cast<size_t>(pixels));
        checkedEndByte += checkedOffset;

        if (checkedEndByte.ValueOrDie() > static_cast<size_t>(pixelPackBuffer->getSize()))
        {
            // Overflow past the end of the buffer
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kParamOverflow);
            return false;
        }

        const auto &typeInfo = GetTypeInfo(type);
        if (reinterpret_cast<uintptr_t>(pixels) % typeInfo.bytes != 0)
        {
            // data is not evenly divisible by the number of basic machine units needed
            // to store in memory the corresponding GL data type from table 8.4 for the
            // type parameter.
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferOffsetNotAligned);
            return false;
        }
    }

    if (pixelPackBuffer == nullptr && length != nullptr)
    {
        if (endByte > static_cast<size_t>(std::numeric_limits<GLsizei>::max()))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kIntegerOverflow);
            return false;
        }

        *length = static_cast<GLsizei>(endByte);
    }

    if (context->isWebGL())
    {
        // WebGL 2.0 disallows the scenario:
        //   GL_PACK_SKIP_PIXELS + width > DataStoreWidth
        // where:
        //   DataStoreWidth = (GL_PACK_ROW_LENGTH ? GL_PACK_ROW_LENGTH : width)
        // Since these two pack parameters can only be set to non-zero values
        // on WebGL 2.0 contexts, verify them for all WebGL contexts.
        GLint dataStoreWidth = pack.rowLength ? pack.rowLength : width;
        if (pack.skipPixels + width > dataStoreWidth)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidPackParametersForWebGL);
            return false;
        }
    }

    return true;
}

bool ValidateReadPixelsBase(const Context *context,
                            angle::EntryPoint entryPoint,
                            GLint x,
                            GLint y,
                            GLsizei width,
                            GLsizei height,
                            GLenum format,
                            GLenum type,
                            GLsizei bufSize,
                            GLsizei *length,
                            GLsizei *columns,
                            GLsizei *rows,
                            const void *pixels)
{
    if (length != nullptr)
    {
        *length = 0;
    }
    if (rows != nullptr)
    {
        *rows = 0;
    }
    if (columns != nullptr)
    {
        *columns = 0;
    }

    if (width < 0 || height < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeSize);
        return false;
    }

    Framebuffer *readFramebuffer = context->getState().getReadFramebuffer();
    ASSERT(readFramebuffer);

    if (!ValidateFramebufferComplete(context, entryPoint, readFramebuffer))
    {
        return false;
    }

    // needIntrinsic = true. Treat renderToTexture textures as single sample since they will be
    // resolved before reading.
    if (!readFramebuffer->isDefault() &&
        !ValidateFramebufferNotMultisampled(context, entryPoint, readFramebuffer, true))
    {
        return false;
    }

    if (readFramebuffer->getReadBufferState() == GL_NONE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kReadBufferNone);
        return false;
    }

    const FramebufferAttachment *readBuffer = nullptr;
    switch (format)
    {
        case GL_DEPTH_COMPONENT:
            readBuffer = readFramebuffer->getDepthAttachment();
            break;
        case GL_STENCIL_INDEX_OES:
        case GL_DEPTH_STENCIL_OES:
            readBuffer = readFramebuffer->getStencilOrDepthStencilAttachment();
            break;
        default:
            readBuffer = readFramebuffer->getReadColorAttachment();
            break;
    }

    // OVR_multiview2, Revision 1:
    // ReadPixels generates an INVALID_FRAMEBUFFER_OPERATION error if
    // the number of views in the current read framebuffer is more than one.
    if (readFramebuffer->readDisallowedByMultiview())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION, kMultiviewReadFramebuffer);
        return false;
    }

    if (context->isWebGL())
    {
        // The ES 2.0 spec states that the format must be "among those defined in table 3.4,
        // excluding formats LUMINANCE and LUMINANCE_ALPHA.".  This requires validating the format
        // and type before validating the combination of format and type.  However, the
        // dEQP-GLES3.functional.negative_api.buffer.read_pixels passes GL_LUMINANCE as a format and
        // verifies that GL_INVALID_OPERATION is generated.
        // TODO(geofflang): Update this check to be done in all/no cases once this is resolved in
        // dEQP/WebGL.
        if (!ValidReadPixelsFormatEnum(context, format))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFormat);
            return false;
        }

        if (!ValidReadPixelsTypeEnum(context, type))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidType);
            return false;
        }
    }

    // WebGL 1.0 [Section 6.26] Reading From a Missing Attachment
    // In OpenGL ES it is undefined what happens when an operation tries to read from a missing
    // attachment and WebGL defines it to be an error. We do the check unconditionally as the
    // situation is an application error that would lead to a crash in ANGLE.
    if (readBuffer == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMissingReadAttachment);
        return false;
    }

    GLenum currentFormat = GL_NONE;
    GLenum currentType   = GL_NONE;

    switch (format)
    {
        case GL_DEPTH_COMPONENT:
        case GL_STENCIL_INDEX_OES:
        case GL_DEPTH_STENCIL_OES:
            // Only rely on ValidReadPixelsFormatType for depth/stencil formats
            break;
        default:
            currentFormat = readFramebuffer->getImplementationColorReadFormat(context);
            currentType   = readFramebuffer->getImplementationColorReadType(context);
            break;
    }

    bool validFormatTypeCombination =
        ValidReadPixelsFormatType(context, readBuffer->getFormat().info, format, type);

    if (!(currentFormat == format && currentType == type) && !validFormatTypeCombination)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedTypeAndFormat);
        return false;
    }

    if (!ValidatePixelPack(context, entryPoint, format, type, x, y, width, height, bufSize, length,
                           pixels))
    {
        return false;
    }

    auto getClippedExtent = [](GLint start, GLsizei length, int bufferSize, GLsizei *outExtent) {
        angle::CheckedNumeric<int> clippedExtent(length);
        if (start < 0)
        {
            // "subtract" the area that is less than 0
            clippedExtent += start;
        }

        angle::CheckedNumeric<int> readExtent = start;
        readExtent += length;
        if (!readExtent.IsValid())
        {
            return false;
        }

        if (readExtent.ValueOrDie() > bufferSize)
        {
            // Subtract the region to the right of the read buffer
            clippedExtent -= (readExtent - bufferSize);
        }

        if (!clippedExtent.IsValid())
        {
            return false;
        }

        *outExtent = std::max<int>(clippedExtent.ValueOrDie(), 0);
        return true;
    };

    GLsizei writtenColumns = 0;
    if (!getClippedExtent(x, width, readBuffer->getSize().width, &writtenColumns))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kIntegerOverflow);
        return false;
    }

    GLsizei writtenRows = 0;
    if (!getClippedExtent(y, height, readBuffer->getSize().height, &writtenRows))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kIntegerOverflow);
        return false;
    }

    if (columns != nullptr)
    {
        *columns = writtenColumns;
    }

    if (rows != nullptr)
    {
        *rows = writtenRows;
    }

    return true;
}

template <typename ParamType>
bool ValidateTexParameterBase(const Context *context,
                              angle::EntryPoint entryPoint,
                              TextureType target,
                              GLenum pname,
                              GLsizei bufSize,
                              bool vectorParams,
                              const ParamType *params)
{
    if ((!ValidTextureTarget(context, target) && !ValidTextureExternalTarget(context, target)) ||
        target == TextureType::Buffer)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
        return false;
    }

    if (context->getTextureByType(target) == nullptr)
    {
        // Should only be possible for external textures
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kTextureNotBound);
        return false;
    }

    const GLsizei minBufSize = GetTexParameterCount(pname);
    if (bufSize >= 0 && bufSize < minBufSize)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInsufficientBufferSize);
        return false;
    }

    if (context->getClientMajorVersion() == 1 && !IsValidGLES1TextureParameter(pname))
    {
        ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
        return false;
    }

    switch (pname)
    {
        case GL_TEXTURE_WRAP_R:
        case GL_TEXTURE_SWIZZLE_R:
        case GL_TEXTURE_SWIZZLE_G:
        case GL_TEXTURE_SWIZZLE_B:
        case GL_TEXTURE_SWIZZLE_A:
        case GL_TEXTURE_BASE_LEVEL:
        case GL_TEXTURE_MAX_LEVEL:
        case GL_TEXTURE_COMPARE_MODE:
        case GL_TEXTURE_COMPARE_FUNC:
        case GL_TEXTURE_MIN_LOD:
        case GL_TEXTURE_MAX_LOD:
            if (context->getClientMajorVersion() < 3 &&
                !(pname == GL_TEXTURE_WRAP_R && context->getExtensions().texture3DOES))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kES3Required);
                return false;
            }
            if (target == TextureType::VideoImage && !context->getExtensions().videoTextureWEBGL)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
            }
            break;

        case GL_GENERATE_MIPMAP:
        case GL_TEXTURE_CROP_RECT_OES:
            if (context->getClientMajorVersion() > 1)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kGLES1Only);
                return false;
            }
            break;

        default:
            break;
    }

    if (target == TextureType::_2DMultisample || target == TextureType::_2DMultisampleArray)
    {
        switch (pname)
        {
            case GL_TEXTURE_MIN_FILTER:
            case GL_TEXTURE_MAG_FILTER:
            case GL_TEXTURE_WRAP_S:
            case GL_TEXTURE_WRAP_T:
            case GL_TEXTURE_WRAP_R:
            case GL_TEXTURE_MIN_LOD:
            case GL_TEXTURE_MAX_LOD:
            case GL_TEXTURE_COMPARE_MODE:
            case GL_TEXTURE_COMPARE_FUNC:
            case GL_TEXTURE_BORDER_COLOR:
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidPname);
                return false;
        }
    }

    switch (pname)
    {
        case GL_TEXTURE_WRAP_S:
        case GL_TEXTURE_WRAP_T:
        case GL_TEXTURE_WRAP_R:
        {
            bool restrictedWrapModes = ((target == TextureType::External &&
                                         !context->getExtensions().EGLImageExternalWrapModesEXT) ||
                                        target == TextureType::Rectangle);
            if (!ValidateTextureWrapModeValue(context, entryPoint, params, restrictedWrapModes))
            {
                return false;
            }
        }
        break;

        case GL_TEXTURE_MIN_FILTER:
        {
            bool restrictedMinFilter =
                target == TextureType::External || target == TextureType::Rectangle;
            if (!ValidateTextureMinFilterValue(context, entryPoint, params, restrictedMinFilter))
            {
                return false;
            }
        }
        break;

        case GL_TEXTURE_MAG_FILTER:
            if (!ValidateTextureMagFilterValue(context, entryPoint, params))
            {
                return false;
            }
            break;

        case GL_TEXTURE_USAGE_ANGLE:
            if (!context->getExtensions().textureUsageANGLE)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }

            switch (ConvertToGLenum(params[0]))
            {
                case GL_NONE:
                case GL_FRAMEBUFFER_ATTACHMENT_ANGLE:
                    break;

                default:
                    ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                    return false;
            }
            break;

        case GL_TEXTURE_MAX_ANISOTROPY_EXT:
        {
            GLfloat paramValue = static_cast<GLfloat>(params[0]);
            if (!ValidateTextureMaxAnisotropyValue(context, entryPoint, paramValue))
            {
                return false;
            }
            ASSERT(static_cast<ParamType>(paramValue) == params[0]);
        }
        break;

        case GL_TEXTURE_MIN_LOD:
        case GL_TEXTURE_MAX_LOD:
            // any value is permissible
            break;

        case GL_TEXTURE_COMPARE_MODE:
            if (!ValidateTextureCompareModeValue(context, entryPoint, params))
            {
                return false;
            }
            break;

        case GL_TEXTURE_COMPARE_FUNC:
            if (!ValidateTextureCompareFuncValue(context, entryPoint, params))
            {
                return false;
            }
            break;

        case GL_TEXTURE_SWIZZLE_R:
        case GL_TEXTURE_SWIZZLE_G:
        case GL_TEXTURE_SWIZZLE_B:
        case GL_TEXTURE_SWIZZLE_A:
            switch (ConvertToGLenum(params[0]))
            {
                case GL_RED:
                case GL_GREEN:
                case GL_BLUE:
                case GL_ALPHA:
                case GL_ZERO:
                case GL_ONE:
                    break;

                default:
                    ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                    return false;
            }
            break;

        case GL_TEXTURE_BASE_LEVEL:
            if (ConvertToGLint(params[0]) < 0)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kBaseLevelNegative);
                return false;
            }
            if (target == TextureType::External && static_cast<GLuint>(params[0]) != 0)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBaseLevelNonZero);
                return false;
            }
            if ((target == TextureType::_2DMultisample ||
                 target == TextureType::_2DMultisampleArray) &&
                static_cast<GLuint>(params[0]) != 0)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBaseLevelNonZero);
                return false;
            }
            if (target == TextureType::Rectangle && static_cast<GLuint>(params[0]) != 0)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBaseLevelNonZero);
                return false;
            }
            break;

        case GL_TEXTURE_MAX_LEVEL:
            if (ConvertToGLint(params[0]) < 0)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevel);
                return false;
            }
            break;

        case GL_DEPTH_STENCIL_TEXTURE_MODE:
            if (context->getClientVersion() < ES_3_1 &&
                !context->getExtensions().stencilTexturingANGLE)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            switch (ConvertToGLenum(params[0]))
            {
                case GL_DEPTH_COMPONENT:
                case GL_STENCIL_INDEX:
                    break;

                default:
                    ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                    return false;
            }
            break;

        case GL_TEXTURE_SRGB_DECODE_EXT:
            if (!ValidateTextureSRGBDecodeValue(context, entryPoint, params))
            {
                return false;
            }
            break;

        case GL_TEXTURE_FORMAT_SRGB_OVERRIDE_EXT:
            if (!ValidateTextureSRGBOverrideValue(context, entryPoint, params))
            {
                return false;
            }
            break;

        case GL_GENERATE_MIPMAP:
            if (context->getClientMajorVersion() > 1)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kGLES1Only);
                return false;
            }
            break;

        case GL_TEXTURE_CROP_RECT_OES:
            if (context->getClientMajorVersion() > 1)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kGLES1Only);
                return false;
            }
            if (!vectorParams)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInsufficientBufferSize);
                return false;
            }
            break;

        case GL_TEXTURE_BORDER_COLOR:
            if (!context->getExtensions().textureBorderClampAny() &&
                context->getClientVersion() < ES_3_2)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kExtensionNotEnabled);
                return false;
            }
            if (!vectorParams)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInsufficientBufferSize);
                return false;
            }
            break;

        case GL_RESOURCE_INITIALIZED_ANGLE:
            if (!context->getExtensions().robustResourceInitializationANGLE)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM,
                                       kRobustResourceInitializationExtensionRequired);
                return false;
            }
            break;

        case GL_TEXTURE_PROTECTED_EXT:
            if (!context->getExtensions().protectedTexturesEXT)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kProtectedTexturesExtensionRequired);
                return false;
            }
            if (ConvertToBool(params[0]) != context->getState().hasProtectedContent())
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                       "Protected Texture must match Protected Context");
                return false;
            }
            break;

        case GL_RENDERABILITY_VALIDATION_ANGLE:
            if (!context->getExtensions().renderabilityValidationANGLE)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        case GL_TEXTURE_TILING_EXT:
            if (!context->getExtensions().memoryObjectEXT)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidMemoryObjectParameter);
                return false;
            }
            switch (ConvertToGLenum(params[0]))
            {
                case GL_OPTIMAL_TILING_EXT:
                case GL_LINEAR_TILING_EXT:
                    break;

                default:
                    ANGLE_VALIDATION_ERROR(
                        GL_INVALID_OPERATION,
                        "Texture Tilling Mode must be OPTIMAL_TILING_EXT or LINEAR_TILING_EXT");
                    return false;
            }
            break;

        case GL_TEXTURE_FOVEATED_FEATURE_BITS_QCOM:
            if (!context->getExtensions().textureFoveatedQCOM)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kFoveatedTextureQcomExtensionRequired);
                return false;
            }
            {
                const GLuint features               = static_cast<GLuint>(params[0]);
                constexpr GLuint kSupportedFeatures = GL_FOVEATION_ENABLE_BIT_QCOM;
                if (features != (features & kSupportedFeatures))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kFoveatedTextureInvalidParameters);
                    return false;
                }
                if ((context->getTextureByType(target)->getFoveatedFeatureBits() &
                     GL_FOVEATION_ENABLE_BIT_QCOM) &&
                    (features & GL_FOVEATION_ENABLE_BIT_QCOM) == 0)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kFoveatedTextureCannotDisable);
                    return false;
                }
            }
            break;

        case GL_TEXTURE_FOVEATED_MIN_PIXEL_DENSITY_QCOM:
            if (!context->getExtensions().textureFoveatedQCOM)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kFoveatedTextureQcomExtensionRequired);
                return false;
            }
            if (static_cast<GLfloat>(params[0]) < 0.0 || static_cast<GLfloat>(params[0]) > 1.0)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kFoveatedTextureInvalidPixelDensity);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
            return false;
    }

    return true;
}

template bool ValidateTexParameterBase(const Context *,
                                       angle::EntryPoint,
                                       TextureType,
                                       GLenum,
                                       GLsizei,
                                       bool,
                                       const GLfloat *);
template bool ValidateTexParameterBase(const Context *,
                                       angle::EntryPoint,
                                       TextureType,
                                       GLenum,
                                       GLsizei,
                                       bool,
                                       const GLint *);
template bool ValidateTexParameterBase(const Context *,
                                       angle::EntryPoint,
                                       TextureType,
                                       GLenum,
                                       GLsizei,
                                       bool,
                                       const GLuint *);

bool ValidateGetActiveUniformBlockivBase(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         ShaderProgramID program,
                                         UniformBlockIndex uniformBlockIndex,
                                         GLenum pname,
                                         GLsizei *length)
{
    if (length)
    {
        *length = 0;
    }

    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (!programObject)
    {
        return false;
    }

    const ProgramExecutable &executable = programObject->getExecutable();
    if (uniformBlockIndex.value >= executable.getUniformBlocks().size())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsActiveUniformBlockCount);
        return false;
    }

    switch (pname)
    {
        case GL_UNIFORM_BLOCK_BINDING:
        case GL_UNIFORM_BLOCK_DATA_SIZE:
        case GL_UNIFORM_BLOCK_NAME_LENGTH:
        case GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS:
        case GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES:
        case GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER:
        case GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER:
            break;

        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
            return false;
    }

    if (length)
    {
        if (pname == GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES)
        {
            const InterfaceBlock &uniformBlock =
                executable.getUniformBlockByIndex(uniformBlockIndex.value);
            *length = static_cast<GLsizei>(uniformBlock.memberIndexes.size());
        }
        else
        {
            *length = 1;
        }
    }

    return true;
}

template <typename ParamType>
bool ValidateSamplerParameterBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  SamplerID sampler,
                                  GLenum pname,
                                  GLsizei bufSize,
                                  bool vectorParams,
                                  const ParamType *params)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (!context->isSampler(sampler))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidSampler);
        return false;
    }

    const GLsizei minBufSize = GetSamplerParameterCount(pname);
    if (bufSize >= 0 && bufSize < minBufSize)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInsufficientBufferSize);
        return false;
    }

    switch (pname)
    {
        case GL_TEXTURE_WRAP_S:
        case GL_TEXTURE_WRAP_T:
        case GL_TEXTURE_WRAP_R:
            if (!ValidateTextureWrapModeValue(context, entryPoint, params, false))
            {
                return false;
            }
            break;

        case GL_TEXTURE_MIN_FILTER:
            if (!ValidateTextureMinFilterValue(context, entryPoint, params, false))
            {
                return false;
            }
            break;

        case GL_TEXTURE_MAG_FILTER:
            if (!ValidateTextureMagFilterValue(context, entryPoint, params))
            {
                return false;
            }
            break;

        case GL_TEXTURE_MIN_LOD:
        case GL_TEXTURE_MAX_LOD:
            // any value is permissible
            break;

        case GL_TEXTURE_COMPARE_MODE:
            if (!ValidateTextureCompareModeValue(context, entryPoint, params))
            {
                return false;
            }
            break;

        case GL_TEXTURE_COMPARE_FUNC:
            if (!ValidateTextureCompareFuncValue(context, entryPoint, params))
            {
                return false;
            }
            break;

        case GL_TEXTURE_SRGB_DECODE_EXT:
            if (!ValidateTextureSRGBDecodeValue(context, entryPoint, params))
            {
                return false;
            }
            break;

        case GL_TEXTURE_MAX_ANISOTROPY_EXT:
        {
            GLfloat paramValue = static_cast<GLfloat>(params[0]);
            if (!ValidateTextureMaxAnisotropyValue(context, entryPoint, paramValue))
            {
                return false;
            }
        }
        break;

        case GL_TEXTURE_BORDER_COLOR:
            if (!context->getExtensions().textureBorderClampAny() &&
                context->getClientVersion() < ES_3_2)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kExtensionNotEnabled);
                return false;
            }
            if (!vectorParams)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInsufficientBufferSize);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
            return false;
    }

    return true;
}

template bool ValidateSamplerParameterBase(const Context *,
                                           angle::EntryPoint,
                                           SamplerID,
                                           GLenum,
                                           GLsizei,
                                           bool,
                                           const GLfloat *);
template bool ValidateSamplerParameterBase(const Context *,
                                           angle::EntryPoint,
                                           SamplerID,
                                           GLenum,
                                           GLsizei,
                                           bool,
                                           const GLint *);
template bool ValidateSamplerParameterBase(const Context *,
                                           angle::EntryPoint,
                                           SamplerID,
                                           GLenum,
                                           GLsizei,
                                           bool,
                                           const GLuint *);

bool ValidateGetSamplerParameterBase(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     SamplerID sampler,
                                     GLenum pname,
                                     GLsizei *length)
{
    if (length)
    {
        *length = 0;
    }

    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (!context->isSampler(sampler))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidSampler);
        return false;
    }

    switch (pname)
    {
        case GL_TEXTURE_WRAP_S:
        case GL_TEXTURE_WRAP_T:
        case GL_TEXTURE_WRAP_R:
        case GL_TEXTURE_MIN_FILTER:
        case GL_TEXTURE_MAG_FILTER:
        case GL_TEXTURE_MIN_LOD:
        case GL_TEXTURE_MAX_LOD:
        case GL_TEXTURE_COMPARE_MODE:
        case GL_TEXTURE_COMPARE_FUNC:
            break;

        case GL_TEXTURE_MAX_ANISOTROPY_EXT:
            if (!ValidateTextureMaxAnisotropyExtensionEnabled(context, entryPoint))
            {
                return false;
            }
            break;

        case GL_TEXTURE_SRGB_DECODE_EXT:
            if (!context->getExtensions().textureSRGBDecodeEXT)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;

        case GL_TEXTURE_BORDER_COLOR:
            if (!context->getExtensions().textureBorderClampAny() &&
                context->getClientVersion() < ES_3_2)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kExtensionNotEnabled);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
            return false;
    }

    if (length)
    {
        *length = GetSamplerParameterCount(pname);
    }
    return true;
}

bool ValidateGetInternalFormativBase(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     GLenum target,
                                     GLenum internalformat,
                                     GLenum pname,
                                     GLsizei bufSize,
                                     GLsizei *numParams)
{
    if (numParams)
    {
        *numParams = 0;
    }

    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    const TextureCaps &formatCaps = context->getTextureCaps().get(internalformat);
    if (!formatCaps.renderbuffer)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kFormatNotRenderable);
        return false;
    }

    switch (target)
    {
        case GL_RENDERBUFFER:
            break;

        case GL_TEXTURE_2D_MULTISAMPLE:
            if (context->getClientVersion() < ES_3_1 &&
                !context->getExtensions().textureMultisampleANGLE)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kMultisampleTextureExtensionOrES31Required);
                return false;
            }
            break;
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
            if (context->getClientVersion() < ES_3_2 &&
                !context->getExtensions().textureStorageMultisample2dArrayOES)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kMultisampleArrayExtensionOrES32Required);
                return false;
            }
            break;
        case GL_TEXTURE_2D:
        case GL_TEXTURE_CUBE_MAP:
        case GL_TEXTURE_3D:
        case GL_TEXTURE_2D_ARRAY:
            if (pname != GL_NUM_SURFACE_COMPRESSION_FIXED_RATES_EXT &&
                pname != GL_SURFACE_COMPRESSION_EXT)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTarget);
                return false;
            }
            if (!context->getExtensions().textureStorageCompressionEXT)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM,
                                       kTextureStorageCompressionExtensionRequired);
                return false;
            }
            break;
        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTarget);
            return false;
    }

    if (bufSize < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInsufficientBufferSize);
        return false;
    }

    GLsizei maxWriteParams = 0;
    switch (pname)
    {
        case GL_NUM_SAMPLE_COUNTS:
            maxWriteParams = 1;
            break;

        case GL_SAMPLES:
            maxWriteParams = static_cast<GLsizei>(formatCaps.sampleCounts.size());
            break;

        case GL_NUM_SURFACE_COMPRESSION_FIXED_RATES_EXT:
        case GL_SURFACE_COMPRESSION_EXT:
            if (!context->getExtensions().textureStorageCompressionEXT)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM,
                                       kTextureStorageCompressionExtensionRequired);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
            return false;
    }

    if (numParams)
    {
        // glGetInternalFormativ will not overflow bufSize
        *numParams = std::min(bufSize, maxWriteParams);
    }

    return true;
}

bool ValidateFramebufferNotMultisampled(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        const Framebuffer *framebuffer,
                                        bool checkReadBufferResourceSamples)
{
    int samples = checkReadBufferResourceSamples
                      ? framebuffer->getReadBufferResourceSamples(context)
                      : framebuffer->getSamples(context);
    if (samples != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidMultisampledFramebufferOperation);
        return false;
    }
    return true;
}

bool ValidateMultitextureUnit(const PrivateState &state,
                              ErrorSet *errors,
                              angle::EntryPoint entryPoint,
                              GLenum texture)
{
    if (texture < GL_TEXTURE0 || texture >= GL_TEXTURE0 + state.getCaps().maxMultitextureUnits)
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidMultitextureUnit);
        return false;
    }
    return true;
}

bool ValidateTexStorageMultisample(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   TextureType target,
                                   GLsizei samples,
                                   GLint internalFormat,
                                   GLsizei width,
                                   GLsizei height)
{
    const Caps &caps = context->getCaps();
    if (width > caps.max2DTextureSize || height > caps.max2DTextureSize)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kTextureWidthOrHeightOutOfRange);
        return false;
    }

    if (samples == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kSamplesZero);
        return false;
    }

    const TextureCaps &formatCaps = context->getTextureCaps().get(internalFormat);
    if (!formatCaps.textureAttachment)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kRenderableInternalFormat);
        return false;
    }

    // The ES3.1 spec(section 8.8) states that an INVALID_ENUM error is generated if internalformat
    // is one of the unsized base internalformats listed in table 8.11.
    const InternalFormat &formatInfo = GetSizedInternalFormatInfo(internalFormat);
    if (formatInfo.internalFormat == GL_NONE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kUnsizedInternalFormatUnsupported);
        return false;
    }

    if (static_cast<GLuint>(samples) > formatCaps.getMaxSamples())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kSamplesOutOfRange);
        return false;
    }

    Texture *texture = context->getTextureByType(target);
    if (!texture || texture->id().value == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kZeroBoundToTarget);
        return false;
    }

    if (texture->getImmutableFormat())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kImmutableTextureBound);
        return false;
    }
    return true;
}

bool ValidateTexStorage2DMultisampleBase(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         TextureType target,
                                         GLsizei samples,
                                         GLint internalFormat,
                                         GLsizei width,
                                         GLsizei height)
{
    if (target != TextureType::_2DMultisample)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTarget);
        return false;
    }

    if (width < 1 || height < 1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kTextureSizeTooSmall);
        return false;
    }

    return ValidateTexStorageMultisample(context, entryPoint, target, samples, internalFormat,
                                         width, height);
}

bool ValidateTexStorage3DMultisampleBase(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         TextureType target,
                                         GLsizei samples,
                                         GLenum internalformat,
                                         GLsizei width,
                                         GLsizei height,
                                         GLsizei depth)
{
    if (target != TextureType::_2DMultisampleArray)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTarget);
        return false;
    }

    if (width < 1 || height < 1 || depth < 1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kTextureSizeTooSmall);
        return false;
    }

    if (depth > context->getCaps().maxArrayTextureLayers)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kTextureDepthOutOfRange);
        return false;
    }

    return ValidateTexStorageMultisample(context, entryPoint, target, samples, internalformat,
                                         width, height);
}

bool ValidateGetTexLevelParameterBase(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      TextureTarget target,
                                      GLint level,
                                      GLenum pname,
                                      GLsizei *length)
{

    if (length)
    {
        *length = 0;
    }

    TextureType type = TextureTargetToType(target);

    if (!ValidTexLevelDestinationTarget(context, type))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
        return false;
    }

    // If type is valid, the texture object must exist
    ASSERT(context->getTextureByType(type) != nullptr);

    if (!ValidMipLevel(context, type, level))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevel);
        return false;
    }

    switch (pname)
    {
        case GL_TEXTURE_RED_TYPE:
        case GL_TEXTURE_GREEN_TYPE:
        case GL_TEXTURE_BLUE_TYPE:
        case GL_TEXTURE_ALPHA_TYPE:
        case GL_TEXTURE_DEPTH_TYPE:
        case GL_TEXTURE_RED_SIZE:
        case GL_TEXTURE_GREEN_SIZE:
        case GL_TEXTURE_BLUE_SIZE:
        case GL_TEXTURE_ALPHA_SIZE:
        case GL_TEXTURE_DEPTH_SIZE:
        case GL_TEXTURE_STENCIL_SIZE:
        case GL_TEXTURE_SHARED_SIZE:
        case GL_TEXTURE_INTERNAL_FORMAT:
        case GL_TEXTURE_WIDTH:
        case GL_TEXTURE_HEIGHT:
        case GL_TEXTURE_COMPRESSED:
            break;

        case GL_TEXTURE_DEPTH:
            if (context->getClientVersion() < ES_3_0 && !context->getExtensions().texture3DOES)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kEnumNotSupported);
                return false;
            }
            break;

        case GL_TEXTURE_SAMPLES:
        case GL_TEXTURE_FIXED_SAMPLE_LOCATIONS:
            if (context->getClientVersion() < ES_3_1 &&
                !context->getExtensions().textureMultisampleANGLE)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kMultisampleTextureExtensionOrES31Required);
                return false;
            }
            break;

        case GL_RESOURCE_INITIALIZED_ANGLE:
            if (!context->getExtensions().robustResourceInitializationANGLE)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM,
                                       kRobustResourceInitializationExtensionRequired);
                return false;
            }
            break;

        case GL_TEXTURE_BUFFER_DATA_STORE_BINDING:
        case GL_TEXTURE_BUFFER_OFFSET:
        case GL_TEXTURE_BUFFER_SIZE:
            if (context->getClientVersion() < Version(3, 2) &&
                !context->getExtensions().textureBufferAny())
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kTextureBufferExtensionNotAvailable);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidPname);
            return false;
    }

    if (length)
    {
        *length = 1;
    }
    return true;
}

bool ValidateGetMultisamplefvBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  GLenum pname,
                                  GLuint index,
                                  const GLfloat *val)
{
    if (pname != GL_SAMPLE_POSITION)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidPname);
        return false;
    }

    Framebuffer *framebuffer = context->getState().getDrawFramebuffer();
    GLint samples            = framebuffer->getSamples(context);

    if (index >= static_cast<GLuint>(samples))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsSamples);
        return false;
    }

    return true;
}

bool ValidateSampleMaskiBase(const PrivateState &state,
                             ErrorSet *errors,
                             angle::EntryPoint entryPoint,
                             GLuint maskNumber,
                             GLbitfield mask)
{
    if (maskNumber >= static_cast<GLuint>(state.getCaps().maxSampleMaskWords))
    {
        errors->validationError(entryPoint, GL_INVALID_VALUE, kInvalidSampleMaskNumber);
        return false;
    }

    return true;
}

void RecordDrawAttribsError(const Context *context, angle::EntryPoint entryPoint)
{
    // An overflow can happen when adding the offset. Check against a special constant.
    if (context->getStateCache().getNonInstancedVertexElementLimit() ==
            VertexAttribute::kIntegerOverflow ||
        context->getStateCache().getInstancedVertexElementLimit() ==
            VertexAttribute::kIntegerOverflow)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kIntegerOverflow);
    }
    else
    {
        // [OpenGL ES 3.0.2] section 2.9.4 page 40:
        // We can return INVALID_OPERATION if our buffer does not have enough backing data.
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInsufficientVertexBufferSize);
    }
}

bool ValidateLoseContextCHROMIUM(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 GraphicsResetStatus current,
                                 GraphicsResetStatus other)
{
    if (!context->getExtensions().loseContextCHROMIUM)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    switch (current)
    {
        case GraphicsResetStatus::GuiltyContextReset:
        case GraphicsResetStatus::InnocentContextReset:
        case GraphicsResetStatus::UnknownContextReset:
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidResetStatus);
    }

    switch (other)
    {
        case GraphicsResetStatus::GuiltyContextReset:
        case GraphicsResetStatus::InnocentContextReset:
        case GraphicsResetStatus::UnknownContextReset:
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidResetStatus);
    }

    return true;
}

// GL_ANGLE_texture_storage_external
bool ValidateTexImage2DExternalANGLE(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     TextureTarget target,
                                     GLint level,
                                     GLint internalformat,
                                     GLsizei width,
                                     GLsizei height,
                                     GLint border,
                                     GLenum format,
                                     GLenum type)
{
    if (!context->getExtensions().textureExternalUpdateANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (!ValidTexture2DDestinationTarget(context, target) &&
        !ValidTextureExternalTarget(context, target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
        return false;
    }

    if (context->getClientMajorVersion() <= 2)
    {
        if (!ValidateES2TexImageParametersBase(context, entryPoint, target, level, internalformat,
                                               false, false, 0, 0, width, height, border, format,
                                               type, -1, nullptr))
        {
            return false;
        }
    }
    else
    {
        if (!ValidateES3TexImageParametersBase(context, entryPoint, target, level, internalformat,
                                               false, false, 0, 0, 0, width, height, 1, border,
                                               format, type, -1, nullptr))
        {
            return false;
        }
    }

    return true;
}

bool ValidateInvalidateTextureANGLE(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    TextureType target)
{
    if (!context->getExtensions().textureExternalUpdateANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (!ValidTextureTarget(context, target) && !ValidTextureExternalTarget(context, target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
        return false;
    }

    return true;
}

bool ValidateProgramExecutableXFBBuffersPresent(const Context *context,
                                                const ProgramExecutable *programExecutable)
{
    size_t programXfbCount = programExecutable->getTransformFeedbackBufferCount();
    const TransformFeedback *transformFeedback = context->getState().getCurrentTransformFeedback();
    for (size_t programXfbIndex = 0; programXfbIndex < programXfbCount; ++programXfbIndex)
    {
        const OffsetBindingPointer<Buffer> &buffer =
            transformFeedback->getIndexedBuffer(programXfbIndex);
        if (!buffer.get())
        {
            return false;
        }
    }

    return true;
}

bool ValidateLogicOpCommon(const PrivateState &state,
                           ErrorSet *errors,
                           angle::EntryPoint entryPoint,
                           LogicalOperation opcodePacked)
{
    switch (opcodePacked)
    {
        case LogicalOperation::And:
        case LogicalOperation::AndInverted:
        case LogicalOperation::AndReverse:
        case LogicalOperation::Clear:
        case LogicalOperation::Copy:
        case LogicalOperation::CopyInverted:
        case LogicalOperation::Equiv:
        case LogicalOperation::Invert:
        case LogicalOperation::Nand:
        case LogicalOperation::Noop:
        case LogicalOperation::Nor:
        case LogicalOperation::Or:
        case LogicalOperation::OrInverted:
        case LogicalOperation::OrReverse:
        case LogicalOperation::Set:
        case LogicalOperation::Xor:
            return true;
        default:
            errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidLogicOp);
            return false;
    }
}
}  // namespace gl
