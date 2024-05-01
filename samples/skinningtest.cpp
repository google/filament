/*
 * Copyright (C) 2023 The Android Open Source Project
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
#include <utils/Path.h>

#include <getopt/getopt.h>

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>

#include <cmath>
#include <iostream>

#include "generated/resources/resources.h"

using namespace filament;
using utils::Entity;
using utils::EntityManager;
using utils::Path;
using utils::FixedCapacityVector;
using namespace filament::math;

struct App {
    VertexBuffer* vbs[10];
    size_t vbCount = 0;
    IndexBuffer *ib, *ib2;
    Material* mat;
    Camera* cam;
    Entity camera;
    Skybox* skybox;
    Entity renderables[4];
    SkinningBuffer *sb, *sb2;
    MorphTargetBuffer *mt;
    BufferObject* bos[10];
    size_t boCount = 0;
    size_t bonesPerVertex;
    FixedCapacityVector<FixedCapacityVector<filament::math::float2>>
        boneDataPerPrimitive,
        boneDataPerPrimitiveMulti;
};

struct Vertex {
    float2 position;
    uint32_t color;
};

static const Vertex TRIANGLE_VERTICES_1[6] = {
    {{ 1, 0}, 0xff00ff00u},
    {{ cos(M_PI * 1 / 3), sin(M_PI * 1 / 3)}, 0xff330088u},
    {{ cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff880033u},
    {{-1, 0}, 0xff00ff00u},
    {{ cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xff330088u},
    {{ cos(M_PI * 5 / 3), sin(M_PI * 5 / 3)}, 0xff880033u},
};

static const Vertex TRIANGLE_VERTICES_2[6] = {
    {{ 1, 0}, 0xff0000ffu},
    {{ cos(M_PI * 1 / 3), sin(M_PI * 1 / 3)}, 0xfff055ffu},
    {{ cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff880088u},
    {{-1, 0}, 0xff0000ffu},
    {{ cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xfff055ffu},
    {{ cos(M_PI * 5 / 3), sin(M_PI * 5 / 3)}, 0xff880088u},
};

static const Vertex TRIANGLE_VERTICES_3[6] = {
    {{ 1, 0}, 0xfff00f88u},
    {{ cos(M_PI * 1 / 3), sin(M_PI * 1 / 3)}, 0xff00ffaau},
    {{ cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff00ffffu},
    {{-1, 0}, 0xfff00f88u},
    {{ cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xff00ffaau},
    {{ cos(M_PI * 5 / 3), sin(M_PI * 5 / 3)}, 0xff00ffffu},
};


static const float3 targets_pos[9] = {
  { -2, 0, 0},{ 0, 2, 0},{ 1, 0, 0},
  { 1, 1, 0},{ -1, 0, 0},{ -1, 0, 0},
  { 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0}
};

static const short4 targets_tan[9] = {
  { 0, 0, 0, 0},{ 0, 0, 0, 0},{ 0, 0, 0, 0},
  { 0, 0, 0, 0},{ 0, 0, 0, 0},{ 0, 0, 0, 0},
  { 0, 0, 0, 0},{ 0, 0, 0, 0},{ 0, 0, 0, 0}};

static const uint16_t skinJoints[] = { 0, 1, 2, 5,
                                     0, 2, 3, 5,
                                     0, 3, 1, 5};

static const float skinWeights[] = { 0.5f, 0.0f, 0.0f, 0.5f,
                                     0.5f, 0.0f, 0.f, 0.5f,
                                     0.5f, 0.0f, 0.f, 0.5f,};

static float2 boneDataArray[48] = {}; //indices and weights for up to 3 vertices with 8 bones

static constexpr uint16_t TRIANGLE_INDICES[3] = { 0, 1, 2 },
TRIANGLE_INDICES_2[6] = { 0, 2, 4, 1, 3, 5 };

mat4f transforms[] = {math::mat4f(1),
                      mat4f::translation(float3(1, 0, 0)),
                      mat4f::translation(float3(1, 1, 0)),
                      mat4f::translation(float3(0, 1, 0)),
                      mat4f::translation(float3(-1, 1, 0)),
                      mat4f::translation(float3(-1, 0, 0)),
                      mat4f::translation(float3(-1, -1, 0)),
                      mat4f::translation(float3(0, -1, 0)),
                      mat4f::translation(float3(1, -1, 0))};


static void printUsage(char* name) {
    std::string exec_name(Path(name).getName());
    std::string usage(
            "SAMPLE is a command-line tool for testing Filament skinning buffers.\n"
            "Usage:\n"
            "    SAMPLE [options]\n"
            "Options:\n"
            "   --help, -h\n"
            "       Prints this message\n\n"
            "   --api, -a\n"
            "       Specify the backend API: opengl (default), vulkan, or metal\n\n"
    );
    const std::string from("HEIGHTFIELD");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    std::cout << usage;
}

static int handleCommandLineArgments(int argc, char* argv[], Config* config) {
    static constexpr const char* OPTSTR = "ha:";
    static const struct option OPTIONS[] = {
            { "help",         no_argument,       nullptr, 'h' },
            { "api",          required_argument, nullptr, 'a' },
            { nullptr, 0, nullptr, 0 }  // termination of the option list
    };
    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, OPTSTR, OPTIONS, &option_index)) >= 0) {
        std::string arg(optarg != nullptr ? optarg : "");
        switch (opt) {
            default:
            case 'h':
                printUsage(argv[0]);
                exit(0);
            case 'a':
                if (arg == "opengl") {
                    config->backend = Engine::Backend::OPENGL;
                } else if (arg == "vulkan") {
                    config->backend = Engine::Backend::VULKAN;
                } else if (arg == "metal") {
                    config->backend = Engine::Backend::METAL;
                } else {
                    std::cerr << "Unrecognized backend. Must be 'opengl'|'vulkan'|'metal'."
                              << std::endl;
                }
                break;
        }
    }

    return optind;
}

int main(int argc, char** argv) {
    App app;

    app.boneDataPerPrimitive = FixedCapacityVector<FixedCapacityVector<float2>>(3);
    app.boneDataPerPrimitiveMulti = FixedCapacityVector<FixedCapacityVector<float2>>(6);
    app.bonesPerVertex = 8;

    Config config;
    config.title = "skinning test with more than 4 bones per vertex";

    handleCommandLineArgments(argc, argv, &config);

    size_t boneCount = app.bonesPerVertex;
    float weight = 1.f / boneCount;
    FixedCapacityVector<float2> boneDataPerVertex(boneCount);
    for (size_t idx = 0; idx < boneCount; idx++) {
        boneDataPerVertex[idx] = float2(idx, weight);
        boneDataArray[idx] = float2(idx, weight);
        boneDataArray[idx + boneCount] = float2(idx, weight);
        boneDataArray[idx + 2 * boneCount] = float2(idx, weight);
        boneDataArray[idx + 3 * boneCount] = float2(idx, weight);
        boneDataArray[idx + 4 * boneCount] = float2(idx, weight);
        boneDataArray[idx + 5 * boneCount] = float2(idx, weight);
    }

    auto idx = 0;
    app.boneDataPerPrimitive[idx++] = boneDataPerVertex;
    app.boneDataPerPrimitive[idx++] = boneDataPerVertex;
    app.boneDataPerPrimitive[idx++] = boneDataPerVertex;

    for (size_t vertex_idx = 0; vertex_idx < 6; vertex_idx++) {
      boneCount = vertex_idx % app.bonesPerVertex + 1;
      weight = 1.f / boneCount;
      FixedCapacityVector<float2> boneDataPerVertex1(boneCount);
      for (size_t idx = 0; idx < boneCount; idx++) {
          boneDataPerVertex1[idx] = float2(idx, weight);
      }
      app.boneDataPerPrimitiveMulti[vertex_idx] = boneDataPerVertex1;
    }

    auto setup =
        [&app](Engine* engine, View* view, Scene* scene) {
        app.skybox = Skybox::Builder().color({ 0.1, 0.125, 0.25, 1.0})
            .build(*engine);

        scene->setSkybox(app.skybox);
        view->setPostProcessingEnabled(false);
        static_assert(sizeof(Vertex) == 12, "Strange vertex size.");

        // primitives for renderable 0 -------------------------
        // primitive 0/1, triangle without skinning
        app.vbs[app.vbCount] = VertexBuffer::Builder()
            .vertexCount(3)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0,
                       VertexBuffer::AttributeType::FLOAT2, 0, 12)
            .attribute(VertexAttribute::COLOR, 0,
                       VertexBuffer::AttributeType::UBYTE4, 8, 12)
            .normalized(VertexAttribute::COLOR)
            .build(*engine);
        app.vbs[app.vbCount]->setBufferAt(*engine, 0,
            VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES_1, 36,
                                           nullptr));
        app.vbCount++;

        // primitive 0/2, triangle without skinning, buffer objects enabled
        app.vbs[app.vbCount] = VertexBuffer::Builder()
            .vertexCount(3)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0,
                       VertexBuffer::AttributeType::FLOAT2, 0, 12)
            .attribute(VertexAttribute::COLOR, 0,
                       VertexBuffer::AttributeType::UBYTE4, 8, 12)
            .normalized(VertexAttribute::COLOR)
            .enableBufferObjects()
            .build(*engine);
        app.bos[app.boCount] = BufferObject::Builder()
            .size(3 * sizeof(Vertex))
            .build(*engine);
        app.bos[app.boCount]->setBuffer(*engine, BufferObject::BufferDescriptor(
            TRIANGLE_VERTICES_1 + 3, app.bos[app.boCount]->getByteCount(),
            nullptr));
        app.vbs[app.vbCount]->setBufferObjectAt(*engine, 0,
                                                app.bos[app.boCount]);
        app.vbCount++;
        app.boCount++;

        // primitives for renderable 1 -------------------------
        // primitive 1/1, triangle with skinning vertex attributes (only 4 bones),
        // buffer object disabled
        app.vbs[app.vbCount] = VertexBuffer::Builder()
            .vertexCount(3)
            .bufferCount(3)
            .attribute(VertexAttribute::POSITION, 0,
                       VertexBuffer::AttributeType::FLOAT2, 0, 12)
            .attribute(VertexAttribute::COLOR, 0,
                       VertexBuffer::AttributeType::UBYTE4, 8, 12)
            .normalized(VertexAttribute::COLOR)
            .attribute(VertexAttribute::BONE_INDICES, 1,
                       VertexBuffer::AttributeType::USHORT4, 0, 8)
            .attribute(VertexAttribute::BONE_WEIGHTS, 2,
                       VertexBuffer::AttributeType::FLOAT4, 0, 16)
            .build(*engine);
        app.vbs[app.vbCount]->setBufferAt(*engine, 0,
            VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES_2, 36,
                                           nullptr));
        app.vbs[app.vbCount]->setBufferAt(*engine, 1,
            VertexBuffer::BufferDescriptor(skinJoints, 24, nullptr));
        app.vbs[app.vbCount]->setBufferAt(*engine, 2,
            VertexBuffer::BufferDescriptor(skinWeights, 48, nullptr));
        app.vbCount++;

        // primitive 1/2, triangle with skinning vertex attributes (only 4 bones),
        // buffer objects enabled
        app.vbs[app.vbCount] = VertexBuffer::Builder()
            .vertexCount(3)
            .bufferCount(3)
            .attribute(VertexAttribute::POSITION, 0,
                       VertexBuffer::AttributeType::FLOAT2, 0, 12)
            .attribute(VertexAttribute::COLOR, 0,
                       VertexBuffer::AttributeType::UBYTE4, 8, 12)
            .normalized(VertexAttribute::COLOR)
            .attribute(VertexAttribute::BONE_INDICES, 1,
                       VertexBuffer::AttributeType::USHORT4, 0, 8)
            .attribute(VertexAttribute::BONE_WEIGHTS, 2,
                       VertexBuffer::AttributeType::FLOAT4, 0, 16)
            .enableBufferObjects()
            .build(*engine);
        app.bos[app.boCount] = BufferObject::Builder()
            .size(3 * sizeof(Vertex))
            .build(*engine);
        app.bos[app.boCount]->setBuffer(*engine, BufferObject::BufferDescriptor(
            TRIANGLE_VERTICES_2 + 2, app.bos[app.boCount]->getByteCount(),
            nullptr));
        app.vbs[app.vbCount]->setBufferObjectAt(*engine, 0,
                                                app.bos[app.boCount]);
        app.boCount++;
        app.bos[app.boCount] = BufferObject::Builder()
            .size(24)
            .build(*engine);
        app.bos[app.boCount]->setBuffer(*engine, BufferObject::BufferDescriptor(
            skinJoints, app.bos[app.boCount]->getByteCount(), nullptr));
        app.vbs[app.vbCount]->setBufferObjectAt(*engine, 1,
                                                app.bos[app.boCount]);
        app.boCount++;
        app.bos[app.boCount] = BufferObject::Builder()
            .size(48)
            .build(*engine);
        app.bos[app.boCount]->setBuffer(*engine, BufferObject::BufferDescriptor(
            skinWeights, app.bos[app.boCount]->getByteCount(), nullptr));
        app.vbs[app.vbCount]->setBufferObjectAt(*engine, 2,
                                                app.bos[app.boCount]);
        app.boCount++;
        app.vbCount++;

        // primitives for renderable 2 -------------------------
        // primitive 2/1, triangle with advanced skinning, buffer objects enabled
        app.vbs[app.vbCount] = VertexBuffer::Builder()
            .vertexCount(3)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0,
                       VertexBuffer::AttributeType::FLOAT2, 0, 12)
            .attribute(VertexAttribute::COLOR, 0,
                       VertexBuffer::AttributeType::UBYTE4, 8, 12)
            .normalized(VertexAttribute::COLOR)
            .enableBufferObjects()
            .advancedSkinning(true)
            .build(*engine);
        app.bos[app.boCount] = BufferObject::Builder()
            .size(3 * sizeof(Vertex))
            .build(*engine);
        app.bos[app.boCount]->setBuffer(*engine, BufferObject::BufferDescriptor(
            TRIANGLE_VERTICES_3, app.bos[app.boCount]->getByteCount(), nullptr));
        app.vbs[app.vbCount]->setBufferObjectAt(*engine, 0,
                                                app.bos[app.boCount]);
        app.boCount++;
        app.vbCount++;

        // primitive 2/2, triangle with advanced skinning, buffer objects enabled, for morph
        app.vbs[app.vbCount] = VertexBuffer::Builder()
            .vertexCount(3)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0,
                       VertexBuffer::AttributeType::FLOAT2, 0, 12)
            .attribute(VertexAttribute::COLOR, 0,
                       VertexBuffer::AttributeType::UBYTE4, 8, 12)
            .normalized(VertexAttribute::COLOR)
            .enableBufferObjects()
            .advancedSkinning(true)
            .build(*engine);
        app.bos[app.boCount] = BufferObject::Builder()
            .size(3 * sizeof(Vertex))
            .build(*engine);
        app.bos[app.boCount]->setBuffer(*engine, BufferObject::BufferDescriptor(
            TRIANGLE_VERTICES_3 + 1, app.bos[app.boCount]->getByteCount(),
            nullptr));
        app.vbs[app.vbCount]->setBufferObjectAt(*engine, 0,
                                                app.bos[app.boCount]);
        app.boCount++;
        app.vbCount++;

        // primitive 2/3, triangle with advanced skinning, buffer objects enabled,
        app.vbs[app.vbCount] = VertexBuffer::Builder()
            .vertexCount(3)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0,
                       VertexBuffer::AttributeType::FLOAT2, 0, 12)
            .attribute(VertexAttribute::COLOR, 0,
                       VertexBuffer::AttributeType::UBYTE4, 8, 12)
            .normalized(VertexAttribute::COLOR)
            .enableBufferObjects()
            .advancedSkinning(true)
            .build(*engine);
        app.bos[app.boCount] = BufferObject::Builder()
            .size(3 * sizeof(Vertex))
            .build(*engine);
        app.bos[app.boCount]->setBuffer(*engine, BufferObject::BufferDescriptor(
            TRIANGLE_VERTICES_3 + 2, app.bos[app.boCount]->getByteCount(),
            nullptr));
        app.vbs[app.vbCount]->setBufferObjectAt(*engine, 0,
                                                app.bos[app.boCount]);
        app.boCount++;
        app.vbCount++;

        // primitives for renderable 3 -------------------------
        // primitive 3/1, two triangles with advanced skinning, buffer objects enabled,
        app.vbs[app.vbCount] = VertexBuffer::Builder()
            .vertexCount(6)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0,
                       VertexBuffer::AttributeType::FLOAT2, 0, 12)
            .attribute(VertexAttribute::COLOR, 0,
                       VertexBuffer::AttributeType::UBYTE4, 8, 12)
            .normalized(VertexAttribute::COLOR)
            .enableBufferObjects()
            .advancedSkinning(true)
            .build(*engine);
        app.bos[app.boCount] = BufferObject::Builder()
            .size(6 * sizeof(Vertex))
            .build(*engine);
        app.bos[app.boCount]->setBuffer(*engine, BufferObject::BufferDescriptor(
                          TRIANGLE_VERTICES_1, app.bos[app.boCount]->getByteCount(),
                          nullptr));
        app.vbs[app.vbCount]->setBufferObjectAt(*engine, 0,
                                                app.bos[app.boCount]);
        app.boCount++;
        app.vbCount++;
        // primitive 3/2, triangle with advanced skinning and morph, buffer objects enabled,
        app.vbs[app.vbCount] = VertexBuffer::Builder()
            .vertexCount(3)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0,
                       VertexBuffer::AttributeType::FLOAT2, 0, 12)
            .attribute(VertexAttribute::COLOR, 0,
                       VertexBuffer::AttributeType::UBYTE4, 8, 12)
            .normalized(VertexAttribute::COLOR)
            .enableBufferObjects()
            .advancedSkinning(true)
            .build(*engine);
        app.bos[app.boCount] = BufferObject::Builder()
            .size(3 * sizeof(Vertex))
            .build(*engine);
        app.bos[app.boCount]->setBuffer(*engine, BufferObject::BufferDescriptor(
                        TRIANGLE_VERTICES_3 + 2, app.bos[app.boCount]->getByteCount(),
                        nullptr));
        app.vbs[app.vbCount]->setBufferObjectAt(*engine, 0,
                                                app.bos[app.boCount]);
        app.boCount++;
        app.vbCount++;

 // Index buffer data
        app.ib = IndexBuffer::Builder()
            .indexCount(3)
            .bufferType(IndexBuffer::IndexType::USHORT)
            .build(*engine);
        app.ib->setBuffer(*engine,
            IndexBuffer::BufferDescriptor(TRIANGLE_INDICES,
                                          3 * sizeof(uint16_t),nullptr));

        app.ib2 = IndexBuffer::Builder()
            .indexCount(6)
            .bufferType(IndexBuffer::IndexType::USHORT)
            .build(*engine);
        app.ib2->setBuffer(*engine,
            IndexBuffer::BufferDescriptor(TRIANGLE_INDICES_2,
                                          6 * sizeof(uint16_t),nullptr));

        app.mat = Material::Builder()
            .package(RESOURCES_BAKEDCOLOR_DATA, RESOURCES_BAKEDCOLOR_SIZE)
            .build(*engine);

// Skinning buffer for renderable 2
        app.sb = SkinningBuffer::Builder()
            .boneCount(9)
            .initialize(true)
            .build(*engine);

// Skinning buffer common for renderable 3
        app.sb2 = SkinningBuffer::Builder()
            .boneCount(9)
            .initialize(true)
            .build(*engine);

        app.sb->setBones(*engine, transforms,9,0);

// Morph target definition to check combination bone skinning and blend shapes
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

// renderable 0: no skinning
// primitive 0 = triangle, no skinning, no morph target
// primitive 1 = triangle, no skinning, morph target
// primitive 2 = triangle, no skinning, no morph target, buffer objects enabled
// primitive 3 = triangle, no skinning, morph target, buffer objects enabled
        app.renderables[0] = EntityManager::get().create();
        RenderableManager::Builder(4)
            .boundingBox({{ -1, -1, -1}, { 1, 1, 1}})
            .material(0, app.mat->getDefaultInstance())
            .material(1, app.mat->getDefaultInstance())
            .material(2, app.mat->getDefaultInstance())
            .material(3, app.mat->getDefaultInstance())
            .geometry(0,RenderableManager::PrimitiveType::TRIANGLES,
                app.vbs[0],app.ib,0,3)
            .geometry(1,RenderableManager::PrimitiveType::TRIANGLES,
                app.vbs[0],app.ib,0,3)
            .geometry(2,RenderableManager::PrimitiveType::TRIANGLES,
                app.vbs[1],app.ib,0,3)
            .geometry(3,RenderableManager::PrimitiveType::TRIANGLES,
                app.vbs[1],app.ib,0,3)
            .culling(false)
            .receiveShadows(false)
            .castShadows(false)
            .morphing(3)
            .morphing(0,1,app.mt)
            .morphing(0,3,app.mt)
            .build(*engine, app.renderables[0]);

// renderable 1: attribute bone data definitions skinning
// primitive 0 = triangle with skinning and with morphing, bone data defined as vertex attributes (buffer object)
// primitive 1 = trinagle with skinning, bone data defined as vertex attributes
// primitive 3 = triangle with skinning, bone data defined as vertex attributes (buffer object)
// primitive 2 = triangle with skinning and with morphing, bone data defined as vertex attributes
        app.renderables[1] = EntityManager::get().create();
        RenderableManager::Builder(4)
            .boundingBox({{ -1, -1, -1}, { 1, 1, 1}})
            .material(0, app.mat->getDefaultInstance())
            .material(1, app.mat->getDefaultInstance())
            .material(2, app.mat->getDefaultInstance())
            .material(3, app.mat->getDefaultInstance())
            .geometry(1,RenderableManager::PrimitiveType::TRIANGLES,
                app.vbs[2],app.ib,0,3)
            .geometry(2,RenderableManager::PrimitiveType::TRIANGLES,
                app.vbs[2],app.ib,0,3)
            .geometry(0,RenderableManager::PrimitiveType::TRIANGLES,
                app.vbs[3],app.ib,0,3)
            .geometry(3,RenderableManager::PrimitiveType::TRIANGLES,
                app.vbs[3],app.ib,0,3)
            .culling(false)
            .receiveShadows(false)
            .castShadows(false)
            .enableSkinningBuffers(true)
            .skinning(app.sb, 9, 0)
            .morphing(3)
            .morphing(0,2,app.mt)
            .morphing(0,0,app.mt)
            .build(*engine, app.renderables[1]);

// renderable 2: various ways of skinning definitions
// primitive 0 = skinned triangle, advanced bone data defined as array per primitive,
// primitive 1 = skinned triangle, advanced bone data defined as vector per primitive,
// primitive 2 = triangle with skinning and with morphing, advanced bone data
//               defined as vector per primitive
        app.renderables[2] = EntityManager::get().create();
        RenderableManager::Builder(3)
            .boundingBox({{ -1, -1, -1}, { 1, 1, 1}})
            .material(0, app.mat->getDefaultInstance())
            .material(1, app.mat->getDefaultInstance())
            .material(2, app.mat->getDefaultInstance())
            .geometry(0,RenderableManager::PrimitiveType::TRIANGLES,
                app.vbs[4], app.ib, 0, 3)
            .geometry(1,RenderableManager::PrimitiveType::TRIANGLES,
                app.vbs[5], app.ib, 0, 3)
            .geometry(2,RenderableManager::PrimitiveType::TRIANGLES,
                app.vbs[6], app.ib, 0, 3)
            .culling(false)
            .receiveShadows(false)
            .castShadows(false)
            .enableSkinningBuffers(true)
            .skinning(app.sb, 9, 0)

            .boneIndicesAndWeights(0, boneDataArray,
                                   3 * app.bonesPerVertex, app.bonesPerVertex)
            .boneIndicesAndWeights(1, app.boneDataPerPrimitive)
            .boneIndicesAndWeights(2, app.boneDataPerPrimitive)

            .morphing(3)
            .morphing(0, 2, app.mt)
            .build(*engine, app.renderables[2]);

// renderable 3: combination attribute and advance bone data
// primitive 0 = triangle with skinning and morphing, bone data defined as vertex attributes
// primitive 1 = skinning of two triangles, advanced bone data defined as vector per primitive,
//               various number of bones per vertex 1, 2, ... 6
// primitive 2 = triangle with skinning and morphing, advanced bone data defined
//               as vector per primitive
        app.renderables[3] = EntityManager::get().create();
        RenderableManager::Builder(3)
            .boundingBox({{ -1, -1, -1}, { 1, 1, 1}})
            .material(0, app.mat->getDefaultInstance())
            .material(1, app.mat->getDefaultInstance())
            .material(2, app.mat->getDefaultInstance())
            .geometry(0,RenderableManager::PrimitiveType::TRIANGLES,
                app.vbs[2], app.ib, 0, 3)
            .geometry(1,RenderableManager::PrimitiveType::TRIANGLES,
                app.vbs[7], app.ib2, 0, 6)
            .geometry(2,RenderableManager::PrimitiveType::TRIANGLES,
                app.vbs[8], app.ib, 0, 3)
            .culling(false)
            .receiveShadows(false)
            .castShadows(false)
            .enableSkinningBuffers(true)
            .skinning(app.sb, 9, 0)
            .boneIndicesAndWeights(1, app.boneDataPerPrimitiveMulti)
            .boneIndicesAndWeights(2, app.boneDataPerPrimitive)
            .morphing(3)
            .morphing(0,0,app.mt)
            .morphing(0,2,app.mt)
            .build(*engine, app.renderables[3]);

        scene->addEntity(app.renderables[0]);
        scene->addEntity(app.renderables[1]);
        scene->addEntity(app.renderables[2]);
        scene->addEntity(app.renderables[3]);
        app.camera = EntityManager::get().create();
        app.cam = engine->createCamera(app.camera);
        view->setCamera(app.cam);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        engine->destroy(app.skybox);
        engine->destroy(app.mat);
        engine->destroy(app.ib);
        engine->destroy(app.ib2);
        engine->destroy(app.sb);
        engine->destroy(app.sb2);
        engine->destroy(app.mt);
        engine->destroyCameraComponent(app.camera);
        EntityManager::get().destroy(app.camera);
        for (auto i = 0; i < app.vbCount; i++) {
            engine->destroy(app.vbs[i]);
        }
        for ( auto i = 0; i < app.boCount; i++) {
            engine->destroy(app.bos[i]);
        }
        for ( auto i = 0; i < 4; i++) {
            engine->destroy(app.renderables[i]);
        }
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

        // Bone skinning animation for more than four bones per vertex
        float t = (float)(now - (int)now);
        size_t offset = ((size_t)now) % 9;
        float s = sin(t * f::PI * 2.f) * 10;
        mat4f trans[9] = {};
        for (size_t i = 0; i < 9; i++) {
            trans[i] = filament::math::mat4f(1);
        }
        mat4f trans2[9] = {};
        for (size_t i = 0; i < 9; i++) {
            trans2[i] = filament::math::mat4f(1);
        }
        mat4f transA[] = {
            mat4f::scaling(float3(s / 10.f,s / 10.f, 1.f)),
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
        trans2[offset] = transA[(offset + 3) % 9];

        app.sb->setBones(*engine,trans, 9, 0);
        app.sb2->setBones(*engine,trans2, 9, 0);

        // Morph targets (blendshapes) animation
        float z = (float)(sin(now)/2.f + 0.5f);
        float weights[] = { 1 - z, 0, z};
        rm.setMorphWeights(rm.getInstance(app.renderables[0]), weights, 3, 0);
        rm.setMorphWeights(rm.getInstance(app.renderables[1]), weights, 3, 0);
        rm.setMorphWeights(rm.getInstance(app.renderables[2]), weights, 3, 0);
        rm.setMorphWeights(rm.getInstance(app.renderables[3]), weights, 3, 0);
    });

    FilamentApp::get().run(config, setup, cleanup);

    return 0;
}
