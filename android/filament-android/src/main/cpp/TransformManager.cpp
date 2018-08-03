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

#include <utils/Entity.h>
#include <filament/TransformManager.h>

using namespace utils;
using namespace filament;

static_assert(sizeof(jint) == sizeof(Entity), "jint and Entity are not compatible!!");

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_TransformManager_nHasComponent(JNIEnv *env, jclass type,
        jlong nativeTransformManager, jint entity) {
    TransformManager *tm = (TransformManager *) nativeTransformManager;
    return (jboolean) tm->hasComponent((Entity &) entity);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TransformManager_nGetInstance(JNIEnv *env, jclass type,
        jlong nativeTransformManager, jint entity) {
    TransformManager *tm = (TransformManager *) nativeTransformManager;
    return tm->getInstance((Entity &) entity);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TransformManager_nCreate__JI(JNIEnv *env, jclass type,
        jlong nativeTransformManager, jint entity) {
    TransformManager *tm = (TransformManager *) nativeTransformManager;
    tm->create((Entity &) entity);
    return tm->getInstance((Entity &) entity);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TransformManager_nCreate__JII_3F(JNIEnv *env, jclass type,
        jlong nativeTransformManager, jint entity, jint parent, jfloatArray localTransform_) {
    TransformManager *tm = (TransformManager *) nativeTransformManager;
    if (localTransform_) {
        jfloat *localTransform = env->GetFloatArrayElements(localTransform_, NULL);
        tm->create((Entity &) entity, (TransformManager::Instance) parent,
                *reinterpret_cast<const math::mat4f *>(localTransform));
        env->ReleaseFloatArrayElements(localTransform_, localTransform, JNI_ABORT);
    } else {
        tm->create((Entity &) entity, (TransformManager::Instance) parent);
    }
    return tm->getInstance((Entity &) entity);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_TransformManager_nDestroy(JNIEnv *env, jclass type,
        jlong nativeTransformManager, jint entity) {
    TransformManager *tm = (TransformManager *) nativeTransformManager;
    tm->destroy((Entity &) entity);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_TransformManager_nSetParent(JNIEnv *env, jclass type,
        jlong nativeTransformManager, jint i, jint newParent) {
    TransformManager *tm = (TransformManager *) nativeTransformManager;
    tm->setParent((TransformManager::Instance) i, (TransformManager::Instance) newParent);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_TransformManager_nSetTransform(JNIEnv *env, jclass type,
        jlong nativeTransformManager, jint i, jfloatArray localTransform_) {
    TransformManager *tm = (TransformManager *) nativeTransformManager;
    jfloat *localTransform = env->GetFloatArrayElements(localTransform_, NULL);
    tm->setTransform((TransformManager::Instance) i,
            *reinterpret_cast<const math::mat4f *>(localTransform));
    env->ReleaseFloatArrayElements(localTransform_, localTransform, JNI_ABORT);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_TransformManager_nGetTransform(JNIEnv *env, jclass type,
        jlong nativeTransformManager, jint i, jfloatArray outLocalTransform_) {
    TransformManager *tm = (TransformManager *) nativeTransformManager;
    jfloat *outLocalTransform = env->GetFloatArrayElements(outLocalTransform_, NULL);
    *reinterpret_cast<math::mat4f *>(outLocalTransform) = tm->getTransform(
            (TransformManager::Instance) i);
    env->ReleaseFloatArrayElements(outLocalTransform_, outLocalTransform, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_TransformManager_nGetWorldTransform(JNIEnv *env, jclass type,
        jlong nativeTransformManager, jint i, jfloatArray outWorldTransform_) {
    TransformManager *tm = (TransformManager *) nativeTransformManager;
    jfloat *outWorldTransform = env->GetFloatArrayElements(outWorldTransform_, NULL);
    *reinterpret_cast<math::mat4f *>(outWorldTransform) = tm->getWorldTransform(
            (TransformManager::Instance) i);
    env->ReleaseFloatArrayElements(outWorldTransform_, outWorldTransform, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_TransformManager_nOpenLocalTransformTransaction(JNIEnv *env,
        jclass type, jlong nativeTransformManager) {
    TransformManager *tm = (TransformManager *) nativeTransformManager;
    tm->openLocalTransformTransaction();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_TransformManager_nCommitLocalTransformTransaction(JNIEnv *env,
        jclass type, jlong nativeTransformManager) {
    TransformManager *tm = (TransformManager *) nativeTransformManager;
    tm->commitLocalTransformTransaction();
}
