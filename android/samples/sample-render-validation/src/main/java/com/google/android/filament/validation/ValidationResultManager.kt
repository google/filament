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

import android.graphics.Bitmap
import android.util.Log
import java.io.File
import java.io.FileOutputStream
import java.util.zip.ZipEntry
import java.util.zip.ZipOutputStream
import org.json.JSONArray
import org.json.JSONObject

data class ValidationResult(
    val testName: String,
    val passed: Boolean,
    val diffMetric: Float = 0f
)

class ValidationResultManager(private val outputDir: File) {

    companion object {
        private const val TAG = "ValidationResultManager"
    }

    private val results = mutableListOf<ValidationResult>()

    init {
        if (!outputDir.exists()) {
            outputDir.mkdirs()
        }
    }

    fun addResult(result: ValidationResult) {
        results.add(result)
    }

    fun saveImage(name: String, bitmap: Bitmap) {
        val file = File(outputDir, "$name.png")
        try {
            FileOutputStream(file).use { out ->
                bitmap.compress(Bitmap.CompressFormat.PNG, 100, out)
            }
        } catch (e: Exception) {
            Log.e(TAG, "Failed to save image $name", e)
        }
    }

    fun getOutputDir(): File {
        return outputDir
    }

    fun finalizeResults(): File? {
        // Write results JSON
        writeResultsJson()

        // Zip results
        val zipFile = File(outputDir, "results.zip")
        try {
            ZipOutputStream(FileOutputStream(zipFile)).use { zos ->
                outputDir.walkTopDown().filter { it.isFile && it.name != "results.zip" }.forEach { file ->
                    val entryName = file.relativeTo(outputDir).path
                    zos.putNextEntry(ZipEntry(entryName))
                    file.inputStream().use { it.copyTo(zos) }
                    zos.closeEntry()
                }
            }
            Log.i(TAG, "Zipped results to ${zipFile.absolutePath}")
            return zipFile
        } catch (e: Exception) {
            Log.e(TAG, "Failed to zip results", e)
            return null
        }
    }

    private fun writeResultsJson() {
        val jsonArray = JSONArray()
        for (result in results) {
            val jsonObject = JSONObject()
            jsonObject.put("test_name", result.testName)
            jsonObject.put("passed", result.passed)
            jsonObject.put("diff_metric", result.diffMetric)
            jsonArray.put(jsonObject)
        }

        val jsonFile = File(outputDir, "results.json")
        try {
            FileOutputStream(jsonFile).use { out ->
                out.write(jsonArray.toString(4).toByteArray())
            }
        } catch (e: Exception) {
            Log.e(TAG, "Failed to write results.json", e)
        }
    }
}
