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

#include "MeshWriter.h"

#include <filameshio/filamesh.h>

#include <meshoptimizer.h>

using namespace filamesh;
using namespace filament::math;
using namespace std;

template<typename T>
void write(ostream& out, const T& value) {
    out.write((const char*) &value, sizeof(T));
}

template<typename T>
void write(ostream& out, const T* data, uint32_t count) {
    out.write((const char*) data, sizeof(T) * count);
}

template<typename T>
size_t write(unsigned char* out, const vector<T>& data) {
    memcpy(out, data.data(), data.size() * sizeof(T));
    return data.size() * sizeof(T);
}

void MeshWriter::optimize(Mesh& mesh) {
    // First, re-order triangles to improve cache locality and reduce the number of VS invocations.
    // Note that assimp already has aiProcess_ImproveCacheLocality, but MeshWriter doesn't know
    // about assimp, and it doesn't hurt to do it again here since this generally runs offline.
    meshopt_optimizeVertexCache(mesh.indices.data(), mesh.indices.data(), mesh.indices.size(),
                mesh.vertexCount);

    // At this point, triangle order has been established but we still need to shuffle vertices to
    // optimize the fetch. This makes it so that lower-numbered indices generally come before
    // higher-numbered indices.
    if (mFlags & INTERLEAVED) {
        meshopt_optimizeVertexFetch(mesh.vertices.data(), mesh.indices.data(),
                mesh.indices.size(), mesh.vertices.data(), mesh.vertices.size(),
                sizeof(Vertex));
    } else {
        const uint32_t vertexCount = mesh.vertexCount;

        // Allocate a remapping table and create a copy of the index buffer.
        vector<uint32_t> remappingVector(vertexCount);
        vector<uint32_t> indicesVector = mesh.indices;
        uint32_t* remapping = remappingVector.data();
        const uint32_t* indices = indicesVector.data();

        // Populate the remapping table.
        meshopt_optimizeVertexFetchRemap(remapping, mesh.indices.data(),
                mesh.indices.size(), vertexCount);

        // Apply the remapping table.
        meshopt_remapIndexBuffer(mesh.indices.data(), indices, mesh.indices.size(), remapping);
        meshopt_remapVertexBuffer(mesh.positions.data(), mesh.positions.data(),
                vertexCount, sizeof(decltype(Vertex::position)), remapping);
        meshopt_remapVertexBuffer(mesh.tangents.data(), mesh.tangents.data(),
                vertexCount, sizeof(decltype(Vertex::tangents)), remapping);
        meshopt_remapVertexBuffer(mesh.colors.data(), mesh.colors.data(),
                vertexCount, sizeof(decltype(Vertex::color)), remapping);
        meshopt_remapVertexBuffer(mesh.uv0.data(), mesh.uv0.data(),
                vertexCount, sizeof(decltype(Vertex::uv0)), remapping);
        if (!mesh.uv1.empty()) {
            meshopt_remapVertexBuffer(mesh.uv1.data(), mesh.uv1.data(),
                    vertexCount, sizeof(decltype(Vertex::uv0)), remapping);
        }
    }

    // As a last step, the meshoptimizer README recommends applying individual meshopt_quantize*
    // functions as needed, but we actually already quantized the data according to our constraints
    // e.g. we already (potentially) use snorm16 for uvs, half-floats for tangents, etc.
}

bool MeshWriter::serialize(ostream& out, Mesh& mesh) {
    const bool hasIndex16 = mesh.vertexCount <= numeric_limits<uint16_t>::max();
    const bool hasUV1 = !mesh.uv1.empty();
    const size_t vertexSize = sizeof(Vertex) + (hasUV1 ? sizeof(ushort2) : 0);
    if ((mFlags & INTERLEAVED) && hasUV1) {
        cerr << "Interleaved vertices can only have 1 UV set." << endl;
        return false;
    }

    // Compute the overall bounding box.
    Box aabb = mesh.parts.at(0).aabb;
    for (size_t i = 1; i < mesh.parts.size(); i++) {
        aabb.unionSelf(mesh.parts.at(i).aabb);
    }

    // It's safe to optimize the mesh regardless of the compression setting.
    optimize(mesh);

    // Perform compression of vertex data if it has been requested.
    CompressionHeader cheader {};
    vector<unsigned char> compressedVertices;
    if (mFlags & COMPRESSION) {
        compressedVertices.resize(meshopt_encodeVertexBufferBound(mesh.vertexCount, vertexSize));
        size_t compressedVertexSize;
        if (mFlags & INTERLEAVED) {
            compressedVertexSize = meshopt_encodeVertexBuffer(compressedVertices.data(),
                    compressedVertices.size(), mesh.vertices.data(), mesh.vertexCount, vertexSize);
        } else {
            unsigned char* cptr = compressedVertices.data();
            unsigned char* cend = compressedVertices.data() + compressedVertices.size();

            cheader.positions = meshopt_encodeVertexBuffer(cptr, cend - cptr, mesh.positions.data(),
                    mesh.vertexCount, sizeof(decltype(Vertex::position)));
            cptr += cheader.positions;

            cheader.tangents = meshopt_encodeVertexBuffer(cptr, cend - cptr, mesh.tangents.data(),
                    mesh.vertexCount, sizeof(decltype(Vertex::tangents)));
            cptr += cheader.tangents;

            cheader.colors = meshopt_encodeVertexBuffer(cptr, cend - cptr, mesh.colors.data(),
                    mesh.vertexCount, sizeof(decltype(Vertex::color)));
            cptr += cheader.colors;

            cheader.uv0 = meshopt_encodeVertexBuffer(cptr, cend - cptr, mesh.uv0.data(),
                    mesh.vertexCount, sizeof(decltype(Vertex::uv0)));
            cptr += cheader.uv0;

            if (hasUV1) {
                cheader.uv1 = meshopt_encodeVertexBuffer(cptr, cend - cptr, mesh.uv1.data(),
                        mesh.vertexCount, sizeof(decltype(Vertex::uv0)));
                cptr += cheader.uv1;
            }

            assert(cend - cptr >= 0);
            compressedVertexSize = cptr - compressedVertices.data();
        }
        if (compressedVertexSize == 0) {
            cerr << "Unable to compress vertex buffer." << endl;
            return false;
        }
        compressedVertices.resize(compressedVertexSize);
    }

    // Perform compression of index data if it has been requested.
    vector<unsigned char> compressedIndices;
    if (mFlags & COMPRESSION) {
        compressedIndices.resize(meshopt_encodeIndexBufferBound(mesh.indices.size(),
                mesh.vertexCount));
        size_t result = meshopt_encodeIndexBuffer(compressedIndices.data(),
                compressedIndices.size(), mesh.indices.data(), mesh.indices.size());
        if (result == 0) {
            cerr << "Unable to compress index buffer." << endl;
            return false;
        }
        compressedIndices.resize(result);
    }

    write(out, "FILAMESH", 8 * sizeof(char));

    Header header;
    header.version = VERSION;
    header.parts = uint32_t(mesh.parts.size());
    header.aabb = aabb;
    header.flags = mFlags;
    if (mFlags & INTERLEAVED) {
        header.offsetPosition = offsetof(Vertex, position);
        header.offsetTangents = offsetof(Vertex, tangents);
        header.offsetColor    = offsetof(Vertex, color);
        header.offsetUV0      = offsetof(Vertex, uv0);
        header.offsetUV1      = numeric_limits<uint32_t>::max();
        header.stridePosition = sizeof(Vertex);
        header.strideTangents = sizeof(Vertex);
        header.strideColor    = sizeof(Vertex);
        header.strideUV0      = sizeof(Vertex);
        header.strideUV1      = numeric_limits<uint32_t>::max();;
    } else {
        header.offsetPosition = 0;
        header.offsetTangents = mesh.vertexCount * sizeof(Vertex::position);
        header.offsetColor    = header.offsetTangents + mesh.vertexCount * sizeof(Vertex::tangents);
        header.offsetUV0      = header.offsetColor + mesh.vertexCount * sizeof(Vertex::color);
        header.offsetUV1      = numeric_limits<uint32_t>::max();;
        header.stridePosition = 0;
        header.strideTangents = 0;
        header.strideColor    = 0;
        header.strideUV0      = 0;
        header.strideUV1      = numeric_limits<uint32_t>::max();;
        if (hasUV1) {
            header.offsetUV1  = header.offsetUV0 + mesh.vertexCount * sizeof(Vertex::uv0);
            header.strideUV1  = 0;
        }
    }
    header.vertexCount = mesh.vertexCount;
    header.indexType = uint32_t(hasIndex16 ? UI16 : UI32);
    header.indexCount = mesh.indices.size();

    if (mFlags & COMPRESSION) {
        header.vertexSize = sizeof(cheader) + compressedVertices.size();
        header.indexSize = compressedIndices.size();
    } else {
        header.vertexSize = mesh.vertexCount * vertexSize;
        header.indexSize = mesh.indices.size() * (hasIndex16 ? sizeof(uint16_t) : sizeof(uint32_t));
    }

    write(out, header);

    if (mFlags & COMPRESSION) {
        write(out, &cheader, 1);
        write(out, compressedVertices.data(), compressedVertices.size());
    } else if (mFlags & INTERLEAVED) {
        write(out, mesh.vertices.data(), uint32_t(mesh.vertices.size()));
    } else {
        write(out, mesh.positions.data(), uint32_t(mesh.positions.size()));
        write(out, mesh.tangents.data(),  uint32_t(mesh.tangents.size()));
        write(out, mesh.colors.data(), uint32_t(mesh.colors.size()));
        write(out, mesh.uv0.data(), uint32_t(mesh.uv0.size()));
        if (hasUV1) {
            write(out, mesh.uv1.data(), uint32_t(mesh.uv1.size()));
        }
    }

    if (mFlags & COMPRESSION) {
        write(out, compressedIndices.data(), compressedIndices.size());
    } else if (!hasIndex16) {
        write(out, mesh.indices.data(), uint32_t(mesh.indices.size()));
    } else {
        vector<uint16_t> smallIndices;
        smallIndices.resize(mesh.indices.size());
        for (size_t i = 0; i < mesh.indices.size(); i++) {
            smallIndices[i] = static_cast<uint16_t>(mesh.indices[i]);
        }
        write(out, smallIndices.data(), uint32_t(smallIndices.size()));
    }

    write(out, mesh.parts.data(), header.parts);

    write(out, uint32_t(mesh.materials.size()));
    for (const auto& name : mesh.materials) {
        write(out, uint32_t(name.size()));
        write(out, name.c_str(), uint32_t(name.size()));
        write(out, char(0));
    }

    return true;
}
