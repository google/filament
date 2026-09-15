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

import org.json.JSONArray
import org.json.JSONObject
import java.io.File
import java.io.IOException

data class RenderTestConfig(
    val name: String,
    val backends: List<String>,
    val models: Map<String, String>, // name -> path
    val tests: List<TestConfig>
)

data class TestConfig(
    val name: String,
    val description: String?,
    val backend: String,
    val models: Set<String>,
    val rendering: JSONObject,
    val tolerance: JSONObject?
)


// See test/renderdiff/FORMAT.md for the full specification matched by this parser.
class ConfigParser {
    companion object {
        fun parseFromPath(path: String): RenderTestConfig {
            val file = File(path)
            val jsonTxt = removeComments(file.readText())
            val json = JSONObject(jsonTxt)
            return parseRenderTestConfig(json, file.parentFile)
        }

        private fun removeComments(json: String): String {
            return json.lines().joinToString("\n") { it.substringBefore("//") }
        }

        private fun parseRenderTestConfig(json: JSONObject, baseDir: File?): RenderTestConfig {
            val name = json.getString("name")
            val backends = (json.optJSONArray("renderers") ?: json.getJSONArray("backends")).toList<String>()

            val modelSearchPaths = json.optJSONArray("model_search_paths")?.toList<String>() ?: emptyList()
            val models = mutableMapOf<String, String>()

            baseDir?.let { dir ->
                modelSearchPaths.forEach { searchPath ->
                    val searchDir = File(dir, searchPath)
                    if (searchDir.exists()) {
                        searchDir.walkTopDown().filter { it.isFile && (it.extension == "glb" || it.extension == "gltf") }.forEach { file ->
                            models[file.nameWithoutExtension] = file.absolutePath
                        }
                    }
                }
            }

            // Explicit models map override
            val modelsJson = json.optJSONObject("models")
            if (modelsJson != null) {
                val keys = modelsJson.keys()
                while (keys.hasNext()) {
                    val mName = keys.next()
                    val path = modelsJson.getString(mName)
                    // Resolve path relative to baseDir if not absolute
                    val file = File(path)
                    if (file.isAbsolute) {
                        models[mName] = path
                    } else if (baseDir != null) {
                        models[mName] = File(baseDir, path).absolutePath
                    } else {
                        models[mName] = path
                    }
                }
            }

            val presetsJson = json.optJSONArray("presets")
            val presets = mutableMapOf<String, PresetConfig>()
            if (presetsJson != null) {
                for (i in 0 until presetsJson.length()) {
                    val p = parsePreset(presetsJson.getJSONObject(i), models.keys)
                    presets[p.name] = p
                }
            }

            val testsJson = json.getJSONArray("tests")
            val tests = mutableListOf<TestConfig>()
            for (i in 0 until testsJson.length()) {
                tests.addAll(parseTestConfig(testsJson.getJSONObject(i), models.keys, presets, backends))
            }

            return RenderTestConfig(name, backends, models, tests)
        }

        private fun parsePreset(json: JSONObject, existingModels: Set<String>): PresetConfig {
            val name = json.getString("name")
            val tolerance = json.optJSONObject("tolerance")

            // Strict schema: reject legacy flat fields
            if (json.has("rendering") || json.has("models") || json.has("model_search_paths") || json.has("model_list_file")) {
                throw IllegalArgumentException("Preset '$name' contains legacy root fields; must use 'gltf_test' or 'sample_test'")
            }
            if (json.has("gltf_test") && json.has("sample_test")) {
                throw IllegalArgumentException("Preset '$name' cannot define both 'gltf_test' and 'sample_test'")
            }

            // Parse gltf_test block if present
            val gltfJson = json.optJSONObject("gltf_test")
            val rendering = gltfJson?.optJSONObject("rendering") ?: JSONObject()
            val models = gltfJson?.optJSONArray("models")?.toList<String>() ?: emptyList()

            // Validate models if present
            if (existingModels.isNotEmpty()) {
                models.forEach { if (!existingModels.contains(it)) throw IllegalArgumentException("Model $it not found") }
            }

            return PresetConfig(name, rendering, models, tolerance)
        }

        private fun parseTestConfig(
            json: JSONObject,
            existingModels: Set<String>,
            presets: Map<String, PresetConfig>,
            defaultBackends: List<String>
        ): List<TestConfig> {
            val name = json.getString("name")
            val description = json.optString("description")
            val backends = (json.optJSONArray("renderers") ?: json.optJSONArray("backends"))?.toList<String>() ?: defaultBackends

            val applyPresets = json.optJSONArray("apply_presets")?.toList<String>() ?: emptyList()

            // Strict schema: reject legacy flat fields
            if (json.has("rendering") || json.has("models") || json.has("model_search_paths") || json.has("model_list_file")) {
                throw IllegalArgumentException("Test '$name' contains legacy root fields; must use 'gltf_test' or 'sample_test'")
            }

            val hasGltf = json.has("gltf_test")
            val hasSample = json.has("sample_test")
            if (!hasGltf && !hasSample) {
                throw IllegalArgumentException("Test '$name' must define exactly one of 'gltf_test' or 'sample_test'")
            }
            if (hasGltf && hasSample) {
                throw IllegalArgumentException("Test '$name' cannot define both 'gltf_test' and 'sample_test'")
            }

            if (hasSample) {
                // Standalone sample tests are desktop binaries, skip for on-device glTF validation
                return emptyList()
            }

            val rendering = JSONObject()
            val combinedModels = mutableSetOf<String>()
            var lastTolerance: JSONObject? = null

            applyPresets.forEach { presetName ->
                val preset = presets[presetName] ?: throw IllegalArgumentException("Unknown preset $presetName")
                // Merge rendering (flat copy)
                val keys = preset.rendering.keys()
                while (keys.hasNext()) {
                    val k = keys.next()
                    rendering.put(k, preset.rendering.get(k))
                }
                combinedModels.addAll(preset.models)
                if (preset.tolerance != null) lastTolerance = preset.tolerance
            }

            val gltfJson = json.getJSONObject("gltf_test")
            val testRendering = gltfJson.optJSONObject("rendering")
            if (testRendering != null) {
                val keys = testRendering.keys()
                while (keys.hasNext()) {
                    val k = keys.next()
                    rendering.put(k, testRendering.get(k))
                }
            }

            val testModels = gltfJson.optJSONArray("models")?.toList<String>() ?: emptyList()
            combinedModels.addAll(testModels)

            // Validate models
            if (existingModels.isNotEmpty()) {
                combinedModels.forEach { if (!existingModels.contains(it)) throw IllegalArgumentException("Model $it not found") }
            }

            val tolerance = json.optJSONObject("tolerance") ?: lastTolerance

            return backends.map { backend ->
                TestConfig(name, description, backend, combinedModels, rendering, tolerance)
            }
        }
    }
}

data class PresetConfig(
    val name: String,
    val rendering: JSONObject,
    val models: List<String>,
    val tolerance: JSONObject?
)

private inline fun <reified T> JSONArray.toList(): List<T> {
    val list = mutableListOf<T>()
    for (i in 0 until length()) {
        list.add(get(i) as T)
    }
    return list
}
