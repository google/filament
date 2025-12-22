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

#include <filament/Engine.h>
#include <filament/SwapChain.h>

#include "common/CallbackUtils.h"
#include "../../../../common/JniExceptionBridge.h"

using namespace filament;

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_SwapChain_nSetFrameCompletedCallback(JNIEnv* env, jclass,
        jlong nativeSwapChain, jobject handler, jobject runnable) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_SwapChain_nSetFrameCompletedCallback", [&]() {
            SwapChain* swapChain = (SwapChain*) nativeSwapChain;
            auto* callback = JniCallback::make(env, handler, runnable);
            swapChain->setFrameCompletedCallback(callback->getHandler(),
                    [callback](SwapChain* swapChain) { JniCallback::postToJavaAndDestroy(callback); });
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_SwapChain_nIsSRGBSwapChainSupported(
        JNIEnv * env, jclass, jlong nativeEngine) {
    return filament::android::jniGuard<jboolean>(env, "Java_com_google_android_filament_SwapChain_nIsSRGBSwapChainSupported", JNI_FALSE, [&]() -> jboolean {
            Engine* engine = (Engine*) nativeEngine;
            return (jboolean)SwapChain::isSRGBSwapChainSupported(*engine);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_SwapChain_nIsMSAASwapChainSupported(
        JNIEnv * env, jclass, jlong nativeEngine, jint samples) {
    return filament::android::jniGuard<jboolean>(env, "Java_com_google_android_filament_SwapChain_nIsMSAASwapChainSupported", JNI_FALSE, [&]() -> jboolean {
            Engine* engine = (Engine*) nativeEngine;
            return (jboolean)SwapChain::isMSAASwapChainSupported(*engine, samples);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_SwapChain_nIsProtectedContentSupported(
        JNIEnv * env, jclass, jlong nativeEngine) {
    return filament::android::jniGuard<jboolean>(env, "Java_com_google_android_filament_SwapChain_nIsProtectedContentSupported", JNI_FALSE, [&]() -> jboolean {
            Engine* engine = (Engine*) nativeEngine;
            return (jboolean)SwapChain::isProtectedContentSupported(*engine);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_SwapChain_nSetFrameScheduledCallback(JNIEnv* env, jclass,
        jlong nativeSwapChain, jobject handler, jobject runnable) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_SwapChain_nSetFrameScheduledCallback", [&]() {
            SwapChain* swapChain = (SwapChain*) nativeSwapChain;
            auto* callback = JniCallback::make(env, handler, runnable);
            swapChain->setFrameScheduledCallback(callback->getHandler(),
                    [callback](backend::PresentCallable) {
                        // Ignore PresentCallable, which is only meaningful with the Metal backend.
                        JniCallback::postToJavaAndDestroy(callback);
                    });
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_SwapChain_nIsFrameScheduledCallbackSet(
        JNIEnv * env, jclass, jlong nativeSwapChain) {
    return filament::android::jniGuard<jboolean>(env, "Java_com_google_android_filament_SwapChain_nIsFrameScheduledCallbackSet", JNI_FALSE, [&]() -> jboolean {
            SwapChain* swapChain = (SwapChain*) nativeSwapChain;
            return (jboolean)swapChain->isFrameScheduledCallbackSet();
    });
}
