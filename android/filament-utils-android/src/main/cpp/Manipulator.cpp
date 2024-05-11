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

using namespace filament::camutils;
using namespace filament::math;

using Builder = Manipulator<float>::Builder;

extern "C" JNIEXPORT jlong Java_com_google_android_filament_utils_Manipulator_nCreateBuilder(JNIEnv*, jclass) {
    return (jlong) new Builder {};
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nDestroyBuilder(JNIEnv*, jclass, jlong nativeBuilder) {
    Builder* builder = (Builder*) nativeBuilder;
    delete builder;
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderViewport(JNIEnv*, jclass, jlong nativeBuilder, jint width, jint height) {
    Builder* builder = (Builder*) nativeBuilder;
    builder->viewport(width, height);
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderTargetPosition(JNIEnv*, jclass, jlong nativeBuilder, jfloat x, jfloat y, jfloat z) {
    Builder* builder = (Builder*) nativeBuilder;
    builder->targetPosition(x, y, z);
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderUpVector(JNIEnv*, jclass, jlong nativeBuilder, jfloat x, jfloat y, jfloat z) {
    Builder* builder = (Builder*) nativeBuilder;
    builder->upVector(x, y, z);
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderZoomSpeed(JNIEnv*, jclass, jlong nativeBuilder, jfloat arg) {
    Builder* builder = (Builder*) nativeBuilder;
    builder->zoomSpeed(arg);
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderOrbitHomePosition(JNIEnv*, jclass, jlong nativeBuilder, jfloat x, jfloat y, jfloat z) {
    Builder* builder = (Builder*) nativeBuilder;
    builder->orbitHomePosition(x, y, z);
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderOrbitSpeed(JNIEnv*, jclass, jlong nativeBuilder, jfloat x, jfloat y) {
    Builder* builder = (Builder*) nativeBuilder;
    builder->orbitSpeed(x, y);
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderFovDirection(JNIEnv*, jclass, jlong nativeBuilder, jint arg) {
    Builder* builder = (Builder*) nativeBuilder;
    builder->fovDirection((Fov) arg);
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderFovDegrees(JNIEnv*, jclass, jlong nativeBuilder, jfloat arg) {
    Builder* builder = (Builder*) nativeBuilder;
    builder->fovDegrees(arg);
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderFarPlane(JNIEnv*, jclass, jlong nativeBuilder, jfloat distance) {
    Builder* builder = (Builder*) nativeBuilder;
    builder->farPlane(distance);
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderMapExtent(JNIEnv*, jclass, jlong nativeBuilder, jfloat width, jfloat height) {
    Builder* builder = (Builder*) nativeBuilder;
    builder->mapExtent(width, height);
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderMapMinDistance(JNIEnv*, jclass, jlong nativeBuilder, jfloat arg) {
    Builder* builder = (Builder*) nativeBuilder;
    builder->mapMinDistance(arg);
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderFlightStartPosition(JNIEnv*, jclass, jlong nativeBuilder, jfloat x, jfloat y, jfloat z) {
    Builder* builder = (Builder*) nativeBuilder;
    builder->flightStartPosition(x, y, z);
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderFlightStartOrientation(JNIEnv*, jclass, jlong nativeBuilder, jfloat pitch, jfloat yaw) {
    Builder* builder = (Builder*) nativeBuilder;
    builder->flightStartOrientation(pitch, yaw);
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderFlightMaxMoveSpeed(JNIEnv*, jclass, jlong nativeBuilder, jfloat maxSpeed) {
    Builder* builder = (Builder*) nativeBuilder;
    builder->flightMaxMoveSpeed(maxSpeed);
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderFlightSpeedSteps(JNIEnv*, jclass, jlong nativeBuilder, jint steps) {
    Builder* builder = (Builder*) nativeBuilder;
    builder->flightSpeedSteps(steps);
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderFlightPanSpeed(JNIEnv*, jclass, jlong nativeBuilder, jfloat x, jfloat y) {
    Builder* builder = (Builder*) nativeBuilder;
    builder->flightPanSpeed(x, y);
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderFlightMoveDamping(JNIEnv*, jclass, jlong nativeBuilder, jfloat damping) {
    Builder* builder = (Builder*) nativeBuilder;
    builder->flightMoveDamping(damping);
}

extern "C" JNIEXPORT void Java_com_google_android_filament_utils_Manipulator_nBuilderGroundPlane(JNIEnv*, jclass, jlong nativeBuilder, jfloat a, jfloat b, jfloat c, jfloat d) {
    Builder* builder = (Builder*) nativeBuilder;
    builder->groundPlane(a, b, c, d);
}

extern "C" JNIEXPORT long Java_com_google_android_filament_utils_Manipulator_nBuilderBuild(JNIEnv*, jclass, jlong nativeBuilder, jint mode) {
    Builder* builder = (Builder*) nativeBuilder;
    return (jlong) builder->build((Mode) mode);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_Manipulator_nDestroyManipulator(JNIEnv*, jclass, jlong nativeManip) {
    auto manip = (Manipulator<float>*) nativeManip;
    delete manip;
}

extern "C" JNIEXPORT jint JNICALL Java_com_google_android_filament_utils_Manipulator_nGetMode(JNIEnv*, jclass, jlong nativeManip) {
    auto manip = (Manipulator<float>*) nativeManip;
    return (int) manip->getMode();
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nSetViewport(JNIEnv*, jclass, jlong nativeManip, jint width, jint height) {
    auto manip = (Manipulator<float>*) nativeManip;
    manip->setViewport(width, height);
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nGetLookAtFloat(JNIEnv* env, jclass, jlong nativeManip, jfloatArray eyePosition, jfloatArray targetPosition, jfloatArray upward) {
    auto manip = (Manipulator<float>*) nativeManip;

    jfloat *eye = env->GetFloatArrayElements(eyePosition, NULL);
    jfloat *target = env->GetFloatArrayElements(targetPosition, NULL);
    jfloat *up = env->GetFloatArrayElements(upward, NULL);

    manip->getLookAt((float3*) eye, (float3*) target, (float3*) up);

    env->ReleaseFloatArrayElements(eyePosition, eye, 0);
    env->ReleaseFloatArrayElements(targetPosition, target, 0);
    env->ReleaseFloatArrayElements(upward, up, 0);
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nGetLookAtDouble(JNIEnv* env, jclass, jlong nativeManip, jdoubleArray eyePosition, jdoubleArray targetPosition, jdoubleArray upward) {
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
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nRaycast(JNIEnv* env, jclass, jlong nativeManip, jint x, jint y, jfloatArray result) {
    auto manip = (Manipulator<float>*) nativeManip;
    jfloat *presult = env->GetFloatArrayElements(result, NULL);
    manip->raycast(x, y, (float3*) presult);
    env->ReleaseFloatArrayElements(result, presult, 0);
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nGrabBegin(JNIEnv*, jclass, jlong nativeManip, jint x, jint y, jboolean strafe) {
    auto manip = (Manipulator<float>*) nativeManip;
    manip->grabBegin(x, y, (bool) strafe);
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nGrabUpdate(JNIEnv*, jclass, jlong nativeManip, jint x, jint y) {
    auto manip = (Manipulator<float>*) nativeManip;
    manip->grabUpdate(x, y);
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nGrabEnd(JNIEnv*, jclass, jlong nativeManip) {
    auto manip = (Manipulator<float>*) nativeManip;
    manip->grabEnd();
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nKeyDown(JNIEnv*, jclass, jlong nativeManip, jint key) {
    auto manip = (Manipulator<float>*) nativeManip;
    manip->keyDown((Manipulator<float>::Key) key);
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nKeyUp(JNIEnv*, jclass, jlong nativeManip, jint key) {
    auto manip = (Manipulator<float>*) nativeManip;
    manip->keyUp((Manipulator<float>::Key) key);
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nScroll(JNIEnv*, jclass, jlong nativeManip, jint x, jint y, jfloat scrolldelta) {
    auto manip = (Manipulator<float>*) nativeManip;
    manip->scroll(x, y, scrolldelta);
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nUpdate(JNIEnv*, jclass, jlong nativeManip, jfloat deltaTime) {
    auto manip = (Manipulator<float>*) nativeManip;
    manip->update(deltaTime);
}

extern "C" JNIEXPORT jlong JNICALL Java_com_google_android_filament_utils_Manipulator_nGetCurrentBookmark(JNIEnv*, jclass, jlong nativeManip) {
    auto manip = (Manipulator<float>*) nativeManip;
    return (jlong) new Bookmark<float>(manip->getCurrentBookmark());
}

extern "C" JNIEXPORT jlong JNICALL Java_com_google_android_filament_utils_Manipulator_nGetHomeBookmark(JNIEnv*, jclass, jlong nativeManip) {
    auto manip = (Manipulator<float>*) nativeManip;
    return (jlong) new Bookmark<float>(manip->getHomeBookmark());
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_utils_Manipulator_nJumpToBookmark(JNIEnv*, jclass, jlong nativeManip, jlong nativeBookmark) {
    auto manip = (Manipulator<float>*) nativeManip;
    auto bookmark = (Bookmark<float>*) nativeBookmark;
    manip->jumpToBookmark(*bookmark);
}

