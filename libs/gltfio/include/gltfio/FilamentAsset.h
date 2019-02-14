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

/**
 * BufferBinding is a read-only structure that tells clients how to load a source blob into a
 * VertexBuffer, IndexBuffer, or "animation buffer".
 * 
 * Each binding instance corresponds to one of the following:
 * 
 *  (a) One call to VertexBuffer::setBufferAt().
 *  (b) One call to IndexBuffer::setBuffer().
 *  (c) One memcpy into an animation buffer.
 * 
 * Animation buffers are blobs of opaque CPU-side memory that hold keyframe values. These are
 * consumed by the AnimationHelper class.
 */
struct BufferBinding {
    const char* uri;    // unique identifier for the source blob
    uint32_t totalSize; // size in bytes of the source blob at the given URI

    int bufferIndex; // only used when the destination is a VertexBuffer
    uint32_t offset; // byte count used only for vertex and index buffers
    uint32_t size;   // byte count used only for vertex and index buffers

    // Only one of the following destinations can be non-null.
    filament::VertexBuffer* vertexBuffer;
    filament::IndexBuffer* indexBuffer;
    uint8_t* animationBuffer;
    
};

/** Describes a binding from a Texture to a MaterialInstance. */
struct TextureBinding {
    const char* uri;
    const char* mimeType;
    filament::MaterialInstance* materialInstance;
    const char* materialParameter;
    filament::TextureSampler sampler;
};

/**
 * FilamentAsset owns a bundle of Filament objects that have been created by AssetLoader.
 *
 * For usage instructions, see the comment block for AssetLoader.
 *
 * This class holds strong references to entities (renderables and transforms) that have been loaded
 * from a glTF asset, as well as strong references to VertexBuffer, IndexBuffer, and
 * MaterialInstance.
 *
 * Clients must iterate over texture uri's and create Texture objects, unless the asset was loaded
 * from a GLB file. Clients should also iterate over buffer uri's and call VertexBuffer::setBufferAt
 * and IndexBuffer::setBuffer as needed. See BindingHelper to simplify this process.
 *
 * TODO: This supports skinning but not morphing.
 * TODO: Only the default glTF scene is loaded, other glTF scenes are ignored.
 * TODO: Cameras, extras, and extensions are ignored.
 */
class FilamentAsset {
public:

    /** Gets the list of renderables and light sources. */
    size_t getEntityCount() const noexcept;
    const utils::Entity* getEntities() const noexcept;

    /** Gets the transform root, this has no renderable component, just an identity transform. */
    utils::Entity getRoot() const noexcept;

    /** Gets all material instances.  These are already bound to renderables and textures. */
    size_t getMaterialInstanceCount() const noexcept;
    const filament::MaterialInstance* const* getMaterialInstances() const noexcept;

    /** Gets loading instructions for vertex buffers, index buffers, and animation buffers. */
    size_t getBufferBindingCount() const noexcept;
    const BufferBinding* getBufferBindings() const noexcept;

    /** Gets loading instructions for textures. */
    size_t getTextureBindingCount() const noexcept;
    const TextureBinding* getTextureBindings() const noexcept;

    /** Gets the bounding box computed from the supplied min / max values in glTF accessors. */
    filament::Aabb getBoundingBox() const noexcept;
    
    /**
     * Reclaims CPU-side memory for URI strings, binding lists, and raw animation data.
     *
     * If using BindingHelper, clients should call this only after calling loadResources.
     * If using AnimationHelper, clients should call this only after constructing the helper object.
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

} // namespace gltfio

#endif // GLTFIO_FILAMENTASSET_H
