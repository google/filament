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

#ifndef TNT_FILAMENT_BACKEND_OPENGL_GLBUFFEROBJECT_H
#define TNT_FILAMENT_BACKEND_OPENGL_GLBUFFEROBJECT_H

#include "DriverBase.h"

#include "gl_headers.h"

#include <backend/DriverEnums.h>

#include <stdint.h>

namespace filament::backend {

struct GLBufferObject : public HwBufferObject {
    using HwBufferObject::HwBufferObject;
    GLBufferObject(uint32_t size,
            BufferObjectBinding bindingType, BufferUsage usage) noexcept
            : HwBufferObject(size), usage(usage), bindingType(bindingType) {
    }

    struct {
        GLuint id;
        union {
            GLenum binding;
            void* buffer;
        };
    } gl;
    BufferUsage usage;
    BufferObjectBinding bindingType;
    uint16_t age = 0;
};

} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_OPENGL_GLBUFFEROBJECT_H
