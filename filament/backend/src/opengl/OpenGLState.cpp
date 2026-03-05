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

#include "OpenGLState.h"

#include "GLUtils.h"

#include <backend/platforms/OpenGLPlatform.h>

#include <utility>

using namespace utils;

namespace filament::backend {

OpenGLState::OpenGLState(OpenGLContext& context) noexcept
        : ext(context.ext),
          bugs(context.bugs),
          gets(context.gets),
          procs(context.procs),
          features(context.features),
          mContext(context),
          mTimerQueryFactory(TimerQueryFactory::init(mContext.getPlatform(), context)),
          mSamplerMap(32) {
    state.vao.p = &mDefaultVAO;

    // Push GL defaults to the new context
    setDefaultState();
}

OpenGLState::~OpenGLState() noexcept {
    delete mTimerQueryFactory;
}

void OpenGLState::terminate() noexcept {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    if (!isES2()) {
        for (auto& item: mSamplerMap) {
            unbindSampler(item.second);
            glDeleteSamplers(1, &item.second);
        }
        mSamplerMap.clear();
    }
#endif
}

void OpenGLState::destroyWithContext(
        size_t index, std::function<void(OpenGLState&)> const& closure) {
    if (index == 0) {
        // Note: we only need to delay the destruction of objects on the unprotected context
        // (index 0) because the protected context is always immediately destroyed and all its
        // active objects and bindings are then automatically destroyed.
        // TODO: this is only guaranteed for EGLPlatform, but that's the only one we care about.
        mDestroyWithNormalContext.push_back(closure);
    }
}

void OpenGLState::unbindEverything() noexcept {
    // TODO:  we're supposed to unbind everything here so that resources don't get
    //        stuck in this context (contextIndex) when destroyed in the other context.
    //        However, because EGLPlatform always immediately destroys the protected context (1),
    //        the bindings will automatically be severed when we switch back to the default context.
    //        Since bindings now only exist in one context, we don't have a ref-counting issue to
    //        worry about.
}

void OpenGLState::synchronizeStateAndCache(size_t index) {

    // if we're just switching back to context 0, run all the pending destructors
    if (index == 0) {
        auto list = std::move(mDestroyWithNormalContext);
        for (auto&& fn: list) {
            fn(*this);
        }
    }

    // the default FBO could be invalid
    mDefaultFbo[index].reset();
    contextIndex = index;
    resetState();
}

void OpenGLState::setDefaultState() noexcept {
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

#if !defined(__EMSCRIPTEN__)
    if (ext.EXT_clip_control) {
#if defined(BACKEND_OPENGL_VERSION_GL)
        glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
#elif defined(GL_EXT_clip_control)
        glClipControlEXT(GL_LOWER_LEFT_EXT, GL_ZERO_TO_ONE_EXT);
#endif
    }
#endif

    if (ext.EXT_clip_cull_distance
            && mContext.getDriverConfig().stereoscopicType == StereoscopicType::INSTANCED) {
        glEnable(GL_CLIP_DISTANCE0);
        glEnable(GL_CLIP_DISTANCE1);
    }
}

GLuint OpenGLState::bindFramebuffer(GLenum target, GLuint buffer) noexcept {
    if (UTILS_UNLIKELY(buffer == 0)) {
        // we're binding the default frame buffer, resolve its actual name
        auto& defaultFboForThisContext = mDefaultFbo[contextIndex];

        if (UTILS_UNLIKELY(!defaultFboForThisContext.has_value())) {
            defaultFboForThisContext = GLuint(mContext.getPlatform().getDefaultFramebufferObject());
        }
        // by construction, defaultFboForThisContext has a value. value_or() avoids a throwing call.
        buffer = defaultFboForThisContext.value_or(0);
    }
    bindFramebufferResolved(target, buffer);
    return buffer;
}

void OpenGLState::unbindFramebuffer(GLenum target) noexcept {
    bindFramebufferResolved(target, 0);
}

void OpenGLState::bindFramebufferResolved(GLenum target, GLuint buffer) noexcept {
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

void OpenGLState::bindBuffer(GLenum target, GLuint buffer) noexcept {
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

void OpenGLState::pixelStore(GLenum pname, GLint param) noexcept {
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
            assert_invariant(mContext.major > 2);
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

void OpenGLState::unbindTexture(
        UTILS_UNUSED_IN_RELEASE GLenum target, GLuint texture_id) noexcept {
    // unbind this texture from all the units it might be bound to
    // no need unbind the texture from FBOs because we're not tracking that state (and there is
    // no need to).
    // Never attempt to unbind texture 0. This could happen with external textures w/ streaming if
    // never populated.
    if (texture_id) {
        UTILS_NOUNROLL
        for (GLuint unit = 0; unit < MAX_TEXTURE_UNIT_COUNT; unit++) {
            if (state.textures.units[unit].id == texture_id) {
                assert_invariant(state.textures.units[unit].target == target);
                unbindTextureUnit(unit);
            }
        }
    }
}

void OpenGLState::unbindTextureUnit(GLuint unit) noexcept {
    update_state(state.textures.units[unit].id, 0u, [&]() {
        activeTexture(unit);
        glBindTexture(state.textures.units[unit].target, 0u);
    });
}

void OpenGLState::unbindSampler(GLuint sampler) noexcept {
    // unbind this sampler from all the units it might be bound to
    UTILS_NOUNROLL
    for (GLuint unit = 0; unit < MAX_TEXTURE_UNIT_COUNT; unit++) {
        if (state.textures.units[unit].sampler == sampler) {
            bindSampler(unit, 0);
        }
    }
}

void OpenGLState::deleteBuffer(GLuint buffer, GLenum target) noexcept {
    glDeleteBuffers(1, &buffer);

    // bindings of bound buffers are reset to 0
    size_t const targetIndex = getIndexForBufferTarget(target);
    auto& genericBinding = state.buffers.genericBinding[targetIndex];
    if (genericBinding == buffer) {
        genericBinding = 0;
    }

    if (UTILS_UNLIKELY(bugs.rebind_buffer_after_deletion)) {
        if (genericBinding) {
            glBindBuffer(target, genericBinding);
        }
    }

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    assert_invariant(mContext.getFeatureLevel() >= FeatureLevel::FEATURE_LEVEL_1 ||
            (target != GL_UNIFORM_BUFFER && target != GL_TRANSFORM_FEEDBACK_BUFFER));

    if (target == GL_UNIFORM_BUFFER || target == GL_TRANSFORM_FEEDBACK_BUFFER) {
        auto& indexedBinding = state.buffers.targets[targetIndex];
        UTILS_NOUNROLL
        for (auto& entry: indexedBinding.buffers) {
            if (entry.name == buffer) {
                entry.name = 0;
                entry.offset = 0;
                entry.size = 0;
            }
        }
    }
#endif
}

void OpenGLState::deleteVertexArray(GLuint vao) noexcept {
    if (UTILS_LIKELY(vao)) {
        procs.deleteVertexArrays(1, &vao);
        // if the destroyed VAO is bound, clear the binding.
        if (state.vao.p->vao[contextIndex] == vao) {
            bindVertexArray(nullptr);
        }
    }
}

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
GLuint OpenGLState::getSamplerSlow(SamplerParams params) const noexcept {
    assert_invariant(mSamplerMap.find(params) == mSamplerMap.end());

    using namespace GLUtils;

    GLuint s;
    glGenSamplers(1, &s);
    glSamplerParameteri(s, GL_TEXTURE_MIN_FILTER,   (GLint)getTextureFilter(params.filterMin));
    glSamplerParameteri(s, GL_TEXTURE_MAG_FILTER,   (GLint)getTextureFilter(params.filterMag));
    glSamplerParameteri(s, GL_TEXTURE_WRAP_S,       (GLint)getWrapMode(params.wrapS));
    glSamplerParameteri(s, GL_TEXTURE_WRAP_T,       (GLint)getWrapMode(params.wrapT));
    glSamplerParameteri(s, GL_TEXTURE_WRAP_R,       (GLint)getWrapMode(params.wrapR));
    glSamplerParameteri(s, GL_TEXTURE_COMPARE_MODE, (GLint)getTextureCompareMode(params.compareMode));
    glSamplerParameteri(s, GL_TEXTURE_COMPARE_FUNC, (GLint)getTextureCompareFunc(params.compareFunc));

#if defined(GL_EXT_texture_filter_anisotropic)
    if (ext.EXT_texture_filter_anisotropic &&
        !bugs.texture_filter_anisotropic_broken_on_sampler) {
        GLfloat const anisotropy = float(1u << params.anisotropyLog2);
        glSamplerParameterf(s, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                std::min(gets.max_anisotropy, anisotropy));
    }
#endif
    CHECK_GL_ERROR()
    mSamplerMap[params] = s;
    return s;
}
#endif


void OpenGLState::resetState() noexcept {
    // Force GL state to match the Filament state

    // increase the state version so other parts of the state know to reset
    state.age++;

    if (mContext.major > 2) {
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
    state.vao.p = nullptr;
    bindVertexArray(nullptr);

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

    if (mContext.major > 2) {
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
        if (mContext.major > 2) {
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
    if (mContext.major > 2) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        glPixelStorei(GL_UNPACK_ROW_LENGTH, state.unpack.row_length);
#endif
    }

    // state.pack
    glPixelStorei(GL_PACK_ALIGNMENT, state.pack.alignment);
    if (mContext.major > 2) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);
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

void OpenGLState::createTimerQuery(GLTimerQuery* query) {
    mTimerQueryFactory->createTimerQuery(query);
}

void OpenGLState::destroyTimerQuery(GLTimerQuery* query) {
    mTimerQueryFactory->destroyTimerQuery(query);
}

void OpenGLState::beginTimeElapsedQuery(GLTimerQuery* query) {
    mTimerQueryFactory->beginTimeElapsedQuery(query);
}

void OpenGLState::endTimeElapsedQuery(OpenGLDriver& driver, GLTimerQuery* query) {
    mTimerQueryFactory->endTimeElapsedQuery(driver, query);
}

} // namespace filament::backend
