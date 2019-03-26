/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "GPUBuffer.h"
#include "private/backend/DriverApi.h"

#include <math/half.h>

namespace filament {

using namespace backend;

static size_t dataTypeToSize(GPUBuffer::Element element) noexcept {
    using ET = GPUBuffer::ElementType;
    switch (element.type) {
        case ET::UINT8:  return element.size * sizeof(uint8_t);
        case ET::INT8:   return element.size * sizeof(int8_t);
        case ET::UINT16: return element.size * sizeof(uint16_t);
        case ET::INT16:  return element.size * sizeof(int16_t);
        case ET::UINT32: return element.size * sizeof(uint32_t);
        case ET::INT32:  return element.size * sizeof(int32_t);
        case ET::HALF:   return element.size * sizeof(math::half);
        case ET::FLOAT:  return element.size * sizeof(float);
    }
}

static backend::TextureFormat dataTypeToTextureFormat(GPUBuffer::Element element) noexcept {
    using TF = backend::TextureFormat;
    static const TF formats[8][4] = {
            { TF::R8UI,  TF::RG8UI,  TF::RGB8UI,  TF::RGBA8UI },
            { TF::R8I,   TF::RG8I,   TF::RGB8I,   TF::RGBA8I },
            { TF::R16UI, TF::RG16UI, TF::RGB16UI, TF::RGBA16UI },
            { TF::R16I,  TF::RG16I,  TF::RGB16I,  TF::RGBA16I },
            { TF::R32UI, TF::RG32UI, TF::RGB32UI, TF::RGBA32UI },
            { TF::R32I,  TF::RG32I,  TF::RGB32I,  TF::RGBA32I },
            { TF::R16F,  TF::RG16F,  TF::RGB16F,  TF::RGBA16F },
            { TF::R32F,  TF::RG32F,  TF::RGB32F,  TF::RGBA32F },
    };

    size_t index = size_t(element.type);
    assert(index < 8 && element.size > 0 && element.size <= 4);
    return formats[index][element.size - 1];
}

GPUBuffer::GPUBuffer(backend::DriverApi& driverApi, Element element, size_t rowSize, size_t rowCount)
        : mElement(element) {
    size_t size = dataTypeToSize(element) * rowSize * rowCount;
    mSize = (uint32_t) size;
    mWidth = (uint16_t) rowSize;
    mHeight = (uint16_t) rowCount;
    mRowSizeInBytes = uint16_t(dataTypeToSize(element) * rowSize);

    backend::TextureFormat format = dataTypeToTextureFormat(element);
    mTexture = driverApi.createTexture(SamplerType::SAMPLER_2D, 1, format, 1, mWidth, mHeight, 1,
            TextureUsage::DEFAULT);


    switch (mElement.size) {
        default: // cannot happen
        case 1: mFormat = backend::PixelDataFormat::R;    break;
        case 2: mFormat = backend::PixelDataFormat::RG;   break;
        case 3: mFormat = backend::PixelDataFormat::RGB;  break;
        case 4: mFormat = backend::PixelDataFormat::RGBA; break;
    }

    if (mElement.type != ElementType::FLOAT && mElement.type != ElementType::HALF) {
        // change R to R_INTEGER, RG to RG_INTEGER, etc.
        mFormat = backend::PixelDataFormat(uint8_t(mFormat) + 1);
    }

    switch (mElement.type) {
        case ElementType::UINT8:    mType = backend::PixelDataType::UBYTE;    break;
        case ElementType::INT8:     mType = backend::PixelDataType::BYTE;     break;
        case ElementType::UINT16:   mType = backend::PixelDataType::USHORT;   break;
        case ElementType::INT16:    mType = backend::PixelDataType::SHORT;    break;
        case ElementType::UINT32:   mType = backend::PixelDataType::UINT;     break;
        case ElementType::INT32:    mType = backend::PixelDataType::INT;      break;
        case ElementType::HALF:     mType = backend::PixelDataType::HALF;     break;
        case ElementType::FLOAT:    mType = backend::PixelDataType::FLOAT;    break;
    }
}

GPUBuffer::GPUBuffer(GPUBuffer&& rhs) noexcept {
    swap(rhs);
}

GPUBuffer& GPUBuffer::operator=(GPUBuffer&& rhs) noexcept {
    swap(rhs);
    return *this;
}

void GPUBuffer::swap(GPUBuffer& rhs) noexcept {
    std::swap(mTexture, rhs.mTexture);
    std::swap(mSize, rhs.mSize);
    std::swap(mWidth, rhs.mWidth);
    std::swap(mHeight, rhs.mHeight);
    std::swap(mRowSizeInBytes, rhs.mRowSizeInBytes);
    std::swap(mElement, rhs.mElement);
    std::swap(mFormat, rhs.mFormat);
    std::swap(mType, rhs.mType);
}

void GPUBuffer::terminate(backend::DriverApi& driverApi) noexcept {
    driverApi.destroyTexture(mTexture);
}

void GPUBuffer::commitSlow(backend::DriverApi& driverApi, void const* begin, void const* end) noexcept {
    const uintptr_t sizeInBytes = uintptr_t(end) - uintptr_t(begin);
    assert(sizeInBytes <= mRowSizeInBytes * mHeight);
    driverApi.update2DImage(mTexture, 0, 0, 0, mWidth, mHeight,
            { begin, sizeInBytes, mFormat, mType });
}

} // namespace filament
