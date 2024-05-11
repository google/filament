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

#include <filament/Fence.h>

using namespace filament;

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Fence_nWait(JNIEnv *env, jclass type, jlong nativeFence, jint mode,
        jlong timeoutNanoSeconds) {
    Fence *fence = (Fence *) nativeFence;
    return (jint) fence->wait((Fence::Mode) mode, (uint64_t) timeoutNanoSeconds);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Fence_nWaitAndDestroy(JNIEnv *env, jclass type, jlong nativeFence,
        jint mode) {
    Fence *fence = (Fence *) nativeFence;
    return (jint) Fence::waitAndDestroy(fence, (Fence::Mode) mode);
}

