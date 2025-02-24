//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// formatutils.h: Queries for GL image formats.

#ifndef LIBANGLE_FORMATUTILS_H_
#define LIBANGLE_FORMATUTILS_H_

#include <stdint.h>
#include <cstddef>
#include <ostream>

#include "angle_gl.h"
#include "common/android_util.h"
#include "common/hash_containers.h"
#include "libANGLE/Caps.h"
#include "libANGLE/Config.h"
#include "libANGLE/Error.h"
#include "libANGLE/Version.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/angletypes.h"

namespace gl
{
struct VertexAttribute;

struct FormatType final
{
    FormatType();
    FormatType(GLenum format_, GLenum type_);
    FormatType(const FormatType &other)            = default;
    FormatType &operator=(const FormatType &other) = default;

    bool operator<(const FormatType &other) const;

    GLenum format;
    GLenum type;
};

struct Type
{
    Type() : bytes(0), bytesShift(0), specialInterpretation(0) {}

    explicit Type(uint32_t packedTypeInfo)
        : bytes(packedTypeInfo & 0xff),
          bytesShift((packedTypeInfo >> 8) & 0xff),
          specialInterpretation((packedTypeInfo >> 16) & 1)
    {}

    GLuint bytes;
    GLuint bytesShift;  // Bit shift by this value to effectively divide/multiply by "bytes" in a
                        // more optimal way
    bool specialInterpretation;
};

uint32_t GetPackedTypeInfo(GLenum type);

ANGLE_INLINE GLenum GetNonLinearFormat(const GLenum format)
{
    switch (format)
    {
        case GL_BGRA8_EXT:
            return GL_BGRA8_SRGB_ANGLEX;
        case GL_RGBA8:
            return GL_SRGB8_ALPHA8;
        case GL_RGB8:
            return GL_SRGB8;
        case GL_BGRX8_ANGLEX:
            return GL_BGRX8_SRGB_ANGLEX;
        case GL_RGBX8_ANGLE:
            return GL_RGBX8_SRGB_ANGLEX;
        case GL_RGBA16F:
            return GL_RGBA16F;
        case GL_RGB10_A2_EXT:
            return GL_RGB10_A2_EXT;
        case GL_SRGB8:
        case GL_SRGB8_ALPHA8:
        case GL_SRGB_ALPHA_EXT:
        case GL_SRGB_EXT:
            return format;
        default:
            return GL_NONE;
    }
}

ANGLE_INLINE GLenum GetLinearFormat(const GLenum format)
{
    switch (format)
    {
        case GL_BGRA8_SRGB_ANGLEX:
            return GL_BGRA8_EXT;
        case GL_SRGB8_ALPHA8:
            return GL_RGBA8;
        case GL_SRGB8:
            return GL_RGB8;
        case GL_BGRX8_SRGB_ANGLEX:
            return GL_BGRX8_ANGLEX;
        case GL_RGBX8_SRGB_ANGLEX:
            return GL_RGBX8_ANGLE;
        default:
            return format;
    }
}

ANGLE_INLINE bool ColorspaceFormatOverride(const EGLenum colorspace, GLenum *rendertargetformat)
{
    // Override the rendertargetformat based on colorpsace
    switch (colorspace)
    {
        case EGL_GL_COLORSPACE_LINEAR:                 // linear colorspace no translation needed
        case EGL_GL_COLORSPACE_SCRGB_LINEAR_EXT:       // linear colorspace no translation needed
        case EGL_GL_COLORSPACE_BT2020_LINEAR_EXT:      // linear colorspace no translation needed
        case EGL_GL_COLORSPACE_DISPLAY_P3_LINEAR_EXT:  // linear colorspace no translation needed
            *rendertargetformat = GetLinearFormat(*rendertargetformat);
            return true;
        case EGL_GL_COLORSPACE_DISPLAY_P3_PASSTHROUGH_EXT:  // App, not the HW, will specify the
                                                            // transfer function
        case EGL_GL_COLORSPACE_SCRGB_EXT:  // App, not the HW, will specify the transfer function
            // No translation
            return true;
        case EGL_GL_COLORSPACE_SRGB_KHR:
        case EGL_GL_COLORSPACE_BT2020_PQ_EXT:
        case EGL_GL_COLORSPACE_BT2020_HLG_EXT:
        case EGL_GL_COLORSPACE_DISPLAY_P3_EXT:
        {
            GLenum nonLinearFormat = GetNonLinearFormat(*rendertargetformat);
            if (nonLinearFormat != GL_NONE)
            {
                *rendertargetformat = nonLinearFormat;
                return true;
            }
            else
            {
                return false;
            }
        }
        break;
        default:
            UNREACHABLE();
            return false;
    }
}

ANGLE_INLINE const Type GetTypeInfo(GLenum type)
{
    return Type(GetPackedTypeInfo(type));
}

// This helpers use tricks based on the assumption that the type has certain values.
static_assert(static_cast<GLuint>(DrawElementsType::UnsignedByte) == 0, "Please update this code.");
static_assert(static_cast<GLuint>(DrawElementsType::UnsignedShort) == 1,
              "Please update this code.");
static_assert(static_cast<GLuint>(DrawElementsType::UnsignedInt) == 2, "Please update this code.");
ANGLE_INLINE GLuint GetDrawElementsTypeSize(DrawElementsType type)
{
    return (1 << static_cast<GLuint>(type));
}

ANGLE_INLINE GLuint GetDrawElementsTypeShift(DrawElementsType type)
{
    return static_cast<GLuint>(type);
}

// Information about an OpenGL internal format.  Can be keyed on the internalFormat and type
// members.
struct InternalFormat
{
    InternalFormat();
    InternalFormat(const InternalFormat &other);
    InternalFormat &operator=(const InternalFormat &other);

    GLuint computePixelBytes(GLenum formatType) const;

    [[nodiscard]] bool computeBufferRowLength(uint32_t width, uint32_t *resultOut) const;
    [[nodiscard]] bool computeBufferImageHeight(uint32_t height, uint32_t *resultOut) const;

    [[nodiscard]] bool computeRowPitch(GLenum formatType,
                                       GLsizei width,
                                       GLint alignment,
                                       GLint rowLength,
                                       GLuint *resultOut) const;
    [[nodiscard]] bool computeDepthPitch(GLsizei height,
                                         GLint imageHeight,
                                         GLuint rowPitch,
                                         GLuint *resultOut) const;
    [[nodiscard]] bool computeDepthPitch(GLenum formatType,
                                         GLsizei width,
                                         GLsizei height,
                                         GLint alignment,
                                         GLint rowLength,
                                         GLint imageHeight,
                                         GLuint *resultOut) const;

    [[nodiscard]] bool computePalettedImageRowPitch(GLsizei width, GLuint *resultOut) const;

    [[nodiscard]] bool computeCompressedImageRowPitch(GLsizei width, GLuint *resultOut) const;

    [[nodiscard]] bool computeCompressedImageDepthPitch(GLsizei height,
                                                        GLuint rowPitch,
                                                        GLuint *resultOut) const;

    [[nodiscard]] bool computeCompressedImageSize(const Extents &size, GLuint *resultOut) const;

    [[nodiscard]] std::pair<GLuint, GLuint> getCompressedImageMinBlocks() const;

    [[nodiscard]] bool computeSkipBytes(GLenum formatType,
                                        GLuint rowPitch,
                                        GLuint depthPitch,
                                        const PixelStoreStateBase &state,
                                        bool is3D,
                                        GLuint *resultOut) const;

    [[nodiscard]] bool computePackUnpackEndByte(GLenum formatType,
                                                const Extents &size,
                                                const PixelStoreStateBase &state,
                                                bool is3D,
                                                GLuint *resultOut) const;

    bool isLUMA() const;
    GLenum getReadPixelsFormat(const Extensions &extensions) const;
    GLenum getReadPixelsType(const Version &version) const;

    // Support upload a portion of image?
    bool supportSubImage() const;

    ANGLE_INLINE bool isChannelSizeCompatible(GLuint redSize,
                                              GLuint greenSize,
                                              GLuint blueSize,
                                              GLuint alphaSize) const
    {
        // We only check for equality in all channel sizes
        return ((redSize == redBits) && (greenSize == greenBits) && (blueSize == blueBits) &&
                (alphaSize == alphaBits));
    }

    // Return true if the format is a required renderbuffer format in the given version of the core
    // spec. Note that it isn't always clear whether all the rules that apply to core required
    // renderbuffer formats also apply to additional formats added by extensions. Because of this
    // extension formats are conservatively not included.
    bool isRequiredRenderbufferFormat(const Version &version) const;

    bool isInt() const;
    bool isDepthOrStencil() const;

    GLuint getEGLConfigBufferSize() const;

    bool operator==(const InternalFormat &other) const;
    bool operator!=(const InternalFormat &other) const;

    GLenum internalFormat;

    bool sized;
    GLenum sizedInternalFormat;

    GLuint redBits;
    GLuint greenBits;
    GLuint blueBits;

    GLuint luminanceBits;

    GLuint alphaBits;
    GLuint sharedBits;

    GLuint depthBits;
    GLuint stencilBits;

    GLuint pixelBytes;

    GLuint componentCount;

    bool compressed;
    GLuint compressedBlockWidth;
    GLuint compressedBlockHeight;
    GLuint compressedBlockDepth;

    bool paletted;
    GLuint paletteBits;

    GLenum format;
    GLenum type;

    GLenum componentType;
    GLenum colorEncoding;

    typedef bool (*SupportCheckFunction)(const Version &, const Extensions &);
    SupportCheckFunction textureSupport;
    SupportCheckFunction filterSupport;
    SupportCheckFunction textureAttachmentSupport;  // glFramebufferTexture2D
    SupportCheckFunction renderbufferSupport;       // glFramebufferRenderbuffer
    SupportCheckFunction blendSupport;
};

// A "Format" wraps an InternalFormat struct, querying it from either a sized internal format or
// unsized internal format and type.
// TODO(geofflang): Remove this, it doesn't add any more information than the InternalFormat object.
struct Format
{
    // Sized types only.
    explicit Format(GLenum internalFormat);

    // Sized or unsized types.
    explicit Format(const InternalFormat &internalFormat) : info(&internalFormat) {}

    Format(GLenum internalFormat, GLenum type);

    Format(const Format &other)            = default;
    Format &operator=(const Format &other) = default;

    bool valid() const;

    static Format Invalid();
    static bool SameSized(const Format &a, const Format &b);
    static bool EquivalentForBlit(const Format &a, const Format &b);

    friend std::ostream &operator<<(std::ostream &os, const Format &fmt);

    // This is the sized info.
    const InternalFormat *info;
};

const InternalFormat &GetSizedInternalFormatInfo(GLenum internalFormat);
const InternalFormat &GetInternalFormatInfo(GLenum internalFormat, GLenum type);

// Strip sizing information from an internal format.  Doesn't necessarily validate that the internal
// format is valid.
GLenum GetUnsizedFormat(GLenum internalFormat);

// Return whether the compressed format requires whole image/mip level to be uploaded to texture.
bool CompressedFormatRequiresWholeImage(GLenum internalFormat);

// In support of GetImage, check for LUMA formats and override with real format
void MaybeOverrideLuminance(GLenum &format, GLenum &type, GLenum actualFormat, GLenum actualType);

typedef std::set<GLenum> FormatSet;
const FormatSet &GetAllSizedInternalFormats();

typedef angle::HashMap<GLenum, angle::HashMap<GLenum, InternalFormat>> InternalFormatInfoMap;
const InternalFormatInfoMap &GetInternalFormatMap();

int GetAndroidHardwareBufferFormatFromChannelSizes(const egl::AttributeMap &attribMap);

GLenum GetConfigColorBufferFormat(const egl::Config *config);
GLenum GetConfigDepthStencilBufferFormat(const egl::Config *config);

ANGLE_INLINE int GetNativeVisualID(const InternalFormat &internalFormat)
{
    int nativeVisualId = 0;
#if defined(ANGLE_PLATFORM_ANDROID)
    nativeVisualId =
        angle::android::GLInternalFormatToNativePixelFormat(internalFormat.internalFormat);
#endif
#if defined(ANGLE_PLATFORM_LINUX) && defined(ANGLE_USES_GBM)
    nativeVisualId = angle::GLInternalFormatToGbmFourCCFormat(internalFormat.internalFormat);
#endif

    return nativeVisualId;
}

// From the ESSL 3.00.4 spec:
// Vertex shader inputs can only be float, floating-point vectors, matrices, signed and unsigned
// integers and integer vectors. Vertex shader inputs cannot be arrays or structures.

enum AttributeType
{
    ATTRIBUTE_FLOAT,
    ATTRIBUTE_VEC2,
    ATTRIBUTE_VEC3,
    ATTRIBUTE_VEC4,
    ATTRIBUTE_INT,
    ATTRIBUTE_IVEC2,
    ATTRIBUTE_IVEC3,
    ATTRIBUTE_IVEC4,
    ATTRIBUTE_UINT,
    ATTRIBUTE_UVEC2,
    ATTRIBUTE_UVEC3,
    ATTRIBUTE_UVEC4,
    ATTRIBUTE_MAT2,
    ATTRIBUTE_MAT3,
    ATTRIBUTE_MAT4,
    ATTRIBUTE_MAT2x3,
    ATTRIBUTE_MAT2x4,
    ATTRIBUTE_MAT3x2,
    ATTRIBUTE_MAT3x4,
    ATTRIBUTE_MAT4x2,
    ATTRIBUTE_MAT4x3,
};

AttributeType GetAttributeType(GLenum enumValue);

typedef std::vector<angle::FormatID> InputLayout;

struct VertexFormat : private angle::NonCopyable
{
    VertexFormat(GLenum typeIn, GLboolean normalizedIn, GLuint componentsIn, bool pureIntegerIn);

    GLenum type;
    GLboolean normalized;
    GLuint components;
    bool pureInteger;
};

constexpr uint32_t kVertexFormatCount = static_cast<uint32_t>(VertexAttribType::EnumCount);
extern const angle::FormatID kVertexFormatPureInteger[kVertexFormatCount][4];
extern const angle::FormatID kVertexFormatNormalized[kVertexFormatCount][4];
extern const angle::FormatID kVertexFormatScaled[kVertexFormatCount][4];

ANGLE_INLINE angle::FormatID GetVertexFormatID(VertexAttribType type,
                                               GLboolean normalized,
                                               GLuint components,
                                               bool pureInteger)
{
    ASSERT(components >= 1 && components <= 4);

    angle::FormatID result;
    int index = static_cast<int>(type);
    if (pureInteger)
    {
        result = kVertexFormatPureInteger[index][components - 1];
    }
    else if (normalized)
    {
        result = kVertexFormatNormalized[index][components - 1];
    }
    else
    {
        result = kVertexFormatScaled[index][components - 1];
    }

    ASSERT(result != angle::FormatID::NONE);
    return result;
}

angle::FormatID GetVertexFormatID(const VertexAttribute &attrib, VertexAttribType currentValueType);
angle::FormatID GetCurrentValueFormatID(VertexAttribType currentValueType);
const VertexFormat &GetVertexFormatFromID(angle::FormatID vertexFormatID);
size_t GetVertexFormatSize(angle::FormatID vertexFormatID);
angle::FormatID ConvertFormatSignedness(const angle::Format &format);

ANGLE_INLINE bool IsS3TCFormat(const GLenum format)
{
    switch (format)
    {
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
            return true;

        default:
            return false;
    }
}

ANGLE_INLINE bool IsRGTCFormat(const GLenum format)
{
    switch (format)
    {
        case GL_COMPRESSED_RED_RGTC1_EXT:
        case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:
        case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
        case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:
            return true;

        default:
            return false;
    }
}

ANGLE_INLINE bool IsBPTCFormat(const GLenum format)
{
    switch (format)
    {
        case GL_COMPRESSED_RGBA_BPTC_UNORM_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT:
        case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT:
        case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT:
            return true;

        default:
            return false;
    }
}

ANGLE_INLINE bool IsASTC2DFormat(const GLenum format)
{
    if ((format >= GL_COMPRESSED_RGBA_ASTC_4x4_KHR &&
         format <= GL_COMPRESSED_RGBA_ASTC_12x12_KHR) ||
        (format >= GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR &&
         format <= GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR))
    {
        return true;
    }
    return false;
}

ANGLE_INLINE bool IsETC1Format(const GLenum format)
{
    switch (format)
    {
        case GL_ETC1_RGB8_OES:
            return true;

        default:
            return false;
    }
}

ANGLE_INLINE bool IsETC2EACFormat(const GLenum format)
{
    // ES 3.1, Table 8.19
    switch (format)
    {
        case GL_COMPRESSED_R11_EAC:
        case GL_COMPRESSED_SIGNED_R11_EAC:
        case GL_COMPRESSED_RG11_EAC:
        case GL_COMPRESSED_SIGNED_RG11_EAC:
        case GL_COMPRESSED_RGB8_ETC2:
        case GL_COMPRESSED_SRGB8_ETC2:
        case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        case GL_COMPRESSED_RGBA8_ETC2_EAC:
        case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
            return true;

        default:
            return false;
    }
}

ANGLE_INLINE constexpr bool IsPVRTC1Format(const GLenum format)
{
    // This function is called for all compressed texture uploads. The expression below generates
    // fewer instructions than a regular switch statement. Two groups of four consecutive values,
    // each group starts with two least significant bits unset.
    return ((format & ~3) == GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG) ||
           ((format & ~3) == GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT);
}
static_assert(IsPVRTC1Format(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG), "0x8C00");
static_assert(IsPVRTC1Format(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG), "0x8C01");
static_assert(IsPVRTC1Format(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG), "0x8C02");
static_assert(IsPVRTC1Format(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG), "0x8C03");
static_assert(IsPVRTC1Format(GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT), "0x8A54");
static_assert(IsPVRTC1Format(GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT), "0x8A55");
static_assert(IsPVRTC1Format(GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT), "0x8A56");
static_assert(IsPVRTC1Format(GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT), "0x8A57");
static_assert(!IsPVRTC1Format(0x8BFF) && !IsPVRTC1Format(0x8C04), "invalid");
static_assert(!IsPVRTC1Format(0x8A53) && !IsPVRTC1Format(0x8A58), "invalid");

ANGLE_INLINE bool IsBGRAFormat(const GLenum internalFormat)
{
    switch (internalFormat)
    {
        case GL_BGRA8_EXT:
        case GL_BGRA4_ANGLEX:
        case GL_BGR5_A1_ANGLEX:
        case GL_BGRA8_SRGB_ANGLEX:
        case GL_BGRX8_ANGLEX:
        case GL_BGR565_ANGLEX:
        case GL_BGR10_A2_ANGLEX:
            return true;

        default:
            return false;
    }
}

// Check if an internal format is ever valid in ES3.  Makes no checks about support for a specific
// context.
bool ValidES3InternalFormat(GLenum internalFormat);

// Implemented in format_map_autogen.cpp
bool ValidES3Format(GLenum format);
bool ValidES3Type(GLenum type);
bool ValidES3FormatCombination(GLenum format, GLenum type, GLenum internalFormat);

// Implemented in es3_copy_conversion_table_autogen.cpp
bool ValidES3CopyConversion(GLenum textureFormat, GLenum framebufferFormat);

ANGLE_INLINE ComponentType GetVertexAttributeComponentType(bool pureInteger, VertexAttribType type)
{
    if (pureInteger)
    {
        switch (type)
        {
            case VertexAttribType::Byte:
            case VertexAttribType::Short:
            case VertexAttribType::Int:
                return ComponentType::Int;

            case VertexAttribType::UnsignedByte:
            case VertexAttribType::UnsignedShort:
            case VertexAttribType::UnsignedInt:
                return ComponentType::UnsignedInt;

            default:
                UNREACHABLE();
                return ComponentType::NoType;
        }
    }
    else
    {
        return ComponentType::Float;
    }
}

constexpr std::size_t kMaxYuvPlaneCount = 3;
template <typename T>
using YuvPlaneArray = std::array<T, kMaxYuvPlaneCount>;

struct YuvFormatInfo
{
    // Sized types only.
    YuvFormatInfo(GLenum internalFormat, const Extents &yPlaneExtent);

    GLenum glInternalFormat;
    uint32_t planeCount;
    YuvPlaneArray<uint32_t> planeBpp;
    YuvPlaneArray<Extents> planeExtent;
    YuvPlaneArray<uint32_t> planePitch;
    YuvPlaneArray<uint32_t> planeSize;
    YuvPlaneArray<uint32_t> planeOffset;
};

bool IsYuvFormat(GLenum format);
uint32_t GetPlaneCount(GLenum format);
uint32_t GetYPlaneBpp(GLenum format);
uint32_t GetChromaPlaneBpp(GLenum format);
void GetSubSampleFactor(GLenum format,
                        int *horizontalSubsampleFactor,
                        int *verticalSubsampleFactor);
}  // namespace gl

#endif  // LIBANGLE_FORMATUTILS_H_
