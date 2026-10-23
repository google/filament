/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include "DracoCache.h"

#include <utils/compiler.h>
#include <utils/Log.h>
#include <utils/Panic.h>

#if GLTFIO_DRACO_SUPPORTED
#include <draco/compression/decode.h>
#endif

#include <cstdlib>
#include <limits>
#include <memory>
#include <vector>

using namespace utils;

#if GLTFIO_DRACO_SUPPORTED
using std::unique_ptr;
using std::vector;
#endif

namespace filament::gltfio {

DracoMesh* DracoCache::findOrCreateMesh(const cgltf_buffer_view* key) {
    if (!key || !key->buffer || !key->buffer->data) {
        return nullptr;
    }
    if (key->offset > key->buffer->size || key->size > key->buffer->size - key->offset) {
        return nullptr;
    }
    auto iter = mCache.find(key);
    if (iter != mCache.end()) {
        return iter->second.get();
    }
    const uint8_t* compressedData = key->offset + (const uint8_t*) key->buffer->data;
    DracoMesh* mesh = DracoMesh::decode(compressedData, key->size);
    mCache.emplace(key, mesh);
    return mesh;
}

#if GLTFIO_DRACO_SUPPORTED
struct DracoMeshDetails {
    unique_ptr<draco::Mesh> mesh;
    uint32_t numFacesOverride = 0;
    vector<unique_ptr<cgltf_buffer_view>> views;
    vector<unique_ptr<cgltf_buffer>> buffers;
};
#else
struct DracoMeshDetails {};
#endif

DracoMesh::DracoMesh(struct DracoMeshDetails* details) : mDetails(details) {}

#if GLTFIO_DRACO_SUPPORTED

DracoMesh::~DracoMesh() {
    for (auto& buffer : mDetails->buffers) {
        free(buffer->data);
    }
}

// Gets the number of components in the given cgltf vector type, or -1 for matrices.
static int getNumComponents(cgltf_type ctype) {
    return ((int) ctype) <= 4 ? ((int) ctype) : -1;
}

// Allocates and populates the given buffer view with indices from the given Draco mesh.
template<typename T>
static bool convertFaces(cgltf_accessor* target, const draco::Mesh* mesh, uint32_t numFaces,
        DracoMeshDetails* details) {
    if (target->stride != sizeof(T)) {
        return false;
    }

    const uint64_t size64 = static_cast<uint64_t>(numFaces) * 3 * sizeof(T);
    if (size64 > std::numeric_limits<uint32_t>::max()) {
        return false;
    }
    if (numFaces > mesh->num_faces()) {
        return false;
    }

    const cgltf_size size = static_cast<cgltf_size>(size64);
    void* data = malloc(size);
    if (size > 0 && !data) {
        return false;
    }

    T* dest = static_cast<T*>(data);
    for (uint32_t id = 0; id < numFaces; ++id) {
        const draco::Mesh::Face& face = mesh->face(draco::FaceIndex(id));
        *dest++ = face[0].value();
        *dest++ = face[1].value();
        *dest++ = face[2].value();
    }

    cgltf_buffer* buffer = new cgltf_buffer {
        .size = size,
        .data = data,
    };
    cgltf_buffer_view* view = new cgltf_buffer_view {
        .buffer = buffer,
        .offset = 0,
        .size = size,
        .stride = 0,
        .type = cgltf_buffer_view_type_indices,
    };

    details->views.emplace_back(view);
    details->buffers.emplace_back(buffer);

    target->offset = 0;
    target->buffer_view = view;
    return true;
}

// Converts vertex attributes into the desired format and populates the given cgltf buffer.
template<typename T>
static bool convertAttribs(cgltf_accessor* target, const draco::PointAttribute* attr, uint32_t n,
        DracoMeshDetails* details) {
    const int8_t ncomps = attr->num_components();
    if (ncomps <= 0 || ncomps > 4 || ncomps != getNumComponents(target->type)) {
        return false;
    }
    if (target->stride != static_cast<cgltf_size>(ncomps) * sizeof(T)) {
        return false;
    }

    const uint64_t size64 = static_cast<uint64_t>(target->stride) * n;
    if (size64 > std::numeric_limits<uint32_t>::max()) {
        return false;
    }
    if (attr->is_mapping_identity() ? (n > attr->size()) : (n > attr->indices_map_size())) {
        return false;
    }

    const cgltf_size size = static_cast<cgltf_size>(size64);
    void* data = malloc(size);
    if (size > 0 && !data) {
        return false;
    }

    T* dest = static_cast<T*>(data);
    for (draco::PointIndex i(0); i < n; ++i, dest += ncomps) {
        const draco::AttributeValueIndex index = attr->mapped_index(i);
        if (index.value() >= attr->size() || !attr->ConvertValue(index, ncomps, dest)) {
            free(data);
            return false;
        }
    }

    cgltf_buffer* buffer = new cgltf_buffer {
        .size = size,
        .data = data,
    };
    cgltf_buffer_view* view = new cgltf_buffer_view {
        .buffer = buffer,
        .offset = 0,
        .size = size,
        .stride = 0,
        .type = cgltf_buffer_view_type_vertices,
    };

    details->views.emplace_back(view);
    details->buffers.emplace_back(buffer);

    target->offset = 0;
    target->buffer_view = view;
    return true;
}

DracoMesh* DracoMesh::decode(const uint8_t* data, size_t dataSize) {
    draco::DecoderBuffer buffer;
    buffer.Init((const char*) data, dataSize);
    draco::Decoder decoder;
    const auto geotype = decoder.GetEncodedGeometryType(&buffer);
    if (!geotype.ok() || geotype.value() != draco::EncodedGeometryType::TRIANGULAR_MESH) {
        return nullptr;
    }
    auto meshStatus = decoder.DecodeMeshFromBuffer(&buffer);
    if (!meshStatus.ok()) {
        return nullptr;
    }
    return new DracoMesh(new DracoMeshDetails { std::move(meshStatus).value() });
}

DracoMesh* DracoMesh::createForTest(draco::Mesh* mesh, uint32_t numFacesOverride) {
    if (!mesh) {
        return nullptr;
    }
    return new DracoMesh(new DracoMeshDetails {
        .mesh = unique_ptr<draco::Mesh>(mesh),
        .numFacesOverride = numFacesOverride,
    });
}

bool DracoMesh::getFaceIndices(cgltf_accessor* target) const {
    // Return early if we've already decompressed this data.
    if (target->buffer_view) {
        return true;
    }

    draco::Mesh* mesh = mDetails->mesh.get();
    const uint32_t numFaces = mDetails->numFacesOverride ?
            mDetails->numFacesOverride : mesh->num_faces();

    // Check the accessor's index count against the number of faces in the Draco mesh.
    // It would be tricky to be robust against a mismatch; see the class comment for DracoMesh.
    const uint64_t count = static_cast<uint64_t>(numFaces) * 3;
    if (count > std::numeric_limits<uint32_t>::max() || target->count != count) {
        slog.e << "The glTF accessor wants " << target->count << " indices, "
               << "but the decoded Draco mesh has " << count << " indices." << io::endl;
        return false;
    }

    switch (target->component_type) {
        case cgltf_component_type_r_16u:
            return convertFaces<uint16_t>(target, mesh, numFaces, mDetails.get());
        case cgltf_component_type_r_32u:
            return convertFaces<uint32_t>(target, mesh, numFaces, mDetails.get());
        case cgltf_component_type_r_8u:
            return convertFaces<uint8_t>(target, mesh, numFaces, mDetails.get());
        default:
            slog.e << "Unexpected component type for Draco indices." << io::endl;
            return false;
    }
}

bool DracoMesh::getVertexAttributes(uint32_t attributeId, cgltf_accessor* target) const {
    // Return early if we've already decompressed this data.
    if (target->buffer_view) {
        return true;
    }

    // Return early if no such attribute exists.
    draco::Mesh* mesh = mDetails->mesh.get();
    const draco::PointAttribute* attr = mesh->GetAttributeByUniqueId(attributeId);
    if (!attr) {
        slog.e << "Unknown Draco point attribute." << io::endl;
        return false;
    }

    // Check if the accessor's vertex count matches with the Draco vertex count. If a mismatch
    // occurs we try to recover by adjusting the vertex count. Even though this is a spec violation,
    // this often occurs in the wild. If we need to be even more robust, see the class comment for
    // DracoMesh.
    uint32_t count = mesh->num_points();
    if (target->count != count) {
        slog.e << "The glTF accessor wants " << target->count << " vertices, "
               << "but the decoded Draco mesh has " << count << " vertices." << io::endl;

        // It is tempting to degrade gracefully by processing only the lesser of the two
        // counts, but doing so would lead to invalid indices in the index buffer.
        return false;
    }

    switch (target->component_type) {
        case cgltf_component_type_r_8:
            return convertAttribs<int8_t>(target, attr, count, mDetails.get());
        case cgltf_component_type_r_8u:
            return convertAttribs<uint8_t>(target, attr, count, mDetails.get());
        case cgltf_component_type_r_16:
            return convertAttribs<int16_t>(target, attr, count, mDetails.get());
        case cgltf_component_type_r_16u:
            return convertAttribs<uint16_t>(target, attr, count, mDetails.get());
        case cgltf_component_type_r_32u:
            return convertAttribs<uint32_t>(target, attr, count, mDetails.get());
        case cgltf_component_type_r_32f:
            return convertAttribs<float>(target, attr, count, mDetails.get());
        default:
            slog.e << "Unexpected component type for Draco vertices." << io::endl;
            return false;
    }
}

#else // #if GLTFIO_DRACO_SUPPORTED

DracoMesh::~DracoMesh() {}
DracoMesh* DracoMesh::decode(const uint8_t* data, size_t dataSize) { return nullptr; }
DracoMesh* DracoMesh::createForTest(draco::Mesh*, uint32_t) { return nullptr; }

bool DracoMesh::getFaceIndices(cgltf_accessor* target) const {
    return false;
}

bool DracoMesh::getVertexAttributes(uint32_t attributeId, cgltf_accessor* target) const {
    return false;
}

#endif

} // namespace filament::gltfio
