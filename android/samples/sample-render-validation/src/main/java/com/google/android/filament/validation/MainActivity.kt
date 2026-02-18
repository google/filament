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
import android.app.AlertDialog
import android.content.Intent
import android.graphics.Bitmap
import android.graphics.Color
import android.os.Bundle
import android.text.Html
import android.util.Log
import android.view.Choreographer
import android.view.SurfaceView
import android.view.View
import android.view.WindowManager
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.Spinner
import android.widget.TextView
import com.google.android.filament.utils.KTX1Loader
import com.google.android.filament.utils.ModelViewer
import com.google.android.filament.utils.Utils
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.File
import java.nio.ByteBuffer
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

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
    private lateinit var testResultsHeader: TextView
    private lateinit var resultsContainer: LinearLayout
    private lateinit var inputManager: ValidationInputManager
    private var currentInput: ValidationInputManager.ValidationInput? = null

    // UI Elements
    private lateinit var runButton: Button
    private lateinit var loadButton: Button
    private lateinit var optionsButton: Button

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
        testResultsHeader = findViewById(R.id.test_results_header)
        resultsContainer = findViewById(R.id.results_container)

        runButton = findViewById(R.id.run_button)
        loadButton = findViewById(R.id.load_button)
        optionsButton = findViewById(R.id.options_button)

        // Setup Run Button
        runButton.setOnClickListener {
            currentInput?.let { input ->
                // Always use the generateGoldens flag from the intent/input
                startValidation(input)
            }
        }

        // Setup Load Button
        loadButton.setOnClickListener {
            showLoadDialog()
        }

        // Setup Options Menu Button
        optionsButton.setOnClickListener { view ->
            val popup = android.widget.PopupMenu(this, view)
            popup.menu.add(0, 1, 0, "Generate Golden")
            popup.menu.add(0, 2, 0, "Export Test")
            popup.menu.add(0, 3, 0, "Export Result")
            popup.menu.add(0, 4, 0, "Test ADB Info")
            popup.menu.add(0, 5, 0, "Result ADB Info")

            popup.setOnMenuItemClickListener { item ->
                when (item.itemId) {
                    1 -> {
                        currentInput?.let { input ->
                            val goldenInput = input.copy(generateGoldens = true)
                            startValidation(goldenInput)
                        }
                    }
                    2 -> exportTestBundleAction()
                    3 -> exportTestResultsAction()
                    4 -> showTestAdbInfo()
                    5 -> showResultAdbInfo()
                }
                true
            }
            popup.show()
        }

        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

        choreographer = Choreographer.getInstance()
        modelViewer = ModelViewer(surfaceView)
        inputManager = ValidationInputManager(this)

        // Initialize IBL
        createIndirectLight()

        handleIntent()
    }

    private fun showLoadDialog() {
        val exportDir = getExternalFilesDir(null) ?: filesDir
        // Filter out result zips (starting with "results_") to only show test bundles
        val zips = exportDir.listFiles { _, name ->
            name.endsWith(".zip") && !name.startsWith("results_")
        }?.sortedByDescending { it.lastModified() } ?: emptyList()

        if (zips.isEmpty()) {
            AlertDialog.Builder(this)
                .setTitle("Load Test")
                .setMessage("No test bundles found.")
                .setPositiveButton("OK", null)
                .show()
            return
        }

        val builder = AlertDialog.Builder(this)
        builder.setTitle("Select Test Bundle")

        val items = zips.map { it.name }.toTypedArray()

        builder.setItems(items) { dialog, which ->
            val selectedFile = zips[which]
            loadZipBundle(selectedFile)
            dialog.dismiss()
        }

        builder.setNegativeButton("Cancel", null)
        builder.show()
    }

    private fun showTestAdbInfo() {
        val exportDir = getExternalFilesDir(null) ?: filesDir
        val path = exportDir.absolutePath
        val isInternal = path.startsWith(filesDir.absolutePath)
        val message = StringBuilder()

        message.append("Storage Path: $path<br><br>")

        message.append("<b>--- PULL FROM DEVICE ---</b><br>")
        if (isInternal) {
            message.append("<tt>adb shell \"run-as $packageName cat files/&lt;filename&gt;\" &gt; &lt;filename&gt;</tt><br><br>")
        } else {
            message.append("<tt>adb pull $path/&lt;filename&gt; .</tt><br><br>")
        }

        message.append("<b>--- PUSH TO DEVICE ---</b><br>")
        if (isInternal) {
            message.append("1. <tt>adb push &lt;filename&gt; /sdcard/Download/</tt><br>")
            message.append("2. <tt>adb shell \"run-as $packageName cp /sdcard/Download/&lt;filename&gt; files/\"</tt><br>")
        } else {
            message.append("<tt>adb push &lt;filename&gt; $path/</tt><br>")
        }
        message.append("<br>Note: Use underscores instead of spaces in &lt;filename&gt;.")

        AlertDialog.Builder(this)
            .setTitle("Test Bundle ADB Info")
            .setMessage(Html.fromHtml(message.toString(), Html.FROM_HTML_MODE_LEGACY))
            .setPositiveButton("OK", null)
            .show()
    }

    private fun showResultAdbInfo() {
        val exportDir = getExternalFilesDir(null) ?: filesDir
        val path = exportDir.absolutePath
        val isInternal = path.startsWith(filesDir.absolutePath)
        val message = StringBuilder()

        message.append("<b>--- PULL RESULTS ---</b><br>")
        if (isInternal) {
            message.append("<tt>adb shell \"run-as $packageName cat files/&lt;filename&gt;\" &gt; &lt;filename&gt;</tt><br><br>")
        } else {
            message.append("<tt>adb pull $path/&lt;filename&gt; .</tt><br><br>")
        }

        message.append("<b>--- AVAILABLE RESULTS ---</b><br>")
        val zips = exportDir.listFiles { _, name ->
            name.endsWith(".zip") && name.startsWith("results_")
        }?.sortedByDescending { it.lastModified() } ?: emptyList()

        if (zips.isEmpty()) {
            message.append("No result zips found.<br>")
        } else {
            zips.forEach { file ->
                message.append("${file.name}<br>")
            }
        }

        AlertDialog.Builder(this)
            .setTitle("Result ADB Info")
            .setMessage(Html.fromHtml(message.toString(), Html.FROM_HTML_MODE_LEGACY))
            .setPositiveButton("OK", null)
            .show()
    }

    private fun loadZipBundle(file: File) {
         statusTextView.text = "Loading ${file.name}..."
         CoroutineScope(Dispatchers.Main).launch {
             try {
                 val config = inputManager.loadFromZip(file)
                 val baseDir = getExternalFilesDir(null) ?: filesDir
                 val outputDir = File(baseDir, "validation_results").apply { mkdirs() }

                 // Clear existing results UI and state
                 resultsContainer.removeAllViews()
                 resultManager = null

                 val newInput = ValidationInputManager.ValidationInput(
                     config = config,
                     outputDir = outputDir,
                     generateGoldens = false,
                     autoRun = false,
                     autoExport = false,
                     autoExportResults = false,
                     sourceZip = file
                 )

                 currentInput = newInput
                 statusTextView.text = "Loaded ${config.name}"
                 Log.i(TAG, "Setting header to: Test Results: ${config.name}")
                 testResultsHeader.text = "${config.name}"
             } catch (e: Exception) {
                 Log.e(TAG, "Failed to load zip", e)
                 statusTextView.text = "Error: ${e.message}"
             }
         }
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

                // Update header
                Log.i(TAG, "handleIntent: Setting header to: Test Results: ${input.config.name}")
                testResultsHeader.text = "${input.config.name}"
                currentInput = input

                if (input.autoRun) {
                    startValidation(input)
                } else {
                    // Just show status
                    statusTextView.text = "Ready: ${input.config.name}"
                }
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

            testResultsHeader.text = "${input.config.name}"

            resultManager = ValidationResultManager(input.outputDir)

            validationRunner = ValidationRunner(this, modelViewer, input.config, resultManager!!)
            validationRunner?.callback = this
            validationRunner?.generateGoldens = input.generateGoldens
            validationRunner?.start()

        } catch (e: Exception) {
            Log.e(TAG, "Failed to start validation", e)
            statusTextView.text = "Error: ${e.message}"
        }
    }

    override fun onResume() {
        super.onResume()
        choreographer.postFrameCallback(frameScheduler)
    }

    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
        setIntent(intent)
        handleIntent()
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

            // Header Layout
            val headerRow = LinearLayout(this)
            headerRow.orientation = LinearLayout.HORIZONTAL
            headerRow.gravity = android.view.Gravity.CENTER_VERTICAL

            // Status Icon + Name
            val statusView = TextView(this)
            val icon = if (result.passed) "✔" else "✖"

            statusView.text = "$icon ${result.testName}"
            statusView.setTextColor(
                if (result.passed) Color.parseColor("#4CAF50") else Color.parseColor("#F44336")
            )
            statusView.textSize = 12f
            statusView.setTypeface(null, android.graphics.Typeface.BOLD)
            headerRow.addView(statusView)

            // Diff Metric (only show if it's relevant/there's a diff or we just always show it like before)
            val diffView = TextView(this)
            diffView.text = "  (Diff: ${result.diffMetric})"
            diffView.textSize = 12f
            diffView.setTextColor(Color.GRAY)
            headerRow.addView(diffView)
            resultContainer.addView(headerRow)

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
                    labelView.textSize = 9f
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
            Log.i(TAG, "All tests finished " + if (currentInput?.autoExport == true) "Exporting bundle" else "x")

            if (currentInput?.autoExport == true) {
                exportTestBundleAction()
            }
            if (currentInput?.autoExportResults == true) {
                exportTestResultsAction()
            }
        }
    }

    private fun exportTestBundleAction() {
        currentInput?.let { input ->
            val timestamp = SimpleDateFormat("yyyyMMdd_HHmmss", Locale.US).format(Date())
            val rm = resultManager ?: ValidationResultManager(input.outputDir)
            val zip = rm.exportTestBundle(input.config, timestamp)
            if (zip != null) {
                val msg = "Exported Bundle: ${zip.name}"
                statusTextView.text = msg
                Log.i(TAG, "Exported test bundle to ${zip.absolutePath}")
            } else {
                statusTextView.text = "Export Bundle failed"
                Log.e(TAG, "Export Bundle failed")
            }
        }
    }

    private fun exportTestResultsAction() {
        currentInput?.let { input ->
            val timestamp = SimpleDateFormat("yyyyMMdd_HHmmss", Locale.US).format(Date())
            val rm = resultManager ?: ValidationResultManager(input.outputDir)
            val zip = rm.exportTestResults(input.sourceZip, timestamp)
            if (zip != null) {
                val msg = "Exported Results: ${zip.name}"
                statusTextView.text = msg
                Log.i(TAG, "Exported results to ${zip.absolutePath}")
            } else {
                statusTextView.text = "Export Results failed"
                Log.e(TAG, "Export Results failed")
            }
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
