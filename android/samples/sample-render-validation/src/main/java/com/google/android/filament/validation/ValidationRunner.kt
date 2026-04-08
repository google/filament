/*
 * Copyright (C) 2026 The Android Open Source Project
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

package com.google.android.filament.validation

import android.content.Context
import android.graphics.Bitmap
import android.util.Log
import com.google.android.filament.utils.AutomationEngine
import com.google.android.filament.utils.ImageDiff
import com.google.android.filament.utils.ModelViewer
import org.json.JSONObject
import java.io.File
import java.io.FileOutputStream
import java.nio.ByteBuffer

class ValidationRunner(
    private val context: Context,
    private val surfaceView: android.view.SurfaceView,
    private val config: RenderTestConfig,
    private val resultManager: ValidationResultManager,
    private val backendFilter: String = "both" // "gles", "vulkan", or "both"
) {

    private var currentState = State.IDLE
    private var currentTestIndex = 0
    private var currentModelIndex = 0
    private var currentEngine: AutomationEngine? = null
    private var currentTestConfig: TestConfig? = null
    private var currentModelName: String? = null
    private var currentBackend: String? = null

    private var frameCounter = 0
    private var suiteStartTime: Long = 0
    private var modelViewer: ModelViewer? = null

    private val sortedTests: List<Pair<TestConfig, String>> by lazy {
        val filtered = config.tests.filter {
            when (backendFilter) {
                "gles" -> it.backend == "opengl"
                "vulkan" -> it.backend == "vulkan"
                else -> true
            }
        }

        val expanded = mutableListOf<Pair<TestConfig, String>>()
        filtered.forEach { test ->
            test.models.forEach { model ->
                expanded.add(test to model)
            }
        }

        // Sort by backend then model. This tries to minimize the number of times we need to
        // recreate the Engine, and then the ModelViewer.
        expanded.sortedWith(compareBy({ it.first.backend }, { it.second }))
    }

    enum class State {
        IDLE,
        WAITING_FOR_RESOURCES,
        WARMUP,
        RUNNING_TEST
    }

    interface Callback {
        fun onTestProgress(current: Int, total: Int)
        fun onTestFinished(result: ValidationResult)
        fun onAllTestsFinished()
        fun onStatusChanged(status: String)
        fun onImageResult(type: String, bitmap: Bitmap)
        fun onModelViewerRecreated(modelViewer: ModelViewer?)
    }

    var callback: Callback? = null
    var generateGoldens: Boolean = false

    fun start() {
        if (sortedTests.isEmpty()) {
            callback?.onAllTestsFinished()
            return
        }
        suiteStartTime = System.currentTimeMillis()
        currentBackend = null
        currentTestIndex = 0
        runTest(sortedTests[0])
    }

    private fun runTest(testPair: Pair<TestConfig, String>) {
        callback?.onTestProgress(currentTestIndex, sortedTests.size)
        val (test, modelName) = testPair
        currentTestConfig = test

        if (currentBackend != test.backend) {
            currentBackend = test.backend
            callback?.onStatusChanged("Switching to backend: ${test.backend}")
            recreateEngine(test.backend)
            // After recreation, we need to reload the model
            currentModelName = null
        }

        startModel(modelName)
    }

    public  fun cleanup() {
        // When the runner ends, we need to destroy the resources to ensure that they are free for
        // another run.
        modelViewer?.destroy()
    }

    private fun recreateEngine(backendName: String) {
        val backend = when (backendName.lowercase()) {
            "vulkan" -> com.google.android.filament.Engine.Backend.VULKAN
            else -> com.google.android.filament.Engine.Backend.OPENGL
        }

        // ModelViewer's detach listener will handle engine destruction when surface is detached,
        // but here we are still using the same surface. We need a way to swap the engine.
        // The easiest way is to recreate ModelViewer with a new Engine.
        modelViewer?.destroy()

        val engine = com.google.android.filament.Engine.create(backend)

        resultManager.addGpuDriverInfo(backendName, com.google.android.filament.utils.DeviceUtils.getGpuDriverInfo(engine))

        // Setting the camera manipulator to null enables changing the camera outside of ModelViewer
        val newModelViewer = ModelViewer(surfaceView, engine, manipulator=null)

        // Update the reference in MainActivity (via callback)
        callback?.onModelViewerRecreated(newModelViewer)
        modelViewer = newModelViewer
    }

    private fun startModel(modelName: String) {
        if (currentModelName == modelName) {
            Log.i("ValidationRunner", "Reusing model $modelName")
            callback?.onStatusChanged("Reusing $modelName for ${currentTestConfig?.name} (${currentTestConfig?.backend})")

            modelViewer?.resetToDefaultState()

            frameCounter = 0
            currentState = State.WARMUP
            return
        }

        currentModelName = modelName
        val modelPath = config.models[modelName]
        if (modelPath == null) {
            Log.e("ValidationRunner", "Model $modelName not found")
            nextTest()
            return
        }
        callback?.onStatusChanged("Loading $modelName for ${currentTestConfig?.name} (${currentTestConfig?.backend})")

        // Load model on main thread (required by ModelViewer)
        loadModel(modelPath)
    }

    private fun loadModel(path: String) {
        // Assume called on Main Thread
        modelViewer?.destroyModel()
        try {
            Log.i("ValidationRunner", "Reading model file: $path")
            val bytes = File(path).readBytes()
            Log.i("ValidationRunner", "Loading GLB buffer... (${bytes.size} bytes)")
            val buffer = ByteBuffer.wrap(bytes)
            modelViewer?.loadModelGlb(buffer)
            Log.i("ValidationRunner", "Model loaded.")
            modelViewer?.transformToUnitCube()
            currentState = State.WAITING_FOR_RESOURCES
            frameCounter = 0
            Log.i("ValidationRunner", "State set to WAITING_FOR_RESOURCES")
        } catch (e: Exception) {
            Log.e("ValidationRunner", "Failed to load $path", e)
            nextTest()
        }
    }

    fun onFrame(frameTimeNanos: Long) {
        when (currentState) {
            State.IDLE -> {}
            State.WAITING_FOR_RESOURCES -> {
                modelViewer?.let { mv ->
                    val progress = mv.progress
                    if (progress >= 1.0f) {
                        Log.i("ValidationRunner", "Resources loaded. Starting warmup.")
                        frameCounter = 0
                        currentState = State.WARMUP
                    }
                }
            }
            State.WARMUP -> {
                frameCounter++
                if (frameCounter > 1) { // 1 frames warmup
                    startAutomation()
                }
            }
            State.RUNNING_TEST -> {
                currentEngine?.let { engine ->
                    modelViewer?.let { mv ->
                        val content = AutomationEngine.ViewerContent()
                        content.view = mv.view
                        content.renderer = mv.renderer
                        content.scene = mv.scene
                        content.lightManager = mv.engine.lightManager

                        // Tick
                        val deltaTime = 1.0f / 60.0f
                        engine.tick(mv.engine, content, deltaTime)

                        frameCounter++
                        if (engine.shouldClose()) {
                            Log.i("ValidationRunner", "Finishing test (frames: $frameCounter)")
                            // Test finished (for this spec)
                            currentState = State.IDLE
                            captureAndCompare()
                        }
                    }
                }
            }
        }
    }

    private fun startAutomation() {
        val test = currentTestConfig!!
        val specJson = JSONObject()
        specJson.put("name", test.name)
        specJson.put("base", test.rendering)
        val fullSpec = "[${specJson.toString()}]"

        currentEngine = AutomationEngine(fullSpec)
        val options = AutomationEngine.Options()
        options.sleepDuration = 0.0f // Minimal sleep, let frames drive it
        options.minFrameCount = 1 // Ensure some frames pass
        currentEngine?.setOptions(options)

        // Use batch mode to ensure shouldClose() works reliably
        currentEngine?.startBatchMode()
        currentEngine?.signalBatchMode() // Start immediately

        frameCounter = 0
        currentState = State.RUNNING_TEST
    }


    private fun captureAndCompare() {
        callback?.onStatusChanged("Comparing ${currentTestConfig?.name} (${currentTestConfig?.backend})...")
        modelViewer?.debugGetNextFrameCallback { bitmap ->
            compareCapturedImage(bitmap)
        }
    }

    private fun compareCapturedImage(bitmap: Bitmap) {
         val testName = currentTestConfig!!.name
         val modelName = currentModelName!!
         val backend = currentTestConfig?.backend ?: "opengl"
         val testFullName = "${testName}.${backend}.${modelName}"

         // Golden path
         val modelFile = File(config.models.get(modelName)!!)
         val modelParent = modelFile.parentFile!!

         // Search for golden in:
         // 1. ../golden/ (standard structure)
         // 2. ../goldens/ (exported structure, sibling of models)
         // 3. ./goldens/ (backup)

         val searchPaths = mutableListOf<File>()
         modelParent.parentFile?.let {
             searchPaths.add(it.resolve("golden"))
             searchPaths.add(it.resolve("goldens"))
         }
         searchPaths.add(modelParent.resolve("goldens"))

         var goldenFile: File? = null
         for (path in searchPaths) {
             val f = path.resolve("${testFullName}.png")
             if (f.exists()) {
                 goldenFile = f
                 break
             }
         }

         if (goldenFile != null) {
             Log.i("ValidationRunner", "Found golden at ${goldenFile.absolutePath}")
         } else {
             Log.w("ValidationRunner", "Golden not found for $testFullName. Searched in: ${searchPaths.joinToString { it.absolutePath }}")
             // Fallback to old behavior for reference if everything else fails
             goldenFile = modelParent.parentFile?.resolve("golden/${testFullName}.png") ?: File("nonexistent")
         }

         Thread {
             try {
                val flipped = bitmap

                callback?.onImageResult("Rendered", flipped)

                var passed = false
                var diffMetric = 0f

                if (generateGoldens) {
                    val targetGolden = goldenFile ?: modelParent.parentFile?.resolve("golden/${testFullName}.png") ?: File(modelParent, "golden/${testFullName}.png")
                    targetGolden.parentFile?.mkdirs()
                    FileOutputStream(targetGolden).use { out ->
                        flipped.compress(Bitmap.CompressFormat.PNG, 100, out)
                    }
                    passed = true // Generating goldens always passes if successful
                    callback?.onStatusChanged("Golden generated")
                } else {
                    if (goldenFile != null && goldenFile.exists()) {
                        val golden = android.graphics.BitmapFactory.decodeFile(goldenFile.absolutePath)
                        if (golden != null) {
                            callback?.onImageResult("Golden", golden)

                            val tol = currentTestConfig?.tolerance ?: org.json.JSONObject()
                            val tolJson = tol.toString()
                            val result = ImageDiff.compare(golden, flipped, tolJson, null)
                            passed = (result.status == ImageDiff.Result.Status.PASSED)
                            diffMetric = result.failingPixelCount.toFloat()

                            if (!passed) {
                                if (result.diffImage != null) {
                                    val diffImg = result.diffImage!!
                                    val width = diffImg.width
                                    val height = diffImg.height
                                    val pixels = IntArray(width * height)
                                    diffImg.getPixels(pixels, 0, width, 0, 0, width, height)

                                    var hasAlphaDiff = false
                                    val alphaPixels = IntArray(width * height)

                                    for (i in pixels.indices) {
                                        val color = pixels[i]

                                        val a = android.graphics.Color.alpha(color)
                                        val r = android.graphics.Color.red(color)
                                        val g = android.graphics.Color.green(color)
                                        val b = android.graphics.Color.blue(color)

                                        if (a > 0) {
                                            hasAlphaDiff = true
                                        }

                                        // Map alpha diff to grayscale RGB
                                        alphaPixels[i] = android.graphics.Color.argb(255, a, a, a)

                                        // Force main diff image alpha to 255
                                        pixels[i] = android.graphics.Color.argb(255, r, g, b)
                                    }

                                    // Apply updated pixels to diff image
                                    diffImg.setPixels(pixels, 0, width, 0, 0, width, height)

                                    // The C++ ImageDiff code sets isPremultiplied to false so Android
                                    // doesn't erase RGB diff values when Alpha diff is 0. However, Android's
                                    // Canvas will crash if we try to draw a non-premultiplied bitmap.
                                    // Since we just forced all alpha values to 255 (fully opaque) in the
                                    // loop above, we can safely mark it as premultiplied again here.
                                    diffImg.isPremultiplied = true
                                    callback?.onImageResult("Diff", diffImg)
                                    resultManager.saveImage("${testFullName}_diff", diffImg)

                                    if (hasAlphaDiff) {
                                        val alphaDiffImg = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888)
                                        alphaDiffImg.setPixels(alphaPixels, 0, width, 0, 0, width, height)
                                        callback?.onImageResult("Alpha Diff", alphaDiffImg)
                                        resultManager.saveImage("${testFullName}_alpha_diff", alphaDiffImg)
                                        alphaDiffImg.recycle()
                                    }
                                    diffImg.recycle()
                                }
                             }
                             golden.recycle()
                        } else {
                            callback?.onStatusChanged("Failed to load golden")
                        }
                    } else {
                        Log.w("ValidationRunner", "Golden not found: ${goldenFile?.absolutePath}")
                        callback?.onStatusChanged("Golden not found")
                    }
                }

                // Save output
                resultManager.saveImage(testFullName, flipped)

                val result = ValidationResult(testFullName, passed, diffMetric, goldenFile?.absolutePath)
                resultManager.addResult(result)
                callback?.onTestFinished(result)

                android.os.Handler(android.os.Looper.getMainLooper()).post {
                    nextTest()
                }

             } catch (e: Throwable) {
                 Log.e("ValidationRunner", "Comparison failed", e)
                 // Recycle even on error
                 bitmap.recycle()
                 android.os.Handler(android.os.Looper.getMainLooper()).post { nextTest() }
             }
         }.start()
    }

    private fun nextTest() {
        currentTestIndex++
        if (currentTestIndex < sortedTests.size) {
            runTest(sortedTests[currentTestIndex])
        } else {
            currentState = State.IDLE

            val totalTimeMs = System.currentTimeMillis() - suiteStartTime

            resultManager.finalizeResults(totalTimeMs)
            callback?.onAllTestsFinished()
        }
    }
}
