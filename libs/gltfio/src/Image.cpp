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

#include "Image.h"

#include <filament/Texture.h>

#define STB_IMAGE_IMPLEMENTATION

// gltfio supports PNG and JPEG, disable all other formats.
#define STBI_NO_BMP
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM

#if !GLTFIO_USE_FILESYSTEM
#define STBI_NO_STDIO
#endif

#include <stb_image.h>

using namespace filament;

static const auto FREE_CALLBACK = [](void* mem, size_t, void*) { free(mem); };

namespace gltfio {

backend::PixelBufferDescriptor decode_from_memory(Engine* engine, uint8_t const *buffer, int len,
        int *x, int *y, int *channels_in_file, int ncomp) {
    uint8_t* data = stbi_load_from_memory(buffer, len, x, y, channels_in_file, ncomp);
    int width = *x;
    int height = *y;
    return backend::PixelBufferDescriptor(data, width * height * 4, Texture::Format::RGBA,
            Texture::Type::UBYTE, FREE_CALLBACK);
}

int decode_info_from_memory(Engine* engine, uint8_t const *buffer, int len, int *x, int *y,
        int *channels_in_file) {
    return stbi_info_from_memory(buffer, len, x, y, channels_in_file);
}

#if GLTFIO_USE_FILESYSTEM

backend::PixelBufferDescriptor decode_from_file(Engine* engine, const char* path, int *x, int *y,
        int *channels_in_file, int ncomp) {
    uint8_t* data = stbi_load(path, x, y, channels_in_file, ncomp);
    int width = *x;
    int height = *y;
    return backend::PixelBufferDescriptor(data, width * height * 4, Texture::Format::RGBA,
            Texture::Type::UBYTE, FREE_CALLBACK);
}

int decode_info_from_file(Engine* engine, const char* path, int *x, int *y, int *channels_in_file) {
    return stbi_info(path, x, y, channels_in_file);
}

#endif // GLTFIO_USE_FILESYSTEM

}
