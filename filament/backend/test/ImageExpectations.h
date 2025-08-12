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

#ifndef TNT_IMAGE_EXPECTATIONS_H
#define TNT_IMAGE_EXPECTATIONS_H

#include <filesystem>
#include <vector>

#include "gtest/gtest.h"

#include "backend/Handle.h"
#include "backend/DriverApiForward.h"

// Arguments are (RenderTargetHandle renderTarget, ImageExpectations& expectations,
// ScreenshotParams screenshotParams)
#define EXPECT_IMAGE(renderTarget, screenshotParams)  \
do {                                                                \
    getExpectations().addExpectation(                               \
        __FILE__,                                                   \
        __LINE__,                                                   \
        renderTarget,                                               \
        screenshotParams);                                          \
} while (0)

namespace test {

/**
 * Stores user-provided configuration values for an image expectation
 */
class ScreenshotParams {
public:
    // TODO(b/422804941): Add a set of environments where this test should use a different golden.
    ScreenshotParams(int width, int height, std::string fileName, uint32_t expectedPixelHash,
            bool isSrgb = false, int numAllowedDeviations = 0, int pixelMatchThreshold = 0);

    int width() const;
    int height() const;
    bool isSrgb() const;
    uint32_t expectedHash() const;

    static std::filesystem::path actualDirectoryPath();
    std::string actualFileName() const;
    std::filesystem::path actualFilePath() const;
    static std::filesystem::path expectedDirectoryPath();
    std::string expectedFileName() const;
    std::filesystem::path expectedFilePath() const;
    const std::string filePrefix() const;
    int allowedPixelDeviations() const;
    int pixelMatchThreshold() const;

private:
    int mWidth;
    int mHeight;
    bool mIsSrgb;
    uint32_t mExpectedPixelHash;
    std::string mFileName;
    int mAllowedPixelDeviations;
    int mPixelMatchThreshold;
};

/**
 * When created adds a command to the GPU pipeline to copy the render target into a buffer and
 * stores the result.
 * If this object is destroyed before the GPU pipeline is flushed it will leak memory in order to
 * avoid the GPU pipeline callback being a use-after-free.
 */
class RenderTargetDump {
public:
    RenderTargetDump(filament::backend::DriverApi& api,
            filament::backend::RenderTargetHandle renderTarget, const ScreenshotParams& params);
    RenderTargetDump(RenderTargetDump&& other) = default;
    RenderTargetDump& operator=(RenderTargetDump&& other) = default;
    ~RenderTargetDump();

    /**
     * Should only be used if BytesFilled returns true.
     * @return The hash of the stored bytes.
     */
    uint32_t hash() const;
    /**
     * Gets the bytes of the render target. The hash should usually be preferable for comparisons
     * but this is available for debugging.
     * @return The stored bytes.
     */
    const std::vector<unsigned char>& bytes() const;
    /**
     * Thread safe as this is backed by an atomic.
     * Once this returns true it will never return false.
     * @return Whether the bytes have actually been copied from the GPU to the buffer.
     */
    bool bytesFilled() const;

private:
    struct Internal {
        explicit Internal(const ScreenshotParams& params);
        ScreenshotParams params;
        std::atomic<bool> bytesFilled = false;
        std::vector<unsigned char> bytes;

        uint32_t hash() const;
    };

    // We need a memory location that won't be invalidated to pass to GPU callbacks as they can't
    // be canceled during the destructor.
    std::unique_ptr<Internal> mInternal;
};

class LoadedPng {
public:
    explicit LoadedPng(std::string filePath);

    uint32_t hash() const;

    const std::vector<unsigned char>& bytes() const;

private:
    std::string mFilePath;
    std::vector<unsigned char> mBytes;
};

class ImageExpectation {
public:
    ImageExpectation(const char* fileName, int lineNumber, filament::backend::DriverApi& api,
            ScreenshotParams params, filament::backend::RenderTargetHandle renderTarget);

    void evaluate();

private:
    void compareImage() const;

    bool mEvaluated = false;
    const char* mFileName;
    int mLineNumber;
    ScreenshotParams mParams;
    RenderTargetDump mResult;
};

class ImageExpectations {
public:
    explicit ImageExpectations(filament::backend::DriverApi& api);
    ~ImageExpectations();

    /**
     * Not meant to be called directly, use EXPECT_IMAGE to get the file name and line number
     */
    void addExpectation(const char* fileName, int lineNumber,
            filament::backend::RenderTargetHandle renderTarget, ScreenshotParams params);

    void evaluate();

private:
    filament::backend::DriverApi& mApi;
    // Store expectations in unique pointers because they are self referential.
    std::vector<std::unique_ptr<ImageExpectation>> mExpectations;
};

} // namespace test

#endif //TNT_IMAGE_EXPECTATIONS_H
