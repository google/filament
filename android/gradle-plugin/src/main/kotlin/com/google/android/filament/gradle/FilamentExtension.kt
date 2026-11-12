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

import javax.inject.Inject
import org.gradle.api.Action
import org.gradle.api.Project
import org.gradle.api.file.DirectoryProperty
import org.gradle.api.file.RegularFileProperty
import org.gradle.api.provider.Property

open class FilamentExtension @Inject constructor(project: Project) {
    val tools: ToolsLocator = ToolsLocator(project)
    open val materialInputDir: DirectoryProperty = project.objects.directoryProperty()
    open val materialOutputDir: DirectoryProperty = project.objects.directoryProperty()
    open val cmgenArgs: Property<String> = project.objects.property(String::class.java)
    open val iblInputFile: RegularFileProperty = project.objects.fileProperty()
    open val iblOutputDir: DirectoryProperty = project.objects.directoryProperty()
    open val meshInputFile: RegularFileProperty = project.objects.fileProperty()
    open val meshOutputDir: DirectoryProperty = project.objects.directoryProperty()

    fun matc(action: Action<in ToolsLocator.ToolConfig>) {
        action.execute(tools.matc)
    }

    fun cmgen(action: Action<in ToolsLocator.ToolConfig>) {
        action.execute(tools.cmgen)
    }

    fun filamesh(action: Action<in ToolsLocator.ToolConfig>) {
        action.execute(tools.filamesh)
    }
}
