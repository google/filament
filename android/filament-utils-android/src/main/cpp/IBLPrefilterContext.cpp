/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <filament-iblprefilter/IBLPrefilterContext.h>

#include <filament/Engine.h>
#include <filament/Texture.h>

using namespace filament;

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_utils_IBLPrefilterContext_nCreate(JNIEnv* env, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) new IBLPrefilterContext(*engine);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_IBLPrefilterContext_nDestroy(JNIEnv*, jclass, jlong native) {
    IBLPrefilterContext* context = (IBLPrefilterContext*) native;
    delete context;
}

extern "C" JNIEXPORT long JNICALL
Java_com_google_android_filament_utils_IBLPrefilterContext_nCreateEquirectHelper(JNIEnv* env, jclass, jlong nativeContext) {
    IBLPrefilterContext* context = (IBLPrefilterContext*) nativeContext;
    return (long) new IBLPrefilterContext::EquirectangularToCubemap(*context);
}

extern "C" JNIEXPORT long JNICALL
Java_com_google_android_filament_utils_IBLPrefilterContext_nEquirectHelperRun(JNIEnv* env, jclass, jlong nativeHelper, long nativeEquirect) {
    auto helper = (IBLPrefilterContext::EquirectangularToCubemap*) nativeHelper;
    auto texture = (filament::Texture*) nativeEquirect;
    auto result = (*helper)(texture);
    return (long) result;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_IBLPrefilterContext_nDestroyEquirectHelper(JNIEnv* env, jclass, jlong nativeObject) {
    delete (IBLPrefilterContext::EquirectangularToCubemap*) nativeObject;
}

extern "C" JNIEXPORT long JNICALL
Java_com_google_android_filament_utils_IBLPrefilterContext_nCreateSpecularFilter(JNIEnv* env, jclass, jlong nativeContext) {
    IBLPrefilterContext* context = (IBLPrefilterContext*) nativeContext;
    return (long) new IBLPrefilterContext::SpecularFilter(*context);
}

extern "C" JNIEXPORT long JNICALL
Java_com_google_android_filament_utils_IBLPrefilterContext_nSpecularFilterRun(JNIEnv* env, jclass, jlong nativeHelper, long nativeSkybox) {
    auto helper = (IBLPrefilterContext::SpecularFilter*) nativeHelper;
    auto texture = (filament::Texture*) nativeSkybox;
    auto result = (*helper)(texture);
    return (long) result;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_IBLPrefilterContext_nDestroySpecularFilter(JNIEnv* env, jclass, jlong nativeObject) {
    delete (IBLPrefilterContext::SpecularFilter*) nativeObject;
}
