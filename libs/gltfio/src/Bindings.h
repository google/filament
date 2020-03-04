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

#ifndef GLTFIO_BINDINGS_H
#define GLTFIO_BINDINGS_H

namespace filament {
    class VertexBuffer;
    class IndexBuffer;
    class MaterialInstance;
    class TextureSampler;
}

namespace gltfio {

/**
 * Read-only structure that tells ResourceLoader how to load a source blob into a
 * filament::VertexBuffer, filament::IndexBuffer, etc.
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
 * Read-only structure that describes a binding between filament::Texture and
 * filament::MaterialInstance.
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

} // namsepace gltfio

#endif // GLTFIO_BINDINGS_H
