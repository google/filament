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

#include <imageio/BasisEncoder.h>

#include <image/ColorTransform.h>
#include <image/ImageOps.h>
#include <utils/debug.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warray-bounds"
#include <basisu_comp.h>
#pragma clang diagnostic pop

namespace image {

using Builder = BasisEncoder::Builder;

struct BasisEncoderBuilderImpl {
    basisu::basis_compressor_params params = {};
    bool grayscale = false;
    bool linear = false;
    bool normals = false;
    bool quiet = false;
    size_t jobs = 4;
    bool error = false;
};

struct BasisEncoderImpl {
    basisu::basis_compressor* encoder;
    basisu::job_pool* jobs;
    bool quiet;
};

Builder::Builder(size_t mipCount, size_t layerCount) noexcept : mImpl(new BasisEncoderBuilderImpl) {
    const bool multiple = mImpl->params.m_source_images.size() > 1;
    mImpl->params.m_tex_type = multiple ? basist::cBASISTexType2DArray : basist::cBASISTexType2D;
    mImpl->params.m_uastc = true;
    mImpl->params.m_source_images.resize(layerCount);
    mImpl->params.m_source_mipmap_images.resize(layerCount);
    if (mipCount > 1) {
        for (size_t layer = 0; layer < layerCount; ++layer) {
            mImpl->params.m_source_mipmap_images[layer].resize(mipCount - 1);
        }
    }
}

Builder::~Builder() noexcept { delete mImpl; }

Builder::Builder(Builder&& that) noexcept {
    std::swap(mImpl, that.mImpl);
}

Builder& Builder::operator=(Builder&& that) noexcept {
    std::swap(mImpl, that.mImpl);
    return *this;
}

Builder& Builder::miplevel(size_t level, size_t layer, const LinearImage& floatImage) noexcept {
    if (layer >= mImpl->params.m_source_images.size()) {
        assert_invariant(false);
        mImpl->error = true;
        return *this;
    }

    auto& basisBaseLevel = mImpl->params.m_source_images[layer];
    auto& basisMipmaps = mImpl->params.m_source_mipmap_images[layer];

    if (level >= basisMipmaps.size() + 1) {
        assert_invariant(false);
        mImpl->error = true;
        return *this;
    }
    basisu::image* basisImage = level == 0 ? &basisBaseLevel : &basisMipmaps[level - 1];

    LinearImage sourceImage = mImpl->normals ? vectorsToColors(floatImage) : floatImage;

    const bool applyTransferFunction = !mImpl->linear;

    if (mImpl->grayscale) {
        std::unique_ptr<uint8_t[]> data = applyTransferFunction ?
            fromLinearTosRGB<uint8_t, 1>(sourceImage) : fromLinearToGrayscale<uint8_t>(sourceImage);
        basisImage->init(data.get(), floatImage.getWidth(), floatImage.getHeight(), 1);
    } else if (sourceImage.getChannels() == 4) {
        std::unique_ptr<uint8_t[]> data = applyTransferFunction ?
            fromLinearTosRGB<uint8_t, 4>(sourceImage) : fromLinearToRGB<uint8_t, 4>(sourceImage);
        basisImage->init(data.get(), floatImage.getWidth(), floatImage.getHeight(), 4);
    } else if (sourceImage.getChannels() == 3) {
        std::unique_ptr<uint8_t[]> data = applyTransferFunction ?
            fromLinearTosRGB<uint8_t, 3>(sourceImage) : fromLinearToRGB<uint8_t, 3>(sourceImage);
        basisImage->init(data.get(), floatImage.getWidth(), floatImage.getHeight(), 3);
    } else {
        assert_invariant(false);
        mImpl->error = true;
        return *this;
    }

    return *this;
}

Builder& Builder::linear(bool enabled) noexcept {
    mImpl->linear = enabled;
    return *this;
}

Builder& Builder::cubemap(bool enabled) noexcept {
    if (enabled) {
        mImpl->params.m_tex_type = basist::cBASISTexTypeCubemapArray;
    } else {
        const bool multi = mImpl->params.m_source_images.size() > 1;
        mImpl->params.m_tex_type = multi ? basist::cBASISTexType2DArray : basist::cBASISTexType2D;
    }
    return *this;
}

Builder& Builder::intermediateFormat(BasisEncoder::IntermediateFormat format) noexcept {
    mImpl->params.m_uastc = format == IntermediateFormat::UASTC;
    return *this;
}

Builder& Builder::grayscale(bool enabled) noexcept {
    mImpl->grayscale = enabled;
    return *this;
}

Builder& Builder::normals(bool enabled) noexcept {
    mImpl->normals = enabled;
    return *this;
}

Builder& Builder::jobs(size_t count) noexcept {
    mImpl->jobs = count;
    return *this;
}

Builder& Builder::quiet(bool enabled) noexcept {
    mImpl->quiet = enabled;
    return *this;
}

BasisEncoder* Builder::build() {
    if (mImpl->error) {
        return nullptr;
    }

    basisu::basisu_encoder_init();

    auto& params = mImpl->params;

    params.m_status_output = !mImpl->quiet;
    params.m_pJob_pool = new basisu::job_pool(mImpl->jobs);
    params.m_create_ktx2_file = true;
    params.m_ktx2_uastc_supercompression = basist::KTX2_SS_ZSTANDARD;

    // This sRGB flag doesn't actually affect the compression scheme or the basis format, it's just
    // an annotation that gets stored in the KTX2 file, which enables the app to choose the right
    // format when it loads and transcodes the texture. Technically however, the transcoder
    // SHOULD know about this, since in some scenarios it needs to interpolate between colors.
    params.m_ktx2_srgb_transfer_func = !mImpl->linear;

    // Select the same quality that the basis tool selects by default (midpoint of range).
    params.m_quality_level = 128;

    // This is the default zstd compression level used by the basisu cmdline cool.
    params.m_ktx2_zstd_supercompression_level = 6;

    // We do not want basis to read from files, we want it to read from "m_source_images"
    params.m_read_source_images = false;

    // We do not want basis to write the file, we want to manually dump "get_output_ktx2_file()"
    params.m_write_output_basis_files = false;

    basisu::basis_compressor* encoder = new basisu::basis_compressor();

    if (!encoder->init(params)) {
        assert_invariant(false);
        delete encoder;
        return nullptr;
    }

    return new BasisEncoder(new BasisEncoderImpl {
        .encoder = encoder,
        .jobs = params.m_pJob_pool,
        .quiet = mImpl->quiet,
    });
}

BasisEncoder::BasisEncoder(BasisEncoderImpl* impl) noexcept : mImpl(impl) {}

BasisEncoder::~BasisEncoder() noexcept {
    delete mImpl->encoder;
    delete mImpl->jobs;
    delete mImpl;
    basisu::basisu_encoder_deinit();
}

BasisEncoder::BasisEncoder(BasisEncoder&& that) noexcept  : mImpl(nullptr) {
    std::swap(mImpl, that.mImpl);
}

BasisEncoder& BasisEncoder::operator=(BasisEncoder&& that) noexcept {
    std::swap(mImpl, that.mImpl);
    return *this;
}

bool BasisEncoder::encode() {
    using namespace basisu;
    basis_compressor::error_code ec = mImpl->encoder->process();
    switch (ec)
    {
    case basis_compressor::cECSuccess:
        if (!mImpl->quiet) {
            puts("Compression succeeded.");
        }
        return true;
    case basis_compressor::cECFailedReadingSourceImages:
        puts("Compressor failed reading a source image!");
        break;
    case basis_compressor::cECFailedValidating:
        puts("Compressor failed 2darray/cubemap/video validation checks!");
        break;
    case basis_compressor::cECFailedEncodeUASTC:
        puts("Compressor UASTC encode failed!");
        break;
    case basis_compressor::cECFailedFrontEnd:
        puts("Compressor frontend stage failed!");
        break;
    case basis_compressor::cECFailedFontendExtract:
        puts("Compressor frontend data extraction failed!");
        break;
    case basis_compressor::cECFailedBackend:
        puts("Compressor backend stage failed!");
        break;
    case basis_compressor::cECFailedCreateBasisFile:
        puts("Compressor failed creating Basis file data!");
        break;
    case basis_compressor::cECFailedWritingOutput:
        puts("Compressor failed writing to output Basis file!");
        break;
    case basis_compressor::cECFailedUASTCRDOPostProcess:
        puts("Compressor failed during the UASTC post process step!");
        break;
    case basis_compressor::cECFailedCreateKTX2File:
        puts("Compressor failed creating KTX2 file data!");
        break;
    default:
        puts("basis_compress::process() failed!");
        break;
    }
    return false;
}

size_t BasisEncoder::getKtx2ByteCount() const noexcept {
    return mImpl->encoder->get_output_ktx2_file().size();
}

uint8_t const* BasisEncoder::getKtx2Data() const noexcept {
    return mImpl->encoder->get_output_ktx2_file().data();
}

} // namespace image
