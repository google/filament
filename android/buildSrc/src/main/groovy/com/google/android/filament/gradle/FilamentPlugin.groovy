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

import org.gradle.api.Plugin
import org.gradle.api.Project

class FilamentPlugin implements Plugin<Project> {
    @Override
    void apply(Project project) {
        project.pluginManager.apply("com.google.osdetector")

        FilamentExtension extension = project.extensions.create("filament", FilamentExtension, project)

        project.afterEvaluate {
            extension.tools.resolve(project)

            project.tasks.register("filamentCompileMaterials", MaterialCompileTask) {
                enabled = extension.materialInputDir.isPresent() && extension.materialOutputDir.isPresent()
                inputDir.set(extension.materialInputDir.getOrNull())
                outputDir.set(extension.materialOutputDir.getOrNull())
                matcTool.from(extension.tools.matcToolFiles)
            }

            project.tasks.register("filamentGenerateIbl", IblGenerateTask) {
                enabled = extension.iblInputFile.isPresent() && extension.iblOutputDir.isPresent()
                cmgenArgs = extension.cmgenArgs
                inputFile.set(extension.iblInputFile.getOrNull())
                outputDir.set(extension.iblOutputDir.getOrNull())
                cmgenTool.from(extension.tools.cmgenToolFiles)
            }

            project.tasks.register("filamentCompileMesh", MeshCompileTask) {
                enabled = extension.meshInputFile.isPresent() && extension.meshOutputDir.isPresent()
                inputFile = extension.meshInputFile.getOrNull()
                outputDir = extension.meshOutputDir.getOrNull()
                filameshTool.from(extension.tools.filameshToolFiles)
            }

            project.preBuild.dependsOn "filamentCompileMaterials"
            project.preBuild.dependsOn "filamentGenerateIbl"
            project.preBuild.dependsOn "filamentCompileMesh"
        }
    }
}
