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

#include <filament/Material.h>

#include "common/NioUtils.h"

using namespace filament;

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Material_nBuilderBuild(JNIEnv *env, jclass,
        jlong nativeEngine, jobject buffer_, jint size) {
    Engine* engine = (Engine*) nativeEngine;
    AutoBuffer buffer(env, buffer_, size);
    Material* material = Material::Builder()
            .package(buffer.getData(), buffer.getSize())
            .build(*engine);
    return (jlong) material;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Material_nGetDefaultInstance(JNIEnv*, jclass,
        jlong nativeMaterial) {
    Material const* material = (Material const*) nativeMaterial;
    return (jlong) material->getDefaultInstance();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Material_nCreateInstance(JNIEnv*, jclass,
        jlong nativeMaterial) {
    Material* material = (Material*) nativeMaterial;
    return (jlong) material->createInstance();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Material_nCreateInstanceWithName(JNIEnv* env, jclass,
        jlong nativeMaterial, jstring name_) {
    Material* material = (Material*) nativeMaterial;
    const char *name = env->GetStringUTFChars(name_, 0);
    jlong instance = (jlong) material->createInstance(name);
    env->ReleaseStringUTFChars(name_, name);
    return instance;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_google_android_filament_Material_nGetName(JNIEnv* env, jclass, jlong nativeMaterial) {
    Material* material = (Material*) nativeMaterial;
    return env->NewStringUTF(material->getName());
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_google_android_filament_Material_nGetShading(JNIEnv*, jclass,
        jlong nativeMaterial) {
    Material* material = (Material*) nativeMaterial;
    return (jint) material->getShading();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_google_android_filament_Material_nGetInterpolation(JNIEnv*, jclass,
        jlong nativeMaterial) {
    Material* material = (Material*) nativeMaterial;
    return (jint) material->getInterpolation();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_google_android_filament_Material_nGetBlendingMode(JNIEnv*, jclass,
        jlong nativeMaterial) {
    Material* material = (Material*) nativeMaterial;
    return (jint) material->getBlendingMode();
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_google_android_filament_Material_nGetRefraction(JNIEnv*, jclass,
        jlong nativeMaterial) {
    Material* material = (Material*) nativeMaterial;
    return (jint)material->getRefractionMode();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_google_android_filament_Material_nGetRefractionType(JNIEnv*, jclass,
        jlong nativeMaterial) {
    Material* material = (Material*) nativeMaterial;
    return (jint) material->getRefractionType();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_google_android_filament_Material_nGetVertexDomain(JNIEnv*, jclass,
        jlong nativeMaterial) {
    Material* material = (Material*) nativeMaterial;
    return (jint) material->getVertexDomain();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_google_android_filament_Material_nGetCullingMode(JNIEnv*, jclass,
        jlong nativeMaterial) {
    Material* material = (Material*) nativeMaterial;
    return (jint) material->getCullingMode();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Material_nIsColorWriteEnabled(JNIEnv*, jclass,
        jlong nativeMaterial) {
    Material* material = (Material*) nativeMaterial;
    return (jboolean) material->isColorWriteEnabled();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Material_nIsDepthWriteEnabled(JNIEnv*, jclass,
        jlong nativeMaterial) {
    Material* material = (Material*) nativeMaterial;
    return (jboolean) material->isDepthWriteEnabled();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Material_nIsDepthCullingEnabled(JNIEnv*, jclass,
        jlong nativeMaterial) {
    Material* material = (Material*) nativeMaterial;
    return (jboolean) material->isDepthCullingEnabled();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Material_nIsDoubleSided(JNIEnv*, jclass,
        jlong nativeMaterial) {
    Material* material = (Material*) nativeMaterial;
    return (jboolean) material->isDoubleSided();
}

extern "C"
JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_Material_nGetMaskThreshold(JNIEnv*, jclass,
        jlong nativeMaterial) {
    Material* material = (Material*) nativeMaterial;
    return material->getMaskThreshold();
}

extern "C"
JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_Material_nGetSpecularAntiAliasingVariance(JNIEnv*, jclass,
        jlong nativeMaterial) {
    Material* material = (Material*) nativeMaterial;
    return material->getSpecularAntiAliasingVariance();
}

extern "C"
JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_Material_nGetSpecularAntiAliasingThreshold(JNIEnv*, jclass,
        jlong nativeMaterial) {
    Material* material = (Material*) nativeMaterial;
    return material->getSpecularAntiAliasingThreshold();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_google_android_filament_Material_nGetParameterCount(JNIEnv*, jclass,
        jlong nativeMaterial) {
    Material* material = (Material*) nativeMaterial;
    return (jint) material->getParameterCount();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_Material_nGetParameters(JNIEnv* env, jclass,
        jlong nativeMaterial, jobject parameters, jint count) {
    Material* material = (Material*) nativeMaterial;

    Material::ParameterInfo* info = new Material::ParameterInfo[count];
    size_t received = material->getParameters(info, (size_t) count);
    assert(received == count);

    jclass parameterClass = env->FindClass("com/google/android/filament/Material$Parameter");
    parameterClass = (jclass) env->NewLocalRef(parameterClass);

    jmethodID parameterAdd = env->GetStaticMethodID(parameterClass, "add",
            "(Ljava/util/List;Ljava/lang/String;III)V");

    jfieldID parameterSamplerOffset = env->GetStaticFieldID(parameterClass,
            "SAMPLER_OFFSET", "I");

    jint offset = env->GetStaticIntField(parameterClass, parameterSamplerOffset);
    for (size_t i = 0; i < received; i++) {
        jint type = info[i].isSampler ? (jint) info[i].samplerType + offset : (jint) info[i].type;

        env->CallStaticVoidMethod(
                parameterClass, parameterAdd,
                parameters, env->NewStringUTF(info[i].name), type, (jint) info[i].precision,
                (jint) info[i].count);
    }

    env->DeleteLocalRef(parameterClass);

    delete[] info;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_google_android_filament_Material_nGetRequiredAttributes(JNIEnv*, jclass,
        jlong nativeMaterial) {
    Material* material = (Material*) nativeMaterial;
    return material->getRequiredAttributes().getValue();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Material_nHasParameter(JNIEnv* env, jclass,
        jlong nativeMaterial, jstring name_) {
    Material* material = (Material*) nativeMaterial;
    const char* name = env->GetStringUTFChars(name_, 0);
    bool hasParameter = material->hasParameter(name);
    env->ReleaseStringUTFChars(name_, name);
    return (jboolean) hasParameter;
}
