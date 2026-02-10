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

import org.gradle.api.Action
import org.gradle.api.Project
import org.gradle.api.file.DirectoryProperty
import org.gradle.api.file.RegularFileProperty
import org.gradle.api.provider.Property

class FilamentExtension {
    final ToolsLocator tools
    final DirectoryProperty materialInputDir
    final DirectoryProperty materialOutputDir
    final Property<String> cmgenArgs
    final RegularFileProperty iblInputFile
    final DirectoryProperty iblOutputDir
    final RegularFileProperty meshInputFile
    final DirectoryProperty meshOutputDir

    FilamentExtension(Project project) {
        this.tools = new ToolsLocator(project)
        this.materialInputDir = project.objects.directoryProperty()
        this.materialOutputDir = project.objects.directoryProperty()
        this.cmgenArgs = project.objects.property(String)
        this.iblInputFile = project.objects.fileProperty()
        this.iblOutputDir = project.objects.directoryProperty()
        this.meshInputFile = project.objects.fileProperty()
        this.meshOutputDir = project.objects.directoryProperty()
    }

    void matc(Action<ToolsLocator.ToolConfig> action) {
        action.execute(tools.matc)
    }

    void cmgen(Action<ToolsLocator.ToolConfig> action) {
        action.execute(tools.cmgen)
    }

    void filamesh(Action<ToolsLocator.ToolConfig> action) {
        action.execute(tools.filamesh)
    }
}
