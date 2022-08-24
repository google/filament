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

#ifndef GLTFIO_MATH_H
#define GLTFIO_MATH_H

#include <math/quat.h>
#include <math/vec3.h>
#include <math/mat3.h>
#include <math/mat4.h>
#include <math/TVecHelpers.h>

#include <utils/compiler.h>

namespace filament::gltfio {

template <typename T>
UTILS_PUBLIC T cubicSpline(const T& vert0, const T& tang0, const T& vert1, const T& tang1, float t) {
    float tt = t * t, ttt = tt * t;
    float s2 = -2 * ttt + 3 * tt, s3 = ttt - tt;
    float s0 = 1 - s2, s1 = s3 - tt + t;
    T p0 = vert0;
    T m0 = tang0;
    T p1 = vert1;
    T m1 = tang1;
    return s0 * p0 + s1 * m0 * t + s2 * p1 + s3 * m1 * t;
}

UTILS_PUBLIC inline void decomposeMatrix(const filament::math::mat4f& mat, filament::math::float3* translation,
        filament::math::quatf* rotation, filament::math::float3* scale) {
    using namespace filament::math;

    // Extract translation.
    *translation = mat[3].xyz;

    // Extract upper-left for determinant computation.
    const float a = mat[0][0];
    const float b = mat[0][1];
    const float c = mat[0][2];
    const float d = mat[1][0];
    const float e = mat[1][1];
    const float f = mat[1][2];
    const float g = mat[2][0];
    const float h = mat[2][1];
    const float i = mat[2][2];
    const float A = e * i - f * h;
    const float B = f * g - d * i;
    const float C = d * h - e * g;

    // Extract scale.
    const float det(a * A + b * B + c * C);
    float scalex = length(float3({a, b, c}));
    float scaley = length(float3({d, e, f}));
    float scalez = length(float3({g, h, i}));
    float3 s = { scalex, scaley, scalez };
    if (det < 0) {
        s = -s;
    }
    *scale = s;

    // Remove scale from the matrix if it is not close to zero.
    mat4f clone = mat;
    if (std::abs(det) > std::numeric_limits<float>::epsilon()) {
        clone[0] /= s.x;
        clone[1] /= s.y;
        clone[2] /= s.z;
        // Extract rotation
        *rotation = clone.toQuaternion();
    } else {
        // Set to identity if close to zero
        *rotation = quatf(1.0f);
    }
}

UTILS_PUBLIC inline filament::math::mat4f composeMatrix(const filament::math::float3& translation,
        const filament::math::quatf& rotation, const filament::math::float3& scale) {
    float tx = translation[0];
    float ty = translation[1];
    float tz = translation[2];
    float qx = rotation[0];
    float qy = rotation[1];
    float qz = rotation[2];
    float qw = rotation[3];
    float sx = scale[0];
    float sy = scale[1];
    float sz = scale[2];
    return filament::math::mat4f(
        (1 - 2 * qy*qy - 2 * qz*qz) * sx,
        (2 * qx*qy + 2 * qz*qw) * sx,
        (2 * qx*qz - 2 * qy*qw) * sx,
        0.f,
        (2 * qx*qy - 2 * qz*qw) * sy,
        (1 - 2 * qx*qx - 2 * qz*qz) * sy,
        (2 * qy*qz + 2 * qx*qw) * sy,
        0.f,
        (2 * qx*qz + 2 * qy*qw) * sz,
        (2 * qy*qz - 2 * qx*qw) * sz,
        (1 - 2 * qx*qx - 2 * qy*qy) * sz,
        0.f, tx, ty, tz, 1.f);
}

inline filament::math::mat3f matrixFromUvTransform(const float offset[2], float rotation,
        const float scale[2]) {
    float tx = offset[0];
    float ty = offset[1];
    float sx = scale[0];
    float sy = scale[1];
    float c = cos(rotation);
    float s = sin(rotation);
    return filament::math::mat3f(sx * c, sx * s, tx, -sy * s, sy * c, ty, 0.0f, 0.0f, 1.0f);
};

} // namespace filament::gltfio

#endif // GLTFIO_MATH_H
