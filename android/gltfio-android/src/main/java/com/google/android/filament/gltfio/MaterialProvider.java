/*
 * Copyright (C) 2021 The Android Open Source Project
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

package com.google.android.filament.gltfio;

import com.google.android.filament.MaterialInstance;
import com.google.android.filament.Material;
import com.google.android.filament.VertexBuffer;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.Size;

import com.google.android.filament.proguard.UsedByNative;

@UsedByNative("AssetLoader.cpp")
public interface MaterialProvider {
    /**
     * MaterialKey specifies the requirements for a requested glTF material.
     * The provider creates Filament materials that fulfill these requirements.
     */
    @UsedByNative("MaterialKey.cpp")
    public static class MaterialKey {
        @UsedByNative("MaterialKey.cpp")
        public boolean doubleSided;
        @UsedByNative("MaterialKey.cpp")
        public boolean unlit;
        @UsedByNative("MaterialKey.cpp")
        public boolean hasVertexColors;
        @UsedByNative("MaterialKey.cpp")
        public boolean hasBaseColorTexture;
        @UsedByNative("MaterialKey.cpp")
        public boolean hasNormalTexture;
        @UsedByNative("MaterialKey.cpp")
        public boolean hasOcclusionTexture;
        @UsedByNative("MaterialKey.cpp")
        public boolean hasEmissiveTexture;
        @UsedByNative("MaterialKey.cpp")
        public boolean useSpecularGlossiness;
        @UsedByNative("MaterialKey.cpp")
        public int alphaMode;                       // 0 = OPAQUE, 1 = MASK, 2 = BLEND
        @UsedByNative("MaterialKey.cpp")
        public boolean enableDiagnostics;
        @UsedByNative("MaterialKey.cpp")
        public boolean hasMetallicRoughnessTexture; // piggybacks with specularRoughness
        @UsedByNative("MaterialKey.cpp")
        public int metallicRoughnessUV;             // piggybacks with specularRoughness
        @UsedByNative("MaterialKey.cpp")
        public int baseColorUV;
        @UsedByNative("MaterialKey.cpp")
        public boolean hasClearCoatTexture;
        @UsedByNative("MaterialKey.cpp")
        public int clearCoatUV;
        @UsedByNative("MaterialKey.cpp")
        public boolean hasClearCoatRoughnessTexture;
        @UsedByNative("MaterialKey.cpp")
        public int clearCoatRoughnessUV;
        @UsedByNative("MaterialKey.cpp")
        public boolean hasClearCoatNormalTexture;
        @UsedByNative("MaterialKey.cpp")
        public int clearCoatNormalUV;
        @UsedByNative("MaterialKey.cpp")
        public boolean hasClearCoat;
        @UsedByNative("MaterialKey.cpp")
        public boolean hasTransmission;
        @UsedByNative("MaterialKey.cpp")
        public boolean hasTextureTransforms;
        @UsedByNative("MaterialKey.cpp")
        public int emissiveUV;
        @UsedByNative("MaterialKey.cpp")
        public int aoUV;
        @UsedByNative("MaterialKey.cpp")
        public int normalUV;
        @UsedByNative("MaterialKey.cpp")
        public boolean hasTransmissionTexture;
        @UsedByNative("MaterialKey.cpp")
        public int transmissionUV;
        @UsedByNative("MaterialKey.cpp")
        public boolean hasSheenColorTexture;
        @UsedByNative("MaterialKey.cpp")
        public int sheenColorUV;
        @UsedByNative("MaterialKey.cpp")
        public boolean hasSheenRoughnessTexture;
        @UsedByNative("MaterialKey.cpp")
        public int sheenRoughnessUV;
        @UsedByNative("MaterialKey.cpp")
        public boolean hasVolumeThicknessTexture;
        @UsedByNative("MaterialKey.cpp")
        public int volumeThicknessUV;
        @UsedByNative("MaterialKey.cpp")
        public boolean hasSheen;
        @UsedByNative("MaterialKey.cpp")
        public boolean hasIOR;

        public MaterialKey() {}
        static {
            nGlobalInit();
        }

        /**
         * Populate UV map according to the material key, altering latter if required.
         *
         * Filament supports up to 2 UV sets. glTF has arbitrary texcoord set indices, but it
         * allows implementations to support only 2 simultaneous sets. Here we build a mapping
         * table with 1-based indices where 0 means unused. Note that the order in which we drop
         * textures can affect the look of certain assets. This "order of degradation" is
         * stipulated by the glTF 2.0 specification.
         *
         * @param uvmap Output argument that gets populated with a small table that maps from a
         *              glTF uv index to a Filament uv index (0 = UNUSED, 1 = UV0, 2 = UV1).
         */
        public void constrainMaterial(@NonNull @Size(min = 8) int[] uvmap) {
            nConstrainMaterial(this, uvmap);
        }

        private static native void nGlobalInit();
        private static native void nConstrainMaterial(MaterialKey materialKey, int[] uvmap);
    };

    /**
     * Creates or fetches a compiled Filament material, then creates an instance from it.
     *
     * @param config Specifies requirements; might be mutated due to resource constraints.
     * @param uvmap Output argument that gets populated with a small table that maps from a glTF uv
     *              index to a Filament uv index (0 = UNUSED, 1 = UV0, 2 = UV1).
     * @param label Optional tag that is not a part of the cache key.
     * @param extras Optional extras as stringified JSON (not a part of the cache key).
     */
    public @Nullable MaterialInstance createMaterialInstance(MaterialKey config,
            @NonNull @Size(min = 8) int[] uvmap, @Nullable String label, @Nullable String extras);

    /**
     * Creates or fetches a compiled Filament material corresponding to the given config.
     */
    public @Nullable Material getMaterial(MaterialKey config, @NonNull @Size(min = 8) int[] uvmap,
            @Nullable String label);

    /**
     * Creates and returns an array containing all cached materials.
     */
    public @NonNull Material[] getMaterials();

    /**
     * Returns true if the presence of the given vertex attribute is required.
     *
     * Some types of providers (e.g. ubershader) require dummy attribute values
     * if the glTF model does not provide them.
     *
     * NOTE: The given attribute is the VertexAttribute enum casted to an integer.
     * This is done to streamline the JNI work between Java and Native layers.
     */
    public boolean needsDummyData(int attrib);

    /**
     * Destroys all cached materials.
     *
     * This is not called automatically when MaterialProvider is destroyed, which allows
     * clients to take ownership of the cache if desired.
     */
    public void destroyMaterials();

    /**
     * Frees memory consumed by the native <code>MaterialProvider</code>, but does not destroy
     * cached materials.
     */
    public void destroy();
}
