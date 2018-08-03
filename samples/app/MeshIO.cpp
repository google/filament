/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include "MeshIO.h"

#include <iostream>
#include <string>
#include <vector>

#include <fcntl.h>
#if !defined(WIN32)
#    include <unistd.h>
#else
#    include <io.h>
#endif

#include <utils/EntityManager.h>
#include <utils/Path.h>

#include <math/vec2.h>
#include <math/vec4.h>

#include <filament/Box.h>
#include <filament/Engine.h>
#include <filament/Fence.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>

using namespace filament;
using namespace math;

static size_t fileSize(int fd) {
    size_t filesize;
    filesize = (size_t) lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    return filesize;
}

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

struct Vertex {
    half4  position;
    short4 tangents;
    ubyte4 color;
    half2  uv0;
};

struct Part {
    uint32_t offset;
    uint32_t indexCount;
    uint32_t minIndex;
    uint32_t maxIndex;
    uint32_t materialID;
    Box      aabb;
};

MeshIO::Mesh MeshIO::loadMeshFromFile(filament::Engine* engine, const utils::Path& path,
        const std::map<std::string, filament::MaterialInstance*>& materials) {

    Mesh mesh;

    int fd = open(path.c_str(), O_RDONLY);

    size_t size = fileSize(fd);
    char* data = (char*) malloc(size);
    read(fd, data, size);

    if (data) {
        char *p = data;

        char magic[9];
        memcpy(magic, (const char*) p, sizeof(char) * 8);
        magic[8] = '\0';
        p += sizeof(char) * 8;

        if (!strcmp("FILAMESH", magic)) {
            Header* header = (Header*) p;
            p += sizeof(Header);

            char* vertexData = p;
            p += header->vertexSize;

            char* indices = p;
            p += header->indexSize;

            Part* parts = (Part*) p;
            p += header->parts * sizeof(Part);

            uint32_t materialCount = (uint32_t) *p;
            p += sizeof(uint32_t);

            std::vector<std::string> partsMaterial;
            partsMaterial.resize(materialCount);

            for (size_t i = 0; i < materialCount; i++) {
                uint32_t nameLength = (uint32_t) *p;
                p += sizeof(uint32_t);

                partsMaterial[i] = p;
                p += nameLength + 1; // null terminated
            }

            mesh.indexBuffer = IndexBuffer::Builder()
                    .indexCount(header->indexCount)
                    .bufferType(header->indexType ? IndexBuffer::IndexType::USHORT
                                                  : IndexBuffer::IndexType::UINT)
                    .build(*engine);

            mesh.indexBuffer->setBuffer(*engine,
                    IndexBuffer::BufferDescriptor(indices, header->indexSize));

            VertexBuffer::Builder vbb;
            vbb.vertexCount(header->vertexCount)
                .bufferCount(1)
                .normalized(VertexAttribute::TANGENTS)
                .normalized(VertexAttribute::COLOR)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::HALF4,
                        header->offsetPosition, uint8_t(header->stridePosition))
                .attribute(VertexAttribute::TANGENTS, 0, VertexBuffer::AttributeType::SHORT4,
                        header->offsetTangents, uint8_t(header->strideTangents))
                .attribute(VertexAttribute::COLOR,    0, VertexBuffer::AttributeType::UBYTE4,
                        header->offsetColor, uint8_t(header->strideColor))
                .attribute(VertexAttribute::UV0,      0, VertexBuffer::AttributeType::HALF2,
                        header->offsetUV0, uint8_t(header->strideUV0));

            if (header->offsetUV1 != std::numeric_limits<uint32_t>::max() &&
                    header->strideUV1 != std::numeric_limits<uint32_t>::max()) {
                vbb.attribute(VertexAttribute::UV1,   0, VertexBuffer::AttributeType::HALF2,
                        header->offsetUV1, uint8_t(header->strideUV1));
            }

            mesh.vertexBuffer = vbb.build(*engine);

            VertexBuffer::BufferDescriptor buffer(vertexData, header->vertexSize);
            mesh.vertexBuffer->setBufferAt(*engine, 0, std::move(buffer));

            RenderableManager::Builder builder(header->parts);
            builder.boundingBox(header->aabb);


            for (size_t i = 0; i < header->parts; i++) {
                builder.geometry(i, RenderableManager::PrimitiveType::TRIANGLES,
                        mesh.vertexBuffer, mesh.indexBuffer, parts[i].offset,
                        parts[i].minIndex, parts[i].maxIndex, parts[i].indexCount);
                auto m = materials.find(partsMaterial[i]);
                if (m != materials.end()) {
                    builder.material(i, m->second);
                } else {
                    builder.material(i, materials.at("DefaultMaterial"));
                }
            }

            mesh.renderable = utils::EntityManager::get().create();
            builder.build(*engine, mesh.renderable);
        }

        Fence::waitAndDestroy(engine->createFence());
        free(data);
    }
    close(fd);

    return mesh;
}
