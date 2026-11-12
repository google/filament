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

package com.google.android.filament.gradle

import com.google.gradle.osdetector.OsDetector
import org.gradle.api.Project
import org.gradle.api.artifacts.Configuration
import org.gradle.api.file.FileCollection
import org.gradle.internal.os.OperatingSystem

class ToolsLocator(private val project: Project) {
    class ToolConfig {
        var artifact: String? = null
        var path: String? = null
        var files: FileCollection? = null
    }

    val matc: ToolConfig = ToolConfig()
    val cmgen: ToolConfig = ToolConfig()
    val filamesh: ToolConfig = ToolConfig()

    val matcToolFiles: FileCollection
        get() = matc.files ?: project.files()

    val cmgenToolFiles: FileCollection
        get() = cmgen.files ?: project.files()

    val filameshToolFiles: FileCollection
        get() = filamesh.files ?: project.files()

    fun resolve(project: Project) {
        resolveTool(matc, "matc")
        resolveTool(cmgen, "cmgen")
        resolveTool(filamesh, "filamesh")
    }

    /**
     * Resolves a specific tool by its name and sets the [ToolConfig.files] property of the
     * provided [ToolConfig] object. It first attempts to locate the tool based on a Gradle
     * property `com.google.android.filament.tools-dir` if present, otherwise it resolves the tool
     * through a Gradle configuration.
     *
     * @param tool The [ToolConfig] object whose `files` property will be set.
     * @param name The name of the tool (e.g., "matc", "cmgen").
     */
    private fun resolveTool(tool: ToolConfig, name: String) {
        // Find the OS classifier, e.g. 'osx-aarch_64'.
        val classifier = project.extensions.getByType(OsDetector::class.java).classifier

        // If com.google.android.filament.tools-dir is set to a non-empty string, we'll use it as
        // the tool's base path.
        val toolsDirProp = project.providers.gradleProperty("com.google.android.filament.tools-dir")
        if (toolsDirProp.isPresent && toolsDirProp.get().trim().isNotEmpty()) {
            val toolsDir = toolsDirProp.get()
            val path = if (OperatingSystem.current().isWindows) {
                "$toolsDir/bin/$name.exe"
            } else {
                "$toolsDir/bin/$name"
            }
            tool.files = project.files(path)
            return
        }

        // If an explicit path for the tool is provided in ToolConfig
        // (e.g. matc { path = 'path/to/tool' }), use it directly.
        val toolPath = tool.path
        if (!toolPath.isNullOrEmpty()) {
            tool.files = project.files(toolPath)
            return
        }

        // Otherwise, if an artifact is provided
        // (e.g. matc { artifact = 'com.google.android.filament:matc:1.68.5' }), resolve it.
        val depString = tool.artifact
        if (!depString.isNullOrEmpty()) {
            // In Gradle, a configuration is a named, manageable group of dependencies.
            // Resolve the tool artifact using a detached configuration. A detached configuration
            // is a temporary, isolated configuration that is not part of the project's regular
            // configuration hierarchy.
            val config: Configuration = project.configurations.detachedConfiguration()
            config.isTransitive = false // We only want the tool itself, not its dependencies

            val dep = project.dependencies.create("$depString:$classifier@exe")
            config.dependencies.add(dep)

            // A Gradle Configuration implements FileCollection. When treated as a FileCollection,
            // it represents the resolved files of its dependencies.
            tool.files = config
        }
    }
}
