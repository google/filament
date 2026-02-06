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
import org.gradle.api.GradleException
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
import org.gradle.api.tasks.TaskAction
import org.gradle.process.ExecOperations
import org.gradle.work.ChangeType
import org.gradle.work.Incremental
import org.gradle.work.InputChanges
import org.gradle.api.tasks.incremental.InputFileDetails
import javax.inject.Inject

abstract class IblGenerateTask extends DefaultTask {

    @Input
    @Optional
    abstract Property<String> getCmgenArgs()

    @Incremental
    @InputFile
    abstract RegularFileProperty getInputFile()

    @OutputDirectory
    abstract DirectoryProperty getOutputDir()

    @InputFiles
    abstract ConfigurableFileCollection getCmgenTool()

    @Inject
    abstract FileSystemOperations getFileSystemOperations()

    @Inject
    abstract ExecOperations getExecOperations()

    @Inject
    abstract ObjectFactory getObjectFactory()

    @TaskAction
    void execute(InputChanges inputs) {
        if (cmgenTool.empty) {
            throw new IllegalStateException(
                "cmgen executable not configured. Please configure the 'cmgen' block in the " +
                "'filament' extension or set the 'com.google.android.filament.tools-dir' " +
                "property."
            )
        }

        File cmgen = getCmgenTool().singleFile
        if (!cmgen.exists()) {
            throw new IllegalStateException("cmgen executable does not exist: ${cmgen.absolutePath}")
        }

        if (!cmgen.canExecute()) {
            cmgen.setExecutable(true)
        }

        if (!inputs.incremental) {
            getFileSystemOperations().delete {
                delete(getObjectFactory().fileTree().from(getOutputDir()).matching { include '*' })
            }
        }

        inputs.getFileChanges(getInputFile()).each { InputFileDetails change ->
            if (change.fileType == FileType.DIRECTORY) return

            def file = change.file

            if (change.changeType == ChangeType.REMOVED) {
                computeOutputFile(file).delete()
            } else {
                println "Generating IBL: ${file.name}"

                def outputPath = getOutputDir().get().asFile
                def commandArgs = getCmgenArgs().getOrNull()
                if (commandArgs == null) {
                    // Default args if not provided
                    commandArgs = '-q -x ' + outputPath + ' --format=rgb32f ' +
                                  '--extract-blur=0.08 --extract=' + outputPath.absolutePath
                }

                def argsList = commandArgs.split(' ').toList()
                argsList.add(file.absolutePath)

                getExecOperations().exec { spec ->
                    spec.executable(cmgen)
                    spec.args(argsList)
                }
            }
        }
    }

    File computeOutputFile(final File file) {
        String name = file.name
        int dotIndex = name.lastIndexOf('.')
        String baseName = dotIndex > 0 ? name.substring(0, dotIndex) : name
        return getOutputDir().file(baseName).get().asFile
    }
}
