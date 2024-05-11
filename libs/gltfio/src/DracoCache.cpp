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

#if GLTFIO_DRACO_SUPPORTED
#include <draco/compression/decode.h>
#endif

#include <utils/compiler.h>
#include <utils/Log.h>

#if GLTFIO_DRACO_SUPPORTED

#include <memory>
#include <vector>

using std::unique_ptr;
using std::vector;

#endif

using namespace utils;

namespace filament::gltfio {

DracoMesh* DracoCache::findOrCreateMesh(const cgltf_buffer_view* key) {
    auto iter = mCache.find(key);
    if (iter != mCache.end()) {
        return iter->second.get();
    }
    assert(key->buffer && key->buffer->data);
    const uint8_t* compressedData = key->offset + (uint8_t*) key->buffer->data;
    DracoMesh* mesh = DracoMesh::decode(compressedData, key->size);
    mCache.emplace(key, mesh);
    return mesh;
}

DracoMesh::DracoMesh(struct DracoMeshDetails* details) : mDetails(details) {}

#if GLTFIO_DRACO_SUPPORTED

struct DracoMeshDetails {
    unique_ptr<draco::Mesh> mesh;
    vector<unique_ptr<cgltf_buffer_view>> views;
    vector<unique_ptr<cgltf_buffer>> buffers;
};

DracoMesh::~DracoMesh() {
    for (auto& buffer : mDetails->buffers) {
        free(buffer->data);
    }
}

// Gets the number of components in the given cgltf vector type, or -1 for matrices.
UTILS_UNUSED_IN_RELEASE static int getNumComponents(cgltf_type ctype) {
    return ((int) ctype) <= 4 ? ((int) ctype) : -1;
}

// Allocates and populates the given buffer view with indices from the given Draco mesh.
template<typename T>
static void convertFaces(cgltf_accessor* target, const draco::Mesh* mesh) {
    assert(target->stride == sizeof(T));

    const cgltf_size size = mesh->num_faces() * 3 * sizeof(T);
    cgltf_buffer_view* view = target->buffer_view;
    cgltf_buffer* buffer = view->buffer;
    *buffer = { nullptr, size, nullptr, malloc(size) };
    *view = { nullptr, buffer, 0, size, 0, cgltf_buffer_view_type_indices };
    T* dest = (T*) buffer->data;
    for (uint32_t id = 0, n = mesh->num_faces(); id < n; ++id) {
        draco::Mesh::Face face = mesh->face(draco::FaceIndex(id));
        *dest++ = face[0].value();
        *dest++ = face[1].value();
        *dest++ = face[2].value();
    }
}

// Converts vertex attributes into the desired format and populates the given cgltf buffer.
template<typename T>
static void convertAttribs(cgltf_accessor* target, const draco::PointAttribute* attr, uint32_t n) {
    const int8_t ncomps = attr->num_components();
    assert(ncomps <= 4 && ncomps == getNumComponents(target->type));
    assert(target->stride == attr->num_components() * sizeof(T));

    const uint32_t size = target->stride * n;
    cgltf_buffer_view* view = target->buffer_view;
    cgltf_buffer* buffer = view->buffer;
    *buffer = { nullptr, size, nullptr, malloc(size) };
    *view = { nullptr, buffer, 0, size, 0, cgltf_buffer_view_type_vertices };
    T* dest = (T*) buffer->data;
    for (draco::PointIndex i(0); i < n; ++i, dest += ncomps) {
        attr->ConvertValue(attr->mapped_index(i), ncomps, dest);
    }
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

bool DracoMesh::getFaceIndices(cgltf_accessor* target) const {
    // Return early if we've already decompressed this data.
    if (target->buffer_view) {
        return true;
    }

    draco::Mesh* mesh = mDetails->mesh.get();

    // Check the accessor's index count against the number of faces in the Draco mesh.
    // It would be tricky to be robust against a mismatch; see the class comment for DracoMesh.
    uint32_t count = mesh->num_faces() * 3;
    if (target->count != count) {
        slog.e << "The glTF accessor wants " << target->count << " indices, "
               << "but the decoded Draco mesh has " <<  count << " indices." << io::endl;
        return false;
    }

    cgltf_buffer_view* view = new cgltf_buffer_view;
    cgltf_buffer* buffer = view->buffer = new cgltf_buffer;

    mDetails->views.emplace_back(view);
    mDetails->buffers.emplace_back(buffer);

    target->offset = 0;
    target->buffer_view = view;

    switch (target->component_type) {
        case cgltf_component_type_r_16u: convertFaces<uint16_t>(target, mesh); break;
        case cgltf_component_type_r_32u: convertFaces<uint32_t>(target, mesh); break;
        case cgltf_component_type_r_8u: convertFaces<uint8_t>(target, mesh); break;
        default:
            slog.e << "Unexpected component type for Draco indices." << io::endl;
            return false;
    }
    return true;
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
               << "but the decoded Draco mesh has " <<  count << " vertices." << io::endl;

        // It is tempting to degrade gracefully by processing only the lesser of the two
        // counts, but doing so would lead to invalid indices in the index buffer.
        return false;
    }

    cgltf_buffer_view* view = new cgltf_buffer_view;
    cgltf_buffer* buffer = view->buffer = new cgltf_buffer;

    mDetails->views.emplace_back(view);
    mDetails->buffers.emplace_back(buffer);

    target->offset = 0;
    target->buffer_view = view;

    switch (target->component_type) {
	    case cgltf_component_type_r_8: convertAttribs<int8_t>(target, attr, count); break;
	    case cgltf_component_type_r_8u: convertAttribs<uint8_t>(target, attr, count); break;
	    case cgltf_component_type_r_16: convertAttribs<int16_t>(target, attr, count); break;
	    case cgltf_component_type_r_16u: convertAttribs<uint16_t>(target, attr, count); break;
	    case cgltf_component_type_r_32u: convertAttribs<uint32_t>(target, attr, count); break;
	    case cgltf_component_type_r_32f: convertAttribs<float>(target, attr, count); break;
        default:
            slog.e << "Unexpected component type for Draco vertices." << io::endl;
            break;
    }

    return true;
}

#else // #if GLTFIO_DRACO_SUPPORTED

DracoMesh::~DracoMesh() {}
struct DracoMeshDetails {};
DracoMesh* DracoMesh::decode(const uint8_t* data, size_t dataSize) { return nullptr; }

bool DracoMesh::getFaceIndices(cgltf_accessor* target) const {
    return false;
}

bool DracoMesh::getVertexAttributes(uint32_t attributeId, cgltf_accessor* target) const {
    return false;
}

#endif

} // namespace filament::gltfio
