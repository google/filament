/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <functional>
#include <stdlib.h>
#include <string.h>

#include <filament/IndexBuffer.h>

#include "API.h"

using namespace filament;
using namespace driver;

IndexBuffer::Builder *Filament_IndexBuffer_CreateBuilder() {
  return new IndexBuffer::Builder();
}

void Filament_IndexBuffer_DestroyBuilder(IndexBuffer::Builder *builder) {
  delete builder;
}

void Filament_IndexBuffer_BuilderIndexCount(IndexBuffer::Builder *builder, uint32_t indexCount) {
  builder->indexCount(indexCount);
}

void Filament_IndexBuffer_BuilderBufferType(IndexBuffer::Builder *builder, IndexBuffer::IndexType indexType) {
  builder->bufferType(indexType);
}

IndexBuffer *Filament_IndexBuffer_BuilderBuild(IndexBuffer::Builder *builder, Engine *engine) {
  return builder->build(*engine);
}

int Filament_IndexBuffer_GetIndexCount(IndexBuffer *indexBuffer) {
  return indexBuffer->getIndexCount();
}

int Filament_IndexBuffer_SetBuffer(
    IndexBuffer *vertexBuffer, Engine *engine, void *data, uint32_t sizeInBytes,
    uint32_t destOffsetInBytes, FFreeBufferFn freeBuffer, void *freeBufferArg) {
  BufferDescriptor desc(data, sizeInBytes,
                        (BufferDescriptor::Callback) freeBuffer, freeBufferArg);

  vertexBuffer->setBuffer(*engine, std::move(desc), destOffsetInBytes,
                          sizeInBytes);

  return 0;
}
