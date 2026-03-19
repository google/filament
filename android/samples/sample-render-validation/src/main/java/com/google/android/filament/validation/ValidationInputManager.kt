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
import java.net.HttpURLConnection
import java.net.URL
import java.util.zip.ZipFile

/**
 * Handles the retrieval and preparation of test configuration and assets.
 * Supports loading from:
 * 1. Intent extras (local path or URL)
 * 2. Default embedded assets (fallback)
 */
class ValidationInputManager(private val context: Context) {

    companion object {
        private const val TAG = "ValidationInputManager"
    }

    data class ValidationInput(
        val config: RenderTestConfig,
        val outputDir: File,
        val generateGoldens: Boolean,
        val autoRun: Boolean = false,
        val autoExport: Boolean = false,
        val autoExportResults: Boolean = false,
        val sourceZip: File? = null
    )

    /**
     * Resolves the test configuration based on the provided intent extras.
     * This may involve extracting assets or downloading files.
     */
    suspend fun resolveConfig(intent: Intent): ValidationInput = withContext(Dispatchers.IO) {
        val testConfigPath = intent.getStringExtra("test_config")
        val urlConfig = intent.getStringExtra("url_config")
        val urlModelsBase = intent.getStringExtra("url_models_base")
        val zipPath = intent.getStringExtra("zip_path")
        val generateGoldens = intent.getBooleanExtra("generate_goldens", false)
        val autoRun = intent.getBooleanExtra("auto_run", false)
        val autoExport = intent.getBooleanExtra("auto_export", false)
        val autoExportResults = intent.getBooleanExtra("auto_export_results", false)
        val outputPath = intent.getStringExtra("output_path")

        Log.i(TAG, "Resolving config with outputPath: $outputPath")

        val baseDir = context.getExternalFilesDir(null) ?: context.filesDir
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

        val sourceZipFile = if (zipPath != null) {
            val file = File(zipPath)
            if (file.isAbsolute) {
                file
            } else {
                File(baseDir, zipPath)
            }
        } else {
            null
        }

        val config = when {
            sourceZipFile != null && sourceZipFile.exists() -> loadFromZip(sourceZipFile)
            urlConfig != null -> downloadConfig(urlConfig, urlModelsBase)
            testConfigPath != null -> ConfigParser.parseFromPath(testConfigPath)
            else -> extractDefaultAssets()
        }

        return@withContext ValidationInput(config, outputDir, generateGoldens, autoRun, autoExport, autoExportResults, sourceZipFile)
    }

    suspend fun loadFromZip(zipFile: File): RenderTestConfig {
        Log.i(TAG, "Unzipping validation bundle: ${zipFile.absolutePath}")
        // Use a unique cache dir based on timestamp or just overwrite a common one
        // Overwriting is safer to avoid filling up disk, but we must ensure we don't conflict with current run
        val baseCacheDir = context.externalCacheDir ?: context.cacheDir
        val cacheDir = File(baseCacheDir, "unzipped_validation")
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

    private suspend fun extractDefaultAssets(): RenderTestConfig {
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
        val models = configJson.getJSONObject("models")

        // Ensure the default model points to the extracted file
        // We can use absolute path to be safe since we know where it is now.
        models.put("DamagedHelmet", modelOut.absolutePath)

        configOut.writeText(configJson.toString(2))

        return ConfigParser.parseFromPath(configOut.absolutePath)
    }

    private suspend fun downloadConfig(urlConfig: String, urlModelsBase: String?): RenderTestConfig {
        Log.i(TAG, "Downloading config from $urlConfig")
        val filesDir = context.getExternalFilesDir(null) ?: context.filesDir
        val configDir = File(filesDir, "config")
        configDir.mkdirs()

        val modelsDir = File(filesDir, "models")
        modelsDir.mkdirs()

        val configName = "downloaded_config.json"
        val configFile = File(configDir, configName)

        downloadFile(urlConfig, configFile)

        if (urlModelsBase != null) {
            val configJson = JSONObject(configFile.readText())
            val models = configJson.optJSONObject("models")
            if (models != null) {
                val keys = models.keys()
                while (keys.hasNext()) {
                    val key = keys.next()
                    val modelPath = models.getString(key)
                    val fileName = File(modelPath).name
                    val modelFile = File(modelsDir, fileName)
                    val modelUrl = "$urlModelsBase/$fileName"

                    Log.i(TAG, "Downloading model: $fileName from $modelUrl")
                    downloadFile(modelUrl, modelFile)

                    // Update config to point to absolute path
                    models.put(key, modelFile.absolutePath)
                }
                configFile.writeText(configJson.toString())
            }
        }

        return ConfigParser.parseFromPath(configFile.absolutePath)
    }

    private fun downloadFile(urlStr: String, destFile: File) {
        val url = URL(urlStr)
        val connection = url.openConnection() as HttpURLConnection
        connection.connect()

        if (connection.responseCode != HttpURLConnection.HTTP_OK) {
             throw Exception("Server returned HTTP ${connection.responseCode} for $urlStr")
        }

        destFile.parentFile?.mkdirs()
        connection.inputStream.use { input ->
            FileOutputStream(destFile).use { output ->
                input.copyTo(output)
            }
        }
    }
}
