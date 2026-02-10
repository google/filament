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
import android.os.Bundle
import android.util.Log
import android.view.Choreographer
import android.view.SurfaceView
import android.view.WindowManager
import android.widget.TextView
import android.widget.Toast
import com.google.android.filament.utils.ModelViewer
import com.google.android.filament.utils.Utils
import java.io.File

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
        
        // Simple layout
        val layout = android.widget.FrameLayout(this)
        surfaceView = SurfaceView(this)
        layout.addView(surfaceView)
        
        statusTextView = TextView(this)
        statusTextView.setTextColor(0xFFFFFFFF.toInt())
        statusTextView.textSize = 16f
        statusTextView.setPadding(20, 20, 20, 20)
        statusTextView.text = "Initializing..."
        layout.addView(statusTextView)
        
        setContentView(layout)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

        choreographer = Choreographer.getInstance()
        modelViewer = ModelViewer(surfaceView)
        
        // Check permissions? 
        // We assume 'adb install -g' or permissions granted.
        // But for scoped storage we might not need PERMISSION if reading from app-specific dirs, 
        // but user mentioned /sdcard/ so we need MANAGE_EXTERNAL_STORAGE or READ_EXTERNAL_STORAGE.
        // For waiting/simplicity, we just try.
        
        handleIntent()
    }
    
    private fun handleIntent() {
        val intent = intent
        val testConfigPath = intent.getStringExtra("test_config")
        
        if (testConfigPath != null) {
            startValidation(testConfigPath)
        } else {
             statusTextView.text = "No test_config provided via Intent.\nUse -e test_config <path>"
             Log.w(TAG, "No test config provided")
        }
    }

    private fun startValidation(configPath: String) {
        try {
            Log.i(TAG, "Parsing config from $configPath")
            val config = ConfigParser.parseFromPath(configPath)
            
            val outputDir = File(getExternalFilesDir(null), "validation_results")
            Log.i(TAG, "Output dir: ${outputDir.absolutePath}")
            
            validationRunner = ValidationRunner(this, modelViewer, config, outputDir)
            validationRunner?.callback = this
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

    override fun onPause() {
        super.onPause()
        choreographer.removeFrameCallback(frameScheduler)
    }

    override fun onDestroy() {
        super.onDestroy()
        choreographer.removeFrameCallback(frameScheduler)
    }

    override fun onTestFinished(result: ValidationRunner.TestResult) {
        runOnUiThread {
            val status = "Test ${result.name} finished: ${if(result.passed) "PASS" else "FAIL"}"
            statusTextView.text = status
            Log.i(TAG, status)
        }
    }

    override fun onAllTestsFinished() {
        runOnUiThread {
            statusTextView.text = "All tests finished!"
            Log.i(TAG, "All tests finished")
            // Optional: Auto-close activity?
            // finish()
        }
    }

    override fun onStatusChanged(status: String) {
        runOnUiThread {
            statusTextView.text = status
        }
    }
}

