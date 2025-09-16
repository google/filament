/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_OPENGL_GLMEMORYMAPPEDBUFFER_H
#define TNT_FILAMENT_BACKEND_OPENGL_GLMEMORYMAPPEDBUFFER_H

#include "DriverBase.h"

#include <private/backend/HandleAllocator.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include "gl_headers.h"

#include <stddef.h>
#include <stdint.h>

namespace filament::backend {

class OpenGLContext;
class OpenGLDriver;
class BufferDescriptor;

struct GLBufferObject;

struct GLMemoryMappedBuffer : public HwMemoryMappedBuffer {
    BufferObjectHandle boh;
    MapBufferAccessFlags access;
    struct {
        void* vaddr = nullptr;
        uint32_t size = 0;
        uint32_t offset = 0;
        GLenum binding = 0;
        GLuint id = 0;
    } gl;

    GLMemoryMappedBuffer();

    GLMemoryMappedBuffer(OpenGLContext& glc, HandleAllocatorGL& handleAllocator,
            BufferObjectHandle boh, size_t offset, size_t size, MapBufferAccessFlags access);

    ~GLMemoryMappedBuffer();

    void unmap(OpenGLContext& gl, HandleAllocatorGL& handleAllocator) const;

    void copy(OpenGLContext& glc, OpenGLDriver& gld,
            size_t offset, size_t size, BufferDescriptor&& data) const;
};

} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_OPENGL_GLMEMORYMAPPEDBUFFER_H
