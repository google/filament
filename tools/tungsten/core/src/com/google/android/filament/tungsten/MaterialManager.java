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

package com.google.android.filament.tungsten;

import com.google.android.filament.Engine;
import com.google.android.filament.Material;

import java.nio.Buffer;
import java.util.concurrent.CompletableFuture;

public class MaterialManager {

    private final MaterialCompiler mCompiler;

    public MaterialManager(String matcPath) {
        mCompiler = new MaterialCompiler(matcPath);
    }

    public CompletableFuture<Material> compileMaterial(String source) {
        CompletableFuture<Material> result = new CompletableFuture<>();
        CompletableFuture.runAsync(() -> {
            try {
                Buffer materialBlob = mCompiler.compile(source);
                // The Material load must happen on the Filament thread
                Filament.getInstance().runOnFilamentThread((Engine e) -> {
                    Material m = uploadMaterialBlob(materialBlob, e);
                    result.complete(m);
                });
            } catch (MaterialCompilationException e) {
                e.printStackTrace();
                result.cancel(true);
            }
        });
        return result;
    }

    private static Material uploadMaterialBlob(Buffer materialBlob, Engine engine) {
        Material.Builder materialBuilder = new Material.Builder()
                .payload(materialBlob, materialBlob.capacity());
        return materialBuilder.build(engine);
    }
}
