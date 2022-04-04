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

#include "FilamentApp.h"

#include <filament/Camera.h>
#include <filament/IndexBuffer.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/Viewport.h>

#include <filameshio/MeshReader.h>
#include <geometry/SurfaceOrientation.h>
#include <ktxreader/Ktx1Reader.h>

#include <sstream>

// This file is generated via the "Run Script" build phase and contains the mesh, materials, and IBL
// textures this app uses.
#include "resources.h"

using namespace filamesh;
using namespace ktxreader;

static constexpr float OBJECT_SCALE = 0.02f;

FilamentApp::FilamentApp(void* nativeLayer, uint32_t width, uint32_t height)
        : nativeLayer(nativeLayer), width(width), height(height) {
    setupFilament();
    setupCameraFeedTriangle();
    setupIbl();
    setupMaterial();
    setupMesh();
    setupView();
}

void FilamentApp::render(const FilamentArFrame& frame) {
    app.cameraFeedTriangle->setCameraFeedTexture(frame.cameraImage);
    app.cameraFeedTriangle->setCameraFeedTransform(frame.cameraTextureTransform);
    camera->setModelMatrix(frame.view);
    camera->setCustomProjection(frame.projection, 0.01, 10);

    // Rotate the mesh a little bit each frame.
    meshRotation *= quatf::fromAxisAngle(float3{1.0f, 0.5f, 0.2f}, 0.05);

    // Update the mesh's transform.
    auto& tcm = engine->getTransformManager();
    auto i = tcm.getInstance(app.renderable);
    tcm.setTransform(i,
                     meshTransform *
                     mat4f(meshRotation) *
                     mat4f::scaling(OBJECT_SCALE));

    if (renderer->beginFrame(swapChain)) {
        renderer->render(view);
        renderer->endFrame();
    }
}

void FilamentApp::setObjectTransform(const mat4f& transform) {
    meshTransform = transform;
}

void FilamentApp::updatePlaneGeometry(const FilamentArPlaneGeometry& geometry) {
    auto& tcm = engine->getTransformManager();

    if (!app.planeGeometry.isNull()) {
        scene->remove(app.planeGeometry);
        engine->destroy(app.planeGeometry);
        tcm.destroy(app.planeGeometry);
        EntityManager::get().destroy(1, &app.planeGeometry);
    }

    if (app.planeVertices) {
        engine->destroy(app.planeVertices);
    }

    if (app.planeIndices) {
        engine->destroy(app.planeIndices);
    }

    if (!app.shadowPlane) {
        app.shadowPlane = Material::Builder()
            .package(RESOURCES_SHADOW_PLANE_DATA, RESOURCES_SHADOW_PLANE_SIZE)
            .build(*engine);
    }

    const size_t vertexCount = geometry.vertexCount;
    const size_t indexCount = geometry.indexCount;

    // Generate a surface tangent quaternion for each vertex. Since every vertex has the same
    // upwards-facing normal, we only need to generate a single quaternion, which can be copied.
    quatf* quats = new quatf[vertexCount];
    static float3 normals[1] = { float3(0, 1, 0) };
    auto helper = geometry::SurfaceOrientation::Builder()
        .vertexCount(1)
        .normals(normals)
        .build();
    helper->getQuats(quats, 1);
    delete helper;
    for (int i = 1; i < vertexCount; i++) {
        quats[i] = quats[0];
    }

    // Copy the position and index buffers. The buffers provided to us by ARKit aren't guaranteed to
    // persist indefinitely.
    float4* verts = (float4*) new uint8_t[vertexCount * sizeof(float4)];
    uint16_t* indices = (uint16_t*) new uint8_t[indexCount * sizeof(uint16_t)];
    std::copy(geometry.vertices, geometry.vertices + vertexCount, verts);
    std::copy(geometry.indices, geometry.indices + indexCount, indices);

    app.planeVertices = VertexBuffer::Builder()
        .vertexCount((uint32_t) vertexCount)
        .bufferCount(2)
    // The position buffer only has x y and z coordinates, but has the same padding as a float4.
    .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3, 0, sizeof(float4))
    .attribute(VertexAttribute::TANGENTS, 1, VertexBuffer::AttributeType::FLOAT4, 0, sizeof(quatf))
    .build(*engine);

    app.planeIndices = IndexBuffer::Builder()
        .indexCount((uint32_t) geometry.indexCount)
        .bufferType(IndexBuffer::IndexType::USHORT)
        .build(*engine);

    const auto deleter = [](void* buffer, size_t size, void* user) {
        delete (uint8_t*) buffer;
    };

    VertexBuffer::BufferDescriptor positionBuffer(verts, vertexCount * sizeof(float4), deleter);
    VertexBuffer::BufferDescriptor tangentbuffer(quats, vertexCount * sizeof(quatf), deleter);
    IndexBuffer::BufferDescriptor indexBuffer(indices, indexCount * sizeof(uint16_t), deleter);
    app.planeVertices->setBufferAt(*engine, 0, std::move(positionBuffer));
    app.planeVertices->setBufferAt(*engine, 1, std::move(tangentbuffer));
    app.planeIndices->setBuffer(*engine, std::move(indexBuffer));

    Box aabb = RenderableManager::computeAABB((float4*) geometry.vertices,
            (uint16_t*) geometry.indices, geometry.vertexCount);
    EntityManager::get().create(1, &app.planeGeometry);
    RenderableManager::Builder(1)
        .boundingBox(aabb)
        .receiveShadows(true)
        .material(0, app.shadowPlane->getDefaultInstance())
        .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.planeVertices,
                app.planeIndices, 0, geometry.indexCount)
        .build(*engine, app.planeGeometry);

    // Allow the ground plane to receive shadows.
    auto& rcm = engine->getRenderableManager();
    rcm.setReceiveShadows(rcm.getInstance(app.planeGeometry), true);

    tcm.create(app.planeGeometry);
    auto i = tcm.getInstance(app.planeGeometry);
    tcm.setTransform(i, geometry.transform);

    scene->addEntity(app.planeGeometry);
}

FilamentApp::~FilamentApp() {
    delete app.cameraFeedTriangle;

    engine->destroy(app.materialInstance);
    engine->destroy(app.mat);
    engine->destroy(app.indirectLight);
    engine->destroy(app.iblTexture);
    engine->destroy(app.renderable);
    engine->destroy(app.sun);
    engine->destroy(app.shadowPlane);

    engine->destroy(renderer);
    engine->destroy(scene);
    engine->destroy(view);
    Entity c = camera->getEntity();
    engine->destroyCameraComponent(c);
    EntityManager::get().destroy(c);
    engine->destroy(swapChain);
    engine->destroy(&engine);
}

void FilamentApp::setupFilament() {
#if FILAMENT_APP_USE_OPENGL
    engine = Engine::create(filament::Engine::Backend::OPENGL);
#elif FILAMENT_APP_USE_METAL
    engine = Engine::create(filament::Engine::Backend::METAL);
#endif
    swapChain = engine->createSwapChain(nativeLayer);
    renderer = engine->createRenderer();
    scene = engine->createScene();
    Entity c = EntityManager::get().create();
    camera = engine->createCamera(c);
    camera->setProjection(60, (float) width / height, 0.1, 10);
}

void FilamentApp::setupIbl() {
    image::Ktx1Bundle* iblBundle = new image::Ktx1Bundle(RESOURCES_VENETIAN_CROSSROADS_2K_IBL_DATA,
                                                       RESOURCES_VENETIAN_CROSSROADS_2K_IBL_SIZE);
    float3 harmonics[9];
    iblBundle->getSphericalHarmonics(harmonics);
    app.iblTexture = Ktx1Reader::createTexture(engine, iblBundle, false);

    app.indirectLight = IndirectLight::Builder()
        .reflections(app.iblTexture)
        .irradiance(3, harmonics)
        .intensity(30000)
        .build(*engine);
    scene->setIndirectLight(app.indirectLight);

    app.sun = EntityManager::get().create();
    LightManager::Builder(LightManager::Type::SUN)
        .castShadows(true)
        .direction({0.0, -1.0, 0.0})
        .build(*engine, app.sun);
    scene->addEntity(app.sun);
}

void FilamentApp::setupMaterial() {
    app.mat = Material::Builder()
        .package(RESOURCES_CLEAR_COAT_DATA, RESOURCES_CLEAR_COAT_SIZE)
        .build(*engine);
    app.materialInstance = app.mat->createInstance();
}

void FilamentApp::setupMesh() {
    MeshReader::Mesh mesh = MeshReader::loadMeshFromBuffer(engine, RESOURCES_CUBE_DATA,
            nullptr, nullptr, app.materialInstance);
    app.materialInstance->setParameter("baseColor", RgbType::sRGB, {0.71f, 0.0f, 0.0f});

    app.renderable = mesh.renderable;
    scene->addEntity(app.renderable);

    // Allow the mesh to cast shadows onto the shadow plane.
    auto& rcm = engine->getRenderableManager();
    rcm.setCastShadows(rcm.getInstance(app.renderable), true);

    // Position the mesh 2 units down the Z axis.
    auto& tcm = engine->getTransformManager();
    tcm.create(app.renderable);
    auto i = tcm.getInstance(app.renderable);
    tcm.setTransform(i,
            mat4f::translation(float3{0.0f, 0.0f, -2.0f}) *
            mat4f::scaling(OBJECT_SCALE));
}

void FilamentApp::setupView() {
    view = engine->createView();
    view->setScene(scene);
    view->setCamera(camera);
    view->setViewport(Viewport(0, 0, width, height));
}

void FilamentApp::setupCameraFeedTriangle() {
    app.cameraFeedTriangle = new FullScreenTriangle(engine);
    scene->addEntity(app.cameraFeedTriangle->getEntity());
}
