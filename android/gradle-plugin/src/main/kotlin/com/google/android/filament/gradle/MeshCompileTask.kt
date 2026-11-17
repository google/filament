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
import org.gradle.api.file.RegularFileProperty
import org.gradle.api.model.ObjectFactory
import org.gradle.api.tasks.CacheableTask
import org.gradle.api.tasks.InputFile
import org.gradle.api.tasks.InputFiles
import org.gradle.api.tasks.Optional
import org.gradle.api.tasks.OutputDirectory
import org.gradle.api.tasks.PathSensitive
import org.gradle.api.tasks.PathSensitivity
import org.gradle.api.tasks.TaskAction
import org.gradle.process.ExecOperations
import org.gradle.work.ChangeType
import org.gradle.work.Incremental
import org.gradle.work.InputChanges

@CacheableTask
abstract class MeshCompileTask : DefaultTask() {

    @get:Optional
    @get:Incremental
    @get:InputFile
    @get:PathSensitive(PathSensitivity.RELATIVE)
    abstract val inputFile: RegularFileProperty

    @get:Optional
    @get:OutputDirectory
    abstract val outputDir: DirectoryProperty

    @get:InputFiles
    @get:PathSensitive(PathSensitivity.NONE)
    abstract val filameshTool: ConfigurableFileCollection

    @get:Inject
    abstract val execOperations: ExecOperations

    @get:Inject
    abstract val fileSystemOperations: FileSystemOperations

    @get:Inject
    abstract val objectFactory: ObjectFactory

    @TaskAction
    fun compile(inputs: InputChanges) {
        if (filameshTool.isEmpty) {
            throw IllegalStateException(
                "filamesh executable not configured. Please configure the 'filamesh' block in the " +
                    "'filament' extension or set the 'com.google.android.filament.tools-dir' " +
                    "property."
            )
        }

        val filamesh = filameshTool.singleFile
        if (!filamesh.exists()) {
            throw IllegalStateException("filamesh executable does not exist: ${filamesh.absolutePath}")
        }

        if (!filamesh.canExecute()) {
            filamesh.setExecutable(true)
        }

        if (!inputs.isIncremental) {
            fileSystemOperations.delete { spec ->
                spec.delete(objectFactory.fileTree().from(outputDir).matching { pattern ->
                    pattern.include("*.filamesh")
                })
            }
        }

        inputs.getFileChanges(inputFile).forEach { change ->
            if (change.fileType == FileType.DIRECTORY) return@forEach

            val file = change.file
            val outputFile = computeOutputFile(file)

            if (change.changeType == ChangeType.REMOVED) {
                outputFile.delete()
            } else {
                println("Compiling mesh: ${file.name}")

                execOperations.exec { spec ->
                    spec.executable(filamesh)
                    spec.args(file.absolutePath, outputFile.absolutePath)
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
        return outputDir.file("$baseName.filamesh").get().asFile
    }
}
