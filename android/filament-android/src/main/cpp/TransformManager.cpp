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

#include <filament/TransformManager.h>

#include <utils/Entity.h>

#include <math/mat4.h>

using namespace utils;
using namespace filament;

static_assert(sizeof(jint) == sizeof(Entity), "jint and Entity are not compatible!!");

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_TransformManager_nHasComponent(JNIEnv*, jclass,
        jlong nativeTransformManager, jint entity_) {
    TransformManager* tm = (TransformManager*) nativeTransformManager;
    Entity& entity = *reinterpret_cast<Entity*>(&entity_);
    return (jboolean) tm->hasComponent(entity);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TransformManager_nGetInstance(JNIEnv*, jclass,
        jlong nativeTransformManager, jint entity_) {
    TransformManager* tm = (TransformManager*) nativeTransformManager;
    Entity& entity = *reinterpret_cast<Entity*>(&entity_);
    return tm->getInstance(entity);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TransformManager_nCreate(JNIEnv*, jclass,
        jlong nativeTransformManager, jint entity_) {
    TransformManager* tm = (TransformManager*) nativeTransformManager;
    Entity& entity = *reinterpret_cast<Entity*>(&entity_);
    tm->create(entity);
    return tm->getInstance(entity);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TransformManager_nCreateArray(JNIEnv* env,
        jclass, jlong nativeTransformManager, jint entity_, jint parent,
        jfloatArray localTransform_) {
    TransformManager* tm = (TransformManager*) nativeTransformManager;
    Entity& entity = *reinterpret_cast<Entity*>(&entity_);
    if (localTransform_) {
        jfloat *localTransform = env->GetFloatArrayElements(localTransform_, NULL);
        tm->create(entity, (TransformManager::Instance) parent,
                *reinterpret_cast<const filament::math::mat4f *>(localTransform));
        env->ReleaseFloatArrayElements(localTransform_, localTransform, JNI_ABORT);
    } else {
        tm->create(entity, (TransformManager::Instance) parent);
    }
    return tm->getInstance(entity);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TransformManager_nCreateArrayFp64(JNIEnv* env,
        jclass, jlong nativeTransformManager, jint entity_, jint parent,
        jdoubleArray localTransform_) {
    TransformManager* tm = (TransformManager*) nativeTransformManager;
    Entity& entity = *reinterpret_cast<Entity*>(&entity_);
    if (localTransform_) {
        jdouble *localTransform = env->GetDoubleArrayElements(localTransform_, NULL);
        tm->create(entity, (TransformManager::Instance) parent,
                *reinterpret_cast<const filament::math::mat4 *>(localTransform));
        env->ReleaseDoubleArrayElements(localTransform_, localTransform, JNI_ABORT);
    } else {
        tm->create(entity, (TransformManager::Instance) parent);
    }
    return tm->getInstance(entity);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_TransformManager_nDestroy(JNIEnv*, jclass,
        jlong nativeTransformManager, jint entity_) {
    TransformManager* tm = (TransformManager*) nativeTransformManager;
    Entity& entity = *reinterpret_cast<Entity*>(&entity_);
    tm->destroy(entity);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_TransformManager_nSetParent(JNIEnv*, jclass,
        jlong nativeTransformManager, jint i, jint newParent) {
    TransformManager* tm = (TransformManager*) nativeTransformManager;
    tm->setParent((TransformManager::Instance) i,
            (TransformManager::Instance) newParent);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TransformManager_nGetParent(JNIEnv*, jclass,
        jlong nativeTransformManager, jint i) {
    TransformManager* tm = (TransformManager*) nativeTransformManager;
    return tm->getParent((TransformManager::Instance) i).getId();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TransformManager_nGetChildCount(JNIEnv*, jclass,
        jlong nativeTransformManager, jint i) {
    TransformManager* tm = (TransformManager*) nativeTransformManager;
    return tm->getChildCount((TransformManager::Instance) i);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_TransformManager_nGetChildren(JNIEnv* env,
        jclass, jlong nativeTransformManager, jint i,
        jintArray outEntities_, jint count) {
    TransformManager* tm = (TransformManager*) nativeTransformManager;
    jint* entities = env->GetIntArrayElements(outEntities_, nullptr);
    // This is very very gross, we just pretend Entity is just like an jint
    // (which it is), but still.
    tm->getChildren((TransformManager::Instance) i,
            reinterpret_cast<Entity *>(entities), (size_t) count);
    env->ReleaseIntArrayElements(outEntities_, entities, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_TransformManager_nSetTransform(JNIEnv* env,
        jclass, jlong nativeTransformManager, jint i,
        jfloatArray localTransform_) {
    TransformManager* tm = (TransformManager*) nativeTransformManager;
    jfloat *localTransform = env->GetFloatArrayElements(localTransform_, NULL);
    tm->setTransform((TransformManager::Instance) i,
            *reinterpret_cast<const filament::math::mat4f *>(localTransform));
    env->ReleaseFloatArrayElements(localTransform_, localTransform, JNI_ABORT);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_TransformManager_nSetTransformFp64(JNIEnv* env,
        jclass, jlong nativeTransformManager, jint i,
        jdoubleArray localTransform_) {
    TransformManager* tm = (TransformManager*) nativeTransformManager;
    jdouble *localTransform = env->GetDoubleArrayElements(localTransform_, NULL);
    tm->setTransform((TransformManager::Instance) i,
            *reinterpret_cast<const filament::math::mat4 *>(localTransform));
    env->ReleaseDoubleArrayElements(localTransform_, localTransform, JNI_ABORT);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_TransformManager_nGetTransform(JNIEnv* env,
        jclass, jlong nativeTransformManager, jint i,
        jfloatArray outLocalTransform_) {
    TransformManager* tm = (TransformManager*) nativeTransformManager;
    jfloat *outLocalTransform = env->GetFloatArrayElements(outLocalTransform_, NULL);
    *reinterpret_cast<filament::math::mat4f *>(outLocalTransform) = tm->getTransform(
            (TransformManager::Instance) i);
    env->ReleaseFloatArrayElements(outLocalTransform_, outLocalTransform, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_TransformManager_nGetTransformFp64(JNIEnv* env,
        jclass, jlong nativeTransformManager, jint i,
        jdoubleArray outLocalTransform_) {
    TransformManager* tm = (TransformManager*) nativeTransformManager;
    jdouble *outLocalTransform = env->GetDoubleArrayElements(outLocalTransform_, NULL);
    *reinterpret_cast<filament::math::mat4 *>(outLocalTransform) = tm->getTransformAccurate(
            (TransformManager::Instance) i);
    env->ReleaseDoubleArrayElements(outLocalTransform_, outLocalTransform, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_TransformManager_nGetWorldTransform(JNIEnv* env,
        jclass, jlong nativeTransformManager, jint i,
        jfloatArray outWorldTransform_) {
    TransformManager* tm = (TransformManager*) nativeTransformManager;
    jfloat *outWorldTransform = env->GetFloatArrayElements(outWorldTransform_, NULL);
    *reinterpret_cast<filament::math::mat4f *>(outWorldTransform) = tm->getWorldTransform(
            (TransformManager::Instance) i);
    env->ReleaseFloatArrayElements(outWorldTransform_, outWorldTransform, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_TransformManager_nGetWorldTransformFp64(JNIEnv* env,
        jclass, jlong nativeTransformManager, jint i,
        jdoubleArray outWorldTransform_) {
    TransformManager* tm = (TransformManager*) nativeTransformManager;
    jdouble *outWorldTransform = env->GetDoubleArrayElements(outWorldTransform_, NULL);
    *reinterpret_cast<filament::math::mat4 *>(outWorldTransform) = tm->getWorldTransformAccurate(
            (TransformManager::Instance) i);
    env->ReleaseDoubleArrayElements(outWorldTransform_, outWorldTransform, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_TransformManager_nOpenLocalTransformTransaction(
        JNIEnv*, jclass, jlong nativeTransformManager) {
    TransformManager* tm = (TransformManager*) nativeTransformManager;
    tm->openLocalTransformTransaction();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_TransformManager_nCommitLocalTransformTransaction(
        JNIEnv*, jclass, jlong nativeTransformManager) {
    TransformManager* tm = (TransformManager*) nativeTransformManager;
    tm->commitLocalTransformTransaction();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_TransformManager_nSetAccurateTranslationsEnabled(JNIEnv*,
        jclass, jlong nativeTransformManager, jboolean enable) {
    TransformManager* tm = (TransformManager*) nativeTransformManager;
    tm->setAccurateTranslationsEnabled((bool)enable);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_TransformManager_nIsAccurateTranslationsEnabled(JNIEnv*,
        jclass, jlong nativeTransformManager) {
    TransformManager* tm = (TransformManager*) nativeTransformManager;
    return (jboolean)tm->isAccurateTranslationsEnabled();
}
