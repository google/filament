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

import android.app.Activity
import android.graphics.Bitmap
import android.os.Bundle
import android.util.Log
import android.view.Choreographer
import android.view.SurfaceView
import android.view.View
import android.view.WindowManager
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.TextView
import com.google.android.filament.utils.ModelViewer
import com.google.android.filament.utils.Utils
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import com.google.android.filament.utils.KTX1Loader
import com.google.android.filament.IndirectLight
import com.google.android.filament.Skybox
import android.graphics.Color
import java.io.File
import java.io.FileOutputStream
import java.net.HttpURLConnection
import java.net.URL
import java.nio.ByteBuffer
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.Spinner
import android.widget.AdapterView

class MainActivity : Activity(), ValidationRunner.Callback {

    companion object {
        init {
            Utils.init()
            System.loadLibrary("filament-utils-jni")
        }
        private const val TAG = "FilamentValidation"
    }

    private lateinit var surfaceView: SurfaceView
    private lateinit var choreographer: Choreographer
    private lateinit var modelViewer: ModelViewer
    private lateinit var statusTextView: TextView
    private lateinit var resultsContainer: LinearLayout
    private lateinit var inputManager: ValidationInputManager
    private var currentInput: ValidationInputManager.ValidationInput? = null
    private lateinit var modeSpinner: Spinner
    private lateinit var runButton: Button
    private var resultManager: ValidationResultManager? = null

    private var validationRunner: ValidationRunner? = null

    // Frame callback
    private val frameScheduler = object : Choreographer.FrameCallback {
        override fun doFrame(frameTimeNanos: Long) {
            choreographer.postFrameCallback(this)
            modelViewer.render(frameTimeNanos)
            validationRunner?.onFrame(frameTimeNanos)
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // SurfaceView container
        surfaceView = findViewById(R.id.surface_view)
        surfaceView.holder.setFixedSize(512, 512)

        statusTextView = findViewById(R.id.status_text)
        modeSpinner = findViewById(R.id.mode_spinner)
        runButton = findViewById(R.id.run_button)
        resultsContainer = findViewById(R.id.results_container)

        // Setup Spinner
        val modes = arrayOf("Run Validation", "Generate Goldens")
        val adapter = ArrayAdapter(this, android.R.layout.simple_spinner_item, modes)
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
        modeSpinner.adapter = adapter

        // Setup Run Button
        runButton.setOnClickListener {
            currentInput?.let { input ->
                val generateGoldens = modeSpinner.selectedItemPosition == 1
                val newInput = input.copy(generateGoldens = generateGoldens)
                startValidation(newInput)
            }
        }

        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

        choreographer = Choreographer.getInstance()
        modelViewer = ModelViewer(surfaceView)
        inputManager = ValidationInputManager(this)

        // Initialize IBL
        createIndirectLight()

        handleIntent()
    }

    private fun createIndirectLight() {
        try {
            val engine = modelViewer.engine
            val scene = modelViewer.scene
            val iblName = "default_env"

            fun readAsset(path: String): ByteBuffer {
                val input = assets.open(path)
                val bytes = input.readBytes()
                return ByteBuffer.wrap(bytes)
            }

            readAsset("envs/$iblName/${iblName}_ibl.ktx").let {
                val bundle = KTX1Loader.createIndirectLight(engine, it)
                scene.indirectLight = bundle.indirectLight
                modelViewer.indirectLightCubemap = bundle.cubemap
                scene.indirectLight!!.intensity = 30_000.0f
            }

            readAsset("envs/$iblName/${iblName}_skybox.ktx").let {
                val bundle = KTX1Loader.createSkybox(engine, it)
                scene.skybox = bundle.skybox
                modelViewer.skyboxCubemap = bundle.cubemap
            }
            Log.i(TAG, "IBL loaded successfully")
        } catch (e: Exception) {
            Log.e(TAG, "Failed to load IBL", e)
            statusTextView.text = "Warning: Failed to load IBL"
        }
    }

    private fun handleIntent() {
        statusTextView.text = "Resolving configuration..."
        CoroutineScope(Dispatchers.Main).launch {
            try {
                val input = inputManager.resolveConfig(intent)
                currentInput = input

                // Sync spinner with intent
                modeSpinner.setSelection(if (input.generateGoldens) 1 else 0)

                startValidation(input)
            } catch (e: Exception) {
                Log.e(TAG, "Failed to resolve config", e)
                statusTextView.text = "Error: ${e.message}"
            }
        }
    }

    private fun startValidation(input: ValidationInputManager.ValidationInput) {
        try {
            resultsContainer.removeAllViews()
            Log.i(TAG, "Starting validation with config: ${input.config.name}")
            Log.i(TAG, "Output dir: ${input.outputDir.absolutePath}")

            resultManager = ValidationResultManager(input.outputDir)

            validationRunner = ValidationRunner(this, modelViewer, input.config, resultManager!!)
            validationRunner?.callback = this
            validationRunner?.generateGoldens = input.generateGoldens
            validationRunner?.start()

            // Sync spinner in case it was called programmatically or changed implicitly
            modeSpinner.setSelection(if (input.generateGoldens) 1 else 0)

        } catch (e: Exception) {
            Log.e(TAG, "Failed to start validation", e)
            statusTextView.text = "Error: ${e.message}"
        }
    }

    override fun onResume() {
        super.onResume()
        choreographer.postFrameCallback(frameScheduler)
    }

    override fun onPause() {
        super.onPause()
        choreographer.removeFrameCallback(frameScheduler)
    }

    override fun onDestroy() {
        super.onDestroy()
        choreographer.removeFrameCallback(frameScheduler)
    }

    private var currentRenderedBitmap: Bitmap? = null
    private var currentGoldenBitmap: Bitmap? = null
    private var currentDiffBitmap: Bitmap? = null

    override fun onTestFinished(result: ValidationResult) {
        runOnUiThread {
            val status = "Test ${result.testName} finished: ${if(result.passed) "PASS" else "FAIL"}"
            statusTextView.text = status
            Log.i(TAG, status)

            // Container for this result
            val resultContainer = LinearLayout(this)
            resultContainer.orientation = LinearLayout.VERTICAL
            resultContainer.setPadding(0, 10, 0, 20)

            // Header
            val resultView = TextView(this)
            resultView.text = "${result.testName}: ${if(result.passed) "PASS" else "FAIL"} (Diff: ${result.diffMetric})"
            resultView.setTextColor(if(result.passed) Color.GREEN else Color.RED)
            resultView.textSize = 16f
            resultView.setTypeface(null, android.graphics.Typeface.BOLD)
            resultContainer.addView(resultView)

            // Images Row
            val imagesRow = LinearLayout(this)
            imagesRow.orientation = LinearLayout.HORIZONTAL

            fun addImage(label: String, bitmap: Bitmap?) {
                if (bitmap != null) {
                    val container = LinearLayout(this)
                    container.orientation = LinearLayout.VERTICAL
                    container.setPadding(0, 0, 10, 0)

                    val labelView = TextView(this)
                    labelView.text = label
                    labelView.textSize = 12f
                    container.addView(labelView)

                    val iv = ImageView(this)
                    iv.setImageBitmap(bitmap) // Use the same bitmap (or copy if needed, but same is usually fine for UI)
                    iv.layoutParams = LinearLayout.LayoutParams(250, 250) // Smaller thumbnails
                    iv.scaleType = ImageView.ScaleType.FIT_CENTER
                    iv.setBackgroundColor(0xFF404040.toInt())
                    container.addView(iv)

                    imagesRow.addView(container)
                }
            }

            addImage("Rendered", currentRenderedBitmap)
            addImage("Golden", currentGoldenBitmap)
            if (!result.passed) {
                addImage("Diff", currentDiffBitmap)
            }

            resultContainer.addView(imagesRow)
            resultsContainer.addView(resultContainer)

            // Clear current images for next test
            currentRenderedBitmap = null
            currentGoldenBitmap = null
            currentDiffBitmap = null
        }
    }

    override fun onAllTestsFinished() {
        runOnUiThread {
            statusTextView.text = "All tests finished!"
            Log.i(TAG, "All tests finished")
        }
    }

    override fun onStatusChanged(status: String) {
        runOnUiThread {
            statusTextView.text = status
        }
    }

    override fun onImageResult(type: String, bitmap: Bitmap) {
        runOnUiThread {
            // Update the "live" views
            when (type) {
                "Rendered" -> {
                    currentRenderedBitmap = bitmap
                }
                "Golden" -> {
                    currentGoldenBitmap = bitmap
                }
                "Diff" -> {
                    currentDiffBitmap = bitmap
                }
            }
        }
    }
}

/*
 * Scripts for reference:
 *
 * generate_goldens.sh:
 * --------------------
 * #!/bin/bash
 * set -e
 *
 * # Config path (on device)
 * CONFIG_PATH=$1
 * if [ -z "$CONFIG_PATH" ]; then
 *     echo "Usage: $0 <device_config_path>"
 *     echo "Example: $0 /sdcard/Android/data/com.google.android.filament.validation/files/default_test.json"
 *     exit 1
 * fi
 *
 * echo "Starting Golden Generation for $CONFIG_PATH..."
 * adb shell am force-stop com.google.android.filament.validation
 * adb shell am start -n com.google.android.filament.validation/.MainActivity \
 *     -e test_config "$CONFIG_PATH" \
 *     --ez generate_goldens true
 *
 * echo "Check device or logcat for progress."
 * echo "adb logcat -s FilamentValidation:I ValidationRunner:I"
 * echo "To pull results: ./samples/sample-render-validation/pull_goldens.sh"
 *
 * pull_goldens.sh:
 * ----------------
 * #!/bin/bash
 * set -e
 *
 * # Default destination is local golden directory relative to script
 * SCRIPT_DIR=$(cd $(dirname $0); pwd)
 * DEST_DIR=${1:-"$SCRIPT_DIR/golden"}
 *
 * echo "Pulling goldens to $DEST_DIR..."
 * mkdir -p "$DEST_DIR"
 *
 * # Path on device
 * DEVICE_GOLDEN_DIR="/storage/emulated/0/Android/data/com.google.android.filament.validation/files/golden/."
 *
 * adb pull "$DEVICE_GOLDEN_DIR" "$DEST_DIR"
 *
 * echo "Done."
 * ls -l "$DEST_DIR"
 */
