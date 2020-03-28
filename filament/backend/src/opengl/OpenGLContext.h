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

#include "GLUtils.h"

#include <set>

namespace filament {

class OpenGLContext {
public:
    static constexpr const size_t MAX_TEXTURE_UNIT_COUNT = 16;   // All mobile GPUs as of 2016
    static constexpr const size_t MAX_BUFFER_BINDINGS = 32;
    typedef math::details::TVec4<GLint> vec4gli;

    struct RenderPrimitive {
        GLuint vao = 0;
        GLenum indicesType = GL_UNSIGNED_INT;
        GLuint elementArray = 0;
        utils::bitset32 vertexAttribArray;
    } gl;

    OpenGLContext() noexcept;

    // this is chosen to minimize code size
    using ExtentionSet = std::set<utils::StaticString>;
    static bool hasExtension(ExtentionSet const& exts, utils::StaticString ext) noexcept;
    void initExtensionsGLES(GLint major, GLint minor, ExtentionSet const& extensionsMap);
    void initExtensionsGL(GLint major, GLint minor, ExtentionSet const& extensionsMap);

    constexpr static inline size_t getIndexForTextureTarget(GLuint target) noexcept;
    constexpr        inline size_t getIndexForCap(GLenum cap) noexcept;
    constexpr static inline size_t getIndexForBufferTarget(GLenum target) noexcept;

    backend::ShaderModel getShaderModel() const noexcept { return mShaderModel; }


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
    inline void polygonOffset(GLfloat factor, GLfloat units) noexcept;
    inline void beginQuery(GLenum target, GLuint query) noexcept;
    inline void endQuery(GLenum target) noexcept;
    inline GLuint getQuery(GLenum target) noexcept;

    inline void setScissor(GLint left, GLint bottom, GLsizei width, GLsizei height) noexcept;
    inline void viewport(GLint left, GLint bottom, GLsizei width, GLsizei height) noexcept;

    void deleteBuffers(GLsizei n, const GLuint* buffers, GLenum target) noexcept;
    void deleteVextexArrays(GLsizei n, const GLuint* arrays) noexcept;

    // glGet*() values
    struct {
        GLint max_renderbuffer_size = 0;
        GLint max_uniform_block_size = 0;
        GLint uniform_buffer_offset_alignment = 256;
        GLfloat maxAnisotropy = 0.0f;
    } gets;

    // features supported by this version of GL or GLES
    struct {
        bool multisample_texture = false;
    } features;

    // supported extensions detected at runtime
    struct {
        bool texture_compression_s3tc = false;
        bool texture_compression_etc2 = false;
        bool texture_filter_anisotropic = false;
        bool QCOM_tiled_rendering = false;
        bool OES_EGL_image_external_essl3 = false;
        bool EXT_debug_marker = false;
        bool EXT_color_buffer_half_float = false;
        bool EXT_color_buffer_float = false;
        bool APPLE_color_buffer_packed_float = false;
        bool EXT_multisampled_render_to_texture = false;
        bool KHR_debug = false;
        bool EXT_texture_sRGB = false;
        bool EXT_texture_compression_s3tc_srgb = false;
        bool EXT_disjoint_timer_query = false;
    } ext;

    struct {
        // Some drivers have issues with UBOs in the fragment shader when
        // glFlush() is called between draw calls.
        bool disable_glFlush = false;

        // Some drivers seem to not store the GL_ELEMENT_ARRAY_BUFFER binding
        // in the VAO state.
        bool vao_doesnt_store_element_array_buffer_binding = false;

        // Some drivers have gl state issues when drawing from shared contexts
        bool disable_shared_context_draws = false;

        // Some drivers require the GL_TEXTURE_EXTERNAL_OES target to be bound when
        // the texture image changes, even if it's already bound to that texture
        bool texture_external_needs_rebind = false;

        // Some web browsers seem to immediately clear the default framebuffer when calling
        // glInvalidateFramebuffer with WebGL 2.0
        bool disable_invalidate_framebuffer = false;

        // Some drivers declare GL_EXT_texture_filter_anisotropic but don't support
        // calling glSamplerParameter() with GL_TEXTURE_MAX_ANISOTROPY_EXT
        bool disable_texture_filter_anisotropic = false;

        // Some drivers don't implement timer queries correctly
        bool dont_use_timer_query = false;
    } bugs;

    // state getters -- as needed.
    GLuint getDrawFbo() const noexcept { return state.draw_fbo; }
    vec4gli const& getViewport() const { return state.window.viewport; }

    // function to handle state changes we don't control
    void updateTexImage(GLenum target, GLuint id) noexcept {
        const size_t index = getIndexForTextureTarget(target);
        state.textures.units[state.textures.active].targets[index].texture_id = id;
    }
    void resetProgram() noexcept { state.program.use = 0; }

private:
    backend::ShaderModel mShaderModel;

    // Try to keep the State structure sorted by data-access patterns
    struct State {
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

        struct PolygonOffset {
            GLfloat factor = 0;
            GLfloat units = 0;
            bool operator != (PolygonOffset const& rhs) noexcept {
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
            } targets[2];   // there are only 2 indexed buffer target (uniform and transform feedback)
            GLuint genericBinding[8] = { 0 };
        } buffers;

        struct {
            GLuint active = 0;      // zero-based
            struct {
                GLuint sampler = 0;
                struct {
                    GLuint texture_id = 0;
                } targets[5];
            } units[MAX_TEXTURE_UNIT_COUNT];
        } textures;

        struct {
            GLint row_length = 0;
            GLint alignment = 4;
            GLint skip_pixels = 0;
            GLint skip_row = 0;
        } unpack;

        struct {
            GLint row_length = 0;
            GLint alignment = 4;
            GLint skip_pixels = 0;
            GLint skip_row = 0;
        } pack;

        struct {
            vec4gli scissor { 0 };
            vec4gli viewport { 0 };
        } window;

        struct {
            GLuint timer = -1u;
        } queries;
    } state;

    RenderPrimitive mDefaultVAO;

    template <typename T, typename F>
    static inline void update_state(T& state, T const& expected, F functor, bool force = false) noexcept {
        if (UTILS_UNLIKELY(force || state != expected)) {
            state = expected;
            functor();
        }
    }

    static constexpr const size_t TEXTURE_TARGET_COUNT =
            sizeof(state.textures.units[0].targets) / sizeof(state.textures.units[0].targets[0]);

};

// ------------------------------------------------------------------------------------------------

constexpr size_t OpenGLContext::getIndexForTextureTarget(GLuint target) noexcept {
    switch (target) {
        case GL_TEXTURE_2D:             return 0;
        case GL_TEXTURE_2D_ARRAY:       return 1;
        case GL_TEXTURE_CUBE_MAP:       return 2;
        case GL_TEXTURE_2D_MULTISAMPLE: return 3;
        case GL_TEXTURE_EXTERNAL_OES:   return 4;
        default:                        return 0;
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
        case GL_PRIMITIVE_RESTART_FIXED_INDEX:  index =  9; break;
        case GL_RASTERIZER_DISCARD:             index = 10; break;
#ifdef GL_ARB_seamless_cube_map
            case GL_TEXTURE_CUBE_MAP_SEAMLESS:      index = 11; break;
#endif
#if GL41_HEADERS
            case GL_PROGRAM_POINT_SIZE:             index = 12; break;
#endif
        default: index = 13; break; // should never happen
    }
    assert(index < 13 && index < state.enables.caps.size());
    return index;
}

constexpr size_t OpenGLContext::getIndexForBufferTarget(GLenum target) noexcept {
    size_t index = 0;
    switch (target) {
        // The indexed buffers MUST be first in this list
        case GL_UNIFORM_BUFFER:             index = 0; break;
        case GL_TRANSFORM_FEEDBACK_BUFFER:  index = 1; break;

        case GL_ARRAY_BUFFER:               index = 2; break;
        case GL_COPY_READ_BUFFER:           index = 3; break;
        case GL_COPY_WRITE_BUFFER:          index = 4; break;
        case GL_ELEMENT_ARRAY_BUFFER:       index = 5; break;
        case GL_PIXEL_PACK_BUFFER:          index = 6; break;
        case GL_PIXEL_UNPACK_BUFFER:        index = 7; break;
        default: index = 8; break; // should never happen
    }
    assert(index < sizeof(state.buffers.genericBinding)/sizeof(state.buffers.genericBinding[0])); // NOLINT(misc-redundant-expression)
    return index;
}

// ------------------------------------------------------------------------------------------------

void OpenGLContext::activeTexture(GLuint unit) noexcept {
    assert(unit < MAX_TEXTURE_UNIT_COUNT);
    update_state(state.textures.active, unit, [&]() {
        glActiveTexture(GL_TEXTURE0 + unit);
    });
}

void OpenGLContext::bindSampler(GLuint unit, GLuint sampler) noexcept {
    assert(unit < MAX_TEXTURE_UNIT_COUNT);
    update_state(state.textures.units[unit].sampler, sampler, [&]() {
        glBindSampler(unit, sampler);
    });
}

void OpenGLContext::setScissor(GLint left, GLint bottom, GLsizei width, GLsizei height) noexcept {
    vec4gli scissor(left, bottom, width, height);
    update_state(state.window.scissor, scissor, [&]() {
        glScissor(left, bottom, width, height);
    });
}

void OpenGLContext::viewport(GLint left, GLint bottom, GLsizei width, GLsizei height) noexcept {
    vec4gli viewport(left, bottom, width, height);
    update_state(state.window.viewport, viewport, [&]() {
        glViewport(left, bottom, width, height);
    });
}

void OpenGLContext::bindVertexArray(RenderPrimitive const* p) noexcept {
    RenderPrimitive* vao = p ? const_cast<RenderPrimitive *>(p) : &mDefaultVAO;
    update_state(state.vao.p, vao, [&]() {
        glBindVertexArray(vao->vao);
        // update GL_ELEMENT_ARRAY_BUFFER, which is updated by glBindVertexArray
        size_t targetIndex = getIndexForBufferTarget(GL_ELEMENT_ARRAY_BUFFER);
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
    size_t targetIndex = getIndexForBufferTarget(target);
    assert(targetIndex <= 1); // sanity check

    // this ALSO sets the generic binding
    if (   state.buffers.targets[targetIndex].buffers[index].name != buffer
           || state.buffers.targets[targetIndex].buffers[index].offset != offset
           || state.buffers.targets[targetIndex].buffers[index].size != size) {
        state.buffers.targets[targetIndex].buffers[index].name = buffer;
        state.buffers.targets[targetIndex].buffers[index].offset = offset;
        state.buffers.targets[targetIndex].buffers[index].size = size;
        state.buffers.genericBinding[targetIndex] = buffer;
        glBindBufferRange(target, index, buffer, offset, size);
    }
}

void OpenGLContext::bindFramebuffer(GLenum target, GLuint buffer) noexcept {
    switch (target) {
        case GL_FRAMEBUFFER:
            if (state.draw_fbo != buffer || state.read_fbo != buffer) {
                state.draw_fbo = state.read_fbo = buffer;
                glBindFramebuffer(target, buffer);
            }
            break;
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
        default:
            break;
    }
}

void OpenGLContext::bindTexture(GLuint unit, GLuint target, GLuint texId, size_t targetIndex) noexcept {
    assert(targetIndex == getIndexForTextureTarget(target));
    assert(targetIndex < TEXTURE_TARGET_COUNT);
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
    assert(state.vao.p);
    assert(index < state.vao.p->vertexAttribArray.size());
    if (UTILS_UNLIKELY(!state.vao.p->vertexAttribArray[index])) {
        state.vao.p->vertexAttribArray.set(index);
        glEnableVertexAttribArray(index);
    }
}

void OpenGLContext::disableVertexAttribArray(GLuint index) noexcept {
    assert(state.vao.p);
    assert(index < state.vao.p->vertexAttribArray.size());
    if (UTILS_UNLIKELY(state.vao.p->vertexAttribArray[index])) {
        state.vao.p->vertexAttribArray.unset(index);
        glDisableVertexAttribArray(index);
    }
}

void OpenGLContext::enable(GLenum cap) noexcept {
    size_t index = getIndexForCap(cap);
    if (UTILS_UNLIKELY(!state.enables.caps[index])) {
        state.enables.caps.set(index);
        glEnable(cap);
    }
}

void OpenGLContext::disable(GLenum cap) noexcept {
    size_t index = getIndexForCap(cap);
    if (UTILS_UNLIKELY(state.enables.caps[index])) {
        state.enables.caps.unset(index);
        glDisable(cap);
    }
}

void OpenGLContext::frontFace(GLenum mode) noexcept {
    // WARNING: don't call this without updating mRasterState
    update_state(state.raster.frontFace, mode, [&]() {
        glFrontFace(mode);
    });
}

void OpenGLContext::cullFace(GLenum mode) noexcept {
    // WARNING: don't call this without updating mRasterState
    update_state(state.raster.cullFace, mode, [&]() {
        glCullFace(mode);
    });
}

void OpenGLContext::blendEquation(GLenum modeRGB, GLenum modeA) noexcept {
    // WARNING: don't call this without updating mRasterState
    if (UTILS_UNLIKELY(
            state.raster.blendEquationRGB != modeRGB || state.raster.blendEquationA != modeA)) {
        state.raster.blendEquationRGB = modeRGB;
        state.raster.blendEquationA   = modeA;
        glBlendEquationSeparate(modeRGB, modeA);
    }
}

void OpenGLContext::blendFunction(GLenum srcRGB, GLenum srcA, GLenum dstRGB, GLenum dstA) noexcept {
    // WARNING: don't call this without updating mRasterState
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
    // WARNING: don't call this without updating mRasterState
    update_state(state.raster.colorMask, flag, [&]() {
        glColorMask(flag, flag, flag, flag);
    });
}
void OpenGLContext::depthMask(GLboolean flag) noexcept {
    // WARNING: don't call this without updating mRasterState
    update_state(state.raster.depthMask, flag, [&]() {
        glDepthMask(flag);
    });
}

void OpenGLContext::depthFunc(GLenum func) noexcept {
    // WARNING: don't call this without updating mRasterState
    update_state(state.raster.depthFunc, func, [&]() {
        glDepthFunc(func);
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

void OpenGLContext::beginQuery(GLenum target, GLuint query) noexcept {
    switch (target) {
        case GL_TIME_ELAPSED:
            if (state.queries.timer != -1u) {
                // this is an error
                break;
            }
            state.queries.timer = query;
            break;
        default:
            return;
    }
    glBeginQuery(target, query);
}

void OpenGLContext::endQuery(GLenum target) noexcept {
    switch (target) {
        case GL_TIME_ELAPSED:
            state.queries.timer = -1u;
            break;
        default:
            return;
    }
    glEndQuery(target);
}

GLuint OpenGLContext::getQuery(GLenum target) noexcept {
    switch (target) {
        case GL_TIME_ELAPSED:
            return state.queries.timer;
        default:
            return 0;
    }
}
} // namesapce filament

#endif //TNT_FILAMENT_BACKEND_OPENGLCONTEXT_H
