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

package com.google.android.filament.taaharness

import android.app.Activity
import android.graphics.Color
import android.graphics.Typeface
import android.os.Bundle
import android.view.WindowManager
import android.widget.ScrollView
import android.widget.TextView

class MainActivity : Activity() {
    private lateinit var statusView: TextView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

        statusView = TextView(this).apply {
            setBackgroundColor(Color.BLACK)
            setTextColor(Color.WHITE)
            typeface = Typeface.MONOSPACE
            textSize = 11f
            setPadding(24, 24, 24, 24)
            text = "Filament TAA Harness ready."
        }
        val scroll = ScrollView(this).apply {
            setBackgroundColor(Color.BLACK)
            addView(statusView)
        }
        setContentView(scroll)

        val fromInstrumentation = intent?.getBooleanExtra("from_instrumentation", false) ?: false
        val argsStr = intent?.getStringExtra("args")
        val fixedPerf = intent?.getBooleanExtra("fixed_perf", false) ?: false
        if (!fromInstrumentation && !argsStr.isNullOrBlank()) {
            statusView.text = "Running taa_harness: $argsStr\n"
            Thread {
                val workDir = TaaHarnessNative.ensureAssetsExtracted(this)
                val parsedArgs = TaaHarnessNative.parseArgs(argsStr)
                val outOutput = arrayOfNulls<String>(1)
                val rc = TaaHarnessNative.nRunHarness(
                    workDir.absolutePath,
                    parsedArgs,
                    fixedPerf,
                    outOutput
                )
                val text = (outOutput[0] ?: "") + "\n[exit_code=$rc]"
                runOnUiThread {
                    statusView.text = text
                }
            }.start()
        }
    }

    fun updateStatus(message: String) {
        runOnUiThread {
            if (::statusView.isInitialized) {
                statusView.text = message
            }
        }
    }
}
