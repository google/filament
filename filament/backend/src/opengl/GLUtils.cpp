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

#include "private/backend/Driver.h"

#include <utils/compiler.h>
#include <utils/ostream.h>
#include <utils/trap.h>

#include <absl/log/log.h>
#include <absl/strings/str_format.h>

#include <string_view>

#include <stddef.h>

namespace filament::backend {

using namespace backend;
using namespace utils;

namespace GLUtils {

UTILS_NOINLINE
std::string_view getGLErrorString(GLenum error) noexcept {
    switch (error) {
        case GL_NO_ERROR:
            return "GL_NO_ERROR";
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";
        default:
            break;
    }
    return "unknown";
}

UTILS_NOINLINE
GLenum checkGLError(const char* function, size_t line) noexcept {
    GLenum const error = glGetError();
    if (UTILS_VERY_UNLIKELY(error != GL_NO_ERROR)) {
        auto const string = getGLErrorString(error);
        LOG(ERROR) << "OpenGL error " << absl::StrFormat("%#x", error) << " (" << string
                   << ") in \"" << function << "\" at line " << line;
    }
    return error;
}

UTILS_NOINLINE
void assertGLError(const char* function, size_t line) noexcept {
    GLenum const err = checkGLError(function, line);
    if (UTILS_VERY_UNLIKELY(err != GL_NO_ERROR)) {
        debug_trap();
    }
}

UTILS_NOINLINE
std::string_view getFramebufferStatusString(GLenum status) noexcept {
    switch (status) {
        case GL_FRAMEBUFFER_COMPLETE:
            return "GL_FRAMEBUFFER_COMPLETE";
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        case GL_FRAMEBUFFER_UNSUPPORTED:
            return "GL_FRAMEBUFFER_UNSUPPORTED";
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        case GL_FRAMEBUFFER_UNDEFINED:
            return "GL_FRAMEBUFFER_UNDEFINED";
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
#endif
        default:
            break;
    }
    return "unknown";
}

UTILS_NOINLINE
GLenum checkFramebufferStatus(GLenum target, const char* function, size_t line) noexcept {
    GLenum const status = glCheckFramebufferStatus(target);
    if (UTILS_VERY_UNLIKELY(status != GL_FRAMEBUFFER_COMPLETE)) {
        auto const string = getFramebufferStatusString(status);
        LOG(ERROR) << "OpenGL framebuffer error " << absl::StrFormat("%#x", status) << " ("
                   << string << ") in \"" << function << "\" at line " << line;
    }
    return status;
}

UTILS_NOINLINE
void assertFramebufferStatus(GLenum target, const char* function, size_t line) noexcept {
    GLenum const status = checkFramebufferStatus(target, function, line);
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
