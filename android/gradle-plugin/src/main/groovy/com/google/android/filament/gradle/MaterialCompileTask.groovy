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

import org.gradle.api.artifacts.component.ModuleComponentIdentifier
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
import org.gradle.api.tasks.SkipWhenEmpty
import org.gradle.api.tasks.TaskAction
import org.gradle.process.ExecOperations
import org.gradle.work.ChangeType
import org.gradle.work.Incremental
import org.gradle.work.InputChanges
import javax.inject.Inject

abstract class MaterialCompileTask extends DefaultTask {

    @Incremental
    @InputDirectory
    @PathSensitive(PathSensitivity.RELATIVE)
    abstract DirectoryProperty getInputDir()

    @OutputDirectory
    abstract DirectoryProperty getOutputDir()

    @InputFiles
    @PathSensitive(PathSensitivity.NONE)
    abstract ConfigurableFileCollection getMatcTool()

    @Inject
    abstract ExecOperations getExecOperations()

    @Inject
    abstract FileSystemOperations getFileSystemOperations()

    @Inject
    abstract ObjectFactory getObjectFactory()

    @Inject
    abstract ProviderFactory getProviderFactory()

    @TaskAction
    void compile(InputChanges inputs) {
        if (matcTool.empty) {
            throw new IllegalStateException(
                "matc executable not configured. Please configure the 'matc' block in the " +
                "'filament' extension or set the 'com.google.android.filament.tools-dir' " +
                "property."
            )
        }

        File matc = matcTool.singleFile
        if (!matc.exists()) {
            throw new IllegalStateException("matc executable does not exist: ${matc.absolutePath}")
        }

        if (!matc.canExecute()) {
            matc.setExecutable(true)
        }

        if (!inputs.incremental) {
            getFileSystemOperations().delete {
                delete(getObjectFactory().fileTree().from(getOutputDir()).matching {
                    include '*.filamat'
                })
            }
        }

        def pf = getProviderFactory()
        def excludeVulkanProperty = pf.gradleProperty("com.google.android.filament.exclude-vulkan")
        def includeWebGpuProperty = pf.gradleProperty("com.google.android.filament.include-webgpu")
        def matNoOptProperty = pf.gradleProperty("com.google.android.filament.matnopt")
        def excludeVulkan = excludeVulkanProperty.orNull == "true"
        def includeWebGpu = includeWebGpuProperty.orNull == "true"
        def matNoOpt = matNoOptProperty.orNull == "true"

        inputs.getFileChanges(getInputDir()).each { change ->
            if (change.fileType == FileType.DIRECTORY) return

            File file = change.file
            File outputFile = computeOutputFile(file)

            if (change.changeType == ChangeType.REMOVED) {
                outputFile.delete()
            } else {
                println "Compiling material: ${file.name}"

                def args = []
                if (!excludeVulkan) {
                    args += ['-a', 'vulkan']
                }

                if (includeWebGpu) {
                    args += ['-a', 'webgpu', '--variant-filter=stereo']
                }

                if (matNoOpt) {
                    args += ['-g']
                }

                args += [
                    '-a', 'opengl', '-p', 'mobile',
                    '-o', outputFile.absolutePath,
                    file.absolutePath
                ]

                getExecOperations().exec { spec ->
                    spec.executable(matc)
                    spec.args(args)
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
        return getOutputDir().file("${baseName}.filamat").get().asFile
    }
}
