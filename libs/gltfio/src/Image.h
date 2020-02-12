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

#ifndef GLTFIO_IMAGE_H
#define GLTFIO_IMAGE_H

#include <filament/Engine.h>

#include <backend/PixelBufferDescriptor.h>

namespace gltfio {

filament::backend::PixelBufferDescriptor decode_from_memory(filament::Engine* engine,
        uint8_t const *buffer, int len,
        int *x, int *y, int *channels_in_file, int ncomp);

int decode_info_from_memory(filament::Engine* engine, uint8_t const *buffer, int len, int *x,
        int *y, int *comp);

#if defined(__EMSCRIPTEN__) || defined(ANDROID)
    #define GLTFIO_USE_FILESYSTEM 0
#else
    #define GLTFIO_USE_FILESYSTEM 1

    filament::backend::PixelBufferDescriptor decode_from_file(filament::Engine* engine,
            const char* path, int *x, int *y,
            int *channels_in_file, int ncomp);

    int decode_info_from_file(filament::Engine* engine, const char* path, int *x, int *y,
            int *comp);

#endif

}

#endif // GLTFIO_IMAGE_H
