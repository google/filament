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

#include "FullScreenTriangle.h"

// Includes the camera feed material definition BLOB.
#include "resources.h"

#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/RenderableManager.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/VertexBuffer.h>

#include <utils/EntityManager.h>

#include <math/half.h>

using namespace filament;
using namespace filament::math;

struct Vertex {
    half4 position;
    half2 uv;
};

static const Vertex vertices[3] = {
    { { -1.0_h, -1.0_h, 1.0_h, 1.0_h }, {  0.0_h,  1.0_h } },
    { {  3.0_h, -1.0_h, 1.0_h, 1.0_h }, {  2.0_h,  1.0_h } },
    { { -1.0_h,  3.0_h, 1.0_h, 1.0_h }, {  0.0_h, -1.0_h } },
};

static constexpr uint16_t indices[3] = { 0, 1, 2 };

FullScreenTriangle::FullScreenTriangle(Engine* engine) : mEngine(engine) {
    createRenderable();
}

FullScreenTriangle::~FullScreenTriangle() {
    mEngine->destroy(mCameraFeedTexture);
    mEngine->destroy(mIndexBuffer);
    mEngine->destroy(mVertexBuffer);
    mEngine->destroy(mMaterial);
    mEngine->destroy(mCameraFeedTriangle);
}

void FullScreenTriangle::createRenderable() {
    mMaterial = Material::Builder()
        .package(RESOURCES_CAMERA_FEED_DATA, RESOURCES_CAMERA_FEED_SIZE)
        .build(*mEngine);

    mCameraFeedTexture = Texture::Builder()
        .levels(1)
        .sampler(Texture::Sampler::SAMPLER_EXTERNAL)
        .build(*mEngine);

    mVertexBuffer = VertexBuffer::Builder()
        .vertexCount(3)
        .bufferCount(1)
        .attribute(VertexAttribute::POSITION, 0,
                   VertexBuffer::AttributeType::HALF4, offsetof(Vertex, position), sizeof(Vertex))
        .attribute(VertexAttribute::UV0, 0,
                   VertexBuffer::AttributeType::HALF2, offsetof(Vertex, uv), sizeof(Vertex))
        .build(*mEngine);
    mVertexBuffer->setBufferAt(*mEngine, 0,
            VertexBuffer::BufferDescriptor(vertices, sizeof(vertices), nullptr));

    mIndexBuffer = filament::IndexBuffer::Builder()
        .indexCount(3)
        .bufferType(IndexBuffer::IndexType::USHORT)
        .build(*mEngine);
    mIndexBuffer->setBuffer(*mEngine,
            IndexBuffer::BufferDescriptor(indices, sizeof(indices), nullptr));

    mCameraFeedTriangle = utils::EntityManager::get().create();
    mMaterialInstance = mMaterial->getDefaultInstance();
    RenderableManager::Builder(1)
        .material(0, mMaterialInstance)
        .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, mVertexBuffer, mIndexBuffer)
        .culling(false)
        .receiveShadows(false)
        .castShadows(false)
        .build(*mEngine, mCameraFeedTriangle);
    mMaterialInstance->setParameter("cameraFeed", mCameraFeedTexture, TextureSampler());
}

void FullScreenTriangle::setCameraFeedTexture(void* pixelBufferRef) {
    // No need to retain the pixel buffer here, as Filament will take ownership and release it when
    // appropriate. The external image is guaranteed to be valid until the next call to
    // setExternalImage, which in our case will be the next frame. Filament ensures that all
    // rendering using the image has completed on the GPU before releasing it.
    // See the README.md for this sample for more information on iOS external images.
    mCameraFeedTexture->setExternalImage(*mEngine, pixelBufferRef);
}

void FullScreenTriangle::setCameraFeedTransform(filament::math::mat3f transform) {
    mMaterialInstance->setParameter("textureTransform", transform);
}
