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

#include "filamesh.h"

#include <filament/Box.h>
#include <filament/RenderableManager.h>

#include <utils/EntityManager.h>

#include <math/mat4.h>
#include <math/vec3.h>

#include <vector>
#include <string>

using namespace filament;
using namespace math;
using namespace utils;
using namespace std;

struct Header {
    uint32_t version;
    uint32_t parts;
    Box      aabb;
    uint32_t interleaved;
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
    uint32_t materialID;
    Box      aabb;
};

Filamesh decodeMesh(Engine& engine, void const* data, off_t offset, MaterialInstance* mi,
        driver::BufferDescriptor::Callback destructor, void* user) {
    const char* p = (const char *) data + offset;
    if (strncmp("FILAMESH", p, 8)) {
        puts("Magic string not found.");
        abort();
    }
    p += 8;

    Header* header = (Header*) p;
    p += sizeof(Header);

    char const* vertexData = p;
    p += header->vertexSize;

    char const* indices = p;
    p += header->indexSize;

    Part* parts = (Part*) p;
    p += header->parts * sizeof(Part);

    uint32_t materialCount = (uint32_t) *p;
    p += sizeof(uint32_t);

    vector<string> partsMaterial;
    partsMaterial.resize(materialCount);

    for (size_t i = 0; i < materialCount; i++) {
        uint32_t nameLength = (uint32_t) *p;
        p += sizeof(uint32_t);
        partsMaterial[i] = p;
        p += nameLength + 1; // null terminated
    }

    Mesh* mesh = new Mesh();

    mesh->indexBuffer = IndexBuffer::Builder()
            .indexCount(header->indexCount)
            .bufferType(header->indexType ? IndexBuffer::IndexType::USHORT
                                            : IndexBuffer::IndexType::UINT)
            .build(engine);

    mesh->indexBuffer->setBuffer(
            engine, IndexBuffer::BufferDescriptor(indices, header->indexSize));

    VertexBuffer::Builder vbb;
    vbb.vertexCount(header->vertexCount)
            .bufferCount(1)
            .normalized(VertexAttribute::TANGENTS);

    mesh->vertexBuffer = vbb
            .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::HALF4,
                        header->offsetPosition, uint8_t(header->stridePosition))
            .attribute(VertexAttribute::TANGENTS, 0, VertexBuffer::AttributeType::SHORT4,
                        header->offsetTangents, uint8_t(header->strideTangents))
            .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::HALF2,
                        header->offsetUV0, uint8_t(header->strideUV0))
            .build(engine);

    VertexBuffer::BufferDescriptor buffer(vertexData, header->vertexSize,
            destructor, user);
    mesh->vertexBuffer->setBufferAt(engine, 0, move(buffer));

    mesh->renderable = EntityManager::get().create();

    RenderableManager::Builder builder(header->parts);
    builder.boundingBox(header->aabb);
    for (size_t i = 0; i < header->parts; i++) {
        builder.geometry(i, RenderableManager::PrimitiveType::TRIANGLES,
                            mesh->vertexBuffer, mesh->indexBuffer, parts[i].offset,
                            parts[i].minIndex, parts[i].maxIndex, parts[i].indexCount);
        builder.material(i, mi);
    }
    builder.build(engine, mesh->renderable);

    return Filamesh(mesh);
}
