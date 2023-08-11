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

#include <math/vec4.h>

#include <utils/CString.h>
#include <utils/debug.h>

#include <backend/Handle.h>

#include "GLUtils.h"

#include <array>
#include <set>
#include <tuple>
#include <utility>

namespace filament::backend {

class OpenGLPlatform;

class OpenGLContext {
public:
    static constexpr const size_t MAX_TEXTURE_UNIT_COUNT = MAX_SAMPLER_COUNT;
    static constexpr const size_t DUMMY_TEXTURE_BINDING = 7; // highest binding guaranteed to work with ES2
    static constexpr const size_t MAX_BUFFER_BINDINGS = 32;
    typedef math::details::TVec4<GLint> vec4gli;
    typedef math::details::TVec2<GLclampf> vec2glf;

    struct RenderPrimitive {
        static_assert(MAX_VERTEX_ATTRIBUTE_COUNT <= 16);

        GLuint vao = 0;                                         // 4
        GLuint elementArray = 0;                                // 4
        utils::bitset<uint16_t> vertexAttribArray;              // 2

        // If this version number does not match vertexBufferWithObjects->bufferObjectsVersion,
        // then the VAO needs to be updated.
        uint8_t vertexBufferVersion = 0;                        // 1
        uint8_t indicesSize = 0;                                // 1

        // The optional 32-bit handle to a GLVertexBuffer is necessary only if the referenced
        // VertexBuffer supports buffer objects. If this is zero, then the VBO handles array is
        // immutable.
        Handle<HwVertexBuffer> vertexBufferWithObjects = {};    // 4

        GLenum getIndicesType() const noexcept {
            return indicesSize == 4 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
        }
    } gl;

    static bool queryOpenGLVersion(GLint* major, GLint* minor) noexcept;

    OpenGLContext() noexcept;

    template<int MAJOR, int MINOR>
    inline bool isAtLeastGL() const noexcept {
#ifdef BACKEND_OPENGL_VERSION_GL
        return state.major > MAJOR || (state.major == MAJOR && state.minor >= MINOR);
#else
        return false;
#endif
    }

    template<int MAJOR, int MINOR>
    inline bool isAtLeastGLES() const noexcept {
#ifdef BACKEND_OPENGL_VERSION_GLES
        return state.major > MAJOR || (state.major == MAJOR && state.minor >= MINOR);
#else
        return false;
#endif
    }

    inline bool isES2() const noexcept {
#if defined(BACKEND_OPENGL_VERSION_GLES) && !defined(IOS)
#   ifndef BACKEND_OPENGL_LEVEL_GLES30
            return true;
#   else
            return state.major == 2;
#   endif
#else
        return false;
#endif
    }

    constexpr static inline size_t getIndexForTextureTarget(GLuint target) noexcept;
    constexpr        inline size_t getIndexForCap(GLenum cap) noexcept;
    constexpr static inline size_t getIndexForBufferTarget(GLenum target) noexcept;

    ShaderModel getShaderModel() const noexcept { return mShaderModel; }

    void resetState() noexcept;

    inline void useProgram(GLuint program) noexcept;

          void pixelStore(GLenum, GLint) noexcept;
    inline void activeTexture(GLuint unit) noexcept;
    inline void bindTexture(GLuint unit, GLuint target, GLuint texId, size_t targetIndex) noexcept;
    inline void bindTexture(GLuint unit, GLuint target, GLuint texId) noexcept;

           void unbindTexture(GLenum target, GLuint id) noexcept;
    inline void bindVertexArray(RenderPrimitive const* p) noexcept;
    inline void bindSampler(GLuint unit, GLuint sampler) noexcept;
           void unbindSampler(GLuint sampler) noexcept;

           void bindBuffer(GLenum target, GLuint buffer) noexcept;
    inline void bindBufferRange(GLenum target, GLuint index, GLuint buffer,
            GLintptr offset, GLsizeiptr size) noexcept;

    inline void bindFramebuffer(GLenum target, GLuint buffer) noexcept;

    inline void enableVertexAttribArray(GLuint index) noexcept;
    inline void disableVertexAttribArray(GLuint index) noexcept;
    inline void enable(GLenum cap) noexcept;
    inline void disable(GLenum cap) noexcept;
    inline void frontFace(GLenum mode) noexcept;
    inline void cullFace(GLenum mode) noexcept;
    inline void blendEquation(GLenum modeRGB, GLenum modeA) noexcept;
    inline void blendFunction(GLenum srcRGB, GLenum srcA, GLenum dstRGB, GLenum dstA) noexcept;
    inline void colorMask(GLboolean flag) noexcept;
    inline void depthMask(GLboolean flag) noexcept;
    inline void depthFunc(GLenum func) noexcept;
    inline void stencilFuncSeparate(GLenum funcFront, GLint refFront, GLuint maskFront,
            GLenum funcBack, GLint refBack, GLuint maskBack) noexcept;
    inline void stencilOpSeparate(GLenum sfailFront, GLenum dpfailFront, GLenum dppassFront,
            GLenum sfailBack, GLenum dpfailBack, GLenum dppassBack) noexcept;
    inline void stencilMaskSeparate(GLuint maskFront, GLuint maskBack) noexcept;
    inline void polygonOffset(GLfloat factor, GLfloat units) noexcept;

    inline void setScissor(GLint left, GLint bottom, GLsizei width, GLsizei height) noexcept;
    inline void viewport(GLint left, GLint bottom, GLsizei width, GLsizei height) noexcept;
    inline void depthRange(GLclampf near, GLclampf far) noexcept;

    void deleteBuffers(GLsizei n, const GLuint* buffers, GLenum target) noexcept;
    void deleteVertexArrays(GLsizei n, const GLuint* arrays) noexcept;

    // glGet*() values
    struct {
        GLfloat max_anisotropy;
        GLint max_draw_buffers;
        GLint max_renderbuffer_size;
        GLint max_samples;
        GLint max_uniform_block_size;
        GLint max_texture_image_units;
        GLint max_combined_texture_image_units;
        GLint max_transform_feedback_separate_attribs;
        GLint max_uniform_buffer_bindings; 
        GLint uniform_buffer_offset_alignment;
    } gets = {};

    // features supported by this version of GL or GLES
    struct {
        bool multisample_texture;
    } features = {};

    // supported extensions detected at runtime
    struct {
        bool APPLE_color_buffer_packed_float;
        bool ARB_shading_language_packing;
        bool EXT_clip_control;
        bool EXT_clip_cull_distance;
        bool EXT_color_buffer_float;
        bool EXT_color_buffer_half_float;
        bool EXT_debug_marker;
        bool EXT_disjoint_timer_query;
        bool EXT_discard_framebuffer;
        bool EXT_multisampled_render_to_texture2;
        bool EXT_multisampled_render_to_texture;
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
        bool OES_depth_texture;
        bool OES_depth24;
        bool OES_packed_depth_stencil;
        bool OES_EGL_image_external_essl3;
        bool OES_rgb8_rgba8;
        bool OES_standard_derivatives;
        bool OES_texture_npot;
        bool OES_vertex_array_object;
        bool WEBGL_compressed_texture_etc;
        bool WEBGL_compressed_texture_s3tc;
        bool WEBGL_compressed_texture_s3tc_srgb;
    } ext = {};

    struct {
        // Some drivers have issues with UBOs in the fragment shader when
        // glFlush() is called between draw calls.
        bool disable_glFlush;

        // Some drivers seem to not store the GL_ELEMENT_ARRAY_BUFFER binding
        // in the VAO state.
        bool vao_doesnt_store_element_array_buffer_binding;

        // Some drivers have gl state issues when drawing from shared contexts
        bool disable_shared_context_draws;

        // Some drivers require the GL_TEXTURE_EXTERNAL_OES target to be bound when
        // the texture image changes, even if it's already bound to that texture
        bool texture_external_needs_rebind;

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

        // The driver has some threads pinned, and we can't easily know on which core, it can hurt
        // performance more if we end-up pinned on the same one.
        bool disable_thread_affinity;

    } bugs = {};

    // state getters -- as needed.
    vec4gli const& getViewport() const { return state.window.viewport; }

    // function to handle state changes we don't control
    void updateTexImage(GLenum target, GLuint id) noexcept {
        const size_t index = getIndexForTextureTarget(target);
        state.textures.units[state.textures.active].targets[index].texture_id = id;
    }
    void resetProgram() noexcept { state.program.use = 0; }

    FeatureLevel getFeatureLevel() const noexcept { return mFeatureLevel; }

    // Try to keep the State structure sorted by data-access patterns
    struct State {
        GLint major = 0;
        GLint minor = 0;

        char const* vendor = nullptr;
        char const* renderer = nullptr;
        char const* version = nullptr;
        char const* shader = nullptr;

        GLuint draw_fbo = 0;
        GLuint read_fbo = 0;

        struct {
            GLuint use = 0;
        } program;

        struct {
            RenderPrimitive* p = nullptr;
        } vao;

        struct {
            GLenum frontFace            = GL_CCW;
            GLenum cullFace             = GL_BACK;
            GLenum blendEquationRGB     = GL_FUNC_ADD;
            GLenum blendEquationA       = GL_FUNC_ADD;
            GLenum blendFunctionSrcRGB  = GL_ONE;
            GLenum blendFunctionSrcA    = GL_ONE;
            GLenum blendFunctionDstRGB  = GL_ZERO;
            GLenum blendFunctionDstA    = GL_ZERO;
            GLboolean colorMask         = GL_TRUE;
            GLboolean depthMask         = GL_TRUE;
            GLenum depthFunc            = GL_LESS;
        } raster;

        struct {
            struct StencilFunc {
                GLenum func             = GL_ALWAYS;
                GLint ref               = 0;
                GLuint mask             = ~GLuint(0);
                bool operator != (StencilFunc const& rhs) const noexcept {
                    return func != rhs.func || ref != rhs.ref || mask != rhs.mask;
                }
            };
            struct StencilOp {
                GLenum sfail            = GL_KEEP;
                GLenum dpfail           = GL_KEEP;
                GLenum dppass           = GL_KEEP;
                bool operator != (StencilOp const& rhs) const noexcept {
                    return sfail != rhs.sfail || dpfail != rhs.dpfail || dppass != rhs.dppass;
                }
            };
            struct {
                StencilFunc func;
                StencilOp op;
                GLuint stencilMask      = ~GLuint(0);
            } front, back;
        } stencil;

        struct PolygonOffset {
            GLfloat factor = 0;
            GLfloat units = 0;
            bool operator != (PolygonOffset const& rhs) const noexcept {
                return factor != rhs.factor || units != rhs.units;
            }
        } polygonOffset;

        struct {
            utils::bitset32 caps;
        } enables;

        struct {
            struct {
                struct {
                    GLuint name = 0;
                    GLintptr offset = 0;
                    GLsizeiptr size = 0;
                } buffers[MAX_BUFFER_BINDINGS];
            } targets[3];   // there are only 3 indexed buffer targets
            GLuint genericBinding[7] = {};
        } buffers;

        struct {
            GLuint active = 0;      // zero-based
            struct {
                GLuint sampler = 0;
                struct {
                    GLuint texture_id = 0;
                } targets[7];  // this must match getIndexForTextureTarget()
            } units[MAX_TEXTURE_UNIT_COUNT];
        } textures;

        struct {
            GLint row_length = 0;
            GLint alignment = 4;
        } unpack;

        struct {
            GLint alignment = 4;
        } pack;

        struct {
            vec4gli scissor { 0 };
            vec4gli viewport { 0 };
            vec2glf depthRange { 0.0f, 1.0f };
        } window;
    } state;

    struct {
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

private:
    ShaderModel mShaderModel = ShaderModel::MOBILE;
    FeatureLevel mFeatureLevel = FeatureLevel::FEATURE_LEVEL_1;

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
            {   bugs.texture_external_needs_rebind,
                    "texture_external_needs_rebind",
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
            {   bugs.disable_thread_affinity,
                    "disable_thread_affinity",
                    ""},
    }};

    RenderPrimitive mDefaultVAO;

    // this is chosen to minimize code size
#if defined(BACKEND_OPENGL_VERSION_GLES)
    void initExtensionsGLES() noexcept;
#endif
#if defined(BACKEND_OPENGL_VERSION_GL)
    void initExtensionsGL() noexcept;
#endif

    template <typename T, typename F>
    static inline void update_state(T& state, T const& expected, F functor, bool force = false) noexcept {
        if (UTILS_UNLIKELY(force || state != expected)) {
            state = expected;
            functor();
        }
    }

    void setDefaultState() noexcept;

    static constexpr const size_t TEXTURE_TARGET_COUNT =
            sizeof(state.textures.units[0].targets) / sizeof(state.textures.units[0].targets[0]);

};

// ------------------------------------------------------------------------------------------------

constexpr size_t OpenGLContext::getIndexForTextureTarget(GLuint target) noexcept {
    // this must match state.textures[].targets[]
    switch (target) {
        case GL_TEXTURE_2D:                     return 0;
        case GL_TEXTURE_2D_ARRAY:               return 1;
        case GL_TEXTURE_CUBE_MAP:               return 2;
#if defined(BACKEND_OPENGL_LEVEL_GLES31)
        case GL_TEXTURE_2D_MULTISAMPLE:         return 3;
#endif
        case GL_TEXTURE_EXTERNAL_OES:           return 4;
        case GL_TEXTURE_3D:                     return 5;
        case GL_TEXTURE_CUBE_MAP_ARRAY:         return 6;
        default:
            return 0;
    }
}

constexpr size_t OpenGLContext::getIndexForCap(GLenum cap) noexcept { //NOLINT
    size_t index = 0;
    switch (cap) {
        case GL_BLEND:                          index =  0; break;
        case GL_CULL_FACE:                      index =  1; break;
        case GL_SCISSOR_TEST:                   index =  2; break;
        case GL_DEPTH_TEST:                     index =  3; break;
        case GL_STENCIL_TEST:                   index =  4; break;
        case GL_DITHER:                         index =  5; break;
        case GL_SAMPLE_ALPHA_TO_COVERAGE:       index =  6; break;
        case GL_SAMPLE_COVERAGE:                index =  7; break;
        case GL_POLYGON_OFFSET_FILL:            index =  8; break;
#ifdef GL_ARB_seamless_cube_map
        case GL_TEXTURE_CUBE_MAP_SEAMLESS:      index =  9; break;
#endif
#ifdef BACKEND_OPENGL_VERSION_GL
        case GL_PROGRAM_POINT_SIZE:             index = 10; break;
#endif
        default: break;
    }
    assert_invariant(index < state.enables.caps.size());
    return index;
}

constexpr size_t OpenGLContext::getIndexForBufferTarget(GLenum target) noexcept {
    size_t index = 0;
    switch (target) {
        // The indexed buffers MUST be first in this list (those usable with bindBufferRange)
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        case GL_UNIFORM_BUFFER:             index = 0; break;
        case GL_TRANSFORM_FEEDBACK_BUFFER:  index = 1; break;
#if defined(BACKEND_OPENGL_LEVEL_GLES31)
        case GL_SHADER_STORAGE_BUFFER:      index = 2; break;
#endif
#endif
        case GL_ARRAY_BUFFER:               index = 3; break;
        case GL_ELEMENT_ARRAY_BUFFER:       index = 4; break;
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        case GL_PIXEL_PACK_BUFFER:          index = 5; break;
        case GL_PIXEL_UNPACK_BUFFER:        index = 6; break;
#endif
        default: break;
    }
    assert_invariant(index < sizeof(state.buffers.genericBinding)/sizeof(state.buffers.genericBinding[0])); // NOLINT(misc-redundant-expression)
    return index;
}

// ------------------------------------------------------------------------------------------------

void OpenGLContext::activeTexture(GLuint unit) noexcept {
    assert_invariant(unit < MAX_TEXTURE_UNIT_COUNT);
    update_state(state.textures.active, unit, [&]() {
        glActiveTexture(GL_TEXTURE0 + unit);
    });
}

void OpenGLContext::bindSampler(GLuint unit, GLuint sampler) noexcept {
    assert_invariant(unit < MAX_TEXTURE_UNIT_COUNT);
    assert_invariant(state.major > 2);
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    update_state(state.textures.units[unit].sampler, sampler, [&]() {
        glBindSampler(unit, sampler);
    });
#endif
}

void OpenGLContext::setScissor(GLint left, GLint bottom, GLsizei width, GLsizei height) noexcept {
    vec4gli const scissor(left, bottom, width, height);
    update_state(state.window.scissor, scissor, [&]() {
        glScissor(left, bottom, width, height);
    });
}

void OpenGLContext::viewport(GLint left, GLint bottom, GLsizei width, GLsizei height) noexcept {
    vec4gli const viewport(left, bottom, width, height);
    update_state(state.window.viewport, viewport, [&]() {
        glViewport(left, bottom, width, height);
    });
}

void OpenGLContext::depthRange(GLclampf near, GLclampf far) noexcept {
    vec2glf const depthRange(near, far);
    update_state(state.window.depthRange, depthRange, [&]() {
        glDepthRangef(near, far);
    });
}

void OpenGLContext::bindVertexArray(RenderPrimitive const* p) noexcept {
    RenderPrimitive* vao = p ? const_cast<RenderPrimitive *>(p) : &mDefaultVAO;
    update_state(state.vao.p, vao, [&]() {
        procs.bindVertexArray(vao->vao);
        // update GL_ELEMENT_ARRAY_BUFFER, which is updated by glBindVertexArray
        size_t const targetIndex = getIndexForBufferTarget(GL_ELEMENT_ARRAY_BUFFER);
        state.buffers.genericBinding[targetIndex] = vao->elementArray;
        if (UTILS_UNLIKELY(bugs.vao_doesnt_store_element_array_buffer_binding)) {
            // This shouldn't be needed, but it looks like some drivers don't do the implicit
            // glBindBuffer().
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao->elementArray);
        }
    });
}

void OpenGLContext::bindBufferRange(GLenum target, GLuint index, GLuint buffer,
        GLintptr offset, GLsizeiptr size) noexcept {
    assert_invariant(state.major > 2);

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
#   ifdef BACKEND_OPENGL_LEVEL_GLES31
        assert_invariant(false
                 || target == GL_UNIFORM_BUFFER
                 || target == GL_TRANSFORM_FEEDBACK_BUFFER
                 || target == GL_SHADER_STORAGE_BUFFER);
#   else
        assert_invariant(false
                || target == GL_UNIFORM_BUFFER
                || target == GL_TRANSFORM_FEEDBACK_BUFFER);
#   endif
    size_t const targetIndex = getIndexForBufferTarget(target);
    // this ALSO sets the generic binding
    assert_invariant(targetIndex < sizeof(state.buffers.targets) / sizeof(*state.buffers.targets));
    if (   state.buffers.targets[targetIndex].buffers[index].name != buffer
           || state.buffers.targets[targetIndex].buffers[index].offset != offset
           || state.buffers.targets[targetIndex].buffers[index].size != size) {
        state.buffers.targets[targetIndex].buffers[index].name = buffer;
        state.buffers.targets[targetIndex].buffers[index].offset = offset;
        state.buffers.targets[targetIndex].buffers[index].size = size;
        state.buffers.genericBinding[targetIndex] = buffer;
        glBindBufferRange(target, index, buffer, offset, size);
    }
#endif
}

void OpenGLContext::bindFramebuffer(GLenum target, GLuint buffer) noexcept {
    switch (target) {
        case GL_FRAMEBUFFER:
            if (state.draw_fbo != buffer || state.read_fbo != buffer) {
                state.draw_fbo = state.read_fbo = buffer;
                glBindFramebuffer(target, buffer);
            }
            break;
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        case GL_DRAW_FRAMEBUFFER:
            if (state.draw_fbo != buffer) {
                state.draw_fbo = buffer;
                glBindFramebuffer(target, buffer);
            }
            break;
        case GL_READ_FRAMEBUFFER:
            if (state.read_fbo != buffer) {
                state.read_fbo = buffer;
                glBindFramebuffer(target, buffer);
            }
            break;
#endif
        default:
            break;
    }
}

void OpenGLContext::bindTexture(GLuint unit, GLuint target, GLuint texId, size_t targetIndex) noexcept {
    assert_invariant(targetIndex == getIndexForTextureTarget(target));
    assert_invariant(targetIndex < TEXTURE_TARGET_COUNT);
    update_state(state.textures.units[unit].targets[targetIndex].texture_id, texId, [&]() {
        activeTexture(unit);
        glBindTexture(target, texId);
    }, (target == GL_TEXTURE_EXTERNAL_OES) && bugs.texture_external_needs_rebind);
}

void OpenGLContext::bindTexture(GLuint unit, GLuint target, GLuint texId) noexcept {
    bindTexture(unit, target, texId, getIndexForTextureTarget(target));
}

void OpenGLContext::useProgram(GLuint program) noexcept {
    update_state(state.program.use, program, [&]() {
        glUseProgram(program);
    });
}

void OpenGLContext::enableVertexAttribArray(GLuint index) noexcept {
    assert_invariant(state.vao.p);
    assert_invariant(index < state.vao.p->vertexAttribArray.size());
    if (UTILS_UNLIKELY(!state.vao.p->vertexAttribArray[index])) {
        state.vao.p->vertexAttribArray.set(index);
        glEnableVertexAttribArray(index);
    }
}

void OpenGLContext::disableVertexAttribArray(GLuint index) noexcept {
    assert_invariant(state.vao.p);
    assert_invariant(index < state.vao.p->vertexAttribArray.size());
    if (UTILS_UNLIKELY(state.vao.p->vertexAttribArray[index])) {
        state.vao.p->vertexAttribArray.unset(index);
        glDisableVertexAttribArray(index);
    }
}

void OpenGLContext::enable(GLenum cap) noexcept {
    size_t const index = getIndexForCap(cap);
    if (UTILS_UNLIKELY(!state.enables.caps[index])) {
        state.enables.caps.set(index);
        glEnable(cap);
    }
}

void OpenGLContext::disable(GLenum cap) noexcept {
    size_t const index = getIndexForCap(cap);
    if (UTILS_UNLIKELY(state.enables.caps[index])) {
        state.enables.caps.unset(index);
        glDisable(cap);
    }
}

void OpenGLContext::frontFace(GLenum mode) noexcept {
    update_state(state.raster.frontFace, mode, [&]() {
        glFrontFace(mode);
    });
}

void OpenGLContext::cullFace(GLenum mode) noexcept {
    update_state(state.raster.cullFace, mode, [&]() {
        glCullFace(mode);
    });
}

void OpenGLContext::blendEquation(GLenum modeRGB, GLenum modeA) noexcept {
    if (UTILS_UNLIKELY(
            state.raster.blendEquationRGB != modeRGB || state.raster.blendEquationA != modeA)) {
        state.raster.blendEquationRGB = modeRGB;
        state.raster.blendEquationA   = modeA;
        glBlendEquationSeparate(modeRGB, modeA);
    }
}

void OpenGLContext::blendFunction(GLenum srcRGB, GLenum srcA, GLenum dstRGB, GLenum dstA) noexcept {
    if (UTILS_UNLIKELY(
            state.raster.blendFunctionSrcRGB != srcRGB ||
            state.raster.blendFunctionSrcA != srcA ||
            state.raster.blendFunctionDstRGB != dstRGB ||
            state.raster.blendFunctionDstA != dstA)) {
        state.raster.blendFunctionSrcRGB = srcRGB;
        state.raster.blendFunctionSrcA = srcA;
        state.raster.blendFunctionDstRGB = dstRGB;
        state.raster.blendFunctionDstA = dstA;
        glBlendFuncSeparate(srcRGB, dstRGB, srcA, dstA);
    }
}

void OpenGLContext::colorMask(GLboolean flag) noexcept {
    update_state(state.raster.colorMask, flag, [&]() {
        glColorMask(flag, flag, flag, flag);
    });
}
void OpenGLContext::depthMask(GLboolean flag) noexcept {
    update_state(state.raster.depthMask, flag, [&]() {
        glDepthMask(flag);
    });
}

void OpenGLContext::depthFunc(GLenum func) noexcept {
    update_state(state.raster.depthFunc, func, [&]() {
        glDepthFunc(func);
    });
}

void OpenGLContext::stencilFuncSeparate(GLenum funcFront, GLint refFront, GLuint maskFront,
        GLenum funcBack, GLint refBack, GLuint maskBack) noexcept {
    update_state(state.stencil.front.func, {funcFront, refFront, maskFront}, [&]() {
        glStencilFuncSeparate(GL_FRONT, funcFront, refFront, maskFront);
    });
    update_state(state.stencil.back.func, {funcBack, refBack, maskBack}, [&]() {
        glStencilFuncSeparate(GL_BACK, funcBack, refBack, maskBack);
    });
}

void OpenGLContext::stencilOpSeparate(GLenum sfailFront, GLenum dpfailFront, GLenum dppassFront,
        GLenum sfailBack, GLenum dpfailBack, GLenum dppassBack) noexcept {
    update_state(state.stencil.front.op, {sfailFront, dpfailFront, dppassFront}, [&]() {
        glStencilOpSeparate(GL_FRONT, sfailFront, dpfailFront, dppassFront);
    });
    update_state(state.stencil.back.op, {sfailBack, dpfailBack, dppassBack}, [&]() {
        glStencilOpSeparate(GL_BACK, sfailBack, dpfailBack, dppassBack);
    });
}

void OpenGLContext::stencilMaskSeparate(GLuint maskFront, GLuint maskBack) noexcept {
    update_state(state.stencil.front.stencilMask, maskFront, [&]() {
        glStencilMaskSeparate(GL_FRONT, maskFront);
    });
    update_state(state.stencil.back.stencilMask, maskBack, [&]() {
        glStencilMaskSeparate(GL_BACK, maskBack);
    });
}

void OpenGLContext::polygonOffset(GLfloat factor, GLfloat units) noexcept {
    update_state(state.polygonOffset, { factor, units }, [&]() {
        if (factor != 0 || units != 0) {
            glPolygonOffset(factor, units);
            enable(GL_POLYGON_OFFSET_FILL);
        } else {
            disable(GL_POLYGON_OFFSET_FILL);
        }
    });
}

} // namespace filament

#endif //TNT_FILAMENT_BACKEND_OPENGLCONTEXT_H
