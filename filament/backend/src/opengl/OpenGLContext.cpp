/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "OpenGLContext.h"

// change to true to display all GL extensions in the console on start-up
#define DEBUG_PRINT_EXTENSIONS false


using namespace utils;

namespace filament {

using namespace backend;

OpenGLContext::OpenGLContext() noexcept {
    state.vao.p = &mDefaultVAO;
    state.enables.caps.set(getIndexForCap(GL_DITHER));

    UTILS_UNUSED char const* const vendor   = (char const*) glGetString(GL_VENDOR);
    UTILS_UNUSED char const* const renderer = (char const*) glGetString(GL_RENDERER);
    UTILS_UNUSED char const* const version  = (char const*) glGetString(GL_VERSION);
    UTILS_UNUSED char const* const shader   = (char const*) glGetString(GL_SHADING_LANGUAGE_VERSION);

#ifndef NDEBUG
    slog.i << vendor << ", " << renderer << ", " << version << ", " << shader << io::endl;
#endif

    // OpenGL (ES) version
    GLint major = 0, minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &gets.max_renderbuffer_size);
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &gets.max_uniform_block_size);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &gets.uniform_buffer_offset_alignment);

#if 0
    // this is useful for development, but too verbose even for debug builds
    slog.i
        << "GL_MAX_RENDERBUFFER_SIZE = " << gets.max_renderbuffer_size << io::endl
        << "GL_MAX_UNIFORM_BLOCK_SIZE = " << gets.max_uniform_block_size << io::endl
        << "GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT = " << gets.uniform_buffer_offset_alignment << io::endl;
#endif

    if (strstr(renderer, "Adreno")) {
        bugs.dont_use_timer_query = true;   // verified
    } else if (strstr(renderer, "Mali")) {
        bugs.dont_use_timer_query = true;   // not verified
        bugs.vao_doesnt_store_element_array_buffer_binding = true;
        if (strstr(renderer, "Mali-T")) {
            bugs.disable_glFlush = true;
            bugs.disable_shared_context_draws = true;
            bugs.texture_external_needs_rebind = true;
        }
        if (strstr(renderer, "Mali-G")) {
            bugs.disable_texture_filter_anisotropic = true;
        }
    } else if (strstr(renderer, "Intel")) {
        bugs.vao_doesnt_store_element_array_buffer_binding = true;
    } else if (strstr(renderer, "PowerVR") || strstr(renderer, "Apple")) {
    } else if (strstr(renderer, "Tegra") || strstr(renderer, "GeForce") || strstr(renderer, "NV")) {
    } else if (strstr(renderer, "Vivante")) {
    } else if (strstr(renderer, "AMD") || strstr(renderer, "ATI")) {
    } else if (strstr(renderer, "Mozilla")) {
        bugs.disable_invalidate_framebuffer = true;
    }

    // Figure out if we have the extension we need
    GLint n;
    glGetIntegerv(GL_NUM_EXTENSIONS, &n);
    ExtentionSet exts;
    for (GLint i = 0; i < n; i++) {
        const char * const extension = (const char*)glGetStringi(GL_EXTENSIONS, (GLuint)i);
        exts.insert(StaticString::make(extension, strlen(extension)));
        if (DEBUG_PRINT_EXTENSIONS) {
            slog.d << extension << io::endl;
        }
    }

    ShaderModel shaderModel = ShaderModel::UNKNOWN;
    if (GLES30_HEADERS) {
        if (major == 3 && minor >= 0) {
            shaderModel = ShaderModel::GL_ES_30;
        }
        if (major == 3 && minor >= 1) {
            features.multisample_texture = true;
        }
        initExtensionsGLES(major, minor, exts);
    } else if (GL41_HEADERS) {
        if (major == 4 && minor >= 1) {
            shaderModel = ShaderModel::GL_CORE_41;
        }
        initExtensionsGL(major, minor, exts);
        features.multisample_texture = true;
    };
    assert(shaderModel != ShaderModel::UNKNOWN);
    mShaderModel = shaderModel;

    /*
     * Set our default state
     */

    disable(GL_DITHER);
    enable(GL_DEPTH_TEST);

    // Point sprite size and seamless cubemap filtering are disabled by default in desktop GL.
    // In OpenGL ES, these flags do not exist because they are always on.
#if GL41_HEADERS
    enable(GL_PROGRAM_POINT_SIZE);
    enable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif

    // TODO: Don't enable scissor when it is not necessary. This optimization could be done here in
    //       the driver by simply deferring the enable until the scissor rect is smaller than the
    //       window.
    enable(GL_SCISSOR_TEST);

#ifdef GL_ARB_seamless_cube_map
    enable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif

#ifdef GL_EXT_texture_filter_anisotropic
    if (ext.texture_filter_anisotropic && !bugs.disable_texture_filter_anisotropic) {
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gets.maxAnisotropy);
    }
#endif

#ifdef GL_FRAGMENT_SHADER_DERIVATIVE_HINT
    glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);
#endif

#if !defined(NDEBUG) && defined(GL_KHR_debug)
    if (ext.KHR_debug) {
        auto cb = [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                const GLchar* message, const void *userParam) {
            io::LogStream* stream;
            switch (severity) {
                case GL_DEBUG_SEVERITY_HIGH:
                    stream = &slog.e;
                    break;
                case GL_DEBUG_SEVERITY_MEDIUM:
                    stream = &slog.w;
                    break;
                case GL_DEBUG_SEVERITY_LOW:
                    stream = &slog.d;
                    break;
                case GL_DEBUG_SEVERITY_NOTIFICATION:
                default:
                    stream = &slog.i;
                    break;
            }
            io::LogStream& out = *stream;
            out << "KHR_debug ";
            switch (type) {
                case GL_DEBUG_TYPE_ERROR:
                    out << "ERROR";
                    break;
                case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
                    out << "DEPRECATED_BEHAVIOR";
                    break;
                case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
                    out << "UNDEFINED_BEHAVIOR";
                    break;
                case GL_DEBUG_TYPE_PORTABILITY:
                    out << "PORTABILITY";
                    break;
                case GL_DEBUG_TYPE_PERFORMANCE:
                    out << "PERFORMANCE";
                    break;
                case GL_DEBUG_TYPE_OTHER:
                    out << "OTHER";
                    break;
                case GL_DEBUG_TYPE_MARKER:
                    out << "MARKER";
                    break;
                default:
                    break;
            }
            out << ": " << message << io::endl;
        };
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(cb, nullptr);
    }
#endif
}

UTILS_NOINLINE
bool OpenGLContext::hasExtension(ExtentionSet const& map, utils::StaticString ext) noexcept {
    return map.find(ext) != map.end();
}

void OpenGLContext::initExtensionsGLES(GLint major, GLint minor, ExtentionSet const& exts) {
    // figure out and initialize the extensions we need
    ext.texture_filter_anisotropic = hasExtension(exts, "GL_EXT_texture_filter_anisotropic");
    ext.texture_compression_etc2 = true;
    ext.QCOM_tiled_rendering = hasExtension(exts, "GL_QCOM_tiled_rendering");
    ext.OES_EGL_image_external_essl3 = hasExtension(exts, "GL_OES_EGL_image_external_essl3");
    ext.EXT_debug_marker = hasExtension(exts, "GL_EXT_debug_marker");
    ext.EXT_color_buffer_half_float = hasExtension(exts, "GL_EXT_color_buffer_half_float");
    ext.EXT_color_buffer_float = hasExtension(exts, "GL_EXT_color_buffer_float");
    ext.APPLE_color_buffer_packed_float = hasExtension(exts, "GL_APPLE_color_buffer_packed_float");
    ext.texture_compression_s3tc = hasExtension(exts, "WEBGL_compressed_texture_s3tc");
    ext.EXT_multisampled_render_to_texture = hasExtension(exts, "GL_EXT_multisampled_render_to_texture");
    ext.EXT_disjoint_timer_query = hasExtension(exts, "GL_EXT_disjoint_timer_query");
    ext.KHR_debug = hasExtension(exts, "GL_KHR_debug");
    ext.EXT_texture_compression_s3tc_srgb = hasExtension(exts, "GL_EXT_texture_compression_s3tc_srgb");
    // ES 3.2 implies EXT_color_buffer_float
    if (major >= 3 && minor >= 2) {
        ext.EXT_color_buffer_float = true;
    }
}

void OpenGLContext::initExtensionsGL(GLint major, GLint minor, ExtentionSet const& exts) {
    ext.texture_filter_anisotropic = hasExtension(exts, "GL_EXT_texture_filter_anisotropic");
    ext.texture_compression_etc2 = hasExtension(exts, "GL_ARB_ES3_compatibility");
    ext.texture_compression_s3tc = hasExtension(exts, "GL_EXT_texture_compression_s3tc");
    ext.OES_EGL_image_external_essl3 = hasExtension(exts, "GL_OES_EGL_image_external_essl3");
    ext.EXT_debug_marker = hasExtension(exts, "GL_EXT_debug_marker");
    ext.EXT_color_buffer_half_float = true;  // Assumes core profile.
    ext.EXT_color_buffer_float = true;  // Assumes core profile.
    ext.APPLE_color_buffer_packed_float = true;  // Assumes core profile.
    ext.KHR_debug = major >= 4 && minor >= 3;
    ext.EXT_texture_sRGB = hasExtension(exts, "GL_EXT_texture_sRGB");
}

void OpenGLContext::bindBuffer(GLenum target, GLuint buffer) noexcept {
    size_t targetIndex = getIndexForBufferTarget(target);
    if (target == GL_ELEMENT_ARRAY_BUFFER) {
        // GL_ELEMENT_ARRAY_BUFFER is a special case, where the currently bound VAO remembers
        // the index buffer, unless there are no VAO bound (see: bindVertexArray)
        assert(state.vao.p);
        if (state.buffers.genericBinding[targetIndex] != buffer
            || ((state.vao.p != &mDefaultVAO) && (state.vao.p->elementArray != buffer))) {
            state.buffers.genericBinding[targetIndex] = buffer;
            if (state.vao.p != &mDefaultVAO) {
                state.vao.p->elementArray = buffer;
            }
            glBindBuffer(target, buffer);
        }
    } else {
        update_state(state.buffers.genericBinding[targetIndex], buffer, [&]() {
            glBindBuffer(target, buffer);
        });
    }
}

void OpenGLContext::pixelStore(GLenum pname, GLint param) noexcept {
    GLint* pcur = nullptr;
    switch (pname) {
        case GL_PACK_ROW_LENGTH:
            pcur = &state.pack.row_length;
            break;
        case GL_PACK_SKIP_ROWS:
            pcur = &state.pack.skip_row;
            break;
        case GL_PACK_SKIP_PIXELS:
            pcur = &state.pack.skip_pixels;
            break;
        case GL_PACK_ALIGNMENT:
            pcur = &state.pack.alignment;
            break;

        case GL_UNPACK_ROW_LENGTH:
            pcur = &state.unpack.row_length;
            break;
        case GL_UNPACK_ALIGNMENT:
            pcur = &state.unpack.alignment;
            break;
        case GL_UNPACK_SKIP_PIXELS:
            pcur = &state.unpack.skip_pixels;
            break;
        case GL_UNPACK_SKIP_ROWS:
            pcur = &state.unpack.skip_row;
            break;
        default:
            goto default_case;
    }

    if (UTILS_UNLIKELY(*pcur != param)) {
        *pcur = param;
default_case:
        glPixelStorei(pname, param);
    }
}

void OpenGLContext::unbindTexture(GLenum target, GLuint texture_id) noexcept {
    // unbind this texture from all the units it might be bound to
    // no need unbind the texture from FBOs because we're not tracking that state (and there is
    // no need to).
    const size_t index = getIndexForTextureTarget(target);
    for (GLuint unit = 0; unit < MAX_TEXTURE_UNIT_COUNT; unit++) {
        if (state.textures.units[unit].targets[index].texture_id == texture_id) {
            bindTexture(unit, target, (GLuint)0, index);
        }
    }
}

void OpenGLContext::unbindSampler(GLuint sampler) noexcept {
    // unbind this sampler from all the units it might be bound to
#pragma nounroll    // clang generates >800B of code!!!
    for (GLuint unit = 0; unit < MAX_TEXTURE_UNIT_COUNT; unit++) {
        if (state.textures.units[unit].sampler == sampler) {
            bindSampler(unit, 0);
        }
    }
}

void OpenGLContext::deleteBuffers(GLsizei n, const GLuint* buffers, GLenum target) noexcept {
    glDeleteBuffers(n, buffers);
    // bindings of bound buffers are reset to 0
    const size_t targetIndex = getIndexForBufferTarget(target);
    auto& genericBuffer = state.buffers.genericBinding[targetIndex];
    #pragma nounroll
    for (GLsizei i = 0; i < n; ++i) {
        if (genericBuffer == buffers[i]) {
            genericBuffer = 0;
        }
    }
    if (target == GL_UNIFORM_BUFFER || target == GL_TRANSFORM_FEEDBACK_BUFFER) {
        auto& indexedBuffer = state.buffers.targets[targetIndex];
        #pragma nounroll // clang generates >1 KiB of code!!
        for (GLsizei i = 0; i < n; ++i) {
            #pragma nounroll
            for (auto& buffer : indexedBuffer.buffers) {
                if (buffer.name == buffers[i]) {
                    buffer.name = 0;
                    buffer.offset = 0;
                    buffer.size = 0;
                }
            }
        }
    }
}

void OpenGLContext::deleteVextexArrays(GLsizei n, const GLuint* arrays) noexcept {
    glDeleteVertexArrays(1, arrays);
    // binding of a bound VAO is reset to 0
    for (GLsizei i = 0; i < n; ++i) {
        if (state.vao.p->vao == arrays[i]) {
            bindVertexArray(nullptr);
        }
    }
}

} // namesapce filament
