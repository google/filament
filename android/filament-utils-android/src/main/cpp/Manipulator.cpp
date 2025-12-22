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

#include <math/mat3.h>
#include <math/vec3.h>

#include <camutils/Manipulator.h>
#include "../../../../common/JniExceptionBridge.h"

using namespace filament::camutils;
using namespace filament::math;

using Builder = Manipulator<float>::Builder;

extern "C" JNIEXPORT jlong Java_com_google_android_filament_utils_Manipulator_nCreateBuilder(JNIEnv* env, jclass) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_utils_Manipulator_nCreateBuilder", 0, [&]() -> jlong {
            return (jlong) new Builder {};
    });
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nDestroyBuilder(JNIEnv* env, jclass, jlong nativeBuilder) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nDestroyBuilder", [&]() {
            Builder* builder = (Builder*) nativeBuilder;
            delete builder;
    });
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderViewport(JNIEnv* env, jclass, jlong nativeBuilder, jint width, jint height) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nBuilderViewport", [&]() {
            Builder* builder = (Builder*) nativeBuilder;
            builder->viewport(width, height);
    });
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderTargetPosition(JNIEnv* env, jclass, jlong nativeBuilder, jfloat x, jfloat y, jfloat z) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nBuilderTargetPosition", [&]() {
            Builder* builder = (Builder*) nativeBuilder;
            builder->targetPosition(x, y, z);
    });
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderUpVector(JNIEnv* env, jclass, jlong nativeBuilder, jfloat x, jfloat y, jfloat z) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nBuilderUpVector", [&]() {
            Builder* builder = (Builder*) nativeBuilder;
            builder->upVector(x, y, z);
    });
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderZoomSpeed(JNIEnv* env, jclass, jlong nativeBuilder, jfloat arg) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nBuilderZoomSpeed", [&]() {
            Builder* builder = (Builder*) nativeBuilder;
            builder->zoomSpeed(arg);
    });
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderOrbitHomePosition(JNIEnv* env, jclass, jlong nativeBuilder, jfloat x, jfloat y, jfloat z) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nBuilderOrbitHomePosition", [&]() {
            Builder* builder = (Builder*) nativeBuilder;
            builder->orbitHomePosition(x, y, z);
    });
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderOrbitSpeed(JNIEnv* env, jclass, jlong nativeBuilder, jfloat x, jfloat y) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nBuilderOrbitSpeed", [&]() {
            Builder* builder = (Builder*) nativeBuilder;
            builder->orbitSpeed(x, y);
    });
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderFovDirection(JNIEnv* env, jclass, jlong nativeBuilder, jint arg) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nBuilderFovDirection", [&]() {
            Builder* builder = (Builder*) nativeBuilder;
            builder->fovDirection((Fov) arg);
    });
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderFovDegrees(JNIEnv* env, jclass, jlong nativeBuilder, jfloat arg) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nBuilderFovDegrees", [&]() {
            Builder* builder = (Builder*) nativeBuilder;
            builder->fovDegrees(arg);
    });
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderFarPlane(JNIEnv* env, jclass, jlong nativeBuilder, jfloat distance) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nBuilderFarPlane", [&]() {
            Builder* builder = (Builder*) nativeBuilder;
            builder->farPlane(distance);
    });
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderMapExtent(JNIEnv* env, jclass, jlong nativeBuilder, jfloat width, jfloat height) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nBuilderMapExtent", [&]() {
            Builder* builder = (Builder*) nativeBuilder;
            builder->mapExtent(width, height);
    });
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderMapMinDistance(JNIEnv* env, jclass, jlong nativeBuilder, jfloat arg) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nBuilderMapMinDistance", [&]() {
            Builder* builder = (Builder*) nativeBuilder;
            builder->mapMinDistance(arg);
    });
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderFlightStartPosition(JNIEnv* env, jclass, jlong nativeBuilder, jfloat x, jfloat y, jfloat z) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nBuilderFlightStartPosition", [&]() {
            Builder* builder = (Builder*) nativeBuilder;
            builder->flightStartPosition(x, y, z);
    });
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderFlightStartOrientation(JNIEnv* env, jclass, jlong nativeBuilder, jfloat pitch, jfloat yaw) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nBuilderFlightStartOrientation", [&]() {
            Builder* builder = (Builder*) nativeBuilder;
            builder->flightStartOrientation(pitch, yaw);
    });
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderFlightMaxMoveSpeed(JNIEnv* env, jclass, jlong nativeBuilder, jfloat maxSpeed) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nBuilderFlightMaxMoveSpeed", [&]() {
            Builder* builder = (Builder*) nativeBuilder;
            builder->flightMaxMoveSpeed(maxSpeed);
    });
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderFlightSpeedSteps(JNIEnv* env, jclass, jlong nativeBuilder, jint steps) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nBuilderFlightSpeedSteps", [&]() {
            Builder* builder = (Builder*) nativeBuilder;
            builder->flightSpeedSteps(steps);
    });
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderFlightPanSpeed(JNIEnv* env, jclass, jlong nativeBuilder, jfloat x, jfloat y) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nBuilderFlightPanSpeed", [&]() {
            Builder* builder = (Builder*) nativeBuilder;
            builder->flightPanSpeed(x, y);
    });
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderFlightMoveDamping(JNIEnv* env, jclass, jlong nativeBuilder, jfloat damping) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nBuilderFlightMoveDamping", [&]() {
            Builder* builder = (Builder*) nativeBuilder;
            builder->flightMoveDamping(damping);
    });
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderGroundPlane(JNIEnv* env, jclass, jlong nativeBuilder, jfloat a, jfloat b, jfloat c, jfloat d) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nBuilderGroundPlane", [&]() {
            Builder* builder = (Builder*) nativeBuilder;
            builder->groundPlane(a, b, c, d);
    });
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderPanning(JNIEnv* env, jclass, jlong nativeBuilder, jboolean enabled) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nBuilderPanning", [&]() {
            Builder* builder = (Builder*) nativeBuilder;
            builder->panning(enabled);
    });
}

extern "C" JNIEXPORT long Java_com_google_android_filament_utils_Manipulator_nBuilderBuild(JNIEnv* env, jclass, jlong nativeBuilder, jint mode) {
    return filament::android::jniGuard<long>(env, "Java_com_google_android_filament_utils_Manipulator_nBuilderBuild", 0, [&]() -> long {
            Builder* builder = (Builder*) nativeBuilder;
            return (jlong) builder->build((Mode) mode);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_Manipulator_nDestroyManipulator(JNIEnv* env, jclass, jlong nativeManip) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nDestroyManipulator", [&]() {
            auto manip = (Manipulator<float>*) nativeManip;
            delete manip;
    });
}

extern "C" JNIEXPORT jint JNICALL Java_com_google_android_filament_utils_Manipulator_nGetMode(JNIEnv* env, jclass, jlong nativeManip) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_utils_Manipulator_nGetMode", 0, [&]() -> jint {
            auto manip = (Manipulator<float>*) nativeManip;
            return (int) manip->getMode();
    });
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nSetViewport(JNIEnv* env, jclass, jlong nativeManip, jint width, jint height) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nSetViewport", [&]() {
            auto manip = (Manipulator<float>*) nativeManip;
            manip->setViewport(width, height);
    });
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nGetLookAtFloat(JNIEnv* env, jclass, jlong nativeManip, jfloatArray eyePosition, jfloatArray targetPosition, jfloatArray upward) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nGetLookAtFloat", [&]() {
            auto manip = (Manipulator<float>*) nativeManip;

            jfloat *eye = env->GetFloatArrayElements(eyePosition, NULL);
            jfloat *target = env->GetFloatArrayElements(targetPosition, NULL);
            jfloat *up = env->GetFloatArrayElements(upward, NULL);

            manip->getLookAt((float3*) eye, (float3*) target, (float3*) up);

            env->ReleaseFloatArrayElements(eyePosition, eye, 0);
            env->ReleaseFloatArrayElements(targetPosition, target, 0);
            env->ReleaseFloatArrayElements(upward, up, 0);
    });
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nGetLookAtDouble(JNIEnv* env, jclass, jlong nativeManip, jdoubleArray eyePosition, jdoubleArray targetPosition, jdoubleArray upward) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nGetLookAtDouble", [&]() {
            auto manip = (Manipulator<float>*) nativeManip;
            float3 eyef, targetf, upf;
            manip->getLookAt(&eyef, &targetf, &upf);

            jdouble *eye = env->GetDoubleArrayElements(eyePosition, NULL);
            jdouble *target = env->GetDoubleArrayElements(targetPosition, NULL);
            jdouble *up = env->GetDoubleArrayElements(upward, NULL);

            *((double3*) eye) = eyef;
            *((double3*) target) = targetf;
            *((double3*) up) = upf;

            env->ReleaseDoubleArrayElements(eyePosition, eye, 0);
            env->ReleaseDoubleArrayElements(targetPosition, target, 0);
            env->ReleaseDoubleArrayElements(upward, up, 0);
    });
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nRaycast(JNIEnv* env, jclass, jlong nativeManip, jint x, jint y, jfloatArray result) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nRaycast", [&]() {
            auto manip = (Manipulator<float>*) nativeManip;
            jfloat *presult = env->GetFloatArrayElements(result, NULL);
            manip->raycast(x, y, (float3*) presult);
            env->ReleaseFloatArrayElements(result, presult, 0);
    });
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nGrabBegin(JNIEnv* env, jclass, jlong nativeManip, jint x, jint y, jboolean strafe) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nGrabBegin", [&]() {
            auto manip = (Manipulator<float>*) nativeManip;
            manip->grabBegin(x, y, (bool) strafe);
    });
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nGrabUpdate(JNIEnv* env, jclass, jlong nativeManip, jint x, jint y) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nGrabUpdate", [&]() {
            auto manip = (Manipulator<float>*) nativeManip;
            manip->grabUpdate(x, y);
    });
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nGrabEnd(JNIEnv* env, jclass, jlong nativeManip) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nGrabEnd", [&]() {
            auto manip = (Manipulator<float>*) nativeManip;
            manip->grabEnd();
    });
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nKeyDown(JNIEnv* env, jclass, jlong nativeManip, jint key) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nKeyDown", [&]() {
            auto manip = (Manipulator<float>*) nativeManip;
            manip->keyDown((Manipulator<float>::Key) key);
    });
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nKeyUp(JNIEnv* env, jclass, jlong nativeManip, jint key) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nKeyUp", [&]() {
            auto manip = (Manipulator<float>*) nativeManip;
            manip->keyUp((Manipulator<float>::Key) key);
    });
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nScroll(JNIEnv* env, jclass, jlong nativeManip, jint x, jint y, jfloat scrolldelta) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nScroll", [&]() {
            auto manip = (Manipulator<float>*) nativeManip;
            manip->scroll(x, y, scrolldelta);
    });
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nUpdate(JNIEnv* env, jclass, jlong nativeManip, jfloat deltaTime) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nUpdate", [&]() {
            auto manip = (Manipulator<float>*) nativeManip;
            manip->update(deltaTime);
    });
}

extern "C" JNIEXPORT jlong JNICALL Java_com_google_android_filament_utils_Manipulator_nGetCurrentBookmark(JNIEnv* env, jclass, jlong nativeManip) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_utils_Manipulator_nGetCurrentBookmark", 0, [&]() -> jlong {
            auto manip = (Manipulator<float>*) nativeManip;
            return (jlong) new Bookmark<float>(manip->getCurrentBookmark());
    });
}

extern "C" JNIEXPORT jlong JNICALL Java_com_google_android_filament_utils_Manipulator_nGetHomeBookmark(JNIEnv* env, jclass, jlong nativeManip) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_utils_Manipulator_nGetHomeBookmark", 0, [&]() -> jlong {
            auto manip = (Manipulator<float>*) nativeManip;
            return (jlong) new Bookmark<float>(manip->getHomeBookmark());
    });
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nJumpToBookmark(JNIEnv* env, jclass, jlong nativeManip, jlong nativeBookmark) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_utils_Manipulator_nJumpToBookmark", [&]() {
            auto manip = (Manipulator<float>*) nativeManip;
            auto bookmark = (Bookmark<float>*) nativeBookmark;
            manip->jumpToBookmark(*bookmark);
    });
}

