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

#ifndef TNT_FILAMENT_BACKEND_OPENGLCONTEXT_H
#define TNT_FILAMENT_BACKEND_OPENGLCONTEXT_H

#include <backend/platforms/OpenGLPlatform.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include "gl_headers.h"

#include <utils/compiler.h>
#include <utils/bitset.h>
#include <utils/debug.h>

#include <math/vec2.h>
#include <math/vec4.h>

#include <array>
#include <tuple>

#include <stddef.h>
#include <stdint.h>

namespace filament::backend {

class OpenGLPlatform;
class OpenGLState;

class OpenGLContext final {
public:
    static constexpr const size_t MAX_TEXTURE_UNIT_COUNT = MAX_SAMPLER_COUNT;
    static constexpr const size_t DUMMY_TEXTURE_BINDING = 7; // highest binding guaranteed to work with ES2
    static constexpr const size_t MAX_BUFFER_BINDINGS = 32;
    typedef math::details::TVec4<GLint> vec4gli;
    typedef math::details::TVec2<GLclampf> vec2glf;

    struct RenderPrimitive {
        static_assert(MAX_VERTEX_ATTRIBUTE_COUNT <= 16);

        GLuint vao[2] = {};                                     // 8
        GLuint elementArray = 0;                                // 4
        GLenum indicesType = 0;                                 // 4

        // The optional 32-bit handle to a GLVertexBuffer is necessary only if the referenced
        // VertexBuffer supports buffer objects. If this is zero, then the VBO handles array is
        // immutable.
        Handle<HwVertexBuffer> vertexBufferWithObjects;         // 4

        mutable utils::bitset<uint16_t> vertexAttribArray;      // 2

        uint8_t reserved[2] = {};                               // 2

        // if this differs from vertexBufferWithObjects->bufferObjectsVersion, this VAO needs to
        // be updated (see OpenGLDriver::updateVertexArrayObject())
        uint8_t vertexBufferVersion = 0;                        // 1

        // if this differs from OpenGLState::state.age, this VAO needs to
        // be updated (see OpenGLDriver::updateVertexArrayObject())
        uint8_t stateVersion = 0;                               // 1

        // If this differs from OpenGLState::state.age, this VAO's name needs to be updated.
        // See OpenGLState::bindVertexArray()
        uint8_t nameVersion = 0;                                // 1

        // Size in bytes of indices in the index buffer (1 or 2)
        uint8_t indicesShift = 0;                                // 1

        GLenum getIndicesType() const noexcept {
            return indicesType;
        }
    };

    static bool queryOpenGLVersion(GLint* major, GLint* minor) noexcept;

    explicit OpenGLContext(OpenGLPlatform& platform,
            Platform::DriverConfig const& driverConfig) noexcept;

    ~OpenGLContext() noexcept;

    // State lifecycle - allocate/deallocate per-thread OpenGLState instances
    OpenGLState* createState() noexcept;
    void destroyState(OpenGLState* state) noexcept;

    // --------------------------------------------------------------------------------------------

    template<int MAJOR, int MINOR>
    inline bool isAtLeastGL() const noexcept {
#ifdef BACKEND_OPENGL_VERSION_GL
        return major > MAJOR || (major == MAJOR && minor >= MINOR);
#else
        return false;
#endif
    }

    template<int MAJOR, int MINOR>
    inline bool isAtLeastGLES() const noexcept {
#ifdef BACKEND_OPENGL_VERSION_GLES
        return major > MAJOR || (major == MAJOR && minor >= MINOR);
#else
        return false;
#endif
    }

    inline bool isES2() const noexcept {
#if defined(BACKEND_OPENGL_VERSION_GLES) && !defined(FILAMENT_IOS)
#   ifndef BACKEND_OPENGL_LEVEL_GLES30
            return true;
#   else
            return mFeatureLevel == FeatureLevel::FEATURE_LEVEL_0;
#   endif
#else
        return false;
#endif
    }

    bool hasFences() const noexcept {
#if defined(BACKEND_OPENGL_VERSION_GLES) && !defined(FILAMENT_IOS) && !defined(__EMSCRIPTEN__)
#   ifndef BACKEND_OPENGL_LEVEL_GLES30
        return false;
#   else
        return mFeatureLevel > FeatureLevel::FEATURE_LEVEL_0;
#   endif
#else
        return true;
#endif
    }

    ShaderModel getShaderModel() const noexcept { return mShaderModel; }
    FeatureLevel getFeatureLevel() const noexcept { return mFeatureLevel; }
    
    OpenGLPlatform& getPlatform() const noexcept { return mPlatform; }
    Platform::DriverConfig const& getDriverConfig() const noexcept { return mDriverConfig; }
    bool isGpuTimeSupported() const noexcept { return mGpuTimeSupported; }


    // glGet*() values
    struct Gets {
        GLfloat max_anisotropy;
        GLint max_combined_texture_image_units;
        GLint max_draw_buffers;
        GLint max_renderbuffer_size;
        GLint max_samples;
        GLint max_texture_image_units;
        GLint max_texture_size;
        GLint max_cubemap_texture_size;
        GLint max_3d_texture_size;
        GLint max_array_texture_layers;
        GLint max_transform_feedback_separate_attribs;
        GLint max_uniform_block_size;
        GLint max_uniform_buffer_bindings;
        GLint num_program_binary_formats;
        GLint uniform_buffer_offset_alignment;
    } gets = {};

    // features supported by this version of GL or GLES
    struct {
        bool multisample_texture;
    } features = {};

    // supported extensions detected at runtime
    struct Extensions {
        bool APPLE_color_buffer_packed_float;
        bool ARB_shading_language_packing;
        bool EXT_clip_control;
        bool EXT_clip_cull_distance;
        bool EXT_color_buffer_float;
        bool EXT_color_buffer_half_float;
        bool EXT_debug_marker;
        bool EXT_depth_clamp;
        bool EXT_discard_framebuffer;
        bool EXT_disjoint_timer_query;
        bool EXT_multisampled_render_to_texture2;
        bool EXT_multisampled_render_to_texture;
        bool EXT_protected_textures;
        bool EXT_shader_framebuffer_fetch;
        bool EXT_texture_compression_bptc;
        bool EXT_texture_compression_etc2;
        bool EXT_texture_compression_rgtc;
        bool EXT_texture_compression_s3tc;
        bool EXT_texture_compression_s3tc_srgb;
        bool EXT_texture_cube_map_array;
        bool EXT_texture_filter_anisotropic;
        bool EXT_texture_sRGB;
        bool GOOGLE_cpp_style_line_directive;
        bool KHR_debug;
        bool KHR_parallel_shader_compile;
        bool KHR_texture_compression_astc_hdr;
        bool KHR_texture_compression_astc_ldr;
        bool OES_EGL_image_external_essl3;
        bool OES_depth24;
        bool OES_depth_texture;
        bool OES_packed_depth_stencil;
        bool OES_rgb8_rgba8;
        bool OES_standard_derivatives;
        bool OES_texture_float_linear;
        bool OES_texture_half_float_linear;
        bool OES_texture_npot;
        bool OES_vertex_array_object;
        bool OVR_multiview2;
        bool WEBGL_compressed_texture_etc;
        bool WEBGL_compressed_texture_s3tc;
        bool WEBGL_compressed_texture_s3tc_srgb;
    } ext = {};

    struct Bugs {
        // Some drivers have issues with UBOs in the fragment shader when
        // glFlush() is called between draw calls.
        bool disable_glFlush;

        // Some drivers seem to not store the GL_ELEMENT_ARRAY_BUFFER binding
        // in the VAO state.
        bool vao_doesnt_store_element_array_buffer_binding;

        // Some drivers have gl state issues when drawing from shared contexts
        bool disable_shared_context_draws;

        // Some web browsers seem to immediately clear the default framebuffer when calling
        // glInvalidateFramebuffer with WebGL 2.0
        bool disable_invalidate_framebuffer;

        // Some drivers declare GL_EXT_texture_filter_anisotropic but don't support
        // calling glSamplerParameter() with GL_TEXTURE_MAX_ANISOTROPY_EXT
        bool texture_filter_anisotropic_broken_on_sampler;

        // Some drivers have issues when reading from a mip while writing to a different mip.
        // In the OpenGL ES 3.0 specification this is covered in section 4.4.3,
        // "Feedback Loops Between Textures and the Framebuffer".
        bool disable_feedback_loops;

        // Some drivers don't implement timer queries correctly
        bool dont_use_timer_query;

        // Some drivers can't blit from a sidecar renderbuffer into a layer of a texture array.
        // This technique is used for VSM with MSAA turned on.
        bool disable_blit_into_texture_array;

        // Some drivers incorrectly flatten the early exit condition in the EASU code, in which
        // case we need an alternative algorithm
        bool split_easu;

        // As of Android R some qualcomm drivers invalidate buffers for the whole render pass
        // even if glInvalidateFramebuffer() is called at the end of it.
        bool invalidate_end_only_if_invalidate_start;

        // GLES doesn't allow feedback loops even if writes are disabled. So take we the point of
        // view that this is generally forbidden. However, this restriction is lifted on desktop
        // GL and Vulkan and probably Metal.
        bool allow_read_only_ancillary_feedback_loop;

        // Some Adreno drivers crash in glDrawXXX() when there's an uninitialized uniform block,
        // even when the shader doesn't access it.
        bool enable_initialize_non_used_uniform_array;

        // Workarounds specific to PowerVR GPUs affecting shaders (currently, we lump them all
        // under one specialization constant).
        // - gl_InstanceID is invalid when used first in the vertex shader
        bool powervr_shader_workarounds;

        // On PowerVR destroying the destination of a glBlitFramebuffer operation is equivalent to
        // a glFinish. So we must delay the destruction until we know the GPU is finished.
        bool delay_fbo_destruction;

        // Mesa and Mozilla(web) sometimes clear the generic buffer binding when *another* buffer
        // is destroyed, if that other buffer is bound on an *indexed* buffer binding.
        bool rebind_buffer_after_deletion;

        // Force feature level 0. Typically used for low end ES3 devices with significant driver
        // bugs or performance issues.
        bool force_feature_level0;

        // Some browsers, such as Firefox on Mac, struggle with slow shader compile/link times when
        // creating programs for the default material, leading to startup stutters. This workaround
        // prevents these stutters by not precaching depth variants of the default material for
        // those particular browsers.
        bool disable_depth_precache_for_default_material;

        // On llvmpipe (mesa), enabling framebuffer fetch causes a crash in draw2
        //   'OpenGL error 0x502 (GL_INVALID_OPERATION) in "draw2" at line 4389'
        // This coincides with the use of framebuffer fetch (ColorGradingAsSubpass). We disable
        // framebuffer fetch in the case of llvmpipe.
        // Some Mali drivers also have problems with this (b/445721121)
        bool disable_framebuffer_fetch_extension;

    } bugs = {};

    struct Procs {
        void (* bindVertexArray)(GLuint array);
        void (* deleteVertexArrays)(GLsizei n, const GLuint* arrays);
        void (* genVertexArrays)(GLsizei n, GLuint* arrays);

        void (* genQueries)(GLsizei n, GLuint* ids);
        void (* deleteQueries)(GLsizei n, const GLuint* ids);
        void (* beginQuery)(GLenum target, GLuint id);
        void (* endQuery)(GLenum target);
        void (* getQueryObjectuiv)(GLuint id, GLenum pname, GLuint* params);
        void (* getQueryObjectui64v)(GLuint id, GLenum pname, GLuint64* params);

        void (* invalidateFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum *attachments);

        void (* maxShaderCompilerThreadsKHR)(GLuint count);
    } procs{};

    // GL version info — immutable after construction
    GLint major = 0;
    GLint minor = 0;

    // GL info strings — immutable after construction
    char const* vendor = nullptr;
    char const* renderer = nullptr;
    char const* version = nullptr;
    char const* shader = nullptr;

private:
    OpenGLPlatform& mPlatform;
    ShaderModel mShaderModel = ShaderModel::MOBILE;
    FeatureLevel mFeatureLevel = FeatureLevel::FEATURE_LEVEL_1;

    Platform::DriverConfig const mDriverConfig;

    bool mGpuTimeSupported = false;

    const std::array<std::tuple<bool const&, char const*, char const*>, sizeof(bugs)> mBugDatabase{{
            {   bugs.disable_glFlush,
                    "disable_glFlush",
                    ""},
            {   bugs.vao_doesnt_store_element_array_buffer_binding,
                    "vao_doesnt_store_element_array_buffer_binding",
                    ""},
            {   bugs.disable_shared_context_draws,
                    "disable_shared_context_draws",
                    ""},
            {   bugs.disable_invalidate_framebuffer,
                    "disable_invalidate_framebuffer",
                    ""},
            {   bugs.texture_filter_anisotropic_broken_on_sampler,
                    "texture_filter_anisotropic_broken_on_sampler",
                    ""},
            {   bugs.disable_feedback_loops,
                    "disable_feedback_loops",
                    ""},
            {   bugs.dont_use_timer_query,
                    "dont_use_timer_query",
                    ""},
            {   bugs.disable_blit_into_texture_array,
                    "disable_blit_into_texture_array",
                    ""},
            {   bugs.split_easu,
                    "split_easu",
                    ""},
            {   bugs.invalidate_end_only_if_invalidate_start,
                    "invalidate_end_only_if_invalidate_start",
                    ""},
            {   bugs.allow_read_only_ancillary_feedback_loop,
                    "allow_read_only_ancillary_feedback_loop",
                    ""},
            {   bugs.enable_initialize_non_used_uniform_array,
                    "enable_initialize_non_used_uniform_array",
                    ""},
            {   bugs.powervr_shader_workarounds,
                    "powervr_shader_workarounds",
                    ""},
            {   bugs.delay_fbo_destruction,
                    "delay_fbo_destruction",
                    ""},
            {   bugs.rebind_buffer_after_deletion,
                    "rebind_buffer_after_deletion",
                    ""},
            {   bugs.force_feature_level0,
                    "force_feature_level0",
                    ""},
            {   bugs.disable_depth_precache_for_default_material,
                    "disable_depth_precache_for_default_material",
                    ""},
            {   bugs.disable_framebuffer_fetch_extension,
                    "disable_framebuffer_fetch_extension",
                    ""},
    }};

    // this is chosen to minimize code size
#if defined(BACKEND_OPENGL_VERSION_GLES)
    static void initExtensionsGLES(Extensions* ext, GLint major, GLint minor) noexcept;
#endif
#if defined(BACKEND_OPENGL_VERSION_GL)
    static void initExtensionsGL(Extensions* ext, GLint major, GLint minor) noexcept;
#endif

    static void initExtensions(Extensions* ext, GLint major, GLint minor) noexcept {
#if defined(BACKEND_OPENGL_VERSION_GLES)
        initExtensionsGLES(ext, major, minor);
#endif
#if defined(BACKEND_OPENGL_VERSION_GL)
        initExtensionsGL(ext, major, minor);
#endif
    }

    static void initBugs(Bugs* bugs, Extensions const& exts,
            GLint major, GLint minor,
            char const* vendor,
            char const* renderer,
            char const* version,
            char const* shader
    );

    static void initProcs(Procs* procs,
            Extensions const& exts, GLint major, GLint minor) noexcept;

    static void initWorkarounds(Bugs const& bugs, Extensions* ext);

    static FeatureLevel resolveFeatureLevel(GLint major, GLint minor,
            Extensions const& exts,
            Gets const& gets,
            Bugs const& bugs) noexcept;
};

} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_OPENGLCONTEXT_H
