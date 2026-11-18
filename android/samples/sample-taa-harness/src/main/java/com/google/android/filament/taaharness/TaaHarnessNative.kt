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

import android.content.Context
import java.io.File

object TaaHarnessNative {
    init {
        System.loadLibrary("taa_harness")
    }

    @JvmStatic
    external fun nRunHarness(
        workingDir: String,
        args: Array<String>,
        fixedPerfMode: Boolean,
        outOutput: Array<String?>
    ): Int

    private val BUNDLED_ASSETS = listOf(
        "third_party/models/Fox/Fox.glb" to "third_party/models/Fox/Fox.glb",
        "third_party/models/DamagedHelmet/DamagedHelmet.glb" to "third_party/models/DamagedHelmet/DamagedHelmet.glb",
        "third_party/models/BusterDrone/BusterDrone.glb" to "third_party/models/BusterDrone/BusterDrone.glb",
        "tools/taa/models/SubpixelStar/SubpixelStar.glb" to "tools/taa/models/SubpixelStar/SubpixelStar.glb",
        "tools/taa/models/SSRReflection/SSRReflection.glb" to "tools/taa/models/SSRReflection/SSRReflection.glb",
        "ibl/lightroom_14b/lightroom_14b_ibl.ktx" to "assets/ibl/lightroom_14b/lightroom_14b_ibl.ktx",
        "ibl/lightroom_14b/lightroom_14b_skybox.ktx" to "assets/ibl/lightroom_14b/lightroom_14b_skybox.ktx",
        "ibl/lightroom_14b/sh.txt" to "assets/ibl/lightroom_14b/sh.txt"
    )

    fun ensureAssetsExtracted(context: Context): File {
        val baseDir = context.getExternalFilesDir(null) ?: context.filesDir
        val assetManager = context.assets
        for ((assetPath, destRelPath) in BUNDLED_ASSETS) {
            val destFile = File(baseDir, destRelPath)
            if (!destFile.exists() || destFile.length() == 0L) {
                destFile.parentFile?.mkdirs()
                assetManager.open(assetPath).use { input ->
                    destFile.outputStream().use { output ->
                        input.copyTo(output)
                    }
                }
            }
        }
        return baseDir
    }

    fun parseArgs(argsStr: String): Array<String> {
        val result = mutableListOf<String>()
        val current = StringBuilder()
        var inSingleQuote = false
        var inDoubleQuote = false
        for (ch in argsStr) {
            when {
                ch == '\'' && !inDoubleQuote -> inSingleQuote = !inSingleQuote
                ch == '"' && !inSingleQuote -> inDoubleQuote = !inDoubleQuote
                ch.isWhitespace() && !inSingleQuote && !inDoubleQuote -> {
                    if (current.isNotEmpty()) {
                        result.add(current.toString())
                        current.setLength(0)
                    }
                }
                else -> current.append(ch)
            }
        }
        if (current.isNotEmpty()) {
            result.add(current.toString())
        }
        return result.toTypedArray()
    }
}
