/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <utils/EntityManager.h>
#include "../../../../common/JniExceptionBridge.h"

using namespace utils;

static_assert(sizeof(jint) == sizeof(Entity), "jint and Entity are not compatible!!");

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_EntityManager_nGetEntityManager(JNIEnv* env, jclass) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_EntityManager_nGetEntityManager", 0, [&]() -> jlong {
            return (jlong) &EntityManager::get();
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_EntityManager_nCreateArray(JNIEnv* env, jclass,
        jlong nativeEntityManager, jint n, jintArray entities_) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_EntityManager_nCreateArray", [&]() {
            EntityManager* em = (EntityManager *) nativeEntityManager;
            jint* entities = env->GetIntArrayElements(entities_, NULL);

            // This is very very gross, we just pretend Entity is just like an jint
            // (which it is), but still.
            em->create((size_t) n, reinterpret_cast<Entity *>(entities));

            env->ReleaseIntArrayElements(entities_, entities, 0);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_EntityManager_nCreate(JNIEnv* env, jclass,
        jlong nativeEntityManager) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_EntityManager_nCreate", 0, [&]() -> jint {
            EntityManager* em = (EntityManager *) nativeEntityManager;
            return em->create().getId();
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_EntityManager_nDestroyArray(JNIEnv* env, jclass,
        jlong nativeEntityManager, jint n, jintArray entities_) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_EntityManager_nDestroyArray", [&]() {
            EntityManager* em = (EntityManager *) nativeEntityManager;
            jint* entities = env->GetIntArrayElements(entities_, NULL);

            // This is very very gross, we just pretend Entity is just like an
            // jint (which it is), but still.
            em->destroy((size_t) n, reinterpret_cast<Entity*>(entities));

            env->ReleaseIntArrayElements(entities_, entities, JNI_ABORT);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_EntityManager_nDestroy(JNIEnv* env, jclass,
        jlong nativeEntityManager, jint entity_) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_EntityManager_nDestroy", [&]() {
            EntityManager *em = (EntityManager *) nativeEntityManager;
            Entity& entity = *reinterpret_cast<Entity*>(&entity_);
            em->destroy(entity);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_EntityManager_nIsAlive(JNIEnv* env, jclass,
        jlong nativeEntityManager, jint entity_) {
    return filament::android::jniGuard<jboolean>(env, "Java_com_google_android_filament_EntityManager_nIsAlive", JNI_FALSE, [&]() -> jboolean {
            EntityManager *em = (EntityManager *) nativeEntityManager;
            Entity& entity = *reinterpret_cast<Entity*>(&entity_);
            return (jboolean) em->isAlive(entity);
    });
}
