/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "CameraManipulator.h"

using namespace filament::math;

template <typename T>
static constexpr inline double radians(T deg) {
    return deg * (M_PI / 180.0);
}

static inline double3 rotateVector(double rx, double ry, const double3& v) {
    return (mat3::rotation(ry, double3{ 0, 1, 0}) * mat3::rotation(rx, double3{ 1, 0, 0})) * v;
}

//------------------------------------------------------------------------------

CameraManipulator::CameraManipulator() = default;

CameraManipulator::CameraManipulator(Camera* camera, size_t width, size_t height)
        : mCamera(camera), mWidth(width), mHeight(height), mAspect(double(width)/height) {
    lookAt(double3(0), double3(0,0,-1));
}

void CameraManipulator::setCamera(Camera* camera) {
    mCamera = camera;
}

void CameraManipulator::setViewport(size_t w, size_t h) {
    mWidth = w;
    mHeight = h;
    mAspect = double(w)/h;
}

//------------------------------------------------------------------------------
void CameraManipulator::lookAt(const double3& eye, const double3& at) {
    mTranslation = eye;
    double3 dt = at - eye;
    double yz_length = std::sqrt((dt.y * dt.y) + (dt.z * dt.z));
    mRotation.z = 0.0;
    mRotation.x = std::atan2(dt.y, -dt.z);
    mRotation.y = std::atan2(dt.x, yz_length);
    mCenterOfInterest = -length(dt);
    updateCameraTransform();
}

//------------------------------------------------------------------------------
void CameraManipulator::track(const double2& delta) {

    double3 d_s = rotateVector(mRotation.x, mRotation.y, { 1.0, 0.0, 0.0});
    double3 d_t = rotateVector(mRotation.x, mRotation.y, { 0.0, 1.0, 0.0});

    double mult_t = 2.0 * mCenterOfInterest * std::tan(radians(mFovx) / 2.0);
    double mult_s = mult_t / mWidth;
    mult_t /= mHeight;

    double s =  mult_s * delta.x;
    double t =  mult_t * delta.y;

    mTranslation += (s * d_s) + (t * d_t);
    updateCameraTransform();
}

//------------------------------------------------------------------------------
void CameraManipulator::dolly(double delta, double dolly_speed) {
    double3 eye = mTranslation;
    double3 v = rotateVector(mRotation.x, mRotation.y, { 0.0, 0.0, mCenterOfInterest});
    double3 view = eye + v;

    // Magic dolly function
    v = normalize(v);
    double t = delta / mWidth;
    double dolly_by = 1.0 - std::exp(-dolly_speed * t);

    dolly_by *= mCenterOfInterest;
    double3 new_eye = eye + (dolly_by * v);

    mTranslation = new_eye;
    v = new_eye - view;
    mCenterOfInterest = -length(v);
    updateCameraTransform();
}

void CameraManipulator::rotate(const double2& delta, double rotate_speed) {
    double rot_x = mRotation.x;
    double rot_y = mRotation.y;
    double rot_z = mRotation.z;
    double3 eye = mTranslation;

    double3 view = eye + rotateVector(rot_x, rot_y, { 0.0, 0.0, mCenterOfInterest});

    rot_y += rotate_speed * (-delta.x / mWidth);
    rot_x += rotate_speed * ( delta.y / mHeight);

    mTranslation = view + rotateVector(rot_x, rot_y, {0.0, 0.0, -mCenterOfInterest});
    mRotation = double3(rot_x, rot_y, rot_z);
    updateCameraTransform();
}

void CameraManipulator::updateCameraTransform() {
    if (mCamera) {
        mat4 rotate_z  = mat4::rotation(mRotation.z, double3(0, 0, 1));
        mat4 rotate_x  = mat4::rotation(mRotation.x, double3(1, 0, 0));
        mat4 rotate_y  = mat4::rotation(mRotation.y, double3(0, 1, 0));
        mat4 translate = mat4::translation(mTranslation);
        mat4 view = translate * (rotate_y * rotate_x * rotate_z);
        mCamera->setModelMatrix(mat4f(view));
        if (mCameraChanged) {
            mCameraChanged(mCamera);
        }
    }
}

void CameraManipulator::updateCameraProjection() {
    if (mCamera) {
        mCamera->setProjection(mFovx, mAspect, mClipNear, mClipFar);
        if (mCameraChanged) {
            mCameraChanged(mCamera);
        }
    }
}

void CameraManipulator::setCameraChangedCallback(CameraManipulator::CameraChangedCallback cb) {
    mCameraChanged = cb;
}
