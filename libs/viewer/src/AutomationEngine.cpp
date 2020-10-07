/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include <viewer/AutomationEngine.h>

#include <imageio/ImageEncoder.h>

#include <image/ColorTransform.h>

#include <filament/Renderer.h>
#include <filament/Viewport.h>

#include <backend/PixelBufferDescriptor.h>

#include <utils/Log.h>
#include <utils/Path.h>

#include <iomanip>
#include <fstream>
#include <sstream>

using namespace image;
using namespace utils;

namespace filament {
namespace viewer {

static std::string gStatus;

template<typename T>
static LinearImage toLinear(size_t w, size_t h, size_t bpr, const uint8_t* src) {
    LinearImage result(w, h, 3);
    filament::math::float3* d = reinterpret_cast<filament::math::float3*>(result.getPixelRef(0, 0));
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src + y * bpr);
        for (size_t x = 0; x < w; ++x, p += 3) {
            filament::math::float3 sRGB(p[0], p[1], p[2]);
            sRGB /= std::numeric_limits<T>::max();
            *d++ = sRGBToLinear(sRGB);
        }
    }
    return result;
}

struct ScreenshotState {
    View* view;
    std::string filename;
    bool autoclose;
    AutomationEngine* engine;
};

void exportScreenshot(View* view, Renderer* renderer, std::string filename,
        bool autoclose, AutomationEngine* automationEngine) {
    const Viewport& vp = view->getViewport();
    const size_t byteCount = vp.width * vp.height * 3;

    // Create a buffer descriptor that writes the PNG after the data becomes ready on the CPU.
    backend::PixelBufferDescriptor buffer(
        new uint8_t[byteCount], byteCount,
        backend::PixelBufferDescriptor::PixelDataFormat::RGB,
        backend::PixelBufferDescriptor::PixelDataType::UBYTE,
        [](void* buffer, size_t size, void* user) {
            ScreenshotState* state = static_cast<ScreenshotState*>(user);
            if (state->engine->isTerminated()) {
                delete[] static_cast<uint8_t*>(buffer);
                delete state;
                return;
            }
            const Viewport& vp = state->view->getViewport();
            LinearImage image(toLinear<uint8_t>(vp.width, vp.height, vp.width * 3,
                    static_cast<uint8_t*>(buffer)));
            Path out(state->filename);
            std::ofstream outputStream(out, std::ios::binary | std::ios::trunc);
            ImageEncoder::encode(outputStream, ImageEncoder::Format::PNG, image, "",
                    state->filename);
            delete[] static_cast<uint8_t*>(buffer);
            if (state->autoclose) {
                state->engine->requestClose();
            }
            delete state;
        },
        new ScreenshotState { view, filename, autoclose, automationEngine }
    );

    // Invoke readPixels asynchronously.
    renderer->readPixels((uint32_t) vp.left, (uint32_t) vp.bottom, vp.width, vp.height,
            std::move(buffer));
}

void AutomationEngine::startRunning() {
    mRequestStart = true;
}

void AutomationEngine::startBatchMode() {
    mRequestStart = true;
    mBatchModeEnabled = true;
}

void AutomationEngine::terminate() {
    stopRunning();
    mTerminated = true;
}

void AutomationEngine::exportSettings(const Settings& settings, const char* filename) {
    std::string contents = writeJson(settings);
    std::ofstream out(filename);
    if (!out) {
        gStatus = "Failed to export settings file.";
    }
    out << contents << std::endl;
    gStatus = "Exported to '" + std::string(filename) + "' in the current folder.";
}

void AutomationEngine::tick(View* view, MaterialInstance* const* materials, size_t materialCount,
        Renderer* renderer, float deltaTime) {
    const auto activateTest = [this, view, materials, materialCount]() {
        mElapsedTime = 0;
        mElapsedFrames = 0;
        mSpec->get(mCurrentTest, mSettings);
        applySettings(mSettings->view, view);
        for (size_t i = 0; i < materialCount; i++) {
            applySettings(mSettings->material, materials[i]);
        }
        if (mOptions.verbose) {
            utils::slog.i << "Running test " << mCurrentTest << utils::io::endl;
        }
    };

    if (!mIsRunning) {
        if (mRequestStart) {
            if ((mBatchModeEnabled && mBatchModeAllowed) || !mBatchModeEnabled) {
                mIsRunning = true;
                mRequestStart = false;
                mCurrentTest = 0;
                activateTest();
            }
        }
        return;
    }

    mElapsedTime += deltaTime;
    mElapsedFrames++;

    if (mElapsedTime < mOptions.sleepDuration || mElapsedFrames < mOptions.minFrameCount) {
        return;
    }

    const bool isLastTest = mCurrentTest == mSpec->size() - 1;

    const int digits = (int) log10 ((double) mSpec->size()) + 1;
    std::ostringstream stringStream;
    stringStream << mSpec->getName(mCurrentTest)
            << std::setfill('0') << std::setw(digits) << mCurrentTest;
    std::string prefix = stringStream.str();

    if (mOptions.exportSettings) {
        std::string filename = prefix + ".json";
        exportSettings(*mSettings, filename.c_str());
    }

    if (mOptions.exportScreenshots) {
        exportScreenshot(view, renderer, prefix + ".png", isLastTest, this);
    }

    if (isLastTest) {
        mIsRunning = false;
        if (mBatchModeEnabled && !mOptions.exportScreenshots) {
            mShouldClose = true;
        }
        return;
    }

    // Increment the case number and apply the next round of settings.
    mCurrentTest++;
    activateTest();
}

const char* AutomationEngine::getStatusMessage() const {
    return gStatus.c_str();
}

} // namespace viewer
} // namespace filament
