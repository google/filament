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

#include <algorithm>
#include <functional>

#include <filament/Engine.h>
#include <filament/Stream.h>
#include <filament/Texture.h>
#include <filament/driver/BufferDescriptor.h>

#include "API.h"

using namespace filament;
using namespace driver;


FBool Filament_Texture_IsFormatSupported(Engine *engine, FTextureFormat format) {
    return Texture::isTextureFormatSupported(*engine, format);
}

size_t Filament_Texture_ComputeDataSize(FPixelDataFormat format, FPixelDataType type,
        size_t stride, size_t height, size_t alignment) {
    return Texture::computeTextureDataSize(format, type, stride, height, alignment);
}

FTextureBuilder *Filament_Texture_Builder_Create() {
    return new Texture::Builder;
}

void Filament_Texture_Builder_Destroy(FTextureBuilder *builder) {
    delete builder;
}

void Filament_Texture_Builder_Width(FTextureBuilder *builder, uint32_t width) {
    builder->width(width);
}

void Filament_Texture_Builder_Height(FTextureBuilder *builder, uint32_t height) {
    builder->height(height);
}

void Filament_Texture_Builder_Depth(FTextureBuilder *builder, uint32_t depth) {
    builder->depth(depth);
}

void Filament_Texture_Builder_Levels(FTextureBuilder *builder, uint8_t levels) {
    builder->levels(levels);
}

void Filament_Texture_Builder_Sampler(FTextureBuilder *builder, FSamplerType sampler) {
    builder->sampler(sampler);
}

void Filament_Texture_Builder_Format(FTextureBuilder *builder, FTextureFormat format) {
    builder->format(format);
}

void Filament_Texture_Builder_Usage(FTextureBuilder *builder, FTextureUsage usage) {
    builder->usage(usage);
}

Texture *Filament_Texture_Builder_Build(FTextureBuilder *builder, Engine *engine) {
    return builder->build(*engine);
}

size_t Filament_Texture_GetWidth(Texture *texture, size_t level) {
    return texture->getWidth(level);
}

size_t Filament_Texture_GetHeight(Texture *texture, size_t level) {
    return texture->getHeight(level);
}

size_t Filament_Texture_GetDepth(Texture *texture, size_t level) {
    return texture->getDepth(level);
}

size_t Filament_Texture_GetLevels(Texture *texture) {
    return texture->getLevels();
}

FSamplerType Filament_Texture_GetTarget(Texture *texture) {
    return texture->getTarget();
}

FTextureFormat Filament_Texture_GetFormat(Texture *texture) {
    return texture->getFormat();
}

static PixelBufferDescriptor convertPixelBufferDescriptor(const FPixelBufferDescriptor &buffer) {
    return PixelBufferDescriptor(
            buffer.buffer,
            buffer.size,
            buffer.format,
            buffer.type,
            buffer.alignment,
            buffer.left,
            buffer.top,
            buffer.stride,
            buffer.freeBufferCallback,
            buffer.freeBufferArg
    );
}

void Filament_Texture_SetImage(Texture *texture, Engine *engine,
        size_t level,
        FPixelBufferDescriptor *buffer) {
    texture->setImage(*engine, level, std::move(convertPixelBufferDescriptor(*buffer)));
}

void Filament_Texture_SetSubImage(Texture *texture, Engine *engine,
        size_t level,
        uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        FPixelBufferDescriptor *buffer) {
    texture->setImage(*engine, level, xoffset, yoffset, width, height,
            std::move(convertPixelBufferDescriptor(*buffer)));
}

void Filament_Texture_SetCubeImage(Texture *texture, Engine *engine,
        size_t level,
        FPixelBufferDescriptor *buffer, FFaceOffsets *faceOffsets) {

    FaceOffsets fFaceOffsets;
    fFaceOffsets.px = faceOffsets->px;
    fFaceOffsets.nx = faceOffsets->nx;
    fFaceOffsets.py = faceOffsets->py;
    fFaceOffsets.ny = faceOffsets->ny;
    fFaceOffsets.pz = faceOffsets->pz;
    fFaceOffsets.nz = faceOffsets->nz;

    texture->setImage(*engine, level, std::move(convertPixelBufferDescriptor(*buffer)), fFaceOffsets);
}

void Filament_Texture_SetExternalImage(Texture *texture, Engine *engine, void *image) {
    texture->setExternalImage(*engine, image);
}

void Filament_Texture_SetExternalStream(Texture *texture, Engine *engine, FStream *stream) {
    texture->setExternalStream(*engine, stream);
}

void Filament_Texture_GenerateMipmaps(Texture *texture, Engine *engine) {
    texture->generateMipmaps(*engine);
}
