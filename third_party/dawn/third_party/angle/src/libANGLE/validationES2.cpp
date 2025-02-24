//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationES2.cpp: Validation functions for OpenGL ES 2.0 entry point parameters

#include "libANGLE/validationES2_autogen.h"

#include <cstdint>

#include "common/BinaryStream.h"
#include "common/angle_version_info.h"
#include "common/mathutil.h"
#include "common/string_utils.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/ErrorStrings.h"
#include "libANGLE/Fence.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/MemoryObject.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/Shader.h"
#include "libANGLE/Texture.h"
#include "libANGLE/Uniform.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/validationES.h"
#include "libANGLE/validationES2.h"
#include "libANGLE/validationES32.h"
#include "libANGLE/validationES3_autogen.h"

namespace gl
{
using namespace err;

namespace
{

bool IsPartialBlit(const Context *context,
                   const FramebufferAttachment *readBuffer,
                   const FramebufferAttachment *writeBuffer,
                   GLint srcX0,
                   GLint srcY0,
                   GLint srcX1,
                   GLint srcY1,
                   GLint dstX0,
                   GLint dstY0,
                   GLint dstX1,
                   GLint dstY1)
{
    const Extents &writeSize = writeBuffer->getSize();
    const Extents &readSize  = readBuffer->getSize();

    if (srcX0 != 0 || srcY0 != 0 || dstX0 != 0 || dstY0 != 0 || dstX1 != writeSize.width ||
        dstY1 != writeSize.height || srcX1 != readSize.width || srcY1 != readSize.height)
    {
        return true;
    }

    if (context->getState().isScissorTestEnabled())
    {
        const Rectangle &scissor = context->getState().getScissor();
        return scissor.x > 0 || scissor.y > 0 || scissor.width < writeSize.width ||
               scissor.height < writeSize.height;
    }

    return false;
}

bool IsValidCopyTextureSourceInternalFormatEnum(GLenum internalFormat)
{
    // Table 1.1 from the CHROMIUM_copy_texture spec
    switch (GetUnsizedFormat(internalFormat))
    {
        case GL_RED:
        case GL_ALPHA:
        case GL_LUMINANCE:
        case GL_LUMINANCE_ALPHA:
        case GL_RGB:
        case GL_RGBA:
        case GL_RGB8:
        case GL_RGBA8:
        case GL_BGRA_EXT:
        case GL_BGRA8_EXT:
        case GL_SRGB_ALPHA_EXT:
            return true;

        default:
            return false;
    }
}

bool IsValidCopySubTextureSourceInternalFormat(GLenum internalFormat)
{
    return IsValidCopyTextureSourceInternalFormatEnum(internalFormat);
}

bool IsValidCopyTextureDestinationInternalFormatEnum(GLint internalFormat)
{
    // Table 1.0 from the CHROMIUM_copy_texture spec
    switch (internalFormat)
    {
        case GL_ALPHA:
        case GL_BGRA8_EXT:
        case GL_BGRA_EXT:
        case GL_LUMINANCE:
        case GL_LUMINANCE_ALPHA:
        case GL_R11F_G11F_B10F:
        case GL_R16F:
        case GL_R32F:
        case GL_R8:
        case GL_R8UI:
        case GL_RG16F:
        case GL_RG32F:
        case GL_RG8:
        case GL_RG8UI:
        case GL_RGB:
        case GL_RGB10_A2:
        case GL_RGB16F:
        case GL_RGB32F:
        case GL_RGB565:
        case GL_RGB5_A1:
        case GL_RGB8:
        case GL_RGB8UI:
        case GL_RGB9_E5:
        case GL_RGBA:
        case GL_RGBA16F:
        case GL_RGBA32F:
        case GL_RGBA4:
        case GL_RGBA8:
        case GL_RGBA8UI:
        case GL_RGBX8_ANGLE:
        case GL_SRGB8:
        case GL_SRGB8_ALPHA8:
        case GL_SRGB_ALPHA_EXT:
        case GL_SRGB_EXT:
            return true;

        default:
            return false;
    }
}

bool IsValidCopySubTextureDestionationInternalFormat(GLenum internalFormat)
{
    return IsValidCopyTextureDestinationInternalFormatEnum(internalFormat);
}

bool IsValidCopyTextureDestinationFormatType(const Context *context,
                                             angle::EntryPoint entryPoint,
                                             GLint internalFormat,
                                             GLenum type)
{
    if (!IsValidCopyTextureDestinationInternalFormatEnum(internalFormat))
    {
        ANGLE_VALIDATION_ERRORF(GL_INVALID_OPERATION, kInvalidInternalFormat, internalFormat);
        return false;
    }

    if (!ValidES3FormatCombination(GetUnsizedFormat(internalFormat), type, internalFormat))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedTypeAndFormat);
        return false;
    }

    const InternalFormat &internalFormatInfo = GetInternalFormatInfo(internalFormat, type);
    if (!internalFormatInfo.textureSupport(context->getClientVersion(), context->getExtensions()))
    {
        ANGLE_VALIDATION_ERRORF(GL_INVALID_OPERATION, kInvalidInternalFormat, internalFormat);
        return false;
    }

    return true;
}

bool IsValidCopyTextureDestinationTargetEnum(const Context *context, TextureTarget target)
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

        default:
            return false;
    }
}

bool IsValidCopyTextureDestinationTarget(const Context *context,
                                         TextureType textureType,
                                         TextureTarget target)
{
    return TextureTargetToType(target) == textureType;
}

bool IsValidCopyTextureSourceTarget(const Context *context, TextureType type)
{
    switch (type)
    {
        case TextureType::_2D:
            return true;
        case TextureType::Rectangle:
            return context->getExtensions().textureRectangleANGLE;
        case TextureType::External:
            return context->getExtensions().EGLImageExternalOES;
        case TextureType::VideoImage:
            return context->getExtensions().videoTextureWEBGL;
        default:
            return false;
    }
}

bool IsValidCopyTextureSourceLevel(const Context *context, TextureType type, GLint level)
{
    if (!ValidMipLevel(context, type, level))
    {
        return false;
    }

    if (level > 0 && context->getClientVersion() < ES_3_0)
    {
        return false;
    }

    return true;
}

bool IsValidCopyTextureDestinationLevel(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        TextureType type,
                                        GLint level,
                                        GLsizei width,
                                        GLsizei height,
                                        bool isSubImage)
{
    if (!ValidImageSizeParameters(context, entryPoint, type, level, width, height, 1, isSubImage))
    {
        return false;
    }

    const Caps &caps = context->getCaps();
    switch (type)
    {
        case TextureType::_2D:
            return width <= (caps.max2DTextureSize >> level) &&
                   height <= (caps.max2DTextureSize >> level);
        case TextureType::Rectangle:
            ASSERT(level == 0);
            return width <= (caps.max2DTextureSize >> level) &&
                   height <= (caps.max2DTextureSize >> level);

        case TextureType::CubeMap:
            return width <= (caps.maxCubeMapTextureSize >> level) &&
                   height <= (caps.maxCubeMapTextureSize >> level);
        default:
            return true;
    }
}

bool IsValidStencilFunc(GLenum func)
{
    switch (func)
    {
        case GL_NEVER:
        case GL_ALWAYS:
        case GL_LESS:
        case GL_LEQUAL:
        case GL_EQUAL:
        case GL_GEQUAL:
        case GL_GREATER:
        case GL_NOTEQUAL:
            return true;

        default:
            return false;
    }
}

bool IsValidStencilFace(GLenum face)
{
    switch (face)
    {
        case GL_FRONT:
        case GL_BACK:
        case GL_FRONT_AND_BACK:
            return true;

        default:
            return false;
    }
}

bool IsValidStencilOp(GLenum op)
{
    switch (op)
    {
        case GL_ZERO:
        case GL_KEEP:
        case GL_REPLACE:
        case GL_INCR:
        case GL_DECR:
        case GL_INVERT:
        case GL_INCR_WRAP:
        case GL_DECR_WRAP:
            return true;

        default:
            return false;
    }
}

static inline bool Valid1to4ComponentFloatColorBufferFormat(const Context *context, GLenum format)
{
    return (context->getExtensions().textureFloatOES &&
            (format == GL_RGBA32F || format == GL_RGB32F || format == GL_RG32F ||
             format == GL_R32F)) ||
           (context->getExtensions().textureHalfFloatOES &&
            (format == GL_RGBA16F || format == GL_RGB16F || format == GL_RG16F ||
             format == GL_R16F));
}

static inline bool Valid2to4ComponentFloatColorBufferFormat(const Context *context, GLenum format)
{
    return (context->getExtensions().textureFloatOES &&
            (format == GL_RGBA32F || format == GL_RGB32F || format == GL_RG32F)) ||
           (context->getExtensions().textureHalfFloatOES &&
            (format == GL_RGBA16F || format == GL_RGB16F || format == GL_RG16F));
}

static inline bool Valid3to4ComponentFloatColorBufferFormat(const Context *context, GLenum format)
{
    return (context->getExtensions().textureFloatOES &&
            (format == GL_RGBA32F || format == GL_RGB32F)) ||
           (context->getExtensions().textureHalfFloatOES &&
            (format == GL_RGBA16F || format == GL_RGB16F));
}

static inline bool Valid4ComponentFloatColorBufferFormat(const Context *context, GLenum format)
{
    return (context->getExtensions().textureFloatOES && format == GL_RGBA32F) ||
           (context->getExtensions().textureHalfFloatOES && format == GL_RGBA16F);
}

bool ValidateES2CopyTexImageParameters(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       TextureTarget target,
                                       GLint level,
                                       GLenum internalformat,
                                       bool isSubImage,
                                       GLint xoffset,
                                       GLint yoffset,
                                       GLint x,
                                       GLint y,
                                       GLsizei width,
                                       GLsizei height,
                                       GLint border)
{
    if (!ValidTexture2DDestinationTarget(context, target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
        return false;
    }

    TextureType texType = TextureTargetToType(target);
    if (!ValidImageSizeParameters(context, entryPoint, texType, level, width, height, 1,
                                  isSubImage))
    {
        // Error is already handled.
        return false;
    }

    Format textureFormat = Format::Invalid();
    if (!ValidateCopyTexImageParametersBase(context, entryPoint, target, level, internalformat,
                                            isSubImage, xoffset, yoffset, 0, x, y, width, height,
                                            border, &textureFormat))
    {
        return false;
    }

    ASSERT(textureFormat.valid() || !isSubImage);

    const Framebuffer *framebuffer = context->getState().getReadFramebuffer();
    GLenum colorbufferFormat =
        framebuffer->getReadColorAttachment()->getFormat().info->sizedInternalFormat;
    const auto &formatInfo = *textureFormat.info;

    // ValidateCopyTexImageParametersBase rejects compressed formats with GL_INVALID_OPERATION.
    ASSERT(!formatInfo.compressed);
    ASSERT(!GetInternalFormatInfo(internalformat, GL_UNSIGNED_BYTE).compressed);

    // ValidateCopyTexImageParametersBase rejects depth formats with GL_INVALID_OPERATION.
    ASSERT(!formatInfo.depthBits);
    ASSERT(!GetInternalFormatInfo(internalformat, GL_UNSIGNED_BYTE).depthBits);

    // [OpenGL ES 2.0.24] table 3.9
    if (isSubImage)
    {
        switch (formatInfo.format)
        {
            case GL_ALPHA:
                if (colorbufferFormat != GL_ALPHA8_EXT && colorbufferFormat != GL_RGBA4 &&
                    colorbufferFormat != GL_RGB5_A1 && colorbufferFormat != GL_RGBA8_OES &&
                    colorbufferFormat != GL_BGRA8_EXT && colorbufferFormat != GL_BGR5_A1_ANGLEX &&
                    !Valid4ComponentFloatColorBufferFormat(context, colorbufferFormat))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFormat);
                    return false;
                }
                break;
            case GL_LUMINANCE:
                if (colorbufferFormat != GL_R8_EXT && colorbufferFormat != GL_RG8_EXT &&
                    colorbufferFormat != GL_RGB565 && colorbufferFormat != GL_RGB8_OES &&
                    colorbufferFormat != GL_RGBA4 && colorbufferFormat != GL_RGB5_A1 &&
                    colorbufferFormat != GL_RGBA8_OES && colorbufferFormat != GL_BGRA8_EXT &&
                    colorbufferFormat != GL_BGR5_A1_ANGLEX &&
                    colorbufferFormat != GL_BGRX8_ANGLEX && colorbufferFormat != GL_RGBX8_ANGLE &&
                    !Valid1to4ComponentFloatColorBufferFormat(context, colorbufferFormat))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFormat);
                    return false;
                }
                break;
            case GL_RED_EXT:
                if (colorbufferFormat != GL_R8_EXT && colorbufferFormat != GL_RG8_EXT &&
                    colorbufferFormat != GL_RGB565 && colorbufferFormat != GL_RGB8_OES &&
                    colorbufferFormat != GL_RGBA4 && colorbufferFormat != GL_RGB5_A1 &&
                    colorbufferFormat != GL_RGBA8_OES && colorbufferFormat != GL_R32F &&
                    colorbufferFormat != GL_RG32F && colorbufferFormat != GL_RGB32F &&
                    colorbufferFormat != GL_RGBA32F && colorbufferFormat != GL_BGRA8_EXT &&
                    colorbufferFormat != GL_BGR5_A1_ANGLEX &&
                    colorbufferFormat != GL_BGRX8_ANGLEX && colorbufferFormat != GL_RGBX8_ANGLE &&
                    !Valid1to4ComponentFloatColorBufferFormat(context, colorbufferFormat))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFormat);
                    return false;
                }
                break;
            case GL_RG_EXT:
                if (colorbufferFormat != GL_RG8_EXT && colorbufferFormat != GL_RGB565 &&
                    colorbufferFormat != GL_RGB8_OES && colorbufferFormat != GL_RGBA4 &&
                    colorbufferFormat != GL_RGB5_A1 && colorbufferFormat != GL_RGBA8_OES &&
                    colorbufferFormat != GL_RG32F && colorbufferFormat != GL_RGB32F &&
                    colorbufferFormat != GL_RGBA32F && colorbufferFormat != GL_BGRA8_EXT &&
                    colorbufferFormat != GL_BGR5_A1_ANGLEX &&
                    colorbufferFormat != GL_BGRX8_ANGLEX && colorbufferFormat != GL_RGBX8_ANGLE &&
                    !Valid2to4ComponentFloatColorBufferFormat(context, colorbufferFormat))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFormat);
                    return false;
                }
                break;
            case GL_RGB:
                if (colorbufferFormat != GL_RGB565 && colorbufferFormat != GL_RGB8_OES &&
                    colorbufferFormat != GL_RGBA4 && colorbufferFormat != GL_RGB5_A1 &&
                    colorbufferFormat != GL_RGBA8_OES && colorbufferFormat != GL_RGB32F &&
                    colorbufferFormat != GL_RGBA32F && colorbufferFormat != GL_BGRA8_EXT &&
                    colorbufferFormat != GL_BGR5_A1_ANGLEX &&
                    colorbufferFormat != GL_BGRX8_ANGLEX && colorbufferFormat != GL_RGBX8_ANGLE &&
                    !Valid3to4ComponentFloatColorBufferFormat(context, colorbufferFormat))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFormat);
                    return false;
                }
                break;
            case GL_LUMINANCE_ALPHA:
            case GL_RGBA:
            case GL_BGRA_EXT:
                if (colorbufferFormat != GL_RGBA4 && colorbufferFormat != GL_RGB5_A1 &&
                    colorbufferFormat != GL_RGBA8_OES && colorbufferFormat != GL_RGBA32F &&
                    colorbufferFormat != GL_BGRA8_EXT && colorbufferFormat != GL_BGR5_A1_ANGLEX &&
                    !Valid4ComponentFloatColorBufferFormat(context, colorbufferFormat))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFormat);
                    return false;
                }
                break;
            default:
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFormat);
                return false;
        }

        if (formatInfo.type == GL_FLOAT && !context->getExtensions().textureFloatOES)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFormat);
            return false;
        }
    }
    else
    {
        switch (internalformat)
        {
            case GL_ALPHA:
            case GL_ALPHA8_OES:
                if (colorbufferFormat != GL_ALPHA8_EXT && colorbufferFormat != GL_RGBA4 &&
                    colorbufferFormat != GL_RGB5_A1 && colorbufferFormat != GL_BGRA8_EXT &&
                    colorbufferFormat != GL_RGBA8_OES && colorbufferFormat != GL_BGR5_A1_ANGLEX &&
                    !Valid4ComponentFloatColorBufferFormat(context, colorbufferFormat))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFormat);
                    return false;
                }
                break;
            case GL_LUMINANCE:
            case GL_LUMINANCE8_OES:
            case GL_RED_EXT:
                if (colorbufferFormat != GL_R8_EXT && colorbufferFormat != GL_RG8_EXT &&
                    colorbufferFormat != GL_RGB565 && colorbufferFormat != GL_RGB8_OES &&
                    colorbufferFormat != GL_RGBA4 && colorbufferFormat != GL_RGB5_A1 &&
                    colorbufferFormat != GL_BGRA8_EXT && colorbufferFormat != GL_RGBA8_OES &&
                    colorbufferFormat != GL_BGR5_A1_ANGLEX &&
                    colorbufferFormat != GL_BGRX8_ANGLEX && colorbufferFormat != GL_RGBX8_ANGLE &&
                    !Valid1to4ComponentFloatColorBufferFormat(context, colorbufferFormat))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFormat);
                    return false;
                }
                break;
            case GL_RG_EXT:
                if (colorbufferFormat != GL_RG8_EXT && colorbufferFormat != GL_RGB565 &&
                    colorbufferFormat != GL_RGB8_OES && colorbufferFormat != GL_RGBA4 &&
                    colorbufferFormat != GL_RGB5_A1 && colorbufferFormat != GL_BGRA8_EXT &&
                    colorbufferFormat != GL_RGBA8_OES && colorbufferFormat != GL_BGR5_A1_ANGLEX &&
                    colorbufferFormat != GL_BGRX8_ANGLEX && colorbufferFormat != GL_RGBX8_ANGLE &&
                    !Valid2to4ComponentFloatColorBufferFormat(context, colorbufferFormat))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFormat);
                    return false;
                }
                break;
            case GL_RGB:
            case GL_RGB565_OES:
            case GL_RGB8_OES:
                if (colorbufferFormat != GL_RGB565 && colorbufferFormat != GL_RGB8_OES &&
                    colorbufferFormat != GL_RGBA4 && colorbufferFormat != GL_RGB5_A1 &&
                    colorbufferFormat != GL_BGRA8_EXT && colorbufferFormat != GL_RGBA8_OES &&
                    colorbufferFormat != GL_BGR5_A1_ANGLEX &&
                    colorbufferFormat != GL_BGRX8_ANGLEX && colorbufferFormat != GL_RGBX8_ANGLE &&
                    !Valid3to4ComponentFloatColorBufferFormat(context, colorbufferFormat))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFormat);
                    return false;
                }
                break;
            case GL_LUMINANCE_ALPHA:
            case GL_LUMINANCE4_ALPHA4_OES:
            case GL_LUMINANCE8_ALPHA8_OES:
            case GL_RGBA:
            case GL_BGRA_EXT:
            case GL_RGBA4_OES:
            case GL_RGB5_A1_OES:
            case GL_RGBA8_OES:
                if (colorbufferFormat != GL_RGBA4 && colorbufferFormat != GL_RGB5_A1 &&
                    colorbufferFormat != GL_BGRA8_EXT && colorbufferFormat != GL_RGBA8_OES &&
                    colorbufferFormat != GL_BGR5_A1_ANGLEX && colorbufferFormat != GL_RGBA16F &&
                    !Valid4ComponentFloatColorBufferFormat(context, colorbufferFormat))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFormat);
                    return false;
                }
                break;
            default:
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, internalformat);
                return false;
        }
    }

    // If width or height is zero, it is a no-op.  Return false without setting an error.
    return (width > 0 && height > 0);
}

// ANGLE_shader_pixel_local_storage: INVALID_OPERATION is generated by Enable(), Disable() if <cap>
// is not one of: CULL_FACE, DEBUG_OUTPUT, DEBUG_OUTPUT_SYNCHRONOUS, DEPTH_CLAMP_EXT, DEPTH_TEST,
// POLYGON_OFFSET_FILL, PRIMITIVE_RESTART_FIXED_INDEX, SCISSOR_TEST, SCISSOR_TEST_EXCLUSIVE_NV,
// STENCIL_TEST, CLIP_DISTANCE[0..7]_EXT
static bool IsCapBannedWithActivePLS(GLenum cap)
{
    switch (cap)
    {
        case GL_BLEND:
        case GL_CULL_FACE:
        case GL_DEBUG_OUTPUT:
        case GL_DEBUG_OUTPUT_SYNCHRONOUS:
        case GL_DEPTH_CLAMP_EXT:
        case GL_DEPTH_TEST:
        case GL_POLYGON_OFFSET_POINT_NV:
        case GL_POLYGON_OFFSET_LINE_NV:  // = GL_POLYGON_OFFSET_LINE_ANGLE
        case GL_POLYGON_OFFSET_FILL:
        case GL_PRIMITIVE_RESTART_FIXED_INDEX:
        case GL_SCISSOR_TEST:
        case GL_SCISSOR_TEST_EXCLUSIVE_NV:
        case GL_STENCIL_TEST:
        case GL_CLIP_DISTANCE0_EXT:
        case GL_CLIP_DISTANCE1_EXT:
        case GL_CLIP_DISTANCE2_EXT:
        case GL_CLIP_DISTANCE3_EXT:
        case GL_CLIP_DISTANCE4_EXT:
        case GL_CLIP_DISTANCE5_EXT:
        case GL_CLIP_DISTANCE6_EXT:
        case GL_CLIP_DISTANCE7_EXT:
            return false;
        default:
            return true;
    }
}

bool ValidCapUncommon(const PrivateState &state, ErrorSet *errors, GLenum cap, bool queryOnly)
{
    switch (cap)
    {
        // EXT_multisample_compatibility
        case GL_MULTISAMPLE_EXT:
        case GL_SAMPLE_ALPHA_TO_ONE_EXT:
            return state.getExtensions().multisampleCompatibilityEXT;

        case GL_SAMPLE_ALPHA_TO_COVERAGE:
        case GL_SAMPLE_COVERAGE:
        case GL_DITHER:
            return true;

        case GL_POLYGON_OFFSET_POINT_NV:
            return state.getExtensions().polygonModeNV;
        case GL_POLYGON_OFFSET_LINE_NV:  // = GL_POLYGON_OFFSET_LINE_ANGLE
            return state.getExtensions().polygonModeAny();

        case GL_DEPTH_CLAMP_EXT:
            return state.getExtensions().depthClampEXT;

        case GL_PRIMITIVE_RESTART_FIXED_INDEX:
        case GL_RASTERIZER_DISCARD:
            return (state.getClientMajorVersion() >= 3);

        case GL_DEBUG_OUTPUT_SYNCHRONOUS:
        case GL_DEBUG_OUTPUT:
            return state.getExtensions().debugKHR;

        case GL_BIND_GENERATES_RESOURCE_CHROMIUM:
            return queryOnly && state.getExtensions().bindGeneratesResourceCHROMIUM;

        case GL_CLIENT_ARRAYS_ANGLE:
            return queryOnly && state.getExtensions().clientArraysANGLE;

        case GL_FRAMEBUFFER_SRGB_EXT:
            return state.getExtensions().sRGBWriteControlEXT;

        case GL_SAMPLE_MASK:
            return state.getClientVersion() >= Version(3, 1) ||
                   state.getExtensions().textureMultisampleANGLE;

        case GL_ROBUST_RESOURCE_INITIALIZATION_ANGLE:
            return queryOnly && state.getExtensions().robustResourceInitializationANGLE;

        case GL_TEXTURE_RECTANGLE_ANGLE:
            return state.isWebGL();

        // GL_APPLE_clip_distance / GL_EXT_clip_cull_distance / GL_ANGLE_clip_cull_distance
        case GL_CLIP_DISTANCE0_EXT:
        case GL_CLIP_DISTANCE1_EXT:
        case GL_CLIP_DISTANCE2_EXT:
        case GL_CLIP_DISTANCE3_EXT:
        case GL_CLIP_DISTANCE4_EXT:
        case GL_CLIP_DISTANCE5_EXT:
        case GL_CLIP_DISTANCE6_EXT:
        case GL_CLIP_DISTANCE7_EXT:
            if (state.getExtensions().clipDistanceAPPLE ||
                state.getExtensions().clipCullDistanceAny())
            {
                return true;
            }
            break;
        case GL_SAMPLE_SHADING:
            return state.getExtensions().sampleShadingOES;
        case GL_SHADING_RATE_PRESERVE_ASPECT_RATIO_QCOM:
            return state.getExtensions().shadingRateQCOM;

        // COLOR_LOGIC_OP is in GLES1, but exposed through an ANGLE extension.
        case GL_COLOR_LOGIC_OP:
            return state.getClientVersion() < Version(2, 0) || state.getExtensions().logicOpANGLE;

        case GL_FETCH_PER_SAMPLE_ARM:
            return state.getExtensions().shaderFramebufferFetchARM;

        case GL_FRAGMENT_SHADER_FRAMEBUFFER_FETCH_MRT_ARM:
            return queryOnly && state.getExtensions().shaderFramebufferFetchARM;

        case GL_BLEND_ADVANCED_COHERENT_KHR:
            return state.getClientVersion() >= Version(2, 0) &&
                   state.getExtensions().blendEquationAdvancedCoherentKHR;

        default:
            break;
    }

    // GLES1 emulation: GLES1-specific caps after this point
    if (state.getClientVersion().major != 1)
    {
        return false;
    }

    switch (cap)
    {
        case GL_ALPHA_TEST:
        case GL_VERTEX_ARRAY:
        case GL_NORMAL_ARRAY:
        case GL_COLOR_ARRAY:
        case GL_TEXTURE_COORD_ARRAY:
        case GL_TEXTURE_2D:
        case GL_LIGHTING:
        case GL_LIGHT0:
        case GL_LIGHT1:
        case GL_LIGHT2:
        case GL_LIGHT3:
        case GL_LIGHT4:
        case GL_LIGHT5:
        case GL_LIGHT6:
        case GL_LIGHT7:
        case GL_NORMALIZE:
        case GL_RESCALE_NORMAL:
        case GL_COLOR_MATERIAL:
        case GL_CLIP_PLANE0:
        case GL_CLIP_PLANE1:
        case GL_CLIP_PLANE2:
        case GL_CLIP_PLANE3:
        case GL_CLIP_PLANE4:
        case GL_CLIP_PLANE5:
        case GL_FOG:
        case GL_POINT_SMOOTH:
        case GL_LINE_SMOOTH:
            return state.getClientVersion() < Version(2, 0);
        case GL_POINT_SIZE_ARRAY_OES:
            return state.getClientVersion() < Version(2, 0) &&
                   state.getExtensions().pointSizeArrayOES;
        case GL_TEXTURE_CUBE_MAP:
            return state.getClientVersion() < Version(2, 0) &&
                   state.getExtensions().textureCubeMapOES;
        case GL_POINT_SPRITE_OES:
            return state.getClientVersion() < Version(2, 0) && state.getExtensions().pointSpriteOES;
        default:
            return false;
    }
}

ANGLE_INLINE bool ValidCap(const PrivateState &state, ErrorSet *errors, GLenum cap, bool queryOnly)
{
    // Most frequent cases inline.
    if (ANGLE_LIKELY(cap == GL_BLEND || cap == GL_DEPTH_TEST || cap == GL_SCISSOR_TEST ||
                     cap == GL_STENCIL_TEST || cap == GL_CULL_FACE ||
                     cap == GL_POLYGON_OFFSET_FILL))
    {
        return true;
    }
    // Other less common cases are a function call.
    return ValidCapUncommon(state, errors, cap, queryOnly);
}

// Return true if a character belongs to the ASCII subset as defined in GLSL ES 1.0 spec section
// 3.1.
bool IsValidESSLCharacter(unsigned char c)
{
    // Printing characters are valid except " $ ` @ \ ' DEL.
    if (c >= 32 && c <= 126 && c != '"' && c != '$' && c != '`' && c != '@' && c != '\\' &&
        c != '\'')
    {
        return true;
    }

    // Horizontal tab, line feed, vertical tab, form feed, carriage return are also valid.
    if (c >= 9 && c <= 13)
    {
        return true;
    }

    return false;
}

bool IsValidESSLString(const char *str, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        if (!IsValidESSLCharacter(str[i]))
        {
            return false;
        }
    }

    return true;
}

bool ValidateWebGLNamePrefix(const Context *context,
                             angle::EntryPoint entryPoint,
                             const GLchar *name)
{
    ASSERT(context->isWebGL());

    // WebGL 1.0 [Section 6.16] GLSL Constructs
    // Identifiers starting with "webgl_" and "_webgl_" are reserved for use by WebGL.
    if (strncmp(name, "webgl_", 6) == 0 || strncmp(name, "_webgl_", 7) == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kWebglBindAttribLocationReservedPrefix);
        return false;
    }

    return true;
}

bool ValidateWebGLNameLength(const Context *context, angle::EntryPoint entryPoint, size_t length)
{
    ASSERT(context->isWebGL());

    if (context->isWebGL1() && length > 256)
    {
        // WebGL 1.0 [Section 6.21] Maxmimum Uniform and Attribute Location Lengths
        // WebGL imposes a limit of 256 characters on the lengths of uniform and attribute
        // locations.
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kWebglNameLengthLimitExceeded);

        return false;
    }
    else if (length > 1024)
    {
        // WebGL 2.0 [Section 4.3.2] WebGL 2.0 imposes a limit of 1024 characters on the lengths of
        // uniform and attribute locations.
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kWebgl2NameLengthLimitExceeded);
        return false;
    }

    return true;
}

bool ValidateSrcBlendFunc(const PrivateState &state,
                          ErrorSet *errors,
                          angle::EntryPoint entryPoint,
                          GLenum val)
{
    switch (val)
    {
        case GL_ZERO:
        case GL_ONE:
        case GL_SRC_COLOR:
        case GL_ONE_MINUS_SRC_COLOR:
        case GL_DST_COLOR:
        case GL_ONE_MINUS_DST_COLOR:
        case GL_SRC_ALPHA:
        case GL_ONE_MINUS_SRC_ALPHA:
        case GL_DST_ALPHA:
        case GL_ONE_MINUS_DST_ALPHA:
        case GL_CONSTANT_COLOR:
        case GL_ONE_MINUS_CONSTANT_COLOR:
        case GL_CONSTANT_ALPHA:
        case GL_ONE_MINUS_CONSTANT_ALPHA:
        case GL_SRC_ALPHA_SATURATE:
            return true;

        // EXT_blend_func_extended, excluding GL_SRC_ALPHA_SATURATE_EXT, which equals
        // GL_SRC_ALPHA_SATURATE and is handled above.
        case GL_SRC1_COLOR_EXT:
        case GL_SRC1_ALPHA_EXT:
        case GL_ONE_MINUS_SRC1_COLOR_EXT:
        case GL_ONE_MINUS_SRC1_ALPHA_EXT:
            if (!state.getExtensions().blendFuncExtendedEXT)
            {
                break;
            }
            if (state.getPixelLocalStorageActivePlanes() != 0)
            {
                // INVALID_OPERATION is generated by BlendFunc*() if <srcRGB>, <dstRGB>, <srcAlpha>,
                // or <dstAlpha> is a blend function requiring the secondary color input, as
                // specified in EXT_blend_func_extended (SRC1_COLOR_EXT, ONE_MINUS_SRC1_COLOR_EXT,
                // SRC1_ALPHA_EXT, ONE_MINUS_SRC1_ALPHA_EXT).
                errors->validationError(entryPoint, GL_INVALID_OPERATION,
                                        kPLSSecondaryBlendNotSupported);
                return false;
            }
            return true;
    }

    errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidBlendFunction);
    return false;
}

bool ValidateDstBlendFunc(const PrivateState &state,
                          ErrorSet *errors,
                          angle::EntryPoint entryPoint,
                          GLenum val)
{
    if (val == GL_SRC_ALPHA_SATURATE)
    {
        // Unextended ES2 does not allow GL_SRC_ALPHA_SATURATE as a dst blend func.
        if (state.getClientMajorVersion() < 3 && !state.getExtensions().blendFuncExtendedEXT)
        {
            errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidBlendFunction);
            return false;
        }
    }

    return ValidateSrcBlendFunc(state, errors, entryPoint, val);
}

bool ValidateES2TexImageParameters(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   TextureTarget target,
                                   GLint level,
                                   GLenum internalformat,
                                   bool isCompressed,
                                   bool isSubImage,
                                   GLint xoffset,
                                   GLint yoffset,
                                   GLsizei width,
                                   GLsizei height,
                                   GLint border,
                                   GLenum format,
                                   GLenum type,
                                   GLsizei imageSize,
                                   const void *pixels)
{
    if (!ValidTexture2DDestinationTarget(context, target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
        return false;
    }

    return ValidateES2TexImageParametersBase(context, entryPoint, target, level, internalformat,
                                             isCompressed, isSubImage, xoffset, yoffset, width,
                                             height, border, format, type, imageSize, pixels);
}

}  // anonymous namespace

bool ValidateES2TexImageParametersBase(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       TextureTarget target,
                                       GLint level,
                                       GLenum internalformat,
                                       bool isCompressed,
                                       bool isSubImage,
                                       GLint xoffset,
                                       GLint yoffset,
                                       GLsizei width,
                                       GLsizei height,
                                       GLint border,
                                       GLenum format,
                                       GLenum type,
                                       GLsizei imageSize,
                                       const void *pixels)
{

    TextureType texType = TextureTargetToType(target);
    if (!ValidImageSizeParameters(context, entryPoint, texType, level, width, height, 1,
                                  isSubImage))
    {
        // Error already handled.
        return false;
    }

    if ((xoffset < 0 || std::numeric_limits<GLsizei>::max() - xoffset < width) ||
        (yoffset < 0 || std::numeric_limits<GLsizei>::max() - yoffset < height))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kResourceMaxTextureSize);
        return false;
    }

    const Caps &caps = context->getCaps();

    switch (texType)
    {
        case TextureType::_2D:
        case TextureType::External:
        case TextureType::VideoImage:
            if (width > (caps.max2DTextureSize >> level) ||
                height > (caps.max2DTextureSize >> level))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kResourceMaxTextureSize);
                return false;
            }
            break;

        case TextureType::Rectangle:
            ASSERT(level == 0);
            if (width > caps.maxRectangleTextureSize || height > caps.maxRectangleTextureSize)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kResourceMaxTextureSize);
                return false;
            }
            if (isCompressed)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kRectangleTextureCompressed);
                return false;
            }
            break;

        case TextureType::CubeMap:
            if (!isSubImage && width != height)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kCubemapFacesEqualDimensions);
                return false;
            }

            if (width > (caps.maxCubeMapTextureSize >> level) ||
                height > (caps.maxCubeMapTextureSize >> level))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kResourceMaxTextureSize);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
            return false;
    }

    Texture *texture = context->getTextureByType(texType);
    if (!texture)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferNotBound);
        return false;
    }

    // Verify zero border
    if (border != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidBorder);
        return false;
    }

    bool nonEqualFormatsAllowed = false;

    if (isCompressed)
    {
        GLenum actualInternalFormat =
            isSubImage ? texture->getFormat(target, level).info->sizedInternalFormat
                       : internalformat;

        const InternalFormat &internalFormatInfo = GetSizedInternalFormatInfo(actualInternalFormat);

        if (!internalFormatInfo.compressed && !internalFormatInfo.paletted)
        {
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kInvalidInternalFormat, internalformat);
            return false;
        }

        if (!internalFormatInfo.textureSupport(context->getClientVersion(),
                                               context->getExtensions()))
        {
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kInvalidInternalFormat, internalformat);
            return false;
        }

        if (isSubImage)
        {
            // From OpenGL ES Version 1.1.12, section 3.7.4 Compressed Paletted
            // Textures:
            //
            // Subimages may not be specified for compressed paletted textures.
            // Calling CompressedTexSubImage2D with any of the PALETTE*
            // arguments in table 3.11 will generate an INVALID OPERATION error.
            if (internalFormatInfo.paletted)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_OPERATION, kInvalidInternalFormat,
                                        internalformat);
                return false;
            }

            // From the OES_compressed_ETC1_RGB8_texture spec:
            //
            // INVALID_OPERATION is generated by CompressedTexSubImage2D, TexSubImage2D, or
            // CopyTexSubImage2D if the texture image <level> bound to <target> has internal format
            // ETC1_RGB8_OES.
            //
            // This is relaxed if GL_EXT_compressed_ETC1_RGB8_sub_texture is supported.
            if (IsETC1Format(actualInternalFormat) &&
                !context->getExtensions().compressedETC1RGB8SubTextureEXT)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_OPERATION, kInvalidInternalFormat,
                                        internalformat);
                return false;
            }

            if (!ValidCompressedSubImageSize(context, actualInternalFormat, xoffset, yoffset, 0,
                                             width, height, 1, texture->getWidth(target, level),
                                             texture->getHeight(target, level),
                                             texture->getDepth(target, level)))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidCompressedImageSize);
                return false;
            }

            if (format != actualInternalFormat)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFormat);
                return false;
            }
        }
        else
        {
            if (!ValidCompressedImageSize(context, actualInternalFormat, level, width, height, 1))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidCompressedImageSize);
                return false;
            }
        }
    }
    else
    {
        // validate <type> by itself (used as secondary key below)
        switch (type)
        {
            case GL_UNSIGNED_BYTE:
            case GL_UNSIGNED_SHORT_5_6_5:
            case GL_UNSIGNED_SHORT_4_4_4_4:
            case GL_UNSIGNED_SHORT_5_5_5_1:
            case GL_UNSIGNED_SHORT:
            case GL_UNSIGNED_INT:
            case GL_UNSIGNED_INT_24_8_OES:
            case GL_HALF_FLOAT_OES:
            case GL_FLOAT:
                break;
            case GL_UNSIGNED_INT_2_10_10_10_REV_EXT:
                if (!context->getExtensions().textureType2101010REVEXT)
                {
                    ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, type);
                    return false;
                }
                break;
            default:
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidType);
                return false;
        }

        // validate <format> + <type> combinations
        // - invalid <format> -> sets INVALID_ENUM
        // - invalid <format>+<type> combination -> sets INVALID_OPERATION
        switch (format)
        {
            case GL_ALPHA:
            case GL_LUMINANCE:
            case GL_LUMINANCE_ALPHA:
                switch (type)
                {
                    case GL_UNSIGNED_BYTE:
                    case GL_FLOAT:
                    case GL_HALF_FLOAT_OES:
                        break;
                    default:
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedTypeAndFormat);
                        return false;
                }
                break;
            case GL_RED:
            case GL_RG:
                if (!context->getExtensions().textureRgEXT)
                {
                    ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, format);
                    return false;
                }
                switch (type)
                {
                    case GL_UNSIGNED_BYTE:
                        break;
                    case GL_FLOAT:
                        if (!context->getExtensions().textureFloatOES)
                        {
                            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, type);
                            return false;
                        }
                        break;
                    case GL_HALF_FLOAT_OES:
                        if (!context->getExtensions().textureFloatOES &&
                            !context->getExtensions().textureHalfFloatOES)
                        {
                            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, type);
                            return false;
                        }
                        break;
                    case GL_SHORT:
                    case GL_UNSIGNED_SHORT:
                        if (!context->getExtensions().textureNorm16EXT)
                        {
                            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, type);
                            return false;
                        }
                        break;
                    default:
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedTypeAndFormat);
                        return false;
                }
                break;
            case GL_RGB:
                switch (type)
                {
                    case GL_UNSIGNED_BYTE:
                    case GL_UNSIGNED_SHORT_5_6_5:
                    case GL_UNSIGNED_INT_2_10_10_10_REV_EXT:
                    case GL_FLOAT:
                    case GL_HALF_FLOAT_OES:
                        break;
                    case GL_SHORT:
                    case GL_UNSIGNED_SHORT:
                        if (!context->getExtensions().textureNorm16EXT)
                        {
                            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedTypeAndFormat);
                            return false;
                        }
                        break;
                    default:
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedTypeAndFormat);
                        return false;
                }
                break;
            case GL_RGBA:
                switch (type)
                {
                    case GL_UNSIGNED_BYTE:
                    case GL_UNSIGNED_SHORT_4_4_4_4:
                    case GL_UNSIGNED_SHORT_5_5_5_1:
                    case GL_FLOAT:
                    case GL_HALF_FLOAT_OES:
                    case GL_UNSIGNED_INT_2_10_10_10_REV_EXT:
                        break;
                    case GL_SHORT:
                    case GL_UNSIGNED_SHORT:
                        if (!context->getExtensions().textureNorm16EXT)
                        {
                            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedTypeAndFormat);
                            return false;
                        }
                        break;
                    default:
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedTypeAndFormat);
                        return false;
                }
                break;
            case GL_BGRA_EXT:
                if (!context->getExtensions().textureFormatBGRA8888EXT)
                {
                    ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, format);
                    return false;
                }
                switch (type)
                {
                    case GL_UNSIGNED_BYTE:
                        break;
                    default:
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedTypeAndFormat);
                        return false;
                }
                break;
            case GL_SRGB_EXT:
            case GL_SRGB_ALPHA_EXT:
                if (!context->getExtensions().sRGBEXT)
                {
                    ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, format);
                    return false;
                }
                switch (type)
                {
                    case GL_UNSIGNED_BYTE:
                        break;
                    default:
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedTypeAndFormat);
                        return false;
                }
                break;
            case GL_DEPTH_COMPONENT:
                switch (type)
                {
                    case GL_UNSIGNED_SHORT:
                    case GL_UNSIGNED_INT:
                        break;
                    case GL_FLOAT:
                        if (!context->getExtensions().depthBufferFloat2NV)
                        {
                            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedTypeAndFormat);
                            return false;
                        }
                        break;
                    default:
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedTypeAndFormat);
                        return false;
                }
                break;
            case GL_DEPTH_STENCIL_OES:
                switch (type)
                {
                    case GL_UNSIGNED_INT_24_8_OES:
                        break;
                    default:
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedTypeAndFormat);
                        return false;
                }
                break;
            default:
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, format);
                return false;
        }

        switch (format)
        {
            case GL_DEPTH_COMPONENT:
            case GL_DEPTH_STENCIL_OES:
                if (!context->getExtensions().depthTextureANGLE &&
                    !((context->getExtensions().packedDepthStencilOES ||
                       context->getExtensions().depthTextureCubeMapOES) &&
                      context->getExtensions().depthTextureOES))
                {
                    ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, format);
                    return false;
                }

                switch (target)
                {
                    case TextureTarget::_2D:
                        break;
                    case TextureTarget::CubeMapNegativeX:
                    case TextureTarget::CubeMapNegativeY:
                    case TextureTarget::CubeMapNegativeZ:
                    case TextureTarget::CubeMapPositiveX:
                    case TextureTarget::CubeMapPositiveY:
                    case TextureTarget::CubeMapPositiveZ:
                        if (!context->getExtensions().depthTextureCubeMapOES)
                        {
                            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                                   kMismatchedTargetAndFormat);
                            return false;
                        }
                        break;
                    default:
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedTargetAndFormat);
                        return false;
                }

                // OES_depth_texture supports loading depth data and multiple levels,
                // but ANGLE_depth_texture does not
                if (!context->getExtensions().depthTextureOES)
                {
                    if (pixels != nullptr)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kPixelDataNotNull);
                        return false;
                    }
                    if (level != 0)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kLevelNotZero);
                        return false;
                    }
                }
                break;
            default:
                break;
        }

        if (!isSubImage)
        {
            switch (internalformat)
            {
                // Core ES 2.0 formats
                case GL_ALPHA:
                case GL_LUMINANCE:
                case GL_LUMINANCE_ALPHA:
                case GL_RGB:
                case GL_RGBA:
                    break;

                case GL_RGBA32F:
                    if (!context->getExtensions().colorBufferFloatRgbaCHROMIUM)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFormat);
                        return false;
                    }

                    nonEqualFormatsAllowed = true;

                    if (type != GL_FLOAT)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedTypeAndFormat);
                        return false;
                    }
                    if (format != GL_RGBA)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedTypeAndFormat);
                        return false;
                    }
                    break;

                case GL_RGB32F:
                    if (!context->getExtensions().colorBufferFloatRgbCHROMIUM)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFormat);
                        return false;
                    }

                    nonEqualFormatsAllowed = true;

                    if (type != GL_FLOAT)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedTypeAndFormat);
                        return false;
                    }
                    if (format != GL_RGB)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedTypeAndFormat);
                        return false;
                    }
                    break;

                case GL_BGRA_EXT:
                    if (!context->getExtensions().textureFormatBGRA8888EXT)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFormat);
                        return false;
                    }
                    break;

                case GL_DEPTH_COMPONENT:
                    if (!(context->getExtensions().depthTextureAny()))
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFormat);
                        return false;
                    }
                    break;

                case GL_DEPTH_STENCIL:
                    if (!(context->getExtensions().depthTextureANGLE ||
                          context->getExtensions().packedDepthStencilOES ||
                          context->getExtensions().depthTextureCubeMapOES))
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFormat);
                        return false;
                    }
                    break;
                // Valid sized internal format for GL_OES_required_internalformat.
                case GL_ALPHA8_OES:
                    if (context->getExtensions().requiredInternalformatOES &&
                        type == GL_UNSIGNED_BYTE && format == GL_ALPHA)
                    {
                        nonEqualFormatsAllowed = true;
                    }
                    break;

                case GL_LUMINANCE8_OES:
                    if (context->getExtensions().requiredInternalformatOES &&
                        type == GL_UNSIGNED_BYTE && format == GL_LUMINANCE)
                    {
                        nonEqualFormatsAllowed = true;
                    }
                    break;

                case GL_LUMINANCE4_ALPHA4_OES:
                    if (context->getExtensions().requiredInternalformatOES &&
                        type == GL_UNSIGNED_BYTE && format == GL_LUMINANCE_ALPHA)
                    {
                        nonEqualFormatsAllowed = true;
                    }
                    break;

                case GL_LUMINANCE8_ALPHA8_OES:
                    if (context->getExtensions().requiredInternalformatOES &&
                        type == GL_UNSIGNED_BYTE && format == GL_LUMINANCE_ALPHA)
                    {
                        nonEqualFormatsAllowed = true;
                    }
                    break;

                case GL_RGB565_OES:
                    if (context->getExtensions().requiredInternalformatOES &&
                        (type == GL_UNSIGNED_BYTE || type == GL_UNSIGNED_SHORT_5_6_5) &&
                        format == GL_RGB)
                    {
                        nonEqualFormatsAllowed = true;
                    }
                    if (context->getExtensions().requiredInternalformatOES &&
                        context->getExtensions().textureType2101010REVEXT &&
                        GL_UNSIGNED_INT_2_10_10_10_REV_EXT && format == GL_RGB)
                    {
                        nonEqualFormatsAllowed = true;
                    }
                    break;

                case GL_RGB8_OES:
                    if (context->getExtensions().requiredInternalformatOES &&
                        type == GL_UNSIGNED_BYTE && format == GL_RGB)
                    {
                        nonEqualFormatsAllowed = true;
                    }
                    if (context->getExtensions().requiredInternalformatOES &&
                        context->getExtensions().textureType2101010REVEXT &&
                        GL_UNSIGNED_INT_2_10_10_10_REV_EXT && format == GL_RGB)
                    {
                        nonEqualFormatsAllowed = true;
                    }
                    break;

                case GL_RGBA4_OES:
                    if (context->getExtensions().requiredInternalformatOES &&
                        (type == GL_UNSIGNED_BYTE || type == GL_UNSIGNED_SHORT_4_4_4_4) &&
                        format == GL_RGBA)
                    {
                        nonEqualFormatsAllowed = true;
                    }
                    break;

                case GL_RGB5_A1:
                    if (context->getExtensions().requiredInternalformatOES &&
                        (type == GL_UNSIGNED_BYTE || type == GL_UNSIGNED_SHORT_5_5_5_1) &&
                        format == GL_RGBA)
                    {
                        nonEqualFormatsAllowed = true;
                    }
                    if (context->getExtensions().requiredInternalformatOES &&
                        context->getExtensions().textureType2101010REVEXT &&
                        type == GL_UNSIGNED_INT_2_10_10_10_REV_EXT && format == GL_RGBA)
                    {
                        nonEqualFormatsAllowed = true;
                    }
                    break;

                case GL_RGBA8_OES:
                    if (context->getExtensions().requiredInternalformatOES &&
                        type == GL_UNSIGNED_BYTE && format == GL_RGBA)
                    {
                        nonEqualFormatsAllowed = true;
                    }
                    break;

                case GL_DEPTH_COMPONENT16_OES:
                    if (context->getExtensions().requiredInternalformatOES &&
                        (type == GL_UNSIGNED_SHORT || type == GL_UNSIGNED_INT) &&
                        format == GL_DEPTH_COMPONENT)
                    {
                        nonEqualFormatsAllowed = true;
                    }
                    break;

                case GL_DEPTH_COMPONENT24_OES:
                    if ((context->getExtensions().requiredInternalformatOES &&
                         context->getExtensions().depth24OES) &&
                        type == GL_UNSIGNED_INT && format == GL_DEPTH_COMPONENT)
                    {
                        nonEqualFormatsAllowed = true;
                    }
                    break;

                case GL_DEPTH_COMPONENT32_OES:
                    if ((context->getExtensions().requiredInternalformatOES &&
                         context->getExtensions().depth32OES) &&
                        type == GL_UNSIGNED_INT && format == GL_DEPTH_COMPONENT)
                    {
                        nonEqualFormatsAllowed = true;
                    }
                    break;

                case GL_DEPTH24_STENCIL8_OES:
                    if ((context->getExtensions().requiredInternalformatOES &&
                         context->getExtensions().packedDepthStencilOES) &&
                        type == GL_UNSIGNED_INT_24_8_OES && format == GL_DEPTH_STENCIL_OES)
                    {
                        nonEqualFormatsAllowed = true;
                    }
                    break;

                case GL_RED:
                case GL_RG:
                    if (!context->getExtensions().textureRgEXT)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFormat);
                        return false;
                    }
                    break;

                case GL_SRGB_EXT:
                case GL_SRGB_ALPHA_EXT:
                    if (!context->getExtensions().sRGBEXT)
                    {
                        ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, internalformat);
                        return false;
                    }
                    break;

                case GL_RGB10_A2_EXT:
                    if (!context->getExtensions().textureType2101010REVEXT)
                    {
                        ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, internalformat);
                        return false;
                    }

                    if (type != GL_UNSIGNED_INT_2_10_10_10_REV_EXT || format != GL_RGBA)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedTypeAndFormat);
                        return false;
                    }

                    nonEqualFormatsAllowed = true;

                    break;

                case GL_RGBX8_ANGLE:
                    if (context->getExtensions().rgbxInternalFormatANGLE &&
                        type == GL_UNSIGNED_BYTE && format == GL_RGB)
                    {
                        nonEqualFormatsAllowed = true;
                    }

                    break;

                case GL_R16_EXT:
                case GL_RG16_EXT:
                case GL_RGB16_EXT:
                case GL_RGBA16_EXT:
                case GL_R16_SNORM_EXT:
                case GL_RG16_SNORM_EXT:
                case GL_RGB16_SNORM_EXT:
                case GL_RGBA16_SNORM_EXT:
                    if (!context->getExtensions().textureNorm16EXT)
                    {
                        ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, internalformat);
                        return false;
                    }
                    break;
                default:
                    // Compressed formats are not valid internal formats for glTexImage*D
                    ANGLE_VALIDATION_ERRORF(GL_INVALID_VALUE, kInvalidInternalFormat,
                                            internalformat);
                    return false;
            }
        }

        if (type == GL_FLOAT)
        {
            if (!context->getExtensions().textureFloatOES)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, type);
                return false;
            }
        }
        else if (type == GL_HALF_FLOAT_OES)
        {
            if (!context->getExtensions().textureHalfFloatOES)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, type);
                return false;
            }
        }
    }

    if (isSubImage)
    {
        const InternalFormat &textureInternalFormat = *texture->getFormat(target, level).info;
        if (textureInternalFormat.internalFormat == GL_NONE)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidTextureLevel);
            return false;
        }

        if (format != textureInternalFormat.format)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, err::kTextureFormatMismatch);
            return false;
        }

        if (context->isWebGL())
        {
            if (GetInternalFormatInfo(format, type).sizedInternalFormat !=
                textureInternalFormat.sizedInternalFormat)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTextureTypeMismatch);
                return false;
            }
        }

        if (static_cast<size_t>(xoffset + width) > texture->getWidth(target, level) ||
            static_cast<size_t>(yoffset + height) > texture->getHeight(target, level))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kOffsetOverflow);
            return false;
        }

        if (width > 0 && height > 0 && pixels == nullptr &&
            context->getState().getTargetBuffer(BufferBinding::PixelUnpack) == nullptr)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kPixelDataNull);
            return false;
        }
    }
    else
    {
        if (texture->getImmutableFormat())
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTextureIsImmutable);
            return false;
        }
    }

    // From GL_CHROMIUM_color_buffer_float_rgb[a]:
    // GL_RGB[A] / GL_RGB[A]32F becomes an allowable format / internalformat parameter pair for
    // TexImage2D. The restriction in section 3.7.1 of the OpenGL ES 2.0 spec that the
    // internalformat parameter and format parameter of TexImage2D must match is lifted for this
    // case.
    if (!isSubImage && !isCompressed && internalformat != format && !nonEqualFormatsAllowed)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFormatCombination);
        return false;
    }

    GLenum sizeCheckFormat = isSubImage ? format : internalformat;
    return ValidImageDataSize(context, entryPoint, texType, width, height, 1, sizeCheckFormat, type,
                              pixels, imageSize);
}

bool ValidateES2TexStorageParametersBase(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         TextureType target,
                                         GLsizei levels,
                                         GLenum internalformat,
                                         GLsizei width,
                                         GLsizei height)
{
    if (target != TextureType::_2D && target != TextureType::CubeMap &&
        target != TextureType::Rectangle)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
        return false;
    }

    if (width < 1 || height < 1 || levels < 1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kTextureSizeTooSmall);
        return false;
    }

    if (target == TextureType::CubeMap && width != height)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kCubemapFacesEqualDimensions);
        return false;
    }

    if (levels != 1 && levels != log2(std::max(width, height)) + 1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidMipLevels);
        return false;
    }

    const InternalFormat &formatInfo = GetSizedInternalFormatInfo(internalformat);
    if (formatInfo.format == GL_NONE || formatInfo.type == GL_NONE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFormat);
        return false;
    }

    const Caps &caps = context->getCaps();

    switch (target)
    {
        case TextureType::_2D:
            if (width > caps.max2DTextureSize || height > caps.max2DTextureSize)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kResourceMaxTextureSize);
                return false;
            }
            break;
        case TextureType::Rectangle:
            if (levels != 1)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevel);
                return false;
            }

            if (width > caps.maxRectangleTextureSize || height > caps.maxRectangleTextureSize)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kResourceMaxTextureSize);
                return false;
            }
            if (formatInfo.compressed)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kRectangleTextureCompressed);
                return false;
            }
            break;
        case TextureType::CubeMap:
            if (width > caps.maxCubeMapTextureSize || height > caps.maxCubeMapTextureSize)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kResourceMaxTextureSize);
                return false;
            }
            break;
        case TextureType::InvalidEnum:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kEnumInvalid);
            return false;
        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, ToGLenum(target));
            return false;
    }

    if (levels != 1 && !context->getExtensions().textureNpotOES)
    {
        if (!isPow2(width) || !isPow2(height))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kDimensionsMustBePow2);
            return false;
        }
    }

    if (!formatInfo.textureSupport(context->getClientVersion(), context->getExtensions()))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFormat);
        return false;
    }

    // Even with OES_texture_npot, some compressed formats may impose extra restrictions.
    if (formatInfo.compressed)
    {
        if (!ValidCompressedImageSize(context, formatInfo.internalFormat, 0, width, height, 1))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidCompressedImageSize);
            return false;
        }
    }

    switch (internalformat)
    {
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT32_OES:
            switch (target)
            {
                case TextureType::_2D:
                    break;
                case TextureType::CubeMap:
                    if (!context->getExtensions().depthTextureCubeMapOES)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidTextureTarget);
                        return false;
                    }
                    break;
                default:
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidTextureTarget);
                    return false;
            }

            // ANGLE_depth_texture only supports 1-level textures
            if (!context->getExtensions().depthTextureOES)
            {
                if (levels != 1)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidMipLevels);
                    return false;
                }
            }
            break;
        case GL_DEPTH24_STENCIL8_OES:
            switch (target)
            {
                case TextureType::_2D:
                    break;
                case TextureType::CubeMap:
                    if (!context->getExtensions().depthTextureCubeMapOES)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidTextureTarget);
                        return false;
                    }
                    break;
                default:
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidTextureTarget);
                    return false;
            }

            if (!context->getExtensions().packedDepthStencilOES &&
                !context->getExtensions().depthTextureCubeMapOES)
            {
                // ANGLE_depth_texture only supports 1-level textures
                if (levels != 1)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidMipLevels);
                    return false;
                }
            }
            break;

        default:
            break;
    }

    Texture *texture = context->getTextureByType(target);
    if (!texture || texture->id().value == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMissingTexture);
        return false;
    }

    if (texture->getImmutableFormat())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTextureIsImmutable);
        return false;
    }

    return true;
}

bool ValidateDiscardFramebufferEXT(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   GLenum target,
                                   GLsizei numAttachments,
                                   const GLenum *attachments)
{
    if (!context->getExtensions().discardFramebufferEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    bool defaultFramebuffer = false;

    switch (target)
    {
        case GL_FRAMEBUFFER:
            defaultFramebuffer =
                (context->getState().getTargetFramebuffer(GL_FRAMEBUFFER)->isDefault());
            break;
        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFramebufferTarget);
            return false;
    }

    return ValidateDiscardFramebufferBase(context, entryPoint, target, numAttachments, attachments,
                                          defaultFramebuffer);
}

bool ValidateBindVertexArrayOES(const Context *context,
                                angle::EntryPoint entryPoint,
                                VertexArrayID array)
{
    if (!context->getExtensions().vertexArrayObjectOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateBindVertexArrayBase(context, entryPoint, array);
}

bool ValidateDeleteVertexArraysOES(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   GLsizei n,
                                   const VertexArrayID *arrays)
{
    if (!context->getExtensions().vertexArrayObjectOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateGenOrDelete(context, entryPoint, n);
}

bool ValidateGenVertexArraysOES(const Context *context,
                                angle::EntryPoint entryPoint,
                                GLsizei n,
                                const VertexArrayID *arrays)
{
    if (!context->getExtensions().vertexArrayObjectOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateGenOrDelete(context, entryPoint, n);
}

bool ValidateIsVertexArrayOES(const Context *context,
                              angle::EntryPoint entryPoint,
                              VertexArrayID array)
{
    if (!context->getExtensions().vertexArrayObjectOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return true;
}

bool ValidateProgramBinaryOES(const Context *context,
                              angle::EntryPoint entryPoint,
                              ShaderProgramID program,
                              GLenum binaryFormat,
                              const void *binary,
                              GLint length)
{
    if (!context->getExtensions().getProgramBinaryOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateProgramBinaryBase(context, entryPoint, program, binaryFormat, binary, length);
}

bool ValidateGetProgramBinaryOES(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 ShaderProgramID program,
                                 GLsizei bufSize,
                                 const GLsizei *length,
                                 const GLenum *binaryFormat,
                                 const void *binary)
{
    if (!context->getExtensions().getProgramBinaryOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateGetProgramBinaryBase(context, entryPoint, program, bufSize, length, binaryFormat,
                                        binary);
}

static bool ValidDebugSource(GLenum source, bool mustBeThirdPartyOrApplication)
{
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        case GL_DEBUG_SOURCE_OTHER:
            // Only THIRD_PARTY and APPLICATION sources are allowed to be manually inserted
            return !mustBeThirdPartyOrApplication;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
        case GL_DEBUG_SOURCE_APPLICATION:
            return true;

        default:
            return false;
    }
}

static bool ValidDebugType(GLenum type)
{
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        case GL_DEBUG_TYPE_PERFORMANCE:
        case GL_DEBUG_TYPE_PORTABILITY:
        case GL_DEBUG_TYPE_OTHER:
        case GL_DEBUG_TYPE_MARKER:
        case GL_DEBUG_TYPE_PUSH_GROUP:
        case GL_DEBUG_TYPE_POP_GROUP:
            return true;

        default:
            return false;
    }
}

static bool ValidDebugSeverity(GLenum severity)
{
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
        case GL_DEBUG_SEVERITY_MEDIUM:
        case GL_DEBUG_SEVERITY_LOW:
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            return true;

        default:
            return false;
    }
}

bool ValidateDebugMessageControlKHR(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    GLenum source,
                                    GLenum type,
                                    GLenum severity,
                                    GLsizei count,
                                    const GLuint *ids,
                                    GLboolean enabled)
{
    if (!context->getExtensions().debugKHR)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (!ValidDebugSource(source, false) && source != GL_DONT_CARE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidDebugSource);
        return false;
    }

    if (!ValidDebugType(type) && type != GL_DONT_CARE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidDebugType);
        return false;
    }

    if (!ValidDebugSeverity(severity) && severity != GL_DONT_CARE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidDebugSeverity);
        return false;
    }

    if (count > 0)
    {
        if (source == GL_DONT_CARE || type == GL_DONT_CARE)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidDebugSourceType);
            return false;
        }

        if (severity != GL_DONT_CARE)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidDebugSeverity);
            return false;
        }
    }

    return true;
}

bool ValidateDebugMessageInsertKHR(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   GLenum source,
                                   GLenum type,
                                   GLuint id,
                                   GLenum severity,
                                   GLsizei length,
                                   const GLchar *buf)
{
    if (!context->getExtensions().debugKHR)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (!context->getState().getDebug().isOutputEnabled())
    {
        // If the DEBUG_OUTPUT state is disabled calls to DebugMessageInsert are discarded and do
        // not generate an error.
        return false;
    }

    if (!ValidDebugSeverity(severity))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidDebugSource);
        return false;
    }

    if (!ValidDebugType(type))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidDebugType);
        return false;
    }

    if (!ValidDebugSource(source, true))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidDebugSource);
        return false;
    }

    size_t messageLength = (length < 0) ? strlen(buf) : length;
    if (messageLength > context->getCaps().maxDebugMessageLength)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsMaxDebugMessageLength);
        return false;
    }

    return true;
}

bool ValidateDebugMessageCallbackKHR(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     GLDEBUGPROCKHR callback,
                                     const void *userParam)
{
    if (!context->getExtensions().debugKHR)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return true;
}

bool ValidateGetDebugMessageLogKHR(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   GLuint count,
                                   GLsizei bufSize,
                                   const GLenum *sources,
                                   const GLenum *types,
                                   const GLuint *ids,
                                   const GLenum *severities,
                                   const GLsizei *lengths,
                                   const GLchar *messageLog)
{
    if (!context->getExtensions().debugKHR)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (bufSize < 0 && messageLog != nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeBufSize);
        return false;
    }

    return true;
}

bool ValidatePushDebugGroupBase(const Context *context,
                                angle::EntryPoint entryPoint,
                                GLenum source,
                                GLuint id,
                                GLsizei length,
                                const GLchar *message)
{
    if (!ValidDebugSource(source, true))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidDebugSource);
        return false;
    }

    size_t messageLength = (length < 0) ? strlen(message) : length;
    if (messageLength > context->getCaps().maxDebugMessageLength)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsMaxDebugMessageLength);
        return false;
    }

    size_t currentStackSize = context->getState().getDebug().getGroupStackDepth();
    if (currentStackSize >= context->getCaps().maxDebugGroupStackDepth)
    {
        ANGLE_VALIDATION_ERROR(GL_STACK_OVERFLOW, kExceedsMaxDebugGroupStackDepth);
        return false;
    }

    return true;
}

bool ValidatePopDebugGroupBase(const Context *context, angle::EntryPoint entryPoint)
{
    size_t currentStackSize = context->getState().getDebug().getGroupStackDepth();
    if (currentStackSize <= 1)
    {
        ANGLE_VALIDATION_ERROR(GL_STACK_UNDERFLOW, kCannotPopDefaultDebugGroup);
        return false;
    }

    return true;
}

bool ValidatePushDebugGroupKHR(const Context *context,
                               angle::EntryPoint entryPoint,
                               GLenum source,
                               GLuint id,
                               GLsizei length,
                               const GLchar *message)
{
    if (!context->getExtensions().debugKHR)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidatePushDebugGroupBase(context, entryPoint, source, id, length, message);
}

bool ValidatePopDebugGroupKHR(const Context *context, angle::EntryPoint entryPoint)
{
    if (!context->getExtensions().debugKHR)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidatePopDebugGroupBase(context, entryPoint);
}

static bool ValidateObjectIdentifierAndName(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            GLenum identifier,
                                            GLuint name)
{
    switch (identifier)
    {
        case GL_BUFFER:
            if (context->getBuffer({name}) == nullptr)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidBufferName);
                return false;
            }
            return true;

        case GL_SHADER:
            if (context->getShaderNoResolveCompile({name}) == nullptr)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidShaderName);
                return false;
            }
            return true;

        case GL_PROGRAM:
            if (context->getProgramNoResolveLink({name}) == nullptr)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidProgramName);
                return false;
            }
            return true;

        case GL_VERTEX_ARRAY:
            if (context->getVertexArray({name}) == nullptr)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidVertexArrayName);
                return false;
            }
            return true;

        case GL_QUERY:
            if (context->getQuery({name}) == nullptr)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidQueryName);
                return false;
            }
            return true;

        case GL_TRANSFORM_FEEDBACK:
            if (context->getTransformFeedback({name}) == nullptr)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidTransformFeedbackName);
                return false;
            }
            return true;

        case GL_SAMPLER:
            if (context->getSampler({name}) == nullptr)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidSamplerName);
                return false;
            }
            return true;

        case GL_TEXTURE:
            if (context->getTexture({name}) == nullptr)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidTextureName);
                return false;
            }
            return true;

        case GL_RENDERBUFFER:
            if (!context->isRenderbuffer({name}))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidRenderbufferName);
                return false;
            }
            return true;

        case GL_FRAMEBUFFER:
            if (context->getFramebuffer({name}) == nullptr)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidFramebufferName);
                return false;
            }
            return true;

        case GL_PROGRAM_PIPELINE:
            if (context->getProgramPipeline({name}) == nullptr)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidProgramPipelineName);
                return false;
            }
            return true;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidIndentifier);
            return false;
    }
}

static bool ValidateLabelLength(const Context *context,
                                angle::EntryPoint entryPoint,
                                GLsizei length,
                                const GLchar *label)
{
    size_t labelLength = 0;

    if (length < 0)
    {
        if (label != nullptr)
        {
            labelLength = strlen(label);
        }
    }
    else
    {
        labelLength = static_cast<size_t>(length);
    }

    if (labelLength > context->getCaps().maxLabelLength)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsMaxLabelLength);
        return false;
    }

    return true;
}

bool ValidateObjectLabelBase(const Context *context,
                             angle::EntryPoint entryPoint,
                             GLenum identifier,
                             GLuint name,
                             GLsizei length,
                             const GLchar *label)
{
    if (!ValidateObjectIdentifierAndName(context, entryPoint, identifier, name))
    {
        return false;
    }

    if (!ValidateLabelLength(context, entryPoint, length, label))
    {
        return false;
    }

    return true;
}

bool ValidateObjectLabelKHR(const Context *context,
                            angle::EntryPoint entryPoint,
                            GLenum identifier,
                            GLuint name,
                            GLsizei length,
                            const GLchar *label)
{
    if (!context->getExtensions().debugKHR)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (!ValidateObjectLabelBase(context, entryPoint, identifier, name, length, label))
    {
        return false;
    }

    return true;
}

bool ValidateGetObjectLabelBase(const Context *context,
                                angle::EntryPoint entryPoint,
                                GLenum identifier,
                                GLuint name,
                                GLsizei bufSize,
                                const GLsizei *length,
                                const GLchar *label)
{
    if (bufSize < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeBufSize);
        return false;
    }

    if (!ValidateObjectIdentifierAndName(context, entryPoint, identifier, name))
    {
        return false;
    }

    return true;
}

bool ValidateGetObjectLabelKHR(const Context *context,
                               angle::EntryPoint entryPoint,
                               GLenum identifier,
                               GLuint name,
                               GLsizei bufSize,
                               const GLsizei *length,
                               const GLchar *label)
{
    if (!context->getExtensions().debugKHR)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (!ValidateGetObjectLabelBase(context, entryPoint, identifier, name, bufSize, length, label))
    {
        return false;
    }

    return true;
}

static bool ValidateObjectPtrName(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  const void *ptr)
{
    if (!context->getSync({unsafe_pointer_to_int_cast<uint32_t>(ptr)}))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidSyncPointer);
        return false;
    }

    return true;
}

bool ValidateObjectPtrLabelBase(const Context *context,
                                angle::EntryPoint entryPoint,
                                const void *ptr,
                                GLsizei length,
                                const GLchar *label)
{
    if (!ValidateObjectPtrName(context, entryPoint, ptr))
    {
        return false;
    }

    if (!ValidateLabelLength(context, entryPoint, length, label))
    {
        return false;
    }

    return true;
}

bool ValidateObjectPtrLabelKHR(const Context *context,
                               angle::EntryPoint entryPoint,
                               const void *ptr,
                               GLsizei length,
                               const GLchar *label)
{
    if (!context->getExtensions().debugKHR)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (!ValidateObjectPtrLabelBase(context, entryPoint, ptr, length, label))
    {
        return false;
    }

    return true;
}

bool ValidateGetObjectPtrLabelBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   const void *ptr,
                                   GLsizei bufSize,
                                   const GLsizei *length,
                                   const GLchar *label)
{
    if (bufSize < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeBufSize);
        return false;
    }

    if (!ValidateObjectPtrName(context, entryPoint, ptr))
    {
        return false;
    }

    return true;
}

bool ValidateGetObjectPtrLabelKHR(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  const void *ptr,
                                  GLsizei bufSize,
                                  const GLsizei *length,
                                  const GLchar *label)
{
    if (!context->getExtensions().debugKHR)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (!ValidateGetObjectPtrLabelBase(context, entryPoint, ptr, bufSize, length, label))
    {
        return false;
    }

    return true;
}

bool ValidateGetPointervKHR(const Context *context,
                            angle::EntryPoint entryPoint,
                            GLenum pname,
                            void *const *params)
{
    if (!context->getExtensions().debugKHR)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateGetPointerv(context, entryPoint, pname, params);
}

bool ValidateGetPointervRobustANGLERobustANGLE(const Context *context,
                                               angle::EntryPoint entryPoint,
                                               GLenum pname,
                                               GLsizei bufSize,
                                               const GLsizei *length,
                                               void *const *params)
{
    UNIMPLEMENTED();
    return false;
}

bool ValidateBlitFramebufferANGLE(const Context *context,
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
    if (!context->getExtensions().framebufferBlitANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBlitExtensionNotAvailable);
        return false;
    }

    if (srcX1 - srcX0 != dstX1 - dstX0 || srcY1 - srcY0 != dstY1 - dstY0)
    {
        // TODO(jmadill): Determine if this should be available on other implementations.
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBlitExtensionScaleOrFlip);
        return false;
    }

    if (filter == GL_LINEAR)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kBlitExtensionLinear);
        return false;
    }

    Framebuffer *readFramebuffer = context->getState().getReadFramebuffer();
    Framebuffer *drawFramebuffer = context->getState().getDrawFramebuffer();

    if (mask & GL_COLOR_BUFFER_BIT)
    {
        const FramebufferAttachment *readColorAttachment =
            readFramebuffer->getReadColorAttachment();
        const FramebufferAttachment *drawColorAttachment =
            drawFramebuffer->getFirstColorAttachment();

        if (readColorAttachment && drawColorAttachment)
        {
            if (!(readColorAttachment->type() == GL_TEXTURE &&
                  (readColorAttachment->getTextureImageIndex().getType() == TextureType::_2D ||
                   readColorAttachment->getTextureImageIndex().getType() ==
                       TextureType::Rectangle)) &&
                readColorAttachment->type() != GL_RENDERBUFFER &&
                readColorAttachment->type() != GL_FRAMEBUFFER_DEFAULT)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                       kBlitExtensionFromInvalidAttachmentType);
                return false;
            }

            for (size_t drawbufferIdx = 0;
                 drawbufferIdx < drawFramebuffer->getDrawbufferStateCount(); ++drawbufferIdx)
            {
                const FramebufferAttachment *attachment =
                    drawFramebuffer->getDrawBuffer(drawbufferIdx);
                if (attachment)
                {
                    if (!(attachment->type() == GL_TEXTURE &&
                          (attachment->getTextureImageIndex().getType() == TextureType::_2D ||
                           attachment->getTextureImageIndex().getType() ==
                               TextureType::Rectangle)) &&
                        attachment->type() != GL_RENDERBUFFER &&
                        attachment->type() != GL_FRAMEBUFFER_DEFAULT)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                               kBlitExtensionToInvalidAttachmentType);
                        return false;
                    }

                    // Return an error if the destination formats do not match
                    if (!Format::EquivalentForBlit(attachment->getFormat(),
                                                   readColorAttachment->getFormat()))
                    {
                        ANGLE_VALIDATION_ERRORF(
                            GL_INVALID_OPERATION, kBlitExtensionFormatMismatch,
                            readColorAttachment->getFormat().info->sizedInternalFormat,
                            attachment->getFormat().info->sizedInternalFormat);
                        return false;
                    }
                }
            }

            GLint samples = readFramebuffer->getSamples(context);
            if (samples != 0 &&
                IsPartialBlit(context, readColorAttachment, drawColorAttachment, srcX0, srcY0,
                              srcX1, srcY1, dstX0, dstY0, dstX1, dstY1))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                       kBlitExtensionMultisampledWholeBufferBlit);
                return false;
            }
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
                if (IsPartialBlit(context, readBuffer, drawBuffer, srcX0, srcY0, srcX1, srcY1,
                                  dstX0, dstY0, dstX1, dstY1))
                {
                    // only whole-buffer copies are permitted
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                           kBlitExtensionDepthStencilWholeBufferBlit);
                    return false;
                }

                if (readBuffer->getResourceSamples() != 0 || drawBuffer->getResourceSamples() != 0)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                           kBlitExtensionMultisampledDepthOrStencil);
                    return false;
                }
            }
        }
    }

    return ValidateBlitFramebufferParameters(context, entryPoint, srcX0, srcY0, srcX1, srcY1, dstX0,
                                             dstY0, dstX1, dstY1, mask, filter);
}

bool ValidateBlitFramebufferNV(const Context *context,
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
    if (!context->getExtensions().framebufferBlitANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBlitExtensionNotAvailable);
        return false;
    }

    return ValidateBlitFramebufferParameters(context, entryPoint, srcX0, srcY0, srcX1, srcY1, dstX0,
                                             dstY0, dstX1, dstY1, mask, filter);
}

bool ValidateClear(const Context *context, angle::EntryPoint entryPoint, GLbitfield mask)
{
    Framebuffer *fbo             = context->getState().getDrawFramebuffer();
    const Extensions &extensions = context->getExtensions();

    if (!ValidateFramebufferComplete(context, entryPoint, fbo))
    {
        return false;
    }

    // The QCOM_framebuffer_foveated spec:
    if (fbo->isFoveationEnabled())
    {
        // INVALID_OPERATION is generated by any API call which causes a framebuffer
        // attachment to be written to if the framebuffer attachments have changed for
        // a foveated fbo.
        if (fbo->hasAnyAttachmentChanged())
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kFramebufferFoveationAttachmentChanged);
            return false;
        }
    }

    if ((mask & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidClearMask);
        return false;
    }

    if (extensions.webglCompatibilityANGLE && (mask & GL_COLOR_BUFFER_BIT) != 0)
    {
        if (GetIntOrUnsignedIntDrawBufferMask(fbo->getDrawBufferTypeMask()).any())
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNoDefinedClearConversion);
            return false;
        }
    }

    if (extensions.renderSharedExponentQCOM && (mask & GL_COLOR_BUFFER_BIT) != 0)
    {
        const State &state = context->getState();
        if (!ValidateColorMasksForSharedExponentColorBuffers(state.getBlendStateExt(),
                                                             state.getDrawFramebuffer()))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                   kUnsupportedColorMaskForSharedExponentColorBuffer);
            return false;
        }
    }

    if ((extensions.multiviewOVR || extensions.multiview2OVR) && extensions.disjointTimerQueryEXT)
    {
        const State &state       = context->getState();
        Framebuffer *framebuffer = state.getDrawFramebuffer();
        if (framebuffer->getNumViews() > 1 && state.isQueryActive(QueryType::TimeElapsed))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMultiviewTimerQuery);
            return false;
        }
    }

    return true;
}

bool ValidateDrawBuffersEXT(const Context *context,
                            angle::EntryPoint entryPoint,
                            GLsizei n,
                            const GLenum *bufs)
{
    if (!context->getExtensions().drawBuffersEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateDrawBuffersBase(context, entryPoint, n, bufs);
}

bool ValidateTexImage2D(const Context *context,
                        angle::EntryPoint entryPoint,
                        TextureTarget target,
                        GLint level,
                        GLint internalformat,
                        GLsizei width,
                        GLsizei height,
                        GLint border,
                        GLenum format,
                        GLenum type,
                        const void *pixels)
{
    if (context->getClientMajorVersion() < 3)
    {
        return ValidateES2TexImageParameters(context, entryPoint, target, level, internalformat,
                                             false, false, 0, 0, width, height, border, format,
                                             type, -1, pixels);
    }

    ASSERT(context->getClientMajorVersion() >= 3);
    return ValidateES3TexImage2DParameters(context, entryPoint, target, level, internalformat,
                                           false, false, 0, 0, 0, width, height, 1, border, format,
                                           type, -1, pixels);
}

bool ValidateTexImage2DRobustANGLE(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   TextureTarget target,
                                   GLint level,
                                   GLint internalformat,
                                   GLsizei width,
                                   GLsizei height,
                                   GLint border,
                                   GLenum format,
                                   GLenum type,
                                   GLsizei bufSize,
                                   const void *pixels)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    if (context->getClientMajorVersion() < 3)
    {
        return ValidateES2TexImageParameters(context, entryPoint, target, level, internalformat,
                                             false, false, 0, 0, width, height, border, format,
                                             type, bufSize, pixels);
    }

    ASSERT(context->getClientMajorVersion() >= 3);
    return ValidateES3TexImage2DParameters(context, entryPoint, target, level, internalformat,
                                           false, false, 0, 0, 0, width, height, 1, border, format,
                                           type, bufSize, pixels);
}

bool ValidateTexSubImage2D(const Context *context,
                           angle::EntryPoint entryPoint,
                           TextureTarget target,
                           GLint level,
                           GLint xoffset,
                           GLint yoffset,
                           GLsizei width,
                           GLsizei height,
                           GLenum format,
                           GLenum type,
                           const void *pixels)
{

    if (context->getClientMajorVersion() < 3)
    {
        return ValidateES2TexImageParameters(context, entryPoint, target, level, GL_NONE, false,
                                             true, xoffset, yoffset, width, height, 0, format, type,
                                             -1, pixels);
    }

    ASSERT(context->getClientMajorVersion() >= 3);
    return ValidateES3TexImage2DParameters(context, entryPoint, target, level, GL_NONE, false, true,
                                           xoffset, yoffset, 0, width, height, 1, 0, format, type,
                                           -1, pixels);
}

bool ValidateTexSubImage2DRobustANGLE(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      TextureTarget target,
                                      GLint level,
                                      GLint xoffset,
                                      GLint yoffset,
                                      GLsizei width,
                                      GLsizei height,
                                      GLenum format,
                                      GLenum type,
                                      GLsizei bufSize,
                                      const void *pixels)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    if (context->getClientMajorVersion() < 3)
    {
        return ValidateES2TexImageParameters(context, entryPoint, target, level, GL_NONE, false,
                                             true, xoffset, yoffset, width, height, 0, format, type,
                                             bufSize, pixels);
    }

    ASSERT(context->getClientMajorVersion() >= 3);
    return ValidateES3TexImage2DParameters(context, entryPoint, target, level, GL_NONE, false, true,
                                           xoffset, yoffset, 0, width, height, 1, 0, format, type,
                                           bufSize, pixels);
}

bool ValidateTexSubImage3DOES(const Context *context,
                              angle::EntryPoint entryPoint,
                              TextureTarget target,
                              GLint level,
                              GLint xoffset,
                              GLint yoffset,
                              GLint zoffset,
                              GLsizei width,
                              GLsizei height,
                              GLsizei depth,
                              GLenum format,
                              GLenum type,
                              const void *pixels)
{
    return ValidateTexSubImage3D(context, entryPoint, target, level, xoffset, yoffset, zoffset,
                                 width, height, depth, format, type, pixels);
}

bool ValidateCompressedTexImage2D(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  TextureTarget target,
                                  GLint level,
                                  GLenum internalformat,
                                  GLsizei width,
                                  GLsizei height,
                                  GLint border,
                                  GLsizei imageSize,
                                  const void *data)
{
    if (context->getClientMajorVersion() < 3)
    {
        if (!ValidateES2TexImageParameters(context, entryPoint, target, level, internalformat, true,
                                           false, 0, 0, width, height, border, GL_NONE, GL_NONE, -1,
                                           data))
        {
            return false;
        }
    }
    else
    {
        ASSERT(context->getClientMajorVersion() >= 3);
        if (!ValidateES3TexImage2DParameters(context, entryPoint, target, level, internalformat,
                                             true, false, 0, 0, 0, width, height, 1, border,
                                             GL_NONE, GL_NONE, -1, data))
        {
            return false;
        }
    }

    const InternalFormat &formatInfo = GetSizedInternalFormatInfo(internalformat);

    GLuint expectedImageSize = 0;
    if (!formatInfo.computeCompressedImageSize(Extents(width, height, 1), &expectedImageSize))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kIntegerOverflow);
        return false;
    }

    if (imageSize < 0 || static_cast<GLuint>(imageSize) != expectedImageSize)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kCompressedTextureDimensionsMustMatchData);
        return false;
    }

    if (target == TextureTarget::Rectangle)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kRectangleTextureCompressed);
        return false;
    }

    return true;
}

bool ValidateCompressedTexImage2DRobustANGLE(const Context *context,
                                             angle::EntryPoint entryPoint,
                                             TextureTarget target,
                                             GLint level,
                                             GLenum internalformat,
                                             GLsizei width,
                                             GLsizei height,
                                             GLint border,
                                             GLsizei imageSize,
                                             GLsizei dataSize,
                                             const void *data)
{
    if (!ValidateRobustCompressedTexImageBase(context, entryPoint, imageSize, dataSize))
    {
        return false;
    }

    return ValidateCompressedTexImage2D(context, entryPoint, target, level, internalformat, width,
                                        height, border, imageSize, data);
}

bool ValidateCompressedTexImage3DOES(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     TextureTarget target,
                                     GLint level,
                                     GLenum internalformat,
                                     GLsizei width,
                                     GLsizei height,
                                     GLsizei depth,
                                     GLint border,
                                     GLsizei imageSize,
                                     const void *data)
{
    return ValidateCompressedTexImage3D(context, entryPoint, target, level, internalformat, width,
                                        height, depth, border, imageSize, data);
}

bool ValidateCompressedTexSubImage2DRobustANGLE(const Context *context,
                                                angle::EntryPoint entryPoint,
                                                TextureTarget target,
                                                GLint level,
                                                GLint xoffset,
                                                GLint yoffset,
                                                GLsizei width,
                                                GLsizei height,
                                                GLenum format,
                                                GLsizei imageSize,
                                                GLsizei dataSize,
                                                const void *data)
{
    if (!ValidateRobustCompressedTexImageBase(context, entryPoint, imageSize, dataSize))
    {
        return false;
    }

    return ValidateCompressedTexSubImage2D(context, entryPoint, target, level, xoffset, yoffset,
                                           width, height, format, imageSize, data);
}

bool ValidateCompressedTexSubImage2D(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     TextureTarget target,
                                     GLint level,
                                     GLint xoffset,
                                     GLint yoffset,
                                     GLsizei width,
                                     GLsizei height,
                                     GLenum format,
                                     GLsizei imageSize,
                                     const void *data)
{
    if (context->getClientMajorVersion() < 3)
    {
        if (!ValidateES2TexImageParameters(context, entryPoint, target, level, GL_NONE, true, true,
                                           xoffset, yoffset, width, height, 0, format, GL_NONE, -1,
                                           data))
        {
            return false;
        }
    }
    else
    {
        ASSERT(context->getClientMajorVersion() >= 3);
        if (!ValidateES3TexImage2DParameters(context, entryPoint, target, level, GL_NONE, true,
                                             true, xoffset, yoffset, 0, width, height, 1, 0, format,
                                             GL_NONE, -1, data))
        {
            return false;
        }
    }

    const InternalFormat &formatInfo = GetSizedInternalFormatInfo(format);
    GLuint blockSize                 = 0;
    if (!formatInfo.computeCompressedImageSize(Extents(width, height, 1), &blockSize))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kIntegerOverflow);
        return false;
    }

    if (imageSize < 0 || static_cast<GLuint>(imageSize) != blockSize)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidCompressedImageSize);
        return false;
    }

    return true;
}

bool ValidateCompressedTexSubImage3DOES(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        TextureTarget target,
                                        GLint level,
                                        GLint xoffset,
                                        GLint yoffset,
                                        GLint zoffset,
                                        GLsizei width,
                                        GLsizei height,
                                        GLsizei depth,
                                        GLenum format,
                                        GLsizei imageSize,
                                        const void *data)
{
    return ValidateCompressedTexSubImage3D(context, entryPoint, target, level, xoffset, yoffset,
                                           zoffset, width, height, depth, format, imageSize, data);
}

bool ValidateGetBufferPointervOES(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  BufferBinding target,
                                  GLenum pname,
                                  void *const *params)
{
    if (!context->getExtensions().mapbufferOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateGetBufferPointervBase(context, entryPoint, target, pname, nullptr, params);
}

bool ValidateMapBufferOES(const Context *context,
                          angle::EntryPoint entryPoint,
                          BufferBinding target,
                          GLenum access)
{
    if (!context->getExtensions().mapbufferOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
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
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferNotMappable);
        return false;
    }

    if (access != GL_WRITE_ONLY_OES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidAccessBits);
        return false;
    }

    // Though there is no explicit mention of an interaction between GL_EXT_buffer_storage
    // and GL_OES_mapbuffer extension, allow it as long as the access type of glmapbufferOES
    // is compatible with the buffer's usage flags specified during glBufferStorageEXT
    if (buffer->isImmutable() && (buffer->getStorageExtUsageFlags() & GL_MAP_WRITE_BIT) == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferNotMappable);
        return false;
    }

    if (buffer->isMapped())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferAlreadyMapped);
        return false;
    }

    return ValidateMapBufferBase(context, entryPoint, target);
}

bool ValidateUnmapBufferOES(const Context *context,
                            angle::EntryPoint entryPoint,
                            BufferBinding target)
{
    if (!context->getExtensions().mapbufferOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateUnmapBufferBase(context, entryPoint, target);
}

bool ValidateMapBufferRangeEXT(const Context *context,
                               angle::EntryPoint entryPoint,
                               BufferBinding target,
                               GLintptr offset,
                               GLsizeiptr length,
                               GLbitfield access)
{
    if (!context->getExtensions().mapBufferRangeEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateMapBufferRangeBase(context, entryPoint, target, offset, length, access);
}

bool ValidateMapBufferBase(const Context *context,
                           angle::EntryPoint entryPoint,
                           BufferBinding target)
{
    Buffer *buffer = context->getState().getTargetBuffer(target);
    ASSERT(buffer != nullptr);

    // Check if this buffer is currently being used as a transform feedback output buffer
    if (context->getState().isTransformFeedbackActive())
    {
        TransformFeedback *transformFeedback = context->getState().getCurrentTransformFeedback();
        for (size_t i = 0; i < transformFeedback->getIndexedBufferCount(); i++)
        {
            const auto &transformFeedbackBuffer = transformFeedback->getIndexedBuffer(i);
            if (transformFeedbackBuffer.get() == buffer)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferBoundForTransformFeedback);
                return false;
            }
        }
    }

    if (buffer->hasWebGLXFBBindingConflict(context->isWebGL()))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferBoundForTransformFeedback);
        return false;
    }

    return true;
}

bool ValidateFlushMappedBufferRangeEXT(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       BufferBinding target,
                                       GLintptr offset,
                                       GLsizeiptr length)
{
    if (!context->getExtensions().mapBufferRangeEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateFlushMappedBufferRangeBase(context, entryPoint, target, offset, length);
}

bool ValidateBindUniformLocationCHROMIUM(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         ShaderProgramID program,
                                         UniformLocation location,
                                         const GLchar *name)
{
    if (!context->getExtensions().bindUniformLocationCHROMIUM)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (!programObject)
    {
        return false;
    }

    if (location.value < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeLocation);
        return false;
    }

    const Caps &caps = context->getCaps();
    if (static_cast<long>(location.value) >=
        (caps.maxVertexUniformVectors + caps.maxFragmentUniformVectors) * 4)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidBindUniformLocation);
        return false;
    }

    // The WebGL spec (section 6.20) disallows strings containing invalid ESSL characters for
    // shader-related entry points
    if (context->isWebGL() && !IsValidESSLString(name, strlen(name)))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidNameCharacters);
        return false;
    }

    if (strncmp(name, "gl_", 3) == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNameBeginsWithGL);
        return false;
    }

    return true;
}

bool ValidateCoverageModulationCHROMIUM(const PrivateState &state,
                                        ErrorSet *errors,
                                        angle::EntryPoint entryPoint,
                                        GLenum components)
{
    if (!state.getExtensions().framebufferMixedSamplesCHROMIUM)
    {
        errors->validationError(entryPoint, GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }
    switch (components)
    {
        case GL_RGB:
        case GL_RGBA:
        case GL_ALPHA:
        case GL_NONE:
            break;
        default:
            errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidCoverageComponents);
            return false;
    }

    return true;
}

bool ValidateCopyTextureCHROMIUM(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 TextureID sourceId,
                                 GLint sourceLevel,
                                 TextureTarget destTarget,
                                 TextureID destId,
                                 GLint destLevel,
                                 GLint internalFormat,
                                 GLenum destType,
                                 GLboolean unpackFlipY,
                                 GLboolean unpackPremultiplyAlpha,
                                 GLboolean unpackUnmultiplyAlpha)
{
    if (!context->getExtensions().copyTextureCHROMIUM)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    const Texture *source = context->getTexture(sourceId);
    if (source == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidSourceTexture);
        return false;
    }

    if (!IsValidCopyTextureSourceTarget(context, source->getType()))
    {
        ANGLE_VALIDATION_ERRORF(GL_INVALID_OPERATION, kInvalidInternalFormat, internalFormat);
        return false;
    }

    TextureType sourceType = source->getType();
    ASSERT(sourceType != TextureType::CubeMap);
    TextureTarget sourceTarget = NonCubeTextureTypeToTarget(sourceType);

    if (!IsValidCopyTextureSourceLevel(context, sourceType, sourceLevel))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidSourceTextureLevel);
        return false;
    }

    GLsizei sourceWidth  = static_cast<GLsizei>(source->getWidth(sourceTarget, sourceLevel));
    GLsizei sourceHeight = static_cast<GLsizei>(source->getHeight(sourceTarget, sourceLevel));
    if (sourceWidth == 0 || sourceHeight == 0)
    {
        ANGLE_VALIDATION_ERRORF(GL_INVALID_OPERATION, kInvalidInternalFormat, internalFormat);
        return false;
    }

    const InternalFormat &sourceFormat = *source->getFormat(sourceTarget, sourceLevel).info;
    if (!IsValidCopyTextureSourceInternalFormatEnum(sourceFormat.internalFormat))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidSourceTextureInternalFormat);
        return false;
    }

    if (!IsValidCopyTextureDestinationTargetEnum(context, destTarget))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
        return false;
    }

    const Texture *dest = context->getTexture(destId);
    if (dest == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidDestinationTexture);
        return false;
    }

    const InternalFormat &destInternalFormatInfo = GetInternalFormatInfo(internalFormat, destType);
    if (sourceType == TextureType::External && destInternalFormatInfo.isInt() &&
        !context->getExtensions().EGLImageExternalEssl3OES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kANGLECopyTextureMissingRequiredExtension);
        return false;
    }

    if (!IsValidCopyTextureDestinationTarget(context, dest->getType(), destTarget))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidDestinationTextureType);
        return false;
    }

    if (!IsValidCopyTextureDestinationLevel(context, entryPoint, dest->getType(), destLevel,
                                            sourceWidth, sourceHeight, false))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevel);
        return false;
    }

    if (!IsValidCopyTextureDestinationFormatType(context, entryPoint, internalFormat, destType))
    {
        return false;
    }

    if (dest->getType() == TextureType::CubeMap && sourceWidth != sourceHeight)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kCubemapFacesEqualDimensions);
        return false;
    }

    if (dest->getImmutableFormat())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kDestinationImmutable);
        return false;
    }

    return true;
}

bool ValidateCopySubTextureCHROMIUM(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    TextureID sourceId,
                                    GLint sourceLevel,
                                    TextureTarget destTarget,
                                    TextureID destId,
                                    GLint destLevel,
                                    GLint xoffset,
                                    GLint yoffset,
                                    GLint x,
                                    GLint y,
                                    GLsizei width,
                                    GLsizei height,
                                    GLboolean unpackFlipY,
                                    GLboolean unpackPremultiplyAlpha,
                                    GLboolean unpackUnmultiplyAlpha)
{
    if (!context->getExtensions().copyTextureCHROMIUM)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    const Texture *source = context->getTexture(sourceId);
    if (source == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidSourceTexture);
        return false;
    }

    if (!IsValidCopyTextureSourceTarget(context, source->getType()))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidSourceTextureType);
        return false;
    }

    TextureType sourceType = source->getType();
    ASSERT(sourceType != TextureType::CubeMap);
    TextureTarget sourceTarget = NonCubeTextureTypeToTarget(sourceType);

    if (!IsValidCopyTextureSourceLevel(context, sourceType, sourceLevel))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevel);
        return false;
    }

    if (source->getWidth(sourceTarget, sourceLevel) == 0 ||
        source->getHeight(sourceTarget, sourceLevel) == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidSourceTextureLevel);
        return false;
    }

    if (x < 0 || y < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeOffset);
        return false;
    }

    if (width < 0 || height < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeSize);
        return false;
    }

    if (static_cast<size_t>(x + width) > source->getWidth(sourceTarget, sourceLevel) ||
        static_cast<size_t>(y + height) > source->getHeight(sourceTarget, sourceLevel))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kSourceTextureTooSmall);
        return false;
    }

    const Format &sourceFormat = source->getFormat(sourceTarget, sourceLevel);
    if (!IsValidCopySubTextureSourceInternalFormat(sourceFormat.info->internalFormat))
    {
        ANGLE_VALIDATION_ERRORF(GL_INVALID_OPERATION, kInvalidInternalFormat,
                                sourceFormat.info->internalFormat);
        return false;
    }

    if (!IsValidCopyTextureDestinationTargetEnum(context, destTarget))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
        return false;
    }

    const Texture *dest = context->getTexture(destId);
    if (dest == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidDestinationTexture);
        return false;
    }

    if (!IsValidCopyTextureDestinationTarget(context, dest->getType(), destTarget))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidDestinationTextureType);
        return false;
    }

    if (!IsValidCopyTextureDestinationLevel(context, entryPoint, dest->getType(), destLevel, width,
                                            height, true))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevel);
        return false;
    }

    if (dest->getWidth(destTarget, destLevel) == 0 || dest->getHeight(destTarget, destLevel) == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kDestinationLevelNotDefined);
        return false;
    }

    const InternalFormat &destFormat = *dest->getFormat(destTarget, destLevel).info;
    if (!IsValidCopySubTextureDestionationInternalFormat(destFormat.internalFormat))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFormatCombination);
        return false;
    }

    if (sourceType == TextureType::External && destFormat.isInt() &&
        !context->getExtensions().EGLImageExternalEssl3OES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kANGLECopyTextureMissingRequiredExtension);
        return false;
    }

    if (xoffset < 0 || yoffset < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeOffset);
        return false;
    }

    if (static_cast<size_t>(xoffset + width) > dest->getWidth(destTarget, destLevel) ||
        static_cast<size_t>(yoffset + height) > dest->getHeight(destTarget, destLevel))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kOffsetOverflow);
        return false;
    }

    return true;
}

bool ValidateCompressedCopyTextureCHROMIUM(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           TextureID sourceId,
                                           TextureID destId)
{
    if (!context->getExtensions().copyCompressedTextureCHROMIUM)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    const Texture *source = context->getTexture(sourceId);
    if (source == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidSourceTexture);
        return false;
    }

    if (source->getType() != TextureType::_2D)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidSourceTextureType);
        return false;
    }

    if (source->getWidth(TextureTarget::_2D, 0) == 0 ||
        source->getHeight(TextureTarget::_2D, 0) == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kSourceTextureLevelZeroDefined);
        return false;
    }

    const Format &sourceFormat = source->getFormat(TextureTarget::_2D, 0);
    if (!sourceFormat.info->compressed)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kSourceTextureMustBeCompressed);
        return false;
    }

    const Texture *dest = context->getTexture(destId);
    if (dest == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidDestinationTexture);
        return false;
    }

    if (dest->getType() != TextureType::_2D)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidDestinationTextureType);
        return false;
    }

    if (dest->getImmutableFormat())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kDestinationImmutable);
        return false;
    }

    return true;
}

bool ValidateCreateShader(const Context *context, angle::EntryPoint entryPoint, ShaderType type)
{
    switch (type)
    {
        case ShaderType::Vertex:
        case ShaderType::Fragment:
            break;

        case ShaderType::Compute:
            if (context->getClientVersion() < ES_3_1)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kES31Required);
                return false;
            }
            break;

        case ShaderType::Geometry:
            if (!context->getExtensions().geometryShaderAny() &&
                context->getClientVersion() < ES_3_2)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidShaderType);
                return false;
            }
            break;

        case ShaderType::TessControl:
            if (!context->getExtensions().tessellationShaderAny() &&
                context->getClientVersion() < ES_3_2)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidShaderType);
                return false;
            }
            break;

        case ShaderType::TessEvaluation:
            if (!context->getExtensions().tessellationShaderAny() &&
                context->getClientVersion() < ES_3_2)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidShaderType);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidShaderType);
            return false;
    }

    return true;
}

bool ValidateBufferData(const Context *context,
                        angle::EntryPoint entryPoint,
                        BufferBinding target,
                        GLsizeiptr size,
                        const void *data,
                        BufferUsage usage)
{
    if (size < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeSize);
        return false;
    }

    switch (usage)
    {
        case BufferUsage::StreamDraw:
        case BufferUsage::StaticDraw:
        case BufferUsage::DynamicDraw:
            break;

        case BufferUsage::StreamRead:
        case BufferUsage::StaticRead:
        case BufferUsage::DynamicRead:
        case BufferUsage::StreamCopy:
        case BufferUsage::StaticCopy:
        case BufferUsage::DynamicCopy:
            if (context->getClientMajorVersion() < 3)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidBufferUsage);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidBufferUsage);
            return false;
    }

    if (!context->isValidBufferBinding(target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidBufferTypes);
        return false;
    }

    Buffer *buffer = context->getState().getTargetBuffer(target);

    if (!buffer)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferNotBound);
        return false;
    }

    if (buffer->hasWebGLXFBBindingConflict(context->isWebGL()))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferBoundForTransformFeedback);
        return false;
    }

    if (buffer->isImmutable())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferImmutable);
        return false;
    }

    return true;
}

bool ValidateBufferSubData(const Context *context,
                           angle::EntryPoint entryPoint,
                           BufferBinding target,
                           GLintptr offset,
                           GLsizeiptr size,
                           const void *data)
{
    if (size < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeSize);
        return false;
    }

    if (offset < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeOffset);
        return false;
    }

    if (!context->isValidBufferBinding(target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidBufferTypes);
        return false;
    }

    Buffer *buffer = context->getState().getTargetBuffer(target);

    if (!buffer)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferNotBound);
        return false;
    }

    // EXT_buffer_storage allows persistently mapped buffers to be updated via glBufferSubData
    bool isPersistent = (buffer->getAccessFlags() & GL_MAP_PERSISTENT_BIT_EXT) != 0;

    // Verify that buffer is not currently mapped unless persistent
    if (buffer->isMapped() && !isPersistent)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferMapped);
        return false;
    }

    if (buffer->hasWebGLXFBBindingConflict(context->isWebGL()))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferBoundForTransformFeedback);
        return false;
    }

    if (buffer->isImmutable() &&
        (buffer->getStorageExtUsageFlags() & GL_DYNAMIC_STORAGE_BIT_EXT) == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferNotUpdatable);
        return false;
    }

    // Check for possible overflow of size + offset
    angle::CheckedNumeric<decltype(size + offset)> checkedSize(size);
    checkedSize += offset;
    if (!checkedSize.IsValid())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kParamOverflow);
        return false;
    }

    if (size + offset > buffer->getSize())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInsufficientBufferSize);
        return false;
    }

    return true;
}

bool ValidateRequestExtensionANGLE(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   const GLchar *name)
{
    if (!context->getExtensions().requestExtensionANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (!context->isExtensionRequestable(name))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotRequestable);
        return false;
    }

    return true;
}

bool ValidateDisableExtensionANGLE(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   const GLchar *name)
{
    if (!context->getExtensions().requestExtensionANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (!context->isExtensionDisablable(name))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotDisablable);
        return false;
    }

    return true;
}

bool ValidateActiveTexture(const PrivateState &state,
                           ErrorSet *errors,
                           angle::EntryPoint entryPoint,
                           GLenum texture)
{
    if (state.getClientMajorVersion() < 2)
    {
        return ValidateMultitextureUnit(state, errors, entryPoint, texture);
    }

    if (texture < GL_TEXTURE0 ||
        texture >
            GL_TEXTURE0 + static_cast<GLuint>(state.getCaps().maxCombinedTextureImageUnits) - 1)
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidCombinedImageUnit);
        return false;
    }

    return true;
}

bool ValidateAttachShader(const Context *context,
                          angle::EntryPoint entryPoint,
                          ShaderProgramID program,
                          ShaderProgramID shader)
{
    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (!programObject)
    {
        return false;
    }

    Shader *shaderObject = GetValidShader(context, entryPoint, shader);
    if (!shaderObject)
    {
        return false;
    }

    if (programObject->getAttachedShader(shaderObject->getType()))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kShaderAttachmentHasShader);
        return false;
    }

    return true;
}

bool ValidateBindAttribLocation(const Context *context,
                                angle::EntryPoint entryPoint,
                                ShaderProgramID program,
                                GLuint index,
                                const GLchar *name)
{
    if (index >= static_cast<GLuint>(context->getCaps().maxVertexAttributes))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxVertexAttribute);
        return false;
    }

    if (strncmp(name, "gl_", 3) == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNameBeginsWithGL);
        return false;
    }

    if (context->isWebGL())
    {
        const size_t length = strlen(name);

        if (!IsValidESSLString(name, length))
        {
            // The WebGL spec (section 6.20) disallows strings containing invalid ESSL characters
            // for shader-related entry points
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidNameCharacters);
            return false;
        }

        if (!ValidateWebGLNameLength(context, entryPoint, length) ||
            !ValidateWebGLNamePrefix(context, entryPoint, name))
        {
            return false;
        }
    }

    return GetValidProgram(context, entryPoint, program) != nullptr;
}

bool ValidateBindFramebuffer(const Context *context,
                             angle::EntryPoint entryPoint,
                             GLenum target,
                             FramebufferID framebuffer)
{
    return ValidateBindFramebufferBase(context, entryPoint, target, framebuffer);
}

bool ValidateBindRenderbuffer(const Context *context,
                              angle::EntryPoint entryPoint,
                              GLenum target,
                              RenderbufferID renderbuffer)
{
    return ValidateBindRenderbufferBase(context, entryPoint, target, renderbuffer);
}

static bool ValidBlendEquationMode(const PrivateState &state, ErrorSet *errors, GLenum mode)
{
    switch (mode)
    {
        case GL_FUNC_ADD:
        case GL_FUNC_SUBTRACT:
        case GL_FUNC_REVERSE_SUBTRACT:
            return true;

        case GL_MIN:
        case GL_MAX:
            return state.getClientVersion() >= ES_3_0 || state.getExtensions().blendMinmaxEXT;

        default:
            return false;
    }
}

static bool ValidAdvancedBlendEquationMode(const PrivateState &state, GLenum mode)
{
    switch (mode)
    {
        case GL_MULTIPLY_KHR:
        case GL_SCREEN_KHR:
        case GL_OVERLAY_KHR:
        case GL_DARKEN_KHR:
        case GL_LIGHTEN_KHR:
        case GL_COLORDODGE_KHR:
        case GL_COLORBURN_KHR:
        case GL_HARDLIGHT_KHR:
        case GL_SOFTLIGHT_KHR:
        case GL_DIFFERENCE_KHR:
        case GL_EXCLUSION_KHR:
        case GL_HSL_HUE_KHR:
        case GL_HSL_SATURATION_KHR:
        case GL_HSL_COLOR_KHR:
        case GL_HSL_LUMINOSITY_KHR:
            return state.getClientVersion() >= ES_3_2 ||
                   state.getExtensions().blendEquationAdvancedKHR;

        default:
            return false;
    }
}

bool ValidateBlendColor(const PrivateState &state,
                        ErrorSet *errors,
                        angle::EntryPoint entryPoint,
                        GLfloat red,
                        GLfloat green,
                        GLfloat blue,
                        GLfloat alpha)
{
    return true;
}

bool ValidateBlendEquation(const PrivateState &state,
                           ErrorSet *errors,
                           angle::EntryPoint entryPoint,
                           GLenum mode)
{
    if (ValidBlendEquationMode(state, errors, mode))
    {
        return true;
    }

    if (ValidAdvancedBlendEquationMode(state, mode))
    {
        if (state.getPixelLocalStorageActivePlanes() != 0)
        {
            // INVALID_OPERATION is generated by BlendEquation*() if <mode> is one of the advanced
            // blend equations defined in KHR_blend_equation_advanced.
            errors->validationError(entryPoint, GL_INVALID_OPERATION,
                                    kPLSAdvancedBlendNotSupported);
            return false;
        }
        return true;
    }

    errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidBlendEquation);
    return false;
}

bool ValidateBlendEquationSeparate(const PrivateState &state,
                                   ErrorSet *errors,
                                   angle::EntryPoint entryPoint,
                                   GLenum modeRGB,
                                   GLenum modeAlpha)
{
    if (!ValidBlendEquationMode(state, errors, modeRGB))
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidBlendEquation);
        return false;
    }

    if (!ValidBlendEquationMode(state, errors, modeAlpha))
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidBlendEquation);
        return false;
    }

    return true;
}

bool ValidateBlendFunc(const PrivateState &state,
                       ErrorSet *errors,
                       angle::EntryPoint entryPoint,
                       GLenum sfactor,
                       GLenum dfactor)
{
    return ValidateBlendFuncSeparate(state, errors, entryPoint, sfactor, dfactor, sfactor, dfactor);
}

bool ValidateBlendFuncSeparate(const PrivateState &state,
                               ErrorSet *errors,
                               angle::EntryPoint entryPoint,
                               GLenum srcRGB,
                               GLenum dstRGB,
                               GLenum srcAlpha,
                               GLenum dstAlpha)
{
    if (!ValidateSrcBlendFunc(state, errors, entryPoint, srcRGB) ||
        !ValidateDstBlendFunc(state, errors, entryPoint, dstRGB) ||
        !ValidateSrcBlendFunc(state, errors, entryPoint, srcAlpha) ||
        !ValidateDstBlendFunc(state, errors, entryPoint, dstAlpha))
    {
        return false;
    }

    if (state.getLimitations().noSimultaneousConstantColorAndAlphaBlendFunc || state.isWebGL())
    {
        bool constantColorUsed =
            (srcRGB == GL_CONSTANT_COLOR || srcRGB == GL_ONE_MINUS_CONSTANT_COLOR ||
             dstRGB == GL_CONSTANT_COLOR || dstRGB == GL_ONE_MINUS_CONSTANT_COLOR);

        bool constantAlphaUsed =
            (srcRGB == GL_CONSTANT_ALPHA || srcRGB == GL_ONE_MINUS_CONSTANT_ALPHA ||
             dstRGB == GL_CONSTANT_ALPHA || dstRGB == GL_ONE_MINUS_CONSTANT_ALPHA);

        if (constantColorUsed && constantAlphaUsed)
        {
            if (state.isWebGL())
            {
                errors->validationError(entryPoint, GL_INVALID_OPERATION, kInvalidConstantColor);
                return false;
            }

            WARN() << kConstantColorAlphaLimitation;
            errors->validationError(entryPoint, GL_INVALID_OPERATION,
                                    kConstantColorAlphaLimitation);
            return false;
        }
    }

    return true;
}

bool ValidateGetString(const Context *context, angle::EntryPoint entryPoint, GLenum name)
{
    switch (name)
    {
        case GL_VENDOR:
        case GL_RENDERER:
        case GL_VERSION:
        case GL_SHADING_LANGUAGE_VERSION:
        case GL_EXTENSIONS:
            break;

        case GL_REQUESTABLE_EXTENSIONS_ANGLE:
            if (!context->getExtensions().requestExtensionANGLE)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidName);
                return false;
            }
            break;

        case GL_SERIALIZED_CONTEXT_STRING_ANGLE:
            if (!context->getExtensions().getSerializedContextStringANGLE)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidName);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidName);
            return false;
    }

    return true;
}

bool ValidateLineWidth(const PrivateState &state,
                       ErrorSet *errors,
                       angle::EntryPoint entryPoint,
                       GLfloat width)
{
    if (width <= 0.0f || isNaN(width))
    {
        errors->validationError(entryPoint, GL_INVALID_VALUE, kInvalidWidth);
        return false;
    }

    return true;
}

bool ValidateDepthRangef(const PrivateState &state,
                         ErrorSet *errors,
                         angle::EntryPoint entryPoint,
                         GLfloat zNear,
                         GLfloat zFar)
{
    if (state.isWebGL() && zNear > zFar)
    {
        errors->validationError(entryPoint, GL_INVALID_OPERATION, kInvalidDepthRange);
        return false;
    }

    return true;
}

bool ValidateRenderbufferStorage(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 GLenum target,
                                 GLenum internalformat,
                                 GLsizei width,
                                 GLsizei height)
{
    return ValidateRenderbufferStorageParametersBase(context, entryPoint, target, 0, internalformat,
                                                     width, height);
}

bool ValidateRenderbufferStorageMultisampleANGLE(const Context *context,
                                                 angle::EntryPoint entryPoint,
                                                 GLenum target,
                                                 GLsizei samples,
                                                 GLenum internalformat,
                                                 GLsizei width,
                                                 GLsizei height)
{
    if (!context->getExtensions().framebufferMultisampleANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    // ANGLE_framebuffer_multisample states that the value of samples must be less than or equal
    // to MAX_SAMPLES_ANGLE (Context::getCaps().maxSamples) otherwise GL_INVALID_VALUE is
    // generated.
    if (samples > context->getCaps().maxSamples)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kSamplesOutOfRange);
        return false;
    }

    // ANGLE_framebuffer_multisample states GL_OUT_OF_MEMORY is generated on a failure to create
    // the specified storage. This is different than ES 3.0 in which a sample number higher
    // than the maximum sample number supported by this format generates a GL_INVALID_VALUE.
    // The TextureCaps::getMaxSamples method is only guarenteed to be valid when the context is ES3.
    if (context->getClientMajorVersion() >= 3)
    {
        const TextureCaps &formatCaps = context->getTextureCaps().get(internalformat);
        if (static_cast<GLuint>(samples) > formatCaps.getMaxSamples())
        {
            ANGLE_VALIDATION_ERROR(GL_OUT_OF_MEMORY, kSamplesOutOfRange);
            return false;
        }
    }

    return ValidateRenderbufferStorageParametersBase(context, entryPoint, target, samples,
                                                     internalformat, width, height);
}

bool ValidateCheckFramebufferStatus(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    GLenum target)
{
    if (!ValidFramebufferTarget(context, target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFramebufferTarget);
        return false;
    }

    return true;
}

bool ValidateClearColor(const PrivateState &state,
                        ErrorSet *errors,
                        angle::EntryPoint entryPoint,
                        GLfloat red,
                        GLfloat green,
                        GLfloat blue,
                        GLfloat alpha)
{
    return true;
}

bool ValidateClearDepthf(const PrivateState &state,
                         ErrorSet *errors,
                         angle::EntryPoint entryPoint,
                         GLfloat depth)
{
    return true;
}

bool ValidateClearStencil(const PrivateState &state,
                          ErrorSet *errors,
                          angle::EntryPoint entryPoint,
                          GLint s)
{
    return true;
}

bool ValidateColorMask(const PrivateState &state,
                       ErrorSet *errors,
                       angle::EntryPoint entryPoint,
                       GLboolean red,
                       GLboolean green,
                       GLboolean blue,
                       GLboolean alpha)
{
    return true;
}

bool ValidateCompileShader(const Context *context,
                           angle::EntryPoint entryPoint,
                           ShaderProgramID shader)
{
    return true;
}

bool ValidateCreateProgram(const Context *context, angle::EntryPoint entryPoint)
{
    return true;
}

bool ValidateCullFace(const PrivateState &state,
                      ErrorSet *errors,
                      angle::EntryPoint entryPoint,
                      CullFaceMode mode)
{
    switch (mode)
    {
        case CullFaceMode::Front:
        case CullFaceMode::Back:
        case CullFaceMode::FrontAndBack:
            break;

        default:
            errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidCullMode);
            return false;
    }

    return true;
}

bool ValidateDeleteProgram(const Context *context,
                           angle::EntryPoint entryPoint,
                           ShaderProgramID program)
{
    if (program.value == 0)
    {
        return false;
    }

    if (!context->getProgramResolveLink(program))
    {
        if (context->getShaderNoResolveCompile(program))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExpectedProgramName);
            return false;
        }
        else
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidProgramName);
            return false;
        }
    }

    return true;
}

bool ValidateDeleteShader(const Context *context,
                          angle::EntryPoint entryPoint,
                          ShaderProgramID shader)
{
    if (shader.value == 0)
    {
        return false;
    }

    if (!context->getShaderNoResolveCompile(shader))
    {
        if (context->getProgramResolveLink(shader))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidShaderName);
            return false;
        }
        else
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExpectedShaderName);
            return false;
        }
    }

    return true;
}

bool ValidateDepthFunc(const PrivateState &state,
                       ErrorSet *errors,
                       angle::EntryPoint entryPoint,
                       GLenum func)
{
    switch (func)
    {
        case GL_NEVER:
        case GL_ALWAYS:
        case GL_LESS:
        case GL_LEQUAL:
        case GL_EQUAL:
        case GL_GREATER:
        case GL_GEQUAL:
        case GL_NOTEQUAL:
            break;

        default:
            errors->validationErrorF(entryPoint, GL_INVALID_ENUM, kEnumNotSupported, func);
            return false;
    }

    return true;
}

bool ValidateDepthMask(const PrivateState &state,
                       ErrorSet *errors,
                       angle::EntryPoint entryPoint,
                       GLboolean flag)
{
    return true;
}

bool ValidateDetachShader(const Context *context,
                          angle::EntryPoint entryPoint,
                          ShaderProgramID program,
                          ShaderProgramID shader)
{
    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (!programObject)
    {
        return false;
    }

    Shader *shaderObject = GetValidShader(context, entryPoint, shader);
    if (!shaderObject)
    {
        return false;
    }

    const Shader *attachedShader = programObject->getAttachedShader(shaderObject->getType());
    if (attachedShader != shaderObject)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kShaderToDetachMustBeAttached);
        return false;
    }

    return true;
}

bool ValidateDisableVertexAttribArray(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      GLuint index)
{
    if (index >= static_cast<GLuint>(context->getCaps().maxVertexAttributes))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxVertexAttribute);
        return false;
    }

    return true;
}

bool ValidateEnableVertexAttribArray(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     GLuint index)
{
    if (index >= static_cast<GLuint>(context->getCaps().maxVertexAttributes))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxVertexAttribute);
        return false;
    }

    return true;
}

bool ValidateFinish(const Context *context, angle::EntryPoint entryPoint)
{
    return true;
}

bool ValidateFlush(const Context *context, angle::EntryPoint entryPoint)
{
    return true;
}

bool ValidateFrontFace(const PrivateState &state,
                       ErrorSet *errors,
                       angle::EntryPoint entryPoint,
                       GLenum mode)
{
    switch (mode)
    {
        case GL_CW:
        case GL_CCW:
            break;
        default:
            errors->validationErrorF(entryPoint, GL_INVALID_ENUM, kEnumNotSupported, mode);
            return false;
    }

    return true;
}

bool ValidateGetActiveAttrib(const Context *context,
                             angle::EntryPoint entryPoint,
                             ShaderProgramID program,
                             GLuint index,
                             GLsizei bufsize,
                             const GLsizei *length,
                             const GLint *size,
                             const GLenum *type,
                             const GLchar *name)
{
    if (bufsize < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeBufSize);
        return false;
    }

    Program *programObject = GetValidProgram(context, entryPoint, program);

    if (!programObject)
    {
        return false;
    }

    if (index >= static_cast<GLuint>(programObject->getExecutable().getProgramInputs().size()))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxActiveUniform);
        return false;
    }

    return true;
}

bool ValidateGetActiveUniform(const Context *context,
                              angle::EntryPoint entryPoint,
                              ShaderProgramID program,
                              GLuint index,
                              GLsizei bufsize,
                              const GLsizei *length,
                              const GLint *size,
                              const GLenum *type,
                              const GLchar *name)
{
    if (bufsize < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeBufSize);
        return false;
    }

    Program *programObject = GetValidProgram(context, entryPoint, program);

    if (!programObject)
    {
        return false;
    }

    if (index >= programObject->getExecutable().getUniforms().size())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxActiveUniform);
        return false;
    }

    return true;
}

bool ValidateGetAttachedShaders(const Context *context,
                                angle::EntryPoint entryPoint,
                                ShaderProgramID program,
                                GLsizei maxcount,
                                const GLsizei *count,
                                const ShaderProgramID *shaders)
{
    if (maxcount < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeMaxCount);
        return false;
    }

    Program *programObject = GetValidProgram(context, entryPoint, program);

    if (!programObject)
    {
        return false;
    }

    return true;
}

bool ValidateGetAttribLocation(const Context *context,
                               angle::EntryPoint entryPoint,
                               ShaderProgramID program,
                               const GLchar *name)
{
    if (strncmp(name, "gl_", 3) == 0)
    {
        return false;
    }

    if (context->isWebGL())
    {
        const size_t length = strlen(name);

        if (!IsValidESSLString(name, length))
        {
            // The WebGL spec (section 6.20) disallows strings containing invalid ESSL characters
            // for shader-related entry points
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidNameCharacters);
            return false;
        }

        if (!ValidateWebGLNameLength(context, entryPoint, length) ||
            strncmp(name, "webgl_", 6) == 0 || strncmp(name, "_webgl_", 7) == 0)
        {
            return false;
        }
    }

    Program *programObject = GetValidProgram(context, entryPoint, program);

    if (!programObject)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kProgramNotBound);
        return false;
    }

    if (!programObject->isLinked())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kProgramNotLinked);
        return false;
    }

    return true;
}

bool ValidateGetBooleanv(const Context *context,
                         angle::EntryPoint entryPoint,
                         GLenum pname,
                         const GLboolean *params)
{
    GLenum nativeType;
    unsigned int numParams = 0;
    return ValidateStateQuery(context, entryPoint, pname, &nativeType, &numParams);
}

bool ValidateGetError(const Context *context, angle::EntryPoint entryPoint)
{
    return true;
}

bool ValidateGetFloatv(const Context *context,
                       angle::EntryPoint entryPoint,
                       GLenum pname,
                       const GLfloat *params)
{
    GLenum nativeType;
    unsigned int numParams = 0;
    return ValidateStateQuery(context, entryPoint, pname, &nativeType, &numParams);
}

bool ValidateGetIntegerv(const Context *context,
                         angle::EntryPoint entryPoint,
                         GLenum pname,
                         const GLint *params)
{
    GLenum nativeType;
    unsigned int numParams = 0;
    return ValidateStateQuery(context, entryPoint, pname, &nativeType, &numParams);
}

bool ValidateGetProgramInfoLog(const Context *context,
                               angle::EntryPoint entryPoint,
                               ShaderProgramID program,
                               GLsizei bufsize,
                               const GLsizei *length,
                               const GLchar *infolog)
{
    if (bufsize < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeBufSize);
        return false;
    }

    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (!programObject)
    {
        return false;
    }

    return true;
}

bool ValidateGetShaderInfoLog(const Context *context,
                              angle::EntryPoint entryPoint,
                              ShaderProgramID shader,
                              GLsizei bufsize,
                              const GLsizei *length,
                              const GLchar *infolog)
{
    if (bufsize < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeBufSize);
        return false;
    }

    Shader *shaderObject = GetValidShader(context, entryPoint, shader);
    if (!shaderObject)
    {
        return false;
    }

    return true;
}

bool ValidateGetShaderPrecisionFormat(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      GLenum shadertype,
                                      GLenum precisiontype,
                                      const GLint *range,
                                      const GLint *precision)
{
    switch (shadertype)
    {
        case GL_VERTEX_SHADER:
        case GL_FRAGMENT_SHADER:
            break;
        case GL_COMPUTE_SHADER:
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kUnimplementedComputeShaderPrecision);
            return false;
        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidShaderType);
            return false;
    }

    switch (precisiontype)
    {
        case GL_LOW_FLOAT:
        case GL_MEDIUM_FLOAT:
        case GL_HIGH_FLOAT:
        case GL_LOW_INT:
        case GL_MEDIUM_INT:
        case GL_HIGH_INT:
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidPrecision);
            return false;
    }

    return true;
}

bool ValidateGetShaderSource(const Context *context,
                             angle::EntryPoint entryPoint,
                             ShaderProgramID shader,
                             GLsizei bufsize,
                             const GLsizei *length,
                             const GLchar *source)
{
    if (bufsize < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeBufSize);
        return false;
    }

    Shader *shaderObject = GetValidShader(context, entryPoint, shader);
    if (!shaderObject)
    {
        return false;
    }

    return true;
}

bool ValidateGetUniformLocation(const Context *context,
                                angle::EntryPoint entryPoint,
                                ShaderProgramID program,
                                const GLchar *name)
{
    if (strstr(name, "gl_") == name)
    {
        return false;
    }

    // The WebGL spec (section 6.20) disallows strings containing invalid ESSL characters for
    // shader-related entry points
    if (context->isWebGL() && !IsValidESSLString(name, strlen(name)))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidNameCharacters);
        return false;
    }

    Program *programObject = GetValidProgram(context, entryPoint, program);

    if (!programObject)
    {
        return false;
    }

    if (!programObject->isLinked())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kProgramNotLinked);
        return false;
    }

    return true;
}

bool ValidateHint(const PrivateState &state,
                  ErrorSet *errors,
                  angle::EntryPoint entryPoint,
                  GLenum target,
                  GLenum mode)
{
    switch (mode)
    {
        case GL_FASTEST:
        case GL_NICEST:
        case GL_DONT_CARE:
            break;

        default:
            errors->validationErrorF(entryPoint, GL_INVALID_ENUM, kEnumNotSupported, mode);
            return false;
    }

    switch (target)
    {
        case GL_GENERATE_MIPMAP_HINT:
            break;

        case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
            if (state.getClientVersion() < ES_3_0 && !state.getExtensions().standardDerivativesOES)
            {
                errors->validationErrorF(entryPoint, GL_INVALID_ENUM, kEnumNotSupported, target);
                return false;
            }
            break;

        case GL_PERSPECTIVE_CORRECTION_HINT:
        case GL_POINT_SMOOTH_HINT:
        case GL_LINE_SMOOTH_HINT:
        case GL_FOG_HINT:
            if (state.getClientMajorVersion() >= 2)
            {
                errors->validationErrorF(entryPoint, GL_INVALID_ENUM, kEnumNotSupported, target);
                return false;
            }
            break;

        default:
            errors->validationErrorF(entryPoint, GL_INVALID_ENUM, kEnumNotSupported, target);
            return false;
    }

    return true;
}

bool ValidateIsBuffer(const Context *context, angle::EntryPoint entryPoint, BufferID buffer)
{
    return true;
}

bool ValidateIsFramebuffer(const Context *context,
                           angle::EntryPoint entryPoint,
                           FramebufferID framebuffer)
{
    return true;
}

bool ValidateIsProgram(const Context *context,
                       angle::EntryPoint entryPoint,
                       ShaderProgramID program)
{
    return true;
}

bool ValidateIsRenderbuffer(const Context *context,
                            angle::EntryPoint entryPoint,
                            RenderbufferID renderbuffer)
{
    return true;
}

bool ValidateIsShader(const Context *context, angle::EntryPoint entryPoint, ShaderProgramID shader)
{
    return true;
}

bool ValidateIsTexture(const Context *context, angle::EntryPoint entryPoint, TextureID texture)
{
    return true;
}

bool ValidatePixelStorei(const PrivateState &state,
                         ErrorSet *errors,
                         angle::EntryPoint entryPoint,
                         GLenum pname,
                         GLint param)
{
    if (state.getClientMajorVersion() < 3)
    {
        switch (pname)
        {
            case GL_UNPACK_IMAGE_HEIGHT:
            case GL_UNPACK_SKIP_IMAGES:
                errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidPname);
                return false;

            case GL_UNPACK_ROW_LENGTH:
            case GL_UNPACK_SKIP_ROWS:
            case GL_UNPACK_SKIP_PIXELS:
                if (!state.getExtensions().unpackSubimageEXT)
                {
                    errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidPname);
                    return false;
                }
                break;

            case GL_PACK_ROW_LENGTH:
            case GL_PACK_SKIP_ROWS:
            case GL_PACK_SKIP_PIXELS:
                if (!state.getExtensions().packSubimageNV)
                {
                    errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidPname);
                    return false;
                }
                break;
        }
    }

    if (param < 0)
    {
        errors->validationError(entryPoint, GL_INVALID_VALUE, kNegativeParam);
        return false;
    }

    switch (pname)
    {
        case GL_UNPACK_ALIGNMENT:
            if (param != 1 && param != 2 && param != 4 && param != 8)
            {
                errors->validationError(entryPoint, GL_INVALID_VALUE, kInvalidUnpackAlignment);
                return false;
            }
            break;

        case GL_PACK_ALIGNMENT:
            if (param != 1 && param != 2 && param != 4 && param != 8)
            {
                errors->validationError(entryPoint, GL_INVALID_VALUE, kInvalidUnpackAlignment);
                return false;
            }
            break;

        case GL_PACK_REVERSE_ROW_ORDER_ANGLE:
            if (!state.getExtensions().packReverseRowOrderANGLE)
            {
                errors->validationErrorF(entryPoint, GL_INVALID_ENUM, kEnumNotSupported, pname);
            }
            break;

        case GL_UNPACK_ROW_LENGTH:
        case GL_UNPACK_IMAGE_HEIGHT:
        case GL_UNPACK_SKIP_IMAGES:
        case GL_UNPACK_SKIP_ROWS:
        case GL_UNPACK_SKIP_PIXELS:
        case GL_PACK_ROW_LENGTH:
        case GL_PACK_SKIP_ROWS:
        case GL_PACK_SKIP_PIXELS:
            break;

        default:
            errors->validationErrorF(entryPoint, GL_INVALID_ENUM, kEnumNotSupported, pname);
            return false;
    }

    return true;
}

bool ValidatePolygonOffset(const PrivateState &state,
                           ErrorSet *errors,
                           angle::EntryPoint entryPoint,
                           GLfloat factor,
                           GLfloat units)
{
    return true;
}

bool ValidateReleaseShaderCompiler(const Context *context, angle::EntryPoint entryPoint)
{
    return true;
}

bool ValidateSampleCoverage(const PrivateState &state,
                            ErrorSet *errors,
                            angle::EntryPoint entryPoint,
                            GLfloat value,
                            GLboolean invert)
{
    return true;
}

bool ValidateScissor(const PrivateState &state,
                     ErrorSet *errors,
                     angle::EntryPoint entryPoint,
                     GLint x,
                     GLint y,
                     GLsizei width,
                     GLsizei height)
{
    if (width < 0 || height < 0)
    {
        errors->validationError(entryPoint, GL_INVALID_VALUE, kNegativeSize);
        return false;
    }

    return true;
}

bool ValidateShaderBinary(const Context *context,
                          angle::EntryPoint entryPoint,
                          GLsizei n,
                          const ShaderProgramID *shaders,
                          GLenum binaryformat,
                          const void *binary,
                          GLsizei length)
{
    const std::vector<GLenum> &shaderBinaryFormats = context->getCaps().shaderBinaryFormats;
    if (std::find(shaderBinaryFormats.begin(), shaderBinaryFormats.end(), binaryformat) ==
        shaderBinaryFormats.end())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidShaderBinaryFormat);
        return false;
    }

    ASSERT(binaryformat == GL_SHADER_BINARY_ANGLE);

    if (n <= 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidShaderCount);
        return false;
    }

    if (length < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeLength);
        return false;
    }

    // GL_SHADER_BINARY_ANGLE shader binaries contain a single shader.
    if (n > 1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidShaderCount);
        return false;
    }

    Shader *shaderObject = GetValidShader(context, entryPoint, shaders[0]);
    if (!shaderObject)
    {
        return false;
    }

    // Check ANGLE version used to generate binary matches the current version.
    BinaryInputStream stream(binary, length);
    std::vector<uint8_t> versionString(angle::GetANGLEShaderProgramVersionHashSize(), 0);
    stream.readBytes(versionString.data(), versionString.size());
    if (memcmp(versionString.data(), angle::GetANGLEShaderProgramVersion(), versionString.size()) !=
        0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidShaderBinary);
        return false;
    }

    // Check that the shader type of the binary matches the type of target shader.
    gl::ShaderType shaderType;
    stream.readEnum(&shaderType);
    if (shaderObject->getType() != shaderType)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedShaderBinaryType);
        return false;
    }

    return true;
}

bool ValidateShaderSource(const Context *context,
                          angle::EntryPoint entryPoint,
                          ShaderProgramID shader,
                          GLsizei count,
                          const GLchar *const *string,
                          const GLint *length)
{
    if (count < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeCount);
        return false;
    }

    Shader *shaderObject = GetValidShader(context, entryPoint, shader);
    if (!shaderObject)
    {
        return false;
    }

    return true;
}

bool ValidateStencilFunc(const PrivateState &state,
                         ErrorSet *errors,
                         angle::EntryPoint entryPoint,
                         GLenum func,
                         GLint ref,
                         GLuint mask)
{
    if (!IsValidStencilFunc(func))
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidStencil);
        return false;
    }

    return true;
}

bool ValidateStencilFuncSeparate(const PrivateState &state,
                                 ErrorSet *errors,
                                 angle::EntryPoint entryPoint,
                                 GLenum face,
                                 GLenum func,
                                 GLint ref,
                                 GLuint mask)
{
    if (!IsValidStencilFace(face))
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidStencil);
        return false;
    }

    if (!IsValidStencilFunc(func))
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidStencil);
        return false;
    }

    return true;
}

bool ValidateStencilMask(const PrivateState &state,
                         ErrorSet *errors,
                         angle::EntryPoint entryPoint,
                         GLuint mask)
{
    return true;
}

bool ValidateStencilMaskSeparate(const PrivateState &state,
                                 ErrorSet *errors,
                                 angle::EntryPoint entryPoint,
                                 GLenum face,
                                 GLuint mask)
{
    if (!IsValidStencilFace(face))
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidStencil);
        return false;
    }

    return true;
}

bool ValidateStencilOp(const PrivateState &state,
                       ErrorSet *errors,
                       angle::EntryPoint entryPoint,
                       GLenum fail,
                       GLenum zfail,
                       GLenum zpass)
{
    if (!IsValidStencilOp(fail))
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidStencil);
        return false;
    }

    if (!IsValidStencilOp(zfail))
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidStencil);
        return false;
    }

    if (!IsValidStencilOp(zpass))
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidStencil);
        return false;
    }

    return true;
}

bool ValidateStencilOpSeparate(const PrivateState &state,
                               ErrorSet *errors,
                               angle::EntryPoint entryPoint,
                               GLenum face,
                               GLenum fail,
                               GLenum zfail,
                               GLenum zpass)
{
    if (!IsValidStencilFace(face))
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidStencil);
        return false;
    }

    return ValidateStencilOp(state, errors, entryPoint, fail, zfail, zpass);
}

bool ValidateValidateProgram(const Context *context,
                             angle::EntryPoint entryPoint,
                             ShaderProgramID program)
{
    Program *programObject = GetValidProgram(context, entryPoint, program);

    if (!programObject)
    {
        return false;
    }

    return true;
}

bool ValidateViewport(const PrivateState &state,
                      ErrorSet *errors,
                      angle::EntryPoint entryPoint,
                      GLint x,
                      GLint y,
                      GLsizei width,
                      GLsizei height)
{
    if (width < 0 || height < 0)
    {
        errors->validationError(entryPoint, GL_INVALID_VALUE, kViewportNegativeSize);
        return false;
    }

    return true;
}

bool ValidateGetFramebufferAttachmentParameteriv(const Context *context,
                                                 angle::EntryPoint entryPoint,
                                                 GLenum target,
                                                 GLenum attachment,
                                                 GLenum pname,
                                                 const GLint *params)
{
    return ValidateGetFramebufferAttachmentParameterivBase(context, entryPoint, target, attachment,
                                                           pname, nullptr);
}

bool ValidateGetProgramiv(const Context *context,
                          angle::EntryPoint entryPoint,
                          ShaderProgramID program,
                          GLenum pname,
                          const GLint *params)
{
    return ValidateGetProgramivBase(context, entryPoint, program, pname, nullptr);
}

bool ValidateCopyTexImage2D(const Context *context,
                            angle::EntryPoint entryPoint,
                            TextureTarget target,
                            GLint level,
                            GLenum internalformat,
                            GLint x,
                            GLint y,
                            GLsizei width,
                            GLsizei height,
                            GLint border)
{
    if (context->getClientMajorVersion() < 3)
    {
        return ValidateES2CopyTexImageParameters(context, entryPoint, target, level, internalformat,
                                                 false, 0, 0, x, y, width, height, border);
    }

    ASSERT(context->getClientMajorVersion() == 3);
    return ValidateES3CopyTexImage2DParameters(context, entryPoint, target, level, internalformat,
                                               false, 0, 0, 0, x, y, width, height, border);
}

bool ValidateCopyTexSubImage2D(const Context *context,
                               angle::EntryPoint entryPoint,
                               TextureTarget target,
                               GLint level,
                               GLint xoffset,
                               GLint yoffset,
                               GLint x,
                               GLint y,
                               GLsizei width,
                               GLsizei height)
{
    if (context->getClientMajorVersion() < 3)
    {
        return ValidateES2CopyTexImageParameters(context, entryPoint, target, level, GL_NONE, true,
                                                 xoffset, yoffset, x, y, width, height, 0);
    }

    return ValidateES3CopyTexImage2DParameters(context, entryPoint, target, level, GL_NONE, true,
                                               xoffset, yoffset, 0, x, y, width, height, 0);
}

bool ValidateCopyTexSubImage3DOES(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  TextureTarget target,
                                  GLint level,
                                  GLint xoffset,
                                  GLint yoffset,
                                  GLint zoffset,
                                  GLint x,
                                  GLint y,
                                  GLsizei width,
                                  GLsizei height)
{
    return ValidateCopyTexSubImage3D(context, entryPoint, target, level, xoffset, yoffset, zoffset,
                                     x, y, width, height);
}

bool ValidateDeleteBuffers(const Context *context,
                           angle::EntryPoint entryPoint,
                           GLint n,
                           const BufferID *buffers)
{
    return ValidateGenOrDelete(context, entryPoint, n);
}

bool ValidateDeleteFramebuffers(const Context *context,
                                angle::EntryPoint entryPoint,
                                GLint n,
                                const FramebufferID *framebuffers)
{
    return ValidateGenOrDelete(context, entryPoint, n);
}

bool ValidateDeleteRenderbuffers(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 GLint n,
                                 const RenderbufferID *renderbuffers)
{
    return ValidateGenOrDelete(context, entryPoint, n);
}

bool ValidateDeleteTextures(const Context *context,
                            angle::EntryPoint entryPoint,
                            GLint n,
                            const TextureID *textures)
{
    return ValidateGenOrDelete(context, entryPoint, n);
}

bool ValidateDisable(const PrivateState &state,
                     ErrorSet *errors,
                     angle::EntryPoint entryPoint,
                     GLenum cap)
{
    if (!ValidCap(state, errors, cap, false))
    {
        errors->validationErrorF(entryPoint, GL_INVALID_ENUM, kEnumNotSupported, cap);
        return false;
    }

    if (state.getPixelLocalStorageActivePlanes() != 0)
    {
        if (IsCapBannedWithActivePLS(cap))
        {
            errors->validationErrorF(entryPoint, GL_INVALID_OPERATION, kPLSCapNotAllowed, cap);
            return false;
        }
    }

    return true;
}

bool ValidateEnable(const PrivateState &state,
                    ErrorSet *errors,
                    angle::EntryPoint entryPoint,
                    GLenum cap)
{
    if (!ValidCap(state, errors, cap, false))
    {
        errors->validationErrorF(entryPoint, GL_INVALID_ENUM, kEnumNotSupported, cap);
        return false;
    }

    if (state.getLimitations().noSampleAlphaToCoverageSupport && cap == GL_SAMPLE_ALPHA_TO_COVERAGE)
    {
        errors->validationError(entryPoint, GL_INVALID_OPERATION,
                                kNoSampleAlphaToCoveragesLimitation);

        // We also output an error message to the debugger window if tracing is active, so that
        // developers can see the error message.
        ERR() << kNoSampleAlphaToCoveragesLimitation;
        return false;
    }

    if (state.getPixelLocalStorageActivePlanes() != 0)
    {
        if (IsCapBannedWithActivePLS(cap))
        {
            errors->validationErrorF(entryPoint, GL_INVALID_OPERATION, kPLSCapNotAllowed, cap);
            return false;
        }
    }

    return true;
}

bool ValidateFramebufferRenderbuffer(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     GLenum target,
                                     GLenum attachment,
                                     GLenum renderbuffertarget,
                                     RenderbufferID renderbuffer)
{
    return ValidateFramebufferRenderbufferBase(context, entryPoint, target, attachment,
                                               renderbuffertarget, renderbuffer);
}

bool ValidateFramebufferTexture2D(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  GLenum target,
                                  GLenum attachment,
                                  TextureTarget textarget,
                                  TextureID texture,
                                  GLint level)
{
    // Attachments are required to be bound to level 0 without ES3 or the GL_OES_fbo_render_mipmap
    // extension
    if (context->getClientMajorVersion() < 3 && !context->getExtensions().fboRenderMipmapOES &&
        level != 0)
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

            case TextureTarget::Rectangle:
            {
                if (level != 0)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevel);
                    return false;
                }
                if (tex->getType() != TextureType::Rectangle)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTextureTargetMismatch);
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

            case TextureTarget::_2DMultisample:
            {
                if (context->getClientVersion() < ES_3_1 &&
                    !context->getExtensions().textureMultisampleANGLE)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                           kMultisampleTextureExtensionOrES31Required);
                    return false;
                }

                if (level != 0)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kLevelNotZero);
                    return false;
                }
                if (tex->getType() != TextureType::_2DMultisample)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTextureTargetMismatch);
                    return false;
                }
            }
            break;

            case TextureTarget::External:
            {
                if (!context->getExtensions().YUVTargetEXT)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kYUVTargetExtensionRequired);
                    return false;
                }

                if (attachment != GL_COLOR_ATTACHMENT0)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidAttachment);
                    return false;
                }

                if (tex->getType() != TextureType::External)
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

bool ValidateFramebufferTexture3DOES(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     GLenum target,
                                     GLenum attachment,
                                     TextureTarget textargetPacked,
                                     TextureID texture,
                                     GLint level,
                                     GLint zoffset)
{
    // We don't call into a base ValidateFramebufferTexture3D here because
    // it doesn't exist for OpenGL ES. This function is replaced by
    // FramebufferTextureLayer in ES 3.x, which has broader support.
    if (!context->getExtensions().texture3DOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    // Attachments are required to be bound to level 0 without ES3 or the
    // GL_OES_fbo_render_mipmap extension
    if (context->getClientMajorVersion() < 3 && !context->getExtensions().fboRenderMipmapOES &&
        level != 0)
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

        switch (textargetPacked)
        {
            case TextureTarget::_3D:
            {
                if (level > log2(caps.max3DTextureSize))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevel);
                    return false;
                }
                if (zoffset >= caps.max3DTextureSize)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidZOffset);
                    return false;
                }
                if (tex->getType() != TextureType::_3D)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidTextureType);
                    return false;
                }
            }
            break;

            default:
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidTextureTarget);
                return false;
        }
    }

    return true;
}

bool ValidateIsEnabled(const PrivateState &state,
                       ErrorSet *errors,
                       angle::EntryPoint entryPoint,
                       GLenum cap)
{
    if (!ValidCap(state, errors, cap, true))
    {
        errors->validationErrorF(entryPoint, GL_INVALID_ENUM, kEnumNotSupported, cap);
        return false;
    }

    return true;
}

bool ValidateLinkProgram(const Context *context,
                         angle::EntryPoint entryPoint,
                         ShaderProgramID program)
{
    if (context->hasActiveTransformFeedback(program))
    {
        // ES 3.0.4 section 2.15 page 91
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTransformFeedbackActiveDuringLink);
        return false;
    }

    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (!programObject)
    {
        return false;
    }

    return true;
}

bool ValidateUseProgram(const Context *context,
                        angle::EntryPoint entryPoint,
                        ShaderProgramID program)
{
    if (program.value != 0)
    {
        Program *programObject = context->getProgramResolveLink(program);
        if (!programObject)
        {
            // ES 3.1.0 section 7.3 page 72
            if (context->getShaderNoResolveCompile(program))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExpectedProgramName);
                return false;
            }
            else
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidProgramName);
                return false;
            }
        }
        if (!programObject->isLinked())
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kProgramNotLinked);
            return false;
        }
    }
    if (context->getState().isTransformFeedbackActiveUnpaused())
    {
        // ES 3.0.4 section 2.15 page 91
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTransformFeedbackUseProgram);
        return false;
    }

    return true;
}

bool ValidateDeleteFencesNV(const Context *context,
                            angle::EntryPoint entryPoint,
                            GLsizei n,
                            const FenceNVID *fences)
{
    if (!context->getExtensions().fenceNV)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNVFenceNotSupported);
        return false;
    }

    if (n < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeCount);
        return false;
    }

    return true;
}

bool ValidateFinishFenceNV(const Context *context, angle::EntryPoint entryPoint, FenceNVID fence)
{
    if (!context->getExtensions().fenceNV)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNVFenceNotSupported);
        return false;
    }

    FenceNV *fenceObject = context->getFenceNV(fence);

    if (fenceObject == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFence);
        return false;
    }

    if (!fenceObject->isSet())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFenceState);
        return false;
    }

    return true;
}

bool ValidateGenFencesNV(const Context *context,
                         angle::EntryPoint entryPoint,
                         GLsizei n,
                         const FenceNVID *fences)
{
    if (!context->getExtensions().fenceNV)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNVFenceNotSupported);
        return false;
    }

    if (n < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeCount);
        return false;
    }

    return true;
}

bool ValidateGetFenceivNV(const Context *context,
                          angle::EntryPoint entryPoint,
                          FenceNVID fence,
                          GLenum pname,
                          const GLint *params)
{
    if (!context->getExtensions().fenceNV)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNVFenceNotSupported);
        return false;
    }

    FenceNV *fenceObject = context->getFenceNV(fence);

    if (fenceObject == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFence);
        return false;
    }

    if (!fenceObject->isSet())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFenceState);
        return false;
    }

    switch (pname)
    {
        case GL_FENCE_STATUS_NV:
        case GL_FENCE_CONDITION_NV:
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidPname);
            return false;
    }

    return true;
}

bool ValidateGetGraphicsResetStatusEXT(const Context *context, angle::EntryPoint entryPoint)
{
    if (!context->getExtensions().robustnessEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return true;
}

bool ValidateGetTranslatedShaderSourceANGLE(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            ShaderProgramID shader,
                                            GLsizei bufsize,
                                            const GLsizei *length,
                                            const GLchar *source)
{
    if (!context->getExtensions().translatedShaderSourceANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (bufsize < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeBufSize);
        return false;
    }

    Shader *shaderObject = context->getShaderNoResolveCompile(shader);

    if (!shaderObject)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidShaderName);
        return false;
    }

    return true;
}

bool ValidateIsFenceNV(const Context *context, angle::EntryPoint entryPoint, FenceNVID fence)
{
    if (!context->getExtensions().fenceNV)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNVFenceNotSupported);
        return false;
    }

    return true;
}

bool ValidateSetFenceNV(const Context *context,
                        angle::EntryPoint entryPoint,
                        FenceNVID fence,
                        GLenum condition)
{
    if (!context->getExtensions().fenceNV)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNVFenceNotSupported);
        return false;
    }

    if (condition != GL_ALL_COMPLETED_NV)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFenceCondition);
        return false;
    }

    FenceNV *fenceObject = context->getFenceNV(fence);

    if (fenceObject == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFence);
        return false;
    }

    return true;
}

bool ValidateTestFenceNV(const Context *context, angle::EntryPoint entryPoint, FenceNVID fence)
{
    if (!context->getExtensions().fenceNV)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNVFenceNotSupported);
        return false;
    }

    FenceNV *fenceObject = context->getFenceNV(fence);

    if (fenceObject == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFence);
        return false;
    }

    if (fenceObject->isSet() != GL_TRUE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFenceState);
        return false;
    }

    return true;
}

bool ValidateTexStorage2DEXT(const Context *context,
                             angle::EntryPoint entryPoint,
                             TextureType type,
                             GLsizei levels,
                             GLenum internalformat,
                             GLsizei width,
                             GLsizei height)
{
    if (!context->getExtensions().textureStorageEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (context->getClientMajorVersion() < 3)
    {
        return ValidateES2TexStorageParametersBase(context, entryPoint, type, levels,
                                                   internalformat, width, height);
    }

    ASSERT(context->getClientMajorVersion() >= 3);
    return ValidateES3TexStorage2DParameters(context, entryPoint, type, levels, internalformat,
                                             width, height, 1);
}

bool ValidateVertexAttribDivisorANGLE(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      GLuint index,
                                      GLuint divisor)
{
    if (!context->getExtensions().instancedArraysANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (index >= static_cast<GLuint>(context->getCaps().maxVertexAttributes))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxVertexAttribute);
        return false;
    }

    if (context->getLimitations().attributeZeroRequiresZeroDivisorInEXT)
    {
        if (index == 0 && divisor != 0)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kAttributeZeroRequiresDivisorLimitation);

            // We also output an error message to the debugger window if tracing is active, so
            // that developers can see the error message.
            ERR() << kAttributeZeroRequiresDivisorLimitation;
            return false;
        }
    }

    return true;
}

bool ValidateVertexAttribDivisorEXT(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    GLuint index,
                                    GLuint divisor)
{
    if (!context->getExtensions().instancedArraysEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (index >= static_cast<GLuint>(context->getCaps().maxVertexAttributes))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxVertexAttribute);
        return false;
    }

    return true;
}

bool ValidateTexImage3DOES(const Context *context,
                           angle::EntryPoint entryPoint,
                           TextureTarget target,
                           GLint level,
                           GLenum internalformat,
                           GLsizei width,
                           GLsizei height,
                           GLsizei depth,
                           GLint border,
                           GLenum format,
                           GLenum type,
                           const void *pixels)
{
    return ValidateTexImage3D(context, entryPoint, target, level, internalformat, width, height,
                              depth, border, format, type, pixels);
}

bool ValidatePopGroupMarkerEXT(const Context *context, angle::EntryPoint entryPoint)
{
    if (!context->getExtensions().debugMarkerEXT)
    {
        // The debug marker calls should not set error state
        // However, it seems reasonable to set an error state if the extension is not enabled
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return true;
}

bool ValidateTexStorage1DEXT(const Context *context,
                             angle::EntryPoint entryPoint,
                             GLenum target,
                             GLsizei levels,
                             GLenum internalformat,
                             GLsizei width)
{
    UNIMPLEMENTED();
    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
    return false;
}

bool ValidateTexStorage3DEXT(const Context *context,
                             angle::EntryPoint entryPoint,
                             TextureType target,
                             GLsizei levels,
                             GLenum internalformat,
                             GLsizei width,
                             GLsizei height,
                             GLsizei depth)
{
    if (!context->getExtensions().textureStorageEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateES3TexStorage3DParameters(context, entryPoint, target, levels, internalformat,
                                             width, height, depth);
}

bool ValidateMaxShaderCompilerThreadsKHR(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         GLuint count)
{
    if (!context->getExtensions().parallelShaderCompileKHR)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }
    return true;
}

bool ValidateMultiDrawArraysANGLE(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  PrimitiveMode mode,
                                  const GLint *firsts,
                                  const GLsizei *counts,
                                  GLsizei drawcount)
{
    if (!context->getExtensions().multiDrawANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }
    for (GLsizei drawID = 0; drawID < drawcount; ++drawID)
    {
        if (!ValidateDrawArrays(context, entryPoint, mode, firsts[drawID], counts[drawID]))
        {
            return false;
        }
    }
    return true;
}

bool ValidateMultiDrawElementsANGLE(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    PrimitiveMode mode,
                                    const GLsizei *counts,
                                    DrawElementsType type,
                                    const GLvoid *const *indices,
                                    GLsizei drawcount)
{
    if (!context->getExtensions().multiDrawANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }
    for (GLsizei drawID = 0; drawID < drawcount; ++drawID)
    {
        if (!ValidateDrawElements(context, entryPoint, mode, counts[drawID], type, indices[drawID]))
        {
            return false;
        }
    }
    return true;
}

bool ValidateFramebufferTexture2DMultisampleEXT(const Context *context,
                                                angle::EntryPoint entryPoint,
                                                GLenum target,
                                                GLenum attachment,
                                                TextureTarget textarget,
                                                TextureID texture,
                                                GLint level,
                                                GLsizei samples)
{
    if (!context->getExtensions().multisampledRenderToTextureEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (samples < 0)
    {
        return false;
    }

    // EXT_multisampled_render_to_texture states that the value of samples
    // must be less than or equal to MAX_SAMPLES_EXT (Context::getCaps().maxSamples)
    // otherwise GL_INVALID_VALUE is generated.
    if (samples > context->getCaps().maxSamples)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kSamplesOutOfRange);
        return false;
    }

    if (!ValidateFramebufferTextureBase(context, entryPoint, target, attachment, texture, level))
    {
        return false;
    }

    // EXT_multisampled_render_to_texture returns INVALID_OPERATION when a sample number higher than
    // the maximum sample number supported by this format is passed.
    // The TextureCaps::getMaxSamples method is only guarenteed to be valid when the context is ES3.
    if (texture.value != 0 && context->getClientMajorVersion() >= 3)
    {
        Texture *tex                  = context->getTexture(texture);
        GLenum sizedInternalFormat    = tex->getFormat(textarget, level).info->sizedInternalFormat;
        const TextureCaps &formatCaps = context->getTextureCaps().get(sizedInternalFormat);
        if (static_cast<GLuint>(samples) > formatCaps.getMaxSamples())
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kSamplesOutOfRange);
            return false;
        }
    }

    // Unless EXT_multisampled_render_to_texture2 is enabled, only color attachment 0 can be used.
    if (!context->getExtensions().multisampledRenderToTexture2EXT &&
        attachment != GL_COLOR_ATTACHMENT0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidAttachment);
        return false;
    }

    if (!ValidTexture2DDestinationTarget(context, textarget))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
        return false;
    }

    return true;
}

bool ValidateRenderbufferStorageMultisampleEXT(const Context *context,
                                               angle::EntryPoint entryPoint,
                                               GLenum target,
                                               GLsizei samples,
                                               GLenum internalformat,
                                               GLsizei width,
                                               GLsizei height)
{
    if (!context->getExtensions().multisampledRenderToTextureEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }
    if (!ValidateRenderbufferStorageParametersBase(context, entryPoint, target, samples,
                                                   internalformat, width, height))
    {
        return false;
    }

    // EXT_multisampled_render_to_texture states that the value of samples
    // must be less than or equal to MAX_SAMPLES_EXT (Context::getCaps().maxSamples)
    // otherwise GL_INVALID_VALUE is generated.
    if (samples > context->getCaps().maxSamples)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kSamplesOutOfRange);
        return false;
    }

    // EXT_multisampled_render_to_texture returns GL_OUT_OF_MEMORY on failure to create
    // the specified storage. This is different than ES 3.0 in which a sample number higher
    // than the maximum sample number supported by this format generates a GL_INVALID_VALUE.
    // The TextureCaps::getMaxSamples method is only guarenteed to be valid when the context is ES3.
    if (context->getClientMajorVersion() >= 3)
    {
        const TextureCaps &formatCaps = context->getTextureCaps().get(internalformat);
        if (static_cast<GLuint>(samples) > formatCaps.getMaxSamples())
        {
            ANGLE_VALIDATION_ERROR(GL_OUT_OF_MEMORY, kSamplesOutOfRange);
            return false;
        }
    }

    return true;
}

bool ValidateBlobCacheCallbacksANGLE(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     GLSETBLOBPROCANGLE set,
                                     GLGETBLOBPROCANGLE get,
                                     const void *userParam)
{
    if (!context->getExtensions().blobCacheANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if ((get == nullptr) != (set == nullptr))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBlobCacheCallbacksUnbalanced);
        return false;
    }

    return true;
}

bool ValidateGetPointervANGLE(const Context *context,
                              angle::EntryPoint entryPoint,
                              GLenum pname,
                              void *const *params)
{
    if (!context->getExtensions().blobCacheANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateGetPointerv(context, entryPoint, pname, params);
}

void RecordBindTextureTypeError(const Context *context,
                                angle::EntryPoint entryPoint,
                                TextureType target)
{
    ASSERT(!context->getStateCache().isValidBindTextureType(target));

    switch (target)
    {
        case TextureType::Rectangle:
            ASSERT(!context->getExtensions().textureRectangleANGLE);
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kTextureRectangleNotSupported);
            break;

        case TextureType::_3D:
        case TextureType::_2DArray:
            ASSERT(context->getClientMajorVersion() < 3);
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kES3Required);
            break;

        case TextureType::_2DMultisample:
            ASSERT(context->getClientVersion() < Version(3, 1) &&
                   !context->getExtensions().textureMultisampleANGLE);
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kMultisampleTextureExtensionOrES31Required);
            break;

        case TextureType::_2DMultisampleArray:
            ASSERT(context->getClientVersion() < Version(3, 2) &&
                   !context->getExtensions().textureStorageMultisample2dArrayOES);
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kMultisampleArrayExtensionOrES32Required);
            break;

        case TextureType::External:
            ASSERT(!context->getExtensions().EGLImageExternalOES &&
                   !context->getExtensions().EGLStreamConsumerExternalNV);
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kExternalTextureNotSupported);
            break;

        case TextureType::VideoImage:
            ASSERT(!context->getExtensions().videoTextureWEBGL);
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kExtensionNotEnabled);
            break;

        case TextureType::Buffer:
            ASSERT(!context->getExtensions().textureBufferOES &&
                   !context->getExtensions().textureBufferEXT);
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kExtensionNotEnabled);
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
    }
}

}  // namespace gl
