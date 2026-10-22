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

using namespace filament;
using namespace filament::gltfio;
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

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_gltfio_FilamentInstance_nGetAnimator(JNIEnv* , jclass,
        jlong nativeInstance) {
    FilamentInstance* instance = (FilamentInstance*) nativeInstance;
    return (jlong) instance->getAnimator();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_FilamentInstance_nApplyMaterialVariant(JNIEnv* env, jclass,
        jlong nativeInstance, jint variantIndex) {
    FilamentInstance* instance = (FilamentInstance*) nativeInstance;
    instance->applyMaterialVariant(variantIndex);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_gltfio_FilamentInstance_nGetMaterialVariantCount(JNIEnv*, jclass,
        jlong nativeInstance) {
    FilamentInstance* instance = (FilamentInstance*) nativeInstance;
    return (jint) instance->getMaterialVariantCount();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_FilamentInstance_nGetMaterialVariantNames(JNIEnv* env, jclass,
        jlong nativeInstance, jobjectArray result) {
    FilamentInstance* instance = (FilamentInstance*) nativeInstance;
    for (int i = 0; i < instance->getMaterialVariantCount(); ++i) {
        const char* name = instance->getMaterialVariantName(i);
        env->SetObjectArrayElement(result, (jsize) i, env->NewStringUTF(name));
    }
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_gltfio_FilamentInstance_nGetMaterialInstanceCount(JNIEnv*, jclass,
        jlong nativeInstance) {
    FilamentInstance* instance = (FilamentInstance*) nativeInstance;
    return instance->getMaterialInstanceCount();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_FilamentInstance_nGetMaterialInstances(JNIEnv* env, jclass,
        jlong nativeInstance, jlongArray result) {
    FilamentInstance* instance = (FilamentInstance*) nativeInstance;
    jsize available = env->GetArrayLength(result);
    jsize count = std::min(available, (jsize) instance->getMaterialInstanceCount());
    jlong* dst = env->GetLongArrayElements(result, nullptr);
    const MaterialInstance * const* src = instance->getMaterialInstances();
    for (jsize i = 0; i < count; i++) {
        dst[i] = (jlong) src[i];
    }
    env->ReleaseLongArrayElements(result, dst, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_FilamentInstance_nAttachSkin(JNIEnv* env, jclass,
        jlong nativeInstance, jint skinIndex, jint targetEntity) {
    FilamentInstance* instance = (FilamentInstance*) nativeInstance;
    Entity target = Entity::import(targetEntity);
    instance->attachSkin(skinIndex, target);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_FilamentInstance_nDetachSkin(JNIEnv* env, jclass,
        jlong nativeInstance, jint skinIndex, jint targetEntity) {
    FilamentInstance* instance = (FilamentInstance*) nativeInstance;
    Entity target = Entity::import(targetEntity);
    instance->detachSkin(skinIndex, target);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_gltfio_FilamentInstance_nGetSkinCount(JNIEnv* , jclass,
        jlong nativeInstance) {
    FilamentInstance* instance = (FilamentInstance*) nativeInstance;
    return (jint) instance->getSkinCount();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_FilamentInstance_nGetSkinNames(JNIEnv* env, jclass,
        jlong nativeInstance, jobjectArray result) {
    FilamentInstance* instance = (FilamentInstance*) nativeInstance;
    jsize available = env->GetArrayLength(result);
    for (int i = 0; i < available; ++i) {
        const char* name = instance->getSkinNameAt(i);
        if (name) {
            env->SetObjectArrayElement(result, (jsize) i, env->NewStringUTF(name));
        }
    }
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_gltfio_FilamentInstance_nGetJointCountAt(JNIEnv* , jclass,
        jlong nativeInstance, jint skinIndex) {
    FilamentInstance* instance = (FilamentInstance*) nativeInstance;
    return (jint) instance->getJointCountAt(skinIndex);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_FilamentInstance_nGetJointsAt(JNIEnv* env, jclass,
        jlong nativeInstance, jint skinIndex, jintArray result) {
    FilamentInstance* instance = (FilamentInstance*) nativeInstance;
    jsize available = env->GetArrayLength(result);
    Entity* entities = (Entity*) env->GetIntArrayElements(result, nullptr);
    std::copy_n(instance->getJointsAt(skinIndex),
        std::min(available, (jsize) instance->getJointCountAt(skinIndex)), entities);
    env->ReleaseIntArrayElements(result, (jint*) entities, 0);
}
