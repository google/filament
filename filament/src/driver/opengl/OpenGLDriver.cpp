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

#include "driver/opengl/OpenGLDriver.h"

#include <set>

#include <utils/compiler.h>
#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/Systrace.h>

#include "driver/DriverApi.h"
#include "driver/CommandStream.h"
#include "driver/opengl/OpenGLProgram.h"
#include "driver/opengl/OpenGLBlitter.h"

#include <filament/driver/Platform.h>

// change to true to display all GL extensions in the console on start-up
#define DEBUG_PRINT_EXTENSIONS false

#if defined(__EMSCRIPTEN__)
#define HAS_MAPBUFFERS 0
#else
#define HAS_MAPBUFFERS 1
#endif

#define DEBUG_MARKER_NONE       0
#define DEBUG_MARKER_OPENGL     1
#define DEBUG_MARKER_SYSTRACE   2

// set to the desired debug marker level
#define DEBUG_MARKER_LEVEL      DEBUG_MARKER_NONE

#if DEBUG_MARKER_LEVEL == DEBUG_MARKER_OPENGL
#   define DEBUG_MARKER() \
        DebugMarker _debug_marker(*this, __PRETTY_FUNCTION__);
#elif DEBUG_MARKER_LEVEL == DEBUG_MARKER_SYSTRACE
#   define DEBUG_MARKER() \
        SYSTRACE_CALL();
#else
#   define DEBUG_MARKER()
#endif

using namespace math;
using namespace utils;

namespace filament {

using namespace driver;
using namespace GLUtils;

Driver* OpenGLDriver::create(
        OpenGLPlatform* const platform, void* const sharedGLContext) noexcept {
    assert(platform);
    OpenGLPlatform* const ec = platform;

    { // here we check we're on a supported version of GL before initializing the driver
        GLint major = 0, minor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);

        if (UTILS_UNLIKELY(glGetError() != GL_NO_ERROR)) {
            PANIC_LOG("Can't get OpenGL version");
            cleanup:
            ec->terminate();
            return {};
        }

        if (GLES31_HEADERS) {
            // we require GLES 3.1 headers, but we support GLES 3.0
            if (UTILS_UNLIKELY(!(major >= 3 && minor >= 0))) {
                PANIC_LOG("OpenGL ES 3.0 minimum needed (current %d.%d)", major, minor);
                goto cleanup;

            }
        } else if (GL41_HEADERS) {
            // we require GL 4.1 headers and minimum version
            if (UTILS_UNLIKELY(!((major == 4 && minor >= 1) || major > 4))) {
                PANIC_LOG("OpenGL 4.1 minimum needed (current %d.%d)", major, minor);
                goto cleanup;
            }
        }
    }

    OpenGLDriver* const driver = new OpenGLDriver(ec);
    return driver;
}

OpenGLDriver::OpenGLDriver(OpenGLPlatform* platform) noexcept
        : DriverBase(new ConcreteDispatcher<OpenGLDriver>(this)),
          mHandleArena("Handles", 2U * 1024U * 1024U), // TODO: set the amount in configuration
          mSamplerMap(32),
          mPlatform(*platform) {
    state.enables.caps.set(getIndexForCap(GL_DITHER));
    state.vao.p = &mDefaultVAO;

    std::fill(mSamplerBindings.begin(), mSamplerBindings.end(), nullptr);

    // set a reasonable default value for our stream array
    mExternalStreams.reserve(8);

   UTILS_UNUSED char const* const vendor   = (char const*) glGetString(GL_VENDOR);
   UTILS_UNUSED char const* const renderer = (char const*) glGetString(GL_RENDERER);
   UTILS_UNUSED char const* const version  = (char const*) glGetString(GL_VERSION);
   UTILS_UNUSED char const* const shader   = (char const*) glGetString(GL_SHADING_LANGUAGE_VERSION);

#ifndef NDEBUG
    slog.i
        << vendor << io::endl
        << renderer << io::endl
        << version << io::endl
        << shader << io::endl
        << "OS version: " << mPlatform.getOSVersion() << io::endl;
#endif

    // OpenGL (ES) version
    GLint major = 0, minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &gets.max_renderbuffer_size);
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &gets.max_uniform_block_size);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &gets.uniform_buffer_offset_alignment);

#ifndef NDEBUG
    slog.i
        << "GL_MAX_RENDERBUFFER_SIZE = " << gets.max_renderbuffer_size << io::endl
        << "GL_MAX_UNIFORM_BLOCK_SIZE = " << gets.max_uniform_block_size << io::endl
        << "GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT = " << gets.uniform_buffer_offset_alignment << io::endl;
#endif

    if (strstr(renderer, "Adreno")) {
        bugs.clears_hurt_performance = true;
    } else if (strstr(renderer, "Mali")) {
        bugs.vao_doesnt_store_element_array_buffer_binding = true;
        if (strstr(renderer, "Mali-T")) {
            bugs.disable_early_fragment_tests = true;
            bugs.disable_shared_context_draws = true;
            bugs.texture_external_needs_rebind = true;
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
    std::set<StaticString> exts;
    for (GLint i = 0; i < n; i++) {
        const char * const ext = (const char*)glGetStringi(GL_EXTENSIONS, (GLuint)i);
        exts.emplace(ext, strlen(ext));
        if (DEBUG_PRINT_EXTENSIONS) {
            slog.d << ext << io::endl;
        }
    }

    ShaderModel shaderModel = ShaderModel::UNKNOWN;
    if (GLES31_HEADERS) {
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
    mShaderModel = shaderModel;

    /*
     * Set our default state
     */

    disable(GL_DITHER);
    enable(GL_DEPTH_TEST);

    // TODO: Don't enable scissor when it is not necessary. This optimization could be done here in
    // the driver by simply deferring the enable until the scissor rect is smaller than the window.
    enable(GL_SCISSOR_TEST);

#ifdef GL_ARB_seamless_cube_map
    enable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif

#ifdef GL_EXT_texture_filter_anisotropic
    if (ext.texture_filter_anisotropic) {
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &mMaxAnisotropy);
    }
#endif

#ifdef GL_GENERATE_MIPMAP_HINT
    glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
#endif

#ifdef GL_FRAGMENT_SHADER_DERIVATIVE_HINT
    glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);
#endif

    // For the shadow pass
    glPolygonOffset(1.0f, 1.0f);

    // On some implementation we need to clear the viewport with a triangle, for performance
    // reasons
    initClearProgram();

    // Initialize the blitter only if we have OES_EGL_image_external_essl3
    if (ext.OES_EGL_image_external_essl3) {
        mOpenGLBlitter = new OpenGLBlitter(*this);
        mOpenGLBlitter->init();
    }
}

OpenGLDriver::~OpenGLDriver() noexcept {
    delete mOpenGLBlitter;
}

// ------------------------------------------------------------------------------------------------
// Driver interface concrete implementation
// ------------------------------------------------------------------------------------------------

bool OpenGLDriver::hasExtension(std::set<StaticString> const& map, const char* ext) noexcept {
    return map.find({ ext, (StaticString::size_type) strlen(ext) }) != map.end();
}

void OpenGLDriver::initExtensionsGLES(GLint major, GLint minor, std::set<StaticString> const& exts) {
    // figure out and initialize the extensions we need
    ext.texture_filter_anisotropic = hasExtension(exts, "GL_EXT_texture_filter_anisotropic");
    ext.texture_compression_etc2 = true;
    ext.QCOM_tiled_rendering = hasExtension(exts, "GL_QCOM_tiled_rendering");
    ext.OES_EGL_image_external_essl3 = hasExtension(exts, "GL_OES_EGL_image_external_essl3");
    ext.EXT_debug_marker = hasExtension(exts, "GL_EXT_debug_marker");
    ext.EXT_color_buffer_half_float = hasExtension(exts, "GL_EXT_color_buffer_half_float");
    ext.texture_compression_s3tc = hasExtension(exts, "WEBGL_compressed_texture_s3tc");
    ext.EXT_multisampled_render_to_texture = hasExtension(exts, "GL_EXT_multisampled_render_to_texture");
}

void OpenGLDriver::initExtensionsGL(GLint major, GLint minor, std::set<StaticString> const& exts) {
    ext.texture_filter_anisotropic = hasExtension(exts, "GL_EXT_texture_filter_anisotropic");
    ext.texture_compression_etc2 = hasExtension(exts, "GL_ARB_ES3_compatibility");
    ext.texture_compression_s3tc = hasExtension(exts, "GL_EXT_texture_compression_s3tc");
    ext.OES_EGL_image_external_essl3 = hasExtension(exts, "GL_OES_EGL_image_external_essl3");
    ext.EXT_debug_marker = hasExtension(exts, "GL_EXT_debug_marker");
    ext.EXT_color_buffer_half_float = true;  // Assumes core profile.
}

void OpenGLDriver::terminate() {
    for (auto& item : mSamplerMap) {
        unbindSampler(item.second);
        glDeleteSamplers(1, &item.second);
    }
    mSamplerMap.clear();
    if (mOpenGLBlitter) {
        mOpenGLBlitter->terminate();
    }
    terminateClearProgram();
    mPlatform.terminate();
}

ShaderModel OpenGLDriver::getShaderModel() const noexcept {
    return mShaderModel;
}

const float2 OpenGLDriver::mClearTriangle[3] = {{ -1.0f,  3.0f },
                                                { -1.0f, -1.0f },
                                                {  3.0f, -1.0f }};

void OpenGLDriver::initClearProgram() noexcept {
    const char clearVertexES[] = R"SHADER(#version 300 es
        uniform float depth;
        in vec4 pos;
        void main() {
            gl_Position = vec4(pos.xy, depth, 1.0);
        }
        )SHADER";

    const char clearFragmentES[] = R"SHADER(#version 300 es
        precision mediump float;
        uniform vec4 color;
        out vec4 fragColor;
        void main() {
            fragColor = color;
        }
        )SHADER";

    if (GLES31_HEADERS) {
        GLint status;
        char const* const vsource = clearVertexES;
        char const* const fsource = clearFragmentES;

        mClearVertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(mClearVertexShader, 1, &vsource, nullptr);
        glCompileShader(mClearVertexShader);
        glGetShaderiv(mClearVertexShader, GL_COMPILE_STATUS, &status);
        assert(status == GL_TRUE);

        mClearFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(mClearFragmentShader, 1, &fsource, nullptr);
        glCompileShader(mClearFragmentShader);
        glGetShaderiv(mClearFragmentShader, GL_COMPILE_STATUS, &status);
        assert(status == GL_TRUE);

        mClearProgram = glCreateProgram();
        glAttachShader(mClearProgram, mClearVertexShader);
        glAttachShader(mClearProgram, mClearFragmentShader);
        glLinkProgram(mClearProgram);
        glGetProgramiv(mClearProgram, GL_LINK_STATUS, &status);
        assert(status == GL_TRUE);

        useProgram(mClearProgram);
        mClearColorLocation = glGetUniformLocation(mClearProgram, "color");
        mClearDepthLocation = glGetUniformLocation(mClearProgram, "depth");

        CHECK_GL_ERROR(utils::slog.e)
    }
}

void OpenGLDriver::terminateClearProgram() noexcept {
    if (GLES31_HEADERS) {
        glDetachShader(mClearProgram, mClearVertexShader);
        glDetachShader(mClearProgram, mClearFragmentShader);
        glDeleteShader(mClearVertexShader);
        glDeleteShader(mClearFragmentShader);
        glDeleteProgram(mClearProgram);
    }
}

// ------------------------------------------------------------------------------------------------
// Change and track GL state
// ------------------------------------------------------------------------------------------------

void OpenGLDriver::setScissor(GLint left, GLint bottom, GLsizei width, GLsizei height) noexcept {
    vec4gli scissor(left, bottom, width, height);
    update_state(state.window.scissor, scissor, [&]() {
        glScissor(left, bottom, width, height);
    });
}

void OpenGLDriver::setViewport(GLint left, GLint bottom, GLsizei width, GLsizei height) noexcept {
    vec4gli viewport(left, bottom, width, height);
    update_state(state.window.viewport, viewport, [&]() {
        glViewport(left, bottom, width, height);
    });
}

void OpenGLDriver::setClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) noexcept {
    float4 color(r, g, b, a);
    update_state(state.clears.color, color, [&]() {
        glClearColor(r, g, b, a);
    });
}

void OpenGLDriver::setClearDepth(GLfloat depth) noexcept {
    update_state(state.clears.depth, depth, [&]() {
        glClearDepthf(depth);
    });
}

void OpenGLDriver::setClearStencil(GLint stencil) noexcept {
    update_state(state.clears.stencil, stencil, [&]() {
        glClearStencil(stencil);
    });
}

void OpenGLDriver::unbindTexture(GLenum target, GLuint texture_id) noexcept {
    // unbind this texture from all the units it might be bound to
    // no need unbind the texture from FBOs because we're not tracking that state (and there is
    // no need to).
    const size_t index = getIndexForTextureTarget(target);
    for (GLuint unit = 0; unit < MAX_TEXTURE_UNITS; unit++) {
        if (state.textures.units[unit].targets[index].texture_id == texture_id) {
            bindTexture(unit, target, (GLuint)0, index);
        }
    }
}

void OpenGLDriver::unbindSampler(GLuint sampler) noexcept {
    // unbind this sampler from all the units it might be bound to
    #pragma nounroll    // clang generates >800B of code!!!
    for (GLuint unit = 0; unit < MAX_TEXTURE_UNITS; unit++) {
        if (state.textures.units[unit].sampler == sampler) {
            bindSampler(unit, 0);
        }
    }
}

void OpenGLDriver::bindBuffer(GLenum target, GLuint buffer) noexcept {
    size_t targetIndex = getIndexForBufferTarget(target);
    if (target == GL_ELEMENT_ARRAY_BUFFER) {
        // GL_ELEMENT_ARRAY_BUFFER is a special case, where the currently bound VAO remembers
        // the index buffer, unless there are no VAO bound (see: bindVertexArray)
        assert(state.vao.p);
        if (state.buffers.genericBinding[targetIndex] != buffer
                || ((state.vao.p != &mDefaultVAO) && (state.vao.p->gl.elementArray != buffer))) {
            state.buffers.genericBinding[targetIndex] = buffer;
            if (state.vao.p != &mDefaultVAO) {
                state.vao.p->gl.elementArray = buffer;
            }
            glBindBuffer(target, buffer);
        }
    } else {
        update_state(state.buffers.genericBinding[targetIndex], buffer, [&]() {
            glBindBuffer(target, buffer);
        });
    }
}

void OpenGLDriver::bindBufferRange(GLenum target, GLuint index, GLuint buffer,
        GLintptr offset, GLsizeiptr size) noexcept {
    size_t targetIndex = getIndexForBufferTarget(target);
    assert(targetIndex <= 1); // sanity check

    // this ALSO sets the generic binding
    if (state.buffers.genericBinding[targetIndex] != buffer
            || state.buffers.targets[targetIndex].buffers[index].name != buffer
            || state.buffers.targets[targetIndex].buffers[index].offset != offset
            || state.buffers.targets[targetIndex].buffers[index].size != size) {
        state.buffers.targets[targetIndex].buffers[index].name = buffer;
        state.buffers.targets[targetIndex].buffers[index].offset = offset;
        state.buffers.targets[targetIndex].buffers[index].size = size;
        state.buffers.genericBinding[targetIndex] = buffer;
        glBindBufferRange(target, index, buffer, offset, size);
    }
}

void OpenGLDriver::bindFramebuffer(GLenum target, GLuint buffer) noexcept {
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

void OpenGLDriver::bindVertexArray(GLRenderPrimitive const* p) noexcept {
    GLRenderPrimitive* vao = p ? const_cast<GLRenderPrimitive *>(p) : &mDefaultVAO;
    update_state(state.vao.p, vao, [&]() {
        glBindVertexArray(vao->gl.vao);
        // update GL_ELEMENT_ARRAY_BUFFER, which is updated by glBindVertexArray
        size_t targetIndex = getIndexForBufferTarget(GL_ELEMENT_ARRAY_BUFFER);
        state.buffers.genericBinding[targetIndex] = vao->gl.elementArray;
        if (UTILS_UNLIKELY(bugs.vao_doesnt_store_element_array_buffer_binding)) {
            // This shouldn't be needed, but it looks like some drivers don't do the implicit
            // glBindBuffer().
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao->gl.elementArray);
        }
    });
}

void OpenGLDriver::bindTexture(GLuint unit, GLuint target, GLuint texId, size_t targetIndex) noexcept {
    assert(targetIndex == getIndexForTextureTarget(target));
    assert(targetIndex < TEXTURE_TARGET_COUNT);
    update_state(state.textures.units[unit].targets[targetIndex].texture_id, texId, [&]() {
        activeTexture(unit);
        glBindTexture(target, texId);
    }, (target == GL_TEXTURE_EXTERNAL_OES) && bugs.texture_external_needs_rebind);
}

void OpenGLDriver::useProgram(GLuint program) noexcept {
    update_state(state.program.use, program, [&]() {
        glUseProgram(program);
    });
}

void OpenGLDriver::useProgram(OpenGLProgram* p) noexcept {
    useProgram(p->gl.program);
    // set-up textures and samplers in the proper TMUs (as specified in setSamplers)
    p->use(this);
}

void OpenGLDriver::enableVertexAttribArray(GLuint index) noexcept {
    assert(state.vao.p);
    assert(index < state.vao.p->gl.vertexAttribArray.size());
    if (UTILS_UNLIKELY(!state.vao.p->gl.vertexAttribArray[index])) {
        state.vao.p->gl.vertexAttribArray.set(index);
        glEnableVertexAttribArray(index);
    }
}

void OpenGLDriver::disableVertexAttribArray(GLuint index) noexcept {
    assert(state.vao.p);
    assert(index < state.vao.p->gl.vertexAttribArray.size());
    if (UTILS_UNLIKELY(state.vao.p->gl.vertexAttribArray[index])) {
        state.vao.p->gl.vertexAttribArray.unset(index);
        glDisableVertexAttribArray(index);
    }
}

void OpenGLDriver::enable(GLenum cap) noexcept {
    size_t index = getIndexForCap(cap);
    if (UTILS_UNLIKELY(!state.enables.caps[index])) {
        state.enables.caps.set(index);
        glEnable(cap);
    }
}

void OpenGLDriver::disable(GLenum cap) noexcept {
    size_t index = getIndexForCap(cap);
    if (UTILS_UNLIKELY(state.enables.caps[index])) {
        state.enables.caps.unset(index);
        glDisable(cap);
    }
}

void OpenGLDriver::cullFace(GLenum mode) noexcept {
    // WARNING: don't call this without updating mRasterState
    update_state(state.raster.cullFace, mode, [&]() {
        glCullFace(mode);
    });
}

void OpenGLDriver::blendEquation(GLenum modeRGB, GLenum modeA) noexcept {
    // WARNING: don't call this without updating mRasterState
    if (UTILS_UNLIKELY(
            state.raster.blendEquationRGB != modeRGB || state.raster.blendEquationA != modeA)) {
        state.raster.blendEquationRGB = modeRGB;
        state.raster.blendEquationA   = modeA;
        glBlendEquationSeparate(modeRGB, modeA);
    }
}

void OpenGLDriver::blendFunction(GLenum srcRGB, GLenum srcA, GLenum dstRGB, GLenum dstA) noexcept {
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
        glBlendFuncSeparate(srcRGB, dstRGB, dstA, srcA);
    }
}

void OpenGLDriver::colorMask(GLboolean flag) noexcept {
    // WARNING: don't call this without updating mRasterState
    update_state(state.raster.colorMask, flag, [&]() {
        glColorMask(flag, flag, flag, flag);
    });
}
void OpenGLDriver::depthMask(GLboolean flag) noexcept {
    // WARNING: don't call this without updating mRasterState
    update_state(state.raster.depthMask, flag, [&]() {
        glDepthMask(flag);
    });
}

void OpenGLDriver::depthFunc(GLenum func) noexcept {
    // WARNING: don't call this without updating mRasterState
    update_state(state.raster.depthFunc, func, [&]() {
        glDepthFunc(func);
    });
}

void OpenGLDriver::polygonOffset(GLfloat factor, GLfloat units) noexcept {
    // if we're in the shadow-pass, the default polygon offset is factor = unit = 1
    // TODO: this should be controlled by filament, instead of being a feature of the driver
    if (factor == 0 && units == 0) {
        TargetBufferFlags clearFlags = (TargetBufferFlags)mRenderPassParams.clear;
        if ((clearFlags & TargetBufferFlags::SHADOW) == TargetBufferFlags::SHADOW) {
            factor = units = 1.0f;
        }
    }

    update_state(state.polygonOffset, { factor, units }, [&]() {
        if (factor != 0 || units != 0) {
            glPolygonOffset(factor, units);
            enable(GL_POLYGON_OFFSET_FILL);
        } else {
            disable(GL_POLYGON_OFFSET_FILL);
        }
    });
}

void OpenGLDriver::setRasterStateSlow(Driver::RasterState rs) noexcept {
    mRasterState = rs;

    // culling state
    switch (rs.culling) {
        case CullingMode::NONE:
            disable(GL_CULL_FACE);
            break;
        case CullingMode::FRONT:
            cullFace(GL_FRONT);
            break;
        case CullingMode::BACK:
            cullFace(GL_BACK);
            break;
        case CullingMode::FRONT_AND_BACK:
            cullFace(GL_FRONT_AND_BACK);
            break;
    }

    if (rs.culling != CullingMode::NONE) {
        enable(GL_CULL_FACE);
    }

    // blending state
    if (!rs.hasBlending()) {
        disable(GL_BLEND);
    } else {
        enable(GL_BLEND);
        blendEquation(
                getBlendEquationMode(rs.blendEquationRGB),
                getBlendEquationMode(rs.blendEquationAlpha));

        blendFunction(
                getBlendFunctionMode(rs.blendFunctionSrcRGB),
                getBlendFunctionMode(rs.blendFunctionSrcAlpha),
                getBlendFunctionMode(rs.blendFunctionDstRGB),
                getBlendFunctionMode(rs.blendFunctionDstAlpha));
    }

    // depth test
    depthFunc(getDepthFunc(rs.depthFunc));

    // write masks
    colorMask(GLboolean(rs.colorWrite));
    depthMask(GLboolean(rs.depthWrite));

    // AA
    if (rs.alphaToCoverage) {
        enable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    } else {
        disable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    }
}

void OpenGLDriver::pixelStore(GLenum pname, GLint param) noexcept {
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

// ------------------------------------------------------------------------------------------------
// Creating driver objects
// ------------------------------------------------------------------------------------------------

// For reference on a 64-bits machine:
//    GLFence                   :  8
//    GLIndexBuffer             : 12        moderate
//    GLSamplerBuffer           : 16        moderate
// -- less than 16 bytes

//    GLRenderPrimitive         : 40        many
//    GLTexture                 : 44        moderate
//    OpenGLProgram             : 40        moderate
//    GLRenderTarget            : 56        few
// -- less than 64 bytes

//    GLVertexBuffer            : 80        moderate
//    GLStream                  : 120       few
//    GLUniformBuffer           : 128       many
// -- less than 128 bytes


OpenGLDriver::HandleAllocator::HandleAllocator(const utils::HeapArea& area)
        : mPool0(area.begin(),
                  pointermath::add(area.begin(), (1 * area.getSize()) / 16)),
          mPool1( pointermath::add(area.begin(), (1 * area.getSize()) / 16),
                  pointermath::add(area.begin(), (6 * area.getSize()) / 16)),
          mPool2( pointermath::add(area.begin(), (6 * area.getSize()) / 16),
                  area.end()) {

#ifndef NDEBUG
    slog.d << "HwFence: " << sizeof(HwFence) << io::endl;
    slog.d << "GLIndexBuffer: " << sizeof(GLIndexBuffer) << io::endl;
    slog.d << "GLSamplerBuffer: " << sizeof(GLSamplerBuffer) << io::endl;
    slog.d << "GLRenderPrimitive: " << sizeof(GLRenderPrimitive) << io::endl;
    slog.d << "GLTexture: " << sizeof(GLTexture) << io::endl;
    slog.d << "OpenGLProgram: " << sizeof(OpenGLProgram) << io::endl;
    slog.d << "GLRenderTarget: " << sizeof(GLRenderTarget) << io::endl;
    slog.d << "GLVertexBuffer: " << sizeof(GLVertexBuffer) << io::endl;
    slog.d << "GLUniformBuffer: " << sizeof(GLUniformBuffer) << io::endl;
    slog.d << "GLStream: " << sizeof(GLStream) << io::endl;
#endif
}

void* OpenGLDriver::HandleAllocator::alloc(size_t size, size_t alignment, size_t extra) noexcept {
    assert(size <= mPool2.getSize());
    if (size <= mPool0.getSize()) return mPool0.alloc(size, 16, extra);
    if (size <= mPool1.getSize()) return mPool1.alloc(size, 32, extra);
    if (size <= mPool2.getSize()) return mPool2.alloc(size, 32, extra);
    return nullptr;
}

void OpenGLDriver::HandleAllocator::free(void* p, size_t size) noexcept {
    if (size <= mPool0.getSize()) { mPool0.free(p); return; }
    if (size <= mPool1.getSize()) { mPool1.free(p); return; }
    if (size <= mPool2.getSize()) { mPool2.free(p); return; }
}


// This is "NOINLINE" because it ends-up generating more code than we'd like because of
// the locking (unfortunately, mHandleArena is accessed from 2 threads)
UTILS_NOINLINE
HandleBase::HandleId OpenGLDriver::allocateHandle(size_t size) noexcept {
    void* addr = mHandleArena.alloc(size);
    char* const base = (char *)mHandleArena.getArea().begin();
    size_t offset = (char*)addr - base;
    return HandleBase::HandleId(offset >> HandleAllocator::MIN_ALIGNMENT_SHIFT);
}

template<typename D, typename B, typename ... ARGS>
typename std::enable_if<std::is_base_of<B, D>::value, D>::type*
OpenGLDriver::construct(Handle<B> const& handle, ARGS&& ... args) noexcept {
    assert(handle);
    static_assert(sizeof(D) <= 128, "Handle<> too large");
    D* addr = handle_cast<D *>(const_cast<Handle<B>&>(handle));
    new(addr) D(std::forward<ARGS>(args)...);
#if !defined(NDEBUG) && UTILS_HAS_RTTI
    addr->typeId = typeid(D).name();
#endif
    return addr;
}

template <typename B, typename D, typename>
void OpenGLDriver::destruct(Handle<B>& handle, D const* p) noexcept {
    // allow to destroy the nullptr, similarly to operator delete
    if (p) {
#if !defined(NDEBUG) && UTILS_HAS_RTTI
        if (UTILS_UNLIKELY(p->typeId != typeid(D).name())) {
            slog.e << "Destroying handle " << handle.getId() << ", type " << typeid(D).name()
                   << ", but handle's actual type is " << p->typeId << io::endl;
            std::terminate();
        }
        const_cast<D *>(p)->typeId = "(deleted)";
#endif
        p->~D();
        mHandleArena.free(const_cast<D*>(p), sizeof(D));
    }
}

Handle<HwVertexBuffer> OpenGLDriver::createVertexBufferSynchronous() noexcept {
    return Handle<HwVertexBuffer>( allocateHandle(sizeof(GLVertexBuffer)) );
}

Handle<HwIndexBuffer> OpenGLDriver::createIndexBufferSynchronous() noexcept {
    return Handle<HwIndexBuffer>( allocateHandle(sizeof(GLIndexBuffer)) );
}

Handle<HwRenderPrimitive> OpenGLDriver::createRenderPrimitiveSynchronous() noexcept {
    return Handle<HwRenderPrimitive>( allocateHandle(sizeof(GLRenderPrimitive)) );
}

Handle<HwProgram> OpenGLDriver::createProgramSynchronous() noexcept {
    return Handle<HwProgram>( allocateHandle(sizeof(OpenGLProgram)) );
}

Handle<HwSamplerBuffer> OpenGLDriver::createSamplerBufferSynchronous() noexcept {
    return Handle<HwSamplerBuffer>( allocateHandle(sizeof(GLSamplerBuffer)) );
}

Handle<HwUniformBuffer> OpenGLDriver::createUniformBufferSynchronous() noexcept {
    return Handle<HwUniformBuffer>( allocateHandle(sizeof(GLUniformBuffer)) );
}

Handle<HwTexture> OpenGLDriver::createTextureSynchronous() noexcept {
    return Handle<HwTexture>( allocateHandle(sizeof(GLTexture)) );
}

Handle<HwRenderTarget> OpenGLDriver::createDefaultRenderTargetSynchronous() noexcept {
    return Handle<HwRenderTarget>( allocateHandle(sizeof(GLRenderTarget)) );
}

Handle<HwRenderTarget> OpenGLDriver::createRenderTargetSynchronous() noexcept {
    return Handle<HwRenderTarget>( allocateHandle(sizeof(GLRenderTarget)) );
}

Handle<HwFence> OpenGLDriver::createFenceSynchronous() noexcept {
    return Handle<HwFence>( allocateHandle(sizeof(HwFence)) );
}

Handle<HwSwapChain> OpenGLDriver::createSwapChainSynchronous() noexcept {
    return Handle<HwSwapChain>( allocateHandle(sizeof(HwSwapChain)) );
}

Handle<HwStream> OpenGLDriver::createStreamFromTextureIdSynchronous() noexcept {
    return Handle<HwStream>( allocateHandle(sizeof(GLStream)) );
}

void OpenGLDriver::createVertexBuffer(
    Driver::VertexBufferHandle vbh,
    uint8_t bufferCount,
    uint8_t attributeCount,
    uint32_t elementCount,
    Driver::AttributeArray attributes,
    Driver::BufferUsage usage) {
    DEBUG_MARKER()

    GLVertexBuffer* vb = construct<GLVertexBuffer>(vbh,
            bufferCount, attributeCount, elementCount, attributes);

    GLsizei n = GLsizei(vb->bufferCount);
    glGenBuffers(n, vb->gl.buffers.data());

    for (GLsizei i = 0; i < n; i++) {
        // figure out the size needed for each buffer
        size_t size = 0;
        for (auto const& item : attributes) {
            if (item.buffer == i) {
                size_t end = item.offset + elementCount * item.stride;
                size = std::max(size, end);
            }
        }
        bindBuffer(GL_ARRAY_BUFFER, vb->gl.buffers[i]);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, getBufferUsage(usage));
    }

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createIndexBuffer(
        Driver::IndexBufferHandle ibh,
        Driver::ElementType elementType,
        uint32_t indexCount,
        Driver::BufferUsage usage) {
    DEBUG_MARKER()

    uint8_t elementSize = static_cast<uint8_t>(getElementTypeSize(elementType));
    GLIndexBuffer* ib = construct<GLIndexBuffer>(ibh, elementSize, indexCount);
    glGenBuffers(1, &ib->gl.buffer);
    GLsizeiptr size = elementSize * indexCount;
    bindVertexArray(nullptr);
    bindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->gl.buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, getBufferUsage(usage));
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createRenderPrimitive(Driver::RenderPrimitiveHandle rph, int) {
    DEBUG_MARKER()

    GLRenderPrimitive* rp = construct<GLRenderPrimitive>(rph);
    glGenVertexArrays(1, &rp->gl.vao);
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createProgram(Driver::ProgramHandle ph, Program&& program) {
    DEBUG_MARKER()

    construct<OpenGLProgram>(ph, this, program);
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createSamplerBuffer(Driver::SamplerBufferHandle sbh, size_t size) {
    DEBUG_MARKER()

    construct<GLSamplerBuffer>(sbh, size);
}

void OpenGLDriver::createUniformBuffer(
        Driver::UniformBufferHandle ubh,
        size_t size,
        Driver::BufferUsage usage) {
    DEBUG_MARKER()

    GLUniformBuffer* ub = construct<GLUniformBuffer>(ubh, size, usage);
    glGenBuffers(1, &ub->gl.ubo.id);
    bindBuffer(GL_UNIFORM_BUFFER, ub->gl.ubo.id);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, getBufferUsage(usage));
    CHECK_GL_ERROR(utils::slog.e)
}


UTILS_NOINLINE
void OpenGLDriver::textureStorage(OpenGLDriver::GLTexture* t,
        uint32_t width, uint32_t height, uint32_t depth) noexcept {

    bindTexture(MAX_TEXTURE_UNITS - 1, t->gl.target, t, t->gl.targetIndex);
    activeTexture(MAX_TEXTURE_UNITS - 1);

    switch (t->gl.target) {
        case GL_TEXTURE_2D:
        case GL_TEXTURE_CUBE_MAP:
            glTexStorage2D(t->gl.target, GLsizei(t->levels), t->gl.internalFormat,
                    GLsizei(width), GLsizei(height));
            break;
        case GL_TEXTURE_3D: {
            glTexStorage3D(t->gl.target, GLsizei(t->levels), t->gl.internalFormat,
                    GLsizei(width), GLsizei(height), GLsizei(depth));
            break;
        }
        case GL_TEXTURE_2D_MULTISAMPLE:
            // NOTE: if there is a mix of texture and renderbuffers, "fixed_sample_locations" must be true
            // NOTE: what's the benefit of setting "fixed_sample_locations" to false?
#if GLES31_HEADERS
            // only supported from GL 4.3 and GLES 3.1
            glTexStorage2DMultisample(t->gl.target, t->samples, t->gl.internalFormat,
                    GLsizei(width), GLsizei(height), GL_TRUE);
#elif GL41_HEADERS
            // only supported in GL (never in GLES)
            // TODO: use glTexStorage2DMultisample() on GL 4.3 and above
            glTexImage2DMultisample(t->gl.target, t->samples, t->gl.internalFormat,
                    GLsizei(width), GLsizei(height), GL_TRUE);
#else
#   error "GL/GLES header version not supported"
#endif
            break;
        default: // cannot happen
            break;
    }

    // textureStorage can be used to reallocate the texture at a new size
    t->width = width;
    t->height = height;
    t->depth = depth;
}

void OpenGLDriver::createTexture(Driver::TextureHandle th, SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
        TextureUsage usage) {
    DEBUG_MARKER()

    GLTexture* t = construct<GLTexture>(th, target, levels, samples, w, h, depth);
    glGenTextures(1, &t->gl.texture_id);

    // below we're using the a = foo(b = C) pattern, this is on purpose, to make sure
    // we don't forget to update targetIndex, and that we do it with the correct value.
    // We DO NOT update targetIndex at function exit to take advantage of the fact that
    // getIndexForTextureTarget() is constexpr -- so all of this disappears at compile time.

    if (UTILS_UNLIKELY(t->target == SamplerType::SAMPLER_EXTERNAL &&
                       ext.OES_EGL_image_external_essl3)) {
        t->gl.targetIndex = (uint8_t)
                getIndexForTextureTarget(t->gl.target = GL_TEXTURE_EXTERNAL_OES);
    } else {
        t->gl.internalFormat = getInternalFormat(format);
        assert(t->gl.internalFormat);

        switch (target) {
            case SamplerType::SAMPLER_EXTERNAL:
                // if we get there, it's because the user is trying to use an external texture
                // but it's not supported, so instead, we behave like a texture2d.
                // fallthrough...
            case SamplerType::SAMPLER_2D:
                t->gl.targetIndex = (uint8_t)
                        getIndexForTextureTarget(t->gl.target = GL_TEXTURE_2D);
                break;
            case SamplerType::SAMPLER_CUBEMAP:
                t->gl.targetIndex = (uint8_t)
                        getIndexForTextureTarget(t->gl.target = GL_TEXTURE_CUBE_MAP);
                break;
        }

        if (t->samples > 1) {
            if (features.multisample_texture) {
                // multi-sample texture on GL 3.2 / GLES 3.1 and above
                t->gl.targetIndex = (uint8_t)
                        getIndexForTextureTarget(t->gl.target = GL_TEXTURE_2D_MULTISAMPLE);
            } else {
                // Turn off multi-sampling for that texture. This works because we're always only
                // doing a resolve-blit (as opposed to doing a manual resolve in the shader -- i.e.:
                // we're never attempt to use a multisample sampler).
                // When EXT_multisampled_render_to_texture is available, we handle  it in
                // framebufferTexture() below; if not, the resolve-blit becomes superfluous,
                // but at least it works.
            }
        }

        textureStorage(t, w, h, depth);
    }

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::framebufferTexture(Driver::TargetBufferInfo& binfo,
        GLRenderTarget* rt, GLenum attachment) noexcept {
    GLTexture const* t = handle_cast<const GLTexture*>(binfo.handle);

    assert(t->target != SamplerType::SAMPLER_EXTERNAL);

    bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);
    switch (t->target) {
        case SamplerType::SAMPLER_2D:
#if GLES31_HEADERS
            if (t->samples > 1 && !features.multisample_texture && ext.EXT_multisampled_render_to_texture) {
                // we have a multi-sample texture, but multi-sampled textures are not supported,
                // however, we have EXT_multisampled_render_to_texture -- phew.
                // In that case, we create a multi-sampled framebuffer into our regular texture.
                // Resolve happens automatically when sampling the texture.
                glext::glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER,
                        attachment, t->gl.target, t->gl.texture_id, binfo.level, t->samples);
            } else
#endif
            {
                // on GL3.2 / GLES3.1 and above multisample is handled when creating the texture.
                // If multisampled textures are not supported and we end-up here, things should
                // still work, albeit without MSAA.
                glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
                        t->gl.target, t->gl.texture_id, binfo.level);
            }
            break;
        case SamplerType::SAMPLER_CUBEMAP: {
            GLenum target = getCubemapTarget(binfo.face);
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
                    target, t->gl.texture_id, binfo.level);
            break;
        }
        case SamplerType::SAMPLER_EXTERNAL:
            // cannot happen by construction
            break;
    }

    CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e)
}

void OpenGLDriver::renderBufferStorage(GLuint rbo, GLenum internalformat, uint32_t width,
        uint32_t height, uint8_t samples) const noexcept {
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    if (samples > 1) {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalformat, width, height);
    } else {
        glRenderbufferStorage(GL_RENDERBUFFER, internalformat, width, height);
    }
}

void OpenGLDriver::framebufferRenderbuffer(GLRenderTarget::GL::RenderBuffer* rb, GLenum attachment,
        GLenum internalformat, uint32_t width, uint32_t height, uint8_t samples, GLuint fbo) noexcept {
    rb->id = framebufferRenderbuffer(width, height, samples, attachment, internalformat, fbo);
    rb->internalFormat = internalformat;
}

GLuint OpenGLDriver::framebufferRenderbuffer(uint32_t width, uint32_t height, uint8_t samples,
        GLenum attachment, GLenum internalformat, GLuint fbo) noexcept {

    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    renderBufferStorage(rbo, internalformat, width, height, samples);

    bindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rbo);

    CHECK_GL_ERROR(utils::slog.e)
    CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e)

    return rbo;
}

void OpenGLDriver::createDefaultRenderTarget(
        Driver::RenderTargetHandle rth, int) {
    DEBUG_MARKER()

    construct<GLRenderTarget>(rth, 0, 0);  // FIXME: we don't know the width/height

    uint32_t framebuffer, colorbuffer, depthbuffer;
    mPlatform.createDefaultRenderTarget(framebuffer, colorbuffer, depthbuffer);

    GLRenderTarget* rt = handle_cast<GLRenderTarget*>(rth);
    rt->gl.fbo = framebuffer;
    rt->gl.color.id = colorbuffer;
    rt->gl.depth.id = depthbuffer;
}

void OpenGLDriver::createRenderTarget(Driver::RenderTargetHandle rth,
        Driver::TargetBufferFlags targets,
        uint32_t width,
        uint32_t height,
        uint8_t samples,
        TextureFormat format,
        Driver::TargetBufferInfo color,
        Driver::TargetBufferInfo depth,
        Driver::TargetBufferInfo stencil) {
    DEBUG_MARKER()

    GLRenderTarget* rt = construct<GLRenderTarget>(rth, width, height);
    glGenFramebuffers(1, &rt->gl.fbo);

    /*
     * Special case for GLES 3.0 (i.e. multi-sample texture not supported):
     * When we get here, textures can't be multi-sample and we can't create a framebuffer with
     * heterogeneous attachments. In that case, we have no choice but to set the sample count to 1.
     *
     * However, if EXT_multisampled_render_to_texture is supported, the situation is different
     * because we can now create a framebuffer with a multi-sampled attachment for that texture.
     */
    if (samples > 1 && !features.multisample_texture && !ext.EXT_multisampled_render_to_texture) {
        if (color.handle || depth.handle || stencil.handle) {
            // do this only if a texture is used (in which case they'll all be single-sample)
            samples = 1;
        }
    }

    rt->gl.samples = samples;

    if (targets & TargetBufferFlags::COLOR) {
        // TODO: handle multiple color attachments
        if (color.handle) {
            rt->gl.color.texture = handle_cast<GLTexture*>(color.handle);
            framebufferTexture(color, rt, GL_COLOR_ATTACHMENT0);
        } else {
            GLenum internalFormat = getInternalFormat(format);
            framebufferRenderbuffer(&rt->gl.color, GL_COLOR_ATTACHMENT0, internalFormat,
                    width, height, samples, rt->gl.fbo);
        }
    }

    // handle special cases first (where depth/stencil are packed)
    bool specialCased = false;
    if ((targets & TargetBufferFlags::DEPTH_AND_STENCIL) == TargetBufferFlags::DEPTH_AND_STENCIL) {
        if (!depth.handle && !stencil.handle) {
            // special case: depth & stencil requested, but both not provided
            specialCased = true;
            framebufferRenderbuffer(&rt->gl.depth, GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8,
                    width, height, samples, rt->gl.fbo);

        } else if (depth.handle == stencil.handle) {
            // special case: depth & stencil requested, and both provided as the same texture
            rt->gl.depth.texture = handle_cast<GLTexture*>(depth.handle);
            specialCased = true;
            framebufferTexture(depth, rt, GL_DEPTH_STENCIL_ATTACHMENT);
        }
    }

    if (!specialCased) {
        if (targets & TargetBufferFlags::DEPTH) {
            if (depth.handle) {
                rt->gl.depth.texture = handle_cast<GLTexture*>(depth.handle);
                framebufferTexture(depth, rt, GL_DEPTH_ATTACHMENT);
            } else {
                framebufferRenderbuffer(&rt->gl.depth, GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT24,
                        width, height, samples, rt->gl.fbo);
            }
        }
        if (targets & TargetBufferFlags::STENCIL) {
            if (stencil.handle) {
                rt->gl.stencil.texture = handle_cast<GLTexture*>(stencil.handle);
                framebufferTexture(stencil, rt, GL_STENCIL_ATTACHMENT);
            } else {
                framebufferRenderbuffer(&rt->gl.stencil, GL_STENCIL_ATTACHMENT, GL_STENCIL_INDEX8,
                        width, height, samples, rt->gl.fbo);
            }
        }
    }

    // unbind the renderbuffer, to avoid any later confusion
    if (rt->gl.color.id || rt->gl.depth.id || rt->gl.stencil.id) {
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createFence(Driver::FenceHandle fh, int) {
    DEBUG_MARKER()

    HwFence* f = construct<HwFence>(fh);
    f->fence = mPlatform.createFence();
}

void OpenGLDriver::createSwapChain(Driver::SwapChainHandle sch, void* nativeWindow, uint64_t flags) {
    DEBUG_MARKER()

    HwSwapChain* sc = construct<HwSwapChain>(sch);
    sc->swapChain = mPlatform.createSwapChain(nativeWindow, flags);
}

void OpenGLDriver::createStreamFromTextureId(Driver::StreamHandle sh,
        intptr_t externalTextureId, uint32_t width, uint32_t height) {
    DEBUG_MARKER()

    GLStream* s = construct<GLStream>(sh);
    // It would be better if we could query the externalTextureId size, unfortunately
    // this is not supported in GL for GL_TEXTURE_EXTERNAL_OES targets
    s->width = width;
    s->height = height;
    s->gl.externalTextureId = static_cast<GLuint>(externalTextureId);
    glGenTextures(GLStream::ROUND_ROBIN_TEXTURE_COUNT, s->user_thread.read);
    glGenTextures(GLStream::ROUND_ROBIN_TEXTURE_COUNT, s->user_thread.write);
    for (auto& info : s->user_thread.infos) {
        info.ets = mPlatform.createExternalTextureStorage();
    }
}

// ------------------------------------------------------------------------------------------------
// Destroying driver objects
// ------------------------------------------------------------------------------------------------

void OpenGLDriver::destroyVertexBuffer(Driver::VertexBufferHandle vbh) {
    DEBUG_MARKER()

    if (vbh) {
        GLVertexBuffer const* eb = handle_cast<const GLVertexBuffer*>(vbh);
        GLsizei n = GLsizei(eb->bufferCount);
        glDeleteBuffers(n, eb->gl.buffers.data());
        // bindings of bound buffers are reset to 0
        const size_t targetIndex = getIndexForBufferTarget(GL_ARRAY_BUFFER);
        auto& target = state.buffers.genericBinding[targetIndex];
        #pragma nounroll
        for (GLuint b : eb->gl.buffers) {
            if (target == b) {
                target = 0;
            }
        }
        destruct(vbh, eb);
    }
}

void OpenGLDriver::destroyIndexBuffer(Driver::IndexBufferHandle ibh) {
    DEBUG_MARKER()

    if (ibh) {
        GLIndexBuffer const* ib = handle_cast<const GLIndexBuffer*>(ibh);
        glDeleteBuffers(1, &ib->gl.buffer);
        // bindings of bound buffers are reset to 0
        const size_t targetIndex = getIndexForBufferTarget(GL_ELEMENT_ARRAY_BUFFER);
        auto& target = state.buffers.genericBinding[targetIndex];
        if (target == ib->gl.buffer) {
            target = 0;
        }
        destruct(ibh, ib);
    }
}

void OpenGLDriver::destroyRenderPrimitive(Driver::RenderPrimitiveHandle rph) {
    DEBUG_MARKER()

    if (rph) {
        GLRenderPrimitive const* rp = handle_cast<const GLRenderPrimitive*>(rph);
        glDeleteVertexArrays(1, &rp->gl.vao);
        // binding of a bound VAO is reset to 0
        if (state.vao.p == rp) {
            state.vao.p = &mDefaultVAO;
        }
        destruct(rph, rp);
    }
}

void OpenGLDriver::destroyProgram(Driver::ProgramHandle ph) {
    DEBUG_MARKER()

    if (ph) {
        OpenGLProgram* p = handle_cast<OpenGLProgram*>(ph);
        destruct(ph, p);
    }
}

void OpenGLDriver::destroySamplerBuffer(Driver::SamplerBufferHandle sbh) {
    DEBUG_MARKER()

    if (sbh) {
        GLSamplerBuffer* sb = handle_cast<GLSamplerBuffer*>(sbh);
        destruct(sbh, sb);
    }
}

void OpenGLDriver::destroyUniformBuffer(Driver::UniformBufferHandle ubh) {
    DEBUG_MARKER()

    if (ubh) {
        GLUniformBuffer* ub = handle_cast<GLUniformBuffer*>(ubh);
        glDeleteBuffers(1, &ub->gl.ubo.id);
        // bindings of bound buffers are reset to 0
        const size_t targetIndex = getIndexForBufferTarget(GL_UNIFORM_BUFFER);
        auto& target = state.buffers.targets[targetIndex];

        #pragma nounroll // clang generates >1 KiB of code!!
        for (auto& buffer : target.buffers) {
            if (buffer.name == ub->gl.ubo.id) {
                buffer.name = 0;
                buffer.offset = 0;
                buffer.size = 0;
            }
        }
        if (state.buffers.genericBinding[targetIndex] == ub->gl.ubo.id) {
            state.buffers.genericBinding[targetIndex] = 0;
        }
        destruct(ubh, ub);
    }
}

void OpenGLDriver::destroyTexture(Driver::TextureHandle th) {
    DEBUG_MARKER()

    if (th) {
        GLTexture* t = handle_cast<GLTexture*>(th);
        unbindTexture(t->gl.target, t->gl.texture_id);
        if (UTILS_UNLIKELY(t->hwStream)) {
            detachStream(t);
        }
        if (t->gl.fence) {
            glDeleteSync(t->gl.fence);
        }
        glDeleteTextures(1, &t->gl.texture_id);
        destruct(th, t);
    }
}

void OpenGLDriver::destroyRenderTarget(Driver::RenderTargetHandle rth) {
    DEBUG_MARKER()

    if (rth) {
        GLRenderTarget* rt = handle_cast<GLRenderTarget*>(rth);
        if (rt->gl.fbo) {
            // first unbind this framebuffer if needed
            bindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        if (rt->gl.color.id) {
            // delete color  renderbuffer if needed
            glDeleteRenderbuffers(1, &rt->gl.color.id);
        }
        if (rt->gl.depth.id) {
            // delete depth (or depth-stencil) renderbuffer if needed
            glDeleteRenderbuffers(1, &rt->gl.depth.id);
        }
        if (rt->gl.stencil.id) {
            // delete stencil renderbuffer if needed
            glDeleteRenderbuffers(1, &rt->gl.stencil.id);
        }
        if (rt->gl.fbo) {
            // finally delete the framebuffer object
            glDeleteFramebuffers(1, &rt->gl.fbo);
        }
        destruct(rth, rt);
    }
}

void OpenGLDriver::destroySwapChain(Driver::SwapChainHandle sch) {
    DEBUG_MARKER()

    if (sch) {
        HwSwapChain* sc = handle_cast<HwSwapChain*>(sch);
        mPlatform.destroySwapChain(sc->swapChain);
        destruct(sch, sc);
    }
}

void OpenGLDriver::destroyStream(Driver::StreamHandle sh) {
    DEBUG_MARKER()

    if (sh) {
        GLStream* s = handle_cast<GLStream*>(sh);

        // if this stream is still attached to a texture, detach it first
        auto& externalStreams = mExternalStreams;
        auto pos = std::find_if(externalStreams.begin(), externalStreams.end(),
                [s](GLTexture const* t) { return t->hwStream == s; });
        if (pos != externalStreams.end()) {
            detachStream(*pos);
        }
        if (s->isNativeStream()) {
            mPlatform.destroyStream(s->stream);
        } else {
            glDeleteTextures(GLStream::ROUND_ROBIN_TEXTURE_COUNT, s->user_thread.read);
            glDeleteTextures(GLStream::ROUND_ROBIN_TEXTURE_COUNT, s->user_thread.write);
            if (s->gl.fbo) {
                glDeleteFramebuffers(1, &s->gl.fbo);
            }
            for (auto const& info : s->user_thread.infos) {
                mPlatform.destroyExternalTextureStorage(info.ets);
            }
        }
        destruct(sh, s);
    }
}

// ------------------------------------------------------------------------------------------------
// Synchronous APIs
// These are called on the application's thread
// ------------------------------------------------------------------------------------------------

Handle<HwStream> OpenGLDriver::createStream(void* nativeStream) {
    Handle<HwStream> sh( allocateHandle(sizeof(GLStream)) );
    Platform::Stream* stream = mPlatform.createStream(nativeStream);
    construct<GLStream>(sh, stream);
    return sh;
}

void OpenGLDriver::updateStreams(driver::DriverApi* driver) {
    if (UTILS_UNLIKELY(!mExternalStreams.empty())) {
        OpenGLBlitter::State state;
        for (GLTexture* t : mExternalStreams) {
            assert(t && t->hwStream);
            if (!static_cast<GLStream*>(t->hwStream)->isNativeStream()) {
                state.setup();
                updateStream(t, driver);
            }
        }
    }
}

void OpenGLDriver::setStreamDimensions(Driver::StreamHandle sh, uint32_t width, uint32_t height) {
    if (sh) {
        GLStream* s = handle_cast<GLStream*>(sh);
        s->width = width;
        s->height = height;
    }
}

int64_t OpenGLDriver::getStreamTimestamp(Driver::StreamHandle sh) {
    if (sh) {
        GLStream* s = handle_cast<GLStream*>(sh);
        return s->user_thread.timestamp;
    }
    return 0;
}

void OpenGLDriver::destroyFence(Driver::FenceHandle fh) {
    if (fh) {
        HwFence* f = handle_cast<HwFence*>(fh);
        mPlatform.destroyFence(f->fence);
        destruct(fh, f);
    }
}

Driver::FenceStatus OpenGLDriver::wait(Driver::FenceHandle fh, uint64_t timeout) {
    if (fh) {
        HwFence* f = handle_cast<HwFence*>(fh);
        return mPlatform.waitFence(f->fence, timeout);
    }
    return FenceStatus::ERROR;
}

bool OpenGLDriver::isTextureFormatSupported(Driver::TextureFormat format) {
    if (driver::isETC2Compression(format)) {
        return ext.texture_compression_etc2;
    }
    if (driver::isS3TCCompression(format)) {
        return ext.texture_compression_s3tc;
    }
    return getInternalFormat(format) != 0;
}

bool OpenGLDriver::isRenderTargetFormatSupported(Driver::TextureFormat format) {
    // Supported formats per http://docs.gl/es3/glRenderbufferStorage, note that desktop OpenGL may
    // support more formats, but it requires querying GL_INTERNALFORMAT_SUPPORTED which is not
    // available in OpenGL ES.
    switch (format) {
        // Core formats.
        case TextureFormat::R8:
        case TextureFormat::R8UI:
        case TextureFormat::R8I:
        case TextureFormat::STENCIL8:
        case TextureFormat::R16UI:
        case TextureFormat::R16I:
        case TextureFormat::RG8:
        case TextureFormat::RG8UI:
        case TextureFormat::RG8I:
        case TextureFormat::RGB565:
        case TextureFormat::RGB5_A1:
        case TextureFormat::RGBA4:
        case TextureFormat::DEPTH16:
        case TextureFormat::RGB8:
        case TextureFormat::DEPTH24:
        case TextureFormat::R32UI:
        case TextureFormat::R32I:
        case TextureFormat::RG16UI:
        case TextureFormat::RG16I:
        case TextureFormat::RGBA8:
        case TextureFormat::SRGB8_A8:
        case TextureFormat::RGB10_A2:
        case TextureFormat::RGBA8UI:
        case TextureFormat::RGBA8I:
        case TextureFormat::DEPTH32F:
        case TextureFormat::DEPTH24_STENCIL8:
        case TextureFormat::DEPTH32F_STENCIL8:
        case TextureFormat::RG32UI:
        case TextureFormat::RG32I:
        case TextureFormat::RGBA16UI:
        case TextureFormat::RGBA16I:
            return true;

        // Half-float formats, requires extension.
        case TextureFormat::RGB16F:
            return ext.EXT_color_buffer_half_float;

        // Float formats from GL_EXT_color_buffer_float, assumed supported.
        case TextureFormat::R16F:
        case TextureFormat::RG16F:
        case TextureFormat::RGBA16F:
        case TextureFormat::R32F:
        case TextureFormat::RG32F:
        case TextureFormat::RGBA32F:
        case TextureFormat::R11F_G11F_B10F:
            return true;

        default:
            return false;
    }
}

bool OpenGLDriver::isFrameTimeSupported() {
    // TODO: Measuring the frame time is currently only done using fences
    return mPlatform.canCreateFence();
}

// ------------------------------------------------------------------------------------------------
// Swap chains
// ------------------------------------------------------------------------------------------------


void OpenGLDriver::commit(Driver::SwapChainHandle sch) {
    DEBUG_MARKER()

    if (sch) {
        HwSwapChain* sc = handle_cast<HwSwapChain*>(sch);
        mPlatform.commit(sc->swapChain);
    }
}

void OpenGLDriver::makeCurrent(Driver::SwapChainHandle schDraw, Driver::SwapChainHandle schRead) {
    DEBUG_MARKER()

    if (schDraw && schRead) {
        HwSwapChain* scDraw = handle_cast<HwSwapChain*>(schDraw);
        HwSwapChain* scRead = handle_cast<HwSwapChain*>(schRead);
        mPlatform.makeCurrent(scDraw->swapChain, scRead->swapChain);
    }
}

// ------------------------------------------------------------------------------------------------
// Updating driver objects
// ------------------------------------------------------------------------------------------------

void OpenGLDriver::updateVertexBuffer(
        Driver::VertexBufferHandle vbh,
        size_t index,
        BufferDescriptor&& p,
        uint32_t byteOffset,
        uint32_t byteSize) {
    DEBUG_MARKER()

    GLVertexBuffer* eb = handle_cast<GLVertexBuffer *>(vbh);

    bindBuffer(GL_ARRAY_BUFFER, eb->gl.buffers[index]);
    glBufferSubData(GL_ARRAY_BUFFER, byteOffset, byteSize, p.buffer);

    scheduleDestroy(std::move(p));

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::updateIndexBuffer(
        Driver::IndexBufferHandle ibh,
        BufferDescriptor&& p, uint32_t byteOffset, uint32_t byteSize) {
    DEBUG_MARKER()

    GLIndexBuffer* ib = handle_cast<GLIndexBuffer *>(ibh);
    assert(ib->elementSize == 2 || ib->elementSize == 4);

    bindVertexArray(nullptr);
    bindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->gl.buffer);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, byteOffset, byteSize, p.buffer);

    scheduleDestroy(std::move(p));

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::updateUniformBuffer(Driver::UniformBufferHandle ubh, BufferDescriptor&& p) {
    DEBUG_MARKER()

    GLUniformBuffer* ub = handle_cast<GLUniformBuffer *>(ubh);
    assert(ub);

    if (p.size > 0) {
        updateBuffer(GL_UNIFORM_BUFFER, &ub->gl.ubo, p,
                (uint32_t)gets.uniform_buffer_offset_alignment);
    }
    scheduleDestroy(std::move(p));
}

void OpenGLDriver::updateBuffer(GLenum target,
        GLBuffer* buffer, BufferDescriptor const& p, uint32_t alignment) noexcept {
    assert(buffer->capacity >= p.size);
    assert(buffer->id);

    bindBuffer(target, buffer->id);
    if (buffer->usage == driver::BufferUsage::STREAM) {

        buffer->size = (uint32_t)p.size;

        // If MapBufferRange is supported, then attempt to use that instead of BufferSubData, which
        // can be quite inefficient on some platforms. Note that WebGL does not support
        // MapBufferRange, but we still allow STREAM semantics for the web platform.
        if (HAS_MAPBUFFERS) {
            uint32_t offset = buffer->base + buffer->size;
            offset = (offset + (alignment - 1u)) & ~(alignment - 1u);

            if (offset + p.size > buffer->capacity) {
                // if we've reached the end of the buffer, we orphan it and allocate a new one.
                // this is assuming the driver actually does that as opposed to stalling. This is
                // the case for Mali and Adreno -- we could use fences instead.
                offset = 0;
                glBufferData(target, buffer->capacity, nullptr, getBufferUsage(buffer->usage));
            }
    retry:
            void* vaddr = glMapBufferRange(target, offset, p.size,
                    GL_MAP_WRITE_BIT |
                    GL_MAP_INVALIDATE_RANGE_BIT |
                    GL_MAP_UNSYNCHRONIZED_BIT);
            if (vaddr) {
                memcpy(vaddr, p.buffer, p.size);
                if (glUnmapBuffer(target) == GL_FALSE) {
                    // According to the spec, UnmapBuffer can return FALSE in rare conditions (e.g.
                    // during a screen mode change). Note that is not a GL error, and we can handle
                    // it by simply making a second attempt.
                    goto retry;
                }
            } else {
                // handle mapping error, revert to glBufferSubData()
                glBufferSubData(target, offset, p.size, p.buffer);
            }
            buffer->base = offset;

            CHECK_GL_ERROR(utils::slog.e)
            return;
        }
    }

    if (p.size == buffer->capacity) {
        // it looks like it's generally faster (or not worse) to use glBufferData()
        glBufferData(target, buffer->capacity, p.buffer, getBufferUsage(buffer->usage));
    } else {
        // glBufferSubData() could be catastrophically inefficient if several are issued
        // during the same frame. Currently, we're not doing that though.
        glBufferSubData(target, 0, p.size, p.buffer);
    }

    CHECK_GL_ERROR(utils::slog.e)
}


void OpenGLDriver::updateSamplerBuffer(Driver::SamplerBufferHandle sbh,
        SamplerBuffer&& samplerBuffer) {
    DEBUG_MARKER()

    GLSamplerBuffer* sb = handle_cast<GLSamplerBuffer *>(sbh);
    *sb->sb = std::move(samplerBuffer); // NOLINT(performance-move-const-arg)
}

void OpenGLDriver::update2DImage(Driver::TextureHandle th,
        uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& data) {
    DEBUG_MARKER()

    GLTexture* t = handle_cast<GLTexture *>(th);
    if (data.type == driver::PixelDataType::COMPRESSED) {
        setCompressedTextureData(t,
                level, xoffset, yoffset, 0, width, height, 1, std::move(data), nullptr);
    } else {
        setTextureData(t,
                level, xoffset, yoffset, 0, width, height, 1, std::move(data), nullptr);
    }
}

void OpenGLDriver::updateCubeImage(Driver::TextureHandle th, uint32_t level,
        PixelBufferDescriptor&& data, FaceOffsets faceOffsets) {
    DEBUG_MARKER()

    GLTexture* t = handle_cast<GLTexture *>(th);
    if (data.type == driver::PixelDataType::COMPRESSED) {
        setCompressedTextureData(t, level, 0, 0, 0, 0, 0, 0, std::move(data), &faceOffsets);
    } else {
        setTextureData(t, level, 0, 0, 0, 0, 0, 0, std::move(data), &faceOffsets);
    }
}

void OpenGLDriver::generateMipmaps(Driver::TextureHandle th) {
    DEBUG_MARKER()

    GLTexture* t = handle_cast<GLTexture *>(th);
    assert(t->gl.target != GL_TEXTURE_2D_MULTISAMPLE);
    // Note: glGenerateMimap can also fail if the internal format is not both
    // color-renderable and filterable (i.e.: doesn't work for depth)
    bindTexture(MAX_TEXTURE_UNITS - 1, t->gl.target, t, t->gl.targetIndex);
    activeTexture(MAX_TEXTURE_UNITS - 1);
    glGenerateMipmap(t->gl.target);

    t->gl.baseLevel = 0;
    t->gl.maxLevel = static_cast<uint8_t>(t->levels - 1);

    glTexParameteri(t->gl.target, GL_TEXTURE_BASE_LEVEL, t->gl.baseLevel);
    glTexParameteri(t->gl.target, GL_TEXTURE_MAX_LEVEL, t->gl.maxLevel);

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::setTextureData(GLTexture* t,
                                  uint32_t level,
                                  uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
                                  uint32_t width, uint32_t height, uint32_t depth,
                                  PixelBufferDescriptor&& p, FaceOffsets const* faceOffsets) {
    DEBUG_MARKER()

    assert(xoffset + width <= t->width >> level);
    assert(yoffset + height <= t->height >> level);
    assert(zoffset + depth <= t->depth);
    assert(t->samples <= 1);

    if (UTILS_UNLIKELY(t->gl.target == GL_TEXTURE_EXTERNAL_OES)) {
        // this is in fact an external texture, this becomes a no-op.
        return;
    }

    GLenum glFormat = getFormat(p.format);
    GLenum glType = getType(p.type);

    pixelStore(GL_UNPACK_ROW_LENGTH, p.stride);
    pixelStore(GL_UNPACK_ALIGNMENT, p.alignment);
    pixelStore(GL_UNPACK_SKIP_PIXELS, p.left);
    pixelStore(GL_UNPACK_SKIP_ROWS, p.top);

    switch (t->target) {
        case SamplerType::SAMPLER_EXTERNAL:
            // if we get there, it's because the user is trying to use an external texture
            // but it's not supported, so instead, we behave like a texture2d.
            // fallthrough...
        case SamplerType::SAMPLER_2D:
            // NOTE: GL_TEXTURE_2D_MULTISAMPLE is not allowed
            assert(t->gl.target == GL_TEXTURE_2D);
            bindTexture(MAX_TEXTURE_UNITS - 1, GL_TEXTURE_2D, t);
            activeTexture(MAX_TEXTURE_UNITS - 1);
            glTexSubImage2D(GL_TEXTURE_2D,
                    GLint(level), GLint(xoffset), GLint(yoffset),
                    width, height, glFormat, glType, p.buffer);
            break;
        case SamplerType::SAMPLER_CUBEMAP: {
            assert(t->gl.target == GL_TEXTURE_CUBE_MAP);
            bindTexture(MAX_TEXTURE_UNITS - 1, GL_TEXTURE_CUBE_MAP, t);
            activeTexture(MAX_TEXTURE_UNITS - 1);
            FaceOffsets const& offsets = *faceOffsets;
#pragma nounroll
            for (size_t face = 0; face < 6; face++) {
                GLenum target = getCubemapTarget(TextureCubemapFace(face));
                glTexSubImage2D(target, GLint(level), 0, 0,
                        t->width >> level, t->height >> level, glFormat, glType,
                        static_cast<uint8_t const*>(p.buffer) + offsets[face]);
            }
            break;
        }
    }

    // update the base/max LOD so we don't access undefined LOD. this allows the app to
    // specify levels as they become available.

    if (uint8_t(level) < t->gl.baseLevel) {
        t->gl.baseLevel = uint8_t(level);
        glTexParameteri(t->gl.target, GL_TEXTURE_BASE_LEVEL, t->gl.baseLevel);
    }
    if (uint8_t(level) > t->gl.maxLevel) {
        t->gl.maxLevel = uint8_t(level);
        glTexParameteri(t->gl.target, GL_TEXTURE_MAX_LEVEL, t->gl.maxLevel);
    }

    scheduleDestroy(std::move(p));

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::setCompressedTextureData(GLTexture* t,
                                            uint32_t level,
                                            uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
                                            uint32_t width, uint32_t height, uint32_t depth,
                                            PixelBufferDescriptor&& p, FaceOffsets const* faceOffsets) {
    DEBUG_MARKER()

    assert(xoffset + width <= t->width >> level);
    assert(yoffset + height <= t->height >> level);
    assert(zoffset + depth <= t->depth);
    assert(t->samples <= 1);

    if (UTILS_UNLIKELY(t->gl.target == GL_TEXTURE_EXTERNAL_OES)) {
        // this is in fact an external texture, this becomes a no-op.
        return;
    }

    // TODO: maybe assert that the CompressedPixelDataType is the same than the internalFormat

    GLsizei imageSize = GLsizei(p.imageSize);

    //  TODO: maybe assert the size is right (b/c we can compute it ourselves)

    switch (t->target) {
        case SamplerType::SAMPLER_EXTERNAL:
            // if we get there, it's because the user is trying to use an external texture
            // but it's not supported, so instead, we behave like a texture2d.
            // fallthrough...
        case SamplerType::SAMPLER_2D:
            // NOTE: GL_TEXTURE_2D_MULTISAMPLE is not allowed
            assert(t->gl.target == GL_TEXTURE_2D);
            bindTexture(MAX_TEXTURE_UNITS-1, GL_TEXTURE_2D, t);
            activeTexture(MAX_TEXTURE_UNITS - 1);
            glCompressedTexSubImage2D(GL_TEXTURE_2D,
                    GLint(level), GLint(xoffset), GLint(yoffset),
                    width, height, t->gl.internalFormat, imageSize, p.buffer);
            break;
        case SamplerType::SAMPLER_CUBEMAP: {
            assert(faceOffsets);
            assert(t->gl.target == GL_TEXTURE_CUBE_MAP);
            bindTexture(MAX_TEXTURE_UNITS - 1, GL_TEXTURE_CUBE_MAP, t);
            activeTexture(MAX_TEXTURE_UNITS - 1);
            FaceOffsets const& offsets = *faceOffsets;
#pragma nounroll
            for (size_t face = 0; face < 6; face++) {
                GLenum target = getCubemapTarget(TextureCubemapFace(face));
                glCompressedTexSubImage2D(target, GLint(level), 0, 0,
                        t->width >> level, t->height >> level, t->gl.internalFormat,
                        imageSize, static_cast<uint8_t const*>(p.buffer) + offsets[face]);
            }
            break;
        }
    }

    // update the base/max LOD so we don't access undefined LOD. this allows the app to
    // specify levels as they become available.

    if (uint8_t(level) < t->gl.baseLevel) {
        t->gl.baseLevel = uint8_t(level);
        glTexParameteri(t->gl.target, GL_TEXTURE_BASE_LEVEL, t->gl.baseLevel);
    }
    if (uint8_t(level) > t->gl.maxLevel) {
        t->gl.maxLevel = uint8_t(level);
        glTexParameteri(t->gl.target, GL_TEXTURE_MAX_LEVEL, t->gl.maxLevel);
    }

    scheduleDestroy(std::move(p));

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::setExternalImage(Driver::TextureHandle th, void* image) {
    if (ext.OES_EGL_image_external_essl3) {
        DEBUG_MARKER()

        GLTexture* t = handle_cast<GLTexture*>(th);

        assert(t->target == SamplerType::SAMPLER_EXTERNAL);
        assert(t->gl.target == GL_TEXTURE_EXTERNAL_OES);

        bindTexture(MAX_TEXTURE_UNITS - 1, GL_TEXTURE_EXTERNAL_OES, t);
        activeTexture(MAX_TEXTURE_UNITS - 1);

#ifdef GL_OES_EGL_image
        glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, static_cast<GLeglImageOES>(image));
#endif
    }
}

void OpenGLDriver::setExternalStream(Driver::TextureHandle th, Driver::StreamHandle sh) {
    if (ext.OES_EGL_image_external_essl3) {
        DEBUG_MARKER()

        GLTexture* t = handle_cast<GLTexture*>(th);
        if (UTILS_LIKELY(sh)) {
            GLStream* s = handle_cast<GLStream*>(sh);
            if (UTILS_LIKELY(!t->hwStream)) {
                // we're not attached alread
                attachStream(t, s);
            } else {
                if (s->stream != t->hwStream->stream) {
                    // attaching to a different stream, detach the old one first
                    replaceStream(t, s);
                }
            }
        } else if (t->hwStream) {
            // do nothing if we're not attached already
            detachStream(t);
        }
    }
}

UTILS_NOINLINE
void OpenGLDriver::attachStream(GLTexture* t, GLStream* hwStream) noexcept {
    mExternalStreams.push_back(t);

    if (hwStream->isNativeStream()) {
        mPlatform.attach(hwStream->stream, t->gl.texture_id);
    } else {
        assert(t->target == SamplerType::SAMPLER_EXTERNAL);
        // The texture doesn't need a texture name anymore, get rid of it
        unbindTexture(t->gl.target, t->gl.texture_id);
        glDeleteTextures(1, &t->gl.texture_id);
        t->gl.texture_id = hwStream->user_thread.read[hwStream->user_thread.cur];
    }
    t->hwStream = hwStream;
}

UTILS_NOINLINE
void OpenGLDriver::detachStream(GLTexture* t) noexcept {
    auto& streams = mExternalStreams;
    auto pos = std::find(streams.begin(), streams.end(), t);
    if (pos != streams.end()) {
        streams.erase(pos);
    }

    GLStream* s = static_cast<GLStream*>(t->hwStream);
    if (s->isNativeStream()) {
        mPlatform.detach(t->hwStream->stream);
        // this deletes the texture id
    }
    glGenTextures(1, &t->gl.texture_id);
    t->hwStream = nullptr;
}

UTILS_NOINLINE
void OpenGLDriver::replaceStream(GLTexture* t, GLStream* hwStream) noexcept {
    GLStream* s = static_cast<GLStream*>(t->hwStream);
    if (s->isNativeStream()) {
        mPlatform.detach(t->hwStream->stream);
        // this deletes the texture id
    }

    if (hwStream->isNativeStream()) {
        glGenTextures(1, &t->gl.texture_id);
        mPlatform.attach(hwStream->stream, t->gl.texture_id);
    } else {
        assert(t->target == SamplerType::SAMPLER_EXTERNAL);
        t->gl.texture_id = hwStream->user_thread.read[hwStream->user_thread.cur];
    }
    t->hwStream = hwStream;
}

void OpenGLDriver::beginRenderPass(Driver::RenderTargetHandle rth,
        const Driver::RenderPassParams& params) {
    DEBUG_MARKER()

    mRenderPassTarget = rth;
    mRenderPassParams = params;
    const TargetBufferFlags clearFlags = (TargetBufferFlags) params.clear;
    const TargetBufferFlags discardFlags = (TargetBufferFlags) params.discardStart;

    GLRenderTarget* rt = handle_cast<GLRenderTarget*>(rth);
    if (UTILS_UNLIKELY(state.draw_fbo != rt->gl.fbo)) {
        bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);

        // glInvalidateFramebuffer appeared on GLES 3.0 and GL4.3, for simplicity we just
        // ignore it on GL (rather than having to do a runtime check).
        if (GLES31_HEADERS && !bugs.disable_invalidate_framebuffer) {
            std::array<GLenum, 3> attachments; // NOLINT(cppcoreguidelines-pro-type-member-init)
            GLsizei attachmentCount = getAttachments(attachments, rt, discardFlags);
            if (attachmentCount) {
#if DEBUG_MARKER_LEVEL == DEBUG_MARKER_SYSTRACE
                SYSTRACE_NAME("glInvalidateFramebuffer");
#endif
                glInvalidateFramebuffer(GL_FRAMEBUFFER, attachmentCount, attachments.data());
                CHECK_GL_ERROR(utils::slog.e)
            }
        }
    }

    if (!(clearFlags & RenderPassParams::IGNORE_VIEWPORT)) {
        viewport(params.left, params.bottom, params.width, params.height);
    }

    const bool respectScissor = !(clearFlags & RenderPassParams::IGNORE_SCISSOR);
    const bool clearColor = clearFlags & TargetBufferFlags::COLOR;
    const bool clearDepth = clearFlags & TargetBufferFlags::DEPTH;
    const bool clearStencil = clearFlags & TargetBufferFlags::STENCIL;
    if (respectScissor && GLES31_HEADERS && bugs.clears_hurt_performance) {
        // With OpenGL ES, we clear the viewport using geometry to improve performance on certain
        // OpenGL drivers. e.g. on Adreno this avoids needless loads from the GMEM.
        clearWithGeometryPipe(clearColor, params.clearColor,
                clearDepth, params.clearDepth,
                clearStencil, params.clearStencil);
    } else {
        if (!respectScissor) {
            disable(GL_SCISSOR_TEST);
        }
        // With OpenGL we always clear using glClear()
        clearWithRasterPipe(clearColor, params.clearColor,
                clearDepth, params.clearDepth,
                clearStencil, params.clearStencil);
        if (!respectScissor) {
            enable(GL_SCISSOR_TEST);
        }
    }
}

void OpenGLDriver::endRenderPass(int) {
    DEBUG_MARKER()
    assert(mRenderPassTarget);
    const TargetBufferFlags discardFlags = (TargetBufferFlags) mRenderPassParams.discardEnd;
    // glInvalidateFramebuffer appeared on GLES 3.0 and GL4.3, for simplicity we just
    // ignore it on GL (rather than having to do a runtime check).
    if (GLES31_HEADERS && !bugs.disable_invalidate_framebuffer) {
        // we wouldn't have to bind the framebuffer if we had glInvalidateNamedFramebuffer()
        GLRenderTarget* rt = handle_cast<GLRenderTarget*>(mRenderPassTarget);
        bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);

        std::array<GLenum, 3> attachments; // NOLINT(cppcoreguidelines-pro-type-member-init)
        GLsizei attachmentCount = getAttachments(attachments, rt, discardFlags);
        if (attachmentCount) {
#if DEBUG_MARKER_LEVEL == DEBUG_MARKER_SYSTRACE
            SYSTRACE_NAME("glInvalidateFramebuffer");
#endif
            glInvalidateFramebuffer(GL_FRAMEBUFFER, attachmentCount, attachments.data());
        }

        CHECK_GL_ERROR(utils::slog.e)
    }
    mRenderPassTarget.clear();
}

void OpenGLDriver::discardSubRenderTargetBuffers(Driver::RenderTargetHandle rth,
        Driver::TargetBufferFlags buffers,
        uint32_t left, uint32_t bottom, uint32_t width, uint32_t height) {
    DEBUG_MARKER()

    // glInvalidateSubFramebuffer appeared on GLES 3.0 and GL4.3, for simplicity we just
    // ignore it on GL (rather than having to do a runtime check).
    if (GLES31_HEADERS) {
        // we wouldn't have to bind the framebuffer if we had glInvalidateNamedFramebuffer()
        GLRenderTarget const* rt = handle_cast<GLRenderTarget const*>(rth);

        // clamping is necessary to avoid GL_INVALID_VALUE
        uint32_t right = std::min(left + width, rt->width);
        uint32_t top   = std::min(bottom + height, rt->height);

        if (left < right && bottom < top) {
            bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);

            std::array<GLenum, 3> attachments; // NOLINT(cppcoreguidelines-pro-type-member-init)
            GLsizei attachmentCount = getAttachments(attachments, rt, buffers);
            if (attachmentCount) {
                glInvalidateSubFramebuffer(GL_FRAMEBUFFER, attachmentCount, attachments.data(),
                        left, bottom, right - left, top - bottom);
            }

            CHECK_GL_ERROR(utils::slog.e)
        }
    }
}

GLsizei OpenGLDriver::getAttachments(std::array<GLenum, 3>& attachments,
        GLRenderTarget const* rt, uint8_t buffers) const noexcept {
    GLsizei attachmentCount = 0;
    // the default framebuffer uses different constants!!!
    const bool defaultFramebuffer = (rt->gl.fbo == 0);
    if (buffers & TargetBufferFlags::COLOR) {
        attachments[attachmentCount++] = defaultFramebuffer ? GL_COLOR : GL_COLOR_ATTACHMENT0;
    }
    if (buffers & TargetBufferFlags::DEPTH) {
        attachments[attachmentCount++] = defaultFramebuffer ? GL_DEPTH : GL_DEPTH_ATTACHMENT;
    }
    if (buffers & TargetBufferFlags::STENCIL) {
        attachments[attachmentCount++] = defaultFramebuffer ? GL_STENCIL : GL_STENCIL_ATTACHMENT;
    }
    return attachmentCount;
}

void OpenGLDriver::resizeRenderTarget(Driver::RenderTargetHandle rth,
        uint32_t width, uint32_t height) {
    DEBUG_MARKER()

    GLRenderTarget* rt = handle_cast<GLRenderTarget*>(rth);

    // can't resize default FBO
    assert(rt->gl.fbo);

    if (rt->gl.color.id) {
        // if we have a depth renderbuffer, reallocate it
        renderBufferStorage(rt->gl.color.id, rt->gl.color.internalFormat, width, height, rt->gl.samples);
    } else if (rt->gl.color.texture) {
        // if it was a texture, reallocate the texture and discard content
        textureStorage(rt->gl.color.texture, width, height, rt->gl.color.texture->depth);
    }

    if (rt->gl.depth.id) {
        // if we have a depth renderbuffer, reallocate it
        renderBufferStorage(rt->gl.depth.id, rt->gl.depth.internalFormat, width, height, rt->gl.samples);
    } else if (rt->gl.depth.texture) {
        // if it was a texture, reallocate the texture and discard content
        textureStorage(rt->gl.depth.texture, width, height, rt->gl.depth.texture->depth);
    }

    if (rt->gl.stencil.id) {
        // if we have a stencil renderbuffer, reallocate it
        renderBufferStorage(rt->gl.stencil.id, rt->gl.stencil.internalFormat, width, height, rt->gl.samples);
    } else if (rt->gl.stencil.texture) {
        // if it was a texture, reallocate the texture and discard content
        textureStorage(rt->gl.stencil.texture, width, height, rt->gl.stencil.texture->depth);
    }

    // unbind the renderbuffer, to avoid any later confusion
    if (rt->gl.color.id || rt->gl.depth.id || rt->gl.stencil.id) {
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
}

void OpenGLDriver::setRenderPrimitiveBuffer(Driver::RenderPrimitiveHandle rph,
        Driver::VertexBufferHandle vbh, Driver::IndexBufferHandle ibh,
        uint32_t enabledAttributes) {
    DEBUG_MARKER()

    if (rph) {
        GLRenderPrimitive* const rp = handle_cast<GLRenderPrimitive*>(rph);
        GLVertexBuffer const* const eb = handle_cast<const GLVertexBuffer*>(vbh);
        GLIndexBuffer const* const ib = handle_cast<const GLIndexBuffer*>(ibh);

        assert(ib->elementSize == 2 || ib->elementSize == 4);

        bindVertexArray(rp);
        CHECK_GL_ERROR(utils::slog.e)

        rp->gl.indicesType = ib->elementSize == 4 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
        rp->maxVertexCount = eb->vertexCount;
        for (size_t i = 0, n = eb->attributes.size(); i < n; i++) {
            if (enabledAttributes & (1U << i)) {
                uint8_t bi = eb->attributes[i].buffer;
                assert(bi != 0xFF);
                bindBuffer(GL_ARRAY_BUFFER, eb->gl.buffers[bi]);
                if (UTILS_UNLIKELY(eb->attributes[i].flags & Attribute::FLAG_INTEGER_TARGET)) {
                    glVertexAttribIPointer(GLuint(i),
                            getComponentCount(eb->attributes[i].type),
                            getComponentType(eb->attributes[i].type),
                            eb->attributes[i].stride,
                            (void*) uintptr_t(eb->attributes[i].offset));
                } else {
                    glVertexAttribPointer(GLuint(i),
                            getComponentCount(eb->attributes[i].type),
                            getComponentType(eb->attributes[i].type),
                            getNormalization(eb->attributes[i].flags & Attribute::FLAG_NORMALIZED),
                            eb->attributes[i].stride,
                            (void*) uintptr_t(eb->attributes[i].offset));
                }

                enableVertexAttribArray(GLuint(i));
            } else {
                disableVertexAttribArray(GLuint(i));
            }
        }
        // this records the index buffer into the currently bound VAO
        bindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->gl.buffer);

        CHECK_GL_ERROR(utils::slog.e)
    }
}

void OpenGLDriver::setRenderPrimitiveRange(Driver::RenderPrimitiveHandle rph,
        Driver::PrimitiveType pt, uint32_t offset,
        uint32_t minIndex, uint32_t maxIndex, uint32_t count) {
    DEBUG_MARKER()

    if (rph) {
        GLRenderPrimitive* const rp = handle_cast<GLRenderPrimitive*>(rph);
        rp->type = pt;
        rp->offset = offset * ((rp->gl.indicesType == GL_UNSIGNED_INT) ? 4 : 2);
        rp->count = count;
        rp->minIndex = minIndex;
        rp->maxIndex = maxIndex > minIndex ? maxIndex : rp->maxVertexCount - 1; // sanitize max index
    }
}

void OpenGLDriver::setViewportScissor(
        int32_t left, int32_t bottom, uint32_t width, uint32_t height) {
    DEBUG_MARKER()

    // In OpenGL, all four scissor parameters are actually signed, so clamp to MAX_INT32.
    const int32_t maxval = std::numeric_limits<int32_t>::max();
    left = std::min(left, maxval);
    bottom = std::min(bottom, maxval);
    width = std::min(width, uint32_t(maxval));
    height = std::min(height, uint32_t(maxval));
    // Compute the intersection of the requested scissor rectangle with the current viewport.
    // Note that the viewport rectangle isn't necessarily equal to the bounds of the current
    // Filament View (e.g., when post-processing is enabled).
    vec4gli scissor;
    scissor.x = std::max((int32_t) left, state.window.viewport[0]);
    scissor.y = std::max((int32_t) bottom, state.window.viewport[1]);
    int32_t right = std::min((int32_t) left + (int32_t) width,
            state.window.viewport[0] + state.window.viewport[2]);
    int32_t top = std::min((int32_t) bottom + (int32_t) height,
            state.window.viewport[1] + state.window.viewport[3]);
    // Compute width / height of the intersected region. If there's no intersection, pass zeroes
    // rather than negative values to satisfy OpenGL requirements.
    scissor.z = std::max(0, right - scissor.x);
    scissor.w = std::max(0, top - scissor.y);
    update_state(state.window.scissor, scissor, [scissor]() {
        glScissor(scissor.x, scissor.y, scissor.z, scissor.w);
    });
}

/*
 * This is called in the user thread
 */

#define DEBUG_NO_EXTERNAL_STREAM_COPY false

void OpenGLDriver::updateStream(GLTexture* t, driver::DriverApi* driver) noexcept {
    GLStream* s = static_cast<GLStream*>(t->hwStream);
    if (UTILS_UNLIKELY(s == nullptr)) {
        // this can happen because we're called synchronously and the setExternalStream() call
        // may bot have been processed yet.
        return;
    }

    if (UTILS_LIKELY(!s->isNativeStream())) {
        // round-robin to the next texture name
        if (UTILS_UNLIKELY(DEBUG_NO_EXTERNAL_STREAM_COPY ||
                           bugs.disable_shared_context_draws || !mOpenGLBlitter)) {
            driver->queueCommand([this, t, s]() {
                // the stream may have been destroyed since we enqueued the command
                // also make sure that this texture is still associated with the same stream
                auto& streams = mExternalStreams;
                if (UTILS_LIKELY(std::find(streams.begin(), streams.end(), t) != streams.end()) &&
                    (t->hwStream == s)) {
                    t->gl.texture_id = s->gl.externalTextureId;
                }
            });
        } else {
            s->user_thread.cur = uint8_t(
                    (s->user_thread.cur + 1) % GLStream::ROUND_ROBIN_TEXTURE_COUNT);
            GLuint writeTexture = s->user_thread.write[s->user_thread.cur];
            GLuint readTexture = s->user_thread.read[s->user_thread.cur];

            // Make sure we're using the proper size
            GLStream::Info& info = s->user_thread.infos[s->user_thread.cur];
            if (UTILS_UNLIKELY(info.width != s->width || info.height != s->height)) {

                // nothing guarantees that this buffer is free (i.e. has been consumed by the
                // GL thread), so we could potentially cause a glitch by reallocating the
                // texture here. This should be very rare though.
                // This could be fixed by always using a new temporary texture here, and
                // replacing it in the queueCommand() below. imho, not worth it.

                info.width = s->width;
                info.height = s->height;

                Platform::ExternalTexture* ets = s->user_thread.infos[s->user_thread.cur].ets;
                mPlatform.reallocateExternalStorage(ets, info.width, info.height,
                        TextureFormat::RGB8);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, writeTexture);
                glBindTexture(GL_TEXTURE_EXTERNAL_OES, readTexture);
#ifdef GL_OES_EGL_image
                glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)ets->image);
                glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, (GLeglImageOES)ets->image);
#endif
            }

            // copy the texture...
#ifndef NDEBUG
            if (t->gl.fence) {
                // we're about to overwrite a buffer that hasn't been consumed
                slog.d << "OpenGLDriver::updateStream(): about to overwrite buffer " <<
                       int(s->user_thread.cur) << " of Texture at " << t << " of Stream at " << s
                       << io::endl;
            }
#endif
            mOpenGLBlitter->blit(s->gl.externalTextureId, writeTexture, s->width, s->height);

            // We need a fence to guarantee that this copy has happened when we need the texture
            // in OpenGLProgram::updateSamplers(), i.e. when we bind textures just before use.
            GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            // Per https://www.khronos.org/opengl/wiki/Sync_Object, flush to make sure that the
            // sync object is in the driver's command queue.
            glFlush();

            // Update the stream timestamp. It's not clear to me that this is correct; which
            // timestamp do we really want? Here we use "now" because we have nothing else we
            // can use.
            s->user_thread.timestamp = std::chrono::steady_clock::now().time_since_epoch().count();

            driver->queueCommand([this, t, s, fence, readTexture, writeTexture]() {
                // the stream may have been destroyed since we enqueued the command
                // also make sure that this texture is still associated with the same stream
                auto& streams = mExternalStreams;
                if (UTILS_LIKELY(std::find(streams.begin(), streams.end(), t) != streams.end()) &&
                    (t->hwStream == s)) {
                    if (UTILS_UNLIKELY(t->gl.fence)) {
                        // if the texture still has a fence set, destroy it now, so it's not leaked.
                        glDeleteSync(t->gl.fence);
                    }
                    t->gl.texture_id = readTexture;
                    t->gl.fence = fence;
                    s->gl.externalTexture2DId = writeTexture;
                } else {
                    glDeleteSync(fence);
                }
            });
        }
    }
}

void OpenGLDriver::readStreamPixels(Driver::StreamHandle sh,
        uint32_t x, uint32_t y, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& p) {
    DEBUG_MARKER()

    GLStream* s = handle_cast<GLStream*>(sh);
    if (UTILS_LIKELY(!s->isNativeStream())) {
        GLuint tid = s->gl.externalTexture2DId;
        if (tid == 0) {
            return;
        }

        GLenum glFormat = getFormat(p.format);
        GLenum glType = getType(p.type);

        pixelStore(GL_PACK_ROW_LENGTH, p.stride);
        pixelStore(GL_PACK_ALIGNMENT, p.alignment);
        pixelStore(GL_PACK_SKIP_PIXELS, p.left);
        pixelStore(GL_PACK_SKIP_ROWS, p.top);

        if (s->gl.fbo == 0) {
            glGenFramebuffers(1, &s->gl.fbo);
        }
        bindFramebuffer(GL_FRAMEBUFFER, s->gl.fbo);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tid, 0);
        CHECK_GL_ERROR(utils::slog.e)

        /*
         * It looks like glReadPixels() behaves differently, or even wrongly,
         * when the FBO is backed by an EXTERNAL texture...
         *
         *
         *  External texture FBO           User buffer
         *
         *  O---+----------------+
         *  |   |                |                .stride         .alignment
         *  |   |                |         ----------------------->-->
         *  |   | y              |         O----------------------+--+   low adresses
         *  |   |                |         |          |           |  |
         *  |   |         w      |         |          | .bottom   |  |
         *  |   V   <--------->  |         |          V           |  |
         *  |       +---------+  |         |     +---------+      |  |
         *  |       |     ^   |  | ======> |     |         |      |  |
         *  |   x   |    h|   |  |         |.left|         |      |  |
         *  +------>|     v   |  |         +---->|         |      |  |
         *  |       +.........+  |         |     +.........+      |  |
         *  |                    |         |                      |  |
         *  |                    |         +----------------------+--+  high adresses
         *  +--------------------+
         *
         *  Origin is at the                The image is NOT y-reversed
         *  top-left corner                 and bottom is counted from
         *                                  the top! "bottom" is in fact treated
         *                                  as "top".
         */

        // The filament API provides yoffset as the "bottom" offset, therefore it needs to
        // be corrected to match glReadPixels()'s behavior.
        y = (s->height - height) - y;

        // TODO: we could use a PBO to make this asynchronous
        glReadPixels(GLint(x), GLint(y), GLint(width), GLint(height), glFormat, glType, p.buffer);
        CHECK_GL_ERROR(utils::slog.e)

        bindFramebuffer(GL_FRAMEBUFFER, 0);
        scheduleDestroy(std::move(p));
    }
}

// ------------------------------------------------------------------------------------------------
// Setting rendering state
// ------------------------------------------------------------------------------------------------

void OpenGLDriver::viewport(ssize_t left, ssize_t bottom, size_t width, size_t height) {
    DEBUG_MARKER()

    GLint l = (GLint)left;
    GLint b = (GLint)bottom;
    GLsizei w = (GLsizei)width;
    GLsizei h = (GLsizei)height;

    // Note that we also set the scissor when binding a material instance, but the following
    // scissor call is still necessary, so that the post-process quad doesn't get clipped.
    setScissor(l, b, w, h);

    setViewport(l, b, w, h);
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::bindUniformBuffer(size_t index, Driver::UniformBufferHandle ubh) {
    DEBUG_MARKER()
    GLUniformBuffer* ub = handle_cast<GLUniformBuffer *>(ubh);
    assert(ub->gl.ubo.base == 0);
    bindBufferRange(GL_UNIFORM_BUFFER, GLuint(index), ub->gl.ubo.id, 0, ub->gl.ubo.capacity);
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::bindUniformBufferRange(size_t index, Driver::UniformBufferHandle ubh,
        size_t offset, size_t size) {
    DEBUG_MARKER()

    GLUniformBuffer* ub = handle_cast<GLUniformBuffer*>(ubh);
    // TODO: Is this assert really needed? Note that size is only populated for STREAM buffers.
    assert(size <= ub->gl.ubo.size);
    assert(ub->gl.ubo.base + offset + size <= ub->gl.ubo.capacity);
    bindBufferRange(GL_UNIFORM_BUFFER, GLuint(index), ub->gl.ubo.id, ub->gl.ubo.base + offset, size);
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::bindSamplers(size_t index, Driver::SamplerBufferHandle sbh) {
    DEBUG_MARKER()

    GLSamplerBuffer* sb = handle_cast<GLSamplerBuffer *>(sbh);
    assert(index < Program::NUM_SAMPLER_BINDINGS);
    mSamplerBindings[index] = sb;
    CHECK_GL_ERROR(utils::slog.e)
}


GLuint OpenGLDriver::getSamplerSlow(driver::SamplerParams params) const noexcept {
    assert(mSamplerMap.find(params.u) == mSamplerMap.end());

    GLuint s;
    glGenSamplers(1, &s);
    glSamplerParameteri(s, GL_TEXTURE_MIN_FILTER,   getTextureFilter(params.filterMin));
    glSamplerParameteri(s, GL_TEXTURE_MAG_FILTER,   getTextureFilter(params.filterMag));
    glSamplerParameteri(s, GL_TEXTURE_WRAP_S,       getWrapMode(params.wrapS));
    glSamplerParameteri(s, GL_TEXTURE_WRAP_T,       getWrapMode(params.wrapT));
    glSamplerParameteri(s, GL_TEXTURE_WRAP_R,       getWrapMode(params.wrapR));
    glSamplerParameteri(s, GL_TEXTURE_COMPARE_MODE, getTextureCompareMode(params.compareMode));
    glSamplerParameteri(s, GL_TEXTURE_COMPARE_FUNC, getTextureCompareFunc(params.compareFunc));
// TODO: Why does this fail with WebGL 2.0? The run-time check should suffice.
#if defined(GL_EXT_texture_filter_anisotropic) && !defined(__EMSCRIPTEN__)
    if (ext.texture_filter_anisotropic) {
        GLfloat anisotropy = float(1 << params.anisotropyLog2);
        glSamplerParameterf(s, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(mMaxAnisotropy, anisotropy));
    }
#endif
    CHECK_GL_ERROR(utils::slog.e)
    mSamplerMap[params.u] = s;
    return s;
}

void OpenGLDriver::insertEventMarker(char const* string, size_t len) {
#ifdef GL_EXT_debug_marker
    if (ext.EXT_debug_marker) {
        glInsertEventMarkerEXT(GLsizei(len ? len : strlen(string)), string);
    }
#endif
}

void OpenGLDriver::pushGroupMarker(char const* string,  size_t len) {
#ifdef GL_EXT_debug_marker
    if (ext.EXT_debug_marker) {
        glPushGroupMarkerEXT(GLsizei(len ? len : strlen(string)), string);
    }
#endif
}

void OpenGLDriver::popGroupMarker(int) {
#ifdef GL_EXT_debug_marker
    if (ext.EXT_debug_marker) {
        glPopGroupMarkerEXT();
    }
#endif
}

// ------------------------------------------------------------------------------------------------
// Read-back ops
// ------------------------------------------------------------------------------------------------

void OpenGLDriver::readPixels(Driver::RenderTargetHandle src,
        uint32_t x, uint32_t y, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& p) {
    DEBUG_MARKER()

    GLenum glFormat = getFormat(p.format);
    GLenum glType = getType(p.type);

    pixelStore(GL_PACK_ROW_LENGTH, p.stride);
    pixelStore(GL_PACK_ALIGNMENT, p.alignment);
    pixelStore(GL_PACK_SKIP_PIXELS, p.left);
    pixelStore(GL_PACK_SKIP_ROWS, p.top);

    /*
     * glReadPixel() operation...
     *
     *  Framebuffer as seen on         User buffer
     *  screen
     *  +--------------------+
     *  |                    |                stride         alignment
     *  |                    |         ----------------------->-->
     *  |                    |         +----------------------+--+   low adresses
     *  |                    |         |          |           |  |
     *  |             w      |         |          | bottom    |  |
     *  |       <--------->  |         |          V           |  |
     *  |       +---------+  |         |     +.........+      |  |
     *  |       |     ^   |  | =====>  |     |         |      |  |
     *  |   x   |    h|   |  |         |left |         |      |  |
     *  +------>|     v   |  |         +---->|         |      |  |
     *  |       +.........+  |         |     +---------+      |  |
     *  |            ^       |         |                      |  |
     *  |          y |       |         +----------------------+--+  high adresses
     *  +------------+-------+
     *                                  Image is "flipped" vertically
     *                                  "bottom" is from the "top" (low addresses)
     *                                  of the buffer.
     */

    GLRenderTarget const* s = handle_cast<GLRenderTarget const*>(src);
    bindFramebuffer(GL_READ_FRAMEBUFFER, s->gl.fbo);

    // TODO: we could use a PBO to make this asynchronous
    glReadPixels(GLint(x), GLint(y), GLint(width), GLint(height), glFormat, glType, p.buffer);

    // now we need to flip the buffer vertically to match our API
    size_t stride = p.stride ? p.stride : width;
    size_t bpp = PixelBufferDescriptor::computeDataSize(p.format, p.type, 1, 1, 1);
    size_t bpr = PixelBufferDescriptor::computeDataSize(p.format, p.type, stride, 1, p.alignment);
    char* head = (char*)p.buffer + p.left * bpp + bpr * p.top;
    char* tail = (char*)p.buffer + p.left * bpp + bpr * (p.top + height - 1);
    // clang vectorizes this loop
    while (head < tail) {
        std::swap_ranges(head, head + bpp * width, tail);
        head += bpr;
        tail -= bpr;
    }

    scheduleDestroy(std::move(p));

    CHECK_GL_ERROR(utils::slog.e)
}

// ------------------------------------------------------------------------------------------------
// Rendering ops
// ------------------------------------------------------------------------------------------------

void OpenGLDriver::beginFrame(int64_t monotonic_clock_ns, uint32_t frameId) {
    insertEventMarker("beginFrame");
    if (UTILS_UNLIKELY(!mExternalStreams.empty())) {
        driver::OpenGLPlatform& platform = mPlatform;
        const size_t index = getIndexForTextureTarget(GL_TEXTURE_EXTERNAL_OES);
        for (GLTexture const* t : mExternalStreams) {
            assert(t && t->hwStream);
            if (static_cast<GLStream*>(t->hwStream)->isNativeStream()) {
                assert(t->hwStream->stream);
                platform.updateTexImage(t->hwStream->stream, &static_cast<GLStream*>(t->hwStream)->user_thread.timestamp);
                // NOTE: We assume that updateTexImage() binds the texture on our behalf
                GLuint activeUnit = state.textures.active;
                state.textures.units[activeUnit].targets[index].texture_id = t->gl.texture_id;
            }
        }
    }
}

void OpenGLDriver::setPresentationTime(int64_t monotonic_clock_ns) {
    mPlatform.setPresentationTime(monotonic_clock_ns);
}

void OpenGLDriver::endFrame(uint32_t frameId) {
    //SYSTRACE_NAME("glFinish");
    //glFinish();
    insertEventMarker("endFrame");
}

void OpenGLDriver::flush(int) {
    DEBUG_MARKER()
    glFlush();
}

UTILS_NOINLINE
void OpenGLDriver::clearWithRasterPipe(
        bool clearColor, float4 const& linearColor,
        bool clearDepth, double depth,
        bool clearStencil, uint32_t stencil) noexcept {
    DEBUG_MARKER()

    GLbitfield bitmask = 0;

    RasterState rs(mRasterState);

    if (clearColor) {
        bitmask |= GL_COLOR_BUFFER_BIT;
        setClearColor(linearColor.r, linearColor.g, linearColor.b, linearColor.a);
        rs.colorWrite = true;
    }
    if (clearDepth) {
        bitmask |= GL_DEPTH_BUFFER_BIT;
        setClearDepth(GLfloat(depth));
        rs.depthWrite = true;
    }
    if (clearStencil) {
        bitmask |= GL_STENCIL_BUFFER_BIT;
        setClearStencil(GLint(stencil));
        // stencil state is not part of RasterState for now
    }

    if (bitmask) {
        setRasterState(rs);
        glClear(bitmask);
    }
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::clearWithGeometryPipe(
        bool clearColor, float4 const& linearColor,
        bool clearDepth, double depth,
        bool clearStencil, uint32_t stencil) noexcept {
    DEBUG_MARKER()

    // GLES is required to use this method; see initClearProgram.
    assert(GLES31_HEADERS);

    // TODO: handle stencil clear with geometry as well
    if (clearStencil) {
        setClearStencil(GLint(stencil));
        glClear(GL_STENCIL_BUFFER_BIT);
        CHECK_GL_ERROR(utils::slog.e)
    }

    RasterState rs;
    rs.depthFunc = RasterState::DepthFunc::A;
    rs.colorWrite = false;
    rs.depthWrite = false;

    if (clearColor) {
        rs.colorWrite = true;
        useProgram(mClearProgram);
        glUniform4f(mClearColorLocation,
                linearColor.r, linearColor.g, linearColor.b, linearColor.a);
        CHECK_GL_ERROR(utils::slog.e)
    }
    if (clearDepth) {
        rs.depthWrite = true;
        useProgram(mClearProgram);
        glUniform1f(mClearDepthLocation, float(depth) * 2.0f - 1.0f);
        CHECK_GL_ERROR(utils::slog.e)
    }

    if (clearColor || clearDepth) {
        // by the time we get here, useProgram() has been called
        setRasterState(rs);
        bindVertexArray(nullptr);
        bindBuffer(GL_ARRAY_BUFFER, 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, mClearTriangle);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        CHECK_GL_ERROR(utils::slog.e)
    }
}

void OpenGLDriver::blit(TargetBufferFlags buffers,
        Driver::RenderTargetHandle dst,
        int32_t dstLeft, int32_t dstBottom, uint32_t dstWidth, uint32_t dstHeight,
        Driver::RenderTargetHandle src,
        int32_t srcLeft, int32_t srcBottom, uint32_t srcWidth, uint32_t srcHeight) {
    DEBUG_MARKER()

    GLbitfield mask = 0;
    if (buffers & TargetBufferFlags::COLOR) {
        mask |= GL_COLOR_BUFFER_BIT;
    }
    if (buffers & TargetBufferFlags::DEPTH) {
        mask |= GL_DEPTH_BUFFER_BIT;
    }
    if (buffers & TargetBufferFlags::STENCIL) {
        mask |= GL_STENCIL_BUFFER_BIT;
    }

    if (mask) {
        GLRenderTarget const* s = handle_cast<GLRenderTarget const*>(src);
        GLRenderTarget const* d = handle_cast<GLRenderTarget const*>(dst);
        bindFramebuffer(GL_READ_FRAMEBUFFER, s->gl.fbo);
        bindFramebuffer(GL_DRAW_FRAMEBUFFER, d->gl.fbo);
        disable(GL_SCISSOR_TEST);
        glBlitFramebuffer(
                srcLeft, srcBottom, srcLeft + srcWidth, srcBottom + srcHeight,
                dstLeft, dstBottom, dstLeft + dstWidth, dstBottom + dstHeight,
                mask, GL_LINEAR);
        enable(GL_SCISSOR_TEST);
        CHECK_GL_ERROR(utils::slog.e)
    }
}

void OpenGLDriver::draw(
        Driver::PipelineState state,
        Driver::RenderPrimitiveHandle rph) {
    DEBUG_MARKER()

    OpenGLProgram* p = handle_cast<OpenGLProgram*>(state.program);
    useProgram(p);

    const GLRenderPrimitive* rp = handle_cast<const GLRenderPrimitive *>(rph);
    bindVertexArray(rp);

    setRasterState(state.rasterState);

    polygonOffset(state.polygonOffset.slope, state.polygonOffset.constant);

    glDrawRangeElements(GLenum(rp->type), rp->minIndex, rp->maxIndex, rp->count,
            rp->gl.indicesType, reinterpret_cast<const void*>(rp->offset));

    CHECK_GL_ERROR(utils::slog.e)
}

// explicit instantiation of the Dispatcher
template class ConcreteDispatcher<OpenGLDriver>;

} // namespace filament
