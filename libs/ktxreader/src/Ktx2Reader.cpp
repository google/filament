/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ktxreader/Ktx2Reader.h>

#include <filament/Engine.h>
#include <filament/Texture.h>

#include <utils/Log.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warray-bounds"
#include <basisu_transcoder.h>
#pragma clang diagnostic pop

using namespace basist;
using namespace filament;

using TransferFunction = ktxreader::Ktx2Reader::TransferFunction;

namespace {
struct FinalFormatInfo {
    bool isSupported;
    bool isCompressed;
    TransferFunction transferFunction;
    transcoder_texture_format basisFormat;
    Texture::CompressedType compressedPixelDataType;
    Texture::Type pixelDataType;
    Texture::Format pixelDataFormat;
};
}

// This function returns various information about a Filament internal format, most notably its
// equivalent BasisU enumerant.
//
// Return by value isn't expensive here due to copy elision.
//
// Note that Filament's internal format list mimics the Vulkan format list, which
// embeds transfer function information (i.e. sRGB or not) into the format, whereas
// the basis format list does not.
//
// The following formats supported by BasisU but are not supported by Filament.
//
//     transcoder_texture_format::cTFETC1_RGB
//     transcoder_texture_format::cTFATC_RGB
//     transcoder_texture_format::cTFATC_RGBA
//     transcoder_texture_format::cTFFXT1_RGB
//     transcoder_texture_format::cTFPVRTC2_4_RGB
//     transcoder_texture_format::cTFPVRTC2_4_RGBA
//     transcoder_texture_format::cTFPVRTC1_4_RGB
//     transcoder_texture_format::cTFPVRTC1_4_RGBA
//     transcoder_texture_format::cTFBC4_R
//     transcoder_texture_format::cTFBC5_RG
//     transcoder_texture_format::cTFBC7_RGBA (this format would add size bloat to the transcoder)
//     transcoder_texture_format::cTFBGR565   (note the blue/red swap)
//
static FinalFormatInfo getFinalFormatInfo(Texture::InternalFormat fmt) {
    using tif = Texture::InternalFormat;
    using tct = Texture::CompressedType;
    using tt = Texture::Type;
    using tf = Texture::Format;
    using ttf = transcoder_texture_format;
    const auto sRGB = ktxreader::Ktx2Reader::sRGB;
    const auto LINEAR = ktxreader::Ktx2Reader::LINEAR;
    switch (fmt) {
        case tif::ETC2_EAC_SRGBA8: return {true, true, sRGB, ttf::cTFETC2_RGBA, tct::ETC2_EAC_RGBA8};
        case tif::ETC2_EAC_RGBA8:  return {true, true, LINEAR, ttf::cTFETC2_RGBA, tct::ETC2_EAC_SRGBA8};
        case tif::DXT1_SRGB: return {true, true, sRGB, ttf::cTFBC1_RGB, tct::DXT1_RGB};
        case tif::DXT1_RGB: return {true, true, LINEAR, ttf::cTFBC1_RGB, tct::DXT1_SRGB};
        case tif::DXT3_SRGBA: return {true, true, sRGB, ttf::cTFBC3_RGBA, tct::DXT3_RGBA};
        case tif::DXT3_RGBA: return {true, true, LINEAR, ttf::cTFBC3_RGBA, tct::DXT3_SRGBA};
        case tif::SRGB8_ALPHA8_ASTC_4x4: return {true, true, sRGB, ttf::cTFASTC_4x4_RGBA, tct::RGBA_ASTC_4x4};
        case tif::RGBA_ASTC_4x4: return {true, true, LINEAR, ttf::cTFASTC_4x4_RGBA, tct::SRGB8_ALPHA8_ASTC_4x4};
        case tif::EAC_R11: return {true, true, LINEAR, ttf::cTFETC2_EAC_R11, tct::EAC_R11};

        // The following format is useful for normal maps.
        // Note that BasisU supports only the unsigned variant.
        case tif::EAC_RG11: return {true, true, LINEAR, ttf::cTFETC2_EAC_RG11, tct::EAC_RG11};

        // Uncompressed formats.
        case tif::SRGB8_A8: return {true, false, sRGB, ttf::cTFRGBA32, {}, tt::UBYTE, tf::RGBA};
        case tif::RGBA8: return {true, false, LINEAR, ttf::cTFRGBA32, {}, tt::UBYTE, tf::RGBA};
        case tif::RGB565: return {true, false, LINEAR, ttf::cTFRGB565, {}, tt::USHORT_565, tf::RGB};
        case tif::RGBA4: return {true, false, LINEAR, ttf::cTFRGBA4444, {}, tt::USHORT, tf::RGBA};

        default: return {false};
    }
}

namespace ktxreader {

Ktx2Reader::Ktx2Reader(Engine& engine, bool quiet) :
    mEngine(engine),
    mQuiet(quiet),
    mTranscoder(new ktx2_transcoder()) {
    mRequestedFormats.reserve((size_t) transcoder_texture_format::cTFTotalTextureFormats);
    basisu_transcoder_init();
}

Ktx2Reader::~Ktx2Reader() {
    delete mTranscoder;
}

bool Ktx2Reader::requestFormat(Texture::InternalFormat format) {
    if (!getFinalFormatInfo(format).isSupported) {
        return false;
    }
    for (Texture::InternalFormat fmt : mRequestedFormats) {
        if (fmt == format) {
            return false;
        }
    }
    mRequestedFormats.push_back(format);
    return true;
}

void Ktx2Reader::unrequestFormat(Texture::InternalFormat format) {
    for (auto iter = mRequestedFormats.begin(); iter != mRequestedFormats.end(); ++iter) {
        if (*iter == format) {
            mRequestedFormats.erase(iter);
            return;
        }
    }
}

Texture* Ktx2Reader::load(const uint8_t* data, size_t size, TransferFunction transfer) {
    if (!mTranscoder->init(data, size)) {
        if (!mQuiet) {
            utils::slog.e << "BasisU transcoder init failed." << utils::io::endl;
        }
        return nullptr;
    }

    if (mTranscoder->get_dfd_transfer_func() == KTX2_KHR_DF_TRANSFER_LINEAR && transfer == sRGB) {
        if (!mQuiet) {
            utils::slog.e << "Source texture is marked linear, but client is requesting sRGB."
                    << utils::io::endl;
        }
        return nullptr;
    }

    if (mTranscoder->get_dfd_transfer_func() == KTX2_KHR_DF_TRANSFER_SRGB && transfer == LINEAR) {
        if (!mQuiet) {
            utils::slog.e << "Source texture is marked sRGB, but client is requesting linear."
                    << utils::io::endl;
        }
        return nullptr;
    }

    if (!mTranscoder->start_transcoding()) {
        if (!mQuiet) {
            utils::slog.e << "BasisU start_transcoding failed." << utils::io::endl;
        }
        return nullptr;
    }

    // TODO: support cubemaps. For now we use KTX1 for cubemaps because basisu does not support HDR.
    if (mTranscoder->get_faces() == 6) {
        if (!mQuiet) {
            utils::slog.e << "Cubemaps are not yet supported." << utils::io::endl;
        }
        return nullptr;
    }

    // TODO: support texture arrays.
    if (mTranscoder->get_layers() > 1) {
        if (!mQuiet) {
            utils::slog.e << "Texture arrays are not yet supported." << utils::io::endl;
        }
        return nullptr;
    }

    // Fierst pass through, just to make sure we can transcode it.
    bool found = false;
    Texture::InternalFormat resolvedFormat;
    for (Texture::InternalFormat requestedFormat : mRequestedFormats) {
        if (!Texture::isTextureFormatSupported(mEngine, requestedFormat)) {
            continue;
        }
        const auto info = getFinalFormatInfo(requestedFormat);
        if (!info.isSupported || info.transferFunction != transfer) {
            continue;
        }
        if (!basis_is_format_supported(info.basisFormat, mTranscoder->get_format())) {
            continue;
        }
        const uint32_t layerIndex = 0;
        const uint32_t faceIndex = 0;
        for (uint32_t levelIndex = 0; levelIndex < mTranscoder->get_levels(); levelIndex++) {
            basist::ktx2_image_level_info info;
            if (!mTranscoder->get_image_level_info(info, levelIndex, layerIndex, faceIndex)) {
                continue;
            }
        }
        found = true;
        resolvedFormat = requestedFormat;
        break;
    }

    if (!found) {
        if (!mQuiet) {
            utils::slog.e << "Unable to decode any of the requested formats." << utils::io::endl;
        }
        return nullptr;
    }

    const auto formatInfo = getFinalFormatInfo(resolvedFormat);

    Texture* texture = Texture::Builder()
        .width(mTranscoder->get_width())
        .height(mTranscoder->get_height())
        .levels(mTranscoder->get_levels())
        .sampler(Texture::Sampler::SAMPLER_2D)
        .format(resolvedFormat)
        .build(mEngine);

    // In theory we could pass "free" directly into the callback but that triggers ASAN warnings.
    Texture::PixelBufferDescriptor::Callback cb = [](void* buf, size_t, void* userdata) {
        free(buf);
    };

    const uint32_t layerIndex = 0;
    const uint32_t faceIndex = 0;
    for (uint32_t levelIndex = 0; levelIndex < mTranscoder->get_levels(); levelIndex++) {
        basist::ktx2_image_level_info levelInfo;
        mTranscoder->get_image_level_info(levelInfo, levelIndex, layerIndex, faceIndex);
        const basisu::texture_format destFormat =
                basis_get_basisu_texture_format(formatInfo.basisFormat);
        if (formatInfo.isCompressed) {
            const uint32_t qwordsPerBlock = basisu::get_qwords_per_block(destFormat);
            const size_t byteCount = sizeof(uint64_t) * qwordsPerBlock * levelInfo.m_total_blocks;
            uint64_t* const blocks = (uint64_t*) malloc(byteCount);
            const uint32_t flags = 0;
            if (!mTranscoder->transcode_image_level(levelIndex, layerIndex, faceIndex, blocks,
                    levelInfo.m_total_blocks, formatInfo.basisFormat, flags)) {
                utils::slog.e << "Failed to transcode level " << levelIndex << utils::io::endl;
                return nullptr;
            }
            Texture::PixelBufferDescriptor pbd(blocks, byteCount,
                    formatInfo.compressedPixelDataType, byteCount, cb, nullptr);
            texture->setImage(mEngine, levelIndex, std::move(pbd));
        } else {
            // The transcoder still does work even for uncompressed formats, because of zstd.
            const uint32_t rowCount = levelInfo.m_orig_height;
            const uint32_t bytesPerPix = basis_get_bytes_per_block_or_pixel(formatInfo.basisFormat);
            const size_t byteCount = bytesPerPix * levelInfo.m_orig_width * rowCount;
            uint64_t* const rows = (uint64_t*) malloc(byteCount);
            const uint32_t flags = 0;
            if (!mTranscoder->transcode_image_level(levelIndex, layerIndex, faceIndex, rows,
                    byteCount / bytesPerPix, formatInfo.basisFormat, flags)) {
                utils::slog.e << "Failed to transcode level " << levelIndex << utils::io::endl;
                return nullptr;
            }
            Texture::PixelBufferDescriptor pbd(rows, byteCount, formatInfo.pixelDataFormat,
                    formatInfo.pixelDataType, cb, nullptr);
            texture->setImage(mEngine, levelIndex, std::move(pbd));
        }
    }

    return texture;
}

} // namespace ktxreader
