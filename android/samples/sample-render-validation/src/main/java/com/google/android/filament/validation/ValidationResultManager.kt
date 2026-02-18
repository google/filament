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
        return null
    }

    /**
     * Exports a zip containing:
     * - results.json
     * - input test bundle (as nested zip), if provided
     * - diff images (if any failure)
     */
    fun exportTestResults(sourceZip: File?, timestamp: String): File? {
        // Safe parent dir resolution
        val parentDir = outputDir.canonicalFile.parentFile ?: outputDir.parentFile
        if (parentDir == null) return null

        val resultZipName = "results_$timestamp"
        val zipFile = File(parentDir, "$resultZipName.zip")

        Log.i(TAG, "Exporting results to ${zipFile.absolutePath}")

        try {
            ZipOutputStream(FileOutputStream(zipFile)).use { zos ->
                // 1. Add results.json
                val resultsJson = File(outputDir, "results.json")
                if (resultsJson.exists()) {
                    zos.putNextEntry(ZipEntry("results.json"))
                    resultsJson.inputStream().use { it.copyTo(zos) }
                    zos.closeEntry()
                }

                // 2. Add source zip if exists
                if (sourceZip != null && sourceZip.exists()) {
                    zos.putNextEntry(ZipEntry(sourceZip.name))
                    sourceZip.inputStream().use { it.copyTo(zos) }
                    zos.closeEntry()
                }

                // 3. Add diff images (any file ending in _diff.png in outputDir)
                outputDir.listFiles { _, name -> name.endsWith("_diff.png") }?.forEach { diffFile ->
                    zos.putNextEntry(ZipEntry(diffFile.name))
                    diffFile.inputStream().use { it.copyTo(zos) }
                    zos.closeEntry()
                }
            }
            Log.i(TAG, "Exported results to ${zipFile.absolutePath}")
            return zipFile
        } catch (e: Exception) {
            Log.e(TAG, "Failed to export results", e)
            return null
        }
    }

    /**
     * Exports a zip bundle containing:
     * - Modified config.json (with updated name and relative paths)
     * - Models (in models/ subdirectory)
     * - Golden images (in goldens/ subdirectory)
     *
     * Structure:
     *   test_name_TIMESTAMP/
     *     config.json
     *     models/
     *       model.glb
     *     goldens/
     *       test_result.png
     */
    fun exportTestBundle(config: RenderTestConfig, timestamp: String): File? {
        Log.i(TAG, "Starting exportTestBundle for ${config.name} at $timestamp")
        Log.i(TAG, "OutputDir: ${outputDir.absolutePath}")

        val parentDir = outputDir.canonicalFile.parentFile ?: outputDir.parentFile
        if (parentDir == null) {
            Log.e(TAG, "OutputDir parent is null: ${outputDir.absolutePath}")
            return null
        }
        Log.i(TAG, "Using parentDir for export: ${parentDir.absolutePath}")

        val testNameWithTimestamp = "${config.name}_$timestamp"
        val exportNameNoSpaces = testNameWithTimestamp.replace(" ", "_")

        val exportDir = File(parentDir, "export_temp_$timestamp")

        Log.i(TAG, "Creating export temp dir: ${exportDir.absolutePath}")
        if (exportDir.exists()) exportDir.deleteRecursively()
        if (!exportDir.mkdirs()) {
             Log.e(TAG, "Failed to create export dir: ${exportDir.absolutePath}")
             return null
        }

        val rootDir = File(exportDir, exportNameNoSpaces)
        rootDir.mkdirs()

        val modelsDir = File(rootDir, "models")
        modelsDir.mkdirs()

        val goldensDir = File(rootDir, "goldens")
        goldensDir.mkdirs()

        try {
            // 1. Copy Models and update config map
            val newModelsMap = mutableMapOf<String, String>()
            Log.i(TAG, "Copying models...")
            for ((modelName, modelPath) in config.models) {
                val sourceFile = File(modelPath)
                if (sourceFile.exists()) {
                    val destFile = File(modelsDir, sourceFile.name)
                    Log.d(TAG, "Copying model $modelName: $modelPath -> ${destFile.name}")
                    sourceFile.copyTo(destFile, overwrite = true)
                    // Use relative path for the new config
                    newModelsMap[modelName] = "models/${sourceFile.name}"
                } else {
                    Log.w(TAG, "Model file not found: $modelPath")
                }
            }

            // 2. Copy Golden Images
            // We assume goldens are in outputDir with .png extension
            Log.i(TAG, "Copying goldens from ${outputDir.absolutePath}...")
            outputDir.listFiles { _, name -> name.endsWith(".png") }?.forEach { file ->
                Log.d(TAG, "Copying golden: ${file.name}")
                file.copyTo(File(goldensDir, file.name), overwrite = true)
            }

            // 3. Create modified config JSON
            Log.i(TAG, "Creating config.json...")
            val newConfigJson = JSONObject()
            newConfigJson.put("name", testNameWithTimestamp) // Keep spaces in JSON name

            // Reconstruct backends
            val backendsArray = JSONArray()
            config.backends.forEach { backendsArray.put(it) }
            newConfigJson.put("backends", backendsArray)

            // Reconstruct models
            val modelsJson = JSONObject()
            for ((k, v) in newModelsMap) {
                modelsJson.put(k, v)
            }
            newConfigJson.put("models", modelsJson)

            // Reconstruct tests
            val testsArray = JSONArray()
            for (test in config.tests) {
                val testJson = JSONObject()
                testJson.put("name", test.name)
                if (test.description != null) testJson.put("description", test.description)

                // Models for this test (set of strings)
                val testModelsArray = JSONArray()
                test.models.forEach { testModelsArray.put(it) }
                testJson.put("models", testModelsArray)

                // Rendering settings
                testJson.put("rendering", test.rendering)

                // Tolerance
                if (test.tolerance != null) {
                    testJson.put("tolerance", test.tolerance)
                }

                // Backends (optional override)
                 val testBackends = JSONArray()
                 test.backends.forEach { testBackends.put(it) }
                 testJson.put("backends", testBackends)

                testsArray.put(testJson)
            }
            newConfigJson.put("tests", testsArray)

            // Write config.json
            File(rootDir, "config.json").writeText(newConfigJson.toString(4))

            // 4. Zip it
            val zipFile = File(parentDir, "$exportNameNoSpaces.zip")
            Log.i(TAG, "Zipping to ${zipFile.absolutePath}...")

            ZipOutputStream(FileOutputStream(zipFile)).use { zos ->
                rootDir.walkTopDown().forEach { file ->
                    if (file.isFile) {
                        val entryName = file.relativeTo(exportDir).path
                        zos.putNextEntry(ZipEntry(entryName))
                        file.inputStream().use { it.copyTo(zos) }
                        zos.closeEntry()
                    }
                }
            }

            // Cleanup temp dir
            exportDir.deleteRecursively()

            Log.i(TAG, "Exported test bundle to ${zipFile.absolutePath}")
            return zipFile

        } catch (e: Exception) {
            Log.e(TAG, "Failed to export test bundle", e)
            exportDir.deleteRecursively()
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
