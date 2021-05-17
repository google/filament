/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include <gltfio/FilamentInstance.h>

#include <algorithm>

using namespace gltfio;
using namespace utils;

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_gltfio_FilamentInstance_nGetRoot(JNIEnv*, jclass,
        jlong nativeInstance) {
    FilamentInstance* instance = (FilamentInstance*) nativeInstance;
    return instance->getRoot().getId();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_gltfio_FilamentInstance_nGetEntityCount(JNIEnv*, jclass,
        jlong nativeInstance) {
    FilamentInstance* instance = (FilamentInstance*) nativeInstance;
    return instance->getEntityCount();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_FilamentInstance_nGetEntities(JNIEnv* env, jclass,
        jlong nativeInstance, jintArray result) {
    FilamentInstance* instance = (FilamentInstance*) nativeInstance;
    jsize available = env->GetArrayLength(result);
    Entity* entities = (Entity*) env->GetIntArrayElements(result, nullptr);
    std::copy_n(instance->getEntities(),
            std::min(available, (jsize) instance->getEntityCount()), entities);
    env->ReleaseIntArrayElements(result, (jint*) entities, 0);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_google_android_filament_gltfio_FilamentInstance_nGetName(JNIEnv* env, jclass,
        jlong nativeInstance, jint entityId) {
    Entity entity = Entity::import(entityId);
    FilamentInstance* instance = (FilamentInstance*) nativeInstance;
    const char* val = instance->getName(entity);
    return val ? env->NewStringUTF(val) : nullptr;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_google_android_filament_gltfio_FilamentInstance_nGetExtras(JNIEnv* env, jclass,
        jlong nativeInstance, jint entityId) {
    Entity entity = Entity::import(entityId);
    FilamentInstance* instance = (FilamentInstance*) nativeInstance;
    const auto val = instance->getExtras(entity);
    return val ? env->NewStringUTF(val) : nullptr;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_gltfio_FilamentInstance_nGetAnimator(JNIEnv* , jclass,
        jlong nativeInstance) {
    FilamentInstance* instance = (FilamentInstance*) nativeInstance;
    return (jlong) instance->getAnimator();
}
