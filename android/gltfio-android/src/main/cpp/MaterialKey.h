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

#include <jni.h>

#include <gltfio/MaterialProvider.h>

#define JAVA_MATERIAL_KEY "com/google/android/filament/gltfio/MaterialProvider$MaterialKey"

class MaterialKeyHelper {
public:
    using MaterialKey = filament::gltfio::MaterialKey;

    static MaterialKeyHelper& get();

    void copy(JNIEnv* env, MaterialKey& dst, jobject src);
    void copy(JNIEnv* env, jobject dst, const MaterialKey& src);

    void init(JNIEnv* env); // called only from the Java static class constructor

private:
    jfieldID doubleSided;
    jfieldID unlit;
    jfieldID hasVertexColors;
    jfieldID hasBaseColorTexture;
    jfieldID hasNormalTexture;
    jfieldID hasOcclusionTexture;
    jfieldID hasEmissiveTexture;
    jfieldID useSpecularGlossiness;
    jfieldID alphaMode;
    jfieldID enableDiagnostics;
    jfieldID hasMetallicRoughnessTexture;
    jfieldID metallicRoughnessUV;
    jfieldID baseColorUV;
    jfieldID hasClearCoatTexture;
    jfieldID clearCoatUV;
    jfieldID hasClearCoatRoughnessTexture;
    jfieldID clearCoatRoughnessUV;
    jfieldID hasClearCoatNormalTexture;
    jfieldID clearCoatNormalUV;
    jfieldID hasClearCoat;
    jfieldID hasTransmission;
    jfieldID hasTextureTransforms;
    jfieldID emissiveUV;
    jfieldID aoUV;
    jfieldID normalUV;
    jfieldID hasTransmissionTexture;
    jfieldID transmissionUV;
    jfieldID hasSheenColorTexture;
    jfieldID sheenColorUV;
    jfieldID hasSheenRoughnessTexture;
    jfieldID sheenRoughnessUV;
    jfieldID hasVolumeThicknessTexture;
    jfieldID volumeThicknessUV;
    jfieldID hasSheen;
    jfieldID hasIOR;
};
