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

#include <filament/Camera.h>

#include "API.h"

using namespace filament;

void Filament_Camera_SetProjectionFrustum(Camera *camera,
                                          Camera::Projection projection,
                                          double left, double right,
                                          double bottom, double top,
                                          double near, double far) {
  camera->setProjection(projection, left, right, bottom, top, near, far);
}

void Filament_Camera_SetProjectionFov(Camera *camera,
                                      double fovInDegrees,
                                      double aspect, double near,
                                      double far, FCameraFov fov) {
  camera->setProjection(fovInDegrees, aspect, near, far, fov);
}

void Filament_Camera_SetProjectionLens(Camera *camera, double focalLength, double near, double far) {
  camera->setLensProjection(focalLength, near, far);
}

void Filament_Camera_SetProjectionMatrix(Camera *camera,
                                         math::mat4 *matrix,
                                         double near, double far) {
  camera->setCustomProjection(*matrix, near, far);
}

void Filament_Camera_GetCullingProjectionMatrix(FCamera *camera, FMat4 *matrixOut) {
  *matrixOut = camera->getCullingProjectionMatrix();
}

void Filament_Camera_LookAt(Camera *camera, FFloat3 *eye, FFloat3 *center, FFloat3 *up) {
  camera->lookAt(*eye, *center, *up);
}

float Filament_Camera_GetNear(Camera *camera) {
  return camera->getNear();
}

float Filament_Camera_GetCullingFar(Camera *camera) {
  return camera->getCullingFar();
}

void Filament_Camera_SetModelMatrix(Camera *camera, math::mat4f *matrix) {
  camera->setModelMatrix(*matrix);
}

void Filament_Camera_GetProjectionMatrix(Camera *camera, math::mat4 *matrixOut) {
  *matrixOut = camera->getProjectionMatrix();
}

void Filament_Camera_GetModelMatrix(Camera *camera, math::mat4f *matrixOut) {
  *matrixOut = camera->getModelMatrix();
}

void Filament_Camera_GetViewMatrix(Camera *camera, math::mat4f *matrixOut) {
  *matrixOut = camera->getViewMatrix();
}

void Filament_Camera_GetPosition(Camera *camera, math::float3 *vectorOut) {
  *vectorOut = camera->getPosition();
}

void Filament_Camera_GetLeftVector(Camera *camera, math::float3 *vectorOut) {
  *vectorOut = camera->getLeftVector();
}

void Filament_Camera_GetUpVector(Camera *camera, math::float3 *vectorOut) {
  *vectorOut = camera->getUpVector();
}

void Filament_Camera_GetForwardVector(Camera *camera, math::float3 *vectorOut) {
  *vectorOut = camera->getForwardVector();
}

void Filament_Camera_GetFrustum(FCamera *camera, FFrustum *frustumOut) {
  *frustumOut = camera->getFrustum();
}

FEntity Filament_Camera_GetEntity(FCamera *camera) {
  return camera->getEntity().getId();
}

void Filament_Camera_SetExposure(Camera *camera, float aperture,
                                 float shutterSpeed,
                                 float sensitivity) {
  camera->setExposure(aperture, shutterSpeed, sensitivity);
}

float Filament_Camera_GetAperture(Camera *camera) {
  return camera->getAperture();
}

float Filament_Camera_GetShutterSpeed(Camera *camera) {
  return camera->getShutterSpeed();
}

float Filament_Camera_GetSensitivity(Camera *camera) {
  return camera->getSensitivity();
}

void Filament_Camera_InverseProjection(FMat4 *projection, FMat4 *invertedOut) {
  *invertedOut = Camera::inverseProjection(*projection);
}

void Filament_Camera_InverseProjectionF(FMat4f *projection, FMat4f *invertedOut) {
  *invertedOut = Camera::inverseProjection(*projection);
}
