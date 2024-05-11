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

#include "MaterialKey.h"

using namespace filament::gltfio;

MaterialKeyHelper& MaterialKeyHelper::get() {
    static MaterialKeyHelper helper;
    return helper;
}

void MaterialKeyHelper::init(JNIEnv* env) {
    const jclass materialKeyClass = env->FindClass(JAVA_MATERIAL_KEY);
    auto field = [materialKeyClass, env](const char* fieldName, const char* signature) {
        return env->GetFieldID(materialKeyClass, fieldName, signature);
    };
    doubleSided = field("doubleSided", "Z");
    unlit = field("unlit", "Z");
    hasVertexColors = field("hasVertexColors", "Z");
    hasBaseColorTexture = field("hasBaseColorTexture", "Z");
    hasNormalTexture = field("hasNormalTexture", "Z");
    hasOcclusionTexture = field("hasOcclusionTexture", "Z");
    hasEmissiveTexture = field("hasEmissiveTexture", "Z");
    useSpecularGlossiness = field("useSpecularGlossiness", "Z");
    alphaMode = field("alphaMode", "I");
    enableDiagnostics = field("enableDiagnostics", "Z");
    hasMetallicRoughnessTexture = field("hasMetallicRoughnessTexture", "Z");
    metallicRoughnessUV = field("metallicRoughnessUV", "I");
    baseColorUV = field("baseColorUV", "I");
    hasClearCoatTexture = field("hasClearCoatTexture", "Z");
    clearCoatUV = field("clearCoatUV", "I");
    hasClearCoatRoughnessTexture = field("hasClearCoatRoughnessTexture", "Z");
    clearCoatRoughnessUV = field("clearCoatRoughnessUV", "I");
    hasClearCoatNormalTexture = field("hasClearCoatNormalTexture", "Z");
    clearCoatNormalUV = field("clearCoatNormalUV", "I");
    hasClearCoat = field("hasClearCoat", "Z");
    hasTransmission = field("hasTransmission", "Z");
    hasTextureTransforms = field("hasTextureTransforms", "Z");
    emissiveUV = field("emissiveUV", "I");
    aoUV = field("aoUV", "I");
    normalUV = field("normalUV", "I");
    hasTransmissionTexture = field("hasTransmissionTexture", "Z");
    transmissionUV = field("transmissionUV", "I");
    hasSheenColorTexture = field("hasSheenColorTexture", "Z");
    sheenColorUV = field("sheenColorUV", "I");
    hasSheenRoughnessTexture = field("hasSheenRoughnessTexture", "Z");
    sheenRoughnessUV = field("sheenRoughnessUV", "I");
    hasVolumeThicknessTexture = field("hasVolumeThicknessTexture", "Z");
    volumeThicknessUV = field("volumeThicknessUV", "I");
    hasSheen = field("hasSheen", "Z");
    hasIOR = field("hasIOR", "Z");
}

void MaterialKeyHelper::copy(JNIEnv* env, MaterialKey& dst, jobject src) {
    dst.doubleSided = env->GetBooleanField(src, doubleSided);
    dst.unlit = env->GetBooleanField(src, unlit);
    dst.hasVertexColors = env->GetBooleanField(src, hasVertexColors);
    dst.hasBaseColorTexture = env->GetBooleanField(src, hasBaseColorTexture);
    dst.hasNormalTexture = env->GetBooleanField(src, hasNormalTexture);
    dst.hasOcclusionTexture = env->GetBooleanField(src, hasOcclusionTexture);
    dst.hasEmissiveTexture = env->GetBooleanField(src, hasEmissiveTexture);
    dst.useSpecularGlossiness = env->GetBooleanField(src, useSpecularGlossiness);
    dst.alphaMode = (AlphaMode) env->GetIntField(src, alphaMode);
    dst.enableDiagnostics = env->GetBooleanField(src, enableDiagnostics);
    dst.hasMetallicRoughnessTexture = env->GetBooleanField(src, hasMetallicRoughnessTexture);
    dst.metallicRoughnessUV = env->GetIntField(src, metallicRoughnessUV);
    dst.baseColorUV = env->GetIntField(src, baseColorUV);
    dst.hasClearCoatTexture = env->GetBooleanField(src, hasClearCoatTexture);
    dst.clearCoatUV = env->GetIntField(src, clearCoatUV);
    dst.hasClearCoatRoughnessTexture = env->GetBooleanField(src, hasClearCoatRoughnessTexture);
    dst.clearCoatRoughnessUV = env->GetIntField(src, clearCoatRoughnessUV);
    dst.hasClearCoatNormalTexture = env->GetBooleanField(src, hasClearCoatNormalTexture);
    dst.clearCoatNormalUV = env->GetIntField(src, clearCoatNormalUV);
    dst.hasClearCoat = env->GetBooleanField(src, hasClearCoat);
    dst.hasTransmission = env->GetBooleanField(src, hasTransmission);
    dst.hasTextureTransforms = env->GetBooleanField(src, hasTextureTransforms);
    dst.emissiveUV = env->GetIntField(src, emissiveUV);
    dst.aoUV = env->GetIntField(src, aoUV);
    dst.normalUV = env->GetIntField(src, normalUV);
    dst.hasTransmissionTexture = env->GetBooleanField(src, hasTransmissionTexture);
    dst.transmissionUV = env->GetIntField(src, transmissionUV);
    dst.hasSheenColorTexture = env->GetBooleanField(src, hasSheenColorTexture);
    dst.sheenColorUV = env->GetIntField(src, sheenColorUV);
    dst.hasSheenRoughnessTexture = env->GetBooleanField(src, hasSheenRoughnessTexture);
    dst.sheenRoughnessUV = env->GetIntField(src, sheenRoughnessUV);
    dst.hasVolumeThicknessTexture = env->GetBooleanField(src, hasVolumeThicknessTexture);
    dst.volumeThicknessUV = env->GetIntField(src, volumeThicknessUV);
    dst.hasSheen = env->GetBooleanField(src, hasSheen);
    dst.hasIOR = env->GetBooleanField(src, hasIOR);
}

void MaterialKeyHelper::copy(JNIEnv* env, jobject dst, const MaterialKey& src) {
    env->SetBooleanField(dst, doubleSided, src.doubleSided);
    env->SetBooleanField(dst, unlit, src.unlit);
    env->SetBooleanField(dst, hasVertexColors, src.hasVertexColors);
    env->SetBooleanField(dst, hasBaseColorTexture, src.hasBaseColorTexture);
    env->SetBooleanField(dst, hasNormalTexture, src.hasNormalTexture);
    env->SetBooleanField(dst, hasOcclusionTexture, src.hasOcclusionTexture);
    env->SetBooleanField(dst, hasEmissiveTexture, src.hasEmissiveTexture);
    env->SetBooleanField(dst, useSpecularGlossiness, src.useSpecularGlossiness);
    env->SetIntField(dst, alphaMode, (int) src.alphaMode);
    env->SetBooleanField(dst, enableDiagnostics, src.enableDiagnostics);
    env->SetBooleanField(dst, hasMetallicRoughnessTexture, src.hasMetallicRoughnessTexture);
    env->SetIntField(dst, metallicRoughnessUV, src.metallicRoughnessUV);
    env->SetIntField(dst, baseColorUV, src.baseColorUV);
    env->SetBooleanField(dst, hasClearCoatTexture, src.hasClearCoatTexture);
    env->SetIntField(dst, clearCoatUV, src.clearCoatUV);
    env->SetBooleanField(dst, hasClearCoatRoughnessTexture, src.hasClearCoatRoughnessTexture);
    env->SetIntField(dst, clearCoatRoughnessUV, src.clearCoatRoughnessUV);
    env->SetBooleanField(dst, hasClearCoatNormalTexture, src.hasClearCoatNormalTexture);
    env->SetIntField(dst, clearCoatNormalUV, src.clearCoatNormalUV);
    env->SetBooleanField(dst, hasClearCoat, src.hasClearCoat);
    env->SetBooleanField(dst, hasTransmission, src.hasTransmission);
    env->SetBooleanField(dst, hasTextureTransforms, src.hasTextureTransforms);
    env->SetIntField(dst, emissiveUV, src.emissiveUV);
    env->SetIntField(dst, aoUV, src.aoUV);
    env->SetIntField(dst, normalUV, src.normalUV);
    env->SetBooleanField(dst, hasTransmissionTexture, src.hasTransmissionTexture);
    env->SetIntField(dst, transmissionUV, src.transmissionUV);
    env->SetBooleanField(dst, hasSheenColorTexture, src.hasSheenColorTexture);
    env->SetIntField(dst, sheenColorUV, src.sheenColorUV);
    env->SetBooleanField(dst, hasSheenRoughnessTexture, src.hasSheenRoughnessTexture);
    env->SetIntField(dst, sheenRoughnessUV, src.sheenRoughnessUV);
    env->SetBooleanField(dst, hasVolumeThicknessTexture, src.hasVolumeThicknessTexture);
    env->SetIntField(dst, volumeThicknessUV, src.volumeThicknessUV);
    env->SetBooleanField(dst, hasSheen, src.hasSheen);
    env->SetBooleanField(dst, hasIOR, src.hasIOR);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_MaterialProvider_00024MaterialKey_nGlobalInit(JNIEnv* env, jclass) {
    MaterialKeyHelper::get().init(env);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_MaterialProvider_00024MaterialKey_nConstrainMaterial(JNIEnv* env, jclass,
        jobject materialKey, jintArray uvMap) {
    MaterialKey nativeMaterialKey = {};

    auto& helper = MaterialKeyHelper::get();
    helper.copy(env, nativeMaterialKey, materialKey);

    UvMap nativeUvMap = {};
    constrainMaterial(&nativeMaterialKey, &nativeUvMap);

    // Copy the UvMap results from the native array into the JVM array.
    jint* elements = env->GetIntArrayElements(uvMap, nullptr);
    if (elements) {
        const size_t javaSize = env->GetArrayLength(uvMap);
        for (int i = 0, n = std::min(javaSize, nativeUvMap.size()); i < n; ++i) {
            elements[i] = nativeUvMap[i];
        }
        env->ReleaseIntArrayElements(uvMap, elements, 0);
    }

    // The config parameter is an in-out parameter so we need to copy the results back to Java.
    helper.copy(env, materialKey, nativeMaterialKey);
}
