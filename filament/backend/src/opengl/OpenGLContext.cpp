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

#include <backend/platforms/OpenGLPlatform.h>

#include <utility>

// change to true to display all GL extensions in the console on start-up
#define DEBUG_PRINT_EXTENSIONS false

using namespace utils;

namespace filament::backend {

bool OpenGLContext::queryOpenGLVersion(GLint* major, GLint* minor) noexcept {
#ifdef BACKEND_OPENGL_VERSION_GLES
#   ifdef BACKEND_OPENGL_LEVEL_GLES30
    char const* version = (char const*)glGetString(GL_VERSION);
    // This works on all versions of GLES
    int const n = version ? sscanf(version, "OpenGL ES %d.%d", major, minor) : 0;
    return n == 2;
#   else
    // when we compile with GLES2.0 only, we force the context version to 2.0
    *major = 2;
    *minor = 0;
    return true;
#   endif
#else
    // OpenGL version
    glGetIntegerv(GL_MAJOR_VERSION, major);
    glGetIntegerv(GL_MINOR_VERSION, minor);
    return (glGetError() == GL_NO_ERROR);
#endif
}

OpenGLContext::OpenGLContext() noexcept {
    state.vao.p = &mDefaultVAO;

    // These queries work with all GL/GLES versions!
    state.vendor   = (char const*)glGetString(GL_VENDOR);
    state.renderer = (char const*)glGetString(GL_RENDERER);
    state.version  = (char const*)glGetString(GL_VERSION);
    state.shader   = (char const*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    slog.v << "[" << state.vendor << "], [" << state.renderer << "], "
              "[" << state.version << "], [" << state.shader << "]" << io::endl;

    /*
     * Figure out GL / GLES version and available features
     */

    queryOpenGLVersion(&state.major, &state.minor);

    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &gets.max_renderbuffer_size);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &gets.max_texture_image_units);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &gets.max_combined_texture_image_units);

    if (state.major > 2) { // this check works for both GL and GLES, but is intended for GLES
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &gets.max_uniform_block_size);
        glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &gets.max_uniform_buffer_bindings);
        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &gets.uniform_buffer_offset_alignment);
        glGetIntegerv(GL_MAX_SAMPLES, &gets.max_samples);
        glGetIntegerv(GL_MAX_DRAW_BUFFERS, &gets.max_draw_buffers);
        glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS,
                &gets.max_transform_feedback_separate_attribs);
#endif
    } else {
        gets.max_uniform_block_size = 0;
        gets.max_uniform_buffer_bindings = 0;
        gets.uniform_buffer_offset_alignment = 0;
        gets.max_samples = 1;
        gets.max_draw_buffers = 1;
        gets.max_transform_feedback_separate_attribs = 0;
    }

    constexpr auto const caps3 = FEATURE_LEVEL_CAPS[+FeatureLevel::FEATURE_LEVEL_3];
    constexpr GLint MAX_VERTEX_SAMPLER_COUNT = caps3.MAX_VERTEX_SAMPLER_COUNT;
    constexpr GLint MAX_FRAGMENT_SAMPLER_COUNT = caps3.MAX_FRAGMENT_SAMPLER_COUNT;

    // default procs that can be overridden based on runtime version
#ifdef BACKEND_OPENGL_LEVEL_GLES30
    procs.genVertexArrays = glGenVertexArrays;
    procs.bindVertexArray = glBindVertexArray;
    procs.deleteVertexArrays = glDeleteVertexArrays;

    // these are core in GL and GLES 3.x
    procs.genQueries = glGenQueries;
    procs.deleteQueries = glDeleteQueries;
    procs.beginQuery = glBeginQuery;
    procs.endQuery = glEndQuery;
    procs.getQueryObjectuiv = glGetQueryObjectuiv;
#   ifdef BACKEND_OPENGL_VERSION_GL
        procs.getQueryObjectui64v = glGetQueryObjectui64v; // only core in GL
#   elif defined(GL_EXT_disjoint_timer_query)
        procs.getQueryObjectui64v = glGetQueryObjectui64vEXT;
#   endif // BACKEND_OPENGL_VERSION_GL

     // core in ES 3.0 and GL 4.3
    procs.invalidateFramebuffer = glInvalidateFramebuffer;
#endif // BACKEND_OPENGL_LEVEL_GLES30

    // no-op if not supported
    procs.maxShaderCompilerThreadsKHR = +[](GLuint) {};

#ifdef BACKEND_OPENGL_VERSION_GLES
    initExtensionsGLES();
    if (state.major == 3) {
        // Runtime OpenGL version is ES 3.x
        assert_invariant(gets.max_texture_image_units >= 16);
        assert_invariant(gets.max_combined_texture_image_units >= 32);
        if (state.minor >= 1) {
            features.multisample_texture = true;
            // figure out our feature level
            if (ext.EXT_texture_cube_map_array) {
                mFeatureLevel = FeatureLevel::FEATURE_LEVEL_2;
                if (gets.max_texture_image_units >= MAX_FRAGMENT_SAMPLER_COUNT &&
                    gets.max_combined_texture_image_units >=
                            (MAX_FRAGMENT_SAMPLER_COUNT + MAX_VERTEX_SAMPLER_COUNT)) {
                    mFeatureLevel = FeatureLevel::FEATURE_LEVEL_3;
                }
            }
        }
    }
#ifndef IOS // IOS is guaranteed to have ES3.x
    else if (UTILS_UNLIKELY(state.major == 2)) {
        // Runtime OpenGL version is ES 2.x

#if defined(BACKEND_OPENGL_LEVEL_GLES30)
        // mandatory extensions (all supported by Mali-400 and Adreno 304)
        assert_invariant(ext.OES_depth_texture);
        assert_invariant(ext.OES_depth24);
        assert_invariant(ext.OES_packed_depth_stencil);
        assert_invariant(ext.OES_rgb8_rgba8);
        assert_invariant(ext.OES_standard_derivatives);
        assert_invariant(ext.OES_texture_npot);
#endif

        if (UTILS_LIKELY(ext.OES_vertex_array_object)) {
            procs.genVertexArrays = glGenVertexArraysOES;
            procs.bindVertexArray = glBindVertexArrayOES;
            procs.deleteVertexArrays = glDeleteVertexArraysOES;
        } else {
            // if we don't have OES_vertex_array_object, just don't do anything with real VAOs,
            // we'll just rebind everything each time. Most Mali-400 support this extension, but
            // a few don't.
            procs.genVertexArrays = +[](GLsizei, GLuint*) {};
            procs.bindVertexArray = +[](GLuint) {};
            procs.deleteVertexArrays = +[](GLsizei, GLuint const*) {};
            // we activate this workaround path, which does the reset of array buffer
            bugs.vao_doesnt_store_element_array_buffer_binding = true;
        }

        // EXT_disjoint_timer_query is optional -- pointers will be null if not available
        procs.genQueries = glGenQueriesEXT;
        procs.deleteQueries = glDeleteQueriesEXT;
        procs.beginQuery = glBeginQueryEXT;
        procs.endQuery = glEndQueryEXT;
        procs.getQueryObjectuiv = glGetQueryObjectuivEXT;
        procs.getQueryObjectui64v = glGetQueryObjectui64vEXT;

        procs.invalidateFramebuffer = glDiscardFramebufferEXT;

        procs.maxShaderCompilerThreadsKHR = glMaxShaderCompilerThreadsKHR;

        mFeatureLevel = FeatureLevel::FEATURE_LEVEL_0;
    }
#endif // IOS
#else
    initExtensionsGL();
    if (state.major == 4) {
        assert_invariant(state.minor >= 1);
        mShaderModel = ShaderModel::DESKTOP;
        if (state.minor >= 3) {
            // cubemap arrays are available as of OpenGL 4.0
            mFeatureLevel = FeatureLevel::FEATURE_LEVEL_2;
            // figure out our feature level
            if (gets.max_texture_image_units >= MAX_FRAGMENT_SAMPLER_COUNT &&
                gets.max_combined_texture_image_units >=
                        (MAX_FRAGMENT_SAMPLER_COUNT + MAX_VERTEX_SAMPLER_COUNT)) {
                mFeatureLevel = FeatureLevel::FEATURE_LEVEL_3;
            }
        }
        features.multisample_texture = true;
    }
    // feedback loops are allowed on GL desktop as long as writes are disabled
    bugs.allow_read_only_ancillary_feedback_loop = true;
    assert_invariant(gets.max_texture_image_units >= 16);
    assert_invariant(gets.max_combined_texture_image_units >= 32);

    procs.maxShaderCompilerThreadsKHR = glMaxShaderCompilerThreadsARB;
#endif

#ifdef GL_EXT_texture_filter_anisotropic
    if (ext.EXT_texture_filter_anisotropic) {
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gets.max_anisotropy);
    }
#endif

    /*
     * Figure out which driver bugs we need to workaround
     */

    const bool isAngle = strstr(state.renderer, "ANGLE");
    if (!isAngle) {
        if (strstr(state.renderer, "Adreno")) {
            // Qualcomm GPU
            bugs.invalidate_end_only_if_invalidate_start = true;

            // On Adreno (As of 3/20) timer query seem to return the CPU time, not the GPU time.
            bugs.dont_use_timer_query = true;

            // Blits to texture arrays are failing
            //   This bug continues to reproduce, though at times we've seen it appear to "go away".
            //   The standalone sample app that was written to show this problem still reproduces.
            //   The working hypothesis is that some other state affects this behavior.
            bugs.disable_blit_into_texture_array = true;

            // early exit condition is flattened in EASU code
            bugs.split_easu = true;

            // initialize the non-used uniform array for Adreno drivers.
            bugs.enable_initialize_non_used_uniform_array = true;

            int maj, min, driverMajor, driverMinor;
            int const c = sscanf(state.version, "OpenGL ES %d.%d V@%d.%d", // NOLINT(cert-err34-c)
                    &maj, &min, &driverMajor, &driverMinor);
            if (c == 4) {
                // Workarounds based on version here.
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
                // We have run into several problems with timer queries on Mali-Gxx:
                // - timer queries seem to cause memory corruptions in some cases on some devices
                //   (see b/233754398)
                //          - appeared at least in: "OpenGL ES 3.2 v1.r26p0-01eac0"
                //          - wasn't present in: "OpenGL ES 3.2 v1.r32p1-00pxl1"
                // - timer queries sometime crash with an NPE (see b/273759031)
                bugs.dont_use_timer_query = true;
            }
            // Mali seems to have no problem with this (which is good for us)
            bugs.allow_read_only_ancillary_feedback_loop = true;
        } else if (strstr(state.renderer, "Intel")) {
            // Intel GPU
            bugs.vao_doesnt_store_element_array_buffer_binding = true;
        } else if (strstr(state.renderer, "PowerVR")) {
            // PowerVR GPU
            // On PowerVR (Rogue GE8320) glFlush doesn't seem to do anything, in particular,
            // it doesn't kick the GPU earlier, so don't issue these calls as they seem to slow
            // things down.
            bugs.disable_glFlush = true;
            // On PowerVR (Rogue GE8320) using gl_InstanceID too early in the shader doesn't work.
            bugs.powervr_shader_workarounds = true;
            // On PowerVR (Rogue GE8320) destroying a fbo after glBlitFramebuffer is effectively
            // equivalent to glFinish.
            bugs.delay_fbo_destruction = true;
            // PowerVR seems to have no problem with this (which is good for us)
            bugs.allow_read_only_ancillary_feedback_loop = true;
            // PowerVR has a shader compiler thread pinned on the last core
            bugs.disable_thread_affinity = true;
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
    } else {
        // When running under ANGLE, it's a different set of workaround that we need.
        if (strstr(state.renderer, "Adreno")) {
            // Qualcomm GPU
            // early exit condition is flattened in EASU code
            // (that should be regardless of ANGLE, but we should double-check)
            bugs.split_easu = true;
        }
        // TODO: see if we could use `bugs.allow_read_only_ancillary_feedback_loop = true`
    }

    slog.v << "Feature level: " << +mFeatureLevel << '\n';
    slog.v << "Active workarounds: " << '\n';
    UTILS_NOUNROLL
    for (auto [enabled, name, _] : mBugDatabase) {
        if (enabled) {
            slog.v << name << '\n';
        }
    }
    flush(slog.v);

#ifndef NDEBUG
    // this is useful for development
    slog.v  << "GL_MAX_DRAW_BUFFERS = " << gets.max_draw_buffers << '\n'
            << "GL_MAX_RENDERBUFFER_SIZE = " << gets.max_renderbuffer_size << '\n'
            << "GL_MAX_SAMPLES = " << gets.max_samples << '\n'
            << "GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT = " << gets.max_anisotropy << '\n'
            << "GL_MAX_UNIFORM_BLOCK_SIZE = " << gets.max_uniform_block_size << '\n'
            << "GL_MAX_TEXTURE_IMAGE_UNITS = " << gets.max_texture_image_units << '\n'
            << "GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT = " << gets.uniform_buffer_offset_alignment << '\n'
            ;
    flush(slog.v);
#endif

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    assert_invariant(state.major <= 2 || gets.max_draw_buffers >= 4); // minspec
#endif

    setDefaultState();

#ifdef GL_EXT_texture_filter_anisotropic
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    if (state.major > 2 && ext.EXT_texture_filter_anisotropic) {
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
#endif

    // in practice KHR_Debug has never been useful, and actually is confusing. We keep this
    // only for our own debugging, in case we need it some day.
#if false && !defined(NDEBUG) && defined(GL_KHR_debug)
    if (ext.KHR_debug) {
        auto cb = +[](GLenum, GLenum type, GLuint, GLenum severity, GLsizei length,
                const GLchar* message, const void *) {
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
            out << "KHR_debug " << level << std::string_view{ message, size_t(length) } << io::endl;
        };
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(cb, nullptr);
    }
#endif
}

void OpenGLContext::setDefaultState() noexcept {
    // We need to make sure our internal state matches the GL state when we start.
    // (some of these calls may be unneeded as they might be the gl defaults)
    GLenum const caps[] = {
        GL_BLEND,
        GL_CULL_FACE,
        GL_SCISSOR_TEST,
        GL_DEPTH_TEST,
        GL_STENCIL_TEST,
        GL_DITHER,
        GL_SAMPLE_ALPHA_TO_COVERAGE,
        GL_SAMPLE_COVERAGE,
        GL_POLYGON_OFFSET_FILL,  
    };

    UTILS_NOUNROLL
    for (auto const capi : caps) {
        size_t const capIndex = getIndexForCap(capi);
        if (state.enables.caps[capIndex]) {
            glEnable(capi);
        } else {
            glDisable(capi);
        }
    }

    // Point sprite size and seamless cubemap filtering are disabled by default in desktop GL.
    // In OpenGL ES, these flags do not exist because they are always on.
#ifdef BACKEND_OPENGL_VERSION_GL
    glEnable(GL_PROGRAM_POINT_SIZE);
    enable(GL_PROGRAM_POINT_SIZE);
#endif

#ifdef GL_ARB_seamless_cube_map
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    enable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif

#ifdef GL_FRAGMENT_SHADER_DERIVATIVE_HINT
    glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);
#endif

    if (ext.EXT_clip_control) {
#if defined(BACKEND_OPENGL_VERSION_GL)
        glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
#elif defined(GL_EXT_clip_control)
        glClipControlEXT(GL_LOWER_LEFT_EXT, GL_ZERO_TO_ONE_EXT);
#endif
    }

    if (ext.EXT_clip_cull_distance) {
        glEnable(GL_CLIP_DISTANCE0);
    }
}

#ifdef BACKEND_OPENGL_VERSION_GLES

void OpenGLContext::initExtensionsGLES() noexcept {
    const char * const extensions = (const char*)glGetString(GL_EXTENSIONS);
    GLUtils::unordered_string_set const exts = GLUtils::split(extensions);
    if constexpr (DEBUG_PRINT_EXTENSIONS) {
        for (auto extension: exts) {
            slog.d << "\"" << std::string_view(extension) << "\"\n";
        }
        flush(slog.d);
    }

    // figure out and initialize the extensions we need
    using namespace std::literals;
    ext.APPLE_color_buffer_packed_float = exts.has("GL_APPLE_color_buffer_packed_float"sv);
    ext.EXT_clip_control = exts.has("GL_EXT_clip_control"sv);
    ext.EXT_clip_cull_distance = exts.has("GL_EXT_clip_cull_distance"sv);
    ext.EXT_color_buffer_float = exts.has("GL_EXT_color_buffer_float"sv);
    ext.EXT_color_buffer_half_float = exts.has("GL_EXT_color_buffer_half_float"sv);
    ext.EXT_debug_marker = exts.has("GL_EXT_debug_marker"sv);
    ext.EXT_discard_framebuffer = exts.has("GL_EXT_discard_framebuffer"sv);
    ext.EXT_disjoint_timer_query = exts.has("GL_EXT_disjoint_timer_query"sv);
    ext.EXT_multisampled_render_to_texture = exts.has("GL_EXT_multisampled_render_to_texture"sv);
    ext.EXT_multisampled_render_to_texture2 = exts.has("GL_EXT_multisampled_render_to_texture2"sv);
    ext.EXT_shader_framebuffer_fetch = exts.has("GL_EXT_shader_framebuffer_fetch"sv);
#if !defined(__EMSCRIPTEN__)
    ext.EXT_texture_compression_etc2 = true;
#endif
    ext.EXT_texture_compression_s3tc = exts.has("GL_EXT_texture_compression_s3tc"sv);
    ext.EXT_texture_compression_s3tc_srgb = exts.has("GL_EXT_texture_compression_s3tc_srgb"sv);
    ext.EXT_texture_compression_rgtc = exts.has("GL_EXT_texture_compression_rgtc"sv);
    ext.EXT_texture_compression_bptc = exts.has("GL_EXT_texture_compression_bptc"sv);
    ext.EXT_texture_cube_map_array = exts.has("GL_EXT_texture_cube_map_array"sv) || exts.has("GL_OES_texture_cube_map_array"sv);
    ext.GOOGLE_cpp_style_line_directive = exts.has("GL_GOOGLE_cpp_style_line_directive"sv);
    ext.KHR_debug = exts.has("GL_KHR_debug"sv);
    ext.KHR_parallel_shader_compile = exts.has("GL_KHR_parallel_shader_compile"sv);
    ext.KHR_texture_compression_astc_hdr = exts.has("GL_KHR_texture_compression_astc_hdr"sv);
    ext.KHR_texture_compression_astc_ldr = exts.has("GL_KHR_texture_compression_astc_ldr"sv);
    ext.OES_depth_texture = exts.has("GL_OES_depth_texture"sv);
    ext.OES_depth24 = exts.has("GL_OES_depth24"sv);
    ext.OES_packed_depth_stencil = exts.has("GL_OES_packed_depth_stencil"sv);
    ext.OES_EGL_image_external_essl3 = exts.has("GL_OES_EGL_image_external_essl3"sv);
    ext.OES_rgb8_rgba8 = exts.has("GL_OES_rgb8_rgba8"sv);
    ext.OES_standard_derivatives = exts.has("GL_OES_standard_derivatives"sv);
    ext.OES_texture_npot = exts.has("GL_OES_texture_npot"sv);
    ext.OES_vertex_array_object = exts.has("GL_OES_vertex_array_object"sv);
    ext.WEBGL_compressed_texture_etc = exts.has("WEBGL_compressed_texture_etc"sv);
    ext.WEBGL_compressed_texture_s3tc = exts.has("WEBGL_compressed_texture_s3tc"sv);
    ext.WEBGL_compressed_texture_s3tc_srgb = exts.has("WEBGL_compressed_texture_s3tc_srgb"sv);

    // ES 3.2 implies EXT_color_buffer_float
    if (state.major > 3 || (state.major == 3 && state.minor >= 2)) {
        ext.EXT_color_buffer_float = true;
    }

    // ES 3.x implies EXT_discard_framebuffer and OES_vertex_array_object
    if (state.major >= 3) {
        ext.EXT_discard_framebuffer = true;
        ext.OES_vertex_array_object = true;
    }
}

#endif // BACKEND_OPENGL_VERSION_GLES

#ifdef BACKEND_OPENGL_VERSION_GL

void OpenGLContext::initExtensionsGL() noexcept {
    GLUtils::unordered_string_set exts;
    GLint n = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &n);
    for (GLint i = 0; i < n; i++) {
        exts.emplace((const char*)glGetStringi(GL_EXTENSIONS, (GLuint)i));
    }
    if constexpr (DEBUG_PRINT_EXTENSIONS) {
        for (auto extension: exts) {
            slog.d << "\"" << std::string_view(extension) << "\"\n";
        }
        flush(slog.d);
    }

    using namespace std::literals;
    ext.APPLE_color_buffer_packed_float = true;  // Assumes core profile.
    ext.ARB_shading_language_packing = exts.has("GL_ARB_shading_language_packing"sv);
    ext.EXT_color_buffer_float = true;  // Assumes core profile.
    ext.EXT_color_buffer_half_float = true;  // Assumes core profile.
    ext.EXT_clip_cull_distance = true;
    ext.EXT_debug_marker = exts.has("GL_EXT_debug_marker"sv);
    ext.EXT_discard_framebuffer = false;
    ext.EXT_disjoint_timer_query = true;
    ext.EXT_multisampled_render_to_texture = false;
    ext.EXT_multisampled_render_to_texture2 = false;
    ext.EXT_shader_framebuffer_fetch = exts.has("GL_EXT_shader_framebuffer_fetch"sv);
    ext.EXT_texture_compression_bptc = exts.has("GL_EXT_texture_compression_bptc"sv);
    ext.EXT_texture_compression_etc2 = exts.has("GL_ARB_ES3_compatibility"sv);
    ext.EXT_texture_compression_rgtc = exts.has("GL_EXT_texture_compression_rgtc"sv);
    ext.EXT_texture_compression_s3tc = exts.has("GL_EXT_texture_compression_s3tc"sv);
    ext.EXT_texture_compression_s3tc_srgb = exts.has("GL_EXT_texture_compression_s3tc_srgb"sv);
    ext.EXT_texture_cube_map_array = true;
    ext.EXT_texture_filter_anisotropic = exts.has("GL_EXT_texture_filter_anisotropic"sv);
    ext.EXT_texture_sRGB = exts.has("GL_EXT_texture_sRGB"sv);
    ext.GOOGLE_cpp_style_line_directive = exts.has("GL_GOOGLE_cpp_style_line_directive"sv);
    ext.KHR_parallel_shader_compile = exts.has("GL_KHR_parallel_shader_compile"sv);
    ext.KHR_texture_compression_astc_hdr = exts.has("GL_KHR_texture_compression_astc_hdr"sv);
    ext.KHR_texture_compression_astc_ldr = exts.has("GL_KHR_texture_compression_astc_ldr"sv);
    ext.OES_depth_texture = true;
    ext.OES_depth24 = true;
    ext.OES_EGL_image_external_essl3 = false;
    ext.OES_rgb8_rgba8 = true;
    ext.OES_standard_derivatives = true;
    ext.OES_texture_npot = true;
    ext.OES_vertex_array_object = true;
    ext.WEBGL_compressed_texture_etc = false;
    ext.WEBGL_compressed_texture_s3tc = false;
    ext.WEBGL_compressed_texture_s3tc_srgb = false;

    auto const major = state.major;
    auto const minor = state.minor;

    // OpenGL 4.2 implies ARB_shading_language_packing
    if (major > 4 || (major == 4 && minor >= 2)) {
        ext.ARB_shading_language_packing = true;
    }
    // OpenGL 4.3 implies EXT_discard_framebuffer
    if (major > 4 || (major == 4 && minor >= 3)) {
        ext.EXT_discard_framebuffer = true;
        ext.KHR_debug = true;
    }
    // OpenGL 4.5 implies EXT_clip_control
    if (major > 4 || (major == 4 && minor >= 5)) {
        ext.EXT_clip_control = true;
    }
}

#endif // BACKEND_OPENGL_VERSION_GL

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
        size_t const targetIndex = getIndexForBufferTarget(target);
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
        case GL_PACK_ALIGNMENT:
            pcur = &state.pack.alignment;
            break;
        case GL_UNPACK_ALIGNMENT:
            pcur = &state.unpack.alignment;
            break;
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        case GL_UNPACK_ROW_LENGTH:
            assert_invariant(state.major > 2);
            pcur = &state.unpack.row_length;
            break;
#endif
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

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    assert_invariant(state.major > 2 ||
            (target != GL_UNIFORM_BUFFER && target != GL_TRANSFORM_FEEDBACK_BUFFER));

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
#endif
}

void OpenGLContext::deleteVertexArrays(GLsizei n, const GLuint* arrays) noexcept {
    procs.deleteVertexArrays(n, arrays);
    // if one of the destroyed VAO is bound, clear the binding.
    for (GLsizei i = 0; i < n; ++i) {
        if (state.vao.p->vao == arrays[i]) {
            bindVertexArray(nullptr);
            break;
        }
    }
}

void OpenGLContext::resetState() noexcept {
    // Force GL state to match the Filament state
    if (state.major > 2) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, state.draw_fbo);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, state.read_fbo);
#endif
    } else {
        assert_invariant(state.read_fbo == state.draw_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, state.draw_fbo);
        state.read_fbo = state.draw_fbo;
    }


    // state.program
    glUseProgram(state.program.use);

    // state.vao
    if (state.vao.p) {
        procs.bindVertexArray(state.vao.p->vao);
    } else {
        bindVertexArray(nullptr);
    }

    // state.raster
    glFrontFace(state.raster.frontFace);
    glCullFace(state.raster.cullFace);
    glBlendEquationSeparate(state.raster.blendEquationRGB, state.raster.blendEquationA);
    glBlendFuncSeparate(
        state.raster.blendFunctionSrcRGB, 
        state.raster.blendFunctionDstRGB,
        state.raster.blendFunctionSrcA,
        state.raster.blendFunctionDstA
    );
    glColorMask(
        state.raster.colorMask, 
        state.raster.colorMask, 
        state.raster.colorMask, 
        state.raster.colorMask
    );
    glDepthMask(state.raster.depthMask);
    glDepthFunc(state.raster.depthFunc);
    
    // state.stencil
    glStencilFuncSeparate(
        GL_FRONT, 
        state.stencil.front.func.func, 
        state.stencil.front.func.ref, 
        state.stencil.front.func.mask
    );
    glStencilFuncSeparate(
        GL_BACK, 
        state.stencil.back.func.func, 
        state.stencil.back.func.ref, 
        state.stencil.back.func.mask
    );
    glStencilOpSeparate(
        GL_FRONT, 
        state.stencil.front.op.sfail,
        state.stencil.front.op.dpfail,
        state.stencil.front.op.dppass
    );
    glStencilOpSeparate(
        GL_BACK, 
        state.stencil.back.op.sfail,
        state.stencil.back.op.dpfail,
        state.stencil.back.op.dppass
    );
    glStencilMaskSeparate(GL_FRONT, state.stencil.front.stencilMask);
    glStencilMaskSeparate(GL_BACK, state.stencil.back.stencilMask);

    // state.polygonOffset
    glPolygonOffset(state.polygonOffset.factor, state.polygonOffset.units);

    // state.enables
    setDefaultState();

    // state.buffers
    // Reset state.buffers to its default state to avoid the complexity and error-prone
    // nature of resetting the GL state to its existing state
    state.buffers = {};

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    if (state.major > 2) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        for (auto const target: {
                GL_UNIFORM_BUFFER,
                GL_TRANSFORM_FEEDBACK_BUFFER,
#if defined(BACKEND_OPENGL_LEVEL_GLES31)
                GL_SHADER_STORAGE_BUFFER,
#endif
                GL_PIXEL_PACK_BUFFER,
                GL_PIXEL_UNPACK_BUFFER,
        }) {
            glBindBuffer(target, 0);
        }

        for (size_t bufferIndex = 0; bufferIndex < MAX_BUFFER_BINDINGS; ++bufferIndex) {
            if (bufferIndex < (size_t)gets.max_uniform_buffer_bindings) {
                glBindBufferBase(GL_UNIFORM_BUFFER, bufferIndex, 0);
            }

            if (bufferIndex < (size_t)gets.max_transform_feedback_separate_attribs) {
                glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, bufferIndex, 0);
            }
        }
#endif
    }

    // state.textures
    // Reset state.textures to its default state to avoid the complexity and error-prone
    // nature of resetting the GL state to its existing state
    state.textures = {};
    const std::pair<GLuint, bool> textureTargets[] = {
            { GL_TEXTURE_2D,                true },
            { GL_TEXTURE_2D_ARRAY,          true },
            { GL_TEXTURE_CUBE_MAP,          true },
            { GL_TEXTURE_3D,                true },
#if defined(BACKEND_OPENGL_LEVEL_GLES31)
            { GL_TEXTURE_2D_MULTISAMPLE,    true },
#endif
#if !defined(__EMSCRIPTEN__)
#if defined(GL_OES_EGL_image_external)
            { GL_TEXTURE_EXTERNAL_OES,      ext.OES_EGL_image_external_essl3 },
#endif
#if defined(BACKEND_OPENGL_VERSION_GL) || defined(GL_EXT_texture_cube_map_array)
            { GL_TEXTURE_CUBE_MAP_ARRAY,    ext.EXT_texture_cube_map_array },
#endif
#endif
    };
    for (GLint unit = 0; unit < gets.max_combined_texture_image_units; ++unit) {
        glActiveTexture(GL_TEXTURE0 + unit);
        if (state.major > 2) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
            glBindSampler(unit, 0);
#endif
        }
        for (auto [target, available] : textureTargets) {
            if (available) {
                glBindTexture(target, 0);
            }
        }
    }
    glActiveTexture(GL_TEXTURE0 + state.textures.active);

    // state.unpack
    glPixelStorei(GL_UNPACK_ALIGNMENT, state.unpack.alignment);
    if (state.major > 2) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        glPixelStorei(GL_UNPACK_ROW_LENGTH, state.unpack.row_length);
#endif
    }


    // state.pack
    glPixelStorei(GL_PACK_ALIGNMENT, state.pack.alignment);
    if (state.major > 2) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        glPixelStorei(GL_PACK_ROW_LENGTH, 0); // we rely on GL_PACK_ROW_LENGTH being zero
#endif
    }

    // state.window
    glScissor(
        state.window.scissor.x, 
        state.window.scissor.y, 
        state.window.scissor.z, 
        state.window.scissor.w
    );
    glViewport(
        state.window.viewport.x,
        state.window.viewport.y,
        state.window.viewport.z,
        state.window.viewport.w
    );
    glDepthRangef(state.window.depthRange.x, state.window.depthRange.y);
    
}

} // namesapce filament
