//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// formatutils.cpp: Queries for GL image formats.

#include "libANGLE/formatutils.h"

#include "anglebase/no_destructor.h"
#include "common/mathutil.h"
#include "gpu_info_util/SystemInfo.h"
#include "libANGLE/Context.h"
#include "libANGLE/Framebuffer.h"

using namespace angle;

namespace gl
{

// ES2 requires that format is equal to internal format at all glTex*Image2D entry points and the
// implementation can decide the true, sized, internal format. The ES2FormatMap determines the
// internal format for all valid format and type combinations.
GLenum GetSizedFormatInternal(GLenum format, GLenum type);

namespace
{
bool CheckedMathResult(const CheckedNumeric<GLuint> &value, GLuint *resultOut)
{
    if (!value.IsValid())
    {
        return false;
    }
    else
    {
        *resultOut = value.ValueOrDie();
        return true;
    }
}

constexpr uint32_t PackTypeInfo(GLuint bytes, bool specialized)
{
    // static_assert within constexpr requires c++17
    // static_assert(isPow2(bytes));
    return bytes | (rx::Log2(bytes) << 8) | (specialized << 16);
}

}  // anonymous namespace

FormatType::FormatType() : format(GL_NONE), type(GL_NONE) {}

FormatType::FormatType(GLenum format_, GLenum type_) : format(format_), type(type_) {}

bool FormatType::operator<(const FormatType &other) const
{
    if (format != other.format)
        return format < other.format;
    return type < other.type;
}

bool operator<(const Type &a, const Type &b)
{
    return memcmp(&a, &b, sizeof(Type)) < 0;
}

// Information about internal formats
static bool AlwaysSupported(const Version &, const Extensions &)
{
    return true;
}

static bool NeverSupported(const Version &, const Extensions &)
{
    return false;
}

static bool RequireES1(const Version &clientVersion, const Extensions &extensions)
{
    return clientVersion.major == 1;
}

template <GLuint minCoreGLMajorVersion, GLuint minCoreGLMinorVersion>
static bool RequireES(const Version &clientVersion, const Extensions &)
{
    return clientVersion >= Version(minCoreGLMajorVersion, minCoreGLMinorVersion);
}

// Check support for a single extension
template <ExtensionBool bool1>
static bool RequireExt(const Version &, const Extensions &extensions)
{
    return extensions.*bool1;
}

// Check for a minimum client version or a single extension
template <GLuint minCoreGLMajorVersion, GLuint minCoreGLMinorVersion, ExtensionBool bool1>
static bool RequireESOrExt(const Version &clientVersion, const Extensions &extensions)
{
    return clientVersion >= Version(minCoreGLMajorVersion, minCoreGLMinorVersion) ||
           extensions.*bool1;
}

// Check for a minimum client version or two extensions
template <GLuint minCoreGLMajorVersion,
          GLuint minCoreGLMinorVersion,
          ExtensionBool bool1,
          ExtensionBool bool2>
static bool RequireESOrExtAndExt(const Version &clientVersion, const Extensions &extensions)
{
    return clientVersion >= Version(minCoreGLMajorVersion, minCoreGLMinorVersion) ||
           (extensions.*bool1 && extensions.*bool2);
}

// Check for a minimum client version or at least one of two extensions
template <GLuint minCoreGLMajorVersion,
          GLuint minCoreGLMinorVersion,
          ExtensionBool bool1,
          ExtensionBool bool2>
static bool RequireESOrExtOrExt(const Version &clientVersion, const Extensions &extensions)
{
    return clientVersion >= Version(minCoreGLMajorVersion, minCoreGLMinorVersion) ||
           extensions.*bool1 || extensions.*bool2;
}

// Check support for two extensions
template <ExtensionBool bool1, ExtensionBool bool2>
static bool RequireExtAndExt(const Version &, const Extensions &extensions)
{
    return extensions.*bool1 && extensions.*bool2;
}

// Check support for either of two extensions
template <ExtensionBool bool1, ExtensionBool bool2>
static bool RequireExtOrExt(const Version &, const Extensions &extensions)
{
    return extensions.*bool1 || extensions.*bool2;
}

// Check support for any of three extensions
template <ExtensionBool bool1, ExtensionBool bool2, ExtensionBool bool3>
static bool RequireExtOrExtOrExt(const Version &, const Extensions &extensions)
{
    return extensions.*bool1 || extensions.*bool2 || extensions.*bool3;
}

// R8, RG8
static bool SizedRGSupport(const Version &clientVersion, const Extensions &extensions)
{
    return clientVersion >= Version(3, 0) ||
           (extensions.textureStorageEXT && extensions.textureRgEXT);
}

// R16F, RG16F with HALF_FLOAT_OES type
static bool SizedHalfFloatOESRGSupport(const Version &clientVersion, const Extensions &extensions)
{
    return extensions.textureStorageEXT && extensions.textureHalfFloatOES &&
           extensions.textureRgEXT;
}

static bool SizedHalfFloatOESRGTextureAttachmentSupport(const Version &clientVersion,
                                                        const Extensions &extensions)
{
    return SizedHalfFloatOESRGSupport(clientVersion, extensions) &&
           extensions.colorBufferHalfFloatEXT;
}

// R16F, RG16F with either HALF_FLOAT_OES or HALF_FLOAT types
static bool SizedHalfFloatRGSupport(const Version &clientVersion, const Extensions &extensions)
{
    // HALF_FLOAT
    if (clientVersion >= Version(3, 0))
    {
        return true;
    }
    // HALF_FLOAT_OES
    else
    {
        return SizedHalfFloatOESRGSupport(clientVersion, extensions);
    }
}

static bool SizedHalfFloatRGTextureAttachmentSupport(const Version &clientVersion,
                                                     const Extensions &extensions)
{
    // HALF_FLOAT
    if (clientVersion >= Version(3, 0))
    {
        // WebGL 2 supports EXT_color_buffer_half_float.
        return extensions.colorBufferFloatEXT ||
               (extensions.webglCompatibilityANGLE && extensions.colorBufferHalfFloatEXT);
    }
    // HALF_FLOAT_OES
    else
    {
        return SizedHalfFloatOESRGTextureAttachmentSupport(clientVersion, extensions);
    }
}

static bool SizedHalfFloatRGRenderbufferSupport(const Version &clientVersion,
                                                const Extensions &extensions)
{
    return (clientVersion >= Version(3, 0) ||
            (extensions.textureHalfFloatOES && extensions.textureRgEXT)) &&
           (extensions.colorBufferFloatEXT || extensions.colorBufferHalfFloatEXT);
}

// RGB16F, RGBA16F with HALF_FLOAT_OES type
static bool SizedHalfFloatOESSupport(const Version &clientVersion, const Extensions &extensions)
{
    return extensions.textureStorageEXT && extensions.textureHalfFloatOES;
}

static bool SizedHalfFloatOESTextureAttachmentSupport(const Version &clientVersion,
                                                      const Extensions &extensions)
{
    return SizedHalfFloatOESSupport(clientVersion, extensions) &&
           extensions.colorBufferHalfFloatEXT;
}

// RGB16F, RGBA16F with either HALF_FLOAT_OES or HALF_FLOAT types
static bool SizedHalfFloatSupport(const Version &clientVersion, const Extensions &extensions)
{
    // HALF_FLOAT
    if (clientVersion >= Version(3, 0))
    {
        return true;
    }
    // HALF_FLOAT_OES
    else
    {
        return SizedHalfFloatOESSupport(clientVersion, extensions);
    }
}

static bool SizedHalfFloatFilterSupport(const Version &clientVersion, const Extensions &extensions)
{
    // HALF_FLOAT
    if (clientVersion >= Version(3, 0))
    {
        return true;
    }
    // HALF_FLOAT_OES
    else
    {
        return extensions.textureHalfFloatLinearOES;
    }
}

static bool SizedHalfFloatRGBTextureAttachmentSupport(const Version &clientVersion,
                                                      const Extensions &extensions)
{
    // HALF_FLOAT
    if (clientVersion >= Version(3, 0))
    {
        // It is unclear how EXT_color_buffer_half_float applies to ES3.0 and above, however,
        // dEQP GLES3 es3fFboColorbufferTests.cpp verifies that texture attachment of GL_RGB16F
        // is possible, so assume that all GLES implementations support it.
        // The WebGL version of the extension explicitly forbids RGB formats.
        return extensions.colorBufferHalfFloatEXT && !extensions.webglCompatibilityANGLE;
    }
    // HALF_FLOAT_OES
    else
    {
        return SizedHalfFloatOESTextureAttachmentSupport(clientVersion, extensions);
    }
}

static bool SizedHalfFloatRGBRenderbufferSupport(const Version &clientVersion,
                                                 const Extensions &extensions)
{
    return !extensions.webglCompatibilityANGLE &&
           ((clientVersion >= Version(3, 0) || extensions.textureHalfFloatOES) &&
            extensions.colorBufferHalfFloatEXT);
}

static bool SizedHalfFloatRGBATextureAttachmentSupport(const Version &clientVersion,
                                                       const Extensions &extensions)
{
    // HALF_FLOAT
    if (clientVersion >= Version(3, 0))
    {
        // WebGL 2 supports EXT_color_buffer_half_float.
        return extensions.colorBufferFloatEXT ||
               (extensions.webglCompatibilityANGLE && extensions.colorBufferHalfFloatEXT);
    }
    // HALF_FLOAT_OES
    else
    {
        return SizedHalfFloatOESTextureAttachmentSupport(clientVersion, extensions);
    }
}

static bool SizedHalfFloatRGBARenderbufferSupport(const Version &clientVersion,
                                                  const Extensions &extensions)
{
    return (clientVersion >= Version(3, 0) || extensions.textureHalfFloatOES) &&
           (extensions.colorBufferFloatEXT || extensions.colorBufferHalfFloatEXT);
}

// R32F, RG32F
static bool SizedFloatRGSupport(const Version &clientVersion, const Extensions &extensions)
{
    return clientVersion >= Version(3, 0) ||
           (extensions.textureStorageEXT && extensions.textureFloatOES && extensions.textureRgEXT);
}

// RGB32F
static bool SizedFloatRGBSupport(const Version &clientVersion, const Extensions &extensions)
{
    return clientVersion >= Version(3, 0) ||
           (extensions.textureStorageEXT && extensions.textureFloatOES) ||
           extensions.colorBufferFloatRgbCHROMIUM;
}

// RGBA32F
static bool SizedFloatRGBASupport(const Version &clientVersion, const Extensions &extensions)
{
    return clientVersion >= Version(3, 0) ||
           (extensions.textureStorageEXT && extensions.textureFloatOES) ||
           extensions.colorBufferFloatRgbaCHROMIUM;
}

static bool SizedFloatRGBARenderableSupport(const Version &clientVersion,
                                            const Extensions &extensions)
{
    // This logic is the same for both Renderbuffers and TextureAttachment.
    return extensions.colorBufferFloatRgbaCHROMIUM ||  // ES2
           extensions.colorBufferFloatEXT;             // ES3
}

static bool Float32BlendableSupport(const Version &clientVersion, const Extensions &extensions)
{
    // EXT_float_blend may be exposed on ES2 client contexts. Ensure that RGBA32F is renderable.
    return (extensions.colorBufferFloatRgbaCHROMIUM || extensions.colorBufferFloatEXT) &&
           extensions.floatBlendEXT;
}

template <ExtensionBool bool1>
static bool ETC2EACSupport(const Version &clientVersion, const Extensions &extensions)
{
    if (extensions.compressedTextureEtcANGLE)
    {
        return true;
    }

    // ETC2/EAC formats are always available in ES 3.0+ but require an extension (checked above)
    // in WebGL. If that extension is not available, hide these formats from WebGL contexts.
    return !extensions.webglCompatibilityANGLE &&
           (clientVersion >= Version(3, 0) || extensions.*bool1);
}

InternalFormat::InternalFormat()
    : internalFormat(GL_NONE),
      sized(false),
      sizedInternalFormat(GL_NONE),
      redBits(0),
      greenBits(0),
      blueBits(0),
      luminanceBits(0),
      alphaBits(0),
      sharedBits(0),
      depthBits(0),
      stencilBits(0),
      pixelBytes(0),
      componentCount(0),
      compressed(false),
      compressedBlockWidth(0),
      compressedBlockHeight(0),
      compressedBlockDepth(0),
      paletted(false),
      paletteBits(0),
      format(GL_NONE),
      type(GL_NONE),
      componentType(GL_NONE),
      colorEncoding(GL_NONE),
      textureSupport(NeverSupported),
      filterSupport(NeverSupported),
      textureAttachmentSupport(NeverSupported),
      renderbufferSupport(NeverSupported),
      blendSupport(NeverSupported)
{}

InternalFormat::InternalFormat(const InternalFormat &other) = default;

InternalFormat &InternalFormat::operator=(const InternalFormat &other) = default;

bool InternalFormat::isLUMA() const
{
    return ((redBits + greenBits + blueBits + depthBits + stencilBits) == 0 &&
            (luminanceBits + alphaBits) > 0);
}

GLenum InternalFormat::getReadPixelsFormat(const Extensions &extensions) const
{
    switch (format)
    {
        case GL_BGRA_EXT:
            // BGRA textures may be enabled but calling glReadPixels with BGRA is disallowed without
            // GL_EXT_texture_format_BGRA8888.  Read as RGBA instead.
            if (!extensions.readFormatBgraEXT)
            {
                return GL_RGBA;
            }
            return GL_BGRA_EXT;

        default:
            return format;
    }
}

GLenum InternalFormat::getReadPixelsType(const Version &version) const
{
    switch (type)
    {
        case GL_HALF_FLOAT:
        case GL_HALF_FLOAT_OES:
            if (version < Version(3, 0))
            {
                // The internal format may have a type of GL_HALF_FLOAT but when exposing this type
                // as the IMPLEMENTATION_READ_TYPE, only HALF_FLOAT_OES is allowed by
                // OES_texture_half_float.  HALF_FLOAT becomes core in ES3 and is acceptable to use
                // as an IMPLEMENTATION_READ_TYPE.
                return GL_HALF_FLOAT_OES;
            }
            return GL_HALF_FLOAT;

        default:
            return type;
    }
}

bool InternalFormat::supportSubImage() const
{
    return !CompressedFormatRequiresWholeImage(internalFormat);
}

bool InternalFormat::isRequiredRenderbufferFormat(const Version &version) const
{
    // GLES 3.0.5 section 4.4.2.2:
    // "Implementations are required to support the same internal formats for renderbuffers as the
    // required formats for textures enumerated in section 3.8.3.1, with the exception of the color
    // formats labelled "texture-only"."
    if (!sized || compressed)
    {
        return false;
    }

    // Luma formats.
    if (isLUMA())
    {
        return false;
    }

    // Depth/stencil formats.
    if (depthBits > 0 || stencilBits > 0)
    {
        // GLES 2.0.25 table 4.5.
        // GLES 3.0.5 section 3.8.3.1.
        // GLES 3.1 table 8.14.

        // Required formats in all versions.
        switch (internalFormat)
        {
            case GL_DEPTH_COMPONENT16:
            case GL_STENCIL_INDEX8:
                // Note that STENCIL_INDEX8 is not mentioned in GLES 3.0.5 section 3.8.3.1, but it
                // is in section 4.4.2.2.
                return true;
            default:
                break;
        }
        if (version.major < 3)
        {
            return false;
        }
        // Required formats in GLES 3.0 and up.
        switch (internalFormat)
        {
            case GL_DEPTH_COMPONENT32F:
            case GL_DEPTH_COMPONENT24:
            case GL_DEPTH32F_STENCIL8:
            case GL_DEPTH24_STENCIL8:
                return true;
            default:
                return false;
        }
    }

    // RGBA formats.
    // GLES 2.0.25 table 4.5.
    // GLES 3.0.5 section 3.8.3.1.
    // GLES 3.1 table 8.13.

    // Required formats in all versions.
    switch (internalFormat)
    {
        case GL_RGBA4:
        case GL_RGB5_A1:
        case GL_RGB565:
            return true;
        default:
            break;
    }
    if (version.major < 3)
    {
        return false;
    }

    if (format == GL_BGRA_EXT)
    {
        return false;
    }

    switch (componentType)
    {
        case GL_SIGNED_NORMALIZED:
        case GL_FLOAT:
            return false;
        case GL_UNSIGNED_INT:
        case GL_INT:
            // Integer RGB formats are not required renderbuffer formats.
            if (alphaBits == 0 && blueBits != 0)
            {
                return false;
            }
            // All integer R and RG formats are required.
            // Integer RGBA formats including RGB10_A2_UI are required.
            return true;
        case GL_UNSIGNED_NORMALIZED:
            if (internalFormat == GL_SRGB8)
            {
                return false;
            }
            return true;
        default:
            UNREACHABLE();
            return false;
    }
}

bool InternalFormat::isInt() const
{
    return componentType == GL_INT || componentType == GL_UNSIGNED_INT;
}

bool InternalFormat::isDepthOrStencil() const
{
    return depthBits != 0 || stencilBits != 0;
}

GLuint InternalFormat::getEGLConfigBufferSize() const
{
    // EGL config's EGL_BUFFER_SIZE is measured in bits and is the sum of all the color channels for
    // color formats or the luma channels for luma formats. It ignores unused bits so compute the
    // bit count by summing instead of using pixelBytes.
    if (isLUMA())
    {
        return luminanceBits + alphaBits;
    }
    else
    {
        return redBits + greenBits + blueBits + alphaBits;
    }
}

Format::Format(GLenum internalFormat) : Format(GetSizedInternalFormatInfo(internalFormat)) {}

Format::Format(GLenum internalFormat, GLenum type)
    : info(&GetInternalFormatInfo(internalFormat, type))
{}

bool Format::valid() const
{
    return info->internalFormat != GL_NONE;
}

// static
bool Format::SameSized(const Format &a, const Format &b)
{
    return a.info->sizedInternalFormat == b.info->sizedInternalFormat;
}

static GLenum EquivalentBlitInternalFormat(GLenum internalformat)
{
    // BlitFramebuffer works if the color channels are identically
    // sized, even if there is a swizzle (for example, blitting from a
    // multisampled RGBA8 renderbuffer to a BGRA8 texture). This could
    // be expanded and/or autogenerated if that is found necessary.
    if (internalformat == GL_BGRA8_EXT || internalformat == GL_BGRA8_SRGB_ANGLEX)
    {
        return GL_RGBA8;
    }

    // GL_ANGLE_rgbx_internal_format: Treat RGBX8 as RGB8, since the X channel is ignored.
    if (internalformat == GL_RGBX8_ANGLE || internalformat == GL_RGBX8_SRGB_ANGLEX)
    {
        return GL_RGB8;
    }

    // Treat ANGLE's BGRX8 as RGB8 since it's swizzled and the X channel is ignored.
    if (internalformat == GL_BGRX8_ANGLEX || internalformat == GL_BGRX8_SRGB_ANGLEX)
    {
        return GL_RGB8;
    }

    return internalformat;
}

// static
bool Format::EquivalentForBlit(const Format &a, const Format &b)
{
    return (EquivalentBlitInternalFormat(a.info->sizedInternalFormat) ==
            EquivalentBlitInternalFormat(b.info->sizedInternalFormat));
}

// static
Format Format::Invalid()
{
    static Format invalid(GL_NONE, GL_NONE);
    return invalid;
}

std::ostream &operator<<(std::ostream &os, const Format &fmt)
{
    // TODO(ynovikov): return string representation when available
    return FmtHex(os, fmt.info->sizedInternalFormat);
}

bool InternalFormat::operator==(const InternalFormat &other) const
{
    // We assume all internal formats are unique if they have the same internal format and type
    return internalFormat == other.internalFormat && type == other.type;
}

bool InternalFormat::operator!=(const InternalFormat &other) const
{
    return !(*this == other);
}

void InsertFormatInfo(InternalFormatInfoMap *map, const InternalFormat &formatInfo)
{
    ASSERT(!formatInfo.sized || (*map).count(formatInfo.internalFormat) == 0);
    ASSERT((*map)[formatInfo.internalFormat].count(formatInfo.type) == 0);
    (*map)[formatInfo.internalFormat][formatInfo.type] = formatInfo;
}

// YuvFormatInfo implementation
YuvFormatInfo::YuvFormatInfo(GLenum internalFormat, const Extents &yPlaneExtent)
{
    ASSERT(gl::IsYuvFormat(internalFormat));
    ASSERT((gl::GetPlaneCount(internalFormat) > 0) && (gl::GetPlaneCount(internalFormat) <= 3));

    glInternalFormat = internalFormat;
    planeCount       = gl::GetPlaneCount(internalFormat);

    // Chroma planes of a YUV format can be subsampled
    int horizontalSubsampleFactor = 0;
    int verticalSubsampleFactor   = 0;
    gl::GetSubSampleFactor(internalFormat, &horizontalSubsampleFactor, &verticalSubsampleFactor);

    // Compute plane Bpp
    planeBpp[0] = gl::GetYPlaneBpp(internalFormat);
    planeBpp[1] = gl::GetChromaPlaneBpp(internalFormat);
    planeBpp[2] = (planeCount > 2) ? planeBpp[1] : 0;

    // Compute plane extent
    planeExtent[0] = yPlaneExtent;
    planeExtent[1] = {(yPlaneExtent.width / horizontalSubsampleFactor),
                      (yPlaneExtent.height / verticalSubsampleFactor), yPlaneExtent.depth};
    planeExtent[2] = (planeCount > 2) ? planeExtent[1] : Extents();

    // Compute plane pitch
    planePitch[0] = planeExtent[0].width * planeBpp[0];
    planePitch[1] = planeExtent[1].width * planeBpp[1];
    planePitch[2] = planeExtent[2].width * planeBpp[2];

    // Compute plane size
    planeSize[0] = planePitch[0] * planeExtent[0].height;
    planeSize[1] = planePitch[1] * planeExtent[1].height;
    planeSize[2] = planePitch[2] * planeExtent[2].height;

    // Compute plane offset
    planeOffset[0] = 0;
    planeOffset[1] = planeSize[0];
    planeOffset[2] = planeSize[0] + planeSize[1];
}

// YUV format related helpers
bool IsYuvFormat(GLenum format)
{
    switch (format)
    {
        case GL_G8_B8R8_2PLANE_420_UNORM_ANGLE:
        case GL_G8_B8_R8_3PLANE_420_UNORM_ANGLE:
        case GL_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_ANGLE:
        case GL_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_ANGLE:
        case GL_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_ANGLE:
        case GL_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_ANGLE:
        case GL_G16_B16R16_2PLANE_420_UNORM_ANGLE:
        case GL_G16_B16_R16_3PLANE_420_UNORM_ANGLE:
            return true;
        default:
            return false;
    }
}

uint32_t GetPlaneCount(GLenum format)
{
    switch (format)
    {
        case GL_G8_B8R8_2PLANE_420_UNORM_ANGLE:
        case GL_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_ANGLE:
        case GL_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_ANGLE:
        case GL_G16_B16R16_2PLANE_420_UNORM_ANGLE:
            return 2;
        case GL_G8_B8_R8_3PLANE_420_UNORM_ANGLE:
        case GL_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_ANGLE:
        case GL_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_ANGLE:
        case GL_G16_B16_R16_3PLANE_420_UNORM_ANGLE:
            return 3;
        default:
            UNREACHABLE();
            return 0;
    }
}

uint32_t GetYPlaneBpp(GLenum format)
{
    switch (format)
    {
        case GL_G8_B8R8_2PLANE_420_UNORM_ANGLE:
        case GL_G8_B8_R8_3PLANE_420_UNORM_ANGLE:
            return 1;
        case GL_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_ANGLE:
        case GL_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_ANGLE:
        case GL_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_ANGLE:
        case GL_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_ANGLE:
        case GL_G16_B16R16_2PLANE_420_UNORM_ANGLE:
        case GL_G16_B16_R16_3PLANE_420_UNORM_ANGLE:
            return 2;
        default:
            UNREACHABLE();
            return 0;
    }
}

uint32_t GetChromaPlaneBpp(GLenum format)
{
    // 2 plane 420 YUV formats have CbCr channels interleaved.
    // 3 plane 420 YUV formats have separate Cb and Cr planes.
    switch (format)
    {
        case GL_G8_B8_R8_3PLANE_420_UNORM_ANGLE:
            return 1;
        case GL_G8_B8R8_2PLANE_420_UNORM_ANGLE:
        case GL_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_ANGLE:
        case GL_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_ANGLE:
        case GL_G16_B16_R16_3PLANE_420_UNORM_ANGLE:
            return 2;
        case GL_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_ANGLE:
        case GL_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_ANGLE:
        case GL_G16_B16R16_2PLANE_420_UNORM_ANGLE:
            return 4;
        default:
            UNREACHABLE();
            return 0;
    }
}

void GetSubSampleFactor(GLenum format, int *horizontalSubsampleFactor, int *verticalSubsampleFactor)
{
    ASSERT(horizontalSubsampleFactor && verticalSubsampleFactor);

    switch (format)
    {
        case GL_G8_B8R8_2PLANE_420_UNORM_ANGLE:
        case GL_G8_B8_R8_3PLANE_420_UNORM_ANGLE:
        case GL_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_ANGLE:
        case GL_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_ANGLE:
        case GL_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_ANGLE:
        case GL_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_ANGLE:
        case GL_G16_B16R16_2PLANE_420_UNORM_ANGLE:
        case GL_G16_B16_R16_3PLANE_420_UNORM_ANGLE:
            *horizontalSubsampleFactor = 2;
            *verticalSubsampleFactor   = 2;
            break;
        default:
            UNREACHABLE();
            break;
    }
}

struct FormatBits
{
    constexpr GLuint pixelBytes() const
    {
        return (red + green + blue + alpha + shared + unused) / 8;
    }
    constexpr GLuint componentCount() const
    {
        return ((red > 0) ? 1 : 0) + ((green > 0) ? 1 : 0) + ((blue > 0) ? 1 : 0) +
               ((alpha > 0) ? 1 : 0);
    }
    constexpr bool valid() const
    {
        return ((red + green + blue + alpha + shared + unused) % 8) == 0;
    }

    GLuint red;
    GLuint green;
    GLuint blue;
    GLuint alpha;
    GLuint unused;
    GLuint shared;
};

template <GLuint red, GLuint green, GLuint blue, GLuint alpha, GLuint unused, GLuint shared>
constexpr FormatBits FB()
{
    constexpr FormatBits formatBits = {red, green, blue, alpha, unused, shared};
    static_assert(formatBits.valid(), "Invalid FormatBits");
    return formatBits;
}

void AddRGBAXFormat(InternalFormatInfoMap *map,
                    GLenum internalFormat,
                    bool sized,
                    const FormatBits &formatBits,
                    GLenum format,
                    GLenum type,
                    GLenum componentType,
                    bool srgb,
                    InternalFormat::SupportCheckFunction textureSupport,
                    InternalFormat::SupportCheckFunction filterSupport,
                    InternalFormat::SupportCheckFunction textureAttachmentSupport,
                    InternalFormat::SupportCheckFunction renderbufferSupport,
                    InternalFormat::SupportCheckFunction blendSupport)
{
    ASSERT(formatBits.valid());

    InternalFormat formatInfo;
    formatInfo.internalFormat = internalFormat;
    formatInfo.sized          = sized;
    formatInfo.sizedInternalFormat =
        sized ? internalFormat : GetSizedFormatInternal(internalFormat, type);
    formatInfo.redBits                  = formatBits.red;
    formatInfo.greenBits                = formatBits.green;
    formatInfo.blueBits                 = formatBits.blue;
    formatInfo.alphaBits                = formatBits.alpha;
    formatInfo.sharedBits               = formatBits.shared;
    formatInfo.pixelBytes               = formatBits.pixelBytes();
    formatInfo.componentCount           = formatBits.componentCount();
    formatInfo.format                   = format;
    formatInfo.type                     = type;
    formatInfo.componentType            = componentType;
    formatInfo.colorEncoding            = (srgb ? GL_SRGB : GL_LINEAR);
    formatInfo.textureSupport           = textureSupport;
    formatInfo.filterSupport            = filterSupport;
    formatInfo.textureAttachmentSupport = textureAttachmentSupport;
    formatInfo.renderbufferSupport      = renderbufferSupport;
    formatInfo.blendSupport             = blendSupport;

    InsertFormatInfo(map, formatInfo);
}

void AddRGBAFormat(InternalFormatInfoMap *map,
                   GLenum internalFormat,
                   bool sized,
                   GLuint red,
                   GLuint green,
                   GLuint blue,
                   GLuint alpha,
                   GLuint shared,
                   GLenum format,
                   GLenum type,
                   GLenum componentType,
                   bool srgb,
                   InternalFormat::SupportCheckFunction textureSupport,
                   InternalFormat::SupportCheckFunction filterSupport,
                   InternalFormat::SupportCheckFunction textureAttachmentSupport,
                   InternalFormat::SupportCheckFunction renderbufferSupport,
                   InternalFormat::SupportCheckFunction blendSupport)
{
    return AddRGBAXFormat(map, internalFormat, sized, {red, green, blue, alpha, 0, shared}, format,
                          type, componentType, srgb, textureSupport, filterSupport,
                          textureAttachmentSupport, renderbufferSupport, blendSupport);
}

static void AddLUMAFormat(InternalFormatInfoMap *map,
                          GLenum internalFormat,
                          bool sized,
                          GLuint luminance,
                          GLuint alpha,
                          GLenum format,
                          GLenum type,
                          GLenum componentType,
                          InternalFormat::SupportCheckFunction textureSupport,
                          InternalFormat::SupportCheckFunction filterSupport,
                          InternalFormat::SupportCheckFunction textureAttachmentSupport,
                          InternalFormat::SupportCheckFunction renderbufferSupport,
                          InternalFormat::SupportCheckFunction blendSupport)
{
    InternalFormat formatInfo;
    formatInfo.internalFormat = internalFormat;
    formatInfo.sized          = sized;
    formatInfo.sizedInternalFormat =
        sized ? internalFormat : GetSizedFormatInternal(internalFormat, type);
    formatInfo.luminanceBits            = luminance;
    formatInfo.alphaBits                = alpha;
    formatInfo.pixelBytes               = (luminance + alpha) / 8;
    formatInfo.componentCount           = ((luminance > 0) ? 1 : 0) + ((alpha > 0) ? 1 : 0);
    formatInfo.format                   = format;
    formatInfo.type                     = type;
    formatInfo.componentType            = componentType;
    formatInfo.colorEncoding            = GL_LINEAR;
    formatInfo.textureSupport           = textureSupport;
    formatInfo.filterSupport            = filterSupport;
    formatInfo.textureAttachmentSupport = textureAttachmentSupport;
    formatInfo.renderbufferSupport      = renderbufferSupport;
    formatInfo.blendSupport             = blendSupport;

    InsertFormatInfo(map, formatInfo);
}

void AddDepthStencilFormat(InternalFormatInfoMap *map,
                           GLenum internalFormat,
                           bool sized,
                           GLuint depthBits,
                           GLuint stencilBits,
                           GLuint unusedBits,
                           GLenum format,
                           GLenum type,
                           GLenum componentType,
                           InternalFormat::SupportCheckFunction textureSupport,
                           InternalFormat::SupportCheckFunction filterSupport,
                           InternalFormat::SupportCheckFunction textureAttachmentSupport,
                           InternalFormat::SupportCheckFunction renderbufferSupport,
                           InternalFormat::SupportCheckFunction blendSupport)
{
    InternalFormat formatInfo;
    formatInfo.internalFormat = internalFormat;
    formatInfo.sized          = sized;
    formatInfo.sizedInternalFormat =
        sized ? internalFormat : GetSizedFormatInternal(internalFormat, type);
    formatInfo.depthBits                = depthBits;
    formatInfo.stencilBits              = stencilBits;
    formatInfo.pixelBytes               = (depthBits + stencilBits + unusedBits) / 8;
    formatInfo.componentCount           = ((depthBits > 0) ? 1 : 0) + ((stencilBits > 0) ? 1 : 0);
    formatInfo.format                   = format;
    formatInfo.type                     = type;
    formatInfo.componentType            = componentType;
    formatInfo.colorEncoding            = GL_LINEAR;
    formatInfo.textureSupport           = textureSupport;
    formatInfo.filterSupport            = filterSupport;
    formatInfo.textureAttachmentSupport = textureAttachmentSupport;
    formatInfo.renderbufferSupport      = renderbufferSupport;
    formatInfo.blendSupport             = blendSupport;

    InsertFormatInfo(map, formatInfo);
}

void AddCompressedFormat(InternalFormatInfoMap *map,
                         GLenum internalFormat,
                         GLuint compressedBlockWidth,
                         GLuint compressedBlockHeight,
                         GLuint compressedBlockDepth,
                         GLuint compressedBlockSize,
                         GLuint componentCount,
                         bool srgb,
                         InternalFormat::SupportCheckFunction textureSupport,
                         InternalFormat::SupportCheckFunction filterSupport,
                         InternalFormat::SupportCheckFunction textureAttachmentSupport,
                         InternalFormat::SupportCheckFunction renderbufferSupport,
                         InternalFormat::SupportCheckFunction blendSupport)
{
    InternalFormat formatInfo;
    formatInfo.internalFormat           = internalFormat;
    formatInfo.sized                    = true;
    formatInfo.sizedInternalFormat      = internalFormat;
    formatInfo.compressedBlockWidth     = compressedBlockWidth;
    formatInfo.compressedBlockHeight    = compressedBlockHeight;
    formatInfo.compressedBlockDepth     = compressedBlockDepth;
    formatInfo.pixelBytes               = compressedBlockSize / 8;
    formatInfo.componentCount           = componentCount;
    formatInfo.format                   = internalFormat;
    formatInfo.type                     = GL_UNSIGNED_BYTE;
    formatInfo.componentType            = GL_UNSIGNED_NORMALIZED;
    formatInfo.colorEncoding            = (srgb ? GL_SRGB : GL_LINEAR);
    formatInfo.compressed               = true;
    formatInfo.textureSupport           = textureSupport;
    formatInfo.filterSupport            = filterSupport;
    formatInfo.textureAttachmentSupport = textureAttachmentSupport;
    formatInfo.renderbufferSupport      = renderbufferSupport;
    formatInfo.blendSupport             = blendSupport;

    InsertFormatInfo(map, formatInfo);
}

void AddPalettedFormat(InternalFormatInfoMap *map,
                       GLenum internalFormat,
                       GLuint paletteBits,
                       GLuint pixelBytes,
                       GLenum format,
                       GLuint componentCount,
                       InternalFormat::SupportCheckFunction textureSupport,
                       InternalFormat::SupportCheckFunction filterSupport,
                       InternalFormat::SupportCheckFunction textureAttachmentSupport,
                       InternalFormat::SupportCheckFunction renderbufferSupport,
                       InternalFormat::SupportCheckFunction blendSupport)
{
    InternalFormat formatInfo;
    formatInfo.internalFormat           = internalFormat;
    formatInfo.sized                    = true;
    formatInfo.sizedInternalFormat      = internalFormat;
    formatInfo.paletteBits              = paletteBits;
    formatInfo.pixelBytes               = pixelBytes;
    formatInfo.componentCount           = componentCount;
    formatInfo.format                   = format;
    formatInfo.type                     = GL_UNSIGNED_BYTE;
    formatInfo.componentType            = GL_UNSIGNED_NORMALIZED;
    formatInfo.colorEncoding            = GL_LINEAR;
    formatInfo.paletted                 = true;
    formatInfo.textureSupport           = textureSupport;
    formatInfo.filterSupport            = filterSupport;
    formatInfo.textureAttachmentSupport = textureAttachmentSupport;
    formatInfo.renderbufferSupport      = renderbufferSupport;
    formatInfo.blendSupport             = blendSupport;

    InsertFormatInfo(map, formatInfo);
}

void AddYUVFormat(InternalFormatInfoMap *map,
                  GLenum internalFormat,
                  bool sized,
                  GLuint cr,
                  GLuint y,
                  GLuint cb,
                  GLuint alpha,
                  GLuint shared,
                  GLenum format,
                  GLenum type,
                  GLenum componentType,
                  bool srgb,
                  InternalFormat::SupportCheckFunction textureSupport,
                  InternalFormat::SupportCheckFunction filterSupport,
                  InternalFormat::SupportCheckFunction textureAttachmentSupport,
                  InternalFormat::SupportCheckFunction renderbufferSupport,
                  InternalFormat::SupportCheckFunction blendSupport)
{
    ASSERT(sized);

    InternalFormat formatInfo;
    formatInfo.internalFormat      = internalFormat;
    formatInfo.sized               = sized;
    formatInfo.sizedInternalFormat = internalFormat;
    formatInfo.redBits             = cr;
    formatInfo.greenBits           = y;
    formatInfo.blueBits            = cb;
    formatInfo.alphaBits           = alpha;
    formatInfo.sharedBits          = shared;
    formatInfo.pixelBytes          = (cr + y + cb + alpha + shared) / 8;
    formatInfo.componentCount =
        ((cr > 0) ? 1 : 0) + ((y > 0) ? 1 : 0) + ((cb > 0) ? 1 : 0) + ((alpha > 0) ? 1 : 0);
    formatInfo.format                   = format;
    formatInfo.type                     = type;
    formatInfo.componentType            = componentType;
    formatInfo.colorEncoding            = (srgb ? GL_SRGB : GL_LINEAR);
    formatInfo.textureSupport           = textureSupport;
    formatInfo.filterSupport            = filterSupport;
    formatInfo.textureAttachmentSupport = textureAttachmentSupport;
    formatInfo.renderbufferSupport      = renderbufferSupport;
    formatInfo.blendSupport             = blendSupport;

    InsertFormatInfo(map, formatInfo);
}

// Notes:
// 1. "Texture supported" includes all the means by which texture can be created, however,
//    GL_EXT_texture_storage in ES2 is a special case, when only glTexStorage* is allowed.
//    The assumption is that ES2 validation will not check textureSupport for sized formats.
//
// 2. Sized half float types are a combination of GL_HALF_FLOAT and GL_HALF_FLOAT_OES support,
//    due to a limitation that only one type for sized formats is allowed.
//
// TODO(ynovikov): http://anglebug.com/42261549 Verify support fields of BGRA, depth, stencil
// and compressed formats. Perform texturable check as part of filterable and attachment checks.
static InternalFormatInfoMap BuildInternalFormatInfoMap()
{
    InternalFormatInfoMap map;

    // From ES 3.0.1 spec, table 3.12
    map[GL_NONE][GL_NONE] = InternalFormat();

    // clang-format off

    //                 | Internal format     |sized| R | G | B | A |S | Format         | Type                             | Component type        | SRGB | Texture supported                                | Filterable     | Texture attachment                               | Renderbuffer                                   | Blend
    AddRGBAFormat(&map, GL_R8,                true,  8,  0,  0,  0, 0, GL_RED,          GL_UNSIGNED_BYTE,                  GL_UNSIGNED_NORMALIZED, false, SizedRGSupport,                                       AlwaysSupported, SizedRGSupport,                                          RequireESOrExt<3, 0, &Extensions::textureRgEXT>, RequireESOrExt<3, 0, &Extensions::textureRgEXT>);
    AddRGBAFormat(&map, GL_R8_SNORM,          true,  8,  0,  0,  0, 0, GL_RED,          GL_BYTE,                           GL_SIGNED_NORMALIZED,   false, RequireES<3, 0>,                                      AlwaysSupported, RequireExt<&Extensions::renderSnormEXT>,                 RequireExt<&Extensions::renderSnormEXT>,         RequireExt<&Extensions::renderSnormEXT>);
    AddRGBAFormat(&map, GL_RG8,               true,  8,  8,  0,  0, 0, GL_RG,           GL_UNSIGNED_BYTE,                  GL_UNSIGNED_NORMALIZED, false, SizedRGSupport,                                       AlwaysSupported, SizedRGSupport,                                          RequireESOrExt<3, 0, &Extensions::textureRgEXT>, RequireESOrExt<3, 0, &Extensions::textureRgEXT>);
    AddRGBAFormat(&map, GL_RG8_SNORM,         true,  8,  8,  0,  0, 0, GL_RG,           GL_BYTE,                           GL_SIGNED_NORMALIZED,   false, RequireES<3, 0>,                                      AlwaysSupported, RequireExt<&Extensions::renderSnormEXT>,                 RequireExt<&Extensions::renderSnormEXT>,         RequireExt<&Extensions::renderSnormEXT>);
    AddRGBAFormat(&map, GL_RGB8,              true,  8,  8,  8,  0, 0, GL_RGB,          GL_UNSIGNED_BYTE,                  GL_UNSIGNED_NORMALIZED, false, RequireESOrExtOrExt<3, 0, &Extensions::textureStorageEXT, &Extensions::requiredInternalformatOES>, AlwaysSupported, RequireESOrExtOrExt<3, 0, &Extensions::textureStorageEXT, &Extensions::requiredInternalformatOES>, RequireESOrExt<3, 0, &Extensions::rgb8Rgba8OES>,    RequireESOrExt<3, 0, &Extensions::rgb8Rgba8OES>);
    AddRGBAFormat(&map, GL_RGB8_SNORM,        true,  8,  8,  8,  0, 0, GL_RGB,          GL_BYTE,                           GL_SIGNED_NORMALIZED,   false, RequireES<3, 0>,                                      AlwaysSupported, NeverSupported,                                          NeverSupported,                                  NeverSupported);
    AddRGBAFormat(&map, GL_RGB565,            true,  5,  6,  5,  0, 0, GL_RGB,          GL_UNSIGNED_SHORT_5_6_5,           GL_UNSIGNED_NORMALIZED, false, RequireESOrExtOrExt<3, 0, &Extensions::textureStorageEXT, &Extensions::requiredInternalformatOES>, AlwaysSupported, RequireESOrExtOrExt<3, 0, &Extensions::textureStorageEXT, &Extensions::requiredInternalformatOES>, RequireESOrExt<2, 0, &Extensions::framebufferObjectOES>, RequireES<2, 0>);
    AddRGBAFormat(&map, GL_RGBA4,             true,  4,  4,  4,  4, 0, GL_RGBA,         GL_UNSIGNED_SHORT_4_4_4_4,         GL_UNSIGNED_NORMALIZED, false, RequireESOrExtOrExt<3, 0, &Extensions::textureStorageEXT, &Extensions::requiredInternalformatOES>, AlwaysSupported, RequireESOrExtOrExt<3, 0, &Extensions::textureStorageEXT, &Extensions::requiredInternalformatOES>, RequireESOrExt<2, 0, &Extensions::framebufferObjectOES>, RequireES<2, 0>);
    AddRGBAFormat(&map, GL_RGB5_A1,           true,  5,  5,  5,  1, 0, GL_RGBA,         GL_UNSIGNED_SHORT_5_5_5_1,         GL_UNSIGNED_NORMALIZED, false, RequireESOrExtOrExt<3, 0, &Extensions::textureStorageEXT, &Extensions::requiredInternalformatOES>, AlwaysSupported, RequireESOrExtOrExt<3, 0, &Extensions::textureStorageEXT, &Extensions::requiredInternalformatOES>, RequireESOrExt<2, 0, &Extensions::framebufferObjectOES>, RequireES<2, 0>);
    AddRGBAFormat(&map, GL_RGBA8,             true,  8,  8,  8,  8, 0, GL_RGBA,         GL_UNSIGNED_BYTE,                  GL_UNSIGNED_NORMALIZED, false, RequireESOrExtOrExt<3, 0, &Extensions::textureStorageEXT, &Extensions::requiredInternalformatOES>, AlwaysSupported, RequireESOrExtOrExt<3, 0, &Extensions::textureStorageEXT, &Extensions::requiredInternalformatOES>, RequireESOrExt<3, 0, &Extensions::rgb8Rgba8OES>,    RequireESOrExt<3, 0, &Extensions::rgb8Rgba8OES>);
    AddRGBAFormat(&map, GL_RGBA8_SNORM,       true,  8,  8,  8,  8, 0, GL_RGBA,         GL_BYTE,                           GL_SIGNED_NORMALIZED,   false, RequireES<3, 0>,                                      AlwaysSupported, RequireExt<&Extensions::renderSnormEXT>,                 RequireExt<&Extensions::renderSnormEXT>,         RequireExt<&Extensions::renderSnormEXT>);
    AddRGBAFormat(&map, GL_RGB10_A2UI,        true, 10, 10, 10,  2, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV,    GL_UNSIGNED_INT,        false, RequireES<3, 0>,                                      NeverSupported,  RequireES<3, 0>,                                         RequireES<3, 0>,                                 NeverSupported);
    AddRGBAFormat(&map, GL_SRGB8,             true,  8,  8,  8,  0, 0, GL_RGB,          GL_UNSIGNED_BYTE,                  GL_UNSIGNED_NORMALIZED, true,  RequireES<3, 0>,                                      AlwaysSupported, NeverSupported,                                          NeverSupported,                                  NeverSupported);
    AddRGBAFormat(&map, GL_SRGB8_ALPHA8,      true,  8,  8,  8,  8, 0, GL_RGBA,         GL_UNSIGNED_BYTE,                  GL_UNSIGNED_NORMALIZED, true,  RequireESOrExt<3, 0, &Extensions::sRGBEXT>,           AlwaysSupported, RequireES<3, 0>,                                         RequireESOrExt<3, 0, &Extensions::sRGBEXT>,      RequireESOrExt<3, 0, &Extensions::sRGBEXT>);
    AddRGBAFormat(&map, GL_R11F_G11F_B10F,    true, 11, 11, 10,  0, 0, GL_RGB,          GL_UNSIGNED_INT_10F_11F_11F_REV,   GL_FLOAT,               false, RequireES<3, 0>,                                      AlwaysSupported, RequireExt<&Extensions::colorBufferFloatEXT>,            RequireExt<&Extensions::colorBufferFloatEXT>,    RequireExt<&Extensions::colorBufferFloatEXT>);
    AddRGBAFormat(&map, GL_RGB9_E5,           true,  9,  9,  9,  0, 5, GL_RGB,          GL_UNSIGNED_INT_5_9_9_9_REV,       GL_FLOAT,               false, RequireES<3, 0>,                                      AlwaysSupported, RequireExt<&Extensions::renderSharedExponentQCOM>,    RequireExt<&Extensions::renderSharedExponentQCOM>,  RequireExt<&Extensions::renderSharedExponentQCOM>);
    AddRGBAFormat(&map, GL_R8I,               true,  8,  0,  0,  0, 0, GL_RED_INTEGER,  GL_BYTE,                           GL_INT,                 false, RequireES<3, 0>,                                      NeverSupported,  RequireES<3, 0>,                                         RequireES<3, 0>,                                 NeverSupported);
    AddRGBAFormat(&map, GL_R8UI,              true,  8,  0,  0,  0, 0, GL_RED_INTEGER,  GL_UNSIGNED_BYTE,                  GL_UNSIGNED_INT,        false, RequireES<3, 0>,                                      NeverSupported,  RequireES<3, 0>,                                         RequireES<3, 0>,                                 NeverSupported);
    AddRGBAFormat(&map, GL_R16I,              true, 16,  0,  0,  0, 0, GL_RED_INTEGER,  GL_SHORT,                          GL_INT,                 false, RequireES<3, 0>,                                      NeverSupported,  RequireES<3, 0>,                                         RequireES<3, 0>,                                 NeverSupported);
    AddRGBAFormat(&map, GL_R16UI,             true, 16,  0,  0,  0, 0, GL_RED_INTEGER,  GL_UNSIGNED_SHORT,                 GL_UNSIGNED_INT,        false, RequireES<3, 0>,                                      NeverSupported,  RequireES<3, 0>,                                         RequireES<3, 0>,                                 NeverSupported);
    AddRGBAFormat(&map, GL_R32I,              true, 32,  0,  0,  0, 0, GL_RED_INTEGER,  GL_INT,                            GL_INT,                 false, RequireES<3, 0>,                                      NeverSupported,  RequireES<3, 0>,                                         RequireES<3, 0>,                                 NeverSupported);
    AddRGBAFormat(&map, GL_R32UI,             true, 32,  0,  0,  0, 0, GL_RED_INTEGER,  GL_UNSIGNED_INT,                   GL_UNSIGNED_INT,        false, RequireES<3, 0>,                                      NeverSupported,  RequireES<3, 0>,                                         RequireES<3, 0>,                                 NeverSupported);
    AddRGBAFormat(&map, GL_RG8I,              true,  8,  8,  0,  0, 0, GL_RG_INTEGER,   GL_BYTE,                           GL_INT,                 false, RequireES<3, 0>,                                      NeverSupported,  RequireES<3, 0>,                                         RequireES<3, 0>,                                 NeverSupported);
    AddRGBAFormat(&map, GL_RG8UI,             true,  8,  8,  0,  0, 0, GL_RG_INTEGER,   GL_UNSIGNED_BYTE,                  GL_UNSIGNED_INT,        false, RequireES<3, 0>,                                      NeverSupported,  RequireES<3, 0>,                                         RequireES<3, 0>,                                 NeverSupported);
    AddRGBAFormat(&map, GL_RG16I,             true, 16, 16,  0,  0, 0, GL_RG_INTEGER,   GL_SHORT,                          GL_INT,                 false, RequireES<3, 0>,                                      NeverSupported,  RequireES<3, 0>,                                         RequireES<3, 0>,                                 NeverSupported);
    AddRGBAFormat(&map, GL_RG16UI,            true, 16, 16,  0,  0, 0, GL_RG_INTEGER,   GL_UNSIGNED_SHORT,                 GL_UNSIGNED_INT,        false, RequireES<3, 0>,                                      NeverSupported,  RequireES<3, 0>,                                         RequireES<3, 0>,                                 NeverSupported);
    AddRGBAFormat(&map, GL_RG32I,             true, 32, 32,  0,  0, 0, GL_RG_INTEGER,   GL_INT,                            GL_INT,                 false, RequireES<3, 0>,                                      NeverSupported,  RequireES<3, 0>,                                         RequireES<3, 0>,                                 NeverSupported);
    AddRGBAFormat(&map, GL_RG32UI,            true, 32, 32,  0,  0, 0, GL_RG_INTEGER,   GL_UNSIGNED_INT,                   GL_UNSIGNED_INT,        false, RequireES<3, 0>,                                      NeverSupported,  RequireES<3, 0>,                                         RequireES<3, 0>,                                 NeverSupported);
    AddRGBAFormat(&map, GL_RGB8I,             true,  8,  8,  8,  0, 0, GL_RGB_INTEGER,  GL_BYTE,                           GL_INT,                 false, RequireES<3, 0>,                                      NeverSupported,  NeverSupported,                                          NeverSupported,                                  NeverSupported);
    AddRGBAFormat(&map, GL_RGB8UI,            true,  8,  8,  8,  0, 0, GL_RGB_INTEGER,  GL_UNSIGNED_BYTE,                  GL_UNSIGNED_INT,        false, RequireES<3, 0>,                                      NeverSupported,  NeverSupported,                                          NeverSupported,                                  NeverSupported);
    AddRGBAFormat(&map, GL_RGB16I,            true, 16, 16, 16,  0, 0, GL_RGB_INTEGER,  GL_SHORT,                          GL_INT,                 false, RequireES<3, 0>,                                      NeverSupported,  NeverSupported,                                          NeverSupported,                                  NeverSupported);
    AddRGBAFormat(&map, GL_RGB16UI,           true, 16, 16, 16,  0, 0, GL_RGB_INTEGER,  GL_UNSIGNED_SHORT,                 GL_UNSIGNED_INT,        false, RequireES<3, 0>,                                      NeverSupported,  NeverSupported,                                          NeverSupported,                                  NeverSupported);
    AddRGBAFormat(&map, GL_RGB32I,            true, 32, 32, 32,  0, 0, GL_RGB_INTEGER,  GL_INT,                            GL_INT,                 false, RequireES<3, 0>,                                      NeverSupported,  NeverSupported,                                          NeverSupported,                                  NeverSupported);
    AddRGBAFormat(&map, GL_RGB32UI,           true, 32, 32, 32,  0, 0, GL_RGB_INTEGER,  GL_UNSIGNED_INT,                   GL_UNSIGNED_INT,        false, RequireES<3, 0>,                                      NeverSupported,  NeverSupported,                                          NeverSupported,                                  NeverSupported);
    AddRGBAFormat(&map, GL_RGBA8I,            true,  8,  8,  8,  8, 0, GL_RGBA_INTEGER, GL_BYTE,                           GL_INT,                 false, RequireES<3, 0>,                                      NeverSupported,  RequireES<3, 0>,                                         RequireES<3, 0>,                                 NeverSupported);
    AddRGBAFormat(&map, GL_RGBA8UI,           true,  8,  8,  8,  8, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE,                  GL_UNSIGNED_INT,        false, RequireES<3, 0>,                                      NeverSupported,  RequireES<3, 0>,                                         RequireES<3, 0>,                                 NeverSupported);
    AddRGBAFormat(&map, GL_RGBA16I,           true, 16, 16, 16, 16, 0, GL_RGBA_INTEGER, GL_SHORT,                          GL_INT,                 false, RequireES<3, 0>,                                      NeverSupported,  RequireES<3, 0>,                                         RequireES<3, 0>,                                 NeverSupported);
    AddRGBAFormat(&map, GL_RGBA16UI,          true, 16, 16, 16, 16, 0, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT,                 GL_UNSIGNED_INT,        false, RequireES<3, 0>,                                      NeverSupported,  RequireES<3, 0>,                                         RequireES<3, 0>,                                 NeverSupported);
    AddRGBAFormat(&map, GL_RGBA32I,           true, 32, 32, 32, 32, 0, GL_RGBA_INTEGER, GL_INT,                            GL_INT,                 false, RequireES<3, 0>,                                      NeverSupported,  RequireES<3, 0>,                                         RequireES<3, 0>,                                 NeverSupported);
    AddRGBAFormat(&map, GL_RGBA32UI,          true, 32, 32, 32, 32, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT,                   GL_UNSIGNED_INT,        false, RequireES<3, 0>,                                      NeverSupported,  RequireES<3, 0>,                                         RequireES<3, 0>,                                 NeverSupported);

    AddRGBAFormat(&map, GL_BGRA8_EXT,         true,  8,  8,  8,  8, 0, GL_BGRA_EXT,     GL_UNSIGNED_BYTE,                  GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::textureFormatBGRA8888EXT>,    AlwaysSupported, RequireExt<&Extensions::textureFormatBGRA8888EXT>,    RequireExt<&Extensions::textureFormatBGRA8888EXT>, RequireExt<&Extensions::textureFormatBGRA8888EXT>);
    AddRGBAFormat(&map, GL_BGRA4_ANGLEX,      true,  4,  4,  4,  4, 0, GL_BGRA_EXT,     GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT, GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::textureFormatBGRA8888EXT>,    AlwaysSupported, RequireExt<&Extensions::textureFormatBGRA8888EXT>,    RequireExt<&Extensions::textureFormatBGRA8888EXT>, RequireExt<&Extensions::textureFormatBGRA8888EXT>);
    AddRGBAFormat(&map, GL_BGR5_A1_ANGLEX,    true,  5,  5,  5,  1, 0, GL_BGRA_EXT,     GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT, GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::textureFormatBGRA8888EXT>,    AlwaysSupported, RequireExt<&Extensions::textureFormatBGRA8888EXT>,    RequireExt<&Extensions::textureFormatBGRA8888EXT>, RequireExt<&Extensions::textureFormatBGRA8888EXT>);

    // Special format that is used for D3D textures that are used within ANGLE via the
    // EGL_ANGLE_d3d_texture_client_buffer extension. We don't allow uploading texture images with
    // this format, but textures in this format can be created from D3D textures, and filtering them
    // and rendering to them is allowed.
    AddRGBAFormat(&map, GL_BGRA8_SRGB_ANGLEX, true,  8,  8,  8,  8, 0, GL_BGRA_EXT,     GL_UNSIGNED_BYTE,                  GL_UNSIGNED_NORMALIZED, true,  NeverSupported,                                    AlwaysSupported, AlwaysSupported,                                   AlwaysSupported,                               AlwaysSupported);

    // Special format which is not really supported, so always false for all supports.
    AddRGBAFormat(&map, GL_BGR565_ANGLEX,     true,  5,  6,  5,  0, 0, GL_BGRA_EXT,     GL_UNSIGNED_SHORT_5_6_5,           GL_UNSIGNED_NORMALIZED, false, NeverSupported,                                    NeverSupported,  NeverSupported,                                    NeverSupported,                                NeverSupported);
    AddRGBAFormat(&map, GL_BGR10_A2_ANGLEX,   true, 10, 10, 10,  2, 0, GL_BGRA_EXT,     GL_UNSIGNED_INT_2_10_10_10_REV,    GL_UNSIGNED_NORMALIZED, false, NeverSupported,                                    NeverSupported,  NeverSupported,                                    NeverSupported,                                NeverSupported);

    // Special format to emulate RGB8 with RGBA8 within ANGLE.
    AddRGBAXFormat(&map, GL_RGBX8_ANGLE,      true,   FB< 8,  8,  8,  0, 8, 0>(), GL_RGB,          GL_UNSIGNED_BYTE,                  GL_UNSIGNED_NORMALIZED, false, AlwaysSupported,                                   AlwaysSupported, AlwaysSupported,                                   AlwaysSupported,                               NeverSupported);
    AddRGBAXFormat(&map, GL_RGBX8_SRGB_ANGLEX,      true,   FB< 8,  8,  8,  0, 8, 0>(), GL_RGB,          GL_UNSIGNED_BYTE,                  GL_UNSIGNED_NORMALIZED, true, AlwaysSupported,                                   AlwaysSupported, AlwaysSupported,                                   AlwaysSupported,                               NeverSupported);

    // Special format to emulate BGR8 with BGRA8 within ANGLE.
    AddRGBAXFormat(&map, GL_BGRX8_ANGLEX,      true,  FB< 8,  8,  8,  0, 8, 0>(), GL_BGRA_EXT,     GL_UNSIGNED_BYTE,                  GL_UNSIGNED_NORMALIZED, false, NeverSupported,                                    AlwaysSupported,  NeverSupported,                                    NeverSupported,                                NeverSupported);
    AddRGBAXFormat(&map, GL_BGRX8_SRGB_ANGLEX,      true,  FB< 8,  8,  8,  0, 8, 0>(), GL_BGRA_EXT,     GL_UNSIGNED_BYTE,                  GL_UNSIGNED_NORMALIZED, true, NeverSupported,                                    AlwaysSupported,  NeverSupported,                                    NeverSupported,                                NeverSupported);

    // This format is supported on ES 2.0 with two extensions, so keep it out-of-line to not widen the table above even more.
    //                 | Internal format     |sized| R | G | B | A |S | Format         | Type                             | Component type        | SRGB | Texture supported                                                                            | Filterable     | Texture attachment                               | Renderbuffer                                   | Blend
    AddRGBAFormat(&map, GL_RGB10_A2,          true, 10, 10, 10,  2, 0, GL_RGBA,         GL_UNSIGNED_INT_2_10_10_10_REV,    GL_UNSIGNED_NORMALIZED, false, RequireESOrExtAndExt<3, 0, &Extensions::textureStorageEXT, &Extensions::textureType2101010REVEXT>,  AlwaysSupported, RequireES<3, 0>,                                   RequireES<3, 0>,                                 RequireES<3, 0>);

    // Floating point formats
    //                 | Internal format |sized| R | G | B | A |S | Format | Type             | Component type | SRGB | Texture supported         | Filterable                                    | Texture attachment                          | Renderbuffer                            | Blend
    // It's not possible to have two entries per sized format.
    // E.g. for GL_RG16F, one with GL_HALF_FLOAT type and the other with GL_HALF_FLOAT_OES type.
    // So, GL_HALF_FLOAT type formats conditions are merged with GL_HALF_FLOAT_OES type conditions.
    AddRGBAFormat(&map, GL_R16F,          true, 16,  0,  0,  0, 0, GL_RED,  GL_HALF_FLOAT,     GL_FLOAT,        false, SizedHalfFloatRGSupport,    SizedHalfFloatFilterSupport,                    SizedHalfFloatRGTextureAttachmentSupport,     SizedHalfFloatRGRenderbufferSupport,       SizedHalfFloatRGRenderbufferSupport);
    AddRGBAFormat(&map, GL_RG16F,         true, 16, 16,  0,  0, 0, GL_RG,   GL_HALF_FLOAT,     GL_FLOAT,        false, SizedHalfFloatRGSupport,    SizedHalfFloatFilterSupport,                    SizedHalfFloatRGTextureAttachmentSupport,     SizedHalfFloatRGRenderbufferSupport,       SizedHalfFloatRGRenderbufferSupport);
    AddRGBAFormat(&map, GL_RGB16F,        true, 16, 16, 16,  0, 0, GL_RGB,  GL_HALF_FLOAT,     GL_FLOAT,        false, SizedHalfFloatSupport,      SizedHalfFloatFilterSupport,                    SizedHalfFloatRGBTextureAttachmentSupport,    SizedHalfFloatRGBRenderbufferSupport,      SizedHalfFloatRGBRenderbufferSupport);
    AddRGBAFormat(&map, GL_RGBA16F,       true, 16, 16, 16, 16, 0, GL_RGBA, GL_HALF_FLOAT,     GL_FLOAT,        false, SizedHalfFloatSupport,      SizedHalfFloatFilterSupport,                    SizedHalfFloatRGBATextureAttachmentSupport,   SizedHalfFloatRGBARenderbufferSupport,     SizedHalfFloatRGBARenderbufferSupport);
    AddRGBAFormat(&map, GL_R32F,          true, 32,  0,  0,  0, 0, GL_RED,  GL_FLOAT,          GL_FLOAT,        false, SizedFloatRGSupport,        RequireExt<&Extensions::textureFloatLinearOES>, RequireExt<&Extensions::colorBufferFloatEXT>,    RequireExt<&Extensions::colorBufferFloatEXT>, Float32BlendableSupport);
    AddRGBAFormat(&map, GL_RG32F,         true, 32, 32,  0,  0, 0, GL_RG,   GL_FLOAT,          GL_FLOAT,        false, SizedFloatRGSupport,        RequireExt<&Extensions::textureFloatLinearOES>, RequireExt<&Extensions::colorBufferFloatEXT>,    RequireExt<&Extensions::colorBufferFloatEXT>, Float32BlendableSupport);
    AddRGBAFormat(&map, GL_RGB32F,        true, 32, 32, 32,  0, 0, GL_RGB,  GL_FLOAT,          GL_FLOAT,        false, SizedFloatRGBSupport,       RequireExt<&Extensions::textureFloatLinearOES>, RequireExt<&Extensions::colorBufferFloatRgbCHROMIUM>, NeverSupported,                            NeverSupported);
    AddRGBAFormat(&map, GL_RGBA32F,       true, 32, 32, 32, 32, 0, GL_RGBA, GL_FLOAT,          GL_FLOAT,        false, SizedFloatRGBASupport,      RequireExt<&Extensions::textureFloatLinearOES>, SizedFloatRGBARenderableSupport,              SizedFloatRGBARenderableSupport,           Float32BlendableSupport);

    // ANGLE Depth stencil formats
    //                         | Internal format         |sized| D |S | X | Format            | Type                             | Component type        | Texture supported                                                                            | Filterable                                                                             | Texture attachment                                                                           | Renderbuffer                                                                                              | Blend
    AddDepthStencilFormat(&map, GL_DEPTH_COMPONENT16,     true, 16, 0,  0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT,                 GL_UNSIGNED_NORMALIZED, RequireESOrExtOrExt<3, 0, &Extensions::depthTextureANGLE, &Extensions::depthTextureOES>,       RequireESOrExtOrExt<3, 0, &Extensions::depthTextureANGLE, &Extensions::depthTextureOES>, RequireES<1, 0>,                                                                               RequireES<1, 0>,                                                                                             NeverSupported);
    AddDepthStencilFormat(&map, GL_DEPTH_COMPONENT24,     true, 24, 0,  8, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,                   GL_UNSIGNED_NORMALIZED, RequireES<3, 0>,                                                                               RequireESOrExt<3, 0, &Extensions::depthTextureANGLE>,                                    RequireES<3, 0>,                                                                               RequireESOrExt<3, 0, &Extensions::depth24OES>,                                                               NeverSupported);
    AddDepthStencilFormat(&map, GL_DEPTH_COMPONENT32F,    true, 32, 0,  0, GL_DEPTH_COMPONENT, GL_FLOAT,                          GL_FLOAT,               RequireES<3, 0>,                                                                               RequireESOrExt<3, 0, &Extensions::depthTextureANGLE>,                                    RequireES<3, 0>,                                                                               RequireES<3, 0>,                                                                                             NeverSupported);
    AddDepthStencilFormat(&map, GL_DEPTH_COMPONENT32_OES, true, 32, 0,  0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,                   GL_UNSIGNED_NORMALIZED, RequireExtOrExt<&Extensions::depthTextureANGLE, &Extensions::depthTextureOES>,                 AlwaysSupported,                                                                         RequireExtOrExt<&Extensions::depthTextureANGLE, &Extensions::depthTextureOES>,                 RequireExtOrExtOrExt<&Extensions::depthTextureANGLE, &Extensions::depthTextureOES, &Extensions::depth32OES>, NeverSupported);
    AddDepthStencilFormat(&map, GL_DEPTH24_STENCIL8,      true, 24, 8,  0, GL_DEPTH_STENCIL,   GL_UNSIGNED_INT_24_8,              GL_UNSIGNED_NORMALIZED, RequireESOrExtOrExt<3, 0, &Extensions::depthTextureANGLE, &Extensions::packedDepthStencilOES>, AlwaysSupported,                                                                         RequireESOrExtOrExt<3, 0, &Extensions::depthTextureANGLE, &Extensions::packedDepthStencilOES>, RequireESOrExtOrExt<3, 0, &Extensions::depthTextureANGLE, &Extensions::packedDepthStencilOES>,               NeverSupported);
    AddDepthStencilFormat(&map, GL_DEPTH32F_STENCIL8,     true, 32, 8, 24, GL_DEPTH_STENCIL,   GL_FLOAT_32_UNSIGNED_INT_24_8_REV, GL_FLOAT,               RequireESOrExt<3, 0, &Extensions::depthBufferFloat2NV>,                                        AlwaysSupported,                                                                         RequireESOrExt<3, 0, &Extensions::depthBufferFloat2NV>,                                        RequireESOrExt<3, 0, &Extensions::depthBufferFloat2NV>,                                                      NeverSupported);
    // STENCIL_INDEX8 is special-cased, see around the bottom of the list.

    // Luminance alpha formats
    //                | Internal format           |sized| L | A | Format            | Type             | Component type        | Texture supported                                                           | Filterable                                     | Texture attachment | Renderbuffer | Blend
    AddLUMAFormat(&map, GL_ALPHA8_EXT,             true,  0,  8, GL_ALPHA,           GL_UNSIGNED_BYTE,  GL_UNSIGNED_NORMALIZED, RequireExtOrExt<&Extensions::textureStorageEXT, &Extensions::requiredInternalformatOES>, AlwaysSupported,                         NeverSupported,      NeverSupported, NeverSupported);
    AddLUMAFormat(&map, GL_LUMINANCE8_EXT,         true,  8,  0, GL_LUMINANCE,       GL_UNSIGNED_BYTE,  GL_UNSIGNED_NORMALIZED, RequireExtOrExt<&Extensions::textureStorageEXT, &Extensions::requiredInternalformatOES>, AlwaysSupported,                         NeverSupported,      NeverSupported, NeverSupported);
    AddLUMAFormat(&map, GL_LUMINANCE4_ALPHA4_OES,  true,  8,  8, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,  GL_UNSIGNED_NORMALIZED, RequireExt<&Extensions::requiredInternalformatOES>,                              AlwaysSupported,                                 NeverSupported,      NeverSupported, NeverSupported);
    AddLUMAFormat(&map, GL_LUMINANCE8_ALPHA8_EXT,  true,  8,  8, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,  GL_UNSIGNED_NORMALIZED, RequireExtOrExt<&Extensions::textureStorageEXT, &Extensions::requiredInternalformatOES>, AlwaysSupported,                         NeverSupported,      NeverSupported, NeverSupported);
    AddLUMAFormat(&map, GL_ALPHA16F_EXT,           true,  0, 16, GL_ALPHA,           GL_HALF_FLOAT_OES, GL_FLOAT,               RequireExtAndExt<&Extensions::textureStorageEXT, &Extensions::textureHalfFloatOES>, RequireExt<&Extensions::textureHalfFloatLinearOES>, NeverSupported,      NeverSupported, NeverSupported);
    AddLUMAFormat(&map, GL_LUMINANCE16F_EXT,       true, 16,  0, GL_LUMINANCE,       GL_HALF_FLOAT_OES, GL_FLOAT,               RequireExtAndExt<&Extensions::textureStorageEXT, &Extensions::textureHalfFloatOES>, RequireExt<&Extensions::textureHalfFloatLinearOES>, NeverSupported,      NeverSupported, NeverSupported);
    AddLUMAFormat(&map, GL_LUMINANCE_ALPHA16F_EXT, true, 16, 16, GL_LUMINANCE_ALPHA, GL_HALF_FLOAT_OES, GL_FLOAT,               RequireExtAndExt<&Extensions::textureStorageEXT, &Extensions::textureHalfFloatOES>, RequireExt<&Extensions::textureHalfFloatLinearOES>, NeverSupported,      NeverSupported, NeverSupported);
    AddLUMAFormat(&map, GL_ALPHA32F_EXT,           true,  0, 32, GL_ALPHA,           GL_FLOAT,          GL_FLOAT,               RequireExtAndExt<&Extensions::textureStorageEXT, &Extensions::textureFloatOES>,  RequireExt<&Extensions::textureFloatLinearOES>,  NeverSupported,      NeverSupported, NeverSupported);
    AddLUMAFormat(&map, GL_LUMINANCE32F_EXT,       true, 32,  0, GL_LUMINANCE,       GL_FLOAT,          GL_FLOAT,               RequireExtAndExt<&Extensions::textureStorageEXT, &Extensions::textureFloatOES>,  RequireExt<&Extensions::textureFloatLinearOES>,  NeverSupported,      NeverSupported, NeverSupported);
    AddLUMAFormat(&map, GL_LUMINANCE_ALPHA32F_EXT, true, 32, 32, GL_LUMINANCE_ALPHA, GL_FLOAT,          GL_FLOAT,               RequireExtAndExt<&Extensions::textureStorageEXT, &Extensions::textureFloatOES>,  RequireExt<&Extensions::textureFloatLinearOES>,  NeverSupported,      NeverSupported, NeverSupported);

    // Compressed formats, From ES 3.0.1 spec, table 3.16
    //                       | Internal format                             |W |H |D | BS |CC| SRGB | Texture supported                                                          | Filterable     | Texture attachment | Renderbuffer  | Blend
    AddCompressedFormat(&map, GL_COMPRESSED_R11_EAC,                        4, 4, 1,  64, 1, false, ETC2EACSupport<&Extensions::compressedEACR11UnsignedTextureOES>,              AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SIGNED_R11_EAC,                 4, 4, 1,  64, 1, false, ETC2EACSupport<&Extensions::compressedEACR11SignedTextureOES>,                AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RG11_EAC,                       4, 4, 1, 128, 2, false, ETC2EACSupport<&Extensions::compressedEACRG11UnsignedTextureOES>,             AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SIGNED_RG11_EAC,                4, 4, 1, 128, 2, false, ETC2EACSupport<&Extensions::compressedEACRG11SignedTextureOES>,               AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGB8_ETC2,                      4, 4, 1,  64, 3, false, ETC2EACSupport<&Extensions::compressedETC2RGB8TextureOES>,                    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ETC2,                     4, 4, 1,  64, 3, true,  ETC2EACSupport<&Extensions::compressedETC2SRGB8TextureOES>,                   AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,  4, 4, 1,  64, 4, false, ETC2EACSupport<&Extensions::compressedETC2PunchthroughARGBA8TextureOES>,      AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, 4, 4, 1,  64, 4, true,  ETC2EACSupport<&Extensions::compressedETC2PunchthroughASRGB8AlphaTextureOES>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA8_ETC2_EAC,                 4, 4, 1, 128, 4, false, ETC2EACSupport<&Extensions::compressedETC2RGBA8TextureOES>,                   AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,          4, 4, 1, 128, 4, true,  ETC2EACSupport<&Extensions::compressedETC2SRGB8Alpha8TextureOES>,             AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);

    // From GL_EXT_texture_compression_dxt1
    //                       | Internal format                   |W |H |D | BS |CC| SRGB | Texture supported                                    | Filterable     | Texture attachment | Renderbuffer  | Blend
    AddCompressedFormat(&map, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,    4, 4, 1,  64, 3, false, RequireExt<&Extensions::textureCompressionDxt1EXT>,       AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,   4, 4, 1,  64, 4, false, RequireExt<&Extensions::textureCompressionDxt1EXT>,       AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);

    // From GL_ANGLE_texture_compression_dxt3
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE, 4, 4, 1, 128, 4, false, RequireExt<&Extensions::textureCompressionDxt3ANGLE>,       AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);

    // From GL_ANGLE_texture_compression_dxt5
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE, 4, 4, 1, 128, 4, false, RequireExt<&Extensions::textureCompressionDxt5ANGLE>,       AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);

    // From GL_OES_compressed_ETC1_RGB8_texture
    AddCompressedFormat(&map, GL_ETC1_RGB8_OES,                   4, 4, 1,  64, 3, false, RequireExt<&Extensions::compressedETC1RGB8TextureOES>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);

    // From GL_EXT_texture_compression_s3tc_srgb
    //                       | Internal format                       |W |H |D | BS |CC|SRGB | Texture supported                                 | Filterable     | Texture attachment | Renderbuffer  | Blend
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,       4, 4, 1,  64, 3, true, RequireExt<&Extensions::textureCompressionS3tcSrgbEXT>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, 4, 4, 1,  64, 4, true, RequireExt<&Extensions::textureCompressionS3tcSrgbEXT>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, 4, 4, 1, 128, 4, true, RequireExt<&Extensions::textureCompressionS3tcSrgbEXT>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 4, 4, 1, 128, 4, true, RequireExt<&Extensions::textureCompressionS3tcSrgbEXT>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);

    // From GL_KHR_texture_compression_astc_ldr and KHR_texture_compression_astc_hdr and GL_OES_texture_compression_astc
    //                       | Internal format                          | W | H |D | BS |CC| SRGB | Texture supported                                    | Filterable     | Texture attachment | Renderbuffer  | Blend
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_4x4_KHR,            4,  4, 1, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_5x4_KHR,            5,  4, 1, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_5x5_KHR,            5,  5, 1, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_6x5_KHR,            6,  5, 1, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_6x6_KHR,            6,  6, 1, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_8x5_KHR,            8,  5, 1, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_8x6_KHR,            8,  6, 1, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_8x8_KHR,            8,  8, 1, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_10x5_KHR,          10,  5, 1, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_10x6_KHR,          10,  6, 1, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_10x8_KHR,          10,  8, 1, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_10x10_KHR,         10, 10, 1, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_12x10_KHR,         12, 10, 1, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_12x12_KHR,         12, 12, 1, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);

    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR,    4,  4, 1, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR,    5,  4, 1, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR,    5,  5, 1, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR,    6,  5, 1, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR,    6,  6, 1, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR,    8,  5, 1, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR,    8,  6, 1, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR,    8,  8, 1, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR,  10,  5, 1, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR,  10,  6, 1, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR,  10,  8, 1, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR, 10, 10, 1, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR, 12, 10, 1, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR, 12, 12, 1, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcLdrKHR>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);

    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_3x3x3_OES,          3,  3, 3, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcOES>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_4x3x3_OES,          4,  3, 3, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcOES>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_4x4x3_OES,          4,  4, 3, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcOES>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_4x4x4_OES,          4,  4, 4, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcOES>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_5x4x4_OES,          5,  4, 4, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcOES>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_5x5x4_OES,          5,  5, 4, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcOES>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_5x5x5_OES,          5,  5, 5, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcOES>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_6x5x5_OES,          6,  5, 5, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcOES>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_6x6x5_OES,          6,  6, 5, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcOES>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_ASTC_6x6x6_OES,          6,  6, 6, 128, 4, false, RequireExt<&Extensions::textureCompressionAstcOES>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);

    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES,  3,  3, 3, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcOES>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES,  4,  3, 3, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcOES>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES,  4,  4, 3, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcOES>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES,  4,  4, 4, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcOES>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES,  5,  4, 4, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcOES>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES,  5,  5, 4, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcOES>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES,  5,  5, 5, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcOES>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES,  6,  5, 5, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcOES>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES,  6,  6, 5, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcOES>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES,  6,  6, 6, 128, 4, true,  RequireExt<&Extensions::textureCompressionAstcOES>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);

    // From EXT_texture_compression_rgtc
    //                       | Internal format                        | W | H |D | BS |CC| SRGB | Texture supported                              | Filterable     | Texture attachment | Renderbuffer  | Blend
    AddCompressedFormat(&map, GL_COMPRESSED_RED_RGTC1_EXT,              4,  4, 1,  64, 1, false, RequireExt<&Extensions::textureCompressionRgtcEXT>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SIGNED_RED_RGTC1_EXT,       4,  4, 1,  64, 1, false, RequireExt<&Extensions::textureCompressionRgtcEXT>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RED_GREEN_RGTC2_EXT,        4,  4, 1, 128, 2, false, RequireExt<&Extensions::textureCompressionRgtcEXT>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT, 4,  4, 1, 128, 2, false, RequireExt<&Extensions::textureCompressionRgtcEXT>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);

    // From EXT_texture_compression_bptc
    //                       | Internal format                         | W | H |D | BS |CC| SRGB | Texture supported                              | Filterable     | Texture attachment | Renderbuffer  | Blend
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT,         4,  4, 1, 128, 4, false, RequireExt<&Extensions::textureCompressionBptcEXT>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT,   4,  4, 1, 128, 4, true,  RequireExt<&Extensions::textureCompressionBptcEXT>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT,   4,  4, 1, 128, 4, false, RequireExt<&Extensions::textureCompressionBptcEXT>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT, 4,  4, 1, 128, 4, false, RequireExt<&Extensions::textureCompressionBptcEXT>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);

    // Paletted formats
    //                      | Internal format       |    | PS | Format | CC | Texture supported | Filterable     | Texture attachment | Renderbuffer  | Blend
    AddPalettedFormat(&map, GL_PALETTE4_RGB8_OES,      4,   3, GL_RGB,    3, RequireES1,         AlwaysSupported, NeverSupported,     NeverSupported, NeverSupported);
    AddPalettedFormat(&map, GL_PALETTE4_RGBA8_OES,     4,   4, GL_RGBA,   4, RequireES1,         AlwaysSupported, NeverSupported,     NeverSupported, NeverSupported);
    AddPalettedFormat(&map, GL_PALETTE4_R5_G6_B5_OES,  4,   2, GL_RGB,    3, RequireES1,         AlwaysSupported, NeverSupported,     NeverSupported, NeverSupported);
    AddPalettedFormat(&map, GL_PALETTE4_RGBA4_OES,     4,   2, GL_RGBA,   4, RequireES1,         AlwaysSupported, NeverSupported,     NeverSupported, NeverSupported);
    AddPalettedFormat(&map, GL_PALETTE4_RGB5_A1_OES,   4,   2, GL_RGBA,   4, RequireES1,         AlwaysSupported, NeverSupported,     NeverSupported, NeverSupported);
    AddPalettedFormat(&map, GL_PALETTE8_RGB8_OES,      8,   3, GL_RGB,    3, RequireES1,         AlwaysSupported, NeverSupported,     NeverSupported, NeverSupported);
    AddPalettedFormat(&map, GL_PALETTE8_RGBA8_OES,     8,   4, GL_RGBA,   4, RequireES1,         AlwaysSupported, NeverSupported,     NeverSupported, NeverSupported);
    AddPalettedFormat(&map, GL_PALETTE8_R5_G6_B5_OES,  8,   2, GL_RGB,    3, RequireES1,         AlwaysSupported, NeverSupported,     NeverSupported, NeverSupported);
    AddPalettedFormat(&map, GL_PALETTE8_RGBA4_OES,     8,   2, GL_RGBA,   4, RequireES1,         AlwaysSupported, NeverSupported,     NeverSupported, NeverSupported);
    AddPalettedFormat(&map, GL_PALETTE8_RGB5_A1_OES,   8,   2, GL_RGBA,   4, RequireES1,         AlwaysSupported, NeverSupported,     NeverSupported, NeverSupported);

    // From GL_IMG_texture_compression_pvrtc
    //                       | Internal format                       | W | H | D | BS |CC| SRGB | Texture supported                                 | Filterable     | Texture attachment | Renderbuffer  | Blend
    AddCompressedFormat(&map, GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG,      4,  4,  1,  64,  3, false, RequireExt<&Extensions::textureCompressionPvrtcIMG>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG,      8,  4,  1,  64,  3, false, RequireExt<&Extensions::textureCompressionPvrtcIMG>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG,     4,  4,  1,  64,  4, false, RequireExt<&Extensions::textureCompressionPvrtcIMG>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG,     8,  4,  1,  64,  4, false, RequireExt<&Extensions::textureCompressionPvrtcIMG>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);

    // From GL_EXT_pvrtc_sRGB
    //                       | Internal format                             | W | H | D | BS |CC| SRGB | Texture supported                                                                               | Filterable     | Texture attachment | Renderbuffer  | Blend
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT,           8,  4,  1,  64,  3, true, RequireExtAndExt<&Extensions::textureCompressionPvrtcIMG, &Extensions::pvrtcSRGBEXT>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT,           4,  4,  1,  64,  3, true, RequireExtAndExt<&Extensions::textureCompressionPvrtcIMG, &Extensions::pvrtcSRGBEXT>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT,     8,  4,  1,  64,  4, true, RequireExtAndExt<&Extensions::textureCompressionPvrtcIMG, &Extensions::pvrtcSRGBEXT>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT,     4,  4,  1,  64,  4, true, RequireExtAndExt<&Extensions::textureCompressionPvrtcIMG, &Extensions::pvrtcSRGBEXT>,    AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);

    // For STENCIL_INDEX8 we chose a normalized component type for the following reasons:
    // - Multisampled buffer are disallowed for non-normalized integer component types and we want to support it for STENCIL_INDEX8
    // - All other stencil formats (all depth-stencil) are either float or normalized
    // - It affects only validation of internalformat in RenderbufferStorageMultisample.
    //                         | Internal format  |sized|D |S |X | Format          | Type            | Component type        | Texture supported                                    | Filterable    | Texture attachment                                   | Renderbuffer   | Blend
    AddDepthStencilFormat(&map, GL_STENCIL_INDEX8, true, 0, 8, 0, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, GL_UNSIGNED_NORMALIZED, RequireESOrExt<3, 2, &Extensions::textureStencil8OES>, NeverSupported, RequireESOrExt<3, 2, &Extensions::textureStencil8OES>, RequireES<1, 0>, NeverSupported);

    // From GL_ANGLE_lossy_etc_decode
    //                       | Internal format                                                |W |H |D |BS |CC| SRGB | Texture supported                      | Filterable     | Texture attachment | Renderbuffer  | Blend
    AddCompressedFormat(&map, GL_ETC1_RGB8_LOSSY_DECODE_ANGLE,                                 4, 4, 1, 64, 3, false, RequireExt<&Extensions::lossyEtcDecodeANGLE>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGB8_LOSSY_DECODE_ETC2_ANGLE,                      4, 4, 1, 64, 3, false, RequireExt<&Extensions::lossyEtcDecodeANGLE>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_LOSSY_DECODE_ETC2_ANGLE,                     4, 4, 1, 64, 3, true,  RequireExt<&Extensions::lossyEtcDecodeANGLE>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE,  4, 4, 1, 64, 3, false, RequireExt<&Extensions::lossyEtcDecodeANGLE>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddCompressedFormat(&map, GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE, 4, 4, 1, 64, 3, true,  RequireExt<&Extensions::lossyEtcDecodeANGLE>, AlwaysSupported, NeverSupported,      NeverSupported, NeverSupported);

    // From GL_EXT_texture_norm16
    //                 | Internal format    |sized| R | G | B | A |S | Format | Type             | Component type        | SRGB | Texture supported                        | Filterable     | Texture attachment                                                          | Renderbuffer                                                                | Blend
    AddRGBAFormat(&map, GL_R16_EXT,          true, 16,  0,  0,  0, 0, GL_RED,  GL_UNSIGNED_SHORT, GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::textureNorm16EXT>, AlwaysSupported, RequireExt<&Extensions::textureNorm16EXT>,                                    RequireExt<&Extensions::textureNorm16EXT>,                                    RequireExt<&Extensions::textureNorm16EXT>);
    AddRGBAFormat(&map, GL_R16_SNORM_EXT,    true, 16,  0,  0,  0, 0, GL_RED,  GL_SHORT,          GL_SIGNED_NORMALIZED,   false, RequireExt<&Extensions::textureNorm16EXT>, AlwaysSupported, RequireExtAndExt<&Extensions::textureNorm16EXT, &Extensions::renderSnormEXT>, RequireExtAndExt<&Extensions::textureNorm16EXT, &Extensions::renderSnormEXT>, RequireExtAndExt<&Extensions::textureNorm16EXT, &Extensions::renderSnormEXT>);
    AddRGBAFormat(&map, GL_RG16_EXT,         true, 16, 16,  0,  0, 0, GL_RG,   GL_UNSIGNED_SHORT, GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::textureNorm16EXT>, AlwaysSupported, RequireExt<&Extensions::textureNorm16EXT>,                                    RequireExt<&Extensions::textureNorm16EXT>,                                    RequireExt<&Extensions::textureNorm16EXT>);
    AddRGBAFormat(&map, GL_RG16_SNORM_EXT,   true, 16, 16,  0,  0, 0, GL_RG,   GL_SHORT,          GL_SIGNED_NORMALIZED,   false, RequireExt<&Extensions::textureNorm16EXT>, AlwaysSupported, RequireExtAndExt<&Extensions::textureNorm16EXT, &Extensions::renderSnormEXT>, RequireExtAndExt<&Extensions::textureNorm16EXT, &Extensions::renderSnormEXT>, RequireExtAndExt<&Extensions::textureNorm16EXT, &Extensions::renderSnormEXT>);
    AddRGBAFormat(&map, GL_RGB16_EXT,        true, 16, 16, 16,  0, 0, GL_RGB,  GL_UNSIGNED_SHORT, GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::textureNorm16EXT>, AlwaysSupported, NeverSupported,                                                               NeverSupported,                                                               NeverSupported);
    AddRGBAFormat(&map, GL_RGB16_SNORM_EXT,  true, 16, 16, 16,  0, 0, GL_RGB,  GL_SHORT,          GL_SIGNED_NORMALIZED,   false, RequireExt<&Extensions::textureNorm16EXT>, AlwaysSupported, NeverSupported,                                                               NeverSupported,                                                               NeverSupported);
    AddRGBAFormat(&map, GL_RGBA16_EXT,       true, 16, 16, 16, 16, 0, GL_RGBA, GL_UNSIGNED_SHORT, GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::textureNorm16EXT>, AlwaysSupported, RequireExt<&Extensions::textureNorm16EXT>,                                    RequireExt<&Extensions::textureNorm16EXT>,                                    RequireExt<&Extensions::textureNorm16EXT>);
    AddRGBAFormat(&map, GL_RGBA16_SNORM_EXT, true, 16, 16, 16, 16, 0, GL_RGBA, GL_SHORT,          GL_SIGNED_NORMALIZED,   false, RequireExt<&Extensions::textureNorm16EXT>, AlwaysSupported, RequireExtAndExt<&Extensions::textureNorm16EXT, &Extensions::renderSnormEXT>, RequireExtAndExt<&Extensions::textureNorm16EXT, &Extensions::renderSnormEXT>, RequireExtAndExt<&Extensions::textureNorm16EXT, &Extensions::renderSnormEXT>);

    // From EXT_texture_sRGB_R8
    //                 | Internal format    |sized| R | G | B | A |S | Format | Type             | Component type        | SRGB | Texture supported                     | Filterable     | Texture attachment                    | Renderbuffer                          | Blend
    AddRGBAFormat(&map, GL_SR8_EXT,          true,  8,  0,  0,  0, 0, GL_RED,  GL_UNSIGNED_BYTE,  GL_UNSIGNED_NORMALIZED, true,  RequireExt<&Extensions::textureSRGBR8EXT>,     AlwaysSupported, NeverSupported,                         NeverSupported,                         NeverSupported);

    // From EXT_texture_sRGB_RG8
    //                 | Internal format    |sized| R | G | B | A |S | Format | Type             | Component type        | SRGB | Texture supported                     | Filterable     | Texture attachment                    | Renderbuffer                          | Blend
    AddRGBAFormat(&map, GL_SRG8_EXT,         true,  8,  8,  0,  0, 0, GL_RG,   GL_UNSIGNED_BYTE,  GL_UNSIGNED_NORMALIZED, true,  RequireExt<&Extensions::textureSRGBRG8EXT>,    AlwaysSupported, NeverSupported,                         NeverSupported,                         NeverSupported);

    // From GL_OES_required_internalformat
    // The |shared| bit shouldn't be 2. But given this hits assertion when bits
    // are checked, it's fine to have this bit set as 2 as a workaround.
    AddRGBAFormat(&map, GL_RGB10_EXT,           true,   10, 10, 10, 0, 2,         GL_RGB,            GL_UNSIGNED_INT_2_10_10_10_REV, GL_UNSIGNED_NORMALIZED, false, RequireExtAndExt<&Extensions::textureType2101010REVEXT,&Extensions::requiredInternalformatOES>, NeverSupported,  NeverSupported, NeverSupported, NeverSupported);

    // Unsized formats
    //                  | Internal format  |sized |    R | G | B | A |S |X   | Format           | Type                          | Component type        | SRGB | Texture supported                                  | Filterable     | Texture attachment                               | Renderbuffer  | Blend
    AddRGBAXFormat(&map, GL_RED,            false, FB< 8,  0,  0,  0, 0, 0>(), GL_RED,            GL_UNSIGNED_BYTE,               GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::textureRgEXT>,               AlwaysSupported, RequireExt<&Extensions::textureRgEXT>,             NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_RED,            false, FB< 8,  0,  0,  0, 0, 0>(), GL_RED,            GL_BYTE,                        GL_SIGNED_NORMALIZED,   false, NeverSupported,                                      NeverSupported,  NeverSupported,                                    NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_RED,            false, FB<16,  0,  0,  0, 0, 0>(), GL_RED,            GL_UNSIGNED_SHORT,              GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::textureNorm16EXT>,           AlwaysSupported, RequireExt<&Extensions::textureNorm16EXT>,         NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_RED,            false, FB<16,  0,  0,  0, 0, 0>(), GL_RED,            GL_SHORT,                       GL_SIGNED_NORMALIZED,   false, NeverSupported,                                      AlwaysSupported, NeverSupported,                                    NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_RG,             false, FB< 8,  8,  0,  0, 0, 0>(), GL_RG,             GL_UNSIGNED_BYTE,               GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::textureRgEXT>,               AlwaysSupported, RequireExt<&Extensions::textureRgEXT>,             NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_RG,             false, FB< 8,  8,  0,  0, 0, 0>(), GL_RG,             GL_BYTE,                        GL_SIGNED_NORMALIZED,   false, NeverSupported,                                      NeverSupported,  NeverSupported,                                    NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_RG,             false, FB<16, 16,  0,  0, 0, 0>(), GL_RG,             GL_UNSIGNED_SHORT,              GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::textureNorm16EXT>,           AlwaysSupported, RequireExt<&Extensions::textureNorm16EXT>,         NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_RG,             false, FB<16, 16,  0,  0, 0, 0>(), GL_RG,             GL_SHORT,                       GL_SIGNED_NORMALIZED,   false, NeverSupported,                                      AlwaysSupported, NeverSupported,                                    NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_RGB,            false, FB< 8,  8,  8,  0, 0, 0>(), GL_RGB,            GL_UNSIGNED_BYTE,               GL_UNSIGNED_NORMALIZED, false, AlwaysSupported,                                     AlwaysSupported, RequireESOrExt<2, 0, &Extensions::framebufferObjectOES>,                                NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_RGB,            false, FB< 5,  6,  5,  0, 0, 0>(), GL_RGB,            GL_UNSIGNED_SHORT_5_6_5,        GL_UNSIGNED_NORMALIZED, false, AlwaysSupported,                                     AlwaysSupported, RequireESOrExt<2, 0, &Extensions::framebufferObjectOES>,                                NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_RGB,            false, FB< 8,  8,  8,  0, 0, 0>(), GL_RGB,            GL_BYTE,                        GL_SIGNED_NORMALIZED,   false, NeverSupported,                                      NeverSupported,  NeverSupported,                                    NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_RGB,            false, FB<10, 10, 10,  0, 0, 2>(), GL_RGB,            GL_UNSIGNED_INT_2_10_10_10_REV, GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::textureType2101010REVEXT>, AlwaysSupported, NeverSupported,                                    NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_RGB,            false, FB<16, 16, 16,  0, 0, 0>(), GL_RGB,            GL_UNSIGNED_SHORT,              GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::textureNorm16EXT>,           AlwaysSupported, RequireExt<&Extensions::textureNorm16EXT>,         NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_RGB,            false, FB<16, 16, 16,  0, 0, 0>(), GL_RGB,            GL_SHORT,                       GL_SIGNED_NORMALIZED,   false, NeverSupported,                                      AlwaysSupported, NeverSupported,                                    NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_RGBA,           false, FB< 4,  4,  4,  4, 0, 0>(), GL_RGBA,           GL_UNSIGNED_SHORT_4_4_4_4,      GL_UNSIGNED_NORMALIZED, false, AlwaysSupported,                                     AlwaysSupported, RequireESOrExt<2, 0, &Extensions::framebufferObjectOES>,                                NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_RGBA,           false, FB< 5,  5,  5,  1, 0, 0>(), GL_RGBA,           GL_UNSIGNED_SHORT_5_5_5_1,      GL_UNSIGNED_NORMALIZED, false, AlwaysSupported,                                     AlwaysSupported, RequireESOrExt<2, 0, &Extensions::framebufferObjectOES>,                                NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_RGBA,           false, FB< 8,  8,  8,  8, 0, 0>(), GL_RGBA,           GL_UNSIGNED_BYTE,               GL_UNSIGNED_NORMALIZED, false, AlwaysSupported,                                     AlwaysSupported, RequireESOrExt<2, 0, &Extensions::framebufferObjectOES>,                                NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_RGBA,           false, FB<16, 16, 16, 16, 0, 0>(), GL_RGBA,           GL_UNSIGNED_SHORT,              GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::textureNorm16EXT>,           AlwaysSupported, RequireExt<&Extensions::textureNorm16EXT>,         NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_RGBA,           false, FB<16, 16, 16, 16, 0, 0>(), GL_RGBA,           GL_SHORT,                       GL_SIGNED_NORMALIZED,   false, NeverSupported,                                      AlwaysSupported, NeverSupported,                                    NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_RGBA,           false, FB<10, 10, 10,  2, 0, 0>(), GL_RGBA,           GL_UNSIGNED_INT_2_10_10_10_REV, GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::textureType2101010REVEXT>, AlwaysSupported, NeverSupported,                                    NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_RGBA,           false, FB< 8,  8,  8,  8, 0, 0>(), GL_RGBA,           GL_BYTE,                        GL_SIGNED_NORMALIZED,   false, NeverSupported,                                      NeverSupported,  NeverSupported,                                    NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_SRGB,           false, FB< 8,  8,  8,  0, 0, 0>(), GL_SRGB,           GL_UNSIGNED_BYTE,               GL_UNSIGNED_NORMALIZED, true,  RequireExt<&Extensions::sRGBEXT>,                    AlwaysSupported, NeverSupported,                                    NeverSupported, NeverSupported);
    AddRGBAXFormat(&map, GL_SRGB_ALPHA_EXT, false, FB< 8,  8,  8,  8, 0, 0>(), GL_SRGB_ALPHA_EXT, GL_UNSIGNED_BYTE,               GL_UNSIGNED_NORMALIZED, true,  RequireExt<&Extensions::sRGBEXT>,                    AlwaysSupported, RequireExt<&Extensions::sRGBEXT>,                  NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_BGRA_EXT,       false,  8,  8,  8,  8, 0, GL_BGRA_EXT,       GL_UNSIGNED_BYTE,               GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::textureFormatBGRA8888EXT>,   AlwaysSupported, RequireExt<&Extensions::textureFormatBGRA8888EXT>, NeverSupported, NeverSupported);

    // Unsized integer formats
    //                 |Internal format |sized | R | G | B | A |S | Format         | Type                          | Component type | SRGB | Texture supported | Filterable    | Texture attachment | Renderbuffer  | Blend
    AddRGBAFormat(&map, GL_RED_INTEGER,  false,  8,  0,  0,  0, 0, GL_RED_INTEGER,  GL_BYTE,                        GL_INT,          false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RED_INTEGER,  false,  8,  0,  0,  0, 0, GL_RED_INTEGER,  GL_UNSIGNED_BYTE,               GL_UNSIGNED_INT, false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RED_INTEGER,  false, 16,  0,  0,  0, 0, GL_RED_INTEGER,  GL_SHORT,                       GL_INT,          false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RED_INTEGER,  false, 16,  0,  0,  0, 0, GL_RED_INTEGER,  GL_UNSIGNED_SHORT,              GL_UNSIGNED_INT, false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RED_INTEGER,  false, 32,  0,  0,  0, 0, GL_RED_INTEGER,  GL_INT,                         GL_INT,          false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RED_INTEGER,  false, 32,  0,  0,  0, 0, GL_RED_INTEGER,  GL_UNSIGNED_INT,                GL_UNSIGNED_INT, false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RG_INTEGER,   false,  8,  8,  0,  0, 0, GL_RG_INTEGER,   GL_BYTE,                        GL_INT,          false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RG_INTEGER,   false,  8,  8,  0,  0, 0, GL_RG_INTEGER,   GL_UNSIGNED_BYTE,               GL_UNSIGNED_INT, false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RG_INTEGER,   false, 16, 16,  0,  0, 0, GL_RG_INTEGER,   GL_SHORT,                       GL_INT,          false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RG_INTEGER,   false, 16, 16,  0,  0, 0, GL_RG_INTEGER,   GL_UNSIGNED_SHORT,              GL_UNSIGNED_INT, false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RG_INTEGER,   false, 32, 32,  0,  0, 0, GL_RG_INTEGER,   GL_INT,                         GL_INT,          false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RG_INTEGER,   false, 32, 32,  0,  0, 0, GL_RG_INTEGER,   GL_UNSIGNED_INT,                GL_UNSIGNED_INT, false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGB_INTEGER,  false,  8,  8,  8,  0, 0, GL_RGB_INTEGER,  GL_BYTE,                        GL_INT,          false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGB_INTEGER,  false,  8,  8,  8,  0, 0, GL_RGB_INTEGER,  GL_UNSIGNED_BYTE,               GL_UNSIGNED_INT, false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGB_INTEGER,  false, 16, 16, 16,  0, 0, GL_RGB_INTEGER,  GL_SHORT,                       GL_INT,          false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGB_INTEGER,  false, 16, 16, 16,  0, 0, GL_RGB_INTEGER,  GL_UNSIGNED_SHORT,              GL_UNSIGNED_INT, false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGB_INTEGER,  false, 32, 32, 32,  0, 0, GL_RGB_INTEGER,  GL_INT,                         GL_INT,          false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGB_INTEGER,  false, 32, 32, 32,  0, 0, GL_RGB_INTEGER,  GL_UNSIGNED_INT,                GL_UNSIGNED_INT, false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGBA_INTEGER, false,  8,  8,  8,  8, 0, GL_RGBA_INTEGER, GL_BYTE,                        GL_INT,          false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGBA_INTEGER, false,  8,  8,  8,  8, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE,               GL_UNSIGNED_INT, false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGBA_INTEGER, false, 16, 16, 16, 16, 0, GL_RGBA_INTEGER, GL_SHORT,                       GL_INT,          false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGBA_INTEGER, false, 16, 16, 16, 16, 0, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT,              GL_UNSIGNED_INT, false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGBA_INTEGER, false, 32, 32, 32, 32, 0, GL_RGBA_INTEGER, GL_INT,                         GL_INT,          false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGBA_INTEGER, false, 32, 32, 32, 32, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT,                GL_UNSIGNED_INT, false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGBA_INTEGER, false, 10, 10, 10,  2, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV, GL_UNSIGNED_INT, false, RequireES<3, 0>,    NeverSupported, NeverSupported,      NeverSupported, NeverSupported);

    // Unsized floating point formats
    //                 |Internal format |sized | R | G | B | A |S | Format | Type                           | Comp    | SRGB | Texture supported                                                         | Filterable                                     | Texture attachment                             | Renderbuffer  | Blend
    AddRGBAFormat(&map, GL_RED,          false, 16,  0,  0,  0, 0, GL_RED,  GL_HALF_FLOAT,                   GL_FLOAT, false, NeverSupported,                                                             NeverSupported,                                  NeverSupported,                                  NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RG,           false, 16, 16,  0,  0, 0, GL_RG,   GL_HALF_FLOAT,                   GL_FLOAT, false, NeverSupported,                                                             NeverSupported,                                  NeverSupported,                                  NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGB,          false, 16, 16, 16,  0, 0, GL_RGB,  GL_HALF_FLOAT,                   GL_FLOAT, false, NeverSupported,                                                             NeverSupported,                                  NeverSupported,                                  NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGBA,         false, 16, 16, 16, 16, 0, GL_RGBA, GL_HALF_FLOAT,                   GL_FLOAT, false, NeverSupported,                                                             NeverSupported,                                  NeverSupported,                                  NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RED,          false, 16,  0,  0,  0, 0, GL_RED,  GL_HALF_FLOAT_OES,               GL_FLOAT, false, RequireExtAndExt<&Extensions::textureHalfFloatOES, &Extensions::textureRgEXT>,    RequireExt<&Extensions::textureHalfFloatLinearOES>, AlwaysSupported,                                 NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RG,           false, 16, 16,  0,  0, 0, GL_RG,   GL_HALF_FLOAT_OES,               GL_FLOAT, false, RequireExtAndExt<&Extensions::textureHalfFloatOES, &Extensions::textureRgEXT>,    RequireExt<&Extensions::textureHalfFloatLinearOES>, AlwaysSupported,                                 NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGB,          false, 16, 16, 16,  0, 0, GL_RGB,  GL_HALF_FLOAT_OES,               GL_FLOAT, false, RequireExt<&Extensions::textureHalfFloatOES>,                                  RequireExt<&Extensions::textureHalfFloatLinearOES>, RequireExt<&Extensions::colorBufferHalfFloatEXT>,   NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGBA,         false, 16, 16, 16, 16, 0, GL_RGBA, GL_HALF_FLOAT_OES,               GL_FLOAT, false, RequireExt<&Extensions::textureHalfFloatOES>,                                  RequireExt<&Extensions::textureHalfFloatLinearOES>, RequireExt<&Extensions::colorBufferHalfFloatEXT>,   NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RED,          false, 32,  0,  0,  0, 0, GL_RED,  GL_FLOAT,                        GL_FLOAT, false, RequireExtAndExt<&Extensions::textureFloatOES, &Extensions::textureRgEXT>,     RequireExt<&Extensions::textureFloatLinearOES>,  AlwaysSupported,                                 NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RG,           false, 32, 32,  0,  0, 0, GL_RG,   GL_FLOAT,                        GL_FLOAT, false, RequireExtAndExt<&Extensions::textureFloatOES, &Extensions::textureRgEXT>,     RequireExt<&Extensions::textureFloatLinearOES>,  AlwaysSupported,                                 NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGB,          false, 32, 32, 32,  0, 0, GL_RGB,  GL_FLOAT,                        GL_FLOAT, false, RequireExt<&Extensions::textureFloatOES>,                                   RequireExt<&Extensions::textureFloatLinearOES>,  NeverSupported,                                  NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGB,          false,  9,  9,  9,  0, 5, GL_RGB,  GL_UNSIGNED_INT_5_9_9_9_REV,     GL_FLOAT, false, NeverSupported,                                                             NeverSupported,                                  NeverSupported,                                  NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGB,          false, 11, 11, 10,  0, 0, GL_RGB,  GL_UNSIGNED_INT_10F_11F_11F_REV, GL_FLOAT, false, NeverSupported,                                                             NeverSupported,                                  NeverSupported,                                  NeverSupported, NeverSupported);
    AddRGBAFormat(&map, GL_RGBA,         false, 32, 32, 32, 32, 0, GL_RGBA, GL_FLOAT,                        GL_FLOAT, false, RequireExt<&Extensions::textureFloatOES>,                                   RequireExt<&Extensions::textureFloatLinearOES>,  NeverSupported,                                  NeverSupported, NeverSupported);

    // Unsized luminance alpha formats
    //                 | Internal format   |sized | L | A | Format            | Type             | Component type        | Texture supported                        | Filterable                                     | Texture attachment | Renderbuffer  | Blend
    AddLUMAFormat(&map, GL_ALPHA,           false,  0,  8, GL_ALPHA,           GL_UNSIGNED_BYTE,  GL_UNSIGNED_NORMALIZED, AlwaysSupported,                           AlwaysSupported,                                 NeverSupported,      NeverSupported, NeverSupported);
    AddLUMAFormat(&map, GL_LUMINANCE,       false,  8,  0, GL_LUMINANCE,       GL_UNSIGNED_BYTE,  GL_UNSIGNED_NORMALIZED, AlwaysSupported,                           AlwaysSupported,                                 NeverSupported,      NeverSupported, NeverSupported);
    AddLUMAFormat(&map, GL_LUMINANCE_ALPHA, false,  8,  8, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,  GL_UNSIGNED_NORMALIZED, AlwaysSupported,                           AlwaysSupported,                                 NeverSupported,      NeverSupported, NeverSupported);
    AddLUMAFormat(&map, GL_ALPHA,           false,  0, 16, GL_ALPHA,           GL_HALF_FLOAT_OES, GL_FLOAT,               RequireExt<&Extensions::textureHalfFloatOES>, RequireExt<&Extensions::textureHalfFloatLinearOES>, NeverSupported,      NeverSupported, NeverSupported);
    AddLUMAFormat(&map, GL_LUMINANCE,       false, 16,  0, GL_LUMINANCE,       GL_HALF_FLOAT_OES, GL_FLOAT,               RequireExt<&Extensions::textureHalfFloatOES>, RequireExt<&Extensions::textureHalfFloatLinearOES>, NeverSupported,      NeverSupported, NeverSupported);
    AddLUMAFormat(&map, GL_LUMINANCE_ALPHA, false, 16, 16, GL_LUMINANCE_ALPHA, GL_HALF_FLOAT_OES, GL_FLOAT,               RequireExt<&Extensions::textureHalfFloatOES>, RequireExt<&Extensions::textureHalfFloatLinearOES>, NeverSupported,      NeverSupported, NeverSupported);
    AddLUMAFormat(&map, GL_ALPHA,           false,  0, 32, GL_ALPHA,           GL_FLOAT,          GL_FLOAT,               RequireExt<&Extensions::textureFloatOES>,  RequireExt<&Extensions::textureFloatLinearOES>,  NeverSupported,      NeverSupported, NeverSupported);
    AddLUMAFormat(&map, GL_LUMINANCE,       false, 32,  0, GL_LUMINANCE,       GL_FLOAT,          GL_FLOAT,               RequireExt<&Extensions::textureFloatOES>,  RequireExt<&Extensions::textureFloatLinearOES>,  NeverSupported,      NeverSupported, NeverSupported);
    AddLUMAFormat(&map, GL_LUMINANCE_ALPHA, false, 32, 32, GL_LUMINANCE_ALPHA, GL_FLOAT,          GL_FLOAT,               RequireExt<&Extensions::textureFloatOES>,  RequireExt<&Extensions::textureFloatLinearOES>,  NeverSupported,      NeverSupported, NeverSupported);

    // Unsized depth stencil formats
    //                         | Internal format   |sized | D |S | X | Format            | Type                             | Component type        | Texture supported                                       | Filterable     | Texture attachment                                                                  | Renderbuffer                                                                       | Blend
    AddDepthStencilFormat(&map, GL_DEPTH_COMPONENT, false, 16, 0,  0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT,                 GL_UNSIGNED_NORMALIZED, RequireES<1, 0>,                                          AlwaysSupported, RequireExtOrExt<&Extensions::depthTextureANGLE, &Extensions::depthTextureOES>,        RequireExtOrExt<&Extensions::depthTextureANGLE, &Extensions::depthTextureOES>,        RequireExtOrExt<&Extensions::depthTextureANGLE, &Extensions::depthTextureOES>);
    AddDepthStencilFormat(&map, GL_DEPTH_COMPONENT, false, 24, 0,  8, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,                   GL_UNSIGNED_NORMALIZED, RequireES<1, 0>,                                          AlwaysSupported, RequireExtOrExt<&Extensions::depthTextureANGLE, &Extensions::depthTextureOES>,        RequireExtOrExt<&Extensions::depthTextureANGLE, &Extensions::depthTextureOES>,        RequireExtOrExt<&Extensions::depthTextureANGLE, &Extensions::depthTextureOES>);
    AddDepthStencilFormat(&map, GL_DEPTH_COMPONENT, false, 32, 0,  0, GL_DEPTH_COMPONENT, GL_FLOAT,                          GL_FLOAT,               RequireES<1, 0>,                                          AlwaysSupported, RequireES<1, 0>,                                                                      RequireES<1, 0>,                                                                      RequireES<1, 0>);
    AddDepthStencilFormat(&map, GL_DEPTH_COMPONENT, false, 24, 8,  0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT_24_8,              GL_UNSIGNED_NORMALIZED, RequireESOrExt<3, 0, &Extensions::packedDepthStencilOES>, AlwaysSupported, RequireExtAndExt<&Extensions::packedDepthStencilOES, &Extensions::depthTextureANGLE>, RequireExtAndExt<&Extensions::packedDepthStencilOES, &Extensions::depthTextureANGLE>, RequireExtAndExt<&Extensions::packedDepthStencilOES, &Extensions::depthTextureANGLE>);
    AddDepthStencilFormat(&map, GL_DEPTH_STENCIL,   false, 24, 8,  0, GL_DEPTH_STENCIL,   GL_UNSIGNED_INT_24_8,              GL_UNSIGNED_NORMALIZED, RequireESOrExt<3, 0, &Extensions::packedDepthStencilOES>, AlwaysSupported, RequireExtAndExt<&Extensions::packedDepthStencilOES, &Extensions::depthTextureANGLE>, RequireExtAndExt<&Extensions::packedDepthStencilOES, &Extensions::depthTextureANGLE>, RequireExtAndExt<&Extensions::packedDepthStencilOES, &Extensions::depthTextureANGLE>);
    AddDepthStencilFormat(&map, GL_DEPTH_STENCIL,   false, 32, 8, 24, GL_DEPTH_STENCIL,   GL_FLOAT_32_UNSIGNED_INT_24_8_REV, GL_FLOAT,               RequireESOrExt<3, 0, &Extensions::packedDepthStencilOES>, AlwaysSupported, RequireExt<&Extensions::packedDepthStencilOES>,                                       RequireExt<&Extensions::packedDepthStencilOES>,                                       RequireExt<&Extensions::packedDepthStencilOES>);
    AddDepthStencilFormat(&map, GL_STENCIL_INDEX,   false,  0, 8,  0, GL_STENCIL_INDEX,   GL_UNSIGNED_BYTE,                  GL_UNSIGNED_NORMALIZED, NeverSupported,                                           NeverSupported , NeverSupported,                                                                       NeverSupported,                                                                       NeverSupported);

    // Non-standard YUV formats
    //                 | Internal format                             | sized | Cr | Y | Cb | A | S | Format                              | Type            | Comp                  | SRGB | Texture supported                                       | Filterable                                              | Texture attachment                                      | Renderbuffer  | Blend
    AddYUVFormat(&map,  GL_G8_B8R8_2PLANE_420_UNORM_ANGLE,            true,   8,   8,  8,   0,  0,  GL_G8_B8R8_2PLANE_420_UNORM_ANGLE,    GL_UNSIGNED_BYTE, GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::yuvInternalFormatANGLE>,          RequireExt<&Extensions::yuvInternalFormatANGLE>,          RequireExt<&Extensions::yuvInternalFormatANGLE>,          NeverSupported, NeverSupported);
    AddYUVFormat(&map,  GL_G8_B8_R8_3PLANE_420_UNORM_ANGLE,           true,   8,   8,  8,   0,  0,  GL_G8_B8_R8_3PLANE_420_UNORM_ANGLE,   GL_UNSIGNED_BYTE, GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::yuvInternalFormatANGLE>,          RequireExt<&Extensions::yuvInternalFormatANGLE>,          RequireExt<&Extensions::yuvInternalFormatANGLE>,          NeverSupported, NeverSupported);

    // clang-format on

    return map;
}

const InternalFormatInfoMap &GetInternalFormatMap()
{
    static const angle::base::NoDestructor<InternalFormatInfoMap> formatMap(
        BuildInternalFormatInfoMap());
    return *formatMap;
}

int GetAndroidHardwareBufferFormatFromChannelSizes(const egl::AttributeMap &attribMap)
{
    // Retrieve channel size from attribute map. The default value should be 0, per spec.
    GLuint redSize   = static_cast<GLuint>(attribMap.getAsInt(EGL_RED_SIZE, 0));
    GLuint greenSize = static_cast<GLuint>(attribMap.getAsInt(EGL_GREEN_SIZE, 0));
    GLuint blueSize  = static_cast<GLuint>(attribMap.getAsInt(EGL_BLUE_SIZE, 0));
    GLuint alphaSize = static_cast<GLuint>(attribMap.getAsInt(EGL_ALPHA_SIZE, 0));

    GLenum glInternalFormat = 0;
    for (GLenum sizedInternalFormat : angle::android::kSupportedSizedInternalFormats)
    {
        const gl::InternalFormat &internalFormat = GetSizedInternalFormatInfo(sizedInternalFormat);
        ASSERT(internalFormat.internalFormat != GL_NONE && internalFormat.sized);

        if (internalFormat.isChannelSizeCompatible(redSize, greenSize, blueSize, alphaSize))
        {
            glInternalFormat = sizedInternalFormat;
            break;
        }
    }

    return (glInternalFormat != 0)
               ? angle::android::GLInternalFormatToNativePixelFormat(glInternalFormat)
               : 0;
}

GLenum GetConfigColorBufferFormat(const egl::Config *config)
{
    GLenum componentType = GL_NONE;
    switch (config->colorComponentType)
    {
        case EGL_COLOR_COMPONENT_TYPE_FIXED_EXT:
            componentType = GL_UNSIGNED_NORMALIZED;
            break;
        case EGL_COLOR_COMPONENT_TYPE_FLOAT_EXT:
            componentType = GL_FLOAT;
            break;
        default:
            UNREACHABLE();
            return GL_NONE;
    }

    GLenum colorEncoding = GL_LINEAR;

    for (GLenum sizedInternalFormat : GetAllSizedInternalFormats())
    {
        const gl::InternalFormat &internalFormat = GetSizedInternalFormatInfo(sizedInternalFormat);

        if (internalFormat.componentType == componentType &&
            internalFormat.colorEncoding == colorEncoding &&
            internalFormat.isChannelSizeCompatible(config->redSize, config->greenSize,
                                                   config->blueSize, config->alphaSize))
        {
            return sizedInternalFormat;
        }
    }

    // Only expect to get here if there is no color bits in the config
    ASSERT(config->redSize == 0 && config->greenSize == 0 && config->blueSize == 0 &&
           config->alphaSize == 0);
    return GL_NONE;
}

GLenum GetConfigDepthStencilBufferFormat(const egl::Config *config)
{
    GLenum componentType = GL_UNSIGNED_NORMALIZED;

    for (GLenum sizedInternalFormat : GetAllSizedInternalFormats())
    {
        const gl::InternalFormat &internalFormat = GetSizedInternalFormatInfo(sizedInternalFormat);

        if (internalFormat.componentType == componentType &&
            static_cast<EGLint>(internalFormat.depthBits) == config->depthSize &&
            static_cast<EGLint>(internalFormat.stencilBits) == config->stencilSize)
        {
            return sizedInternalFormat;
        }
    }

    // Only expect to get here if there is no depth or stencil bits in the config
    ASSERT(config->depthSize == 0 && config->stencilSize == 0);
    return GL_NONE;
}

static FormatSet BuildAllSizedInternalFormatSet()
{
    FormatSet result;

    for (const auto &internalFormat : GetInternalFormatMap())
    {
        for (const auto &type : internalFormat.second)
        {
            if (type.second.sized)
            {
                // TODO(jmadill): Fix this hack.
                if (internalFormat.first == GL_BGR565_ANGLEX)
                    continue;

                result.insert(internalFormat.first);
            }
        }
    }

    return result;
}

uint32_t GetPackedTypeInfo(GLenum type)
{
    switch (type)
    {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
        {
            static constexpr uint32_t kPacked = PackTypeInfo(1, false);
            return kPacked;
        }
        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
        case GL_HALF_FLOAT:
        case GL_HALF_FLOAT_OES:
        {
            static constexpr uint32_t kPacked = PackTypeInfo(2, false);
            return kPacked;
        }
        case GL_UNSIGNED_INT:
        case GL_INT:
        case GL_FLOAT:
        {
            static constexpr uint32_t kPacked = PackTypeInfo(4, false);
            return kPacked;
        }
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT:
        {
            static constexpr uint32_t kPacked = PackTypeInfo(2, true);
            return kPacked;
        }
        case GL_UNSIGNED_INT_2_10_10_10_REV:
        case GL_UNSIGNED_INT_24_8:
        case GL_UNSIGNED_INT_10F_11F_11F_REV:
        case GL_UNSIGNED_INT_5_9_9_9_REV:
        {
            ASSERT(GL_UNSIGNED_INT_24_8_OES == GL_UNSIGNED_INT_24_8);
            static constexpr uint32_t kPacked = PackTypeInfo(4, true);
            return kPacked;
        }
        case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
        {
            static constexpr uint32_t kPacked = PackTypeInfo(8, true);
            return kPacked;
        }
        default:
        {
            return 0;
        }
    }
}

const InternalFormat &GetSizedInternalFormatInfo(GLenum internalFormat)
{
    static const InternalFormat defaultInternalFormat;
    const InternalFormatInfoMap &formatMap = GetInternalFormatMap();
    auto iter                              = formatMap.find(internalFormat);

    // Sized internal formats only have one type per entry
    if (iter == formatMap.end() || iter->second.size() != 1)
    {
        return defaultInternalFormat;
    }

    const InternalFormat &internalFormatInfo = iter->second.begin()->second;
    if (!internalFormatInfo.sized)
    {
        return defaultInternalFormat;
    }

    return internalFormatInfo;
}

const InternalFormat &GetInternalFormatInfo(GLenum internalFormat, GLenum type)
{
    static const InternalFormat defaultInternalFormat;
    const InternalFormatInfoMap &formatMap = GetInternalFormatMap();

    auto internalFormatIter = formatMap.find(internalFormat);
    if (internalFormatIter == formatMap.end())
    {
        return defaultInternalFormat;
    }

    // If the internal format is sized, simply return it without the type check.
    if (internalFormatIter->second.size() == 1 && internalFormatIter->second.begin()->second.sized)
    {
        return internalFormatIter->second.begin()->second;
    }

    auto typeIter = internalFormatIter->second.find(type);
    if (typeIter == internalFormatIter->second.end())
    {
        return defaultInternalFormat;
    }

    return typeIter->second;
}

GLuint InternalFormat::computePixelBytes(GLenum formatType) const
{
    const auto &typeInfo = GetTypeInfo(formatType);
    GLuint components    = componentCount;
    // It shouldn't be these internal formats
    ASSERT(sizedInternalFormat != GL_RGBX8_SRGB_ANGLEX &&
           sizedInternalFormat != GL_BGRX8_SRGB_ANGLEX);
    if (sizedInternalFormat == GL_RGBX8_ANGLE)
    {
        components = 4;
    }
    else if (typeInfo.specialInterpretation)
    {
        components = 1;
    }
    return components * typeInfo.bytes;
}

bool InternalFormat::computeBufferRowLength(uint32_t width, uint32_t *resultOut) const
{
    CheckedNumeric<GLuint> checkedWidth(width);

    if (compressed)
    {
        angle::CheckedNumeric<uint32_t> checkedRowLength =
            rx::CheckedRoundUp<uint32_t>(width, compressedBlockWidth);

        return CheckedMathResult(checkedRowLength, resultOut);
    }

    return CheckedMathResult(checkedWidth, resultOut);
}

bool InternalFormat::computeBufferImageHeight(uint32_t height, uint32_t *resultOut) const
{
    CheckedNumeric<GLuint> checkedHeight(height);

    if (compressed)
    {
        angle::CheckedNumeric<uint32_t> checkedImageHeight =
            rx::CheckedRoundUp<uint32_t>(height, compressedBlockHeight);

        return CheckedMathResult(checkedImageHeight, resultOut);
    }

    return CheckedMathResult(checkedHeight, resultOut);
}

bool InternalFormat::computePalettedImageRowPitch(GLsizei width, GLuint *resultOut) const
{
    ASSERT(paletted);
    switch (paletteBits)
    {
        case 4:
            *resultOut = (width + 1) / 2;
            return true;
        case 8:
            *resultOut = width;
            return true;
        default:
            UNREACHABLE();
            return false;
    }
}

bool InternalFormat::computeRowPitch(GLenum formatType,
                                     GLsizei width,
                                     GLint alignment,
                                     GLint rowLength,
                                     GLuint *resultOut) const
{
    if (paletted)
    {
        return computePalettedImageRowPitch(width, resultOut);
    }

    // Compressed images do not use pack/unpack parameters (rowLength).
    if (compressed)
    {
        return computeCompressedImageRowPitch(width, resultOut);
    }

    CheckedNumeric<GLuint> checkedWidth(rowLength > 0 ? rowLength : width);
    CheckedNumeric<GLuint> checkedRowBytes = checkedWidth * computePixelBytes(formatType);

    ASSERT(alignment > 0 && isPow2(alignment));
    CheckedNumeric<GLuint> checkedAlignment(alignment);
    auto aligned = rx::roundUp(checkedRowBytes, checkedAlignment);
    return CheckedMathResult(aligned, resultOut);
}

bool InternalFormat::computeDepthPitch(GLsizei height,
                                       GLint imageHeight,
                                       GLuint rowPitch,
                                       GLuint *resultOut) const
{
    // Compressed images do not use pack/unpack parameters (imageHeight).
    if (compressed)
    {
        return computeCompressedImageDepthPitch(height, rowPitch, resultOut);
    }

    CheckedNumeric<GLuint> rowCount((imageHeight > 0) ? static_cast<GLuint>(imageHeight)
                                                      : static_cast<GLuint>(height));
    CheckedNumeric<GLuint> checkedRowPitch(rowPitch);

    return CheckedMathResult(checkedRowPitch * rowCount, resultOut);
}

bool InternalFormat::computeDepthPitch(GLenum formatType,
                                       GLsizei width,
                                       GLsizei height,
                                       GLint alignment,
                                       GLint rowLength,
                                       GLint imageHeight,
                                       GLuint *resultOut) const
{
    GLuint rowPitch = 0;
    if (!computeRowPitch(formatType, width, alignment, rowLength, &rowPitch))
    {
        return false;
    }
    return computeDepthPitch(height, imageHeight, rowPitch, resultOut);
}

bool InternalFormat::computeCompressedImageRowPitch(GLsizei width, GLuint *resultOut) const
{
    ASSERT(compressed);

    CheckedNumeric<GLuint> checkedWidth(width);
    CheckedNumeric<GLuint> checkedBlockWidth(compressedBlockWidth);
    const GLuint minBlockWidth = getCompressedImageMinBlocks().first;

    auto numBlocksWide = (checkedWidth + checkedBlockWidth - 1u) / checkedBlockWidth;
    if (numBlocksWide.IsValid() && numBlocksWide.ValueOrDie() < minBlockWidth)
    {
        numBlocksWide = minBlockWidth;
    }
    return CheckedMathResult(numBlocksWide * pixelBytes, resultOut);
}

bool InternalFormat::computeCompressedImageDepthPitch(GLsizei height,
                                                      GLuint rowPitch,
                                                      GLuint *resultOut) const
{
    ASSERT(compressed);
    ASSERT(rowPitch > 0 && rowPitch % pixelBytes == 0);

    CheckedNumeric<GLuint> checkedHeight(height);
    CheckedNumeric<GLuint> checkedRowPitch(rowPitch);
    CheckedNumeric<GLuint> checkedBlockHeight(compressedBlockHeight);
    const GLuint minBlockHeight = getCompressedImageMinBlocks().second;

    auto numBlocksHigh = (checkedHeight + checkedBlockHeight - 1u) / checkedBlockHeight;
    if (numBlocksHigh.IsValid() && numBlocksHigh.ValueOrDie() < minBlockHeight)
    {
        numBlocksHigh = minBlockHeight;
    }
    return CheckedMathResult(numBlocksHigh * checkedRowPitch, resultOut);
}

bool InternalFormat::computeCompressedImageSize(const Extents &size, GLuint *resultOut) const
{
    CheckedNumeric<GLuint> checkedWidth(size.width);
    CheckedNumeric<GLuint> checkedHeight(size.height);
    CheckedNumeric<GLuint> checkedDepth(size.depth);

    if (paletted)
    {
        ASSERT(!compressed);

        GLuint paletteSize  = 1 << paletteBits;
        GLuint paletteBytes = paletteSize * pixelBytes;

        GLuint rowPitch;
        if (!computePalettedImageRowPitch(size.width, &rowPitch))
        {
            return false;
        }

        if (size.depth != 1)
        {
            return false;
        }

        CheckedNumeric<GLuint> checkedPaletteBytes(paletteBytes);
        CheckedNumeric<GLuint> checkedRowPitch(rowPitch);

        return CheckedMathResult(checkedPaletteBytes + checkedRowPitch * checkedHeight, resultOut);
    }

    CheckedNumeric<GLuint> checkedBlockWidth(compressedBlockWidth);
    CheckedNumeric<GLuint> checkedBlockHeight(compressedBlockHeight);
    CheckedNumeric<GLuint> checkedBlockDepth(compressedBlockDepth);
    GLuint minBlockWidth, minBlockHeight;
    std::tie(minBlockWidth, minBlockHeight) = getCompressedImageMinBlocks();

    ASSERT(compressed);
    auto numBlocksWide = (checkedWidth + checkedBlockWidth - 1u) / checkedBlockWidth;
    auto numBlocksHigh = (checkedHeight + checkedBlockHeight - 1u) / checkedBlockHeight;
    auto numBlocksDeep = (checkedDepth + checkedBlockDepth - 1u) / checkedBlockDepth;
    if (numBlocksWide.IsValid() && numBlocksWide.ValueOrDie() < minBlockWidth)
    {
        numBlocksWide = minBlockWidth;
    }
    if (numBlocksHigh.IsValid() && numBlocksHigh.ValueOrDie() < minBlockHeight)
    {
        numBlocksHigh = minBlockHeight;
    }
    auto bytes = numBlocksWide * numBlocksHigh * numBlocksDeep * pixelBytes;
    return CheckedMathResult(bytes, resultOut);
}

std::pair<GLuint, GLuint> InternalFormat::getCompressedImageMinBlocks() const
{
    GLuint minBlockWidth  = 0;
    GLuint minBlockHeight = 0;

    // Per the specification, a PVRTC block needs information from the 3 nearest blocks.
    // GL_IMG_texture_compression_pvrtc specifies the minimum size requirement in pixels, but
    // ANGLE's texture tables are written in terms of blocks. The 4BPP formats use 4x4 blocks, and
    // the 2BPP formats, 8x4 blocks. Therefore, both kinds of formats require a minimum of 2x2
    // blocks.
    if (IsPVRTC1Format(internalFormat))
    {
        minBlockWidth  = 2;
        minBlockHeight = 2;
    }

    return std::make_pair(minBlockWidth, minBlockHeight);
}

bool InternalFormat::computeSkipBytes(GLenum formatType,
                                      GLuint rowPitch,
                                      GLuint depthPitch,
                                      const PixelStoreStateBase &state,
                                      bool is3D,
                                      GLuint *resultOut) const
{
    CheckedNumeric<GLuint> checkedRowPitch(rowPitch);
    CheckedNumeric<GLuint> checkedDepthPitch(depthPitch);
    CheckedNumeric<GLuint> checkedSkipImages(static_cast<GLuint>(state.skipImages));
    CheckedNumeric<GLuint> checkedSkipRows(static_cast<GLuint>(state.skipRows));
    CheckedNumeric<GLuint> checkedSkipPixels(static_cast<GLuint>(state.skipPixels));
    CheckedNumeric<GLuint> checkedPixelBytes(computePixelBytes(formatType));
    auto checkedSkipImagesBytes = checkedSkipImages * checkedDepthPitch;
    if (!is3D)
    {
        checkedSkipImagesBytes = 0;
    }
    auto skipBytes = checkedSkipImagesBytes + checkedSkipRows * checkedRowPitch +
                     checkedSkipPixels * checkedPixelBytes;
    return CheckedMathResult(skipBytes, resultOut);
}

bool InternalFormat::computePackUnpackEndByte(GLenum formatType,
                                              const Extents &size,
                                              const PixelStoreStateBase &state,
                                              bool is3D,
                                              GLuint *resultOut) const
{
    GLuint rowPitch = 0;
    if (!computeRowPitch(formatType, size.width, state.alignment, state.rowLength, &rowPitch))
    {
        return false;
    }

    GLuint depthPitch = 0;
    if (is3D && !computeDepthPitch(size.height, state.imageHeight, rowPitch, &depthPitch))
    {
        return false;
    }

    CheckedNumeric<GLuint> checkedCopyBytes(0);
    if (compressed)
    {
        GLuint copyBytes = 0;
        if (!computeCompressedImageSize(size, &copyBytes))
        {
            return false;
        }
        checkedCopyBytes = copyBytes;
    }
    else if (size.height != 0 && (!is3D || size.depth != 0))
    {
        CheckedNumeric<GLuint> bytes = computePixelBytes(formatType);
        checkedCopyBytes += size.width * bytes;

        CheckedNumeric<GLuint> heightMinusOne = size.height - 1;
        checkedCopyBytes += heightMinusOne * rowPitch;

        if (is3D)
        {
            CheckedNumeric<GLuint> depthMinusOne = size.depth - 1;
            checkedCopyBytes += depthMinusOne * depthPitch;
        }
    }

    GLuint skipBytes = 0;
    if (!computeSkipBytes(formatType, rowPitch, depthPitch, state, is3D, &skipBytes))
    {
        return false;
    }

    CheckedNumeric<GLuint> endByte = checkedCopyBytes + CheckedNumeric<GLuint>(skipBytes);

    return CheckedMathResult(endByte, resultOut);
}

GLenum GetUnsizedFormat(GLenum internalFormat)
{
    auto sizedFormatInfo = GetSizedInternalFormatInfo(internalFormat);
    if (sizedFormatInfo.internalFormat != GL_NONE)
    {
        return sizedFormatInfo.format;
    }

    return internalFormat;
}

bool CompressedFormatRequiresWholeImage(GLenum internalFormat)
{
    // List of compressed texture format that require that the sub-image size is equal to texture's
    // respective mip level's size
    return IsPVRTC1Format(internalFormat);
}

void MaybeOverrideLuminance(GLenum &format, GLenum &type, GLenum actualFormat, GLenum actualType)
{
    gl::InternalFormat internalFormat = gl::GetInternalFormatInfo(format, type);
    if (internalFormat.isLUMA())
    {
        // Ensure the format and type are compatible
        ASSERT(internalFormat.pixelBytes ==
               gl::GetInternalFormatInfo(actualFormat, actualType).pixelBytes);

        // For Luminance formats, override with the internal format. Since this is not
        // renderable, our pixel pack routines don't handle it correctly.
        format = actualFormat;
        type   = actualType;
    }
}

const FormatSet &GetAllSizedInternalFormats()
{
    static angle::base::NoDestructor<FormatSet> formatSet(BuildAllSizedInternalFormatSet());
    return *formatSet;
}

AttributeType GetAttributeType(GLenum enumValue)
{
    switch (enumValue)
    {
        case GL_FLOAT:
            return ATTRIBUTE_FLOAT;
        case GL_FLOAT_VEC2:
            return ATTRIBUTE_VEC2;
        case GL_FLOAT_VEC3:
            return ATTRIBUTE_VEC3;
        case GL_FLOAT_VEC4:
            return ATTRIBUTE_VEC4;
        case GL_INT:
            return ATTRIBUTE_INT;
        case GL_INT_VEC2:
            return ATTRIBUTE_IVEC2;
        case GL_INT_VEC3:
            return ATTRIBUTE_IVEC3;
        case GL_INT_VEC4:
            return ATTRIBUTE_IVEC4;
        case GL_UNSIGNED_INT:
            return ATTRIBUTE_UINT;
        case GL_UNSIGNED_INT_VEC2:
            return ATTRIBUTE_UVEC2;
        case GL_UNSIGNED_INT_VEC3:
            return ATTRIBUTE_UVEC3;
        case GL_UNSIGNED_INT_VEC4:
            return ATTRIBUTE_UVEC4;
        case GL_FLOAT_MAT2:
            return ATTRIBUTE_MAT2;
        case GL_FLOAT_MAT3:
            return ATTRIBUTE_MAT3;
        case GL_FLOAT_MAT4:
            return ATTRIBUTE_MAT4;
        case GL_FLOAT_MAT2x3:
            return ATTRIBUTE_MAT2x3;
        case GL_FLOAT_MAT2x4:
            return ATTRIBUTE_MAT2x4;
        case GL_FLOAT_MAT3x2:
            return ATTRIBUTE_MAT3x2;
        case GL_FLOAT_MAT3x4:
            return ATTRIBUTE_MAT3x4;
        case GL_FLOAT_MAT4x2:
            return ATTRIBUTE_MAT4x2;
        case GL_FLOAT_MAT4x3:
            return ATTRIBUTE_MAT4x3;
        default:
            UNREACHABLE();
            return ATTRIBUTE_FLOAT;
    }
}

// kVertexFormat* tables below rely on these values
static_assert(kVertexFormatCount == 18);
static_assert(static_cast<uint32_t>(VertexAttribType::Byte) == 0);
static_assert(static_cast<uint32_t>(VertexAttribType::UnsignedByte) == 1);
static_assert(static_cast<uint32_t>(VertexAttribType::Short) == 2);
static_assert(static_cast<uint32_t>(VertexAttribType::UnsignedShort) == 3);
static_assert(static_cast<uint32_t>(VertexAttribType::Int) == 4);
static_assert(static_cast<uint32_t>(VertexAttribType::UnsignedInt) == 5);
static_assert(static_cast<uint32_t>(VertexAttribType::Float) == 6);
static_assert(static_cast<uint32_t>(VertexAttribType::Unused1) == 7);
static_assert(static_cast<uint32_t>(VertexAttribType::Unused2) == 8);
static_assert(static_cast<uint32_t>(VertexAttribType::Unused3) == 9);
static_assert(static_cast<uint32_t>(VertexAttribType::Unused4) == 10);
static_assert(static_cast<uint32_t>(VertexAttribType::HalfFloat) == 11);
static_assert(static_cast<uint32_t>(VertexAttribType::Fixed) == 12);
static_assert(static_cast<uint32_t>(VertexAttribType::UnsignedInt2101010) == 13);
static_assert(static_cast<uint32_t>(VertexAttribType::HalfFloatOES) == 14);
static_assert(static_cast<uint32_t>(VertexAttribType::Int2101010) == 15);
static_assert(static_cast<uint32_t>(VertexAttribType::UnsignedInt1010102) == 16);
static_assert(static_cast<uint32_t>(VertexAttribType::Int1010102) == 17);

const angle::FormatID kVertexFormatPureInteger[kVertexFormatCount][4] = {
    // Byte
    {angle::FormatID::R8_SINT, angle::FormatID::R8G8_SINT, angle::FormatID::R8G8B8_SINT,
     angle::FormatID::R8G8B8A8_SINT},
    // UnsignedByte
    {angle::FormatID::R8_UINT, angle::FormatID::R8G8_UINT, angle::FormatID::R8G8B8_UINT,
     angle::FormatID::R8G8B8A8_UINT},
    // Short
    {angle::FormatID::R16_SINT, angle::FormatID::R16G16_SINT, angle::FormatID::R16G16B16_SINT,
     angle::FormatID::R16G16B16A16_SINT},
    // UnsignedShort
    {angle::FormatID::R16_UINT, angle::FormatID::R16G16_UINT, angle::FormatID::R16G16B16_UINT,
     angle::FormatID::R16G16B16A16_UINT},
    // Int
    {angle::FormatID::R32_SINT, angle::FormatID::R32G32_SINT, angle::FormatID::R32G32B32_SINT,
     angle::FormatID::R32G32B32A32_SINT},
    // UnsignedInt
    {angle::FormatID::R32_UINT, angle::FormatID::R32G32_UINT, angle::FormatID::R32G32B32_UINT,
     angle::FormatID::R32G32B32A32_UINT},
    // Float
    {angle::FormatID::R32_FLOAT, angle::FormatID::R32G32_FLOAT, angle::FormatID::R32G32B32_FLOAT,
     angle::FormatID::R32G32B32A32_FLOAT},
    // Unused1
    {angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE},
    // Unused2
    {angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE},
    // Unused3
    {angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE},
    // Unused4
    {angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE},
    // HalfFloat
    {angle::FormatID::R16_FLOAT, angle::FormatID::R16G16_FLOAT, angle::FormatID::R16G16B16_FLOAT,
     angle::FormatID::R16G16B16A16_FLOAT},
    // Fixed
    {angle::FormatID::R32_FIXED, angle::FormatID::R32G32_FIXED, angle::FormatID::R32G32B32_FIXED,
     angle::FormatID::R32G32B32A32_FIXED},
    // UnsignedInt2101010
    {angle::FormatID::R10G10B10A2_UINT, angle::FormatID::R10G10B10A2_UINT,
     angle::FormatID::R10G10B10A2_UINT, angle::FormatID::R10G10B10A2_UINT},
    // HalfFloatOES
    {angle::FormatID::R16_FLOAT, angle::FormatID::R16G16_FLOAT, angle::FormatID::R16G16B16_FLOAT,
     angle::FormatID::R16G16B16A16_FLOAT},
    // Int2101010
    {angle::FormatID::R10G10B10A2_SINT, angle::FormatID::R10G10B10A2_SINT,
     angle::FormatID::R10G10B10A2_SINT, angle::FormatID::R10G10B10A2_SINT},
    // UnsignedInt1010102
    {angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::X2R10G10B10_UINT_VERTEX,
     angle::FormatID::A2R10G10B10_UINT_VERTEX},
    // Int1010102
    {angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::X2R10G10B10_SINT_VERTEX,
     angle::FormatID::A2R10G10B10_SINT_VERTEX},
};

const angle::FormatID kVertexFormatNormalized[kVertexFormatCount][4] = {
    // Byte
    {angle::FormatID::R8_SNORM, angle::FormatID::R8G8_SNORM, angle::FormatID::R8G8B8_SNORM,
     angle::FormatID::R8G8B8A8_SNORM},
    // UnsignedByte
    {angle::FormatID::R8_UNORM, angle::FormatID::R8G8_UNORM, angle::FormatID::R8G8B8_UNORM,
     angle::FormatID::R8G8B8A8_UNORM},
    // Short
    {angle::FormatID::R16_SNORM, angle::FormatID::R16G16_SNORM, angle::FormatID::R16G16B16_SNORM,
     angle::FormatID::R16G16B16A16_SNORM},
    // UnsignedShort
    {angle::FormatID::R16_UNORM, angle::FormatID::R16G16_UNORM, angle::FormatID::R16G16B16_UNORM,
     angle::FormatID::R16G16B16A16_UNORM},
    // Int
    {angle::FormatID::R32_SNORM, angle::FormatID::R32G32_SNORM, angle::FormatID::R32G32B32_SNORM,
     angle::FormatID::R32G32B32A32_SNORM},
    // UnsignedInt
    {angle::FormatID::R32_UNORM, angle::FormatID::R32G32_UNORM, angle::FormatID::R32G32B32_UNORM,
     angle::FormatID::R32G32B32A32_UNORM},
    // Float
    {angle::FormatID::R32_FLOAT, angle::FormatID::R32G32_FLOAT, angle::FormatID::R32G32B32_FLOAT,
     angle::FormatID::R32G32B32A32_FLOAT},
    // Unused1
    {angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE},
    // Unused2
    {angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE},
    // Unused3
    {angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE},
    // Unused4
    {angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE},
    // HalfFloat
    {angle::FormatID::R16_FLOAT, angle::FormatID::R16G16_FLOAT, angle::FormatID::R16G16B16_FLOAT,
     angle::FormatID::R16G16B16A16_FLOAT},
    // Fixed
    {angle::FormatID::R32_FIXED, angle::FormatID::R32G32_FIXED, angle::FormatID::R32G32B32_FIXED,
     angle::FormatID::R32G32B32A32_FIXED},
    // UnsignedInt2101010
    {angle::FormatID::R10G10B10A2_UNORM, angle::FormatID::R10G10B10A2_UNORM,
     angle::FormatID::R10G10B10A2_UNORM, angle::FormatID::R10G10B10A2_UNORM},
    // HalfFloatOES
    {angle::FormatID::R16_FLOAT, angle::FormatID::R16G16_FLOAT, angle::FormatID::R16G16B16_FLOAT,
     angle::FormatID::R16G16B16A16_FLOAT},
    // Int2101010
    {angle::FormatID::R10G10B10A2_SNORM, angle::FormatID::R10G10B10A2_SNORM,
     angle::FormatID::R10G10B10A2_SNORM, angle::FormatID::R10G10B10A2_SNORM},
    // UnsignedInt1010102
    {angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::X2R10G10B10_UNORM_VERTEX,
     angle::FormatID::A2R10G10B10_UNORM_VERTEX},
    // Int1010102
    {angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::X2R10G10B10_SNORM_VERTEX,
     angle::FormatID::A2R10G10B10_SNORM_VERTEX},
};

const angle::FormatID kVertexFormatScaled[kVertexFormatCount][4] = {
    // Byte
    {angle::FormatID::R8_SSCALED, angle::FormatID::R8G8_SSCALED, angle::FormatID::R8G8B8_SSCALED,
     angle::FormatID::R8G8B8A8_SSCALED},
    // UnsignedByte
    {angle::FormatID::R8_USCALED, angle::FormatID::R8G8_USCALED, angle::FormatID::R8G8B8_USCALED,
     angle::FormatID::R8G8B8A8_USCALED},
    // Short
    {angle::FormatID::R16_SSCALED, angle::FormatID::R16G16_SSCALED,
     angle::FormatID::R16G16B16_SSCALED, angle::FormatID::R16G16B16A16_SSCALED},
    // UnsignedShort
    {angle::FormatID::R16_USCALED, angle::FormatID::R16G16_USCALED,
     angle::FormatID::R16G16B16_USCALED, angle::FormatID::R16G16B16A16_USCALED},
    // Int
    {angle::FormatID::R32_SSCALED, angle::FormatID::R32G32_SSCALED,
     angle::FormatID::R32G32B32_SSCALED, angle::FormatID::R32G32B32A32_SSCALED},
    // UnsignedInt
    {angle::FormatID::R32_USCALED, angle::FormatID::R32G32_USCALED,
     angle::FormatID::R32G32B32_USCALED, angle::FormatID::R32G32B32A32_USCALED},
    // Float
    {angle::FormatID::R32_FLOAT, angle::FormatID::R32G32_FLOAT, angle::FormatID::R32G32B32_FLOAT,
     angle::FormatID::R32G32B32A32_FLOAT},
    // Unused1
    {angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE},
    // Unused2
    {angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE},
    // Unused3
    {angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE},
    // Unused4
    {angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::NONE},
    // HalfFloat
    {angle::FormatID::R16_FLOAT, angle::FormatID::R16G16_FLOAT, angle::FormatID::R16G16B16_FLOAT,
     angle::FormatID::R16G16B16A16_FLOAT},
    // Fixed
    {angle::FormatID::R32_FIXED, angle::FormatID::R32G32_FIXED, angle::FormatID::R32G32B32_FIXED,
     angle::FormatID::R32G32B32A32_FIXED},
    // UnsignedInt2101010
    {angle::FormatID::R10G10B10A2_USCALED, angle::FormatID::R10G10B10A2_USCALED,
     angle::FormatID::R10G10B10A2_USCALED, angle::FormatID::R10G10B10A2_USCALED},
    // HalfFloatOES
    {angle::FormatID::R16_FLOAT, angle::FormatID::R16G16_FLOAT, angle::FormatID::R16G16B16_FLOAT,
     angle::FormatID::R16G16B16A16_FLOAT},
    // Int2101010
    {angle::FormatID::R10G10B10A2_SSCALED, angle::FormatID::R10G10B10A2_SSCALED,
     angle::FormatID::R10G10B10A2_SSCALED, angle::FormatID::R10G10B10A2_SSCALED},
    // UnsignedInt1010102
    {angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::X2R10G10B10_USCALED_VERTEX,
     angle::FormatID::A2R10G10B10_USCALED_VERTEX},
    // Int1010102
    {angle::FormatID::NONE, angle::FormatID::NONE, angle::FormatID::X2R10G10B10_SSCALED_VERTEX,
     angle::FormatID::A2R10G10B10_SSCALED_VERTEX},
};

angle::FormatID GetVertexFormatID(const VertexAttribute &attrib, VertexAttribType currentValueType)
{
    if (!attrib.enabled)
    {
        return GetCurrentValueFormatID(currentValueType);
    }
    return attrib.format->id;
}

angle::FormatID GetCurrentValueFormatID(VertexAttribType currentValueType)
{
    switch (currentValueType)
    {
        case VertexAttribType::Float:
            return angle::FormatID::R32G32B32A32_FLOAT;
        case VertexAttribType::Int:
            return angle::FormatID::R32G32B32A32_SINT;
        case VertexAttribType::UnsignedInt:
            return angle::FormatID::R32G32B32A32_UINT;
        default:
            UNREACHABLE();
            return angle::FormatID::NONE;
    }
}

const VertexFormat &GetVertexFormatFromID(angle::FormatID vertexFormatID)
{
    switch (vertexFormatID)
    {
        case angle::FormatID::R8_SSCALED:
        {
            static const VertexFormat format(GL_BYTE, GL_FALSE, 1, false);
            return format;
        }
        case angle::FormatID::R8_SNORM:
        {
            static const VertexFormat format(GL_BYTE, GL_TRUE, 1, false);
            return format;
        }
        case angle::FormatID::R8G8_SSCALED:
        {
            static const VertexFormat format(GL_BYTE, GL_FALSE, 2, false);
            return format;
        }
        case angle::FormatID::R8G8_SNORM:
        {
            static const VertexFormat format(GL_BYTE, GL_TRUE, 2, false);
            return format;
        }
        case angle::FormatID::R8G8B8_SSCALED:
        {
            static const VertexFormat format(GL_BYTE, GL_FALSE, 3, false);
            return format;
        }
        case angle::FormatID::R8G8B8_SNORM:
        {
            static const VertexFormat format(GL_BYTE, GL_TRUE, 3, false);
            return format;
        }
        case angle::FormatID::R8G8B8A8_SSCALED:
        {
            static const VertexFormat format(GL_BYTE, GL_FALSE, 4, false);
            return format;
        }
        case angle::FormatID::R8G8B8A8_SNORM:
        {
            static const VertexFormat format(GL_BYTE, GL_TRUE, 4, false);
            return format;
        }
        case angle::FormatID::R8_USCALED:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_FALSE, 1, false);
            return format;
        }
        case angle::FormatID::R8_UNORM:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_TRUE, 1, false);
            return format;
        }
        case angle::FormatID::R8G8_USCALED:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_FALSE, 2, false);
            return format;
        }
        case angle::FormatID::R8G8_UNORM:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_TRUE, 2, false);
            return format;
        }
        case angle::FormatID::R8G8B8_USCALED:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_FALSE, 3, false);
            return format;
        }
        case angle::FormatID::R8G8B8_UNORM:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_TRUE, 3, false);
            return format;
        }
        case angle::FormatID::R8G8B8A8_USCALED:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_FALSE, 4, false);
            return format;
        }
        case angle::FormatID::R8G8B8A8_UNORM:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_TRUE, 4, false);
            return format;
        }
        case angle::FormatID::R16_SSCALED:
        {
            static const VertexFormat format(GL_SHORT, GL_FALSE, 1, false);
            return format;
        }
        case angle::FormatID::R16_SNORM:
        {
            static const VertexFormat format(GL_SHORT, GL_TRUE, 1, false);
            return format;
        }
        case angle::FormatID::R16G16_SSCALED:
        {
            static const VertexFormat format(GL_SHORT, GL_FALSE, 2, false);
            return format;
        }
        case angle::FormatID::R16G16_SNORM:
        {
            static const VertexFormat format(GL_SHORT, GL_TRUE, 2, false);
            return format;
        }
        case angle::FormatID::R16G16B16_SSCALED:
        {
            static const VertexFormat format(GL_SHORT, GL_FALSE, 3, false);
            return format;
        }
        case angle::FormatID::R16G16B16_SNORM:
        {
            static const VertexFormat format(GL_SHORT, GL_TRUE, 3, false);
            return format;
        }
        case angle::FormatID::R16G16B16A16_SSCALED:
        {
            static const VertexFormat format(GL_SHORT, GL_FALSE, 4, false);
            return format;
        }
        case angle::FormatID::R16G16B16A16_SNORM:
        {
            static const VertexFormat format(GL_SHORT, GL_TRUE, 4, false);
            return format;
        }
        case angle::FormatID::R16_USCALED:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_FALSE, 1, false);
            return format;
        }
        case angle::FormatID::R16_UNORM:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_TRUE, 1, false);
            return format;
        }
        case angle::FormatID::R16G16_USCALED:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_FALSE, 2, false);
            return format;
        }
        case angle::FormatID::R16G16_UNORM:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_TRUE, 2, false);
            return format;
        }
        case angle::FormatID::R16G16B16_USCALED:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_FALSE, 3, false);
            return format;
        }
        case angle::FormatID::R16G16B16_UNORM:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_TRUE, 3, false);
            return format;
        }
        case angle::FormatID::R16G16B16A16_USCALED:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_FALSE, 4, false);
            return format;
        }
        case angle::FormatID::R16G16B16A16_UNORM:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_TRUE, 4, false);
            return format;
        }
        case angle::FormatID::R32_SSCALED:
        {
            static const VertexFormat format(GL_INT, GL_FALSE, 1, false);
            return format;
        }
        case angle::FormatID::R32_SNORM:
        {
            static const VertexFormat format(GL_INT, GL_TRUE, 1, false);
            return format;
        }
        case angle::FormatID::R32G32_SSCALED:
        {
            static const VertexFormat format(GL_INT, GL_FALSE, 2, false);
            return format;
        }
        case angle::FormatID::R32G32_SNORM:
        {
            static const VertexFormat format(GL_INT, GL_TRUE, 2, false);
            return format;
        }
        case angle::FormatID::R32G32B32_SSCALED:
        {
            static const VertexFormat format(GL_INT, GL_FALSE, 3, false);
            return format;
        }
        case angle::FormatID::R32G32B32_SNORM:
        {
            static const VertexFormat format(GL_INT, GL_TRUE, 3, false);
            return format;
        }
        case angle::FormatID::R32G32B32A32_SSCALED:
        {
            static const VertexFormat format(GL_INT, GL_FALSE, 4, false);
            return format;
        }
        case angle::FormatID::R32G32B32A32_SNORM:
        {
            static const VertexFormat format(GL_INT, GL_TRUE, 4, false);
            return format;
        }
        case angle::FormatID::R32_USCALED:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_FALSE, 1, false);
            return format;
        }
        case angle::FormatID::R32_UNORM:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_TRUE, 1, false);
            return format;
        }
        case angle::FormatID::R32G32_USCALED:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_FALSE, 2, false);
            return format;
        }
        case angle::FormatID::R32G32_UNORM:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_TRUE, 2, false);
            return format;
        }
        case angle::FormatID::R32G32B32_USCALED:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_FALSE, 3, false);
            return format;
        }
        case angle::FormatID::R32G32B32_UNORM:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_TRUE, 3, false);
            return format;
        }
        case angle::FormatID::R32G32B32A32_USCALED:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_FALSE, 4, false);
            return format;
        }
        case angle::FormatID::R32G32B32A32_UNORM:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_TRUE, 4, false);
            return format;
        }
        case angle::FormatID::R8_SINT:
        {
            static const VertexFormat format(GL_BYTE, GL_FALSE, 1, true);
            return format;
        }
        case angle::FormatID::R8G8_SINT:
        {
            static const VertexFormat format(GL_BYTE, GL_FALSE, 2, true);
            return format;
        }
        case angle::FormatID::R8G8B8_SINT:
        {
            static const VertexFormat format(GL_BYTE, GL_FALSE, 3, true);
            return format;
        }
        case angle::FormatID::R8G8B8A8_SINT:
        {
            static const VertexFormat format(GL_BYTE, GL_FALSE, 4, true);
            return format;
        }
        case angle::FormatID::R8_UINT:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_FALSE, 1, true);
            return format;
        }
        case angle::FormatID::R8G8_UINT:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_FALSE, 2, true);
            return format;
        }
        case angle::FormatID::R8G8B8_UINT:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_FALSE, 3, true);
            return format;
        }
        case angle::FormatID::R8G8B8A8_UINT:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_FALSE, 4, true);
            return format;
        }
        case angle::FormatID::R16_SINT:
        {
            static const VertexFormat format(GL_SHORT, GL_FALSE, 1, true);
            return format;
        }
        case angle::FormatID::R16G16_SINT:
        {
            static const VertexFormat format(GL_SHORT, GL_FALSE, 2, true);
            return format;
        }
        case angle::FormatID::R16G16B16_SINT:
        {
            static const VertexFormat format(GL_SHORT, GL_FALSE, 3, true);
            return format;
        }
        case angle::FormatID::R16G16B16A16_SINT:
        {
            static const VertexFormat format(GL_SHORT, GL_FALSE, 4, true);
            return format;
        }
        case angle::FormatID::R16_UINT:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_FALSE, 1, true);
            return format;
        }
        case angle::FormatID::R16G16_UINT:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_FALSE, 2, true);
            return format;
        }
        case angle::FormatID::R16G16B16_UINT:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_FALSE, 3, true);
            return format;
        }
        case angle::FormatID::R16G16B16A16_UINT:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_FALSE, 4, true);
            return format;
        }
        case angle::FormatID::R32_SINT:
        {
            static const VertexFormat format(GL_INT, GL_FALSE, 1, true);
            return format;
        }
        case angle::FormatID::R32G32_SINT:
        {
            static const VertexFormat format(GL_INT, GL_FALSE, 2, true);
            return format;
        }
        case angle::FormatID::R32G32B32_SINT:
        {
            static const VertexFormat format(GL_INT, GL_FALSE, 3, true);
            return format;
        }
        case angle::FormatID::R32G32B32A32_SINT:
        {
            static const VertexFormat format(GL_INT, GL_FALSE, 4, true);
            return format;
        }
        case angle::FormatID::R32_UINT:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_FALSE, 1, true);
            return format;
        }
        case angle::FormatID::R32G32_UINT:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_FALSE, 2, true);
            return format;
        }
        case angle::FormatID::R32G32B32_UINT:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_FALSE, 3, true);
            return format;
        }
        case angle::FormatID::R32G32B32A32_UINT:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_FALSE, 4, true);
            return format;
        }
        case angle::FormatID::R32_FIXED:
        {
            static const VertexFormat format(GL_FIXED, GL_FALSE, 1, false);
            return format;
        }
        case angle::FormatID::R32G32_FIXED:
        {
            static const VertexFormat format(GL_FIXED, GL_FALSE, 2, false);
            return format;
        }
        case angle::FormatID::R32G32B32_FIXED:
        {
            static const VertexFormat format(GL_FIXED, GL_FALSE, 3, false);
            return format;
        }
        case angle::FormatID::R32G32B32A32_FIXED:
        {
            static const VertexFormat format(GL_FIXED, GL_FALSE, 4, false);
            return format;
        }
        case angle::FormatID::R16_FLOAT:
        {
            static const VertexFormat format(GL_HALF_FLOAT, GL_FALSE, 1, false);
            return format;
        }
        case angle::FormatID::R16G16_FLOAT:
        {
            static const VertexFormat format(GL_HALF_FLOAT, GL_FALSE, 2, false);
            return format;
        }
        case angle::FormatID::R16G16B16_FLOAT:
        {
            static const VertexFormat format(GL_HALF_FLOAT, GL_FALSE, 3, false);
            return format;
        }
        case angle::FormatID::R16G16B16A16_FLOAT:
        {
            static const VertexFormat format(GL_HALF_FLOAT, GL_FALSE, 4, false);
            return format;
        }
        case angle::FormatID::R32_FLOAT:
        {
            static const VertexFormat format(GL_FLOAT, GL_FALSE, 1, false);
            return format;
        }
        case angle::FormatID::R32G32_FLOAT:
        {
            static const VertexFormat format(GL_FLOAT, GL_FALSE, 2, false);
            return format;
        }
        case angle::FormatID::R32G32B32_FLOAT:
        {
            static const VertexFormat format(GL_FLOAT, GL_FALSE, 3, false);
            return format;
        }
        case angle::FormatID::R32G32B32A32_FLOAT:
        {
            static const VertexFormat format(GL_FLOAT, GL_FALSE, 4, false);
            return format;
        }
        case angle::FormatID::R10G10B10A2_SSCALED:
        {
            static const VertexFormat format(GL_INT_2_10_10_10_REV, GL_FALSE, 4, false);
            return format;
        }
        case angle::FormatID::R10G10B10A2_USCALED:
        {
            static const VertexFormat format(GL_UNSIGNED_INT_2_10_10_10_REV, GL_FALSE, 4, false);
            return format;
        }
        case angle::FormatID::R10G10B10A2_SNORM:
        {
            static const VertexFormat format(GL_INT_2_10_10_10_REV, GL_TRUE, 4, false);
            return format;
        }
        case angle::FormatID::R10G10B10A2_UNORM:
        {
            static const VertexFormat format(GL_UNSIGNED_INT_2_10_10_10_REV, GL_TRUE, 4, false);
            return format;
        }
        case angle::FormatID::R10G10B10A2_SINT:
        {
            static const VertexFormat format(GL_INT_2_10_10_10_REV, GL_FALSE, 4, true);
            return format;
        }
        case angle::FormatID::R10G10B10A2_UINT:
        {
            static const VertexFormat format(GL_UNSIGNED_INT_2_10_10_10_REV, GL_FALSE, 4, true);
            return format;
        }
        case angle::FormatID::A2R10G10B10_SSCALED_VERTEX:
        {
            static const VertexFormat format(GL_INT_10_10_10_2_OES, GL_FALSE, 4, false);
            return format;
        }
        case angle::FormatID::A2R10G10B10_USCALED_VERTEX:
        {
            static const VertexFormat format(GL_UNSIGNED_INT_10_10_10_2_OES, GL_FALSE, 4, false);
            return format;
        }
        case angle::FormatID::A2R10G10B10_SNORM_VERTEX:
        {
            static const VertexFormat format(GL_INT_10_10_10_2_OES, GL_TRUE, 4, false);
            return format;
        }
        case angle::FormatID::A2R10G10B10_UNORM_VERTEX:
        {
            static const VertexFormat format(GL_UNSIGNED_INT_10_10_10_2_OES, GL_TRUE, 4, false);
            return format;
        }
        case angle::FormatID::A2R10G10B10_SINT_VERTEX:
        {
            static const VertexFormat format(GL_INT_10_10_10_2_OES, GL_FALSE, 4, true);
            return format;
        }
        case angle::FormatID::A2R10G10B10_UINT_VERTEX:
        {
            static const VertexFormat format(GL_UNSIGNED_INT_10_10_10_2_OES, GL_FALSE, 4, true);
            return format;
        }
        case angle::FormatID::X2R10G10B10_SSCALED_VERTEX:
        {
            static const VertexFormat format(GL_INT_10_10_10_2_OES, GL_FALSE, 4, false);
            return format;
        }
        case angle::FormatID::X2R10G10B10_USCALED_VERTEX:
        {
            static const VertexFormat format(GL_UNSIGNED_INT_10_10_10_2_OES, GL_FALSE, 4, false);
            return format;
        }
        case angle::FormatID::X2R10G10B10_SNORM_VERTEX:
        {
            static const VertexFormat format(GL_INT_10_10_10_2_OES, GL_TRUE, 4, false);
            return format;
        }
        case angle::FormatID::X2R10G10B10_UNORM_VERTEX:
        {
            static const VertexFormat format(GL_UNSIGNED_INT_10_10_10_2_OES, GL_TRUE, 4, false);
            return format;
        }
        case angle::FormatID::X2R10G10B10_SINT_VERTEX:
        {
            static const VertexFormat format(GL_INT_10_10_10_2_OES, GL_FALSE, 4, true);
            return format;
        }
        default:
        {
            static const VertexFormat format(GL_NONE, GL_FALSE, 0, false);
            return format;
        }
    }
}

size_t GetVertexFormatSize(angle::FormatID vertexFormatID)
{
    switch (vertexFormatID)
    {
        case angle::FormatID::R8_SSCALED:
        case angle::FormatID::R8_SNORM:
        case angle::FormatID::R8_USCALED:
        case angle::FormatID::R8_UNORM:
        case angle::FormatID::R8_SINT:
        case angle::FormatID::R8_UINT:
            return 1;

        case angle::FormatID::R8G8_SSCALED:
        case angle::FormatID::R8G8_SNORM:
        case angle::FormatID::R8G8_USCALED:
        case angle::FormatID::R8G8_UNORM:
        case angle::FormatID::R8G8_SINT:
        case angle::FormatID::R8G8_UINT:
        case angle::FormatID::R16_SSCALED:
        case angle::FormatID::R16_SNORM:
        case angle::FormatID::R16_USCALED:
        case angle::FormatID::R16_UNORM:
        case angle::FormatID::R16_SINT:
        case angle::FormatID::R16_UINT:
        case angle::FormatID::R16_FLOAT:
            return 2;

        case angle::FormatID::R8G8B8_SSCALED:
        case angle::FormatID::R8G8B8_SNORM:
        case angle::FormatID::R8G8B8_USCALED:
        case angle::FormatID::R8G8B8_UNORM:
        case angle::FormatID::R8G8B8_SINT:
        case angle::FormatID::R8G8B8_UINT:
            return 3;

        case angle::FormatID::R8G8B8A8_SSCALED:
        case angle::FormatID::R8G8B8A8_SNORM:
        case angle::FormatID::R8G8B8A8_USCALED:
        case angle::FormatID::R8G8B8A8_UNORM:
        case angle::FormatID::R8G8B8A8_SINT:
        case angle::FormatID::R8G8B8A8_UINT:
        case angle::FormatID::R16G16_SSCALED:
        case angle::FormatID::R16G16_SNORM:
        case angle::FormatID::R16G16_USCALED:
        case angle::FormatID::R16G16_UNORM:
        case angle::FormatID::R16G16_SINT:
        case angle::FormatID::R16G16_UINT:
        case angle::FormatID::R32_SSCALED:
        case angle::FormatID::R32_SNORM:
        case angle::FormatID::R32_USCALED:
        case angle::FormatID::R32_UNORM:
        case angle::FormatID::R32_SINT:
        case angle::FormatID::R32_UINT:
        case angle::FormatID::R16G16_FLOAT:
        case angle::FormatID::R32_FIXED:
        case angle::FormatID::R32_FLOAT:
        case angle::FormatID::R10G10B10A2_SSCALED:
        case angle::FormatID::R10G10B10A2_USCALED:
        case angle::FormatID::R10G10B10A2_SNORM:
        case angle::FormatID::R10G10B10A2_UNORM:
        case angle::FormatID::R10G10B10A2_SINT:
        case angle::FormatID::R10G10B10A2_UINT:
        case angle::FormatID::A2R10G10B10_SSCALED_VERTEX:
        case angle::FormatID::A2R10G10B10_USCALED_VERTEX:
        case angle::FormatID::A2R10G10B10_SINT_VERTEX:
        case angle::FormatID::A2R10G10B10_UINT_VERTEX:
        case angle::FormatID::A2R10G10B10_SNORM_VERTEX:
        case angle::FormatID::A2R10G10B10_UNORM_VERTEX:
        case angle::FormatID::X2R10G10B10_SSCALED_VERTEX:
        case angle::FormatID::X2R10G10B10_USCALED_VERTEX:
        case angle::FormatID::X2R10G10B10_SINT_VERTEX:
        case angle::FormatID::X2R10G10B10_UINT_VERTEX:
        case angle::FormatID::X2R10G10B10_SNORM_VERTEX:
        case angle::FormatID::X2R10G10B10_UNORM_VERTEX:
            return 4;

        case angle::FormatID::R16G16B16_SSCALED:
        case angle::FormatID::R16G16B16_SNORM:
        case angle::FormatID::R16G16B16_USCALED:
        case angle::FormatID::R16G16B16_UNORM:
        case angle::FormatID::R16G16B16_SINT:
        case angle::FormatID::R16G16B16_UINT:
        case angle::FormatID::R16G16B16_FLOAT:
            return 6;

        case angle::FormatID::R16G16B16A16_SSCALED:
        case angle::FormatID::R16G16B16A16_SNORM:
        case angle::FormatID::R16G16B16A16_USCALED:
        case angle::FormatID::R16G16B16A16_UNORM:
        case angle::FormatID::R16G16B16A16_SINT:
        case angle::FormatID::R16G16B16A16_UINT:
        case angle::FormatID::R32G32_SSCALED:
        case angle::FormatID::R32G32_SNORM:
        case angle::FormatID::R32G32_USCALED:
        case angle::FormatID::R32G32_UNORM:
        case angle::FormatID::R32G32_SINT:
        case angle::FormatID::R32G32_UINT:
        case angle::FormatID::R16G16B16A16_FLOAT:
        case angle::FormatID::R32G32_FIXED:
        case angle::FormatID::R32G32_FLOAT:
            return 8;

        case angle::FormatID::R32G32B32_SSCALED:
        case angle::FormatID::R32G32B32_SNORM:
        case angle::FormatID::R32G32B32_USCALED:
        case angle::FormatID::R32G32B32_UNORM:
        case angle::FormatID::R32G32B32_SINT:
        case angle::FormatID::R32G32B32_UINT:
        case angle::FormatID::R32G32B32_FIXED:
        case angle::FormatID::R32G32B32_FLOAT:
            return 12;

        case angle::FormatID::R32G32B32A32_SSCALED:
        case angle::FormatID::R32G32B32A32_SNORM:
        case angle::FormatID::R32G32B32A32_USCALED:
        case angle::FormatID::R32G32B32A32_UNORM:
        case angle::FormatID::R32G32B32A32_SINT:
        case angle::FormatID::R32G32B32A32_UINT:
        case angle::FormatID::R32G32B32A32_FIXED:
        case angle::FormatID::R32G32B32A32_FLOAT:
            return 16;

        case angle::FormatID::NONE:
        default:
            UNREACHABLE();
            return 0;
    }
}

angle::FormatID ConvertFormatSignedness(const angle::Format &format)
{
    switch (format.id)
    {
        // 1 byte signed to unsigned
        case angle::FormatID::R8_SINT:
            return angle::FormatID::R8_UINT;
        case angle::FormatID::R8_SNORM:
            return angle::FormatID::R8_UNORM;
        case angle::FormatID::R8_SSCALED:
            return angle::FormatID::R8_USCALED;
        case angle::FormatID::R8G8_SINT:
            return angle::FormatID::R8G8_UINT;
        case angle::FormatID::R8G8_SNORM:
            return angle::FormatID::R8G8_UNORM;
        case angle::FormatID::R8G8_SSCALED:
            return angle::FormatID::R8G8_USCALED;
        case angle::FormatID::R8G8B8_SINT:
            return angle::FormatID::R8G8B8_UINT;
        case angle::FormatID::R8G8B8_SNORM:
            return angle::FormatID::R8G8B8_UNORM;
        case angle::FormatID::R8G8B8_SSCALED:
            return angle::FormatID::R8G8B8_USCALED;
        case angle::FormatID::R8G8B8A8_SINT:
            return angle::FormatID::R8G8B8A8_UINT;
        case angle::FormatID::R8G8B8A8_SNORM:
            return angle::FormatID::R8G8B8A8_UNORM;
        case angle::FormatID::R8G8B8A8_SSCALED:
            return angle::FormatID::R8G8B8A8_USCALED;
        // 1 byte unsigned to signed
        case angle::FormatID::R8_UINT:
            return angle::FormatID::R8_SINT;
        case angle::FormatID::R8_UNORM:
            return angle::FormatID::R8_SNORM;
        case angle::FormatID::R8_USCALED:
            return angle::FormatID::R8_SSCALED;
        case angle::FormatID::R8G8_UINT:
            return angle::FormatID::R8G8_SINT;
        case angle::FormatID::R8G8_UNORM:
            return angle::FormatID::R8G8_SNORM;
        case angle::FormatID::R8G8_USCALED:
            return angle::FormatID::R8G8_SSCALED;
        case angle::FormatID::R8G8B8_UINT:
            return angle::FormatID::R8G8B8_SINT;
        case angle::FormatID::R8G8B8_UNORM:
            return angle::FormatID::R8G8B8_SNORM;
        case angle::FormatID::R8G8B8_USCALED:
            return angle::FormatID::R8G8B8_SSCALED;
        case angle::FormatID::R8G8B8A8_UINT:
            return angle::FormatID::R8G8B8A8_SINT;
        case angle::FormatID::R8G8B8A8_UNORM:
            return angle::FormatID::R8G8B8A8_SNORM;
        case angle::FormatID::R8G8B8A8_USCALED:
            return angle::FormatID::R8G8B8A8_SSCALED;
        // 2 byte signed to unsigned
        case angle::FormatID::R16_SINT:
            return angle::FormatID::R16_UINT;
        case angle::FormatID::R16_SNORM:
            return angle::FormatID::R16_UNORM;
        case angle::FormatID::R16_SSCALED:
            return angle::FormatID::R16_USCALED;
        case angle::FormatID::R16G16_SINT:
            return angle::FormatID::R16G16_UINT;
        case angle::FormatID::R16G16_SNORM:
            return angle::FormatID::R16G16_UNORM;
        case angle::FormatID::R16G16_SSCALED:
            return angle::FormatID::R16G16_USCALED;
        case angle::FormatID::R16G16B16_SINT:
            return angle::FormatID::R16G16B16_UINT;
        case angle::FormatID::R16G16B16_SNORM:
            return angle::FormatID::R16G16B16_UNORM;
        case angle::FormatID::R16G16B16_SSCALED:
            return angle::FormatID::R16G16B16_USCALED;
        case angle::FormatID::R16G16B16A16_SINT:
            return angle::FormatID::R16G16B16A16_UINT;
        case angle::FormatID::R16G16B16A16_SNORM:
            return angle::FormatID::R16G16B16A16_UNORM;
        case angle::FormatID::R16G16B16A16_SSCALED:
            return angle::FormatID::R16G16B16A16_USCALED;
        // 2 byte unsigned to signed
        case angle::FormatID::R16_UINT:
            return angle::FormatID::R16_SINT;
        case angle::FormatID::R16_UNORM:
            return angle::FormatID::R16_SNORM;
        case angle::FormatID::R16_USCALED:
            return angle::FormatID::R16_SSCALED;
        case angle::FormatID::R16G16_UINT:
            return angle::FormatID::R16G16_SINT;
        case angle::FormatID::R16G16_UNORM:
            return angle::FormatID::R16G16_SNORM;
        case angle::FormatID::R16G16_USCALED:
            return angle::FormatID::R16G16_SSCALED;
        case angle::FormatID::R16G16B16_UINT:
            return angle::FormatID::R16G16B16_SINT;
        case angle::FormatID::R16G16B16_UNORM:
            return angle::FormatID::R16G16B16_SNORM;
        case angle::FormatID::R16G16B16_USCALED:
            return angle::FormatID::R16G16B16_SSCALED;
        case angle::FormatID::R16G16B16A16_UINT:
            return angle::FormatID::R16G16B16A16_SINT;
        case angle::FormatID::R16G16B16A16_UNORM:
            return angle::FormatID::R16G16B16A16_SNORM;
        case angle::FormatID::R16G16B16A16_USCALED:
            return angle::FormatID::R16G16B16A16_SSCALED;
        // 4 byte signed to unsigned
        case angle::FormatID::R32_SINT:
            return angle::FormatID::R32_UINT;
        case angle::FormatID::R32_SNORM:
            return angle::FormatID::R32_UNORM;
        case angle::FormatID::R32_SSCALED:
            return angle::FormatID::R32_USCALED;
        case angle::FormatID::R32G32_SINT:
            return angle::FormatID::R32G32_UINT;
        case angle::FormatID::R32G32_SNORM:
            return angle::FormatID::R32G32_UNORM;
        case angle::FormatID::R32G32_SSCALED:
            return angle::FormatID::R32G32_USCALED;
        case angle::FormatID::R32G32B32_SINT:
            return angle::FormatID::R32G32B32_UINT;
        case angle::FormatID::R32G32B32_SNORM:
            return angle::FormatID::R32G32B32_UNORM;
        case angle::FormatID::R32G32B32_SSCALED:
            return angle::FormatID::R32G32B32_USCALED;
        case angle::FormatID::R32G32B32A32_SINT:
            return angle::FormatID::R32G32B32A32_UINT;
        case angle::FormatID::R32G32B32A32_SNORM:
            return angle::FormatID::R32G32B32A32_UNORM;
        case angle::FormatID::R32G32B32A32_SSCALED:
            return angle::FormatID::R32G32B32A32_USCALED;
        // 4 byte unsigned to signed
        case angle::FormatID::R32_UINT:
            return angle::FormatID::R32_SINT;
        case angle::FormatID::R32_UNORM:
            return angle::FormatID::R32_SNORM;
        case angle::FormatID::R32_USCALED:
            return angle::FormatID::R32_SSCALED;
        case angle::FormatID::R32G32_UINT:
            return angle::FormatID::R32G32_SINT;
        case angle::FormatID::R32G32_UNORM:
            return angle::FormatID::R32G32_SNORM;
        case angle::FormatID::R32G32_USCALED:
            return angle::FormatID::R32G32_SSCALED;
        case angle::FormatID::R32G32B32_UINT:
            return angle::FormatID::R32G32B32_SINT;
        case angle::FormatID::R32G32B32_UNORM:
            return angle::FormatID::R32G32B32_SNORM;
        case angle::FormatID::R32G32B32_USCALED:
            return angle::FormatID::R32G32B32_SSCALED;
        case angle::FormatID::R32G32B32A32_UINT:
            return angle::FormatID::R32G32B32A32_SINT;
        case angle::FormatID::R32G32B32A32_UNORM:
            return angle::FormatID::R32G32B32A32_SNORM;
        case angle::FormatID::R32G32B32A32_USCALED:
            return angle::FormatID::R32G32B32A32_SSCALED;
        // 1010102 signed to unsigned
        case angle::FormatID::R10G10B10A2_SINT:
            return angle::FormatID::R10G10B10A2_UINT;
        case angle::FormatID::R10G10B10A2_SSCALED:
            return angle::FormatID::R10G10B10A2_USCALED;
        case angle::FormatID::R10G10B10A2_SNORM:
            return angle::FormatID::R10G10B10A2_UNORM;
        // 1010102 unsigned to signed
        case angle::FormatID::R10G10B10A2_UINT:
            return angle::FormatID::R10G10B10A2_SINT;
        case angle::FormatID::R10G10B10A2_USCALED:
            return angle::FormatID::R10G10B10A2_SSCALED;
        case angle::FormatID::R10G10B10A2_UNORM:
            return angle::FormatID::R10G10B10A2_SNORM;
        default:
            UNREACHABLE();
            return angle::FormatID::NONE;
    }
}

bool ValidES3InternalFormat(GLenum internalFormat)
{
    const InternalFormatInfoMap &formatMap = GetInternalFormatMap();
    return internalFormat != GL_NONE && formatMap.find(internalFormat) != formatMap.end();
}

VertexFormat::VertexFormat(GLenum typeIn,
                           GLboolean normalizedIn,
                           GLuint componentsIn,
                           bool pureIntegerIn)
    : type(typeIn), normalized(normalizedIn), components(componentsIn), pureInteger(pureIntegerIn)
{
    // float -> !normalized
    ASSERT(!(type == GL_FLOAT || type == GL_HALF_FLOAT || type == GL_FIXED) ||
           normalized == GL_FALSE);
}
}  // namespace gl
