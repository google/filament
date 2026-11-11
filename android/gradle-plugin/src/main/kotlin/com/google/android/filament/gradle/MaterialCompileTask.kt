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

import java.io.File
import javax.inject.Inject
import org.gradle.api.DefaultTask
import org.gradle.api.file.ConfigurableFileCollection
import org.gradle.api.file.DirectoryProperty
import org.gradle.api.file.FileSystemOperations
import org.gradle.api.file.FileType
import org.gradle.api.model.ObjectFactory
import org.gradle.api.provider.ProviderFactory
import org.gradle.api.tasks.InputDirectory
import org.gradle.api.tasks.InputFiles
import org.gradle.api.tasks.OutputDirectory
import org.gradle.api.tasks.PathSensitive
import org.gradle.api.tasks.PathSensitivity
import org.gradle.api.tasks.TaskAction
import org.gradle.process.ExecOperations
import org.gradle.work.ChangeType
import org.gradle.work.DisableCachingByDefault
import org.gradle.work.Incremental
import org.gradle.work.InputChanges

@DisableCachingByDefault
abstract class MaterialCompileTask : DefaultTask() {

    @get:Incremental
    @get:InputDirectory
    @get:PathSensitive(PathSensitivity.RELATIVE)
    abstract val inputDir: DirectoryProperty

    @get:OutputDirectory
    abstract val outputDir: DirectoryProperty

    @get:InputFiles
    @get:PathSensitive(PathSensitivity.NONE)
    abstract val matcTool: ConfigurableFileCollection

    @get:Inject
    abstract val execOperations: ExecOperations

    @get:Inject
    abstract val fileSystemOperations: FileSystemOperations

    @get:Inject
    abstract val objectFactory: ObjectFactory

    @get:Inject
    abstract val providerFactory: ProviderFactory

    @TaskAction
    fun compile(inputs: InputChanges) {
        if (matcTool.isEmpty) {
            throw IllegalStateException(
                "matc executable not configured. Please configure the 'matc' block in the " +
                    "'filament' extension or set the 'com.google.android.filament.tools-dir' " +
                    "property."
            )
        }

        val matc = matcTool.singleFile
        if (!matc.exists()) {
            throw IllegalStateException("matc executable does not exist: ${matc.absolutePath}")
        }

        if (!matc.canExecute()) {
            matc.setExecutable(true)
        }

        if (!inputs.isIncremental) {
            fileSystemOperations.delete { spec ->
                spec.delete(objectFactory.fileTree().from(outputDir).matching { pattern ->
                    pattern.include("*.filamat")
                })
            }
        }

        val pf = providerFactory
        val excludeVulkanProperty = pf.gradleProperty("com.google.android.filament.exclude-vulkan")
        val matNoOptProperty = pf.gradleProperty("com.google.android.filament.matnopt")
        val excludeVulkan = excludeVulkanProperty.orNull == "true"
        val matNoOpt = matNoOptProperty.orNull == "true"

        inputs.getFileChanges(inputDir).forEach { change ->
            if (change.fileType == FileType.DIRECTORY) return@forEach

            val file = change.file
            val outputFile = computeOutputFile(file)

            if (change.changeType == ChangeType.REMOVED) {
                outputFile.delete()
            } else {
                println("Compiling material: ${file.name}")

                val args = mutableListOf<String>()
                if (!excludeVulkan) {
                    args += listOf("-a", "vulkan")
                }

                if (matNoOpt) {
                    args += listOf("-g")
                }

                args += listOf(
                    "-a", "opengl", "-p", "mobile",
                    "-o", outputFile.absolutePath,
                    file.absolutePath
                )

                execOperations.exec { spec ->
                    spec.executable(matc)
                    spec.args(args)
                }
            }
        }
    }

    fun computeOutputFile(inputFile: File): File {
        var baseName = inputFile.name
        val dotIndex = baseName.lastIndexOf('.')
        if (dotIndex > 0) {
            baseName = baseName.substring(0, dotIndex)
        }
        return outputDir.file("$baseName.filamat").get().asFile
    }
}
