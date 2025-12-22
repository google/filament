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
#include "../../../../common/JniExceptionBridge.h"

using namespace filament;
using namespace filament::math;
using namespace filament::gltfio;
using namespace utils;

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_Animator_nApplyAnimation(JNIEnv* env, jclass, jlong nativeAnimator,
        jint index, jfloat time) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_gltfio_Animator_nApplyAnimation", [&]() {
            Animator* animator = (Animator*) nativeAnimator;
            animator->applyAnimation(static_cast<size_t>(index), time);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_Animator_nUpdateBoneMatrices(JNIEnv* env, jclass, jlong nativeAnimator) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_gltfio_Animator_nUpdateBoneMatrices", [&]() {
            Animator* animator = (Animator*) nativeAnimator;
            animator->updateBoneMatrices();
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_Animator_nApplyCrossFade(JNIEnv* env, jclass, jlong nativeAnimator,
        jint previousAnimIndex, jfloat previousAnimTime, jfloat alpha) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_gltfio_Animator_nApplyCrossFade", [&]() {
            Animator* animator = (Animator*) nativeAnimator;
            animator->applyCrossFade(previousAnimIndex, previousAnimTime, alpha);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_Animator_nResetBoneMatrices(JNIEnv* env, jclass, jlong nativeAnimator) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_gltfio_Animator_nResetBoneMatrices", [&]() {
            Animator* animator = (Animator*) nativeAnimator;
            animator->resetBoneMatrices();
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_gltfio_Animator_nGetAnimationCount(JNIEnv* env, jclass, jlong nativeAnimator) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_gltfio_Animator_nGetAnimationCount", 0, [&]() -> jint {
            Animator* animator = (Animator*) nativeAnimator;
            return animator->getAnimationCount();
    });
}

extern "C" JNIEXPORT float JNICALL
Java_com_google_android_filament_gltfio_Animator_nGetAnimationDuration(JNIEnv* env, jclass,
        jlong nativeAnimator, jint index) {
    return filament::android::jniGuard<float>(env, "Java_com_google_android_filament_gltfio_Animator_nGetAnimationDuration", 0, [&]() -> float {
            Animator* animator = (Animator*) nativeAnimator;
            return animator->getAnimationDuration(static_cast<size_t>(index));
    });
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_google_android_filament_gltfio_Animator_nGetAnimationName(JNIEnv* env, jclass,
        jlong nativeAnimator, jint index) {
    return filament::android::jniGuard<jstring>(env, "Java_com_google_android_filament_gltfio_Animator_nGetAnimationName", 0, [&]() -> jstring {
            Animator* animator = (Animator*) nativeAnimator;
            const char* val = animator->getAnimationName(static_cast<size_t>(index));
            return val ? env->NewStringUTF(val) : nullptr;

    });
}
