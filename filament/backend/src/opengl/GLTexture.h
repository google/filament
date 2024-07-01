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

#include <backend/Handle.h>
#include <backend/platforms/OpenGLPlatform.h>

#include <stdint.h>

namespace filament::backend {

struct GLTextureRef {
    GLTextureRef() = default;
    uint16_t count = 1;
    int8_t baseLevel = 127;
    int8_t maxLevel = -1;
};

struct GLTexture : public HwTexture {
    using HwTexture::HwTexture;
    struct GL {
        GL() noexcept : imported(false), sidecarSamples(1), reserved1(0) {}
        GLuint id = 0;          // texture or renderbuffer id
        GLenum target = 0;
        GLenum internalFormat = 0;
        GLuint sidecarRenderBufferMS = 0;  // multi-sample sidecar renderbuffer

        // texture parameters go here too
        GLfloat anisotropy = 1.0;
        int8_t baseLevel = 127;
        int8_t maxLevel = -1;
        uint8_t reserved0 = 0;
        bool imported           : 1;
        uint8_t sidecarSamples  : 4;
        uint8_t reserved1       : 3;
    } gl;
    mutable Handle<GLTextureRef> ref;
    OpenGLPlatform::ExternalTexture* externalTexture = nullptr;
};


} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_OPENGL_GLTEXTURE_H
