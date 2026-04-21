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
#include "OpenGLState.h"

#include "GLUtils.h"
#include "OpenGLTimerQuery.h"

#include <backend/platforms/OpenGLPlatform.h>
#include <backend/DriverEnums.h>
#include <backend/Platform.h>

#include <utils/Logger.h>
#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/ostream.h>

#include <algorithm>
#include <string_view>

#include <stddef.h>
#include <stdio.h>
#include <string.h>

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

OpenGLContext::OpenGLContext(OpenGLPlatform& platform,
        Platform::DriverConfig const& driverConfig) noexcept
        : mPlatform(platform),
          mDriverConfig(driverConfig) {

    // These queries work with all GL/GLES versions!
    vendor   = (char const*)glGetString(GL_VENDOR);
    renderer = (char const*)glGetString(GL_RENDERER);
    version  = (char const*)glGetString(GL_VERSION);
    shader   = (char const*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    LOG(INFO) << "[" << vendor << "], [" << renderer << "], "
                 "[" << version << "], [" << shader << "]";

    /*
     * Figure out GL / GLES version, extensions and capabilities we need to
     * determine the feature level
     */

    queryOpenGLVersion(&major, &minor);

    #if defined(BACKEND_OPENGL_VERSION_GLES)
    if (UTILS_UNLIKELY(driverConfig.forceGLES2Context)) {
        major = 2;
        minor = 0;
    }
    #endif

    initExtensions(&ext, major, minor);

    initProcs(&procs, ext, major, minor);

    initBugs(&bugs, ext, major, minor,
            vendor, renderer, version, shader);

    initWorkarounds(bugs, &ext);

    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE,             &gets.max_renderbuffer_size);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,           &gets.max_texture_image_units);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,  &gets.max_combined_texture_image_units);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE,                  &gets.max_texture_size);
    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE,         &gets.max_cubemap_texture_size);
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE,               &gets.max_3d_texture_size);
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS,          &gets.max_array_texture_layers);

    mFeatureLevel = resolveFeatureLevel(major, minor, ext, gets, bugs);

#ifdef BACKEND_OPENGL_VERSION_GLES
    mShaderModel = ShaderModel::MOBILE;
#else
    mShaderModel = ShaderModel::DESKTOP;
#endif

#ifdef BACKEND_OPENGL_VERSION_GLES
    if (mFeatureLevel >= FeatureLevel::FEATURE_LEVEL_2) {
        features.multisample_texture = true;
    }
#else
    if (mFeatureLevel >= FeatureLevel::FEATURE_LEVEL_1) {
        features.multisample_texture = true;
    }
#endif

    if (mFeatureLevel >= FeatureLevel::FEATURE_LEVEL_1) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
#ifdef GL_EXT_texture_filter_anisotropic
        if (ext.EXT_texture_filter_anisotropic) {
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gets.max_anisotropy);
        }
#endif
        glGetIntegerv(GL_MAX_DRAW_BUFFERS,
                &gets.max_draw_buffers);
        glGetIntegerv(GL_MAX_SAMPLES,
                &gets.max_samples);
        glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS,
                &gets.max_transform_feedback_separate_attribs);
        glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE,
                &gets.max_uniform_block_size);
        glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS,
                &gets.max_uniform_buffer_bindings);
        glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS,
                &gets.num_program_binary_formats);
        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,
                &gets.uniform_buffer_offset_alignment);
#endif
    }

#ifdef BACKEND_OPENGL_VERSION_GLES
    else {
        gets.max_anisotropy = 1;
        gets.max_draw_buffers = 1;
        gets.max_samples = 1;
        gets.max_transform_feedback_separate_attribs = 0;
        gets.max_uniform_block_size = 0;
        gets.max_uniform_buffer_bindings = 0;
        gets.num_program_binary_formats = 0;
        gets.uniform_buffer_offset_alignment = 0;
    }
#endif

    LOG(INFO) << "Feature level: " << +mFeatureLevel;
    LOG(INFO) << "Active workarounds: ";
    UTILS_NOUNROLL
    for (auto [enabled, name, _] : mBugDatabase) {
        if (enabled) {
            LOG(INFO) << name;
        }
    }

#ifndef NDEBUG
    // this is useful for development
    LOG(INFO) << "GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT = " << gets.max_anisotropy;
    LOG(INFO) << "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS = " << gets.max_combined_texture_image_units;
    LOG(INFO) << "GL_MAX_TEXTURE_SIZE = " << gets.max_texture_size;
    LOG(INFO) << "GL_MAX_CUBE_MAP_TEXTURE_SIZE = " << gets.max_cubemap_texture_size;
    LOG(INFO) << "GL_MAX_3D_TEXTURE_SIZE = " << gets.max_3d_texture_size;
    LOG(INFO) << "GL_MAX_ARRAY_TEXTURE_LAYERS = " << gets.max_array_texture_layers;
    LOG(INFO) << "GL_MAX_DRAW_BUFFERS = " << gets.max_draw_buffers;
    LOG(INFO) << "GL_MAX_RENDERBUFFER_SIZE = " << gets.max_renderbuffer_size;
    LOG(INFO) << "GL_MAX_SAMPLES = " << gets.max_samples;
    LOG(INFO) << "GL_MAX_TEXTURE_IMAGE_UNITS = " << gets.max_texture_image_units;
    LOG(INFO) << "GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS = "
              << gets.max_transform_feedback_separate_attribs;
    LOG(INFO) << "GL_MAX_UNIFORM_BLOCK_SIZE = " << gets.max_uniform_block_size;
    LOG(INFO) << "GL_MAX_UNIFORM_BUFFER_BINDINGS = " << gets.max_uniform_buffer_bindings;
    LOG(INFO) << "GL_NUM_PROGRAM_BINARY_FORMATS = " << gets.num_program_binary_formats;
    LOG(INFO) << "GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT = " << gets.uniform_buffer_offset_alignment;
#endif

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    assert_invariant(mFeatureLevel == FeatureLevel::FEATURE_LEVEL_0 || gets.max_draw_buffers >= 4); // minspec
#endif

#ifdef GL_EXT_texture_filter_anisotropic
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    if (mFeatureLevel >= FeatureLevel::FEATURE_LEVEL_1 && ext.EXT_texture_filter_anisotropic) {
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

#if defined(BACKEND_OPENGL_VERSION_GL) || defined(GL_EXT_disjoint_timer_query)
    if (ext.EXT_disjoint_timer_query) {
        mGpuTimeSupported = true;
    } else
#endif
    if (platform.canCreateFence()) {
        mGpuTimeSupported = true;
    }

    // in practice KHR_Debug has never been useful, and actually is confusing. We keep this
    // only for our own debugging, in case we need it some day.
#if false && !defined(NDEBUG) && defined(GL_KHR_debug)
    if (ext.KHR_debug) {
        auto cb = +[](GLenum, GLenum type, GLuint, GLenum severity, GLsizei length,
                const GLchar* message, const void *) {
            auto logSeverity = utils::LogSeverity::kInfo;
            switch (severity) {
                case GL_DEBUG_SEVERITY_HIGH:    logSeverity = utils::LogSeverity::kError;   break;
                case GL_DEBUG_SEVERITY_MEDIUM:  logSeverity = utils::LogSeverity::kWarning; break;
                case GL_DEBUG_SEVERITY_LOW:     logSeverity = utils::LogSeverity::kInfo;    break;
                case GL_DEBUG_SEVERITY_NOTIFICATION:
                default: break;
            }
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
            LOG(LEVEL(logSeverity)) << "KHR_debug " << level << std::string_view{ message, size_t(length) };
        };
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(cb, nullptr);
    }
#endif
}

OpenGLContext::~OpenGLContext() noexcept = default;

OpenGLState* OpenGLContext::createState() noexcept {
    auto* state = new(std::nothrow) OpenGLState(*this);
    assert_invariant(state);
    return state;
}

void OpenGLContext::destroyState(OpenGLState* state) noexcept {
    if (state) {
        state->terminate();
        delete state;
    }
}


void OpenGLContext::initProcs(Procs* procs,
        Extensions const& ext, GLint major, GLint) noexcept {
    (void)ext;
    (void)major;

    // default procs that can be overridden based on runtime version
#ifdef BACKEND_OPENGL_LEVEL_GLES30
    procs->genVertexArrays = glGenVertexArrays;
    procs->bindVertexArray = glBindVertexArray;
    procs->deleteVertexArrays = glDeleteVertexArrays;

    // these are core in GL and GLES 3.x
    procs->genQueries = glGenQueries;
    procs->deleteQueries = glDeleteQueries;
    procs->beginQuery = glBeginQuery;
    procs->endQuery = glEndQuery;
    procs->getQueryObjectuiv = glGetQueryObjectuiv;
#   ifdef BACKEND_OPENGL_VERSION_GL
    procs->getQueryObjectui64v = glGetQueryObjectui64v; // only core in GL
#   elif defined(GL_EXT_disjoint_timer_query)
#       ifndef __EMSCRIPTEN__
            procs->getQueryObjectui64v = glGetQueryObjectui64vEXT;
#       endif
#   endif // BACKEND_OPENGL_VERSION_GL

    // core in ES 3.0 and GL 4.3
    procs->invalidateFramebuffer = glInvalidateFramebuffer;
#endif // BACKEND_OPENGL_LEVEL_GLES30

    // no-op if not supported
    procs->maxShaderCompilerThreadsKHR = +[](GLuint) {};

#ifdef BACKEND_OPENGL_VERSION_GLES
#   ifndef FILAMENT_IOS // FILAMENT_IOS is guaranteed to have ES3.x
#       ifndef __EMSCRIPTEN__
    if (UTILS_UNLIKELY(major == 2)) {
        // Runtime OpenGL version is ES 2.x
        if (UTILS_LIKELY(ext.OES_vertex_array_object)) {
            procs->genVertexArrays = glGenVertexArraysOES;
            procs->bindVertexArray = glBindVertexArrayOES;
            procs->deleteVertexArrays = glDeleteVertexArraysOES;
        } else {
            // if we don't have OES_vertex_array_object, just don't do anything with real VAOs,
            // we'll just rebind everything each time. Most Mali-400 support this extension, but
            // a few don't.
            procs->genVertexArrays = +[](GLsizei, GLuint*) {};
            procs->bindVertexArray = +[](GLuint) {};
            procs->deleteVertexArrays = +[](GLsizei, GLuint const*) {};
        }

        // EXT_disjoint_timer_query is optional -- pointers will be null if not available
        procs->genQueries = glGenQueriesEXT;
        procs->deleteQueries = glDeleteQueriesEXT;
        procs->beginQuery = glBeginQueryEXT;
        procs->endQuery = glEndQueryEXT;
        procs->getQueryObjectuiv = glGetQueryObjectuivEXT;
        procs->getQueryObjectui64v = glGetQueryObjectui64vEXT;

        procs->invalidateFramebuffer = glDiscardFramebufferEXT;

        procs->maxShaderCompilerThreadsKHR = glMaxShaderCompilerThreadsKHR;
    }
#       endif // __EMSCRIPTEN__
#   endif // FILAMENT_IOS
#else
    procs->maxShaderCompilerThreadsKHR = glMaxShaderCompilerThreadsARB;
#endif
}

void OpenGLContext::initBugs(Bugs* bugs, Extensions const& exts,
        GLint major, GLint minor,
        char const* vendor,
        char const* renderer,
        char const* version,
        char const* shader) {

    (void)major;
    (void)minor;
    (void)vendor;
    (void)renderer;
    (void)version;
    (void)shader;

    const bool isAngle = strstr(renderer, "ANGLE");
    if (!isAngle) {
        if (strstr(renderer, "Adreno")) {
            // Qualcomm GPU
            bugs->invalidate_end_only_if_invalidate_start = true;

            // On Adreno (As of 3/20) timer query seem to return the CPU time, not the GPU time.
            bugs->dont_use_timer_query = true;

            // Blits to texture arrays are failing
            //   This bug continues to reproduce, though at times we've seen it appear to "go away".
            //   The standalone sample app that was written to show this problem still reproduces.
            //   The working hypothesis is that some other state affects this behavior.
            bugs->disable_blit_into_texture_array = true;

            // early exit condition is flattened in EASU code
            bugs->split_easu = true;

            // initialize the non-used uniform array for Adreno drivers.
            bugs->enable_initialize_non_used_uniform_array = true;

            int maj, min, driverMajor, driverMinor;
            int const c = sscanf(version, "OpenGL ES %d.%d V@%d.%d", // NOLINT(cert-err34-c)
                    &maj, &min, &driverMajor, &driverMinor);
            if (c == 4) {
                // Workarounds based on version here.
                // Notes:
                //  bugs.invalidate_end_only_if_invalidate_start
                //  - appeared at least in
                //      "OpenGL ES 3.2 V@0490.0 (GIT@85da404, I46ff5fc46f, 1606794520) (Date:11/30/20)"
                //  - wasn't present in
                //      "OpenGL ES 3.2 V@0490.0 (GIT@0905e9f, Ia11ce2d146, 1599072951) (Date:09/02/20)"
                //  - has been confirmed fixed in V@570.1 by Qualcomm
                if (driverMajor < 490 || driverMajor > 570 ||
                    (driverMajor == 570 && driverMinor >= 1)) {
                    bugs->invalidate_end_only_if_invalidate_start = false;
                }
            }

            // qualcomm seems to have no problem with this (which is good for us)
            bugs->allow_read_only_ancillary_feedback_loop = true;

#ifndef __EMSCRIPTEN__
            // Older Adreno devices that support ES3.0 only tend to be extremely buggy, so we
            // fall back to ES2.0.
            if (major == 3 && minor == 0) {
                bugs->force_feature_level0 = true;
            }
#endif
        } else if (strstr(renderer, "Mali")) {
            // ARM GPU
            bugs->vao_doesnt_store_element_array_buffer_binding = true;

            // We have run into several problems with timer queries on Mali-Gxx:
            // - timer queries seem to cause memory corruptions in some cases on some devices
            //   (see b/233754398)
            //          - appeared at least in: "OpenGL ES 3.2 v1.r26p0-01eac0"
            //          - wasn't present in: "OpenGL ES 3.2 v1.r32p1-00pxl1"
            // - timer queries sometime crash with an NPE (see b/273759031)
            //   (see b/273759031)
            //          - possibly not present in: "OpenGL ES 3.2 v1.r46p0"
            bugs->dont_use_timer_query = true;

            // at least some version of Mali have problems with framebuffer_fetch
            // (see b/445721121, https://github.com/google/filament/issues/7794)
            bugs->disable_framebuffer_fetch_extension = true;

            if (strstr(renderer, "Mali-T")) {
                bugs->disable_glFlush = true;
                bugs->disable_shared_context_draws = true;
            }
            if (strstr(renderer, "Mali-G")) {
                int maj, min, driverVersion, driverRelease;
                int const c = sscanf(version, "OpenGL ES %d.%d v%d.r%d",
                        &maj, &min, &driverVersion, &driverRelease);
                if (UTILS_LIKELY(c == 4)) {
                    // we were able to parse the version string
                    if (driverVersion > 1 || (driverVersion == 1 && driverRelease >= 46)) {
                        // this driver version is known to be good
                        bugs->dont_use_timer_query = false;
                    }
                    if (driverVersion > 1 || (driverVersion == 1 && driverRelease >= 53)) {
                        // this driver version is known to be good
                        bugs->disable_framebuffer_fetch_extension = false;
                    }
                }
            }
            // Mali seems to have no problem with this (which is good for us)
            bugs->allow_read_only_ancillary_feedback_loop = true;
        } else if (strstr(renderer, "Intel")) {
            // Intel GPU
            bugs->vao_doesnt_store_element_array_buffer_binding = true;

            if (strstr(renderer, "Mesa")) {
                // Mesa Intel driver on Linux/Android
                // Renderer of the form [Mesa Intel(R) HD Graphics 505 (APL 3)]
                // b/405252622
                bugs->disable_invalidate_framebuffer = true;
            }
        } else if (strstr(renderer, "PowerVR")) {
            // PowerVR GPU
            // On PowerVR (Rogue GE8320) glFlush doesn't seem to do anything, in particular,
            // it doesn't kick the GPU earlier, so don't issue these calls as they seem to slow
            // things down.
            bugs->disable_glFlush = true;
            // On PowerVR (Rogue GE8320) using gl_InstanceID too early in the shader doesn't work.
            bugs->powervr_shader_workarounds = true;
            // On PowerVR (Rogue GE8320) destroying a fbo after glBlitFramebuffer is effectively
            // equivalent to glFinish.
            bugs->delay_fbo_destruction = true;
            // PowerVR seems to have no problem with this (which is good for us)
            bugs->allow_read_only_ancillary_feedback_loop = true;
        } else if (strstr(renderer, "Apple")) {
            // Apple GPU
        } else if (strstr(renderer, "Tegra") ||
                   strstr(renderer, "GeForce") ||
                   strstr(renderer, "NV")) {
            // NVIDIA GPU
        } else if (strstr(renderer, "Vivante")) {
            // Vivante GPU
        } else if (strstr(renderer, "AMD") ||
                   strstr(renderer, "ATI")) {
            // AMD/ATI GPU
        } else if (strstr(renderer, "Mozilla")) {
            bugs->disable_invalidate_framebuffer = true;
        }

        if (strstr(vendor, "Mesa")) {
            if (strstr(renderer, "llvmpipe")) {
                // Seen on
                //  [Mesa],
                //  [llvmpipe (LLVM 17.0.6, 256 bits)],
                //  [4.5 (Core Profile) Mesa 24.0.6-1],
                //  [4.50]
                // not known which version are affected
                bugs->rebind_buffer_after_deletion = true;

                // Seen on
                // [Mesa]
                // [llvmpipe (LLVM 17.0.6, 256 bits)]
                // [4.5 (Core Profile) Mesa 24.2.1 (git-c222f7299c)]
                // [4.50]
                bugs->disable_framebuffer_fetch_extension = true;
            }
        }
    } else {
        // When running under ANGLE, it's a different set of workaround that we need.
        if (strstr(renderer, "Adreno")) {
            // Qualcomm GPU
            // early exit condition is flattened in EASU code
            // (that should be regardless of ANGLE, but we should double-check)
            bugs->split_easu = true;
        }
    }

    if (strstr(vendor, "Mozilla")) {
        // Seen on
        //  [Mozilla],
        //  [GeForce GTX 980, or similar]
        //    or [ANGLE (NVIDIA, NVIDIA GeForce GTX 980 Direct3D11 vs_5_0 ps_5_0), or similar]
        //    or anything else,
        //  [OpenGL ES 3.0 (WebGL 2.0)],
        //  [OpenGL ES GLSL ES 3.00 (WebGL GLSL ES 3.00)]
        // For Mozilla, the issue appears to be observed regardless of whether the renderer is
        // ANGLE or not. (b/376125497)
        bugs->rebind_buffer_after_deletion = true;

        // We disable depth precache for the default material on Mozilla FireFox. It struggles with
        // slow shader compile/link times if the shader contains large arrays of uniform. Some depth
        // program variants have skinning-related data, which incurs this slowness and end up
        // causing an initial startup stalls. (b/392917621)
        bugs->disable_depth_precache_for_default_material = true;
    }

#ifdef BACKEND_OPENGL_VERSION_GLES
#   ifndef FILAMENT_IOS // FILAMENT_IOS is guaranteed to have ES3.x
    if (UTILS_UNLIKELY(major == 2)) {
        if (UTILS_UNLIKELY(!exts.OES_vertex_array_object)) {
            // we activate this workaround path, which does the reset of array buffer
            bugs->vao_doesnt_store_element_array_buffer_binding = true;
        }
    }
#   endif // FILAMENT_IOS
#else
    // feedback loops are allowed on GL desktop as long as writes are disabled
    bugs->allow_read_only_ancillary_feedback_loop = true;
#endif
}

void OpenGLContext::initWorkarounds(Bugs const& bugs, Extensions* ext) {
    if (bugs.disable_framebuffer_fetch_extension) {
        ext->EXT_shader_framebuffer_fetch = false;
    }
}

FeatureLevel OpenGLContext::resolveFeatureLevel(GLint major, GLint minor,
        Extensions const& exts,
        Gets const& gets,
        Bugs const& bugs) noexcept {

    constexpr auto const caps3 = FEATURE_LEVEL_CAPS[+FeatureLevel::FEATURE_LEVEL_3];
    constexpr GLint MAX_VERTEX_SAMPLER_COUNT = caps3.MAX_VERTEX_SAMPLER_COUNT;
    constexpr GLint MAX_FRAGMENT_SAMPLER_COUNT = caps3.MAX_FRAGMENT_SAMPLER_COUNT;

    (void)exts;
    (void)gets;
    (void)bugs;

    FeatureLevel featureLevel = FeatureLevel::FEATURE_LEVEL_1;

#ifdef BACKEND_OPENGL_VERSION_GLES
    if (major == 3) {
        // Runtime OpenGL version is ES 3.x
        assert_invariant(gets.max_texture_image_units >= 16);
        assert_invariant(gets.max_combined_texture_image_units >= 32);
        if (minor >= 1) {
            // figure out our feature level
            if (exts.EXT_texture_cube_map_array) {
                featureLevel = FeatureLevel::FEATURE_LEVEL_2;
                if (gets.max_texture_image_units >= MAX_FRAGMENT_SAMPLER_COUNT &&
                    gets.max_combined_texture_image_units >=
                            (MAX_FRAGMENT_SAMPLER_COUNT + MAX_VERTEX_SAMPLER_COUNT)) {
                    featureLevel = FeatureLevel::FEATURE_LEVEL_3;
                }
            }
        }
    }
#   ifndef FILAMENT_IOS // FILAMENT_IOS is guaranteed to have ES3.x
    else if (UTILS_UNLIKELY(major == 2)) {
        // Runtime OpenGL version is ES 2.x
        // note: mandatory extensions (all supported by Mali-400 and Adreno 304)
        //      OES_depth_texture
        //      OES_depth24
        //      OES_packed_depth_stencil
        //      OES_rgb8_rgba8
        //      OES_standard_derivatives
        //      OES_texture_npot
        featureLevel = FeatureLevel::FEATURE_LEVEL_0;
    }
#   endif // FILAMENT_IOS
#else
    assert_invariant(gets.max_texture_image_units >= 16);
    assert_invariant(gets.max_combined_texture_image_units >= 32);
    if (major == 4) {
        assert_invariant(minor >= 1);
        if (minor >= 3) {
            // cubemap arrays are available as of OpenGL 4.0
            featureLevel = FeatureLevel::FEATURE_LEVEL_2;
            // figure out our feature level
            if (gets.max_texture_image_units >= MAX_FRAGMENT_SAMPLER_COUNT &&
                gets.max_combined_texture_image_units >=
                (MAX_FRAGMENT_SAMPLER_COUNT + MAX_VERTEX_SAMPLER_COUNT)) {
                featureLevel = FeatureLevel::FEATURE_LEVEL_3;
            }
        }
    }
#endif

    if (bugs.force_feature_level0) {
        featureLevel = FeatureLevel::FEATURE_LEVEL_0;
    }

    return featureLevel;
}

#ifdef BACKEND_OPENGL_VERSION_GLES

void OpenGLContext::initExtensionsGLES(Extensions* ext, GLint major, GLint minor) noexcept {
    const char * const extensions = (const char*)glGetString(GL_EXTENSIONS);
    GLUtils::unordered_string_set const exts = GLUtils::split(extensions);
    if constexpr (DEBUG_PRINT_EXTENSIONS) {
        for (auto extension: exts) {
            DLOG(INFO) << "\"" << std::string_view(extension) << "\"";
        }
    }

    // figure out and initialize the extensions we need
    using namespace std::literals;
    ext->APPLE_color_buffer_packed_float = exts.has("GL_APPLE_color_buffer_packed_float"sv);
#ifndef __EMSCRIPTEN__
    ext->EXT_clip_control = exts.has("GL_EXT_clip_control"sv);
#endif
    ext->EXT_clip_cull_distance = exts.has("GL_EXT_clip_cull_distance"sv);
    ext->EXT_color_buffer_float = exts.has("GL_EXT_color_buffer_float"sv);
    ext->EXT_color_buffer_half_float = exts.has("GL_EXT_color_buffer_half_float"sv);
#ifndef __EMSCRIPTEN__
    ext->EXT_debug_marker = exts.has("GL_EXT_debug_marker"sv);
#endif
    ext->EXT_depth_clamp = exts.has("GL_EXT_depth_clamp"sv);
    ext->EXT_discard_framebuffer = exts.has("GL_EXT_discard_framebuffer"sv);
#ifndef __EMSCRIPTEN__
    ext->EXT_disjoint_timer_query = exts.has("GL_EXT_disjoint_timer_query"sv);
    ext->EXT_multisampled_render_to_texture = exts.has("GL_EXT_multisampled_render_to_texture"sv);
    ext->EXT_multisampled_render_to_texture2 = exts.has("GL_EXT_multisampled_render_to_texture2"sv);
    ext->EXT_protected_textures = exts.has("GL_EXT_protected_textures"sv);
#endif
    ext->EXT_shader_framebuffer_fetch = exts.has("GL_EXT_shader_framebuffer_fetch"sv);
#ifndef __EMSCRIPTEN__
    ext->EXT_texture_compression_etc2 = true;
#endif
    ext->EXT_texture_compression_s3tc = exts.has("GL_EXT_texture_compression_s3tc"sv);
    ext->EXT_texture_compression_s3tc_srgb = exts.has("GL_EXT_texture_compression_s3tc_srgb"sv);
    ext->EXT_texture_compression_rgtc = exts.has("GL_EXT_texture_compression_rgtc"sv);
    ext->EXT_texture_compression_bptc = exts.has("GL_EXT_texture_compression_bptc"sv);
    ext->EXT_texture_cube_map_array = exts.has("GL_EXT_texture_cube_map_array"sv) || exts.has("GL_OES_texture_cube_map_array"sv);
    ext->EXT_texture_filter_anisotropic = exts.has("GL_EXT_texture_filter_anisotropic"sv);
    ext->GOOGLE_cpp_style_line_directive = exts.has("GL_GOOGLE_cpp_style_line_directive"sv);
    ext->KHR_debug = exts.has("GL_KHR_debug"sv);
    ext->KHR_parallel_shader_compile = exts.has("GL_KHR_parallel_shader_compile"sv);
    ext->KHR_texture_compression_astc_hdr = exts.has("GL_KHR_texture_compression_astc_hdr"sv);
    ext->KHR_texture_compression_astc_ldr = exts.has("GL_KHR_texture_compression_astc_ldr"sv);
    ext->OES_depth_texture = exts.has("GL_OES_depth_texture"sv);
    ext->OES_depth24 = exts.has("GL_OES_depth24"sv);
    ext->OES_packed_depth_stencil = exts.has("GL_OES_packed_depth_stencil"sv);
    ext->OES_EGL_image_external_essl3 = exts.has("GL_OES_EGL_image_external_essl3"sv);
    ext->OES_rgb8_rgba8 = exts.has("GL_OES_rgb8_rgba8"sv);
    ext->OES_standard_derivatives = exts.has("GL_OES_standard_derivatives"sv);
    ext->OES_texture_float_linear = exts.has("GL_OES_texture_float_linear"sv);
    ext->OES_texture_half_float_linear = exts.has("GL_OES_texture_half_float_linear"sv);
    ext->OES_texture_npot = exts.has("GL_OES_texture_npot"sv);
    ext->OES_vertex_array_object = exts.has("GL_OES_vertex_array_object"sv);
    ext->OVR_multiview2 = exts.has("GL_OVR_multiview2"sv);
    ext->WEBGL_compressed_texture_etc = exts.has("WEBGL_compressed_texture_etc"sv);
    ext->WEBGL_compressed_texture_s3tc = exts.has("WEBGL_compressed_texture_s3tc"sv);
    ext->WEBGL_compressed_texture_s3tc_srgb = exts.has("WEBGL_compressed_texture_s3tc_srgb"sv);

    // ES 3.2 implies EXT_color_buffer_float (but not necessarily filterable)
    if (major > 3 || (major == 3 && minor >= 2)) {
        ext->EXT_color_buffer_float = true;
    }
    // ES 3.x implies EXT_discard_framebuffer, OES_vertex_array_object and OES_texture_half_float_linear
    if (major >= 3) {
        ext->EXT_discard_framebuffer = true;
        ext->OES_vertex_array_object = true;
        ext->OES_texture_half_float_linear = true;
    }
}

#endif // BACKEND_OPENGL_VERSION_GLES

#ifdef BACKEND_OPENGL_VERSION_GL

void OpenGLContext::initExtensionsGL(Extensions* ext, GLint major, GLint minor) noexcept {
    GLUtils::unordered_string_set exts;
    GLint n = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &n);
    for (GLint i = 0; i < n; i++) {
        exts.emplace((const char*)glGetStringi(GL_EXTENSIONS, (GLuint)i));
    }
    if constexpr (DEBUG_PRINT_EXTENSIONS) {
        for (auto extension: exts) {
            DLOG(INFO) << "\"" << std::string_view(extension) << "\"";
        }
    }

    using namespace std::literals;
    ext->APPLE_color_buffer_packed_float = true;  // Assumes core profile.
    ext->ARB_shading_language_packing = exts.has("GL_ARB_shading_language_packing"sv);
    ext->EXT_color_buffer_float = true;  // Assumes core profile.
    ext->EXT_color_buffer_half_float = true;  // Assumes core profile.
    ext->EXT_clip_cull_distance = true;
    ext->EXT_debug_marker = exts.has("GL_EXT_debug_marker"sv);
    ext->EXT_depth_clamp = true;
    ext->EXT_discard_framebuffer = false;
    ext->EXT_disjoint_timer_query = true;
    ext->EXT_multisampled_render_to_texture = false;
    ext->EXT_multisampled_render_to_texture2 = false;
    ext->EXT_shader_framebuffer_fetch = exts.has("GL_EXT_shader_framebuffer_fetch"sv);
    ext->EXT_texture_compression_bptc = exts.has("GL_EXT_texture_compression_bptc"sv);
    ext->EXT_texture_compression_etc2 = exts.has("GL_ARB_ES3_compatibility"sv);
    ext->EXT_texture_compression_rgtc = exts.has("GL_EXT_texture_compression_rgtc"sv);
    ext->EXT_texture_compression_s3tc = exts.has("GL_EXT_texture_compression_s3tc"sv);
    ext->EXT_texture_compression_s3tc_srgb = exts.has("GL_EXT_texture_compression_s3tc_srgb"sv);
    ext->EXT_texture_cube_map_array = true;
    ext->EXT_texture_filter_anisotropic = exts.has("GL_EXT_texture_filter_anisotropic"sv);
    ext->EXT_texture_sRGB = exts.has("GL_EXT_texture_sRGB"sv);
    ext->GOOGLE_cpp_style_line_directive = exts.has("GL_GOOGLE_cpp_style_line_directive"sv);
    ext->KHR_parallel_shader_compile = exts.has("GL_KHR_parallel_shader_compile"sv);
    ext->KHR_texture_compression_astc_hdr = exts.has("GL_KHR_texture_compression_astc_hdr"sv);
    ext->KHR_texture_compression_astc_ldr = exts.has("GL_KHR_texture_compression_astc_ldr"sv);
    ext->OES_depth_texture = true;
    ext->OES_depth24 = true;
    ext->OES_EGL_image_external_essl3 = false;
    ext->OES_rgb8_rgba8 = true;
    ext->OES_standard_derivatives = true;
    ext->OES_texture_float_linear = true;
    ext->OES_texture_half_float_linear = true;
    ext->OES_texture_npot = true;
    ext->OES_vertex_array_object = true;
    ext->OVR_multiview2 = exts.has("GL_OVR_multiview2"sv);
    ext->WEBGL_compressed_texture_etc = false;
    ext->WEBGL_compressed_texture_s3tc = false;
    ext->WEBGL_compressed_texture_s3tc_srgb = false;

    // OpenGL 4.2 implies ARB_shading_language_packing
    if (major > 4 || (major == 4 && minor >= 2)) {
        ext->ARB_shading_language_packing = true;
    }
    // OpenGL 4.3 implies EXT_discard_framebuffer
    if (major > 4 || (major == 4 && minor >= 3)) {
        ext->EXT_discard_framebuffer = true;
        ext->KHR_debug = true;
    }
    // OpenGL 4.5 implies EXT_clip_control
    if (major > 4 || (major == 4 && minor >= 5)) {
        ext->EXT_clip_control = true;
    }
}

#endif // BACKEND_OPENGL_VERSION_GL

} // namespace filament::backend
