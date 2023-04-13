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

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/BufferObject.h>
#include <filament/SkinningBuffer.h>
#include <filament/View.h>

#include <utils/EntityManager.h>

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>

#include <cmath>

#include "generated/resources/resources.h"

using namespace filament;
using utils::Entity;
using utils::EntityManager;
using namespace filament::math;

struct App {
    VertexBuffer *vb0, *vb1, *vb2, *vb3, *vb4, *vb5, *vb6;
    IndexBuffer* ib, *ib2;
    Material* mat;
    Camera* cam;
    Entity camera;
    Skybox* skybox;
    Entity renderable1, renderable2, renderable3;
    SkinningBuffer *sb, *sb2;
    MorphTargetBuffer *mt;
    BufferObject *boTriangle, *boVertices, *boJoints, *boWeights;
};

struct Vertex {
    float2 position;
    uint32_t color;
};

static const Vertex TRIANGLE_VERTICES[6] = {
    {{1, 0}, 0xffff0000u},
    {{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff00ff00u},
    {{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xff0000ffu},
    {{0, 1}, 0xffffff00u},
    {{-cos(M_PI * 2 / 3), -sin(M_PI * 2 / 3)}, 0xff00ffffu},
    {{-cos(M_PI * 4 / 3), -sin(M_PI * 4 / 3)}, 0xffff00ffu},
};

static const float3 targets_pos[9] = {
  {-2, 0, 0},{0, 2, 0},{1, 0, 0},
  {1, 1, 0},{-1, 0, 0},{-1, 0, 0},
  {0, 0, 0},{0, 0, 0},{0, 0, 0}
};

static const short4 targets_tan[9] = {
  {0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},
  {0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},
  {0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}
};

static const ushort skinJoints[] = {0,1,2,5,
                                     0,2,3,5,
                                     0,3,1,5
};

static const float skinWeights[] = {0.5f,0.0f,0.0f,0.5f,
                                     0.5f,0.0f,0.f,0.5f,
                                     0.5f,0.0f,0.f,0.5f,
};

static float2 boneDataArray[48] = {}; //index and weight for 3 vertices X 8 bones

static constexpr uint16_t TRIANGLE_INDICES[6] = { 0, 1, 2, 3, 4, 5 };

mat4f transforms[] = {math::mat4f(1),
                      mat4f::translation(float3(1, 0, 0)),
                      mat4f::translation(float3(1, 1, 0)),
                      mat4f::translation(float3(0, 1, 0)),
                      mat4f::translation(float3(-1, 1, 0)),
                      mat4f::translation(float3(-1, 0, 0)),
                      mat4f::translation(float3(-1, -1, 0)),
                      mat4f::translation(float3(0, -1, 0)),
                      mat4f::translation(float3(1, -1, 0))};

std::vector<std::vector<float2>> boneDataPerPrimitive, boneDataPerPrimitive2;
std::vector<std::vector<std::vector<float2>>> boneDataPerRenderable;

int main(int argc, char** argv) {
    Config config;
    config.title = "skinning test with more than 4 bones per vertex";

    std::vector<float2> boneDataPerVertex;
    float w =   1 / 8.f;
    for (uint i = 0; i < 8; i++){
        boneDataArray[i] = float2 (i, w);
        boneDataArray[i + 8] = float2 (i, w);
        boneDataArray[i + 16] = float2 (i, w);
        boneDataPerVertex.push_back(float2 (i, w));
    }
    std::vector<float2> boneDataPerVertex2;
    w =   1 / 3.f;
    for (uint i = 0; i < 3; i++){
        boneDataPerVertex2.push_back(float2 (i, w));
    }

    std::vector<float2> emptyBoneDataPerVertex(0);
    std::vector<std::vector<float2>> emptyBoneDataPerPrimitive(0);
    boneDataPerPrimitive.push_back(boneDataPerVertex);
    boneDataPerPrimitive.push_back(boneDataPerVertex);
    boneDataPerPrimitive.push_back(boneDataPerVertex);

    boneDataPerPrimitive2.push_back(boneDataPerVertex);
    boneDataPerPrimitive2.push_back(boneDataPerVertex2);
    boneDataPerPrimitive2.push_back(boneDataPerVertex);
    boneDataPerPrimitive2.push_back(boneDataPerVertex);
    boneDataPerPrimitive2.push_back(boneDataPerVertex2);
    boneDataPerPrimitive2.push_back(boneDataPerVertex);
//    boneDataPerPrimitive.push_back(emptyBoneDataPerVertex);

    boneDataPerRenderable.push_back(boneDataPerPrimitive);
    boneDataPerRenderable.push_back(boneDataPerPrimitive);
    boneDataPerRenderable.push_back(emptyBoneDataPerPrimitive);

    App app;
    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
        app.skybox = Skybox::Builder().color({0.1, 0.125, 0.25, 1.0}).build(*engine);

        scene->setSkybox(app.skybox);
        view->setPostProcessingEnabled(false);
        static_assert(sizeof(Vertex) == 12, "Strange vertex size.");
        //primitive 0, triangle without skinning vertex attributes
        app.vb0 = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
                .normalized(VertexAttribute::COLOR)
                .build(*engine);

        //primitive 1, triangle without skinning vertex attributes
        app.vb1 = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
                .normalized(VertexAttribute::COLOR)
                .build(*engine);

        //primitive 2, triangle with skinning vertex attributes, buffer object disabled
        app.vb2 = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(3)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
                .normalized(VertexAttribute::COLOR)
                .attribute(VertexAttribute::BONE_INDICES, 1, VertexBuffer::AttributeType::USHORT4, 0, 8)
                .attribute(VertexAttribute::BONE_WEIGHTS, 2, VertexBuffer::AttributeType::FLOAT4, 0, 16)
                //.enableBufferObjects()
                .build(*engine);

        //primitive 3, triangle without skinning vertex attributes, buffer object enabled
        app.vb3 = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
                .normalized(VertexAttribute::COLOR)
                .enableBufferObjects()
                .build(*engine);

        //primitive 4, triangle without skinning vertex attributes
        app.vb4 = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
                .normalized(VertexAttribute::COLOR)
                .build(*engine);

        //primitive 5, two triangles without skinning vertex attributes
        app.vb5 = VertexBuffer::Builder()
                .vertexCount(6)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
                .normalized(VertexAttribute::COLOR)
                .build(*engine);

        //primitive 6, triangle without skinning vertex attributes
        app.vb6 = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
                .normalized(VertexAttribute::COLOR)
                .build(*engine);
//--------data for primitive 0
        app.vb0->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES, 36, nullptr));
//--------data for primitive 1
        app.vb1->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES + 3, 36, nullptr));
        app.vb2->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES + 3, 36, nullptr));

//--------data for primitive 2
        app.vb2->setBufferAt(*engine, 1,
                 VertexBuffer::BufferDescriptor(skinJoints, 24, nullptr));
        app.vb2->setBufferAt(*engine, 2,
                VertexBuffer::BufferDescriptor(skinWeights, 48, nullptr));

//--------data for primitive 3
        app.boTriangle = BufferObject::Builder()
            .size(3 * 4 * sizeof(Vertex))
            .build(*engine);
        app.boTriangle->setBuffer(*engine, BufferObject::BufferDescriptor(
                    TRIANGLE_VERTICES + 2, app.boTriangle->getByteCount(), nullptr));
        app.vb3->setBufferObjectAt(*engine, 0,app.boTriangle);

//--------data for primitive 4
        app.vb4->setBufferAt(*engine, 0,
                             VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES + 1, 36, nullptr),0);
//--------data for primitive 5
        app.vb5->setBufferAt(*engine, 0,
                             VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES + 0, 72, nullptr),0);
//--------data for primitive 6
        app.vb6->setBufferAt(*engine, 0,
                             VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES + 0, 36, nullptr),0);
//index buffer data
        app.ib = IndexBuffer::Builder()
                .indexCount(3)
                .bufferType(IndexBuffer::IndexType::USHORT)
                .build(*engine);
        app.ib->setBuffer(*engine,
                IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, nullptr));

        app.ib2 = IndexBuffer::Builder()
                .indexCount(6)
                .bufferType(IndexBuffer::IndexType::USHORT)
                .build(*engine);
        app.ib2->setBuffer(*engine,
                IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 12, nullptr));

        app.mat = Material::Builder()
                .package(RESOURCES_BAKEDCOLOR_DATA, RESOURCES_BAKEDCOLOR_SIZE)
                .build(*engine);

//skinning buffer for renderable 1
        app.sb = SkinningBuffer::Builder()
            .boneCount(9)
            .initialize(true)
            .build(*engine);

//skinning buffer common for renderable 2 and 3
        app.sb2 = SkinningBuffer::Builder()
            .boneCount(9)
            .initialize(true)
            .build(*engine);

        app.sb->setBones(*engine, transforms,9,0);

//morph target definition to check combination bone skinning and blend shapes
        app.mt = MorphTargetBuffer::Builder()
            .vertexCount(9)
            .count(3)
            .build( *engine);

        app.mt->setPositionsAt(*engine,0, targets_pos, 3, 0);
        app.mt->setPositionsAt(*engine,1, targets_pos+3, 3, 0);
        app.mt->setPositionsAt(*engine,2, targets_pos+6, 3, 0);
        app.mt->setTangentsAt(*engine,0, targets_tan, 9, 0);
        app.mt->setTangentsAt(*engine,1, targets_tan, 9, 0);
        app.mt->setTangentsAt(*engine,2, targets_tan, 9, 0);

//renderable 1:
//primitive 0 = skinned triangle, bone data defined as vector per primitive,
//primitive 1 = skinned triangle, bone data defined as array per primitive,
//primitive 2 = triangle without skinning and with morphing, bone data defined as vertex attributes
        app.renderable1 = EntityManager::get().create();
        RenderableManager::Builder(3)
            .boundingBox({{-1, -1, -1}, {1, 1, 1}})
            .material(0, app.mat->getDefaultInstance())
            .material(1, app.mat->getDefaultInstance())
            .material(2, app.mat->getDefaultInstance())
            .geometry(0,RenderableManager::PrimitiveType::TRIANGLES,
                      app.vb0,app.ib,0,3)
            .geometry(1,RenderableManager::PrimitiveType::TRIANGLES,
                      app.vb3,app.ib,0,3)
            .geometry(2,RenderableManager::PrimitiveType::TRIANGLES,
                      app.vb2,app.ib,0,3)
            .culling(false)
            .receiveShadows(false)
            .castShadows(false)
            .enableSkinningBuffers(true)
            .skinning(app.sb, 9, 0)

            .boneIndicesAndWeights(0, boneDataPerPrimitive)
            .boneIndicesAndWeights(1, boneDataArray, 24)
            .morphing(3)
            .morphing(0,2,app.mt)
            .build(*engine, app.renderable1);

//renderable 2:
//primitive 0 = skinning and morphing triangle, bone data defined as vector for all primitives of renderable,
//primitive 1 = skinning triangle, bone data defined as vector for all primitives of renderable,
        app.renderable2 = EntityManager::get().create();
        RenderableManager::Builder(2)
            .boundingBox({{-1, -1, -1}, {1, 1, 1}})
            .material(0, app.mat->getDefaultInstance())
            .material(1, app.mat->getDefaultInstance())
            .geometry(0,RenderableManager::PrimitiveType::TRIANGLES,
                      app.vb1,app.ib,0,3)
            .geometry(1,RenderableManager::PrimitiveType::TRIANGLES,
                      app.vb6,app.ib,0,3)
            .culling(false)
            .receiveShadows(false)
            .castShadows(false)
            .enableSkinningBuffers(true)
            .skinning(app.sb2, 9, 0)
            .boneIndicesAndWeights(boneDataPerRenderable)
            .morphing(3)
            .morphing(0,0,app.mt)
            .build(*engine, app.renderable2);

//renderable 3:
//primitive 0 = skinning of two triangles, bone data defined as vector with various number of bones per vertex,
//primitive 1 = skinning triangle, bone data defined as vector with various number of bones per vertex,
        app.renderable3 = EntityManager::get().create();
        RenderableManager::Builder(2)
            .boundingBox({{-1, -1, -1}, {1, 1, 1}})
            .material(0, app.mat->getDefaultInstance())
            .material(1, app.mat->getDefaultInstance())
            .geometry(0,RenderableManager::PrimitiveType::TRIANGLES,
                      app.vb5,app.ib2,0,6)
            .geometry(1,RenderableManager::PrimitiveType::TRIANGLES,
                      app.vb4,app.ib,0,3)
            .culling(false)
            .receiveShadows(false)
            .castShadows(false)
            .enableSkinningBuffers(true)
            .skinning(app.sb2, 9, 0)

            .boneIndicesAndWeights( 1, boneDataPerPrimitive)
            .boneIndicesAndWeights( 0, boneDataPerPrimitive2)
            .build(*engine, app.renderable3);

        scene->addEntity(app.renderable1);
        scene->addEntity(app.renderable2);
        scene->addEntity(app.renderable3);
        app.camera = utils::EntityManager::get().create();
        app.cam = engine->createCamera(app.camera);
        view->setCamera(app.cam);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        engine->destroy(app.skybox);
        engine->destroy(app.renderable1);
        engine->destroy(app.renderable2);
        engine->destroy(app.renderable3);
        engine->destroy(app.mat);
        engine->destroy(app.vb0);
        engine->destroy(app.vb1);
        engine->destroy(app.vb2);
        engine->destroy(app.vb3);
        engine->destroy(app.vb4);
        engine->destroy(app.vb5);
        engine->destroy(app.vb6);
        engine->destroy(app.ib);
        engine->destroy(app.ib2);
        engine->destroy(app.sb);
        engine->destroy(app.sb2);
        engine->destroy(app.mt);
        engine->destroy(app.boTriangle);
        engine->destroyCameraComponent(app.camera);
        utils::EntityManager::get().destroy(app.camera);
    };

    FilamentApp::get().animate([&app](Engine* engine, View* view, double now) {
        constexpr float ZOOM = 1.5f;
        const uint32_t w = view->getViewport().width;
        const uint32_t h = view->getViewport().height;
        const float aspect = (float) w / h;
        app.cam->setProjection(Camera::Projection::ORTHO,
            -aspect * ZOOM, aspect * ZOOM,
            -ZOOM, ZOOM, 0, 1);

        auto& rm = engine->getRenderableManager();

        //skinning/bone animation for more than four bones per vertex
        float t = (float)(now - (int)now);
        uint offset = ((uint)now) % 9;
        float s = sin(t * f::PI * 2.f) * 10;
        mat4f trans[9] = {};
        for(uint i = 0; i < 9; i++){
            trans[i] = filament::math::mat4f(1);//transforms[0];
        }
        mat4f trans2[9] = {};
        for(uint i = 0; i < 9; i++){
            trans2[i] = filament::math::mat4f(1);//transforms[0];
        }
        mat4f transA[] = {
            mat4f::scaling(float3(s / 10.f,s / 10.f, 1)),
                //filament::math::mat4f(1),
            mat4f::translation(float3(s, 0, 0)),
            mat4f::translation(float3(s, s, 0)),
            mat4f::translation(float3(0, s, 0)),
            mat4f::translation(float3(-s, s, 0)),
            mat4f::translation(float3(-s, 0, 0)),
            mat4f::translation(float3(-s, -s, 0)),
            mat4f::translation(float3(0, -s, 0)),
            mat4f::translation(float3(s, -s, 0)),
            filament::math::mat4f(1)};
        trans[offset] = transA[offset];
        trans2[offset] = transA[(offset + 0) % 9];

        app.sb->setBones(*engine,trans,9,0);
        app.sb2->setBones(*engine,trans2,9,0);

        //morphTarget/blendshapes animation
        float z = (float)(sin(now)/2.f + 0.5f);
        float weights[] = {1 - z, 0, z};
        rm.setMorphWeights(rm.getInstance(app.renderable1), weights, 3, 0);
        rm.setMorphWeights(rm.getInstance(app.renderable2), weights, 3, 0);

    });

    FilamentApp::get().run(config, setup, cleanup);

    return 0;
}
