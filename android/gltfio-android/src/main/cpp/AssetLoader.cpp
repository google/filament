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

#include <filament/Engine.h>

#include <utils/EntityManager.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/MaterialProvider.h>

#include "common/NioUtils.h"

using namespace filament;
using namespace gltfio;
using namespace utils;

extern void registerCallbackUtils(JNIEnv*);
extern void registerNioUtils(JNIEnv*);

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    registerCallbackUtils(env);
    registerNioUtils(env);

    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_gltfio_AssetLoader_nCreateAssetLoader(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeProvider, jlong nativeEntities) {
    Engine* engine = (Engine*) nativeEngine;
    MaterialProvider* materials = (MaterialProvider*) nativeProvider;
    EntityManager* entities = (EntityManager*) nativeEntities;
    return (jlong) AssetLoader::create({engine, materials, nullptr, entities});
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_AssetLoader_nDestroyAssetLoader(JNIEnv*, jclass,
        jlong nativeLoader) {
    AssetLoader* loader = (AssetLoader*) nativeLoader;
    AssetLoader::destroy(&loader);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_gltfio_AssetLoader_nCreateAssetFromBinary(JNIEnv* env, jclass,
        jlong nativeLoader, jobject javaBuffer, jint remaining) {
    AssetLoader* loader = (AssetLoader*) nativeLoader;
    AutoBuffer buffer(env, javaBuffer, remaining);
    return (jlong) loader->createAssetFromBinary((const uint8_t *) buffer.getData(),
            buffer.getSize());
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_AssetLoader_nEnableDiagnostics(JNIEnv*, jclass,
        jlong nativeLoader, jboolean enable) {
    AssetLoader* loader = (AssetLoader*) nativeLoader;
    loader->enableDiagnostics(enable);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_AssetLoader_nDestroyAsset(JNIEnv*, jclass,
        jlong nativeLoader, jlong nativeAsset) {
    AssetLoader* loader = (AssetLoader*) nativeLoader;    
    FilamentAsset* asset = (FilamentAsset*) nativeAsset;
    loader->destroyAsset(asset);
}
