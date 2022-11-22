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

#include "Wireframe.h"
#include "FFilamentAsset.h"

#include <filament/Box.h>
#include <filament/Engine.h>
#include <filament/VertexBuffer.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>

#include <utils/EntityManager.h>

#include <math/vec3.h>
#include <math/mat4.h>

#include <functional>

using namespace filament;
using namespace filament::math;
using namespace std;
using namespace utils;

static const auto FREE_CALLBACK = [](void* mem, size_t, void*) { free(mem); };

namespace filament::gltfio {

struct FFilamentAsset;

Wireframe::Wireframe(FFilamentAsset* asset) : mAsset(asset) {
    Engine* engine = mAsset->mEngine;
    TransformManager& tm = engine->getTransformManager();
    RenderableManager& rm = engine->getRenderableManager();

    // Traverse the transform hierachy and count renderables.
    function<uint32_t(utils::Entity)> count;
    count = [&count, &tm, &rm](Entity node) {
        uint32_t n = rm.getInstance(node) ? 1 : 0;
        auto transformable = tm.getInstance(node);
        vector<Entity> children(tm.getChildCount(transformable));
        tm.getChildren(transformable, children.data(), children.size());
        for (auto ce : children) {
            n += count(ce);
        }
        return n;
    };
    size_t renderableCount = count(mAsset->mRoot);
    size_t vertCount = renderableCount * 8;
    size_t indCount = renderableCount * 24;
    float3* verts = (float3*) malloc(sizeof(float3) * vertCount);
    uint32_t* inds = (uint32_t*) malloc(sizeof(uint32_t) * indCount);

    // Traverse the hierarchy a second time to populate the vertex & index buffers.
    function<void(utils::Entity)> create;
    float3* pvert = verts;
    uint32_t* pindx = inds;
    create = [&create, &pvert, &pindx, &rm, &tm, verts](Entity node) {
        auto transformable = tm.getInstance(node);
        auto renderable = rm.getInstance(node);
        if (renderable) {
            Box aabb = rm.getAxisAlignedBoundingBox(renderable);
            float3 minpt = aabb.getMin();
            float3 maxpt = aabb.getMax();
            mat4f worldTransform = tm.getWorldTransform(transformable);

            // Write coordinates for the 8 corners of the cuboid.
            pvert[0] = (worldTransform * float4(minpt.x, minpt.y, minpt.z, 1.0)).xyz;
            pvert[1] = (worldTransform * float4(minpt.x, minpt.y, maxpt.z, 1.0)).xyz;
            pvert[2] = (worldTransform * float4(minpt.x, maxpt.y, minpt.z, 1.0)).xyz;
            pvert[3] = (worldTransform * float4(minpt.x, maxpt.y, maxpt.z, 1.0)).xyz;
            pvert[4] = (worldTransform * float4(maxpt.x, minpt.y, minpt.z, 1.0)).xyz;
            pvert[5] = (worldTransform * float4(maxpt.x, minpt.y, maxpt.z, 1.0)).xyz;
            pvert[6] = (worldTransform * float4(maxpt.x, maxpt.y, minpt.z, 1.0)).xyz;
            pvert[7] = (worldTransform * float4(maxpt.x, maxpt.y, maxpt.z, 1.0)).xyz;

            uint32_t i = pvert - verts;
            pvert += 8;

            // Generate 4 lines around face at -X.
            pindx[0] = i + 0;
            pindx[1] = i + 1;
            pindx[2] = i + 1;
            pindx[3] = i + 3;
            pindx[4] = i + 3;
            pindx[5] = i + 2;
            pindx[6] = i + 2;
            pindx[7] = i + 0;
            // Generate 4 lines around face at +X.
            pindx[ 8] = i + 4;
            pindx[ 9] = i + 5;
            pindx[10] = i + 5;
            pindx[11] = i + 7;
            pindx[12] = i + 7;
            pindx[13] = i + 6;
            pindx[14] = i + 6;
            pindx[15] = i + 4;
            // Generate 2 horizontal lines at -Z.
            pindx[16] = i + 0;
            pindx[17] = i + 4;
            pindx[18] = i + 2;
            pindx[19] = i + 6;
            // Generate 2 horizontal lines at +Z.
            pindx[20] = i + 1;
            pindx[21] = i + 5;
            pindx[22] = i + 3;
            pindx[23] = i + 7;
            pindx += 24;
        }

        vector<Entity> children(tm.getChildCount(transformable));
        tm.getChildren(transformable, children.data(), children.size());
        for (auto ce : children) {
            create(ce);
        }
    };
    create(mAsset->mRoot);

    mVertexBuffer = VertexBuffer::Builder()
        .bufferCount(1)
        .vertexCount(vertCount)
        .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
        .build(*engine);

    mIndexBuffer = IndexBuffer::Builder()
        .indexCount(indCount)
        .bufferType(IndexBuffer::IndexType::UINT)
        .build(*engine);

    mVertexBuffer->setBufferAt(*engine, 0, VertexBuffer::BufferDescriptor(
                    verts, mVertexBuffer->getVertexCount() * sizeof(float3), FREE_CALLBACK));

    mIndexBuffer->setBuffer(*engine, IndexBuffer::BufferDescriptor(
                    inds, mIndexBuffer->getIndexCount() * sizeof(uint32_t), FREE_CALLBACK));

    mEntity = EntityManager::get().create();

    RenderableManager::Builder(1)
        .culling(false)
        .castShadows(false)
        .receiveShadows(false)
        .geometry(0, RenderableManager::PrimitiveType::LINES, mVertexBuffer, mIndexBuffer)
        .build(*engine, mEntity);
}

Wireframe::~Wireframe() {
    Engine* engine = mAsset->mEngine;
    engine->destroy(mEntity);
    engine->destroy(mVertexBuffer);
    engine->destroy(mIndexBuffer);
}

} // namsepace gltfio
