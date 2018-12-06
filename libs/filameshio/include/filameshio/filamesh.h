/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef TNT_FILAMENT_FILAMESHIO_FILAMESH_H
#define TNT_FILAMENT_FILAMESHIO_FILAMESH_H

#include <filament/Box.h>

namespace filamesh {

using Box = filament::Box;

static const char MAGICID[] { 'F', 'I', 'L', 'A', 'M', 'E', 'S', 'H' };

static const uint32_t VERSION = 1;

enum IndexType : uint32_t {
    UI32 = 0,
    UI16 = 1,
};

enum Flags : uint32_t {
    INTERLEAVED         = 1 << 0,
    TEXCOORD_SNORM16    = 1 << 1,
    COMPRESSION         = 1 << 2,
};

// Each of these fields specifies a number of bytes within the compressed data. This is ignored
// when the INTERLEAVED flag is enabled.
struct CompressionHeader {
    uint32_t positions;
    uint32_t tangents;
    uint32_t colors;
    uint32_t uv0;
    uint32_t uv1;
};

struct Header {
    uint32_t version;
    uint32_t parts;
    Box      aabb;
    uint32_t flags;
    uint32_t offsetPosition;
    uint32_t stridePosition;
    uint32_t offsetTangents;
    uint32_t strideTangents;
    uint32_t offsetColor;
    uint32_t strideColor;
    uint32_t offsetUV0;
    uint32_t strideUV0;
    uint32_t offsetUV1;
    uint32_t strideUV1;
    uint32_t vertexCount;
    uint32_t vertexSize;
    uint32_t indexType;
    uint32_t indexCount;
    uint32_t indexSize;
};

struct Part {
    uint32_t offset;
    uint32_t indexCount;
    uint32_t minIndex;
    uint32_t maxIndex;
    uint32_t material;
    Box aabb;
};

} // namespace filamesh

#endif // TNT_FILAMENT_FILAMESHIO_FILAMESH_H
