/*
 * Copyright (C) 2018 The Android Open Source Project
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

extern "C" {
jlong createNativeSurface(jint width, jint height);
void destroyNativeSurface(jlong surface);
};

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_NativeSurface_nCreateSurface(JNIEnv*, jclass,
        jint width, jint height) {
    return createNativeSurface(width, height);
}


extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_NativeSurface_nDestroySurface(JNIEnv*, jclass, jlong surface){
    destroyNativeSurface(surface);
}