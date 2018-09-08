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

#include <assert.h>

#include <filament/Stream.h>
#include <filament/driver/PixelBufferDescriptor.h>

#include "API.h"

using namespace filament;
using namespace driver;

Stream::Builder *Filament_Stream_Builder_Create() {
    return new Stream::Builder;
}

void Filament_Stream_Builder_Destroy(Stream::Builder *builder) {
    delete builder;
}

void Filament_Stream_Builder_StreamNative(Stream::Builder *builder, void *stream) {
    builder->stream(stream);
}

void Filament_Stream_Builder_StreamCopy(Stream::Builder *builder, intptr_t externalTextureId) {
    builder->stream(externalTextureId);
}

void Filament_Stream_Builder_Width(Stream::Builder *builder, uint32_t width) {
    builder->width(width);
}

void Filament_Stream_Builder_Height(Stream::Builder *builder, uint32_t height) {
    builder->height(height);
}

Stream *Filament_Stream_Builder_Build(Stream::Builder *builder, FEngine *engine) {
    return builder->build(*engine);
}

FBool Filament_Stream_IsNativeStream(Stream *stream) {
    return stream->isNativeStream();
}

void Filament_Stream_SetDimensions(Stream *stream, uint32_t width, uint32_t height) {
    return stream->setDimensions(width, height);
}

void Filament_Stream_ReadPixels(Stream *stream, uint32_t xoffset, uint32_t yoffset,
        uint32_t width, uint32_t height,
        FPixelBufferDescriptor *buffer) {

    PixelBufferDescriptor desc(
            buffer, buffer->size, buffer->format,
            buffer->type, buffer->alignment, buffer->left,
            buffer->top, buffer->stride, buffer->freeBufferCallback, buffer->freeBufferArg
    );
    stream->readPixels(xoffset, yoffset, width, height, std::move(desc));

}
