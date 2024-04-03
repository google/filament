/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_OPENGL_GLTEXTURE_H
#define TNT_FILAMENT_BACKEND_OPENGL_GLTEXTURE_H

#include "DriverBase.h"

#include "gl_headers.h"

#include <backend/platforms/OpenGLPlatform.h>

#include <stdint.h>

namespace filament::backend {

struct GLTexture : public HwTexture {
    using HwTexture::HwTexture;
    struct GL {
        GL() noexcept : imported(false), sidecarSamples(1), reserved(0) {}
        GLuint id = 0;          // texture or renderbuffer id
        GLenum target = 0;
        GLenum internalFormat = 0;
        GLuint sidecarRenderBufferMS = 0;  // multi-sample sidecar renderbuffer

        // texture parameters go here too
        GLfloat anisotropy = 1.0;
        int8_t baseLevel = 127;
        int8_t maxLevel = -1;
        uint8_t targetIndex = 0;    // optimization: index corresponding to target
        bool imported           : 1;
        uint8_t sidecarSamples  : 4;
        uint8_t reserved        : 3;
    } gl;

    OpenGLPlatform::ExternalTexture* externalTexture = nullptr;
};


} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_OPENGL_GLTEXTURE_H
