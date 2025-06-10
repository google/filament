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

#include <atomic>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warray-bounds"
#include <basisu_transcoder.h>
#pragma clang diagnostic pop

using namespace basist;
using namespace filament;

using TransferFunction = ktxreader::Ktx2Reader::TransferFunction;
using Result = ktxreader::Ktx2Reader::Result;
using Async = ktxreader::Ktx2Reader::Async;
using Buffer = std::vector<uint8_t>;

namespace {
struct FinalFormatInfo {
    const char* name; // <-- for debug purposes only
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
//     transcoder_texture_format::cTFBGR565   (note the blue/red swap)
//
static FinalFormatInfo getFinalFormatInfo(Texture::InternalFormat fmt) {
    using tif = Texture::InternalFormat;
    using tct = Texture::CompressedType;
    using tt = Texture::Type;
    using tf = Texture::Format;
    using ttf = transcoder_texture_format;
    const auto sRGB = TransferFunction::sRGB;
    const auto LINEAR = TransferFunction::LINEAR;
    switch (fmt) {
        case tif::ETC2_EAC_SRGBA8: return {"ETC2_EAC_SRGBA8", true, true, sRGB, ttf::cTFETC2_RGBA, tct::ETC2_EAC_RGBA8};
        case tif::ETC2_EAC_RGBA8:  return {"ETC2_EAC_RGBA8", true, true, LINEAR, ttf::cTFETC2_RGBA, tct::ETC2_EAC_SRGBA8};
        case tif::DXT1_SRGB: return {"DXT1_SRGB", true, true, sRGB, ttf::cTFBC1_RGB, tct::DXT1_RGB};
        case tif::DXT1_RGB: return {"DXT1_RGB", true, true, LINEAR, ttf::cTFBC1_RGB, tct::DXT1_SRGB};
        case tif::DXT5_SRGBA: return {"DXT5_SRGBA", true, true, sRGB, ttf::cTFBC3_RGBA, tct::DXT5_RGBA};
        case tif::DXT5_RGBA: return {"DXT5_RGBA", true, true, LINEAR, ttf::cTFBC3_RGBA, tct::DXT5_SRGBA};
        case tif::RED_RGTC1: return {"RED_RGTC1", true, true, LINEAR, ttf::cTFBC4_R, tct::RED_RGTC1};
        case tif::RED_GREEN_RGTC2: return {"RED_GREEN_RGTC2", true, true, LINEAR, ttf::cTFBC5_RG, tct::RED_GREEN_RGTC2};
        case tif::RGBA_BPTC_UNORM: return {"RGBA_BPTC_UNORM", true, true, LINEAR, ttf::cTFBC7_RGBA, tct::RGBA_BPTC_UNORM};
        case tif::SRGB_ALPHA_BPTC_UNORM: return {"SRGB_ALPHA_BPTC_UNORM", true, true, sRGB, ttf::cTFBC7_RGBA, tct::SRGB_ALPHA_BPTC_UNORM};
        case tif::SRGB8_ALPHA8_ASTC_4x4: return {"SRGB8_ALPHA8_ASTC_4x4", true, true, sRGB, ttf::cTFASTC_4x4_RGBA, tct::RGBA_ASTC_4x4};
        case tif::RGBA_ASTC_4x4: return {"RGBA_ASTC_4x4", true, true, LINEAR, ttf::cTFASTC_4x4_RGBA, tct::SRGB8_ALPHA8_ASTC_4x4};
        case tif::EAC_R11: return {"EAC_R11", true, true, LINEAR, ttf::cTFETC2_EAC_R11, tct::EAC_R11};

        // The following format is useful for normal maps.
        // Note that BasisU supports only the unsigned variant.
        case tif::EAC_RG11: return {"EAC_RG11", true, true, LINEAR, ttf::cTFETC2_EAC_RG11, tct::EAC_RG11};

        // Uncompressed formats.
        case tif::SRGB8_A8: return {"SRGB8_A8", true, false, sRGB, ttf::cTFRGBA32, {}, tt::UBYTE, tf::RGBA};
        case tif::RGBA8: return {"RGBA8", true, false, LINEAR, ttf::cTFRGBA32, {}, tt::UBYTE, tf::RGBA};
        case tif::RGB565: return {"RGB565", true, false, LINEAR, ttf::cTFRGB565, {}, tt::USHORT_565, tf::RGB};
        case tif::RGBA4: return {"RGBA4", true, false, LINEAR, ttf::cTFRGBA4444, {}, tt::USHORT, tf::RGBA};

        default: return {};
    }
}

// In theory we could pass "free" directly into the callback but doing so triggers ASAN warnings.
static void freeCallback(void* buf, size_t, void* userdata) {
    free(buf);
}

// This helper is used by both the asynchronous and synchronous API's.
static Result transcodeImageLevel(ktx2_transcoder& transcoder,
        ktx2_transcoder_state& transcoderState, Texture::InternalFormat format,
        uint32_t levelIndex, Texture::PixelBufferDescriptor** pbd)  {
    using basisu::texture_format;
    assert_invariant(levelIndex < KTX2_MAX_SUPPORTED_LEVEL_COUNT);
    const FinalFormatInfo formatInfo = getFinalFormatInfo(format);
    const texture_format destFormat = basis_get_basisu_texture_format(formatInfo.basisFormat);
    const uint32_t layerIndex = 0;
    const uint32_t faceIndex = 0;
    const uint32_t decodeFlags = 0;
    const uint32_t outputRowPitch = 0;
    const uint32_t outputRowCount = 0;
    const int channel0 = 0;
    const int channel1 = 0;

    basist::ktx2_image_level_info levelInfo;
    transcoder.get_image_level_info(levelInfo, levelIndex, layerIndex, faceIndex);

    if (formatInfo.isCompressed) {
        const uint32_t qwordsPerBlock = basisu::get_qwords_per_block(destFormat);
        const size_t byteCount = sizeof(uint64_t) * qwordsPerBlock * levelInfo.m_total_blocks;
        uint64_t* const blocks = (uint64_t*) malloc(byteCount);
        if (!transcoder.transcode_image_level(levelIndex, layerIndex, faceIndex, blocks,
                levelInfo.m_total_blocks, formatInfo.basisFormat, decodeFlags,
                outputRowPitch, outputRowCount, channel0,
                channel1, &transcoderState)) {
            return Result::COMPRESSED_TRANSCODE_FAILURE;
        }
        *pbd = new Texture::PixelBufferDescriptor(blocks,
                byteCount, formatInfo.compressedPixelDataType, byteCount, freeCallback);
        return Result::SUCCESS;
    }

    const uint32_t rowCount = levelInfo.m_orig_height;
    const uint32_t bytesPerPix = basis_get_bytes_per_block_or_pixel(formatInfo.basisFormat);
    const size_t byteCount = bytesPerPix * levelInfo.m_orig_width * rowCount;
    uint64_t* const rows = (uint64_t*) malloc(byteCount);
    if (!transcoder.transcode_image_level(levelIndex, layerIndex, faceIndex, rows,
            byteCount / bytesPerPix, formatInfo.basisFormat, decodeFlags,
            outputRowPitch, outputRowCount, channel0, channel1, &transcoderState)) {
        return Result::UNCOMPRESSED_TRANSCODE_FAILURE;
    }
    *pbd = new Texture::PixelBufferDescriptor(rows, byteCount,
            formatInfo.pixelDataFormat, formatInfo.pixelDataType, freeCallback);
    return Result::SUCCESS;
}

namespace ktxreader {

class FAsync : public Async {
public:
    FAsync(Texture* texture, Engine& engine, ktx2_transcoder* transcoder, Buffer&& buf) :
            mTexture(texture), mEngine(engine), mTranscoder(transcoder),
            mSourceBuffer(std::move(buf)) {}
    Texture* getTexture() const noexcept { return mTexture; }
    Result doTranscoding();
    void uploadImages();

protected:
    ~FAsync();

private:
    using TranscoderResult = std::atomic<Texture::PixelBufferDescriptor*>;

    // After each level is transcoded, the results are stashed in the following array until the
    // foreground thread calls uploadImages(). Each slot in the array corresponds to a single
    // miplevel in the texture.
    TranscoderResult mTranscoderResults[KTX2_MAX_SUPPORTED_LEVEL_COUNT] = {};

    Texture* const mTexture;
    Engine& mEngine;

    // We do not share the BasisU trancoder between Async objects. The BasisU transcoder
    // allows parallelization at "level" granularity, but does not permit parallelization at
    // "texture" granularity. i.e. the transcode_image_level() method is thread-safe but the
    // start_transcoding() method is not.
    std::unique_ptr<ktx2_transcoder> const mTranscoder;

    // Storage for the content of the KTX2 file.
    Buffer mSourceBuffer;
};

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

Result Ktx2Reader::requestFormat(Texture::InternalFormat format) noexcept {
    if (!getFinalFormatInfo(format).isSupported) {
        return Result::FORMAT_UNSUPPORTED;
    }
    for (Texture::InternalFormat fmt : mRequestedFormats) {
        if (fmt == format) {
            return Result::FORMAT_ALREADY_REQUESTED;
        }
    }
    mRequestedFormats.push_back(format);
    return Result::SUCCESS;
}

void Ktx2Reader::unrequestFormat(Texture::InternalFormat format) noexcept {
    for (auto iter = mRequestedFormats.begin(); iter != mRequestedFormats.end(); ++iter) {
        if (*iter == format) {
            mRequestedFormats.erase(iter);
            return;
        }
    }
}

Texture* Ktx2Reader::load(const void* data, size_t size, TransferFunction transfer) {
    Texture* texture = createTexture(mTranscoder, data, size, transfer);
    if (texture == nullptr) {
        return nullptr;
    }

    if (!mTranscoder->start_transcoding()) {
        mEngine.destroy(texture);
        if (!mQuiet) {
            utils::slog.e << "BasisU start_transcoding failed." << utils::io::endl;
        }
        return nullptr;
    }

    ktx2_transcoder_state basisThreadState;
    basisThreadState.clear();

    for (uint32_t levelIndex = 0, n = 1 /*mTranscoder->get_levels()*/; levelIndex < n; levelIndex++) {
        Texture::PixelBufferDescriptor* pbd;
        Result result = transcodeImageLevel(*mTranscoder, basisThreadState, texture->getFormat(),
                levelIndex, &pbd);
        if (UTILS_UNLIKELY(result != Result::SUCCESS)) {
            mEngine.destroy(texture);
            if (!mQuiet) {
                utils::slog.e << "Failed to transcode level " << levelIndex << utils::io::endl;
            }
            return nullptr;
        }
        texture->setImage(mEngine, levelIndex, std::move(*pbd));
    }
    return texture;
}

FAsync::~FAsync() {
    for (TranscoderResult& level : mTranscoderResults) {
        Texture::PixelBufferDescriptor* pbd = level.load();
        if (pbd) {
            delete pbd;
        }
    }
}

Result FAsync::doTranscoding() {
    ktx2_transcoder_state basisThreadState;
    basisThreadState.clear();
    for (uint32_t levelIndex = 0, n = 1 /*mTranscoder->get_levels()*/; levelIndex < n; levelIndex++) {
        Texture::PixelBufferDescriptor* pbd;
        Result result = transcodeImageLevel(*mTranscoder, basisThreadState, mTexture->getFormat(),
                levelIndex, &pbd);
        if (UTILS_UNLIKELY(result != Result::SUCCESS)) {
            return result;
        }
        mTranscoderResults[levelIndex].store(pbd);
    }
    return Result::SUCCESS;
}

void FAsync::uploadImages() {
    size_t levelIndex = 0;
    UTILS_NOUNROLL
    for (TranscoderResult& level : mTranscoderResults) {
        Texture::PixelBufferDescriptor* pbd = level.load();
        if (pbd) {
            level.store(nullptr);
            mTexture->setImage(mEngine, levelIndex, std::move(*pbd));
            delete pbd;
        }
        ++levelIndex;
    }
}

Async* Ktx2Reader::asyncCreate(const void* data, size_t size, TransferFunction transfer) {
    Buffer ktx2content((uint8_t*)data, (uint8_t*)data + size);
    ktx2_transcoder* transcoder = new ktx2_transcoder();
    Texture* texture = createTexture(transcoder, ktx2content.data(), ktx2content.size(), transfer);
    if (texture == nullptr) {
        delete transcoder;
        return nullptr;
    }
    if (!transcoder->start_transcoding()) {
        delete transcoder;
        mEngine.destroy(texture);
        return nullptr;
    }
    // There's no need to do any further work at this point but it should be noted that this is the
    // point at which we first come to know the number of miplevels, dimensions, etc. If we had a
    // dynamically sized array to store decoder results, we would reserve it here.
    return new FAsync(texture, mEngine, transcoder, std::move(ktx2content));
}

void Ktx2Reader::asyncDestroy(Async** async) {
    delete *async;
    *async = nullptr;
}

Texture* Ktx2Reader::createTexture(ktx2_transcoder* transcoder, const void* data, size_t size,
        TransferFunction transfer) {
    if (!transcoder->init(data, size)) {
        if (!mQuiet) {
            utils::slog.e << "BasisU transcoder init failed." << utils::io::endl;
        }
        return nullptr;
    }

    if (transcoder->get_dfd_transfer_func() == KTX2_KHR_DF_TRANSFER_LINEAR &&
            transfer == TransferFunction::sRGB) {
        if (!mQuiet) {
            utils::slog.e << "Source texture is marked linear, but client is requesting sRGB."
                    << utils::io::endl;
        }
        return nullptr;
    }

    if (transcoder->get_dfd_transfer_func() == KTX2_KHR_DF_TRANSFER_SRGB &&
            transfer == TransferFunction::LINEAR) {
        if (!mQuiet) {
            utils::slog.e << "Source texture is marked sRGB, but client is requesting linear."
                    << utils::io::endl;
        }
        return nullptr;
    }

    // TODO: support cubemaps. For now we use KTX1 for cubemaps because basisu does not support HDR.
    if (transcoder->get_faces() == 6) {
        if (!mQuiet) {
            utils::slog.e << "Cubemaps are not yet supported." << utils::io::endl;
        }
        return nullptr;
    }

    // TODO: support texture arrays.
    if (transcoder->get_layers() > 1) {
        if (!mQuiet) {
            utils::slog.e << "Texture arrays are not yet supported." << utils::io::endl;
        }
        return nullptr;
    }

    // First pass through, just to make sure we can transcode it.
    bool found = false;
    Texture::InternalFormat resolvedFormat;
    FinalFormatInfo info;
    for (Texture::InternalFormat requestedFormat : mRequestedFormats) {
        if (!Texture::isTextureFormatSupported(mEngine, requestedFormat)) {
            continue;
        }
        info = getFinalFormatInfo(requestedFormat);
        if (!info.isSupported || info.transferFunction != transfer) {
            continue;
        }
        if (!basis_is_format_supported(info.basisFormat, transcoder->get_format())) {
            continue;
        }
        const uint32_t layerIndex = 0;
        const uint32_t faceIndex = 0;
        for (uint32_t levelIndex = 0; levelIndex < 1 /*transcoder->get_levels()*/; levelIndex++) {
            basist::ktx2_image_level_info info;
            if (!transcoder->get_image_level_info(info, levelIndex, layerIndex, faceIndex)) {
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

    Texture* texture = Texture::Builder()
        .width(transcoder->get_width())
        .height(transcoder->get_height())
        .levels(1)
        .sampler(Texture::Sampler::SAMPLER_2D)
        .format(resolvedFormat)
        .build(mEngine);

    if (texture == nullptr && !mQuiet) {
        utils::slog.e << "Unable to construct texture using BasisU info." << utils::io::endl;
    }

    #if BASISU_FORCE_DEVEL_MESSAGES
    utils::slog.e << "Ktx2Reader created "
            << transcoder->get_width() << "x" << transcoder->get_height() << " texture with format "
            << info.name << utils::io::endl;
    #endif

    return texture;
}

Async::~Async() = default;

Texture* Async::getTexture() const noexcept {
    return static_cast<FAsync const*>(this)->getTexture();
}

Result Async::doTranscoding() {
    return static_cast<FAsync*>(this)->doTranscoding();
}

void Async::uploadImages() {
    return static_cast<FAsync*>(this)->uploadImages();
}

} // namespace ktxreader
