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

#ifndef GLTFIO_DRACO_CACHE_H
#define GLTFIO_DRACO_CACHE_H

#include <cgltf.h>

#include <tsl/robin_map.h>

#include <memory>

#ifndef GLTFIO_DRACO_SUPPORTED
#define GLTFIO_DRACO_SUPPORTED 0
#endif

namespace filament::gltfio {

class DracoMesh;

// Manages a set of Draco meshes that can be looked up using cgltf_buffer_view.
//
// The cache key is the buffer view that holds the compressed data. This allows the loader to
// avoid duplicated work when a single Draco mesh is referenced from multiple primitives.
class DracoCache {
public:
    DracoMesh* findOrCreateMesh(const cgltf_buffer_view* key);
private:
    tsl::robin_map<const cgltf_buffer_view*, std::unique_ptr<DracoMesh>> mCache;
};

// Decodes a Draco mesh upon construction and retains the results.
//
// The DracoMesh API leverages cgltf accessor structs in a way that bears explanation. These are
// read / write parameters that tell the decoder where to write the decoded data, and what format
// is desired. The buffer_view in the accessor should be null unless decompressed data is already
// loaded. This tells the decoder that it should create a buffer_view and a buffer. The buffer
// view, the buffer, and the buffer's data are all automatically freed when DracoMesh is destroyed.
//
// Note that in the gltfio architecture, the AssetLoader has the job of constructing VertexBuffer
// objects while the ResourceLoader has the job of populating them asychronously. This means that
// our Draco decoder relies on the accessor fields being 100% correct. If we had to be robust
// against faulty accessor information, we would need to replace the VertexBuffer object that was
// created in the AssetLoader, which would be a messy process.
class DracoMesh {
public:
    static DracoMesh* decode(const uint8_t* compressedData, size_t compressedSize);
    bool getFaceIndices(cgltf_accessor* destination) const;
    bool getVertexAttributes(uint32_t attributeId, cgltf_accessor* destination) const;
    ~DracoMesh();
private:
    DracoMesh(struct DracoMeshDetails* details);
    std::unique_ptr<struct DracoMeshDetails> mDetails;
};

} // namespace filament::gltfio

#endif // GLTFIO_DRACO_CACHE_H
