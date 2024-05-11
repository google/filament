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


#include <filamentapp/Sphere.h>

#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <utils/EntityManager.h>
#include <math/norm.h>

#include <geometry/SurfaceOrientation.h>

#include <filamentapp/IcoSphere.h>

using namespace filament;
using namespace filament::math;
using namespace utils;


struct Geometry {
    IcoSphere sphere = IcoSphere{ 2 };
    std::vector<filament::math::short4> tangents;
    filament::VertexBuffer* vertexBuffer = nullptr;
    filament::IndexBuffer* indexBuffer = nullptr;
};

// note: this will be leaked since we don't have a good time to free it.
// this should be a cache indexed on the sphere's subdivisions
static Geometry* gGeometry = nullptr;

Sphere::Sphere(Engine& engine, Material const* material, bool culling)
        : mEngine(engine) {
    Geometry* geometry = gGeometry;

    static_assert(sizeof(IcoSphere::Triangle) == sizeof(IcoSphere::Index) * 3,
            "indices are not packed");

    if (geometry == nullptr) {
        geometry = gGeometry = new Geometry;

        auto const& indices = geometry->sphere.getIndices();
        auto const& vertices = geometry->sphere.getVertices();
        uint32_t indexCount = (uint32_t)(indices.size() * 3);

        geometry->tangents.resize(vertices.size());
        auto* quats = geometry::SurfaceOrientation::Builder()
            .vertexCount(vertices.size())
            .normals(vertices.data(), sizeof(float3))
            .build();
        quats->getQuats((short4*) geometry->tangents.data(), vertices.size(),
                sizeof(filament::math::short4));
        delete quats;

        // todo produce correct u,v

        geometry->vertexBuffer = VertexBuffer::Builder()
                .vertexCount((uint32_t)vertices.size())
                .bufferCount(2)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
                .attribute(VertexAttribute::TANGENTS, 1, VertexBuffer::AttributeType::SHORT4)
                .normalized(VertexAttribute::TANGENTS)
                .build(engine);

        geometry->vertexBuffer->setBufferAt(engine, 0,
                VertexBuffer::BufferDescriptor(vertices.data(), vertices.size() * sizeof(float3)));

        geometry->vertexBuffer->setBufferAt(engine, 1,
                VertexBuffer::BufferDescriptor(geometry->tangents.data(), geometry->tangents.size() * sizeof(filament::math::short4)));


        geometry->indexBuffer = IndexBuffer::Builder()
                .bufferType(IndexBuffer::IndexType::USHORT)
                .indexCount(indexCount)
                .build(engine);

        geometry->indexBuffer->setBuffer(engine,
                IndexBuffer::BufferDescriptor(indices.data(), indexCount * sizeof(IcoSphere::Index)));
    }

    if (material) {
        mMaterialInstance = material->createInstance();
    }

    utils::EntityManager& em = utils::EntityManager::get();
    mRenderable = em.create();
    RenderableManager::Builder(1)
            .boundingBox({{0}, {1}})
            .material(0, mMaterialInstance)
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, geometry->vertexBuffer, geometry->indexBuffer)
            .culling(culling)
            .build(engine, mRenderable);
}

Sphere::~Sphere() {
    mEngine.destroy(mMaterialInstance);
    mEngine.destroy(mRenderable);
    utils::EntityManager& em = utils::EntityManager::get();
    em.destroy(mRenderable);
}

Sphere& Sphere::setPosition(filament::math::float3 const& position) noexcept {
    auto& tcm = mEngine.getTransformManager();
    auto ci = tcm.getInstance(mRenderable);
    mat4f model = tcm.getTransform(ci);
    model[3].xyz = position;
    tcm.setTransform(ci, model);
    return *this;
}

Sphere& Sphere::setRadius(float radius) noexcept {
    auto& tcm = mEngine.getTransformManager();
    auto ci = tcm.getInstance(mRenderable);
    mat4f model = tcm.getTransform(ci);
    model[0].x = radius;
    model[1].y = radius;
    model[2].z = radius;
    tcm.setTransform(ci, model);
    return *this;
}
