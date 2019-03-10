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
 * FilamentAsset owns a bundle of Filament objects that have been created by AssetLoader.
 * For usage instructions, see the comment block for AssetLoader.
 *
 * This class owns a hierarchy of entities that have been loaded from a glTF asset. Every entity has
 * a TransformManager component, and some entities also have Name and/or Renderable components.
 *
 * In addition to the aforementioned entities, an asset has strong ownership over a list of
 * VertexBuffer, IndexBuffer, MaterialInstance, Texture, and, optionally, a simple animation engine.
 *
 * Clients must use ResourceLoader to create Texture objects, compute tangent quaternions, and
 * upload data into vertex buffers and index buffers.
 *
 * TODO: This supports skinning but not morphing.
 * TODO: Only the default glTF scene is loaded, other glTF scenes are ignored.
 * TODO: Cameras, extras, and extensions are ignored.
 */
class FilamentAsset {
public:

    /**
     * Gets the list of entities, one for each glTF node. All of these have a Transform component.
     * Some of the returned entities may also have a Renderable component.
     */
    size_t getEntityCount() const noexcept;
    const utils::Entity* getEntities() const noexcept;

    /** Gets the transform root for the asset, which has no matching glTF node. */
    utils::Entity getRoot() const noexcept;

    /** Gets all material instances. These are already bound to renderables. */
    size_t getMaterialInstanceCount() const noexcept;
    const filament::MaterialInstance* const* getMaterialInstances() const noexcept;

    /** Gets loading instructions for vertex buffers and index buffers. */
    size_t getBufferBindingCount() const noexcept;
    const BufferBinding* getBufferBindings() const noexcept;

    /** Gets loading instructions for textures. */
    size_t getTextureBindingCount() const noexcept;
    const TextureBinding* getTextureBindings() const noexcept;

    /** Gets the bounding box computed from the supplied min / max values in glTF accessors. */
    filament::Aabb getBoundingBox() const noexcept;

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
     * Reclaims CPU-side memory for URI strings, binding lists, and raw animation data.
     *
     * This should only be called after ResourceLoader::loadResources().
     * If using Animator, this should be called after getAnimator().
     */
    void releaseSourceData() noexcept;

protected:
    FilamentAsset() noexcept = default;
    ~FilamentAsset() = default;

public:
    FilamentAsset(FilamentAsset const&) = delete;
    FilamentAsset(FilamentAsset&&) = delete;
    FilamentAsset& operator=(FilamentAsset const&) = delete;
    FilamentAsset& operator=(FilamentAsset&&) = delete;
};

/**
 * BufferBinding is a read-only structure that tells clients how to load a source blob into a
 * VertexBuffer slot or IndexBuffer.
 *
 * Each binding instance corresponds to one of the following:
 *
 *  (a) One call to VertexBuffer::setBufferAt().
 *  (b) One call to IndexBuffer::setBuffer().
 *
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
};

/** Describes a binding from a Texture to a MaterialInstance. */
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
