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

#include <math/norm.h>

using namespace filamesh;
using namespace math;
using namespace std;

template<typename T>
void write(ostream& out, const T& value) {
    out.write((const char*) &value, sizeof(T));
}

template<typename T>
void write(ostream& out, const T* data, uint32_t count) {
    out.write((const char*) data, sizeof(T) * count);
}

void MeshWriter::serialize(ostream& out, const Mesh& mesh) {

    const uint32_t maxint = std::numeric_limits<uint32_t>::max();
    const bool hasIndex16 = mesh.vertexCount < maxint;
    const bool hasUV1 = !mesh.uv1.empty();

    // Compute the overall bounding box.
    Box aabb = mesh.parts.at(0).aabb;
    for (size_t i = 1; i < mesh.parts.size(); i++) {
        aabb.unionSelf(mesh.parts.at(i).aabb);
    }

    write(out, "FILAMESH", 8 * sizeof(char));

    Header header;
    header.version = VERSION;
    header.parts = uint32_t(mesh.parts.size());
    header.aabb = aabb;
    header.flags = 0;
    header.flags |= mInterleaved ? INTERLEAVED : 0;
    header.flags |= mSnormUVs ? TEXCOORD_SNORM16 : 0;
    if (mInterleaved) {
        header.offsetPosition = offsetof(Vertex, position);
        header.offsetTangents = offsetof(Vertex, tangents);
        header.offsetColor    = offsetof(Vertex, color);
        header.offsetUV0      = offsetof(Vertex, uv0);
        header.offsetUV1      = maxint;
        header.stridePosition = sizeof(Vertex);
        header.strideTangents = sizeof(Vertex);
        header.strideColor    = sizeof(Vertex);
        header.strideUV0      = sizeof(Vertex);
        header.strideUV1      = maxint;
    } else {
        header.offsetPosition = 0;
        header.offsetTangents = mesh.vertexCount * sizeof(Vertex::position);
        header.offsetColor    = header.offsetTangents + mesh.vertexCount * sizeof(Vertex::tangents);
        header.offsetUV0      = header.offsetColor + mesh.vertexCount * sizeof(Vertex::color);
        header.offsetUV1      = maxint;
        header.stridePosition = 0;
        header.strideTangents = 0;
        header.strideColor    = 0;
        header.strideUV0      = 0;
        header.strideUV1      = maxint;

        if (hasUV1) {
            header.offsetUV1  = header.offsetUV0 + mesh.vertexCount * sizeof(Vertex::uv0);
            header.strideUV1  = 0;
        }
    }
    header.vertexCount = mesh.vertexCount;
    header.vertexSize = mesh.vertexCount * sizeof(Vertex);
    header.indexType = uint32_t(hasIndex16 ? UI16 : UI32);
    header.indexCount = mesh.indices.size();
    header.indexSize = mesh.indices.size() * (hasIndex16 ? sizeof(uint16_t) : sizeof(uint32_t));

    write(out, header);

    if (mInterleaved) {
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

    if (!hasIndex16) {
        write(out, mesh.indices.data(), uint32_t(mesh.indices.size()));
    } else {
        std::vector<uint16_t> smallIndices;
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
}
