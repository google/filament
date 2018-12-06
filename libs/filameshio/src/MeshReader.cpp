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

#include <filameshio/MeshReader.h>
#include <filameshio/filamesh.h>

#include <filament/Box.h>
#include <filament/Engine.h>
#include <filament/Fence.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>

#include <meshoptimizer.h>

#include <utils/EntityManager.h>
#include <utils/Log.h>
#include <utils/Path.h>

#include <string>
#include <vector>

#include <fcntl.h>
#if !defined(WIN32)
#    include <unistd.h>
#else
#    include <io.h>
#endif

using namespace filament;
using namespace filamesh;
using namespace math;

#define DEFAULT_MATERIAL "DefaultMaterial"

static size_t fileSize(int fd) {
    size_t filesize;
    filesize = (size_t) lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    return filesize;
}

MeshReader::Mesh MeshReader::loadMeshFromFile(filament::Engine* engine, const utils::Path& path,
        const MaterialRegistry& materials) {

    Mesh mesh;

    int fd = open(path.c_str(), O_RDONLY);

    size_t size = fileSize(fd);
    char* data = (char*) malloc(size);
    read(fd, data, size);

    if (data) {
        char *p = data;
        char *magic = p;
        p += sizeof(MAGICID);

        if (!strncmp(MAGICID, magic, 8)) {
            mesh = loadMeshFromBuffer(engine, data, nullptr, nullptr, materials);
        }

        Fence::waitAndDestroy(engine->createFence());
        free(data);
    }
    close(fd);

    return mesh;
}

MeshReader::Mesh MeshReader::loadMeshFromBuffer(filament::Engine* engine,
        void const* data, Callback destructor, void* user,
        MaterialInstance* defaultMaterial) {
    MaterialRegistry reg;
    reg[DEFAULT_MATERIAL] = defaultMaterial;
    return loadMeshFromBuffer(engine, data, destructor, user, reg);
}

MeshReader::Mesh MeshReader::loadMeshFromBuffer(filament::Engine* engine,
        void const* data, Callback destructor, void* user,
        const MaterialRegistry& materials) {
    const uint8_t* p = (const uint8_t*) data;
    if (strncmp(MAGICID, (const char *) p, 8)) {
        utils::slog.e << "Magic string not found." << utils::io::endl;
        return {};
    }
    p += 8;

    Header* header = (Header*) p;
    p += sizeof(Header);

    uint8_t const* vertexData = p;
    p += header->vertexSize;

    uint8_t const* indices = p;
    p += header->indexSize;

    Part* parts = (Part*) p;
    p += header->parts * sizeof(Part);

    uint32_t materialCount = (uint32_t) *p;
    p += sizeof(uint32_t);

    std::vector<std::string> partsMaterial(materialCount);
    for (size_t i = 0; i < materialCount; i++) {
        uint32_t nameLength = (uint32_t) *p;
        p += sizeof(uint32_t);
        partsMaterial[i] = (const char*) p;
        p += nameLength + 1; // null terminated
    }

    Mesh mesh;

    mesh.indexBuffer = IndexBuffer::Builder()
            .indexCount(header->indexCount)
            .bufferType(header->indexType == UI16 ? IndexBuffer::IndexType::USHORT
                    : IndexBuffer::IndexType::UINT)
            .build(*engine);

    // If the index buffer is compressed, then decode the indices into a temporary buffer.
    // The user callback can be called immediately afterwards because the source data does not get
    // passed to the GPU.
    const size_t indicesSize = header->indexSize;
    if (header->flags & COMPRESSION) {
        size_t indexSize = header->indexType == UI16 ? sizeof(uint16_t) : sizeof(uint32_t);
        size_t indexCount = header->indexCount;
        size_t uncompressedSize = indexSize * indexCount;
        void* uncompressed = malloc(uncompressedSize);
        int err = meshopt_decodeIndexBuffer(uncompressed, indexCount, indexSize, indices,
                indicesSize);
        if (err) {
            utils::slog.e << "Unable to decode index buffer." << utils::io::endl;
            return {};
        }
        if (destructor) {
            destructor((void*) indices, indicesSize, user);
        }
        auto freecb = [](void* buffer, size_t size, void* user) { free(buffer); };
        mesh.indexBuffer->setBuffer(*engine,
                IndexBuffer::BufferDescriptor(uncompressed, uncompressedSize, freecb, nullptr));
    } else {
        mesh.indexBuffer->setBuffer(*engine,
                IndexBuffer::BufferDescriptor(indices, indicesSize, destructor, user));
    }

    VertexBuffer::Builder vbb;
    vbb.vertexCount(header->vertexCount)
            .bufferCount(1)
            .normalized(VertexAttribute::COLOR)
            .normalized(VertexAttribute::TANGENTS);

    VertexBuffer::AttributeType uvtype = (header->flags & TEXCOORD_SNORM16) ?
            VertexBuffer::AttributeType::SHORT2 : VertexBuffer::AttributeType::HALF2;

    vbb
            .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::HALF4,
                        header->offsetPosition, uint8_t(header->stridePosition))
            .attribute(VertexAttribute::TANGENTS, 0, VertexBuffer::AttributeType::SHORT4,
                        header->offsetTangents, uint8_t(header->strideTangents))
            .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4,
                        header->offsetColor, uint8_t(header->strideColor))
            .attribute(VertexAttribute::UV0, 0, uvtype,
                        header->offsetUV0, uint8_t(header->strideUV0));

    if (header->flags & TEXCOORD_SNORM16) {
        vbb.normalized(VertexAttribute::UV0);
    }

    constexpr uint32_t uintmax = std::numeric_limits<uint32_t>::max();
    const bool hasUV1 = header->offsetUV1 != uintmax && header->strideUV1 != uintmax;

    if (hasUV1) {
        vbb.attribute(VertexAttribute::UV1, 0, VertexBuffer::AttributeType::HALF2,
                header->offsetUV1, uint8_t(header->strideUV1));
        if (header->flags & TEXCOORD_SNORM16) {
            vbb.normalized(VertexAttribute::UV1);
        }
    }

    mesh.vertexBuffer = vbb.build(*engine);

    // If the vertex buffer is compressed, then decode the vertices into a temporary buffer.
    // The user callback can be called immediately afterwards because the source data does not get
    // passed to the GPU.
    const size_t verticesSize = header->vertexSize;
    if (header->flags & COMPRESSION) {
        size_t vertexSize = sizeof(half4) + sizeof(short4) + sizeof(ubyte4) + sizeof(ushort2) +
                (hasUV1 ? sizeof(ushort2) : 0);
        size_t vertexCount = header->vertexCount;
        size_t uncompressedSize = vertexSize * vertexCount;
        void* uncompressed = malloc(uncompressedSize);
        const uint8_t* srcdata = vertexData + sizeof(CompressionHeader);
        int err = 0;
        if (header->flags & INTERLEAVED) {
            err |= meshopt_decodeVertexBuffer(uncompressed, vertexCount, vertexSize, srcdata,
                    vertexSize);
        } else {
            const CompressionHeader* sizes = (CompressionHeader*) vertexData;
            uint8_t* dstdata = (uint8_t*) uncompressed;
            auto decode = meshopt_decodeVertexBuffer;

            err |= decode(dstdata, vertexCount, sizeof(half4), srcdata, sizes->positions);
            srcdata += sizes->positions;
            dstdata += sizeof(half4) * vertexCount;

            err |= decode(dstdata, vertexCount, sizeof(short4), srcdata, sizes->tangents);
            srcdata += sizes->tangents;
            dstdata += sizeof(short4) * vertexCount;

            err |= decode(dstdata, vertexCount, sizeof(ubyte4), srcdata, sizes->colors);
            srcdata += sizes->colors;
            dstdata += sizeof(ubyte4) * vertexCount;

            err |= decode(dstdata, vertexCount, sizeof(ushort2), srcdata, sizes->uv0);

            if (sizes->uv1) {
                srcdata += sizes->uv0;
                dstdata += sizeof(ushort2) * vertexCount;
                err |= decode(dstdata, vertexCount, sizeof(ushort2), srcdata, sizes->uv1);
            }
        }
        if (err) {
            utils::slog.e << "Unable to decode vertex buffer." << utils::io::endl;
            return {};
        }
        if (destructor) {
            destructor((void*) vertexData, verticesSize, user);
        }
        auto freecb = [](void* buffer, size_t size, void* user) { free(buffer); };
        mesh.vertexBuffer->setBufferAt(*engine, 0,
                IndexBuffer::BufferDescriptor(uncompressed, uncompressedSize, freecb, nullptr));
    } else {
        mesh.vertexBuffer->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(vertexData, verticesSize, destructor, user));
    }

    mesh.renderable = utils::EntityManager::get().create();

    RenderableManager::Builder builder(header->parts);
    builder.boundingBox(header->aabb);
    const auto defaultmi = materials.at(DEFAULT_MATERIAL);
    for (size_t i = 0; i < header->parts; i++) {
        builder.geometry(i, RenderableManager::PrimitiveType::TRIANGLES,
                            mesh.vertexBuffer, mesh.indexBuffer, parts[i].offset,
                            parts[i].minIndex, parts[i].maxIndex, parts[i].indexCount);
        const auto miter = materials.find(partsMaterial[i]);
        builder.material(i, miter == materials.end() ? defaultmi : miter->second);
    }
    builder.build(*engine, mesh.renderable);

    return mesh;
}
