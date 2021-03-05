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

#include <filament/Renderer.h>
#include <filament/Viewport.h>

#include <backend/PixelBufferDescriptor.h>

#include <utils/Log.h>
#include <utils/Path.h>

#include <iomanip>
#include <fstream>
#include <sstream>

using namespace utils;

namespace filament {
namespace viewer {

static std::string gStatus;

struct ScreenshotState {
    View* view;
    std::string filename;
    bool autoclose;
    AutomationEngine* engine;
};

static void exportScreenshot(View* view, Renderer* renderer, std::string filename,
        bool autoclose, AutomationEngine* automationEngine) {
    const Viewport& vp = view->getViewport();
    const size_t byteCount = vp.width * vp.height * 3;

    // Create a buffer descriptor that writes the PPM after the data becomes ready on the CPU.
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
            Path out(state->filename);
            std::ofstream ppmStream(out);
            ppmStream << "P6 " << vp.width << " " << vp.height << " " << 255 << std::endl;
            ppmStream.write(static_cast<char*>(buffer), vp.width * vp.height * 3);
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

AutomationEngine* AutomationEngine::createFromJSON(const char* jsonSpec, size_t size) {
    AutomationSpec* spec = AutomationSpec::generate(jsonSpec, size);
    if (!spec) {
        return nullptr;
    }
    Settings* settings = new Settings();
    AutomationEngine* result = new AutomationEngine(spec, settings);
    result->mOwnsSettings = true;
    return result;
}

AutomationEngine* AutomationEngine::createDefault() {
    AutomationSpec* spec = AutomationSpec::generateDefaultTestCases();
    if (!spec) {
        return nullptr;
    }
    Settings* settings = new Settings();
    AutomationEngine* result = new AutomationEngine(spec, settings);
    result->mOwnsSettings = true;
    return result;
}

AutomationEngine::~AutomationEngine() {
    if (mOwnsSettings) {
        delete mSpec;
        delete mSettings;
    }
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
    JsonSerializer serializer;
    std::string contents = serializer.writeJson(settings);
    std::ofstream out(filename);
    if (!out) {
        gStatus = "Failed to export settings file.";
    }
    out << contents << std::endl;
    gStatus = "Exported to '" + std::string(filename) + "' in the current folder.";
}

void AutomationEngine::applySettings(const char* json, size_t jsonLength, View* view,
        MaterialInstance* const* materials, size_t materialCount, IndirectLight* ibl,
        utils::Entity sunlight, LightManager* lm, Scene* scene) {
    JsonSerializer serializer;
    serializer.readJson(json, jsonLength, mSettings);
    viewer::applySettings(mSettings->view, view);
    for (size_t i = 0; i < materialCount; i++) {
        viewer::applySettings(mSettings->material, materials[i]);
    }
    viewer::applySettings(mSettings->lighting, ibl, sunlight, lm, scene);
}

void AutomationEngine::tick(View* view, MaterialInstance* const* materials, size_t materialCount,
        Renderer* renderer, float deltaTime) {
    const auto activateTest = [this, view, materials, materialCount]() {
        mElapsedTime = 0;
        mElapsedFrames = 0;
        mSpec->get(mCurrentTest, mSettings);
        viewer::applySettings(mSettings->view, view);
        for (size_t i = 0; i < materialCount; i++) {
            viewer::applySettings(mSettings->material, materials[i]);
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
        exportScreenshot(view, renderer, prefix + ".ppm", isLastTest, this);
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
