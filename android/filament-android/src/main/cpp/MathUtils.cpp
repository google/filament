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

#include <math/mat3.h>
#include <math/quat.h>

using namespace filament::math;

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_MathUtils_nPackTangentFrame(JNIEnv *env, jclass,
        jfloat tangentX, jfloat tangentY, jfloat tangentZ,
        jfloat bitangentX, jfloat bitangentY, jfloat bitangentZ,
        jfloat normalX, jfloat normalY, jfloat normalZ,
        jfloatArray quaternion_, jint offset) {

    float3 tangent{tangentX, tangentY, tangentZ};
    float3 bitangent{bitangentX, bitangentY, bitangentZ};
    float3 normal{normalX, normalY, normalZ};
    quatf q = mat3f::packTangentFrame({tangent, bitangent, normal});

    env->SetFloatArrayRegion(quaternion_, offset, 4,
             reinterpret_cast<jfloat*>(&q));
}
