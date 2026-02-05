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

import org.gradle.api.Action
import org.gradle.api.Project
import org.gradle.api.artifacts.Configuration
import org.gradle.api.file.FileCollection
import org.gradle.internal.os.OperatingSystem

import java.nio.file.Paths

class ToolsLocator {
    static class ToolConfig {
        String artifact
        String path
        FileCollection files
    }

    final ToolConfig matc = new ToolConfig()
    final ToolConfig cmgen = new ToolConfig()
    final ToolConfig filamesh = new ToolConfig()
    private final Project project

    ToolsLocator(Project project) {
        this.project = project
    }

    void resolve(Project project) {
        resolveTool(matc, "matc")
        resolveTool(cmgen, "cmgen")
        resolveTool(filamesh, "filamesh")
    }

    /**
     * Resolves a specific tool by its name and sets the {@link ToolConfig#files} property of the
     * provided {@link ToolConfig} object. It first attempts to locate the tool based on a Gradle
     * property `com.google.android.filament.tools-dir` if present, otherwise it resolves the tool
     * through a Gradle configuration.
     *
     * @param tool The {@link ToolConfig} object whose {@code files} property will be set.
     * @param name The name of the tool (e.g., "matc", "cmgen").
     */
    private void resolveTool(ToolConfig tool, String name) {
        // Find the OS classifier, e.g. 'osx-aarch_64'.
        def classifier =
                project.extensions.getByType(com.google.gradle.osdetector.OsDetector).classifier

        // If com.google.android.filament.tools-dir is set, we'll use it as the tool's base path.
        def toolsDirProp = project.providers.gradleProperty("com.google.android.filament.tools-dir")
        if (toolsDirProp.isPresent()) {
            def toolsDir = toolsDirProp.get()
            def path = OperatingSystem.current().isWindows() ?
                "${toolsDir}/bin/${name}.exe" :
                "${toolsDir}/bin/${name}"
            tool.files = project.files(path)
            return
        }

        // If an explicit path for the tool is provided in ToolConfig
        // (e.g. matc { path = 'path/to/tool' }), use it directly.
        if (tool.path) {
            tool.files = project.files(tool.path)
            return
        }

        // Otherwise, if an artifact is provided
        // (e.g. matc { artifact = 'com.google.android.filament:matc:1.68.5' }), resolve it.
        if (tool.artifact) {
            String depString = tool.artifact

            // In Gradle, a configuration is a named, manageable group of dependencies.
            // Resolve the tool artifact using a detached configuration. A detached configuration
            // is a temporary, isolated configuration that is not part of the project's regular
            // configuration hierarchy.
            Configuration config = project.configurations.detachedConfiguration()
            config.setTransitive(false) // We only want the tool itself, not its dependencies

            def dep = project.dependencies.create("${depString}:${classifier}@exe")
            config.dependencies.add(dep)

            // A Gradle Configuration implements FileCollection. When treated as a FileCollection,
            // it represents the resolved files of its dependencies.
            tool.files = config
        }
    }

    FileCollection getMatcToolFiles() {
        return matc.files ?: project.files()
    }

    FileCollection getCmgenToolFiles() {
        return cmgen.files ?: project.files()
    }

    FileCollection getFilameshToolFiles() {
        return filamesh.files ?: project.files()
    }
}
