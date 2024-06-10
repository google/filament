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

#include <filament/Scene.h>

#include <utils/Entity.h>

using namespace filament;
using namespace utils;

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Scene_nSetSkybox(JNIEnv *env, jclass type, jlong nativeScene,
        jlong nativeSkybox) {
    Scene* scene = (Scene*) nativeScene;
    Skybox* skybox = (Skybox*) nativeSkybox;
    scene->setSkybox(skybox);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Scene_nSetIndirectLight(JNIEnv *env, jclass type,
        jlong nativeScene, jlong nativeIndirectLight) {
    Scene *scene = (Scene *) nativeScene;
    IndirectLight* indirectLight = (IndirectLight*) nativeIndirectLight;
    scene->setIndirectLight(indirectLight);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Scene_nAddEntity(JNIEnv *env, jclass type, jlong nativeScene,
        jint entity) {
    Scene* scene = (Scene*) nativeScene;
    scene->addEntity((Entity&) entity);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Scene_nAddEntities(JNIEnv *env, jclass type, jlong nativeScene,
        jintArray entities) {
    Scene* scene = (Scene*) nativeScene;
    Entity* nativeEntities = (Entity*) env->GetIntArrayElements(entities, nullptr);
    scene->addEntities(nativeEntities, env->GetArrayLength(entities));
    env->ReleaseIntArrayElements(entities, (jint*) nativeEntities, JNI_ABORT);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Scene_nRemove(JNIEnv *env, jclass type, jlong nativeScene,
        jint entity) {
    Scene* scene = (Scene*) nativeScene;
    scene->remove((Entity&) entity);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Scene_nRemoveEntities(JNIEnv *env, jclass type, jlong nativeScene,
        jintArray entities) {
    Scene* scene = (Scene*) nativeScene;
    Entity* nativeEntities = (Entity*) env->GetIntArrayElements(entities, nullptr);
    scene->removeEntities(nativeEntities, env->GetArrayLength(entities));
    env->ReleaseIntArrayElements(entities, (jint*) nativeEntities, JNI_ABORT);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Scene_nGetEntityCount(JNIEnv *env, jclass type,
        jlong nativeScene) {
    Scene* scene = (Scene*) nativeScene;
    return (jint) scene->getEntityCount();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Scene_nGetRenderableCount(JNIEnv *env, jclass type,
        jlong nativeScene) {
    Scene* scene = (Scene*) nativeScene;
    return (jint) scene->getRenderableCount();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Scene_nGetLightCount(JNIEnv *env, jclass type, jlong nativeScene) {
    Scene* scene = (Scene*) nativeScene;
    return (jint) scene->getLightCount();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Scene_nHasEntity(JNIEnv *env, jclass type, jlong nativeScene,
        jint entityId) {
    Scene* scene = (Scene*) nativeScene;
    Entity entity = Entity::import(entityId);
    return (jboolean) scene->hasEntity(entity);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Scene_nGetEntities(JNIEnv *env, jclass ,
        jlong nativeScene, jintArray outArray, jint length) {
    Scene const* const scene = (Scene*) nativeScene;
    if (length < scene->getEntityCount()) {
        // should not happen because we already checked on the java side
        return JNI_FALSE;
    }
    jint *out = (jint *) env->GetIntArrayElements(outArray, nullptr);
    scene->forEach([out, length, i = 0](Entity entity)mutable {
        if (i < length) { // this is just paranoia here
            out[i++] = (jint) entity.getId();
        }
    });
    env->ReleaseIntArrayElements(outArray, (jint*) out, 0);
    return JNI_TRUE;
}
