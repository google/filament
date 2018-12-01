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

#include <filament/Box.h>

#include <math/half.h>
#include <math/norm.h>
#include <math/quat.h>
#include <math/vec2.h>
#include <math/vec4.h>

namespace filamesh {

    using Box = filament::Box;
    using half4 = math::half4;
    using short4 = math::short4;
    using ubyte4 = math::ubyte4;
    using half2 = math::half2;
    using float3 = math::float3;
    using float4 = math::float4;
    using quatf = math::quatf;

    static const uint32_t VERSION = 1;

    inline constexpr math::half operator"" _h(long double v) {
        return math::half(static_cast<float>(v));
    }

    enum Flags : uint32_t {
        INTERLEAVED = (1 << 0)
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

    struct Vertex {
        Vertex(const float3& position, const quatf& tangents, const float4& color,
                const float3& uv0):
                position(position, 1.0_h),
                tangents(math::packSnorm16(tangents.xyzw)),
                color(clamp(color, 0.0f, 1.0f) * 255.0f),
                uv0(uv0.xy) {
        }

        half4  position;
        short4 tangents;
        ubyte4 color;
        half2  uv0;
    };

    struct Mesh {
        Mesh(uint32_t offset, uint32_t count, uint32_t minIndex, uint32_t maxIndex,
                uint32_t material, const Box& aabb):
                offset(offset),
                count(count),
                minIndex(minIndex),
                maxIndex(maxIndex),
                material(material),
                aabb(aabb) {
        }

        uint32_t offset;
        uint32_t count;
        uint32_t minIndex;
        uint32_t maxIndex;
        uint32_t material;
        Box aabb;
    };

}
