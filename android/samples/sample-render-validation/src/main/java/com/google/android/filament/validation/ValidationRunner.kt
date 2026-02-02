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
import com.google.android.filament.Engine
import com.google.android.filament.Renderer
import com.google.android.filament.View
import com.google.android.filament.utils.AutomationEngine
import com.google.android.filament.utils.ImageDiff
import com.google.android.filament.utils.ModelViewer
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.json.JSONObject
import java.io.File
import java.io.FileOutputStream
import java.nio.ByteBuffer
import java.nio.ByteOrder

class ValidationRunner(
    private val context: Context,
    private val modelViewer: ModelViewer,
    private val config: RenderTestConfig,
    private val outputDir: File
) {

    private var currentState = State.IDLE
    private var currentTestIndex = 0
    private var currentModelIndex = 0
    private var currentEngine: AutomationEngine? = null
    private var currentTestConfig: TestConfig? = null
    private var currentModelName: String? = null
    
    private var loadStartFence: com.google.android.filament.Fence? = null
    private var loadStartTime = 0L

    enum class State {
        IDLE,
        LOADING_MODEL,
        WAITING_FOR_FENCE,
        RUNNING_TEST,
        COMPARING
    }

    interface Callback {
        fun onTestFinished(result: TestResult)
        fun onAllTestsFinished()
        fun onStatusChanged(status: String)
    }

    var callback: Callback? = null

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
        
        currentState = State.LOADING_MODEL
        callback?.onStatusChanged("Loading $modelName for ${currentTestConfig?.name}")
        
        // Load model on main thread (required by ModelViewer)
        // We assume this is called from main thread or we dispatch
        loadModel(modelPath)
    }

    private fun loadModel(path: String) {
        // Assume called on Main Thread
        modelViewer.destroyModel()
        try {
            val bytes = File(path).readBytes()
            val buffer = ByteBuffer.wrap(bytes)
            modelViewer.loadModelGlb(buffer)
            modelViewer.transformToUnitCube()
            loadStartFence = modelViewer.engine.createFence()
            loadStartTime = System.nanoTime()
            currentState = State.WAITING_FOR_FENCE
        } catch (e: Exception) {
             Log.e("ValidationRunner", "Failed to load $path", e)
             nextModel()
        }
    }

    fun onFrame(frameTimeNanos: Long) {
        when (currentState) {
            State.IDLE -> {}
            State.WAITING_FOR_FENCE -> {
                loadStartFence?.let { fence ->
                     if (fence.wait(com.google.android.filament.Fence.Mode.FLUSH, 0) == com.google.android.filament.Fence.FenceStatus.CONDITION_SATISFIED) {
                         modelViewer.engine.destroyFence(fence)
                         loadStartFence = null
                         
                         // Compile materials (simplified)
                         modelViewer.scene.forEach { entity ->
                             // ... existing material compilation logic ...
                         }
                         
                         startAutomation()
                     }
                }
            }
            State.RUNNING_TEST -> {
                currentEngine?.let { engine ->
                    val content = AutomationEngine.ViewerContent()
                    content.view = modelViewer.view
                    content.renderer = modelViewer.renderer
                    content.scene = modelViewer.scene
                    content.lightManager = modelViewer.engine.lightManager
                    
                    // Tick
                    // Delta time? 
                    val deltaTime = 1.0f / 60.0f // Fixed step for consistency?
                    engine.tick(modelViewer.engine, content, deltaTime)
                    
                    if (!engine.isRunning) {
                        // Test finished (for this spec)
                        currentState = State.COMPARING
                        captureAndCompare()
                    }
                }
            }
            State.COMPARING -> {} // Busy
            State.LOADING_MODEL -> {}
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
        currentEngine?.startRunning()
        currentState = State.RUNNING_TEST
    }


    private fun captureAndCompare() {
        callback?.onStatusChanged("Comparing ${currentTestConfig?.name}...")
        val view = modelViewer.view
        val renderer = modelViewer.renderer
        val width = view.viewport.width
        val height = view.viewport.height

        val buffer = ByteBuffer.allocateDirect(width * height * 4)
        
        val pbd = com.google.android.filament.Texture.PixelBufferDescriptor(
            buffer,
            com.google.android.filament.Texture.Format.RGBA,
            com.google.android.filament.Texture.Type.UBYTE,
            1, 0, 0, 0, 0, // alignment, left, top, stride (0=default)
            null // handler (null = current thread? no, handler is for callback)
        ) {
            // Callback when readPixels is done
            // Dispatch to background thread for comparison to avoid blocking UI?
            // "it" is undefined here? The callback interface is Runnable?
            // Kotlin lambda for Runnable.
            compareCapturedImage(buffer, width, height)
        }
        renderer.readPixels(0, 0, width, height, pbd)
    }

    private fun compareCapturedImage(buffer: java.nio.Buffer, width: Int, height: Int) {
         // This runs on... which thread? Filament driver thread possibly.
         // We should use a helper to process.
         
         val testName = currentTestConfig!!.name
         val modelName = currentModelName!!
         val backend = "opengl" // Hardcoded for now, or get from View/Engine?
         val testFullName = "${testName}.${backend}.${modelName}"
         
         // Golden path
         // We expect a golden directory.
         val goldenFile = File(config.models.get(modelName)!!).parentFile.parentFile.resolve("golden/${testFullName}.png") 
         // Strategy: models are in .../models/model.glb
         // Goldens are in .../golden/
         
         Thread {
             try {
                // Convert buffer to Bitmap
                val bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888)
                bitmap.copyPixelsFromBuffer(buffer)
                
                // Flip Y? ReadPixels is typically bottom-up?
                // Filament readPixels is bottom-left? YES.
                // Bitmap is top-left.
                // We need to flip.
                val matrix = android.graphics.Matrix()
                matrix.postScale(1f, -1f)
                val flipped = Bitmap.createBitmap(bitmap, 0, 0, width, height, matrix, true)
                
                var passed = false
                if (goldenFile.exists()) {
                    val golden = android.graphics.BitmapFactory.decodeFile(goldenFile.absolutePath)
                    if (golden != null) {
                        // Populate tolerance from config
                        val tol = currentTestConfig?.tolerance ?: org.json.JSONObject()
                        val tolJson = tol.toString()
                        
                        val result = ImageDiff.compare(golden, flipped, tolJson, null)
                        passed = (result.status == ImageDiff.Result.Status.PASSED)
                        
                        // Save diff if failed?
                         if (!passed) {
                            val diffFile = File(outputDir, "${testFullName}_diff.png")
                            if (result.diffImage != null) {
                                FileOutputStream(diffFile).use { out ->
                                    result.diffImage.compress(Bitmap.CompressFormat.PNG, 100, out)
                                }
                            }
                         }
                    } else {
                        Log.e("ValidationRunner", "Failed to load golden: ${goldenFile.absolutePath}")
                    }
                } else {
                    Log.w("ValidationRunner", "Golden not found: ${goldenFile.absolutePath}")
                }
                
                // Save output
                val outFile = File(outputDir, "${testFullName}.png")
                FileOutputStream(outFile).use { out ->
                    flipped.compress(Bitmap.CompressFormat.PNG, 100, out)
                }

                callback?.onTestFinished(TestResult(testFullName, passed))
                
                // Schedule next model on main thread
                // Use Handler or View.post
                modelViewer.view.viewport
                // dispatch nextModel()
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
            zipResults()
            callback?.onAllTestsFinished()
        }
    }

    private fun zipResults() {
        callback?.onStatusChanged("Zipping results...")
        val zipFile = File(outputDir, "results.zip")
        try {
            java.util.zip.ZipOutputStream(java.io.FileOutputStream(zipFile)).use { zos ->
                outputDir.walkTopDown().filter { it.isFile && it.name != "results.zip" }.forEach { file ->
                    val entryName = file.relativeTo(outputDir).path
                    zos.putNextEntry(java.util.zip.ZipEntry(entryName))
                    file.inputStream().use { it.copyTo(zos) }
                    zos.closeEntry()
                }
            }
            Log.i("ValidationRunner", "Zipped results to ${zipFile.absolutePath}")
        } catch (e: Exception) {
            Log.e("ValidationRunner", "Failed to zip results", e)
        }
    }

    data class TestResult(val name: String, val passed: Boolean)

