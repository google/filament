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

#include <filament/MaterialInstance.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

using namespace filament;

template<typename T>
static void setParameter(JNIEnv* env, jlong nativeMaterialInstance, jstring name_, T v) {
    MaterialInstance* instance = (MaterialInstance*) nativeMaterialInstance;
    const char *name = env->GetStringUTFChars(name_, 0);
    instance->setParameter(name, v);
    env->ReleaseStringUTFChars(name_, name);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetParameterBool(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_, jboolean x) {
    setParameter(env, nativeMaterialInstance, name_, bool(x));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetParameterBool2(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_, jboolean x, jboolean y) {
    setParameter(env, nativeMaterialInstance, name_, filament::math::bool2{x, y});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetParameterBool3(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_, jboolean x, jboolean y, jboolean z) {
    setParameter(env, nativeMaterialInstance, name_, filament::math::bool3{x, y, z});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetParameterBool4(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_,
        jboolean x, jboolean y, jboolean z, jboolean w) {
    setParameter(env, nativeMaterialInstance, name_, filament::math::bool4{x, y, z, w});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetParameterInt(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_, jint x) {
    setParameter(env, nativeMaterialInstance, name_, int32_t(x));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetParameterInt2(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_, jint x, jint y) {
    setParameter(env, nativeMaterialInstance, name_, filament::math::int2{x, y});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetParameterInt3(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_, jint x, jint y, jint z) {
    setParameter(env, nativeMaterialInstance, name_, filament::math::int3{x, y, z});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetParameterInt4(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_,
        jint x, jint y, jint z, jint w) {
    setParameter(env, nativeMaterialInstance, name_, filament::math::int4{x, y, z, w});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetParameterFloat(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_, jfloat x) {
    setParameter(env, nativeMaterialInstance, name_, float(x));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetParameterFloat2(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_, jfloat x, jfloat y) {
    setParameter(env, nativeMaterialInstance, name_, filament::math::float2{x, y});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetParameterFloat3(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_, jfloat x, jfloat y, jfloat z) {
    setParameter(env, nativeMaterialInstance, name_, filament::math::float3{x, y, z});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetParameterFloat4(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_,
        jfloat x, jfloat y, jfloat z, jfloat w) {
    setParameter(env, nativeMaterialInstance, name_, filament::math::float4{x, y, z, w});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetBooleanParameterArray(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_, jint element,
        jbooleanArray v_, jint offset, jint count) {
    MaterialInstance* instance = (MaterialInstance*) nativeMaterialInstance;

    const char* name = env->GetStringUTFChars(name_, 0);
    size_t size = (size_t) element + 1;
    jboolean* v = env->GetBooleanArrayElements(v_, NULL);
    instance->setParameter(name, (bool*) (v + offset * size), (size_t) (count * size));
    env->ReleaseBooleanArrayElements(v_, v, 0);

    env->ReleaseStringUTFChars(name_, name);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetIntParameterArray(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_, jint element,
        jintArray v_, jint offset, jint count) {
    MaterialInstance* instance = (MaterialInstance*) nativeMaterialInstance;

    const char* name = env->GetStringUTFChars(name_, 0);
    size_t size = (size_t) element + 1;
    jint* v = env->GetIntArrayElements(v_, NULL);
    instance->setParameter(name, reinterpret_cast<int32_t*>(v + offset * size),
           (size_t) (count * size));
    env->ReleaseIntArrayElements(v_, v, 0);

    env->ReleaseStringUTFChars(name_, name);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetFloatParameterArray(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_, jint element,
        jfloatArray v_, jint offset, jint count) {
    MaterialInstance* instance = (MaterialInstance*) nativeMaterialInstance;

    const char* name = env->GetStringUTFChars(name_, 0);
    size_t size = (size_t) element + 1;
    if (size == 5) {
        // mat3
        size = 9;
    } else if (size == 6) {
        // mat4
        size = 16;
    }
    jfloat* v = env->GetFloatArrayElements(v_, NULL);
    instance->setParameter(name, v + offset * size, (size_t) (count * size));
    env->ReleaseFloatArrayElements(v_, v, 0);

    env->ReleaseStringUTFChars(name_, name);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetParameterTexture(
        JNIEnv *env, jclass, jlong nativeMaterialInstance, jstring name_,
        jlong nativeTexture, jint sampler_) {
    MaterialInstance* instance = (MaterialInstance*) nativeMaterialInstance;
    Texture* texture = (Texture*) nativeTexture;
    TextureSampler& sampler = reinterpret_cast<TextureSampler&>(sampler_);

    const char *name = env->GetStringUTFChars(name_, 0);
    instance->setParameter(name, texture, sampler);
    env->ReleaseStringUTFChars(name_, name);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetScissor(
        JNIEnv *env, jclass, jlong nativeMaterialInstance, jint left,
        jint bottom, jint width, jint height) {
    MaterialInstance* instance = (MaterialInstance*) nativeMaterialInstance;
    instance->setScissor(left, bottom, width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nUnsetScissor(
        JNIEnv *env, jclass, jlong nativeMaterialInstance) {
    MaterialInstance* instance = (MaterialInstance*) nativeMaterialInstance;
    instance->unsetScissor();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetPolygonOffset(JNIEnv*,
        jclass, jlong nativeMaterialInstance, jfloat scale, jfloat constant) {
    MaterialInstance* instance = (MaterialInstance*) nativeMaterialInstance;
    instance->setPolygonOffset(scale, constant);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetMaskThreshold(JNIEnv*,
        jclass, jlong nativeMaterialInstance, jfloat threshold) {
    MaterialInstance* instance = (MaterialInstance*) nativeMaterialInstance;
    instance->setMaskThreshold(threshold);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetDoubleSided(JNIEnv*,
        jclass, jlong nativeMaterialInstance, jboolean doubleSided) {
    MaterialInstance* instance = (MaterialInstance*) nativeMaterialInstance;
    instance->setDoubleSided(doubleSided);
}
