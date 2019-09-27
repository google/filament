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

#include <rays/PathTracer.h>

#include <image/LinearImage.h>

#include <utils/Path.h>

#include <stdint.h>

namespace gltfio {

/**
 * \struct AssetPipeline AssetPipeline.h gltfio/AssetPipeline.h
 * \brief Offers access to glTF processing facilities, independent of the Filament renderer.
 *
 * This is the place for things like atlasing, baking, and optimization of glTF scenes.
 *
 * \warning AssetPipeline is not required for simple glTF loading and is not included in
 * \c libgltfio_core.
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
     * bin file that only contains buffer data. Returns false if an error occurred.
     *
     * The supplied binPath should live in the same folder as the jsonPath.
     */
    bool save(AssetHandle, const utils::Path& jsonPath, const utils::Path& binPath);

    /**
     * Flattens and sanitizes a scene by baking transforms, dereferencing shared meshes, providing
     * 32-bit indices, converting positions to fp32, etc.
     *
     * This also removes non-triangle primitives and pulls out individual primitives such that each
     * resulting mesh has only one primitive. The triangles in the resulting asset should be
     * visually equivalent to the source, although the underlying scene structure and
     * resource-sharing will be lost.
     *
     * Assets with animation, skins, and morph targets are not supported.
     */
    AssetHandle flatten(AssetHandle source, uint32_t flags = ~0u);

    static constexpr uint32_t SCALE_TO_UNIT = 1 << 0;
    static constexpr uint32_t FILTER_TRIANGLES = 1 << 1;

    static const char* const BAKED_UV_ATTRIB;
    static const int BAKED_UV_ATTRIB_INDEX;

    /**
     * Generates atlases and UV data for a flattened asset that are suitable for baking lightmaps.
     *
     * The topology of the meshes in the resulting asset is potentially different from the source
     * asset (e.g., new vertices might be inserted). The newly generated UV set is placed into the
     * attribute slot named BAKED_UV_ATTRIB.
     *
     * If some of the generated charts are badly sized, increasing the maxIterations argument can
     * help by allowing xatlas to try different seed points.
     */
    AssetHandle parameterize(AssetHandle source, int maxIterations = 2);

    /**
     * Strips all textures and materials from a flattened asset, replacing them with a nonlit
     * material that samples from the texture at the given path.
     */
    AssetHandle generatePreview(AssetHandle source, const utils::Path& texture);

    /**
     * Replaces or adds the given occlusion texture to all primitives that have BAKED_UV_ATTRIB.
     */
    AssetHandle replaceOcclusion(AssetHandle source, const utils::Path& texture);

    /**
     * Modifies the occlusion texture URI for all primitives that have BAKED_UV_ATTRIB.
     */
    void setOcclusionUri(AssetHandle source, const utils::Path& texture);

    /**
     * Modifies the baseColor texture URI for all materials.
     */
    void setBaseColorUri(AssetHandle source, const utils::Path& texture);

    /**
     * Signals that a region of a path-traced image is available (used for progress notification).
     * This can be called from any thread.
     */
    using RenderTileCallback = filament::rays::TileCallback;

    /**
     * Signals that a complete path-traced image has become available.
     * This can be called from any thread.
     */
    using RenderDoneCallback = filament::rays::DoneCallback;

    struct RenderOptions {
        RenderTileCallback progress = nullptr;
        RenderDoneCallback done = nullptr;
        void* userData = nullptr;
        size_t samplesPerPixel = 256;
        float aoRayNear = std::numeric_limits<float>::epsilon() * 100.0f;
        bool enableDenoise = true;
        bool enableDilation = true;
    };

    /**
     * Consumes a parameterized glTF asset and produces a single-channel image with ambient
     * occlusion. Requires the presence of BAKED_UV_ATTRIB in the source asset.
     *
     * This invokes a simple path tracer that operates on tiles of the target image. It spins
     * up a number of threads and triggers a callback every time a new tile has been rendered.
     * Clients should not destroy the AssetPipeline before the done callback is triggered.
     */
    void bakeAmbientOcclusion(AssetHandle source, image::LinearImage target,
            const RenderOptions& options);

    /**
     * Consumes a glTF asset and produces a single-channel image with ambient occlusion rendered
     * from the given camera.
     * 
     * This method is not related to baking and was initially authored for diagnostic purposes.
     * The progress callback is similar to bakeAmbientOcclusion.
     * Clients should not destroy the AssetPipeline before the done callback is triggered.
     */
    void renderAmbientOcclusion(AssetHandle source, image::LinearImage target,
            const filament::rays::SimpleCamera& camera,
            const RenderOptions& options);

    /** Similar to bakeAmbientOcclusion but produces all outputs. */
    void bakeAllOutputs(AssetHandle source,
            image::LinearImage targets[filament::rays::OUTPUTPLANE_COUNT],
            const RenderOptions& options);

    static bool isFlattened(AssetHandle source);

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
