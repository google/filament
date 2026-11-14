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
import org.gradle.api.provider.Property
import org.gradle.api.tasks.Input
import org.gradle.api.tasks.InputFile
import org.gradle.api.tasks.InputFiles
import org.gradle.api.tasks.Optional
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
abstract class IblGenerateTask : DefaultTask() {

    @get:Input
    @get:Optional
    abstract val cmgenArgs: Property<String>

    @get:Incremental
    @get:InputFile
    @get:PathSensitive(PathSensitivity.RELATIVE)
    abstract val inputFile: RegularFileProperty

    @get:OutputDirectory
    abstract val outputDir: DirectoryProperty

    @get:InputFiles
    @get:PathSensitive(PathSensitivity.NONE)
    abstract val cmgenTool: ConfigurableFileCollection

    @get:Inject
    abstract val fileSystemOperations: FileSystemOperations

    @get:Inject
    abstract val execOperations: ExecOperations

    @get:Inject
    abstract val objectFactory: ObjectFactory

    @TaskAction
    fun execute(inputs: InputChanges) {
        if (cmgenTool.isEmpty) {
            throw IllegalStateException(
                "cmgen executable not configured. Please configure the 'cmgen' block in the " +
                    "'filament' extension or set the 'com.google.android.filament.tools-dir' " +
                    "property."
            )
        }

        val cmgen = cmgenTool.singleFile
        if (!cmgen.exists()) {
            throw IllegalStateException("cmgen executable does not exist: ${cmgen.absolutePath}")
        }

        if (!cmgen.canExecute()) {
            cmgen.setExecutable(true)
        }

        if (!inputs.isIncremental) {
            fileSystemOperations.delete { spec ->
                spec.delete(objectFactory.fileTree().from(outputDir).matching { pattern ->
                    pattern.include("*")
                })
            }
        }

        inputs.getFileChanges(inputFile).forEach { change ->
            if (change.fileType == FileType.DIRECTORY) return@forEach

            val file = change.file

            if (change.changeType == ChangeType.REMOVED) {
                computeOutputFile(file).delete()
            } else {
                println("Generating IBL: ${file.name}")

                val outputPath = outputDir.get().asFile
                val commandArgs = cmgenArgs.orNull
                    ?: "-q -x $outputPath --format=rgb32f --extract-blur=0.08 --extract=${outputPath.absolutePath}"

                val argsList = commandArgs.split(" ").toMutableList()
                argsList.add(file.absolutePath)

                execOperations.exec { spec ->
                    spec.executable(cmgen)
                    spec.args(argsList)
                }
            }
        }
    }

    fun computeOutputFile(file: File): File {
        val name = file.name
        val dotIndex = name.lastIndexOf('.')
        val baseName = if (dotIndex > 0) name.substring(0, dotIndex) else name
        return outputDir.file(baseName).get().asFile
    }
}
