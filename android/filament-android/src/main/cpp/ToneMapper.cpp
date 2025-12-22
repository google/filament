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

#include <filament/ToneMapper.h>
#include "../../../../common/JniExceptionBridge.h"

using namespace filament;

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ToneMapper_nDestroyToneMapper(JNIEnv *env, jclass clazz,
        jlong toneMapper_) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_ToneMapper_nDestroyToneMapper", [&]() {
            ToneMapper* toneMapper = (ToneMapper*) toneMapper_;
            delete toneMapper;
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_ToneMapper_nCreateLinearToneMapper(JNIEnv* env, jclass) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_ToneMapper_nCreateLinearToneMapper", 0, [&]() -> jlong {
            return (jlong) new LinearToneMapper();
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_ToneMapper_nCreateACESToneMapper(JNIEnv* env, jclass) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_ToneMapper_nCreateACESToneMapper", 0, [&]() -> jlong {
            return (jlong) new ACESToneMapper();
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_ToneMapper_nCreateACESLegacyToneMapper(JNIEnv* env, jclass) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_ToneMapper_nCreateACESLegacyToneMapper", 0, [&]() -> jlong {
            return (jlong) new ACESLegacyToneMapper();
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_ToneMapper_nCreateFilmicToneMapper(JNIEnv* env, jclass) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_ToneMapper_nCreateFilmicToneMapper", 0, [&]() -> jlong {
            return (jlong) new FilmicToneMapper();
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_ToneMapper_nCreatePBRNeutralToneMapper(JNIEnv* env, jclass) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_ToneMapper_nCreatePBRNeutralToneMapper", 0, [&]() -> jlong {
            return (jlong) new PBRNeutralToneMapper();
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_ToneMapper_nCreateAgxToneMapper(JNIEnv* env, jclass, jint look) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_ToneMapper_nCreateAgxToneMapper", 0, [&]() -> jlong {
            return (jlong) new AgxToneMapper(AgxToneMapper::AgxLook(look));
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_ToneMapper_nCreateGenericToneMapper(JNIEnv* env, jclass,
        jfloat contrast, jfloat midGrayIn, jfloat midGrayOut, jfloat hdrMax) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_ToneMapper_nCreateGenericToneMapper", 0, [&]() -> jlong {
            return (jlong) new GenericToneMapper(contrast, midGrayIn, midGrayOut, hdrMax);
    });
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_ToneMapper_nGenericGetContrast(JNIEnv* env, jclass, jlong nativeObject) {
    return filament::android::jniGuard<jfloat>(env, "Java_com_google_android_filament_ToneMapper_nGenericGetContrast", 0.0f, [&]() -> jfloat {
            return ((GenericToneMapper*) nativeObject)->getContrast();
    });
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_ToneMapper_nGenericGetMidGrayIn(JNIEnv* env, jclass, jlong nativeObject) {
    return filament::android::jniGuard<jfloat>(env, "Java_com_google_android_filament_ToneMapper_nGenericGetMidGrayIn", 0.0f, [&]() -> jfloat {
            return ((GenericToneMapper*) nativeObject)->getMidGrayIn();
    });
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_ToneMapper_nGenericGetMidGrayOut(JNIEnv* env, jclass, jlong nativeObject) {
    return filament::android::jniGuard<jfloat>(env, "Java_com_google_android_filament_ToneMapper_nGenericGetMidGrayOut", 0.0f, [&]() -> jfloat {
            return ((GenericToneMapper*) nativeObject)->getMidGrayOut();
    });
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_ToneMapper_nGenericGetHdrMax(JNIEnv* env, jclass, jlong nativeObject) {
    return filament::android::jniGuard<jfloat>(env, "Java_com_google_android_filament_ToneMapper_nGenericGetHdrMax", 0.0f, [&]() -> jfloat {
            return ((GenericToneMapper*) nativeObject)->getHdrMax();
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ToneMapper_nGenericSetContrast(JNIEnv* env, jclass,
        jlong nativeObject, jfloat contrast) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_ToneMapper_nGenericSetContrast", [&]() {
            ((GenericToneMapper*) nativeObject)->setContrast(contrast);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ToneMapper_nGenericSetMidGrayIn(JNIEnv* env, jclass,
        jlong nativeObject, jfloat midGrayIn) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_ToneMapper_nGenericSetMidGrayIn", [&]() {
            ((GenericToneMapper*) nativeObject)->setMidGrayIn(midGrayIn);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ToneMapper_nGenericSetMidGrayOut(JNIEnv* env, jclass,
        jlong nativeObject, jfloat midGrayOut) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_ToneMapper_nGenericSetMidGrayOut", [&]() {
            ((GenericToneMapper*) nativeObject)->setMidGrayOut(midGrayOut);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ToneMapper_nGenericSetHdrMax(JNIEnv* env, jclass,
        jlong nativeObject, jfloat hdrMax) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_ToneMapper_nGenericSetHdrMax", [&]() {
            ((GenericToneMapper*) nativeObject)->setHdrMax(hdrMax);
    });
}
