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

#include <filament/Camera.h>

#include <utils/Entity.h>

#include <math/mat4.h>

using namespace filament;

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nSetProjection(JNIEnv*, jclass, jlong nativeCamera,
        jint projection, jdouble left, jdouble right, jdouble bottom, jdouble top, jdouble near,
        jdouble far) {
    Camera *camera = (Camera *) nativeCamera;
    camera->setProjection((Camera::Projection) projection, left, right, bottom, top, near, far);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nSetProjectionFov(JNIEnv*, jclass ,
        jlong nativeCamera, jdouble fovInDegrees, jdouble aspect, jdouble near, jdouble far,
        jint fov) {
    Camera *camera = (Camera *) nativeCamera;
    camera->setProjection(fovInDegrees, aspect, near, far, (Camera::Fov) fov);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nSetLensProjection(JNIEnv*, jclass,
        jlong nativeCamera, jdouble focalLength, jdouble aspect, jdouble near, jdouble far) {
    Camera *camera = (Camera *) nativeCamera;
    camera->setLensProjection(focalLength, aspect, near, far);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nSetCustomProjection(JNIEnv *env, jclass,
        jlong nativeCamera, jdoubleArray inProjection_, jdoubleArray inProjectionForCulling_,
        jdouble near, jdouble far) {
    Camera *camera = (Camera *) nativeCamera;
    jdouble *inProjection = env->GetDoubleArrayElements(inProjection_, NULL);
    jdouble *inProjectionForCulling = env->GetDoubleArrayElements(inProjectionForCulling_, NULL);
    camera->setCustomProjection(
            *reinterpret_cast<const filament::math::mat4 *>(inProjection),
            *reinterpret_cast<const filament::math::mat4 *>(inProjectionForCulling),
            near, far);
    env->ReleaseDoubleArrayElements(inProjection_, inProjection, JNI_ABORT);
    env->ReleaseDoubleArrayElements(inProjectionForCulling_, inProjectionForCulling, JNI_ABORT);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nSetScaling(JNIEnv* env, jclass,
        jlong nativeCamera, jdouble x, jdouble y) {
    Camera *camera = (Camera *) nativeCamera;
    camera->setScaling({(double)x, (double)y});
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nSetShift(JNIEnv* env, jclass,
        jlong nativeCamera, jdouble x, jdouble y) {
    Camera *camera = (Camera *) nativeCamera;
    camera->setShift({(double)x, (double)y});
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nLookAt(JNIEnv*, jclass, jlong nativeCamera,
        jdouble eye_x, jdouble eye_y, jdouble eye_z, jdouble center_x, jdouble center_y,
        jdouble center_z, jdouble up_x, jdouble up_y, jdouble up_z) {
    Camera *camera = (Camera *) nativeCamera;
    camera->lookAt({eye_x, eye_y, eye_z}, {center_x, center_y, center_z}, {up_x, up_y, up_z});
}

extern "C" JNIEXPORT jdouble JNICALL
Java_com_google_android_filament_Camera_nGetNear(JNIEnv*, jclass, jlong nativeCamera) {
    Camera *camera = (Camera *) nativeCamera;
    return camera->getNear();
}

extern "C" JNIEXPORT jdouble JNICALL
Java_com_google_android_filament_Camera_nGetCullingFar(JNIEnv*, jclass,
        jlong nativeCamera) {
    Camera *camera = (Camera *) nativeCamera;
    return camera->getCullingFar();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nSetModelMatrix(JNIEnv *env, jclass,
        jlong nativeCamera, jfloatArray in_) {
    Camera* camera = (Camera *) nativeCamera;
    jfloat *in = env->GetFloatArrayElements(in_, NULL);
    camera->setModelMatrix((math::mat4)*reinterpret_cast<const filament::math::mat4f*>(in));
    env->ReleaseFloatArrayElements(in_, in, JNI_ABORT);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nSetModelMatrixFp64(JNIEnv *env, jclass,
        jlong nativeCamera, jdoubleArray in_) {
    Camera* camera = (Camera *) nativeCamera;
    jdouble *in = env->GetDoubleArrayElements(in_, NULL);
    camera->setModelMatrix(*reinterpret_cast<const filament::math::mat4*>(in));
    env->ReleaseDoubleArrayElements(in_, in, JNI_ABORT);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nGetProjectionMatrix(JNIEnv *env, jclass,
        jlong nativeCamera, jdoubleArray out_) {
    Camera *camera = (Camera *) nativeCamera;
    jdouble *out = env->GetDoubleArrayElements(out_, NULL);
    const filament::math::mat4& m = camera->getProjectionMatrix();
    std::copy_n(&m[0][0], 16, out);
    env->ReleaseDoubleArrayElements(out_, out, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nGetCullingProjectionMatrix(JNIEnv *env, jclass,
        jlong nativeCamera, jdoubleArray out_) {
    Camera *camera = (Camera *) nativeCamera;
    jdouble *out = env->GetDoubleArrayElements(out_, NULL);
    const filament::math::mat4& m = camera->getCullingProjectionMatrix();
    std::copy_n(&m[0][0], 16, out);
    env->ReleaseDoubleArrayElements(out_, out, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nGetScaling(JNIEnv *env, jclass,
        jlong nativeCamera, jdoubleArray out_) {
    Camera *camera = (Camera *) nativeCamera;
    jdouble *out = env->GetDoubleArrayElements(out_, NULL);
    const filament::math::double4& s = camera->getScaling();
    std::copy_n(&s[0], 4, out);
    env->ReleaseDoubleArrayElements(out_, out, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nGetModelMatrix(JNIEnv *env, jclass,
        jlong nativeCamera, jfloatArray out_) {
    Camera *camera = (Camera *) nativeCamera;
    jfloat *out = env->GetFloatArrayElements(out_, NULL);
    const filament::math::mat4f& m = (math::mat4f)camera->getModelMatrix();
    std::copy_n(&m[0][0], 16, out);
    env->ReleaseFloatArrayElements(out_, out, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nGetModelMatrixFp64(JNIEnv *env, jclass,
        jlong nativeCamera, jdoubleArray out_) {
    Camera *camera = (Camera *) nativeCamera;
    jdouble *out = env->GetDoubleArrayElements(out_, NULL);
    const filament::math::mat4& m = camera->getModelMatrix();
    std::copy_n(&m[0][0], 16, out);
    env->ReleaseDoubleArrayElements(out_, out, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nGetViewMatrix(JNIEnv *env, jclass, jlong nativeCamera,
        jfloatArray out_) {
    Camera *camera = (Camera *) nativeCamera;
    jfloat *out = env->GetFloatArrayElements(out_, NULL);
    const filament::math::mat4f& m = (math::mat4f)camera->getViewMatrix();
    std::copy_n(&m[0][0], 16, out);
    env->ReleaseFloatArrayElements(out_, out, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nGetViewMatrixFp64(JNIEnv *env, jclass, jlong nativeCamera,
        jdoubleArray out_) {
    Camera *camera = (Camera *) nativeCamera;
    jdouble *out = env->GetDoubleArrayElements(out_, NULL);
    const filament::math::mat4& m = camera->getViewMatrix();
    std::copy_n(&m[0][0], 16, out);
    env->ReleaseDoubleArrayElements(out_, out, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nGetPosition(JNIEnv *env, jclass, jlong nativeCamera,
        jfloatArray out_) {
    Camera *camera = (Camera *) nativeCamera;
    jfloat *out = env->GetFloatArrayElements(out_, NULL);
    reinterpret_cast<filament::math::float3&>(*out) = camera->getPosition();
    env->ReleaseFloatArrayElements(out_, out, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nGetLeftVector(JNIEnv *env, jclass, jlong nativeCamera,
        jfloatArray out_) {
    Camera *camera = (Camera *) nativeCamera;
    jfloat *out = env->GetFloatArrayElements(out_, NULL);
    reinterpret_cast<filament::math::float3&>(*out) = camera->getLeftVector();
    env->ReleaseFloatArrayElements(out_, out, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nGetUpVector(JNIEnv *env, jclass, jlong nativeCamera,
        jfloatArray out_) {
    Camera *camera = (Camera *) nativeCamera;
    jfloat *out = env->GetFloatArrayElements(out_, NULL);
    reinterpret_cast<filament::math::float3&>(*out) = camera->getUpVector();
    env->ReleaseFloatArrayElements(out_, out, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nGetForwardVector(JNIEnv *env, jclass,
        jlong nativeCamera, jfloatArray out_) {
    Camera *camera = (Camera *) nativeCamera;
    jfloat *out = env->GetFloatArrayElements(out_, NULL);
    reinterpret_cast<filament::math::float3&>(*out) = camera->getForwardVector();
    env->ReleaseFloatArrayElements(out_, out, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nSetExposure(JNIEnv*, jclass , jlong nativeCamera,
        jfloat aperture, jfloat shutterSpeed, jfloat sensitivity) {
    Camera *camera = (Camera *) nativeCamera;
    camera->setExposure(aperture, shutterSpeed, sensitivity);
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_Camera_nGetAperture(JNIEnv*, jclass, jlong nativeCamera) {
    Camera *camera = (Camera *) nativeCamera;
    return camera->getAperture();
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_Camera_nGetShutterSpeed(JNIEnv*, jclass,
        jlong nativeCamera) {
    Camera *camera = (Camera *) nativeCamera;
    return camera->getShutterSpeed();
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_Camera_nGetSensitivity(JNIEnv*, jclass,
        jlong nativeCamera) {
    Camera *camera = (Camera *) nativeCamera;
    return camera->getSensitivity();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Camera_nSetFocusDistance(JNIEnv*, jclass,
        jlong nativeCamera, jfloat focusDistance) {
    Camera *camera = (Camera *) nativeCamera;
    camera->setFocusDistance(focusDistance);
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_Camera_nGetFocusDistance(JNIEnv*, jclass,
        jlong nativeCamera) {
    Camera *camera = (Camera *) nativeCamera;
    return camera->getFocusDistance();
}

extern "C" JNIEXPORT jdouble JNICALL
Java_com_google_android_filament_Camera_nGetFocalLength(JNIEnv*, jclass,
        jlong nativeCamera) {
    Camera *camera = (Camera *) nativeCamera;
    return camera->getFocalLength();
}

extern "C" JNIEXPORT jdouble JNICALL
Java_com_google_android_filament_Camera_nComputeEffectiveFocalLength(JNIEnv*, jclass,
        jdouble focalLength, jdouble focusDistance) {
    return Camera::computeEffectiveFocalLength(focalLength, focusDistance);
}

extern "C" JNIEXPORT jdouble JNICALL
Java_com_google_android_filament_Camera_nComputeEffectiveFov(JNIEnv*, jclass,
        jdouble fovInDegrees, jdouble focusDistance) {
    return Camera::computeEffectiveFov(fovInDegrees, focusDistance);
}
