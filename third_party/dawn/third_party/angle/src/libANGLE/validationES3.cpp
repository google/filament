//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationES3.cpp: Validation functions for OpenGL ES 3.0 entry point parameters

#include "libANGLE/validationES3_autogen.h"

#include "anglebase/numerics/safe_conversions.h"
#include "common/mathutil.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/ErrorStrings.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/PixelLocalStorage.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/Texture.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/validationES.h"
#include "libANGLE/validationES3.h"

using namespace angle;

namespace gl
{
using namespace err;

namespace
{
bool ValidateFramebufferTextureMultiviewBaseANGLE(const Context *context,
                                                  angle::EntryPoint entryPoint,
                                                  GLenum target,
                                                  GLenum attachment,
                                                  TextureID texture,
                                                  GLint level,
                                                  GLsizei numViews)
{
    if (!(context->getExtensions().multiviewOVR || context->getExtensions().multiview2OVR))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMultiviewNotAvailable);
        return false;
    }

    if (!ValidateFramebufferTextureBase(context, entryPoint, target, attachment, texture, level))
    {
        return false;
    }

    if (texture.value != 0 && numViews < 1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kMultiviewViewsTooSmall);
        return false;
    }

    if (static_cast<GLuint>(numViews) > context->getCaps().maxViews)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kMultiviewViewsTooLarge);
        return false;
    }

    return true;
}

bool ValidateFramebufferTextureMultiviewLevelAndFormat(const Context *context,
                                                       angle::EntryPoint entryPoint,
                                                       const Texture *texture,
                                                       GLint level)
{
    TextureType type = texture->getType();
    if (!ValidMipLevel(context, type, level))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevel);
        return false;
    }

    const auto &format = texture->getFormat(NonCubeTextureTypeToTarget(type), level);
    if (format.info->compressed)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kCompressedTexturesNotAttachable);
        return false;
    }
    return true;
}

bool ValidateUniformES3(const Context *context,
                        angle::EntryPoint entryPoint,
                        GLenum uniformType,
                        UniformLocation location,
                        GLint count)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateUniform(context, entryPoint, uniformType, location, count);
}

bool ValidateUniformMatrixES3(const Context *context,
                              angle::EntryPoint entryPoint,
                              GLenum valueType,
                              UniformLocation location,
                              GLsizei count,
                              GLboolean transpose)
{
    // Check for ES3 uniform entry points
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateUniformMatrix(context, entryPoint, valueType, location, count, transpose);
}

bool ValidateGenOrDeleteES3(const Context *context, angle::EntryPoint entryPoint, GLint n)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }
    return ValidateGenOrDelete(context, entryPoint, n);
}

bool ValidateGenOrDeleteCountES3(const Context *context, angle::EntryPoint entryPoint, GLint count)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }
    if (count < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeCount);
        return false;
    }
    return true;
}

bool ValidateCopyTexture3DCommon(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 const Texture *source,
                                 GLint sourceLevel,
                                 GLint srcInternalFormat,
                                 const Texture *dest,
                                 GLint destLevel,
                                 GLint internalFormat,
                                 TextureTarget destTarget)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (!context->getExtensions().copyTexture3dANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kANGLECopyTexture3DUnavailable);
        return false;
    }

    if (!ValidTexture3DTarget(context, source->getType()))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
        return false;
    }

    // Table 1.1 from the ANGLE_copy_texture_3d spec
    switch (GetUnsizedFormat(srcInternalFormat))
    {
        case GL_ALPHA:
        case GL_LUMINANCE:
        case GL_LUMINANCE_ALPHA:
        case GL_RED:
        case GL_RED_INTEGER:
        case GL_RG:
        case GL_RG_INTEGER:
        case GL_RGB:
        case GL_RGB_INTEGER:
        case GL_RGBA:
        case GL_RGBA_INTEGER:
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_STENCIL:
            break;
        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_OPERATION, kInvalidInternalFormat,
                                    srcInternalFormat);
            return false;
    }

    if (!ValidTexture3DTarget(context, TextureTargetToType(destTarget)))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
        return false;
    }

    // Table 1.0 from the ANGLE_copy_texture_3d spec
    switch (internalFormat)
    {
        case GL_RGB:
        case GL_RGBA:
        case GL_LUMINANCE:
        case GL_LUMINANCE_ALPHA:
        case GL_ALPHA:
        case GL_R8:
        case GL_R8_SNORM:
        case GL_R16F:
        case GL_R32F:
        case GL_R8UI:
        case GL_R8I:
        case GL_R16UI:
        case GL_R16I:
        case GL_R32UI:
        case GL_R32I:
        case GL_RG:
        case GL_RG8:
        case GL_RG8_SNORM:
        case GL_RG16F:
        case GL_RG32F:
        case GL_RG8UI:
        case GL_RG8I:
        case GL_RG16UI:
        case GL_RG16I:
        case GL_RG32UI:
        case GL_RG32I:
        case GL_RGB8:
        case GL_RGBX8_ANGLE:
        case GL_SRGB8:
        case GL_RGB565:
        case GL_RGB8_SNORM:
        case GL_R11F_G11F_B10F:
        case GL_RGB9_E5:
        case GL_RGB16F:
        case GL_RGB32F:
        case GL_RGB8UI:
        case GL_RGB8I:
        case GL_RGB16UI:
        case GL_RGB16I:
        case GL_RGB32UI:
        case GL_RGB32I:
        case GL_RGBA8:
        case GL_SRGB8_ALPHA8:
        case GL_RGBA8_SNORM:
        case GL_RGB5_A1:
        case GL_RGBA4:
        case GL_RGB10_A2:
        case GL_RGBA16F:
        case GL_RGBA32F:
        case GL_RGBA8UI:
        case GL_RGBA8I:
        case GL_RGB10_A2UI:
        case GL_RGBA16UI:
        case GL_RGBA16I:
        case GL_RGBA32I:
        case GL_RGBA32UI:
            break;
        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_OPERATION, kInvalidInternalFormat, internalFormat);
            return false;
    }

    return true;
}

bool ValidateColorMaskForSharedExponentColorBuffer(const Context *context,
                                                   angle::EntryPoint entryPoint,
                                                   GLint drawbuffer)
{
    const State &state                      = context->getState();
    const FramebufferAttachment *attachment = state.getDrawFramebuffer()->getDrawBuffer(drawbuffer);
    if (attachment && attachment->getFormat().info->internalFormat == GL_RGB9_E5)
    {
        bool r, g, b, a;
        state.getBlendStateExt().getColorMaskIndexed(drawbuffer, &r, &g, &b, &a);
        if (r != g || g != b)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                   kUnsupportedColorMaskForSharedExponentColorBuffer);
            return false;
        }
    }

    return true;
}
}  // anonymous namespace

bool ValidateTexImageFormatCombination(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       TextureType target,
                                       GLenum internalFormat,
                                       GLenum format,
                                       GLenum type)
{
    // The type and format are valid if any supported internal format has that type and format.
    // ANGLE_texture_external_yuv_sampling extension adds support for YUV formats
    if (gl::IsYuvFormat(format))
    {
        if (!context->getExtensions().yuvInternalFormatANGLE)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFormat);
            return false;
        }
    }
    else
    {
        if (!ValidES3Format(format))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFormat);
            return false;
        }
    }

    if (!ValidES3Type(type) || (type == GL_HALF_FLOAT_OES && context->isWebGL()))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidType);
        return false;
    }

    // For historical reasons, glTexImage2D and glTexImage3D pass in their internal format as a
    // GLint instead of a GLenum. Therefor an invalid internal format gives a GL_INVALID_VALUE
    // error instead of a GL_INVALID_ENUM error. As this validation function is only called in
    // the validation codepaths for glTexImage2D/3D, we record a GL_INVALID_VALUE error.
    if (!ValidES3InternalFormat(internalFormat))
    {
        ANGLE_VALIDATION_ERRORF(GL_INVALID_VALUE, kInvalidInternalFormat, internalFormat);
        return false;
    }

    // From the ES 3.0 spec section 3.8.3:
    // Textures with a base internal format of DEPTH_COMPONENT or DEPTH_STENCIL are supported by
    // texture image specification commands only if target is TEXTURE_2D, TEXTURE_2D_ARRAY, or
    // TEXTURE_CUBE_MAP.Using these formats in conjunction with any other target will result in an
    // INVALID_OPERATION error.
    //
    // Similar language exists in OES_texture_stencil8.
    if (target == TextureType::_3D &&
        (format == GL_DEPTH_COMPONENT || format == GL_DEPTH_STENCIL || format == GL_STENCIL_INDEX))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, k3DDepthStencil);
        return false;
    }

    // Check if this is a valid format combination to load texture data
    // ANGLE_texture_external_yuv_sampling extension adds support for YUV formats
    if (gl::IsYuvFormat(format))
    {
        if (type != GL_UNSIGNED_BYTE)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFormatCombination);
            return false;
        }
    }
    else
    {
        if (!ValidES3FormatCombination(format, type, internalFormat))
        {
            bool extensionFormatsAllowed = false;
            switch (internalFormat)
            {
                case GL_LUMINANCE4_ALPHA4_OES:
                    if (context->getExtensions().requiredInternalformatOES &&
                        type == GL_UNSIGNED_BYTE && format == GL_LUMINANCE_ALPHA)
                    {
                        extensionFormatsAllowed = true;
                    }
                    break;
                case GL_DEPTH_COMPONENT32_OES:
                    if ((context->getExtensions().requiredInternalformatOES &&
                         context->getExtensions().depth32OES) &&
                        type == GL_UNSIGNED_INT && format == GL_DEPTH_COMPONENT)
                    {
                        extensionFormatsAllowed = true;
                    }
                    break;
                case GL_RGB10_EXT:
                case GL_RGB8_OES:
                case GL_RGB565_OES:
                    if (context->getExtensions().requiredInternalformatOES &&
                        context->getExtensions().textureType2101010REVEXT &&
                        type == GL_UNSIGNED_INT_2_10_10_10_REV_EXT && format == GL_RGB)
                    {
                        extensionFormatsAllowed = true;
                    }
                    break;
                default:
                    break;
            }
            if (!extensionFormatsAllowed)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFormatCombination);
                return false;
            }
        }
    }

    const InternalFormat &formatInfo = GetInternalFormatInfo(internalFormat, type);
    if (!formatInfo.textureSupport(context->getClientVersion(), context->getExtensions()))
    {
        ANGLE_VALIDATION_ERRORF(GL_INVALID_OPERATION, kInvalidInternalFormat, internalFormat);
        return false;
    }

    return true;
}

static bool ValidateES3CompressedFormatForTexture2DArray(const Context *context,
                                                         angle::EntryPoint entryPoint,
                                                         GLenum format)
{
    if ((IsETC1Format(format) && !context->getExtensions().compressedETC1RGB8SubTextureEXT) ||
        IsPVRTC1Format(format))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInternalFormatRequiresTexture2D);
        return false;
    }

    return true;
}

static bool ValidateES3CompressedFormatForTexture3D(const Context *context,
                                                    angle::EntryPoint entryPoint,
                                                    GLenum format)
{
    if (IsETC1Format(format) || IsPVRTC1Format(format))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInternalFormatRequiresTexture2D);
        return false;
    }

    if (IsETC2EACFormat(format))
    {
        // ES 3.1, Section 8.7, page 169.
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInternalFormatRequiresTexture2DArray);
        return false;
    }

    if (IsASTC2DFormat(format) && !(context->getExtensions().textureCompressionAstcHdrKHR ||
                                    context->getExtensions().textureCompressionAstcSliced3dKHR))
    {
        // GL_KHR_texture_compression_astc_hdr, TEXTURE_3D is not supported without HDR profile
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInternalFormatRequiresTexture2DArrayASTC);
        return false;
    }

    if (IsS3TCFormat(format))
    {
        // GL_EXT_texture_compression_s3tc
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInternalFormatRequiresTexture2DArrayS3TC);
        return false;
    }

    if (IsRGTCFormat(format))
    {
        // GL_EXT_texture_compression_rgtc
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInternalFormatRequiresTexture2DArrayRGTC);
        return false;
    }

    if (IsBPTCFormat(format) && (context->getLimitations().noCompressedTexture3D))
    {
        // GL_EXT_texture_compression_bptc
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInternalFormatRequiresTexture2DArrayBPTC);
        return false;
    }

    return true;
}

bool ValidateES3TexImageParametersBase(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       TextureTarget target,
                                       GLint level,
                                       GLenum internalformat,
                                       bool isCompressed,
                                       bool isSubImage,
                                       GLint xoffset,
                                       GLint yoffset,
                                       GLint zoffset,
                                       GLsizei width,
                                       GLsizei height,
                                       GLsizei depth,
                                       GLint border,
                                       GLenum format,
                                       GLenum type,
                                       GLsizei imageSize,
                                       const void *pixels)
{
    TextureType texType = TextureTargetToType(target);

    if (gl::IsYuvFormat(format))
    {
        // According to ANGLE_yuv_internal_format, the texture needs to be an immutable
        // texture, texture target can only be TEXTURE_2D and there is no mipmap support
        if (!context->getExtensions().yuvInternalFormatANGLE || !isSubImage)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFormat);
            return false;
        }

        if (target != TextureTarget::_2D)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
            return false;
        }

        if (level != 0)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevel);
            return false;
        }
    }

    // Validate image size
    if (!ValidImageSizeParameters(context, entryPoint, texType, level, width, height, depth,
                                  isSubImage))
    {
        // Error already processed.
        return false;
    }

    // Verify zero border
    if (border != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidBorder);
        return false;
    }

    if (xoffset < 0 || yoffset < 0 || zoffset < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeOffset);
        return false;
    }

    if (std::numeric_limits<GLsizei>::max() - xoffset < width ||
        std::numeric_limits<GLsizei>::max() - yoffset < height ||
        std::numeric_limits<GLsizei>::max() - zoffset < depth)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kOffsetOverflow);
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

            if (width > (caps.maxCubeMapTextureSize >> level))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kResourceMaxTextureSize);
                return false;
            }
            break;

        case TextureType::_3D:
            if (width > (caps.max3DTextureSize >> level) ||
                height > (caps.max3DTextureSize >> level) ||
                depth > (caps.max3DTextureSize >> level))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kResourceMaxTextureSize);
                return false;
            }
            break;

        case TextureType::_2DArray:
            if (width > (caps.max2DTextureSize >> level) ||
                height > (caps.max2DTextureSize >> level) || depth > caps.maxArrayTextureLayers)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kResourceMaxTextureSize);
                return false;
            }
            break;

        case TextureType::CubeMapArray:
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

            if (width > (caps.max3DTextureSize >> level) ||
                height > (caps.max3DTextureSize >> level) || depth > caps.max3DTextureSize)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kResourceMaxTextureSize);
                return false;
            }

            if (!isSubImage && depth % 6 != 0)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kCubemapInvalidDepth);
                return false;
            }
            break;

        case TextureType::InvalidEnum:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kEnumInvalid);
            return false;
        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, ToGLenum(texType));
            return false;
    }

    Texture *texture = context->getTextureByType(texType);
    if (!texture)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMissingTexture);
        return false;
    }

    if (texture->getImmutableFormat() && !isSubImage)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTextureIsImmutable);
        return false;
    }

    // Validate texture formats
    GLenum actualInternalFormat =
        isSubImage ? texture->getFormat(target, level).info->internalFormat : internalformat;
    if (isSubImage && actualInternalFormat == GL_NONE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidMipLevel);
        return false;
    }

    const InternalFormat &actualFormatInfo = isSubImage
                                                 ? *texture->getFormat(target, level).info
                                                 : GetInternalFormatInfo(internalformat, type);
    if (isCompressed)
    {
        // compressedTexSubImage does not generate GL_INVALID_ENUM when format is unknown or invalid
        if (!isSubImage)
        {
            if (!actualFormatInfo.compressed && !actualFormatInfo.paletted)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kCompressedMismatch);
                return false;
            }

            if (!actualFormatInfo.textureSupport(context->getClientVersion(),
                                                 context->getExtensions()))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFormat);
                return false;
            }
        }

        if (texType == TextureType::_2DArray)
        {
            GLenum compressedDataFormat = isSubImage ? format : internalformat;
            if (!ValidateES3CompressedFormatForTexture2DArray(context, entryPoint,
                                                              compressedDataFormat))
            {
                // Error already generated.
                return false;
            }
        }

        if (texType == TextureType::_3D)
        {
            GLenum compressedDataFormat = isSubImage ? format : internalformat;
            if (!ValidateES3CompressedFormatForTexture3D(context, entryPoint, compressedDataFormat))
            {
                // Error already generated.
                return false;
            }
        }

        if (isSubImage)
        {
            if (!ValidCompressedSubImageSize(
                    context, actualFormatInfo.internalFormat, xoffset, yoffset, zoffset, width,
                    height, depth, texture->getWidth(target, level),
                    texture->getHeight(target, level), texture->getDepth(target, level)))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidCompressedImageSize);
                return false;
            }

            if (format != actualInternalFormat)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMismatchedFormat);
                return false;
            }

            // GL_EXT_compressed_ETC1_RGB8_sub_texture allows this format
            if (IsETC1Format(actualInternalFormat) &&
                !context->getExtensions().compressedETC1RGB8SubTextureEXT)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_OPERATION, kInvalidInternalFormat,
                                        internalformat);
                return false;
            }
        }
        else
        {
            if (!ValidCompressedImageSize(context, actualInternalFormat, level, width, height,
                                          depth))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidCompressedImageSize);
                return false;
            }
        }

        // Disallow 3D-only compressed formats from being set on 2D textures
        if (actualFormatInfo.compressedBlockDepth > 1 && texType != TextureType::_3D)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidTextureTarget);
            return false;
        }
    }
    else
    {
        // Compressed formats are not valid internal formats for glTexImage*D
        if (!isSubImage)
        {
            const InternalFormat &internalFormatInfo = GetSizedInternalFormatInfo(internalformat);
            if (internalFormatInfo.compressed)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_VALUE, kInvalidInternalFormat, internalformat);
                return false;
            }
        }

        if (!ValidateTexImageFormatCombination(context, entryPoint, texType, actualInternalFormat,
                                               format, type))
        {
            return false;
        }
    }

    // Validate sub image parameters
    if (isSubImage)
    {
        if (isCompressed != actualFormatInfo.compressed)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kCompressedMismatch);
            return false;
        }

        // Already validated above
        ASSERT(xoffset >= 0 && yoffset >= 0 && zoffset >= 0);
        ASSERT(std::numeric_limits<GLsizei>::max() - xoffset >= width &&
               std::numeric_limits<GLsizei>::max() - yoffset >= height &&
               std::numeric_limits<GLsizei>::max() - zoffset >= depth);

        if (static_cast<size_t>(xoffset + width) > texture->getWidth(target, level) ||
            static_cast<size_t>(yoffset + height) > texture->getHeight(target, level) ||
            static_cast<size_t>(zoffset + depth) > texture->getDepth(target, level))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kOffsetOverflow);
            return false;
        }

        if (width > 0 && height > 0 && depth > 0 && pixels == nullptr &&
            context->getState().getTargetBuffer(BufferBinding::PixelUnpack) == nullptr)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kPixelDataNull);
            return false;
        }
    }

    GLenum sizeCheckFormat = isSubImage ? format : internalformat;
    if (!ValidImageDataSize(context, entryPoint, texType, width, height, depth, sizeCheckFormat,
                            type, pixels, imageSize))
    {
        return false;
    }

    // Check for pixel unpack buffer related API errors
    Buffer *pixelUnpackBuffer = context->getState().getTargetBuffer(BufferBinding::PixelUnpack);
    if (pixelUnpackBuffer != nullptr)
    {
        // ...data is not evenly divisible into the number of bytes needed to store in memory a
        // datum
        // indicated by type.
        if (!isCompressed)
        {
            size_t offset            = reinterpret_cast<size_t>(pixels);
            size_t dataBytesPerPixel = static_cast<size_t>(GetTypeInfo(type).bytes);

            if ((offset % dataBytesPerPixel) != 0)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kDataTypeNotAligned);
                return false;
            }
        }

        // ...the buffer object's data store is currently mapped but not persistently.
        if (pixelUnpackBuffer->isMapped() && !pixelUnpackBuffer->isPersistentlyMapped())
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferMapped);
            return false;
        }
    }

    if (context->getExtensions().webglCompatibilityANGLE)
    {
        // Define:
        //   DataStoreWidth  = (GL_UNPACK_ROW_LENGTH ? GL_UNPACK_ROW_LENGTH : width)
        //   DataStoreHeight = (GL_UNPACK_IMAGE_HEIGHT ? GL_UNPACK_IMAGE_HEIGHT : height)
        //
        // WebGL 2.0 imposes the following additional constraints:
        //
        // 1) texImage2D and texSubImage2D generate INVALID_OPERATION if:
        //      GL_UNPACK_SKIP_PIXELS + width > DataStoreWidth
        //    except for texImage2D if no GL_PIXEL_UNPACK_BUFFER is
        //    bound and _pixels_ is null.
        //
        // 2) texImage3D and texSubImage3D generate INVALID_OPERATION if:
        //      GL_UNPACK_SKIP_PIXELS + width > DataStoreWidth
        //      GL_UNPACK_SKIP_ROWS + height > DataStoreHeight
        //    except for texImage3D if no GL_PIXEL_UNPACK_BUFFER is
        //    bound and _pixels_ is null.
        if (!pixelUnpackBuffer && !pixels && !isSubImage)
        {
            // Exception case for texImage2D or texImage3D, above.
        }
        else
        {
            const auto &unpack   = context->getState().getUnpackState();
            GLint dataStoreWidth = unpack.rowLength ? unpack.rowLength : width;
            if (unpack.skipPixels + width > dataStoreWidth)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidUnpackParametersForWebGL);
                return false;
            }
            if (target == TextureTarget::_3D || target == TextureTarget::_2DArray)
            {
                GLint dataStoreHeight = unpack.imageHeight ? unpack.imageHeight : height;
                if (unpack.skipRows + height > dataStoreHeight)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidUnpackParametersForWebGL);
                    return false;
                }
            }
        }
    }

    return true;
}

bool ValidateES3TexImage2DParameters(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     TextureTarget target,
                                     GLint level,
                                     GLenum internalformat,
                                     bool isCompressed,
                                     bool isSubImage,
                                     GLint xoffset,
                                     GLint yoffset,
                                     GLint zoffset,
                                     GLsizei width,
                                     GLsizei height,
                                     GLsizei depth,
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

    return ValidateES3TexImageParametersBase(
        context, entryPoint, target, level, internalformat, isCompressed, isSubImage, xoffset,
        yoffset, zoffset, width, height, depth, border, format, type, imageSize, pixels);
}

bool ValidateES3TexImage3DParameters(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     TextureTarget target,
                                     GLint level,
                                     GLenum internalformat,
                                     bool isCompressed,
                                     bool isSubImage,
                                     GLint xoffset,
                                     GLint yoffset,
                                     GLint zoffset,
                                     GLsizei width,
                                     GLsizei height,
                                     GLsizei depth,
                                     GLint border,
                                     GLenum format,
                                     GLenum type,
                                     GLsizei bufSize,
                                     const void *pixels)
{
    if (!ValidTexture3DDestinationTarget(context, target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
        return false;
    }

    return ValidateES3TexImageParametersBase(
        context, entryPoint, target, level, internalformat, isCompressed, isSubImage, xoffset,
        yoffset, zoffset, width, height, depth, border, format, type, bufSize, pixels);
}

struct EffectiveInternalFormatInfo
{
    GLenum effectiveFormat;
    GLenum destFormat;
    GLuint minRedBits;
    GLuint maxRedBits;
    GLuint minGreenBits;
    GLuint maxGreenBits;
    GLuint minBlueBits;
    GLuint maxBlueBits;
    GLuint minAlphaBits;
    GLuint maxAlphaBits;
};

static bool QueryEffectiveFormatList(const InternalFormat &srcFormat,
                                     GLenum targetFormat,
                                     const EffectiveInternalFormatInfo *list,
                                     size_t size,
                                     GLenum *outEffectiveFormat)
{
    for (size_t curFormat = 0; curFormat < size; ++curFormat)
    {
        const EffectiveInternalFormatInfo &formatInfo = list[curFormat];
        if ((formatInfo.destFormat == targetFormat) &&
            (formatInfo.minRedBits <= srcFormat.redBits &&
             formatInfo.maxRedBits >= srcFormat.redBits) &&
            (formatInfo.minGreenBits <= srcFormat.greenBits &&
             formatInfo.maxGreenBits >= srcFormat.greenBits) &&
            (formatInfo.minBlueBits <= srcFormat.blueBits &&
             formatInfo.maxBlueBits >= srcFormat.blueBits) &&
            (formatInfo.minAlphaBits <= srcFormat.alphaBits &&
             formatInfo.maxAlphaBits >= srcFormat.alphaBits))
        {
            *outEffectiveFormat = formatInfo.effectiveFormat;
            return true;
        }
    }

    *outEffectiveFormat = GL_NONE;
    return false;
}

bool GetSizedEffectiveInternalFormatInfo(const InternalFormat &srcFormat,
                                         GLenum *outEffectiveFormat)
{
    // OpenGL ES 3.0.3 Specification, Table 3.17, pg 141:
    // Effective internal format coresponding to destination internal format and linear source
    // buffer component sizes.
    //                                       | Source channel min/max sizes |
    //   Effective Internal Format   |  N/A  |  R   |  G   |  B   |  A      |
    // clang-format off
    constexpr EffectiveInternalFormatInfo list[] = {
        { GL_ALPHA8_EXT,              GL_NONE, 0,  0, 0,  0, 0,  0, 1, 8 },
        { GL_R8,                      GL_NONE, 1,  8, 0,  0, 0,  0, 0, 0 },
        { GL_RG8,                     GL_NONE, 1,  8, 1,  8, 0,  0, 0, 0 },
        { GL_RGB565,                  GL_NONE, 1,  5, 1,  6, 1,  5, 0, 0 },
        { GL_RGB8,                    GL_NONE, 6,  8, 7,  8, 6,  8, 0, 0 },
        { GL_RGBA4,                   GL_NONE, 1,  4, 1,  4, 1,  4, 1, 4 },
        { GL_RGB5_A1,                 GL_NONE, 5,  5, 5,  5, 5,  5, 1, 1 },
        { GL_RGBA8,                   GL_NONE, 5,  8, 5,  8, 5,  8, 2, 8 },
        { GL_RGB10_A2,                GL_NONE, 9, 10, 9, 10, 9, 10, 2, 2 },
    };
    // clang-format on

    return QueryEffectiveFormatList(srcFormat, GL_NONE, list, ArraySize(list), outEffectiveFormat);
}

bool GetUnsizedEffectiveInternalFormatInfo(const InternalFormat &srcFormat,
                                           const InternalFormat &destFormat,
                                           GLenum *outEffectiveFormat)
{
    constexpr GLuint umax = UINT_MAX;

    // OpenGL ES 3.0.3 Specification, Table 3.17, pg 141:
    // Effective internal format coresponding to destination internal format andlinear source buffer
    // component sizes.
    //                                                   |   Source channel min/max sizes   |
    //     Effective Internal Format |   Dest Format     |   R   |    G   |    B   |    A   |
    // clang-format off
    constexpr EffectiveInternalFormatInfo list[] = {
        { GL_ALPHA8_EXT,             GL_ALPHA,           0, umax, 0, umax, 0, umax, 1,    8 },
        { GL_LUMINANCE8_EXT,         GL_LUMINANCE,       1,    8, 0, umax, 0, umax, 0, umax },
        { GL_LUMINANCE8_ALPHA8_EXT,  GL_LUMINANCE_ALPHA, 1,    8, 0, umax, 0, umax, 1,    8 },
        { GL_RGB565,                 GL_RGB,             1,    5, 1,    6, 1,    5, 0, umax },
        { GL_RGB8,                   GL_RGB,             6,    8, 7,    8, 6,    8, 0, umax },
        { GL_RGBA4,                  GL_RGBA,            1,    4, 1,    4, 1,    4, 1,    4 },
        { GL_RGB5_A1,                GL_RGBA,            5,    5, 5,    5, 5,    5, 1,    1 },
        { GL_RGBA8,                  GL_RGBA,            5,    8, 5,    8, 5,    8, 5,    8 },
    };
    // clang-format on

    return QueryEffectiveFormatList(srcFormat, destFormat.format, list, ArraySize(list),
                                    outEffectiveFormat);
}

static bool GetEffectiveInternalFormat(const InternalFormat &srcFormat,
                                       const InternalFormat &destFormat,
                                       GLenum *outEffectiveFormat)
{
    if (destFormat.sized)
    {
        return GetSizedEffectiveInternalFormatInfo(srcFormat, outEffectiveFormat);
    }
    else
    {
        return GetUnsizedEffectiveInternalFormatInfo(srcFormat, destFormat, outEffectiveFormat);
    }
}

static bool EqualOrFirstZero(GLuint first, GLuint second)
{
    return first == 0 || first == second;
}

static bool IsValidES3CopyTexImageCombination(const InternalFormat &textureFormatInfo,
                                              const InternalFormat &framebufferFormatInfo,
                                              FramebufferID readBufferHandle)
{
    if (!ValidES3CopyConversion(textureFormatInfo.format, framebufferFormatInfo.format))
    {
        return false;
    }

    // Section 3.8.5 of the GLES 3.0.3 spec states that source and destination formats
    // must both be signed, unsigned, or fixed point and both source and destinations
    // must be either both SRGB or both not SRGB. EXT_color_buffer_float adds allowed
    // conversion between fixed and floating point.

    if ((textureFormatInfo.colorEncoding == GL_SRGB) !=
        (framebufferFormatInfo.colorEncoding == GL_SRGB))
    {
        return false;
    }

    if (((textureFormatInfo.componentType == GL_INT) !=
         (framebufferFormatInfo.componentType == GL_INT)) ||
        ((textureFormatInfo.componentType == GL_UNSIGNED_INT) !=
         (framebufferFormatInfo.componentType == GL_UNSIGNED_INT)))
    {
        return false;
    }

    if ((textureFormatInfo.componentType == GL_UNSIGNED_NORMALIZED ||
         textureFormatInfo.componentType == GL_SIGNED_NORMALIZED) &&
        !(framebufferFormatInfo.componentType == GL_UNSIGNED_NORMALIZED ||
          framebufferFormatInfo.componentType == GL_SIGNED_NORMALIZED))
    {
        return false;
    }

    if ((textureFormatInfo.componentType == GL_SIGNED_NORMALIZED) !=
        (framebufferFormatInfo.componentType == GL_SIGNED_NORMALIZED))
    {
        return false;
    }

    // Section 3.8.5 of the GLES 3.0.3 (and section 8.6 of the GLES 3.2) spec has a caveat, that
    // the KHR dEQP tests enforce:
    //
    // Note that the above rules disallow matches where some components sizes are smaller and
    // others are larger (such as RGB10_A2).
    if (!textureFormatInfo.sized && (framebufferFormatInfo.internalFormat == GL_RGB10_A2))
    {
        return false;
    }

    // GLES specification 3.0.3, sec 3.8.5, pg 139-140:
    // The effective internal format of the source buffer is determined with the following rules
    // applied in order:
    //    * If the source buffer is a texture or renderbuffer that was created with a sized internal
    //      format then the effective internal format is the source buffer's sized internal format.
    //    * If the source buffer is a texture that was created with an unsized base internal format,
    //      then the effective internal format is the source image array's effective internal
    //      format, as specified by table 3.12, which is determined from the <format> and <type>
    //      that were used when the source image array was specified by TexImage*.
    //    * Otherwise the effective internal format is determined by the row in table 3.17 or 3.18
    //      where Destination Internal Format matches internalformat and where the [source channel
    //      sizes] are consistent with the values of the source buffer's [channel sizes]. Table 3.17
    //      is used if the FRAMEBUFFER_ATTACHMENT_ENCODING is LINEAR and table 3.18 is used if the
    //      FRAMEBUFFER_ATTACHMENT_ENCODING is SRGB.
    const InternalFormat *sourceEffectiveFormat = nullptr;
    if (readBufferHandle.value != 0)
    {
        // Not the default framebuffer, therefore the read buffer must be a user-created texture or
        // renderbuffer
        if (framebufferFormatInfo.sized)
        {
            sourceEffectiveFormat = &framebufferFormatInfo;
        }
        else
        {
            // Renderbuffers cannot be created with an unsized internal format, so this must be an
            // unsized-format texture. We can use the same table we use when creating textures to
            // get its effective sized format.
            sourceEffectiveFormat =
                &GetSizedInternalFormatInfo(framebufferFormatInfo.sizedInternalFormat);
        }
    }
    else
    {
        // The effective internal format must be derived from the source framebuffer's channel
        // sizes. This is done in GetEffectiveInternalFormat for linear buffers (table 3.17)
        if (framebufferFormatInfo.colorEncoding == GL_LINEAR)
        {
            GLenum effectiveFormat;
            if (GetEffectiveInternalFormat(framebufferFormatInfo, textureFormatInfo,
                                           &effectiveFormat))
            {
                sourceEffectiveFormat = &GetSizedInternalFormatInfo(effectiveFormat);
            }
            else
            {
                return false;
            }
        }
        else if (framebufferFormatInfo.colorEncoding == GL_SRGB)
        {
            // SRGB buffers can only be copied to sized format destinations according to table 3.18
            if (textureFormatInfo.sized &&
                (framebufferFormatInfo.redBits >= 1 && framebufferFormatInfo.redBits <= 8) &&
                (framebufferFormatInfo.greenBits >= 1 && framebufferFormatInfo.greenBits <= 8) &&
                (framebufferFormatInfo.blueBits >= 1 && framebufferFormatInfo.blueBits <= 8) &&
                (framebufferFormatInfo.alphaBits >= 1 && framebufferFormatInfo.alphaBits <= 8))
            {
                sourceEffectiveFormat = &GetSizedInternalFormatInfo(GL_SRGB8_ALPHA8);
            }
            else
            {
                return false;
            }
        }
        else
        {
            UNREACHABLE();
            return false;
        }
    }

    if (textureFormatInfo.sized)
    {
        // Section 3.8.5 of the GLES 3.0.3 spec, pg 139, requires that, if the destination format is
        // sized, component sizes of the source and destination formats must exactly match if the
        // destination format exists.
        if (!EqualOrFirstZero(textureFormatInfo.redBits, sourceEffectiveFormat->redBits) ||
            !EqualOrFirstZero(textureFormatInfo.greenBits, sourceEffectiveFormat->greenBits) ||
            !EqualOrFirstZero(textureFormatInfo.blueBits, sourceEffectiveFormat->blueBits) ||
            !EqualOrFirstZero(textureFormatInfo.alphaBits, sourceEffectiveFormat->alphaBits))
        {
            return false;
        }
    }

    return true;  // A conversion function exists, and no rule in the specification has precluded
                  // conversion between these formats.
}

bool ValidateES3CopyTexImageParametersBase(const Context *context,
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
                                           GLint border)
{
    Format textureFormat = Format::Invalid();
    if (!ValidateCopyTexImageParametersBase(context, entryPoint, target, level, internalformat,
                                            isSubImage, xoffset, yoffset, zoffset, x, y, width,
                                            height, border, &textureFormat))
    {
        return false;
    }
    ASSERT(textureFormat.valid() || !isSubImage);

    const auto &state               = context->getState();
    Framebuffer *framebuffer        = state.getReadFramebuffer();
    FramebufferID readFramebufferID = framebuffer->id();

    if (!ValidateFramebufferComplete(context, entryPoint, framebuffer))
    {
        return false;
    }

    // needIntrinsic = true. Treat renderToTexture textures as single sample since they will be
    // resolved before copying
    if (!framebuffer->isDefault() &&
        !ValidateFramebufferNotMultisampled(context, entryPoint, framebuffer, true))
    {
        return false;
    }

    const FramebufferAttachment *source = framebuffer->getReadColorAttachment();

    // According to ES 3.x spec, if the internalformat of the texture
    // is RGB9_E5 and copy to such a texture, generate INVALID_OPERATION.
    if (textureFormat.info->internalFormat == GL_RGB9_E5)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidFormat);
        return false;
    }

    if (isSubImage)
    {
        if (!IsValidES3CopyTexImageCombination(*textureFormat.info, *source->getFormat().info,
                                               readFramebufferID))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidCopyCombination);
            return false;
        }
    }
    else
    {
        // Use format/type from the source FBO. (Might not be perfect for all cases?)
        const InternalFormat &framebufferFormat = *source->getFormat().info;
        const InternalFormat &copyFormat = GetInternalFormatInfo(internalformat, GL_UNSIGNED_BYTE);
        if (!IsValidES3CopyTexImageCombination(copyFormat, framebufferFormat, readFramebufferID))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidCopyCombination);
            return false;
        }
    }

    // If width or height is zero, it is a no-op.  Return false without setting an error.
    return (width > 0 && height > 0);
}

bool ValidateES3CopyTexImage2DParameters(const Context *context,
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
                                         GLint border)
{
    if (!ValidTexture2DDestinationTarget(context, target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
        return false;
    }

    return ValidateES3CopyTexImageParametersBase(context, entryPoint, target, level, internalformat,
                                                 isSubImage, xoffset, yoffset, zoffset, x, y, width,
                                                 height, border);
}

bool ValidateES3CopyTexImage3DParameters(const Context *context,
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
                                         GLint border)
{
    if (!ValidTexture3DDestinationTarget(context, target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
        return false;
    }

    return ValidateES3CopyTexImageParametersBase(context, entryPoint, target, level, internalformat,
                                                 isSubImage, xoffset, yoffset, zoffset, x, y, width,
                                                 height, border);
}

bool ValidateES3TexStorageParametersLevel(const Context *context,
                                          angle::EntryPoint entryPoint,
                                          TextureType target,
                                          GLsizei levels,
                                          GLsizei width,
                                          GLsizei height,
                                          GLsizei depth)
{
    GLsizei maxDim = std::max(width, height);
    if (target != TextureType::_2DArray)
    {
        maxDim = std::max(maxDim, depth);
    }

    if (levels > log2(maxDim) + 1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidMipLevels);
        return false;
    }

    return true;
}

bool ValidateES3TexStorageParametersExtent(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           TextureType target,
                                           GLsizei levels,
                                           GLsizei width,
                                           GLsizei height,
                                           GLsizei depth)
{
    const Caps &caps = context->getCaps();

    switch (target)
    {
        case TextureType::_2D:
        {
            if (width > caps.max2DTextureSize || height > caps.max2DTextureSize)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kResourceMaxTextureSize);
                return false;
            }
        }
        break;

        case TextureType::Rectangle:
        {
            if (levels != 1)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevels);
                return false;
            }

            if (width > caps.maxRectangleTextureSize || height > caps.maxRectangleTextureSize)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kResourceMaxTextureSize);
                return false;
            }
        }
        break;

        case TextureType::CubeMap:
        {
            if (width != height)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kCubemapFacesEqualDimensions);
                return false;
            }

            if (width > caps.maxCubeMapTextureSize)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kResourceMaxTextureSize);
                return false;
            }
        }
        break;

        case TextureType::_3D:
        {
            if (width > caps.max3DTextureSize || height > caps.max3DTextureSize ||
                depth > caps.max3DTextureSize)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kResourceMaxTextureSize);
                return false;
            }
        }
        break;

        case TextureType::_2DArray:
        {
            if (width > caps.max2DTextureSize || height > caps.max2DTextureSize ||
                depth > caps.maxArrayTextureLayers)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kResourceMaxTextureSize);
                return false;
            }
        }
        break;

        case TextureType::CubeMapArray:
        {
            if (width != height)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kCubemapFacesEqualDimensions);
                return false;
            }

            if (width > caps.maxCubeMapTextureSize)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kResourceMaxTextureSize);
                return false;
            }

            if (width > caps.max3DTextureSize || height > caps.max3DTextureSize ||
                depth > caps.max3DTextureSize)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kResourceMaxTextureSize);
                return false;
            }

            if (depth % 6 != 0)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kCubemapInvalidDepth);
                return false;
            }
        }
        break;

        default:
            UNREACHABLE();
            return false;
    }

    return true;
}

bool ValidateES3TexStorageParametersTexObject(const Context *context,
                                              angle::EntryPoint entryPoint,
                                              TextureType target)
{
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

bool ValidateES3TexStorageParametersFormat(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           TextureType target,
                                           GLsizei levels,
                                           GLenum internalformat,
                                           GLsizei width,
                                           GLsizei height,
                                           GLsizei depth)
{
    // From ANGLE_texture_external_yuv_sampling:
    // Texture target can only be TEXTURE_2D, there is no mipmap support
    if (gl::IsYuvFormat(internalformat))
    {
        if (!context->getExtensions().yuvInternalFormatANGLE)
        {
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kInvalidInternalFormat, internalformat);
            return false;
        }

        if (target != TextureType::_2D)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
            return false;
        }

        if (levels != 1)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevel);
            return false;
        }
    }

    const InternalFormat &formatInfo = GetSizedInternalFormatInfo(internalformat);
    if (!formatInfo.textureSupport(context->getClientVersion(), context->getExtensions()))
    {
        ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kInvalidInternalFormat, internalformat);
        return false;
    }

    if (!formatInfo.sized)
    {
        ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kInvalidInternalFormat, internalformat);
        return false;
    }

    if (formatInfo.compressed)
    {
        if (target == TextureType::Rectangle)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kRectangleTextureCompressed);
            return false;
        }

        if (target == TextureType::_2DArray)
        {
            if (!ValidateES3CompressedFormatForTexture2DArray(context, entryPoint,
                                                              formatInfo.internalFormat))
            {
                // Error already generated.
                return false;
            }
        }

        if (target == TextureType::_3D)
        {
            if (!ValidateES3CompressedFormatForTexture3D(context, entryPoint,
                                                         formatInfo.internalFormat))
            {
                // Error already generated.
                return false;
            }
        }

        if (!ValidCompressedImageSize(context, formatInfo.internalFormat, 0, width, height, depth))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidCompressedImageSize);
            return false;
        }
    }

    // From the ES 3.0 spec section 3.8.3:
    // Textures with a base internal format of DEPTH_COMPONENT or DEPTH_STENCIL are supported by
    // texture image specification commands only if target is TEXTURE_2D, TEXTURE_2D_ARRAY, or
    // TEXTURE_CUBE_MAP.Using these formats in conjunction with any other target will result in an
    // INVALID_OPERATION error.
    //
    // Similar language exists in OES_texture_stencil8.
    if (target == TextureType::_3D && formatInfo.isDepthOrStencil())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, k3DDepthStencil);
        return false;
    }

    return true;
}

bool ValidateES3TexStorageParametersBase(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         TextureType target,
                                         GLsizei levels,
                                         GLenum internalformat,
                                         GLsizei width,
                                         GLsizei height,
                                         GLsizei depth)
{
    if (width < 1 || height < 1 || depth < 1 || levels < 1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kTextureSizeTooSmall);
        return false;
    }

    if (!ValidateES3TexStorageParametersLevel(context, entryPoint, target, levels, width, height,
                                              depth))
    {
        // Error already generated.
        return false;
    }

    if (!ValidateES3TexStorageParametersExtent(context, entryPoint, target, levels, width, height,
                                               depth))
    {
        // Error already generated.
        return false;
    }

    if (!ValidateES3TexStorageParametersTexObject(context, entryPoint, target))
    {
        // Error already generated.
        return false;
    }

    if (!ValidateES3TexStorageParametersFormat(context, entryPoint, target, levels, internalformat,
                                               width, height, depth))
    {
        // Error already generated.
        return false;
    }

    return true;
}

bool ValidateES3TexStorage2DParameters(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       TextureType target,
                                       GLsizei levels,
                                       GLenum internalformat,
                                       GLsizei width,
                                       GLsizei height,
                                       GLsizei depth)
{
    if (!ValidTexture2DTarget(context, target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
        return false;
    }

    return ValidateES3TexStorageParametersBase(context, entryPoint, target, levels, internalformat,
                                               width, height, depth);
}

bool ValidateES3TexStorage3DParameters(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       TextureType target,
                                       GLsizei levels,
                                       GLenum internalformat,
                                       GLsizei width,
                                       GLsizei height,
                                       GLsizei depth)
{
    if (!ValidTexture3DTarget(context, target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
        return false;
    }

    return ValidateES3TexStorageParametersBase(context, entryPoint, target, levels, internalformat,
                                               width, height, depth);
}

bool ValidateBeginQuery(const Context *context,
                        angle::EntryPoint entryPoint,
                        QueryType target,
                        QueryID id)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateBeginQueryBase(context, entryPoint, target, id);
}

bool ValidateEndQuery(const Context *context, angle::EntryPoint entryPoint, QueryType target)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateEndQueryBase(context, entryPoint, target);
}

bool ValidateGetQueryiv(const Context *context,
                        angle::EntryPoint entryPoint,
                        QueryType target,
                        GLenum pname,
                        const GLint *params)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateGetQueryivBase(context, entryPoint, target, pname, nullptr);
}

bool ValidateGetQueryObjectuiv(const Context *context,
                               angle::EntryPoint entryPoint,
                               QueryID id,
                               GLenum pname,
                               const GLuint *params)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateGetQueryObjectValueBase(context, entryPoint, id, pname, nullptr);
}

bool ValidateFramebufferTextureLayer(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     GLenum target,
                                     GLenum attachment,
                                     TextureID texture,
                                     GLint level,
                                     GLint layer)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (!ValidateFramebufferTextureBase(context, entryPoint, target, attachment, texture, level))
    {
        return false;
    }

    const Caps &caps = context->getCaps();
    if (texture.value != 0)
    {
        if (layer < 0)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeLayer);
            return false;
        }

        Texture *tex = context->getTexture(texture);
        ASSERT(tex);

        switch (tex->getType())
        {
            case TextureType::_2DArray:
            {
                if (level > log2(caps.max2DTextureSize))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kFramebufferTextureInvalidMipLevel);
                    return false;
                }

                if (layer >= caps.maxArrayTextureLayers)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kFramebufferTextureInvalidLayer);
                    return false;
                }
            }
            break;

            case TextureType::_3D:
            {
                if (level > log2(caps.max3DTextureSize))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kFramebufferTextureInvalidMipLevel);
                    return false;
                }

                if (layer >= caps.max3DTextureSize)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kFramebufferTextureInvalidLayer);
                    return false;
                }
            }
            break;

            case TextureType::_2DMultisampleArray:
            {
                if (level != 0)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kFramebufferTextureInvalidMipLevel);
                    return false;
                }

                if (layer >= caps.maxArrayTextureLayers)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kFramebufferTextureInvalidLayer);
                    return false;
                }
            }
            break;

            case TextureType::CubeMap:
            {
                if (level > log2(caps.maxCubeMapTextureSize))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kFramebufferTextureInvalidMipLevel);
                    return false;
                }

                if (layer >= static_cast<GLint>(kCubeFaceCount))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kFramebufferTextureInvalidLayer);
                    return false;
                }
            }
            break;

            case TextureType::CubeMapArray:
            {
                if (level > log2(caps.maxCubeMapTextureSize))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kFramebufferTextureInvalidMipLevel);
                    return false;
                }

                if (layer >= caps.maxArrayTextureLayers)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kFramebufferTextureInvalidLayer);
                    return false;
                }
            }
            break;

            default:
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                       kFramebufferTextureLayerIncorrectTextureType);
                return false;
        }

        const auto &format = tex->getFormat(TextureTypeToTarget(tex->getType(), layer), level);
        if (format.info->compressed)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kCompressedTexturesNotAttachable);
            return false;
        }
    }

    return true;
}

bool ValidateInvalidateFramebuffer(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   GLenum target,
                                   GLsizei numAttachments,
                                   const GLenum *attachments)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    bool defaultFramebuffer = false;

    switch (target)
    {
        case GL_DRAW_FRAMEBUFFER:
        case GL_FRAMEBUFFER:
            defaultFramebuffer = context->getState().getDrawFramebuffer()->isDefault();
            break;
        case GL_READ_FRAMEBUFFER:
            defaultFramebuffer = context->getState().getReadFramebuffer()->isDefault();
            break;
        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFramebufferTarget);
            return false;
    }

    return ValidateDiscardFramebufferBase(context, entryPoint, target, numAttachments, attachments,
                                          defaultFramebuffer);
}

bool ValidateInvalidateSubFramebuffer(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      GLenum target,
                                      GLsizei numAttachments,
                                      const GLenum *attachments,
                                      GLint x,
                                      GLint y,
                                      GLsizei width,
                                      GLsizei height)
{
    if (width < 0 || height < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeSize);
        return false;
    }

    return ValidateInvalidateFramebuffer(context, entryPoint, target, numAttachments, attachments);
}

bool ValidateClearBuffer(const Context *context, angle::EntryPoint entryPoint)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    Framebuffer *drawFramebuffer = context->getState().getDrawFramebuffer();
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

    return true;
}

bool ValidateDrawRangeElements(const Context *context,
                               angle::EntryPoint entryPoint,
                               PrimitiveMode mode,
                               GLuint start,
                               GLuint end,
                               GLsizei count,
                               DrawElementsType type,
                               const void *indices)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (end < start)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidElementRange);
        return false;
    }

    if (!ValidateDrawElementsCommon(context, entryPoint, mode, count, type, indices, 1))
    {
        return false;
    }

    // Skip range checks for no-op calls.
    if (count <= 0)
    {
        return true;
    }

    return true;
}

bool ValidateGetUniformuiv(const Context *context,
                           angle::EntryPoint entryPoint,
                           ShaderProgramID program,
                           UniformLocation location,
                           const GLuint *params)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateGetUniformBase(context, entryPoint, program, location);
}

bool ValidateReadBuffer(const Context *context, angle::EntryPoint entryPoint, GLenum src)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    const Framebuffer *readFBO = context->getState().getReadFramebuffer();

    if (readFBO == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNoReadFramebuffer);
        return false;
    }

    if (src == GL_NONE)
    {
        return true;
    }

    if (src != GL_BACK && (src < GL_COLOR_ATTACHMENT0 || src > GL_COLOR_ATTACHMENT31))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidReadBuffer);
        return false;
    }

    if (readFBO->isDefault())
    {
        if (src != GL_BACK)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidDefaultReadBuffer);
            return false;
        }
    }
    else
    {
        GLuint drawBuffer = static_cast<GLuint>(src - GL_COLOR_ATTACHMENT0);

        if (drawBuffer >= static_cast<GLuint>(context->getCaps().maxColorAttachments))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExceedsMaxColorAttachments);
            return false;
        }
    }

    return true;
}

bool ValidateCompressedTexImage3D(const Context *context,
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
    if ((context->getClientMajorVersion() < 3) && !context->getExtensions().texture3DOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (!ValidTextureTarget(context, TextureTargetToType(target)))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
        return false;
    }

    // Validate image size
    if (!ValidImageSizeParameters(context, entryPoint, TextureTargetToType(target), level, width,
                                  height, depth, false))
    {
        // Error already generated.
        return false;
    }

    const InternalFormat &formatInfo = GetSizedInternalFormatInfo(internalformat);
    if (!formatInfo.compressed)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidCompressedFormat);
        return false;
    }

    GLuint blockSize = 0;
    if (!formatInfo.computeCompressedImageSize(Extents(width, height, depth), &blockSize))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIntegerOverflow);
        return false;
    }

    if (imageSize < 0 || static_cast<GLuint>(imageSize) != blockSize)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidCompressedImageSize);
        return false;
    }

    // 3D texture target validation
    if (target != TextureTarget::_3D && target != TextureTarget::_2DArray)
    {
        if (context->getClientVersion() < ES_3_2 || target != TextureTarget::CubeMapArray)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidTextureTarget);
            return false;
        }
    }

    // validateES3TexImageFormat sets the error code if there is an error
    if (!ValidateES3TexImage3DParameters(context, entryPoint, target, level, internalformat, true,
                                         false, 0, 0, 0, width, height, depth, border, GL_NONE,
                                         GL_NONE, -1, data))
    {
        return false;
    }

    return true;
}

bool ValidateCompressedTexImage3DRobustANGLE(const Context *context,
                                             angle::EntryPoint entryPoint,
                                             TextureTarget target,
                                             GLint level,
                                             GLenum internalformat,
                                             GLsizei width,
                                             GLsizei height,
                                             GLsizei depth,
                                             GLint border,
                                             GLsizei imageSize,
                                             GLsizei dataSize,
                                             const void *data)
{
    if (!ValidateRobustCompressedTexImageBase(context, entryPoint, imageSize, dataSize))
    {
        return false;
    }

    return ValidateCompressedTexImage3D(context, entryPoint, target, level, internalformat, width,
                                        height, depth, border, imageSize, data);
}

bool ValidateBindVertexArray(const Context *context,
                             angle::EntryPoint entryPoint,
                             VertexArrayID array)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateBindVertexArrayBase(context, entryPoint, array);
}

bool ValidateIsVertexArray(const Context *context,
                           angle::EntryPoint entryPoint,
                           VertexArrayID array)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return true;
}

static bool ValidateBindBufferCommon(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     BufferBinding target,
                                     GLuint index,
                                     BufferID buffer,
                                     GLintptr offset,
                                     GLsizeiptr size)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (buffer.value != 0 && offset < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeOffset);
        return false;
    }

    if (!context->getState().isBindGeneratesResourceEnabled() &&
        !context->isBufferGenerated(buffer))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kObjectNotGenerated);
        return false;
    }

    const Caps &caps = context->getCaps();
    switch (target)
    {
        case BufferBinding::TransformFeedback:
        {
            if (index >= static_cast<GLuint>(caps.maxTransformFeedbackSeparateAttributes))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE,
                                       kIndexExceedsTransformFeedbackBufferBindings);
                return false;
            }
            if (buffer.value != 0 && ((offset % 4) != 0 || (size % 4) != 0))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kOffsetAndSizeAlignment);
                return false;
            }

            if (context->getState().isTransformFeedbackActive())
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTransformFeedbackTargetActive);
                return false;
            }
            break;
        }
        case BufferBinding::Uniform:
        {
            if (index >= static_cast<GLuint>(caps.maxUniformBufferBindings))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxUniformBufferBindings);
                return false;
            }

            ASSERT(caps.uniformBufferOffsetAlignment);
            if (buffer.value != 0 && (offset % caps.uniformBufferOffsetAlignment) != 0)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kUniformBufferOffsetAlignment);
                return false;
            }
            break;
        }
        case BufferBinding::AtomicCounter:
        {
            if (context->getClientVersion() < ES_3_1)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kEnumRequiresGLES31);
                return false;
            }
            if (index >= static_cast<GLuint>(caps.maxAtomicCounterBufferBindings))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE,
                                       kIndexExceedsMaxAtomicCounterBufferBindings);
                return false;
            }
            if (buffer.value != 0 && (offset % 4) != 0)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kOffsetAlignment);
                return false;
            }
            break;
        }
        case BufferBinding::ShaderStorage:
        {
            if (context->getClientVersion() < ES_3_1)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kEnumRequiresGLES31);
                return false;
            }
            if (index >= static_cast<GLuint>(caps.maxShaderStorageBufferBindings))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsMaxShaderStorageBufferBindings);
                return false;
            }
            ASSERT(caps.shaderStorageBufferOffsetAlignment);
            if (buffer.value != 0 && (offset % caps.shaderStorageBufferOffsetAlignment) != 0)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kShaderStorageBufferOffsetAlignment);
                return false;
            }
            break;
        }
        case BufferBinding::Texture:
        {
            if (!context->getExtensions().textureBufferAny())
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kTextureBufferExtensionNotAvailable);
                return false;
            }
            if (index != 0)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxUniformBufferBindings);
                return false;
            }
            if (buffer.value != 0 && (offset % caps.textureBufferOffsetAlignment) != 0)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kTextureBufferOffsetAlignment);
                return false;
            }
            break;
        }
        case BufferBinding::InvalidEnum:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kEnumInvalid);
            return false;
        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, ToGLenum(target));
            return false;
    }

    return true;
}

bool ValidateBindBufferBase(const Context *context,
                            angle::EntryPoint entryPoint,
                            BufferBinding target,
                            GLuint index,
                            BufferID buffer)
{
    return ValidateBindBufferCommon(context, entryPoint, target, index, buffer, 0, 0);
}

bool ValidateBindBufferRange(const Context *context,
                             angle::EntryPoint entryPoint,
                             BufferBinding target,
                             GLuint index,
                             BufferID buffer,
                             GLintptr offset,
                             GLsizeiptr size)
{
    if (buffer.value != 0 && size <= 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidBindBufferSize);
        return false;
    }
    return ValidateBindBufferCommon(context, entryPoint, target, index, buffer, offset, size);
}

bool ValidateProgramBinary(const Context *context,
                           angle::EntryPoint entryPoint,
                           ShaderProgramID program,
                           GLenum binaryFormat,
                           const void *binary,
                           GLsizei length)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateProgramBinaryBase(context, entryPoint, program, binaryFormat, binary, length);
}

bool ValidateGetProgramBinary(const Context *context,
                              angle::EntryPoint entryPoint,
                              ShaderProgramID program,
                              GLsizei bufSize,
                              const GLsizei *length,
                              const GLenum *binaryFormat,
                              const void *binary)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateGetProgramBinaryBase(context, entryPoint, program, bufSize, length, binaryFormat,
                                        binary);
}

bool ValidateProgramParameteriBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID program,
                                   GLenum pname,
                                   GLint value)
{
    if (GetValidProgram(context, entryPoint, program) == nullptr)
    {
        return false;
    }

    switch (pname)
    {
        case GL_PROGRAM_BINARY_RETRIEVABLE_HINT:
            if (value != GL_FALSE && value != GL_TRUE)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidBooleanValue);
                return false;
            }
            break;

        case GL_PROGRAM_SEPARABLE:
            if (context->getClientVersion() < ES_3_1)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kES31Required);
                return false;
            }

            if (value != GL_FALSE && value != GL_TRUE)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidBooleanValue);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidPname);
            return false;
    }

    return true;
}

bool ValidateProgramParameteri(const Context *context,
                               angle::EntryPoint entryPoint,
                               ShaderProgramID program,
                               GLenum pname,
                               GLint value)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateProgramParameteriBase(context, entryPoint, program, pname, value);
}

bool ValidateBlitFramebuffer(const Context *context,
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
    if (context->getClientMajorVersion() < 3 && !context->getExtensions().framebufferBlitNV)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateBlitFramebufferParameters(context, entryPoint, srcX0, srcY0, srcX1, srcY1, dstX0,
                                             dstY0, dstX1, dstY1, mask, filter);
}

bool ValidateClearBufferiv(const Context *context,
                           angle::EntryPoint entryPoint,
                           GLenum buffer,
                           GLint drawbuffer,
                           const GLint *value)
{
    switch (buffer)
    {
        case GL_COLOR:
            if (drawbuffer < 0 || drawbuffer >= context->getCaps().maxDrawBuffers)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxDrawBuffer);
                return false;
            }
            if (static_cast<size_t>(drawbuffer) >=
                context->getState().getDrawFramebuffer()->getDrawbufferStateCount())
            {
                // Clearing a non-existent draw buffer is a no-op.
                break;
            }
            if (context->getExtensions().webglCompatibilityANGLE)
            {
                const gl::ComponentTypeMask mask =
                    context->getState().getDrawFramebuffer()->getDrawBufferTypeMask();
                if (IsComponentTypeFloatOrUnsignedInt(mask, drawbuffer))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNoDefinedClearConversion);
                    return false;
                }
            }
            if (context->getExtensions().renderSharedExponentQCOM)
            {
                if (!ValidateColorMaskForSharedExponentColorBuffer(context, entryPoint, drawbuffer))
                {
                    return false;
                }
            }
            break;

        case GL_STENCIL:
            if (drawbuffer != 0)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidDepthStencilDrawBuffer);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, buffer);
            return false;
    }

    return ValidateClearBuffer(context, entryPoint);
}

bool ValidateClearBufferuiv(const Context *context,
                            angle::EntryPoint entryPoint,
                            GLenum buffer,
                            GLint drawbuffer,
                            const GLuint *value)
{
    switch (buffer)
    {
        case GL_COLOR:
            if (drawbuffer < 0 || drawbuffer >= context->getCaps().maxDrawBuffers)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxDrawBuffer);
                return false;
            }
            if (static_cast<size_t>(drawbuffer) >=
                context->getState().getDrawFramebuffer()->getDrawbufferStateCount())
            {
                // Clearing a non-existent draw buffer is a no-op.
                break;
            }
            if (context->getExtensions().webglCompatibilityANGLE)
            {
                const gl::ComponentTypeMask mask =
                    context->getState().getDrawFramebuffer()->getDrawBufferTypeMask();
                if (IsComponentTypeFloatOrInt(mask, drawbuffer))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNoDefinedClearConversion);
                    return false;
                }
            }
            if (context->getExtensions().renderSharedExponentQCOM)
            {
                if (!ValidateColorMaskForSharedExponentColorBuffer(context, entryPoint, drawbuffer))
                {
                    return false;
                }
            }
            break;

        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, buffer);
            return false;
    }

    return ValidateClearBuffer(context, entryPoint);
}

bool ValidateClearBufferfv(const Context *context,
                           angle::EntryPoint entryPoint,
                           GLenum buffer,
                           GLint drawbuffer,
                           const GLfloat *value)
{
    switch (buffer)
    {
        case GL_COLOR:
            if (drawbuffer < 0 || drawbuffer >= context->getCaps().maxDrawBuffers)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxDrawBuffer);
                return false;
            }
            if (static_cast<size_t>(drawbuffer) >=
                context->getState().getDrawFramebuffer()->getDrawbufferStateCount())
            {
                // Clearing a non-existent draw buffer is a no-op.
                break;
            }
            if (context->getExtensions().webglCompatibilityANGLE)
            {
                const gl::ComponentTypeMask mask =
                    context->getState().getDrawFramebuffer()->getDrawBufferTypeMask();
                if (IsComponentTypeIntOrUnsignedInt(mask, drawbuffer))
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNoDefinedClearConversion);
                    return false;
                }
            }
            if (context->getExtensions().renderSharedExponentQCOM)
            {
                if (!ValidateColorMaskForSharedExponentColorBuffer(context, entryPoint, drawbuffer))
                {
                    return false;
                }
            }
            break;

        case GL_DEPTH:
            if (drawbuffer != 0)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidDepthStencilDrawBuffer);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, buffer);
            return false;
    }

    return ValidateClearBuffer(context, entryPoint);
}

bool ValidateClearBufferfi(const Context *context,
                           angle::EntryPoint entryPoint,
                           GLenum buffer,
                           GLint drawbuffer,
                           GLfloat depth,
                           GLint stencil)
{
    switch (buffer)
    {
        case GL_DEPTH_STENCIL:
            if (drawbuffer != 0)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidDepthStencilDrawBuffer);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, buffer);
            return false;
    }

    return ValidateClearBuffer(context, entryPoint);
}

bool ValidateDrawBuffers(const Context *context,
                         angle::EntryPoint entryPoint,
                         GLsizei n,
                         const GLenum *bufs)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateDrawBuffersBase(context, entryPoint, n, bufs);
}

bool ValidateCopyTexSubImage3D(const Context *context,
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
    if ((context->getClientMajorVersion() < 3) && !context->getExtensions().texture3DOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateES3CopyTexImage3DParameters(context, entryPoint, target, level, GL_NONE, true,
                                               xoffset, yoffset, zoffset, x, y, width, height, 0);
}

bool ValidateCopyTexture3DANGLE(const Context *context,
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
    const Texture *source = context->getTexture(sourceId);
    if (source == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidSourceTexture);
        return false;
    }

    TextureType sourceType = source->getType();
    ASSERT(sourceType != TextureType::CubeMap);
    TextureTarget sourceTarget = NonCubeTextureTypeToTarget(sourceType);
    const Format &sourceFormat = source->getFormat(sourceTarget, sourceLevel);

    const Texture *dest = context->getTexture(destId);
    if (dest == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidDestinationTexture);
        return false;
    }

    if (!ValidateCopyTexture3DCommon(context, entryPoint, source, sourceLevel,
                                     sourceFormat.info->internalFormat, dest, destLevel,
                                     internalFormat, destTarget))
    {
        return false;
    }

    if (!ValidMipLevel(context, source->getType(), sourceLevel))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidSourceTextureLevel);
        return false;
    }

    GLsizei sourceWidth  = static_cast<GLsizei>(source->getWidth(sourceTarget, sourceLevel));
    GLsizei sourceHeight = static_cast<GLsizei>(source->getHeight(sourceTarget, sourceLevel));
    if (sourceWidth == 0 || sourceHeight == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidSourceTextureSize);
        return false;
    }

    if (dest->getImmutableFormat())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kDestinationImmutable);
        return false;
    }

    return true;
}

bool ValidateCopySubTexture3DANGLE(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   TextureID sourceId,
                                   GLint sourceLevel,
                                   TextureTarget destTarget,
                                   TextureID destId,
                                   GLint destLevel,
                                   GLint xoffset,
                                   GLint yoffset,
                                   GLint zoffset,
                                   GLint x,
                                   GLint y,
                                   GLint z,
                                   GLsizei width,
                                   GLsizei height,
                                   GLsizei depth,
                                   GLboolean unpackFlipY,
                                   GLboolean unpackPremultiplyAlpha,
                                   GLboolean unpackUnmultiplyAlpha)
{
    const Texture *source = context->getTexture(sourceId);
    if (source == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidSourceTexture);
        return false;
    }

    TextureType sourceType = source->getType();
    ASSERT(sourceType != TextureType::CubeMap);
    TextureTarget sourceTarget = NonCubeTextureTypeToTarget(sourceType);
    const Format &sourceFormat = source->getFormat(sourceTarget, sourceLevel);

    const Texture *dest = context->getTexture(destId);
    if (dest == nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidDestinationTexture);
        return false;
    }

    const InternalFormat &destFormat = *dest->getFormat(destTarget, destLevel).info;

    if (!ValidateCopyTexture3DCommon(context, entryPoint, source, sourceLevel,
                                     sourceFormat.info->internalFormat, dest, destLevel,
                                     destFormat.internalFormat, destTarget))
    {
        return false;
    }

    if (x < 0 || y < 0 || z < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeXYZ);
        return false;
    }

    if (width < 0 || height < 0 || depth < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeHeightWidthDepth);
        return false;
    }

    if (static_cast<size_t>(x + width) > source->getWidth(sourceTarget, sourceLevel) ||
        static_cast<size_t>(y + height) > source->getHeight(sourceTarget, sourceLevel) ||
        static_cast<size_t>(z + depth) > source->getDepth(sourceTarget, sourceLevel))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kSourceTextureTooSmall);
        return false;
    }

    if (TextureTargetToType(destTarget) != dest->getType())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidDestinationTextureType);
        return false;
    }

    if (xoffset < 0 || yoffset < 0 || zoffset < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeOffset);
        return false;
    }

    if (static_cast<size_t>(xoffset + width) > dest->getWidth(destTarget, destLevel) ||
        static_cast<size_t>(yoffset + height) > dest->getHeight(destTarget, destLevel) ||
        static_cast<size_t>(zoffset + depth) > dest->getDepth(destTarget, destLevel))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kDestinationTextureTooSmall);
        return false;
    }

    return true;
}

bool ValidateTexImage3D(const Context *context,
                        angle::EntryPoint entryPoint,
                        TextureTarget target,
                        GLint level,
                        GLint internalformat,
                        GLsizei width,
                        GLsizei height,
                        GLsizei depth,
                        GLint border,
                        GLenum format,
                        GLenum type,
                        const void *pixels)
{
    if ((context->getClientMajorVersion() < 3) && !context->getExtensions().texture3DOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateES3TexImage3DParameters(context, entryPoint, target, level, internalformat,
                                           false, false, 0, 0, 0, width, height, depth, border,
                                           format, type, -1, pixels);
}

bool ValidateTexImage3DRobustANGLE(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   TextureTarget target,
                                   GLint level,
                                   GLint internalformat,
                                   GLsizei width,
                                   GLsizei height,
                                   GLsizei depth,
                                   GLint border,
                                   GLenum format,
                                   GLenum type,
                                   GLsizei bufSize,
                                   const void *pixels)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    return ValidateES3TexImage3DParameters(context, entryPoint, target, level, internalformat,
                                           false, false, 0, 0, 0, width, height, depth, border,
                                           format, type, bufSize, pixels);
}

bool ValidateTexSubImage3D(const Context *context,
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
    if ((context->getClientMajorVersion() < 3) && !context->getExtensions().texture3DOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateES3TexImage3DParameters(context, entryPoint, target, level, GL_NONE, false, true,
                                           xoffset, yoffset, zoffset, width, height, depth, 0,
                                           format, type, -1, pixels);
}

bool ValidateTexSubImage3DRobustANGLE(const Context *context,
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
                                      GLsizei bufSize,
                                      const void *pixels)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    return ValidateES3TexImage3DParameters(context, entryPoint, target, level, GL_NONE, false, true,
                                           xoffset, yoffset, zoffset, width, height, depth, 0,
                                           format, type, bufSize, pixels);
}

bool ValidateCompressedTexSubImage3D(const Context *context,
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
    if ((context->getClientMajorVersion() < 3) && !context->getExtensions().texture3DOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (!ValidateES3TexImage3DParameters(context, entryPoint, target, level, GL_NONE, true, true,
                                         xoffset, yoffset, zoffset, width, height, depth, 0, format,
                                         GL_NONE, -1, data))
    {
        return false;
    }

    const InternalFormat &formatInfo = GetSizedInternalFormatInfo(format);

    if (!formatInfo.compressed)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidCompressedFormat);
        return false;
    }

    GLuint blockSize = 0;
    if (!formatInfo.computeCompressedImageSize(Extents(width, height, depth), &blockSize))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kIntegerOverflow);
        return false;
    }

    if (imageSize < 0 || static_cast<GLuint>(imageSize) != blockSize)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidCompressedImageSize);
        return false;
    }

    if (data == nullptr)
    {
        if (context->getState().getTargetBuffer(BufferBinding::PixelUnpack) == nullptr)
        {
            // If data is null, we need an unpack buffer to read from
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kPixelDataNull);
            return false;
        }
    }

    return true;
}

bool ValidateCompressedTexSubImage3DRobustANGLE(const Context *context,
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
                                                GLsizei dataSize,
                                                const void *data)
{
    if (!ValidateRobustCompressedTexImageBase(context, entryPoint, imageSize, dataSize))
    {
        return false;
    }

    return ValidateCompressedTexSubImage3D(context, entryPoint, target, level, xoffset, yoffset,
                                           zoffset, width, height, depth, format, imageSize, data);
}

bool ValidateGenQueries(const Context *context,
                        angle::EntryPoint entryPoint,
                        GLsizei n,
                        const QueryID *queries)
{
    return ValidateGenOrDeleteES3(context, entryPoint, n);
}

bool ValidateDeleteQueries(const Context *context,
                           angle::EntryPoint entryPoint,
                           GLsizei n,
                           const QueryID *queries)
{
    return ValidateGenOrDeleteES3(context, entryPoint, n);
}

bool ValidateGenSamplers(const Context *context,
                         angle::EntryPoint entryPoint,
                         GLsizei count,
                         const SamplerID *samplers)
{
    return ValidateGenOrDeleteCountES3(context, entryPoint, count);
}

bool ValidateDeleteSamplers(const Context *context,
                            angle::EntryPoint entryPoint,
                            GLsizei count,
                            const SamplerID *samplers)
{
    return ValidateGenOrDeleteCountES3(context, entryPoint, count);
}

bool ValidateGenTransformFeedbacks(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   GLsizei n,
                                   const TransformFeedbackID *ids)
{
    return ValidateGenOrDeleteES3(context, entryPoint, n);
}

bool ValidateDeleteTransformFeedbacks(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      GLsizei n,
                                      const TransformFeedbackID *ids)
{
    if (!ValidateGenOrDeleteES3(context, entryPoint, n))
    {
        return false;
    }
    for (GLint i = 0; i < n; ++i)
    {
        auto *transformFeedback = context->getTransformFeedback(ids[i]);
        if (transformFeedback != nullptr && transformFeedback->isActive())
        {
            // ES 3.0.4 section 2.15.1 page 86
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTransformFeedbackActiveDelete);
            return false;
        }
    }
    return true;
}

bool ValidateGenVertexArrays(const Context *context,
                             angle::EntryPoint entryPoint,
                             GLsizei n,
                             const VertexArrayID *arrays)
{
    return ValidateGenOrDeleteES3(context, entryPoint, n);
}

bool ValidateDeleteVertexArrays(const Context *context,
                                angle::EntryPoint entryPoint,
                                GLsizei n,
                                const VertexArrayID *arrays)
{
    return ValidateGenOrDeleteES3(context, entryPoint, n);
}

bool ValidateBeginTransformFeedback(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    PrimitiveMode primitiveMode)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }
    switch (primitiveMode)
    {
        case PrimitiveMode::Triangles:
        case PrimitiveMode::Lines:
        case PrimitiveMode::Points:
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidPrimitiveMode);
            return false;
    }

    TransformFeedback *transformFeedback = context->getState().getCurrentTransformFeedback();
    ASSERT(transformFeedback != nullptr);

    if (transformFeedback->isActive())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTransfomFeedbackAlreadyActive);
        return false;
    }

    for (size_t i = 0; i < transformFeedback->getIndexedBufferCount(); i++)
    {
        const OffsetBindingPointer<Buffer> &buffer = transformFeedback->getIndexedBuffer(i);
        if (buffer.get())
        {
            if (buffer->isMapped())
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferMapped);
                return false;
            }
            if ((context->getLimitations().noDoubleBoundTransformFeedbackBuffers ||
                 context->getExtensions().webglCompatibilityANGLE) &&
                buffer->isDoubleBoundForTransformFeedback())
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                       kTransformFeedbackBufferMultipleOutputs);
                return false;
            }
        }
    }

    const ProgramExecutable *programExecutable =
        context->getState().getLinkedProgramExecutable(context);
    if (!programExecutable)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kProgramNotBound);
        return false;
    }

    if (programExecutable->getLinkedTransformFeedbackVaryings().empty())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNoTransformFeedbackOutputVariables);
        return false;
    }

    if (!ValidateProgramExecutableXFBBuffersPresent(context, programExecutable))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTransformFeedbackBufferMissing);
        return false;
    }

    return true;
}

bool ValidateGetBufferPointerv(const Context *context,
                               angle::EntryPoint entryPoint,
                               BufferBinding target,
                               GLenum pname,
                               void *const *params)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateGetBufferPointervBase(context, entryPoint, target, pname, nullptr, params);
}

bool ValidateGetBufferPointervRobustANGLE(const Context *context,
                                          angle::EntryPoint entryPoint,
                                          BufferBinding target,
                                          GLenum pname,
                                          GLsizei bufSize,
                                          const GLsizei *length,
                                          void *const *params)
{
    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei numParams = 0;

    if (context->getClientMajorVersion() < 3 && !context->getExtensions().mapbufferOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    if (!ValidateGetBufferPointervBase(context, entryPoint, target, pname, &numParams, params))
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

bool ValidateUnmapBuffer(const Context *context, angle::EntryPoint entryPoint, BufferBinding target)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateUnmapBufferBase(context, entryPoint, target);
}

bool ValidateMapBufferRange(const Context *context,
                            angle::EntryPoint entryPoint,
                            BufferBinding target,
                            GLintptr offset,
                            GLsizeiptr length,
                            GLbitfield access)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateMapBufferRangeBase(context, entryPoint, target, offset, length, access);
}

bool ValidateFlushMappedBufferRange(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    BufferBinding target,
                                    GLintptr offset,
                                    GLsizeiptr length)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateFlushMappedBufferRangeBase(context, entryPoint, target, offset, length);
}

bool ValidateIndexedStateQuery(const Context *context,
                               angle::EntryPoint entryPoint,
                               GLenum pname,
                               GLuint index,
                               GLsizei *length)
{
    if (length)
    {
        *length = 0;
    }

    GLenum nativeType;
    unsigned int numParams;
    if (!context->getIndexedQueryParameterInfo(pname, &nativeType, &numParams))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidPname);
        return false;
    }

    const Caps &caps = context->getCaps();
    switch (pname)
    {
        case GL_BLEND_SRC_RGB:
        case GL_BLEND_SRC_ALPHA:
        case GL_BLEND_DST_RGB:
        case GL_BLEND_DST_ALPHA:
        case GL_BLEND_EQUATION_RGB:
        case GL_BLEND_EQUATION_ALPHA:
        case GL_COLOR_WRITEMASK:
            ASSERT(context->getClientVersion() >= ES_3_2 ||
                   context->getExtensions().drawBuffersIndexedAny());
            if (index >= static_cast<GLuint>(caps.maxDrawBuffers))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxDrawBuffer);
                return false;
            }
            break;
        case GL_TRANSFORM_FEEDBACK_BUFFER_START:
        case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
        case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
            if (index >= static_cast<GLuint>(caps.maxTransformFeedbackSeparateAttributes))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxTransformFeedbackAttribs);
                return false;
            }
            break;

        case GL_UNIFORM_BUFFER_START:
        case GL_UNIFORM_BUFFER_SIZE:
        case GL_UNIFORM_BUFFER_BINDING:
            if (index >= static_cast<GLuint>(caps.maxUniformBufferBindings))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxUniformBufferBindings);
                return false;
            }
            break;

        case GL_MAX_COMPUTE_WORK_GROUP_SIZE:
        case GL_MAX_COMPUTE_WORK_GROUP_COUNT:
            ASSERT(context->getClientVersion() >= ES_3_1);
            if (index >= 3u)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxWorkgroupDimensions);
                return false;
            }
            break;

        case GL_ATOMIC_COUNTER_BUFFER_START:
        case GL_ATOMIC_COUNTER_BUFFER_SIZE:
        case GL_ATOMIC_COUNTER_BUFFER_BINDING:
            ASSERT(context->getClientVersion() >= ES_3_1);
            if (index >= static_cast<GLuint>(caps.maxAtomicCounterBufferBindings))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE,
                                       kIndexExceedsMaxAtomicCounterBufferBindings);
                return false;
            }
            break;

        case GL_SHADER_STORAGE_BUFFER_START:
        case GL_SHADER_STORAGE_BUFFER_SIZE:
        case GL_SHADER_STORAGE_BUFFER_BINDING:
            ASSERT(context->getClientVersion() >= ES_3_1);
            if (index >= static_cast<GLuint>(caps.maxShaderStorageBufferBindings))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsMaxShaderStorageBufferBindings);
                return false;
            }
            break;

        case GL_VERTEX_BINDING_BUFFER:
        case GL_VERTEX_BINDING_DIVISOR:
        case GL_VERTEX_BINDING_OFFSET:
        case GL_VERTEX_BINDING_STRIDE:
            ASSERT(context->getClientVersion() >= ES_3_1);
            if (index >= static_cast<GLuint>(caps.maxVertexAttribBindings))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsMaxVertexAttribBindings);
                return false;
            }
            break;
        case GL_SAMPLE_MASK_VALUE:
            ASSERT(context->getClientVersion() >= ES_3_1 ||
                   context->getExtensions().textureMultisampleANGLE);
            if (index >= static_cast<GLuint>(caps.maxSampleMaskWords))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidSampleMaskNumber);
                return false;
            }
            break;
        case GL_IMAGE_BINDING_NAME:
        case GL_IMAGE_BINDING_LEVEL:
        case GL_IMAGE_BINDING_LAYERED:
        case GL_IMAGE_BINDING_LAYER:
        case GL_IMAGE_BINDING_ACCESS:
        case GL_IMAGE_BINDING_FORMAT:
            ASSERT(context->getClientVersion() >= ES_3_1);
            if (index >= static_cast<GLuint>(caps.maxImageUnits))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsMaxImageUnits);
                return false;
            }
            break;
        default:
            UNREACHABLE();
            return false;
    }

    if (length)
    {
        if (pname == GL_COLOR_WRITEMASK)
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

bool ValidateGetIntegeri_v(const Context *context,
                           angle::EntryPoint entryPoint,
                           GLenum target,
                           GLuint index,
                           const GLint *data)
{
    if (context->getClientVersion() < ES_3_0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }
    return ValidateIndexedStateQuery(context, entryPoint, target, index, nullptr);
}

bool ValidateGetIntegeri_vRobustANGLE(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      GLenum target,
                                      GLuint index,
                                      GLsizei bufSize,
                                      const GLsizei *length,
                                      const GLint *data)
{
    if (context->getClientVersion() < ES_3_0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei numParams = 0;

    if (!ValidateIndexedStateQuery(context, entryPoint, target, index, &numParams))
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

bool ValidateGetInteger64i_v(const Context *context,
                             angle::EntryPoint entryPoint,
                             GLenum target,
                             GLuint index,
                             const GLint64 *data)
{
    if (context->getClientVersion() < ES_3_0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }
    return ValidateIndexedStateQuery(context, entryPoint, target, index, nullptr);
}

bool ValidateGetInteger64i_vRobustANGLE(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        GLenum target,
                                        GLuint index,
                                        GLsizei bufSize,
                                        const GLsizei *length,
                                        const GLint64 *data)
{
    if (context->getClientVersion() < ES_3_0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei numParams = 0;

    if (!ValidateIndexedStateQuery(context, entryPoint, target, index, &numParams))
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

bool ValidateCopyBufferSubData(const Context *context,
                               angle::EntryPoint entryPoint,
                               BufferBinding readTarget,
                               BufferBinding writeTarget,
                               GLintptr readOffset,
                               GLintptr writeOffset,
                               GLsizeiptr size)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (!context->isValidBufferBinding(readTarget) || !context->isValidBufferBinding(writeTarget))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidBufferTypes);
        return false;
    }

    Buffer *readBuffer  = context->getState().getTargetBuffer(readTarget);
    Buffer *writeBuffer = context->getState().getTargetBuffer(writeTarget);

    if (!readBuffer || !writeBuffer)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferNotBound);
        return false;
    }

    // EXT_buffer_storage allows persistently mapped buffers to be updated via glCopyBufferSubData
    bool isReadPersistent  = (readBuffer->getAccessFlags() & GL_MAP_PERSISTENT_BIT_EXT) != 0;
    bool isWritePersistent = (writeBuffer->getAccessFlags() & GL_MAP_PERSISTENT_BIT_EXT) != 0;

    // Verify that readBuffer and writeBuffer are not currently mapped unless persistent
    if ((readBuffer->isMapped() && !isReadPersistent) ||
        (writeBuffer->isMapped() && !isWritePersistent))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferMapped);
        return false;
    }

    if (readBuffer->hasWebGLXFBBindingConflict(context->isWebGL()) ||
        writeBuffer->hasWebGLXFBBindingConflict(context->isWebGL()))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferBoundForTransformFeedback);
        return false;
    }

    CheckedNumeric<GLintptr> checkedReadOffset(readOffset);
    CheckedNumeric<GLintptr> checkedWriteOffset(writeOffset);
    CheckedNumeric<GLintptr> checkedSize(size);

    auto checkedReadSum  = checkedReadOffset + checkedSize;
    auto checkedWriteSum = checkedWriteOffset + checkedSize;

    if (!checkedReadSum.IsValid() || !checkedWriteSum.IsValid() ||
        !IsValueInRangeForNumericType<GLintptr>(readBuffer->getSize()) ||
        !IsValueInRangeForNumericType<GLintptr>(writeBuffer->getSize()))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIntegerOverflow);
        return false;
    }

    if (readOffset < 0 || writeOffset < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeOffset);
        return false;
    }

    if (size < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeSize);
        return false;
    }

    if (checkedReadSum.ValueOrDie() > readBuffer->getSize() ||
        checkedWriteSum.ValueOrDie() > writeBuffer->getSize())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kBufferOffsetOverflow);
        return false;
    }

    if (readBuffer == writeBuffer)
    {
        auto checkedOffsetDiff = (checkedReadOffset - checkedWriteOffset).Abs();
        if (!checkedOffsetDiff.IsValid())
        {
            // This shold not be possible.
            UNREACHABLE();
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIntegerOverflow);
            return false;
        }

        if (checkedOffsetDiff.ValueOrDie() < size)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kCopyAlias);
            return false;
        }
    }

    return true;
}

bool ValidateGetStringi(const Context *context,
                        angle::EntryPoint entryPoint,
                        GLenum name,
                        GLuint index)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    switch (name)
    {
        case GL_EXTENSIONS:
            if (index >= context->getExtensionStringCount())
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsNumExtensions);
                return false;
            }
            break;

        case GL_REQUESTABLE_EXTENSIONS_ANGLE:
            if (!context->getExtensions().requestExtensionANGLE)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidName);
                return false;
            }
            if (index >= context->getRequestableExtensionStringCount())
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsNumRequestableExtensions);
                return false;
            }
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidName);
            return false;
    }

    return true;
}

bool ValidateRenderbufferStorageMultisample(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            GLenum target,
                                            GLsizei samples,
                                            GLenum internalformat,
                                            GLsizei width,
                                            GLsizei height)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (!ValidateRenderbufferStorageParametersBase(context, entryPoint, target, samples,
                                                   internalformat, width, height))
    {
        return false;
    }

    // The ES3 spec(section 4.4.2) states that the internal format must be sized and not an integer
    // format if samples is greater than zero. In ES3.1(section 9.2.5), it can support integer
    // multisample renderbuffer, but the samples should not be greater than MAX_INTEGER_SAMPLES.
    const InternalFormat &formatInfo = GetSizedInternalFormatInfo(internalformat);
    if (formatInfo.isInt())
    {
        if ((samples > 0 && context->getClientVersion() == ES_3_0) ||
            samples > context->getCaps().maxIntegerSamples)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kSamplesOutOfRange);
            return false;
        }
    }

    // The behavior is different than the ANGLE version, which would generate a GL_OUT_OF_MEMORY.
    const TextureCaps &formatCaps = context->getTextureCaps().get(internalformat);
    if (static_cast<GLuint>(samples) > formatCaps.getMaxSamples())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kSamplesOutOfRange);
        return false;
    }

    return true;
}

bool ValidateVertexAttribIPointer(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  GLuint index,
                                  GLint size,
                                  VertexAttribType type,
                                  GLsizei stride,
                                  const void *pointer)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (!ValidateIntegerVertexFormat(context, entryPoint, index, size, type))
    {
        return false;
    }

    if (stride < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeStride);
        return false;
    }

    const Caps &caps = context->getCaps();
    if (context->getClientVersion() >= ES_3_1)
    {
        if (stride > caps.maxVertexAttribStride)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsMaxVertexAttribStride);
            return false;
        }

        // [OpenGL ES 3.1] Section 10.3.1 page 245:
        // glVertexAttribBinding is part of the equivalent code of VertexAttribIPointer, so its
        // validation should be inherited.
        if (index >= static_cast<GLuint>(caps.maxVertexAttribBindings))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsMaxVertexAttribBindings);
            return false;
        }
    }

    // [OpenGL ES 3.0.2] Section 2.8 page 24:
    // An INVALID_OPERATION error is generated when a non-zero vertex array object
    // is bound, zero is bound to the ARRAY_BUFFER buffer object binding point,
    // and the pointer argument is not NULL.
    if (context->getState().getVertexArrayId().value != 0 &&
        context->getState().getTargetBuffer(BufferBinding::Array) == 0 && pointer != nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kClientDataInVertexArray);
        return false;
    }

    if (context->getExtensions().webglCompatibilityANGLE)
    {
        if (!ValidateWebGLVertexAttribPointer(context, entryPoint, type, false, stride, pointer,
                                              true))
        {
            return false;
        }
    }

    return true;
}

bool ValidateGetSynciv(const Context *context,
                       angle::EntryPoint entryPoint,
                       SyncID syncPacked,
                       GLenum pname,
                       GLsizei bufSize,
                       const GLsizei *length,
                       const GLint *values)
{
    if ((context->getClientMajorVersion() < 3) && !context->getExtensions().syncARB)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (bufSize < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeBufSize);
        return false;
    }

    if (context->isContextLost())
    {
        ANGLE_VALIDATION_ERROR(GL_CONTEXT_LOST, kContextLost);

        if (pname == GL_SYNC_STATUS)
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

    Sync *syncObject = context->getSync(syncPacked);
    if (!syncObject)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kSyncMissing);
        return false;
    }

    switch (pname)
    {
        case GL_OBJECT_TYPE:
        case GL_SYNC_CONDITION:
        case GL_SYNC_FLAGS:
        case GL_SYNC_STATUS:
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidPname);
            return false;
    }

    return true;
}

bool ValidateDrawElementsInstanced(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   PrimitiveMode mode,
                                   GLsizei count,
                                   DrawElementsType type,
                                   const void *indices,
                                   GLsizei instanceCount)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateDrawElementsInstancedBase(context, entryPoint, mode, count, type, indices,
                                             instanceCount, 0);
}

bool ValidateMultiDrawArraysInstancedANGLE(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           PrimitiveMode mode,
                                           const GLint *firsts,
                                           const GLsizei *counts,
                                           const GLsizei *instanceCounts,
                                           GLsizei drawcount)
{
    if (!context->getExtensions().multiDrawANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }
    if (context->getClientMajorVersion() < 3)
    {
        if (!context->getExtensions().instancedArraysAny())
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
            return false;
        }
        if (!ValidateDrawInstancedANGLE(context, entryPoint))
        {
            return false;
        }
    }
    for (GLsizei drawID = 0; drawID < drawcount; ++drawID)
    {
        if (!ValidateDrawArraysInstancedBase(context, entryPoint, mode, firsts[drawID],
                                             counts[drawID], instanceCounts[drawID], 0))
        {
            return false;
        }
    }
    return true;
}

bool ValidateMultiDrawElementsInstancedANGLE(const Context *context,
                                             angle::EntryPoint entryPoint,
                                             PrimitiveMode mode,
                                             const GLsizei *counts,
                                             DrawElementsType type,
                                             const GLvoid *const *indices,
                                             const GLsizei *instanceCounts,
                                             GLsizei drawcount)
{
    if (!context->getExtensions().multiDrawANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }
    if (context->getClientMajorVersion() < 3)
    {
        if (!context->getExtensions().instancedArraysAny())
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
            return false;
        }
        if (!ValidateDrawInstancedANGLE(context, entryPoint))
        {
            return false;
        }
    }
    for (GLsizei drawID = 0; drawID < drawcount; ++drawID)
    {
        if (!ValidateDrawElementsInstancedBase(context, entryPoint, mode, counts[drawID], type,
                                               indices[drawID], instanceCounts[drawID], 0))
        {
            return false;
        }
    }
    return true;
}

bool ValidateDrawArraysInstancedBaseInstanceANGLE(const Context *context,
                                                  angle::EntryPoint entryPoint,
                                                  PrimitiveMode mode,
                                                  GLint first,
                                                  GLsizei count,
                                                  GLsizei instanceCount,
                                                  GLuint baseInstance)
{
    if (!context->getExtensions().baseVertexBaseInstanceANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateDrawArraysInstancedBase(context, entryPoint, mode, first, count, instanceCount,
                                           baseInstance);
}

bool ValidateDrawElementsInstancedBaseVertexBaseInstanceANGLE(const Context *context,
                                                              angle::EntryPoint entryPoint,
                                                              PrimitiveMode mode,
                                                              GLsizei count,
                                                              DrawElementsType type,
                                                              const GLvoid *indices,
                                                              GLsizei instanceCount,
                                                              GLint baseVertex,
                                                              GLuint baseInstance)
{
    if (!context->getExtensions().baseVertexBaseInstanceANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateDrawElementsInstancedBase(context, entryPoint, mode, count, type, indices,
                                             instanceCount, baseInstance);
}

bool ValidateMultiDrawArraysInstancedBaseInstanceANGLE(const Context *context,
                                                       angle::EntryPoint entryPoint,
                                                       PrimitiveMode modePacked,
                                                       const GLint *firsts,
                                                       const GLsizei *counts,
                                                       const GLsizei *instanceCounts,
                                                       const GLuint *baseInstances,
                                                       GLsizei drawcount)
{
    if (!context->getExtensions().multiDrawANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }
    if (drawcount < 0)
    {
        return false;
    }
    for (GLsizei drawID = 0; drawID < drawcount; ++drawID)
    {
        if (!ValidateDrawArraysInstancedBase(context, entryPoint, modePacked, firsts[drawID],
                                             counts[drawID], instanceCounts[drawID],
                                             baseInstances[drawID]))
        {
            return false;
        }
    }
    return true;
}

bool ValidateMultiDrawElementsInstancedBaseVertexBaseInstanceANGLE(const Context *context,
                                                                   angle::EntryPoint entryPoint,
                                                                   PrimitiveMode modePacked,
                                                                   const GLsizei *counts,
                                                                   DrawElementsType typePacked,
                                                                   const GLvoid *const *indices,
                                                                   const GLsizei *instanceCounts,
                                                                   const GLint *baseVertices,
                                                                   const GLuint *baseInstances,
                                                                   GLsizei drawcount)
{
    if (!context->getExtensions().multiDrawANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }
    if (drawcount < 0)
    {
        return false;
    }
    for (GLsizei drawID = 0; drawID < drawcount; ++drawID)
    {
        if (!ValidateDrawElementsInstancedBase(context, entryPoint, modePacked, counts[drawID],
                                               typePacked, indices[drawID], instanceCounts[drawID],
                                               baseInstances[drawID]))
        {
            return false;
        }
    }
    return true;
}

bool ValidateFramebufferTextureMultiviewOVR(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            GLenum target,
                                            GLenum attachment,
                                            TextureID texture,
                                            GLint level,
                                            GLint baseViewIndex,
                                            GLsizei numViews)
{
    if (!ValidateFramebufferTextureMultiviewBaseANGLE(context, entryPoint, target, attachment,
                                                      texture, level, numViews))
    {
        return false;
    }

    if (texture.value != 0)
    {
        if (baseViewIndex < 0)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeBaseViewIndex);
            return false;
        }

        Texture *tex = context->getTexture(texture);
        ASSERT(tex);

        switch (tex->getType())
        {
            case TextureType::_2DArray:
            case TextureType::_2DMultisampleArray:
            {
                if (tex->getType() == TextureType::_2DMultisampleArray)
                {
                    if (!context->getExtensions().multiviewMultisampleANGLE)
                    {
                        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidTextureType);
                        return false;
                    }
                }

                const Caps &caps = context->getCaps();
                if (baseViewIndex + numViews > caps.maxArrayTextureLayers)
                {
                    ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kViewsExceedMaxArrayLayers);
                    return false;
                }

                break;
            }
            default:
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidTextureType);
                return false;
        }

        if (!ValidateFramebufferTextureMultiviewLevelAndFormat(context, entryPoint, tex, level))
        {
            return false;
        }
    }

    return true;
}

bool ValidateUniform1ui(const Context *context,
                        angle::EntryPoint entryPoint,
                        UniformLocation location,
                        GLuint v0)
{
    return ValidateUniformES3(context, entryPoint, GL_UNSIGNED_INT, location, 1);
}

bool ValidateUniform2ui(const Context *context,
                        angle::EntryPoint entryPoint,
                        UniformLocation location,
                        GLuint v0,
                        GLuint v1)
{
    return ValidateUniformES3(context, entryPoint, GL_UNSIGNED_INT_VEC2, location, 1);
}

bool ValidateUniform3ui(const Context *context,
                        angle::EntryPoint entryPoint,
                        UniformLocation location,
                        GLuint v0,
                        GLuint v1,
                        GLuint v2)
{
    return ValidateUniformES3(context, entryPoint, GL_UNSIGNED_INT_VEC3, location, 1);
}

bool ValidateUniform4ui(const Context *context,
                        angle::EntryPoint entryPoint,
                        UniformLocation location,
                        GLuint v0,
                        GLuint v1,
                        GLuint v2,
                        GLuint v3)
{
    return ValidateUniformES3(context, entryPoint, GL_UNSIGNED_INT_VEC4, location, 1);
}

bool ValidateUniform1uiv(const Context *context,
                         angle::EntryPoint entryPoint,
                         UniformLocation location,
                         GLsizei count,
                         const GLuint *value)
{
    return ValidateUniformES3(context, entryPoint, GL_UNSIGNED_INT, location, count);
}

bool ValidateUniform2uiv(const Context *context,
                         angle::EntryPoint entryPoint,
                         UniformLocation location,
                         GLsizei count,
                         const GLuint *value)
{
    return ValidateUniformES3(context, entryPoint, GL_UNSIGNED_INT_VEC2, location, count);
}

bool ValidateUniform3uiv(const Context *context,
                         angle::EntryPoint entryPoint,
                         UniformLocation location,
                         GLsizei count,
                         const GLuint *value)
{
    return ValidateUniformES3(context, entryPoint, GL_UNSIGNED_INT_VEC3, location, count);
}

bool ValidateUniform4uiv(const Context *context,
                         angle::EntryPoint entryPoint,
                         UniformLocation location,
                         GLsizei count,
                         const GLuint *value)
{
    return ValidateUniformES3(context, entryPoint, GL_UNSIGNED_INT_VEC4, location, count);
}

bool ValidateIsQuery(const Context *context, angle::EntryPoint entryPoint, QueryID id)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return true;
}

bool ValidateUniformMatrix2x3fv(const Context *context,
                                angle::EntryPoint entryPoint,
                                UniformLocation location,
                                GLsizei count,
                                GLboolean transpose,
                                const GLfloat *value)
{
    return ValidateUniformMatrixES3(context, entryPoint, GL_FLOAT_MAT2x3, location, count,
                                    transpose);
}

bool ValidateUniformMatrix3x2fv(const Context *context,
                                angle::EntryPoint entryPoint,
                                UniformLocation location,
                                GLsizei count,
                                GLboolean transpose,
                                const GLfloat *value)
{
    return ValidateUniformMatrixES3(context, entryPoint, GL_FLOAT_MAT3x2, location, count,
                                    transpose);
}

bool ValidateUniformMatrix2x4fv(const Context *context,
                                angle::EntryPoint entryPoint,
                                UniformLocation location,
                                GLsizei count,
                                GLboolean transpose,
                                const GLfloat *value)
{
    return ValidateUniformMatrixES3(context, entryPoint, GL_FLOAT_MAT2x4, location, count,
                                    transpose);
}

bool ValidateUniformMatrix4x2fv(const Context *context,
                                angle::EntryPoint entryPoint,
                                UniformLocation location,
                                GLsizei count,
                                GLboolean transpose,
                                const GLfloat *value)
{
    return ValidateUniformMatrixES3(context, entryPoint, GL_FLOAT_MAT4x2, location, count,
                                    transpose);
}

bool ValidateUniformMatrix3x4fv(const Context *context,
                                angle::EntryPoint entryPoint,
                                UniformLocation location,
                                GLsizei count,
                                GLboolean transpose,
                                const GLfloat *value)
{
    return ValidateUniformMatrixES3(context, entryPoint, GL_FLOAT_MAT3x4, location, count,
                                    transpose);
}

bool ValidateUniformMatrix4x3fv(const Context *context,
                                angle::EntryPoint entryPoint,
                                UniformLocation location,
                                GLsizei count,
                                GLboolean transpose,
                                const GLfloat *value)
{
    return ValidateUniformMatrixES3(context, entryPoint, GL_FLOAT_MAT4x3, location, count,
                                    transpose);
}

bool ValidateEndTransformFeedback(const Context *context, angle::EntryPoint entryPoint)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    TransformFeedback *transformFeedback = context->getState().getCurrentTransformFeedback();
    ASSERT(transformFeedback != nullptr);

    if (!transformFeedback->isActive())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTransformFeedbackNotActive);
        return false;
    }

    return true;
}

bool ValidateTransformFeedbackVaryings(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       ShaderProgramID program,
                                       GLsizei count,
                                       const GLchar *const *varyings,
                                       GLenum bufferMode)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (count < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeCount);
        return false;
    }

    switch (bufferMode)
    {
        case GL_INTERLEAVED_ATTRIBS:
            break;
        case GL_SEPARATE_ATTRIBS:
        {
            const Caps &caps = context->getCaps();
            if (count > caps.maxTransformFeedbackSeparateAttributes)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidTransformFeedbackAttribsCount);
                return false;
            }
            break;
        }
        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, bufferMode);
            return false;
    }

    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (!programObject)
    {
        return false;
    }

    return true;
}

bool ValidateGetTransformFeedbackVarying(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         ShaderProgramID program,
                                         GLuint index,
                                         GLsizei bufSize,
                                         const GLsizei *length,
                                         const GLsizei *size,
                                         const GLenum *type,
                                         const GLchar *name)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (bufSize < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeBufSize);
        return false;
    }

    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (!programObject)
    {
        return false;
    }

    if (index >= static_cast<GLuint>(
                     programObject->getExecutable().getLinkedTransformFeedbackVaryings().size()))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kTransformFeedbackVaryingIndexOutOfRange);
        return false;
    }

    return true;
}

bool ValidateBindTransformFeedback(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   GLenum target,
                                   TransformFeedbackID id)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    switch (target)
    {
        case GL_TRANSFORM_FEEDBACK:
        {
            // Cannot bind a transform feedback object if the current one is started and not
            // paused (3.0.2 pg 85 section 2.14.1)
            if (context->getState().isTransformFeedbackActiveUnpaused())
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTransformFeedbackNotPaused);
                return false;
            }

            // Cannot bind a transform feedback object that does not exist (3.0.2 pg 85 section
            // 2.14.1)
            if (!context->isTransformFeedbackGenerated(id))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTransformFeedbackDoesNotExist);
                return false;
            }
        }
        break;

        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, target);
            return false;
    }

    return true;
}

bool ValidateIsTransformFeedback(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 TransformFeedbackID id)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return true;
}

bool ValidatePauseTransformFeedback(const Context *context, angle::EntryPoint entryPoint)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    TransformFeedback *transformFeedback = context->getState().getCurrentTransformFeedback();
    ASSERT(transformFeedback != nullptr);

    // Current transform feedback must be active and not paused in order to pause (3.0.2 pg 86)
    if (!transformFeedback->isActive())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTransformFeedbackNotActive);
        return false;
    }

    if (transformFeedback->isPaused())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTransformFeedbackPaused);
        return false;
    }

    return true;
}

bool ValidateResumeTransformFeedback(const Context *context, angle::EntryPoint entryPoint)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    TransformFeedback *transformFeedback = context->getState().getCurrentTransformFeedback();
    ASSERT(transformFeedback != nullptr);

    // Current transform feedback must be active and paused in order to resume (3.0.2 pg 86)
    if (!transformFeedback->isActive())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTransformFeedbackNotActive);
        return false;
    }

    if (!transformFeedback->isPaused())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTransformFeedbackNotPaused);
        return false;
    }

    if (!ValidateProgramExecutableXFBBuffersPresent(
            context, context->getState().getLinkedProgramExecutable(context)))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTransformFeedbackBufferMissing);
        return false;
    }

    return true;
}

bool ValidateVertexAttribI4i(const PrivateState &state,
                             ErrorSet *errors,
                             angle::EntryPoint entryPoint,
                             GLuint index,
                             GLint x,
                             GLint y,
                             GLint z,
                             GLint w)
{
    if (state.getClientMajorVersion() < 3)
    {
        errors->validationError(entryPoint, GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateVertexAttribIndex(state, errors, entryPoint, index);
}

bool ValidateVertexAttribI4ui(const PrivateState &state,
                              ErrorSet *errors,
                              angle::EntryPoint entryPoint,
                              GLuint index,
                              GLuint x,
                              GLuint y,
                              GLuint z,
                              GLuint w)
{
    if (state.getClientMajorVersion() < 3)
    {
        errors->validationError(entryPoint, GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateVertexAttribIndex(state, errors, entryPoint, index);
}

bool ValidateVertexAttribI4iv(const PrivateState &state,
                              ErrorSet *errors,
                              angle::EntryPoint entryPoint,
                              GLuint index,
                              const GLint *v)
{
    if (state.getClientMajorVersion() < 3)
    {
        errors->validationError(entryPoint, GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateVertexAttribIndex(state, errors, entryPoint, index);
}

bool ValidateVertexAttribI4uiv(const PrivateState &state,
                               ErrorSet *errors,
                               angle::EntryPoint entryPoint,
                               GLuint index,
                               const GLuint *v)
{
    if (state.getClientMajorVersion() < 3)
    {
        errors->validationError(entryPoint, GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateVertexAttribIndex(state, errors, entryPoint, index);
}

bool ValidateGetFragDataLocation(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 ShaderProgramID program,
                                 const GLchar *name)
{
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

    if (!programObject->isLinked())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kProgramNotLinked);
        return false;
    }

    return true;
}

bool ValidateGetUniformIndices(const Context *context,
                               angle::EntryPoint entryPoint,
                               ShaderProgramID program,
                               GLsizei uniformCount,
                               const GLchar *const *uniformNames,
                               const GLuint *uniformIndices)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (uniformCount < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeCount);
        return false;
    }

    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (!programObject)
    {
        return false;
    }

    return true;
}

bool ValidateGetActiveUniformsiv(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 ShaderProgramID program,
                                 GLsizei uniformCount,
                                 const GLuint *uniformIndices,
                                 GLenum pname,
                                 const GLint *params)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (uniformCount < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeCount);
        return false;
    }

    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (!programObject)
    {
        return false;
    }

    switch (pname)
    {
        case GL_UNIFORM_TYPE:
        case GL_UNIFORM_SIZE:
            break;
        case GL_UNIFORM_NAME_LENGTH:
            if (context->getExtensions().webglCompatibilityANGLE)
            {
                ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
                return false;
            }
            break;
        case GL_UNIFORM_BLOCK_INDEX:
        case GL_UNIFORM_OFFSET:
        case GL_UNIFORM_ARRAY_STRIDE:
        case GL_UNIFORM_MATRIX_STRIDE:
        case GL_UNIFORM_IS_ROW_MAJOR:
            break;

        default:
            ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, kEnumNotSupported, pname);
            return false;
    }

    const size_t programUniformCount = programObject->getExecutable().getUniforms().size();
    if (uniformCount > static_cast<GLsizei>(programUniformCount))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxActiveUniform);
        return false;
    }

    for (int uniformId = 0; uniformId < uniformCount; uniformId++)
    {
        const GLuint index = uniformIndices[uniformId];

        if (index >= programUniformCount)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxActiveUniform);
            return false;
        }
    }

    return true;
}

bool ValidateGetUniformBlockIndex(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID program,
                                  const GLchar *uniformBlockName)
{
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

    return true;
}

bool ValidateGetActiveUniformBlockiv(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     ShaderProgramID program,
                                     UniformBlockIndex uniformBlockIndex,
                                     GLenum pname,
                                     const GLint *params)
{
    return ValidateGetActiveUniformBlockivBase(context, entryPoint, program, uniformBlockIndex,
                                               pname, nullptr);
}

bool ValidateGetActiveUniformBlockName(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       ShaderProgramID program,
                                       UniformBlockIndex uniformBlockIndex,
                                       GLsizei bufSize,
                                       const GLsizei *length,
                                       const GLchar *uniformBlockName)
{
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

    if (uniformBlockIndex.value >= programObject->getExecutable().getUniformBlocks().size())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxActiveUniformBlock);
        return false;
    }

    return true;
}

bool ValidateUniformBlockBinding(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 ShaderProgramID program,
                                 UniformBlockIndex uniformBlockIndex,
                                 GLuint uniformBlockBinding)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (uniformBlockBinding >= static_cast<GLuint>(context->getCaps().maxUniformBufferBindings))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxUniformBufferBindings);
        return false;
    }

    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (!programObject)
    {
        return false;
    }

    // if never linked, there won't be any uniform blocks
    if (uniformBlockIndex.value >= programObject->getExecutable().getUniformBlocks().size())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxUniformBufferBindings);
        return false;
    }

    return true;
}

bool ValidateDrawArraysInstanced(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 PrimitiveMode mode,
                                 GLint first,
                                 GLsizei count,
                                 GLsizei primcount)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateDrawArraysInstancedBase(context, entryPoint, mode, first, count, primcount, 0);
}

bool ValidateFenceSync(const Context *context,
                       angle::EntryPoint entryPoint,
                       GLenum condition,
                       GLbitfield flags)
{
    if ((context->getClientMajorVersion() < 3) && !context->getExtensions().syncARB)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (condition != GL_SYNC_GPU_COMMANDS_COMPLETE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidFenceCondition);
        return false;
    }

    if (flags != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidFlags);
        return false;
    }

    return true;
}

bool ValidateIsSync(const Context *context, angle::EntryPoint entryPoint, SyncID syncPacked)
{
    if ((context->getClientMajorVersion() < 3) && !context->getExtensions().syncARB)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return true;
}

bool ValidateDeleteSync(const Context *context, angle::EntryPoint entryPoint, SyncID syncPacked)
{
    if ((context->getClientMajorVersion() < 3) && !context->getExtensions().syncARB)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (syncPacked.value != 0 && !context->getSync(syncPacked))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kSyncMissing);
        return false;
    }

    return true;
}

bool ValidateClientWaitSync(const Context *context,
                            angle::EntryPoint entryPoint,
                            SyncID syncPacked,
                            GLbitfield flags,
                            GLuint64 timeout)
{
    if ((context->getClientMajorVersion() < 3) && !context->getExtensions().syncARB)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if ((flags & ~(GL_SYNC_FLUSH_COMMANDS_BIT)) != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidFlags);
        return false;
    }

    Sync *clientWaitSync = context->getSync(syncPacked);
    if (!clientWaitSync)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kSyncMissing);
        return false;
    }

    return true;
}

bool ValidateWaitSync(const Context *context,
                      angle::EntryPoint entryPoint,
                      SyncID syncPacked,
                      GLbitfield flags,
                      GLuint64 timeout)
{
    if ((context->getClientMajorVersion() < 3) && !context->getExtensions().syncARB)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (flags != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidFlags);
        return false;
    }

    if (timeout != GL_TIMEOUT_IGNORED)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidTimeout);
        return false;
    }

    Sync *waitSync = context->getSync(syncPacked);
    if (!waitSync)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kSyncMissing);
        return false;
    }

    return true;
}

bool ValidateGetInteger64v(const Context *context,
                           angle::EntryPoint entryPoint,
                           GLenum pname,
                           const GLint64 *params)
{
    if ((context->getClientMajorVersion() < 3) && !context->getExtensions().syncARB)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    GLenum nativeType      = GL_NONE;
    unsigned int numParams = 0;
    if (!ValidateStateQuery(context, entryPoint, pname, &nativeType, &numParams))
    {
        return false;
    }

    return true;
}

bool ValidateIsSampler(const Context *context, angle::EntryPoint entryPoint, SamplerID sampler)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return true;
}

bool ValidateBindSampler(const Context *context,
                         angle::EntryPoint entryPoint,
                         GLuint unit,
                         SamplerID sampler)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (GetIDValue(sampler) != 0 && !context->isSampler(sampler))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidSampler);
        return false;
    }

    if (unit >= static_cast<GLuint>(context->getCaps().maxCombinedTextureImageUnits))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidCombinedImageUnit);
        return false;
    }

    return true;
}

bool ValidateVertexAttribDivisor(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 GLuint index,
                                 GLuint divisor)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    return ValidateVertexAttribIndex(context->getPrivateState(),
                                     context->getMutableErrorSetForValidation(), entryPoint, index);
}

bool ValidateTexStorage2D(const Context *context,
                          angle::EntryPoint entryPoint,
                          TextureType target,
                          GLsizei levels,
                          GLenum internalformat,
                          GLsizei width,
                          GLsizei height)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (!ValidateES3TexStorage2DParameters(context, entryPoint, target, levels, internalformat,
                                           width, height, 1))
    {
        return false;
    }

    return true;
}

bool ValidateTexStorage3D(const Context *context,
                          angle::EntryPoint entryPoint,
                          TextureType target,
                          GLsizei levels,
                          GLenum internalformat,
                          GLsizei width,
                          GLsizei height,
                          GLsizei depth)
{
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }

    if (!ValidateES3TexStorage3DParameters(context, entryPoint, target, levels, internalformat,
                                           width, height, depth))
    {
        return false;
    }

    return true;
}

bool ValidateGetBufferParameteri64v(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    BufferBinding target,
                                    GLenum pname,
                                    const GLint64 *params)
{
    return ValidateGetBufferParameterBase(context, entryPoint, target, pname, false, nullptr);
}

bool ValidateGetSamplerParameterfv(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   SamplerID sampler,
                                   GLenum pname,
                                   const GLfloat *params)
{
    return ValidateGetSamplerParameterBase(context, entryPoint, sampler, pname, nullptr);
}

bool ValidateGetSamplerParameteriv(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   SamplerID sampler,
                                   GLenum pname,
                                   const GLint *params)
{
    return ValidateGetSamplerParameterBase(context, entryPoint, sampler, pname, nullptr);
}

bool ValidateSamplerParameterf(const Context *context,
                               angle::EntryPoint entryPoint,
                               SamplerID sampler,
                               GLenum pname,
                               GLfloat param)
{
    return ValidateSamplerParameterBase(context, entryPoint, sampler, pname, -1, false, &param);
}

bool ValidateSamplerParameterfv(const Context *context,
                                angle::EntryPoint entryPoint,
                                SamplerID sampler,
                                GLenum pname,
                                const GLfloat *params)
{
    return ValidateSamplerParameterBase(context, entryPoint, sampler, pname, -1, true, params);
}

bool ValidateSamplerParameteri(const Context *context,
                               angle::EntryPoint entryPoint,
                               SamplerID sampler,
                               GLenum pname,
                               GLint param)
{
    return ValidateSamplerParameterBase(context, entryPoint, sampler, pname, -1, false, &param);
}

bool ValidateSamplerParameteriv(const Context *context,
                                angle::EntryPoint entryPoint,
                                SamplerID sampler,
                                GLenum pname,
                                const GLint *params)
{
    return ValidateSamplerParameterBase(context, entryPoint, sampler, pname, -1, true, params);
}

bool ValidateGetVertexAttribIiv(const Context *context,
                                angle::EntryPoint entryPoint,
                                GLuint index,
                                GLenum pname,
                                const GLint *params)
{
    return ValidateGetVertexAttribBase(context, entryPoint, index, pname, nullptr, false, true);
}

bool ValidateGetVertexAttribIuiv(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 GLuint index,
                                 GLenum pname,
                                 const GLuint *params)
{
    return ValidateGetVertexAttribBase(context, entryPoint, index, pname, nullptr, false, true);
}

bool ValidateGetInternalformativ(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 GLenum target,
                                 GLenum internalformat,
                                 GLenum pname,
                                 GLsizei bufSize,
                                 const GLint *params)
{
    return ValidateGetInternalFormativBase(context, entryPoint, target, internalformat, pname,
                                           bufSize, nullptr);
}

bool ValidateBindFragDataLocationIndexedEXT(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            ShaderProgramID program,
                                            GLuint colorNumber,
                                            GLuint index,
                                            const char *name)
{
    if (!context->getExtensions().blendFuncExtendedEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }
    if (context->getClientMajorVersion() < 3)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES3Required);
        return false;
    }
    if (index > 1)
    {
        // This error is not explicitly specified but the spec does say that "<index> may be zero or
        // one to specify that the color be used as either the first or second color input to the
        // blend equation, respectively"
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kFragDataBindingIndexOutOfRange);
        return false;
    }
    if (index == 1)
    {
        if (colorNumber >= context->getCaps().maxDualSourceDrawBuffers)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE,
                                   kColorNumberGreaterThanMaxDualSourceDrawBuffers);
            return false;
        }
    }
    else
    {
        if (colorNumber >= static_cast<GLuint>(context->getCaps().maxDrawBuffers))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kColorNumberGreaterThanMaxDrawBuffers);
            return false;
        }
    }
    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (!programObject)
    {
        return false;
    }
    return true;
}

bool ValidateBindFragDataLocationEXT(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     ShaderProgramID program,
                                     GLuint colorNumber,
                                     const char *name)
{
    return ValidateBindFragDataLocationIndexedEXT(context, entryPoint, program, colorNumber, 0u,
                                                  name);
}

bool ValidateGetFragDataIndexEXT(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 ShaderProgramID program,
                                 const char *name)
{
    if (!context->getExtensions().blendFuncExtendedEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
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
    if (!programObject->isLinked())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kProgramNotLinked);
        return false;
    }
    return true;
}

bool ValidateTexStorage2DMultisampleANGLE(const Context *context,
                                          angle::EntryPoint entryPoint,
                                          TextureType target,
                                          GLsizei samples,
                                          GLenum internalFormat,
                                          GLsizei width,
                                          GLsizei height,
                                          GLboolean fixedSampleLocations)
{
    if (!context->getExtensions().textureMultisampleANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMultisampleTextureExtensionOrES31Required);
        return false;
    }

    return ValidateTexStorage2DMultisampleBase(context, entryPoint, target, samples, internalFormat,
                                               width, height);
}

bool ValidateGetTexLevelParameterfvANGLE(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         TextureTarget target,
                                         GLint level,
                                         GLenum pname,
                                         const GLfloat *params)
{
    if (!context->getExtensions().getTexLevelParameterANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateGetTexLevelParameterBase(context, entryPoint, target, level, pname, nullptr);
}

bool ValidateGetTexLevelParameterivANGLE(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         TextureTarget target,
                                         GLint level,
                                         GLenum pname,
                                         const GLint *params)
{
    if (!context->getExtensions().getTexLevelParameterANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return ValidateGetTexLevelParameterBase(context, entryPoint, target, level, pname, nullptr);
}

bool ValidateGetMultisamplefvANGLE(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   GLenum pname,
                                   GLuint index,
                                   const GLfloat *val)
{
    if (!context->getExtensions().textureMultisampleANGLE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMultisampleTextureExtensionOrES31Required);
        return false;
    }

    return ValidateGetMultisamplefvBase(context, entryPoint, pname, index, val);
}

bool ValidateSampleMaskiANGLE(const PrivateState &state,
                              ErrorSet *errors,
                              angle::EntryPoint entryPoint,
                              GLuint maskNumber,
                              GLbitfield mask)
{
    if (!state.getExtensions().textureMultisampleANGLE)
    {
        errors->validationError(entryPoint, GL_INVALID_OPERATION,
                                kMultisampleTextureExtensionOrES31Required);
        return false;
    }

    return ValidateSampleMaskiBase(state, errors, entryPoint, maskNumber, mask);
}
}  // namespace gl
