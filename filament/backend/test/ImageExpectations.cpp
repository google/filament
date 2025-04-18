/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "ImageExpectations.h"

#include "gmock/gmock.h"
#include "absl/strings/str_format.h"
#include "utils/Hash.h"
#include <fstream>

#include "backend/PixelBufferDescriptor.h"
#include "private/backend/DriverApi.h"

#ifndef FILAMENT_IOS

#include <imageio/ImageEncoder.h>
#include <imageio/ImageDecoder.h>
#include <image/ColorTransform.h>

#endif

ScreenshotParams::ScreenshotParams(int width, int height, std::string fileName,
        uint32_t expectedHash)
    : mWidth(width),
      mHeight(height),
      mExpectedPixelHash(expectedHash),
      mFileName(std::move(fileName)) {}

int ScreenshotParams::width() const {
    return mWidth;
}

int ScreenshotParams::height() const {
    return mHeight;
}

uint32_t ScreenshotParams::expectedHash() const {
    return mExpectedPixelHash;
}

std::string ScreenshotParams::actualDirectoryPath() {
    return "images/actual_images";
}

std::string ScreenshotParams::actualFileName() const {
    return absl::StrFormat("%s_actual.png", mFileName);
}

std::string ScreenshotParams::actualFilePath() const {
    return absl::StrFormat("%s/%s", actualDirectoryPath(), actualFileName());
}

std::string ScreenshotParams::expectedDirectoryPath() {
    return "images/expected_images";
}

std::string ScreenshotParams::expectedFileName() const {
    return absl::StrFormat("%s.png", mFileName);
}

std::string ScreenshotParams::expectedFilePath() const {
    return absl::StrFormat("%s/%s", expectedDirectoryPath(), expectedFileName());
}

ImageExpectation::ImageExpectation(const char* fileName, int lineNumber,
        filament::backend::DriverApi& api, ScreenshotParams params,
        filament::backend::RenderTargetHandle renderTarget)
        : mFileName(fileName), mLineNumber(lineNumber), mParams(std::move(params)),
          mResult(api, renderTarget, mParams) {}

void ImageExpectation::evaluate() {
    // Ensure this is only evaluated once.
    if (mEvaluated) {
        return;
    }
    mEvaluated = true;

    // Do the actual image comparison inside a scoped trace with the stored file and line.
    {
        testing::ScopedTrace trace(mFileName, mLineNumber, "");
        compareImage();
    }
}

void ImageExpectation::compareImage() const {
    bool bytesFilled = mResult.bytesFilled();
    EXPECT_THAT(bytesFilled, testing::IsTrue())
                        << "Render target wasn't copied to the buffer for " << mFileName;
    if (bytesFilled) {
        LoadedPng loadedImage(mParams.expectedFilePath());
        // Rather than directly compare the two images compare their hashes because comparing very
        // large arrays generates way too much debug output to be useful.
        uint32_t actualHash = mResult.hash();
        uint32_t loadedImageHash = loadedImage.hash();
        EXPECT_THAT(actualHash, testing::Eq(loadedImageHash));
        EXPECT_THAT(actualHash, testing::Eq(mParams.expectedHash()));
        // TODO: Add better debug output, such as generating a diff image.
    }
}

ImageExpectations::ImageExpectations(filament::backend::DriverApi& api) : mApi(api) {}

ImageExpectations::~ImageExpectations() {
    // Guarantee that all expectations are evaluated when this leaves scope even if the caller
    // forgot to manually evaluate them.
    evaluate();
}

void ImageExpectations::addExpectation(const char* fileName, int lineNumber,
        filament::backend::RenderTargetHandle renderTarget, ScreenshotParams params) {
    mExpectations.emplace_back(fileName, lineNumber, mApi, std::move(params), renderTarget);
}

void ImageExpectations::evaluate() {
    for (auto& expectation: mExpectations) {
        expectation.evaluate();
    }
    mExpectations.clear();
}

RenderTargetDump::RenderTargetDump(filament::backend::DriverApi& api,
        filament::backend::RenderTargetHandle renderTarget, const ScreenshotParams& params)
        : mInternal(std::make_unique<RenderTargetDump::Internal>(params)) {
#ifdef FILAMENT_IOS
    bytesFilled_ = true;
    bytes_.resize(size);
    std::fill(bytes_.begin(), bytes_.end(), 0);
#else
    const size_t size = mInternal->params.width() * mInternal->params.height() * 4;
    mInternal->bytes.resize(size);

    auto cb = [](void* buffer, size_t size, void* user) {
        auto* internal = static_cast<RenderTargetDump::Internal*>(user);
        image::LinearImage image(internal->params.width(), internal->params.width(), 4);
        image = image::toLinearWithAlpha<uint8_t>(internal->params.width(),
                internal->params.height(),
                internal->params.width() * 4, (uint8_t*)buffer);
        std::string filePath = internal->params.actualFilePath();
        std::ofstream pngStream(filePath, std::ios::binary | std::ios::trunc);
        image::ImageEncoder::encode(pngStream, image::ImageEncoder::Format::PNG, image, "",
                filePath);
        internal->bytesFilled = true;
    };
    filament::backend::PixelBufferDescriptor pb(mInternal->bytes.data(), size,
            filament::backend::PixelDataFormat::RGBA, filament::backend::PixelDataType::UBYTE, cb,
            (void*)mInternal.get());
    api.readPixels(renderTarget, 0, 0, mInternal->params.width(), mInternal->params.height(),
            std::move(pb));
#endif
}

RenderTargetDump::~RenderTargetDump() {
    // If the GPU callback hasn't been made yet then there's a callback elsewhere that has a copy of
    // the internal pointer. But there's no guarantee that the callback will be ever made if the GPU
    // pipeline wasn't run for some reason. So it is necessary to leak the memory.
    // It would be possible to try to coordinate with the callback to have it clean up the memory,
    // but if this condition happens there's already an issue with the test case so there's no need.
    if (!bytesFilled()) {
        mInternal.release();
    }
}

uint32_t RenderTargetDump::hash() const {
    return utils::hash::murmur3((uint32_t*)mInternal->bytes.data(), mInternal->bytes.size() / 4, 0);
}

bool RenderTargetDump::bytesFilled() const {
    return mInternal->bytesFilled;
}

RenderTargetDump::Internal::Internal(const ScreenshotParams& params) : params(params) {}

LoadedPng::LoadedPng(std::string filePath) {
#ifndef FILAMENT_IOS
    std::ifstream pngStream(filePath, std::ios::binary);
    image::LinearImage loadedImage = image::ImageDecoder::decode(pngStream, filePath,
            image::ImageDecoder::ColorSpace::LINEAR);
    size_t valuesInImage = loadedImage.getWidth() * loadedImage.getHeight() *
                          loadedImage.getChannels();
    // The linear image is loaded with each component as [0.0, 1.0] but should be [0, 255], so
    // convert them.
    mBytes = std::vector<unsigned char>(valuesInImage);
    for (int i = 0; i < valuesInImage; ++i) {
        mBytes[i] = static_cast<uint8_t>(loadedImage.get<float>()[i] * 255.0f);
    }
#endif
    // For platforms that don't support the image loading library, leave the loaded data blank.
}

uint32_t LoadedPng::hash() const {
    EXPECT_THAT(mBytes, testing::Not(testing::IsEmpty()));
    if (mBytes.empty()) {
        return 0;
    }
    return utils::hash::murmur3((uint32_t*)mBytes.data(), mBytes.size() / 4, 0);
}
