/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <stdlib.h>
#include <string.h>

#include <filament/VertexBuffer.h>

#include "API.h"

using namespace filament;
using namespace driver;

VertexBuffer::Builder *Filament_VertexBuffer_CreateBuilder() {
    return new VertexBuffer::Builder();
}

void Filament_VertexBuffer_DestroyBuilder(VertexBuffer::Builder *builder) {
    delete builder;
}

void Filament_VertexBuffer_BuilderVertexCount(
        VertexBuffer::Builder *builder, uint32_t vertexCount) {
    builder->vertexCount(vertexCount);
}

void Filament_VertexBuffer_BuilderBufferCount(
        VertexBuffer::Builder *builder, uint8_t bufferCount) {
    builder->bufferCount(bufferCount);
}

void Filament_VertexBuffer_BuilderAttribute(
        VertexBuffer::Builder *builder, VertexAttribute attribute,
        uint8_t bufferIndex, VertexBuffer::AttributeType attributeType,
        uint32_t byteOffset, uint8_t byteStride) {
    builder->attribute(attribute, bufferIndex, attributeType, byteOffset,
            byteStride);
}

void Filament_VertexBuffer_BuilderNormalized(
        VertexBuffer::Builder *builder, VertexAttribute attribute) {
    builder->normalized(attribute);
}

VertexBuffer *Filament_VertexBuffer_BuilderBuild(VertexBuffer::Builder *builder, Engine *engine) {
    return builder->build(*engine);
}

int Filament_VertexBuffer_GetVertexCount(VertexBuffer *vertexBuffer) {
    return vertexBuffer->getVertexCount();
}

int Filament_VertexBuffer_SetBufferAt(
        VertexBuffer *vertexBuffer, Engine *engine, uint8_t bufferIndex, void *data,
        uint32_t sizeInBytes, uint32_t destOffsetInBytes, FFreeBufferFn freeBuffer,
        void *freeBufferArg) {
    BufferDescriptor desc(data, sizeInBytes,
            (BufferDescriptor::Callback) freeBuffer, freeBufferArg);

    vertexBuffer->setBufferAt(*engine, bufferIndex, std::move(desc),
            destOffsetInBytes, sizeInBytes);

    return 0;
}
