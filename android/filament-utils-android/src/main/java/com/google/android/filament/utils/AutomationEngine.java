/*
 * Copyright (C) 2021 The Android Open Source Project
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

package com.google.android.filament.utils;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.google.android.filament.ColorGrading;
import com.google.android.filament.Engine;
import com.google.android.filament.Entity;
import com.google.android.filament.IndirectLight;
import com.google.android.filament.LightManager;
import com.google.android.filament.Scene;
import com.google.android.filament.View;
import com.google.android.filament.MaterialInstance;
import com.google.android.filament.Renderer;

/**
 * The AutomationEngine makes it easy to push a bag of settings values to Filament.
 * It can also be used to iterate through settings permutations for testing purposes.
 *
 * When creating an automation engine for testing purposes, clients give it a JSON string that
 * tells it how to generate permutations of settings. Automation is always in one of two states:
 * running or idle. The running state can be entered immediately (startRunning) or by requesting
 * batch mode (startBatchMode).
 *
 * When executing a test, clients should call tick() after each frame is rendered, which provides an
 * opportunity to push settings to Filament, increment the current test index (if enough time has
 * elapsed), and request an asynchronous screenshot.
 *
 * The time to sleep between tests is configurable and can be set to zero. Automation also waits a
 * specified minimum number of frames between tests.
 *
 * Batch mode is meant for non-interactive applications. In batch mode, automation defers applying
 * the first test case until the client unblocks it via signalBatchMode(). This is useful when
 * waiting for a large model file to become fully loaded. Batch mode also offers a query
 * (shouldClose) that is triggered after the last test has been invoked.
 */
public class AutomationEngine {
    private final long mNativeObject;
    private ColorGrading mColorGrading;

    /**
     * Allows users to toggle screenshots, change the sleep duration between tests, etc.
     */
    public static class Options {
        /**
         * Minimum time that automation waits between applying a settings object and advancing
         * to the next test case. Specified in seconds.
         */
        public float sleepDuration = 0.2f;

        /**
         * Similar to sleepDuration, but expressed as a frame count. Both the minimum sleep time
         * and the minimum frame count must be elapsed before automation advances to the next test.
         */
        public int minFrameCount = 2;

        /**
         * If true, test progress is dumped to the utils Log (info priority).
         */
        public boolean verbose = true;
    }

    /**
     * Collection of Filament objects that can be modified by the automation engine.
     */
    public static class ViewerContent {
        public View view;
        public Renderer renderer;
        public MaterialInstance[] materials;
        public LightManager lightManager;
        public Scene scene;
        public IndirectLight indirectLight;
        public @Entity int sunlight;
        public @Entity int[] assetLights;
    }

    /**
     * Allows remote control for the viewer.
     */
    public static class ViewerOptions {
        public float cameraAperture = 16.0f;
        public float cameraSpeed = 125.0f;
        public float cameraISO = 100.0f;
        public float cameraNear = 0.1f;
        public float cameraFar = 100.0f;
        public float groundShadowStrength = 0.75f;
        public boolean groundPlaneEnabled = false;
        public boolean skyboxEnabled = true;
        public float cameraFocalLength = 28.0f;
        public float cameraFocusDistance = 0.0f;
        public boolean autoScaleEnabled = true;
        public boolean autoInstancingEnabled = false;
    }

    /**
     * Creates an automation engine from a JSON specification.
     *
     * An example of a JSON spec can be found by searching the repo for DEFAULT_AUTOMATION.
     * This is documented using a JSON schema (look for viewer/schemas/automation.json).
     *
     * @param jsonSpec Valid JSON string that conforms to the automation schema.
     */
    public AutomationEngine(@NonNull String jsonSpec) {
        mNativeObject = nCreateAutomationEngine(jsonSpec);
        if (mNativeObject == 0) throw new IllegalStateException("Couldn't create AutomationEngine");
    }

    /**
     * Creates an automation engine for the sole purpose of pushing settings, or for executing
     * the default test sequence.
     *
     * To see how the default test sequence is generated, search for DEFAULT_AUTOMATION.
     */
    public AutomationEngine() {
        mNativeObject = nCreateDefaultAutomationEngine();
        if (mNativeObject == 0) throw new IllegalStateException("Couldn't create AutomationEngine");
    }

    /**
     * Configures the automation engine for users who wish to set up a custom sleep time
     * between tests, etc.
     */
    public void setOptions(@NonNull Options options) {
        nSetOptions(mNativeObject, options.sleepDuration, options.minFrameCount, options.verbose);
    }

    /**
     * Activates the automation test. During the subsequent call to tick(), the first test is
     * applied and automation enters the running state.
     */
    public void startRunning() { nStartRunning(mNativeObject); }

    /**
     * Activates the automation test, but enters a paused state until the user calls
     * signalBatchMode().
     */
    public void startBatchMode() { nStartBatchMode(mNativeObject); }

    /**
     * Notifies the automation engine that time has passed and a new frame has been rendered.
     *
     * This is when settings get applied, screenshots are (optionally) exported, and the internal
     * test counter is potentially incremented.
     *
     * @param engine        The filament Engine of interest.
     * @param content       Contains the Filament View, Materials, and Renderer that get modified.
     * @param deltaTime     The amount of time that has passed since the previous tick in seconds.
     */
    public void tick(@NonNull Engine engine, @NonNull ViewerContent content, float deltaTime) {
        if (content.view == null || content.renderer == null) {
            throw new IllegalStateException("Must provide a View and Renderer");
        }
        long[] nativeMaterialInstances = null;
        if (content.materials != null) {
            nativeMaterialInstances = new long[content.materials.length];
            for (int i = 0; i < nativeMaterialInstances.length; i++) {
                nativeMaterialInstances[i] = content.materials[i].getNativeObject();
            }
        }
        long nativeView = content.view.getNativeObject();
        long nativeRenderer = content.renderer.getNativeObject();
        nTick(mNativeObject, engine.getNativeObject(), nativeView, nativeMaterialInstances, nativeRenderer, deltaTime);
    }

    /**
     * Mutates a set of client-owned Filament objects according to a JSON string.
     *
     * This method is an alternative to tick(). It allows clients to use the automation engine as a
     * remote control, as opposed to iterating through a predetermined test sequence.
     *
     * This updates the stashed Settings object, then pushes those settings to the given
     * Filament objects. Clients can optionally call getColorGrading() after calling this method.
     *
     * @param engine        Filament Engine to use.
     * @param settingsJson  Contains the JSON string with a set of changes that need to be pushed.
     * @param content       Contains a set of Filament objects that you want to mutate.
     */
    public void applySettings(@NonNull Engine engine, @NonNull String settingsJson,
        @NonNull ViewerContent content) {
        if (content.view == null || content.renderer == null) {
            throw new IllegalStateException("Must provide a View and Renderer");
        }
        long[] nativeMaterialInstances = null;
        if (content.lightManager == null || content.scene == null) {
            throw new IllegalStateException("Must provide a LightManager and Scene");
        }
        if (content.materials != null) {
            nativeMaterialInstances = new long[content.materials.length];
            for (int i = 0; i < nativeMaterialInstances.length; i++) {
                nativeMaterialInstances[i] = content.materials[i].getNativeObject();
            }
        }
        long nativeView = content.view.getNativeObject();
        long nativeIbl = content.indirectLight == null ? 0 : content.indirectLight.getNativeObject();
        long nativeLm = content.lightManager.getNativeObject();
        long nativeScene = content.scene.getNativeObject();
        long nativeRenderer = content.renderer.getNativeObject();
        nApplySettings(mNativeObject, engine.getNativeObject(),
                settingsJson, nativeView, nativeMaterialInstances,
                nativeIbl, content.sunlight, content.assetLights, nativeLm, nativeScene,
                nativeRenderer);
    }

    /**
     * Gets the current viewer options.
     *
     * NOTE: Focal length here might be different from the user-specified value, due to DoF options.
     */
    @NonNull
    public ViewerOptions getViewerOptions() {
        ViewerOptions result = new ViewerOptions();
        nGetViewerOptions(mNativeObject, result);
        return result;
    }

    /**
     * Gets a color grading object that corresponds to the latest settings.
     *
     * This method either returns a cached instance, or it destroys the cached instance and creates
     * a new one.
     */
    @NonNull
    public ColorGrading getColorGrading(@NonNull Engine engine) {
        // The native layer automatically destroys the old color grading instance,
        // so there is no need to call Engine#destroyColorGrading here.
        long nativeCg = nGetColorGrading(mNativeObject, engine.getNativeObject());
        if (mColorGrading == null || mColorGrading.getNativeObject() != nativeCg) {
            mColorGrading = nativeCg == 0 ? null : new ColorGrading(nativeCg);
        }
        return mColorGrading;
    }

    /**
     * Signals that batch mode can begin. Call this after all meshes and textures finish loading.
     */
    public void signalBatchMode() { nSignalBatchMode(mNativeObject); }

    /**
     * Cancels an in-progress automation session.
     */
    public void stopRunning() { nStopRunning(mNativeObject); }

    /**
     * Returns true if automation is in batch mode and all tests have finished.
     */
    public boolean shouldClose() { return nShouldClose(mNativeObject); }

    @Override
    protected void finalize() throws Throwable {
        nDestroy(mNativeObject);
        super.finalize();
    }

    private static native long nCreateAutomationEngine(String jsonSpec);
    private static native long nCreateDefaultAutomationEngine();
    private static native void nSetOptions(long nativeObject, float sleepDuration,
            int minFrameCount, boolean verbose);
    private static native void nStartRunning(long nativeObject);
    private static native void nStartBatchMode(long nativeObject);
    private static native void nTick(long nativeObject, long nativeEngine,
            long view, long[] materials, long renderer, float deltaTime);
    private static native void nApplySettings(long nativeObject, long nativeEngine,
            String jsonSettings, long view,
            long[] materials, long ibl, int sunlight, int[] assetLights, long lightManager,
            long scene, long renderer);
    private static native void nGetViewerOptions(long nativeObject, Object result);
    private static native long nGetColorGrading(long nativeObject, long nativeEngine);
    private static native void nSignalBatchMode(long nativeObject);
    private static native void nStopRunning(long nativeObject);
    private static native boolean nShouldClose(long nativeObject);
    private static native void nDestroy(long nativeObject);
}
