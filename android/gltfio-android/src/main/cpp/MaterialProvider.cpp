/*
 * Copyright (C) 2019 The Android Open Source Project
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

using namespace filament;
using namespace gltfio;

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_gltfio_MaterialProvider_nCreateMaterialProvider(JNIEnv* jniEnv,
        jclass, jlong nativeEngine, jlong nativeMaterialSource,
        jobject externalSource) {

    Engine* engine = (Engine*) nativeEngine;

    if (nativeMaterialSource == 0 /* GENERATE_SHADERS */) {
        return (jlong) 0;
    }

    if (nativeMaterialSource == 1 /* LOAD_UBERSHADERS */) {
        return (jlong) createUbershaderLoader(engine);
    }

    if (nativeMaterialSource == 2 /* EXTERNAL */) {
        jclass jMaterialProviderCls = jniEnv->FindClass(
                "com/google/android/filament/gltfio/MaterialProvider");
        jMaterialProviderCls = (jclass) jniEnv->NewGlobalRef(jMaterialProviderCls);
        jmethodID jMaterialProviderNewMaterial = jniEnv->GetStaticMethodID(
                jMaterialProviderCls,
                "newMaterial",
                "(J)Lcom/google/android/filament/Material;");
        jmethodID jMaterialProviderGetMaterialNativeObject = jniEnv->GetStaticMethodID(
                jMaterialProviderCls,
                "getMaterialNativeObject",
                "(Lcom/google/android/filament/Material;)J");
        jmethodID jMaterialProviderGetMaterialInstanceNativeObject = jniEnv->GetStaticMethodID(
                jMaterialProviderCls,
                "getMaterialInstanceNativeObject",
                "(Lcom/google/android/filament/MaterialInstance;)J");

        jclass jExternalSourceCls = jniEnv->FindClass(
                "com/google/android/filament/gltfio/MaterialProvider$ExternalSource");
        jmethodID jExternalSourceResolveMaterial = jniEnv->GetMethodID(
                jExternalSourceCls,
                "resolveMaterial",
                "(Lcom/google/android/filament/gltfio/MaterialProvider$MaterialConfig;Lcom/google/android/filament/gltfio/MaterialProvider$UvMap;Ljava/lang/String;)Lcom/google/android/filament/Material;");
        jmethodID jExternalSourceInstantiateMaterial = jniEnv->GetMethodID(
                jExternalSourceCls,
                "instantiateMaterial",
                "(Lcom/google/android/filament/Material;Lcom/google/android/filament/gltfio/MaterialProvider$MaterialConfig;Lcom/google/android/filament/gltfio/MaterialProvider$UvMap;)Lcom/google/android/filament/MaterialInstance;");

        jclass jMaterialConfigCls = jniEnv->FindClass(
                "com/google/android/filament/gltfio/MaterialProvider$MaterialConfig");
        jMaterialConfigCls = (jclass) jniEnv->NewGlobalRef(jMaterialConfigCls);
        jmethodID jMaterialConfigCtor = jniEnv->GetMethodID(jMaterialConfigCls, "<init>", "(JJ)V");

        jclass jUvMapCls = jniEnv->FindClass(
                "com/google/android/filament/gltfio/MaterialProvider$UvMap");
        jUvMapCls = (jclass) jniEnv->NewGlobalRef(jUvMapCls);
        jmethodID jUvMapCtor = jniEnv->GetMethodID(jUvMapCls, "<init>", "(J)V");

        externalSource = jniEnv->NewGlobalRef(externalSource);

        const ExternalSourceMaterialResolver resolverWrapper =
                [jniEnv, jMaterialConfigCls, jMaterialConfigCtor, jUvMapCls, jUvMapCtor,
                        jExternalSourceResolveMaterial, externalSource,
                        jMaterialProviderCls, jMaterialProviderGetMaterialNativeObject]
                (Engine* engine, MaterialKey* config, UvMap* uvMap, const char* name) {
                    std::uint64_t jMaterialConfigValue0 = reinterpret_cast<std::uint64_t*>(config)[0];
                    std::uint64_t jMaterialConfigValue1 = reinterpret_cast<std::uint32_t*>(config)[2];
                    jobject jMaterialConfig = jniEnv->NewObject(
                            jMaterialConfigCls, jMaterialConfigCtor, jMaterialConfigValue0,
                            jMaterialConfigValue1);

                    jobject jUvMap = jniEnv->NewObject(
                            jUvMapCls, jUvMapCtor, *reinterpret_cast<std::uint64_t*>(uvMap->data()));

                    jstring jName = jniEnv->NewStringUTF(name);

                    jobject jMaterial = jniEnv->CallObjectMethod(
                            externalSource, jExternalSourceResolveMaterial, jMaterialConfig, jUvMap,
                            jName);
                    if (!jMaterial) {
                        return (filament::Material*) nullptr;
                    }

                    return (filament::Material*) jniEnv->CallStaticLongMethod(jMaterialProviderCls, jMaterialProviderGetMaterialNativeObject, jMaterial);
                };

        const ExternalSourceMaterialInstantiator instantiatorWrapper =
                [jniEnv, jMaterialProviderCls, jMaterialProviderGetMaterialInstanceNativeObject,
                        jMaterialProviderNewMaterial, jMaterialConfigCls, jMaterialConfigCtor,
                        jUvMapCls, jUvMapCtor, jExternalSourceInstantiateMaterial, externalSource]
                (Engine* engine, Material* material, MaterialKey* config, UvMap* uvMap) {
                    jobject jMaterial = jniEnv->CallStaticObjectMethod(jMaterialProviderCls, jMaterialProviderNewMaterial, (jlong) material);

                    std::uint64_t jMaterialConfigValue0 = reinterpret_cast<std::uint64_t*>(config)[0];
                    std::uint64_t jMaterialConfigValue1 = reinterpret_cast<std::uint32_t*>(config)[2];
                    jobject jMaterialConfig = jniEnv->NewObject(
                            jMaterialConfigCls, jMaterialConfigCtor, jMaterialConfigValue0,
                            jMaterialConfigValue1);

                    jobject jUvMap = jniEnv->NewObject(
                            jUvMapCls, jUvMapCtor, *reinterpret_cast<std::uint64_t*>(uvMap->data()));

                    jobject jMaterialInstance = jniEnv->CallObjectMethod(
                            externalSource, jExternalSourceInstantiateMaterial, jMaterial, jMaterialConfig, jUvMap);
                    if (!jMaterialInstance) {
                        return (filament::MaterialInstance*) nullptr;
                    }

                    return (filament::MaterialInstance*) jniEnv->CallStaticLongMethod(
                            jMaterialProviderCls,
                            jMaterialProviderGetMaterialInstanceNativeObject,
                            jMaterialInstance);
                };

        return (jlong) createExternalMaterialLoader(engine, resolverWrapper, instantiatorWrapper);
    }
    return 0L;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_MaterialProvider_nDestroyMaterialProvider(JNIEnv*, jclass,
        jlong nativeProvider) {
    auto provider = (MaterialProvider*) nativeProvider;
    delete provider;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_MaterialProvider_nDestroyMaterials(JNIEnv*, jclass,
        jlong nativeProvider) {
    auto provider = (MaterialProvider*) nativeProvider;
    provider->destroyMaterials();
}
