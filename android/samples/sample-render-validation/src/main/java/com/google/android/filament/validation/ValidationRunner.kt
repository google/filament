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
    private val modelViewer: ModelViewer,
    private val config: RenderTestConfig,
    private val resultManager: ValidationResultManager
) {

    private var currentState = State.IDLE
    private var currentTestIndex = 0
    private var currentModelIndex = 0
    private var currentEngine: AutomationEngine? = null
    private var currentTestConfig: TestConfig? = null
    private var currentModelName: String? = null

    private var frameCounter = 0

    enum class State {
        IDLE,
        WAITING_FOR_RESOURCES,
        WARMUP,
        RUNNING_TEST
    }

    interface Callback {
        fun onTestFinished(result: ValidationResult)
        fun onAllTestsFinished()
        fun onStatusChanged(status: String)
        fun onImageResult(type: String, bitmap: Bitmap)
    }

    var callback: Callback? = null
    var generateGoldens: Boolean = false

    fun start() {
        if (config.tests.isEmpty()) {
            callback?.onAllTestsFinished()
            return
        }
        currentTestIndex = 0
        currentModelIndex = 0
        startTest(config.tests[0])
    }

    private fun startTest(test: TestConfig) {
        currentTestConfig = test
        if (test.models.isEmpty()) {
            nextTest()
            return
        }
        currentModelIndex = 0
        startModel(test.models.elementAt(0))
    }

    private fun startModel(modelName: String) {
        currentModelName = modelName
        val modelPath = config.models[modelName]
        if (modelPath == null) {
            Log.e("ValidationRunner", "Model $modelName not found")
            nextModel()
            return
        }
        callback?.onStatusChanged("Loading $modelName for ${currentTestConfig?.name}")

        // Load model on main thread (required by ModelViewer)
        loadModel(modelPath)
    }

    private fun loadModel(path: String) {
        // Assume called on Main Thread
        modelViewer.destroyModel()
        try {
            Log.i("ValidationRunner", "Reading model file: $path")
            val bytes = File(path).readBytes()
            Log.i("ValidationRunner", "Loading GLB buffer... (${bytes.size} bytes)")
            val buffer = ByteBuffer.wrap(bytes)
            modelViewer.loadModelGlb(buffer)
            Log.i("ValidationRunner", "Model loaded.")
            modelViewer.transformToUnitCube()
            currentState = State.WAITING_FOR_RESOURCES
            frameCounter = 0
            Log.i("ValidationRunner", "State set to WAITING_FOR_RESOURCES")
        } catch (e: Exception) {
            Log.e("ValidationRunner", "Failed to load $path", e)
            nextModel()
        }
    }

    fun onFrame(frameTimeNanos: Long) {
        if (frameCounter % 60 == 0) {
            Log.i("ValidationRunner", "onFrame: $currentState (frame: $frameCounter)")
        }

        when (currentState) {
            State.IDLE -> {}
            State.WAITING_FOR_RESOURCES -> {
                 val progress = modelViewer.progress
                 if (progress >= 1.0f) {
                     Log.i("ValidationRunner", "Resources loaded. Starting warmup.")
                     frameCounter = 0
                     currentState = State.WARMUP
                 }
            }
            State.WARMUP -> {
                frameCounter++
                if (frameCounter > 5) { // 5 frames warmup
                    startAutomation()
                }
            }
            State.RUNNING_TEST -> {
                // Log.i("ValidationRunner", "Running test...")
                currentEngine?.let { engine ->
                    val content = AutomationEngine.ViewerContent()
                    content.view = modelViewer.view
                    content.renderer = modelViewer.renderer
                    content.scene = modelViewer.scene
                    content.lightManager = modelViewer.engine.lightManager

                    // Tick
                    val deltaTime = 1.0f / 60.0f
                    engine.tick(modelViewer.engine, content, deltaTime)

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

    private fun startAutomation() {
        val test = currentTestConfig!!
        val specJson = JSONObject()
        specJson.put("name", test.name)
        specJson.put("base", test.rendering)
        val fullSpec = "[${specJson.toString()}]"

        currentEngine = AutomationEngine(fullSpec)
        val options = AutomationEngine.Options()
        options.sleepDuration = 0.0f // Minimal sleep, let frames drive it
        options.minFrameCount = 5 // Ensure some frames pass
        currentEngine?.setOptions(options)

        // Use batch mode to ensure shouldClose() works reliably
        currentEngine?.startBatchMode()
        currentEngine?.signalBatchMode() // Start immediately

        frameCounter = 0
        currentState = State.RUNNING_TEST
    }


    private fun captureAndCompare() {
        callback?.onStatusChanged("Comparing ${currentTestConfig?.name}...")
        modelViewer.debugGetNextFrameCallback { bitmap ->
            compareCapturedImage(bitmap)
        }
    }

    private fun compareCapturedImage(bitmap: Bitmap) {
         val testName = currentTestConfig!!.name
         val modelName = currentModelName!!
         val backend = currentTestConfig?.backends?.firstOrNull() ?: "opengl"
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
                                    callback?.onImageResult("Diff", result.diffImage!!)
                                    resultManager.saveImage("${testFullName}_diff", result.diffImage!!)
                                }
                             }
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

                val result = ValidationResult(testFullName, passed, diffMetric)
                resultManager.addResult(result)
                callback?.onTestFinished(result)

                android.os.Handler(android.os.Looper.getMainLooper()).post {
                    nextModel()
                }

             } catch (e: Exception) {
                 Log.e("ValidationRunner", "Comparison failed", e)
                 android.os.Handler(android.os.Looper.getMainLooper()).post { nextModel() }
             }
         }.start()
    }

    private fun nextModel() {
        currentModelIndex++
        if (currentTestConfig != null && currentModelIndex < currentTestConfig!!.models.size) {
            startModel(currentTestConfig!!.models.elementAt(currentModelIndex))
        } else {
            nextTest()
        }
    }


    private fun nextTest() {
        currentTestIndex++
        if (currentTestIndex < config.tests.size) {
            startTest(config.tests[currentTestIndex])
        } else {
            currentState = State.IDLE
            resultManager.finalizeResults()
            callback?.onAllTestsFinished()
        }
    }
}
