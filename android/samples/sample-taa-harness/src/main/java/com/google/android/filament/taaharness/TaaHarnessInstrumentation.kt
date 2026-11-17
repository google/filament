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
import android.app.Instrumentation
import android.content.Intent
import android.os.Bundle

class TaaHarnessInstrumentation : Instrumentation() {
    private var arguments: Bundle? = null

    override fun onCreate(arguments: Bundle?) {
        super.onCreate(arguments)
        this.arguments = arguments
        start()
    }

    override fun onStart() {
        super.onStart()
        val argsStr = arguments?.getString("args") ?: "-b -a vulkan"
        val fixedPerfStr = arguments?.getString("fixed_perf") ?: "0"
        val fixedPerf = fixedPerfStr == "1" || fixedPerfStr.equals("true", ignoreCase = true)

        val workDir = TaaHarnessNative.ensureAssetsExtracted(targetContext)

        val launchIntent = Intent(targetContext, MainActivity::class.java).apply {
            addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
            putExtra("from_instrumentation", true)
        }
        val activity = startActivitySync(launchIntent) as? MainActivity
        activity?.updateStatus("Running: taa_harness $argsStr\nWorkDir: ${workDir.absolutePath}")

        val parsedArgs = TaaHarnessNative.parseArgs(argsStr)
        val outCaptured = arrayOfNulls<String>(1)
        val rc = TaaHarnessNative.nRunHarness(
            workDir.absolutePath,
            parsedArgs,
            fixedPerf,
            outCaptured
        )

        val outputText = outCaptured[0] ?: ""
        activity?.updateStatus(outputText + "\n[exit_code=$rc]")
        activity?.finish()

        val results = Bundle().apply {
            putString(
                REPORT_KEY_STREAMRESULT,
                "\n$outputText\n[taa_harness exit_code=$rc working_dir=${workDir.absolutePath}]\n"
            )
            putInt("exit_code", rc)
            putString("working_dir", workDir.absolutePath)
        }
        finish(if (rc == 0) Activity.RESULT_OK else Activity.RESULT_CANCELED, results)
    }
}
