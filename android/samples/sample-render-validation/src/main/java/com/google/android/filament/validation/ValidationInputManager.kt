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
import android.content.Intent
import android.util.Log
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.json.JSONObject
import java.io.File
import java.io.FileOutputStream
import java.util.zip.ZipFile

class ValidationInputManager(private val context: Context) {

    companion object {
        private const val TAG = "ValidationInputManager"
    }

    data class ValidationInput(
        val config: RenderTestConfig?,
        val outputDir: File,
        val generateGoldens: Boolean,
        val autoRun: Boolean = false,
        val sourceZip: File? = null
    )

    public fun getBaseDir() : File {
        // Note that this is the app's internal directory. Not visible unless you're root or you're
        // this app.
        return context.filesDir
    }

    public fun hasDefaultTest(): Boolean {
        val defaultTestZip = File(getBaseDir(), "default_test.zip")
        return defaultTestZip.exists()
    }

    public fun hasAnyTest(): Boolean {
        val dir = getBaseDir()
        val matchedFiles = dir.listFiles {_, name ->
            name.endsWith("_test.zip")
        }
        return matchedFiles != null && matchedFiles.isNotEmpty()
    }

    /**
     * Resolves the test configuration based on the provided intent extras.
     * This may involve extracting assets or downloading files.
     */
    suspend fun resolveConfig(intent: Intent): ValidationInput = withContext(Dispatchers.IO) {
        val zipPath = intent.getStringExtra("zip_path")
        val generateGoldens = intent.getBooleanExtra("generate_goldens", false)

        // If we get a generateGoldens signal, then it should trigger a run
        val autoRun = intent.getBooleanExtra("auto_run", false) ||
            intent.getBooleanExtra("generate_goldens", false)

        val outputPath = intent.getStringExtra("output_path")

        Log.i(TAG, "Resolving config with outputPath: $outputPath")

        val baseDir = getBaseDir()
        Log.i(TAG, "Base directory for resolution: ${baseDir.absolutePath}")

        val outputDir = if (!outputPath.isNullOrBlank()) {
            val file = File(outputPath)
            val resolved = if (file.isAbsolute) {
                file
            } else {
                File(baseDir, outputPath)
            }

            // Critical check: if resolved is root or very short, it's likely wrong
            if (resolved.absolutePath == "/" || resolved.parent == null) {
                Log.w(TAG, "Resolved outputDir is root ($resolved), defaulting to app-specific dir")
                File(baseDir, "validation_results")
            } else {
                resolved
            }
        } else {
            File(baseDir, "validation_results")
        }

        if (!outputDir.exists() && !outputDir.mkdirs()) {
            Log.e(TAG, "Failed to create outputDir: ${outputDir.absolutePath}")
        }
        Log.i(TAG, "Final outputDir: ${outputDir.absolutePath}")

        var sourceZipFile = if (zipPath != null) {
            val file = File(zipPath)
            val resolvedFile = if (file.isAbsolute) {
                file
            } else {
                File(baseDir, zipPath)
            }
            Log.i(TAG, "Resolved zipPath '$zipPath' to ${resolvedFile.absolutePath} (exists: ${resolvedFile.exists()})")
            resolvedFile
        } else {
            null
        }

        // Auto-load logic: if no zipPath provided, and default_test.zip is the ONLY zip, auto-load it
        if (sourceZipFile == null) {
            val exportDir = baseDir
            val availableZips = exportDir.listFiles { _, name ->
                name.endsWith(".zip") && !name.startsWith("results_")
            }?.toList() ?: emptyList()

            if (availableZips.size == 1 && availableZips[0].name == "default_test.zip") {
                sourceZipFile = availableZips[0]
                Log.i(TAG, "Auto-loaded default_test.zip since it's the only test bundle available.")
            }
        }

        val config = when {
            sourceZipFile != null && sourceZipFile.exists() -> loadFromZip(sourceZipFile)
            generateGoldens -> extractDefaultAssets()
            else -> null
        }

        return@withContext ValidationInput(config, outputDir, generateGoldens, autoRun, sourceZipFile)
    }

    private var lastUnzippedFile: String? = null
    private var lastUnzippedTime: Long = 0

    suspend fun loadValidationInputFromZip(file: File) : ValidationInput {
        val config = loadFromZip(file)
        val baseDir = getBaseDir()
        val outputDir = File(baseDir, "validation_results").apply { mkdirs() }

        val newInput = ValidationInputManager.ValidationInput(
            config = config,
            outputDir = outputDir,
            generateGoldens = false,
            autoRun = false,
            sourceZip = file
        )
        return newInput
    }

    private suspend fun loadFromZip(zipFile: File): RenderTestConfig {
        val baseCacheDir = context.externalCacheDir ?: context.cacheDir
        val cacheDir = File(baseCacheDir, "unzipped_validation")

        if (lastUnzippedFile == zipFile.absolutePath && lastUnzippedTime == zipFile.lastModified() &&
                cacheDir.exists()) {
            Log.i(TAG, "Zip file unmodified, skipping unzip to preserve generated goldens.")
        } else {
            Log.i(TAG, "Unzipping validation bundle: ${zipFile.absolutePath}")
            if (cacheDir.exists()) cacheDir.deleteRecursively()
            cacheDir.mkdirs()

            Log.i(TAG, "Using cacheDir: ${cacheDir.absolutePath}")

            ZipFile(zipFile).use { zip ->
                val entries = zip.entries()
                while (entries.hasMoreElements()) {
                    val entry = entries.nextElement()
                    val entryFile = File(cacheDir, entry.name)
                    // specific check to avoid zip slip vulnerability (though low risk here)
                    if (!entryFile.canonicalPath.startsWith(cacheDir.canonicalPath)) {
                        throw SecurityException("Zip entry is outside of the target dir: ${entry.name}")
                    }

                    if (entry.isDirectory) {
                        entryFile.mkdirs()
                    } else {
                        entryFile.parentFile?.mkdirs()
                        zip.getInputStream(entry).use { input ->
                             FileOutputStream(entryFile).use { output ->
                                 input.copyTo(output)
                             }
                        }
                    }
                }
            }
            lastUnzippedFile = zipFile.absolutePath
            lastUnzippedTime = zipFile.lastModified()
        }

        // Find config.json
        // We look for a file ending in .json within the unzipped structure
        // Exclude results.json if it happened to be there
        val jsonFiles = cacheDir.walkTopDown()
            .filter { it.isFile && it.extension == "json" && it.name != "results.json" }
            .toList()

        if (jsonFiles.isEmpty()) throw IllegalStateException("No config.json found in zip")

        // Prefer one named config.json or take the first one
        val configFile = jsonFiles.find { it.name == "config.json" } ?: jsonFiles.first()

        Log.i(TAG, "Parsed config from ${configFile.absolutePath}")
        return ConfigParser.parseFromPath(configFile.absolutePath)
    }

    suspend fun extractDefaultAssets(): RenderTestConfig {
        Log.i(TAG, "Extracting default assets...")
        val filesDir = context.getExternalFilesDir(null) ?: context.filesDir
        val assetManager = context.assets

        // Copy default_test.json
        val configDir = File(filesDir, "config")
        configDir.mkdirs()
        val configOut = File(configDir, "default_test.json")

        assetManager.open("default_test.json").use { input ->
            FileOutputStream(configOut).use { output ->
                input.copyTo(output)
            }
        }

        // Copy DamagedHelmet.glb
        val modelsDir = File(filesDir, "models")
        modelsDir.mkdirs()
        val modelOut = File(modelsDir, "helmet.glb")

        assetManager.open("models/helmet.glb").use { input ->
            FileOutputStream(modelOut).use { output ->
                input.copyTo(output)
            }
        }

        // Update config to point to relative path (standardizing on relative for portability where possible)
        // or absolute. Here we use relative as per previous logic.
        val configJson = JSONObject(configOut.readText())

        // Dynamically name the test bundle based on device model
        configJson.put("name", "${android.os.Build.MODEL} Test")

        val models = configJson.getJSONObject("models")

        // Ensure the default model points to the extracted file
        // We can use absolute path to be safe since we know where it is now.
        models.put("DamagedHelmet", modelOut.absolutePath)

        configOut.writeText(configJson.toString(2))

        return ConfigParser.parseFromPath(configOut.absolutePath)
    }
}
