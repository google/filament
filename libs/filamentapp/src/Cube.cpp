/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <filamentapp/Cube.h>

#include <utils/EntityManager.h>
#include <filament/VertexBuffer.h>
#include <filament/IndexBuffer.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>

using namespace filament::math;
using namespace filament;

const uint32_t Cube::mIndices[] = {
        // solid
        2,0,1, 2,1,3,  // far
        6,4,5, 6,5,7,  // near
        2,0,4, 2,4,6,  // left
        3,1,5, 3,5,7,  // right
        0,4,5, 0,5,1,  // bottom
        2,6,7, 2,7,3,  // top

        // wire-frame
        0,1, 1,3, 3,2, 2,0,     // far
        4,5, 5,7, 7,6, 6,4,     // near
        0,4, 1,5, 3,7, 2,6,
};

const filament::math::float3 Cube::mVertices[] = {
        { -1, -1,  1},  // 0. left bottom far
        {  1, -1,  1},  // 1. right bottom far
        { -1,  1,  1},  // 2. left top far
        {  1,  1,  1},  // 3. right top far
        { -1, -1, -1},  // 4. left bottom near
        {  1, -1, -1},  // 5. right bottom near
        { -1,  1, -1},  // 6. left top near
        {  1,  1, -1}}; // 7. right top near


Cube::Cube(Engine& engine, filament::Material const* material, float3 linearColor, bool culling) :
        mEngine(engine),
        mMaterial(material) {

    mVertexBuffer = VertexBuffer::Builder()
            .vertexCount(8)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
            .build(engine);

    mIndexBuffer = IndexBuffer::Builder()
            .indexCount(12*2 + 3*2*6)
            .build(engine);

    if (mMaterial) {
        mMaterialInstanceSolid = mMaterial->createInstance();
        mMaterialInstanceWireFrame = mMaterial->createInstance();
        mMaterialInstanceSolid->setParameter("color", RgbaType::LINEAR,
                LinearColorA{linearColor.r, linearColor.g, linearColor.b, 0.05f});
        mMaterialInstanceWireFrame->setParameter("color", RgbaType::LINEAR,
                LinearColorA{linearColor.r, linearColor.g, linearColor.b, 0.25f});
    }

    mVertexBuffer->setBufferAt(engine, 0,
            VertexBuffer::BufferDescriptor(
                    mVertices, mVertexBuffer->getVertexCount() * sizeof(mVertices[0])));

    mIndexBuffer->setBuffer(engine,
            IndexBuffer::BufferDescriptor(
                    mIndices, mIndexBuffer->getIndexCount() * sizeof(uint32_t)));

    utils::EntityManager& em = utils::EntityManager::get();
    mSolidRenderable = em.create();
    RenderableManager::Builder(1)
            .boundingBox({{ 0, 0, 0 },
                          { 1, 1, 1 }})
            .material(0, mMaterialInstanceSolid)
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, mVertexBuffer, mIndexBuffer, 0, 3*2*6)
            .priority(7)
            .culling(culling)
            .build(engine, mSolidRenderable);

    mWireFrameRenderable = em.create();
    RenderableManager::Builder(1)
            .boundingBox({{ 0, 0, 0 },
                          { 1, 1, 1 }})
            .material(0, mMaterialInstanceWireFrame)
            .geometry(0, RenderableManager::PrimitiveType::LINES, mVertexBuffer, mIndexBuffer, WIREFRAME_OFFSET, 24)
            .priority(6)
            .culling(culling)
            .build(engine, mWireFrameRenderable);
}

Cube::Cube(Cube&& rhs) noexcept
        : mEngine(rhs.mEngine) {
    using std::swap;
    swap(rhs.mVertexBuffer, mVertexBuffer);
    swap(rhs.mIndexBuffer, mIndexBuffer);
    swap(rhs.mMaterial, mMaterial);
    swap(rhs.mMaterialInstanceSolid, mMaterialInstanceSolid);
    swap(rhs.mMaterialInstanceWireFrame, mMaterialInstanceWireFrame);
    swap(rhs.mSolidRenderable, mSolidRenderable);
    swap(rhs.mWireFrameRenderable, mWireFrameRenderable);
}

void Cube::mapFrustum(filament::Engine& engine, Camera const* camera) {
    // the Camera far plane is at infinity, but we want it closer for display
    const mat4 vm(camera->getModelMatrix());
    mat4 p(vm * inverse(camera->getCullingProjectionMatrix()));
    mapFrustum(engine, p);
}

void Cube::mapFrustum(filament::Engine& engine, filament::math::mat4 const& transform) {
    // the Camera far plane is at infinity, but we want it closer for display
    mat4f p(transform);
    auto& tcm = engine.getTransformManager();
    tcm.setTransform(tcm.getInstance(mSolidRenderable), p);
    tcm.setTransform(tcm.getInstance(mWireFrameRenderable), p);
}


void Cube::mapAabb(filament::Engine& engine, filament::Box const& box) {
    mat4 p = mat4::translation(box.center) * mat4::scaling(box.halfExtent);
    mapFrustum(engine, p);
}

Cube::~Cube() {
    mEngine.destroy(mVertexBuffer);
    mEngine.destroy(mIndexBuffer);
    mEngine.destroy(mMaterialInstanceSolid);
    mEngine.destroy(mMaterialInstanceWireFrame);
    // We don't own the material, only instances
    mEngine.destroy(mSolidRenderable);
    mEngine.destroy(mWireFrameRenderable);

    utils::EntityManager& em = utils::EntityManager::get();
    em.destroy(mSolidRenderable);
    em.destroy(mWireFrameRenderable);
}
