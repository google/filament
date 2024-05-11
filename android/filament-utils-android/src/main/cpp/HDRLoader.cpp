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

#include <imageio/HDRDecoder.h>

#include <utils/Log.h>

#include <sstream>

#include "common/NioUtils.h"

using namespace filament;
using namespace image;
using namespace utils;

using PixelBufferDescriptor = Texture::PixelBufferDescriptor;

jlong nCreateHDRTexture(JNIEnv* env, jclass,
        jlong nativeEngine, jobject javaBuffer, jint remaining, jint internalFormat) {

    Engine* engine = (Engine*) nativeEngine;
    AutoBuffer buffer(env, javaBuffer, remaining);
    Texture::InternalFormat textureFormat = (Texture::InternalFormat) internalFormat;

    auto dataPtr = (char const*) buffer.getData();
    const size_t byteCount = buffer.getSize();

    // This creates a copy but it's the easest way to create a memory stream.
    std::string ins(dataPtr, byteCount);
    std::istringstream in(ins);

    LinearImage* image = new LinearImage(ImageDecoder::decode(in, "memory.hdr"));

    // This can happen if a decoding error occurs.
    if (image->getChannels() != 3) {
        delete image;
        return 0;
    }

    Texture* texture = Texture::Builder()
        .width(image->getWidth())
        .height(image->getHeight())
        .levels(0xff)
        .sampler(Texture::Sampler::SAMPLER_2D)
        .format(textureFormat)
        .build(*engine);

    if (texture == nullptr) {
        slog.e << "Unable to create Filament Texture from HDR image." << io::endl;
        delete image;
        return 0;
    }

    PixelBufferDescriptor::Callback freeCallback = [](void* buf, size_t, void* userdata) {
        delete (LinearImage*) userdata;
    };

    PixelBufferDescriptor pbd(
        (void const* ) image->getPixelRef(),
        image->getWidth() * image->getHeight() * 3 * sizeof(float),
        PixelBufferDescriptor::PixelDataFormat::RGB,
        PixelBufferDescriptor::PixelDataType::FLOAT,
        freeCallback,
        image);

    // Note that the setImage call could fail (e.g. due to an invalid combination of internal format
    // and PixelDataFormat) but there is no way of detecting such a failure.
    texture->setImage(*engine, 0, std::move(pbd));

    texture->generateMipmaps(*engine);

    return (jlong) texture;
}
