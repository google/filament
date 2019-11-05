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

#include <gltfio/Animator.h>

using namespace filament;
using namespace filament::math;
using namespace gltfio;
using namespace utils;

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_Animator_nApplyAnimation(JNIEnv*, jclass, jlong nativeAnimator,
        jint index, jfloat time) {
    Animator* animator = (Animator*) nativeAnimator;
    animator->applyAnimation(static_cast<size_t>(index), time);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_Animator_nUpdateBoneMatrices(JNIEnv*, jclass, jlong nativeAnimator) {
    Animator* animator = (Animator*) nativeAnimator;
    animator->updateBoneMatrices();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_gltfio_Animator_nGetAnimationCount(JNIEnv*, jclass, jlong nativeAnimator) {
    Animator* animator = (Animator*) nativeAnimator;
    return animator->getAnimationCount();
}

extern "C" JNIEXPORT float JNICALL
Java_com_google_android_filament_gltfio_Animator_nGetAnimationDuration(JNIEnv*, jclass,
        jlong nativeAnimator, jint index) {
    Animator* animator = (Animator*) nativeAnimator;
    return animator->getAnimationDuration(static_cast<size_t>(index));
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_google_android_filament_gltfio_Animator_nGetAnimationName(JNIEnv* env, jclass,
        jlong nativeAnimator, jint index) {
    Animator* animator = (Animator*) nativeAnimator;
    const char* val = animator->getAnimationName(static_cast<size_t>(index));
    return val ? env->NewStringUTF(val) : nullptr;

}
