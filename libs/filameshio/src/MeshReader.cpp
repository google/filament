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
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/VertexBuffer.h>

#include <meshoptimizer.h>

#include <utils/EntityManager.h>
#include <utils/Log.h>
#include <utils/Path.h>

#include <string>
#include <vector>
#include <map>
#include <limits>

#include <fcntl.h>
#if !defined(WIN32)
#    include <unistd.h>
#else
#    include <io.h>
#endif

using namespace filament;
using namespace filamesh;
using namespace filament::math;

#define DEFAULT_MATERIAL "DefaultMaterial"

//------------------------------------------------------------------------------
//-------------------------Begin Material Registry------------------------------
//------------------------------------------------------------------------------

struct MeshReader::MaterialRegistry::MaterialRegistryImpl {
    std::map<utils::CString, filament::MaterialInstance*> materialMap;
};

// Create the implementation
MeshReader::MaterialRegistry::MaterialRegistry()
    : mImpl(new MaterialRegistryImpl()) {
}

// Deep copy the implementation
MeshReader::MaterialRegistry::MaterialRegistry(const MaterialRegistry& rhs)
    : mImpl(new MaterialRegistryImpl(*rhs.mImpl)) {
}

MeshReader::MaterialRegistry& MeshReader::MaterialRegistry::operator=(const MaterialRegistry& rhs) {
    *mImpl = *rhs.mImpl;
    return *this;
}
// Delete the implementation
MeshReader::MaterialRegistry::~MaterialRegistry() {
    delete mImpl;
}

// Default move construction
MeshReader::MaterialRegistry::MaterialRegistry(MaterialRegistry&& rhs) 
    : mImpl(nullptr) {
    std::swap(mImpl, rhs.mImpl);
}

MeshReader::MaterialRegistry& MeshReader::MaterialRegistry::operator=(MaterialRegistry&& rhs) {
    delete mImpl;
    mImpl = nullptr;
    std::swap(mImpl, rhs.mImpl);
    return *this;
}

filament::MaterialInstance* MeshReader::MaterialRegistry::getMaterialInstance(
        const utils::CString& name) {
    // Try to find the requested material
    auto miter = mImpl->materialMap.find(name);
    // If it exists, return it
    if (miter != mImpl->materialMap.end()) {
        return miter->second;
    }
    // If it doesn't exist, give a dummy value
    return nullptr;
}

void MeshReader::MaterialRegistry::registerMaterialInstance(const utils::CString& name,
        filament::MaterialInstance* materialInstance) {
    // Add the material to our map
    mImpl->materialMap[name] = materialInstance;
}

void MeshReader::MaterialRegistry::unregisterMaterialInstance(const utils::CString& name) {
    auto miter = mImpl->materialMap.find(name);
    // Remove it from the map if it existed
    if (miter != mImpl->materialMap.end()) {
        mImpl->materialMap.erase(miter);
    }
}
void MeshReader::MaterialRegistry::unregisterAll() {
    mImpl->materialMap.clear();
}

std::size_t MeshReader::MaterialRegistry::numRegistered() const noexcept {
    return mImpl->materialMap.size();
}

void MeshReader::MaterialRegistry::getRegisteredMaterials(filament::MaterialInstance** materialList,
        utils::CString* materialNameList) const {
    for (const auto& materialPair : mImpl->materialMap) {
        (*materialNameList++) = materialPair.first;
        (*materialList++) = materialPair.second;
    }
}

void MeshReader::MaterialRegistry::getRegisteredMaterials(
        filament::MaterialInstance** materialList) const {
    for (const auto& materialPair : mImpl->materialMap) {
        (*materialList++) = materialPair.second;
    }
}

void MeshReader::MaterialRegistry::getRegisteredMaterialNames(
        utils::CString* materialNameList) const {
    for (const auto& materialPair : mImpl->materialMap) {
        (*materialNameList++) = materialPair.first;
    }
}

//------------------------------------------------------------------------------
//---------------------------End Material Registry------------------------------
//------------------------------------------------------------------------------


static size_t fileSize(int fd) {
    size_t filesize;
    filesize = (size_t) lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    return filesize;
}

namespace filamesh {

template <typename T>
inline bool readChecked(T& dest, const uint8_t*& p, const uint8_t* end) {
    if (size_t(end - p) < sizeof(T)) {
        return false;
    }
    memcpy(&dest, p, sizeof(T));
    p += sizeof(T);
    return true;
}

MeshReader::Mesh MeshReader::loadMeshFromFile(filament::Engine* engine, const utils::Path& path,
        MaterialRegistry& materials) {

    Mesh mesh;

    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        utils::slog.e << "Unable to open filamesh file: " << path.c_str() << utils::io::endl;
        return {};
    }

    size_t size = fileSize(fd);
    char* data = (char*) malloc(size);
    if (!data) {
        close(fd);
        return {};
    }

    size_t bytesRead = read(fd, data, size);
    if (bytesRead == size) {
        mesh = loadMeshFromBuffer(engine, data, size, nullptr, nullptr, materials);
    }

    Fence::waitAndDestroy(engine->createFence());
    free(data);
    close(fd);

    return mesh;
}

MeshReader::Mesh MeshReader::loadMeshFromBuffer(filament::Engine* engine,
        void const* data, size_t dataSize, Callback destructor, void* user,
        MaterialInstance* defaultMaterial) {
    MaterialRegistry reg;
    reg.registerMaterialInstance(utils::CString(DEFAULT_MATERIAL), defaultMaterial);
    return loadMeshFromBuffer(engine, data, dataSize, destructor, user, reg);
}

MeshReader::Mesh MeshReader::loadMeshFromBuffer(filament::Engine* engine,
        void const* data, size_t dataSize, Callback destructor, void* user,
        MaterialRegistry& materials) {
    const uint8_t* p = (const uint8_t*) data;
    const uint8_t* end = p + dataSize;
    if (end < p) { // check pointer wraparound
        utils::slog.e << "Invalid (too large) dataSize: " << dataSize << utils::io::endl;
        return {};
    }

    if (dataSize < 8 || strncmp(MAGICID, (const char *) p, 8)) {
        utils::slog.e << "Magic string not found or buffer too small." << utils::io::endl;
        return {};
    }
    p += 8;

    Header header;
    if (!readChecked(header, p, end)) {
        utils:: slog.e << "Header truncation." << utils::io::endl;
        return {};
    }

    if (header.version != VERSION) {
        utils::slog.e << "Unsupported filamesh version: " << header.version << utils::io::endl;
        return {};
    }

    // Check integer overflow on header.parts * sizeof(Part)
    if (size_t(header.parts) > std::numeric_limits<size_t>::max() / sizeof(Part)) {
        utils::slog.e << "Too many mesh parts (overflow)." << utils::io::endl;
        return {};
    }
    const size_t partsBytes = header.parts * sizeof(Part);

    // Aligned payload verification (mathematically foolproof subtraction checks)
    size_t requiredSize = header.vertexSize;
    if (std::numeric_limits<size_t>::max() - requiredSize < header.indexSize) {
        utils::slog.e << "Payload sizes overflow." << utils::io::endl;
        return {};
    }
    requiredSize += header.indexSize;

    if (std::numeric_limits<size_t>::max() - requiredSize < partsBytes) {
        utils::slog.e << "Payload sizes overflow." << utils::io::endl;
        return {};
    }
    requiredSize += partsBytes;

    if (size_t(end - p) < requiredSize) {
        utils::slog.e << "Buffer truncation in mesh payload." << utils::io::endl;
        return {};
    }

    uint8_t const* vertexData = p;
    p += header.vertexSize;

    uint8_t const* indices = p;
    p += header.indexSize;

    std::vector<Part> parts(header.parts);
    for (size_t i = 0; i < header.parts; i++) {
        if (!readChecked(parts[i], p, end)) {
            utils::slog.e << "Buffer truncation in parts array." << utils::io::endl;
            return {};
        }
    }

    uint32_t materialCount = 0;
    if (!readChecked(materialCount, p, end)) {
        utils::slog.e << "Buffer truncation reading material count." << utils::io::endl;
        return {};
    }

    std::vector<std::string> partsMaterial(materialCount);
    for (size_t i = 0; i < materialCount; i++) {
        uint32_t nameLength = 0;
        if (!readChecked(nameLength, p, end)) {
            utils::slog.e << "Buffer truncation reading material name length." << utils::io::endl;
            return {};
        }
        if (size_t(nameLength) >= std::numeric_limits<size_t>::max() - 1) {
            utils::slog.e << "Material name length overflow." << utils::io::endl;
            return {};
        }
        const size_t nameBytes = size_t(nameLength) + 1; // including null terminator
        if (size_t(end - p) < nameBytes) {
            utils::slog.e << "Buffer truncation reading material name." << utils::io::endl;
            return {};
        }
        partsMaterial[i] = std::string((const char*) p, nameLength);
        p += nameBytes;
    }

    Mesh mesh;

    mesh.indexBuffer = IndexBuffer::Builder()
            .indexCount(header.indexCount)
            .bufferType(header.indexType == UI16 ? IndexBuffer::IndexType::USHORT
                    : IndexBuffer::IndexType::UINT)
            .build(*engine);

    const size_t indicesSize = header.indexSize;
    if (header.flags & COMPRESSION) {
        size_t indexSize = header.indexType == UI16 ? sizeof(uint16_t) : sizeof(uint32_t);
        size_t indexCount = header.indexCount;
        
        if (indexCount > std::numeric_limits<size_t>::max() / indexSize) {
            utils::slog.e << "Uncompressed index count overflow." << utils::io::endl;
            engine->destroy(mesh.indexBuffer);
            return {};
        }
        size_t uncompressedSize = indexSize * indexCount;
        void* uncompressed = malloc(uncompressedSize);
        if (!uncompressed) {
            utils::slog.e << "Out of memory allocating uncompressed index buffer." << utils::io::endl;
            engine->destroy(mesh.indexBuffer);
            return {};
        }

        int err = meshopt_decodeIndexBuffer(uncompressed, indexCount, indexSize, indices,
                indicesSize);
        if (err) {
            utils::slog.e << "Unable to decode index buffer." << utils::io::endl;
            free(uncompressed);
            engine->destroy(mesh.indexBuffer);
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
    vbb.vertexCount(header.vertexCount)
            .bufferCount(1)
            .normalized(VertexAttribute::COLOR)
            .normalized(VertexAttribute::TANGENTS);

    VertexBuffer::AttributeType uvtype = (header.flags & TEXCOORD_SNORM16) ?
            VertexBuffer::AttributeType::SHORT2 : VertexBuffer::AttributeType::HALF2;

    vbb
            .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::HALF4,
                        header.offsetPosition, uint8_t(header.stridePosition))
            .attribute(VertexAttribute::TANGENTS, 0, VertexBuffer::AttributeType::SHORT4,
                        header.offsetTangents, uint8_t(header.strideTangents))
            .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4,
                        header.offsetColor, uint8_t(header.strideColor))
            .attribute(VertexAttribute::UV0, 0, uvtype,
                        header.offsetUV0, uint8_t(header.strideUV0))
            .normalized(VertexAttribute::UV0, header.flags & TEXCOORD_SNORM16);

    constexpr uint32_t uintmax = std::numeric_limits<uint32_t>::max();
    const bool hasUV1 = header.offsetUV1 != uintmax && header.strideUV1 != uintmax;

    if (hasUV1) {
        vbb
            .attribute(VertexAttribute::UV1, 0, VertexBuffer::AttributeType::HALF2,
                    header.offsetUV1, uint8_t(header.strideUV1))
            .normalized(VertexAttribute::UV1);
    }

    mesh.vertexBuffer = vbb.build(*engine);

    const size_t verticesSize = header.vertexSize;
    if (header.flags & COMPRESSION) {
        size_t vertexSize = sizeof(half4) + sizeof(short4) + sizeof(ubyte4) + sizeof(ushort2) +
                (hasUV1 ? sizeof(ushort2) : 0);
        size_t vertexCount = header.vertexCount;

        if (vertexCount > std::numeric_limits<size_t>::max() / vertexSize) {
            utils::slog.e << "Uncompressed vertex count overflow." << utils::io::endl;
            engine->destroy(mesh.vertexBuffer);
            engine->destroy(mesh.indexBuffer);
            return {};
        }
        size_t uncompressedSize = vertexSize * vertexCount;
        void* uncompressed = malloc(uncompressedSize);
        if (!uncompressed) {
            utils::slog.e << "Out of memory allocating uncompressed vertex buffer." << utils::io::endl;
            engine->destroy(mesh.vertexBuffer);
            engine->destroy(mesh.indexBuffer);
            return {};
        }

        if (verticesSize < sizeof(CompressionHeader)) {
            utils::slog.e << "Compressed vertex buffer too small for header." << utils::io::endl;
            free(uncompressed);
            engine->destroy(mesh.vertexBuffer);
            engine->destroy(mesh.indexBuffer);
            return {};
        }

        const uint8_t* srcdata = vertexData + sizeof(CompressionHeader);
        int err = 0;
        if (header.flags & INTERLEAVED) {
            err |= meshopt_decodeVertexBuffer(uncompressed, vertexCount, vertexSize, srcdata,
                    vertexSize);
        } else {
            CompressionHeader sizes;
            memcpy(&sizes, vertexData, sizeof(CompressionHeader));

            // Validate that CompressionHeader.uv1 is consistent with hasUV1.
            // The destination buffer was allocated using hasUV1 (derived from
            // Header.offsetUV1/strideUV1). If CompressionHeader.uv1 is
            // non-zero but hasUV1 is false, the uv1 decode below would write
            // sizeof(ushort2)*vertexCount bytes past the allocation.
            if (!hasUV1 && sizes.uv1 != 0) {
                utils::slog.e << "CompressionHeader.uv1 is non-zero but header"
                        " lacks UV1 attribute." << utils::io::endl;
                free(uncompressed);
                engine->destroy(mesh.vertexBuffer);
                engine->destroy(mesh.indexBuffer);
                return {};
            }

            size_t compressedSum = sizeof(CompressionHeader) +
                                   size_t(sizes.positions) +
                                   size_t(sizes.tangents) +
                                   size_t(sizes.colors) +
                                   size_t(sizes.uv0) +
                                   size_t(sizes.uv1);
            if (compressedSum > verticesSize) {
                utils::slog.e << "Compressed vertex buffer elements exceed bounds." << utils::io::endl;
                free(uncompressed);
                engine->destroy(mesh.vertexBuffer);
                engine->destroy(mesh.indexBuffer);
                return {};
            }

            uint8_t* dstdata = (uint8_t*) uncompressed;
            auto decode = meshopt_decodeVertexBuffer;

            err |= decode(dstdata, vertexCount, sizeof(half4), srcdata, sizes.positions);
            srcdata += sizes.positions;
            dstdata += sizeof(half4) * vertexCount;

            err |= decode(dstdata, vertexCount, sizeof(short4), srcdata, sizes.tangents);
            srcdata += sizes.tangents;
            dstdata += sizeof(short4) * vertexCount;

            err |= decode(dstdata, vertexCount, sizeof(ubyte4), srcdata, sizes.colors);
            srcdata += sizes.colors;
            dstdata += sizeof(ubyte4) * vertexCount;

            err |= decode(dstdata, vertexCount, sizeof(ushort2), srcdata, sizes.uv0);

            if (sizes.uv1) {
                srcdata += sizes.uv0;
                dstdata += sizeof(ushort2) * vertexCount;
                err |= decode(dstdata, vertexCount, sizeof(ushort2), srcdata, sizes.uv1);
            }
        }
        if (err) {
            utils::slog.e << "Unable to decode vertex buffer." << utils::io::endl;
            free(uncompressed);
            engine->destroy(mesh.vertexBuffer);
            engine->destroy(mesh.indexBuffer);
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

    RenderableManager::Builder builder(header.parts);
    builder.boundingBox(header.aabb);

    const auto defaultmi = materials.getMaterialInstance(utils::CString(DEFAULT_MATERIAL));
    for (size_t i = 0; i < header.parts; i++) {
        builder.geometry(i, RenderableManager::PrimitiveType::TRIANGLES,
                mesh.vertexBuffer, mesh.indexBuffer, parts[i].offset,
                parts[i].minIndex, parts[i].maxIndex, parts[i].indexCount);

        uint32_t materialIndex = parts[i].material;
        if (materialIndex >= partsMaterial.size()) {
            utils::slog.e << "Material index (" << materialIndex << ") of mesh part ("
                    << i << ") is out of bounds (" << partsMaterial.size() << ")" << utils::io::endl;
            continue;
        }

        const utils::CString materialName(
                partsMaterial[materialIndex].c_str(), partsMaterial[materialIndex].size());
        const auto mat = materials.getMaterialInstance(materialName);
        if (mat == nullptr) {
            builder.material(i, defaultmi);
            materials.registerMaterialInstance(materialName, defaultmi);
        } else {
            builder.material(i, mat);
        }
    }
    builder.build(*engine, mesh.renderable);

    return mesh;
}

} // namespace filamesh
