/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "GLUtils.h"

#include <utils/trap.h>

#include "private/backend/Driver.h"

namespace filament::backend {

using namespace backend;
using namespace utils;

namespace GLUtils {

UTILS_NOINLINE
const char* getGLError(GLenum error) noexcept {
    const char* string = "unknown";
    switch (error) {
        case GL_NO_ERROR:
            string = "GL_NO_ERROR";
            break;
        case GL_INVALID_ENUM:
            string = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            string = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            string = "GL_INVALID_OPERATION";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            string = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            string = "GL_OUT_OF_MEMORY";
            break;
        default:
            break;
    }
    return string;
}

UTILS_NOINLINE
GLenum checkGLError(io::ostream& out, const char* function, size_t line) noexcept {
    GLenum const error = glGetError();
    if (UTILS_VERY_UNLIKELY(error != GL_NO_ERROR)) {
        const char* string = getGLError(error);
        out << "OpenGL error " << io::hex << error << " (" << string << ") in \""
            << function << "\" at line " << io::dec << line << io::endl;
    }
    return error;
}

UTILS_NOINLINE
void assertGLError(io::ostream& out, const char* function, size_t line) noexcept {
    GLenum const err = checkGLError(out, function, line);
    if (UTILS_VERY_UNLIKELY(err != GL_NO_ERROR)) {
        debug_trap();
    }
}

UTILS_NOINLINE
const char* getFramebufferStatus(GLenum status) noexcept {
    const char* string = "unknown";
    switch (status) {
        case GL_FRAMEBUFFER_COMPLETE:
            string = "GL_FRAMEBUFFER_COMPLETE";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            string = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            string = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            string = "GL_FRAMEBUFFER_UNSUPPORTED";
            break;
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        case GL_FRAMEBUFFER_UNDEFINED:
            string = "GL_FRAMEBUFFER_UNDEFINED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            string = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
            break;
#endif
        default:
            break;
    }
    return string;
}

UTILS_NOINLINE
GLenum checkFramebufferStatus(io::ostream& out, GLenum target, const char* function, size_t line) noexcept {
    GLenum const status = glCheckFramebufferStatus(target);
    if (UTILS_VERY_UNLIKELY(status != GL_FRAMEBUFFER_COMPLETE)) {
        const char* string = getFramebufferStatus(status);
        out << "OpenGL framebuffer error " << io::hex << status << " (" << string << ") in \""
            << function << "\" at line " << io::dec << line << io::endl;
    }
    return status;
}

UTILS_NOINLINE
void assertFramebufferStatus(io::ostream& out, GLenum target, const char* function, size_t line) noexcept {
    GLenum const status = checkFramebufferStatus(out, target, function, line);
    if (UTILS_VERY_UNLIKELY(status != GL_FRAMEBUFFER_COMPLETE)) {
        debug_trap();
    }
}

bool unordered_string_set::has(std::string_view str) const noexcept {
    return find(str) != end();
}

unordered_string_set split(const char* extensions) noexcept {
    unordered_string_set set;
    std::string_view string(extensions);
    do {
        auto p = string.find(' ');
        p = (p == std::string_view::npos ? string.length() : p);
        set.emplace(string.data(), p);
        string.remove_prefix(p == string.length() ? p : p + 1);
    } while(!string.empty());
    return set;
}

} // namespace GLUtils
} // namespace filament::backend
