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

import com.android.build.api.dsl.CommonExtension
import org.gradle.api.Plugin
import org.gradle.api.Project

class FilamentPlugin : Plugin<Project> {
    override fun apply(project: Project) {
        project.pluginManager.apply("com.google.osdetector")

        val extension = project.extensions.create("filament", FilamentExtension::class.java, project)

        project.plugins.withId("com.android.base") {
            project.extensions.configure(CommonExtension::class.java) { android ->
                android.sourceSets.getByName("main").assets.directories.add(
                    project.layout.buildDirectory.dir("generated/assets/filament").get().asFile.path
                )
            }
        }

        val compileMaterials = project.tasks.register("filamentCompileMaterials", MaterialCompileTask::class.java) { task ->
            task.onlyIf { task.inputDir.isPresent && task.outputDir.isPresent }
            task.inputDir.set(extension.materialInputDir)
            task.outputDir.set(extension.materialOutputDir)
            task.matcTool.from(project.provider {
                if (extension.materialInputDir.isPresent && extension.materialOutputDir.isPresent) {
                    extension.tools.matcToolFiles
                } else {
                    project.files()
                }
            })
            task.excludeVulkan.set(
                project.providers.gradleProperty("com.google.android.filament.exclude-vulkan")
                    .map { it != "false" }
                    .orElse(false)
            )
            task.matNoOpt.set(
                project.providers.gradleProperty("com.google.android.filament.matnopt")
                    .map { it != "false" }
                    .orElse(false)
            )
        }

        val generateIbl = project.tasks.register("filamentGenerateIbl", IblGenerateTask::class.java) { task ->
            task.onlyIf { task.inputFile.isPresent && task.outputDir.isPresent }
            task.cmgenArgs.set(extension.cmgenArgs)
            task.inputFile.set(extension.iblInputFile)
            task.outputDir.set(extension.iblOutputDir)
            task.cmgenTool.from(project.provider {
                if (extension.iblInputFile.isPresent && extension.iblOutputDir.isPresent) {
                    extension.tools.cmgenToolFiles
                } else {
                    project.files()
                }
            })
        }

        val compileMesh = project.tasks.register("filamentCompileMesh", MeshCompileTask::class.java) { task ->
            task.onlyIf { task.inputFile.isPresent && task.outputDir.isPresent }
            task.inputFile.set(extension.meshInputFile)
            task.outputDir.set(extension.meshOutputDir)
            task.filameshTool.from(project.provider {
                if (extension.meshInputFile.isPresent && extension.meshOutputDir.isPresent) {
                    extension.tools.filameshToolFiles
                } else {
                    project.files()
                }
            })
        }

        val generateAssets = project.tasks.register("filamentGenerateAssets") { task ->
            task.dependsOn(compileMaterials, generateIbl, compileMesh)
        }

        project.tasks.configureEach { task ->
            val name = task.name
            if ((name.startsWith("generate") && name.endsWith("Assets")) ||
                (name.startsWith("generate") && name.endsWith("Model") && name.contains("Lint")) ||
                name.startsWith("lintAnalyze") ||
                name.startsWith("lintVitalAnalyze")
            ) {
                task.dependsOn(generateAssets)
            }
        }
    }
}
