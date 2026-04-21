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
import android.os.FileObserver
import android.text.Html
import android.util.Log
import android.view.Choreographer
import android.view.SurfaceView
import android.view.View
import android.view.WindowManager
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.FrameLayout
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.ScrollView
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
    private lateinit var statusTextView: TextView
    private lateinit var testResultsHeader: TextView
    private lateinit var progressContainer: FrameLayout
    private lateinit var testProgress: com.google.android.filament.validation.TestProgressBar
    private lateinit var progressTriangle: ImageView
    private lateinit var scrollView: ScrollView
    private lateinit var testSummaryText: TextView
    private lateinit var deviceInfoText: TextView
    private lateinit var resultsContainer: LinearLayout
    private lateinit var inputManager: ValidationInputManager
    private var currentInput: ValidationInputManager.ValidationInput? = null
    private var fileObserver: FileObserver? = null

    private var currentAlphaDiffBitmap: Bitmap? = null
    private var globalEnhancementFactor: Float = 1.0f
    private var modelViewer: ModelViewer? = null

    private data class TestImages(
        val testName: String,
        val golden: File?,
        val rendered: File?,
        val diff: File?,
        val alphaDiff: File?
    )

    private val diffImageViews = mutableListOf<ImageView>()

    // UI Elements
    private lateinit var runButton: Button
    private lateinit var loadButton: Button
    private lateinit var optionsButton: Button
    private lateinit var enhancementContainer: LinearLayout
    private lateinit var backendFilterContainer: LinearLayout
    private lateinit var backendRadioGroup: android.widget.RadioGroup
    private lateinit var enhancementLabel: TextView
    private lateinit var enhancementSlider: android.widget.SeekBar

    private var resultManager: ValidationResultManager? = null
    private var validationRunner: ValidationRunner? = null

    // Frame callback
    private val frameScheduler = object : Choreographer.FrameCallback {
        override fun doFrame(frameTimeNanos: Long) {
            choreographer.postFrameCallback(this)
            val rendered = modelViewer?.render(frameTimeNanos) ?: false
            if (rendered) {
                validationRunner?.onFrame(frameTimeNanos)
            }
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
        progressContainer = findViewById(R.id.progress_container)
        testProgress = findViewById(R.id.test_progress)
        progressTriangle = findViewById(R.id.progress_triangle)
        scrollView = findViewById(R.id.scroll_view)
        testSummaryText = findViewById(R.id.test_summary_text)
        deviceInfoText = findViewById(R.id.device_info_text)
        deviceInfoText.text = "Running on: ${android.os.Build.MODEL}"
        resultsContainer = findViewById(R.id.results_container)

        scrollView.viewTreeObserver.addOnScrollChangedListener {
            updateTrianglePosition()
        }
        scrollView.viewTreeObserver.addOnGlobalLayoutListener {
            updateTrianglePosition()
        }

        runButton = findViewById(R.id.run_button)
        loadButton = findViewById(R.id.load_button)
        optionsButton = findViewById(R.id.options_button)
        enhancementContainer = findViewById(R.id.enhancement_container)
        backendFilterContainer = findViewById(R.id.backend_filter_container)
        backendRadioGroup = findViewById(R.id.backend_radio_group)
        enhancementLabel = findViewById(R.id.enhancement_label)
        enhancementSlider = findViewById(R.id.enhancement_slider)

        enhancementSlider.setOnSeekBarChangeListener(object : android.widget.SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: android.widget.SeekBar?, progress: Int, fromUser: Boolean) {
                globalEnhancementFactor = 1.0f + (progress / 100f) * 49.0f
                enhancementLabel.text = String.format(Locale.US, "Enhancement: %.1fx", globalEnhancementFactor)
                applyGlobalEnhancement()
            }
            override fun onStartTrackingTouch(seekBar: android.widget.SeekBar?) {}
            override fun onStopTrackingTouch(seekBar: android.widget.SeekBar?) {}
        })

        // Setup Run Button (Listener is set in handleIntent based on state)

        // Setup Load Button
        loadButton.setOnClickListener {
            showLoadDialog()
        }

        // Setup Options Menu Button
        optionsButton.setOnClickListener { view ->
            val popup = android.widget.PopupMenu(this, view)
            popup.menu.add(0, 4, 0, "Test ADB Info")
            popup.menu.add(0, 5, 0, "Result ADB Info")
            popup.menu.add(0, 6, 0, "Toggle Enhancement Slider")
            popup.menu.add(0, 7, 0, "Toggle Backend Filter")

            popup.setOnMenuItemClickListener { item ->
                when (item.itemId) {
                    4 -> showTestAdbInfo()
                    5 -> showResultAdbInfo()
                    6 -> {
                        enhancementContainer.visibility =
                            if (enhancementContainer.visibility == View.VISIBLE) View.GONE else View.VISIBLE
                    }
                    7 -> {
                        backendFilterContainer.visibility =
                            if (backendFilterContainer.visibility == View.VISIBLE) View.GONE else View.VISIBLE
                    }
                }
                true
            }
            popup.show()
        }

        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

        choreographer = Choreographer.getInstance()
        inputManager = ValidationInputManager(this)

        setupFileObserver()
        handleIntent()
    }

    private fun showLoadDialog() {
        val exportDir = inputManager.getBaseDir()
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
        val exportDir = inputManager.getBaseDir()
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
            message.append("<tt>cat &lt;filename&gt; | adb shell \"run-as $packageName sh -c 'cat &gt; files/&lt;filename&gt;'\"</tt><br>")
        } else {
            message.append("<tt>adb push &lt;filename&gt; $path/</tt><br>")
        }
        message.append("<br>Note: Use underscores instead of spaces in &lt;filename&gt;. The default bundle is <tt>default_test.zip</tt>.")

        AlertDialog.Builder(this)
            .setTitle("Test Bundle ADB Info")
            .setMessage(Html.fromHtml(message.toString(), Html.FROM_HTML_MODE_LEGACY))
            .setPositiveButton("OK", null)
            .show()
    }

    private fun showResultAdbInfo() {
        val exportDir = inputManager.getBaseDir()
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
                 // Clear existing results UI and state
                 resultsContainer.removeAllViews()
                 diffImageViews.clear()
                 resultManager = null

                 val input = inputManager.loadValidationInputFromZip(file)
                 onInputUpdated(input)
                 Log.i(TAG, "Setting header to: Test Results: ${input!!.config!!.name}")
             } catch (e: Exception) {
                 Log.e(TAG, "Failed to load zip", e)
                 statusTextView.text = "Error: ${e.message}"
             }
         }
    }

    private fun createIndirectLight() {
        try {
            modelViewer?.let { mv ->
                val engine = mv.engine
                val scene = mv.scene
                val iblName = "default_env"

                fun readAsset(path: String): ByteBuffer {
                    val input = assets.open(path)
                    val bytes = input.readBytes()
                    return ByteBuffer.wrap(bytes)
                }

                readAsset("envs/$iblName/${iblName}_ibl.ktx").let {
                    val bundle = KTX1Loader.createIndirectLight(engine, it)
                    scene.indirectLight = bundle.indirectLight
                    mv.indirectLightCubemap = bundle.cubemap
                    scene.indirectLight!!.intensity = 30_000.0f
                }

                readAsset("envs/$iblName/${iblName}_skybox.ktx").let {
                    val bundle = KTX1Loader.createSkybox(engine, it)
                    scene.skybox = bundle.skybox
                    mv.skyboxCubemap = bundle.cubemap
                }
                Log.i(TAG, "IBL loaded successfully")
            }
        } catch (e: Exception) {
            Log.e(TAG, "Failed to load IBL", e)
            statusTextView.text = "Warning: Failed to load IBL"
        }
    }

    private fun onInputUpdated(input: ValidationInputManager.ValidationInput) {
        loadButton.isEnabled = inputManager.hasAnyTest();

        if (input.config == null  && !inputManager.hasDefaultTest()) {
            statusTextView.text = "No test loaded. Tap Generate Goldens to create a default test."
            testResultsHeader.text = "No Test Loaded"
            runButton.text = "Generate Goldens"
            runButton.isEnabled = true
            runButton.setOnClickListener {
                statusTextView.text = "Generating default goldens..."
                CoroutineScope(Dispatchers.Main).launch {
                    try {
                        // Pressing this button is as if we received an intent to generate goldens
                        val emptyIntent = Intent()
                        emptyIntent.putExtra("generate_goldens", true)
                        onInputUpdated(inputManager.resolveConfig(emptyIntent))
                    } catch (e: Exception) {
                        statusTextView.text = "Failed to generate goldens: ${e.message}"
                        Log.e(TAG, "Failed to generate goldens", e)
                    }
                }
            }
            return;
        }

        if (input.config == null && inputManager.hasAnyTest()) {
            // User can choose to load at least one test
            statusTextView.text = "No test loaded. Please select a test to run."
            runButton.text = "No Test Loaded"
            runButton.isEnabled = false
            runButton.setOnClickListener(null)
            return;
        }

        // Here the input has a configuration, meaning we can run it, so just copy it but account
        // for the existance of the default test.
        currentInput = input.copy(
            generateGoldens = input.generateGoldens && !inputManager.hasDefaultTest()
        )
        currentInput?.let {
            runButton.text = "Run Test"
            runButton.isEnabled = true
            // Update header
            val name = it.config!!.name
            Log.i(TAG, "handleIntent: Setting header to: Test Results: ${name}")
            testResultsHeader.text = "${name}"

            if (it.autoRun) {
                startValidation(it)
                return
            }

            // Test is not running, but it is ready
            runButton.setOnClickListener {
                currentInput?.let { startValidation(it) }
            }

            // Clear existing results UI and state
            clearUIState();
            validationRunner?.let {
                it.cleanup()
                modelViewer = null
            }
            validationRunner = null

            // Just show status
            statusTextView.text = "Ready: ${name}"
        }
    }

    private fun handleIntent() {
        statusTextView.text = "Resolving configuration..."
        CoroutineScope(Dispatchers.Main).launch {
            try {
                onInputUpdated(inputManager.resolveConfig(intent))
            } catch (e: Exception) {
                Log.e(TAG, "Failed to resolve config", e)
                statusTextView.text = "Error: ${e.message}"
            }
        }
    }

    private fun setupFileObserver() {
        val baseDir = inputManager.getBaseDir()
        fileObserver = object : FileObserver(baseDir.absolutePath) {
            override fun onEvent(event: Int, path: String?) {
                if (path == null) return
                val isRelevant = (event and (FileObserver.CLOSE_WRITE or FileObserver.MOVED_TO)) != 0
                if (isRelevant && path.endsWith(".zip")) {
                    Log.i(TAG, "FileObserver: Detected new zip file: $path")

                    if (currentInput?.config == null) {
                        CoroutineScope(Dispatchers.Main).launch {
                            try {
                                // Pressing this button is as if we received an intent to generate goldens
                                val emptyIntent = Intent()
                                onInputUpdated(inputManager.resolveConfig(emptyIntent))
                            } catch (e: Exception) {
                                statusTextView.text = "Failed to update input on file change: ${e.message}"
                                Log.e(TAG, "Failed to update input on file change", e)
                            }
                        }
                    }
                }
            }
        }
        fileObserver?.startWatching()
    }
    private fun createResultManager(outputDir: File): ValidationResultManager {
        return ValidationResultManager(
            outputDir = outputDir,
            deviceName = android.os.Build.MODEL,
            deviceHardware = android.os.Build.HARDWARE,
            deviceCodeName = android.os.Build.DEVICE,
            androidVersion = android.os.Build.VERSION.RELEASE,
            androidBuildNumber = android.os.Build.DISPLAY
        )
    }

    private fun clearUIState() {
        resultsContainer.removeAllViews()
        diffImageViews.clear()
        resultManager = null
    }

    private fun startValidation(input: ValidationInputManager.ValidationInput) {
        try {
            val config = input.config ?: return

            clearUIState();

            // Disable UI while running
            enhancementSlider.isEnabled = false
            runButton.isEnabled = false
            loadButton.isEnabled = false
            optionsButton.isEnabled = false
            backendRadioGroup.isEnabled = false
            for (i in 0 until backendRadioGroup.childCount) {
                backendRadioGroup.getChildAt(i).isEnabled = false
            }

            Log.i(TAG, "Starting validation with config: ${config.name}")
            Log.i(TAG, "Output dir: ${input.outputDir.absolutePath}")

            testProgress.visibility = View.VISIBLE
            progressContainer.visibility = View.VISIBLE
            testSummaryText.visibility = View.GONE
            deviceInfoText.visibility = View.VISIBLE
            totalPassed = 0
            totalFailed = 0
            testProgress.reset(1)

            resultManager = createResultManager(input.outputDir)

            val backendFilter = when (backendRadioGroup.checkedRadioButtonId) {
                R.id.radio_gles -> "gles"
                R.id.radio_vulkan -> "vulkan"
                else -> "both"
            }

            // If we are starting another run, we need to clean up the previous runner's
            // resources before we can proceed.
            validationRunner?.let {
                it.cleanup()
                modelViewer = null
            }

            validationRunner = ValidationRunner(this, surfaceView, config, resultManager!!, backendFilter)
            validationRunner?.callback = this
            validationRunner?.generateGoldens = input.generateGoldens
            validationRunner?.testFilter = input.testFilter
            validationRunner?.start()

        } catch (e: Exception) {
            Log.e(TAG, "Failed to start validation", e)
            statusTextView.text = "Error: ${e.message}"
        }
    }

    override fun onModelViewerRecreated(modelViewer: ModelViewer?) {
        runOnUiThread {
            this.modelViewer = modelViewer
            // Re-apply IBL to the new engine/scene
            createIndirectLight()
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
        fileObserver?.stopWatching()
        validationRunner?.let {
            it.cleanup()
            modelViewer = null
        }
    }

    private var currentRenderedBitmap: Bitmap? = null
    private var currentGoldenBitmap: Bitmap? = null
    private var currentDiffBitmap: Bitmap? = null
    private var totalPassed = 0
    private var totalFailed = 0
    private var totalTestsCount = 1

    private fun updateTrianglePosition() {
        if (progressContainer.visibility != View.VISIBLE || resultsContainer.childCount == 0) return

        val scrollY = scrollView.scrollY
        val visibleHeight = scrollView.height
        val centerY = scrollY + visibleHeight / 2f

        var bestIndex = -1
        var minDistance = Float.MAX_VALUE
        for (i in 0 until resultsContainer.childCount) {
            val child = resultsContainer.getChildAt(i)
            val childCenterY = child.top + child.height / 2f
            val dist = Math.abs(childCenterY - centerY)
            if (dist < minDistance) {
                minDistance = dist
                bestIndex = i
            }
        }

        if (bestIndex >= 0) {
            val progressWidth = testProgress.width
            if (progressWidth > 0 && totalTestsCount > 0) {
                val segmentWidth = progressWidth.toFloat() / totalTestsCount
                val targetX = (bestIndex * segmentWidth) + (segmentWidth / 2f)
                progressTriangle.translationX = targetX - progressTriangle.width / 2f
            }
        }
    }

    override fun onTestFinished(result: ValidationResult) {
        runOnUiThread {
            if (result.passed) totalPassed++ else totalFailed++
            testProgress.addResult(result.passed)

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

            val outDir = resultManager!!.getOutputDir()
            val renderedFile = File(outDir, "${result.testName}.png")
            val diffFile = File(outDir, "${result.testName}_diff.png")
            val alphaDiffFile = File(outDir, "${result.testName}_alpha_diff.png")
            val goldenFile = result.goldenPath?.let { File(it) }

            val testImages = TestImages(
                testName = result.testName,
                golden = goldenFile?.takeIf { currentGoldenBitmap != null },
                rendered = renderedFile.takeIf { currentRenderedBitmap != null },
                diff = diffFile.takeIf { currentDiffBitmap != null },
                alphaDiff = alphaDiffFile.takeIf { currentAlphaDiffBitmap != null }
            )

            fun addImage(label: String, bitmap: Bitmap?, isDiff: Boolean) {
                if (bitmap == null) {
                    return;
                }
                val container = LinearLayout(this)
                container.orientation = LinearLayout.VERTICAL
                container.setPadding(0, 0, 10, 0)

                val labelView = TextView(this)
                labelView.text = label
                labelView.textSize = 9f
                container.addView(labelView)

                val iv = ImageView(this)
                iv.setImageBitmap(bitmap!!) // Use the same bitmap (or copy if needed, but same is usually fine for UI)
                iv.layoutParams = LinearLayout.LayoutParams(250, 250) // Smaller thumbnails
                iv.scaleType = ImageView.ScaleType.FIT_CENTER
                iv.setBackgroundColor(0xFF404040.toInt())

                if (isDiff) {
                    diffImageViews.add(iv)
                    applyEnhancementToView(iv, globalEnhancementFactor)
                }

                iv.setOnClickListener {
                    showImageDialog(testImages, label)
                }
                container.addView(iv)
                imagesRow.addView(container)
            }

            addImage("Rendered", currentRenderedBitmap, false)
            addImage("Golden", currentGoldenBitmap, false)
            if (!result.passed) {
                addImage("Diff", currentDiffBitmap, true)
            }
            if (currentAlphaDiffBitmap != null) {
                addImage("Alpha Diff", currentAlphaDiffBitmap, true)
            }

            resultContainer.addView(imagesRow)
            resultsContainer.addView(resultContainer)
            resultsContainer.post { updateTrianglePosition() }

            // Clear current images for next test
            currentRenderedBitmap = null
            currentGoldenBitmap = null
            currentDiffBitmap = null
            currentAlphaDiffBitmap = null
        }
    }

    override fun onTestProgress(current: Int, total: Int) {
        runOnUiThread {
            totalTestsCount = Math.max(1, total)
            testProgress.setMax(total)
            updateTrianglePosition()
        }
    }

    override fun onAllTestsFinished() {
        runOnUiThread {
            val total = totalPassed + totalFailed
            val colorPassed = if (totalPassed > 0) "#4CAF50" else "#000000"
            val colorFailed = if (totalFailed > 0) "#F44336" else "#000000"
            val html = "Passed: <font color='$colorPassed'><b>$totalPassed</b></font> / $total &nbsp;&nbsp;&nbsp; Failed: <font color='$colorFailed'><b>$totalFailed</b></font>"
            testSummaryText.text = Html.fromHtml(html, Html.FROM_HTML_MODE_LEGACY)
            testSummaryText.visibility = View.VISIBLE
            deviceInfoText.visibility = View.GONE
            statusTextView.text = "All tests finished!"

            // Re-enable UI
            enhancementSlider.isEnabled = true
            runButton.isEnabled = true
            loadButton.isEnabled = true
            optionsButton.isEnabled = true
            backendRadioGroup.isEnabled = true
            for (i in 0 until backendRadioGroup.childCount) {
                backendRadioGroup.getChildAt(i).isEnabled = true
            }

            Log.i(TAG, "All tests finished")

            currentInput?.let {
                if (it.generateGoldens) {
                    exportTestBundleAction(it)
                } else {
                    exportTestResultsAction(it)
                }
            }
        }
    }

    private fun exportTestBundleAction(input: ValidationInputManager.ValidationInput) {
        val config = input.config ?: return
        val timestamp = SimpleDateFormat("yyyyMMdd_HHmmss", Locale.US).format(Date())
        val rm = resultManager ?: createResultManager(input.outputDir)
        val bundleNameOverride = if (input.sourceZip == null) "default_test" else null
        val zip = rm.exportTestBundle(config, timestamp, bundleNameOverride)
        if (zip != null) {
            if (input.sourceZip == null) {
                val msg = "Exported Default Bundle: ${zip.name}."
                statusTextView.text = msg
                Log.i(TAG, "Exported test bundle to ${zip.absolutePath}.")
                CoroutineScope(Dispatchers.Main).launch {
                    try {
                        // here we don't want to call onInputUpdated(newInput) since we want to keep
                        // the state after the test ran
                        currentInput = inputManager.loadValidationInputFromZip(zip)

                        Log.i(TAG, "Auto-loaded default bundle.")
                    } catch (e: Exception) {
                        statusTextView.text = "Failed to auto-load default bundle: ${e.message}"
                        Log.e(TAG, "Failed to load default bundle", e)
                    }
                }
            } else {
                val msg = "Exported Bundle: ${zip.name}"
                statusTextView.text = msg
                Log.i(TAG, "Exported test bundle to ${zip.absolutePath}")
                }
        } else {
            statusTextView.text = "Export Bundle failed"
            Log.e(TAG, "Export Bundle failed")
        }
    }

    private fun exportTestResultsAction(input: ValidationInputManager.ValidationInput) {
        if (input.config == null) {
            return
        }
        val config = input.config!!
        val timestamp = SimpleDateFormat("yyyyMMdd_HHmmss", Locale.US).format(Date())
        val rm = resultManager ?: createResultManager(input.outputDir)
        val zip = rm.exportTestResults(input.sourceZip, config, timestamp)
        if (zip != null) {
            val msg = "Exported Results: ${zip.name}"
            statusTextView.text = msg
            Log.i(TAG, "Exported results to ${zip.absolutePath}")
        } else {
            statusTextView.text = "Export Results failed"
            Log.e(TAG, "Export Results failed")
        }
    }

    override fun onStatusChanged(status: String) {
        runOnUiThread {
            statusTextView.text = status
        }
    }

    override fun onImageResult(type: String, bitmap: Bitmap) {
        // Create a scaled-down thumbnail (e.g. 128x128) to save memory in the UI scroll view.
        // We use true for filter to smooth the scaling.
        val scaledBitmap = Bitmap.createScaledBitmap(bitmap, 128, 128, true)

        runOnUiThread {
            // Update the "live" views
            when (type) {
                "Rendered" -> {
                    currentRenderedBitmap = scaledBitmap
                }
                "Golden" -> {
                    currentGoldenBitmap = scaledBitmap
                }
                "Diff" -> {
                    currentDiffBitmap = scaledBitmap
                }
                "Alpha Diff" -> {
                    currentAlphaDiffBitmap = scaledBitmap
                }
            }
        }
    }

    private fun applyEnhancementToView(iv: ImageView, factor: Float) {
        val cm = android.graphics.ColorMatrix()
        cm.setScale(factor, factor, factor, 1.0f)
        iv.colorFilter = android.graphics.ColorMatrixColorFilter(cm)
    }

    private fun applyGlobalEnhancement() {
        for (iv in diffImageViews) {
            applyEnhancementToView(iv, globalEnhancementFactor)
        }
    }

    private fun showImageDialog(images: TestImages, initialLabel: String) {
        val dialogView = layoutInflater.inflate(R.layout.dialog_image_viewer, null)
        val dialog = AlertDialog.Builder(this)
            .setView(dialogView)
            .create()

        val titleView = dialogView.findViewById<TextView>(R.id.dialog_title)
        val typeView = dialogView.findViewById<TextView>(R.id.dialog_image_type)
        val imageView = dialogView.findViewById<ImageView>(R.id.dialog_image)
        val btnClose = dialogView.findViewById<View>(R.id.btn_close)
        val btnReset = dialogView.findViewById<View>(R.id.btn_reset)
        val btnPrev = dialogView.findViewById<View>(R.id.btn_prev)
        val btnNext = dialogView.findViewById<View>(R.id.btn_next)

        val enhancementContainer = dialogView.findViewById<View>(R.id.dialog_enhancement_container)
        val enhancementLabel = dialogView.findViewById<TextView>(R.id.dialog_enhancement_label)
        val enhancementSlider = dialogView.findViewById<android.widget.SeekBar>(R.id.dialog_enhancement_slider)

        titleView.text = images.testName

        val availableFiles = mutableListOf<Pair<String, File>>()
        images.rendered?.takeIf { it.exists() }?.let { availableFiles.add(Pair("Rendered", it)) }
        images.golden?.takeIf { it.exists() }?.let { availableFiles.add(Pair("Golden", it)) }
        images.diff?.takeIf { it.exists() }?.let { availableFiles.add(Pair("Diff", it)) }
        images.alphaDiff?.takeIf { it.exists() }?.let { availableFiles.add(Pair("Alpha Diff", it)) }

        if (availableFiles.isEmpty()) return

        var currentIndex = availableFiles.indexOfFirst { it.first == initialLabel }
        if (currentIndex == -1) currentIndex = 0

        var currentDialogBitmap: Bitmap? = null
        var currentDialogEnhancement = globalEnhancementFactor

        val matrix = android.graphics.Matrix()
        // Save initial values for translation tracking
        var lastTouchX = 0f
        var lastTouchY = 0f
        var isDragging = false

        // this is to prevent resetting the view matrix when flipping through goloden, rendered,
        // diff images.  These actions will trigger the OnLayoutChangeListener, but in these cases,
        // We want to keep the same view matrix.
        var matrixResettable = true

        val scaleDetector = android.view.ScaleGestureDetector(this, object : android.view.ScaleGestureDetector.SimpleOnScaleGestureListener() {
            override fun onScale(detector: android.view.ScaleGestureDetector): Boolean {
                matrix.postScale(detector.scaleFactor, detector.scaleFactor, detector.focusX, detector.focusY)
                imageView.imageMatrix = matrix
                return true
            }
        })

        imageView.setOnTouchListener { _, event ->
            scaleDetector.onTouchEvent(event)
            when (event.actionMasked) {
                android.view.MotionEvent.ACTION_DOWN -> {
                    lastTouchX = event.x
                    lastTouchY = event.y
                    isDragging = true
                }
                android.view.MotionEvent.ACTION_MOVE -> {
                    if (isDragging && !scaleDetector.isInProgress) {
                        val dx = event.x - lastTouchX
                        val dy = event.y - lastTouchY
                        matrix.postTranslate(dx, dy)
                        imageView.imageMatrix = matrix
                    }
                    lastTouchX = event.x
                    lastTouchY = event.y
                }
                android.view.MotionEvent.ACTION_UP, android.view.MotionEvent.ACTION_CANCEL -> {
                    isDragging = false
                }
            }
            true
        }

        fun updateView() {
            val (label, file) = availableFiles[currentIndex]
            typeView.text = label

            val oldBitmap = currentDialogBitmap
            currentDialogBitmap = android.graphics.BitmapFactory.decodeFile(file.absolutePath)

            imageView.setImageBitmap(currentDialogBitmap)
            (imageView.drawable as? android.graphics.drawable.BitmapDrawable)?.setAntiAlias(false)
            (imageView.drawable as? android.graphics.drawable.BitmapDrawable)?.setFilterBitmap(false)
            imageView.imageMatrix = matrix

            if (label == "Diff" || label == "Alpha Diff") {
                enhancementContainer.visibility = View.VISIBLE
                applyEnhancementToView(imageView, currentDialogEnhancement)
            } else {
                enhancementContainer.visibility = View.GONE
                imageView.colorFilter = null
            }
             oldBitmap?.recycle()
        }

        fun resetMatrix() {
            val drawable = imageView.drawable ?: return
            val width = imageView.width.toFloat()
            val height = imageView.height.toFloat()
            if (width <= 0f || height <= 0f) return

            val dw = drawable.intrinsicWidth.toFloat()
            val dh = drawable.intrinsicHeight.toFloat()
            if (dw <= 0f || dh <= 0f) return

            val scaleX = width / dw
            val scaleY = height / dh
            val scale = Math.min(scaleX, scaleY)

            val dx = (width - dw * scale) / 2f
            val dy = (height - dh * scale) / 2f

            matrix.reset()
            matrix.postScale(scale, scale)
            matrix.postTranslate(dx, dy)
            imageView.imageMatrix = matrix
        }

        imageView.addOnLayoutChangeListener { _, left, top, right, bottom, oldLeft, oldTop, oldRight, oldBottom ->
            if (matrixResettable &&
                    (left != oldLeft || right != oldRight || top != oldTop || bottom != oldBottom)) {
                resetMatrix()
            }
        }

        dialog.setOnDismissListener {
            matrixResettable = true;
            currentDialogBitmap?.recycle()
            currentDialogBitmap = null
        }


        btnClose.setOnClickListener { dialog.dismiss() }
        btnReset.setOnClickListener {
            matrixResettable = true
            resetMatrix()
        }
        btnPrev.setOnClickListener {
            currentIndex = (currentIndex - 1 + availableFiles.size) % availableFiles.size
            matrixResettable = false
            updateView()
        }
        btnNext.setOnClickListener {
            currentIndex = (currentIndex + 1) % availableFiles.size
            matrixResettable = false
            updateView()
        }

        val defaultProgress = ((currentDialogEnhancement - 1.0f) / 49.0f * 100).toInt()
        val safeProgress = Math.max(0, Math.min(100, defaultProgress))
        enhancementSlider.progress = safeProgress
        enhancementLabel.text = String.format(Locale.US, "Enhance: %.1fx", currentDialogEnhancement)

        enhancementSlider.setOnSeekBarChangeListener(object : android.widget.SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: android.widget.SeekBar?, progress: Int, fromUser: Boolean) {
                currentDialogEnhancement = 1.0f + (progress / 100f) * 49.0f
                enhancementLabel.text = String.format(Locale.US, "Enhance: %.1fx", currentDialogEnhancement)
                matrixResettable = false
                updateView()
            }
            override fun onStartTrackingTouch(seekBar: android.widget.SeekBar?) {}
            override fun onStopTrackingTouch(seekBar: android.widget.SeekBar?) {}
        })

        updateView()
        dialog.show()
    }
}
