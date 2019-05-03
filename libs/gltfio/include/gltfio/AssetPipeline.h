/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef GLTFIO_ASSETPIPELINE_H
#define GLTFIO_ASSETPIPELINE_H

#include <stdint.h>

#include <utils/Path.h>

namespace gltfio {

/**
 * AssetPipeline offers access to glTF processing facilities, independent of the Filament renderer.
 *
 * This is the place for things like atlasing, baking, and optimization of glTF scenes.
 *
 * One instance of AssetPipeline is a context object that makes it simple to manage memory for all
 * involved resources. When the pipeline dies, all its associated asset data are freed, and all
 * associated AssetHandle objects become invalid.
 * 
 * Example usage:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AssetPipeline pipeline;
 * AssetPipeline::AssetHandle asset = pipeline.load("source.gltf");
 * asset = pipeline.flatten(asset);
 * asset = pipeline.parameterize(asset);
 * asset = pipeline.optimize(asset);
 * pipeline.save(asset, "result.gltf", "result.bin");
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
class AssetPipeline final {
public:

    /**
     * Opaque handle to an immutable glTF asset.
     * 
     * If any method returns a null handle, then something went wrong. This is a weak reference.
     * The underlying asset is freed only when the entire pipeline object is destroyed.
     */
    using AssetHandle = const void*;

    /**
     * Loads a glTF 2.0 asset from the filesystem, as well as all of its external buffers.
     */
    AssetHandle load(const utils::Path& fileOrDirectory);

    /**
     * Saves a flattened asset to the filesystem as a JSON-based glTF 2.0 file, as well as a sidecar
     * bin file that only contains buffer data.
     *
     * The supplied binPath should live in the same folder as the jsonPath.
     */
    void save(AssetHandle, const utils::Path& jsonPath, const utils::Path& binPath);

    /**
     * Flattens and sanitizes a scene by baking transforms, dereferencing shared meshes, providing
     * 32-bit indices, converting positions to fp32, etc.
     *
     * This also removes non-triangle primitives and pulls out individual primitives such that each
     * resulting mesh has only one primitive. The triangles in the resulting asset should be
     * visually equivalent to the source, although the underlying scene structure and
     * resource-sharing will be lost.
     */
    AssetHandle flatten(AssetHandle source, uint32_t flags = ~0u);

    static constexpr uint32_t DISCARD_TEXTURES = 1 << 0;
    static constexpr uint32_t FILTER_TRIANGLES = 1 << 1;

    /**
     * Generates atlases and UV data for a flattened asset that are suitable for baking lightmaps.
     *
     * The topology of the meshes in the resulting asset is potentially different from the source
     * asset (e.g., new vertices might be inserted).
     */
    AssetHandle parameterize(AssetHandle source);

    AssetPipeline();
    ~AssetPipeline();

    AssetPipeline(AssetPipeline const&) = delete;
    AssetPipeline(AssetPipeline&&) = delete;
    AssetPipeline& operator=(AssetPipeline const&) = delete;
    AssetPipeline& operator=(AssetPipeline&&) = delete;

private:
    void* mImpl;
};

} // namespace gltfio

#endif // GLTFIO_ASSETPIPELINE_H
