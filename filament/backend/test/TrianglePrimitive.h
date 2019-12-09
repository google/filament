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

#ifndef TNT_TRIANGLEPRIMITIVE_H
#define TNT_TRIANGLEPRIMITIVE_H

#include "private/backend/DriverApi.h"

#include <math/vec2.h>

namespace test {

/**
 * A wrapper class that manages the necessary resources for a simple triangle renderable used for
 * test cases.
 */
class TrianglePrimitive {
public:

    using PrimitiveHandle = filament::backend::Handle<filament::backend::HwRenderPrimitive>;
    using VertexHandle = filament::backend::Handle<filament::backend::HwVertexBuffer>;
    using IndexHandle = filament::backend::Handle<filament::backend::HwIndexBuffer>;

    TrianglePrimitive(filament::backend::DriverApi& driverApi, bool allocateLargeBuffers = false);
    ~TrianglePrimitive();

    PrimitiveHandle getRenderPrimitive() const noexcept;

    void updateVertices(const filament::math::float2 vertices[3]) noexcept;
    void updateIndices(const short indices[3]) noexcept;

private:

    size_t mVertexCount = 3;
    size_t mIndexCount = 3;

    filament::backend::DriverApi& mDriverApi;

    PrimitiveHandle mRenderPrimitive;
    VertexHandle mVertexBuffer;
    IndexHandle mIndexBuffer;

};

} // namespace test

#endif
