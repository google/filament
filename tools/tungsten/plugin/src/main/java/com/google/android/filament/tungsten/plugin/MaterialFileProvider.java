/*
 * Copyright (C) 2018 The Android Open Source Project
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

package com.google.android.filament.tungsten.plugin;

import com.google.android.filament.tungsten.Filament;
import com.google.android.filament.tungsten.MaterialManager;
import com.google.android.filament.tungsten.util.OperatingSystem;
import com.intellij.openapi.components.ApplicationComponent;
import com.intellij.openapi.fileEditor.FileEditor;
import com.intellij.openapi.fileEditor.FileEditorPolicy;
import com.intellij.openapi.fileEditor.FileEditorProvider;
import com.intellij.openapi.project.Project;
import com.intellij.openapi.vfs.VirtualFile;
import java.io.File;
import java.io.IOException;
import org.jetbrains.annotations.NotNull;

public class MaterialFileProvider implements ApplicationComponent, FileEditorProvider {

    private static final String COMPONENT_NAME = "MaterialFileProvider";
    private static final String EDITOR_TYPE_ID = "FilamentMaterial";

    private String mMatcPath;
    private MaterialManager mMaterialManager;

    @Override
    public void initComponent() {
        try {
            prepareFilament();
            Filament.getInstance().start();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void disposeComponent() {
        Filament.getInstance().shutdown();
    }

    @NotNull
    @Override
    public String getComponentName() {
        return COMPONENT_NAME;
    }

    @Override
    public boolean accept(@NotNull Project project, @NotNull VirtualFile file) {
        String materialFileExtension = MaterialFileType.getInstance().getDefaultExtension();
        return materialFileExtension.equalsIgnoreCase(file.getExtension());
    }

    @NotNull
    @Override
    public FileEditor createEditor(@NotNull Project project, @NotNull VirtualFile file) {
        return new MaterialFileEditor(mMaterialManager, file);
    }

    @NotNull
    @Override
    public String getEditorTypeId() {
        return EDITOR_TYPE_ID;
    }

    @NotNull
    @Override
    public FileEditorPolicy getPolicy() {
        return FileEditorPolicy.HIDE_DEFAULT_EDITOR;
    }

    private void prepareFilament() throws IOException {
        ResourceLoader.loadDynamicLibrary("filament-jni");
        String matcFileName;
        if (OperatingSystem.isWindows()) {
            matcFileName = "matc.exe";
        } else {
            matcFileName = "matc";
        }
        File tempMatc = ResourceLoader.getResourceAsFile(matcFileName, "", true);
        mMatcPath = tempMatc.getAbsolutePath();
        mMaterialManager = new MaterialManager(mMatcPath);
    }
}
