/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "OpenGLBlobCache.h"

#include <backend/Platform.h>
#include <backend/Program.h>

#include <utils/Systrace.h>

namespace filament::backend {

struct OpenGLBlobCache::Blob {
    GLenum format;
    char data[];
};

GLuint OpenGLBlobCache::retrieve(BlobCacheKey* outKey, Platform& platform,
        Program const& program) noexcept {
    SYSTRACE_CALL();

    if (!platform.hasBlobFunc()) {
        // the key is never updated in that case
        return 0;
    }

    SYSTRACE_CONTEXT();

    GLuint programId = 0;

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    BlobCacheKey key{ program.getCacheId(), program.getSpecializationConstants() };

    // FIXME: use a static buffer to avoid systematic allocation
    // always attempt with 64 KiB
    constexpr size_t DEFAULT_BLOB_SIZE = 65536;
    std::unique_ptr<Blob, decltype(&::free)> blob{ (Blob*)malloc(DEFAULT_BLOB_SIZE), &::free };

    size_t const blobSize = platform.retrieveBlob(
            key.data(), key.size(), blob.get(), DEFAULT_BLOB_SIZE);

    if (blobSize > 0) {
        if (blobSize > DEFAULT_BLOB_SIZE) {
            // our buffer was too small, retry with the correct size
            blob.reset((Blob*)malloc(blobSize));
            platform.retrieveBlob(
                    key.data(), key.size(), blob.get(), blobSize);
        }

        GLsizei const programBinarySize = GLsizei(blobSize - sizeof(Blob));

        programId = glCreateProgram();

        SYSTRACE_NAME("glProgramBinary");
        glProgramBinary(programId, blob->format, blob->data, programBinarySize);

        if (UTILS_UNLIKELY(glGetError() != GL_NO_ERROR)) {
            // glProgramBinary can fail if for instance the driver has been updated
            glDeleteProgram(programId);
            programId = 0;
        }
    }

    if (UTILS_LIKELY(outKey)) {
        using std::swap;
        swap(*outKey, key);
    }
#endif

    return programId;
}

void OpenGLBlobCache::insert(Platform& platform,
        BlobCacheKey const& key, GLuint program) noexcept {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    SYSTRACE_CALL();
    if (platform.hasBlobFunc()) {
        SYSTRACE_CONTEXT();
        GLenum format;
        GLint programBinarySize;
        SYSTRACE_NAME("glGetProgramiv");
        glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &programBinarySize);
        if (programBinarySize) {
            size_t const size = sizeof(Blob) + programBinarySize;
            std::unique_ptr<Blob, decltype(&::free)> blob{ (Blob*)malloc(size), &::free };
            SYSTRACE_NAME("glGetProgramBinary");
            glGetProgramBinary(program, programBinarySize, &programBinarySize, &format, blob->data);
            GLenum const error = glGetError();
            if (error == GL_NO_ERROR) {
                blob->format = format;
                platform.insertBlob(key.data(), key.size(), blob.get(), size);
            }
        }
    }
#endif
}

void OpenGLBlobCache::insert(Platform& platform, BlobCacheKey const& key,
        GLenum format, void* data, GLsizei programBinarySize) noexcept {
    SYSTRACE_CALL();
    if (platform.hasBlobFunc()) {
        if (programBinarySize) {
            size_t const size = sizeof(Blob) + programBinarySize;
            std::unique_ptr<Blob, decltype(&::free)> blob{ (Blob*)malloc(size), &::free };
            blob->format = format;
            memcpy(blob->data, data, programBinarySize);
            platform.insertBlob(key.data(), key.size(), blob.get(), size);
        }
    }
}

} // namespace filament::backend
