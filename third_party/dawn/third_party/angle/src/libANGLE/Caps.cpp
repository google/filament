//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/Caps.h"

#include "common/angleutils.h"
#include "common/debug.h"

#include "libANGLE/formatutils.h"

#include "angle_gl.h"

#include <algorithm>
#include <sstream>

static void InsertExtensionString(const std::string &extension,
                                  bool supported,
                                  std::vector<std::string> *extensionVector)
{
    if (supported)
    {
        extensionVector->push_back(extension);
    }
}

namespace gl
{

TextureCaps::TextureCaps() = default;

TextureCaps::TextureCaps(const TextureCaps &other) = default;

TextureCaps &TextureCaps::operator=(const TextureCaps &other) = default;

TextureCaps::~TextureCaps() = default;

GLuint TextureCaps::getMaxSamples() const
{
    return !sampleCounts.empty() ? *sampleCounts.rbegin() : 0;
}

GLuint TextureCaps::getNearestSamples(GLuint requestedSamples) const
{
    if (requestedSamples == 0)
    {
        return 0;
    }

    for (SupportedSampleSet::const_iterator i = sampleCounts.begin(); i != sampleCounts.end(); i++)
    {
        GLuint samples = *i;
        if (samples >= requestedSamples)
        {
            return samples;
        }
    }

    return 0;
}

TextureCaps GenerateMinimumTextureCaps(GLenum sizedInternalFormat,
                                       const Version &clientVersion,
                                       const Extensions &extensions)
{
    TextureCaps caps;

    const InternalFormat &internalFormatInfo = GetSizedInternalFormatInfo(sizedInternalFormat);
    caps.texturable        = internalFormatInfo.textureSupport(clientVersion, extensions);
    caps.filterable        = internalFormatInfo.filterSupport(clientVersion, extensions);
    caps.textureAttachment = internalFormatInfo.textureAttachmentSupport(clientVersion, extensions);
    caps.renderbuffer      = internalFormatInfo.renderbufferSupport(clientVersion, extensions);
    caps.blendable         = internalFormatInfo.blendSupport(clientVersion, extensions);

    caps.sampleCounts.insert(0);
    if (internalFormatInfo.isRequiredRenderbufferFormat(clientVersion))
    {
        if ((clientVersion.major >= 3 && clientVersion.minor >= 1) ||
            (clientVersion.major >= 3 && !internalFormatInfo.isInt()))
        {
            caps.sampleCounts.insert(4);
        }
    }

    return caps;
}

TextureCapsMap::TextureCapsMap() {}

TextureCapsMap::~TextureCapsMap() {}

void TextureCapsMap::insert(GLenum internalFormat, const TextureCaps &caps)
{
    angle::FormatID formatID = angle::Format::InternalFormatToID(internalFormat);
    get(formatID)            = caps;
}

void TextureCapsMap::clear()
{
    mFormatData.fill(TextureCaps());
}

const TextureCaps &TextureCapsMap::get(GLenum internalFormat) const
{
    angle::FormatID formatID = angle::Format::InternalFormatToID(internalFormat);
    return get(formatID);
}

const TextureCaps &TextureCapsMap::get(angle::FormatID formatID) const
{
    return mFormatData[formatID];
}

TextureCaps &TextureCapsMap::get(angle::FormatID formatID)
{
    return mFormatData[formatID];
}

void TextureCapsMap::set(angle::FormatID formatID, const TextureCaps &caps)
{
    get(formatID) = caps;
}

void InitMinimumTextureCapsMap(const Version &clientVersion,
                               const Extensions &extensions,
                               TextureCapsMap *capsMap)
{
    for (GLenum internalFormat : GetAllSizedInternalFormats())
    {
        capsMap->insert(internalFormat,
                        GenerateMinimumTextureCaps(internalFormat, clientVersion, extensions));
    }
}

Extensions::Extensions() = default;

Extensions::Extensions(const Extensions &other) = default;

Extensions &Extensions::operator=(const Extensions &other) = default;

std::vector<std::string> Extensions::getStrings() const
{
    std::vector<std::string> extensionStrings;

    for (const auto &extensionInfo : GetExtensionInfoMap())
    {
        if (this->*(extensionInfo.second.ExtensionsMember))
        {
            extensionStrings.push_back(extensionInfo.first);
        }
    }

    return extensionStrings;
}

Limitations::Limitations()                         = default;
Limitations::Limitations(const Limitations &other) = default;

Limitations &Limitations::operator=(const Limitations &other) = default;

static bool GetFormatSupportBase(const TextureCapsMap &textureCaps,
                                 const GLenum *requiredFormats,
                                 size_t requiredFormatsSize,
                                 bool requiresTexturing,
                                 bool requiresFiltering,
                                 bool requiresAttachingTexture,
                                 bool requiresRenderbufferSupport,
                                 bool requiresBlending)
{
    for (size_t i = 0; i < requiredFormatsSize; i++)
    {
        const TextureCaps &cap = textureCaps.get(requiredFormats[i]);
        if (requiresTexturing && !cap.texturable)
        {
            return false;
        }

        if (requiresFiltering && !cap.filterable)
        {
            return false;
        }

        if (requiresAttachingTexture && !cap.textureAttachment)
        {
            return false;
        }

        if (requiresRenderbufferSupport && !cap.renderbuffer)
        {
            return false;
        }

        if (requiresBlending && !cap.blendable)
        {
            return false;
        }
    }

    return true;
}

template <size_t N>
static bool GetFormatSupport(const TextureCapsMap &textureCaps,
                             const GLenum (&requiredFormats)[N],
                             bool requiresTexturing,
                             bool requiresFiltering,
                             bool requiresAttachingTexture,
                             bool requiresRenderbufferSupport,
                             bool requiresBlending)
{
    return GetFormatSupportBase(textureCaps, requiredFormats, N, requiresTexturing,
                                requiresFiltering, requiresAttachingTexture,
                                requiresRenderbufferSupport, requiresBlending);
}

// Check for GL_OES_packed_depth_stencil support
static bool DeterminePackedDepthStencilSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_DEPTH24_STENCIL8,
    };

    return GetFormatSupport(textureCaps, requiredFormats, false, false, true, true, false);
}

// Checks for GL_NV_read_depth support
static bool DetermineReadDepthSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_DEPTH_COMPONENT16,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, false, true, false, false);
}

// Checks for GL_NV_read_stencil support
static bool DetermineReadStencilSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_STENCIL_INDEX8,
    };

    return GetFormatSupport(textureCaps, requiredFormats, false, false, true, false, false);
}

// Checks for GL_NV_depth_buffer_float2 support
static bool DetermineDepthBufferFloat2Support(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_DEPTH_COMPONENT32F,
        GL_DEPTH32F_STENCIL8,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, false, true, false, false);
}

// Checks for GL_ARM_rgba8 support
static bool DetermineRGBA8TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_RGBA8,
    };

    return GetFormatSupport(textureCaps, requiredFormats, false, false, false, true, false);
}

// Checks for GL_OES_required_internalformat support
static bool DetermineRequiredInternalFormatTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredTexturingFormats[] = {
        GL_ALPHA8_OES,
        GL_LUMINANCE8_OES,
        GL_LUMINANCE8_ALPHA8_OES,
        GL_LUMINANCE4_ALPHA4_OES,
        GL_RGB565_OES,
        GL_RGB8_OES,
        GL_RGBA4_OES,
        GL_RGB5_A1_OES,
        GL_RGBA8_OES,
    };

    constexpr GLenum requiredRenderingFormats[] = {
        GL_RGB565_OES, GL_RGB8_OES, GL_RGBA4_OES, GL_RGB5_A1_OES, GL_RGBA8_OES,
    };

    return GetFormatSupport(textureCaps, requiredTexturingFormats, true, false, false, false,
                            false) &&
           GetFormatSupport(textureCaps, requiredRenderingFormats, false, false, false, true,
                            false);
}

// Checks for GL_OES_rgb8_rgba8 support
static bool DetermineRGB8TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_RGB8,
    };

    return GetFormatSupport(textureCaps, requiredFormats, false, false, false, true, false);
}

// Checks for GL_EXT_texture_format_BGRA8888 support
static bool DetermineBGRA8TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_BGRA8_EXT,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, true, true, false);
}

// Checks for GL_EXT_read_format_bgra support
static bool DetermineBGRAReadFormatSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_BGRA8_EXT,
        // TODO(http://anglebug.com/42262931): GL_EXT_read_format_bgra specifies 2 more types, which
        // are currently ignored. The equivalent formats would be: GL_BGRA4_ANGLEX,
        // GL_BGR5_A1_ANGLEX
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, false, true, true, false);
}

// Checks for GL_OES_color_buffer_half_float support
static bool DetermineColorBufferHalfFloatSupport(const TextureCapsMap &textureCaps)
{
    // EXT_color_buffer_half_float issue #2 states that an implementation doesn't need to support
    // rendering to any of the formats but is expected to be able to render to at least one. WebGL
    // requires that at least RGBA16F is renderable so we make the same requirement.
    constexpr GLenum requiredFormats[] = {
        GL_RGBA16F,
    };

    return GetFormatSupport(textureCaps, requiredFormats, false, false, true, true, false);
}

// Checks for GL_OES_texture_half_float support
static bool DetermineHalfFloatTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_RGBA16F, GL_RGB16F, GL_LUMINANCE_ALPHA16F_EXT, GL_LUMINANCE16F_EXT, GL_ALPHA16F_EXT,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, false, false, false, false);
}

// Checks for GL_OES_texture_half_float_linear support
static bool DetermineHalfFloatTextureFilteringSupport(const TextureCapsMap &textureCaps,
                                                      bool checkLegacyFormats)
{
    constexpr GLenum requiredFormats[] = {GL_RGBA16F, GL_RGB16F};
    // If GL_OES_texture_half_float is present, this extension must also support legacy formats
    // introduced by that extension
    constexpr GLenum requiredFormatsES2[] = {GL_LUMINANCE_ALPHA16F_EXT, GL_LUMINANCE16F_EXT,
                                             GL_ALPHA16F_EXT};

    if (checkLegacyFormats &&
        !GetFormatSupport(textureCaps, requiredFormatsES2, false, true, false, false, false))
    {
        return false;
    }

    return GetFormatSupport(textureCaps, requiredFormats, false, true, false, false, false);
}

// Checks for GL_OES_texture_float support
static bool DetermineFloatTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_RGBA32F, GL_RGB32F, GL_LUMINANCE_ALPHA32F_EXT, GL_LUMINANCE32F_EXT, GL_ALPHA32F_EXT,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, false, false, false, false);
}

// Checks for GL_OES_texture_float_linear support
static bool DetermineFloatTextureFilteringSupport(const TextureCapsMap &textureCaps,
                                                  bool checkLegacyFormats)
{
    constexpr GLenum requiredFormats[] = {
        GL_RGBA32F,
        GL_RGB32F,
    };
    // If GL_OES_texture_float is present, this extension must also support legacy formats
    // introduced by that extension
    constexpr GLenum requiredFormatsES2[] = {
        GL_LUMINANCE_ALPHA32F_EXT,
        GL_LUMINANCE32F_EXT,
        GL_ALPHA32F_EXT,
    };

    if (checkLegacyFormats &&
        !GetFormatSupport(textureCaps, requiredFormatsES2, false, true, false, false, false))
    {
        return false;
    }

    return GetFormatSupport(textureCaps, requiredFormats, false, true, false, false, false);
}

// Checks for GL_EXT_texture_rg support
static bool DetermineRGTextureSupport(const TextureCapsMap &textureCaps,
                                      bool checkHalfFloatFormats,
                                      bool checkFloatFormats)
{
    constexpr GLenum requiredFormats[] = {
        GL_R8,
        GL_RG8,
    };
    constexpr GLenum requiredHalfFloatFormats[] = {
        GL_R16F,
        GL_RG16F,
    };
    constexpr GLenum requiredFloatFormats[] = {
        GL_R32F,
        GL_RG32F,
    };

    if (checkHalfFloatFormats &&
        !GetFormatSupport(textureCaps, requiredHalfFloatFormats, true, false, false, false, false))
    {
        return false;
    }

    if (checkFloatFormats &&
        !GetFormatSupport(textureCaps, requiredFloatFormats, true, false, false, false, false))
    {
        return false;
    }

    return GetFormatSupport(textureCaps, requiredFormats, true, true, true, true, false);
}

static bool DetermineTextureFormat2101010Support(const TextureCapsMap &textureCaps)
{
    // GL_EXT_texture_type_2_10_10_10_REV specifies both RGBA and RGB support.
    constexpr GLenum requiredFormats[] = {
        GL_RGB10_A2,
        GL_RGB10_EXT,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for GL_EXT_texture_compression_dxt1 support
static bool DetermineDXT1TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
        GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for GL_ANGLE_texture_compression_dxt3 support
static bool DetermineDXT3TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for GL_ANGLE_texture_compression_dxt5 support
static bool DetermineDXT5TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for GL_EXT_texture_compression_s3tc_srgb support
static bool DetermineS3TCsRGBTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,
        GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,
        GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,
        GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for GL_KHR_texture_compression_astc_ldr support
static bool DetermineASTCLDRTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RGBA_ASTC_4x4_KHR,           GL_COMPRESSED_RGBA_ASTC_5x4_KHR,
        GL_COMPRESSED_RGBA_ASTC_5x5_KHR,           GL_COMPRESSED_RGBA_ASTC_6x5_KHR,
        GL_COMPRESSED_RGBA_ASTC_6x6_KHR,           GL_COMPRESSED_RGBA_ASTC_8x5_KHR,
        GL_COMPRESSED_RGBA_ASTC_8x6_KHR,           GL_COMPRESSED_RGBA_ASTC_8x8_KHR,
        GL_COMPRESSED_RGBA_ASTC_10x5_KHR,          GL_COMPRESSED_RGBA_ASTC_10x6_KHR,
        GL_COMPRESSED_RGBA_ASTC_10x8_KHR,          GL_COMPRESSED_RGBA_ASTC_10x10_KHR,
        GL_COMPRESSED_RGBA_ASTC_12x10_KHR,         GL_COMPRESSED_RGBA_ASTC_12x12_KHR,
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR,
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR,
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR,
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR,
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR,  GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR,
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR,  GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR,
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for GL_OES_texture_compression_astc support
static bool DetermineASTCOESTExtureSupport(const TextureCapsMap &textureCaps)
{
    if (!DetermineASTCLDRTextureSupport(textureCaps))
    {
        return false;
    }

    // The OES version of the extension also requires the 3D ASTC formats
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RGBA_ASTC_3x3x3_OES,         GL_COMPRESSED_RGBA_ASTC_4x3x3_OES,
        GL_COMPRESSED_RGBA_ASTC_4x4x3_OES,         GL_COMPRESSED_RGBA_ASTC_4x4x4_OES,
        GL_COMPRESSED_RGBA_ASTC_5x4x4_OES,         GL_COMPRESSED_RGBA_ASTC_5x5x4_OES,
        GL_COMPRESSED_RGBA_ASTC_5x5x5_OES,         GL_COMPRESSED_RGBA_ASTC_6x5x5_OES,
        GL_COMPRESSED_RGBA_ASTC_6x6x5_OES,         GL_COMPRESSED_RGBA_ASTC_6x6x6_OES,
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES,
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES,
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES,
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES,
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for GL_ETC1_RGB8_OES support
static bool DetermineETC1RGB8TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_ETC1_RGB8_OES,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for OES_compressed_ETC2_RGB8_texture support
static bool DetermineETC2RGB8TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RGB8_ETC2,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for OES_compressed_ETC2_sRGB8_texture support
static bool DetermineETC2sRGB8TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_SRGB8_ETC2,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for OES_compressed_ETC2_punchthroughA_RGBA8_texture support
static bool DetermineETC2PunchthroughARGB8TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for OES_compressed_ETC2_punchthroughA_sRGB8_alpha_texture support
static bool DetermineETC2PunchthroughAsRGB8AlphaTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for OES_compressed_ETC2_RGBA8_texture support
static bool DetermineETC2RGBA8TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RGBA8_ETC2_EAC,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for OES_compressed_ETC2_sRGB8_alpha8_texture support
static bool DetermineETC2sRGB8Alpha8TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for OES_compressed_EAC_R11_unsigned_texture support
static bool DetermineEACR11UnsignedTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_R11_EAC,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for OES_compressed_EAC_R11_signed_texture support
static bool DetermineEACR11SignedTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_SIGNED_R11_EAC,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for OES_compressed_EAC_RG11_unsigned_texture support
static bool DetermineEACRG11UnsignedTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RG11_EAC,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for OES_compressed_EAC_RG11_signed_texture support
static bool DetermineEACRG11SignedTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_SIGNED_RG11_EAC,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for GL_EXT_sRGB support
static bool DetermineSRGBTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFilterFormats[] = {
        GL_SRGB8,
        GL_SRGB8_ALPHA8,
    };

    constexpr GLenum requiredRenderFormats[] = {
        GL_SRGB8_ALPHA8,
    };

    return GetFormatSupport(textureCaps, requiredFilterFormats, true, true, false, false, false) &&
           GetFormatSupport(textureCaps, requiredRenderFormats, true, false, true, true, false);
}

// Check for GL_EXT_texture_sRGB_R8 support
static bool DetermineSRGBR8TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFilterFormats[] = {GL_SR8_EXT};

    return GetFormatSupport(textureCaps, requiredFilterFormats, true, true, false, false, false);
}

// Check for GL_EXT_texture_sRGB_RG8 support
static bool DetermineSRGBRG8TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFilterFormats[] = {GL_SRG8_EXT};

    return GetFormatSupport(textureCaps, requiredFilterFormats, true, true, false, false, false);
}

// Check for GL_ANGLE_depth_texture support
static bool DetermineDepthTextureANGLESupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_DEPTH_COMPONENT16,
#if !ANGLE_PLATFORM_IOS_FAMILY
        // anglebug.com/42264611
        // TODO(dino): Temporarily Removing the need for GL_DEPTH_COMPONENT32_OES
        // because it is not supported on iOS.
        // TODO(dino): I think this needs to be a runtime check when running an iOS app on Mac.
        GL_DEPTH_COMPONENT32_OES,
#endif
        GL_DEPTH24_STENCIL8_OES,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, false, true, false, false);
}

// Check for GL_OES_depth_texture support
static bool DetermineDepthTextureOESSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_DEPTH_COMPONENT16,
#if !ANGLE_PLATFORM_IOS_FAMILY
        // anglebug.com/42264611
        // TODO(dino): Temporarily Removing the need for GL_DEPTH_COMPONENT32_OES
        // because it is not supported on iOS.
        // TODO(dino): I think this needs to be a runtime check when running an iOS app on Mac.
        GL_DEPTH_COMPONENT32_OES,
#endif
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, false, true, true, false);
}

// Check for GL_OES_depth24
static bool DetermineDepth24OESSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_DEPTH_COMPONENT24_OES,
    };

    return GetFormatSupport(textureCaps, requiredFormats, false, false, false, true, false);
}

// Check for GL_OES_depth32 support
static bool DetermineDepth32Support(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_DEPTH_COMPONENT32_OES,
    };

    return GetFormatSupport(textureCaps, requiredFormats, false, false, true, true, false);
}

// Check for GL_CHROMIUM_color_buffer_float_rgb support
static bool DetermineColorBufferFloatRGBSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_RGB32F,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, false, true, false, false);
}

// Check for GL_CHROMIUM_color_buffer_float_rgba support
static bool DetermineColorBufferFloatRGBASupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_RGBA32F,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, false, true, true, false);
}

// Check for GL_EXT_color_buffer_float support
static bool DetermineColorBufferFloatSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum nonBlendableFormats[] = {
        GL_R32F,
        GL_RG32F,
        GL_RGBA32F,
    };

    constexpr GLenum blendableFormats[] = {
        GL_R16F,
        GL_RG16F,
        GL_RGBA16F,
        GL_R11F_G11F_B10F,
    };

    return GetFormatSupport(textureCaps, nonBlendableFormats, true, false, true, true, false) &&
           GetFormatSupport(textureCaps, blendableFormats, true, false, true, true, true);
}

// Check for GL_EXT_float_blend support
static bool DetermineFloatBlendSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_R32F,
        GL_RG32F,
        GL_RGBA32F,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, false, true, true, true);
}

// Check for GL_EXT_texture_norm16 support
static bool DetermineTextureNorm16Support(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFilterFormats[] = {
        GL_R16_EXT,       GL_RG16_EXT,       GL_RGB16_EXT,       GL_RGBA16_EXT,
        GL_R16_SNORM_EXT, GL_RG16_SNORM_EXT, GL_RGB16_SNORM_EXT, GL_RGBA16_SNORM_EXT,
    };

    constexpr GLenum requiredRenderFormats[] = {
        GL_R16_EXT,
        GL_RG16_EXT,
        GL_RGBA16_EXT,
    };

    return GetFormatSupport(textureCaps, requiredFilterFormats, true, true, false, false, false) &&
           GetFormatSupport(textureCaps, requiredRenderFormats, true, false, true, true, false);
}

// Check for EXT_texture_compression_rgtc support
static bool DetermineRGTCTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RED_RGTC1_EXT, GL_COMPRESSED_SIGNED_RED_RGTC1_EXT,
        GL_COMPRESSED_RED_GREEN_RGTC2_EXT, GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT};

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for EXT_texture_compression_bptc support
static bool DetermineBPTCTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT,
        GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT};

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for GL_IMG_texture_compression_pvrtc support
static bool DeterminePVRTCTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG,
        GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG};

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Check for GL_EXT_pvrtc_sRGB support
static bool DeterminePVRTCsRGBTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT, GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT,
        GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT, GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT};

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

bool DetermineCompressedTextureETCSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {GL_COMPRESSED_R11_EAC,
                                          GL_COMPRESSED_SIGNED_R11_EAC,
                                          GL_COMPRESSED_RG11_EAC,
                                          GL_COMPRESSED_SIGNED_RG11_EAC,
                                          GL_COMPRESSED_RGB8_ETC2,
                                          GL_COMPRESSED_SRGB8_ETC2,
                                          GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,
                                          GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,
                                          GL_COMPRESSED_RGBA8_ETC2_EAC,
                                          GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC};

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false, false);
}

// Checks for GL_OES_texture_stencil8 support
static bool DetermineStencilIndex8Support(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_STENCIL_INDEX8,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, false, true, false, false);
}

// Checks for GL_QCOM_render_shared_exponent support
static bool DetermineRenderSharedExponentSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_RGB9_E5,
    };

    return GetFormatSupport(textureCaps, requiredFormats, false, false, true, true, true);
}

// Check for GL_EXT_render_snorm support
bool DetermineRenderSnormSupport(const TextureCapsMap &textureCaps, bool textureNorm16EXT)
{
    constexpr GLenum requiredSnorm8Formats[] = {
        GL_R8_SNORM,
        GL_RG8_SNORM,
        GL_RGBA8_SNORM,
    };

    constexpr GLenum requiredSnorm16Formats[] = {
        GL_R16_SNORM_EXT,
        GL_RG16_SNORM_EXT,
        GL_RGBA16_SNORM_EXT,
    };

    if (textureNorm16EXT &&
        !GetFormatSupport(textureCaps, requiredSnorm16Formats, false, false, true, true, true))
    {
        return false;
    }

    return GetFormatSupport(textureCaps, requiredSnorm8Formats, false, false, true, true, true);
}

void Extensions::setTextureExtensionSupport(const TextureCapsMap &textureCaps)
{
    // TODO(ynovikov): rgb8Rgba8OES, colorBufferHalfFloatEXT, textureHalfFloatOES,
    // textureHalfFloatLinearOES, textureFloatOES, textureFloatLinearOES, textureRgEXT, sRGB,
    // colorBufferFloatRgbCHROMIUM, colorBufferFloatRgbaCHROMIUM and colorBufferFloatEXT were
    // verified. Verify the rest.
    packedDepthStencilOES    = DeterminePackedDepthStencilSupport(textureCaps);
    rgba8ARM                 = DetermineRGBA8TextureSupport(textureCaps);
    rgb8Rgba8OES             = rgba8ARM && DetermineRGB8TextureSupport(textureCaps);
    readDepthNV              = DetermineReadDepthSupport(textureCaps);
    readStencilNV            = DetermineReadStencilSupport(textureCaps);
    depthBufferFloat2NV      = DetermineDepthBufferFloat2Support(textureCaps);
    requiredInternalformatOES = DetermineRequiredInternalFormatTextureSupport(textureCaps);
    textureFormatBGRA8888EXT = DetermineBGRA8TextureSupport(textureCaps);
    readFormatBgraEXT        = DetermineBGRAReadFormatSupport(textureCaps);
    textureHalfFloatOES      = DetermineHalfFloatTextureSupport(textureCaps);
    textureHalfFloatLinearOES =
        DetermineHalfFloatTextureFilteringSupport(textureCaps, textureHalfFloatOES);
    textureFloatOES       = DetermineFloatTextureSupport(textureCaps);
    textureFloatLinearOES = DetermineFloatTextureFilteringSupport(textureCaps, textureFloatOES);
    textureRgEXT = DetermineRGTextureSupport(textureCaps, textureHalfFloatOES, textureFloatOES);
    colorBufferHalfFloatEXT =
        textureHalfFloatOES && DetermineColorBufferHalfFloatSupport(textureCaps);
    textureType2101010REVEXT      = DetermineTextureFormat2101010Support(textureCaps);
    textureCompressionDxt1EXT     = DetermineDXT1TextureSupport(textureCaps);
    textureCompressionDxt3ANGLE   = DetermineDXT3TextureSupport(textureCaps);
    textureCompressionDxt5ANGLE   = DetermineDXT5TextureSupport(textureCaps);
    textureCompressionS3tcSrgbEXT = DetermineS3TCsRGBTextureSupport(textureCaps);
    textureCompressionAstcLdrKHR  = DetermineASTCLDRTextureSupport(textureCaps);
    textureCompressionAstcOES     = DetermineASTCOESTExtureSupport(textureCaps);
    compressedETC1RGB8TextureOES  = DetermineETC1RGB8TextureSupport(textureCaps);
    compressedETC2RGB8TextureOES  = DetermineETC2RGB8TextureSupport(textureCaps);
    compressedETC2SRGB8TextureOES = DetermineETC2sRGB8TextureSupport(textureCaps);
    compressedETC2PunchthroughARGBA8TextureOES =
        DetermineETC2PunchthroughARGB8TextureSupport(textureCaps);
    compressedETC2PunchthroughASRGB8AlphaTextureOES =
        DetermineETC2PunchthroughAsRGB8AlphaTextureSupport(textureCaps);
    compressedETC2RGBA8TextureOES       = DetermineETC2RGBA8TextureSupport(textureCaps);
    compressedETC2SRGB8Alpha8TextureOES = DetermineETC2sRGB8Alpha8TextureSupport(textureCaps);
    compressedEACR11UnsignedTextureOES  = DetermineEACR11UnsignedTextureSupport(textureCaps);
    compressedEACR11SignedTextureOES    = DetermineEACR11SignedTextureSupport(textureCaps);
    compressedEACRG11UnsignedTextureOES = DetermineEACRG11UnsignedTextureSupport(textureCaps);
    compressedEACRG11SignedTextureOES   = DetermineEACRG11SignedTextureSupport(textureCaps);
    sRGBEXT                             = DetermineSRGBTextureSupport(textureCaps);
    textureSRGBR8EXT                    = DetermineSRGBR8TextureSupport(textureCaps);
    textureSRGBRG8EXT                   = DetermineSRGBRG8TextureSupport(textureCaps);
    depthTextureANGLE                   = DetermineDepthTextureANGLESupport(textureCaps);
    depthTextureOES                     = DetermineDepthTextureOESSupport(textureCaps);
    depth24OES                          = DetermineDepth24OESSupport(textureCaps);
    depth32OES                          = DetermineDepth32Support(textureCaps);
    colorBufferFloatRgbCHROMIUM         = DetermineColorBufferFloatRGBSupport(textureCaps);
    colorBufferFloatRgbaCHROMIUM        = DetermineColorBufferFloatRGBASupport(textureCaps);
    colorBufferFloatEXT                 = DetermineColorBufferFloatSupport(textureCaps);
    floatBlendEXT                       = DetermineFloatBlendSupport(textureCaps);
    textureNorm16EXT                    = DetermineTextureNorm16Support(textureCaps);
    textureCompressionRgtcEXT           = DetermineRGTCTextureSupport(textureCaps);
    textureCompressionBptcEXT           = DetermineBPTCTextureSupport(textureCaps);
    textureCompressionPvrtcIMG          = DeterminePVRTCTextureSupport(textureCaps);
    pvrtcSRGBEXT                        = DeterminePVRTCsRGBTextureSupport(textureCaps);
    textureStencil8OES                  = DetermineStencilIndex8Support(textureCaps);
    renderSharedExponentQCOM            = DetermineRenderSharedExponentSupport(textureCaps);
    renderSnormEXT = DetermineRenderSnormSupport(textureCaps, textureNorm16EXT);
}

TypePrecision::TypePrecision() = default;

TypePrecision::TypePrecision(const TypePrecision &other) = default;

TypePrecision &TypePrecision::operator=(const TypePrecision &other) = default;

void TypePrecision::setIEEEFloat()
{
    range     = {{127, 127}};
    precision = 23;
}

void TypePrecision::setIEEEHalfFloat()
{
    range     = {{15, 15}};
    precision = 10;
}

void TypePrecision::setTwosComplementInt(unsigned int bits)
{
    range     = {{static_cast<GLint>(bits) - 1, static_cast<GLint>(bits) - 2}};
    precision = 0;
}

void TypePrecision::setSimulatedFloat(unsigned int r, unsigned int p)
{
    range     = {{static_cast<GLint>(r), static_cast<GLint>(r)}};
    precision = static_cast<GLint>(p);
}

void TypePrecision::setSimulatedInt(unsigned int r)
{
    range     = {{static_cast<GLint>(r), static_cast<GLint>(r)}};
    precision = 0;
}

void TypePrecision::get(GLint *returnRange, GLint *returnPrecision) const
{
    std::copy(range.begin(), range.end(), returnRange);
    *returnPrecision = precision;
}

Caps::Caps()                             = default;
Caps::Caps(const Caps &other)            = default;
Caps::~Caps()                            = default;
Caps &Caps::operator=(const Caps &other) = default;

Caps GenerateMinimumCaps(const Version &clientVersion, const Extensions &extensions)
{
    Caps caps;

    // EXT_draw_buffers. Set to 1 even if the extension is not present. Framebuffer and blend state
    // depends on this being > 0.
    caps.maxDrawBuffers      = 1;
    caps.maxColorAttachments = 1;

    // GLES1 emulation (Minimums taken from Table 6.20 / 6.22 (ES 1.1 spec))
    if (clientVersion < Version(2, 0))
    {
        caps.maxMultitextureUnits = 2;
        caps.maxLights            = 8;
        caps.maxClipPlanes        = 1;

        caps.maxModelviewMatrixStackDepth  = 16;
        caps.maxProjectionMatrixStackDepth = 2;
        caps.maxTextureMatrixStackDepth    = 2;

        caps.minSmoothPointSize = 1.0f;
        caps.maxSmoothPointSize = 1.0f;
    }

    if (clientVersion >= Version(2, 0))
    {
        // Table 6.18
        caps.max2DTextureSize      = 64;
        caps.maxCubeMapTextureSize = 16;
        caps.maxViewportWidth      = caps.max2DTextureSize;
        caps.maxViewportHeight     = caps.max2DTextureSize;
        caps.minAliasedPointSize   = 1;
        caps.maxAliasedPointSize   = 1;
        caps.minAliasedLineWidth   = 1;
        caps.maxAliasedLineWidth   = 1;

        // Table 6.19
        caps.vertexHighpFloat.setSimulatedFloat(62, 16);
        caps.vertexMediumpFloat.setSimulatedFloat(14, 10);
        caps.vertexLowpFloat.setSimulatedFloat(1, 8);
        caps.vertexHighpInt.setSimulatedInt(16);
        caps.vertexMediumpInt.setSimulatedInt(10);
        caps.vertexLowpInt.setSimulatedInt(8);
        caps.fragmentHighpFloat.setSimulatedFloat(62, 16);
        caps.fragmentMediumpFloat.setSimulatedFloat(14, 10);
        caps.fragmentLowpFloat.setSimulatedFloat(1, 8);
        caps.fragmentHighpInt.setSimulatedInt(16);
        caps.fragmentMediumpInt.setSimulatedInt(10);
        caps.fragmentLowpInt.setSimulatedInt(8);

        // Table 6.20
        caps.maxVertexAttributes                              = 8;
        caps.maxVertexUniformVectors                          = 128;
        caps.maxVaryingVectors                                = 8;
        caps.maxCombinedTextureImageUnits                     = 8;
        caps.maxShaderTextureImageUnits[ShaderType::Fragment] = 8;
        caps.maxFragmentUniformVectors                        = 16;
        caps.maxRenderbufferSize                              = 1;

        // Table 3.35
        caps.maxSamples = 4;
    }

    if (clientVersion >= Version(3, 0))
    {
        // Table 6.28
        caps.maxElementIndex       = (1 << 24) - 1;
        caps.max3DTextureSize      = 256;
        caps.max2DTextureSize      = 2048;
        caps.maxArrayTextureLayers = 256;
        caps.maxLODBias            = 2.0f;
        caps.maxCubeMapTextureSize = 2048;
        caps.maxRenderbufferSize   = 2048;
        caps.maxDrawBuffers        = 4;
        caps.maxColorAttachments   = 4;
        caps.maxViewportWidth      = caps.max2DTextureSize;
        caps.maxViewportHeight     = caps.max2DTextureSize;

        // Table 6.29
        caps.compressedTextureFormats.push_back(GL_COMPRESSED_R11_EAC);
        caps.compressedTextureFormats.push_back(GL_COMPRESSED_SIGNED_R11_EAC);
        caps.compressedTextureFormats.push_back(GL_COMPRESSED_RG11_EAC);
        caps.compressedTextureFormats.push_back(GL_COMPRESSED_SIGNED_RG11_EAC);
        caps.compressedTextureFormats.push_back(GL_COMPRESSED_RGB8_ETC2);
        caps.compressedTextureFormats.push_back(GL_COMPRESSED_SRGB8_ETC2);
        caps.compressedTextureFormats.push_back(GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2);
        caps.compressedTextureFormats.push_back(GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2);
        caps.compressedTextureFormats.push_back(GL_COMPRESSED_RGBA8_ETC2_EAC);
        caps.compressedTextureFormats.push_back(GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC);
        caps.vertexHighpFloat.setIEEEFloat();
        caps.vertexHighpInt.setTwosComplementInt(32);
        caps.vertexMediumpInt.setTwosComplementInt(16);
        caps.vertexLowpInt.setTwosComplementInt(8);
        caps.fragmentHighpFloat.setIEEEFloat();
        caps.fragmentHighpInt.setSimulatedInt(32);
        caps.fragmentMediumpInt.setTwosComplementInt(16);
        caps.fragmentLowpInt.setTwosComplementInt(8);
        caps.maxServerWaitTimeout = 0;

        // Table 6.31
        caps.maxVertexAttributes                            = 16;
        caps.maxShaderUniformComponents[ShaderType::Vertex] = 1024;
        caps.maxVertexUniformVectors                        = 256;
        caps.maxShaderUniformBlocks[ShaderType::Vertex]     = limits::kMinimumShaderUniformBlocks;
        caps.maxVertexOutputComponents = limits::kMinimumVertexOutputComponents;
        caps.maxShaderTextureImageUnits[ShaderType::Vertex] = 16;

        // Table 6.32
        caps.maxShaderUniformComponents[ShaderType::Fragment] = 896;
        caps.maxFragmentUniformVectors                        = 224;
        caps.maxShaderUniformBlocks[ShaderType::Fragment]     = limits::kMinimumShaderUniformBlocks;
        caps.maxFragmentInputComponents                       = 60;
        caps.maxShaderTextureImageUnits[ShaderType::Fragment] = 16;
        caps.minProgramTexelOffset                            = -8;
        caps.maxProgramTexelOffset                            = 7;

        // Table 6.33
        caps.maxUniformBufferBindings     = 24;
        caps.maxUniformBlockSize          = 16384;
        caps.uniformBufferOffsetAlignment = 256;
        caps.maxCombinedUniformBlocks     = 24;
        caps.maxVaryingComponents         = 60;
        caps.maxVaryingVectors            = 15;
        caps.maxCombinedTextureImageUnits = 32;

        // Table 6.34
        caps.maxTransformFeedbackInterleavedComponents = 64;
        caps.maxTransformFeedbackSeparateAttributes    = 4;
        caps.maxTransformFeedbackSeparateComponents    = 4;
    }

    if (clientVersion >= Version(3, 1))
    {
        // Table 20.40
        caps.maxFramebufferWidth    = 2048;
        caps.maxFramebufferHeight   = 2048;
        caps.maxFramebufferSamples  = 4;
        caps.maxSampleMaskWords     = 1;
        caps.maxColorTextureSamples = 1;
        caps.maxDepthTextureSamples = 1;
        caps.maxIntegerSamples      = 1;

        // Table 20.41
        caps.maxVertexAttribRelativeOffset = 2047;
        caps.maxVertexAttribBindings       = 16;
        caps.maxVertexAttribStride         = 2048;

        // Table 20.43
        caps.maxShaderAtomicCounterBuffers[ShaderType::Vertex] = 0;
        caps.maxShaderAtomicCounters[ShaderType::Vertex]       = 0;
        caps.maxShaderImageUniforms[ShaderType::Vertex]        = 0;
        caps.maxShaderStorageBlocks[ShaderType::Vertex]        = 0;

        // Table 20.44
        caps.maxShaderUniformComponents[ShaderType::Fragment]    = 1024;
        caps.maxFragmentUniformVectors                           = 256;
        caps.maxShaderAtomicCounterBuffers[ShaderType::Fragment] = 0;
        caps.maxShaderAtomicCounters[ShaderType::Fragment]       = 0;
        caps.maxShaderImageUniforms[ShaderType::Fragment]        = 0;
        caps.maxShaderStorageBlocks[ShaderType::Fragment]        = 0;
        caps.minProgramTextureGatherOffset                       = caps.minProgramTexelOffset;
        caps.maxProgramTextureGatherOffset                       = caps.maxProgramTexelOffset;

        // Table 20.45
        caps.maxComputeWorkGroupCount                        = {{65535, 65535, 65535}};
        caps.maxComputeWorkGroupSize                         = {{128, 128, 64}};
        caps.maxComputeWorkGroupInvocations                  = 128;
        caps.maxShaderUniformBlocks[ShaderType::Compute]     = limits::kMinimumShaderUniformBlocks;
        caps.maxShaderTextureImageUnits[ShaderType::Compute] = 16;
        caps.maxComputeSharedMemorySize                      = 16384;
        caps.maxShaderUniformComponents[ShaderType::Compute] = 1024;
        caps.maxShaderAtomicCounterBuffers[ShaderType::Compute] = 1;
        caps.maxShaderAtomicCounters[ShaderType::Compute]       = 8;
        caps.maxShaderImageUniforms[ShaderType::Compute]        = 4;
        caps.maxShaderStorageBlocks[ShaderType::Compute]        = 4;

        // Table 20.46
        caps.maxUniformBufferBindings         = 36;
        caps.maxCombinedTextureImageUnits     = 48;
        caps.maxCombinedShaderOutputResources = 4;

        // Table 20.47
        caps.maxUniformLocations                = 1024;
        caps.maxAtomicCounterBufferBindings     = 1;
        caps.maxAtomicCounterBufferSize         = 32;
        caps.maxCombinedAtomicCounterBuffers    = 1;
        caps.maxCombinedAtomicCounters          = 8;
        caps.maxImageUnits                      = 4;
        caps.maxCombinedImageUniforms           = 4;
        caps.maxShaderStorageBufferBindings     = 4;
        caps.maxShaderStorageBlockSize          = 1 << 27;
        caps.maxCombinedShaderStorageBlocks     = 4;
        caps.shaderStorageBufferOffsetAlignment = 256;
    }

    if (clientVersion >= Version(3, 2))
    {
        // Table 21.40
        caps.lineWidthGranularity    = 1.0;
        caps.minMultisampleLineWidth = 1.0;
        caps.maxMultisampleLineWidth = 1.0;
    }

    if (extensions.blendFuncExtendedEXT)
    {
        caps.maxDualSourceDrawBuffers = 1;
    }

    if (extensions.textureRectangleANGLE)
    {
        caps.maxRectangleTextureSize = 64;
    }

    if (extensions.geometryShaderAny())
    {
        // Table 20.40 (GL_EXT_geometry_shader)
        caps.maxFramebufferLayers = 256;
        caps.layerProvokingVertex = GL_LAST_VERTEX_CONVENTION_EXT;

        // Table 20.43gs (GL_EXT_geometry_shader)
        caps.maxShaderUniformComponents[ShaderType::Geometry] = 1024;
        caps.maxShaderUniformBlocks[ShaderType::Geometry]     = limits::kMinimumShaderUniformBlocks;
        caps.maxGeometryInputComponents                       = 64;
        caps.maxGeometryOutputComponents                      = 64;
        caps.maxGeometryOutputVertices                        = 256;
        caps.maxGeometryTotalOutputComponents                 = 1024;
        caps.maxShaderTextureImageUnits[ShaderType::Geometry] = 16;
        caps.maxShaderAtomicCounterBuffers[ShaderType::Geometry] = 0;
        caps.maxShaderAtomicCounters[ShaderType::Geometry]       = 0;
        caps.maxShaderStorageBlocks[ShaderType::Geometry]        = 0;
        caps.maxGeometryShaderInvocations                        = 32;

        // Table 20.46 (GL_EXT_geometry_shader)
        caps.maxShaderImageUniforms[ShaderType::Geometry] = 0;

        // Table 20.46 (GL_EXT_geometry_shader)
        caps.maxUniformBufferBindings     = 48;
        caps.maxCombinedUniformBlocks     = 36;
        caps.maxCombinedTextureImageUnits = 64;
    }

    if (extensions.tessellationShaderAny())
    {
        // Table 20.43 "Implementation Dependent Tessellation Shader Limits"
        caps.maxTessControlInputComponents                          = 64;
        caps.maxTessControlOutputComponents                         = 64;
        caps.maxShaderTextureImageUnits[ShaderType::TessControl]    = 16;
        caps.maxShaderUniformComponents[ShaderType::TessControl]    = 1024;
        caps.maxTessControlTotalOutputComponents                    = 2048;
        caps.maxShaderImageUniforms[ShaderType::TessControl]        = 0;
        caps.maxShaderAtomicCounters[ShaderType::TessControl]       = 0;
        caps.maxShaderAtomicCounterBuffers[ShaderType::TessControl] = 0;

        caps.maxTessPatchComponents = 120;
        caps.maxPatchVertices       = 32;
        caps.maxTessGenLevel        = 64;

        caps.maxTessEvaluationInputComponents                          = 64;
        caps.maxTessEvaluationOutputComponents                         = 64;
        caps.maxShaderTextureImageUnits[ShaderType::TessEvaluation]    = 16;
        caps.maxShaderUniformComponents[ShaderType::TessEvaluation]    = 1024;
        caps.maxShaderImageUniforms[ShaderType::TessEvaluation]        = 0;
        caps.maxShaderAtomicCounters[ShaderType::TessEvaluation]       = 0;
        caps.maxShaderAtomicCounterBuffers[ShaderType::TessEvaluation] = 0;

        // Table 20.46 "Implementation Dependent Aggregate Shader Limits"
        caps.maxUniformBufferBindings     = 72;
        caps.maxCombinedUniformBlocks     = 60;
        caps.maxCombinedTextureImageUnits = 96;
    }

    for (ShaderType shaderType : AllShaderTypes())
    {
        caps.maxCombinedShaderUniformComponents[shaderType] =
            caps.maxShaderUniformBlocks[shaderType] *
                static_cast<GLuint>(caps.maxUniformBlockSize / 4) +
            caps.maxShaderUniformComponents[shaderType];
    }

    return caps;
}
}  // namespace gl

namespace egl
{

Caps::Caps() = default;

DisplayExtensions::DisplayExtensions() = default;

std::vector<std::string> DisplayExtensions::getStrings() const
{
    std::vector<std::string> extensionStrings;

    // clang-format off
    //                   | Extension name                                       | Supported flag                    | Output vector   |
    InsertExtensionString("EGL_EXT_create_context_robustness",                   createContextRobustness,            &extensionStrings);
    InsertExtensionString("EGL_ANGLE_d3d_share_handle_client_buffer",            d3dShareHandleClientBuffer,         &extensionStrings);
    InsertExtensionString("EGL_ANGLE_d3d_texture_client_buffer",                 d3dTextureClientBuffer,             &extensionStrings);
    InsertExtensionString("EGL_ANGLE_surface_d3d_texture_2d_share_handle",       surfaceD3DTexture2DShareHandle,     &extensionStrings);
    InsertExtensionString("EGL_ANGLE_query_surface_pointer",                     querySurfacePointer,                &extensionStrings);
    InsertExtensionString("EGL_ANGLE_window_fixed_size",                         windowFixedSize,                    &extensionStrings);
    InsertExtensionString("EGL_ANGLE_keyed_mutex",                               keyedMutex,                         &extensionStrings);
    InsertExtensionString("EGL_ANGLE_surface_orientation",                       surfaceOrientation,                 &extensionStrings);
    InsertExtensionString("EGL_ANGLE_direct_composition",                        directComposition,                  &extensionStrings);
    InsertExtensionString("EGL_ANGLE_windows_ui_composition",                    windowsUIComposition,               &extensionStrings);
    InsertExtensionString("EGL_NV_post_sub_buffer",                              postSubBuffer,                      &extensionStrings);
    InsertExtensionString("EGL_KHR_create_context",                              createContext,                      &extensionStrings);
    InsertExtensionString("EGL_KHR_image",                                       image,                              &extensionStrings);
    InsertExtensionString("EGL_KHR_image_base",                                  imageBase,                          &extensionStrings);
    InsertExtensionString("EGL_KHR_image_pixmap",                                imagePixmap,                        &extensionStrings);
    InsertExtensionString("EGL_EXT_image_gl_colorspace",                         imageGlColorspace,                  &extensionStrings);
    InsertExtensionString("EGL_KHR_gl_colorspace",                               glColorspace,                       &extensionStrings);
    InsertExtensionString("EGL_EXT_gl_colorspace_scrgb",                         glColorspaceScrgb,                  &extensionStrings);
    InsertExtensionString("EGL_EXT_gl_colorspace_scrgb_linear",                  glColorspaceScrgbLinear,            &extensionStrings);
    InsertExtensionString("EGL_EXT_gl_colorspace_display_p3",                    glColorspaceDisplayP3,              &extensionStrings);
    InsertExtensionString("EGL_EXT_gl_colorspace_display_p3_linear",             glColorspaceDisplayP3Linear,        &extensionStrings);
    InsertExtensionString("EGL_EXT_gl_colorspace_display_p3_passthrough",        glColorspaceDisplayP3Passthrough,   &extensionStrings);
    InsertExtensionString("EGL_ANGLE_colorspace_attribute_passthrough",          eglColorspaceAttributePassthroughANGLE,  &extensionStrings);
    InsertExtensionString("EGL_EXT_gl_colorspace_bt2020_linear",                 glColorspaceBt2020Linear,           &extensionStrings);
    InsertExtensionString("EGL_EXT_gl_colorspace_bt2020_pq",                     glColorspaceBt2020Pq,               &extensionStrings);
    InsertExtensionString("EGL_EXT_gl_colorspace_bt2020_hlg",                    glColorspaceBt2020Hlg,              &extensionStrings);
    InsertExtensionString("EGL_KHR_gl_texture_2D_image",                         glTexture2DImage,                   &extensionStrings);
    InsertExtensionString("EGL_KHR_gl_texture_cubemap_image",                    glTextureCubemapImage,              &extensionStrings);
    InsertExtensionString("EGL_KHR_gl_texture_3D_image",                         glTexture3DImage,                   &extensionStrings);
    InsertExtensionString("EGL_KHR_gl_renderbuffer_image",                       glRenderbufferImage,                &extensionStrings);
    InsertExtensionString("EGL_KHR_get_all_proc_addresses",                      getAllProcAddresses,                &extensionStrings);
    InsertExtensionString("EGL_KHR_stream",                                      stream,                             &extensionStrings);
    InsertExtensionString("EGL_KHR_stream_consumer_gltexture",                   streamConsumerGLTexture,            &extensionStrings);
    InsertExtensionString("EGL_NV_stream_consumer_gltexture_yuv",                streamConsumerGLTextureYUV,         &extensionStrings);
    InsertExtensionString("EGL_KHR_fence_sync",                                  fenceSync,                          &extensionStrings);
    InsertExtensionString("EGL_KHR_wait_sync",                                   waitSync,                           &extensionStrings);
    InsertExtensionString("EGL_ANGLE_stream_producer_d3d_texture",               streamProducerD3DTexture,           &extensionStrings);
    InsertExtensionString("EGL_ANGLE_create_context_webgl_compatibility",        createContextWebGLCompatibility,    &extensionStrings);
    InsertExtensionString("EGL_CHROMIUM_create_context_bind_generates_resource", createContextBindGeneratesResource, &extensionStrings);
    InsertExtensionString("EGL_CHROMIUM_sync_control",                           syncControlCHROMIUM,                &extensionStrings);
    InsertExtensionString("EGL_ANGLE_sync_control_rate",                         syncControlRateANGLE,               &extensionStrings);
    InsertExtensionString("EGL_KHR_swap_buffers_with_damage",                    swapBuffersWithDamage,              &extensionStrings);
    InsertExtensionString("EGL_EXT_pixel_format_float",                          pixelFormatFloat,                   &extensionStrings);
    InsertExtensionString("EGL_KHR_surfaceless_context",                         surfacelessContext,                 &extensionStrings);
    InsertExtensionString("EGL_ANGLE_display_texture_share_group",               displayTextureShareGroup,           &extensionStrings);
    InsertExtensionString("EGL_ANGLE_display_semaphore_share_group",             displaySemaphoreShareGroup,         &extensionStrings);
    InsertExtensionString("EGL_ANGLE_create_context_client_arrays",              createContextClientArrays,          &extensionStrings);
    InsertExtensionString("EGL_ANGLE_program_cache_control",                     programCacheControlANGLE,           &extensionStrings);
    InsertExtensionString("EGL_ANGLE_robust_resource_initialization",            robustResourceInitializationANGLE,  &extensionStrings);
    InsertExtensionString("EGL_ANGLE_iosurface_client_buffer",                   iosurfaceClientBuffer,              &extensionStrings);
    InsertExtensionString("EGL_ANGLE_metal_texture_client_buffer",               mtlTextureClientBuffer,             &extensionStrings);
    InsertExtensionString("EGL_ANGLE_create_context_extensions_enabled",         createContextExtensionsEnabled,     &extensionStrings);
    InsertExtensionString("EGL_ANDROID_presentation_time",                       presentationTime,                   &extensionStrings);
    InsertExtensionString("EGL_ANDROID_blob_cache",                              blobCache,                          &extensionStrings);
    InsertExtensionString("EGL_ANDROID_framebuffer_target",                      framebufferTargetANDROID,           &extensionStrings);
    InsertExtensionString("EGL_ANDROID_image_native_buffer",                     imageNativeBuffer,                  &extensionStrings);
    InsertExtensionString("EGL_ANDROID_get_frame_timestamps",                    getFrameTimestamps,                 &extensionStrings);
    InsertExtensionString("EGL_ANDROID_front_buffer_auto_refresh",               frontBufferAutoRefreshANDROID,      &extensionStrings);
    InsertExtensionString("EGL_ANGLE_timestamp_surface_attribute",               timestampSurfaceAttributeANGLE,     &extensionStrings);
    InsertExtensionString("EGL_ANDROID_recordable",                              recordable,                         &extensionStrings);
    InsertExtensionString("EGL_ANGLE_power_preference",                          powerPreference,                    &extensionStrings);
    InsertExtensionString("EGL_ANGLE_wait_until_work_scheduled",                 waitUntilWorkScheduled,             &extensionStrings);
    InsertExtensionString("EGL_ANGLE_image_d3d11_texture",                       imageD3D11Texture,                  &extensionStrings);
    InsertExtensionString("EGL_ANDROID_create_native_client_buffer",             createNativeClientBufferANDROID,    &extensionStrings);
    InsertExtensionString("EGL_ANDROID_get_native_client_buffer",                getNativeClientBufferANDROID,       &extensionStrings);
    InsertExtensionString("EGL_ANDROID_native_fence_sync",                       nativeFenceSyncANDROID,             &extensionStrings);
    InsertExtensionString("EGL_ANGLE_create_context_backwards_compatible",       createContextBackwardsCompatible,   &extensionStrings);
    InsertExtensionString("EGL_KHR_no_config_context",                           noConfigContext,                    &extensionStrings);
    InsertExtensionString("EGL_IMG_context_priority",                            contextPriority,                    &extensionStrings);
    InsertExtensionString("EGL_KHR_create_context_no_error",                     createContextNoError,               &extensionStrings);
    InsertExtensionString("EGL_EXT_image_dma_buf_import",                        imageDmaBufImportEXT,               &extensionStrings);
    InsertExtensionString("EGL_EXT_image_dma_buf_import_modifiers",              imageDmaBufImportModifiersEXT,      &extensionStrings);
    InsertExtensionString("EGL_NOK_texture_from_pixmap",                         textureFromPixmapNOK,               &extensionStrings);
    InsertExtensionString("EGL_NV_robustness_video_memory_purge",                robustnessVideoMemoryPurgeNV,       &extensionStrings);
    InsertExtensionString("EGL_KHR_reusable_sync",                               reusableSyncKHR,                    &extensionStrings);
    InsertExtensionString("EGL_ANGLE_external_context_and_surface",              externalContextAndSurface,          &extensionStrings);
    InsertExtensionString("EGL_EXT_buffer_age",                                  bufferAgeEXT,                       &extensionStrings);
    InsertExtensionString("EGL_KHR_mutable_render_buffer",                       mutableRenderBufferKHR,             &extensionStrings);
    InsertExtensionString("EGL_EXT_protected_content",                           protectedContentEXT,                &extensionStrings);
    InsertExtensionString("EGL_ANGLE_create_surface_swap_interval",              createSurfaceSwapIntervalANGLE,     &extensionStrings);
    InsertExtensionString("EGL_ANGLE_context_virtualization",                    contextVirtualizationANGLE,         &extensionStrings);
    InsertExtensionString("EGL_KHR_lock_surface3",                               lockSurface3KHR,                    &extensionStrings);
    InsertExtensionString("EGL_ANGLE_vulkan_image",                              vulkanImageANGLE,                   &extensionStrings);
    InsertExtensionString("EGL_ANGLE_metal_create_context_ownership_identity",   metalCreateContextOwnershipIdentityANGLE, &extensionStrings);
    InsertExtensionString("EGL_KHR_partial_update",                              partialUpdateKHR,                   &extensionStrings);
    InsertExtensionString("EGL_ANGLE_metal_shared_event_sync",                   mtlSyncSharedEventANGLE,            &extensionStrings);
    InsertExtensionString("EGL_ANGLE_global_fence_sync",                         globalFenceSyncANGLE,               &extensionStrings);
    InsertExtensionString("EGL_ANGLE_memory_usage_report",                       memoryUsageReportANGLE,             &extensionStrings);
    InsertExtensionString("EGL_EXT_surface_compression",                         surfaceCompressionEXT,              &extensionStrings);
    // clang-format on

    return extensionStrings;
}

DeviceExtensions::DeviceExtensions() = default;

std::vector<std::string> DeviceExtensions::getStrings() const
{
    std::vector<std::string> extensionStrings;

    // clang-format off
    //                   | Extension name                                 | Supported flag                | Output vector   |
    InsertExtensionString("EGL_ANGLE_device_d3d",                          deviceD3D,                      &extensionStrings);
    InsertExtensionString("EGL_ANGLE_device_d3d9",                         deviceD3D9,                     &extensionStrings);
    InsertExtensionString("EGL_ANGLE_device_d3d11",                        deviceD3D11,                    &extensionStrings);
    InsertExtensionString("EGL_ANGLE_device_cgl",                          deviceCGL,                      &extensionStrings);
    InsertExtensionString("EGL_ANGLE_device_metal",                        deviceMetal,                    &extensionStrings);
    InsertExtensionString("EGL_ANGLE_device_vulkan",                       deviceVulkan,                   &extensionStrings);
    InsertExtensionString("EGL_EXT_device_drm",                            deviceDrmEXT,                   &extensionStrings);
    InsertExtensionString("EGL_EXT_device_drm_render_node",                deviceDrmRenderNodeEXT,         &extensionStrings);

    // clang-format on

    return extensionStrings;
}

ClientExtensions::ClientExtensions()                              = default;
ClientExtensions::ClientExtensions(const ClientExtensions &other) = default;

std::vector<std::string> ClientExtensions::getStrings() const
{
    std::vector<std::string> extensionStrings;

    // clang-format off
    //                   | Extension name                                    | Supported flag                   | Output vector   |
    InsertExtensionString("EGL_EXT_client_extensions",                        clientExtensions,                   &extensionStrings);
    InsertExtensionString("EGL_EXT_device_query",                             deviceQueryEXT,                     &extensionStrings);
    InsertExtensionString("EGL_EXT_platform_base",                            platformBase,                       &extensionStrings);
    InsertExtensionString("EGL_EXT_platform_device",                          platformDevice,                     &extensionStrings);
    InsertExtensionString("EGL_KHR_platform_gbm",                             platformGbmKHR,                     &extensionStrings);
    InsertExtensionString("EGL_EXT_platform_wayland",                         platformWaylandEXT,                 &extensionStrings);
    InsertExtensionString("EGL_MESA_platform_surfaceless",                    platformSurfacelessMESA,            &extensionStrings);
    InsertExtensionString("EGL_ANGLE_platform_angle",                         platformANGLE,                      &extensionStrings);
    InsertExtensionString("EGL_ANGLE_platform_angle_d3d",                     platformANGLED3D,                   &extensionStrings);
    InsertExtensionString("EGL_ANGLE_platform_angle_d3d11on12",               platformANGLED3D11ON12,             &extensionStrings);
    InsertExtensionString("EGL_ANGLE_platform_angle_d3d_luid",                platformANGLED3DLUID,               &extensionStrings);
    InsertExtensionString("EGL_ANGLE_platform_angle_device_type_egl_angle",   platformANGLEDeviceTypeEGLANGLE,    &extensionStrings);
    InsertExtensionString("EGL_ANGLE_platform_angle_device_type_swiftshader", platformANGLEDeviceTypeSwiftShader, &extensionStrings);
    InsertExtensionString("EGL_ANGLE_platform_angle_opengl",                  platformANGLEOpenGL,                &extensionStrings);
    InsertExtensionString("EGL_ANGLE_platform_angle_null",                    platformANGLENULL,                  &extensionStrings);
    InsertExtensionString("EGL_ANGLE_platform_angle_webgpu",                  platformANGLEWebgpu,                &extensionStrings);
    InsertExtensionString("EGL_ANGLE_platform_angle_vulkan",                  platformANGLEVulkan,                &extensionStrings);
    InsertExtensionString("EGL_ANGLE_platform_angle_vulkan_device_uuid",      platformANGLEVulkanDeviceUUID,      &extensionStrings);
    InsertExtensionString("EGL_ANGLE_platform_angle_metal",                   platformANGLEMetal,                 &extensionStrings);
    InsertExtensionString("EGL_ANGLE_platform_device_context_volatile_cgl",   platformANGLEDeviceContextVolatileCgl, &extensionStrings);
    InsertExtensionString("EGL_ANGLE_platform_angle_device_id",               platformANGLEDeviceId,              &extensionStrings);
    InsertExtensionString("EGL_ANGLE_device_creation",                        deviceCreation,                     &extensionStrings);
    InsertExtensionString("EGL_ANGLE_device_creation_d3d11",                  deviceCreationD3D11,                &extensionStrings);
    InsertExtensionString("EGL_ANGLE_x11_visual",                             x11Visual,                          &extensionStrings);
    InsertExtensionString("EGL_ANGLE_experimental_present_path",              experimentalPresentPath,            &extensionStrings);
    InsertExtensionString("EGL_KHR_client_get_all_proc_addresses",            clientGetAllProcAddresses,          &extensionStrings);
    InsertExtensionString("EGL_KHR_debug",                                    debug,                              &extensionStrings);
    InsertExtensionString("EGL_ANGLE_feature_control",                        featureControlANGLE,                &extensionStrings);
    InsertExtensionString("EGL_ANGLE_display_power_preference",               displayPowerPreferenceANGLE,        &extensionStrings);
    InsertExtensionString("EGL_ANGLE_no_error",                               noErrorANGLE,                       &extensionStrings);
    // clang-format on

    return extensionStrings;
}

}  // namespace egl
