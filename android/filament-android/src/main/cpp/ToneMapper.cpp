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

using namespace filament;

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ToneMapper_nDestroyToneMapper(JNIEnv *env, jclass clazz,
        jlong toneMapper_) {
    ToneMapper* toneMapper = (ToneMapper*) toneMapper_;
    delete toneMapper;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_ToneMapper_nCreateLinearToneMapper(JNIEnv*, jclass) {
    return (jlong) new LinearToneMapper();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_ToneMapper_nCreateACESToneMapper(JNIEnv*, jclass) {
    return (jlong) new ACESToneMapper();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_ToneMapper_nCreateACESLegacyToneMapper(JNIEnv*, jclass) {
    return (jlong) new ACESLegacyToneMapper();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_ToneMapper_nCreateFilmicToneMapper(JNIEnv*, jclass) {
    return (jlong) new FilmicToneMapper();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_ToneMapper_nCreatePBRNeutralToneMapper(JNIEnv*, jclass) {
    return (jlong) new PBRNeutralToneMapper();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_ToneMapper_nCreateAgxToneMapper(JNIEnv*, jclass, jint look) {
    return (jlong) new AgxToneMapper(AgxToneMapper::AgxLook(look));
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_ToneMapper_nCreateGenericToneMapper(JNIEnv*, jclass,
        jfloat contrast, jfloat midGrayIn, jfloat midGrayOut, jfloat hdrMax) {
    return (jlong) new GenericToneMapper(contrast, midGrayIn, midGrayOut, hdrMax);
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_ToneMapper_nGenericGetContrast(JNIEnv*, jclass, jlong nativeObject) {
    return ((GenericToneMapper*) nativeObject)->getContrast();
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_ToneMapper_nGenericGetMidGrayIn(JNIEnv*, jclass, jlong nativeObject) {
    return ((GenericToneMapper*) nativeObject)->getMidGrayIn();
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_ToneMapper_nGenericGetMidGrayOut(JNIEnv*, jclass, jlong nativeObject) {
    return ((GenericToneMapper*) nativeObject)->getMidGrayOut();
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_ToneMapper_nGenericGetHdrMax(JNIEnv*, jclass, jlong nativeObject) {
    return ((GenericToneMapper*) nativeObject)->getHdrMax();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ToneMapper_nGenericSetContrast(JNIEnv*, jclass,
        jlong nativeObject, jfloat contrast) {
    ((GenericToneMapper*) nativeObject)->setContrast(contrast);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ToneMapper_nGenericSetMidGrayIn(JNIEnv*, jclass,
        jlong nativeObject, jfloat midGrayIn) {
    ((GenericToneMapper*) nativeObject)->setMidGrayIn(midGrayIn);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ToneMapper_nGenericSetMidGrayOut(JNIEnv*, jclass,
        jlong nativeObject, jfloat midGrayOut) {
    ((GenericToneMapper*) nativeObject)->setMidGrayOut(midGrayOut);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ToneMapper_nGenericSetHdrMax(JNIEnv*, jclass,
        jlong nativeObject, jfloat hdrMax) {
    ((GenericToneMapper*) nativeObject)->setHdrMax(hdrMax);
}
