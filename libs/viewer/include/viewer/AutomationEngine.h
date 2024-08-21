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

#ifndef VIEWER_AUTOMATION_ENGINE_H
#define VIEWER_AUTOMATION_ENGINE_H

#include <viewer/AutomationSpec.h>

namespace filament {

class ColorGrading;
class Engine;
class LightManager;
class MaterialInstance;
class Renderer;
class View;

namespace viewer {

/**
 * The AutomationEngine makes it easy to push a bag of settings values to Filament.
 * It can also be used to iterate through settings permutations for testing purposes.
 *
 * When creating an automation engine for testing purposes, clients give it an immutable reference
 * to an AutomationSpec. It is always in one of two states: running or idle. The running state can
 * be entered immediately (startRunning) or by requesting batch mode (startBatchMode).
 *
 * When executing a test, clients should call tick() after each frame is rendered, which gives
 * automation an opportunity to push settings to Filament, increment the current test index (if
 * enough time has elapsed), and request an asynchronous screenshot.
 *
 * The time to sleep between tests is configurable and can be set to zero. Automation also waits a
 * specified minimum number of frames between tests.
 *
 * Batch mode is meant for non-interactive applications. In batch mode, automation defers applying
 * the first test case until the client unblocks it via signalBatchMode(). This is useful when
 * waiting for a large model file to become fully loaded. Batch mode also offers a query
 * (shouldClose) that is triggered after the last test has been invoked.
 */
class UTILS_PUBLIC AutomationEngine {
public:
    /**
     * Allows users to toggle screenshots, change the sleep duration between tests, etc.
     */
    struct Options {
        /**
         * Minimum time that automation waits between applying a settings object and advancing
         * to the next test case. Specified in seconds.
         */
        float sleepDuration = 0.2f;

        /**
         * Similar to sleepDuration, but expressed as a frame count. Both the minimum sleep time
         * and the minimum frame count must be elapsed before automation advances to the next test.
         */
        int minFrameCount = 2;

        /**
         * If true, test progress is dumped to the utils Log (info priority).
         */
        bool verbose = true;

        /**
         * If true, the tick function writes out a screenshot before advancing to the next test.
         */
        bool exportScreenshots = false;

        /**
         * If true, the tick function writes out a settings JSON file before advancing.
         */
        bool exportSettings = false;
    };

    /**
     * Collection of Filament objects that can be modified by the automation engine.
     */
    struct ViewerContent {
        View* view;
        Renderer* renderer;
        MaterialInstance* const* materials;
        size_t materialCount;
        LightManager* lightManager;
        Scene* scene;
        IndirectLight* indirectLight;
        utils::Entity sunlight;
        utils::Entity* assetLights;
        size_t assetLightCount;
    };

    /**
     * Creates an automation engine and places it in an idle state.
     *
     * @param spec     Specifies a set of settings permutations (owned by the client).
     * @param settings Client-owned settings object. This not only supplies the initial
     *                 state, it also receives changes during tick(). This is useful when
     *                 building automation into an application that has a settings UI.
     *
     * @see setOptions
     * @see startRunning
     */
    AutomationEngine(const AutomationSpec* spec, Settings* settings) :
            mSpec(spec), mSettings(settings) {}

    /**
     * Shortcut constructor that creates an automation engine from a JSON string.
     *
     * This constructor can be used if the user does not need to monitor how the settings
     * change over time and does not need ownership over the AutomationSpec.
     *
     * An example of a JSON spec can be found by searching the repo for DEFAULT_AUTOMATION.
     * This is documented using a JSON schema (look for viewer/schemas/automation.json).
     *
     * @param jsonSpec Valid JSON string that conforms to the automation schema.
     * @param size     Number of characters in the JSON string.
     * @return         Automation engine or null if unable to read the JSON.
     */
    static AutomationEngine* createFromJSON(const char* jsonSpec, size_t size);

    /**
     * Creates an automation engine for the sole purpose of pushing settings, or for executing
     * the default test sequence.
     *
     * To see how the default test sequence is generated, search for DEFAULT_AUTOMATION.
     */
    static AutomationEngine* createDefault();

    /**
     * Activates the automation test. During the subsequent call to tick(), the first test is
     * applied and automation enters the running state.
     */
    void startRunning();

    /**
     * Activates the automation test, but enters a paused state until the user calls
     * signalBatchMode().
     */
    void startBatchMode();

    /**
     * Notifies the automation engine that time has passed, a new frame has been rendered.
     *
     * This is when settings get applied, screenshots are (optionally) exported, and the internal
     * test counter is potentially incremented.
     *
     * @param content       Contains the Filament View, Materials, and Renderer that get modified.
     * @param deltaTime     The amount of time that has passed since the previous tick in seconds.
     */
    void tick(Engine* engine, const ViewerContent& content, float deltaTime);

    /**
     * Mutates a set of client-owned Filament objects according to a JSON string.
     *
     * This method is an alternative to tick(). It allows clients to use the automation engine as a
     * remote control, as opposed to iterating through a predetermined test sequence.
     *
     * This updates the stashed Settings object, then pushes those settings to the given
     * Filament objects. Clients can optionally call getColorGrading() after calling this method.
     *
     * @param json       Contains the JSON string with a set of changes that need to be pushed.
     * @param jsonLength Number of characters in the json string.
     * @param content    Contains a set of Filament objects that you want to mutate.
     */
    void applySettings(Engine* engine, const char* json, size_t jsonLength, const ViewerContent& content);

    /**
     * Gets a color grading object that corresponds to the latest settings.
     *
     * This method either returns a cached instance, or it destroys the cached instance and creates
     * a new one.
     */
    ColorGrading* getColorGrading(Engine* engine);

    /**
     * Gets the current viewer options.
     *
     * NOTE: Focal length here might be different from the user-specified value, due to DoF options.
     */
    ViewerOptions getViewerOptions() const;

    /**
     * Signals that batch mode can begin. Call this after all meshes and textures finish loading.
     */
    void signalBatchMode() { mBatchModeAllowed = true; }

    /**
     * Cancels an in-progress automation session.
     */
    void stopRunning() { mIsRunning = false; }

    /**
     * Signals that the application is closing, so all pending screenshots should be cancelled.
     */
    void terminate();

    /**
     * Configures the automation engine for users who wish to set up a custom sleep time
     * between tests, etc.
     */
    void setOptions(Options options) { mOptions = options; }

    /**
     * Returns true if automation is in batch mode and all tests have finished.
     */
    bool shouldClose() const { return mShouldClose; }

    /**
     * Convenience function that writes out a JSON file to disk containing all settings.
     *
     * @param Settings State vector to serialize.
     * @param filename Desired JSON filename.
     */
    static void exportSettings(const Settings& settings, const char* filename);

    static void exportScreenshot(View* view, Renderer* renderer, std::string filename,
            bool autoclose, AutomationEngine* automationEngine);

    Options getOptions() const { return mOptions; }
    bool isRunning() const { return mIsRunning; }
    size_t currentTest() const { return mCurrentTest; }
    size_t testCount() const { return mSpec->size(); }
    bool isBatchModeEnabled() const { return mBatchModeEnabled; }
    const char* getStatusMessage() const;
    ~AutomationEngine();

private:
    AutomationSpec const * const mSpec;
    Settings * const mSettings;
    Options mOptions;

    Engine* mColorGradingEngine = nullptr;
    ColorGrading* mColorGrading = nullptr;
    ColorGradingSettings mColorGradingSettings = {};

    size_t mCurrentTest;
    float mElapsedTime;
    int mElapsedFrames;
    bool mIsRunning = false;
    bool mBatchModeEnabled = false;
    bool mRequestStart = false;
    bool mShouldClose = false;
    bool mBatchModeAllowed = false;
    bool mTerminated = false;
    bool mOwnsSettings = false;

public:
    // For internal use from a screenshot callback.
    void requestClose() { mShouldClose = true; }
    bool isTerminated() const { return mTerminated; }
};

} // namespace viewer
} // namespace filament

#endif // VIEWER_AUTOMATION_ENGINE_H
