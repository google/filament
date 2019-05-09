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

#ifndef GLTFIO_SIMPLECAMERA_H
#define GLTFIO_SIMPLECAMERA_H

#include <math/vec3.h>

namespace gltfio {

/** Used to configure the embedded path tracer in AssetPipeline. */
struct SimpleCamera {
    float aspectRatio;
    filament::math::float3 eyePosition;
    filament::math::float3 targetPosition;
    filament::math::float3 upVector;
    float vfovDegrees;
};

} // namespace gltfio

#endif // GLTFIO_SIMPLECAMERA_H
