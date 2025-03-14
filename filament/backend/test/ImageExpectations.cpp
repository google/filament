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

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gtest/internal/gtest-internal.h"

#include "backend/PixelBufferDescriptor.h"
#include "private/backend/DriverApi.h"

#include "absl/strings/str_format.h"
#include "utils/Hash.h"
#include <fstream>

#ifndef FILAMENT_IOS
#include <imageio/ImageEncoder.h>
#include <image/ColorTransform.h>
#endif

ScreenshotParams::ScreenshotParams(int width, int height, std::string fileName, uint32_t expectedPixelHash)
    : width_(width)
    , height_(height)
    , fileName_(std::move(fileName))
    , expectedPixelHash_(expectedPixelHash) {}

int ScreenshotParams::Width() const {
    return width_;
}

int ScreenshotParams::Height() const {
    return height_;
}

uint32_t ScreenshotParams::ExpectedHash() const {
    return expectedPixelHash_;
}

std::string ScreenshotParams::OutputDirectoryPath() const {
    return ".";
}
std::string ScreenshotParams::GeneratedActualFileName() const {
    return absl::StrFormat("%s_actual.png", fileName_);
}
std::string ScreenshotParams::GeneratedActualFilePath() const {
    return absl::StrFormat("%s/%s", OutputDirectoryPath(), GeneratedActualFileName());
}
std::string ScreenshotParams::GoldenFileName() const {
    return absl::StrFormat("%s_golden.png", fileName_);
}
std::string ScreenshotParams::GoldenFilePath() const {
    return absl::StrFormat("%s/%s", OutputDirectoryPath(), GoldenFileName());
}

ImageExpectation::ImageExpectation(const char* fileName, int lineNumber, filament::backend::DriverApi& api, ScreenshotParams params, filament::backend::RenderTargetHandle renderTarget)
    : fileName_(fileName)
    , lineNumber_(lineNumber)
    , params_(std::move(params))
    , result_(api, renderTarget, params_) {}

void ImageExpectation::Evaluate() {
    // Ensure this is only evaluated once.
    if (evaluated_) {
        return;
    }
    evaluated_ = true;

    // Do the actual image comparison inside a scoped trace with the stored file and line.
    {
        testing::ScopedTrace trace(fileName_, lineNumber_, "");
        CompareImage();
    }
}

void ImageExpectation::CompareImage() const {
    bool bytesFilled = result_.BytesFilled();
    EXPECT_THAT(bytesFilled, testing::IsTrue()) << "Render target wasn't copied to the buffer for " << fileName_;
    if (bytesFilled) {
        uint32_t actualHash = result_.Hash();
        EXPECT_THAT(actualHash, testing::Eq(params_.ExpectedHash()));
    }
}

ImageExpectations::ImageExpectations(filament::backend::DriverApi& api) : api_(api) {}

ImageExpectations::~ImageExpectations() {
    // Guarantee that all expectations are evaluated when this leaves scope even if the caller
    // forgot to manually evaluate them.
    Evaluate();
}

void ImageExpectations::AddExpectation(const char* fileName, int lineNumber, filament::backend::RenderTargetHandle renderTarget, ScreenshotParams params) {
    expectations_.emplace_back(fileName, lineNumber, api_, std::move(params), renderTarget);
}

void ImageExpectations::Evaluate() {
    for (auto& expectation : expectations_) {
        expectation.Evaluate();
    }
    expectations_.clear();
}

RenderTargetDump::RenderTargetDump(filament::backend::DriverApi& api, filament::backend::RenderTargetHandle renderTarget, const ScreenshotParams& params)
    : internal_(std::make_unique<RenderTargetDump::Internal>(params)) {
#ifdef FILAMENT_IOS
    bytesFilled_ = true;
    bytes_.resize(size);
    std::fill(bytes_.begin(), bytes_.end(), 0);
#else
    const size_t size = internal_->params.Width() * internal_->params.Height() * 4;
    internal_->bytes.resize(size);

    auto cb = [](void* buffer, size_t size, void* user) {
        auto* internal = static_cast<RenderTargetDump::Internal*>(user);
        image::LinearImage image(internal->params.Width(), internal->params.Width(), 4);
        image = image::toLinearWithAlpha<uint8_t>(internal->params.Width(), internal->params.Height(), internal->params.Width() * 4, (uint8_t*) buffer);
        std::string filePath = internal->params.GeneratedActualFilePath();
        std::ofstream pngStream(filePath, std::ios::binary | std::ios::trunc);
        image::ImageEncoder::encode(pngStream, image::ImageEncoder::Format::PNG, image, "", filePath);
        internal->bytesFilled = true;
    };
    filament::backend::PixelBufferDescriptor pb(internal_->bytes.data(), size, filament::backend::PixelDataFormat::RGBA, filament::backend::PixelDataType::UBYTE, cb,
            (void*) internal_.get());
    api.readPixels(renderTarget, 0, 0, internal_->params.Width(), internal_->params.Height(), std::move(pb));
#endif
}

RenderTargetDump::~RenderTargetDump() {
    // If the GPU callback hasn't been made yet then there's a callback elsewhere that has a copy of
    // the internal pointer. But there's no guarantee that the callback will be ever made if the GPU
    // pipeline wasn't run for some reason. So it is necessary to leak the memory.
    // It would be possible to try to coordinate with the callback to have it clean up the memory,
    // but if this condition happens there's already an issue with the test case so there's no need.
    if (!BytesFilled()) {
        internal_.release();
    }
}

uint32_t RenderTargetDump::Hash() const {
    return utils::hash::murmur3((uint32_t*)internal_->bytes.data(), internal_->bytes.size() / 4, 0);
}

bool RenderTargetDump::BytesFilled() const {
    return internal_->bytesFilled;
}

RenderTargetDump::Internal::Internal(const ScreenshotParams& params) : params(params) {}