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

using namespace gltfio;

void nativeFromJava(JNIEnv* env, MaterialKey& dst, jobject src) {
    const jclass materialKeyClass = env->FindClass(JAVA_MATERIAL_KEY);
    auto field = [materialKeyClass, env](const char* fieldName, const char* signature) {
        return env->GetFieldID(materialKeyClass, fieldName, signature);
    };
    dst.doubleSided = env->GetBooleanField(src, field("doubleSided", "Z"));
    dst.unlit = env->GetBooleanField(src, field("unlit", "Z"));
    dst.hasVertexColors = env->GetBooleanField(src, field("hasVertexColors", "Z"));
    dst.hasBaseColorTexture = env->GetBooleanField(src, field("hasBaseColorTexture", "Z"));
    dst.hasNormalTexture = env->GetBooleanField(src, field("hasNormalTexture", "Z"));
    dst.hasOcclusionTexture = env->GetBooleanField(src, field("hasOcclusionTexture", "Z"));
    dst.hasEmissiveTexture = env->GetBooleanField(src, field("hasEmissiveTexture", "Z"));
    dst.useSpecularGlossiness = env->GetBooleanField(src, field("useSpecularGlossiness", "Z"));
    dst.alphaMode = (AlphaMode) env->GetIntField(src, field("alphaMode", "I"));
    dst.enableDiagnostics = env->GetBooleanField(src, field("enableDiagnostics", "Z"));
    dst.hasMetallicRoughnessTexture = env->GetBooleanField(src, field("hasMetallicRoughnessTexture", "Z"));
    dst.metallicRoughnessUV = env->GetIntField(src, field("metallicRoughnessUV", "I"));
    dst.baseColorUV = env->GetIntField(src, field("baseColorUV", "I"));
    dst.hasClearCoatTexture = env->GetBooleanField(src, field("hasClearCoatTexture", "Z"));
    dst.clearCoatUV = env->GetIntField(src, field("clearCoatUV", "I"));
    dst.hasClearCoatRoughnessTexture = env->GetBooleanField(src, field("hasClearCoatRoughnessTexture", "Z"));
    dst.clearCoatRoughnessUV = env->GetIntField(src, field("clearCoatRoughnessUV", "I"));
    dst.hasClearCoatNormalTexture = env->GetBooleanField(src, field("hasClearCoatNormalTexture", "Z"));
    dst.clearCoatNormalUV = env->GetIntField(src, field("clearCoatNormalUV", "I"));
    dst.hasClearCoat = env->GetBooleanField(src, field("hasClearCoat", "Z"));
    dst.hasTransmission = env->GetBooleanField(src, field("hasTransmission", "Z"));
    dst.hasTextureTransforms = env->GetBooleanField(src, field("hasTextureTransforms", "Z"));
    dst.emissiveUV = env->GetIntField(src, field("emissiveUV", "I"));
    dst.aoUV = env->GetIntField(src, field("aoUV", "I"));
    dst.normalUV = env->GetIntField(src, field("normalUV", "I"));
    dst.hasTransmissionTexture = env->GetBooleanField(src, field("hasTransmissionTexture", "Z"));
    dst.transmissionUV = env->GetIntField(src, field("transmissionUV", "I"));
    dst.hasSheenColorTexture = env->GetBooleanField(src, field("hasSheenColorTexture", "Z"));
    dst.sheenColorUV = env->GetIntField(src, field("sheenColorUV", "I"));
    dst.hasSheenRoughnessTexture = env->GetBooleanField(src, field("hasSheenRoughnessTexture", "Z"));
    dst.sheenRoughnessUV = env->GetIntField(src, field("sheenRoughnessUV", "I"));
    dst.hasSheen = env->GetBooleanField(src, field("hasSheen", "Z"));
    dst.hasIOR = env->GetBooleanField(src, field("hasIOR", "Z"));
}

void nativeToJava(JNIEnv* env, MaterialKey& src, jobject dst) {
    const jclass materialKeyClass = env->FindClass(JAVA_MATERIAL_KEY);
    auto field = [materialKeyClass, env](const char* fieldName, const char* signature) {
        return env->GetFieldID(materialKeyClass, fieldName, signature);
    };
    env->SetBooleanField(dst, field("doubleSided", "Z"), src.doubleSided);
    env->SetBooleanField(dst, field("unlit", "Z"), src.unlit);
    env->SetBooleanField(dst, field("hasVertexColors", "Z"), src.hasVertexColors);
    env->SetBooleanField(dst, field("hasBaseColorTexture", "Z"), src.hasBaseColorTexture);
    env->SetBooleanField(dst, field("hasNormalTexture", "Z"), src.hasNormalTexture);
    env->SetBooleanField(dst, field("hasOcclusionTexture", "Z"), src.hasOcclusionTexture);
    env->SetBooleanField(dst, field("hasEmissiveTexture", "Z"), src.hasEmissiveTexture);
    env->SetBooleanField(dst, field("useSpecularGlossiness", "Z"), src.useSpecularGlossiness);
    env->SetIntField(dst, field("alphaMode", "I"), (int) src.alphaMode);
    env->SetBooleanField(dst, field("enableDiagnostics", "Z"), src.enableDiagnostics);
    env->SetBooleanField(dst, field("hasMetallicRoughnessTexture", "Z"), src.hasMetallicRoughnessTexture);
    env->SetIntField(dst, field("metallicRoughnessUV", "I"), src.metallicRoughnessUV);
    env->SetIntField(dst, field("baseColorUV", "I"), src.baseColorUV);
    env->SetBooleanField(dst, field("hasClearCoatTexture", "Z"), src.hasClearCoatTexture);
    env->SetIntField(dst, field("clearCoatUV", "I"), src.clearCoatUV);
    env->SetBooleanField(dst, field("hasClearCoatRoughnessTexture", "Z"), src.hasClearCoatRoughnessTexture);
    env->SetIntField(dst, field("clearCoatRoughnessUV", "I"), src.clearCoatRoughnessUV);
    env->SetBooleanField(dst, field("hasClearCoatNormalTexture", "Z"), src.hasClearCoatNormalTexture);
    env->SetIntField(dst, field("clearCoatNormalUV", "I"), src.clearCoatNormalUV);
    env->SetBooleanField(dst, field("hasClearCoat", "Z"), src.hasClearCoat);
    env->SetBooleanField(dst, field("hasTransmission", "Z"), src.hasTransmission);
    env->SetBooleanField(dst, field("hasTextureTransforms", "Z"), src.hasTextureTransforms);
    env->SetIntField(dst, field("emissiveUV", "I"), src.emissiveUV);
    env->SetIntField(dst, field("aoUV", "I"), src.aoUV);
    env->SetIntField(dst, field("normalUV", "I"), src.normalUV);
    env->SetBooleanField(dst, field("hasTransmissionTexture", "Z"), src.hasTransmissionTexture);
    env->SetIntField(dst, field("transmissionUV", "I"), src.transmissionUV);
    env->SetBooleanField(dst, field("hasSheenColorTexture", "Z"), src.hasSheenColorTexture);
    env->SetIntField(dst, field("sheenColorUV", "I"), src.sheenColorUV);
    env->SetBooleanField(dst, field("hasSheenRoughnessTexture", "Z"), src.hasSheenRoughnessTexture);
    env->SetIntField(dst, field("sheenRoughnessUV", "I"), src.sheenRoughnessUV);
    env->SetBooleanField(dst, field("hasSheen", "Z"), src.hasSheen);
    env->SetBooleanField(dst, field("hasIOR", "Z"), src.hasIOR);
}
