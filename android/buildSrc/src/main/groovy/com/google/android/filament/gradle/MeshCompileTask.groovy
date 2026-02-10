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

import org.gradle.api.DefaultTask
import org.gradle.api.file.ConfigurableFileCollection
import org.gradle.api.file.DirectoryProperty
import org.gradle.api.file.FileSystemOperations
import org.gradle.api.file.FileType
import org.gradle.api.file.RegularFileProperty
import org.gradle.api.model.ObjectFactory
import org.gradle.api.tasks.InputFile
import org.gradle.api.tasks.InputFiles
import org.gradle.api.tasks.OutputDirectory
import org.gradle.api.tasks.PathSensitive
import org.gradle.api.tasks.PathSensitivity
import org.gradle.api.tasks.TaskAction
import org.gradle.process.ExecOperations
import org.gradle.work.ChangeType
import org.gradle.work.Incremental
import org.gradle.work.InputChanges
import javax.inject.Inject

abstract class MeshCompileTask extends DefaultTask {

    @Incremental
    @InputFile
    @PathSensitive(PathSensitivity.RELATIVE)
    abstract RegularFileProperty getInputFile()

    @OutputDirectory
    abstract DirectoryProperty getOutputDir()

    @InputFiles
    @PathSensitive(PathSensitivity.NONE)
    abstract ConfigurableFileCollection getFilameshTool()

    @Inject
    abstract ExecOperations getExecOperations()

    @Inject
    abstract FileSystemOperations getFileSystemOperations()

    @Inject
    abstract ObjectFactory getObjectFactory()

    @TaskAction
    void compile(InputChanges inputs) {
        if (filameshTool.empty) {
            throw new IllegalStateException(
                "filamesh executable not configured. Please configure the 'filamesh' block in the " +
                "'filament' extension or set the 'com.google.android.filament.tools-dir' " +
                "property."
            )
        }

        File filamesh = filameshTool.singleFile
        if (!filamesh.exists()) {
            throw new IllegalStateException("filamesh executable does not exist: ${filamesh.absolutePath}")
        }

        if (!filamesh.canExecute()) {
            filamesh.setExecutable(true)
        }

        if (!inputs.incremental) {
            getFileSystemOperations().delete {
                delete(getObjectFactory().fileTree().from(getOutputDir()).matching {
                    include '*.filamesh'
                })
            }
        }

        inputs.getFileChanges(inputFile).each { change ->
            if (change.fileType == FileType.DIRECTORY) return

            File file = change.file
            File outputFile = computeOutputFile(file)

            if (change.changeType == ChangeType.REMOVED) {
                outputFile.delete()
            } else {
                println "Compiling mesh: ${file.name}"

                getExecOperations().exec { spec ->
                    spec.executable(filamesh)
                    spec.args(file.absolutePath, outputFile.absolutePath)
                }
            }
        }
    }

    File computeOutputFile(File inputFile) {
        String baseName = inputFile.name
        int dotIndex = baseName.lastIndexOf('.')
        if (dotIndex > 0) {
            baseName = baseName.substring(0, dotIndex)
        }
        return getOutputDir().file("${baseName}.filamesh").get().asFile
    }
}
