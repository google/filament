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

#include "GLMemoryMappedBuffer.h"

#include "GLBufferObject.h"
#include "GLUtils.h"
#include "OpenGLContext.h"
#include "OpenGLDriver.h"

#include "gl_headers.h"

#include <private/backend/HandleAllocator.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/BitmaskEnum.h>

#include <limits>
#include <utility>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace filament::backend {

GLMemoryMappedBuffer::GLMemoryMappedBuffer() = default;

GLMemoryMappedBuffer::~GLMemoryMappedBuffer() = default;

GLMemoryMappedBuffer::GLMemoryMappedBuffer(OpenGLContext& glc, HandleAllocatorGL& handleAllocator,
        BufferObjectHandle boh,
        size_t const offset, size_t const size, MapBufferAccessFlags const access)
            : boh(boh), access(access) {

    GLBufferObject* const bo = handleAllocator.handle_cast<GLBufferObject*>(boh);

    assert_invariant(bo);
    assert_invariant(bo->mappingCount < std::numeric_limits<uint8_t>::max());
    assert_invariant(offset + size <= bo->byteCount);

    if (any(access & MapBufferAccessFlags::WRITE_BIT)) {
        assert_invariant(any(bo->usage & BufferUsage::SHARED_WRITE_BIT));
    }

    if (UTILS_UNLIKELY(glc.isES2())) {
        gl.vaddr = static_cast<char*>(bo->gl.buffer) + offset;
        gl.size = size;
        gl.offset = offset;
        gl.binding = 0; // in ES2 mode, bo->gl.binding is not available
        gl.id = bo->gl.id;
        bo->age++; // technically we could do this only in copy()
        bo->mappingCount++;
        return;
    }

#if !defined(FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2)
        void* addr = nullptr;

#   if !defined(__EMSCRIPTEN__)
        // unsynchronized is implied (but is incompatible with reading)
        GLbitfield gl_access = GL_MAP_UNSYNCHRONIZED_BIT;
        if (any(access & MapBufferAccessFlags::WRITE_BIT)) {
            gl_access |= GL_MAP_WRITE_BIT;
        }
        if (any(access & MapBufferAccessFlags::INVALIDATE_RANGE_BIT)) {
            // note: GL_MAP_INVALIDATE_RANGE_BIT is incompatible with GL_MAP_READ_BIT
            gl_access |= GL_MAP_INVALIDATE_RANGE_BIT;
        }

        glc.bindBuffer(bo->gl.binding, bo->gl.id);

        addr = glMapBufferRange(bo->gl.binding, GLsizeiptr(offset), GLsizeiptr(size), gl_access);

        CHECK_GL_ERROR();
#   endif

    // if we failed here, addr would be nullptr
    gl.vaddr = addr;
    gl.size = size;
    gl.offset = offset;
    gl.binding = bo->gl.binding;
    gl.id = bo->gl.id;
    bo->mappingCount++;

#endif
}

void GLMemoryMappedBuffer::unmap(OpenGLContext& glc, HandleAllocatorGL& handleAllocator) const {
    GLBufferObject* const bo = handleAllocator.handle_cast<GLBufferObject*>(boh);
    assert_invariant(bo);
    assert_invariant(bo->mappingCount > 0);

    bo->mappingCount--;

    if (UTILS_UNLIKELY(glc.isES2())) {
        // nothing to do
        return;
    }

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    glc.bindBuffer(gl.binding, gl.id);
#   if !defined(__EMSCRIPTEN__)
        // don't unmap if we don't have a mapping or it didn't work
        if (UTILS_LIKELY(gl.vaddr)) {
            if (UTILS_UNLIKELY(glUnmapBuffer(gl.binding) == GL_FALSE)) {
                // TODO: According to the spec, UnmapBuffer can return FALSE in rare conditions (e.g.
                //   during a screen mode change). Note that this is not a GL error, but the whole
                //   mapping is lost. This is very problematic for us here.
            }
            CHECK_GL_ERROR();
        }
#   endif
#endif
}

void GLMemoryMappedBuffer::copy(OpenGLContext& glc, OpenGLDriver& gld,
        size_t const offset, BufferDescriptor&& data) const {
    assert_invariant(any(access & MapBufferAccessFlags::WRITE_BIT));
    assert_invariant(offset + data.size <= gl.size);

    if (UTILS_LIKELY(gl.vaddr)) {
        memcpy(static_cast<char *>(gl.vaddr) + offset, data.buffer, data.size);
    } else {
        assert_invariant(!glc.isES2());
        // we couldn't map (WebGL or error), revert to glBufferSubData. In the future we can
        // improve this by keeping around the BufferDescriptor and coalescing the calls
        // to glBufferSubData.
        glc.bindBuffer(gl.binding, gl.id);
        glBufferSubData(gl.binding, GLintptr(gl.offset + offset), GLsizeiptr(data.size), data.buffer);
        CHECK_GL_ERROR();
    }

    gld.scheduleDestroy(std::move(data));
}

} // namespace filament::backend
