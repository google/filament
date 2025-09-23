/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include <filamentapp/Grid.h>

#include <filament/Box.h>
#include <filament/Camera.h>
#include <filament/Color.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialEnums.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>

#include <utils/EntityManager.h>

#include <math/vec3.h>
#include <math/mat4.h>

#include <cstdint>
#include <cstddef>
#include <utility>
#include <vector>
#include <utils/Log.h>

using namespace filament;

using namespace filament::math;
using namespace filament;

Grid::Grid(Engine& engine, Material const* material, float3 linearColor)
        : mEngine(engine),
          mMaterial(material) {

    if (mMaterial) {
        mMaterialInstanceWireFrame = mMaterial->createInstance();
        mMaterialInstanceWireFrame->setDepthCulling(true);
        mMaterialInstanceWireFrame->setParameter("color", RgbaType::LINEAR,
                LinearColorA{linearColor.r, linearColor.g, linearColor.b, 0.25f});
    }

    utils::EntityManager& em = utils::EntityManager::get();
    mWireFrameRenderable = em.create();

    RenderableManager::Builder(1)
            .boundingBox({ { -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } })
            .material(0, mMaterialInstanceWireFrame)
            .priority(6)
            .culling(false)
            .build(engine, mWireFrameRenderable);
}

Grid::Grid(Grid&& rhs) noexcept
        : mEngine(rhs.mEngine) {
    using std::swap;
    swap(rhs.mVertexBuffer, mVertexBuffer);
    swap(rhs.mIndexBuffer, mIndexBuffer);
    swap(rhs.mMaterial, mMaterial);
    swap(rhs.mMaterialInstanceWireFrame, mMaterialInstanceWireFrame);
    swap(rhs.mWireFrameRenderable, mWireFrameRenderable);
}

Grid::~Grid() {
    mEngine.destroy(mVertexBuffer);
    mEngine.destroy(mIndexBuffer);

    // We don't own the material, only instances
    mEngine.destroy(mWireFrameRenderable);

    // material instances must be destroyed after the renderables
    mEngine.destroy(mMaterialInstanceWireFrame);

    utils::EntityManager& em = utils::EntityManager::get();
    em.destroy(mWireFrameRenderable);
}

void Grid::mapFrustum(Engine& engine, Camera const* camera) {
    // the Camera far plane is at infinity, but we want it closer for display
    const mat4 vm(camera->getModelMatrix());
    mat4 const p(vm * inverse(camera->getProjectionMatrix()));
    mapFrustum(engine, p);
}

void Grid::mapFrustum(Engine& engine, mat4 const& transform) {
    // the Camera far plane is at infinity, but we want it closer for display
    mat4f const p(transform);
    auto& tcm = engine.getTransformManager();
    tcm.setTransform(tcm.getInstance(mWireFrameRenderable), p);
}

void Grid::mapAabb(Engine& engine, Box const& box) {
    mat4 const p = mat4::translation(box.center) * mat4::scaling(box.halfExtent);
    mapFrustum(engine, p);
}

void Grid::update(uint32_t width, uint32_t height, uint32_t depth) {
    // [-1, 1] range (default behavior)
    update(width, height, depth,
            [](int const index) { return float(index) * 2.0f - 1.0f; },
            [](int const index) { return float(index) * 2.0f - 1.0f; },
            [](int const index) { return float(index) * 2.0f - 1.0f; });
}

void Grid::update(uint32_t const width, uint32_t const height, uint32_t const depth,
            Generator const& genWidth, Generator const& genHeight, Generator const& genDepth) {

    // First, destroy the existing buffers.
    mEngine.destroy(mVertexBuffer);
    mEngine.destroy(mIndexBuffer);

    std::vector<float3> vertices;
    std::vector<uint32_t> indices;
    size_t const verticeCount = (depth + 1) * (height + 1) * (width + 1);
    vertices.reserve(verticeCount);
    indices.reserve(verticeCount * 2);

    // Generate vertices
    // The vertices are generated to form a grid in the [-1, 1] range on all axes.
    for (int k = 0; k <= depth; ++k) {
        auto const z = genDepth(k);
        for (int j = 0; j <= height; ++j) {
            auto const y = genHeight(j);
            for (int i = 0; i <= width; ++i) {
                auto const x = genWidth(i);
                vertices.push_back({ x, y, z });
            }
        }
    }

    // Generate indices for the lines
    // The indices connect the vertices to form the grid lines.
    auto getVertexIndex = [=](uint32_t const i, uint32_t const j, uint32_t const k) {
        return k * (width + 1) * (height + 1) + j * (width + 1) + i;
    };

    // Lines along X axis
    for (uint32_t k = 0; k <= depth; ++k) {
        for (uint32_t j = 0; j <= height; ++j) {
            indices.push_back(getVertexIndex(0, j, k));
            indices.push_back(getVertexIndex(width, j, k));
        }
    }

    // Lines along Y axis
    for (uint32_t k = 0; k <= depth; ++k) {
        for (uint32_t i = 0; i <= width; ++i) {
            indices.push_back(getVertexIndex(i, 0, k));
            indices.push_back(getVertexIndex(i, height, k));
        }
    }

    // Lines along Z axis
    for (uint32_t j = 0; j <= height; ++j) {
        for (uint32_t i = 0; i <= width; ++i) {
            indices.push_back(getVertexIndex(i, j, 0));
            indices.push_back(getVertexIndex(i, j, depth));
        }
    }

    const size_t vertexCount = vertices.size();
    mVertexBuffer = VertexBuffer::Builder()
            .vertexCount(vertexCount)
            .bufferCount(1)
            .attribute(POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
            .build(mEngine);

    auto vertexData = vertices.data();
    mVertexBuffer->setBufferAt(mEngine, 0,
            VertexBuffer::BufferDescriptor::make(
                    vertexData, vertexCount * sizeof(vertices[0]),
                    [v = std::move(vertices)](void*, size_t) {}));

    const size_t indexCount = indices.size();
    mIndexBuffer = IndexBuffer::Builder()
            .indexCount(indexCount)
            .build(mEngine);

    auto indexData = indices.data();
    mIndexBuffer->setBuffer(mEngine,
            IndexBuffer::BufferDescriptor::make(
                indexData, indexCount * sizeof(uint32_t),
                [i = std::move(indices)](void*, size_t) {}));

    auto& rcm = mEngine.getRenderableManager();
    auto instance = rcm.getInstance(mWireFrameRenderable);
    rcm.setGeometryAt(instance, 0, RenderableManager::PrimitiveType::LINES,
            mVertexBuffer, mIndexBuffer, 0, indexCount);
}
