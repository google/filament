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

#ifndef GLTFIO_FILAMENTASSET_H
#define GLTFIO_FILAMENTASSET_H

#include <filament/Box.h>
#include <filament/TextureSampler.h>

#include <utils/Entity.h>

namespace filament {
    class Engine;
    class IndexBuffer;
    class MaterialInstance;
    class VertexBuffer;
}

namespace gltfio {

struct BufferBinding;
struct TextureBinding;
class Animator;

/**
 * \class FilamentAsset FilamentAsset.h gltfio/FilamentAsset.h
 * \brief Owns a bundle of Filament objects that have been created by AssetLoader.
 *
 * For usage instructions, see the documentation for AssetLoader.
 *
 * This class owns a hierarchy of entities that have been loaded from a glTF asset. Every entity has
 * a filament::TransformManager component, and some entities also have \c Name and/or \c Renderable
 * components.
 *
 * In addition to the aforementioned entities, an asset has strong ownership over a list of
 * filament::VertexBuffer, filament::IndexBuffer, filament::MaterialInstance, filament::Texture,
 * and, optionally, a simple animation engine (gltfio::Animator).
 *
 * Clients must use ResourceLoader to create filament::Texture objects, compute tangent quaternions,
 * and upload data into vertex buffers and index buffers.
 *
 * \todo Only the default glTF scene is loaded, other glTF scenes are ignored.
 * \todo Cameras, extras, and extensions are ignored.
 */
class FilamentAsset {
public:

    /**
     * Gets the list of entities, one for each glTF node. All of these have a Transform component.
     * Some of the returned entities may also have a Renderable component.
     */
    const utils::Entity* getEntities() const noexcept;

    /**
     * Gets the number of entities returned by getEntities().
     */
    size_t getEntityCount() const noexcept;

    /** Gets the transform root for the asset, which has no matching glTF node. */
    utils::Entity getRoot() const noexcept;

    /**
     * Pops a ready renderable off the queue, or returns 0 if no renderables have become ready.
     *
     * This helper method allows clients to progressively add renderables to the scene as textures
     * gradually become ready through asynchronous loading. For example, on every frame progressive
     * applications can do something like this:
     *
     *    while (utils::Entity e = popRenderable()) { scene.addEntity(e); }
     *
     * See ResourceLoader#asyncBeginLoad.
     */
    utils::Entity popRenderable() noexcept;

    /** Gets all material instances. These are already bound to renderables. */
    const filament::MaterialInstance* const* getMaterialInstances() const noexcept;

    /** Gets all material instances (non-const). These are already bound to renderables. */
    filament::MaterialInstance* const* getMaterialInstances() noexcept;

    /** Gets the number of materials returned by getMaterialInstances(). */
    size_t getMaterialInstanceCount() const noexcept;

    /** Gets resource URIs for all externally-referenced buffers. */
    const char* const* getResourceUris() const noexcept;

    /** Gets the number of resource URIs returned by getResourceUris(). */
    size_t getResourceUriCount() const noexcept;

    /** Gets the bounding box computed from the supplied min / max values in glTF accessors. */
    filament::Aabb getBoundingBox() const noexcept;

    /** Gets the NameComponentManager label for the given entity, if it exists. */
    const char* getName(utils::Entity) const noexcept;

    /**
     * Lazily creates the animation engine or returns it from the cache.
     * The animator is owned by the asset and should not be manually deleted.
     */
    Animator* getAnimator() noexcept;

    /**
     * Lazily creates a single LINES renderable that draws the transformed bounding-box hierarchy
     * for diagnostic purposes. The wireframe is owned by the asset so clients should not delete it.
     */
    utils::Entity getWireframe() noexcept;

    /**
     * Returns the Filament engine associated with the AssetLoader that created this asset.
     */
    filament::Engine* getEngine() const noexcept;

    /**
     * Reclaims CPU-side memory for URI strings, binding lists, and raw animation data.
     *
     * This should only be called after ResourceLoader::loadResources().
     * If using Animator, this should be called after getAnimator().
     */
    void releaseSourceData() noexcept;

    /**
     * Returns a weak reference to the underlying cgltf hierarchy. This becomes invalid after
     * calling releaseSourceData();
     */
    const void* getSourceAsset() noexcept;

    const BufferBinding* getBufferBindings() const noexcept;   //!< \deprecated please use ResourceLoader
    size_t getBufferBindingCount() const noexcept;             //!< \deprecated please use ResourceLoader
    const TextureBinding* getTextureBindings() const noexcept; //!< \deprecated please use ResourceLoader
    size_t getTextureBindingCount() const noexcept;            //!< \deprecated please use ResourceLoader

    /*! \cond PRIVATE */
protected:
    FilamentAsset() noexcept = default;
    ~FilamentAsset() = default;

public:
    FilamentAsset(FilamentAsset const&) = delete;
    FilamentAsset(FilamentAsset&&) = delete;
    FilamentAsset& operator=(FilamentAsset const&) = delete;
    FilamentAsset& operator=(FilamentAsset&&) = delete;
    /*! \endcond */
};

/**
 * \struct BufferBinding FilamentAsset.h gltfio/FilamentAsset.h
 * \brief Read-only structure that tells the resource loader how to load a source blob into a
 * filament::VertexBuffer, filament::IndexBuffer, etc.
 *
 * \warning Clients usually do not need to interact with BufferBinding directly, they can use
 * ResourceLoader instead.
 *
 * Each binding instance corresponds to one of the following:
 *
 * - One call to VertexBuffer::setBufferAt().
 * - One call to IndexBuffer::setBuffer().
 */
struct BufferBinding {
    const char* uri;      // unique identifier for the source blob
    uint32_t totalSize;   // size in bytes of the source blob at the given URI
    uint8_t bufferIndex;  // only used when the destination is a VertexBuffer
    uint32_t offset;      // byte count used only for vertex and index buffers
    uint32_t size;        // byte count used only for vertex and index buffers
    void** data;          // pointer to the resource data in the source asset (if loaded)

    // Only one of the following two destinations can be non-null.
    filament::VertexBuffer* vertexBuffer;
    filament::IndexBuffer* indexBuffer;

    bool convertBytesToShorts;   // the resource loader must convert the buffer from u8 to u16
    bool generateTrivialIndices; // the resource loader must generate indices like: 0, 1, 2, ...
    bool generateDummyData;      // the resource loader should generate a sequence of 1.0 values
    bool generateTangents;       // the resource loader should generate tangents
    bool sparseAccessor;         // the resource loader should apply a sparse data set

    bool isMorphTarget;
    uint8_t morphTargetIndex;
};

/**
 * \struct TextureBinding FilamentAsset.h gltfio/FilamentAsset.h
 * \brief Read-only structure that describes a binding between filament::Texture and
 * filament::MaterialInstance.
 *
 * \warning Clients usually do not need to interact with TextureBinding directly, they can use
 * ResourceLoader instead.
 */
struct TextureBinding {
    const char* uri;
    uint32_t totalSize;
    const char* mimeType;
    void** data;
    size_t offset;
    filament::MaterialInstance* materialInstance;
    const char* materialParameter;
    filament::TextureSampler sampler;
    bool srgb;
};

} // namespace gltfio

#endif // GLTFIO_FILAMENTASSET_H
