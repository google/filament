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

#include "TIFFExport.h"

#include <filament/Camera.h>
#include <filament/Engine.h>
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

namespace {

void convertRGBAtoRGB(void* buffer, uint32_t width, uint32_t height) {
    uint8_t* writePtr = static_cast<uint8_t*>(buffer);
    uint8_t const* readPtr = static_cast<uint8_t const*>(buffer);
    for (uint32_t i = 0, n = width * height; i < n; ++i) {
        writePtr[0] = readPtr[0];
        writePtr[1] = readPtr[1];
        writePtr[2] = readPtr[2];
        writePtr += 3;
        readPtr += 4;
    }
}

void exportPPM(void* buffer, uint32_t width, uint32_t height, std::ofstream& outstream) {
    // ReadPixels on Metal only supports RGBA, but the PPM format only supports RGB.
    // So, manually perform a quick transformation here.
    convertRGBAtoRGB(buffer, width, height);

    outstream << "P6 " << width << " " << height << " " << 255 << std::endl;
    outstream.write(static_cast<char*>(buffer), width * height * 3);
}

using ExportFormat = AutomationEngine::Options::ExportFormat;
constexpr char const* getExportFormatExtension(ExportFormat format) {
    switch (format) {
        case ExportFormat::PPM: return ".ppm";
        case ExportFormat::TIFF: return ".tif";
    }
}

} // anonymous namespace

void AutomationEngine::exportScreenshot(View* view, Renderer* renderer, std::string filename,
        bool autoclose, AutomationEngine* automationEngine) {
    const Viewport& vp = view->getViewport();
    const size_t byteCount = vp.width * vp.height * 4;

    // Create a buffer descriptor that writes the PPM after the data becomes ready on the CPU.
    backend::PixelBufferDescriptor buffer(
        new uint8_t[byteCount], byteCount,
        backend::PixelBufferDescriptor::PixelDataFormat::RGBA,
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
            std::ofstream outstream(out);

            auto extension = out.getExtension();
            if (extension == "ppm") {
                exportPPM(buffer, vp.width, vp.height, outstream);
            } else if (extension == "tif" || extension == "tiff") {
                exportTIFF(buffer, vp.width, vp.height, outstream);
            } else {
                utils::slog.e << out.c_str() << " does not specify a supported file extension."
                              << utils::io::endl;
            }

            outstream.close();

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
    if (mColorGrading) {
        mColorGradingEngine->destroy(mColorGrading);
    }
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

void AutomationEngine::applySettings(Engine* engine, const char* json, size_t jsonLength,
        const ViewerContent& content) {
    JsonSerializer serializer;
    if (!serializer.readJson(json, jsonLength, mSettings)) {
        std::string jsonWithTerminator(json, json + jsonLength);
        slog.e << "Badly formed JSON:\n" << jsonWithTerminator.c_str() << io::endl;
        return;
    }
    viewer::applySettings(engine, mSettings->view, content.view);
    for (size_t i = 0; i < content.materialCount; i++) {
        viewer::applySettings(engine, mSettings->material, content.materials[i]);
    }
    viewer::applySettings(engine, mSettings->lighting, content.indirectLight, content.sunlight,
            content.assetLights, content.assetLightCount, content.lightManager, content.scene, content.view);
    Camera* camera = &content.view->getCamera();
    Skybox* skybox = content.scene->getSkybox();
    viewer::applySettings(engine, mSettings->viewer, camera, skybox, content.renderer);
}

ColorGrading* AutomationEngine::getColorGrading(Engine* engine) {
    if (mSettings->view.colorGrading != mColorGradingSettings) {
        mColorGradingSettings = mSettings->view.colorGrading;
        if (mColorGrading) {
            mColorGradingEngine->destroy(mColorGrading);
        }
        mColorGrading = createColorGrading(mColorGradingSettings, engine);
        mColorGradingEngine = engine;
    }
    return mColorGrading;
}

ViewerOptions AutomationEngine::getViewerOptions() const {
    ViewerOptions options = mSettings->viewer;
    const auto dofOptions = mSettings->view.dof;
    if (dofOptions.enabled) {
        options.cameraFocalLength = Camera::computeEffectiveFocalLength(
                options.cameraFocalLength / 1000.0,
                std::max(0.1f, options.cameraFocusDistance)) * 1000.0;

    }
    return mSettings->viewer;
}

void AutomationEngine::tick(Engine* engine, const ViewerContent& content, float deltaTime) {
    const auto activateTest = [this, engine, content]() {
        mElapsedTime = 0;
        mElapsedFrames = 0;
        mSpec->get(mCurrentTest, mSettings);
        viewer::applySettings(engine, mSettings->view, content.view);
        for (size_t i = 0; i < content.materialCount; i++) {
            viewer::applySettings(engine, mSettings->material, content.materials[i]);
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
        AutomationEngine::exportScreenshot(content.view, content.renderer,
                prefix + getExportFormatExtension(mOptions.exportFormat), isLastTest, this);
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
