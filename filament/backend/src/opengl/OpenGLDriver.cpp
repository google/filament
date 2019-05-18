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

#include "OpenGLDriver.h"

#include "private/backend/DriverApi.h"
#include "private/backend/OpenGLPlatform.h"

#include "CommandStreamDispatcher.h"
#include "OpenGLBlitter.h"
#include "OpenGLDriverFactory.h"
#include "OpenGLProgram.h"

#include <utils/compiler.h>
#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/Systrace.h>

#include <set>

// change to true to display all GL extensions in the console on start-up
#define DEBUG_PRINT_EXTENSIONS false

// To emulate EXT_multisampled_render_to_texture properly we need to be able to copy from
// a non-ms texture to an ms attachment. This is only allowed with OpenGL (not GLES), which
// would be fine for us. However, this is also not trivial to implement in Metal so for now
// we don't want to rely on it.
#define ALLOW_REVERSE_MULTISAMPLE_RESOLVE false

#if defined(__EMSCRIPTEN__)
#define HAS_MAPBUFFERS 0
#else
#define HAS_MAPBUFFERS 1
#endif

#define DEBUG_MARKER_NONE       0
#define DEBUG_MARKER_OPENGL     1

// set to the desired debug marker level
#define DEBUG_MARKER_LEVEL      DEBUG_MARKER_NONE

#if DEBUG_MARKER_LEVEL == DEBUG_MARKER_OPENGL
#   define DEBUG_MARKER() \
        DebugMarker _debug_marker(*this, __PRETTY_FUNCTION__);
#else
#   define DEBUG_MARKER()
#endif

using namespace filament::math;
using namespace utils;

namespace filament {
namespace backend {

Driver* OpenGLDriverFactory::create(
        OpenGLPlatform* const platform, void* const sharedGLContext) noexcept {
    return OpenGLDriver::create(platform, sharedGLContext);
}

} // namesapce backend

using namespace backend;
using namespace GLUtils;

UTILS_NOINLINE
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

// ------------------------------------------------------------------------------------------------

OpenGLDriver::DebugMarker::DebugMarker(OpenGLDriver& driver, const char* string) noexcept
: driver(driver) {
        const char* const begin = string + sizeof("virtual void filament::OpenGLDriver::") - 1;
        const char* const end = strchr(begin, '(');
        driver.pushGroupMarker(begin, end - begin);
}

OpenGLDriver::DebugMarker::~DebugMarker() noexcept {
    driver.popGroupMarker();
}

// ------------------------------------------------------------------------------------------------

OpenGLDriver::OpenGLDriver(OpenGLPlatform* platform) noexcept
        : DriverBase(new ConcreteDispatcher<OpenGLDriver>()),
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
            bugs.disable_glFlush = true;
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
    ExtentionSet exts;
    for (GLint i = 0; i < n; i++) {
        const char * const ext = (const char*)glGetStringi(GL_EXTENSIONS, (GLuint)i);
        exts.insert(StaticString::make(ext, strlen(ext)));
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

#ifdef GL_FRAGMENT_SHADER_DERIVATIVE_HINT
    glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);
#endif

    // On some implementation we need to clear the viewport with a triangle, for performance
    // reasons
    initClearProgram();

    // Initialize the blitter only if we have OES_EGL_image_external_essl3
    if (ext.OES_EGL_image_external_essl3) {
        mOpenGLBlitter = new OpenGLBlitter();
        mOpenGLBlitter->init();
        state.program.use = 0;
    }
}

OpenGLDriver::~OpenGLDriver() noexcept {
    delete mOpenGLBlitter;
}

// ------------------------------------------------------------------------------------------------
// Driver interface concrete implementation
// ------------------------------------------------------------------------------------------------

UTILS_NOINLINE
bool OpenGLDriver::hasExtension(ExtentionSet const& map, utils::StaticString ext) noexcept {
    return map.find(ext) != map.end();
}

void OpenGLDriver::initExtensionsGLES(GLint major, GLint minor, ExtentionSet const& exts) {
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

void OpenGLDriver::initExtensionsGL(GLint major, GLint minor, ExtentionSet const& exts) {
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
    for (GLuint unit = 0; unit < MAX_TEXTURE_UNIT_COUNT; unit++) {
        if (state.textures.units[unit].targets[index].texture_id == texture_id) {
            bindTexture(unit, target, (GLuint)0, index);
        }
    }
}

void OpenGLDriver::unbindSampler(GLuint sampler) noexcept {
    // unbind this sampler from all the units it might be bound to
    #pragma nounroll    // clang generates >800B of code!!!
    for (GLuint unit = 0; unit < MAX_TEXTURE_UNIT_COUNT; unit++) {
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

void OpenGLDriver::bindTexture(GLuint unit, GLTexture const* t) noexcept {
    assert(t != nullptr);
    bindTexture(unit, t->gl.target, t->gl.id, t->gl.targetIndex);
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

void OpenGLDriver::frontFace(GLenum mode) noexcept {
    // WARNING: don't call this without updating mRasterState
    update_state(state.raster.frontFace, mode, [&]() {
        glFrontFace(mode);
    });
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
        glBlendFuncSeparate(srcRGB, dstRGB, srcA, dstA);
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
    update_state(state.polygonOffset, { factor, units }, [&]() {
        if (factor != 0 || units != 0) {
            glPolygonOffset(factor, units);
            enable(GL_POLYGON_OFFSET_FILL);
        } else {
            disable(GL_POLYGON_OFFSET_FILL);
        }
    });
}

void OpenGLDriver::setRasterStateSlow(RasterState rs) noexcept {
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

    frontFace(rs.inverseFrontFaces ? GL_CW : GL_CCW);

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
    if (rs.depthFunc == RasterState::DepthFunc::A && !rs.depthWrite) {
        disable(GL_DEPTH_TEST);
    } else {
        enable(GL_DEPTH_TEST);
        depthFunc(getDepthFunc(rs.depthFunc));
        depthMask(GLboolean(rs.depthWrite));
    }

    // write masks
    colorMask(GLboolean(rs.colorWrite));

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
//    GLSamplerGroup           : 16        moderate
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
    slog.d << "GLSamplerGroup: " << sizeof(GLSamplerGroup) << io::endl;
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

Handle<HwVertexBuffer> OpenGLDriver::createVertexBufferS() noexcept {
    return Handle<HwVertexBuffer>( allocateHandle(sizeof(GLVertexBuffer)) );
}

Handle<HwIndexBuffer> OpenGLDriver::createIndexBufferS() noexcept {
    return Handle<HwIndexBuffer>( allocateHandle(sizeof(GLIndexBuffer)) );
}

Handle<HwRenderPrimitive> OpenGLDriver::createRenderPrimitiveS() noexcept {
    return Handle<HwRenderPrimitive>( allocateHandle(sizeof(GLRenderPrimitive)) );
}

Handle<HwProgram> OpenGLDriver::createProgramS() noexcept {
    return Handle<HwProgram>( allocateHandle(sizeof(OpenGLProgram)) );
}

Handle<HwSamplerGroup> OpenGLDriver::createSamplerGroupS() noexcept {
    return Handle<HwSamplerGroup>( allocateHandle(sizeof(GLSamplerGroup)) );
}

Handle<HwUniformBuffer> OpenGLDriver::createUniformBufferS() noexcept {
    return Handle<HwUniformBuffer>( allocateHandle(sizeof(GLUniformBuffer)) );
}

Handle<HwTexture> OpenGLDriver::createTextureS() noexcept {
    return Handle<HwTexture>( allocateHandle(sizeof(GLTexture)) );
}

Handle<HwRenderTarget> OpenGLDriver::createDefaultRenderTargetS() noexcept {
    return Handle<HwRenderTarget>( allocateHandle(sizeof(GLRenderTarget)) );
}

Handle<HwRenderTarget> OpenGLDriver::createRenderTargetS() noexcept {
    return Handle<HwRenderTarget>( allocateHandle(sizeof(GLRenderTarget)) );
}

Handle<HwFence> OpenGLDriver::createFenceS() noexcept {
    return Handle<HwFence>( allocateHandle(sizeof(HwFence)) );
}

Handle<HwSwapChain> OpenGLDriver::createSwapChainS() noexcept {
    return Handle<HwSwapChain>( allocateHandle(sizeof(HwSwapChain)) );
}

Handle<HwStream> OpenGLDriver::createStreamFromTextureIdS() noexcept {
    return Handle<HwStream>( allocateHandle(sizeof(GLStream)) );
}

void OpenGLDriver::createVertexBufferR(
        Handle<HwVertexBuffer> vbh,
    uint8_t bufferCount,
    uint8_t attributeCount,
    uint32_t elementCount,
    AttributeArray attributes,
    BufferUsage usage) {
    DEBUG_MARKER()

    GLVertexBuffer* vb = construct<GLVertexBuffer>(vbh,
            bufferCount, attributeCount, elementCount, attributes);

    GLsizei n = GLsizei(vb->bufferCount);

    assert(n <= (GLsizei)vb->gl.buffers.size());
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

void OpenGLDriver::createIndexBufferR(
        Handle<HwIndexBuffer> ibh,
        ElementType elementType,
        uint32_t indexCount,
        BufferUsage usage) {
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

void OpenGLDriver::createRenderPrimitiveR(Handle<HwRenderPrimitive> rph, int) {
    DEBUG_MARKER()

    GLRenderPrimitive* rp = construct<GLRenderPrimitive>(rph);
    glGenVertexArrays(1, &rp->gl.vao);
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createProgramR(Handle<HwProgram> ph, Program&& program) {
    DEBUG_MARKER()

    construct<OpenGLProgram>(ph, this, program);
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createSamplerGroupR(Handle<HwSamplerGroup> sbh, size_t size) {
    DEBUG_MARKER()

    construct<GLSamplerGroup>(sbh, size);
}

void OpenGLDriver::createUniformBufferR(
        Handle<HwUniformBuffer> ubh,
        size_t size,
        BufferUsage usage) {
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

    bindTexture(MAX_TEXTURE_UNIT_COUNT - 1, t);
    activeTexture(MAX_TEXTURE_UNIT_COUNT - 1);

    switch (t->gl.target) {
        case GL_TEXTURE_2D:
        case GL_TEXTURE_CUBE_MAP:
            glTexStorage2D(t->gl.target, GLsizei(t->levels), t->gl.internalFormat,
                    GLsizei(width), GLsizei(height));
            break;
        case GL_TEXTURE_2D_ARRAY: {
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

void OpenGLDriver::createTextureR(Handle<HwTexture> th, SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
        TextureUsage usage) {
    DEBUG_MARKER()

    GLTexture* t = construct<GLTexture>(th, target, levels, samples, w, h, depth, format, usage);
    if (UTILS_LIKELY(usage & TextureUsage::SAMPLEABLE)) {
        glGenTextures(1, &t->gl.id);

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
                    if (depth <= 1) {
                        t->gl.targetIndex = (uint8_t)
                                getIndexForTextureTarget(t->gl.target = GL_TEXTURE_2D);
                    } else {
                        t->gl.targetIndex = (uint8_t)
                                getIndexForTextureTarget(t->gl.target = GL_TEXTURE_2D_ARRAY);
                    }
                    break;
                case SamplerType::SAMPLER_CUBEMAP:
                    t->gl.targetIndex = (uint8_t)
                            getIndexForTextureTarget(t->gl.target = GL_TEXTURE_CUBE_MAP);
                    break;
            }

            if (t->samples > 1) {
                // Note: we can't be here in practice because filament's user API doesn't
                // allow the creation of multi-sampled textures.
                if (features.multisample_texture) {
                    // multi-sample texture on GL 3.2 / GLES 3.1 and above
                    t->gl.targetIndex = (uint8_t)
                            getIndexForTextureTarget(t->gl.target = GL_TEXTURE_2D_MULTISAMPLE);
                } else {
                    // Turn off multi-sampling for that texture. It's just not supported.
                }
            }

            textureStorage(t, w, h, depth);
        }
    } else {
        assert(usage & (
                TextureUsage::COLOR_ATTACHMENT |
                TextureUsage::DEPTH_ATTACHMENT |
                TextureUsage::STENCIL_ATTACHMENT));
        assert(levels == 1);
        assert(target == SamplerType::SAMPLER_2D);
        t->gl.internalFormat = getInternalFormat(format);
        t->gl.target = GL_RENDERBUFFER;
        glGenRenderbuffers(1, &t->gl.id);
        renderBufferStorage(t->gl.id, t->gl.internalFormat, w, h, samples);
    }

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::framebufferTexture(backend::TargetBufferInfo const& binfo,
        GLRenderTarget const* rt, GLenum attachment) noexcept {
    GLTexture const* t = handle_cast<const GLTexture*>(binfo.handle);

    assert(t->target != SamplerType::SAMPLER_EXTERNAL);

    GLenum target = GL_TEXTURE_2D;
    switch (t->target) {
        case SamplerType::SAMPLER_2D:
            target = t->gl.target;  // this could be GL_TEXTURE_2D_MULTISAMPLE or GL_TEXTURE_2D_ARRAY
            // note: multi-sampled textures can't have mipmaps
            break;
        case SamplerType::SAMPLER_CUBEMAP:
            target = getCubemapTarget(binfo.face);
            // note: cubemaps can't be multi-sampled
            break;
        default:
            break;
    }

    if (rt->gl.samples <= 1 ||
        (rt->gl.samples > 1 && t->samples > 1 && features.multisample_texture)) {
        // on GL3.2 / GLES3.1 and above multisample is handled when creating the texture.
        // If multisampled textures are not supported and we end-up here, things should
        // still work, albeit without MSAA.
        bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);
        switch (target) {
            case GL_TEXTURE_2D:
            case GL_TEXTURE_2D_MULTISAMPLE:
                glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
                        target, t->gl.id, binfo.level);
                break;
            case GL_TEXTURE_2D_ARRAY:
                // GL_TEXTURE_2D_MULTISAMPLE_ARRAY is not supported in GLES
                glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment,
                        t->gl.id, binfo.level, binfo.layer);
                break;
            default:
                // we shouldn't be here
                break;
        }
        CHECK_GL_ERROR(utils::slog.e)
    } else
#if GLES31_HEADERS
    if (ext.EXT_multisampled_render_to_texture && t->depth <= 1) {
        assert(rt->gl.samples > 1);
        // We have a multi-sample rendertarget and we have EXT_multisampled_render_to_texture,
        // so, we can directly use a 1-sample texture as attachment, multi-sample resolve,
        // will happen automagically and efficiently in the driver.
        // This extension only exists on OpenGL ES.
        bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);
        glext::glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER,
                        attachment, target, t->gl.id, binfo.level, rt->gl.samples);
    } else
#endif
    { // here we emulate ext.EXT_multisampled_render_to_texture
        assert(rt->gl.samples > 1);

        // If the texture doesn't already have one, create a sidecar multi-sampled renderbuffer,
        // which is where drawing will actually take place, make that our attachment.
        bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);
        if (t->gl.rb == 0) {
            glGenRenderbuffers(1, &t->gl.rb);
            renderBufferStorage(t->gl.rb,
                    t->gl.internalFormat, rt->width, rt->height, rt->gl.samples);
        }
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, t->gl.rb);

        // We also need a "read" sidecar fbo, used later for the resolve, which takes place in
        // endRenderPass().
        if (!rt->gl.fbo_read) {
            glGenFramebuffers(1, &rt->gl.fbo_read);
        }
        bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo_read);
        switch (target) {
            case GL_TEXTURE_2D:
                glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
                        target, t->gl.id, binfo.level);
                break;
            case GL_TEXTURE_2D_ARRAY:
                glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment,
                        t->gl.id, binfo.level, binfo.layer);
                break;
            default:
                // we shouldn't be here
                break;
        }

        CHECK_GL_ERROR(utils::slog.e)

        switch (attachment) {
            case GL_COLOR_ATTACHMENT0:
                rt->gl.resolve |= TargetBufferFlags::COLOR;
                break;
            case GL_DEPTH_ATTACHMENT:
                rt->gl.resolve |= TargetBufferFlags::DEPTH;
                break;
            case GL_STENCIL_ATTACHMENT:
                rt->gl.resolve |= TargetBufferFlags::STENCIL;
                break;
            case GL_DEPTH_STENCIL_ATTACHMENT:
                rt->gl.resolve |= TargetBufferFlags::DEPTH;
                rt->gl.resolve |= TargetBufferFlags::STENCIL;
                break;
            default:
                break;
        }
    }

    CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e)
}

void OpenGLDriver::renderBufferStorage(GLuint rbo, GLenum internalformat, uint32_t width,
        uint32_t height, uint8_t samples) const noexcept {
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    if (samples > 1) {
#if GLES31_HEADERS
        if (ext.EXT_multisampled_render_to_texture) {
            // FIXME: if real multi-sample textures are/will be used as attachment,
            //        we shouldn't be here. But we don't have an easy way to know at this point.
            glext::glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, samples, internalformat, width, height);
        } else
#endif
        {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalformat, width, height);
        }
    } else {
        glRenderbufferStorage(GL_RENDERBUFFER, internalformat, width, height);
    }
}

void OpenGLDriver::framebufferRenderbuffer(GLTexture const* t,
        GLRenderTarget const* rt, GLenum attachment) noexcept {
    bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, t->gl.id);
    // unbind the renderbuffer, to avoid any later confusion
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    CHECK_GL_ERROR(utils::slog.e)
    CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e)
}

void OpenGLDriver::createDefaultRenderTargetR(
        Handle<HwRenderTarget> rth, int) {
    DEBUG_MARKER()

    construct<GLRenderTarget>(rth, 0, 0);  // FIXME: we don't know the width/height

    uint32_t framebuffer, colorbuffer, depthbuffer;
    mPlatform.createDefaultRenderTarget(framebuffer, colorbuffer, depthbuffer);

    GLRenderTarget* rt = handle_cast<GLRenderTarget*>(rth);
    rt->gl.fbo = framebuffer;
    rt->gl.samples = 1;
}

void OpenGLDriver::createRenderTargetR(Handle<HwRenderTarget> rth,
        TargetBufferFlags targets,
        uint32_t width,
        uint32_t height,
        uint8_t samples,
        TargetBufferInfo color,
        TargetBufferInfo depth,
        TargetBufferInfo stencil) {
    DEBUG_MARKER()

    GLRenderTarget* rt = construct<GLRenderTarget>(rth, width, height);
    glGenFramebuffers(1, &rt->gl.fbo);

    /*
     * The GLES 3.0 spec states:
     *
     * GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE is returned
     * - if the value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers or,
     * - if the attached images are a mix of renderbuffers and textures,
     *      the value of GL_RENDERBUFFER_SAMPLES is not zero.
     *
     * GLES 3.1 (spec, refpages are wrong) and EXT_multisampled_render_to_texture add:
     *
     * The value of RENDERBUFFER_SAMPLES is the same for all
     *    attached renderbuffers; the value of TEXTURE_SAMPLES
     *    is the same for all texture attachments; and, if the attached
     *    images are a mix of renderbuffers and textures, the value of
     *    RENDERBUFFER_SAMPLES matches the value of TEXTURE_SAMPLES.
     *
     *
     * In other words, heterogeneous (renderbuffer/textures) attachments are not supported in
     * GLES3.0, unless EXT_multisampled_render_to_texture is present.
     *
     * 'features.multisample_texture' below is a proxy for "GLES3.1 or GL4.x".
     *
     */

    rt->gl.samples = samples;

    if (targets & TargetBufferFlags::COLOR) {
        // TODO: handle multiple color attachments
        assert(color.handle);
        rt->gl.color.texture = handle_cast<GLTexture*>(color.handle);
        rt->gl.color.level = color.level;
        assert(width == rt->gl.color.texture->width && height == rt->gl.color.texture->height);
        if (rt->gl.color.texture->usage & TextureUsage::SAMPLEABLE) {
            framebufferTexture(color, rt, GL_COLOR_ATTACHMENT0);
        } else {
            framebufferRenderbuffer(rt->gl.color.texture, rt, GL_COLOR_ATTACHMENT0);
        }
#ifndef NDEBUG
        // clear the color buffer we just allocated to yellow
        setClearColor(1, 1, 0, 1);
        bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);
        disable(GL_SCISSOR_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
#endif
    }

    // handle special cases first (where depth/stencil are packed)
    bool specialCased = false;
    if ((targets & TargetBufferFlags::DEPTH_AND_STENCIL) == TargetBufferFlags::DEPTH_AND_STENCIL) {
        assert(depth.handle);
        assert(!stencil.handle || stencil.handle == depth.handle);
        rt->gl.depth.texture = handle_cast<GLTexture*>(depth.handle);
        rt->gl.depth.level = depth.level;
        assert(width == rt->gl.depth.texture->width && height == rt->gl.depth.texture->height);
        if (rt->gl.depth.texture->usage & TextureUsage::SAMPLEABLE) {
            // special case: depth & stencil requested, and both provided as the same texture
            specialCased = true;
            framebufferTexture(depth, rt, GL_DEPTH_STENCIL_ATTACHMENT);
        } else if (!depth.handle && !stencil.handle) {
            // special case: depth & stencil requested, but both not provided
            specialCased = true;
            framebufferRenderbuffer(rt->gl.depth.texture, rt, GL_DEPTH_STENCIL_ATTACHMENT);
        }
    }

    if (!specialCased) {
        if (targets & TargetBufferFlags::DEPTH) {
            assert(depth.handle);
            rt->gl.depth.texture = handle_cast<GLTexture*>(depth.handle);
            rt->gl.depth.level = depth.level;
            assert(width == rt->gl.depth.texture->width && height == rt->gl.depth.texture->height);
            if (rt->gl.depth.texture->usage & TextureUsage::SAMPLEABLE) {
                framebufferTexture(depth, rt, GL_DEPTH_ATTACHMENT);
            } else {
                framebufferRenderbuffer(rt->gl.depth.texture, rt, GL_DEPTH_ATTACHMENT);
            }
        }
        if (targets & TargetBufferFlags::STENCIL) {
            assert(stencil.handle);
            rt->gl.stencil.texture = handle_cast<GLTexture*>(stencil.handle);
            rt->gl.stencil.level = stencil.level;
            assert(width == rt->gl.stencil.texture->width && height == rt->gl.stencil.texture->height);
            if (rt->gl.stencil.texture->usage & TextureUsage::SAMPLEABLE) {
                framebufferTexture(stencil, rt, GL_STENCIL_ATTACHMENT);
            } else {
                framebufferRenderbuffer(rt->gl.stencil.texture, rt, GL_STENCIL_ATTACHMENT);
            }
        }
    }

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createFenceR(Handle<HwFence> fh, int) {
    DEBUG_MARKER()

    HwFence* f = construct<HwFence>(fh);
    f->fence = mPlatform.createFence();
}

void OpenGLDriver::createSwapChainR(Handle<HwSwapChain> sch, void* nativeWindow, uint64_t flags) {
    DEBUG_MARKER()

    HwSwapChain* sc = construct<HwSwapChain>(sch);
    sc->swapChain = mPlatform.createSwapChain(nativeWindow, flags);
}

void OpenGLDriver::createStreamFromTextureIdR(Handle<HwStream> sh,
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

void OpenGLDriver::destroyVertexBuffer(Handle<HwVertexBuffer> vbh) {
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

void OpenGLDriver::destroyIndexBuffer(Handle<HwIndexBuffer> ibh) {
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

void OpenGLDriver::destroyRenderPrimitive(Handle<HwRenderPrimitive> rph) {
    DEBUG_MARKER()

    if (rph) {
        GLRenderPrimitive const* rp = handle_cast<const GLRenderPrimitive*>(rph);
        glDeleteVertexArrays(1, &rp->gl.vao);
        // binding of a bound VAO is reset to 0
        if (state.vao.p == rp) {
            bindVertexArray(nullptr);
        }
        destruct(rph, rp);
    }
}

void OpenGLDriver::destroyProgram(Handle<HwProgram> ph) {
    DEBUG_MARKER()

    if (ph) {
        OpenGLProgram* p = handle_cast<OpenGLProgram*>(ph);
        destruct(ph, p);
    }
}

void OpenGLDriver::destroySamplerGroup(Handle<HwSamplerGroup> sbh) {
    DEBUG_MARKER()

    if (sbh) {
        GLSamplerGroup* sb = handle_cast<GLSamplerGroup*>(sbh);
        destruct(sbh, sb);
    }
}

void OpenGLDriver::destroyUniformBuffer(Handle<HwUniformBuffer> ubh) {
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

void OpenGLDriver::destroyTexture(Handle<HwTexture> th) {
    DEBUG_MARKER()

    if (th) {
        GLTexture* t = handle_cast<GLTexture*>(th);
        if (UTILS_LIKELY(t->usage & TextureUsage::SAMPLEABLE)) {
            unbindTexture(t->gl.target, t->gl.id);
            if (UTILS_UNLIKELY(t->hwStream)) {
                detachStream(t);
            }
            if (t->gl.rb) {
                glDeleteRenderbuffers(1, &t->gl.rb);
            }
            glDeleteTextures(1, &t->gl.id);
        } else {
            assert(t->gl.target == GL_RENDERBUFFER);
            assert(t->gl.rb == 0);
            glDeleteRenderbuffers(1, &t->gl.id);
        }
        if (t->gl.fence) {
            glDeleteSync(t->gl.fence);
        }
        destruct(th, t);
    }
}

void OpenGLDriver::destroyRenderTarget(Handle<HwRenderTarget> rth) {
    DEBUG_MARKER()

    if (rth) {
        GLRenderTarget* rt = handle_cast<GLRenderTarget*>(rth);
        if (rt->gl.fbo) {
            // first unbind this framebuffer if needed
            bindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, &rt->gl.fbo);
        }
        if (rt->gl.fbo_read) {
            // first unbind this framebuffer if needed
            bindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, &rt->gl.fbo_read);
        }
        destruct(rth, rt);
    }
}

void OpenGLDriver::destroySwapChain(Handle<HwSwapChain> sch) {
    DEBUG_MARKER()

    if (sch) {
        HwSwapChain* sc = handle_cast<HwSwapChain*>(sch);
        mPlatform.destroySwapChain(sc->swapChain);
        destruct(sch, sc);
    }
}

void OpenGLDriver::destroyStream(Handle<HwStream> sh) {
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

void OpenGLDriver::updateStreams(DriverApi* driver) {
    if (UTILS_UNLIKELY(!mExternalStreams.empty())) {
        OpenGLBlitter::State state;
        for (GLTexture* t : mExternalStreams) {
            assert(t);

            GLStream* s = static_cast<GLStream*>(t->hwStream);
            if (UTILS_UNLIKELY(s == nullptr)) {
                // this can happen because we're called synchronously and the setExternalStream()
                // call may bot have been processed yet.
                continue;
            }

            if (!s->isNativeStream()) {
                state.setup();
                updateStream(t, driver);
            }
        }
    }
}

void OpenGLDriver::setStreamDimensions(Handle<HwStream> sh, uint32_t width, uint32_t height) {
    if (sh) {
        GLStream* s = handle_cast<GLStream*>(sh);
        s->width = width;
        s->height = height;
    }
}

int64_t OpenGLDriver::getStreamTimestamp(Handle<HwStream> sh) {
    if (sh) {
        GLStream* s = handle_cast<GLStream*>(sh);
        return s->user_thread.timestamp;
    }
    return 0;
}

void OpenGLDriver::destroyFence(Handle<HwFence> fh) {
    if (fh) {
        HwFence* f = handle_cast<HwFence*>(fh);
        mPlatform.destroyFence(f->fence);
        destruct(fh, f);
    }
}

FenceStatus OpenGLDriver::wait(Handle<HwFence> fh, uint64_t timeout) {
    if (fh) {
        HwFence* f = handle_cast<HwFence*>(fh);
        return mPlatform.waitFence(f->fence, timeout);
    }
    return FenceStatus::ERROR;
}

bool OpenGLDriver::isTextureFormatSupported(TextureFormat format) {
    if (isETC2Compression(format)) {
        return ext.texture_compression_etc2;
    }
    if (isS3TCCompression(format)) {
        return ext.texture_compression_s3tc;
    }
    return getInternalFormat(format) != 0;
}

bool OpenGLDriver::isRenderTargetFormatSupported(TextureFormat format) {
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

        // Three-component SRGB is a color-renderable texture format in core OpenGL on desktop.
        case TextureFormat::SRGB8:
            return GL41_HEADERS;

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


void OpenGLDriver::commit(Handle<HwSwapChain> sch) {
    DEBUG_MARKER()

    if (sch) {
        HwSwapChain* sc = handle_cast<HwSwapChain*>(sch);
        mPlatform.commit(sc->swapChain);
    }
}

void OpenGLDriver::makeCurrent(Handle<HwSwapChain> schDraw, Handle<HwSwapChain> schRead) {
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

void OpenGLDriver::updateVertexBuffer(Handle<HwVertexBuffer> vbh,
        size_t index, BufferDescriptor&& p, uint32_t byteOffset) {
    DEBUG_MARKER()

    GLVertexBuffer* eb = handle_cast<GLVertexBuffer *>(vbh);

    bindBuffer(GL_ARRAY_BUFFER, eb->gl.buffers[index]);
    glBufferSubData(GL_ARRAY_BUFFER, byteOffset, p.size, p.buffer);

    scheduleDestroy(std::move(p));

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::updateIndexBuffer(
        Handle<HwIndexBuffer> ibh, BufferDescriptor&& p, uint32_t byteOffset) {
    DEBUG_MARKER()

    GLIndexBuffer* ib = handle_cast<GLIndexBuffer *>(ibh);
    assert(ib->elementSize == 2 || ib->elementSize == 4);

    bindVertexArray(nullptr);
    bindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->gl.buffer);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, byteOffset, p.size, p.buffer);

    scheduleDestroy(std::move(p));

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::loadUniformBuffer(Handle<HwUniformBuffer> ubh, BufferDescriptor&& p) {
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
    if (buffer->usage == BufferUsage::STREAM) {

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
        // when loading less that the buffer size, it's okay to assume the back of the buffer
        // is undefined.
        // glBufferSubData() could be catastrophically inefficient if several are issued
        // during the same frame. Currently, we're not doing that though.
        // TODO: investigate if it'll be faster to use glBufferData().
        glBufferSubData(target, 0, p.size, p.buffer);
    }

    CHECK_GL_ERROR(utils::slog.e)
}


void OpenGLDriver::updateSamplerGroup(Handle<HwSamplerGroup> sbh,
        SamplerGroup&& samplerGroup) {
    DEBUG_MARKER()

    GLSamplerGroup* sb = handle_cast<GLSamplerGroup *>(sbh);
    *sb->sb = std::move(samplerGroup); // NOLINT(performance-move-const-arg)
}

void OpenGLDriver::update2DImage(Handle<HwTexture> th,
        uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& data) {
    DEBUG_MARKER()

    GLTexture* t = handle_cast<GLTexture *>(th);
    if (data.type == PixelDataType::COMPRESSED) {
        setCompressedTextureData(t,
                level, xoffset, yoffset, 0, width, height, 1, std::move(data), nullptr);
    } else {
        setTextureData(t,
                level, xoffset, yoffset, 0, width, height, 1, std::move(data), nullptr);
    }
}

void OpenGLDriver::updateCubeImage(Handle<HwTexture> th, uint32_t level,
        PixelBufferDescriptor&& data, FaceOffsets faceOffsets) {
    DEBUG_MARKER()

    GLTexture* t = handle_cast<GLTexture *>(th);
    if (data.type == PixelDataType::COMPRESSED) {
        setCompressedTextureData(t, level, 0, 0, 0, 0, 0, 0, std::move(data), &faceOffsets);
    } else {
        setTextureData(t, level, 0, 0, 0, 0, 0, 0, std::move(data), &faceOffsets);
    }
}

void OpenGLDriver::generateMipmaps(Handle<HwTexture> th) {
    DEBUG_MARKER()

    GLTexture* t = handle_cast<GLTexture *>(th);
    assert(t->gl.target != GL_TEXTURE_2D_MULTISAMPLE);
    // Note: glGenerateMimap can also fail if the internal format is not both
    // color-renderable and filterable (i.e.: doesn't work for depth)
    bindTexture(MAX_TEXTURE_UNIT_COUNT - 1, t);
    activeTexture(MAX_TEXTURE_UNIT_COUNT - 1);

    t->gl.baseLevel = 0;
    t->gl.maxLevel = static_cast<uint8_t>(t->levels - 1);

    glTexParameteri(t->gl.target, GL_TEXTURE_BASE_LEVEL, t->gl.baseLevel);
    glTexParameteri(t->gl.target, GL_TEXTURE_MAX_LEVEL, t->gl.maxLevel);

    glGenerateMipmap(t->gl.target);

    CHECK_GL_ERROR(utils::slog.e)
}

bool OpenGLDriver::canGenerateMipmaps() {
    return true;
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
            bindTexture(MAX_TEXTURE_UNIT_COUNT - 1, t);
            activeTexture(MAX_TEXTURE_UNIT_COUNT - 1);
            switch (t->gl.target) {
                case GL_TEXTURE_2D:
                    glTexSubImage2D(t->gl.target, GLint(level),
                            GLint(xoffset), GLint(yoffset),
                            width, height, glFormat, glType, p.buffer);
                    break;
                case GL_TEXTURE_2D_ARRAY:
                    glTexSubImage3D(t->gl.target, GLint(level),
                            GLint(xoffset), GLint(yoffset), GLint(zoffset),
                            width, height, depth, glFormat, glType, p.buffer);
                    break;
                default:
                    // we shouldn't be here
                    break;
            }
            break;
        case SamplerType::SAMPLER_CUBEMAP: {
            assert(t->gl.target == GL_TEXTURE_CUBE_MAP);
            bindTexture(MAX_TEXTURE_UNIT_COUNT - 1, t);
            activeTexture(MAX_TEXTURE_UNIT_COUNT - 1);
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

    if (int8_t(level) < t->gl.baseLevel) {
        t->gl.baseLevel = int8_t(level);
        glTexParameteri(t->gl.target, GL_TEXTURE_BASE_LEVEL, t->gl.baseLevel);
    }
    if (int8_t(level) > t->gl.maxLevel) {
        t->gl.maxLevel = int8_t(level);
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
            bindTexture(MAX_TEXTURE_UNIT_COUNT - 1, t);
            activeTexture(MAX_TEXTURE_UNIT_COUNT - 1);
            switch (t->gl.target) {
                case GL_TEXTURE_2D:
                    glCompressedTexSubImage2D(t->gl.target, GLint(level),
                            GLint(xoffset), GLint(yoffset),
                            width, height, t->gl.internalFormat, imageSize, p.buffer);
                    break;
                case GL_TEXTURE_2D_ARRAY:
                    glCompressedTexSubImage3D(t->gl.target, GLint(level),
                            GLint(xoffset), GLint(yoffset), GLint(zoffset),
                            width, height, depth, t->gl.internalFormat, imageSize, p.buffer);
                    break;
                default:
                    // we shouldn't be here
                    break;
            }
            break;
        case SamplerType::SAMPLER_CUBEMAP: {
            assert(faceOffsets);
            assert(t->gl.target == GL_TEXTURE_CUBE_MAP);
            bindTexture(MAX_TEXTURE_UNIT_COUNT - 1, t);
            activeTexture(MAX_TEXTURE_UNIT_COUNT - 1);
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

void OpenGLDriver::setupExternalImage(void* image) {
}

void OpenGLDriver::cancelExternalImage(void* image) {
}

void OpenGLDriver::setExternalImage(Handle<HwTexture> th, void* image) {
    if (ext.OES_EGL_image_external_essl3) {
        DEBUG_MARKER()

        GLTexture* t = handle_cast<GLTexture*>(th);

        assert(t->target == SamplerType::SAMPLER_EXTERNAL);
        assert(t->gl.target == GL_TEXTURE_EXTERNAL_OES);

        bindTexture(MAX_TEXTURE_UNIT_COUNT - 1, t);
        activeTexture(MAX_TEXTURE_UNIT_COUNT - 1);

#ifdef GL_OES_EGL_image
        glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, static_cast<GLeglImageOES>(image));
#endif
    }
}

void OpenGLDriver::setExternalStream(Handle<HwTexture> th, Handle<HwStream> sh) {
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
        mPlatform.attach(hwStream->stream, t->gl.id);
    } else {
        assert(t->target == SamplerType::SAMPLER_EXTERNAL);
        // The texture doesn't need a texture name anymore, get rid of it
        unbindTexture(t->gl.target, t->gl.id);
        glDeleteTextures(1, &t->gl.id);
        t->gl.id = hwStream->user_thread.read[hwStream->user_thread.cur];
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
    glGenTextures(1, &t->gl.id);
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
        glGenTextures(1, &t->gl.id);
        mPlatform.attach(hwStream->stream, t->gl.id);
    } else {
        assert(t->target == SamplerType::SAMPLER_EXTERNAL);
        t->gl.id = hwStream->user_thread.read[hwStream->user_thread.cur];
    }
    t->hwStream = hwStream;
}

void OpenGLDriver::beginRenderPass(Handle<HwRenderTarget> rth,
        const RenderPassParams& params) {
    DEBUG_MARKER()

    mRenderPassTarget = rth;
    mRenderPassParams = params;
    const uint8_t clearFlags = params.flags.clear;
    TargetBufferFlags discardFlags = params.flags.discardStart;

    GLRenderTarget* rt = handle_cast<GLRenderTarget*>(rth);
    if (UTILS_UNLIKELY(state.draw_fbo != rt->gl.fbo)) {
        bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);

        // glInvalidateFramebuffer appeared on GLES 3.0 and GL4.3, for simplicity we just
        // ignore it on GL (rather than having to do a runtime check).
        if (GLES31_HEADERS) {
            if (!bugs.disable_invalidate_framebuffer) {
                std::array<GLenum, 3> attachments; // NOLINT(cppcoreguidelines-pro-type-member-init)
                GLsizei attachmentCount = getAttachments(attachments, rt, discardFlags);
                if (attachmentCount) {
                    glInvalidateFramebuffer(GL_FRAMEBUFFER, attachmentCount, attachments.data());
                }
                CHECK_GL_ERROR(utils::slog.e)
            }
        }
    }

    if (rt->gl.fbo_read) {
        // we have a multi-sample RenderTarget with non multi-sample attachments (i.e. this is the
        // EXT_multisampled_render_to_texture emulation).
        // We need to perform a "backward" resolve, i.e. load the resolved texture into the tile,
        // everything must appear as though the multi-sample buffer was lost.
        if (ALLOW_REVERSE_MULTISAMPLE_RESOLVE) {
            // We only copy the non msaa buffers that were not discarded or cleared.
            const TargetBufferFlags discarded = discardFlags |
                    TargetBufferFlags(clearFlags & TargetBufferFlags::ALL);
            resolvePass(ResolveAction::LOAD, rt, discarded);
        } else {
            // However, for now filament specifies that a non multi-sample attachment to a
            // multi-sample RenderTarget is always discarded. We do this because implementing
            // the load on Metal is not trivial and it's not a feature we rely on at this time.
            discardFlags |= rt->gl.resolve;
        }
    }

    setViewport(params.viewport.left, params.viewport.bottom,
            params.viewport.width, params.viewport.height);

    setScissor(params.viewport.left, params.viewport.bottom,
            params.viewport.width, params.viewport.height);

    if (clearFlags & TargetBufferFlags::ALL) {
        const bool respectScissor = !(clearFlags & RenderPassFlags::IGNORE_SCISSOR);
        const bool clearColor = clearFlags & TargetBufferFlags::COLOR;
        const bool clearDepth = clearFlags & TargetBufferFlags::DEPTH;
        const bool clearStencil = clearFlags & TargetBufferFlags::STENCIL;
        if (respectScissor) {
            enable(GL_SCISSOR_TEST);
        } else {
            disable(GL_SCISSOR_TEST);
        }
        if (respectScissor && GLES31_HEADERS && bugs.clears_hurt_performance) {
            // With OpenGL ES, we clear the viewport using geometry to improve performance on certain
            // OpenGL drivers. e.g. on Adreno this avoids needless loads from the GMEM.
            clearWithGeometryPipe(clearColor, params.clearColor,
                    clearDepth, params.clearDepth,
                    clearStencil, params.clearStencil);
        } else {
            // With OpenGL we always clear using glClear()
            clearWithRasterPipe(clearColor, params.clearColor,
                    clearDepth, params.clearDepth,
                    clearStencil, params.clearStencil);
        }
    }

#ifndef NDEBUG
    // clear the discarded (but not the cleared ones) buffers in debug builds
    setClearColor(1, 0, 0, 1);
    bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);
    disable(GL_SCISSOR_TEST);
    glClear(getAttachmentBitfield(discardFlags & ~TargetBufferFlags(clearFlags)));
#endif
}

void OpenGLDriver::endRenderPass(int) {
    DEBUG_MARKER()
    assert(mRenderPassTarget);

    GLRenderTarget const* const rt = handle_cast<GLRenderTarget*>(mRenderPassTarget);

    const TargetBufferFlags discardFlags = mRenderPassParams.flags.discardEnd;
    resolvePass(ResolveAction::STORE, rt, discardFlags);

    // glInvalidateFramebuffer appeared on GLES 3.0 and GL4.3, for simplicity we just
    // ignore it on GL (rather than having to do a runtime check).
    if (GLES31_HEADERS) {
        if (!bugs.disable_invalidate_framebuffer) {
            // we wouldn't have to bind the framebuffer if we had glInvalidateNamedFramebuffer()
            bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);
            std::array<GLenum, 3> attachments; // NOLINT(cppcoreguidelines-pro-type-member-init)
            GLsizei attachmentCount = getAttachments(attachments, rt, discardFlags);
            if (attachmentCount) {
                glInvalidateFramebuffer(GL_FRAMEBUFFER, attachmentCount, attachments.data());
            }
           CHECK_GL_ERROR(utils::slog.e)
        }
    }

#ifndef NDEBUG
    // clear the discarded buffers in debug builds
    setClearColor(0, 1, 0, 1);
    bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);
    disable(GL_SCISSOR_TEST);
    glClear(getAttachmentBitfield(discardFlags));
#endif

    mRenderPassTarget.clear();
}


void OpenGLDriver::resolvePass(ResolveAction action, GLRenderTarget const* rt,
        backend::TargetBufferFlags discardFlags) noexcept {
    const TargetBufferFlags resolve = rt->gl.resolve & ~discardFlags;
    GLbitfield mask = getAttachmentBitfield(resolve);
    if (UTILS_UNLIKELY(mask)) {
        GLint read = rt->gl.fbo_read;
        GLint draw = rt->gl.fbo;
        if (action == ResolveAction::STORE) {
            std::swap(read, draw);
        }
        bindFramebuffer(GL_READ_FRAMEBUFFER, read);
        bindFramebuffer(GL_DRAW_FRAMEBUFFER, draw);
        disable(GL_SCISSOR_TEST);
        glBlitFramebuffer(0, 0, rt->width, rt->height, 0, 0, rt->width, rt->height, mask, GL_NEAREST);
        CHECK_GL_ERROR(utils::slog.e)
    }
}

void OpenGLDriver::discardSubRenderTargetBuffers(Handle<HwRenderTarget> rth,
        TargetBufferFlags buffers,
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

void OpenGLDriver::setRenderPrimitiveBuffer(Handle<HwRenderPrimitive> rph,
        Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh,
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

void OpenGLDriver::setRenderPrimitiveRange(Handle<HwRenderPrimitive> rph,
        PrimitiveType pt, uint32_t offset,
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

// Sets up a scissor rectangle that automatically gets clipped against the viewport.
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
    setScissor(scissor.x, scissor.y, scissor.z, scissor.w);
}

/*
 * This is called in the user thread
 */

#define DEBUG_NO_EXTERNAL_STREAM_COPY false

void OpenGLDriver::updateStream(GLTexture* t, DriverApi* driver) noexcept {
    SYSTRACE_CALL();

    GLStream* s = static_cast<GLStream*>(t->hwStream);
    assert(s);
    assert(!s->isNativeStream());

    // round-robin to the next texture name
    if (UTILS_UNLIKELY(DEBUG_NO_EXTERNAL_STREAM_COPY ||
                       bugs.disable_shared_context_draws || !mOpenGLBlitter)) {
        driver->queueCommand([this, t, s]() {
            // the stream may have been destroyed since we enqueued the command
            // also make sure that this texture is still associated with the same stream
            auto& streams = mExternalStreams;
            if (UTILS_LIKELY(std::find(streams.begin(), streams.end(), t) != streams.end()) &&
                (t->hwStream == s)) {
                t->gl.id = s->gl.externalTextureId;
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
            mPlatform.reallocateExternalStorage(ets, info.width, info.height, TextureFormat::RGB8);

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
                t->gl.id = readTexture;
                t->gl.fence = fence;
                s->gl.externalTexture2DId = writeTexture;
            } else {
                glDeleteSync(fence);
            }
        });
    }
}

void OpenGLDriver::readStreamPixels(Handle<HwStream> sh,
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

void OpenGLDriver::bindUniformBuffer(size_t index, Handle<HwUniformBuffer> ubh) {
    DEBUG_MARKER()
    GLUniformBuffer* ub = handle_cast<GLUniformBuffer *>(ubh);
    assert(ub->gl.ubo.base == 0);
    bindBufferRange(GL_UNIFORM_BUFFER, GLuint(index), ub->gl.ubo.id, 0, ub->gl.ubo.capacity);
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::bindUniformBufferRange(size_t index, Handle<HwUniformBuffer> ubh,
        size_t offset, size_t size) {
    DEBUG_MARKER()

    GLUniformBuffer* ub = handle_cast<GLUniformBuffer*>(ubh);
    // TODO: Is this assert really needed? Note that size is only populated for STREAM buffers.
    assert(size <= ub->gl.ubo.size);
    assert(ub->gl.ubo.base + offset + size <= ub->gl.ubo.capacity);
    bindBufferRange(GL_UNIFORM_BUFFER, GLuint(index), ub->gl.ubo.id, ub->gl.ubo.base + offset, size);
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::bindSamplers(size_t index, Handle<HwSamplerGroup> sbh) {
    DEBUG_MARKER()

    GLSamplerGroup* sb = handle_cast<GLSamplerGroup *>(sbh);
    assert(index < Program::SAMPLER_BINDING_COUNT);
    mSamplerBindings[index] = sb;
    CHECK_GL_ERROR(utils::slog.e)
}


GLuint OpenGLDriver::getSamplerSlow(SamplerParams params) const noexcept {
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

void OpenGLDriver::readPixels(Handle<HwRenderTarget> src,
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
        OpenGLPlatform& platform = mPlatform;
        const size_t index = getIndexForTextureTarget(GL_TEXTURE_EXTERNAL_OES);
        for (GLTexture const* t : mExternalStreams) {
            assert(t && t->hwStream);
            if (static_cast<GLStream*>(t->hwStream)->isNativeStream()) {
                assert(t->hwStream->stream);
                platform.updateTexImage(t->hwStream->stream, &static_cast<GLStream*>(t->hwStream)->user_thread.timestamp);
                // NOTE: We assume that updateTexImage() binds the texture on our behalf
                GLuint activeUnit = state.textures.active;
                state.textures.units[activeUnit].targets[index].texture_id = t->gl.id;
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
    if (!bugs.disable_glFlush) {
        glFlush();
    }
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
        Handle<HwRenderTarget> dst, Viewport dstRect,
        Handle<HwRenderTarget> src, Viewport srcRect,
        SamplerMagFilter filter) {
    DEBUG_MARKER()

    GLbitfield mask = getAttachmentBitfield(buffers);
    if (mask) {
        GLenum glFilterMode = (filter == SamplerMagFilter::NEAREST) ? GL_NEAREST : GL_LINEAR;
        if (mask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) {
            // GL_INVALID_OPERATION is generated if mask contains any of the GL_DEPTH_BUFFER_BIT or
            // GL_STENCIL_BUFFER_BIT and filter is not GL_NEAREST.
            glFilterMode = GL_NEAREST;
        }

        // note: for msaa RenderTargets withh non-msaa attachments, we copy from the msaa sidecar
        // buffer -- this should produce the same output that if we copied from the resolved
        // texture. EXT_multisampled_render_to_texture seems to allow both behaviours, and this
        // is an emulation of that.  We cannot use the resolved texture easily because it's not
        // actually attached to the this RenderTarget. Another implementation would be to do a
        // reverse-resolve, but that wouldn't buy us anything.
        GLRenderTarget const* s = handle_cast<GLRenderTarget const*>(src);
        GLRenderTarget const* d = handle_cast<GLRenderTarget const*>(dst);

        if (!ALLOW_REVERSE_MULTISAMPLE_RESOLVE) {
            // With GLES 3.x, GL_INVALID_OPERATION is generated if the value of GL_SAMPLE_BUFFERS
            // for the draw buffer is greater than zero. This works with OpenGL, so we want to
            // make sure to catch this scenario.
            assert(d->gl.samples <= 1);
        }

        // GL_INVALID_OPERATION is generated if GL_SAMPLE_BUFFERS for the read buffer is greater
        // than zero and the formats of draw and read buffers are not identical.
        // However, it's not well defined in the spec what "format" means. So it's difficult
        // to have an assert here -- especially when dealing with the default framebuffer

        // GL_INVALID_OPERATION is generated if GL_SAMPLE_BUFFERS for the read buffer is greater
        // than zero and (...) the source and destination rectangles are not defined with the
        // same (X0, Y0) and (X1, Y1) bounds.
        if (s->gl.samples > 1) {
            assert(!memcmp(&dstRect, &srcRect, sizeof(srcRect)));
        }

        bindFramebuffer(GL_READ_FRAMEBUFFER, s->gl.fbo);
        bindFramebuffer(GL_DRAW_FRAMEBUFFER, d->gl.fbo);
        disable(GL_SCISSOR_TEST);
        glBlitFramebuffer(
                srcRect.left, srcRect.bottom, srcRect.right(), srcRect.top(),
                dstRect.left, dstRect.bottom, dstRect.right(), dstRect.top(),
                mask, glFilterMode);
        CHECK_GL_ERROR(utils::slog.e)

        // In a sense, blitting to a texture level is similar to calling setTextureData on it; in
        // both cases, we update the base/max LOD to give shaders access to levels as they become
        // available.
        if (mask & GL_COLOR_BUFFER_BIT) {
            updateTextureLodRange(d->gl.color.texture, d->gl.color.level);
        }
        if (mask & GL_DEPTH_BUFFER_BIT) {
            updateTextureLodRange(d->gl.depth.texture, d->gl.depth.level);
        }
        if (mask & GL_STENCIL_BUFFER_BIT) {
            updateTextureLodRange(d->gl.stencil.texture, d->gl.stencil.level);
        }
    }
}

void OpenGLDriver::updateTextureLodRange(GLTexture* texture, int8_t targetLevel) noexcept {
    if (texture && (texture->usage & TextureUsage::SAMPLEABLE)) {
        if (targetLevel < texture->gl.baseLevel || targetLevel > texture->gl.maxLevel) {
            bindTexture(MAX_TEXTURE_UNIT_COUNT - 1, texture);
            activeTexture(MAX_TEXTURE_UNIT_COUNT - 1);
            if (targetLevel < texture->gl.baseLevel) {
                texture->gl.baseLevel = targetLevel;
                glTexParameteri(texture->gl.target, GL_TEXTURE_BASE_LEVEL, targetLevel);
            }
            if (targetLevel > texture->gl.maxLevel) {
                texture->gl.maxLevel = targetLevel;
                glTexParameteri(texture->gl.target, GL_TEXTURE_MAX_LEVEL, targetLevel);
            }
        }
        CHECK_GL_ERROR(utils::slog.e)
    }
}

void OpenGLDriver::draw(PipelineState state, Handle<HwRenderPrimitive> rph) {
    DEBUG_MARKER()

    OpenGLProgram* p = handle_cast<OpenGLProgram*>(state.program);
    useProgram(p);

    const GLRenderPrimitive* rp = handle_cast<const GLRenderPrimitive *>(rph);
    bindVertexArray(rp);

    setRasterState(state.rasterState);

    polygonOffset(state.polygonOffset.slope, state.polygonOffset.constant);

    enable(GL_SCISSOR_TEST);

    glDrawRangeElements(GLenum(rp->type), rp->minIndex, rp->maxIndex, rp->count,
            rp->gl.indicesType, reinterpret_cast<const void*>(rp->offset));

    CHECK_GL_ERROR(utils::slog.e)
}

// explicit instantiation of the Dispatcher
template class backend::ConcreteDispatcher<OpenGLDriver>;

} // namespace filament
