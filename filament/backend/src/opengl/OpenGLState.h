/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_OPENGL_OPENGLSTATE_H
#define TNT_FILAMENT_BACKEND_OPENGL_OPENGLSTATE_H

#include "OpenGLContext.h"
#include "OpenGLTimerQuery.h"

#include <backend/DriverEnums.h>

#include "gl_headers.h"

#include <utils/compiler.h>
#include <utils/bitset.h>
#include <utils/debug.h>

#include <math/vec2.h>
#include <math/vec4.h>

#include <tsl/robin_map.h>

#include <functional>
#include <optional>
#include <vector>

#include <stddef.h>
#include <stdint.h>

namespace filament::backend {

class OpenGLDriver;

class OpenGLState final : public TimerQueryFactoryInterface {
public:
    using RenderPrimitive = OpenGLContext::RenderPrimitive;
    using Extensions = OpenGLContext::Extensions;
    using Bugs = OpenGLContext::Bugs;
    using Gets = OpenGLContext::Gets;
    using Procs = OpenGLContext::Procs;
    using vec4gli = OpenGLContext::vec4gli;
    using vec2glf = OpenGLContext::vec2glf;

    static constexpr size_t MAX_TEXTURE_UNIT_COUNT = OpenGLContext::MAX_TEXTURE_UNIT_COUNT;
    static constexpr size_t DUMMY_TEXTURE_BINDING = OpenGLContext::DUMMY_TEXTURE_BINDING;
    static constexpr size_t MAX_BUFFER_BINDINGS = OpenGLContext::MAX_BUFFER_BINDINGS;

    explicit OpenGLState(OpenGLContext& context) noexcept;
    ~OpenGLState() noexcept override;

    // TimerQueryInterface ------------------------------------------------------------------------

    // note: OpenGLState being final ensures (clang) these are not called through the vtable
    void createTimerQuery(GLTimerQuery* query) override;
    void destroyTimerQuery(GLTimerQuery* query) override;
    void beginTimeElapsedQuery(GLTimerQuery* query) override;
    void endTimeElapsedQuery(OpenGLDriver& driver, GLTimerQuery* query) override;

    // --------------------------------------------------------------------------------------------

    // Immutable config forwarding (enables gl.ext.XXX, gl.bugs.XXX, etc)
    Extensions const& ext;
    Bugs const& bugs;
    Gets const& gets;
    Procs const& procs;
    decltype(OpenGLContext::features) const& features;

    // Read-only query forwarding
    bool isES2() const noexcept { return mContext.isES2(); }
    bool hasFences() const noexcept { return mContext.hasFences(); }
    FeatureLevel getFeatureLevel() const noexcept { return mContext.getFeatureLevel(); }
    ShaderModel getShaderModel() const noexcept { return mContext.getShaderModel(); }
    template<int MAJOR, int MINOR>
    bool isAtLeastGL() const noexcept { return mContext.isAtLeastGL<MAJOR, MINOR>(); }
    template<int MAJOR, int MINOR>
    bool isAtLeastGLES() const noexcept { return mContext.isAtLeastGLES<MAJOR, MINOR>(); }

    OpenGLContext const& context() const noexcept { return mContext; }

    // --------------------------------------------------------------------------------------------

    constexpr inline size_t getIndexForCap(GLenum cap) noexcept;
    constexpr static inline size_t getIndexForBufferTarget(GLenum target) noexcept;

    void terminate() noexcept;

    void resetState() noexcept;

    inline void useProgram(GLuint program) noexcept;

           void pixelStore(GLenum, GLint) noexcept;
    inline void activeTexture(GLuint unit) noexcept;
    inline void bindTexture(GLuint unit, GLuint target, GLuint texId, bool external) noexcept;

           void unbindTexture(GLenum target, GLuint id) noexcept;
           void unbindTextureUnit(GLuint unit) noexcept;
    inline void bindVertexArray(RenderPrimitive const* p) noexcept;
    inline void bindSampler(GLuint unit, GLuint sampler) noexcept;
           void unbindSampler(GLuint sampler) noexcept;

           void bindBuffer(GLenum target, GLuint buffer) noexcept;
    inline void bindBufferRange(GLenum target, GLuint index, GLuint buffer,
            GLintptr offset, GLsizeiptr size) noexcept;

    GLuint bindFramebuffer(GLenum target, GLuint buffer) noexcept;
    void unbindFramebuffer(GLenum target) noexcept;

    inline void enableVertexAttribArray(RenderPrimitive const* rp, GLuint index) noexcept;
    inline void disableVertexAttribArray(RenderPrimitive const* rp, GLuint index) noexcept;
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

    void deleteBuffer(GLuint buffer, GLenum target) noexcept;
    void deleteVertexArray(GLuint vao) noexcept;

    void destroyWithContext(size_t index, std::function<void(OpenGLState&)> const& closure);

    // state getters
    vec4gli const& getViewport() const { return state.window.viewport; }

    // function to handle state changes we don't control
    void updateTexImage(GLenum target, GLuint id) noexcept {
        assert_invariant(target == GL_TEXTURE_EXTERNAL_OES);
        // if another target is bound to this texture unit, unbind that texture
        if (UTILS_UNLIKELY(state.textures.units[state.textures.active].target != target)) {
            glBindTexture(state.textures.units[state.textures.active].target, 0);
            state.textures.units[state.textures.active].target = GL_TEXTURE_EXTERNAL_OES;
        }
        // the texture is already bound to `target`, we just update our internal state
        state.textures.units[state.textures.active].id = id;
    }
    void resetProgram() noexcept { state.program.use = 0; }

    void unbindEverything() noexcept;
    void synchronizeStateAndCache(size_t index);

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    GLuint getSamplerSlow(SamplerParams sp) const noexcept;

    inline GLuint getSampler(SamplerParams sp) const noexcept {
        assert_invariant(!sp.padding0);
        assert_invariant(!sp.padding1);
        assert_invariant(!sp.padding2);
        auto& samplerMap = mSamplerMap;
        auto pos = samplerMap.find(sp);
        if (UTILS_UNLIKELY(pos == samplerMap.end())) {
            return getSamplerSlow(sp);
        }
        return pos->second;
    }
#endif

    // This is the index of the context in use. Must be either 0 (Unprotected) or 1 (Protected).
    // This is used to manage the OpenGL name of ContainerObjects within each context.
    uint32_t contextIndex = 0;

    // Try to keep the State structure sorted by data-access patterns
    struct State {
        State() noexcept = default;
        // make sure we don't copy this state by accident
        State(State const& rhs) = delete;
        State(State&& rhs) noexcept = delete;
        State& operator=(State const& rhs) = delete;
        State& operator=(State&& rhs) noexcept = delete;

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
                GLuint target = 0;
                GLuint id = 0;
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
        uint8_t age = 0;
    } state;

private:
    OpenGLContext const& mContext;
    TimerQueryFactoryInterface* mTimerQueryFactory = nullptr;
    std::vector<std::function<void(OpenGLState&)>> mDestroyWithNormalContext;
    RenderPrimitive mDefaultVAO;
    std::optional<GLuint> mDefaultFbo[2]; // 0:Unprotected, 1:Protected
    mutable tsl::robin_map<SamplerParams, GLuint,
            SamplerParams::Hasher, SamplerParams::EqualTo> mSamplerMap;

    void bindFramebufferResolved(GLenum target, GLuint buffer) noexcept;

    void setDefaultState() noexcept;

    template <typename T, typename F>
    static inline void update_state(T& state, T const& expected, F functor, bool force = false) noexcept {
        if (UTILS_UNLIKELY(force || state != expected)) {
            state = expected;
            functor();
        }
    }
};

// ------------------------------------------------------------------------------------------------

constexpr size_t OpenGLState::getIndexForCap(GLenum cap) noexcept { //NOLINT
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
        case GL_DEPTH_CLAMP:                    index = 11; break;
        default: break;
    }
    assert_invariant(index < state.enables.caps.size());
    return index;
}

constexpr size_t OpenGLState::getIndexForBufferTarget(GLenum target) noexcept {
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

void OpenGLState::activeTexture(GLuint unit) noexcept {
    assert_invariant(unit < MAX_TEXTURE_UNIT_COUNT);
    update_state(state.textures.active, unit, [&]() {
        glActiveTexture(GL_TEXTURE0 + unit);
    });
}

void OpenGLState::bindSampler(GLuint unit, GLuint sampler) noexcept {
    assert_invariant(unit < MAX_TEXTURE_UNIT_COUNT);
    assert_invariant(mContext.getFeatureLevel() >= FeatureLevel::FEATURE_LEVEL_1);
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    update_state(state.textures.units[unit].sampler, sampler, [&]() {
        glBindSampler(unit, sampler);
    });
#endif
}

void OpenGLState::setScissor(GLint left, GLint bottom, GLsizei width, GLsizei height) noexcept {
    vec4gli const scissor(left, bottom, width, height);
    update_state(state.window.scissor, scissor, [&]() {
        glScissor(left, bottom, width, height);
    });
}

void OpenGLState::viewport(GLint left, GLint bottom, GLsizei width, GLsizei height) noexcept {
    vec4gli const viewport(left, bottom, width, height);
    update_state(state.window.viewport, viewport, [&]() {
        glViewport(left, bottom, width, height);
    });
}

void OpenGLState::depthRange(GLclampf near, GLclampf far) noexcept {
    vec2glf const depthRange(near, far);
    update_state(state.window.depthRange, depthRange, [&]() {
        glDepthRangef(near, far);
    });
}

void OpenGLState::bindVertexArray(RenderPrimitive const* p) noexcept {
    RenderPrimitive* vao = p ? const_cast<RenderPrimitive *>(p) : &mDefaultVAO;
    update_state(state.vao.p, vao, [&]() {

        // See if we need to create a name for this VAO on the fly, this would happen if:
        // - we're not the default VAO, because its name is always 0
        // - our name is 0, this could happen if this VAO was created in the "other" context
        // - the nameVersion is out of date *and* we're on the protected context, in this case:
        //      - the name must be stale from a previous use of this context because we always
        //        destroy the protected context when we're done with it.
        bool const recreateVaoName = vao != &mDefaultVAO &&
                ((vao->vao[contextIndex] == 0) ||
                        (vao->nameVersion != state.age && contextIndex == 1));
        if (UTILS_UNLIKELY(recreateVaoName)) {
            vao->nameVersion = state.age;
            procs.genVertexArrays(1, &vao->vao[contextIndex]);
        }

        procs.bindVertexArray(vao->vao[contextIndex]);
        // update GL_ELEMENT_ARRAY_BUFFER, which is updated by glBindVertexArray
        size_t const targetIndex = getIndexForBufferTarget(GL_ELEMENT_ARRAY_BUFFER);
        state.buffers.genericBinding[targetIndex] = vao->elementArray;
        if (UTILS_UNLIKELY(bugs.vao_doesnt_store_element_array_buffer_binding || recreateVaoName)) {
            // This shouldn't be needed, but it looks like some drivers don't do the implicit
            // glBindBuffer().
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao->elementArray);
        }
    });
}

void OpenGLState::bindBufferRange(GLenum target, GLuint index, GLuint buffer,
        GLintptr offset, GLsizeiptr size) noexcept {
    assert_invariant(mContext.getFeatureLevel() >= FeatureLevel::FEATURE_LEVEL_1);

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

void OpenGLState::bindTexture(GLuint unit, GLuint target, GLuint texId, bool external) noexcept {
    //  another texture is bound to the same unit with a different target,
    //  unbind the texture from the current target
    update_state(state.textures.units[unit].target, target, [&]() {
        activeTexture(unit);
        glBindTexture(state.textures.units[unit].target, 0);
    });
    update_state(state.textures.units[unit].id, texId, [&]() {
        activeTexture(unit);
        glBindTexture(target, texId);
    }, external);
}

void OpenGLState::useProgram(GLuint program) noexcept {
    update_state(state.program.use, program, [&]() {
        glUseProgram(program);
    });
}

void OpenGLState::enableVertexAttribArray(RenderPrimitive const* rp, GLuint index) noexcept {
    assert_invariant(rp);
    assert_invariant(index < rp->vertexAttribArray.size());
    bool const force = rp->stateVersion != state.age;
    if (UTILS_UNLIKELY(force || !rp->vertexAttribArray[index])) {
        rp->vertexAttribArray.set(index);
        glEnableVertexAttribArray(index);
    }
}

void OpenGLState::disableVertexAttribArray(RenderPrimitive const* rp, GLuint index) noexcept {
    assert_invariant(rp);
    assert_invariant(index < rp->vertexAttribArray.size());
    bool const force = rp->stateVersion != state.age;
    if (UTILS_UNLIKELY(force || rp->vertexAttribArray[index])) {
        rp->vertexAttribArray.unset(index);
        glDisableVertexAttribArray(index);
    }
}

void OpenGLState::enable(GLenum cap) noexcept {
    size_t const index = getIndexForCap(cap);
    if (UTILS_UNLIKELY(!state.enables.caps[index])) {
        state.enables.caps.set(index);
        glEnable(cap);
    }
}

void OpenGLState::disable(GLenum cap) noexcept {
    size_t const index = getIndexForCap(cap);
    if (UTILS_UNLIKELY(state.enables.caps[index])) {
        state.enables.caps.unset(index);
        glDisable(cap);
    }
}

void OpenGLState::frontFace(GLenum mode) noexcept {
    update_state(state.raster.frontFace, mode, [&]() {
        glFrontFace(mode);
    });
}

void OpenGLState::cullFace(GLenum mode) noexcept {
    update_state(state.raster.cullFace, mode, [&]() {
        glCullFace(mode);
    });
}

void OpenGLState::blendEquation(GLenum modeRGB, GLenum modeA) noexcept {
    if (UTILS_UNLIKELY(
            state.raster.blendEquationRGB != modeRGB || state.raster.blendEquationA != modeA)) {
        state.raster.blendEquationRGB = modeRGB;
        state.raster.blendEquationA   = modeA;
        glBlendEquationSeparate(modeRGB, modeA);
    }
}

void OpenGLState::blendFunction(GLenum srcRGB, GLenum srcA, GLenum dstRGB, GLenum dstA) noexcept {
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

void OpenGLState::colorMask(GLboolean flag) noexcept {
    update_state(state.raster.colorMask, flag, [&]() {
        glColorMask(flag, flag, flag, flag);
    });
}
void OpenGLState::depthMask(GLboolean flag) noexcept {
    update_state(state.raster.depthMask, flag, [&]() {
        glDepthMask(flag);
    });
}

void OpenGLState::depthFunc(GLenum func) noexcept {
    update_state(state.raster.depthFunc, func, [&]() {
        glDepthFunc(func);
    });
}

void OpenGLState::stencilFuncSeparate(GLenum funcFront, GLint refFront, GLuint maskFront,
        GLenum funcBack, GLint refBack, GLuint maskBack) noexcept {
    update_state(state.stencil.front.func, {funcFront, refFront, maskFront}, [&]() {
        glStencilFuncSeparate(GL_FRONT, funcFront, refFront, maskFront);
    });
    update_state(state.stencil.back.func, {funcBack, refBack, maskBack}, [&]() {
        glStencilFuncSeparate(GL_BACK, funcBack, refBack, maskBack);
    });
}

void OpenGLState::stencilOpSeparate(GLenum sfailFront, GLenum dpfailFront, GLenum dppassFront,
        GLenum sfailBack, GLenum dpfailBack, GLenum dppassBack) noexcept {
    update_state(state.stencil.front.op, {sfailFront, dpfailFront, dppassFront}, [&]() {
        glStencilOpSeparate(GL_FRONT, sfailFront, dpfailFront, dppassFront);
    });
    update_state(state.stencil.back.op, {sfailBack, dpfailBack, dppassBack}, [&]() {
        glStencilOpSeparate(GL_BACK, sfailBack, dpfailBack, dppassBack);
    });
}

void OpenGLState::stencilMaskSeparate(GLuint maskFront, GLuint maskBack) noexcept {
    update_state(state.stencil.front.stencilMask, maskFront, [&]() {
        glStencilMaskSeparate(GL_FRONT, maskFront);
    });
    update_state(state.stencil.back.stencilMask, maskBack, [&]() {
        glStencilMaskSeparate(GL_BACK, maskBack);
    });
}

void OpenGLState::polygonOffset(GLfloat factor, GLfloat units) noexcept {
    update_state(state.polygonOffset, { factor, units }, [&]() {
        if (factor != 0 || units != 0) {
            glPolygonOffset(factor, units);
            enable(GL_POLYGON_OFFSET_FILL);
        } else {
            disable(GL_POLYGON_OFFSET_FILL);
        }
    });
}

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_OPENGL_OPENGLSTATE_H
