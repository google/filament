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
#include "OpenGLTimerQuery.h"
#include "OpenGLContext.h"

#include <utils/compiler.h>
#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/Systrace.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

// We can only support this feature on OpenGL ES 3.1+
// Support is currently disabled as we don't need it
#define TEXTURE_2D_MULTISAMPLE_SUPPORTED false

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

namespace filament::backend {

Driver* OpenGLDriverFactory::create(
        OpenGLPlatform* const platform, void* const sharedGLContext) noexcept {
    return OpenGLDriver::create(platform, sharedGLContext);
}

using namespace GLUtils;

// ------------------------------------------------------------------------------------------------

UTILS_NOINLINE
Driver* OpenGLDriver::create(
        OpenGLPlatform* const platform, void* const sharedGLContext) noexcept {
    assert_invariant(platform);
    OpenGLPlatform* const ec = platform;

#if 0
    // this is useful for development, but too verbose even for debug builds
    // For reference on a 64-bits machine in Release mode:
    //    GLFence                   :   8       few
    //    GLIndexBuffer             :   8       moderate
    //    GLSamplerGroup            :   8       few
    // -- less than or equal 16 bytes
    //    GLBufferObject            :  24       many
    //    GLSync                    :  24       few
    //    GLTimerQuery              :  32       few
    //    OpenGLProgram             :  32       moderate
    //    GLRenderPrimitive         :  48       many
    // -- less than or equal 64 bytes
    //    GLTexture                 :  72       moderate
    //    GLRenderTarget            : 112       few
    //    GLStream                  : 184       few
    //    GLVertexBuffer            : 200       moderate
    // -- less than or equal to 208 bytes

    slog.d
           << "HwFence: " << sizeof(HwFence)
           << "\nGLBufferObject: " << sizeof(GLBufferObject)
           << "\nGLVertexBuffer: " << sizeof(GLVertexBuffer)
           << "\nGLIndexBuffer: " << sizeof(GLIndexBuffer)
           << "\nGLSamplerGroup: " << sizeof(GLSamplerGroup)
           << "\nGLRenderPrimitive: " << sizeof(GLRenderPrimitive)
           << "\nGLTexture: " << sizeof(GLTexture)
           << "\nGLTimerQuery: " << sizeof(GLTimerQuery)
           << "\nGLStream: " << sizeof(GLStream)
           << "\nGLRenderTarget: " << sizeof(GLRenderTarget)
           << "\nGLSync: " << sizeof(GLSync)
           << "\nOpenGLProgram: " << sizeof(OpenGLProgram)
           << io::endl;
#endif

    // here we check we're on a supported version of GL before initializing the driver
    GLint major = 0, minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    if (UTILS_UNLIKELY(glGetError() != GL_NO_ERROR)) {
        PANIC_LOG("Can't get OpenGL version");
        cleanup:
        ec->terminate();
        return {};
    }

    if constexpr (BACKEND_OPENGL_VERSION == BACKEND_OPENGL_VERSION_GLES) {
        if (UTILS_UNLIKELY(!(major >= 3 && minor >= 0))) {
            PANIC_LOG("OpenGL ES 3.0 minimum needed (current %d.%d)", major, minor);
            goto cleanup;
        }
    } else if constexpr (BACKEND_OPENGL_VERSION == BACKEND_OPENGL_VERSION_GL) {
        // we require GL 4.1 headers and minimum version
        if (UTILS_UNLIKELY(!((major == 4 && minor >= 1) || major > 4))) {
            PANIC_LOG("OpenGL 4.1 minimum needed (current %d.%d)", major, minor);
            goto cleanup;
        }
    }

    OpenGLDriver* const driver = new OpenGLDriver(ec);
    return driver;
}

// ------------------------------------------------------------------------------------------------

OpenGLDriver::DebugMarker::DebugMarker(OpenGLDriver& driver, const char* string) noexcept
        : driver(driver) {
    // FIXME: this is not safe nor portable
    const char* const begin = string + sizeof("virtual void filament::backend::OpenGLDriver::") - 1;
    const char* const end = strchr(begin, '(');
    driver.pushGroupMarker(begin, end - begin);
}

OpenGLDriver::DebugMarker::~DebugMarker() noexcept {
    driver.popGroupMarker();
}

// ------------------------------------------------------------------------------------------------

OpenGLDriver::OpenGLDriver(OpenGLPlatform* platform) noexcept
        : mHandleAllocator("Handles", FILAMENT_OPENGL_HANDLE_ARENA_SIZE_IN_MB * 1024U * 1024U), // TODO: set the amount in configuration
          mSamplerMap(32),
          mPlatform(*platform) {
  
    std::fill(mSamplerBindings.begin(), mSamplerBindings.end(), nullptr);

    // set a reasonable default value for our stream array
    mExternalStreams.reserve(8);

#ifndef NDEBUG
    slog.i << "OS version: " << mPlatform.getOSVersion() << io::endl;
#endif

    // Initialize the blitter only if we have OES_EGL_image_external_essl3
    if (mContext.ext.OES_EGL_image_external_essl3) {
        mOpenGLBlitter = new OpenGLBlitter(mContext);
        mOpenGLBlitter->init();
        mContext.resetProgram();
    }

    if (mContext.ext.EXT_disjoint_timer_query ||
            BACKEND_OPENGL_VERSION == BACKEND_OPENGL_VERSION_GL) {
        // timer queries are available
        if (mContext.bugs.dont_use_timer_query && mPlatform.canCreateFence()) {
            // however, they don't work well, revert to using fences if we can.
            mTimerQueryImpl = new OpenGLTimerQueryFence(mPlatform);
        } else {
            mTimerQueryImpl = new TimerQueryNative(mContext);
        }
        mFrameTimeSupported = true;
    } else if (mPlatform.canCreateFence()) {
        // no timer queries, but we can use fences
        mTimerQueryImpl = new OpenGLTimerQueryFence(mPlatform);
        mFrameTimeSupported = true;
    } else {
        // no queries, no fences -- that's a problem
        mTimerQueryImpl = new TimerQueryFallback();
        mFrameTimeSupported = false;
    }
}

OpenGLDriver::~OpenGLDriver() noexcept {
    delete mOpenGLBlitter;
}

Dispatcher OpenGLDriver::getDispatcher() const noexcept {
    return ConcreteDispatcher<OpenGLDriver>::make();
}

// ------------------------------------------------------------------------------------------------
// Driver interface concrete implementation
// ------------------------------------------------------------------------------------------------

void OpenGLDriver::terminate() {
    // wait for the GPU to finish executing all commands
    glFinish();

    // and make sure to execute all the GpuCommandCompleteOps callbacks
    executeGpuCommandsCompleteOps();

    // because we called glFinish(), all callbacks should have been executed
    assert_invariant(mGpuCommandCompleteOps.empty());

    for (auto& item : mSamplerMap) {
        mContext.unbindSampler(item.second);
        glDeleteSamplers(1, &item.second);
    }
    mSamplerMap.clear();
    if (mOpenGLBlitter) {
        mOpenGLBlitter->terminate();
    }

    delete mTimerQueryImpl;

    mPlatform.terminate();
}

ShaderModel OpenGLDriver::getShaderModel() const noexcept {
    return mContext.getShaderModel();
}

// ------------------------------------------------------------------------------------------------
// Change and track GL state
// ------------------------------------------------------------------------------------------------

void OpenGLDriver::bindSampler(GLuint unit, SamplerParams params) noexcept {
    mContext.bindSampler(unit, getSampler(params));
}

void OpenGLDriver::bindTexture(GLuint unit, GLTexture const* t) noexcept {
    assert_invariant(t != nullptr);
    mContext.bindTexture(unit, t->gl.target, t->gl.id, t->gl.targetIndex);
}

void OpenGLDriver::useProgram(OpenGLProgram* p) noexcept {
    // set-up textures and samplers in the proper TMUs (as specified in setSamplers)
    p->use(this, mContext);
}


void OpenGLDriver::setRasterStateSlow(RasterState rs) noexcept {
    mRasterState = rs;
    auto& gl = mContext;

    // culling state
    switch (rs.culling) {
        case CullingMode::NONE:
            gl.disable(GL_CULL_FACE);
            break;
        case CullingMode::FRONT:
            gl.cullFace(GL_FRONT);
            break;
        case CullingMode::BACK:
            gl.cullFace(GL_BACK);
            break;
        case CullingMode::FRONT_AND_BACK:
            gl.cullFace(GL_FRONT_AND_BACK);
            break;
    }

    gl.frontFace(rs.inverseFrontFaces ? GL_CW : GL_CCW);

    if (rs.culling != CullingMode::NONE) {
        gl.enable(GL_CULL_FACE);
    }

    // blending state
    if (!rs.hasBlending()) {
        gl.disable(GL_BLEND);
    } else {
        gl.enable(GL_BLEND);
        gl.blendEquation(
                getBlendEquationMode(rs.blendEquationRGB),
                getBlendEquationMode(rs.blendEquationAlpha));

        gl.blendFunction(
                getBlendFunctionMode(rs.blendFunctionSrcRGB),
                getBlendFunctionMode(rs.blendFunctionSrcAlpha),
                getBlendFunctionMode(rs.blendFunctionDstRGB),
                getBlendFunctionMode(rs.blendFunctionDstAlpha));
    }

    // depth test
    if (rs.depthFunc == RasterState::DepthFunc::A && !rs.depthWrite) {
        gl.disable(GL_DEPTH_TEST);
    } else {
        gl.enable(GL_DEPTH_TEST);
        gl.depthFunc(getDepthFunc(rs.depthFunc));
        gl.depthMask(GLboolean(rs.depthWrite));
    }

    // write masks
    gl.colorMask(GLboolean(rs.colorWrite));

    // AA
    if (rs.alphaToCoverage) {
        gl.enable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    } else {
        gl.disable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    }
}

// ------------------------------------------------------------------------------------------------
// Creating driver objects
// ------------------------------------------------------------------------------------------------

Handle<HwVertexBuffer> OpenGLDriver::createVertexBufferS() noexcept {
    return initHandle<GLVertexBuffer>();
}

Handle<HwIndexBuffer> OpenGLDriver::createIndexBufferS() noexcept {
    return initHandle<GLIndexBuffer>();
}

Handle<HwBufferObject> OpenGLDriver::createBufferObjectS() noexcept {
    return initHandle<GLBufferObject>();
}

Handle<HwRenderPrimitive> OpenGLDriver::createRenderPrimitiveS() noexcept {
    return initHandle<GLRenderPrimitive>();
}

Handle<HwProgram> OpenGLDriver::createProgramS() noexcept {
    return initHandle<OpenGLProgram>();
}

Handle<HwSamplerGroup> OpenGLDriver::createSamplerGroupS() noexcept {
    return initHandle<GLSamplerGroup>();
}

Handle<HwTexture> OpenGLDriver::createTextureS() noexcept {
    return initHandle<GLTexture>();
}

Handle<HwTexture> OpenGLDriver::createTextureSwizzledS() noexcept {
    return initHandle<GLTexture>();
}

Handle<HwTexture> OpenGLDriver::importTextureS() noexcept {
    return initHandle<GLTexture>();
}

Handle<HwRenderTarget> OpenGLDriver::createDefaultRenderTargetS() noexcept {
    return initHandle<GLRenderTarget>();
}

Handle<HwRenderTarget> OpenGLDriver::createRenderTargetS() noexcept {
    return initHandle<GLRenderTarget>();
}

Handle<HwFence> OpenGLDriver::createFenceS() noexcept {
    return initHandle<HwFence>();
}

Handle<HwSync> OpenGLDriver::createSyncS() noexcept {
    return initHandle<GLSync>();
}

Handle<HwSwapChain> OpenGLDriver::createSwapChainS() noexcept {
    return initHandle<HwSwapChain>();
}

Handle<HwSwapChain> OpenGLDriver::createSwapChainHeadlessS() noexcept {
    return initHandle<HwSwapChain>();
}

Handle<HwStream> OpenGLDriver::createStreamFromTextureIdS() noexcept {
    return initHandle<GLStream>();
}

Handle<HwTimerQuery> OpenGLDriver::createTimerQueryS() noexcept {
    return initHandle<GLTimerQuery>();
}

void OpenGLDriver::createVertexBufferR(
        Handle<HwVertexBuffer> vbh,
        uint8_t bufferCount,
        uint8_t attributeCount,
        uint32_t elementCount,
        AttributeArray attributes) {
    DEBUG_MARKER()
    construct<GLVertexBuffer>(vbh, bufferCount, attributeCount, elementCount, attributes);
}

void OpenGLDriver::createIndexBufferR(
        Handle<HwIndexBuffer> ibh,
        ElementType elementType,
        uint32_t indexCount,
        BufferUsage usage) {
    DEBUG_MARKER()

    auto& gl = mContext;
    uint8_t elementSize = static_cast<uint8_t>(getElementTypeSize(elementType));
    GLIndexBuffer* ib = construct<GLIndexBuffer>(ibh, elementSize, indexCount);
    glGenBuffers(1, &ib->gl.buffer);
    GLsizeiptr size = elementSize * indexCount;
    gl.bindVertexArray(nullptr);
    gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->gl.buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, getBufferUsage(usage));
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createBufferObjectR(Handle<HwBufferObject> boh,
        uint32_t byteCount, BufferObjectBinding bindingType, BufferUsage usage) {
    DEBUG_MARKER()
    assert_invariant(byteCount > 0);

    auto& gl = mContext;
    if (bindingType == BufferObjectBinding::VERTEX) {
        gl.bindVertexArray(nullptr);
    }

    GLBufferObject* bo = construct<GLBufferObject>(boh, byteCount, bindingType, usage);
    glGenBuffers(1, &bo->gl.id);
    gl.bindBuffer(bo->gl.binding, bo->gl.id);
    glBufferData(bo->gl.binding, byteCount, nullptr, getBufferUsage(usage));
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createRenderPrimitiveR(Handle<HwRenderPrimitive> rph, int) {
    DEBUG_MARKER()

    GLRenderPrimitive* rp = handle_cast<GLRenderPrimitive*>(rph);
    glGenVertexArrays(1, &rp->gl.vao);
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createProgramR(Handle<HwProgram> ph, Program&& program) {
    DEBUG_MARKER()

    construct<OpenGLProgram>(ph, *this, std::move(program));
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createSamplerGroupR(Handle<HwSamplerGroup> sbh, uint32_t size) {
    DEBUG_MARKER()

    construct<GLSamplerGroup>(sbh, size);
}

UTILS_NOINLINE
void OpenGLDriver::textureStorage(OpenGLDriver::GLTexture* t,
        uint32_t width, uint32_t height, uint32_t depth) noexcept {

    auto& gl = mContext;

    bindTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1, t);
    gl.activeTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1);

    switch (t->gl.target) {
        case GL_TEXTURE_2D:
        case GL_TEXTURE_CUBE_MAP:
            glTexStorage2D(t->gl.target, GLsizei(t->levels), t->gl.internalFormat,
                    GLsizei(width), GLsizei(height));
            break;
        case GL_TEXTURE_3D:
        case GL_TEXTURE_2D_ARRAY: {
            glTexStorage3D(t->gl.target, GLsizei(t->levels), t->gl.internalFormat,
                    GLsizei(width), GLsizei(height), GLsizei(depth));
            break;
        }
        case GL_TEXTURE_2D_MULTISAMPLE:
            if constexpr (TEXTURE_2D_MULTISAMPLE_SUPPORTED) {
                // NOTE: if there is a mix of texture and renderbuffers, "fixed_sample_locations" must be true
                // NOTE: what's the benefit of setting "fixed_sample_locations" to false?
#if BACKEND_OPENGL_LEVEL >= BACKEND_OPENGL_LEVEL_GLES31
                // only supported from GL 4.3 and GLES 3.1 headers
                glTexStorage2DMultisample(t->gl.target, t->samples, t->gl.internalFormat,
                        GLsizei(width), GLsizei(height), GL_TRUE);
#elif defined(GL_VERSION_4_1)
                // only supported in GL (GL4.1 doesn't support glTexStorage2DMultisample)
                glTexImage2DMultisample(t->gl.target, t->samples, t->gl.internalFormat,
                        GLsizei(width), GLsizei(height), GL_TRUE);
#endif
            } else {
                PANIC_LOG("GL_TEXTURE_2D_MULTISAMPLE is not supported");
            }
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

    auto& gl = mContext;
    samples = std::clamp(samples, uint8_t(1u), uint8_t(gl.gets.max_samples));
    GLTexture* t = construct<GLTexture>(th, target, levels, samples, w, h, depth, format, usage);
    if (UTILS_LIKELY(usage & TextureUsage::SAMPLEABLE)) {
        if (UTILS_UNLIKELY(t->target == SamplerType::SAMPLER_EXTERNAL)) {
            mPlatform.createExternalImageTexture(t);
        } else {
            glGenTextures(1, &t->gl.id);

            t->gl.internalFormat = getInternalFormat(format);
            assert_invariant(t->gl.internalFormat);

            // We DO NOT update targetIndex at function exit to take advantage of the fact that
            // getIndexForTextureTarget() is constexpr -- so all of this disappears at compile time.
            switch (target) {
                case SamplerType::SAMPLER_EXTERNAL:
                    // we can't be here -- doesn't mater what we do
                case SamplerType::SAMPLER_2D:
                    t->gl.target = GL_TEXTURE_2D;
                    t->gl.targetIndex = (uint8_t)gl.getIndexForTextureTarget(GL_TEXTURE_2D);
                    break;
                case SamplerType::SAMPLER_3D:
                    t->gl.target = GL_TEXTURE_3D;
                    t->gl.targetIndex = (uint8_t)gl.getIndexForTextureTarget(GL_TEXTURE_3D);
                    break;
                case SamplerType::SAMPLER_2D_ARRAY:
                    t->gl.target = GL_TEXTURE_2D_ARRAY;
                    t->gl.targetIndex = (uint8_t)gl.getIndexForTextureTarget(GL_TEXTURE_2D_ARRAY);
                    break;
                case SamplerType::SAMPLER_CUBEMAP:
                    t->gl.target = GL_TEXTURE_CUBE_MAP;
                    t->gl.targetIndex = (uint8_t)gl.getIndexForTextureTarget(GL_TEXTURE_CUBE_MAP);
                    break;
            }

            if (t->samples > 1) {
                // Note: we can't be here in practice because filament's user API doesn't
                // allow the creation of multi-sampled textures.
                if (gl.features.multisample_texture) {
                    // multi-sample texture on GL 3.2 / GLES 3.1 and above
                    t->gl.target = GL_TEXTURE_2D_MULTISAMPLE;
                    t->gl.targetIndex = (uint8_t)
                            gl.getIndexForTextureTarget(GL_TEXTURE_2D_MULTISAMPLE);
                } else {
                    // Turn off multi-sampling for that texture. It's just not supported.
                }
            }
            textureStorage(t, w, h, depth);
        }
    } else {
        assert_invariant(any(usage & (
                TextureUsage::COLOR_ATTACHMENT |
                TextureUsage::DEPTH_ATTACHMENT |
                TextureUsage::STENCIL_ATTACHMENT)));
        assert_invariant(levels == 1);
        assert_invariant(target == SamplerType::SAMPLER_2D);
        t->gl.internalFormat = getInternalFormat(format);
        t->gl.target = GL_RENDERBUFFER;
        glGenRenderbuffers(1, &t->gl.id);
        renderBufferStorage(t->gl.id, t->gl.internalFormat, w, h, samples);
    }

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createTextureSwizzledR(Handle<HwTexture> th,
        SamplerType target, uint8_t levels, TextureFormat format, uint8_t samples,
        uint32_t w, uint32_t h, uint32_t depth, TextureUsage usage,
        TextureSwizzle r, TextureSwizzle g, TextureSwizzle b, TextureSwizzle a) {
    DEBUG_MARKER()

    assert_invariant(uint8_t(usage) & uint8_t(TextureUsage::SAMPLEABLE));

    createTextureR(th, target, levels, format, samples, w, h, depth, usage);

    // WebGL does not support swizzling. We assert for this in the Texture builder,
    // so it is probably fine to silently ignore the swizzle state here.
#if !defined(__EMSCRIPTEN__)

    // the texture is still bound and active from createTextureR
    GLTexture* t = handle_cast<GLTexture *>(th);

    glTexParameteri(t->gl.target, GL_TEXTURE_SWIZZLE_R, getSwizzleChannel(r));
    glTexParameteri(t->gl.target, GL_TEXTURE_SWIZZLE_G, getSwizzleChannel(g));
    glTexParameteri(t->gl.target, GL_TEXTURE_SWIZZLE_B, getSwizzleChannel(b));
    glTexParameteri(t->gl.target, GL_TEXTURE_SWIZZLE_A, getSwizzleChannel(a));

#endif

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::importTextureR(Handle<HwTexture> th, intptr_t id,
        SamplerType target, uint8_t levels, TextureFormat format, uint8_t samples,
        uint32_t w, uint32_t h, uint32_t depth, TextureUsage usage) {
    DEBUG_MARKER()

    auto& gl = mContext;
    samples = std::clamp(samples, uint8_t(1u), uint8_t(gl.gets.max_samples));
    GLTexture* t = construct<GLTexture>(th, target, levels, samples, w, h, depth, format, usage);

    t->gl.id = (GLuint)id;
    t->gl.imported = true;
    t->gl.internalFormat = getInternalFormat(format);
    assert_invariant(t->gl.internalFormat);

    // We DO NOT update targetIndex at function exit to take advantage of the fact that
    // getIndexForTextureTarget() is constexpr -- so all of this disappears at compile time.
    switch (target) {
        case SamplerType::SAMPLER_EXTERNAL:
            t->gl.target = GL_TEXTURE_EXTERNAL_OES;
            t->gl.targetIndex = (uint8_t)gl.getIndexForTextureTarget(GL_TEXTURE_EXTERNAL_OES);
            break;
        case SamplerType::SAMPLER_2D:
            t->gl.target = GL_TEXTURE_2D;
            t->gl.targetIndex = (uint8_t)gl.getIndexForTextureTarget(GL_TEXTURE_2D);
            break;
        case SamplerType::SAMPLER_3D:
            t->gl.target = GL_TEXTURE_3D;
            t->gl.targetIndex = (uint8_t)gl.getIndexForTextureTarget(GL_TEXTURE_3D);
            break;
        case SamplerType::SAMPLER_2D_ARRAY:
            t->gl.target = GL_TEXTURE_2D_ARRAY;
            t->gl.targetIndex = (uint8_t)gl.getIndexForTextureTarget(GL_TEXTURE_2D_ARRAY);
            break;
        case SamplerType::SAMPLER_CUBEMAP:
            t->gl.target = GL_TEXTURE_CUBE_MAP;
            t->gl.targetIndex = (uint8_t)gl.getIndexForTextureTarget(GL_TEXTURE_CUBE_MAP);
            break;
    }

    if (t->samples > 1) {
        // Note: we can't be here in practice because filament's user API doesn't
        // allow the creation of multi-sampled textures.
        if (gl.features.multisample_texture) {
            // multi-sample texture on GL 3.2 / GLES 3.1 and above
            t->gl.target = GL_TEXTURE_2D_MULTISAMPLE;
            t->gl.targetIndex = (uint8_t)gl.getIndexForTextureTarget(GL_TEXTURE_2D_MULTISAMPLE);
        } else {
            // Turn off multi-sampling for that texture. It's just not supported.
        }
    }

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::updateVertexArrayObject(GLRenderPrimitive* rp, GLVertexBuffer const* vb) {

    auto& gl = mContext;

    // NOTE: this is called from draw() and must be as efficient as possible.

    // The VAO for the given render primitive must already be bound.
    #ifndef NDEBUG
    GLint vaoBinding;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vaoBinding);
    assert_invariant(vaoBinding == (GLint) rp->gl.vao);
    #endif

    rp->gl.vertexBufferVersion = vb->bufferObjectsVersion;
    rp->maxVertexCount = vb->vertexCount;
    for (size_t i = 0, n = vb->attributes.size(); i < n; i++) {
        const auto& attribute = vb->attributes[i];
        const uint8_t bi = attribute.buffer;

        // Invoking glVertexAttribPointer without a bound VBO is an invalid operation, so we must
        // take care to avoid it. This can occur when VertexBuffer is only partially populated with
        // BufferObject items.
        if (bi != Attribute::BUFFER_UNUSED && UTILS_LIKELY(vb->gl.buffers[bi] != 0)) {
            gl.bindBuffer(GL_ARRAY_BUFFER, vb->gl.buffers[bi]);
            if (UTILS_UNLIKELY(attribute.flags & Attribute::FLAG_INTEGER_TARGET)) {
                glVertexAttribIPointer(GLuint(i),
                        getComponentCount(attribute.type),
                        getComponentType(attribute.type),
                        attribute.stride,
                        (void*) uintptr_t(attribute.offset));
            } else {
                glVertexAttribPointer(GLuint(i),
                        getComponentCount(attribute.type),
                        getComponentType(attribute.type),
                        getNormalization(attribute.flags & Attribute::FLAG_NORMALIZED),
                        attribute.stride,
                        (void*) uintptr_t(attribute.offset));
            }

            gl.enableVertexAttribArray(GLuint(i));
        } else {

            // In some OpenGL implementations, we must supply a properly-typed placeholder for
            // every integer input that is declared in the vertex shader.
            if (UTILS_UNLIKELY(attribute.flags & Attribute::FLAG_INTEGER_TARGET)) {
                glVertexAttribI4ui(GLuint(i), 0, 0, 0, 0);
            } else {
                glVertexAttrib4f(GLuint(i), 0, 0, 0, 0);
            }

            gl.disableVertexAttribArray(GLuint(i));
        }
    }
}

void OpenGLDriver::framebufferTexture(TargetBufferInfo const& binfo,
        GLRenderTarget const* rt, GLenum attachment) noexcept {

#if !defined(NDEBUG)
    // Only used by assert_invariant() checks below
    UTILS_UNUSED_IN_RELEASE auto valueForLevel = [](size_t level, size_t value) {
        return std::max(size_t(1), value >> level);
    };
#endif

    GLTexture* t = handle_cast<GLTexture*>(binfo.handle);

    assert_invariant(t->target != SamplerType::SAMPLER_EXTERNAL);
    assert_invariant(rt->width  <= valueForLevel(binfo.level, t->width) &&
           rt->height <= valueForLevel(binfo.level, t->height));

    // Declare a small mask of bits that will later be OR'd into the texture's resolve mask.
    TargetBufferFlags resolveFlags = {};

    switch (attachment) {
        case GL_COLOR_ATTACHMENT0:
        case GL_COLOR_ATTACHMENT1:
        case GL_COLOR_ATTACHMENT2:
        case GL_COLOR_ATTACHMENT3:
        case GL_COLOR_ATTACHMENT4:
        case GL_COLOR_ATTACHMENT5:
        case GL_COLOR_ATTACHMENT6:
        case GL_COLOR_ATTACHMENT7:
            static_assert(MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT == 8);
            resolveFlags = getTargetBufferFlagsAt(attachment - GL_COLOR_ATTACHMENT0);
            break;
        case GL_DEPTH_ATTACHMENT:
            resolveFlags = TargetBufferFlags::DEPTH;
            break;
        case GL_STENCIL_ATTACHMENT:
            resolveFlags = TargetBufferFlags::STENCIL;
            break;
        case GL_DEPTH_STENCIL_ATTACHMENT:
            resolveFlags = TargetBufferFlags::DEPTH;
            resolveFlags |= TargetBufferFlags::STENCIL;
            break;
        default:
            break;
    }

    // depth/stencil attachment must match the rendertarget sample count
    // this is because EXT_multisampled_render_to_texture doesn't guarantee depth/stencil
    // is resolved.
    UTILS_UNUSED bool attachmentTypeNotSupportedByMSRTT = false;
    switch (attachment) {
        case GL_DEPTH_ATTACHMENT:
        case GL_STENCIL_ATTACHMENT:
        case GL_DEPTH_STENCIL_ATTACHMENT:
            attachmentTypeNotSupportedByMSRTT = rt->gl.samples != t->samples;
            break;
        default:
            break;
    }

    auto& gl = mContext;

    GLenum target = GL_TEXTURE_2D;
    if (any(t->usage & TextureUsage::SAMPLEABLE)) {
        switch (t->target) {
            case SamplerType::SAMPLER_2D:
            case SamplerType::SAMPLER_2D_ARRAY:
                // this could be GL_TEXTURE_2D_MULTISAMPLE or GL_TEXTURE_2D_ARRAY
                target = t->gl.target;
                // note: multi-sampled textures can't have mipmaps
                break;
            case SamplerType::SAMPLER_CUBEMAP:
                target = getCubemapTarget(binfo.layer);
                // note: cubemaps can't be multi-sampled
                break;
            default:
                break;
        }
    }

    // We can't use FramebufferTexture2DMultisampleEXT with GL_TEXTURE_2D_ARRAY.
    if (!(target == GL_TEXTURE_2D ||
          target == GL_TEXTURE_CUBE_MAP_POSITIVE_X ||
          target == GL_TEXTURE_CUBE_MAP_NEGATIVE_X ||
          target == GL_TEXTURE_CUBE_MAP_POSITIVE_Y ||
          target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y ||
          target == GL_TEXTURE_CUBE_MAP_POSITIVE_Z ||
          target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)) {
        attachmentTypeNotSupportedByMSRTT = true;
    }

    // There's a bug with certain drivers preventing us from emulating
    // EXT_multisampled_render_to_texture when the texture is a TEXTURE_2D_ARRAY, so we'll simply
    // fall back to non-MSAA rendering for now. Also, MSRTT is never available for TEXTURE_2D_ARRAY,
    // and since we are a 2D array, we know we're sampleable and therefore that a resolve will
    // be needed.
    // Note that this affects VSM shadows in particular.
    // TODO: a better workaround would be to do the resolve by hand in that case
    const bool disableMultisampling =
            gl.bugs.disable_sidecar_blit_into_texture_array &&
            rt->gl.samples > 1 && t->samples <= 1 &&
            target == GL_TEXTURE_2D_ARRAY; // implies MSRTT is not available

    if (rt->gl.samples <= 1 ||
        (rt->gl.samples > 1 && t->samples > 1 && gl.features.multisample_texture) ||
        disableMultisampling) {
        // on GL3.2 / GLES3.1 and above multisample is handled when creating the texture.
        // If multisampled textures are not supported and we end-up here, things should
        // still work, albeit without MSAA.
        gl.bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);
        switch (target) {
            case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            case GL_TEXTURE_2D:
            case GL_TEXTURE_2D_MULTISAMPLE:
                if (any(t->usage & TextureUsage::SAMPLEABLE)) {
                    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
                            target, t->gl.id, binfo.level);
                } else {
                    assert_invariant(target == GL_TEXTURE_2D);
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment,
                            GL_RENDERBUFFER, t->gl.id);
                }
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
#ifdef GL_EXT_multisampled_render_to_texture
        // EXT_multisampled_render_to_texture only support GL_COLOR_ATTACHMENT0
    if (!attachmentTypeNotSupportedByMSRTT && (t->depth <= 1)
        && ((gl.ext.EXT_multisampled_render_to_texture && attachment == GL_COLOR_ATTACHMENT0)
            || gl.ext.EXT_multisampled_render_to_texture2)) {
        assert_invariant(rt->gl.samples > 1);
        // We have a multi-sample rendertarget and we have EXT_multisampled_render_to_texture,
        // so, we can directly use a 1-sample texture as attachment, multi-sample resolve,
        // will happen automagically and efficiently in the driver.
        // This extension only exists on OpenGL ES.
        gl.bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);
        if (any(t->usage & TextureUsage::SAMPLEABLE)) {
            glext::glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER,
                    attachment, target, t->gl.id, binfo.level, rt->gl.samples);
        } else {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment,
                    GL_RENDERBUFFER, t->gl.id);
        }
        CHECK_GL_ERROR(utils::slog.e)
    } else
#endif
    if (!any(t->usage & TextureUsage::SAMPLEABLE) && t->samples > 1) {
        assert_invariant(rt->gl.samples > 1);
        assert_invariant(glIsRenderbuffer(t->gl.id));

        // Since this attachment is not sampleable, there is no need for a sidecar or explicit
        // resolve. We can simply render directly into the renderbuffer that was allocated in
        // createTexture.
        gl.bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, t->gl.id);

        // Clear the resolve bit for this particular attachment. Note that other attachment(s)
        // might be sampleable, so this does not necessarily prevent the resolve from occurring.
        resolveFlags = TargetBufferFlags::NONE;

    } else {
        // Here we emulate EXT_multisampled_render_to_texture.
        //
        // This attachment needs to be explicitly resolved in endRenderPass().
        // The first step is to create a sidecar multi-sampled renderbuffer, which is where drawing
        // will actually take place, and use that in lieu of the requested attachment.
        // The sidecar will be destroyed when the render target handle is destroyed.

        assert_invariant(rt->gl.samples > 1);

        gl.bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);

        if (UTILS_UNLIKELY(t->gl.sidecarRenderBufferMS == 0 ||
                rt->gl.samples != t->gl.sidecarSamples))
        {
            if (t->gl.sidecarRenderBufferMS == 0) {
                glGenRenderbuffers(1, &t->gl.sidecarRenderBufferMS);
            }
            renderBufferStorage(t->gl.sidecarRenderBufferMS,
                    t->gl.internalFormat, t->width, t->height, rt->gl.samples);
            t->gl.sidecarSamples = rt->gl.samples;
        }

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER,
                t->gl.sidecarRenderBufferMS);

        // Here we lazily create a "read" sidecar FBO, used later as the resolve target. Note that
        // at least one of the render target's attachments needs to be both MSAA and sampleable in
        // order for fbo_read to be created. If we never bother to create it, then endRenderPass()
        // will skip doing an explicit resolve.
        if (!rt->gl.fbo_read) {
            glGenFramebuffers(1, &rt->gl.fbo_read);
        }
        gl.bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo_read);
        switch (target) {
            case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            case GL_TEXTURE_2D:
                if (any(t->usage & TextureUsage::SAMPLEABLE)) {
                    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
                            target, t->gl.id, binfo.level);
                } else {
                    assert_invariant(target == GL_TEXTURE_2D);
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment,
                            GL_RENDERBUFFER, t->gl.id);
                }
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
    }

    rt->gl.resolve |= resolveFlags;

    if (any(t->usage & TextureUsage::SAMPLEABLE)) {
        // In a sense, drawing to a texture level is similar to calling setTextureData on it; in
        // both cases, we update the base/max LOD to give shaders access to levels as they become
        // available.  Note that this can only expand the LOD range (never shrink it), and that
        // users can override this range by calling setMinMaxLevels().
        updateTextureLodRange(t, binfo.level);
    }

    CHECK_GL_ERROR(utils::slog.e)
    CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e, GL_FRAMEBUFFER)
}

void OpenGLDriver::renderBufferStorage(GLuint rbo, GLenum internalformat, uint32_t width,
        uint32_t height, uint8_t samples) const noexcept {
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    if (samples > 1) {
#ifdef GL_EXT_multisampled_render_to_texture
        auto& gl = mContext;
        if (gl.ext.EXT_multisampled_render_to_texture ||
            gl.ext.EXT_multisampled_render_to_texture2) {
            glext::glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, samples, internalformat, width, height);
        } else
#endif
        {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalformat, width, height);
        }
    } else {
        glRenderbufferStorage(GL_RENDERBUFFER, internalformat, width, height);
    }
    // unbind the renderbuffer, to avoid any later confusion
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createDefaultRenderTargetR(
        Handle<HwRenderTarget> rth, int) {
    DEBUG_MARKER()

    construct<GLRenderTarget>(rth, 0, 0);  // FIXME: we don't know the width/height

    uint32_t framebuffer = 0;
    uint32_t colorbuffer = 0;
    uint32_t depthbuffer = 0;
    mPlatform.createDefaultRenderTarget(framebuffer, colorbuffer, depthbuffer);

    GLRenderTarget* rt = handle_cast<GLRenderTarget*>(rth);
    rt->gl.fbo = framebuffer;
    rt->gl.samples = 1;
    // FIXME: these flags should reflect the actual attachments present
    rt->targets = TargetBufferFlags::COLOR0 | TargetBufferFlags::DEPTH;
}

void OpenGLDriver::createRenderTargetR(Handle<HwRenderTarget> rth,
        TargetBufferFlags targets,
        uint32_t width,
        uint32_t height,
        uint8_t samples,
        MRT color,
        TargetBufferInfo depth,
        TargetBufferInfo stencil) {
    DEBUG_MARKER()

    GLRenderTarget* rt = construct<GLRenderTarget>(rth, width, height);
    glGenFramebuffers(1, &rt->gl.fbo);

    /*
     * The GLES 3.0 spec states:
     *
     *                             --------------
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
     *                             --------------
     *
     * About the size of the attachments:
     *
     *  If the attachment sizes are not all identical, the results of rendering are defined only
     *  within the largest area that can fit in all of the attachments. This area is defined as
     *  the intersection of rectangles having a lower left of (0, 0) and an upper right of
     *  (width, height) for each attachment. Contents of attachments outside this area are
     *  undefined after execution of a rendering command.
     */

    samples = std::clamp(samples, uint8_t(1u), uint8_t(mContext.gets.max_samples));

    rt->gl.samples = samples;
    rt->targets = targets;

    if (any(targets & TargetBufferFlags::COLOR_ALL)) {
        GLenum bufs[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = { GL_NONE };
        const size_t maxDrawBuffers = getMaxDrawBuffers();
        for (size_t i = 0; i < maxDrawBuffers; i++) {
            if (any(targets & getTargetBufferFlagsAt(i))) {
                rt->gl.color[i] = handle_cast<GLTexture*>(color[i].handle);
                framebufferTexture(color[i], rt, GL_COLOR_ATTACHMENT0 + i);
                bufs[i] = GL_COLOR_ATTACHMENT0 + i;
            }
        }
        glDrawBuffers(maxDrawBuffers, bufs);
        CHECK_GL_ERROR(utils::slog.e)
    }

    // handle special cases first (where depth/stencil are packed)
    bool specialCased = false;
    if ((targets & TargetBufferFlags::DEPTH_AND_STENCIL) == TargetBufferFlags::DEPTH_AND_STENCIL) {
        assert_invariant(!stencil.handle || stencil.handle == depth.handle);
        rt->gl.depth = handle_cast<GLTexture*>(depth.handle);
        if (any(rt->gl.depth->usage & TextureUsage::SAMPLEABLE) ||
            (!depth.handle && !stencil.handle)) {
            // special case: depth & stencil requested, and both provided as the same texture
            // special case: depth & stencil requested, but both not provided
            specialCased = true;
            framebufferTexture(depth, rt, GL_DEPTH_STENCIL_ATTACHMENT);
        }
    }

    if (!specialCased) {
        if (any(targets & TargetBufferFlags::DEPTH)) {
            rt->gl.depth = handle_cast<GLTexture*>(depth.handle);
            framebufferTexture(depth, rt, GL_DEPTH_ATTACHMENT);
        }
        if (any(targets & TargetBufferFlags::STENCIL)) {
            rt->gl.stencil = handle_cast<GLTexture*>(stencil.handle);
            framebufferTexture(stencil, rt, GL_STENCIL_ATTACHMENT);
        }
    }

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createFenceR(Handle<HwFence> fh, int) {
    DEBUG_MARKER()

    HwFence* f = handle_cast<HwFence*>(fh);
    f->fence = mPlatform.createFence();
}

void OpenGLDriver::createSyncR(Handle<HwSync> fh, int) {
    DEBUG_MARKER()

    GLSync* f = handle_cast<GLSync *>(fh);
    f->gl.sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    CHECK_GL_ERROR(utils::slog.e)

    // check the status of the sync once a frame, since we must do this from our thread
    std::weak_ptr<GLSync::State> weak = f->result;
    runEveryNowAndThen([sync = f->gl.sync, weak]() -> bool {
        auto result = weak.lock();
        if (result) {
            GLenum status = glClientWaitSync(sync, 0, 0u);
            result->status.store(status, std::memory_order_relaxed);
            return (status != GL_TIMEOUT_EXPIRED);
        }
        return true; // we're done
    });
}

void OpenGLDriver::createSwapChainR(Handle<HwSwapChain> sch, void* nativeWindow, uint64_t flags) {
    DEBUG_MARKER()

    HwSwapChain* sc = handle_cast<HwSwapChain*>(sch);
    sc->swapChain = mPlatform.createSwapChain(nativeWindow, flags);
}

void OpenGLDriver::createSwapChainHeadlessR(Handle<HwSwapChain> sch,
        uint32_t width, uint32_t height, uint64_t flags) {
    DEBUG_MARKER()

    HwSwapChain* sc = handle_cast<HwSwapChain*>(sch);
    sc->swapChain = mPlatform.createSwapChain(width, height, flags);
}

void OpenGLDriver::createStreamFromTextureIdR(Handle<HwStream> sh,
        intptr_t externalTextureId, uint32_t width, uint32_t height) {
    DEBUG_MARKER()

    GLStream* s = handle_cast<GLStream*>(sh);
    // It would be better if we could query the externalTextureId size, unfortunately
    // this is not supported in GL for GL_TEXTURE_EXTERNAL_OES targets
    s->width = width;
    s->height = height;
    s->gl.externalTextureId = static_cast<GLuint>(externalTextureId);
    s->streamType = StreamType::TEXTURE_ID;
    glGenTextures(GLStream::ROUND_ROBIN_TEXTURE_COUNT, s->user_thread.read);
    glGenTextures(GLStream::ROUND_ROBIN_TEXTURE_COUNT, s->user_thread.write);
    for (auto& info : s->user_thread.infos) {
        info.ets = mPlatform.createExternalTextureStorage();
    }
}

void OpenGLDriver::createTimerQueryR(Handle<HwTimerQuery> tqh, int) {
    DEBUG_MARKER()

    GLTimerQuery* tq = handle_cast<GLTimerQuery*>(tqh);
    glGenQueries(1u, &tq->gl.query);
    CHECK_GL_ERROR(utils::slog.e)
}

// ------------------------------------------------------------------------------------------------
// Destroying driver objects
// ------------------------------------------------------------------------------------------------

void OpenGLDriver::destroyVertexBuffer(Handle<HwVertexBuffer> vbh) {
    DEBUG_MARKER()
    if (vbh) {
        GLVertexBuffer const* vb = handle_cast<const GLVertexBuffer*>(vbh);
        destruct(vbh, vb);
    }
}

void OpenGLDriver::destroyIndexBuffer(Handle<HwIndexBuffer> ibh) {
    DEBUG_MARKER()

    if (ibh) {
        auto& gl = mContext;
        GLIndexBuffer const* ib = handle_cast<const GLIndexBuffer*>(ibh);
        gl.deleteBuffers(1, &ib->gl.buffer, GL_ELEMENT_ARRAY_BUFFER);
        destruct(ibh, ib);
    }
}

void OpenGLDriver::destroyBufferObject(Handle<HwBufferObject> boh) {
    DEBUG_MARKER()
    if (boh) {
        auto& gl = mContext;
        GLBufferObject const* bo = handle_cast<const GLBufferObject*>(boh);
        gl.deleteBuffers(1, &bo->gl.id, bo->gl.binding);
        destruct(boh, bo);
    }
}

void OpenGLDriver::destroyRenderPrimitive(Handle<HwRenderPrimitive> rph) {
    DEBUG_MARKER()

    if (rph) {
        auto& gl = mContext;
        GLRenderPrimitive const* rp = handle_cast<const GLRenderPrimitive*>(rph);
        gl.deleteVextexArrays(1, &rp->gl.vao);
        destruct(rph, rp);
    }
}

void OpenGLDriver::destroyProgram(Handle<HwProgram> ph) {
    DEBUG_MARKER()
    if (ph) {
        OpenGLProgram* p = handle_cast<OpenGLProgram*>(ph);
        cancelRunAtNextPassOp(p);
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

void OpenGLDriver::destroyTexture(Handle<HwTexture> th) {
    DEBUG_MARKER()

    if (th) {
        GLTexture* t = handle_cast<GLTexture*>(th);
        if (UTILS_LIKELY(!t->gl.imported)) {
            auto& gl = mContext;
            if (UTILS_LIKELY(t->usage & TextureUsage::SAMPLEABLE)) {
                gl.unbindTexture(t->gl.target, t->gl.id);
                if (UTILS_UNLIKELY(t->hwStream)) {
                    detachStream(t);
                }
                if (UTILS_UNLIKELY(t->target == SamplerType::SAMPLER_EXTERNAL)) {
                    mPlatform.destroyExternalImage(t);
                } else {
                    glDeleteTextures(1, &t->gl.id);
                }
            } else {
                assert_invariant(t->gl.target == GL_RENDERBUFFER);
                glDeleteRenderbuffers(1, &t->gl.id);
            }
            if (t->gl.fence) {
                glDeleteSync(t->gl.fence);
            }
            if (t->gl.sidecarRenderBufferMS) {
                glDeleteRenderbuffers(1, &t->gl.sidecarRenderBufferMS);
            }
        }
        destruct(th, t);
    }
}

void OpenGLDriver::destroyRenderTarget(Handle<HwRenderTarget> rth) {
    DEBUG_MARKER()

    if (rth) {
        auto& gl = mContext;
        GLRenderTarget* rt = handle_cast<GLRenderTarget*>(rth);
        if (rt->gl.fbo) {
            // first unbind this framebuffer if needed
            gl.bindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, &rt->gl.fbo);
        }
        if (rt->gl.fbo_read) {
            // first unbind this framebuffer if needed
            gl.bindFramebuffer(GL_FRAMEBUFFER, 0);
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
        if (s->streamType == StreamType::NATIVE) {
            mPlatform.destroyStream(s->stream);
        } else if (s->streamType == StreamType::TEXTURE_ID) {
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

void OpenGLDriver::destroyTimerQuery(Handle<HwTimerQuery> tqh) {
    DEBUG_MARKER()

    if (tqh) {
        GLTimerQuery* tq = handle_cast<GLTimerQuery*>(tqh);
        glDeleteQueries(1u, &tq->gl.query);
        destruct(tqh, tq);
    }
}

void OpenGLDriver::destroySync(Handle<HwSync> sh) {
    DEBUG_MARKER()

    if (sh) {
        GLSync* s = handle_cast<GLSync*>(sh);
        glDeleteSync(s->gl.sync);
        destruct(sh, s);
    }
}

// ------------------------------------------------------------------------------------------------
// Synchronous APIs
// These are called on the application's thread
// ------------------------------------------------------------------------------------------------

Handle<HwStream> OpenGLDriver::createStreamNative(void* nativeStream) {
    Platform::Stream* stream = mPlatform.createStream(nativeStream);
    return initHandle<GLStream>(stream);
}

Handle<HwStream> OpenGLDriver::createStreamAcquired() {
    return initHandle<GLStream>();
}

// Stashes an acquired external image and a release callback. The image is not bound to OpenGL until
// the subsequent call to beginFrame (see updateStreamAcquired).
//
// setAcquiredImage should be called by the user outside of beginFrame / endFrame, and should be
// called only once per frame. If the user pushes images to the same stream multiple times in a
// single frame, we emit a warning and honor only the final image, but still invoke all callbacks.
void OpenGLDriver::setAcquiredImage(Handle<HwStream> sh, void* hwbuffer,
        CallbackHandler* handler, StreamCallback cb, void* userData) {
    GLStream* glstream = handle_cast<GLStream*>(sh);
    if (glstream->user_thread.pending.image) {
        scheduleRelease(glstream->user_thread.pending);
        slog.w << "Acquired image is set more than once per frame." << io::endl;
    }
    glstream->user_thread.pending = mPlatform.transformAcquiredImage({
            hwbuffer, cb, userData, handler });
}

void OpenGLDriver::updateStreams(DriverApi* driver) {
    if (UTILS_UNLIKELY(!mExternalStreams.empty())) {
        OpenGLBlitter::State state;
        for (GLTexture* t : mExternalStreams) {
            assert_invariant(t);

            GLStream* s = static_cast<GLStream*>(t->hwStream);
            if (UTILS_UNLIKELY(s == nullptr)) {
                // this can happen because we're called synchronously and the setExternalStream()
                // call may not have been processed yet.
                continue;
            }

            if (s->streamType == StreamType::TEXTURE_ID) {
                state.setup();
                updateStreamTexId(t, driver);
            }

            if (s->streamType == StreamType::ACQUIRED) {
                updateStreamAcquired(t, driver);
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
        if (f->fence == nullptr) {
            // we can end-up here if:
            // - the platform doesn't support h/w fences
            // - wait() was called before the fence was asynchronously created.
            //   This case is not handled in OpenGLDriver but is handled by FFence.
            //   TODO: move FFence logic into the backend.
            return FenceStatus::ERROR;
        }
        return mPlatform.waitFence(f->fence, timeout);
    }
    return FenceStatus::ERROR;
}

bool OpenGLDriver::isTextureFormatSupported(TextureFormat format) {
    const auto& ext = mContext.ext;
    if (isETC2Compression(format)) {
        return ext.EXT_texture_compression_etc2 ||
               ext.WEBGL_compressed_texture_etc; // WEBGL specific, apparently contains ETC2
    }
    if (isS3TCSRGBCompression(format)) {
        // see https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_texture_sRGB.txt
        return  ext.WEBGL_compressed_texture_s3tc_srgb || // this is WEBGL specific
                ext.EXT_texture_compression_s3tc_srgb || // this is ES specific
               (ext.EXT_texture_compression_s3tc && ext.EXT_texture_sRGB);
    }
    if (isS3TCCompression(format)) {
        return  ext.EXT_texture_compression_s3tc || // this is ES specific
                ext.WEBGL_compressed_texture_s3tc; // this is WEBGL specific
    }
    return getInternalFormat(format) != 0;
}

bool OpenGLDriver::isTextureSwizzleSupported() {
    return true;
}

bool OpenGLDriver::isTextureFormatMipmappable(TextureFormat format) {
    // The OpenGL spec for GenerateMipmap stipulates that it returns INVALID_OPERATION unless
    // the sized internal format is both color-renderable and texture-filterable.
    switch (format) {
        case TextureFormat::DEPTH16:
        case TextureFormat::DEPTH24:
        case TextureFormat::DEPTH32F:
        case TextureFormat::DEPTH24_STENCIL8:
        case TextureFormat::DEPTH32F_STENCIL8:
            return false;
        default:
            return isRenderTargetFormatSupported(format);
    }
}

bool OpenGLDriver::isRenderTargetFormatSupported(TextureFormat format) {
    // Supported formats per http://docs.gl/es3/glRenderbufferStorage, note that desktop OpenGL may
    // support more formats, but it requires querying GL_INTERNALFORMAT_SUPPORTED which is not
    // available in OpenGL ES.
    auto& gl = mContext;
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
            return BACKEND_OPENGL_VERSION == BACKEND_OPENGL_VERSION_GL;

        // Half-float formats, requires extension.
        case TextureFormat::R16F:
        case TextureFormat::RG16F:
        case TextureFormat::RGBA16F:
            return gl.ext.EXT_color_buffer_float || gl.ext.EXT_color_buffer_half_float;

        // RGB16F is only supported with EXT_color_buffer_half_float, however
        // some WebGL implementations do not consider this extension to be sufficient:
        // https://bugs.chromium.org/p/chromium/issues/detail?id=941671#c10
        case TextureFormat::RGB16F:
        #if defined(__EMSCRIPTEN__)
            return false;
        #else
            return gl.ext.EXT_color_buffer_half_float;
        #endif

        // Float formats from GL_EXT_color_buffer_float
        case TextureFormat::R32F:
        case TextureFormat::RG32F:
        case TextureFormat::RGBA32F:
            return gl.ext.EXT_color_buffer_float;

        // RGB_11_11_10 is only supported with some  specific extensions
        case TextureFormat::R11F_G11F_B10F:
            return gl.ext.EXT_color_buffer_float || gl.ext.APPLE_color_buffer_packed_float;

        default:
            return false;
    }
}

bool OpenGLDriver::isFrameBufferFetchSupported() {
    auto& gl = mContext;
    return gl.ext.EXT_shader_framebuffer_fetch;
}

bool OpenGLDriver::isFrameBufferFetchMultiSampleSupported() {
    return isFrameBufferFetchSupported();
}

bool OpenGLDriver::isFrameTimeSupported() {
    return mFrameTimeSupported;
}

bool OpenGLDriver::isAutoDepthResolveSupported() {
    // TODO: this should return true only for GLES3.1+ and EXT_multisampled_render_to_texture2
    return true;
}

bool OpenGLDriver::isWorkaroundNeeded(Workaround workaround) {
    switch (workaround) {
        case Workaround::SPLIT_EASU:
            return mContext.bugs.split_easu;
        case Workaround::ALLOW_READ_ONLY_ANCILLARY_FEEDBACK_LOOP:
            return mContext.bugs.allow_read_only_ancillary_feedback_loop;
    }
    return false;
}

math::float2 OpenGLDriver::getClipSpaceParams() {
    return mContext.ext.EXT_clip_control ?
           // z-coordinate of virtual and physical clip-space is in [-w, 0]
           math::float2{ 1.0f, 0.0f } :
           // z-coordinate of virtual clip-space is in [-w,0], physical is in [-w, w]
           math::float2{ 2.0f, -1.0f };
}

uint8_t OpenGLDriver::getMaxDrawBuffers() {
    return std::min(MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT, uint8_t(mContext.gets.max_draw_buffers));
}

// ------------------------------------------------------------------------------------------------
// Swap chains
// ------------------------------------------------------------------------------------------------


void OpenGLDriver::commit(Handle<HwSwapChain> sch) {
    DEBUG_MARKER()

    HwSwapChain* sc = handle_cast<HwSwapChain*>(sch);
    mPlatform.commit(sc->swapChain);
}

void OpenGLDriver::makeCurrent(Handle<HwSwapChain> schDraw, Handle<HwSwapChain> schRead) {
    DEBUG_MARKER()

    HwSwapChain* scDraw = handle_cast<HwSwapChain*>(schDraw);
    HwSwapChain* scRead = handle_cast<HwSwapChain*>(schRead);
    mPlatform.makeCurrent(scDraw->swapChain, scRead->swapChain);
}

// ------------------------------------------------------------------------------------------------
// Updating driver objects
// ------------------------------------------------------------------------------------------------

void OpenGLDriver::setVertexBufferObject(Handle<HwVertexBuffer> vbh,
        uint32_t index, Handle<HwBufferObject> boh) {
   DEBUG_MARKER()

    GLVertexBuffer* vb = handle_cast<GLVertexBuffer *>(vbh);
    GLBufferObject* bo = handle_cast<GLBufferObject *>(boh);

    assert_invariant(bo->gl.binding == GL_ARRAY_BUFFER);

    // If the specified VBO handle is different from what's already in the slot, then update the
    // slot and bump the cyclical version number. Dependent VAOs use the version number to detect
    // when they should be updated.
    if (vb->gl.buffers[index] != bo->gl.id) {
        vb->gl.buffers[index] = bo->gl.id;
        static constexpr uint32_t kMaxVersion =
                std::numeric_limits<decltype(vb->bufferObjectsVersion)>::max();
        const uint32_t version = vb->bufferObjectsVersion;
        vb->bufferObjectsVersion = (version + 1) % kMaxVersion;
    }

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::updateIndexBuffer(
        Handle<HwIndexBuffer> ibh, BufferDescriptor&& p, uint32_t byteOffset) {
    DEBUG_MARKER()

    auto& gl = mContext;
    GLIndexBuffer* ib = handle_cast<GLIndexBuffer *>(ibh);
    assert_invariant(ib->elementSize == 2 || ib->elementSize == 4);

    gl.bindVertexArray(nullptr);
    gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->gl.buffer);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, byteOffset, p.size, p.buffer);

    scheduleDestroy(std::move(p));

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::updateBufferObject(
        Handle<HwBufferObject> boh, BufferDescriptor&& bd, uint32_t byteOffset) {
    DEBUG_MARKER()

    auto& gl = mContext;
    GLBufferObject* bo = handle_cast<GLBufferObject *>(boh);

    assert_invariant(bd.size + byteOffset <= bo->byteCount);

    if (bo->gl.binding == GL_UNIFORM_BUFFER) {
        // TODO: use updateBuffer() for all types of buffer? Make sure GL supports that.
        updateBuffer(bo, bd, byteOffset, (uint32_t)gl.gets.uniform_buffer_offset_alignment);
    } else {
        if (bo->gl.binding == GL_ARRAY_BUFFER) {
            gl.bindVertexArray(nullptr);
        }
        gl.bindBuffer(bo->gl.binding, bo->gl.id);
        if (byteOffset == 0 && bd.size == bo->byteCount) {
            // it looks like it's generally faster (or not worse) to use glBufferData()
            glBufferData(bo->gl.binding, bd.size, bd.buffer, getBufferUsage(bo->usage));
        } else {
            // glBufferSubData() could be catastrophically inefficient if several are
            // issued during the same frame. Currently, we're not doing that though.
            glBufferSubData(bo->gl.binding, byteOffset, bd.size, bd.buffer);
        }
    }

    scheduleDestroy(std::move(bd));

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::updateBuffer(GLBufferObject* buffer, BufferDescriptor const& p,
        uint32_t byteOffset, uint32_t alignment) noexcept {
    assert_invariant(buffer->byteCount >= p.size);
    assert_invariant(buffer->gl.id);

    auto& gl = mContext;
    gl.bindBuffer(buffer->gl.binding, buffer->gl.id);
    if (buffer->usage == BufferUsage::STREAM) {

        // in streaming mode it's meaningless to have an offset specified
        assert_invariant(byteOffset == 0);

        buffer->size = (uint32_t)p.size;

        // If MapBufferRange is supported, then attempt to use that instead of BufferSubData, which
        // can be quite inefficient on some platforms. Note that WebGL does not support
        // MapBufferRange, but we still allow STREAM semantics for the web platform.
        if (HAS_MAPBUFFERS) {
            uint32_t offset = buffer->base + buffer->size;
            offset = (offset + (alignment - 1u)) & ~(alignment - 1u);

            if (offset + p.size > buffer->byteCount) {
                // if we've reached the end of the buffer, we orphan it and allocate a new one.
                // this is assuming the driver actually does that as opposed to stalling. This is
                // the case for Mali and Adreno -- we could use fences instead.
                offset = 0;
                glBufferData(buffer->gl.binding, buffer->byteCount, nullptr, getBufferUsage(buffer->usage));
            }
    retry:
            void* vaddr = glMapBufferRange(buffer->gl.binding, offset, p.size,
                    GL_MAP_WRITE_BIT |
                    GL_MAP_INVALIDATE_RANGE_BIT |
                    GL_MAP_UNSYNCHRONIZED_BIT);
            if (vaddr) {
                memcpy(vaddr, p.buffer, p.size);
                if (glUnmapBuffer(buffer->gl.binding) == GL_FALSE) {
                    // According to the spec, UnmapBuffer can return FALSE in rare conditions (e.g.
                    // during a screen mode change). Note that is not a GL error, and we can handle
                    // it by simply making a second attempt.
                    goto retry; // NOLINT(cppcoreguidelines-avoid-goto,hicpp-avoid-goto)
                }
            } else {
                // handle mapping error, revert to glBufferSubData()
                glBufferSubData(buffer->gl.binding, offset, p.size, p.buffer);
            }
            buffer->base = offset;

            CHECK_GL_ERROR(utils::slog.e)
        } else {
            // In stream mode we're allowed to allocate a whole new buffer
            glBufferData(buffer->gl.binding, p.size, p.buffer, getBufferUsage(buffer->usage));
        }
    } else {
        if (byteOffset == 0 && p.size == buffer->byteCount) {
            // it looks like it's generally faster (or not worse) to use glBufferData()
            glBufferData(buffer->gl.binding, buffer->byteCount, p.buffer,
                    getBufferUsage(buffer->usage));
        } else {
            // glBufferSubData() could be catastrophically inefficient if several are
            // issued during the same frame. Currently, we're not doing that though.
            glBufferSubData(buffer->gl.binding, byteOffset, p.size, p.buffer);
        }
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

void OpenGLDriver::setMinMaxLevels(Handle<HwTexture> th, uint32_t minLevel, uint32_t maxLevel) {
    DEBUG_MARKER()
    auto& gl = mContext;

    GLTexture* t = handle_cast<GLTexture *>(th);
    bindTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1, t);
    gl.activeTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1);

    // Must fit within int8_t.
    assert_invariant(minLevel <= 0x7f && maxLevel <= 0x7f);

    t->gl.baseLevel = minLevel;
    glTexParameteri(t->gl.target, GL_TEXTURE_BASE_LEVEL, t->gl.baseLevel);

    t->gl.maxLevel = maxLevel; // NOTE: according to the GL spec, the default value of this 1000
    glTexParameteri(t->gl.target, GL_TEXTURE_MAX_LEVEL, t->gl.maxLevel);
}

void OpenGLDriver::update3DImage(Handle<HwTexture> th,
        uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
        uint32_t width, uint32_t height, uint32_t depth,
        PixelBufferDescriptor&& data) {
    DEBUG_MARKER()

    GLTexture* t = handle_cast<GLTexture *>(th);
    if (data.type == PixelDataType::COMPRESSED) {
        setCompressedTextureData(t,
                level, xoffset, yoffset, zoffset, width, height, depth, std::move(data), nullptr);
    } else {
        setTextureData(t,
                level, xoffset, yoffset, zoffset, width, height, depth, std::move(data), nullptr);
    }
}

void OpenGLDriver::updateCubeImage(Handle<HwTexture> th, uint32_t level,
        PixelBufferDescriptor&& data, FaceOffsets faceOffsets) {
    DEBUG_MARKER()

    GLTexture* t = handle_cast<GLTexture *>(th);
    auto width = std::max(1u, t->width >> level);
    auto height = std::max(1u, t->height >> level);
    if (data.type == PixelDataType::COMPRESSED) {
        setCompressedTextureData(t, level, 0, 0, 0, width, height, 0, std::move(data), &faceOffsets);
    } else {
        setTextureData(t, level, 0, 0, 0, width, height, 0, std::move(data), &faceOffsets);
    }
}

void OpenGLDriver::generateMipmaps(Handle<HwTexture> th) {
    DEBUG_MARKER()

    auto& gl = mContext;
    GLTexture* t = handle_cast<GLTexture *>(th);
    assert_invariant(t->gl.target != GL_TEXTURE_2D_MULTISAMPLE);
    // Note: glGenerateMimap can also fail if the internal format is not both
    // color-renderable and filterable (i.e.: doesn't work for depth)
    bindTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1, t);
    gl.activeTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1);

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
    auto& gl = mContext;

    assert_invariant(xoffset + width <= std::max(1u, t->width >> level));
    assert_invariant(yoffset + height <= std::max(1u, t->height >> level));
    assert_invariant(t->samples <= 1);

    if (UTILS_UNLIKELY(t->gl.target == GL_TEXTURE_EXTERNAL_OES)) {
        // this is in fact an external texture, this becomes a no-op.
        return;
    }

    GLenum glFormat = getFormat(p.format);
    GLenum glType = getType(p.type);

    gl.pixelStore(GL_UNPACK_ROW_LENGTH, p.stride);
    gl.pixelStore(GL_UNPACK_ALIGNMENT, p.alignment);
    gl.pixelStore(GL_UNPACK_SKIP_PIXELS, p.left);
    gl.pixelStore(GL_UNPACK_SKIP_ROWS, p.top);

    switch (t->target) {
        case SamplerType::SAMPLER_EXTERNAL:
            // if we get there, it's because the user is trying to use an external texture
            // but it's not supported, so instead, we behave like a texture2d.
            // fallthrough...
        case SamplerType::SAMPLER_2D:
            // NOTE: GL_TEXTURE_2D_MULTISAMPLE is not allowed
            bindTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1, t);
            gl.activeTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1);
            assert_invariant(t->gl.target == GL_TEXTURE_2D);
            glTexSubImage2D(t->gl.target, GLint(level),
                    GLint(xoffset), GLint(yoffset),
                    width, height, glFormat, glType, p.buffer);
            break;
        case SamplerType::SAMPLER_3D:
            assert_invariant(zoffset + depth <= std::max(1u, t->depth >> level));
            bindTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1, t);
            gl.activeTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1);
            assert_invariant(t->gl.target == GL_TEXTURE_3D);
            glTexSubImage3D(t->gl.target, GLint(level),
                    GLint(xoffset), GLint(yoffset), GLint(zoffset),
                    width, height, depth, glFormat, glType, p.buffer);
            break;
        case SamplerType::SAMPLER_2D_ARRAY:
            assert_invariant(zoffset + depth <= t->depth);
            // NOTE: GL_TEXTURE_2D_MULTISAMPLE is not allowed
            bindTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1, t);
            gl.activeTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1);
            assert_invariant(t->gl.target == GL_TEXTURE_2D_ARRAY);
            glTexSubImage3D(t->gl.target, GLint(level),
                    GLint(xoffset), GLint(yoffset), GLint(zoffset),
                    width, height, depth, glFormat, glType, p.buffer);
            break;
        case SamplerType::SAMPLER_CUBEMAP: {
            assert_invariant(t->gl.target == GL_TEXTURE_CUBE_MAP);
            bindTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1, t);
            gl.activeTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1);
            FaceOffsets const& offsets = *faceOffsets;
            UTILS_NOUNROLL
            for (size_t face = 0; face < 6; face++) {
                GLenum target = getCubemapTarget(face);
                glTexSubImage2D(target, GLint(level), 0, 0,
                        width, height, glFormat, glType,
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

void OpenGLDriver::setCompressedTextureData(GLTexture* t,  uint32_t level,
        uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
        uint32_t width, uint32_t height, uint32_t depth,
        PixelBufferDescriptor&& p, FaceOffsets const* faceOffsets) {
    DEBUG_MARKER()
    auto& gl = mContext;

    assert_invariant(xoffset + width <= std::max(1u, t->width >> level));
    assert_invariant(yoffset + height <= std::max(1u, t->height >> level));
    assert_invariant(zoffset + depth <= t->depth);
    assert_invariant(t->samples <= 1);

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
            bindTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1, t);
            gl.activeTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1);
            assert_invariant(t->gl.target == GL_TEXTURE_2D);
            glCompressedTexSubImage2D(t->gl.target, GLint(level),
                    GLint(xoffset), GLint(yoffset),
                    width, height, t->gl.internalFormat, imageSize, p.buffer);
            break;
        case SamplerType::SAMPLER_3D:
            bindTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1, t);
            gl.activeTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1);
            assert_invariant(t->gl.target == GL_TEXTURE_3D);
            glCompressedTexSubImage3D(t->gl.target, GLint(level),
                    GLint(xoffset), GLint(yoffset), GLint(zoffset),
                    width, height, depth, t->gl.internalFormat, imageSize, p.buffer);
            break;
        case SamplerType::SAMPLER_2D_ARRAY:
            assert_invariant(t->gl.target == GL_TEXTURE_2D_ARRAY);
            glCompressedTexSubImage3D(t->gl.target, GLint(level),
                    GLint(xoffset), GLint(yoffset), GLint(zoffset),
                    width, height, depth, t->gl.internalFormat, imageSize, p.buffer);
            break;
        case SamplerType::SAMPLER_CUBEMAP: {
            assert_invariant(faceOffsets);
            assert_invariant(t->gl.target == GL_TEXTURE_CUBE_MAP);
            bindTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1, t);
            gl.activeTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1);
            FaceOffsets const& offsets = *faceOffsets;
            UTILS_NOUNROLL
            for (size_t face = 0; face < 6; face++) {
                GLenum target = getCubemapTarget(face);
                glCompressedTexSubImage2D(target, GLint(level), 0, 0,
                        width, height, t->gl.internalFormat,
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
    mPlatform.retainExternalImage(image);
}

void OpenGLDriver::cancelExternalImage(void* image) {
    mPlatform.releaseExternalImage(image);
}

void OpenGLDriver::setExternalImage(Handle<HwTexture> th, void* image) {
    mPlatform.setExternalImage(image, handle_cast<GLTexture*>(th));
    setExternalTexture(handle_cast<GLTexture*>(th), image);
}

void OpenGLDriver::setExternalImagePlane(Handle<HwTexture> th, void* image, uint32_t plane) {
}

void OpenGLDriver::setExternalTexture(GLTexture* t, void* image) {
    auto& gl = mContext;

    // TODO: move this logic to PlatformEGL.
    if (gl.ext.OES_EGL_image_external_essl3) {
        DEBUG_MARKER()

        assert_invariant(t->target == SamplerType::SAMPLER_EXTERNAL);
        assert_invariant(t->gl.target == GL_TEXTURE_EXTERNAL_OES);

        bindTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1, t);
        gl.activeTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1);

#ifdef GL_OES_EGL_image
        glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, static_cast<GLeglImageOES>(image));
#endif
    }
}

void OpenGLDriver::setExternalStream(Handle<HwTexture> th, Handle<HwStream> sh) {
    auto& gl = mContext;
    if (gl.ext.OES_EGL_image_external_essl3) {
        DEBUG_MARKER()

        GLTexture* t = handle_cast<GLTexture*>(th);
        if (UTILS_LIKELY(sh)) {
            GLStream* s = handle_cast<GLStream*>(sh);
            if (UTILS_LIKELY(!t->hwStream)) {
                // we're not attached already
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
    auto& gl = mContext;
    mExternalStreams.push_back(t);

    switch (hwStream->streamType) {
        case StreamType::NATIVE:
            mPlatform.attach(hwStream->stream, t->gl.id);
            break;
        case StreamType::TEXTURE_ID:
            assert_invariant(t->target == SamplerType::SAMPLER_EXTERNAL);
            // The texture doesn't need a texture name anymore, get rid of it
            gl.unbindTexture(t->gl.target, t->gl.id);
            glDeleteTextures(1, &t->gl.id);
            t->gl.id = hwStream->user_thread.read[hwStream->user_thread.cur];
            break;
        case StreamType::ACQUIRED:
            break;
    }
    t->hwStream = hwStream;
}

UTILS_NOINLINE
void OpenGLDriver::detachStream(GLTexture* t) noexcept {
    auto& gl = mContext;
    auto& streams = mExternalStreams;
    auto pos = std::find(streams.begin(), streams.end(), t);
    if (pos != streams.end()) {
        streams.erase(pos);
    }

    GLStream* s = static_cast<GLStream*>(t->hwStream);
    switch (s->streamType) {
        case StreamType::NATIVE:
            mPlatform.detach(t->hwStream->stream);
            // ^ this deletes the texture id
            break;
        case StreamType::TEXTURE_ID:
            break;
        case StreamType::ACQUIRED:
            gl.unbindTexture(t->gl.target, t->gl.id);
            glDeleteTextures(1, &t->gl.id);
            break;
    }

    glGenTextures(1, &t->gl.id);

    t->hwStream = nullptr;
}

UTILS_NOINLINE
void OpenGLDriver::replaceStream(GLTexture* texture, GLStream* newStream) noexcept {
    assert_invariant(newStream && "Do not use replaceStream to detach a stream.");

    // This could be implemented via detachStream + attachStream but inlining allows
    // a few small optimizations, like not touching the mExternalStreams list.

    GLStream* oldStream = static_cast<GLStream*>(texture->hwStream);
    switch (oldStream->streamType) {
        case StreamType::NATIVE:
            mPlatform.detach(texture->hwStream->stream);
            // ^ this deletes the texture id
            break;
        case StreamType::TEXTURE_ID:
        case StreamType::ACQUIRED:
            break;
    }

    switch (newStream->streamType) {
        case StreamType::NATIVE:
            glGenTextures(1, &texture->gl.id);
            mPlatform.attach(newStream->stream, texture->gl.id);
            break;
        case StreamType::TEXTURE_ID:
            assert_invariant(texture->target == SamplerType::SAMPLER_EXTERNAL);
            texture->gl.id = newStream->user_thread.read[newStream->user_thread.cur];
            break;
        case StreamType::ACQUIRED:
            // Just re-use the old texture id.
            break;
    }

    texture->hwStream = newStream;
}

void OpenGLDriver::beginTimerQuery(Handle<HwTimerQuery> tqh) {
    GLTimerQuery* tq = handle_cast<GLTimerQuery*>(tqh);
    // reset the state of the result availability
    tq->elapsed.store(0, std::memory_order_relaxed);
    mTimerQueryImpl->beginTimeElapsedQuery(tq);
}

void OpenGLDriver::endTimerQuery(Handle<HwTimerQuery> tqh) {
    GLTimerQuery* tq = handle_cast<GLTimerQuery*>(tqh);
    mTimerQueryImpl->endTimeElapsedQuery(tq);

    runEveryNowAndThen([this, tq]() -> bool {
        if (!mTimerQueryImpl->queryResultAvailable(tq)) {
            // we need to try this one again later
            return false;
        }
        tq->elapsed.store(mTimerQueryImpl->queryResult(tq), std::memory_order_relaxed);
        return true;
    });
}

bool OpenGLDriver::getTimerQueryValue(Handle<HwTimerQuery> tqh, uint64_t* elapsedTime) {
    GLTimerQuery* tq = handle_cast<GLTimerQuery*>(tqh);
    uint64_t d = tq->elapsed.load(std::memory_order_relaxed);
    if (!d) {
        return false;
    }
    if (elapsedTime) {
        *elapsedTime = d;
    }
    return true;
}

SyncStatus OpenGLDriver::getSyncStatus(Handle<HwSync> sh) {
    GLSync* s = handle_cast<GLSync*>(sh);
    if (!s->result) {
        return SyncStatus::NOT_SIGNALED;
    }
    auto status = s->result->status.load(std::memory_order_relaxed);
    switch (status) {
        case GL_CONDITION_SATISFIED:
        case GL_ALREADY_SIGNALED:
            return SyncStatus::SIGNALED;
        case GL_TIMEOUT_EXPIRED:
            return SyncStatus::NOT_SIGNALED;
        case GL_WAIT_FAILED:
        default:
            return SyncStatus::ERROR;
    }
}

void OpenGLDriver::beginRenderPass(Handle<HwRenderTarget> rth,
        const RenderPassParams& params) {
    DEBUG_MARKER()

    executeRenderPassOps();

    auto& gl = mContext;

    mRenderPassTarget = rth;
    mRenderPassParams = params;

    GLRenderTarget* rt = handle_cast<GLRenderTarget*>(rth);

    const TargetBufferFlags clearFlags = params.flags.clear & rt->targets;
    TargetBufferFlags discardFlags = params.flags.discardStart & rt->targets;

    gl.bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);
    CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e, GL_FRAMEBUFFER)

    // glInvalidateFramebuffer appeared on GLES 3.0 and GL4.3, for simplicity we just
    // ignore it on GL (rather than having to do a runtime check).
    if (BACKEND_OPENGL_VERSION == BACKEND_OPENGL_VERSION_GLES &&
            BACKEND_OPENGL_LEVEL >= BACKEND_OPENGL_LEVEL_GLES30) {
        if (!gl.bugs.disable_invalidate_framebuffer) {
            AttachmentArray attachments; // NOLINT
            GLsizei attachmentCount = getAttachments(attachments, rt, discardFlags);
            if (attachmentCount) {
                glInvalidateFramebuffer(GL_FRAMEBUFFER, attachmentCount, attachments.data());
            }
            CHECK_GL_ERROR(utils::slog.e)
        }
    } else {
        // on GL desktop we assume we don't have glInvalidateFramebuffer, but even if the GPU is
        // not a tiler, it's important to clear the framebuffer before drawing, as it resets
        // the fb to a known state (resets fb compression and possibly other things).
        // So we use glClear instead of glInvalidateFramebuffer
        gl.disable(GL_SCISSOR_TEST);
        clearWithRasterPipe(discardFlags & ~clearFlags, { 0.0f }, 0.0f, 0);
    }

    if (rt->gl.fbo_read) {
        // we have a multi-sample RenderTarget with non multi-sample attachments (i.e. this is the
        // EXT_multisampled_render_to_texture emulation).
        // We would need to perform a "backward" resolve, i.e. load the resolved texture into the
        // tile, everything must appear as though the multi-sample buffer was lost.
        // However Filament specifies that a non multi-sample attachment to a
        // multi-sample RenderTarget is always discarded. We do this because implementing
        // the load on Metal is not trivial and it's not a feature we rely on at this time.
        discardFlags |= rt->gl.resolve;
    }

    if (any(clearFlags)) {
        gl.disable(GL_SCISSOR_TEST);
        clearWithRasterPipe(clearFlags,
                params.clearColor, params.clearDepth, params.clearStencil);
    }

    // we need to reset those after we call clearWithRasterPipe()
    mRenderPassColorWrite = any(clearFlags & TargetBufferFlags::COLOR_ALL);
    mRenderPassDepthWrite = any(clearFlags & TargetBufferFlags::DEPTH);

    gl.viewport(params.viewport.left, params.viewport.bottom,
            params.viewport.width, params.viewport.height);

    gl.depthRange(params.depthRange.near, params.depthRange.far);

#ifndef NDEBUG
    // clear the discarded (but not the cleared ones) buffers in debug builds
    gl.disable(GL_SCISSOR_TEST);
    clearWithRasterPipe(discardFlags & ~clearFlags,
            { 1, 0, 0, 1 }, 1.0, 0);
#endif
}

void OpenGLDriver::endRenderPass(int) {
    DEBUG_MARKER()
    auto& gl = mContext;

    assert_invariant(mRenderPassTarget); // endRenderPass() called without beginRenderPass()?

    GLRenderTarget const* const rt = handle_cast<GLRenderTarget*>(mRenderPassTarget);

    TargetBufferFlags discardFlags = mRenderPassParams.flags.discardEnd & rt->targets;
    if (rt->gl.fbo_read) {
        resolvePass(ResolveAction::STORE, rt, discardFlags);
    }

    if (!mRenderPassColorWrite) {
        discardFlags &= ~TargetBufferFlags::COLOR_ALL;
    }
    if (!mRenderPassDepthWrite) {
        discardFlags &= ~TargetBufferFlags::DEPTH;
    }

    // glInvalidateFramebuffer appeared on GLES 3.0 and GL4.3, for simplicity we just
    // ignore it on GL (rather than having to do a runtime check).
    if (BACKEND_OPENGL_VERSION == BACKEND_OPENGL_VERSION_GLES &&
            BACKEND_OPENGL_LEVEL >= BACKEND_OPENGL_LEVEL_GLES30) {
        auto effectiveDiscardFlags = discardFlags;
        if (gl.bugs.invalidate_end_only_if_invalidate_start) {
            effectiveDiscardFlags &= mRenderPassParams.flags.discardStart;
        }
        if (!gl.bugs.disable_invalidate_framebuffer) {
            // we wouldn't have to bind the framebuffer if we had glInvalidateNamedFramebuffer()
            gl.bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);
            AttachmentArray attachments; // NOLINT
            GLsizei attachmentCount = getAttachments(attachments, rt, effectiveDiscardFlags);
            if (attachmentCount) {
                glInvalidateFramebuffer(GL_FRAMEBUFFER, attachmentCount, attachments.data());
            }
           CHECK_GL_ERROR(utils::slog.e)
        }
    }

#ifndef NDEBUG
    // clear the discarded buffers in debug builds
    mContext.bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);
    mContext.disable(GL_SCISSOR_TEST);
    clearWithRasterPipe(discardFlags,
            { 0, 1, 0, 1 }, 1.0, 0);
#endif

    mRenderPassTarget.clear();
}


void OpenGLDriver::nextSubpass(int) {}


void OpenGLDriver::resolvePass(ResolveAction action, GLRenderTarget const* rt,
        TargetBufferFlags discardFlags) noexcept {
    assert_invariant(rt->gl.fbo_read);
    auto& gl = mContext;
    const TargetBufferFlags resolve = rt->gl.resolve & ~discardFlags;
    GLbitfield mask = getAttachmentBitfield(resolve);
    if (UTILS_UNLIKELY(mask)) {

        // we can only resolve COLOR0 at the moment
        assert_invariant(!(rt->targets &
                (TargetBufferFlags::COLOR_ALL & ~TargetBufferFlags::COLOR0)));

        GLint read = rt->gl.fbo_read;
        GLint draw = rt->gl.fbo;
        if (action == ResolveAction::STORE) {
            std::swap(read, draw);
        }
        gl.bindFramebuffer(GL_READ_FRAMEBUFFER, read);
        gl.bindFramebuffer(GL_DRAW_FRAMEBUFFER, draw);

        CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e, GL_READ_FRAMEBUFFER)
        CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e, GL_DRAW_FRAMEBUFFER)

        gl.disable(GL_SCISSOR_TEST);
        glBlitFramebuffer(0, 0, rt->width, rt->height,
                0, 0, rt->width, rt->height, mask, GL_NEAREST);
        CHECK_GL_ERROR(utils::slog.e)
    }
}

GLsizei OpenGLDriver::getAttachments(AttachmentArray& attachments,
        GLRenderTarget const* rt, TargetBufferFlags buffers) noexcept {
    assert_invariant(buffers <= rt->targets);

    GLsizei attachmentCount = 0;
    // the default framebuffer uses different constants!!!
    const bool defaultFramebuffer = (rt->gl.fbo == 0);
    if (any(buffers & TargetBufferFlags::COLOR0)) {
        attachments[attachmentCount++] = defaultFramebuffer ? GL_COLOR : GL_COLOR_ATTACHMENT0;
    }
    if (any(buffers & TargetBufferFlags::COLOR1)) {
        assert_invariant(!defaultFramebuffer);
        attachments[attachmentCount++] = GL_COLOR_ATTACHMENT1;
    }
    if (any(buffers & TargetBufferFlags::COLOR2)) {
        assert_invariant(!defaultFramebuffer);
        attachments[attachmentCount++] = GL_COLOR_ATTACHMENT2;
    }
    if (any(buffers & TargetBufferFlags::COLOR3)) {
        assert_invariant(!defaultFramebuffer);
        attachments[attachmentCount++] = GL_COLOR_ATTACHMENT3;
    }
    if (any(buffers & TargetBufferFlags::COLOR4)) {
        assert_invariant(!defaultFramebuffer);
        attachments[attachmentCount++] = GL_COLOR_ATTACHMENT4;
    }
    if (any(buffers & TargetBufferFlags::COLOR5)) {
        assert_invariant(!defaultFramebuffer);
        attachments[attachmentCount++] = GL_COLOR_ATTACHMENT5;
    }
    if (any(buffers & TargetBufferFlags::COLOR6)) {
        assert_invariant(!defaultFramebuffer);
        attachments[attachmentCount++] = GL_COLOR_ATTACHMENT6;
    }
    if (any(buffers & TargetBufferFlags::COLOR7)) {
        assert_invariant(!defaultFramebuffer);
        attachments[attachmentCount++] = GL_COLOR_ATTACHMENT7;
    }
    if (any(buffers & TargetBufferFlags::DEPTH)) {
        attachments[attachmentCount++] = defaultFramebuffer ? GL_DEPTH : GL_DEPTH_ATTACHMENT;
    }
    if (any(buffers & TargetBufferFlags::STENCIL)) {
        attachments[attachmentCount++] = defaultFramebuffer ? GL_STENCIL : GL_STENCIL_ATTACHMENT;
    }
    return attachmentCount;
}

void OpenGLDriver::setRenderPrimitiveBuffer(Handle<HwRenderPrimitive> rph,
        Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh) {
    DEBUG_MARKER()
    auto& gl = mContext;

    if (rph) {
        GLRenderPrimitive* const rp = handle_cast<GLRenderPrimitive*>(rph);
        GLVertexBuffer const* const eb = handle_cast<const GLVertexBuffer*>(vbh);
        GLIndexBuffer const* const ib = handle_cast<const GLIndexBuffer*>(ibh);

        assert_invariant(ib->elementSize == 2 || ib->elementSize == 4);

        gl.bindVertexArray(&rp->gl);
        CHECK_GL_ERROR(utils::slog.e)

        rp->gl.indicesSize = (ib->elementSize == 4u) ? 4u : 2u;
        rp->gl.vertexBufferWithObjects = vbh;

        // update the VBO bindings in the VAO
        updateVertexArrayObject(rp, eb);

        // this records the index buffer into the currently bound VAO
        gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->gl.buffer);

        CHECK_GL_ERROR(utils::slog.e)
    }
}

void OpenGLDriver::setRenderPrimitiveRange(Handle<HwRenderPrimitive> rph,
        PrimitiveType pt, uint32_t offset,
        uint32_t minIndex, uint32_t maxIndex, uint32_t count) {
    DEBUG_MARKER()

    GLRenderPrimitive* const rp = handle_cast<GLRenderPrimitive*>(rph);
    rp->type = pt;
    rp->offset = offset * rp->gl.indicesSize;
    rp->count = count;
    rp->minIndex = minIndex;
    rp->maxIndex = maxIndex > minIndex ? maxIndex : rp->maxVertexCount - 1; // sanitize max index
}

// Sets up a scissor rectangle that automatically gets clipped against the viewport.
void OpenGLDriver::setViewportScissor(Viewport const& viewportScissor) noexcept {
    DEBUG_MARKER()
    auto& gl = mContext;

    // In OpenGL, all four scissor parameters are actually signed, so clamp to MAX_INT32.
    const int32_t maxval = std::numeric_limits<int32_t>::max();
    int32_t left = std::min(viewportScissor.left, maxval);
    int32_t bottom = std::min(viewportScissor.bottom, maxval);
    uint32_t width = std::min(viewportScissor.width, uint32_t(maxval));
    uint32_t height = std::min(viewportScissor.height, uint32_t(maxval));
    // Compute the intersection of the requested scissor rectangle with the current viewport.
    // Note that the viewport rectangle isn't necessarily equal to the bounds of the current
    // Filament View (e.g., when post-processing is enabled).
    OpenGLContext::vec4gli scissor;
    OpenGLContext::vec4gli viewport = gl.getViewport();
    scissor.x = std::max((int32_t)left, viewport[0]);
    scissor.y = std::max((int32_t)bottom, viewport[1]);
    int32_t right = std::min((int32_t)left + (int32_t)width, viewport[0] + viewport[2]);
    int32_t top = std::min((int32_t)bottom + (int32_t)height, viewport[1] + viewport[3]);
    // Compute width / height of the intersected region. If there's no intersection, pass zeroes
    // rather than negative values to satisfy OpenGL requirements.
    scissor.z = std::max(0, right - scissor.x);
    scissor.w = std::max(0, top - scissor.y);
    gl.setScissor(scissor.x, scissor.y, scissor.z, scissor.w);
    gl.enable(GL_SCISSOR_TEST);
}

// Binds the external image stashed in the associated stream.
//
// updateStreamAcquired() and setAcquiredImage() are both called from on the application's thread
// and therefore do not require synchronization. The former is always called immediately before
// beginFrame, the latter is called by the user from anywhere outside beginFrame / endFrame.
void OpenGLDriver::updateStreamAcquired(GLTexture* gltexture, DriverApi* driver) noexcept {
    SYSTRACE_CALL();

    GLStream* glstream = static_cast<GLStream*>(gltexture->hwStream);
    assert_invariant(glstream);
    assert_invariant(glstream->streamType == StreamType::ACQUIRED);

    // If there's no pending image, do nothing. Note that GL_OES_EGL_image does not let you pass
    // NULL to glEGLImageTargetTexture2DOES, and there is no concept of "detaching" an EGLimage from
    // a texture.
    if (glstream->user_thread.pending.image == nullptr) {
        return;
    }

    AcquiredImage previousImage = glstream->user_thread.acquired;
    glstream->user_thread.acquired = glstream->user_thread.pending;
    glstream->user_thread.pending = {0};

    // Bind the stashed EGLImage to its corresponding GL texture as soon as we start making the GL
    // calls for the upcoming frame.
    void* image = glstream->user_thread.acquired.image;
    driver->queueCommand([this, gltexture, image, previousImage]() {
        setExternalTexture(gltexture, image);
        if (previousImage.image) {
            scheduleRelease(AcquiredImage(previousImage));
        }
    });
}

#define DEBUG_NO_EXTERNAL_STREAM_COPY false

void OpenGLDriver::updateStreamTexId(GLTexture* t, DriverApi* driver) noexcept {
    SYSTRACE_CALL();
    auto& gl = mContext;

    GLStream* s = static_cast<GLStream*>(t->hwStream);
    assert_invariant(s);
    assert_invariant(s->streamType == StreamType::TEXTURE_ID);

    // round-robin to the next texture name
    if (UTILS_UNLIKELY(DEBUG_NO_EXTERNAL_STREAM_COPY ||
                       gl.bugs.disable_shared_context_draws || !mOpenGLBlitter)) {
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
    auto& gl = mContext;

    GLStream* s = handle_cast<GLStream*>(sh);

    if (UTILS_UNLIKELY(s->streamType == StreamType::ACQUIRED)) {
        PANIC_LOG("readStreamPixels with ACQUIRED streams is not yet implemented.");
        return;
    }

    if (UTILS_LIKELY(s->streamType == StreamType::TEXTURE_ID)) {
        GLuint tid = s->gl.externalTexture2DId;
        if (tid == 0) {
            return;
        }

        GLenum glFormat = getFormat(p.format);
        GLenum glType = getType(p.type);

        gl.pixelStore(GL_PACK_ROW_LENGTH, p.stride);
        gl.pixelStore(GL_PACK_ALIGNMENT, p.alignment);
        gl.pixelStore(GL_PACK_SKIP_PIXELS, p.left);
        gl.pixelStore(GL_PACK_SKIP_ROWS, p.top);

        if (s->gl.fbo == 0) {
            glGenFramebuffers(1, &s->gl.fbo);
        }
        gl.bindFramebuffer(GL_FRAMEBUFFER, s->gl.fbo);

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

        gl.bindFramebuffer(GL_FRAMEBUFFER, 0);
        scheduleDestroy(std::move(p));
    }
}

// ------------------------------------------------------------------------------------------------
// Setting rendering state
// ------------------------------------------------------------------------------------------------

void OpenGLDriver::bindUniformBuffer(uint32_t index, Handle<HwBufferObject> ubh) {
    DEBUG_MARKER()
    auto& gl = mContext;
    GLBufferObject* ub = handle_cast<GLBufferObject *>(ubh);
    assert_invariant(ub->gl.binding == GL_UNIFORM_BUFFER);
    assert_invariant(ub->base == 0);
    gl.bindBufferRange(ub->gl.binding, GLuint(index), ub->gl.id, 0, ub->byteCount);
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::bindUniformBufferRange(uint32_t index, Handle<HwBufferObject> ubh,
        uint32_t offset, uint32_t size) {
    DEBUG_MARKER()
    auto& gl = mContext;

    GLBufferObject* ub = handle_cast<GLBufferObject *>(ubh);
    assert_invariant(ub->gl.binding == GL_UNIFORM_BUFFER);
    assert_invariant(ub->base + offset + size <= ub->byteCount);
    gl.bindBufferRange(ub->gl.binding, GLuint(index), ub->gl.id, ub->base + offset, size);
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::bindSamplers(uint32_t index, Handle<HwSamplerGroup> sbh) {
    DEBUG_MARKER()
    assert_invariant(index < Program::BINDING_COUNT);
    GLSamplerGroup* sb = handle_cast<GLSamplerGroup *>(sbh);
    mSamplerBindings[index] = sb;
    CHECK_GL_ERROR(utils::slog.e)
}


GLuint OpenGLDriver::getSamplerSlow(SamplerParams params) const noexcept {
    assert_invariant(mSamplerMap.find(params.u) == mSamplerMap.end());

    GLuint s;
    glGenSamplers(1, &s);
    glSamplerParameteri(s, GL_TEXTURE_MIN_FILTER,   getTextureFilter(params.filterMin));
    glSamplerParameteri(s, GL_TEXTURE_MAG_FILTER,   getTextureFilter(params.filterMag));
    glSamplerParameteri(s, GL_TEXTURE_WRAP_S,       getWrapMode(params.wrapS));
    glSamplerParameteri(s, GL_TEXTURE_WRAP_T,       getWrapMode(params.wrapT));
    glSamplerParameteri(s, GL_TEXTURE_WRAP_R,       getWrapMode(params.wrapR));
    glSamplerParameteri(s, GL_TEXTURE_COMPARE_MODE, getTextureCompareMode(params.compareMode));
    glSamplerParameteri(s, GL_TEXTURE_COMPARE_FUNC, getTextureCompareFunc(params.compareFunc));

#if defined(GL_EXT_texture_filter_anisotropic)
    auto& gl = mContext;
    if (gl.ext.EXT_texture_filter_anisotropic &&
            !gl.bugs.texture_filter_anisotropic_broken_on_sampler) {
        GLfloat anisotropy = float(1u << params.anisotropyLog2);
        glSamplerParameterf(s, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                std::min(gl.gets.max_anisotropy, anisotropy));
    }
#endif
    CHECK_GL_ERROR(utils::slog.e)
    mSamplerMap[params.u] = s;
    return s;
}

void OpenGLDriver::insertEventMarker(char const* string, uint32_t len) {
#ifdef GL_EXT_debug_marker
    auto& gl = mContext;
    if (gl.ext.EXT_debug_marker) {
        glInsertEventMarkerEXT(GLsizei(len ? len : strlen(string)), string);
    }
#endif
}

void OpenGLDriver::pushGroupMarker(char const* string, uint32_t len) {
#ifdef GL_EXT_debug_marker
    auto& gl = mContext;
    if (UTILS_LIKELY(gl.ext.EXT_debug_marker)) {
        glPushGroupMarkerEXT(GLsizei(len ? len : strlen(string)), string);
    } else
#endif
    {
        SYSTRACE_CONTEXT();
        SYSTRACE_NAME_BEGIN(string);
    }
}

void OpenGLDriver::popGroupMarker(int) {
#ifdef GL_EXT_debug_marker
    auto& gl = mContext;
    if (UTILS_LIKELY(gl.ext.EXT_debug_marker)) {
        glPopGroupMarkerEXT();
    } else
#endif
    {
        SYSTRACE_CONTEXT();
        SYSTRACE_NAME_END();
    }
}

void OpenGLDriver::startCapture(int) {
}

void OpenGLDriver::stopCapture(int) {
}

// ------------------------------------------------------------------------------------------------
// Read-back ops
// ------------------------------------------------------------------------------------------------

void OpenGLDriver::readPixels(Handle<HwRenderTarget> src,
        uint32_t x, uint32_t y, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& p) {
    DEBUG_MARKER()
    auto& gl = mContext;

    GLenum glFormat = getFormat(p.format);
    GLenum glType = getType(p.type);

    gl.pixelStore(GL_PACK_ROW_LENGTH, p.stride);
    gl.pixelStore(GL_PACK_ALIGNMENT, p.alignment);
    gl.pixelStore(GL_PACK_SKIP_PIXELS, p.left);
    gl.pixelStore(GL_PACK_SKIP_ROWS, p.top);

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
    gl.bindFramebuffer(GL_READ_FRAMEBUFFER, s->gl.fbo);

    GLuint pbo;
    glGenBuffers(1, &pbo);
    gl.bindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_PACK_BUFFER, p.size, nullptr, GL_STATIC_DRAW);
    glReadPixels(GLint(x), GLint(y), GLint(width), GLint(height), glFormat, glType, nullptr);
    gl.bindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    CHECK_GL_ERROR(utils::slog.e)

    // we're forced to make a copy on the heap because otherwise it deletes std::function<> copy
    // constructor.
    auto* pUserBuffer = new PixelBufferDescriptor(std::move(p));
    whenGpuCommandsComplete([this, width, height, pbo, pUserBuffer]() mutable {
        PixelBufferDescriptor& p = *pUserBuffer;
        auto& gl = mContext;
        gl.bindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
        void* vaddr = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0,  p.size, GL_MAP_READ_BIT);
        if (vaddr) {
            // now we need to flip the buffer vertically to match our API
            size_t stride = p.stride ? p.stride : width;
            size_t bpp = PixelBufferDescriptor::computeDataSize(
                    p.format, p.type, 1, 1, 1);
            size_t bpr = PixelBufferDescriptor::computeDataSize(
                    p.format, p.type, stride, 1, p.alignment);
            char const* head = (char const*)vaddr + p.left * bpp + bpr * p.top;
            char* tail = (char*)p.buffer + p.left * bpp + bpr * (p.top + height - 1);
            for (size_t i = 0; i < height; ++i) {
                memcpy(tail, head, bpp * width);
                head += bpr;
                tail -= bpr;
            }
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        }
        gl.bindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        glDeleteBuffers(1, &pbo);
        scheduleDestroy(std::move(p));
        delete pUserBuffer;
        CHECK_GL_ERROR(utils::slog.e)
    });
}

void OpenGLDriver::whenGpuCommandsComplete(std::function<void()> fn) noexcept {
    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    mGpuCommandCompleteOps.emplace_back(sync, std::move(fn));
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::runEveryNowAndThen(std::function<bool()> fn) noexcept {
    mEveryNowAndThenOps.push_back(std::move(fn));
}

void OpenGLDriver::executeGpuCommandsCompleteOps() noexcept {
    auto& v = mGpuCommandCompleteOps;
    auto it = v.begin();
    while (it != v.end()) {
        GLenum status = glClientWaitSync(it->first, 0, 0);
        if (status == GL_ALREADY_SIGNALED || status == GL_CONDITION_SATISFIED) {
            it->second();
            glDeleteSync(it->first);
            it = v.erase(it);
        } else if (UTILS_UNLIKELY(status == GL_WAIT_FAILED)) {
            // This should never happen, but is very problematic if it does, as we might leak
            // some data depending on what the callback does. However, we clean-up our own state.
            glDeleteSync(it->first);
            it = v.erase(it);
        } else {
            ++it;
        }
    }
}

void OpenGLDriver::executeEveryNowAndThenOps() noexcept {
    auto& v = mEveryNowAndThenOps;
    auto it = v.begin();
    while (it != v.end()) {
        if ((*it)()) {
            it = v.erase(it);
        } else {
            ++it;
        }
    }
}

void OpenGLDriver::runAtNextRenderPass(void* token, std::function<void()> fn) noexcept {
    assert_invariant(mRunAtNextRenderPassOps.find(token) == mRunAtNextRenderPassOps.end());
    mRunAtNextRenderPassOps[token] = std::move(fn);
}

void OpenGLDriver::cancelRunAtNextPassOp(void* token) noexcept {
    mRunAtNextRenderPassOps.erase(token);
}

void OpenGLDriver::executeRenderPassOps() noexcept {
    auto& ops = mRunAtNextRenderPassOps;
    if (!ops.empty()) {
        for (auto& item: ops) {
            item.second();
        }
        ops.clear();
    }
}

// ------------------------------------------------------------------------------------------------
// Rendering ops
// ------------------------------------------------------------------------------------------------

void OpenGLDriver::tick(int) {
    executeGpuCommandsCompleteOps();
    executeEveryNowAndThenOps();
}

void OpenGLDriver::beginFrame(int64_t monotonic_clock_ns, uint32_t frameId) {
    auto& gl = mContext;
    insertEventMarker("beginFrame");
    if (UTILS_UNLIKELY(!mExternalStreams.empty())) {
        OpenGLPlatform& platform = mPlatform;
        for (GLTexture const* t : mExternalStreams) {
            assert_invariant(t && t->hwStream);
            if (t->hwStream->streamType == StreamType::NATIVE) {
                assert_invariant(t->hwStream->stream);
                platform.updateTexImage(t->hwStream->stream,
                        &static_cast<GLStream*>(t->hwStream)->user_thread.timestamp);
                // NOTE: We assume that updateTexImage() binds the texture on our behalf
                gl.updateTexImage(GL_TEXTURE_EXTERNAL_OES, t->gl.id);
            }
        }
    }
}

void OpenGLDriver::setFrameScheduledCallback(Handle<HwSwapChain> sch,
        FrameScheduledCallback callback, void* user) {

}

void OpenGLDriver::setFrameCompletedCallback(Handle<HwSwapChain> sch,
        FrameCompletedCallback callback, void* user) {

}

void OpenGLDriver::setPresentationTime(int64_t monotonic_clock_ns) {
    mPlatform.setPresentationTime(monotonic_clock_ns);
}

void OpenGLDriver::endFrame(uint32_t frameId) {
    //SYSTRACE_NAME("glFinish");
#if defined(__EMSCRIPTEN__)
    // WebGL builds are single-threaded so users might manipulate various GL state after we're
    // done with the frame. We do NOT officially support using Filament in this way, but we can
    // at least do some minimal safety things here, such as resetting the VAO to 0.
    auto& gl = mContext;
    gl.bindVertexArray(nullptr);
    for (int unit = OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1; unit >= 0; unit--) {
        gl.bindTexture(unit, GL_TEXTURE_2D, 0);
    }
    gl.disable(GL_CULL_FACE);
    gl.depthFunc(GL_LESS);
    gl.disable(GL_SCISSOR_TEST);
#endif
    //glFinish();
    insertEventMarker("endFrame");
}

void OpenGLDriver::flush(int) {
    DEBUG_MARKER()
    auto& gl = mContext;
    if (!gl.bugs.disable_glFlush) {
        glFlush();
    }
    mTimerQueryImpl->flush();
}

void OpenGLDriver::finish(int) {
    DEBUG_MARKER()
    glFinish();
    mTimerQueryImpl->flush();
    executeGpuCommandsCompleteOps();
    executeEveryNowAndThenOps();
    // Note: since we executed a glFinish(), all pending tasks should be done
    assert_invariant(mGpuCommandCompleteOps.empty());

    // however, some tasks rely on a separated thread to publish their result (e.g.
    // endTimerQuery), so the result could very well not be ready, and the task will
    // linger a bit longer, this is only true for mEveryNowAndThenOps tasks.
    // The fallout of this is that we can't assert that mEveryNowAndThenOps is empty.
}

UTILS_NOINLINE
void OpenGLDriver::clearWithRasterPipe(TargetBufferFlags clearFlags,
        math::float4 const& linearColor, GLfloat depth, GLint stencil) noexcept {
    DEBUG_MARKER()
    RasterState rs(mRasterState);

    if (any(clearFlags & TargetBufferFlags::COLOR_ALL)) {
        rs.colorWrite = true;
    }
    if (any(clearFlags & TargetBufferFlags::DEPTH)) {
        rs.depthWrite = true;
    }
    // stencil state is not part of the RasterState currently
    if (any(clearFlags & (TargetBufferFlags::COLOR_ALL | TargetBufferFlags::DEPTH))) {
        setRasterState(rs);
    }

    if (any(clearFlags & TargetBufferFlags::COLOR0)) {
        glClearBufferfv(GL_COLOR, 0, linearColor.v);
    }
    if (any(clearFlags & TargetBufferFlags::COLOR1)) {
        glClearBufferfv(GL_COLOR, 1, linearColor.v);
    }
    if (any(clearFlags & TargetBufferFlags::COLOR2)) {
        glClearBufferfv(GL_COLOR, 2, linearColor.v);
    }
    if (any(clearFlags & TargetBufferFlags::COLOR3)) {
        glClearBufferfv(GL_COLOR, 3, linearColor.v);
    }
    if (any(clearFlags & TargetBufferFlags::COLOR4)) {
        glClearBufferfv(GL_COLOR, 4, linearColor.v);
    }
    if (any(clearFlags & TargetBufferFlags::COLOR5)) {
        glClearBufferfv(GL_COLOR, 5, linearColor.v);
    }
    if (any(clearFlags & TargetBufferFlags::COLOR6)) {
        glClearBufferfv(GL_COLOR, 6, linearColor.v);
    }
    if (any(clearFlags & TargetBufferFlags::COLOR7)) {
        glClearBufferfv(GL_COLOR, 7, linearColor.v);
    }

    if ((clearFlags & TargetBufferFlags::DEPTH_AND_STENCIL) == TargetBufferFlags::DEPTH_AND_STENCIL) {
        glClearBufferfi(GL_DEPTH_STENCIL, 0, depth, stencil);
    } else {
        if (any(clearFlags & TargetBufferFlags::DEPTH)) {
            glClearBufferfv(GL_DEPTH, 0, &depth);
        }
        if (any(clearFlags & TargetBufferFlags::STENCIL)) {
            glClearBufferiv(GL_STENCIL, 0, &stencil);
        }
    }
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::blit(TargetBufferFlags buffers,
        Handle<HwRenderTarget> dst, Viewport dstRect,
        Handle<HwRenderTarget> src, Viewport srcRect,
        SamplerMagFilter filter) {
    DEBUG_MARKER()
    auto& gl = mContext;

    GLbitfield mask = getAttachmentBitfield(buffers);
    if (mask) {
        GLenum glFilterMode = (filter == SamplerMagFilter::NEAREST) ? GL_NEAREST : GL_LINEAR;
        if (mask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) {
            // GL_INVALID_OPERATION is generated if mask contains any of the GL_DEPTH_BUFFER_BIT or
            // GL_STENCIL_BUFFER_BIT and filter is not GL_NEAREST.
            glFilterMode = GL_NEAREST;
        }

        // note: for msaa RenderTargets with non-msaa attachments, we copy from the msaa sidecar
        // buffer -- this should produce the same output that if we copied from the resolved
        // texture. EXT_multisampled_render_to_texture seems to allow both behaviours, and this
        // is an emulation of that.  We cannot use the resolved texture easily because it's not
        // actually attached to the this RenderTarget. Another implementation would be to do a
        // reverse-resolve, but that wouldn't buy us anything.
        GLRenderTarget const* s = handle_cast<GLRenderTarget const*>(src);
        GLRenderTarget const* d = handle_cast<GLRenderTarget const*>(dst);

        // blit operations are only supported from RenderTargets that have only COLOR0 (or
        // no color buffer at all)
        assert_invariant(
                !(s->targets & (TargetBufferFlags::COLOR_ALL & ~TargetBufferFlags::COLOR0)));

        assert_invariant(
                !(d->targets & (TargetBufferFlags::COLOR_ALL & ~TargetBufferFlags::COLOR0)));

        // With GLES 3.x, GL_INVALID_OPERATION is generated if the value of GL_SAMPLE_BUFFERS
        // for the draw buffer is greater than zero. This works with OpenGL, so we want to
        // make sure to catch this scenario.
        assert_invariant(d->gl.samples <= 1);

        // GL_INVALID_OPERATION is generated if GL_SAMPLE_BUFFERS for the read buffer is greater
        // than zero and the formats of draw and read buffers are not identical.
        // However, it's not well-defined in the spec what "format" means. So it's difficult
        // to have an assert here -- especially when dealing with the default framebuffer

        // GL_INVALID_OPERATION is generated if GL_SAMPLE_BUFFERS for the read buffer is greater
        // than zero and (...) the source and destination rectangles are not defined with the
        // same (X0, Y0) and (X1, Y1) bounds.

        // Additionally, the EXT_multisampled_render_to_texture extension doesn't specify what
        // happens when blitting from an "implicit" resolve render target (does it work?), so
        // to ere on the safe side, we don't allow it.
        if (s->gl.samples > 1) {
            assert_invariant(!memcmp(&dstRect, &srcRect, sizeof(srcRect)));
        }

        gl.bindFramebuffer(GL_READ_FRAMEBUFFER, s->gl.fbo);
        gl.bindFramebuffer(GL_DRAW_FRAMEBUFFER, d->gl.fbo);

        CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e, GL_READ_FRAMEBUFFER)
        CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e, GL_DRAW_FRAMEBUFFER)

        gl.disable(GL_SCISSOR_TEST);
        glBlitFramebuffer(
                srcRect.left, srcRect.bottom, srcRect.right(), srcRect.top(),
                dstRect.left, dstRect.bottom, dstRect.right(), dstRect.top(),
                mask, glFilterMode);
        CHECK_GL_ERROR(utils::slog.e)
    }
}

void OpenGLDriver::updateTextureLodRange(GLTexture* texture, int8_t targetLevel) noexcept {
    auto& gl = mContext;
    if (texture && any(texture->usage & TextureUsage::SAMPLEABLE)) {
        if (targetLevel < texture->gl.baseLevel || targetLevel > texture->gl.maxLevel) {
            bindTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1, texture);
            gl.activeTexture(OpenGLContext::MAX_TEXTURE_UNIT_COUNT - 1);
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

void OpenGLDriver::draw(PipelineState state, Handle<HwRenderPrimitive> rph, uint32_t instanceCount) {
    DEBUG_MARKER()
    auto& gl = mContext;

    OpenGLProgram* p = handle_cast<OpenGLProgram*>(state.program);

    // If the material debugger is enabled, avoid fatal (or cascading) errors and that can occur
    // during the draw call when the program is invalid. The shader compile error has already been
    // dumped to the console at this point, so it's fine to simply return early.
    if (FILAMENT_ENABLE_MATDBG && UTILS_UNLIKELY(!p->isValid())) {
        return;
    }

    useProgram(p);

    GLRenderPrimitive* rp = handle_cast<GLRenderPrimitive *>(rph);

    // Gracefully do nothing if the render primitive has not been set up.
    VertexBufferHandle vb = rp->gl.vertexBufferWithObjects;
    if (UTILS_UNLIKELY(!vb)) {
        return;
    }

    gl.bindVertexArray(&rp->gl);

    // If necessary, mutate the bindings in the VAO.
    const GLVertexBuffer* glvb = handle_cast<GLVertexBuffer*>(vb);
    if (UTILS_UNLIKELY(rp->gl.vertexBufferVersion != glvb->bufferObjectsVersion)) {
        updateVertexArrayObject(rp, glvb);
    }

    setRasterState(state.rasterState);

    gl.polygonOffset(state.polygonOffset.slope, state.polygonOffset.constant);

    setViewportScissor(state.scissor);

    if (UTILS_LIKELY(instanceCount <= 1)) {
        glDrawRangeElements(GLenum(rp->type), rp->minIndex, rp->maxIndex, rp->count,
                rp->gl.getIndicesType(), reinterpret_cast<const void*>(rp->offset));
    } else {
        glDrawElementsInstanced(GLenum(rp->type), rp->count,
                rp->gl.getIndicesType(), reinterpret_cast<const void*>(rp->offset),
                instanceCount);
    }

    CHECK_GL_ERROR(utils::slog.e)
}

// explicit instantiation of the Dispatcher
template class ConcreteDispatcher<OpenGLDriver>;

} // namespace filament::backend
