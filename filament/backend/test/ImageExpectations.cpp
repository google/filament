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

#include "BackendTest.h"
#include "backend/PixelBufferDescriptor.h"
#include "private/backend/DriverApi.h"

#ifndef FILAMENT_IOS

#include <imageio/ImageEncoder.h>
#include <imageio/ImageDecoder.h>
#include <image/ColorTransform.h>

#endif

namespace test {

ScreenshotParams::ScreenshotParams(int width, int height, std::string fileName,
        uint32_t expectedHash, bool isSrgb, int numAllowedDeviations, int pixelMatchThreshold)
    : mWidth(width),
      mHeight(height),
      mIsSrgb(isSrgb),
      mExpectedPixelHash(expectedHash),
      mFileName(std::move(fileName)),
      mAllowedPixelDeviations(numAllowedDeviations),
      mPixelMatchThreshold(pixelMatchThreshold) {}

int ScreenshotParams::width() const {
    return mWidth;
}

int ScreenshotParams::height() const {
    return mHeight;
}

bool ScreenshotParams::isSrgb() const {
    return mIsSrgb;
}

uint32_t ScreenshotParams::expectedHash() const {
    return mExpectedPixelHash;
}

std::filesystem::path ScreenshotParams::actualDirectoryPath() {
    return BackendTest::binaryDirectory().append("images/actual_images");
}

std::string ScreenshotParams::actualFileName() const {
    return absl::StrFormat("%s_actual.png", mFileName);
}

std::filesystem::path ScreenshotParams::actualFilePath() const {
    return actualDirectoryPath().append(actualFileName());
}

std::filesystem::path ScreenshotParams::expectedDirectoryPath() {
    return BackendTest::binaryDirectory().append("images/expected_images");
}

std::string ScreenshotParams::expectedFileName() const {
    return absl::StrFormat("%s.png", mFileName);
}

std::filesystem::path ScreenshotParams::expectedFilePath() const {
    return expectedDirectoryPath().append(expectedFileName());
}

std::filesystem::path ScreenshotParams::diffFilePath() const {
    return diffDirectoryPath().append(diffFileName());
}

std::filesystem::path ScreenshotParams::diffDirectoryPath() {
    return BackendTest::binaryDirectory().append("images/diff_images");
}

std::string ScreenshotParams::diffFileName() const {
    return absl::StrFormat("%s_diff.png", mFileName);
}

const std::string ScreenshotParams::filePrefix() const {
    // TODO(b/422804941): If there are platform specific goldens, when on those platforms append a
    //  unique platform identifying string to this.
    return mFileName;
}

int ScreenshotParams::allowedPixelDeviations() const {
    return mAllowedPixelDeviations;
}

int ScreenshotParams::pixelMatchThreshold() const {
    return mPixelMatchThreshold;
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
    // If this fails, it likely means that BackendTest::flushAndWait needs to be called before
    // ImageExpectations is evaluated or destroyed.
    EXPECT_THAT(bytesFilled, testing::IsTrue())
                        << "Render target wasn't copied to the buffer for " << mFileName;
    if (bytesFilled) {
#ifndef FILAMENT_IOS
        LoadedPng loadedImage(mParams.expectedFilePath());
        if (loadedImage.bytes().size() != mResult.bytes().size()) {
            // Something is wrong with the size of the expected result, which usually means the file
            // is missing. Fail the test and early return so later steps can assume the image is
            // there.
            BackendTest::markImageAsFailure(mParams.filePrefix());
            return;
        }

        // Initialize the vector to full saturation on all channels, bad pixels will have
        // color channels but not alpha set to 0 to produce highly contrasting black pixels.
        std::vector<unsigned char> imageDiff( loadedImage.bytes().size(), 255 );
        int pixelDeviations = 0;
        for (int i = 0; i < mResult.bytes().size(); i += 4) {
            // In order to handle color channels propoerly for the output, stride 4 bytes at a time.
            // A failure of any byte in a pixel counts as a failure of the whole pixel.
            for (int j = 0; j < 4; ++j) {
                if( std::abs( mResult.bytes()[i+j] - loadedImage.bytes()[i+j] ) >
                    mParams.pixelMatchThreshold() ) {
                    pixelDeviations++;
                    imageDiff[i] = 0;
                    imageDiff[i+1] = 0;
                    imageDiff[i+2] = 0;
                    break;
                }
            }
        }
        EXPECT_LE(pixelDeviations, mParams.allowedPixelDeviations());
        if (pixelDeviations > mParams.allowedPixelDeviations()) {
            BackendTest::markImageAsFailure(mParams.filePrefix());
            image::LinearImage image;
            image = image::toLinearWithAlpha<uint8_t>(
                    mParams.width(), mParams.height(),
                    mParams.width() * 4, (uint8_t *)imageDiff.data(),
                    [](uint8_t value) -> float { return value; },
                    [](filament::math::float4 rgba) -> filament::math::float4 { return rgba; });
            std::string filePath = mParams.diffFilePath();
            std::ofstream pngStream(filePath, std::ios::binary | std::ios::trunc);
            // To avoid going from linear -> sRGB -> linear save the PNG as linear.
            image::ImageEncoder::encode(pngStream, image::ImageEncoder::Format::PNG_LINEAR, image, "",
                    filePath);
        }
        EXPECT_LE(pixelDeviations, mParams.allowedPixelDeviations());
#else
        // For builds that can't load PNGs (currently iOS only) use the expected hash.
        uint32_t actualHash = mResult.hash();
        EXPECT_THAT(actualHash, testing::Eq(mParams.expectedHash())) << mParams.expectedFileName();
#endif
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
    mExpectations.emplace_back(std::make_unique<ImageExpectation>(fileName, lineNumber, mApi,
            std::move(params), renderTarget));
}

void ImageExpectations::evaluate() {
    for (auto& expectation: mExpectations) {
        expectation->evaluate();
    }
    mExpectations.clear();
}

RenderTargetDump::RenderTargetDump(filament::backend::DriverApi& api,
        filament::backend::RenderTargetHandle renderTarget, const ScreenshotParams& params)
        : mInternal(std::make_unique<RenderTargetDump::Internal>(params)) {
    const size_t size = mInternal->params.width() * mInternal->params.height() * 4;
    mInternal->bytes.resize(size);

    auto cb = [](void* buffer, size_t size, void* user) {
        auto* internal = static_cast<RenderTargetDump::Internal*>(user);
        internal->bytesFilled = true;
#ifndef FILAMENT_IOS
        image::LinearImage image;
        if (internal->params.isSrgb()) {
            image = image::toLinearWithAlpha<uint8_t>(internal->params.width(),
                    internal->params.height(),
                    internal->params.width() * 4, (uint8_t*)buffer);
        } else {
            // The image data is already linear, so pass in transforms that simply go from uint8_t
            // to float. toLinearWithAlpha divides the float values by uint8_t max so there's no
            // need to scale it to [0, 1]
            image = image::toLinearWithAlpha<uint8_t>(
                    internal->params.width(), internal->params.height(),
                    internal->params.width() * 4, (uint8_t*) buffer,
                    [](uint8_t value) -> float { return value; },
                    [](filament::math::float4 rgba) -> filament::math::float4 { return rgba; });
        }
        std::string filePath = internal->params.actualFilePath();
        std::ofstream pngStream(filePath, std::ios::binary | std::ios::trunc);
        // To avoid going from linear -> sRGB -> linear save the PNG as linear.
        image::ImageEncoder::encode(pngStream, image::ImageEncoder::Format::PNG_LINEAR, image, "",
                filePath);
#endif
    };
    filament::backend::PixelBufferDescriptor pb(mInternal->bytes.data(), size,
            filament::backend::PixelDataFormat::RGBA, filament::backend::PixelDataType::UBYTE, cb,
            (void*)mInternal.get());
    api.readPixels(renderTarget, 0, 0, mInternal->params.width(), mInternal->params.height(),
            std::move(pb));
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

uint32_t RenderTargetDump::Internal::hash() const {
    return utils::hash::murmur3((uint32_t*)bytes.data(), bytes.size() / 4, 0);
}

uint32_t RenderTargetDump::hash() const {
    return mInternal->hash();
}

const std::vector<unsigned char>& RenderTargetDump::bytes() const {
    return mInternal->bytes;
}

bool RenderTargetDump::bytesFilled() const {
    return mInternal->bytesFilled;
}

RenderTargetDump::Internal::Internal(const ScreenshotParams& params) : params(params) {}

LoadedPng::LoadedPng(std::string filePath) : mFilePath(std::move(filePath)) {
#ifndef FILAMENT_IOS
    std::ifstream pngStream(mFilePath, std::ios::binary);
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
    EXPECT_THAT(mBytes, testing::Not(testing::IsEmpty()))
            << "Failed to load expected test result: " << mFilePath << ".\n"
            << "Did you forget to sync CMake after updating the expected image in the source "
               "directory?";
    if (mBytes.empty()) {
        return 0;
    }
    return utils::hash::murmur3((uint32_t*)mBytes.data(), mBytes.size() / 4, 0);
}

const std::vector<unsigned char>& LoadedPng::bytes() const {
    return mBytes;
}

} // namespace test
