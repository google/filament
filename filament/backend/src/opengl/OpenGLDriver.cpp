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

#include "CommandStreamDispatcher.h"
#include "OpenGLContext.h"
#include "OpenGLDriverFactory.h"
#include "OpenGLProgram.h"
#include "OpenGLTimerQuery.h"

#include <backend/platforms/OpenGLPlatform.h>
#include <backend/SamplerDescriptor.h>

#include <utils/compiler.h>
#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/Systrace.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

// We can only support this feature on OpenGL ES 3.1+
// Support is currently disabled as we don't need it
#define TEXTURE_2D_MULTISAMPLE_SUPPORTED false

#if defined(__EMSCRIPTEN__)
#define HAS_MAPBUFFERS 0
#else
#define HAS_MAPBUFFERS 1
#endif

#define DEBUG_MARKER_NONE       0x00    // no debug marker
#define DEBUG_MARKER_OPENGL     0x01    // markers in the gl command queue (req. driver support)
#define DEBUG_MARKER_BACKEND    0x02    // markers on the backend side (systrace)
#define DEBUG_MARKER_ALL        0x03    // all markers

// set to the desired debug marker level
#define DEBUG_MARKER_LEVEL      DEBUG_MARKER_NONE

#if DEBUG_MARKER_LEVEL > DEBUG_MARKER_NONE
#   define DEBUG_MARKER() \
        DebugMarker _debug_marker(*this, __func__);
#else
#   define DEBUG_MARKER()
#endif

using namespace filament::math;
using namespace utils;

namespace filament::backend {

Driver* OpenGLDriverFactory::create(
        OpenGLPlatform* const platform,
        void* const sharedGLContext,
        const Platform::DriverConfig& driverConfig) noexcept {
    return OpenGLDriver::create(platform, sharedGLContext, driverConfig);
}

using namespace GLUtils;

// ------------------------------------------------------------------------------------------------

UTILS_NOINLINE
Driver* OpenGLDriver::create(OpenGLPlatform* const platform,
        void* const sharedGLContext, const Platform::DriverConfig& driverConfig) noexcept {
    assert_invariant(platform);
    OpenGLPlatform* const ec = platform;

#if 0
    // this is useful for development, but too verbose even for debug builds
    // For reference on a 64-bits machine in Release mode:
    //    GLIndexBuffer             :   8       moderate
    //    GLSamplerGroup            :  16       few
    //    GLSwapChain               :  16       few
    //    GLTimerQuery              :  16       few
    // -- less than or equal 16 bytes
    //    GLFence                   :  24       few
    //    GLBufferObject            :  32       many
    //    GLRenderPrimitive         :  40       many
    //    OpenGLProgram             :  56       moderate
    //    GLTexture                 :  64       moderate
    // -- less than or equal 64 bytes
    //    GLStream                  : 104       few
    //    GLRenderTarget            : 112       few
    //    GLVertexBuffer            : 200       moderate
    // -- less than or equal to 208 bytes

    slog.d
           << "\nGLSwapChain: " << sizeof(GLSwapChain)
           << "\nGLBufferObject: " << sizeof(GLBufferObject)
           << "\nGLVertexBuffer: " << sizeof(GLVertexBuffer)
           << "\nGLIndexBuffer: " << sizeof(GLIndexBuffer)
           << "\nGLSamplerGroup: " << sizeof(GLSamplerGroup)
           << "\nGLRenderPrimitive: " << sizeof(GLRenderPrimitive)
           << "\nGLTexture: " << sizeof(GLTexture)
           << "\nGLTimerQuery: " << sizeof(GLTimerQuery)
           << "\nGLStream: " << sizeof(GLStream)
           << "\nGLRenderTarget: " << sizeof(GLRenderTarget)
           << "\nGLFence: " << sizeof(GLFence)
           << "\nOpenGLProgram: " << sizeof(OpenGLProgram)
           << io::endl;
#endif

    // here we check we're on a supported version of GL before initializing the driver
    GLint major = 0, minor = 0;
    bool const success = OpenGLContext::queryOpenGLVersion(&major, &minor);

    if (UTILS_UNLIKELY(!success)) {
        PANIC_LOG("Can't get OpenGL version");
        cleanup:
        ec->terminate();
        return {};
    }

#if defined(BACKEND_OPENGL_VERSION_GLES)
    if (UTILS_UNLIKELY(!(major >= 2 && minor >= 0))) {
        PANIC_LOG("OpenGL ES 2.0 minimum needed (current %d.%d)", major, minor);
        goto cleanup;
    }
#else
    // we require GL 4.1 headers and minimum version
    if (UTILS_UNLIKELY(!((major == 4 && minor >= 1) || major > 4))) {
        PANIC_LOG("OpenGL 4.1 minimum needed (current %d.%d)", major, minor);
        goto cleanup;
    }
#endif

    size_t const defaultSize = FILAMENT_OPENGL_HANDLE_ARENA_SIZE_IN_MB * 1024U * 1024U;
    Platform::DriverConfig validConfig{ driverConfig };
    validConfig.handleArenaSize = std::max(driverConfig.handleArenaSize, defaultSize);
    OpenGLDriver* const driver = new(std::nothrow) OpenGLDriver(ec, validConfig);
    return driver;
}

// ------------------------------------------------------------------------------------------------

OpenGLDriver::DebugMarker::DebugMarker(OpenGLDriver& driver, const char* string) noexcept
        : driver(driver) {
    driver.pushGroupMarker(string, strlen(string));
}

OpenGLDriver::DebugMarker::~DebugMarker() noexcept {
    driver.popGroupMarker();
}

// ------------------------------------------------------------------------------------------------

OpenGLDriver::OpenGLDriver(OpenGLPlatform* platform, const Platform::DriverConfig& driverConfig) noexcept
        : mPlatform(*platform),
          mContext(),
          mShaderCompilerService(*this),
          mHandleAllocator("Handles", driverConfig.handleArenaSize),
          mSamplerMap(32),
          mDriverConfig(driverConfig) {

    std::fill(mSamplerBindings.begin(), mSamplerBindings.end(), nullptr);

    // set a reasonable default value for our stream array
    mTexturesWithStreamsAttached.reserve(8);
    mStreamsWithPendingAcquiredImage.reserve(8);

#ifndef NDEBUG
    slog.i << "OS version: " << mPlatform.getOSVersion() << io::endl;
#endif

    // Timer queries are core in GL 3.3, otherwise we need EXT_disjoint_timer_query
    // iOS headers don't define GL_EXT_disjoint_timer_query, so make absolutely sure
    // we won't use it.

#if defined(BACKEND_OPENGL_VERSION_GL)
    assert_invariant(mContext.ext.EXT_disjoint_timer_query);
#endif

    mTimerQueryImpl = OpenGLTimerQueryFactory::init(mPlatform, *this);

    mShaderCompilerService.init();
}

OpenGLDriver::~OpenGLDriver() noexcept { // NOLINT(modernize-use-equals-default)
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

    mShaderCompilerService.terminate();

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    // and make sure to execute all the GpuCommandCompleteOps callbacks
    executeGpuCommandsCompleteOps();

    // as well as the FrameCompleteOps callbacks
    if (UTILS_UNLIKELY(!mFrameCompleteOps.empty())) {
        for (auto&& op: mFrameCompleteOps) {
            op();
        }
        mFrameCompleteOps.clear();
    }

    // because we called glFinish(), all callbacks should have been executed
    assert_invariant(mGpuCommandCompleteOps.empty());

    if (!getContext().isES2()) {
        for (auto& item: mSamplerMap) {
            mContext.unbindSampler(item.second);
            glDeleteSamplers(1, &item.second);
        }
        mSamplerMap.clear();
    }
#endif

    delete mTimerQueryImpl;

    mPlatform.terminate();
}

ShaderModel OpenGLDriver::getShaderModel() const noexcept {
    return mContext.getShaderModel();
}

// ------------------------------------------------------------------------------------------------
// Change and track GL state
// ------------------------------------------------------------------------------------------------
void OpenGLDriver::resetState(int) {
    mContext.resetState();
}

void OpenGLDriver::bindSampler(GLuint unit, GLuint sampler) noexcept {
    mContext.bindSampler(unit, sampler);
}

void OpenGLDriver::bindTexture(GLuint unit, GLTexture const* t) noexcept {
    assert_invariant(t != nullptr);
    mContext.bindTexture(unit, t->gl.target, t->gl.id, t->gl.targetIndex);
}

bool OpenGLDriver::useProgram(OpenGLProgram* p) noexcept {
    if (UTILS_UNLIKELY(!p->isValid())) {
        // If the program is not valid, we can't call use().
        return false;
    }

    // set-up textures and samplers in the proper TMUs (as specified in setSamplers)
    p->use(this, mContext);

    if (UTILS_UNLIKELY(mContext.isES2())) {
        for (uint32_t i = 0; i < Program::UNIFORM_BINDING_COUNT; i++) {
            auto [id, buffer, age] = mUniformBindings[i];
            if (buffer) {
                p->updateUniforms(i, id, buffer, age);
            }
        }
        // Set the output colorspace for this program (linear or rec709). This in only relevant
        // when mPlatform.isSRGBSwapChainSupported() is false (no need to check though).
        p->setRec709ColorSpace(mRec709OutputColorspace);
    }
    return true;
}


void OpenGLDriver::setRasterState(RasterState rs) noexcept {
    auto& gl = mContext;

    mRenderPassColorWrite |= rs.colorWrite;
    mRenderPassDepthWrite |= rs.depthWrite;

    // culling state
    if (rs.culling == CullingMode::NONE) {
        gl.disable(GL_CULL_FACE);
    } else {
        gl.enable(GL_CULL_FACE);
        gl.cullFace(getCullingMode(rs.culling));
    }

    gl.frontFace(rs.inverseFrontFaces ? GL_CW : GL_CCW);

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

void OpenGLDriver::setStencilState(StencilState ss) noexcept {
    auto& gl = mContext;

    mRenderPassStencilWrite |= ss.stencilWrite;

    // stencil test / operation
    // GL_STENCIL_TEST must be enabled if we're testing OR writing to the stencil buffer.
    if (UTILS_LIKELY(
            ss.front.stencilFunc == StencilState::StencilFunction::A &&
            ss.back.stencilFunc == StencilState::StencilFunction::A &&
            ss.front.stencilOpDepthFail == StencilOperation::KEEP &&
            ss.back.stencilOpDepthFail == StencilOperation::KEEP &&
            ss.front.stencilOpStencilFail == StencilOperation::KEEP &&
            ss.back.stencilOpStencilFail == StencilOperation::KEEP &&
            ss.front.stencilOpDepthStencilPass == StencilOperation::KEEP &&
            ss.back.stencilOpDepthStencilPass == StencilOperation::KEEP)) {
        // that's equivalent to having the stencil test disabled
        gl.disable(GL_STENCIL_TEST);
    } else {
        gl.enable(GL_STENCIL_TEST);
    }

    // glStencilFuncSeparate() also sets the reference value, which may be used depending
    // on the stencilOp, so we always need to call glStencilFuncSeparate().
    gl.stencilFuncSeparate(
            getStencilFunc(ss.front.stencilFunc), ss.front.ref, ss.front.readMask,
            getStencilFunc(ss.back.stencilFunc), ss.back.ref, ss.back.readMask);

    if (UTILS_LIKELY(!ss.stencilWrite)) {
        gl.stencilMaskSeparate(0x00, 0x00);
    } else {
        // Stencil ops are only relevant when stencil write is enabled
        gl.stencilOpSeparate(
                getStencilOp(ss.front.stencilOpStencilFail),
                getStencilOp(ss.front.stencilOpDepthFail),
                getStencilOp(ss.front.stencilOpDepthStencilPass),
                getStencilOp(ss.back.stencilOpStencilFail),
                getStencilOp(ss.back.stencilOpDepthFail),
                getStencilOp(ss.back.stencilOpDepthStencilPass));
        gl.stencilMaskSeparate(ss.front.writeMask, ss.back.writeMask);
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
    return initHandle<GLFence>();
}

Handle<HwSwapChain> OpenGLDriver::createSwapChainS() noexcept {
    return initHandle<GLSwapChain>();
}

Handle<HwSwapChain> OpenGLDriver::createSwapChainHeadlessS() noexcept {
    return initHandle<GLSwapChain>();
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
    uint8_t const elementSize = static_cast<uint8_t>(getElementTypeSize(elementType));
    GLIndexBuffer* ib = construct<GLIndexBuffer>(ibh, elementSize, indexCount);
    glGenBuffers(1, &ib->gl.buffer);
    GLsizeiptr const size = elementSize * indexCount;
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
    if (UTILS_UNLIKELY(bindingType == BufferObjectBinding::UNIFORM && gl.isES2())) {
        bo->gl.id = ++mLastAssignedEmulatedUboId;
        bo->gl.buffer = malloc(byteCount);
        memset(bo->gl.buffer, 0, byteCount);
    } else {
        bo->gl.binding = GLUtils::getBufferBindingType(bindingType);
        glGenBuffers(1, &bo->gl.id);
        gl.bindBuffer(bo->gl.binding, bo->gl.id);
        glBufferData(bo->gl.binding, byteCount, nullptr, getBufferUsage(usage));
    }

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createRenderPrimitiveR(Handle<HwRenderPrimitive> rph,
        Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh,
        PrimitiveType pt) {
    DEBUG_MARKER()

    auto& gl = mContext;

    GLVertexBuffer const* const eb = handle_cast<const GLVertexBuffer*>(vbh);
    GLIndexBuffer const* const ib = handle_cast<const GLIndexBuffer*>(ibh);
    assert_invariant(ib->elementSize == 2 || ib->elementSize == 4);

    GLRenderPrimitive* rp = handle_cast<GLRenderPrimitive*>(rph);
    rp->gl.indicesSize = (ib->elementSize == 4u) ? 4u : 2u;
    rp->gl.vertexBufferWithObjects = vbh;
    rp->type = pt;

    gl.procs.genVertexArrays(1, &rp->gl.vao);

    gl.bindVertexArray(&rp->gl);

    // update the VBO bindings in the VAO
    updateVertexArrayObject(rp, eb);

    // this records the index buffer into the currently bound VAO
    gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->gl.buffer);

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createProgramR(Handle<HwProgram> ph, Program&& program) {
    DEBUG_MARKER()


    if (UTILS_UNLIKELY(mContext.isES2())) {
        // Here we patch the specialization constants to enable or not the rec709 output
        // color space emulation in this program. Obviously, the backend shouldn't know about
        // specific spec-constants, so we need to handle failures gracefully. This cannot be
        // done at Material creation time because only the backend has access to
        // Platform.isSRGBSwapChainSupported().
        if (!mPlatform.isSRGBSwapChainSupported()) {
            auto& specializationConstants = program.getSpecializationConstants();
            auto pos = std::find_if(specializationConstants.begin(), specializationConstants.end(),
                    [](auto&& sc) {
                        // This constant must match
                        // ReservedSpecializationConstants::CONFIG_SRGB_SWAPCHAIN_EMULATION
                        // which we can't use here because it's defined in EngineEnums.h.
                        // (we're breaking layering here, but it's for the good cause).
                        return sc.id == 3;
                    });
            if (pos != specializationConstants.end()) {
                pos->value = true;
            }
        }
    }

    construct<OpenGLProgram>(ph, *this, std::move(program));
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createSamplerGroupR(Handle<HwSamplerGroup> sbh, uint32_t size,
        utils::FixedSizeString<32> debugName) {
    DEBUG_MARKER()

    construct<GLSamplerGroup>(sbh, size);
}

UTILS_NOINLINE
void OpenGLDriver::textureStorage(OpenGLDriver::GLTexture* t,
        uint32_t width, uint32_t height, uint32_t depth) noexcept {

    auto& gl = mContext;

    bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
    gl.activeTexture(OpenGLContext::DUMMY_TEXTURE_BINDING);

    switch (t->gl.target) {
        case GL_TEXTURE_2D:
        case GL_TEXTURE_CUBE_MAP:
            if (UTILS_LIKELY(!gl.isES2())) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
                glTexStorage2D(t->gl.target, GLsizei(t->levels), t->gl.internalFormat,
                        GLsizei(width), GLsizei(height));
#endif
            }
#ifdef BACKEND_OPENGL_VERSION_GLES
            else {
                // FIXME: handle compressed texture format
                auto [format, type] = textureFormatToFormatAndType(t->format);
                assert_invariant(format != GL_NONE && type != GL_NONE);
                for (GLint level = 0 ; level < t->levels ; level++ ) {
                    if (t->gl.target == GL_TEXTURE_CUBE_MAP) {
                        for (GLint face = 0 ; face < 6 ; face++) {
                            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                                    level, GLint(t->gl.internalFormat),
                                    GLsizei(width), GLsizei(height), 0,
                                    format, type, nullptr);
                        }
                    } else {
                        glTexImage2D(t->gl.target, level, GLint(t->gl.internalFormat),
                                std::max(GLsizei(1), GLsizei(width >> level)),
                                std::max(GLsizei(1), GLsizei(height >> level)),
                                0, format, type, nullptr);
                    }
                }
            }
#endif
            break;
        case GL_TEXTURE_3D:
        case GL_TEXTURE_2D_ARRAY: {
            assert_invariant(!gl.isES2());
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
            glTexStorage3D(t->gl.target, GLsizei(t->levels), t->gl.internalFormat,
                    GLsizei(width), GLsizei(height), GLsizei(depth));
#endif
            break;
        }
        case GL_TEXTURE_CUBE_MAP_ARRAY: {
            assert_invariant(!gl.isES2());
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
            glTexStorage3D(t->gl.target, GLsizei(t->levels), t->gl.internalFormat,
                    GLsizei(width), GLsizei(height), GLsizei(depth) * 6);
#endif
            break;
        }
#ifdef BACKEND_OPENGL_LEVEL_GLES31
        case GL_TEXTURE_2D_MULTISAMPLE:
            if constexpr (TEXTURE_2D_MULTISAMPLE_SUPPORTED) {
                // NOTE: if there is a mix of texture and renderbuffers, "fixed_sample_locations" must be true
                // NOTE: what's the benefit of setting "fixed_sample_locations" to false?

                if (mContext.isAtLeastGL<4, 3>() || mContext.isAtLeastGLES<3, 1>()) {
                    // only supported from GL 4.3 and GLES 3.1 headers
                    glTexStorage2DMultisample(t->gl.target, t->samples, t->gl.internalFormat,
                            GLsizei(width), GLsizei(height), GL_TRUE);
                }
#ifdef BACKEND_OPENGL_VERSION_GL
                else {
                    // only supported in GL (GL4.1 doesn't support glTexStorage2DMultisample)
                    glTexImage2DMultisample(t->gl.target, t->samples, t->gl.internalFormat,
                            GLsizei(width), GLsizei(height), GL_TRUE);
                }
#endif
            } else {
                PANIC_LOG("GL_TEXTURE_2D_MULTISAMPLE is not supported");
            }
            break;
#endif // BACKEND_OPENGL_LEVEL_GLES31
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

    GLenum internalFormat = getInternalFormat(format);
    assert_invariant(internalFormat);

    auto& gl = mContext;
    samples = std::clamp(samples, uint8_t(1u), uint8_t(gl.gets.max_samples));
    GLTexture* t = construct<GLTexture>(th, target, levels, samples, w, h, depth, format, usage);
    if (UTILS_LIKELY(usage & TextureUsage::SAMPLEABLE)) {

        if (UTILS_UNLIKELY(gl.isES2())) {
            // on ES2, format and internal format must match
            // FIXME: handle compressed texture format
            internalFormat = textureFormatToFormatAndType(format).first;
        }

        if (UTILS_UNLIKELY(t->target == SamplerType::SAMPLER_EXTERNAL)) {
            t->externalTexture = mPlatform.createExternalImageTexture();
            if (t->externalTexture) {
                t->gl.target = t->externalTexture->target;
                t->gl.id = t->externalTexture->id;
                t->gl.targetIndex = (uint8_t)OpenGLContext::getIndexForTextureTarget(t->gl.target);
                // internalFormat actually depends on the external image, but it doesn't matter
                // because it's not used anywhere for anything important.
                t->gl.internalFormat = internalFormat;
                t->gl.baseLevel = 0;
                t->gl.maxLevel = 0;
            }
        } else {
            glGenTextures(1, &t->gl.id);

            t->gl.internalFormat = internalFormat;

            // We DO NOT update targetIndex at function exit to take advantage of the fact that
            // getIndexForTextureTarget() is constexpr -- so all of this disappears at compile time.
            switch (target) {
                case SamplerType::SAMPLER_EXTERNAL:
                    // we can't be here -- doesn't matter what we do
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
                case SamplerType::SAMPLER_CUBEMAP_ARRAY:
                    t->gl.target = GL_TEXTURE_CUBE_MAP_ARRAY;
                    t->gl.targetIndex = (uint8_t)gl.getIndexForTextureTarget(GL_TEXTURE_CUBE_MAP_ARRAY);
                    break;
            }

            if (t->samples > 1) {
                // Note: we can't be here in practice because filament's user API doesn't
                // allow the creation of multi-sampled textures.
#if defined(BACKEND_OPENGL_LEVEL_GLES31)
                if (gl.features.multisample_texture) {
                    // multi-sample texture on GL 3.2 / GLES 3.1 and above
                    t->gl.target = GL_TEXTURE_2D_MULTISAMPLE;
                    t->gl.targetIndex = (uint8_t)
                            gl.getIndexForTextureTarget(GL_TEXTURE_2D_MULTISAMPLE);
                } else {
                    // Turn off multi-sampling for that texture. It's just not supported.
                }
#endif
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
        t->gl.internalFormat = internalFormat;
        t->gl.target = GL_RENDERBUFFER;
        glGenRenderbuffers(1, &t->gl.id);
        renderBufferStorage(t->gl.id, internalFormat, w, h, samples);
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
#if !defined(__EMSCRIPTEN__)  && !defined(FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2)
    if (!mContext.isES2()) {
        // the texture is still bound and active from createTextureR
        GLTexture* t = handle_cast<GLTexture*>(th);
        glTexParameteri(t->gl.target, GL_TEXTURE_SWIZZLE_R, (GLint)getSwizzleChannel(r));
        glTexParameteri(t->gl.target, GL_TEXTURE_SWIZZLE_G, (GLint)getSwizzleChannel(g));
        glTexParameteri(t->gl.target, GL_TEXTURE_SWIZZLE_B, (GLint)getSwizzleChannel(b));
        glTexParameteri(t->gl.target, GL_TEXTURE_SWIZZLE_A, (GLint)getSwizzleChannel(a));
    }
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
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            t->gl.target = GL_TEXTURE_CUBE_MAP_ARRAY;
            t->gl.targetIndex = (uint8_t)gl.getIndexForTextureTarget(GL_TEXTURE_CUBE_MAP_ARRAY);
            break;
    }

    if (t->samples > 1) {
        // Note: we can't be here in practice because filament's user API doesn't
        // allow the creation of multi-sampled textures.
#if defined(BACKEND_OPENGL_LEVEL_GLES31)
        if (gl.features.multisample_texture) {
            // multi-sample texture on GL 3.2 / GLES 3.1 and above
            t->gl.target = GL_TEXTURE_2D_MULTISAMPLE;
            t->gl.targetIndex = (uint8_t)gl.getIndexForTextureTarget(GL_TEXTURE_2D_MULTISAMPLE);
        } else {
            // Turn off multi-sampling for that texture. It's just not supported.
        }
#endif
    }

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::updateVertexArrayObject(GLRenderPrimitive* rp, GLVertexBuffer const* vb) {

    auto& gl = mContext;

    // NOTE: this is called from draw() and must be as efficient as possible.


    if (UTILS_LIKELY(gl.ext.OES_vertex_array_object)) {
        // The VAO for the given render primitive must already be bound.
#ifndef NDEBUG
        GLint vaoBinding;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vaoBinding);
        assert_invariant(vaoBinding == (GLint)rp->gl.vao);
#endif
        rp->gl.vertexBufferVersion = vb->bufferObjectsVersion;
    } else {
        // if we don't have OES_vertex_array_object, we never update the buffer version so
        // that it's always reset in draw
    }

    for (size_t i = 0, n = vb->attributes.size(); i < n; i++) {
        const auto& attribute = vb->attributes[i];
        const uint8_t bi = attribute.buffer;

        // Invoking glVertexAttribPointer without a bound VBO is an invalid operation, so we must
        // take care to avoid it. This can occur when VertexBuffer is only partially populated with
        // BufferObject items.
        if (bi != Attribute::BUFFER_UNUSED && UTILS_LIKELY(vb->gl.buffers[bi] != 0)) {

            assert_invariant(!(gl.isES2() && (attribute.flags & Attribute::FLAG_INTEGER_TARGET)));

            gl.bindBuffer(GL_ARRAY_BUFFER, vb->gl.buffers[bi]);
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
            if (UTILS_UNLIKELY(attribute.flags & Attribute::FLAG_INTEGER_TARGET)) {
                glVertexAttribIPointer(GLuint(i),
                        (GLint)getComponentCount(attribute.type),
                        getComponentType(attribute.type),
                        attribute.stride,
                        (void*) uintptr_t(attribute.offset));
            } else
#endif
            {
                glVertexAttribPointer(GLuint(i),
                        (GLint)getComponentCount(attribute.type),
                        getComponentType(attribute.type),
                        getNormalization(attribute.flags & Attribute::FLAG_NORMALIZED),
                        attribute.stride,
                        (void*) uintptr_t(attribute.offset));
            }

            gl.enableVertexAttribArray(GLuint(i));
        } else {

            // In some OpenGL implementations, we must supply a properly-typed placeholder for
            // every integer input that is declared in the vertex shader, even if disabled.
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
            if (UTILS_UNLIKELY(attribute.flags & Attribute::FLAG_INTEGER_TARGET)) {
                if (!gl.isES2()) {
                    // on ES2, we know the shader doesn't have integer attributes
                    glVertexAttribI4ui(GLuint(i), 0, 0, 0, 0);
                }
            } else
#endif
            {
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

    assert_invariant(t);
    assert_invariant(t->target != SamplerType::SAMPLER_EXTERNAL);
    assert_invariant(rt->width  <= valueForLevel(binfo.level, t->width) &&
           rt->height <= valueForLevel(binfo.level, t->height));

    // Declare a small mask of bits that will later be OR'd into the texture's resolve mask.
    TargetBufferFlags resolveFlags = {};

    switch (attachment) {
        case GL_COLOR_ATTACHMENT0:
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        case GL_COLOR_ATTACHMENT1:
        case GL_COLOR_ATTACHMENT2:
        case GL_COLOR_ATTACHMENT3:
        case GL_COLOR_ATTACHMENT4:
        case GL_COLOR_ATTACHMENT5:
        case GL_COLOR_ATTACHMENT6:
        case GL_COLOR_ATTACHMENT7:
#endif
            assert_invariant((attachment != GL_COLOR_ATTACHMENT0 && !mContext.isES2())
                             || attachment == GL_COLOR_ATTACHMENT0);

            static_assert(MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT == 8);

            resolveFlags = getTargetBufferFlagsAt(attachment - GL_COLOR_ATTACHMENT0);
            break;
        case GL_DEPTH_ATTACHMENT:
            resolveFlags = TargetBufferFlags::DEPTH;
            break;
        case GL_STENCIL_ATTACHMENT:
            resolveFlags = TargetBufferFlags::STENCIL;
            break;
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        case GL_DEPTH_STENCIL_ATTACHMENT:
            assert_invariant(!mContext.isES2());
            resolveFlags = TargetBufferFlags::DEPTH;
            resolveFlags |= TargetBufferFlags::STENCIL;
            break;
#endif
        default:
            break;
    }

    // depth/stencil attachments must match the rendertarget sample count
    // because EXT_multisampled_render_to_texture[2] doesn't resolve the depth/stencil
    // buffers:
    // for EXT_multisampled_render_to_texture
    //      "the contents of the multisample buffer become undefined"
    // for EXT_multisampled_render_to_texture2
    //      "the contents of the multisample buffer is discarded rather than resolved -
    //       equivalent to the application calling InvalidateFramebuffer for this attachment"
    UTILS_UNUSED bool attachmentTypeNotSupportedByMSRTT = false;
    switch (attachment) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        case GL_DEPTH_STENCIL_ATTACHMENT:
            assert_invariant(!mContext.isES2());
            UTILS_FALLTHROUGH;
#endif
        case GL_DEPTH_ATTACHMENT:
        case GL_STENCIL_ATTACHMENT:
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
            case SamplerType::SAMPLER_CUBEMAP_ARRAY:
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

    // We can't use FramebufferTexture2DMultisampleEXT with GL_TEXTURE_2D_ARRAY or GL_TEXTURE_CUBE_MAP_ARRAY.
    if (!(target == GL_TEXTURE_2D ||
          target == GL_TEXTURE_CUBE_MAP_POSITIVE_X ||
          target == GL_TEXTURE_CUBE_MAP_NEGATIVE_X ||
          target == GL_TEXTURE_CUBE_MAP_POSITIVE_Y ||
          target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y ||
          target == GL_TEXTURE_CUBE_MAP_POSITIVE_Z ||
          target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)) {
        attachmentTypeNotSupportedByMSRTT = true;
    }

    if (rt->gl.samples <= 1 ||
        (rt->gl.samples > 1 && t->samples > 1 && gl.features.multisample_texture)) {
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
#if defined(BACKEND_OPENGL_LEVEL_GLES31)
            case GL_TEXTURE_2D_MULTISAMPLE:
#endif
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
            case GL_TEXTURE_CUBE_MAP_ARRAY:
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
                // GL_TEXTURE_2D_MULTISAMPLE_ARRAY is not supported in GLES
                glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment,
                        t->gl.id, binfo.level, binfo.layer);
#endif
                break;
            default:
                // we shouldn't be here
                break;
        }
        CHECK_GL_ERROR(utils::slog.e)
    } else
#ifndef __EMSCRIPTEN__
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
            glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER,
                    attachment, target, t->gl.id, binfo.level, rt->gl.samples);
        } else {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment,
                    GL_RENDERBUFFER, t->gl.id);
        }
        CHECK_GL_ERROR(utils::slog.e)
    } else
#endif // GL_EXT_multisampled_render_to_texture
#endif // __EMSCRIPTEN__
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
            case GL_TEXTURE_CUBE_MAP_ARRAY:
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
                glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment,
                        t->gl.id, binfo.level, binfo.layer);
#endif
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
        updateTextureLodRange(t, (int8_t)binfo.level);
    }

    CHECK_GL_ERROR(utils::slog.e)
    CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e, GL_FRAMEBUFFER)
}

void OpenGLDriver::renderBufferStorage(GLuint rbo, GLenum internalformat, uint32_t width, // NOLINT(readability-convert-member-functions-to-static)
        uint32_t height, uint8_t samples) const noexcept {
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    if (samples > 1) {
#ifndef __EMSCRIPTEN__
#ifdef GL_EXT_multisampled_render_to_texture
        auto& gl = mContext;
        if (gl.ext.EXT_multisampled_render_to_texture ||
            gl.ext.EXT_multisampled_render_to_texture2) {
            glext::glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER,
                    samples, internalformat, width, height);
        } else
#endif // GL_EXT_multisampled_render_to_texture
#endif // __EMSCRIPTEN__
        {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
            glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                    samples, internalformat, (GLsizei)width, (GLsizei)height);
#endif
        }
    } else {
        glRenderbufferStorage(GL_RENDERBUFFER, internalformat, (GLsizei)width, (GLsizei)height);
    }
    // unbind the renderbuffer, to avoid any later confusion
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createDefaultRenderTargetR(
        Handle<HwRenderTarget> rth, int) {
    DEBUG_MARKER()

    construct<GLRenderTarget>(rth, 0, 0);  // FIXME: we don't know the width/height

    uint32_t const framebuffer = mPlatform.createDefaultRenderTarget();

    GLRenderTarget* rt = handle_cast<GLRenderTarget*>(rth);
    rt->gl.isDefault = true;
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
     *  within the largest area that can fit in all the attachments. This area is defined as
     *  the intersection of rectangles having a lower left of (0, 0) and an upper right of
     *  (width, height) for each attachment. Contents of attachments outside this area are
     *  undefined after execution of a rendering command.
     */

    samples = std::clamp(samples, uint8_t(1u), uint8_t(mContext.gets.max_samples));

    rt->gl.samples = samples;
    rt->targets = targets;

    UTILS_UNUSED_IN_RELEASE math::vec2<uint32_t> tmin = { std::numeric_limits<uint32_t>::max() };
    UTILS_UNUSED_IN_RELEASE math::vec2<uint32_t> tmax = { 0 };
    auto checkDimensions = [&tmin, &tmax](GLTexture* t, uint8_t level) {
        const auto twidth = std::max(1u, t->width >> level);
        const auto theight = std::max(1u, t->height >> level);
        tmin = { std::min(tmin.x, twidth), std::min(tmin.y, theight) };
        tmax = { std::max(tmax.x, twidth), std::max(tmax.y, theight) };
    };


#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    if (any(targets & TargetBufferFlags::COLOR_ALL)) {
        GLenum bufs[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = { GL_NONE };
        const size_t maxDrawBuffers = getMaxDrawBuffers();
        for (size_t i = 0; i < maxDrawBuffers; i++) {
            if (any(targets & getTargetBufferFlagsAt(i))) {
                assert_invariant(color[i].handle);
                rt->gl.color[i] = handle_cast<GLTexture*>(color[i].handle);
                framebufferTexture(color[i], rt, GL_COLOR_ATTACHMENT0 + i);
                bufs[i] = GL_COLOR_ATTACHMENT0 + i;
                checkDimensions(rt->gl.color[i], color[i].level);
            }
        }
        if (UTILS_LIKELY(!getContext().isES2())) {
            glDrawBuffers((GLsizei)maxDrawBuffers, bufs);
        }
        CHECK_GL_ERROR(utils::slog.e)
    }
#endif

    // handle special cases first (where depth/stencil are packed)
    bool specialCased = false;

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    if (!getContext().isES2() &&
            (targets & TargetBufferFlags::DEPTH_AND_STENCIL) == TargetBufferFlags::DEPTH_AND_STENCIL) {
        assert_invariant(depth.handle);
        // either we supplied only the depth handle or both depth/stencil are identical and not null
        if (depth.handle && (stencil.handle == depth.handle || !stencil.handle)) {
            rt->gl.depth = handle_cast<GLTexture*>(depth.handle);
            framebufferTexture(depth, rt, GL_DEPTH_STENCIL_ATTACHMENT);
            specialCased = true;
            checkDimensions(rt->gl.depth, depth.level);
        }
    }
#endif

    if (!specialCased) {
        if (any(targets & TargetBufferFlags::DEPTH)) {
            assert_invariant(depth.handle);
            rt->gl.depth = handle_cast<GLTexture*>(depth.handle);
            framebufferTexture(depth, rt, GL_DEPTH_ATTACHMENT);
            checkDimensions(rt->gl.depth, depth.level);
        }
        if (any(targets & TargetBufferFlags::STENCIL)) {
            assert_invariant(stencil.handle);
            rt->gl.stencil = handle_cast<GLTexture*>(stencil.handle);
            framebufferTexture(stencil, rt, GL_STENCIL_ATTACHMENT);
            checkDimensions(rt->gl.stencil, stencil.level);
        }
    }

    // Verify that all attachments have the same dimensions.
    assert_invariant(any(targets & TargetBufferFlags::ALL));
    assert_invariant(tmin == tmax);

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createFenceR(Handle<HwFence> fh, int) {
    DEBUG_MARKER()

    GLFence* f = handle_cast<GLFence*>(fh);

    if (mPlatform.canCreateFence() || mContext.isES2()) {
        assert_invariant(mPlatform.canCreateFence());
        f->fence = mPlatform.createFence();
    }
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    else {
        std::weak_ptr<GLFence::State> const weak = f->state;
        whenGpuCommandsComplete([weak](){
            auto state = weak.lock();
            if (state) {
                std::lock_guard const lock(state->lock);
                state->status = FenceStatus::CONDITION_SATISFIED;
                state->cond.notify_all();
            }
        });
    }
#endif
}

void OpenGLDriver::createSwapChainR(Handle<HwSwapChain> sch, void* nativeWindow, uint64_t flags) {
    DEBUG_MARKER()

    GLSwapChain* sc = handle_cast<GLSwapChain*>(sch);
    sc->swapChain = mPlatform.createSwapChain(nativeWindow, flags);

    // See if we need the emulated rec709 output conversion
    if (UTILS_UNLIKELY(mContext.isES2())) {
        sc->rec709 = (flags & SWAP_CHAIN_CONFIG_SRGB_COLORSPACE &&
                !mPlatform.isSRGBSwapChainSupported());
    }
}

void OpenGLDriver::createSwapChainHeadlessR(Handle<HwSwapChain> sch,
        uint32_t width, uint32_t height, uint64_t flags) {
    DEBUG_MARKER()

    GLSwapChain* sc = handle_cast<GLSwapChain*>(sch);
    sc->swapChain = mPlatform.createSwapChain(width, height, flags);

    // See if we need the emulated rec709 output conversion
    if (UTILS_UNLIKELY(mContext.isES2())) {
        sc->rec709 = (flags & SWAP_CHAIN_CONFIG_SRGB_COLORSPACE &&
                      !mPlatform.isSRGBSwapChainSupported());
    }
}

void OpenGLDriver::createTimerQueryR(Handle<HwTimerQuery> tqh, int) {
    DEBUG_MARKER()
    GLTimerQuery* tq = handle_cast<GLTimerQuery*>(tqh);
    mTimerQueryImpl->createTimerQuery(tq);
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
        if (UTILS_UNLIKELY(bo->bindingType == BufferObjectBinding::UNIFORM && gl.isES2())) {
            free(bo->gl.buffer);
        } else {
            gl.deleteBuffers(1, &bo->gl.id, bo->gl.binding);
        }
        destruct(boh, bo);
    }
}

void OpenGLDriver::destroyRenderPrimitive(Handle<HwRenderPrimitive> rph) {
    DEBUG_MARKER()

    if (rph) {
        auto& gl = mContext;
        GLRenderPrimitive const* rp = handle_cast<const GLRenderPrimitive*>(rph);
        gl.deleteVertexArrays(1, &rp->gl.vao);
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
        for (auto& binding : mSamplerBindings) {
            if (binding == sb) {
                binding = nullptr;
            }
        }
        destruct(sbh, sb);
    }
}

void OpenGLDriver::destroyTexture(Handle<HwTexture> th) {
    DEBUG_MARKER()

    if (th) {
        auto& gl = mContext;
        GLTexture* t = handle_cast<GLTexture*>(th);
        if (UTILS_LIKELY(!t->gl.imported)) {
            if (UTILS_LIKELY(t->usage & TextureUsage::SAMPLEABLE)) {
                gl.unbindTexture(t->gl.target, t->gl.id);
                if (UTILS_UNLIKELY(t->hwStream)) {
                    detachStream(t);
                }
                if (UTILS_UNLIKELY(t->target == SamplerType::SAMPLER_EXTERNAL)) {
                    mPlatform.destroyExternalImage(t->externalTexture);
                } else {
                    glDeleteTextures(1, &t->gl.id);
                }
            } else {
                assert_invariant(t->gl.target == GL_RENDERBUFFER);
                glDeleteRenderbuffers(1, &t->gl.id);
            }
            if (t->gl.sidecarRenderBufferMS) {
                glDeleteRenderbuffers(1, &t->gl.sidecarRenderBufferMS);
            }
        } else {
            gl.unbindTexture(t->gl.target, t->gl.id);
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
        }
        if (rt->gl.fbo_read) {
            // first unbind this framebuffer if needed
            gl.bindFramebuffer(GL_FRAMEBUFFER, 0);
        }

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        if (UTILS_UNLIKELY(gl.bugs.delay_fbo_destruction)) {
            if (rt->gl.fbo) {
                whenFrameComplete([fbo = rt->gl.fbo]() {
                    glDeleteFramebuffers(1, &fbo);
                });
            }
            if (rt->gl.fbo_read) {
                whenFrameComplete([fbo_read = rt->gl.fbo_read]() {
                    glDeleteFramebuffers(1, &fbo_read);
                });
            }
        } else
#endif
        {
            if (rt->gl.fbo) {
                glDeleteFramebuffers(1, &rt->gl.fbo);
            }
            if (rt->gl.fbo_read) {
                glDeleteFramebuffers(1, &rt->gl.fbo_read);
            }
        }
        destruct(rth, rt);
    }
}

void OpenGLDriver::destroySwapChain(Handle<HwSwapChain> sch) {
    DEBUG_MARKER()

    if (sch) {
        GLSwapChain* sc = handle_cast<GLSwapChain*>(sch);
        mPlatform.destroySwapChain(sc->swapChain);
        destruct(sch, sc);
    }
}

void OpenGLDriver::destroyStream(Handle<HwStream> sh) {
    DEBUG_MARKER()

    if (sh) {
        GLStream* s = handle_cast<GLStream*>(sh);

        // if this stream is still attached to a texture, detach it first
        auto& texturesWithStreamsAttached = mTexturesWithStreamsAttached;
        auto pos = std::find_if(
                texturesWithStreamsAttached.begin(), texturesWithStreamsAttached.end(),
                [s](GLTexture const* t) {
                    return t->hwStream == s;
                });

        if (pos != texturesWithStreamsAttached.end()) {
            detachStream(*pos);
        }

        // and then destroy the stream. Only NATIVE streams have Platform::Stream associated.
        if (s->streamType == StreamType::NATIVE) {
            mPlatform.destroyStream(s->stream);
        }

        // finally destroy the HwStream handle
        destruct(sh, s);
    }
}

void OpenGLDriver::destroyTimerQuery(Handle<HwTimerQuery> tqh) {
    DEBUG_MARKER()

    if (tqh) {
        GLTimerQuery* tq = handle_cast<GLTimerQuery*>(tqh);
        mTimerQueryImpl->destroyTimerQuery(tq);
        destruct(tqh, tq);
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
    assert_invariant(glstream->streamType == StreamType::ACQUIRED);

    if (UTILS_UNLIKELY(glstream->user_thread.pending.image)) {
        scheduleRelease(glstream->user_thread.pending);
        slog.w << "Acquired image is set more than once per frame." << io::endl;
    }

    glstream->user_thread.pending = mPlatform.transformAcquiredImage({
            hwbuffer, cb, userData, handler });

    if (glstream->user_thread.pending.image != nullptr) {
        // If there's no pending image, do nothing. Note that GL_OES_EGL_image does not let you pass
        // NULL to glEGLImageTargetTexture2DOES, and there is no concept of "detaching" an
        // EGLimage from a texture.
        mStreamsWithPendingAcquiredImage.push_back(glstream);
    }
}

// updateStreams() and setAcquiredImage() are both called from on the application's thread
// and therefore do not require synchronization. The former is always called immediately before
// beginFrame, the latter is called by the user from anywhere outside beginFrame / endFrame.
void OpenGLDriver::updateStreams(DriverApi* driver) {
    if (UTILS_UNLIKELY(!mStreamsWithPendingAcquiredImage.empty())) {
        for (GLStream* s : mStreamsWithPendingAcquiredImage) {
            assert_invariant(s);
            assert_invariant(s->streamType == StreamType::ACQUIRED);

            AcquiredImage const previousImage = s->user_thread.acquired;
            s->user_thread.acquired = s->user_thread.pending;
            s->user_thread.pending = { nullptr };

            // Bind the stashed EGLImage to its corresponding GL texture as soon as we start
            // making the GL calls for the upcoming frame.
            driver->queueCommand([this, s, image = s->user_thread.acquired.image, previousImage]() {

                auto& streams = mTexturesWithStreamsAttached;
                auto pos = std::find_if(streams.begin(), streams.end(),
                        [s](GLTexture const* t) {
                            return t->hwStream == s;
                        });
                if (pos != streams.end()) {
                    GLTexture* t = *pos;
                    bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
                    if (mPlatform.setExternalImage(image, t->externalTexture)) {
                        // the target and id can be reset each time
                        t->gl.target = t->externalTexture->target;
                        t->gl.id = t->externalTexture->id;
                        t->gl.targetIndex = (uint8_t)OpenGLContext::getIndexForTextureTarget(t->gl.target);
                        bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
                    }
                }

                if (previousImage.image) {
                    scheduleRelease(AcquiredImage(previousImage));
                }
            });
        }
        mStreamsWithPendingAcquiredImage.clear();
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
        GLFence* f = handle_cast<GLFence*>(fh);
        if (mPlatform.canCreateFence() || mContext.isES2()) {
            mPlatform.destroyFence(f->fence);
        }
        destruct(fh, f);
    }
}

FenceStatus OpenGLDriver::getFenceStatus(Handle<HwFence> fh) {
    if (fh) {
        GLFence* f = handle_cast<GLFence*>(fh);
        if (mPlatform.canCreateFence() || mContext.isES2()) {
            if (f->fence == nullptr) {
                // we can end-up here if:
                // - the platform doesn't support h/w fences
                if (UTILS_UNLIKELY(!mPlatform.canCreateFence())) {
                    return FenceStatus::ERROR;
                }
                // - wait() was called before the fence was asynchronously created.
                return FenceStatus::TIMEOUT_EXPIRED;
            }
            return mPlatform.waitFence(f->fence, 0);
        }
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        else {
            assert_invariant(f->state);
            std::unique_lock lock(f->state->lock);
            f->state->cond.wait_for(lock, std::chrono::nanoseconds(0), [&state = f->state]() {
                return state->status != FenceStatus::TIMEOUT_EXPIRED;
            });
            return f->state->status;
        }
#endif
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
    if (isRGTCCompression(format)) {
        return  ext.EXT_texture_compression_rgtc;
    }
    if (isBPTCCompression(format)) {
        return  ext.EXT_texture_compression_bptc;
    }
    if (isASTCCompression(format)) {
        return ext.KHR_texture_compression_astc_hdr;
    }
    if (mContext.isES2()) {
        return textureFormatToFormatAndType(format).first != GL_NONE;
    }
    return getInternalFormat(format) != 0;
}

bool OpenGLDriver::isTextureSwizzleSupported() {
#if defined(__EMSCRIPTEN__)
    // WebGL2 doesn't support texture swizzle
    // see https://registry.khronos.org/webgl/specs/latest/2.0/#5.19
    return false;
#elif defined(BACKEND_OPENGL_VERSION_GLES)
    return !mContext.isES2();
#else
    return true;
#endif
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
    if (UTILS_UNLIKELY(gl.isES2())) {
        auto [es2format, type] = textureFormatToFormatAndType(format);
        return es2format != GL_NONE && type != GL_NONE;
    }
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
            return mContext.isAtLeastGL<4, 5>();

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
    return OpenGLTimerQueryFactory::isGpuTimeSupported();
}

bool OpenGLDriver::isAutoDepthResolveSupported() {
    // TODO: this should return true only for GLES3.1+ and EXT_multisampled_render_to_texture2
    return true;
}

bool OpenGLDriver::isSRGBSwapChainSupported() {
    if (UTILS_UNLIKELY(mContext.isES2())) {
        // On ES2 backend (i.e. feature level 0), we always pretend to the client that sRGB
        // SwapChain are available. If we actually have that feature, it'll be used, otherwise
        // we emulate it in the shaders.
        return true;
    }
    return mPlatform.isSRGBSwapChainSupported();
}

bool OpenGLDriver::isStereoSupported(backend::StereoscopicType stereoscopicType) {
    // Instanced-stereo requires instancing and EXT_clip_cull_distance.
    // Multiview-stereo requires ES 3.0 and OVR_multiview2.
    if (UTILS_UNLIKELY(mContext.isES2())) {
        return false;
    }
    switch (stereoscopicType) {
    case backend::StereoscopicType::INSTANCED:
        return mContext.ext.EXT_clip_cull_distance;
    case backend::StereoscopicType::MULTIVIEW:
        return mContext.ext.OVR_multiview2;
    default:
        return false;
    }
}

bool OpenGLDriver::isParallelShaderCompileSupported() {
    return mShaderCompilerService.isParallelShaderCompileSupported();
}

bool OpenGLDriver::isDepthStencilResolveSupported() {
    return true;
}

bool OpenGLDriver::isWorkaroundNeeded(Workaround workaround) {
    switch (workaround) {
        case Workaround::SPLIT_EASU:
            return mContext.bugs.split_easu;
        case Workaround::ALLOW_READ_ONLY_ANCILLARY_FEEDBACK_LOOP:
            return mContext.bugs.allow_read_only_ancillary_feedback_loop;
        case Workaround::ADRENO_UNIFORM_ARRAY_CRASH:
            return mContext.bugs.enable_initialize_non_used_uniform_array;
        case Workaround::DISABLE_BLIT_INTO_TEXTURE_ARRAY:
            return mContext.bugs.disable_blit_into_texture_array;
        case Workaround::POWER_VR_SHADER_WORKAROUNDS:
            return mContext.bugs.powervr_shader_workarounds;
        case Workaround::DISABLE_THREAD_AFFINITY:
            return mContext.bugs.disable_thread_affinity;
        default:
            return false;
    }
    return false;
}

FeatureLevel OpenGLDriver::getFeatureLevel() {
    return mContext.getFeatureLevel();
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

size_t OpenGLDriver::getMaxUniformBufferSize() {
    return mContext.gets.max_uniform_block_size;
}

// ------------------------------------------------------------------------------------------------
// Swap chains
// ------------------------------------------------------------------------------------------------


void OpenGLDriver::commit(Handle<HwSwapChain> sch) {
    DEBUG_MARKER()

    GLSwapChain* sc = handle_cast<GLSwapChain*>(sch);
    mPlatform.commit(sc->swapChain);

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    if (UTILS_UNLIKELY(!mFrameCompleteOps.empty())) {
        whenGpuCommandsComplete([ops = std::move(mFrameCompleteOps)]() {
            for (auto&& op: ops) {
                op();
            }
        });
    }
#endif
}

void OpenGLDriver::makeCurrent(Handle<HwSwapChain> schDraw, Handle<HwSwapChain> schRead) {
    DEBUG_MARKER()

    GLSwapChain* scDraw = handle_cast<GLSwapChain*>(schDraw);
    GLSwapChain* scRead = handle_cast<GLSwapChain*>(schRead);
    mPlatform.makeCurrent(scDraw->swapChain, scRead->swapChain);
    mCurrentDrawSwapChain = scDraw;

    // From the GL spec for glViewport and glScissor:
    // When a GL context is first attached to a window, width and height are set to the
    // dimensions of that window.
    // So basically, our viewport/scissor can be reset to "something" here.
    mContext.state.window.viewport = {};
    mContext.state.window.scissor = {};
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
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, byteOffset, (GLsizeiptr)p.size, p.buffer);

    scheduleDestroy(std::move(p));

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::updateBufferObject(
        Handle<HwBufferObject> boh, BufferDescriptor&& bd, uint32_t byteOffset) {
    DEBUG_MARKER()

    auto& gl = mContext;
    GLBufferObject* bo = handle_cast<GLBufferObject *>(boh);

    assert_invariant(bd.size + byteOffset <= bo->byteCount);

    if (bo->gl.binding == GL_ARRAY_BUFFER) {
        gl.bindVertexArray(nullptr);
    }

    if (UTILS_UNLIKELY(bo->bindingType == BufferObjectBinding::UNIFORM && gl.isES2())) {
        assert_invariant(bo->gl.buffer);
        memcpy(static_cast<uint8_t*>(bo->gl.buffer) + byteOffset, bd.buffer, bd.size);
        bo->age++;
    } else {
        assert_invariant(bo->gl.id);
        gl.bindBuffer(bo->gl.binding, bo->gl.id);
        if (byteOffset == 0 && bd.size == bo->byteCount) {
            // it looks like it's generally faster (or not worse) to use glBufferData()
            glBufferData(bo->gl.binding, (GLsizeiptr)bd.size, bd.buffer, getBufferUsage(bo->usage));
        } else {
            // glBufferSubData() could be catastrophically inefficient if several are
            // issued during the same frame. Currently, we're not doing that though.
            glBufferSubData(bo->gl.binding, byteOffset, (GLsizeiptr)bd.size, bd.buffer);
        }
    }

    scheduleDestroy(std::move(bd));

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::updateBufferObjectUnsynchronized(
        Handle<HwBufferObject> boh, BufferDescriptor&& bd, uint32_t byteOffset) {
    DEBUG_MARKER()

    if (UTILS_UNLIKELY(mContext.isES2())) {
        updateBufferObject(boh, std::move(bd), byteOffset);
        return;
    }

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    if constexpr (!HAS_MAPBUFFERS) {
        updateBufferObject(boh, std::move(bd), byteOffset);
    } else {
        GLBufferObject* bo = handle_cast<GLBufferObject*>(boh);

        assert_invariant(bo->gl.id);
        assert_invariant(bd.size + byteOffset <= bo->byteCount);

        if (bo->gl.binding != GL_UNIFORM_BUFFER) {
            // TODO: use updateBuffer() for all types of buffer? Make sure GL supports that.
            updateBufferObject(boh, std::move(bd), byteOffset);
        } else {
            auto& gl = mContext;
            gl.bindBuffer(bo->gl.binding, bo->gl.id);
retry:
            void* const vaddr = glMapBufferRange(bo->gl.binding, byteOffset, (GLsizeiptr)bd.size,
                    GL_MAP_WRITE_BIT |
                    GL_MAP_INVALIDATE_RANGE_BIT |
                    GL_MAP_UNSYNCHRONIZED_BIT);
            if (UTILS_LIKELY(vaddr)) {
                memcpy(vaddr, bd.buffer, bd.size);
                if (UTILS_UNLIKELY(glUnmapBuffer(bo->gl.binding) == GL_FALSE)) {
                    // According to the spec, UnmapBuffer can return FALSE in rare conditions (e.g.
                    // during a screen mode change). Note that this is not a GL error, and we can handle
                    // it by simply making a second attempt.
                    goto retry; // NOLINT(cppcoreguidelines-avoid-goto,hicpp-avoid-goto)
                }
            } else {
                // handle mapping error, revert to glBufferSubData()
                glBufferSubData(bo->gl.binding, byteOffset, (GLsizeiptr)bd.size, bd.buffer);
            }
            scheduleDestroy(std::move(bd));
        }
    }
    CHECK_GL_ERROR(utils::slog.e)
#endif
}

void OpenGLDriver::resetBufferObject(Handle<HwBufferObject> boh) {
    DEBUG_MARKER()

    auto& gl = mContext;
    GLBufferObject* bo = handle_cast<GLBufferObject*>(boh);

    if (UTILS_UNLIKELY(bo->bindingType == BufferObjectBinding::UNIFORM && gl.isES2())) {
        // nothing to do here
    } else {
        assert_invariant(bo->gl.id);
        gl.bindBuffer(bo->gl.binding, bo->gl.id);
        glBufferData(bo->gl.binding, bo->byteCount, nullptr, getBufferUsage(bo->usage));
    }
}

void OpenGLDriver::updateSamplerGroup(Handle<HwSamplerGroup> sbh,
        BufferDescriptor&& data) {
    DEBUG_MARKER()

    OpenGLContext const& context = getContext();

#if defined(GL_EXT_texture_filter_anisotropic)
    const bool anisotropyWorkaround =
            context.ext.EXT_texture_filter_anisotropic &&
            context.bugs.texture_filter_anisotropic_broken_on_sampler;
#endif

    GLSamplerGroup* const sb = handle_cast<GLSamplerGroup *>(sbh);
    assert_invariant(sb->textureUnitEntries.size() == data.size / sizeof(SamplerDescriptor));

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    bool const es2 = context.isES2();
#endif

    auto const* const pSamplers = (SamplerDescriptor const*)data.buffer;
    for (size_t i = 0, c = sb->textureUnitEntries.size(); i < c; i++) {
        GLuint samplerId = 0u;
        const GLTexture* t = nullptr;
        if (UTILS_LIKELY(pSamplers[i].t)) {
            t = handle_cast<const GLTexture*>(pSamplers[i].t);
            assert_invariant(t);

            SamplerParams params = pSamplers[i].s;
            if (UTILS_UNLIKELY(t->target == SamplerType::SAMPLER_EXTERNAL)) {
                // From OES_EGL_image_external spec:
                // "The default s and t wrap modes are CLAMP_TO_EDGE, and it is an INVALID_ENUM
                //  error to set the wrap mode to any other value."
                params.wrapS = SamplerWrapMode::CLAMP_TO_EDGE;
                params.wrapT = SamplerWrapMode::CLAMP_TO_EDGE;
                params.wrapR = SamplerWrapMode::CLAMP_TO_EDGE;
            }
            // GLES3.x specification forbids depth textures to be filtered.
            if (UTILS_UNLIKELY(isDepthFormat(t->format)
                               && params.compareMode == SamplerCompareMode::NONE
                               && params.filterMag != SamplerMagFilter::NEAREST
                               && params.filterMin != SamplerMinFilter::NEAREST
                               && params.filterMin != SamplerMinFilter::NEAREST_MIPMAP_NEAREST)) {
                params.filterMag = SamplerMagFilter::NEAREST;
                params.filterMin = SamplerMinFilter::NEAREST;
#ifndef NDEBUG
                slog.w << "HwSamplerGroup specifies a filtered depth texture, which is not allowed."
                       << io::endl;
#endif
            }
#if defined(GL_EXT_texture_filter_anisotropic)
            if (UTILS_UNLIKELY(anisotropyWorkaround)) {
                // Driver claims to support anisotropic filtering, but it fails when set on
                // the sampler, we have to set it on the texture instead.
                // The texture is already bound here.
                GLfloat const anisotropy = float(1u << params.anisotropyLog2);
                glTexParameterf(t->gl.target, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                        std::min(context.gets.max_anisotropy, anisotropy));
            }
#endif
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
            if (UTILS_LIKELY(!es2)) {
                samplerId = getSampler(params);
            } else
#endif
            {
                // in ES2 the sampler parameters need to be set on the texture itself
                glTexParameteri(t->gl.target, GL_TEXTURE_MIN_FILTER,
                        (GLint)getTextureFilter(params.filterMin));
                glTexParameteri(t->gl.target, GL_TEXTURE_MAG_FILTER,
                        (GLint)getTextureFilter(params.filterMag));
                glTexParameteri(t->gl.target, GL_TEXTURE_WRAP_S,
                        (GLint)getWrapMode(params.wrapS));
                glTexParameteri(t->gl.target, GL_TEXTURE_WRAP_T,
                        (GLint)getWrapMode(params.wrapT));
            }
        } else {
            // this happens if the program doesn't use all samplers of a sampler group,
            // which is not an error.
        }

        sb->textureUnitEntries[i] = { t, samplerId };
    }
    scheduleDestroy(std::move(data));
}

void OpenGLDriver::setMinMaxLevels(Handle<HwTexture> th, uint32_t minLevel, uint32_t maxLevel) {
    DEBUG_MARKER()

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    auto& gl = mContext;
    if (!gl.isES2()) {
        GLTexture* t = handle_cast<GLTexture*>(th);
        bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
        gl.activeTexture(OpenGLContext::DUMMY_TEXTURE_BINDING);

        // Must fit within int8_t.
        assert_invariant(minLevel <= 0x7f && maxLevel <= 0x7f);

        t->gl.baseLevel = (int8_t)minLevel;
        glTexParameteri(t->gl.target, GL_TEXTURE_BASE_LEVEL, t->gl.baseLevel);

        t->gl.maxLevel = (int8_t)maxLevel; // NOTE: according to the GL spec, the default value of this 1000
        glTexParameteri(t->gl.target, GL_TEXTURE_MAX_LEVEL, t->gl.maxLevel);
    }
#endif
}

void OpenGLDriver::update3DImage(Handle<HwTexture> th,
        uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
        uint32_t width, uint32_t height, uint32_t depth,
        PixelBufferDescriptor&& data) {
    DEBUG_MARKER()

    GLTexture* t = handle_cast<GLTexture *>(th);
    if (data.type == PixelDataType::COMPRESSED) {
        setCompressedTextureData(t,
                level, xoffset, yoffset, zoffset, width, height, depth, std::move(data));
    } else {
        setTextureData(t,
                level, xoffset, yoffset, zoffset, width, height, depth, std::move(data));
    }
}

void OpenGLDriver::generateMipmaps(Handle<HwTexture> th) {
    DEBUG_MARKER()

    auto& gl = mContext;
    GLTexture* t = handle_cast<GLTexture *>(th);
#if defined(BACKEND_OPENGL_LEVEL_GLES31)
    assert_invariant(t->gl.target != GL_TEXTURE_2D_MULTISAMPLE);
#endif
    // Note: glGenerateMimap can also fail if the internal format is not both
    // color-renderable and filterable (i.e.: doesn't work for depth)
    bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
    gl.activeTexture(OpenGLContext::DUMMY_TEXTURE_BINDING);

    t->gl.baseLevel = 0;
    t->gl.maxLevel = static_cast<int8_t>(t->levels - 1);

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    if (!gl.isES2()) {
        glTexParameteri(t->gl.target, GL_TEXTURE_BASE_LEVEL, t->gl.baseLevel);
        glTexParameteri(t->gl.target, GL_TEXTURE_MAX_LEVEL, t->gl.maxLevel);
    }
#endif

    glGenerateMipmap(t->gl.target);

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::setTextureData(GLTexture* t, uint32_t level,
        uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
        uint32_t width, uint32_t height, uint32_t depth,
        PixelBufferDescriptor&& p) {
    auto& gl = mContext;

    assert_invariant(t != nullptr);
    assert_invariant(xoffset + width <= std::max(1u, t->width >> level));
    assert_invariant(yoffset + height <= std::max(1u, t->height >> level));
    assert_invariant(t->samples <= 1);

    if (UTILS_UNLIKELY(t->gl.target == GL_TEXTURE_EXTERNAL_OES)) {
        // this is in fact an external texture, this becomes a no-op.
        return;
    }

    GLenum glFormat;
    GLenum glType;
    if (mContext.isES2()) {
        auto formatAndType = textureFormatToFormatAndType(t->format);
        glFormat = formatAndType.first;
        glType = formatAndType.second;
    } else {
        glFormat = getFormat(p.format);
        glType = getType(p.type);
    }

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    if (!gl.isES2()) {
        gl.pixelStore(GL_UNPACK_ROW_LENGTH, GLint(p.stride));
    }
#endif
    gl.pixelStore(GL_UNPACK_ALIGNMENT, GLint(p.alignment));

    // This is equivalent to using GL_UNPACK_SKIP_PIXELS and GL_UNPACK_SKIP_ROWS
    using PBD = PixelBufferDescriptor;
    size_t const stride = p.stride ? p.stride : width;
    size_t const bpp = PBD::computeDataSize(p.format, p.type, 1, 1, 1);
    size_t const bpr = PBD::computeDataSize(p.format, p.type, stride, 1, p.alignment);
    void const* const buffer = static_cast<char const*>(p.buffer) + p.left * bpp + bpr * p.top;

    switch (t->target) {
        case SamplerType::SAMPLER_EXTERNAL:
            // if we get there, it's because the user is trying to use an external texture,
            // but it's not supported, so instead, we behave like a texture2d.
            // fallthrough...
        case SamplerType::SAMPLER_2D:
            // NOTE: GL_TEXTURE_2D_MULTISAMPLE is not allowed
            bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
            gl.activeTexture(OpenGLContext::DUMMY_TEXTURE_BINDING);
            assert_invariant(t->gl.target == GL_TEXTURE_2D);
            glTexSubImage2D(t->gl.target, GLint(level),
                    GLint(xoffset), GLint(yoffset),
                    GLsizei(width), GLsizei(height), glFormat, glType, buffer);
            break;
        case SamplerType::SAMPLER_3D:
            assert_invariant(!gl.isES2());
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
            assert_invariant(zoffset + depth <= std::max(1u, t->depth >> level));
            bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
            gl.activeTexture(OpenGLContext::DUMMY_TEXTURE_BINDING);
            assert_invariant(t->gl.target == GL_TEXTURE_3D);
            glTexSubImage3D(t->gl.target, GLint(level),
                    GLint(xoffset), GLint(yoffset), GLint(zoffset),
                    GLsizei(width), GLsizei(height), GLsizei(depth), glFormat, glType, buffer);
#endif
            break;
        case SamplerType::SAMPLER_2D_ARRAY:
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            assert_invariant(!gl.isES2());
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
            assert_invariant(zoffset + depth <= t->depth);
            // NOTE: GL_TEXTURE_2D_MULTISAMPLE is not allowed
            bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
            gl.activeTexture(OpenGLContext::DUMMY_TEXTURE_BINDING);
            assert_invariant(t->gl.target == GL_TEXTURE_2D_ARRAY ||
                    t->gl.target == GL_TEXTURE_CUBE_MAP_ARRAY);
            glTexSubImage3D(t->gl.target, GLint(level),
                    GLint(xoffset), GLint(yoffset), GLint(zoffset),
                    GLsizei(width), GLsizei(height), GLsizei(depth), glFormat, glType, buffer);
#endif
            break;
        case SamplerType::SAMPLER_CUBEMAP: {
            assert_invariant(t->gl.target == GL_TEXTURE_CUBE_MAP);
            bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
            gl.activeTexture(OpenGLContext::DUMMY_TEXTURE_BINDING);

            assert_invariant(width == height);
            const size_t faceSize = PixelBufferDescriptor::computeDataSize(
                    p.format, p.type, p.stride ? p.stride : width, height, p.alignment);
            assert_invariant(zoffset + depth <= 6);
            UTILS_NOUNROLL
            for (size_t face = 0; face < depth; face++) {
                GLenum const target = getCubemapTarget(zoffset + face);
                glTexSubImage2D(target, GLint(level), GLint(xoffset), GLint(yoffset),
                        GLsizei(width), GLsizei(height), glFormat, glType,
                        static_cast<uint8_t const*>(buffer) + faceSize * face);
            }
            break;
        }
    }

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    if (!gl.isES2()) {
        // update the base/max LOD, so we don't access undefined LOD. this allows the app to
        // specify levels as they become available.
        if (int8_t(level) < t->gl.baseLevel) {
            t->gl.baseLevel = int8_t(level);
            glTexParameteri(t->gl.target, GL_TEXTURE_BASE_LEVEL, t->gl.baseLevel);
        }
        if (int8_t(level) > t->gl.maxLevel) {
            t->gl.maxLevel = int8_t(level);
            glTexParameteri(t->gl.target, GL_TEXTURE_MAX_LEVEL, t->gl.maxLevel);
        }
    }
#endif

    scheduleDestroy(std::move(p));

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::setCompressedTextureData(GLTexture* t, uint32_t level,
        uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
        uint32_t width, uint32_t height, uint32_t depth,
        PixelBufferDescriptor&& p) {
    auto& gl = mContext;

    assert_invariant(xoffset + width <= std::max(1u, t->width >> level));
    assert_invariant(yoffset + height <= std::max(1u, t->height >> level));
    assert_invariant(zoffset + depth <= t->depth);
    assert_invariant(t->samples <= 1);

    if (UTILS_UNLIKELY(t->gl.target == GL_TEXTURE_EXTERNAL_OES)) {
        // this is in fact an external texture, this becomes a no-op.
        return;
    }

    // TODO: maybe assert that the CompressedPixelDataType is the same as the internalFormat

    GLsizei const imageSize = GLsizei(p.imageSize);

    //  TODO: maybe assert the size is right (b/c we can compute it ourselves)

    switch (t->target) {
        case SamplerType::SAMPLER_EXTERNAL:
            // if we get there, it's because the user is trying to use an external texture,
            // but it's not supported, so instead, we behave like a texture2d.
            // fallthrough...
        case SamplerType::SAMPLER_2D:
            // NOTE: GL_TEXTURE_2D_MULTISAMPLE is not allowed
            bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
            gl.activeTexture(OpenGLContext::DUMMY_TEXTURE_BINDING);
            assert_invariant(t->gl.target == GL_TEXTURE_2D);
            glCompressedTexSubImage2D(t->gl.target, GLint(level),
                    GLint(xoffset), GLint(yoffset),
                    GLsizei(width), GLsizei(height),
                    t->gl.internalFormat, imageSize, p.buffer);
            break;
        case SamplerType::SAMPLER_3D:
            assert_invariant(!gl.isES2());
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
            bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
            gl.activeTexture(OpenGLContext::DUMMY_TEXTURE_BINDING);
            assert_invariant(t->gl.target == GL_TEXTURE_3D);
            glCompressedTexSubImage3D(t->gl.target, GLint(level),
                    GLint(xoffset), GLint(yoffset), GLint(zoffset),
                    GLsizei(width), GLsizei(height), GLsizei(depth),
                    t->gl.internalFormat, imageSize, p.buffer);
#endif
            break;
        case SamplerType::SAMPLER_2D_ARRAY:
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            assert_invariant(!gl.isES2());
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
            assert_invariant(t->gl.target == GL_TEXTURE_2D_ARRAY ||
                    t->gl.target == GL_TEXTURE_CUBE_MAP_ARRAY);
            glCompressedTexSubImage3D(t->gl.target, GLint(level),
                    GLint(xoffset), GLint(yoffset), GLint(zoffset),
                    GLsizei(width), GLsizei(height), GLsizei(depth),
                    t->gl.internalFormat, imageSize, p.buffer);
#endif
            break;
        case SamplerType::SAMPLER_CUBEMAP: {
            assert_invariant(t->gl.target == GL_TEXTURE_CUBE_MAP);
            bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
            gl.activeTexture(OpenGLContext::DUMMY_TEXTURE_BINDING);

            assert_invariant(width == height);
            const size_t faceSize = PixelBufferDescriptor::computeDataSize(
                    p.format, p.type, p.stride ? p.stride : width, height, p.alignment);

            UTILS_NOUNROLL
            for (size_t face = 0; face < depth; face++) {
                GLenum const target = getCubemapTarget(zoffset + face);
                glCompressedTexSubImage2D(target, GLint(level), GLint(xoffset), GLint(yoffset),
                        GLsizei(width), GLsizei(height), t->gl.internalFormat,
                        imageSize, static_cast<uint8_t const*>(p.buffer) + faceSize * face);
            }
            break;
        }
    }

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    if (!gl.isES2()) {
        // update the base/max LOD, so we don't access undefined LOD. this allows the app to
        // specify levels as they become available.
        if (int8_t(level) < t->gl.baseLevel) {
            t->gl.baseLevel = int8_t(level);
            glTexParameteri(t->gl.target, GL_TEXTURE_BASE_LEVEL, t->gl.baseLevel);
        }
        if (int8_t(level) > t->gl.maxLevel) {
            t->gl.maxLevel = int8_t(level);
            glTexParameteri(t->gl.target, GL_TEXTURE_MAX_LEVEL, t->gl.maxLevel);
        }
    }
#endif

    scheduleDestroy(std::move(p));

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::setupExternalImage(void* image) {
    mPlatform.retainExternalImage(image);
}

void OpenGLDriver::setExternalImage(Handle<HwTexture> th, void* image) {
    DEBUG_MARKER()
    GLTexture* t = handle_cast<GLTexture*>(th);
    assert_invariant(t);
    assert_invariant(t->target == SamplerType::SAMPLER_EXTERNAL);

    bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
    if (mPlatform.setExternalImage(image, t->externalTexture)) {
        // the target and id can be reset each time
        t->gl.target = t->externalTexture->target;
        t->gl.id = t->externalTexture->id;
        t->gl.targetIndex = (uint8_t)OpenGLContext::getIndexForTextureTarget(t->gl.target);
        bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
    }
}

void OpenGLDriver::setExternalImagePlane(Handle<HwTexture> th, void* image, uint32_t plane) {
    DEBUG_MARKER()
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
    mTexturesWithStreamsAttached.push_back(t);

    switch (hwStream->streamType) {
        case StreamType::NATIVE:
            mPlatform.attach(hwStream->stream, t->gl.id);
            break;
        case StreamType::ACQUIRED:
            break;
    }
    t->hwStream = hwStream;
}

UTILS_NOINLINE
void OpenGLDriver::detachStream(GLTexture* t) noexcept {
    auto& gl = mContext;
    auto& texturesWithStreamsAttached = mTexturesWithStreamsAttached;
    auto pos = std::find(texturesWithStreamsAttached.begin(), texturesWithStreamsAttached.end(), t);
    if (pos != texturesWithStreamsAttached.end()) {
        texturesWithStreamsAttached.erase(pos);
    }

    GLStream* s = static_cast<GLStream*>(t->hwStream); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    switch (s->streamType) {
        case StreamType::NATIVE:
            mPlatform.detach(t->hwStream->stream);
            // ^ this deletes the texture id
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

    GLStream* oldStream = static_cast<GLStream*>(texture->hwStream); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    switch (oldStream->streamType) {
        case StreamType::NATIVE:
            mPlatform.detach(texture->hwStream->stream);
            // ^ this deletes the texture id
            break;
        case StreamType::ACQUIRED:
            break;
    }

    switch (newStream->streamType) {
        case StreamType::NATIVE:
            glGenTextures(1, &texture->gl.id);
            mPlatform.attach(newStream->stream, texture->gl.id);
            break;
        case StreamType::ACQUIRED:
            // Just re-use the old texture id.
            break;
    }

    texture->hwStream = newStream;
}

void OpenGLDriver::beginTimerQuery(Handle<HwTimerQuery> tqh) {
    DEBUG_MARKER()
    GLTimerQuery* tq = handle_cast<GLTimerQuery*>(tqh);
    mTimerQueryImpl->beginTimeElapsedQuery(tq);
}

void OpenGLDriver::endTimerQuery(Handle<HwTimerQuery> tqh) {
    DEBUG_MARKER()
    GLTimerQuery* tq = handle_cast<GLTimerQuery*>(tqh);
    mTimerQueryImpl->endTimeElapsedQuery(tq);
}

bool OpenGLDriver::getTimerQueryValue(Handle<HwTimerQuery> tqh, uint64_t* elapsedTime) {
    GLTimerQuery* tq = handle_cast<GLTimerQuery*>(tqh);
    return OpenGLTimerQueryInterface::getTimerQueryValue(tq, elapsedTime);
}

void OpenGLDriver::compilePrograms(CompilerPriorityQueue priority,
        CallbackHandler* handler, CallbackHandler::Callback callback, void* user) {
    if (callback) {
        getShaderCompilerService().notifyWhenAllProgramsAreReady(handler, callback, user);
    }
}

void OpenGLDriver::beginRenderPass(Handle<HwRenderTarget> rth,
        const RenderPassParams& params) {
    DEBUG_MARKER()

    getShaderCompilerService().tick();

    auto& gl = mContext;

    mRenderPassTarget = rth;
    mRenderPassParams = params;

    GLRenderTarget* rt = handle_cast<GLRenderTarget*>(rth);

    // If we're rendering into the default render target (i.e. into the current SwapChain),
    // get the value of the output colorspace from there, otherwise it's always linear.
    assert_invariant(!rt->gl.isDefault || mCurrentDrawSwapChain);
    mRec709OutputColorspace = rt->gl.isDefault ? mCurrentDrawSwapChain->rec709 : false;

    const TargetBufferFlags clearFlags = params.flags.clear & rt->targets;
    TargetBufferFlags discardFlags = params.flags.discardStart & rt->targets;

    gl.bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);
    CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e, GL_FRAMEBUFFER)

    if (gl.ext.EXT_discard_framebuffer
            && !gl.bugs.disable_invalidate_framebuffer) {
        AttachmentArray attachments; // NOLINT
        GLsizei const attachmentCount = getAttachments(attachments, rt, discardFlags);
        if (attachmentCount) {
            gl.procs.invalidateFramebuffer(GL_FRAMEBUFFER, attachmentCount, attachments.data());
        }
        CHECK_GL_ERROR(utils::slog.e)
    } else {
        // It's important to clear the framebuffer before drawing, as it resets
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
        // However, Filament specifies that a non multi-sample attachment to a
        // multi-sample RenderTarget is always discarded. We do this because implementing
        // the load on Metal is not trivial, and it's not a feature we rely on at this time.
        discardFlags |= rt->gl.resolve;
    }

    if (any(clearFlags)) {
        gl.disable(GL_SCISSOR_TEST);
        clearWithRasterPipe(clearFlags,
                params.clearColor, (GLfloat)params.clearDepth, (GLint)params.clearStencil);
    }

    // we need to reset those after we call clearWithRasterPipe()
    mRenderPassColorWrite   = any(clearFlags & TargetBufferFlags::COLOR_ALL);
    mRenderPassDepthWrite   = any(clearFlags & TargetBufferFlags::DEPTH);
    mRenderPassStencilWrite = any(clearFlags & TargetBufferFlags::STENCIL);

    static_assert(sizeof(GLsizei) >= sizeof(uint32_t));
    gl.viewport(params.viewport.left, params.viewport.bottom,
            (GLsizei)std::min(uint32_t(std::numeric_limits<int32_t>::max()), params.viewport.width),
            (GLsizei)std::min(uint32_t(std::numeric_limits<int32_t>::max()), params.viewport.height));

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
        // ignore discard flags if the buffer wasn't written at all
        discardFlags &= ~TargetBufferFlags::COLOR_ALL;
    }
    if (!mRenderPassDepthWrite) {
        // ignore discard flags if the buffer wasn't written at all
        discardFlags &= ~TargetBufferFlags::DEPTH;
    }
    if (!mRenderPassStencilWrite) {
        // ignore discard flags if the buffer wasn't written at all
        discardFlags &= ~TargetBufferFlags::STENCIL;
    }

    if (rt->gl.isDefault) {
        assert_invariant(mCurrentDrawSwapChain);
        discardFlags &= ~mPlatform.getPreservedFlags(mCurrentDrawSwapChain->swapChain);
    }

    if (gl.ext.EXT_discard_framebuffer) {
        auto effectiveDiscardFlags = discardFlags;
        if (gl.bugs.invalidate_end_only_if_invalidate_start) {
            effectiveDiscardFlags &= mRenderPassParams.flags.discardStart;
        }
        if (!gl.bugs.disable_invalidate_framebuffer) {
            // we wouldn't have to bind the framebuffer if we had glInvalidateNamedFramebuffer()
            gl.bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);
            AttachmentArray attachments; // NOLINT
            GLsizei const attachmentCount = getAttachments(attachments, rt, effectiveDiscardFlags);
            if (attachmentCount) {
                gl.procs.invalidateFramebuffer(GL_FRAMEBUFFER, attachmentCount, attachments.data());
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

    if (UTILS_UNLIKELY(getContext().isES2())) {
        // ES2 doesn't have manual resolve capabilities
        return;
    }

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    assert_invariant(rt->gl.fbo_read);
    auto& gl = mContext;
    const TargetBufferFlags resolve = rt->gl.resolve & ~discardFlags;
    GLbitfield const mask = getAttachmentBitfield(resolve);
    if (UTILS_UNLIKELY(mask)) {

        // we can only resolve COLOR0 at the moment
        assert_invariant(!(rt->targets &
                (TargetBufferFlags::COLOR_ALL & ~TargetBufferFlags::COLOR0)));

        GLint read = (GLint)rt->gl.fbo_read;
        GLint draw = (GLint)rt->gl.fbo;
        if (action == ResolveAction::STORE) {
            std::swap(read, draw);
        }
        gl.bindFramebuffer(GL_READ_FRAMEBUFFER, read);
        gl.bindFramebuffer(GL_DRAW_FRAMEBUFFER, draw);

        CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e, GL_READ_FRAMEBUFFER)
        CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e, GL_DRAW_FRAMEBUFFER)

        gl.disable(GL_SCISSOR_TEST);
        glBlitFramebuffer(0, 0, (GLint)rt->width, (GLint)rt->height,
                0, 0, (GLint)rt->width, (GLint)rt->height, mask, GL_NEAREST);
        CHECK_GL_ERROR(utils::slog.e)
    }
#endif
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
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
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
#endif
    if (any(buffers & TargetBufferFlags::DEPTH)) {
        attachments[attachmentCount++] = defaultFramebuffer ? GL_DEPTH : GL_DEPTH_ATTACHMENT;
    }
    if (any(buffers & TargetBufferFlags::STENCIL)) {
        attachments[attachmentCount++] = defaultFramebuffer ? GL_STENCIL : GL_STENCIL_ATTACHMENT;
    }
    return attachmentCount;
}

// Sets up a scissor rectangle that automatically gets clipped against the viewport.
void OpenGLDriver::setScissor(Viewport const& scissor) noexcept {
    constexpr uint32_t maxvalu = std::numeric_limits<int32_t>::max();

    auto& gl = mContext;

    // TODO: disable scissor when it is bigger than the current surface?
    if (scissor.left == 0 && scissor.bottom == 0 &&
        scissor.width >= maxvalu && scissor.height >= maxvalu) {
        gl.disable(GL_SCISSOR_TEST);
        return;
    }

    gl.setScissor(
            GLint(scissor.left), GLint(scissor.bottom),
            GLint(scissor.width), GLint(scissor.height));
    gl.enable(GL_SCISSOR_TEST);
}

// ------------------------------------------------------------------------------------------------
// Setting rendering state
// ------------------------------------------------------------------------------------------------

void OpenGLDriver::bindUniformBuffer(uint32_t index, Handle<HwBufferObject> ubh) {
    DEBUG_MARKER()
    GLBufferObject* ub = handle_cast<GLBufferObject *>(ubh);
    assert_invariant(ub->bindingType == BufferObjectBinding::UNIFORM);
    bindBufferRange(BufferObjectBinding::UNIFORM, index, ubh, 0, ub->byteCount);
}

void OpenGLDriver::bindBufferRange(BufferObjectBinding bindingType, uint32_t index,
        Handle<HwBufferObject> ubh, uint32_t offset, uint32_t size) {
    DEBUG_MARKER()
    auto& gl = mContext;

    assert_invariant(bindingType == BufferObjectBinding::SHADER_STORAGE ||
                     bindingType == BufferObjectBinding::UNIFORM);

    GLBufferObject* ub = handle_cast<GLBufferObject *>(ubh);

    assert_invariant(offset + size <= ub->byteCount);

    if (UTILS_UNLIKELY(ub->bindingType == BufferObjectBinding::UNIFORM && gl.isES2())) {
        mUniformBindings[index] = {
                ub->gl.id,
                static_cast<uint8_t const*>(ub->gl.buffer) + offset,
                ub->age,
        };
    } else {
        GLenum const target = GLUtils::getBufferBindingType(bindingType);

        assert_invariant(bindingType == BufferObjectBinding::SHADER_STORAGE ||
                         ub->gl.binding == target);

        gl.bindBufferRange(target, GLuint(index), ub->gl.id, offset, size);
    }

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::unbindBuffer(BufferObjectBinding bindingType, uint32_t index) {
    DEBUG_MARKER()
    auto& gl = mContext;

    if (UTILS_UNLIKELY(bindingType == BufferObjectBinding::UNIFORM && gl.isES2())) {
        mUniformBindings[index] = {};
        return;
    }

    GLenum const target = GLUtils::getBufferBindingType(bindingType);
    gl.bindBufferRange(target, GLuint(index), 0, 0, 0);
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::bindSamplers(uint32_t index, Handle<HwSamplerGroup> sbh) {
    DEBUG_MARKER()
    assert_invariant(index < Program::SAMPLER_BINDING_COUNT);
    GLSamplerGroup* sb = handle_cast<GLSamplerGroup *>(sbh);
    mSamplerBindings[index] = sb;
    CHECK_GL_ERROR(utils::slog.e)
}


#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
GLuint OpenGLDriver::getSamplerSlow(SamplerParams params) const noexcept {
    assert_invariant(mSamplerMap.find(params) == mSamplerMap.end());

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
    auto& gl = mContext;
    if (gl.ext.EXT_texture_filter_anisotropic &&
            !gl.bugs.texture_filter_anisotropic_broken_on_sampler) {
        GLfloat const anisotropy = float(1u << params.anisotropyLog2);
        glSamplerParameterf(s, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                std::min(gl.gets.max_anisotropy, anisotropy));
    }
#endif
    CHECK_GL_ERROR(utils::slog.e)
    mSamplerMap[params] = s;
    return s;
}
#endif

void OpenGLDriver::insertEventMarker(char const* string, uint32_t len) {
#ifndef __EMSCRIPTEN__
#ifdef GL_EXT_debug_marker
    auto& gl = mContext;
    if (gl.ext.EXT_debug_marker) {
        glInsertEventMarkerEXT(GLsizei(len ? len : strlen(string)), string);
    }
#endif
#endif
}

void OpenGLDriver::pushGroupMarker(char const* string, uint32_t len) {
#ifndef __EMSCRIPTEN__
#ifdef GL_EXT_debug_marker
#if DEBUG_MARKER_LEVEL & DEBUG_MARKER_OPENGL
    if (UTILS_LIKELY(mContext.ext.EXT_debug_marker)) {
        glPushGroupMarkerEXT(GLsizei(len ? len : strlen(string)), string);
    }
#endif
#endif

#if DEBUG_MARKER_LEVEL & DEBUG_MARKER_BACKEND
    SYSTRACE_CONTEXT();
    SYSTRACE_NAME_BEGIN(string);
#endif
#endif
}

void OpenGLDriver::popGroupMarker(int) {
#ifndef __EMSCRIPTEN__
#ifdef GL_EXT_debug_marker
#if DEBUG_MARKER_LEVEL & DEBUG_MARKER_OPENGL
    if (UTILS_LIKELY(mContext.ext.EXT_debug_marker)) {
        glPopGroupMarkerEXT();
    }
#endif
#endif

#if DEBUG_MARKER_LEVEL & DEBUG_MARKER_BACKEND
    SYSTRACE_CONTEXT();
    SYSTRACE_NAME_END();
#endif
#endif
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

    GLenum const glFormat = getFormat(p.format);
    GLenum const glType = getType(p.type);

    gl.pixelStore(GL_PACK_ALIGNMENT, (GLint)p.alignment);

    /*
     * glReadPixel() operation...
     *
     *  Framebuffer as seen on         User buffer
     *  screen
     *  +--------------------+
     *  |                    |                stride         alignment
     *  |                    |         ----------------------->-->
     *  |                    |         +----------------------+--+   low addresses
     *  |                    |         |          |           |  |
     *  |             w      |         |          | bottom    |  |
     *  |       <--------->  |         |          V           |  |
     *  |       +---------+  |         |     +.........+      |  |
     *  |       |     ^   |  | =====>  |     |         |      |  |
     *  |   x   |    h|   |  |         |left |         |      |  |
     *  +------>|     v   |  |         +---->|         |      |  |
     *  |       +.........+  |         |     +---------+      |  |
     *  |            ^       |         |                      |  |
     *  |          y |       |         +----------------------+--+  high addresses
     *  +------------+-------+
     *                                  Image is "flipped" vertically
     *                                  "bottom" is from the "top" (low addresses)
     *                                  of the buffer.
     */

    GLRenderTarget const* s = handle_cast<GLRenderTarget const*>(src);

    using PBD = PixelBufferDescriptor;

    // The PBO only needs to accommodate the area we're reading, with alignment.
    auto const pboSize = (GLsizeiptr)PBD::computeDataSize(
            p.format, p.type, width, height, p.alignment);

    if (UTILS_UNLIKELY(gl.isES2())) {
        void* buffer = malloc(pboSize);
        if (buffer) {
            gl.bindFramebuffer(GL_FRAMEBUFFER, s->gl.fbo_read ? s->gl.fbo_read : s->gl.fbo);
            glReadPixels(GLint(x), GLint(y), GLint(width), GLint(height), glFormat, glType, buffer);
            CHECK_GL_ERROR(utils::slog.e)

            // now we need to flip the buffer vertically to match our API
            size_t const stride = p.stride ? p.stride : width;
            size_t const bpp = PBD::computeDataSize(p.format, p.type, 1, 1, 1);
            size_t const dstBpr = PBD::computeDataSize(p.format, p.type, stride, 1, p.alignment);
            char* pDst = (char*)p.buffer + p.left * bpp + dstBpr * (p.top + height - 1);

            size_t const srcBpr = PBD::computeDataSize(p.format, p.type, width, 1, p.alignment);
            char const* pSrc = (char const*)buffer;
            for (size_t i = 0; i < height; ++i) {
                memcpy(pDst, pSrc, bpp * width);
                pSrc += srcBpr;
                pDst -= dstBpr;
            }
        }
        free(buffer);
        scheduleDestroy(std::move(p));
        return;
    }

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    // glReadPixel doesn't resolve automatically, but it does with the auto-resolve extension,
    // which we're always emulating. So if we have a resolved fbo (fbo_read), use that instead.
    gl.bindFramebuffer(GL_READ_FRAMEBUFFER, s->gl.fbo_read ? s->gl.fbo_read : s->gl.fbo);

    GLuint pbo;
    glGenBuffers(1, &pbo);
    gl.bindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_PACK_BUFFER, pboSize, nullptr, GL_STATIC_DRAW);
    glReadPixels(GLint(x), GLint(y), GLint(width), GLint(height), glFormat, glType, nullptr);
    gl.bindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    CHECK_GL_ERROR(utils::slog.e)

    // we're forced to make a copy on the heap because otherwise it deletes std::function<> copy
    // constructor.
    auto* const pUserBuffer = new PixelBufferDescriptor(std::move(p));
    whenGpuCommandsComplete([this, width, height, pbo, pboSize, pUserBuffer]() mutable {
        PixelBufferDescriptor& p = *pUserBuffer;
        auto& gl = mContext;
        gl.bindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
        void* vaddr = nullptr;
#if defined(__EMSCRIPTEN__)
        std::unique_ptr<uint8_t[]> clientBuffer = std::make_unique<uint8_t[]>(pboSize);
        glGetBufferSubData(GL_PIXEL_PACK_BUFFER, 0, pboSize, clientBuffer.get());
        vaddr = clientBuffer.get();
#else
        vaddr = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, pboSize, GL_MAP_READ_BIT);
#endif
        if (vaddr) {
            // now we need to flip the buffer vertically to match our API
            size_t const stride = p.stride ? p.stride : width;
            size_t const bpp = PBD::computeDataSize(p.format, p.type, 1, 1, 1);
            size_t const dstBpr = PBD::computeDataSize(p.format, p.type, stride, 1, p.alignment);
            char* pDst = (char*)p.buffer + p.left * bpp + dstBpr * (p.top + height - 1);

            size_t const srcBpr = PBD::computeDataSize(p.format, p.type, width, 1, p.alignment);
            char const* pSrc = (char const*)vaddr;

            for (size_t i = 0; i < height; ++i) {
                memcpy(pDst, pSrc, bpp * width);
                pSrc += srcBpr;
                pDst -= dstBpr;
            }
#if !defined(__EMSCRIPTEN__)
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
#endif
        }
        gl.bindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        glDeleteBuffers(1, &pbo);
        scheduleDestroy(std::move(p));
        delete pUserBuffer;
        CHECK_GL_ERROR(utils::slog.e)
    });
#endif
}

void OpenGLDriver::readBufferSubData(backend::BufferObjectHandle boh,
        uint32_t offset, uint32_t size, backend::BufferDescriptor&& p) {
    UTILS_UNUSED_IN_RELEASE auto& gl = mContext;
    assert_invariant(!gl.isES2());

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    GLBufferObject const* bo = handle_cast<GLBufferObject const*>(boh);

    // TODO: measure the two solutions
    if constexpr (true) {
        // schedule a copy of the buffer we're reading into a PBO, this *should* happen
        // asynchronously without stalling the CPU.
        GLuint pbo;
        glGenBuffers(1, &pbo);
        gl.bindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
        glBufferData(GL_PIXEL_PACK_BUFFER, (GLsizeiptr)size, nullptr, GL_STATIC_DRAW);
        gl.bindBuffer(bo->gl.binding, bo->gl.id);
        glCopyBufferSubData(bo->gl.binding, GL_PIXEL_PACK_BUFFER, offset, 0, size);
        gl.bindBuffer(bo->gl.binding, 0);
        gl.bindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        CHECK_GL_ERROR(utils::slog.e)

        // then, we schedule a mapBuffer of the PBO later, once the fence has signaled
        auto* pUserBuffer = new BufferDescriptor(std::move(p));
        whenGpuCommandsComplete([this, size, pbo, pUserBuffer]() mutable {
            BufferDescriptor& p = *pUserBuffer;
            auto& gl = mContext;
            gl.bindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
            void* vaddr = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, size, GL_MAP_READ_BIT);
            if (vaddr) {
                memcpy(p.buffer, vaddr, size);
                glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            }
            gl.bindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            glDeleteBuffers(1, &pbo);
            scheduleDestroy(std::move(p));
            delete pUserBuffer;
            CHECK_GL_ERROR(utils::slog.e)
        });
    } else {
        gl.bindBuffer(bo->gl.binding, bo->gl.id);
        // TODO: this glMapBufferRange may stall. Ideally we want to use whenGpuCommandsComplete
        //       but that's tricky because boh could be destroyed right after this call.
        void* vaddr = glMapBufferRange(bo->gl.binding, offset, size, GL_MAP_READ_BIT);
        if (vaddr) {
            memcpy(p.buffer, vaddr, size);
            glUnmapBuffer(bo->gl.binding);
        }
        gl.bindBuffer(bo->gl.binding, 0);
        scheduleDestroy(std::move(p));
        CHECK_GL_ERROR(utils::slog.e)
    }
#endif
}


void OpenGLDriver::runEveryNowAndThen(std::function<bool()> fn) noexcept {
    mEveryNowAndThenOps.push_back(std::move(fn));
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

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
void OpenGLDriver::whenFrameComplete(const std::function<void()>& fn) noexcept {
    mFrameCompleteOps.push_back(fn);
}

void OpenGLDriver::whenGpuCommandsComplete(const std::function<void()>& fn) noexcept {
    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    mGpuCommandCompleteOps.emplace_back(sync, fn);
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::executeGpuCommandsCompleteOps() noexcept {
    auto& v = mGpuCommandCompleteOps;
    auto it = v.begin();
    while (it != v.end()) {
        auto const& [sync, fn] = *it;
        GLenum const syncStatus = glClientWaitSync(sync, 0, 0u);
        switch (syncStatus) {
            case GL_TIMEOUT_EXPIRED:
                // not ready
                ++it;
                break;
            case GL_ALREADY_SIGNALED:
            case GL_CONDITION_SATISFIED:
                // ready
                it->second();
                glDeleteSync(sync);
                it = v.erase(it);
                break;
            default:
                // This should never happen, but is very problematic if it does, as we might leak
                // some data depending on what the callback does. However, we clean up our own state.
                glDeleteSync(sync);
                it = v.erase(it);
                break;
        }
    }
}
#endif

// ------------------------------------------------------------------------------------------------
// Rendering ops
// ------------------------------------------------------------------------------------------------

void OpenGLDriver::tick(int) {
    DEBUG_MARKER()
    executeGpuCommandsCompleteOps();
    executeEveryNowAndThenOps();
    getShaderCompilerService().tick();
}

void OpenGLDriver::beginFrame(
        UTILS_UNUSED int64_t monotonic_clock_ns,
        UTILS_UNUSED uint32_t frameId) {
    DEBUG_MARKER()
    auto& gl = mContext;
    insertEventMarker("beginFrame");
    if (UTILS_UNLIKELY(!mTexturesWithStreamsAttached.empty())) {
        OpenGLPlatform& platform = mPlatform;
        for (GLTexture const* t : mTexturesWithStreamsAttached) {
            assert_invariant(t && t->hwStream);
            if (t->hwStream->streamType == StreamType::NATIVE) {
                assert_invariant(t->hwStream->stream);
                platform.updateTexImage(t->hwStream->stream,
                        &static_cast<GLStream*>(t->hwStream)->user_thread.timestamp); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
                // NOTE: We assume that updateTexImage() binds the texture on our behalf
                gl.updateTexImage(GL_TEXTURE_EXTERNAL_OES, t->gl.id);
            }
        }
    }
}

void OpenGLDriver::setFrameScheduledCallback(Handle<HwSwapChain> sch,
        FrameScheduledCallback callback, void* user) {
    DEBUG_MARKER()
}

void OpenGLDriver::setFrameCompletedCallback(Handle<HwSwapChain> sch,
        CallbackHandler* handler, CallbackHandler::Callback callback, void* user) {
    DEBUG_MARKER()
}

void OpenGLDriver::setPresentationTime(int64_t monotonic_clock_ns) {
    DEBUG_MARKER()
    mPlatform.setPresentationTime(monotonic_clock_ns);
}

void OpenGLDriver::endFrame(UTILS_UNUSED uint32_t frameId) {
    DEBUG_MARKER()
#if defined(__EMSCRIPTEN__)
    // WebGL builds are single-threaded so users might manipulate various GL state after we're
    // done with the frame. We do NOT officially support using Filament in this way, but we can
    // at least do some minimal safety things here, such as resetting the VAO to 0.
    auto& gl = mContext;
    gl.bindVertexArray(nullptr);
    for (int unit = OpenGLContext::DUMMY_TEXTURE_BINDING; unit >= 0; unit--) {
        gl.bindTexture(unit, GL_TEXTURE_2D, 0);
    }
    gl.disable(GL_CULL_FACE);
    gl.depthFunc(GL_LESS);
    gl.disable(GL_SCISSOR_TEST);
#endif
    //SYSTRACE_NAME("glFinish");
    //glFinish();
    insertEventMarker("endFrame");
}

void OpenGLDriver::flush(int) {
    DEBUG_MARKER()
    auto& gl = mContext;
    if (!gl.bugs.disable_glFlush) {
        glFlush();
    }
}

void OpenGLDriver::finish(int) {
    DEBUG_MARKER()
    glFinish();
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    executeGpuCommandsCompleteOps();
    assert_invariant(mGpuCommandCompleteOps.empty());
#endif
    executeEveryNowAndThenOps();
    // Note: since we executed a glFinish(), all pending tasks should be done

    // However, some tasks rely on a separated thread to publish their result (e.g.
    // endTimerQuery), so the result could very well not be ready, and the task will
    // linger a bit longer, this is only true for mEveryNowAndThenOps tasks.
    // The fallout of this is that we can't assert that mEveryNowAndThenOps is empty.
}

UTILS_NOINLINE
void OpenGLDriver::clearWithRasterPipe(TargetBufferFlags clearFlags,
        math::float4 const& linearColor, GLfloat depth, GLint stencil) noexcept {

    if (any(clearFlags & TargetBufferFlags::COLOR_ALL)) {
        mContext.colorMask(GL_TRUE);
    }
    if (any(clearFlags & TargetBufferFlags::DEPTH)) {
        mContext.depthMask(GL_TRUE);
    }
    if (any(clearFlags & TargetBufferFlags::STENCIL)) {
        mContext.stencilMaskSeparate(0xFF, mContext.state.stencil.back.stencilMask);
    }

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    if (UTILS_LIKELY(!mContext.isES2())) {
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
    } else
#endif
    {
        GLbitfield mask = 0;
        if (any(clearFlags & TargetBufferFlags::COLOR0)) {
            glClearColor(linearColor.r, linearColor.g, linearColor.b, linearColor.a);
            mask |= GL_COLOR_BUFFER_BIT;
        }
        if (any(clearFlags & TargetBufferFlags::DEPTH)) {
            glClearDepthf(depth);
            mask |= GL_DEPTH_BUFFER_BIT;
        }
        if (any(clearFlags & TargetBufferFlags::STENCIL)) {
            glClearStencil(stencil);
            mask |= GL_STENCIL_BUFFER_BIT;
        }
        if (mask) {
            glClear(mask);
        }
    }

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::resolve(
        Handle<HwTexture> dst, uint8_t srcLevel, uint8_t srcLayer,
        Handle<HwTexture> src, uint8_t dstLevel, uint8_t dstLayer) {
    DEBUG_MARKER()
    GLTexture const* const s = handle_cast<GLTexture*>(src);
    GLTexture const* const d = handle_cast<GLTexture*>(dst);
    assert_invariant(s);
    assert_invariant(d);

    ASSERT_PRECONDITION(
            d->width == s->width && d->height == s->height,
            "invalid resolve: src and dst sizes don't match");

    ASSERT_PRECONDITION(s->samples > 1 && d->samples == 1,
            "invalid resolve: src.samples=%u, dst.samples=%u",
            +s->samples, +d->samples);

    blit(   dst, dstLevel, dstLayer, {},
            src, srcLevel, srcLayer, {},
            { d->width, d->height });
}

void OpenGLDriver::blit(
        Handle<HwTexture> dst, uint8_t srcLevel, uint8_t srcLayer, uint2 dstOrigin,
        Handle<HwTexture> src, uint8_t dstLevel, uint8_t dstLayer, uint2 srcOrigin,
        uint2 size) {
    DEBUG_MARKER()
    UTILS_UNUSED_IN_RELEASE auto& gl = mContext;
    assert_invariant(!gl.isES2());

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2

    GLTexture* d = handle_cast<GLTexture*>(dst);
    GLTexture* s = handle_cast<GLTexture*>(src);
    assert_invariant(d);
    assert_invariant(s);

    ASSERT_PRECONDITION_NON_FATAL(any(d->usage & TextureUsage::BLIT_DST),
            "texture doesn't have BLIT_DST");

    ASSERT_PRECONDITION_NON_FATAL(any(s->usage & TextureUsage::BLIT_SRC),
            "texture doesn't have BLIT_SRC");

    ASSERT_PRECONDITION_NON_FATAL(s->format == d->format,
            "src and dst texture format don't match");

    enum class AttachmentType : GLenum {
        COLOR = GL_COLOR_ATTACHMENT0,
        DEPTH = GL_DEPTH_ATTACHMENT,
        STENCIL = GL_STENCIL_ATTACHMENT,
        DEPTH_STENCIL = GL_DEPTH_STENCIL_ATTACHMENT,
    };

    auto getFormatType = [](TextureFormat format) -> AttachmentType {
        bool const depth = isDepthFormat(format);
        bool const stencil = isStencilFormat(format);
        if (depth && stencil) return AttachmentType::DEPTH_STENCIL;
        if (depth) return AttachmentType::DEPTH;
        if (stencil) return AttachmentType::STENCIL;
        return AttachmentType::COLOR;
    };

    AttachmentType const type = getFormatType(d->format);
    assert_invariant(type == getFormatType(s->format));

    // GL_INVALID_OPERATION is generated if mask contains any of the GL_DEPTH_BUFFER_BIT or
    // GL_STENCIL_BUFFER_BIT and filter is not GL_NEAREST.
    GLbitfield mask = {};
    switch (type) {
        case AttachmentType::COLOR:
            mask = GL_COLOR_BUFFER_BIT;
            break;
        case AttachmentType::DEPTH:
            mask = GL_DEPTH_BUFFER_BIT;
            break;
        case AttachmentType::STENCIL:
            mask = GL_STENCIL_BUFFER_BIT;
            break;
        case AttachmentType::DEPTH_STENCIL:
            mask = GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
            break;
    };

    GLuint fbo[2] = {};
    glGenFramebuffers(2, fbo);

    gl.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[0]);
    switch (d->target) {
        case SamplerType::SAMPLER_2D:
            if (any(d->usage & TextureUsage::SAMPLEABLE)) {
                glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GLenum(type),
                        GL_TEXTURE_2D, d->gl.id, dstLevel);
            } else {
                glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GLenum(type),
                        GL_RENDERBUFFER, d->gl.id);
            }
            break;
        case SamplerType::SAMPLER_CUBEMAP:
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GLenum(type),
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + dstLayer, d->gl.id, dstLevel);
            break;
        case SamplerType::SAMPLER_2D_ARRAY:
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
        case SamplerType::SAMPLER_3D:
            glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GLenum(type),
                    d->gl.id, dstLevel, dstLayer);
            break;
        case SamplerType::SAMPLER_EXTERNAL:
            break;
    }
    CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e, GL_READ_FRAMEBUFFER)

    gl.bindFramebuffer(GL_READ_FRAMEBUFFER, fbo[1]);
    switch (s->target) {
        case SamplerType::SAMPLER_2D:
            if (any(s->usage & TextureUsage::SAMPLEABLE)) {
                glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GLenum(type),
                        GL_TEXTURE_2D, s->gl.id, srcLevel);
            } else {
                glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GLenum(type),
                        GL_RENDERBUFFER, s->gl.id);
            }
            break;
        case SamplerType::SAMPLER_CUBEMAP:
            glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GLenum(type),
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + srcLayer, s->gl.id, srcLevel);
            break;
        case SamplerType::SAMPLER_2D_ARRAY:
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
        case SamplerType::SAMPLER_3D:
            glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GLenum(type),
                    s->gl.id, srcLevel, srcLayer);
            break;
        case SamplerType::SAMPLER_EXTERNAL:
            break;
    }
    CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e, GL_DRAW_FRAMEBUFFER)

    gl.disable(GL_SCISSOR_TEST);
    glBlitFramebuffer(
            srcOrigin.x, srcOrigin.y, srcOrigin.x + size.x, srcOrigin.y + size.y,
            dstOrigin.x, dstOrigin.y, dstOrigin.x + size.x, dstOrigin.y + size.y,
            mask, GL_NEAREST);
    CHECK_GL_ERROR(utils::slog.e)

    gl.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    gl.bindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glDeleteFramebuffers(2, fbo);

    if (any(d->usage & TextureUsage::SAMPLEABLE)) {
        // In a sense, blitting to a texture level is similar to calling setTextureData on it; in
        // both cases, we update the base/max LOD to give shaders access to levels as they become
        // available.  Note that this can only expand the LOD range (never shrink it), and that
        // users can override this range by calling setMinMaxLevels().
        updateTextureLodRange(d, int8_t(dstLevel));
    }

#endif
}

void OpenGLDriver::blitDEPRECATED(TargetBufferFlags buffers,
        Handle<HwRenderTarget> dst, Viewport dstRect,
        Handle<HwRenderTarget> src, Viewport srcRect,
        SamplerMagFilter filter) {

    // Note: blitDEPRECATED is only used by Renderer::copyFrame

    DEBUG_MARKER()
    UTILS_UNUSED_IN_RELEASE auto& gl = mContext;
    assert_invariant(!gl.isES2());

    ASSERT_PRECONDITION(buffers == TargetBufferFlags::COLOR0,
            "blitDEPRECATED only supports COLOR0");

    ASSERT_PRECONDITION(srcRect.left >= 0 && srcRect.bottom >= 0 &&
                        dstRect.left >= 0 && dstRect.bottom >= 0,
            "Source and destination rects must be positive.");

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2

    GLenum const glFilterMode = (filter == SamplerMagFilter::NEAREST) ? GL_NEAREST : GL_LINEAR;

    // note: for msaa RenderTargets with non-msaa attachments, we copy from the msaa sidecar
    // buffer -- this should produce the same output that if we copied from the resolved
    // texture. EXT_multisampled_render_to_texture seems to allow both behaviours, and this
    // is an emulation of that.  We cannot use the resolved texture easily because it's not
    // actually attached to the RenderTarget. Another implementation would be to do a
    // reverse-resolve, but that wouldn't buy us anything.
    GLRenderTarget const* s = handle_cast<GLRenderTarget const*>(src);
    GLRenderTarget const* d = handle_cast<GLRenderTarget const*>(dst);

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
            GL_COLOR_BUFFER_BIT, glFilterMode);
    CHECK_GL_ERROR(utils::slog.e)
#endif
}

void OpenGLDriver::updateTextureLodRange(GLTexture* texture, int8_t targetLevel) noexcept {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    auto& gl = mContext;
    if (!gl.isES2()) {
        if (texture && any(texture->usage & TextureUsage::SAMPLEABLE)) {
            if (targetLevel < texture->gl.baseLevel || targetLevel > texture->gl.maxLevel) {
                bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, texture);
                gl.activeTexture(OpenGLContext::DUMMY_TEXTURE_BINDING);
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
#endif
}

void OpenGLDriver::draw(PipelineState state, Handle<HwRenderPrimitive> rph,
        uint32_t const indexOffset, uint32_t const indexCount, uint32_t const instanceCount) {
    DEBUG_MARKER()
    auto& gl = mContext;

    OpenGLProgram* const p = handle_cast<OpenGLProgram*>(state.program);

    bool const success = useProgram(p);
    if (UTILS_UNLIKELY(!success)) {
        // Avoid fatal (or cascading) errors that can occur during the draw call when the program
        // is invalid. The shader compile error has already been dumped to the console at this
        // point, so it's fine to simply return early.
        return;
    }

    GLRenderPrimitive* const rp = handle_cast<GLRenderPrimitive*>(rph);

    // Gracefully do nothing if the render primitive has not been set up.
    VertexBufferHandle vb = rp->gl.vertexBufferWithObjects;
    if (UTILS_UNLIKELY(!vb)) {
        return;
    }

    gl.bindVertexArray(&rp->gl);

    // If necessary, mutate the bindings in the VAO.
    GLVertexBuffer const* const glvb = handle_cast<GLVertexBuffer*>(vb);
    if (UTILS_UNLIKELY(rp->gl.vertexBufferVersion != glvb->bufferObjectsVersion)) {
        updateVertexArrayObject(rp, glvb);
    }

    setRasterState(state.rasterState);
    setStencilState(state.stencilState);

    gl.polygonOffset(state.polygonOffset.slope, state.polygonOffset.constant);

    setScissor(state.scissor);

    if (UTILS_LIKELY(instanceCount <= 1)) {
        glDrawElements(GLenum(rp->type), (GLsizei)indexCount, rp->gl.getIndicesType(),
                reinterpret_cast<const void*>(indexOffset * rp->gl.indicesSize));
    } else {
        assert_invariant(!mContext.isES2());
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        glDrawElementsInstanced(GLenum(rp->type), (GLsizei)indexCount,
                rp->gl.getIndicesType(),
                reinterpret_cast<const void*>(indexOffset * rp->gl.indicesSize),
                (GLsizei)instanceCount);
#endif
    }

#ifdef FILAMENT_ENABLE_MATDBG
    CHECK_GL_ERROR_NON_FATAL(utils::slog.e)
#else
    CHECK_GL_ERROR(utils::slog.e)
#endif
}

void OpenGLDriver::dispatchCompute(Handle<HwProgram> program, math::uint3 workGroupCount) {
    getShaderCompilerService().tick();

    OpenGLProgram* const p = handle_cast<OpenGLProgram*>(program);

    bool const success = useProgram(p);
    if (UTILS_UNLIKELY(!success)) {
        // Avoid fatal (or cascading) errors that can occur during the draw call when the program
        // is invalid. The shader compile error has already been dumped to the console at this
        // point, so it's fine to simply return early.
        return;
    }

#if defined(BACKEND_OPENGL_LEVEL_GLES31)

#if defined(__ANDROID__)
    // on Android, GLES3.1 and above entry-points are defined in glext
    // (this is temporary, until we phase-out API < 21)
    using glext::glDispatchCompute;
#endif

    glDispatchCompute(workGroupCount.x, workGroupCount.y, workGroupCount.z);
#endif // BACKEND_OPENGL_LEVEL_GLES31

#ifdef FILAMENT_ENABLE_MATDBG
    CHECK_GL_ERROR_NON_FATAL(utils::slog.e)
#else
    CHECK_GL_ERROR(utils::slog.e)
#endif
}

// explicit instantiation of the Dispatcher
template class ConcreteDispatcher<OpenGLDriver>;

} // namespace filament::backend

#pragma clang diagnostic pop
