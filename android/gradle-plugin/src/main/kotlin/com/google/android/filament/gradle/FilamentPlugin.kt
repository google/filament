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

class FilamentPlugin : Plugin<Project> {
    override fun apply(project: Project) {
        project.pluginManager.apply("com.google.osdetector")

        val extension = project.extensions.create("filament", FilamentExtension::class.java, project)

        project.afterEvaluate {
            extension.tools.resolve(project)

            project.tasks.register("filamentCompileMaterials", MaterialCompileTask::class.java) { task ->
                task.enabled = extension.materialInputDir.isPresent && extension.materialOutputDir.isPresent
                task.inputDir.set(extension.materialInputDir.orNull)
                task.outputDir.set(extension.materialOutputDir.orNull)
                task.matcTool.from(extension.tools.matcToolFiles)
            }

            project.tasks.register("filamentGenerateIbl", IblGenerateTask::class.java) { task ->
                task.enabled = extension.iblInputFile.isPresent && extension.iblOutputDir.isPresent
                task.cmgenArgs.set(extension.cmgenArgs)
                task.inputFile.set(extension.iblInputFile.orNull)
                task.outputDir.set(extension.iblOutputDir.orNull)
                task.cmgenTool.from(extension.tools.cmgenToolFiles)
            }

            project.tasks.register("filamentCompileMesh", MeshCompileTask::class.java) { task ->
                task.enabled = extension.meshInputFile.isPresent && extension.meshOutputDir.isPresent
                task.inputFile.set(extension.meshInputFile.orNull)
                task.outputDir.set(extension.meshOutputDir.orNull)
                task.filameshTool.from(extension.tools.filameshToolFiles)
            }

            project.tasks.named("preBuild").configure { task ->
                task.dependsOn(
                    "filamentCompileMaterials",
                    "filamentGenerateIbl",
                    "filamentCompileMesh"
                )
            }
        }
    }
}
