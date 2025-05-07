/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "Utility.h"

#include "DracoCache.h"
#include "FFilamentAsset.h"
#include "GltfEnums.h"

#include <utils/Log.h>
#include <utils/Systrace.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
#include <meshoptimizer.h>

namespace filament::gltfio::utility {

using namespace utils;

void decodeDracoMeshes(cgltf_data const* gltf, cgltf_primitive const* prim,
        DracoCache* dracoCache) {
    if (!prim->has_draco_mesh_compression) {
        return;
    }

    // For a given primitive and attribute, find the corresponding accessor.
    auto findAccessor = [](const cgltf_primitive* prim, cgltf_attribute_type type, cgltf_int idx) {
        for (cgltf_size i = 0; i < prim->attributes_count; i++) {
            const cgltf_attribute& attr = prim->attributes[i];
            if (attr.type == type && attr.index == idx) {
                return attr.data;
            }
        }
        return (cgltf_accessor*) nullptr;
    };
    const cgltf_draco_mesh_compression& draco = prim->draco_mesh_compression;

    // Check if we have already decoded this mesh.
    DracoMesh* mesh = dracoCache->findOrCreateMesh(draco.buffer_view);
    if (!mesh) {
        slog.e << "Cannot decompress mesh, Draco decoding error." << io::endl;
        return;
    }

    // Copy over the decompressed data, converting the data type if necessary.
    if (prim->indices && !mesh->getFaceIndices(prim->indices)) {
        return;
    }

    // Go through each attribute in the decompressed mesh.
    for (cgltf_size i = 0; i < draco.attributes_count; i++) {

        // In cgltf, each Draco attribute's data pointer is an attribute id, not an accessor.
        const uint32_t id = draco.attributes[i].data - gltf->accessors;

        // Find the destination accessor; this contains the desired component type, etc.
        const cgltf_attribute_type type = draco.attributes[i].type;
        const cgltf_int index = draco.attributes[i].index;
        cgltf_accessor* accessor = findAccessor(prim, type, index);
        if (!accessor) {
            slog.w << "Cannot find matching accessor for Draco id " << id << io::endl;
            continue;
        }

        // Copy over the decompressed data, converting the data type if necessary.
        if (!mesh->getVertexAttributes(id, accessor)) {
            break;
        }
    }
}

void decodeMeshoptCompression(cgltf_data* data) {
    for (size_t i = 0; i < data->buffer_views_count; ++i) {
        if (!data->buffer_views[i].has_meshopt_compression) {
            continue;
        }
        cgltf_meshopt_compression* compression = &data->buffer_views[i].meshopt_compression;
        const uint8_t* source = (const uint8_t*) compression->buffer->data;
        assert_invariant(source);
        source += compression->offset;

        // This memory is freed by cgltf.
        void* destination = malloc(compression->count * compression->stride);
        assert_invariant(destination);

        UTILS_UNUSED_IN_RELEASE int error = 0;
        switch (compression->mode) {
            case cgltf_meshopt_compression_mode_invalid:
                break;
            case cgltf_meshopt_compression_mode_attributes:
                error = meshopt_decodeVertexBuffer(destination, compression->count,
                        compression->stride, source, compression->size);
                break;
            case cgltf_meshopt_compression_mode_triangles:
                error = meshopt_decodeIndexBuffer(destination, compression->count,
                        compression->stride, source, compression->size);
                break;
            case cgltf_meshopt_compression_mode_indices:
                error = meshopt_decodeIndexSequence(destination, compression->count,
                        compression->stride, source, compression->size);
                break;
            default:
                assert_invariant(false);
                break;
        }
        assert_invariant(!error);

        switch (compression->filter) {
            case cgltf_meshopt_compression_filter_none:
                break;
            case cgltf_meshopt_compression_filter_octahedral:
                meshopt_decodeFilterOct(destination, compression->count, compression->stride);
                break;
            case cgltf_meshopt_compression_filter_quaternion:
                meshopt_decodeFilterQuat(destination, compression->count, compression->stride);
                break;
            case cgltf_meshopt_compression_filter_exponential:
                meshopt_decodeFilterExp(destination, compression->count, compression->stride);
                break;
            default:
                assert_invariant(false);
                break;
        }

        data->buffer_views[i].data = destination;
    }
}

bool primitiveHasVertexColor(cgltf_primitive* inPrim) {
    for (int slot = 0; slot < inPrim->attributes_count; slot++) {
        const cgltf_attribute& inputAttribute = inPrim->attributes[slot];
        if (inputAttribute.type == cgltf_attribute_type_color) {
            return true;
        }
    }
    return false;
}

// Sometimes a glTF bufferview includes unused data at the end (e.g. in skinning.gltf) so we need to
// compute the correct size of the vertex buffer. Filament automatically infers the size of
// driver-level vertex buffers from the attribute data (stride, count, offset) and clients are
// expected to avoid uploading data blobs that exceed this size. Since this information doesn't
// exist in the glTF we need to compute it manually. This is a bit of a cheat, cgltf_calc_size is
// private but its implementation file is available in this cpp file.
uint32_t computeBindingSize(cgltf_accessor const* accessor) {
    cgltf_size element_size = cgltf_calc_size(accessor->type, accessor->component_type);
    return uint32_t(accessor->stride * (accessor->count - 1) + element_size);
}

void convertBytesToShorts(uint16_t* dst, uint8_t const* src, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        dst[i] = src[i];
    }
}

uint32_t computeBindingOffset(cgltf_accessor const* accessor) {
    return uint32_t(accessor->offset + accessor->buffer_view->offset);
}

bool requiresConversion(cgltf_accessor const* accessor) {
    if (UTILS_UNLIKELY(accessor->is_sparse)) {
        return true;
    }
    const cgltf_type type = accessor->type;
    const cgltf_component_type ctype = accessor->component_type;
    filament::VertexBuffer::AttributeType permitted;
    filament::VertexBuffer::AttributeType actual;
    UTILS_UNUSED_IN_RELEASE bool supported = getElementType(type, ctype, &permitted, &actual);
    assert_invariant(supported && "Unsupported types");
    return permitted != actual;
}

bool requiresPacking(cgltf_accessor const* accessor) {
    if (requiresConversion(accessor)) {
        return true;
    }
    const size_t dim = cgltf_num_components(accessor->type);
    switch (accessor->component_type) {
        case cgltf_component_type_r_8:
        case cgltf_component_type_r_8u:
            return accessor->stride != dim;
        case cgltf_component_type_r_16:
        case cgltf_component_type_r_16u:
            return accessor->stride != dim * 2;
        case cgltf_component_type_r_32u:
        case cgltf_component_type_r_32f:
            return accessor->stride != dim * 4;
        default:
            assert_invariant(false);
            return true;
    }
}

bool loadCgltfBuffers(cgltf_data const* gltf, char const* gltfPath,
        UriDataCacheHandle uriDataCacheHandle) {
    SYSTRACE_CONTEXT();
    SYSTRACE_NAME_BEGIN("Load buffers");
    cgltf_options options{};

    // For emscripten and Android builds we supply a custom file reader callback that looks inside a
    // cache of externally-supplied data blobs, rather than loading from the filesystem.

#if !GLTFIO_USE_FILESYSTEM
    struct Closure {
        UriDataCacheHandle uriDataCache;
        const cgltf_data* gltf;
    };

    Closure closure = { uriDataCacheHandle, gltf };

    options.file.user_data = &closure;

    options.file.read = [](const cgltf_memory_options* memoryOpts,
                                const cgltf_file_options* fileOpts, const char* path,
                                cgltf_size* size, void** data) {
        Closure* closure = (Closure*) fileOpts->user_data;
        auto& uriDataCache = closure->uriDataCache;

        if (auto iter = uriDataCache->find(path); iter != uriDataCache->end()) {
            *size = iter->second.size;
            *data = iter->second.buffer;
        } else {
            // Even if we don't find the given resource in the cache, we still return a successful
            // error code, because we allow downloads to finish after the decoding work starts.
            *size = 0;
            *data = 0;
        }

        return cgltf_result_success;
    };
#endif

    // Read data from the file system and base64 URIs.
    cgltf_result result = cgltf_load_buffers(&options, (cgltf_data*) gltf, gltfPath);
    if (result != cgltf_result_success) {
        slog.e << "Unable to load resources." << io::endl;
        return false;
    }

    SYSTRACE_NAME_END();

#ifndef NDEBUG
    if (cgltf_validate((cgltf_data*) gltf) != cgltf_result_success) {
        slog.e << "Failed cgltf validation." << io::endl;
        return false;
    }
#endif
    return true;
}

} // namespace filament::gltfio::utility
