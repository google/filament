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

namespace filament::backend {

OpenGLContext::OpenGLContext() noexcept {
    state.vao.p = &mDefaultVAO;

    // There query work with all GL/GLES versions!
    state.vendor   = (char const*)glGetString(GL_VENDOR);
    state.renderer = (char const*)glGetString(GL_RENDERER);
    state.version  = (char const*)glGetString(GL_VERSION);
    state.shader   = (char const*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    slog.v << "[" << state.vendor << "], [" << state.renderer << "], "
              "[" << state.version << "], [" << state.shader << "]" << io::endl;

    /*
     * Figure out GL / GLES version and available features
     */

    if constexpr (BACKEND_OPENGL_VERSION == BACKEND_OPENGL_VERSION_GLES) {
        // This works on all versions of GLES
        sscanf(state.version, "OpenGL ES %d.%d", &state.major, &state.minor);
        if (state.major == 3 && state.minor >= 0) {
            mShaderModel = ShaderModel::GL_ES_30;
        }
        if (state.major == 3 && state.minor >= 1) {
            features.multisample_texture = true;
        }
        initExtensionsGLES();
    } else if constexpr (BACKEND_OPENGL_VERSION == BACKEND_OPENGL_VERSION_GL) {
        // OpenGL version
        glGetIntegerv(GL_MAJOR_VERSION, &state.major);
        glGetIntegerv(GL_MINOR_VERSION, &state.minor);
        if (state.major == 4 && state.minor >= 1) {
            mShaderModel = ShaderModel::GL_CORE_41;
        }
        features.multisample_texture = true;
        // feedback loops are allowed on GL desktop as long as writes are disabled
        bugs.allow_read_only_ancillary_feedback_loop = true;
        initExtensionsGL();
    }

    assert_invariant(mShaderModel != ShaderModel::UNKNOWN);

    /*
     * Figure out which driver bugs we need to workaround
     */

    if (strstr(state.renderer, "Adreno")) {
        // Qualcomm GPU
        bugs.invalidate_end_only_if_invalidate_start = true;

        // On Adreno (As of 3/20) timer query seem to return the CPU time, not the GPU time.
        bugs.dont_use_timer_query = true;

        // Blits to texture arrays are failing
        //   This bug continues to reproduce, though at times we've seen it appear to "go away". The
        //   standalone sample app that was written to show this problem still reproduces.
        //   The working hypthesis is that some other state affects this behavior.
        bugs.disable_sidecar_blit_into_texture_array = true;

        // early exit condition is flattened in EASU code
        bugs.split_easu = true;

        int maj, min, driverMajor, driverMinor;
        int c = sscanf(state.version, "OpenGL ES %d.%d V@%d.%d", // NOLINT(cert-err34-c)
                &maj, &min, &driverMajor, &driverMinor);
        if (c == 4) {
            // workarounds based on version here.
            // notes:
            //  bugs.invalidate_end_only_if_invalidate_start
            //  - appeared at least in
            //      "OpenGL ES 3.2 V@0490.0 (GIT@85da404, I46ff5fc46f, 1606794520) (Date:11/30/20)"
            //  - wasn't present in
            //      "OpenGL ES 3.2 V@0490.0 (GIT@0905e9f, Ia11ce2d146, 1599072951) (Date:09/02/20)"
            //  - has been confirmed fixed in V@570.1 by Qualcomm
            if (driverMajor < 490 || driverMajor > 570 ||
                (driverMajor == 570 && driverMinor >= 1)) {
                bugs.invalidate_end_only_if_invalidate_start = false;
            }
        }

        // qualcomm seems to have no problem with this (which is good for us)
        bugs.allow_read_only_ancillary_feedback_loop = true;
    } else if (strstr(state.renderer, "Mali")) {
        // ARM GPU
        bugs.vao_doesnt_store_element_array_buffer_binding = true;
        if (strstr(state.renderer, "Mali-T")) {
            bugs.disable_glFlush = true;
            bugs.disable_shared_context_draws = true;
            bugs.texture_external_needs_rebind = true;
            // We have not verified that timer queries work on Mali-T, so we disable to be safe.
            bugs.dont_use_timer_query = true;
        }
        if (strstr(state.renderer, "Mali-G")) {
            // note: We have verified that timer queries work well at least on some Mali-G.
        }
        // Mali seems to have no problem with this (which is good for us)
        bugs.allow_read_only_ancillary_feedback_loop = true;
    } else if (strstr(state.renderer, "Intel")) {
        // Intel GPU
        bugs.vao_doesnt_store_element_array_buffer_binding = true;
    } else if (strstr(state.renderer, "PowerVR")) {
        // PowerVR GPU
    } else if (strstr(state.renderer, "Apple")) {
        // Apple GPU
    } else if (strstr(state.renderer, "Tegra") ||
               strstr(state.renderer, "GeForce") ||
               strstr(state.renderer, "NV")) {
        // NVIDIA GPU
    } else if (strstr(state.renderer, "Vivante")) {
        // Vivante GPU
    } else if (strstr(state.renderer, "AMD") ||
               strstr(state.renderer, "ATI")) {
        // AMD/ATI GPU
    } else if (strstr(state.renderer, "Mozilla")) {
        bugs.disable_invalidate_framebuffer = true;
    }

    slog.v << "Active workarounds: " << '\n';
    UTILS_NOUNROLL
    for (auto [enabled, name, _] : mBugDatabase) {
        if (enabled) {
            slog.v << name << '\n';
        }
    }
    flush(slog.v);

    // now we can query getter and features
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &gets.max_renderbuffer_size);
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &gets.max_uniform_block_size);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &gets.uniform_buffer_offset_alignment);
    glGetIntegerv(GL_MAX_SAMPLES, &gets.max_samples);
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &gets.max_draw_buffers);
#ifdef GL_EXT_texture_filter_anisotropic
    if (ext.EXT_texture_filter_anisotropic) {
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gets.max_anisotropy);
    }
#endif

    assert_invariant(gets.max_draw_buffers >= 4); // minspec

#ifndef NDEBUG
    // this is useful for development
    slog.v  << "GL_MAX_DRAW_BUFFERS = " << gets.max_draw_buffers << '\n'
            << "GL_MAX_RENDERBUFFER_SIZE = " << gets.max_renderbuffer_size << '\n'
            << "GL_MAX_SAMPLES = " << gets.max_samples << '\n'
            << "GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT = " << gets.max_anisotropy << '\n'
            << "GL_MAX_UNIFORM_BLOCK_SIZE = " << gets.max_uniform_block_size << '\n'
            << "GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT = " << gets.uniform_buffer_offset_alignment << '\n'
            ;
    flush(slog.v);
#endif

    /*
     * Set our default state
     */

    // We need to make sure our internal state matches the GL state when we start.
    // (some of these calls may be unneeded as they might be the gl defaults)
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DITHER);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glDisable(GL_SAMPLE_COVERAGE);
    glDisable(GL_POLYGON_OFFSET_FILL);

    // Point sprite size and seamless cubemap filtering are disabled by default in desktop GL.
    // In OpenGL ES, these flags do not exist because they are always on.
#if BACKEND_OPENGL_VERSION == BACKEND_OPENGL_VERSION_GL
    enable(GL_PROGRAM_POINT_SIZE);
#endif

#ifdef GL_ARB_seamless_cube_map
    enable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif

#ifdef GL_EXT_texture_filter_anisotropic
    if (ext.EXT_texture_filter_anisotropic) {
        // make sure we don't have any error flag
        while (glGetError() != GL_NO_ERROR) { }

        // check that we can actually set the anisotropy on the sampler
        GLuint s;
        glGenSamplers(1, &s);
        glSamplerParameterf(s, GL_TEXTURE_MAX_ANISOTROPY_EXT, gets.max_anisotropy);
        if (glGetError() != GL_NO_ERROR) {
            // some drivers only allow to set the anisotropy on the texture itself
            bugs.texture_filter_anisotropic_broken_on_sampler = true;
        }
        glDeleteSamplers(1, &s);
    }
#endif

#ifdef GL_FRAGMENT_SHADER_DERIVATIVE_HINT
    glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);
#endif

#if !defined(NDEBUG) && defined(GL_KHR_debug)
    if (ext.KHR_debug) {
        auto cb = [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                const GLchar* message, const void *userParam) {
            io::ostream* stream = &slog.i;
            switch (severity) {
                case GL_DEBUG_SEVERITY_HIGH:    stream = &slog.e;   break;
                case GL_DEBUG_SEVERITY_MEDIUM:  stream = &slog.w;   break;
                case GL_DEBUG_SEVERITY_LOW:     stream = &slog.d;   break;
                case GL_DEBUG_SEVERITY_NOTIFICATION:
                default: break;
            }
            io::ostream& out = *stream;
            const char* level = ": ";
            switch (type) {
                case GL_DEBUG_TYPE_ERROR:               level = "ERROR: ";               break;
                case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: level = "DEPRECATED_BEHAVIOR: "; break;
                case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  level = "UNDEFINED_BEHAVIOR: ";  break;
                case GL_DEBUG_TYPE_PORTABILITY:         level = "PORTABILITY: ";         break;
                case GL_DEBUG_TYPE_PERFORMANCE:         level = "PERFORMANCE: ";         break;
                case GL_DEBUG_TYPE_OTHER:               level = "OTHER: ";               break;
                case GL_DEBUG_TYPE_MARKER:              level = "MARKER: ";              break;
                default: break;
            }
            out << "KHR_debug " << level << message << io::endl;
        };
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(cb, nullptr);
    }
#endif

#if defined(GL_EXT_clip_control) || defined(GL_ARB_clip_control) || defined(GL_VERSION_4_5)
    if (ext.EXT_clip_control) {
        glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    }
#endif
}

void OpenGLContext::initExtensionsGLES() noexcept {
    const char * const extensions = (const char*)glGetString(GL_EXTENSIONS);
    GLUtils::unordered_string_set exts = GLUtils::split(extensions);
    if constexpr (DEBUG_PRINT_EXTENSIONS) {
        for (auto extension: exts) {
            slog.d << "\"" << std::string(extension) << "\"\n";
        }
        flush(slog.d);
    }

    // figure out and initialize the extensions we need
    using namespace std::literals;
    ext.APPLE_color_buffer_packed_float = exts.has("GL_APPLE_color_buffer_packed_float"sv);
    ext.EXT_clip_control = exts.has("GL_EXT_clip_control"sv);
    ext.EXT_color_buffer_float = exts.has("GL_EXT_color_buffer_float"sv);
    ext.EXT_color_buffer_half_float = exts.has("GL_EXT_color_buffer_half_float"sv);
    ext.EXT_debug_marker = exts.has("GL_EXT_debug_marker"sv);
    ext.EXT_disjoint_timer_query = exts.has("GL_EXT_disjoint_timer_query"sv);
    ext.EXT_multisampled_render_to_texture = exts.has("GL_EXT_multisampled_render_to_texture"sv);
    ext.EXT_multisampled_render_to_texture2 = exts.has("GL_EXT_multisampled_render_to_texture2"sv);
    ext.EXT_shader_framebuffer_fetch = exts.has("GL_EXT_shader_framebuffer_fetch"sv);
    ext.EXT_texture_compression_etc2 = true;
    ext.EXT_texture_filter_anisotropic = exts.has("GL_EXT_texture_filter_anisotropic"sv);
    ext.GOOGLE_cpp_style_line_directive = exts.has("GL_GOOGLE_cpp_style_line_directive"sv);
    ext.KHR_debug = exts.has("GL_KHR_debug"sv);
    ext.OES_EGL_image_external_essl3 = exts.has("GL_OES_EGL_image_external_essl3"sv);
    ext.QCOM_tiled_rendering = exts.has("GL_QCOM_tiled_rendering"sv);
    ext.EXT_texture_compression_s3tc = exts.has("GL_EXT_texture_compression_s3tc"sv);
    ext.EXT_texture_compression_s3tc_srgb = exts.has("GL_EXT_texture_compression_s3tc_srgb"sv);
    ext.WEBGL_compressed_texture_s3tc = exts.has("WEBGL_compressed_texture_s3tc"sv);
    ext.WEBGL_compressed_texture_s3tc_srgb = exts.has("WEBGL_compressed_texture_s3tc_srgb"sv);
    ext.WEBGL_compressed_texture_etc = exts.has("WEBGL_compressed_texture_etc"sv);
    // ES 3.2 implies EXT_color_buffer_float
    if (state.major >= 3 && state.minor >= 2) {
        ext.EXT_color_buffer_float = true;
    }
}

void OpenGLContext::initExtensionsGL() noexcept {
    GLUtils::unordered_string_set exts;
    GLint n = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &n);
    for (GLint i = 0; i < n; i++) {
        exts.emplace((const char*)glGetStringi(GL_EXTENSIONS, (GLuint)i));
    }
    if constexpr (DEBUG_PRINT_EXTENSIONS) {
        for (auto extension: exts) {
            slog.d << "\"" << std::string(extension) << "\"\n";
        }
        flush(slog.d);
    }

    using namespace std::literals;
    auto major = state.major;
    auto minor = state.minor;
    ext.APPLE_color_buffer_packed_float = true;  // Assumes core profile.
    ext.ARB_shading_language_packing = exts.has("GL_ARB_shading_language_packing"sv) || (major == 4 && minor >= 2);
    ext.EXT_clip_control = exts.has("GL_ARB_clip_control"sv) || (major == 4 && minor >= 5);
    ext.EXT_color_buffer_float = true;  // Assumes core profile.
    ext.EXT_color_buffer_half_float = true;  // Assumes core profile.
    ext.EXT_debug_marker = exts.has("GL_EXT_debug_marker"sv);
    ext.EXT_shader_framebuffer_fetch = exts.has("GL_EXT_shader_framebuffer_fetch"sv);
    ext.EXT_texture_compression_etc2 = exts.has("GL_ARB_ES3_compatibility"sv);
    ext.EXT_texture_filter_anisotropic = exts.has("GL_EXT_texture_filter_anisotropic"sv);
    ext.EXT_texture_sRGB = exts.has("GL_EXT_texture_sRGB"sv);
    ext.GOOGLE_cpp_style_line_directive = exts.has("GL_GOOGLE_cpp_style_line_directive"sv);
    ext.KHR_debug = major >= 4 && minor >= 3;
    ext.OES_EGL_image_external_essl3 = exts.has("GL_OES_EGL_image_external_essl3"sv);
    ext.EXT_texture_compression_s3tc = exts.has("GL_EXT_texture_compression_s3tc"sv);
    ext.EXT_texture_compression_s3tc_srgb = exts.has("GL_EXT_texture_compression_s3tc_srgb"sv);
}

void OpenGLContext::bindBuffer(GLenum target, GLuint buffer) noexcept {
    if (target == GL_ELEMENT_ARRAY_BUFFER) {
        constexpr size_t targetIndex = getIndexForBufferTarget(GL_ELEMENT_ARRAY_BUFFER);
        // GL_ELEMENT_ARRAY_BUFFER is a special case, where the currently bound VAO remembers
        // the index buffer, unless there are no VAO bound (see: bindVertexArray)
        assert_invariant(state.vao.p);
        if (state.buffers.genericBinding[targetIndex] != buffer
            || ((state.vao.p != &mDefaultVAO) && (state.vao.p->elementArray != buffer))) {
            state.buffers.genericBinding[targetIndex] = buffer;
            if (state.vao.p != &mDefaultVAO) {
                state.vao.p->elementArray = buffer;
            }
            glBindBuffer(target, buffer);
        }
    } else {
        size_t targetIndex = getIndexForBufferTarget(target);
        update_state(state.buffers.genericBinding[targetIndex], buffer, [&]() {
            glBindBuffer(target, buffer);
        });
    }
}

void OpenGLContext::pixelStore(GLenum pname, GLint param) noexcept {
    GLint* pcur;

    // Note: GL_UNPACK_SKIP_PIXELS, GL_UNPACK_SKIP_ROWS and
    //       GL_PACK_SKIP_PIXELS, GL_PACK_SKIP_ROWS
    // are actually provided as conveniences to the programmer; they provide no functionality
    // that cannot be duplicated at the call site (e.g. glTexImage2D or glReadPixels)

    switch (pname) {
        case GL_PACK_ALIGNMENT:     pcur = &state.pack.alignment;       break;
        case GL_PACK_ROW_LENGTH:    pcur = &state.pack.row_length;      break;
        case GL_PACK_SKIP_PIXELS:   pcur = &state.pack.skip_pixels;     break;  // convenience
        case GL_PACK_SKIP_ROWS:     pcur = &state.pack.skip_row;        break;  // convenience
        case GL_UNPACK_ALIGNMENT:   pcur = &state.unpack.alignment;     break;
        case GL_UNPACK_ROW_LENGTH:  pcur = &state.unpack.row_length;    break;
        case GL_UNPACK_SKIP_PIXELS: pcur = &state.unpack.skip_pixels;   break;  // convenience
        case GL_UNPACK_SKIP_ROWS:   pcur = &state.unpack.skip_row;      break;  // convenience
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
    UTILS_NOUNROLL
    for (GLuint unit = 0; unit < MAX_TEXTURE_UNIT_COUNT; unit++) {
        if (state.textures.units[unit].targets[index].texture_id == texture_id) {
            bindTexture(unit, target, (GLuint)0, index);
        }
    }
}

void OpenGLContext::unbindSampler(GLuint sampler) noexcept {
    // unbind this sampler from all the units it might be bound to
    UTILS_NOUNROLL  // clang generates >800B of code!!!
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
    UTILS_NOUNROLL
    for (GLsizei i = 0; i < n; ++i) {
        if (genericBuffer == buffers[i]) {
            genericBuffer = 0;
        }
    }
    if (target == GL_UNIFORM_BUFFER || target == GL_TRANSFORM_FEEDBACK_BUFFER) {
        auto& indexedBuffer = state.buffers.targets[targetIndex];
        UTILS_NOUNROLL // clang generates >1 KiB of code!!
        for (GLsizei i = 0; i < n; ++i) {
            UTILS_NOUNROLL
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
