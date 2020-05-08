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
using namespace filament::math;

enum BooleanElement {
    BOOL,
    BOOL2,
    BOOL3,
    BOOL4
};

enum IntElement {
    INT,
    INT2,
    INT3,
    INT4
};

enum FloatElement {
    FLOAT,
    FLOAT2,
    FLOAT3,
    FLOAT4,
    MAT3,
    MAT4
};

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
    setParameter(env, nativeMaterialInstance, name_, bool2{x, y});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetParameterBool3(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_, jboolean x, jboolean y, jboolean z) {
    setParameter(env, nativeMaterialInstance, name_, bool3{x, y, z});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetParameterBool4(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_,
        jboolean x, jboolean y, jboolean z, jboolean w) {
    setParameter(env, nativeMaterialInstance, name_, bool4{x, y, z, w});
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
    setParameter(env, nativeMaterialInstance, name_, int2{x, y});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetParameterInt3(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_, jint x, jint y, jint z) {
    setParameter(env, nativeMaterialInstance, name_, int3{x, y, z});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetParameterInt4(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_,
        jint x, jint y, jint z, jint w) {
    setParameter(env, nativeMaterialInstance, name_, int4{x, y, z, w});
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
    setParameter(env, nativeMaterialInstance, name_, float2{x, y});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetParameterFloat3(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_, jfloat x, jfloat y, jfloat z) {
    setParameter(env, nativeMaterialInstance, name_, float3{x, y, z});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetParameterFloat4(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_,
        jfloat x, jfloat y, jfloat z, jfloat w) {
    setParameter(env, nativeMaterialInstance, name_, float4{x, y, z, w});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetBooleanParameterArray(JNIEnv *env, jclass,
        jlong nativeMaterialInstance, jstring name_, jint element,
        jbooleanArray v_, jint offset, jint count) {
    MaterialInstance* instance = (MaterialInstance*) nativeMaterialInstance;

    const char* name = env->GetStringUTFChars(name_, 0);
    jboolean* v = env->GetBooleanArrayElements(v_, NULL);

    // NOTE: In C++, bool has an implementation-defined size. Here we assume
    // it has the same size as jboolean, which is 1 byte.

    switch ((BooleanElement) element) {
        case BOOL:
            instance->setParameter(name, ((const bool*) v) + offset, count);
            break;
        case BOOL2:
            instance->setParameter(name, ((const bool2*) v) + offset, count);
            break;
        case BOOL3:
            instance->setParameter(name, ((const bool3*) v) + offset, count);
            break;
        case BOOL4:
            instance->setParameter(name, ((const bool4*) v) + offset, count);
            break;
    }

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
    jint* v = env->GetIntArrayElements(v_, NULL);

    switch ((IntElement) element) {
        case INT:
            instance->setParameter(name, ((const int32_t*) v) + offset, count);
            break;
        case INT2:
            instance->setParameter(name, ((const int2*) v) + offset, count);
            break;
        case INT3:
            instance->setParameter(name, ((const int3*) v) + offset, count);
            break;
        case INT4:
            instance->setParameter(name, ((const int4*) v) + offset, count);
            break;
    }

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
    jfloat* v = env->GetFloatArrayElements(v_, NULL);

    switch ((FloatElement) element) {
        case FLOAT:
            instance->setParameter(name, ((const float*) v) + offset, count);
            break;
        case FLOAT2:
            instance->setParameter(name, ((const float2*) v) + offset, count);
            break;
        case FLOAT3:
            instance->setParameter(name, ((const float3*) v) + offset, count);
            break;
        case FLOAT4:
            instance->setParameter(name, ((const float4*) v) + offset, count);
            break;
        case MAT3:
            instance->setParameter(name, ((const mat3f*) v) + offset, count);
            break;
        case MAT4:
            instance->setParameter(name, ((const mat4f*) v) + offset, count);
            break;
    }

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
Java_com_google_android_filament_MaterialInstance_nSetSpecularAntiAliasingVariance(JNIEnv*,
        jclass, jlong nativeMaterialInstance, jfloat variance) {
    MaterialInstance* instance = (MaterialInstance*) nativeMaterialInstance;
    instance->setSpecularAntiAliasingVariance(variance);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetSpecularAntiAliasingThreshold(JNIEnv*,
        jclass, jlong nativeMaterialInstance, jfloat threshold) {
    MaterialInstance* instance = (MaterialInstance*) nativeMaterialInstance;
    instance->setSpecularAntiAliasingThreshold(threshold);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetDoubleSided(JNIEnv*,
        jclass, jlong nativeMaterialInstance, jboolean doubleSided) {
    MaterialInstance* instance = (MaterialInstance*) nativeMaterialInstance;
    instance->setDoubleSided(doubleSided);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetCullingMode(JNIEnv*,
        jclass, jlong nativeMaterialInstance, jlong cullingMode) {
    MaterialInstance* instance = (MaterialInstance*) nativeMaterialInstance;
    instance->setCullingMode((MaterialInstance::CullingMode) cullingMode);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetColorWrite(JNIEnv*,
        jclass, jlong nativeMaterialInstance, jboolean enable) {
    MaterialInstance* instance = (MaterialInstance*) nativeMaterialInstance;
    instance->setColorWrite(enable);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetDepthWrite(JNIEnv*,
        jclass, jlong nativeMaterialInstance, jboolean enable) {
    MaterialInstance* instance = (MaterialInstance*) nativeMaterialInstance;
    instance->setDepthWrite(enable);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MaterialInstance_nSetDepthCulling(JNIEnv*,
        jclass, jlong nativeMaterialInstance, jboolean enable) {
    MaterialInstance* instance = (MaterialInstance*) nativeMaterialInstance;
    instance->setDepthCulling(enable);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_google_android_filament_MaterialInstance_nGetName(JNIEnv* env, jclass,
        jlong nativeMaterialInstance) {
    MaterialInstance* instance = (MaterialInstance*) nativeMaterialInstance;
    return env->NewStringUTF(instance->getName());
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_google_android_filament_MaterialInstance_nGetMaterial(JNIEnv* env, jclass,
        jlong nativeMaterialInstance) {
    MaterialInstance* instance = (MaterialInstance*) nativeMaterialInstance;
    return (jlong) instance->getMaterial();
}
