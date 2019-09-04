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

namespace filament {

using namespace backend;
using namespace utils;

namespace GLUtils {

void checkGLError(io::ostream& out, const char* function, size_t line) noexcept {
    GLenum err = glGetError();
    const char* error = "unknown";
    switch (err) {
        case GL_NO_ERROR:
            return;
        case GL_INVALID_ENUM:
            error = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "GL_INVALID_OPERATION";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            error = "GL_OUT_OF_MEMORY";
            break;
        default:
            break;
    }
    out << "OpenGL error " << io::hex << err << " (" << error << ") in \""
        << function << "\" at line " << io::dec << line << io::endl;
    debug_trap();
}

void checkFramebufferStatus(io::ostream& out, const char* function, size_t line) noexcept {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    const char* error = "unknown";
    switch (status) {
        case GL_FRAMEBUFFER_COMPLETE:
            // success!
            return;

        case GL_FRAMEBUFFER_UNDEFINED:
            error = "GL_FRAMEBUFFER_UNDEFINED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            error = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            error = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            error = "GL_FRAMEBUFFER_UNSUPPORTED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            error = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
            break;
        default:
            break;
    }
    out << "OpenGL framebuffer error " << io::hex << status << " (" << error << ") in \""
        << function << "\" at line " << io::dec << line << io::endl;
    debug_trap();
}

} // namespace GLUtils
} // namespace filament
