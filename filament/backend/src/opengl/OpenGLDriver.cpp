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

#include "CommandStreamDispatcher.h"
#include "GLTexture.h"
#include "GLUtils.h"
#include "OpenGLContext.h"
#include "OpenGLDriverFactory.h"
#include "OpenGLProgram.h"
#include "OpenGLTimerQuery.h"
#include "SystraceProfile.h"
#include "gl_headers.h"

#include <backend/platforms/OpenGLPlatform.h>

#include <backend/BufferDescriptor.h>
#include <backend/CallbackHandler.h>
#include <backend/DescriptorSetOffsetArray.h>
#include <backend/DriverApiForward.h>
#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/PipelineState.h>
#include <backend/Platform.h>
#include <backend/Program.h>
#include <backend/TargetBufferInfo.h>

#include "private/backend/CommandStream.h"
#include "private/backend/Dispatcher.h"
#include "private/backend/DriverApi.h"

#include <type_traits>
#include <utils/BitmaskEnum.h>
#include <utils/FixedCapacityVector.h>
#include <utils/CString.h>
#include <utils/Invocable.h>
#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/Systrace.h>
#include <utils/Slice.h>
#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/ostream.h>

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/mat3.h>

#include <algorithm>
#include <chrono>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <new>
#include <type_traits>
#include <utility>
#include <variant>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#endif

// We can only support this feature on OpenGL ES 3.1+
// Support is currently disabled as we don't need it
#define TEXTURE_2D_MULTISAMPLE_SUPPORTED false

#if defined(__EMSCRIPTEN__)
#define HAS_MAPBUFFERS 0
#else
#define HAS_MAPBUFFERS 1
#endif

#define DEBUG_GROUP_MARKER_NONE       0x00    // no debug marker
#define DEBUG_GROUP_MARKER_OPENGL     0x01    // markers in the gl command queue (req. driver support)
#define DEBUG_GROUP_MARKER_BACKEND    0x02    // markers on the backend side (systrace)
#define DEBUG_GROUP_MARKER_ALL        0xFF    // all markers

#define DEBUG_MARKER_NONE             0x00    // no debug marker
#define DEBUG_MARKER_OPENGL           0x01    // markers in the gl command queue (req. driver support)
#define DEBUG_MARKER_BACKEND          0x02    // markers on the backend side (systrace)
#define DEBUG_MARKER_PROFILE          0x04    // profiling on the backend side (systrace)
#define DEBUG_MARKER_ALL              (0xFF & ~DEBUG_MARKER_PROFILE) // all markers

// set to the desired debug marker level (for user markers [default: All])
#define DEBUG_GROUP_MARKER_LEVEL      DEBUG_GROUP_MARKER_ALL

// set to the desired debug level (for internal debugging [Default: None])
#define DEBUG_MARKER_LEVEL            DEBUG_MARKER_NONE

// Override the debug markers if we are forcing profiling mode
#if defined(FILAMENT_FORCE_PROFILING_MODE)
#   undef DEBUG_GROUP_MARKER_LEVEL
#   undef DEBUG_MARKER_LEVEL

#   define DEBUG_GROUP_MARKER_LEVEL   DEBUG_GROUP_MARKER_NONE
#   define DEBUG_MARKER_LEVEL         DEBUG_MARKER_PROFILE
#endif

#if DEBUG_MARKER_LEVEL == DEBUG_MARKER_PROFILE
#   define DEBUG_MARKER()
#   define PROFILE_MARKER(marker) PROFILE_SCOPE(marker);
#   if DEBUG_GROUP_MARKER_LEVEL != DEBUG_GROUP_MARKER_NONE
#      error PROFILING is exclusive; group markers must be disabled.
#   endif
#   ifndef NDEBUG
#      error PROFILING is meaningless in DEBUG mode.
#   endif
#elif DEBUG_MARKER_LEVEL > DEBUG_MARKER_NONE
#   define DEBUG_MARKER() DebugMarker _debug_marker(*this, __func__);
#   define PROFILE_MARKER(marker) DEBUG_MARKER()
#   if DEBUG_MARKER_LEVEL & DEBUG_MARKER_PROFILE
#      error PROFILING is exclusive; all other debug features must be disabled.
#   endif
#else
#   define DEBUG_MARKER()
#   define PROFILE_MARKER(marker)
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
OpenGLDriver* OpenGLDriver::create(OpenGLPlatform* const platform,
        void* const /*sharedGLContext*/, const Platform::DriverConfig& driverConfig) noexcept {
    assert_invariant(platform);
    OpenGLPlatform* const ec = platform;

#if 0
    // this is useful for development, but too verbose even for debug builds
    // For reference on a 64-bits machine in Release mode:
    //    GLIndexBuffer             :   8       moderate
    //    GLSwapChain               :  16       few
    //    GLTimerQuery              :  16       few
    //    GLFence                   :  24       few
    //    GLRenderPrimitive         :  32       many
    //    GLBufferObject            :  32       many
    // -- less than or equal 32 bytes
    //    GLTexture                 :  64       moderate
    //    GLVertexBuffer            :  76       moderate
    //    OpenGLProgram             :  96       moderate
    // -- less than or equal 96 bytes
    //    GLStream                  : 104       few
    //    GLRenderTarget            : 112       few
    //    GLVertexBufferInfo        : 132       moderate
    // -- less than or equal to 136 bytes

    slog.d
           << "\nGLSwapChain: " << sizeof(GLSwapChain)
           << "\nGLBufferObject: " << sizeof(GLBufferObject)
           << "\nGLVertexBuffer: " << sizeof(GLVertexBuffer)
           << "\nGLVertexBufferInfo: " << sizeof(GLVertexBufferInfo)
           << "\nGLIndexBuffer: " << sizeof(GLIndexBuffer)
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
    if (UTILS_UNLIKELY(driverConfig.forceGLES2Context)) {
        major = 2;
        minor = 0;
    }
#else
    // we require GL 4.1 headers and minimum version
    if (UTILS_UNLIKELY(!((major == 4 && minor >= 1) || major > 4))) {
        PANIC_LOG("OpenGL 4.1 minimum needed (current %d.%d)", major, minor);
        goto cleanup;
    }
#endif

    constexpr size_t defaultSize = FILAMENT_OPENGL_HANDLE_ARENA_SIZE_IN_MB * 1024U * 1024U;
    Platform::DriverConfig validConfig{ driverConfig };
    validConfig.handleArenaSize = std::max(driverConfig.handleArenaSize, defaultSize);
    OpenGLDriver* const driver = new(std::nothrow) OpenGLDriver(ec, validConfig);
    return driver;
}

// ------------------------------------------------------------------------------------------------

OpenGLDriver::DebugMarker::DebugMarker(OpenGLDriver& driver, const char* string) noexcept
        : driver(driver) {
#ifndef __EMSCRIPTEN__
#ifdef GL_EXT_debug_marker
#if DEBUG_MARKER_LEVEL & DEBUG_MARKER_OPENGL
    if (UTILS_LIKELY(driver.getContext().ext.EXT_debug_marker)) {
        glPushGroupMarkerEXT(GLsizei(strlen(string)), string);
    }
#endif
#endif

#if DEBUG_MARKER_LEVEL & DEBUG_MARKER_BACKEND
    SYSTRACE_CONTEXT();
    SYSTRACE_NAME_BEGIN(string);
#endif
#endif
}

OpenGLDriver::DebugMarker::~DebugMarker() noexcept {
#ifndef __EMSCRIPTEN__
#ifdef GL_EXT_debug_marker
#if DEBUG_MARKER_LEVEL & DEBUG_MARKER_OPENGL
    if (UTILS_LIKELY(driver.getContext().ext.EXT_debug_marker)) {
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

// ------------------------------------------------------------------------------------------------

OpenGLDriver::OpenGLDriver(OpenGLPlatform* platform, const Platform::DriverConfig& driverConfig) noexcept
        : mPlatform(*platform),
          mContext(mPlatform, driverConfig),
          mShaderCompilerService(*this),
          mHandleAllocator("Handles",
                  driverConfig.handleArenaSize,
                  driverConfig.disableHandleUseAfterFreeCheck,
                  driverConfig.disableHeapHandleTags),
          mDriverConfig(driverConfig),
          mCurrentPushConstants(new(std::nothrow) PushConstantBundle{}) {
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

    mShaderCompilerService.init();
}

OpenGLDriver::~OpenGLDriver() noexcept { // NOLINT(modernize-use-equals-default)
    // this is called from the main thread. Can't call GL.
}

Dispatcher OpenGLDriver::getDispatcher() const noexcept {
    auto dispatcher = ConcreteDispatcher<OpenGLDriver>::make();
    if (mContext.isES2()) {
        dispatcher.draw2_ = +[](Driver& driver, CommandBase* base, intptr_t* next){
            using Cmd = COMMAND_TYPE(draw2);
            OpenGLDriver& concreteDriver = static_cast<OpenGLDriver&>(driver);
            Cmd::execute(&OpenGLDriver::draw2GLES2, concreteDriver, base, next);
        };
    }
    return dispatcher;
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
#endif

    delete mCurrentPushConstants;
    mCurrentPushConstants = nullptr;

    mContext.terminate();

    mPlatform.terminate();
}

ShaderModel OpenGLDriver::getShaderModel() const noexcept {
    return mContext.getShaderModel();
}

ShaderLanguage OpenGLDriver::getShaderLanguage() const noexcept {
    return mContext.isES2() ? ShaderLanguage::ESSL1 : ShaderLanguage::ESSL3;
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

void OpenGLDriver::setPushConstant(ShaderStage stage, uint8_t index,
        PushConstantVariant value) {
    assert_invariant(stage == ShaderStage::VERTEX || stage == ShaderStage::FRAGMENT);

#if FILAMENT_ENABLE_MATDBG
    if (UTILS_UNLIKELY(!mValidProgram)) {
        return;
    }
#endif

    Slice<std::pair<GLint, ConstantType>> constants;
    if (stage == ShaderStage::VERTEX) {
        constants = mCurrentPushConstants->vertexConstants;
    } else if (stage == ShaderStage::FRAGMENT) {
        constants = mCurrentPushConstants->fragmentConstants;
    }

    assert_invariant(index < constants.size());
    auto const& [location, type] = constants[index];

    // This push constant wasn't found in the shader. It's ok to return without error-ing here.
    if (location < 0) {
        return;
    }

    if (std::holds_alternative<bool>(value)) {
        assert_invariant(type == ConstantType::BOOL);
        bool const bval = std::get<bool>(value);
        glUniform1i(location, bval ? 1 : 0);
    } else if (std::holds_alternative<float>(value)) {
        assert_invariant(type == ConstantType::FLOAT);
        float const fval = std::get<float>(value);
        glUniform1f(location, fval);
    } else {
        assert_invariant(type == ConstantType::INT);
        int const ival = std::get<int>(value);
        glUniform1i(location, ival);
    }
}

void OpenGLDriver::bindTexture(GLuint unit, GLTexture const* t) noexcept {
    assert_invariant(t != nullptr);
    mContext.bindTexture(unit, t->gl.target, t->gl.id, t->gl.external);
}

bool OpenGLDriver::useProgram(OpenGLProgram* p) noexcept {
    bool success = true;
    if (mBoundProgram != p) {
        // compile/link the program if needed and call glUseProgram
        success = p->use(this, mContext);
        assert_invariant(success == p->isValid());
        if (success) {
            // TODO: we could even improve this if the program could tell us which of the descriptors
            //       bindings actually changed. In practice, it is likely that set 0 or 1 might not
            //       change often.
            decltype(mInvalidDescriptorSetBindings) changed;
            changed.setValue((1 << MAX_DESCRIPTOR_SET_COUNT) - 1);
            mInvalidDescriptorSetBindings |= changed;

            mBoundProgram = p;
        }
    }

    if (UTILS_UNLIKELY(mContext.isES2() && success)) {
        // Set the output colorspace for this program (linear or rec709). This is only relevant
        // when mPlatform.isSRGBSwapChainSupported() is false (no need to check though).
        p->setRec709ColorSpace(mRec709OutputColorspace);
    }

    return success;
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

    if (gl.ext.EXT_depth_clamp) {
        if (rs.depthClamp) {
            gl.enable(GL_DEPTH_CLAMP);
        } else {
            gl.disable(GL_DEPTH_CLAMP);
        }
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

Handle<HwVertexBufferInfo> OpenGLDriver::createVertexBufferInfoS() noexcept {
    return initHandle<GLVertexBufferInfo>();
}

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

Handle<HwTexture> OpenGLDriver::createTextureS() noexcept {
    return initHandle<GLTexture>();
}

Handle<HwTexture> OpenGLDriver::createTextureViewS() noexcept {
    return initHandle<GLTexture>();
}

Handle<HwTexture> OpenGLDriver::createTextureViewSwizzleS() noexcept {
    return initHandle<GLTexture>();
}

Handle<HwTexture> OpenGLDriver::createTextureExternalImage2S() noexcept {
    return initHandle<GLTexture>();
}

Handle<HwTexture> OpenGLDriver::createTextureExternalImageS() noexcept {
    return initHandle<GLTexture>();
}

Handle<HwTexture> OpenGLDriver::createTextureExternalImagePlaneS() noexcept {
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

Handle<HwDescriptorSetLayout> OpenGLDriver::createDescriptorSetLayoutS() noexcept {
    return initHandle<GLDescriptorSetLayout>();
}

Handle<HwDescriptorSet> OpenGLDriver::createDescriptorSetS() noexcept {
    return initHandle<GLDescriptorSet>();
}

void OpenGLDriver::createVertexBufferInfoR(
        Handle<HwVertexBufferInfo> vbih,
        uint8_t bufferCount,
        uint8_t attributeCount,
        AttributeArray attributes) {
    DEBUG_MARKER()
    construct<GLVertexBufferInfo>(vbih, bufferCount, attributeCount, attributes);
}

void OpenGLDriver::createVertexBufferR(
        Handle<HwVertexBuffer> vbh,
        uint32_t vertexCount,
        Handle<HwVertexBufferInfo> vbih) {
    DEBUG_MARKER()
    construct<GLVertexBuffer>(vbh, vertexCount, vbih);
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
        bo->gl.binding = getBufferBindingType(bindingType);
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

    GLIndexBuffer const* const ib = handle_cast<const GLIndexBuffer*>(ibh);
    assert_invariant(ib->elementSize == 2 || ib->elementSize == 4);

    GLVertexBuffer* const vb = handle_cast<GLVertexBuffer*>(vbh);
    GLRenderPrimitive* const rp = handle_cast<GLRenderPrimitive*>(rph);
    rp->gl.indicesShift = (ib->elementSize == 4u) ? 2u : 1u;
    rp->gl.indicesType  = (ib->elementSize == 4u) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
    rp->gl.vertexBufferWithObjects = vbh;
    rp->type = pt;
    rp->vbih = vb->vbih;

    // create a name for this VAO in the current context
    gl.procs.genVertexArrays(1, &rp->gl.vao[gl.contextIndex]);

    // this implies our name is up-to-date
    rp->gl.nameVersion = gl.state.age;

    // binding the VAO will actually create it
    gl.bindVertexArray(&rp->gl);

    // Note: we don't update the vertex buffer bindings in the VAO just yet, we will do that
    // later in draw() or bindRenderPrimitive(). At this point, the HwVertexBuffer might not
    // have all its buffers set.

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

UTILS_NOINLINE
void OpenGLDriver::textureStorage(GLTexture* t,
        uint32_t width, uint32_t height, uint32_t depth, bool useProtectedMemory) noexcept {

    auto& gl = mContext;

    bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
    gl.activeTexture(OpenGLContext::DUMMY_TEXTURE_BINDING);

#ifdef GL_EXT_protected_textures
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    if (UTILS_UNLIKELY(useProtectedMemory)) {
        assert_invariant(gl.ext.EXT_protected_textures);
        glTexParameteri(t->gl.target, GL_TEXTURE_PROTECTED_EXT, 1);
    }
#endif
#endif

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

    if (any(usage & TextureUsage::PROTECTED)) {
        // renderbuffers don't have a protected mode, so we need to use a texture
        // because protected textures are only supported on GLES 3.2, MSAA will be available.
        usage |= TextureUsage::SAMPLEABLE;
    } else if (any(usage & TextureUsage::UPLOADABLE)) {
        // if we have the uploadable flag, we also need to use a texture
        usage |= TextureUsage::SAMPLEABLE;
    } else if (target != SamplerType::SAMPLER_2D) {
        // renderbuffers can only be 2D
        usage |= TextureUsage::SAMPLEABLE;
    } else if (levels > 1) {
        // renderbuffers can't have mip levels
        usage |= TextureUsage::SAMPLEABLE;
    }

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
                if (target == SamplerType::SAMPLER_EXTERNAL) {
                    if (UTILS_LIKELY(gl.ext.OES_EGL_image_external_essl3)) {
                        t->externalTexture->target = GL_TEXTURE_EXTERNAL_OES;
                    } else {
                        // revert to texture 2D if external is not supported; what else can we do?
                        t->externalTexture->target = GL_TEXTURE_2D;
                    }
                } else {
                    t->externalTexture->target = getTextureTargetNotExternal(target);
                }

                t->gl.target = t->externalTexture->target;
                t->gl.id = t->externalTexture->id;
                // internalFormat actually depends on the external image, but it doesn't matter
                // because it's not used anywhere for anything important.
                t->gl.internalFormat = internalFormat;
                t->gl.baseLevel = 0;
                t->gl.maxLevel = 0;
            }
        } else {
            glGenTextures(1, &t->gl.id);

            t->gl.internalFormat = internalFormat;

            t->gl.target = getTextureTargetNotExternal(target);

            if (t->samples > 1) {
                // Note: we can't be here in practice because filament's user API doesn't
                // allow the creation of multi-sampled textures.
#if defined(BACKEND_OPENGL_LEVEL_GLES31)
                if (gl.features.multisample_texture) {
                    // multi-sample texture on GL 3.2 / GLES 3.1 and above
                    t->gl.target = GL_TEXTURE_2D_MULTISAMPLE;
                } else {
                    // Turn off multi-sampling for that texture. It's just not supported.
                }
#endif
            }

            textureStorage(t, w, h, depth, bool(usage & TextureUsage::PROTECTED));
        }
    } else {
        t->gl.internalFormat = internalFormat;
        t->gl.target = GL_RENDERBUFFER;
        glGenRenderbuffers(1, &t->gl.id);
        renderBufferStorage(t->gl.id, internalFormat, w, h, samples);
    }

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createTextureViewR(Handle<HwTexture> th,
        Handle<HwTexture> srch, uint8_t baseLevel, uint8_t levelCount) {
    DEBUG_MARKER()
    GLTexture const* const src = handle_cast<GLTexture const*>(srch);

    FILAMENT_CHECK_PRECONDITION(any(src->usage & TextureUsage::SAMPLEABLE))
            << "TextureView can only be created on a SAMPLEABLE texture";

    FILAMENT_CHECK_PRECONDITION(!src->gl.imported)
            << "TextureView can't be created on imported textures";

    if (!src->ref) {
        // lazily create the ref handle, because most textures will never get a texture view
        src->ref = initHandle<GLTextureRef>();
    }

    GLTexture* t = construct<GLTexture>(th,
            src->target,
            src->levels,
            src->samples,
            src->width, src->height, src->depth,
            src->format,
            src->usage);

    t->gl = src->gl;
    t->gl.sidecarRenderBufferMS = 0;
    t->gl.sidecarSamples = 1;

    auto srcBaseLevel = src->gl.baseLevel;
    auto srcMaxLevel = src->gl.maxLevel;
    if (srcBaseLevel > srcMaxLevel) {
        srcBaseLevel = 0;
        srcMaxLevel = 127;
    }
    t->gl.baseLevel = (int8_t)std::min(127, srcBaseLevel + baseLevel);
    t->gl.maxLevel  = (int8_t)std::min(127, srcBaseLevel + baseLevel + levelCount - 1);

    // increase reference count to this texture handle
    t->ref = src->ref;
    GLTextureRef* ref = handle_cast<GLTextureRef*>(t->ref);
    assert_invariant(ref);
    ref->count++;

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createTextureViewSwizzleR(Handle<HwTexture> th, Handle<HwTexture> srch,
        TextureSwizzle r, TextureSwizzle g, TextureSwizzle b, TextureSwizzle a) {

    DEBUG_MARKER()
    GLTexture const* const src = handle_cast<GLTexture const*>(srch);

    FILAMENT_CHECK_PRECONDITION(any(src->usage & TextureUsage::SAMPLEABLE))
                    << "TextureView can only be created on a SAMPLEABLE texture";

    FILAMENT_CHECK_PRECONDITION(!src->gl.imported)
                    << "TextureView can't be created on imported textures";

    if (!src->ref) {
        // lazily create the ref handle, because most textures will never get a texture view
        src->ref = initHandle<GLTextureRef>();
    }

    GLTexture* t = construct<GLTexture>(th,
            src->target,
            src->levels,
            src->samples,
            src->width, src->height, src->depth,
            src->format,
            src->usage);

    t->gl = src->gl;
    t->gl.baseLevel = src->gl.baseLevel;
    t->gl.maxLevel = src->gl.maxLevel;
    t->gl.sidecarRenderBufferMS = 0;
    t->gl.sidecarSamples = 1;

    auto getChannel = [&swizzle = src->gl.swizzle](TextureSwizzle ch) {
        switch (ch) {
            case TextureSwizzle::SUBSTITUTE_ZERO:
            case TextureSwizzle::SUBSTITUTE_ONE:
                return ch;
            case TextureSwizzle::CHANNEL_0:
                return swizzle[0];
            case TextureSwizzle::CHANNEL_1:
                return swizzle[1];
            case TextureSwizzle::CHANNEL_2:
                return swizzle[2];
            case TextureSwizzle::CHANNEL_3:
                return swizzle[3];
        }
    };

    t->gl.swizzle = {
            getChannel(r),
            getChannel(g),
            getChannel(b),
            getChannel(a),
    };

    // increase reference count to this texture handle
    t->ref = src->ref;
    GLTextureRef* ref = handle_cast<GLTextureRef*>(t->ref);
    assert_invariant(ref);
    ref->count++;

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::createTextureExternalImage2R(Handle<HwTexture> th, SamplerType target,
    TextureFormat format, uint32_t width, uint32_t height, TextureUsage usage,
    Platform::ExternalImageHandleRef image) {
    DEBUG_MARKER()

    usage |= TextureUsage::SAMPLEABLE;
    usage &= ~TextureUsage::UPLOADABLE;

    auto& gl = mContext;
    GLenum internalFormat = getInternalFormat(format);
    if (UTILS_UNLIKELY(gl.isES2())) {
        // on ES2, format and internal format must match
        // FIXME: handle compressed texture format
        internalFormat = textureFormatToFormatAndType(format).first;
    }
    assert_invariant(internalFormat);

    GLTexture* const t = construct<GLTexture>(th, target, 1, 1, width, height, 1, format, usage);
    assert_invariant(t);

    t->externalTexture = mPlatform.createExternalImageTexture();
    if (t->externalTexture) {
        if (target == SamplerType::SAMPLER_EXTERNAL) {
            if (UTILS_LIKELY(gl.ext.OES_EGL_image_external_essl3)) {
                t->externalTexture->target = GL_TEXTURE_EXTERNAL_OES;
            } else {
                // revert to texture 2D if external is not supported; what else can we do?
                t->externalTexture->target = GL_TEXTURE_2D;
            }
        } else {
            t->externalTexture->target = getTextureTargetNotExternal(target);
        }

        t->gl.target = t->externalTexture->target;
        t->gl.id = t->externalTexture->id;
        // internalFormat actually depends on the external image, but it doesn't matter
        // because it's not used anywhere for anything important.
        t->gl.internalFormat = internalFormat;
        t->gl.baseLevel = 0;
        t->gl.maxLevel = 0;
        t->gl.external = true; // forces bindTexture() call (they're never cached)
    }

    bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
    if (mPlatform.setExternalImage(image, t->externalTexture)) {
        // the target and id can be reset each time
        t->gl.target = t->externalTexture->target;
        t->gl.id = t->externalTexture->id;
    }
}

void OpenGLDriver::createTextureExternalImageR(Handle<HwTexture> th, SamplerType target,
    TextureFormat format, uint32_t width, uint32_t height, TextureUsage usage, void* image) {
    DEBUG_MARKER()

    usage |= TextureUsage::SAMPLEABLE;
    usage &= ~TextureUsage::UPLOADABLE;

    auto& gl = mContext;
    GLenum internalFormat = getInternalFormat(format);
    if (UTILS_UNLIKELY(gl.isES2())) {
        // on ES2, format and internal format must match
        // FIXME: handle compressed texture format
        internalFormat = textureFormatToFormatAndType(format).first;
    }
    assert_invariant(internalFormat);

    GLTexture* const t = construct<GLTexture>(th, target, 1, 1, width, height, 1, format, usage);
    assert_invariant(t);

    t->externalTexture = mPlatform.createExternalImageTexture();
    if (t->externalTexture) {
        if (target == SamplerType::SAMPLER_EXTERNAL) {
            if (UTILS_LIKELY(gl.ext.OES_EGL_image_external_essl3)) {
                t->externalTexture->target = GL_TEXTURE_EXTERNAL_OES;
            } else {
                // revert to texture 2D if external is not supported; what else can we do?
                t->externalTexture->target = GL_TEXTURE_2D;
            }
        } else {
            t->externalTexture->target = getTextureTargetNotExternal(target);
        }
        t->gl.target = t->externalTexture->target;
        t->gl.id = t->externalTexture->id;
        // internalFormat actually depends on the external image, but it doesn't matter
        // because it's not used anywhere for anything important.
        t->gl.internalFormat = internalFormat;
        t->gl.baseLevel = 0;
        t->gl.maxLevel = 0;
        t->gl.external = true; // forces bindTexture() call (they're never cached)
    }

    bindTexture(OpenGLContext::DUMMY_TEXTURE_BINDING, t);
    if (mPlatform.setExternalImage(image, t->externalTexture)) {
        // the target and id can be reset each time
        t->gl.target = t->externalTexture->target;
        t->gl.id = t->externalTexture->id;
    }
}

void OpenGLDriver::createTextureExternalImagePlaneR(Handle<HwTexture> th,
        TextureFormat format, uint32_t width, uint32_t height, TextureUsage usage,
        void* image, uint32_t plane) {
    // not relevant for the OpenGL backend
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

    switch (target) {
        case SamplerType::SAMPLER_EXTERNAL:
            t->gl.target = GL_TEXTURE_EXTERNAL_OES;
            t->gl.external = true; // forces bindTexture() call (they're never cached)
            break;
        case SamplerType::SAMPLER_2D:
            t->gl.target = GL_TEXTURE_2D;
            break;
        case SamplerType::SAMPLER_3D:
            t->gl.target = GL_TEXTURE_3D;
            break;
        case SamplerType::SAMPLER_2D_ARRAY:
            t->gl.target = GL_TEXTURE_2D_ARRAY;
            break;
        case SamplerType::SAMPLER_CUBEMAP:
            t->gl.target = GL_TEXTURE_CUBE_MAP;
            break;
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            t->gl.target = GL_TEXTURE_CUBE_MAP_ARRAY;
            break;
    }

    if (t->samples > 1) {
        // Note: we can't be here in practice because filament's user API doesn't
        // allow the creation of multi-sampled textures.
#if defined(BACKEND_OPENGL_LEVEL_GLES31)
        if (gl.features.multisample_texture) {
            // multi-sample texture on GL 3.2 / GLES 3.1 and above
            t->gl.target = GL_TEXTURE_2D_MULTISAMPLE;
        } else {
            // Turn off multi-sampling for that texture. It's just not supported.
        }
#endif
    }

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::updateVertexArrayObject(GLRenderPrimitive* rp, GLVertexBuffer const* vb) {
    // NOTE: this is called often and must be as efficient as possible.

    auto& gl = mContext;

#ifndef NDEBUG
    if (UTILS_LIKELY(gl.ext.OES_vertex_array_object)) {
        // The VAO for the given render primitive must already be bound.
        GLint vaoBinding;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vaoBinding);
        assert_invariant(vaoBinding == (GLint)rp->gl.vao[gl.contextIndex]);
    }
#endif

    if (UTILS_LIKELY(rp->gl.vertexBufferVersion == vb->bufferObjectsVersion &&
                     rp->gl.stateVersion == gl.state.age)) {
        return;
    }

    GLVertexBufferInfo const* const vbi = handle_cast<const GLVertexBufferInfo*>(vb->vbih);

    for (size_t i = 0, n = vbi->attributes.size(); i < n; i++) {
        const auto& attribute = vbi->attributes[i];
        const uint8_t bi = attribute.buffer;
        if (bi != Attribute::BUFFER_UNUSED) {
            // if a buffer is defined it must not be invalid.
            assert_invariant(vb->gl.buffers[bi]);

            // if we're on ES2, the user shouldn't use FLAG_INTEGER_TARGET
            assert_invariant(!(gl.isES2() && (attribute.flags & Attribute::FLAG_INTEGER_TARGET)));

            gl.bindBuffer(GL_ARRAY_BUFFER, vb->gl.buffers[bi]);
            GLuint const index = i;
            GLint const size = (GLint)getComponentCount(attribute.type);
            GLenum const type = getComponentType(attribute.type);
            GLboolean const normalized = getNormalization(attribute.flags & Attribute::FLAG_NORMALIZED);
            GLsizei const stride = attribute.stride;
            void const* pointer = reinterpret_cast<void const *>(attribute.offset);

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
            if (UTILS_UNLIKELY(attribute.flags & Attribute::FLAG_INTEGER_TARGET)) {
                // integer attributes can't be floats
                assert_invariant(type == GL_BYTE || type == GL_UNSIGNED_BYTE || type == GL_SHORT ||
                        type == GL_UNSIGNED_SHORT || type == GL_INT || type == GL_UNSIGNED_INT);
                glVertexAttribIPointer(index, size, type, stride, pointer);
            } else
#endif
            {
                glVertexAttribPointer(index, size, type, normalized, stride, pointer);
            }

            gl.enableVertexAttribArray(&rp->gl, GLuint(i));
        } else {
            // In some OpenGL implementations, we must supply a properly-typed placeholder for
            // every integer input that is declared in the vertex shader.
            // Note that the corresponding doesn't have to be enabled and in fact won't be. If it
            // was enabled, it would indicate a user-error (providing the wrong type of array).
            // With a disabled array, the vertex shader gets the attribute from glVertexAttrib,
            // and must have the proper intergerness.
            // But at this point, we don't know what the shader requirements are, and so we must
            // rely on the attribute.

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

            gl.disableVertexAttribArray(&rp->gl, GLuint(i));
        }
    }

    rp->gl.stateVersion = gl.state.age;
    if (UTILS_LIKELY(gl.ext.OES_vertex_array_object)) {
        rp->gl.vertexBufferVersion = vb->bufferObjectsVersion;
    } else {
        // if we don't have OES_vertex_array_object, we never update the buffer version so
        // that it's always reset in draw
    }
}

void OpenGLDriver::framebufferTexture(TargetBufferInfo const& binfo,
        GLRenderTarget const* rt, GLenum attachment, uint8_t layerCount) noexcept {

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
            case SamplerType::SAMPLER_3D:
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
            case SamplerType::SAMPLER_EXTERNAL:
                // This is an error. We have asserted in debug build.
                target = t->gl.target;
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
        // On GL3.2 / GLES3.1 and above multisample is handled when creating the texture.
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
            case GL_TEXTURE_3D:
            case GL_TEXTURE_2D_ARRAY:
            case GL_TEXTURE_CUBE_MAP_ARRAY:
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2

                // TODO: support multiview for iOS and WebGL
#if !defined(__EMSCRIPTEN__) && !defined(FILAMENT_IOS)
                if (layerCount > 1) {
                    // if layerCount > 1, it means we use the multiview extension.
                    glFramebufferTextureMultiviewOVR(GL_FRAMEBUFFER, attachment,
                        t->gl.id, 0, binfo.layer, layerCount);
                } else
#endif // !defined(__EMSCRIPTEN__) && !defined(FILAMENT_IOS)
                {
                    // GL_TEXTURE_2D_MULTISAMPLE_ARRAY is not supported in GLES
                    glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment,
                        t->gl.id, binfo.level, binfo.layer);
                }
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
        // We have a multi-sample rendertarget, and we have EXT_multisampled_render_to_texture,
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
            case GL_TEXTURE_3D:
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

    GLRenderTarget* rt = handle_cast<GLRenderTarget*>(rth);
    rt->gl.isDefault = true;
    rt->gl.fbo = 0; // the actual id is resolved at binding time
    rt->gl.samples = 1;
    // FIXME: these flags should reflect the actual attachments present
    rt->targets = TargetBufferFlags::COLOR0 | TargetBufferFlags::DEPTH;
}

void OpenGLDriver::createRenderTargetR(Handle<HwRenderTarget> rth,
        TargetBufferFlags targets,
        uint32_t width,
        uint32_t height,
        uint8_t samples,
        uint8_t layerCount,
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

    UTILS_UNUSED_IN_RELEASE vec2<uint32_t> tmin = { std::numeric_limits<uint32_t>::max() };
    UTILS_UNUSED_IN_RELEASE vec2<uint32_t> tmax = { 0 };
    auto checkDimensions = [&tmin, &tmax](GLTexture const* t, uint8_t level) {
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
                framebufferTexture(color[i], rt, GL_COLOR_ATTACHMENT0 + i, layerCount);
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
            framebufferTexture(depth, rt, GL_DEPTH_STENCIL_ATTACHMENT, layerCount);
            specialCased = true;
            checkDimensions(rt->gl.depth, depth.level);
        }
    }
#endif

    if (!specialCased) {
        if (any(targets & TargetBufferFlags::DEPTH)) {
            assert_invariant(depth.handle);
            rt->gl.depth = handle_cast<GLTexture*>(depth.handle);
            framebufferTexture(depth, rt, GL_DEPTH_ATTACHMENT, layerCount);
            checkDimensions(rt->gl.depth, depth.level);
        }
        if (any(targets & TargetBufferFlags::STENCIL)) {
            assert_invariant(stencil.handle);
            rt->gl.stencil = handle_cast<GLTexture*>(stencil.handle);
            framebufferTexture(stencil, rt, GL_STENCIL_ATTACHMENT, layerCount);
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

#if !defined(__EMSCRIPTEN__)
    // note: in practice this should never happen on Android
    FILAMENT_CHECK_POSTCONDITION(sc->swapChain) << "createSwapChain(" << nativeWindow << ", "
                                                << flags << ") failed. See logs for details.";
#endif

    // See if we need the emulated rec709 output conversion
    if (UTILS_UNLIKELY(mContext.isES2())) {
        sc->rec709 = ((flags & SWAP_CHAIN_CONFIG_SRGB_COLORSPACE) &&
                !mPlatform.isSRGBSwapChainSupported());
    }
}

void OpenGLDriver::createSwapChainHeadlessR(Handle<HwSwapChain> sch,
        uint32_t width, uint32_t height, uint64_t flags) {
    DEBUG_MARKER()

    GLSwapChain* sc = handle_cast<GLSwapChain*>(sch);
    sc->swapChain = mPlatform.createSwapChain(width, height, flags);

#if !defined(__EMSCRIPTEN__)
    // note: in practice this should never happen on Android
    FILAMENT_CHECK_POSTCONDITION(sc->swapChain)
            << "createSwapChainHeadless(" << width << ", " << height << ", " << flags
            << ") failed. See logs for details.";
#endif

    // See if we need the emulated rec709 output conversion
    if (UTILS_UNLIKELY(mContext.isES2())) {
        sc->rec709 = (flags & SWAP_CHAIN_CONFIG_SRGB_COLORSPACE &&
                      !mPlatform.isSRGBSwapChainSupported());
    }
}

void OpenGLDriver::createTimerQueryR(Handle<HwTimerQuery> tqh, int) {
    DEBUG_MARKER()
    GLTimerQuery* tq = handle_cast<GLTimerQuery*>(tqh);
    mContext.createTimerQuery(tq);
}

void OpenGLDriver::createDescriptorSetLayoutR(Handle<HwDescriptorSetLayout> dslh,
        DescriptorSetLayout&& info) {
    DEBUG_MARKER()
    construct<GLDescriptorSetLayout>(dslh, std::move(info));
}

void OpenGLDriver::createDescriptorSetR(Handle<HwDescriptorSet> dsh,
        Handle<HwDescriptorSetLayout> dslh) {
    DEBUG_MARKER()
    GLDescriptorSetLayout const* dsl = handle_cast<GLDescriptorSetLayout*>(dslh);
    construct<GLDescriptorSet>(dsh, mContext, dslh, dsl);
}

// ------------------------------------------------------------------------------------------------
// Destroying driver objects
// ------------------------------------------------------------------------------------------------

void OpenGLDriver::destroyVertexBufferInfo(Handle<HwVertexBufferInfo> vbih) {
    DEBUG_MARKER()
    if (vbih) {
        GLVertexBufferInfo const* vbi = handle_cast<const GLVertexBufferInfo*>(vbih);
        destruct(vbih, vbi);
    }
}

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
        gl.deleteBuffer(ib->gl.buffer, GL_ELEMENT_ARRAY_BUFFER);
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
            gl.deleteBuffer(bo->gl.id, bo->gl.binding);
        }
        destruct(boh, bo);
    }
}

void OpenGLDriver::destroyRenderPrimitive(Handle<HwRenderPrimitive> rph) {
    DEBUG_MARKER()

    if (rph) {
        auto& gl = mContext;
        GLRenderPrimitive const* rp = handle_cast<const GLRenderPrimitive*>(rph);
        gl.deleteVertexArray(rp->gl.vao[gl.contextIndex]);

        // If we have a name in the "other" context, we need to schedule the destroy for
        // later, because it can't be done here. VAOs are "container objects" and are not
        // shared between contexts.
        size_t const otherContextIndex = 1 - gl.contextIndex;
        GLuint const nameInOtherContext = rp->gl.vao[otherContextIndex];
        if (UTILS_UNLIKELY(nameInOtherContext)) {
            gl.destroyWithContext(otherContextIndex,
                    [name = nameInOtherContext](OpenGLContext& gl) {
                gl.deleteVertexArray(name);
            });
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

void OpenGLDriver::destroyTexture(Handle<HwTexture> th) {
    DEBUG_MARKER()

    if (th) {
        auto& gl = mContext;
        GLTexture* t = handle_cast<GLTexture*>(th);

        if (UTILS_LIKELY(!t->gl.imported)) {
            if (UTILS_LIKELY(t->usage & TextureUsage::SAMPLEABLE)) {
                // drop a reference
                uint16_t count = 0;
                if (UTILS_UNLIKELY(t->ref)) {
                    // the common case is that we don't have a ref handle
                    GLTextureRef* const ref = handle_cast<GLTextureRef*>(t->ref);
                    count = --(ref->count);
                    if (count == 0) {
                        destruct(t->ref, ref);
                    }
                }
                if (count == 0) {
                    // if this was the last reference, we destroy the refcount as well as
                    // the GL texture name itself.
                    gl.unbindTexture(t->gl.target, t->gl.id);
                    if (UTILS_UNLIKELY(t->hwStream)) {
                        detachStream(t);
                    }
                    if (UTILS_UNLIKELY(t->target == SamplerType::SAMPLER_EXTERNAL)) {
                        mPlatform.destroyExternalImageTexture(t->externalTexture);
                    } else {
                        glDeleteTextures(1, &t->gl.id);
                    }
                } else {
                    // The Handle<HwTexture> is always destroyed. For extra precaution we also
                    // check that the GLTexture has a trivial destructor.
                    static_assert(std::is_trivially_destructible_v<GLTexture>);
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
            gl.unbindFramebuffer(GL_FRAMEBUFFER);
        }
        if (rt->gl.fbo_read) {
            // first unbind this framebuffer if needed
            gl.unbindFramebuffer(GL_FRAMEBUFFER);
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
        mContext.destroyTimerQuery(tq);
        destruct(tqh, tq);
    }
}

void OpenGLDriver::destroyDescriptorSetLayout(Handle<HwDescriptorSetLayout> dslh) {
    DEBUG_MARKER()
    if (dslh) {
        GLDescriptorSetLayout* dsl = handle_cast<GLDescriptorSetLayout*>(dslh);
        destruct(dslh, dsl);
    }
}

void OpenGLDriver::destroyDescriptorSet(Handle<HwDescriptorSet> dsh) {
    DEBUG_MARKER()
    if (dsh) {
        // unbind the descriptor-set, to avoid use-after-free
        for (auto& bound : mBoundDescriptorSets) {
            if (bound.dsh == dsh) {
                bound = {};
            }
        }
        GLDescriptorSet* ds = handle_cast<GLDescriptorSet*>(dsh);
        destruct(dsh, ds);
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

math::mat3f OpenGLDriver::getStreamTransformMatrix(Handle<HwStream> sh) {
    if (sh) {
        GLStream* s = handle_cast<GLStream*>(sh);        
        return mPlatform.getTransformMatrix(s->stream);
    }
    return math::mat3f();
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
    return TimerQueryFactory::isGpuTimeSupported();
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

bool OpenGLDriver::isProtectedContentSupported() {
    return mPlatform.isProtectedContextSupported();
}

bool OpenGLDriver::isStereoSupported() {
    // Instanced-stereo requires instancing and EXT_clip_cull_distance.
    // Multiview-stereo requires ES 3.0 and OVR_multiview2.
    if (UTILS_UNLIKELY(mContext.isES2())) {
        return false;
    }
    switch (mDriverConfig.stereoscopicType) {
        case StereoscopicType::INSTANCED:
            return mContext.ext.EXT_clip_cull_distance;
        case StereoscopicType::MULTIVIEW:
            return mContext.ext.OVR_multiview2;
        case StereoscopicType::NONE:
            return false;
    }
}

bool OpenGLDriver::isParallelShaderCompileSupported() {
    return mShaderCompilerService.isParallelShaderCompileSupported();
}

bool OpenGLDriver::isDepthStencilResolveSupported() {
    return true;
}

bool OpenGLDriver::isDepthStencilBlitSupported(TextureFormat) {
    return true;
}

bool OpenGLDriver::isProtectedTexturesSupported() {
    return getContext().ext.EXT_protected_textures;
}

bool OpenGLDriver::isDepthClampSupported() {
    return getContext().ext.EXT_depth_clamp;
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
        default:
            return false;
    }
    return false;
}

FeatureLevel OpenGLDriver::getFeatureLevel() {
    return mContext.getFeatureLevel();
}

float2 OpenGLDriver::getClipSpaceParams() {
    return mContext.ext.EXT_clip_control ?
           // z-coordinate of virtual and physical clip-space is in [-w, 0]
           float2{ 1.0f, 0.0f } :
           // z-coordinate of virtual clip-space is in [-w,0], physical is in [-w, w]
           float2{ 2.0f, -1.0f };
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

    mPlatform.makeCurrent(scDraw->swapChain, scRead->swapChain,
            [this]() {
                // OpenGL context is about to change, unbind everything
                mContext.unbindEverything();
            },
            [this](size_t index) {
                // OpenGL context has changed, resynchronize the state with the cache
                mContext.synchronizeStateAndCache(index);
                slog.d << "*** OpenGL context change : " << (index ? "protected" : "default") << io::endl;
            });

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

void OpenGLDriver::setDebugTag(HandleBase::HandleId handleId, CString tag) {
    mHandleAllocator.associateTagToHandle(handleId, std::move(tag));
}

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

    glGenerateMipmap(t->gl.target);

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::setTextureData(GLTexture const* t, uint32_t level,
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
    size_t const bpl = bpr * height; // TODO: PBD should have a "layer stride"
    void const* const buffer = static_cast<char const*>(p.buffer)
            + bpp* p.left + bpr * p.top + bpl * 0; // TODO: PBD should have a p.depth

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

    scheduleDestroy(std::move(p));

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::setCompressedTextureData(GLTexture const* t, uint32_t level,
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

    scheduleDestroy(std::move(p));

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLDriver::setupExternalImage2(Platform::ExternalImageHandleRef image) {
    mPlatform.retainExternalImage(image);
}

void OpenGLDriver::setupExternalImage(void* image) {
    mPlatform.retainExternalImage(image);
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
    mContext.beginTimeElapsedQuery(tq);
}

void OpenGLDriver::endTimerQuery(Handle<HwTimerQuery> tqh) {
    DEBUG_MARKER()
    GLTimerQuery* tq = handle_cast<GLTimerQuery*>(tqh);
    mContext.endTimeElapsedQuery(*this, tq);
}

TimerQueryResult OpenGLDriver::getTimerQueryValue(Handle<HwTimerQuery> tqh, uint64_t* elapsedTime) {
    GLTimerQuery* tq = handle_cast<GLTimerQuery*>(tqh);
    return TimerQueryFactoryInterface::getTimerQueryValue(tq, elapsedTime);
}

void OpenGLDriver::compilePrograms(CompilerPriorityQueue,
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

    GLuint const fbo = gl.bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);
    CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e, GL_FRAMEBUFFER)

    // each render-pass starts with a disabled scissor
    gl.disable(GL_SCISSOR_TEST);

    if (gl.ext.EXT_discard_framebuffer
            && !gl.bugs.disable_invalidate_framebuffer) {
        AttachmentArray attachments; // NOLINT
        GLsizei const attachmentCount = getAttachments(attachments, discardFlags, !fbo);
        if (attachmentCount) {
            gl.procs.invalidateFramebuffer(GL_FRAMEBUFFER, attachmentCount, attachments.data());
        }
        CHECK_GL_ERROR(utils::slog.e)
    } else {
        // It's important to clear the framebuffer before drawing, as it resets
        // the fb to a known state (resets fb compression and possibly other things).
        // So we use glClear instead of glInvalidateFramebuffer
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
            GLuint const fbo = gl.bindFramebuffer(GL_FRAMEBUFFER, rt->gl.fbo);
            AttachmentArray attachments; // NOLINT
            GLsizei const attachmentCount = getAttachments(attachments, effectiveDiscardFlags, !fbo);
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
        TargetBufferFlags buffers, bool isDefaultFramebuffer) noexcept {
    GLsizei attachmentCount = 0;
    // the default framebuffer uses different constants!!!

    if (any(buffers & TargetBufferFlags::COLOR0)) {
        attachments[attachmentCount++] = isDefaultFramebuffer ? GL_COLOR : GL_COLOR_ATTACHMENT0;
    }
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    if (any(buffers & TargetBufferFlags::COLOR1)) {
        assert_invariant(!isDefaultFramebuffer);
        attachments[attachmentCount++] = GL_COLOR_ATTACHMENT1;
    }
    if (any(buffers & TargetBufferFlags::COLOR2)) {
        assert_invariant(!isDefaultFramebuffer);
        attachments[attachmentCount++] = GL_COLOR_ATTACHMENT2;
    }
    if (any(buffers & TargetBufferFlags::COLOR3)) {
        assert_invariant(!isDefaultFramebuffer);
        attachments[attachmentCount++] = GL_COLOR_ATTACHMENT3;
    }
    if (any(buffers & TargetBufferFlags::COLOR4)) {
        assert_invariant(!isDefaultFramebuffer);
        attachments[attachmentCount++] = GL_COLOR_ATTACHMENT4;
    }
    if (any(buffers & TargetBufferFlags::COLOR5)) {
        assert_invariant(!isDefaultFramebuffer);
        attachments[attachmentCount++] = GL_COLOR_ATTACHMENT5;
    }
    if (any(buffers & TargetBufferFlags::COLOR6)) {
        assert_invariant(!isDefaultFramebuffer);
        attachments[attachmentCount++] = GL_COLOR_ATTACHMENT6;
    }
    if (any(buffers & TargetBufferFlags::COLOR7)) {
        assert_invariant(!isDefaultFramebuffer);
        attachments[attachmentCount++] = GL_COLOR_ATTACHMENT7;
    }
#endif
    if (any(buffers & TargetBufferFlags::DEPTH)) {
        attachments[attachmentCount++] = isDefaultFramebuffer ? GL_DEPTH : GL_DEPTH_ATTACHMENT;
    }
    if (any(buffers & TargetBufferFlags::STENCIL)) {
        attachments[attachmentCount++] = isDefaultFramebuffer ? GL_STENCIL : GL_STENCIL_ATTACHMENT;
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

void OpenGLDriver::insertEventMarker(char const* string) {
#ifndef __EMSCRIPTEN__
#ifdef GL_EXT_debug_marker
    auto& gl = mContext;
    if (gl.ext.EXT_debug_marker) {
        glInsertEventMarkerEXT(GLsizei(strlen(string)), string);
    }
#endif
#endif
}

void OpenGLDriver::pushGroupMarker(char const* string) {
#ifndef __EMSCRIPTEN__
#ifdef GL_EXT_debug_marker
#if DEBUG_GROUP_MARKER_LEVEL & DEBUG_GROUP_MARKER_OPENGL
    if (UTILS_LIKELY(mContext.ext.EXT_debug_marker)) {
        glPushGroupMarkerEXT(GLsizei(strlen(string)), string);
    }
#endif
#endif

#if DEBUG_GROUP_MARKER_LEVEL & DEBUG_GROUP_MARKER_BACKEND
    SYSTRACE_CONTEXT();
    SYSTRACE_NAME_BEGIN(string);
#endif
#endif
}

void OpenGLDriver::popGroupMarker(int) {
#ifndef __EMSCRIPTEN__
#ifdef GL_EXT_debug_marker
#if DEBUG_GROUP_MARKER_LEVEL & DEBUG_GROUP_MARKER_OPENGL
    if (UTILS_LIKELY(mContext.ext.EXT_debug_marker)) {
        glPopGroupMarkerEXT();
    }
#endif
#endif

#if DEBUG_GROUP_MARKER_LEVEL & DEBUG_GROUP_MARKER_BACKEND
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

void OpenGLDriver::readBufferSubData(BufferObjectHandle boh,
        uint32_t offset, uint32_t size, BufferDescriptor&& p) {
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
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    executeGpuCommandsCompleteOps();
#endif
    executeEveryNowAndThenOps();
    getShaderCompilerService().tick();
}

void OpenGLDriver::beginFrame(
        UTILS_UNUSED int64_t monotonic_clock_ns,
        UTILS_UNUSED int64_t refreshIntervalNs,
        UTILS_UNUSED uint32_t frameId) {
    PROFILE_MARKER(PROFILE_NAME_BEGINFRAME)
    auto& gl = mContext;
    insertEventMarker("beginFrame");
    mPlatform.beginFrame(monotonic_clock_ns, refreshIntervalNs, frameId);
    if (UTILS_UNLIKELY(!mTexturesWithStreamsAttached.empty())) {
        OpenGLPlatform& platform = mPlatform;
        for (GLTexture const* t : mTexturesWithStreamsAttached) {
            assert_invariant(t && t->hwStream);
            if (t->hwStream->streamType == StreamType::NATIVE) {
                assert_invariant(t->hwStream->stream);
                platform.updateTexImage(t->hwStream->stream,
                        &static_cast<GLStream*>(t->hwStream)->user_thread.timestamp); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
                // NOTE: We assume that OpenGLPlatform::updateTexImage() binds the texture on our behalf
                gl.updateTexImage(GL_TEXTURE_EXTERNAL_OES, t->gl.id);
            }
        }
    }
}

void OpenGLDriver::setFrameScheduledCallback(Handle<HwSwapChain>, CallbackHandler*,
        FrameScheduledCallback&& /*callback*/, uint64_t /*flags*/) {
    DEBUG_MARKER()
}

void OpenGLDriver::setFrameCompletedCallback(Handle<HwSwapChain>,
        CallbackHandler*, Invocable<void()>&& /*callback*/) {
    DEBUG_MARKER()
}

void OpenGLDriver::setPresentationTime(int64_t monotonic_clock_ns) {
    DEBUG_MARKER()
    mPlatform.setPresentationTime(monotonic_clock_ns);
}

void OpenGLDriver::endFrame(UTILS_UNUSED uint32_t frameId) {
    PROFILE_MARKER(PROFILE_NAME_ENDFRAME)
#if defined(__EMSCRIPTEN__)
    // WebGL builds are single-threaded so users might manipulate various GL state after we're
    // done with the frame. We do NOT officially support using Filament in this way, but we can
    // at least do some minimal safety things here, such as resetting the VAO to 0.
    auto& gl = mContext;
    gl.bindVertexArray(nullptr);
    for (int unit = OpenGLContext::DUMMY_TEXTURE_BINDING; unit >= 0; unit--) {
        gl.bindTexture(unit, GL_TEXTURE_2D, 0, false);
    }
    gl.disable(GL_CULL_FACE);
    gl.depthFunc(GL_LESS);
    gl.disable(GL_SCISSOR_TEST);
#endif
    //SYSTRACE_NAME("glFinish");
    //glFinish();
    mPlatform.endFrame(frameId);
    insertEventMarker("endFrame");
}

void OpenGLDriver::updateDescriptorSetBuffer(
        DescriptorSetHandle dsh,
        descriptor_binding_t binding,
        BufferObjectHandle boh,
        uint32_t offset, uint32_t size) {
    GLDescriptorSet* ds = handle_cast<GLDescriptorSet*>(dsh);
    GLBufferObject* bo = boh ? handle_cast<GLBufferObject*>(boh) : nullptr;
    ds->update(mContext, binding, bo, offset, size);
}

void OpenGLDriver::updateDescriptorSetTexture(
        DescriptorSetHandle dsh,
        descriptor_binding_t binding,
        TextureHandle th,
        SamplerParams params) {
    GLDescriptorSet* ds = handle_cast<GLDescriptorSet*>(dsh);
    GLTexture* t = th ? handle_cast<GLTexture*>(th) : nullptr;
    ds->update(mContext, binding, t, params);
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
        float4 const& linearColor, GLfloat depth, GLint stencil) noexcept {

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

    FILAMENT_CHECK_PRECONDITION(d->width == s->width && d->height == s->height)
            << "invalid resolve: src and dst sizes don't match";

    FILAMENT_CHECK_PRECONDITION(s->samples > 1 && d->samples == 1)
            << "invalid resolve: src.samples=" << +s->samples << ", dst.samples=" << +d->samples;

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
    CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e, GL_DRAW_FRAMEBUFFER)

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
    CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e, GL_READ_FRAMEBUFFER)

    gl.disable(GL_SCISSOR_TEST);
    glBlitFramebuffer(
            srcOrigin.x, srcOrigin.y, srcOrigin.x + size.x, srcOrigin.y + size.y,
            dstOrigin.x, dstOrigin.y, dstOrigin.x + size.x, dstOrigin.y + size.y,
            mask, GL_NEAREST);
    CHECK_GL_ERROR(utils::slog.e)

    gl.unbindFramebuffer(GL_DRAW_FRAMEBUFFER);
    gl.unbindFramebuffer(GL_READ_FRAMEBUFFER);
    glDeleteFramebuffers(2, fbo);
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

    FILAMENT_CHECK_PRECONDITION(buffers == TargetBufferFlags::COLOR0)
            << "blitDEPRECATED only supports COLOR0";

    FILAMENT_CHECK_PRECONDITION(
            srcRect.left >= 0 && srcRect.bottom >= 0 && dstRect.left >= 0 && dstRect.bottom >= 0)
            << "Source and destination rects must be positive.";

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

void OpenGLDriver::bindPipeline(PipelineState const& state) {
    DEBUG_MARKER()
    auto& gl = mContext;
    setRasterState(state.rasterState);
    setStencilState(state.stencilState);
    gl.polygonOffset(state.polygonOffset.slope, state.polygonOffset.constant);
    OpenGLProgram* const p = handle_cast<OpenGLProgram*>(state.program);
    mValidProgram = useProgram(p);
    (*mCurrentPushConstants) = p->getPushConstants();
    mCurrentSetLayout = state.pipelineLayout.setLayout;
    // TODO: we should validate that the pipeline layout matches the program's
}

void OpenGLDriver::bindRenderPrimitive(Handle<HwRenderPrimitive> rph) {
    DEBUG_MARKER()
    auto& gl = mContext;

    GLRenderPrimitive* const rp = handle_cast<GLRenderPrimitive*>(rph);

    // Gracefully do nothing if the render primitive has not been set up.
    VertexBufferHandle vb = rp->gl.vertexBufferWithObjects;
    if (UTILS_UNLIKELY(!vb)) {
        mBoundRenderPrimitive = nullptr;
        return;
    }

    // If necessary, mutate the bindings in the VAO.
    gl.bindVertexArray(&rp->gl);
    GLVertexBuffer const* const glvb = handle_cast<GLVertexBuffer*>(vb);
    updateVertexArrayObject(rp, glvb);

    mBoundRenderPrimitive = rp;
}

void OpenGLDriver::bindDescriptorSet(
        DescriptorSetHandle dsh,
        descriptor_set_t set,
        DescriptorSetOffsetArray&& offsets) {

    if (UTILS_UNLIKELY(!dsh)) {
        mBoundDescriptorSets[set].dsh = dsh;
        mInvalidDescriptorSetBindings.set(set, true);
        mInvalidDescriptorSetBindingOffsets.set(set, true);
        return;
    }

    // handle_cast<> here also serves to validate the handle (it actually cannot return nullptr)
    GLDescriptorSet const* const ds = handle_cast<GLDescriptorSet*>(dsh);
    if (ds) {
        assert_invariant(set < MAX_DESCRIPTOR_SET_COUNT);
        if (mBoundDescriptorSets[set].dsh != dsh) {
            // if the descriptor itself changed, we mark this descriptor binding
            // invalid -- it will be re-bound at the next draw.
            mInvalidDescriptorSetBindings.set(set, true);
        } else if (!offsets.empty()) {
            // if we reset offsets, we mark the offsets invalid so these descriptors only can
            // be re-bound at the next draw.
            mInvalidDescriptorSetBindingOffsets.set(set, true);
        }

        // `offsets` data's lifetime will end when this function returns. We have to make a copy.
        // (the data is allocated inside the CommandStream)
        mBoundDescriptorSets[set].dsh = dsh;
        assert_invariant(offsets.data() != nullptr || ds->getDynamicBufferCount() == 0);
        std::copy_n(offsets.data(), ds->getDynamicBufferCount(),
                mBoundDescriptorSets[set].offsets.data());
    }
}

void OpenGLDriver::updateDescriptors(bitset8 invalidDescriptorSets) noexcept {
    assert_invariant(mBoundProgram);
    auto const offsetOnly = mInvalidDescriptorSetBindingOffsets & ~mInvalidDescriptorSetBindings;
    invalidDescriptorSets.forEachSetBit([this, offsetOnly,
            &boundDescriptorSets = mBoundDescriptorSets,
            &context = mContext,
            &boundProgram = *mBoundProgram](size_t set) {
        assert_invariant(set < MAX_DESCRIPTOR_SET_COUNT);
        auto const& entry = boundDescriptorSets[set];
        if (entry.dsh) {
            GLDescriptorSet* const ds = handle_cast<GLDescriptorSet*>(entry.dsh);
#ifndef NDEBUG
            if (UTILS_UNLIKELY(!offsetOnly[set])) {
                // validate that this descriptor-set layout matches the layout set in the pipeline
                // we don't need to do the check if only the offset is changing
                ds->validate(mHandleAllocator, mCurrentSetLayout[set]);
            }
#endif
            ds->bind(context, mHandleAllocator, boundProgram,
                    set, entry.offsets.data(), offsetOnly[set]);
        }
    });
    mInvalidDescriptorSetBindings.clear();
    mInvalidDescriptorSetBindingOffsets.clear();
}

void OpenGLDriver::draw2(uint32_t indexOffset, uint32_t indexCount, uint32_t instanceCount) {
    DEBUG_MARKER()
    assert_invariant(!mContext.isES2());
    assert_invariant(mBoundRenderPrimitive);
#if FILAMENT_ENABLE_MATDBG
    if (UTILS_UNLIKELY(!mValidProgram)) {
        return;
    }
#endif
    assert_invariant(mBoundProgram);
    assert_invariant(mValidProgram);

    // When the program changes, we might have to rebind all or some descriptors
    auto const invalidDescriptorSets =
            mInvalidDescriptorSetBindings | mInvalidDescriptorSetBindingOffsets;
    if (UTILS_UNLIKELY(invalidDescriptorSets.any())) {
        updateDescriptors(invalidDescriptorSets);
    }

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    GLRenderPrimitive const* const rp = mBoundRenderPrimitive;
    glDrawElementsInstanced(GLenum(rp->type), (GLsizei)indexCount,
            rp->gl.getIndicesType(),
            reinterpret_cast<const void*>(indexOffset << rp->gl.indicesShift),
            (GLsizei)instanceCount);
#endif

#if FILAMENT_ENABLE_MATDBG
    CHECK_GL_ERROR_NON_FATAL(utils::slog.e)
#else
    CHECK_GL_ERROR(utils::slog.e)
#endif
}

// This is the ES2 version of draw2().
void OpenGLDriver::draw2GLES2(uint32_t indexOffset, uint32_t indexCount, uint32_t instanceCount) {
    DEBUG_MARKER()
    assert_invariant(mContext.isES2());
    assert_invariant(mBoundRenderPrimitive);
#if FILAMENT_ENABLE_MATDBG
    if (UTILS_UNLIKELY(!mValidProgram)) {
        return;
    }
#endif
    assert_invariant(mBoundProgram);
    assert_invariant(mValidProgram);

    // When the program changes, we might have to rebind all or some descriptors
    auto const invalidDescriptorSets =
            mInvalidDescriptorSetBindings | mInvalidDescriptorSetBindingOffsets;
    if (UTILS_UNLIKELY(invalidDescriptorSets.any())) {
        updateDescriptors(invalidDescriptorSets);
    }

    GLRenderPrimitive const* const rp = mBoundRenderPrimitive;
    assert_invariant(instanceCount == 1);
    glDrawElements(GLenum(rp->type), (GLsizei)indexCount, rp->gl.getIndicesType(),
            reinterpret_cast<const void*>(indexOffset << rp->gl.indicesShift));

#if FILAMENT_ENABLE_MATDBG
    CHECK_GL_ERROR_NON_FATAL(utils::slog.e)
#else
    CHECK_GL_ERROR(utils::slog.e)
#endif
}

void OpenGLDriver::scissor(Viewport scissor) {
    DEBUG_MARKER()
    setScissor(scissor);
}

void OpenGLDriver::draw(PipelineState state, Handle<HwRenderPrimitive> rph,
        uint32_t const indexOffset, uint32_t const indexCount, uint32_t const instanceCount) {
    DEBUG_MARKER()
    GLRenderPrimitive* const rp = handle_cast<GLRenderPrimitive*>(rph);
    state.primitiveType = rp->type;
    state.vertexBufferInfo = rp->vbih;
    bindPipeline(state);
    bindRenderPrimitive(rph);
    if (UTILS_UNLIKELY(mContext.isES2())) {
        draw2GLES2(indexOffset, indexCount, instanceCount);
    } else {
        draw2(indexOffset, indexCount, instanceCount);
    }
}

void OpenGLDriver::dispatchCompute(Handle<HwProgram> program, uint3 workGroupCount) {
    DEBUG_MARKER()
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

#if FILAMENT_ENABLE_MATDBG
    CHECK_GL_ERROR_NON_FATAL(utils::slog.e)
#else
    CHECK_GL_ERROR(utils::slog.e)
#endif
}

// explicit instantiation of the Dispatcher
template class ConcreteDispatcher<OpenGLDriver>;

} // namespace filament::backend

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
