//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// mtl_format_utils.mm:
//      Implements Format conversion utilities classes that convert from angle formats
//      to respective MTLPixelFormat and MTLVertexFormat.
//

#include "libANGLE/renderer/metal/mtl_format_utils.h"

#include "common/debug.h"
#include "libANGLE/renderer/Format.h"
#include "libANGLE/renderer/load_functions_table.h"
#include "libANGLE/renderer/metal/DisplayMtl.h"

namespace rx
{
namespace mtl
{

namespace priv
{

template <typename T>
inline T *OffsetDataPointer(uint8_t *data, size_t y, size_t z, size_t rowPitch, size_t depthPitch)
{
    return reinterpret_cast<T *>(data + (y * rowPitch) + (z * depthPitch));
}

template <typename T>
inline const T *OffsetDataPointer(const uint8_t *data,
                                  size_t y,
                                  size_t z,
                                  size_t rowPitch,
                                  size_t depthPitch)
{
    return reinterpret_cast<const T *>(data + (y * rowPitch) + (z * depthPitch));
}

}  // namespace priv

namespace
{

bool OverrideTextureCaps(const DisplayMtl *display, angle::FormatID formatId, gl::TextureCaps *caps)
{
    // NOTE(hqle): Auto generate this.
    switch (formatId)
    {
        // NOTE: even though iOS devices don't support filtering depth textures, we still report as
        // supported here in order for the OES_depth_texture extension to be enabled.
        // During draw call, the filter modes will be converted to nearest.
        case angle::FormatID::D16_UNORM:
        case angle::FormatID::D24_UNORM_S8_UINT:
        case angle::FormatID::D32_FLOAT_S8X24_UINT:
        case angle::FormatID::D32_FLOAT:
        case angle::FormatID::D32_UNORM:
            caps->texturable = caps->filterable = caps->textureAttachment = caps->renderbuffer =
                true;
            return true;
        default:
            // NOTE(hqle): Handle more cases
            return false;
    }
}

void GenerateTextureCapsMap(const FormatTable &formatTable,
                            const DisplayMtl *display,
                            gl::TextureCapsMap *capsMapOut,
                            uint32_t *maxSamplesOut)
{
    auto &textureCapsMap = *capsMapOut;
    auto formatVerifier  = [&](const gl::InternalFormat &internalFormatInfo) {
        angle::FormatID angleFormatId =
            angle::Format::InternalFormatToID(internalFormatInfo.sizedInternalFormat);
        const Format &mtlFormat = formatTable.getPixelFormat(angleFormatId);
        if (!mtlFormat.valid())
        {
            return;
        }
        const FormatCaps &formatCaps = mtlFormat.getCaps();

        gl::TextureCaps textureCaps;

        // First let check whether we can override certain special cases.
        if (!OverrideTextureCaps(display, mtlFormat.intendedFormatId, &textureCaps))
        {
            // Fill the texture caps using pixel format's caps
            textureCaps.filterable = mtlFormat.getCaps().filterable;
            textureCaps.renderbuffer =
                mtlFormat.getCaps().colorRenderable || mtlFormat.getCaps().depthRenderable;
            textureCaps.texturable        = true;
            textureCaps.textureAttachment = textureCaps.renderbuffer;
            textureCaps.blendable         = mtlFormat.getCaps().blendable;
        }

        if (formatCaps.multisample)
        {
            constexpr uint32_t sampleCounts[] = {2, 4, 8};
            for (auto sampleCount : sampleCounts)
            {
                if ([display->getMetalDevice() supportsTextureSampleCount:sampleCount])
                {
                    textureCaps.sampleCounts.insert(sampleCount);
                    *maxSamplesOut = std::max(*maxSamplesOut, sampleCount);
                }
            }
        }

        textureCapsMap.set(mtlFormat.intendedFormatId, textureCaps);
    };

    // Texture caps map.
    const gl::FormatSet &internalFormats = gl::GetAllSizedInternalFormats();
    for (const auto internalFormat : internalFormats)
    {
        const gl::InternalFormat &internalFormatInfo =
            gl::GetSizedInternalFormatInfo(internalFormat);

        formatVerifier(internalFormatInfo);
    }
}

}  // namespace

// FormatBase implementation
const angle::Format &FormatBase::actualAngleFormat() const
{
    return angle::Format::Get(actualFormatId);
}

const angle::Format &FormatBase::intendedAngleFormat() const
{
    return angle::Format::Get(intendedFormatId);
}

// Format implementation
const gl::InternalFormat &Format::intendedInternalFormat() const
{
    return gl::GetSizedInternalFormatInfo(intendedAngleFormat().glInternalFormat);
}

const gl::InternalFormat &Format::actualInternalFormat() const
{
    return gl::GetSizedInternalFormatInfo(actualAngleFormat().glInternalFormat);
}

bool Format::needConversion(angle::FormatID srcFormatId) const
{
    if ((srcFormatId == angle::FormatID::BC1_RGB_UNORM_BLOCK &&
         actualFormatId == angle::FormatID::BC1_RGBA_UNORM_BLOCK) ||
        (srcFormatId == angle::FormatID::BC1_RGB_UNORM_SRGB_BLOCK &&
         actualFormatId == angle::FormatID::BC1_RGBA_UNORM_SRGB_BLOCK))
    {
        // When texture swizzling is available, DXT1 RGB format will be swizzled with RGB1.
        // WebGL allows unswizzled mapping when swizzling is not available. No need to convert.
        return false;
    }
    return srcFormatId != actualFormatId;
}

bool Format::isPVRTC() const
{
    switch (metalFormat)
    {
        case MTLPixelFormatPVRTC_RGB_2BPP:
        case MTLPixelFormatPVRTC_RGB_2BPP_sRGB:
        case MTLPixelFormatPVRTC_RGB_4BPP:
        case MTLPixelFormatPVRTC_RGB_4BPP_sRGB:
        case MTLPixelFormatPVRTC_RGBA_2BPP:
        case MTLPixelFormatPVRTC_RGBA_2BPP_sRGB:
        case MTLPixelFormatPVRTC_RGBA_4BPP:
        case MTLPixelFormatPVRTC_RGBA_4BPP_sRGB:
            return true;
        default:
            return false;
    }
}

// FormatTable implementation
angle::Result FormatTable::initialize(const DisplayMtl *display)
{
    mMaxSamples = 0;

    // Initialize native format caps
    initNativeFormatCaps(display);

    for (size_t i = 0; i < angle::kNumANGLEFormats; ++i)
    {
        const auto formatId = static_cast<angle::FormatID>(i);

        mPixelFormatTable[i].init(display, formatId);
        mPixelFormatTable[i].caps = mNativePixelFormatCapsTable[mPixelFormatTable[i].metalFormat];

        if (mPixelFormatTable[i].actualFormatId != mPixelFormatTable[i].intendedFormatId)
        {
            mPixelFormatTable[i].textureLoadFunctions = angle::GetLoadFunctionsMap(
                mPixelFormatTable[i].intendedAngleFormat().glInternalFormat,
                mPixelFormatTable[i].actualFormatId);
        }

        mVertexFormatTables[0][i].init(formatId, false);
        mVertexFormatTables[1][i].init(formatId, true);
    }

    // TODO(anglebug.com/40096755): unmerged change from WebKit was here -
    // D24S8 fallback to D32_FLOAT_S8X24_UINT, since removed.

    return angle::Result::Continue;
}

void FormatTable::generateTextureCaps(const DisplayMtl *display, gl::TextureCapsMap *capsMapOut)
{
    GenerateTextureCapsMap(*this, display, capsMapOut, &mMaxSamples);
}

const Format &FormatTable::getPixelFormat(angle::FormatID angleFormatId) const
{
    return mPixelFormatTable[static_cast<size_t>(angleFormatId)];
}
const FormatCaps &FormatTable::getNativeFormatCaps(MTLPixelFormat mtlFormat) const
{
    ASSERT(mNativePixelFormatCapsTable.count(mtlFormat));
    return mNativePixelFormatCapsTable.at(mtlFormat);
}
const VertexFormat &FormatTable::getVertexFormat(angle::FormatID angleFormatId,
                                                 bool tightlyPacked) const
{
    auto tableIdx = tightlyPacked ? 1 : 0;
    return mVertexFormatTables[tableIdx][static_cast<size_t>(angleFormatId)];
}

void FormatTable::setFormatCaps(MTLPixelFormat formatId,
                                bool filterable,
                                bool writable,
                                bool blendable,
                                bool multisample,
                                bool resolve,
                                bool colorRenderable)
{
    setFormatCaps(formatId, filterable, writable, blendable, multisample, resolve, colorRenderable,
                  false, 0);
}
void FormatTable::setFormatCaps(MTLPixelFormat formatId,
                                bool filterable,
                                bool writable,
                                bool blendable,
                                bool multisample,
                                bool resolve,
                                bool colorRenderable,
                                NSUInteger pixelBytes,
                                NSUInteger channels)
{
    setFormatCaps(formatId, filterable, writable, blendable, multisample, resolve, colorRenderable,
                  false, pixelBytes, channels);
}

void FormatTable::setFormatCaps(MTLPixelFormat formatId,
                                bool filterable,
                                bool writable,
                                bool blendable,
                                bool multisample,
                                bool resolve,
                                bool colorRenderable,
                                bool depthRenderable)
{
    setFormatCaps(formatId, filterable, writable, blendable, multisample, resolve, colorRenderable,
                  depthRenderable, 0, 0);
}
void FormatTable::setFormatCaps(MTLPixelFormat id,
                                bool filterable,
                                bool writable,
                                bool blendable,
                                bool multisample,
                                bool resolve,
                                bool colorRenderable,
                                bool depthRenderable,
                                NSUInteger pixelBytes,
                                NSUInteger channels)
{
    mNativePixelFormatCapsTable[id].filterable      = filterable;
    mNativePixelFormatCapsTable[id].writable        = writable;
    mNativePixelFormatCapsTable[id].colorRenderable = colorRenderable;
    mNativePixelFormatCapsTable[id].depthRenderable = depthRenderable;
    mNativePixelFormatCapsTable[id].blendable       = blendable;
    mNativePixelFormatCapsTable[id].multisample     = multisample;
    mNativePixelFormatCapsTable[id].resolve         = resolve;
    mNativePixelFormatCapsTable[id].pixelBytes      = pixelBytes;
    mNativePixelFormatCapsTable[id].pixelBytesMSAA  = pixelBytes;
    mNativePixelFormatCapsTable[id].channels        = channels;
    if (channels != 0)
        mNativePixelFormatCapsTable[id].alignment = MAX(pixelBytes / channels, 1U);
}

void FormatTable::setCompressedFormatCaps(MTLPixelFormat formatId, bool filterable)
{
    setFormatCaps(formatId, filterable, false, false, false, false, false, false);
}

void FormatTable::adjustFormatCapsForDevice(const mtl::ContextDevice &device,
                                            MTLPixelFormat id,
                                            bool supportsiOS2,
                                            bool supportsiOS4)
{
#if !(TARGET_OS_OSX || TARGET_OS_MACCATALYST)

    NSUInteger pixelBytesRender     = mNativePixelFormatCapsTable[id].pixelBytes;
    NSUInteger pixelBytesRenderMSAA = mNativePixelFormatCapsTable[id].pixelBytesMSAA;
    NSUInteger alignment            = mNativePixelFormatCapsTable[id].alignment;

// Override the current pixelBytesRender
#    define SPECIFIC(_pixelFormat, _pixelBytesRender)                                            \
        case _pixelFormat:                                                                       \
            pixelBytesRender     = _pixelBytesRender;                                            \
            pixelBytesRenderMSAA = _pixelBytesRender;                                            \
            alignment =                                                                          \
                supportsiOS4 ? _pixelBytesRender / mNativePixelFormatCapsTable[id].channels : 4; \
            break
// Override the current pixel bytes render, and MSAA
#    define SPECIFIC_MSAA(_pixelFormat, _pixelBytesRender, _pixelBytesRenderMSAA)                \
        case _pixelFormat:                                                                       \
            pixelBytesRender     = _pixelBytesRender;                                            \
            pixelBytesRenderMSAA = _pixelBytesRenderMSAA;                                        \
            alignment =                                                                          \
                supportsiOS4 ? _pixelBytesRender / mNativePixelFormatCapsTable[id].channels : 4; \
            break
// Override the current pixelBytesRender, and alignment
#    define SPECIFIC_ALIGN(_pixelFormat, _pixelBytesRender, _alignment) \
        case _pixelFormat:                                              \
            pixelBytesRender     = _pixelBytesRender;                   \
            pixelBytesRenderMSAA = _pixelBytesRender;                   \
            alignment            = _alignment;                          \
            break

    if (!mNativePixelFormatCapsTable[id].compressed)
    {
        // On AppleGPUFamily4+, there is no 4byte minimum requirement for render targets
        uint32_t minSize     = supportsiOS4 ? 1U : 4U;
        pixelBytesRender     = MAX(mNativePixelFormatCapsTable[id].pixelBytes, minSize);
        pixelBytesRenderMSAA = pixelBytesRender;
        alignment =
            supportsiOS4 ? MAX(pixelBytesRender / mNativePixelFormatCapsTable[id].channels, 1U) : 4;
    }

    // This list of tables starts from a general multi-platform table,
    // to specific platforms (i.e. ios2, ios4) inheriting from the previous tables

    // Start off with the general case
    switch ((NSUInteger)id)
    {
        SPECIFIC(MTLPixelFormatB5G6R5Unorm, 4U);
        SPECIFIC(MTLPixelFormatA1BGR5Unorm, 4U);
        SPECIFIC(MTLPixelFormatABGR4Unorm, 4U);
        SPECIFIC(MTLPixelFormatBGR5A1Unorm, 4U);

        SPECIFIC(MTLPixelFormatRGBA8Unorm, 4U);
        SPECIFIC(MTLPixelFormatBGRA8Unorm, 4U);

        SPECIFIC_MSAA(MTLPixelFormatRGBA8Unorm_sRGB, 4U, 8U);
        SPECIFIC_MSAA(MTLPixelFormatBGRA8Unorm_sRGB, 4U, 8U);
        SPECIFIC_MSAA(MTLPixelFormatRGBA8Snorm, 4U, 8U);
        SPECIFIC_MSAA(MTLPixelFormatRGB10A2Uint, 4U, 8U);

        SPECIFIC(MTLPixelFormatRGB10A2Unorm, 8U);
        SPECIFIC(MTLPixelFormatBGR10A2Unorm, 8U);

        SPECIFIC(MTLPixelFormatRG11B10Float, 8U);

        SPECIFIC(MTLPixelFormatRGB9E5Float, 8U);

        SPECIFIC(MTLPixelFormatStencil8, 1U);
    }

    // Override based ios2
    if (supportsiOS2)
    {
        switch ((NSUInteger)id)
        {
            SPECIFIC(MTLPixelFormatB5G6R5Unorm, 8U);
            SPECIFIC(MTLPixelFormatA1BGR5Unorm, 8U);
            SPECIFIC(MTLPixelFormatABGR4Unorm, 8U);
            SPECIFIC(MTLPixelFormatBGR5A1Unorm, 8U);
            SPECIFIC_MSAA(MTLPixelFormatRGBA8Unorm, 4U, 8U);
            SPECIFIC_MSAA(MTLPixelFormatBGRA8Unorm, 4U, 8U);
        }
    }

    // Override based on ios4
    if (supportsiOS4)
    {
        switch ((NSUInteger)id)
        {
            SPECIFIC_ALIGN(MTLPixelFormatB5G6R5Unorm, 6U, 2U);
            SPECIFIC(MTLPixelFormatRGBA8Unorm, 4U);
            SPECIFIC(MTLPixelFormatBGRA8Unorm, 4U);

            SPECIFIC(MTLPixelFormatRGBA8Unorm_sRGB, 4U);
            SPECIFIC(MTLPixelFormatBGRA8Unorm_sRGB, 4U);

            SPECIFIC(MTLPixelFormatRGBA8Snorm, 4U);

            SPECIFIC_ALIGN(MTLPixelFormatRGB10A2Unorm, 4U, 4U);
            SPECIFIC_ALIGN(MTLPixelFormatBGR10A2Unorm, 4U, 4U);
            SPECIFIC(MTLPixelFormatRGB10A2Uint, 8U);

            SPECIFIC_ALIGN(MTLPixelFormatRG11B10Float, 4U, 4U);

            SPECIFIC_ALIGN(MTLPixelFormatRGB9E5Float, 4U, 4U);
        }
    }
    mNativePixelFormatCapsTable[id].pixelBytes     = pixelBytesRender;
    mNativePixelFormatCapsTable[id].pixelBytesMSAA = pixelBytesRenderMSAA;
    mNativePixelFormatCapsTable[id].alignment      = alignment;

#    undef SPECIFIC
#    undef SPECIFIC_ALIGN
#    undef SPECIFIC_MSAA
#endif
    // macOS does not need to perform any additoinal adjustment. These values are only used to check
    // valid MRT sizes on iOS.
}

void FormatTable::initNativeFormatCaps(const DisplayMtl *display)
{
    initNativeFormatCapsAutogen(display);
}

}  // namespace mtl
}  // namespace rx
