/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <jni.h>

#include <filament/Engine.h>
#include <filament/Texture.h>

#include <utils/Log.h>

#include "common/NioUtils.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_HDR

#include <stb_image.h>

using namespace filament;
using namespace utils;

using PixelBufferDescriptor = Texture::PixelBufferDescriptor;

jlong nCreateHDRTexture(JNIEnv* env, jclass,
        jlong nativeEngine, jobject javaBuffer, jint remaining, jint internalFormat) {

    Engine* engine = (Engine*) nativeEngine;
    AutoBuffer buffer(env, javaBuffer, remaining);
    Texture::InternalFormat textureFormat = (Texture::InternalFormat) internalFormat;

    auto dataPtr = (const stbi_uc*) buffer.getData();
    const size_t byteCount = buffer.getSize();

    int width, height, nchan;

    float* const floatsPtr = stbi_loadf_from_memory(dataPtr, byteCount, &width, &height, &nchan, 3);
    if (floatsPtr == nullptr) {
        slog.e << "Unable to decode HDR image: " << stbi_failure_reason() << io::endl;
        return 0;
    }

    Texture* texture = Texture::Builder()
        .width(width)
        .height(height)
        .levels(0xff)
        .sampler(Texture::Sampler::SAMPLER_2D)
        .format(textureFormat)
        .build(*engine);

    if (texture == nullptr) {
        slog.e << "Unable to create Filament Texture from HDR image." << io::endl;
        stbi_image_free(floatsPtr);
        return 0;
    }

    PixelBufferDescriptor::Callback freeCallback = [](void* buf, size_t, void*) {
        stbi_image_free(buf);
    };

    PixelBufferDescriptor pbd(
        (void const* ) floatsPtr,
        width * height * 3 * sizeof(float),
        PixelBufferDescriptor::PixelDataFormat::RGB,
        PixelBufferDescriptor::PixelDataType::FLOAT,
        freeCallback);

    // Note that the setImage call could fail (e.g. due to an invalid combination of internal format
    // and PixelDataFormat) but there is no way of detecting such a failure.
    texture->setImage(*engine, 0, std::move(pbd));

    texture->generateMipmaps(*engine);

    return (jlong) texture;
}
