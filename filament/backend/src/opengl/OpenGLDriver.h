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

#ifndef TNT_FILAMENT_BACKEND_OPENGL_OPENGLDRIVER_H
#define TNT_FILAMENT_BACKEND_OPENGL_OPENGLDRIVER_H

#include "DriverBase.h"
#include "OpenGLContext.h"
#include "OpenGLTimerQuery.h"
#include "GLBufferObject.h"
#include "GLTexture.h"
#include "ShaderCompilerService.h"

#include <backend/platforms/OpenGLPlatform.h>

#include <backend/AcquiredImage.h>
#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/Platform.h>
#include <backend/Program.h>
#include <backend/TargetBufferInfo.h>

#include "private/backend/Driver.h"
#include "private/backend/HandleAllocator.h"

#include <utils/FixedCapacityVector.h>
#include <utils/compiler.h>
#include <utils/debug.h>

#include <math/vec4.h>

#include <tsl/robin_map.h>

#include <array>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <stddef.h>
#include <stdint.h>

#ifndef FILAMENT_OPENGL_HANDLE_ARENA_SIZE_IN_MB
#    define FILAMENT_OPENGL_HANDLE_ARENA_SIZE_IN_MB 4
#endif

namespace filament::backend {

class OpenGLPlatform;
class PixelBufferDescriptor;
struct TargetBufferInfo;

class OpenGLProgram;
class TimerQueryFactoryInterface;

struct PushConstantBundle;

class OpenGLDriver final : public DriverBase {
    inline explicit OpenGLDriver(OpenGLPlatform* platform,
            const Platform::DriverConfig& driverConfig) noexcept;
    ~OpenGLDriver() noexcept final;
    Dispatcher getDispatcher() const noexcept final;

public:
    static Driver* create(OpenGLPlatform* platform, void* sharedGLContext,
            const Platform::DriverConfig& driverConfig) noexcept;

    class DebugMarker {
        OpenGLDriver& driver;
    public:
        DebugMarker(OpenGLDriver& driver, const char* string) noexcept;
        ~DebugMarker() noexcept;
    };

    // OpenGLDriver specific fields

    struct GLSwapChain : public HwSwapChain {
        using HwSwapChain::HwSwapChain;
        bool rec709 = false;
    };

    struct GLVertexBufferInfo : public HwVertexBufferInfo {
        GLVertexBufferInfo() noexcept = default;
        GLVertexBufferInfo(uint8_t bufferCount, uint8_t attributeCount,
                AttributeArray const& attributes)
                : HwVertexBufferInfo(bufferCount, attributeCount),
                  attributes(attributes) {
        }
        AttributeArray attributes;
    };

    struct GLVertexBuffer : public HwVertexBuffer {
        GLVertexBuffer() noexcept = default;
        GLVertexBuffer(uint32_t vertexCount, Handle<HwVertexBufferInfo> vbih)
                : HwVertexBuffer(vertexCount), vbih(vbih) {
        }
        Handle<HwVertexBufferInfo> vbih;
        struct {
            // 4 * MAX_VERTEX_ATTRIBUTE_COUNT bytes
            std::array<GLuint, MAX_VERTEX_ATTRIBUTE_COUNT> buffers{};
        } gl;
    };

    struct GLIndexBuffer : public HwIndexBuffer {
        using HwIndexBuffer::HwIndexBuffer;
        struct {
            GLuint buffer{};
        } gl;
    };

    struct GLSamplerGroup : public HwSamplerGroup {
        using HwSamplerGroup::HwSamplerGroup;
        struct Entry {
            GLTexture const* texture = nullptr;
            GLuint sampler = 0u;
        };
        utils::FixedCapacityVector<Entry> textureUnitEntries;
        explicit GLSamplerGroup(size_t size) noexcept : textureUnitEntries(size) { }
    };

    struct GLRenderPrimitive : public HwRenderPrimitive {
        using HwRenderPrimitive::HwRenderPrimitive;
        OpenGLContext::RenderPrimitive gl;
        Handle<HwVertexBufferInfo> vbih;
    };

    using GLBufferObject = filament::backend::GLBufferObject;

    using GLTexture = filament::backend::GLTexture;

    using GLTimerQuery = filament::backend::GLTimerQuery;

    struct GLStream : public HwStream {
        using HwStream::HwStream;
        struct Info {
            GLuint width = 0;
            GLuint height = 0;
        };
        /*
         * The fields below are accessed from the main application thread
         * (not the GL thread)
         */
        struct {
            int64_t timestamp = 0;
            uint8_t cur = 0;
            AcquiredImage acquired;
            AcquiredImage pending;
        } user_thread;
    };

    struct GLRenderTarget : public HwRenderTarget {
        using HwRenderTarget::HwRenderTarget;
        struct {
            // field ordering to optimize size on 64-bits
            GLTexture* color[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT];
            GLTexture* depth;
            GLTexture* stencil;
            GLuint fbo = 0;
            mutable GLuint fbo_read = 0;
            mutable TargetBufferFlags resolve = TargetBufferFlags::NONE; // attachments in fbo_draw to resolve
            uint8_t samples = 1;
            bool isDefault = false;
        } gl;
        TargetBufferFlags targets = {};
    };

    struct GLFence : public HwFence {
        using HwFence::HwFence;
        struct State {
            std::mutex lock;
            std::condition_variable cond;
            FenceStatus status{ FenceStatus::TIMEOUT_EXPIRED };
        };
        std::shared_ptr<State> state{ std::make_shared<GLFence::State>() };
    };

    OpenGLDriver(OpenGLDriver const&) = delete;
    OpenGLDriver& operator=(OpenGLDriver const&) = delete;

private:
    OpenGLPlatform& mPlatform;
    OpenGLContext mContext;
    ShaderCompilerService mShaderCompilerService;

    friend class TimerQueryFactory;
    friend class TimerQueryNativeFactory;
    OpenGLContext& getContext() noexcept { return mContext; }

    ShaderCompilerService& getShaderCompilerService() noexcept {
        return mShaderCompilerService;
    }

    ShaderModel getShaderModel() const noexcept final;

    /*
     * Driver interface
     */

    template<typename T>
    friend class ConcreteDispatcher;

#define DECL_DRIVER_API(methodName, paramsDecl, params) \
    UTILS_ALWAYS_INLINE inline void methodName(paramsDecl);

#define DECL_DRIVER_API_SYNCHRONOUS(RetType, methodName, paramsDecl, params) \
    RetType methodName(paramsDecl) override;

#define DECL_DRIVER_API_RETURN(RetType, methodName, paramsDecl, params) \
    RetType methodName##S() noexcept override; \
    UTILS_ALWAYS_INLINE inline void methodName##R(RetType, paramsDecl);

#include "private/backend/DriverAPI.inc"

    // Memory management...

    // See also the explicit template instantiation in HandleAllocator.cpp
    HandleAllocatorGL mHandleAllocator;

    template<typename D, typename ... ARGS>
    Handle<D> initHandle(ARGS&& ... args) {
        return mHandleAllocator.allocateAndConstruct<D>(std::forward<ARGS>(args) ...);
    }

    template<typename D, typename B, typename ... ARGS>
    typename std::enable_if<std::is_base_of<B, D>::value, D>::type*
    construct(Handle<B> const& handle, ARGS&& ... args) {
        return mHandleAllocator.destroyAndConstruct<D, B>(handle, std::forward<ARGS>(args) ...);
    }

    template<typename B, typename D,
            typename = typename std::enable_if<std::is_base_of<B, D>::value, D>::type>
    void destruct(Handle<B>& handle, D const* p) noexcept {
        return mHandleAllocator.deallocate(handle, p);
    }

    template<typename Dp, typename B>
    typename std::enable_if_t<
            std::is_pointer_v<Dp> &&
            std::is_base_of_v<B, typename std::remove_pointer_t<Dp>>, Dp>
    handle_cast(Handle<B>& handle) {
        return mHandleAllocator.handle_cast<Dp, B>(handle);
    }

    template<typename Dp, typename B>
    inline typename std::enable_if_t<
            std::is_pointer_v<Dp> &&
            std::is_base_of_v<B, typename std::remove_pointer_t<Dp>>, Dp>
    handle_cast(Handle<B> const& handle) {
        return mHandleAllocator.handle_cast<Dp, B>(handle);
    }

    friend class OpenGLProgram;
    friend class ShaderCompilerService;

    /* Extension management... */

    using MustCastToRightType = void (*)();
    using GetProcAddressType = MustCastToRightType (*)(const char* name);
    GetProcAddressType getProcAddress = nullptr;

    /* Misc... */

    void updateVertexArrayObject(GLRenderPrimitive* rp, GLVertexBuffer const* vb);

    void framebufferTexture(TargetBufferInfo const& binfo,
            GLRenderTarget const* rt, GLenum attachment, uint8_t layerCount) noexcept;

    void setRasterState(RasterState rs) noexcept;

    void setStencilState(StencilState ss) noexcept;

    void setTextureData(GLTexture* t,
            uint32_t level,
            uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
            uint32_t width, uint32_t height, uint32_t depth,
            PixelBufferDescriptor&& p);

    void setCompressedTextureData(GLTexture* t,
            uint32_t level,
            uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
            uint32_t width, uint32_t height, uint32_t depth,
            PixelBufferDescriptor&& p);

    void renderBufferStorage(GLuint rbo, GLenum internalformat, uint32_t width,
            uint32_t height, uint8_t samples) const noexcept;

    void textureStorage(OpenGLDriver::GLTexture* t, uint32_t width, uint32_t height,
            uint32_t depth, bool useProtectedMemory) noexcept;

    /* State tracking GL wrappers... */

           void bindTexture(GLuint unit, GLTexture const* t) noexcept;
           void bindSampler(GLuint unit, GLuint sampler) noexcept;
    inline bool useProgram(OpenGLProgram* p) noexcept;

    enum class ResolveAction { LOAD, STORE };
    void resolvePass(ResolveAction action, GLRenderTarget const* rt,
            TargetBufferFlags discardFlags) noexcept;

    const std::array<GLSamplerGroup*, Program::SAMPLER_BINDING_COUNT>& getSamplerBindings() const {
        return mSamplerBindings;
    }

    using AttachmentArray = std::array<GLenum, MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + 2>;
    static GLsizei getAttachments(AttachmentArray& attachments, TargetBufferFlags buffers,
            bool isDefaultFramebuffer) noexcept;

    // state required to represent the current render pass
    Handle<HwRenderTarget> mRenderPassTarget;
    RenderPassParams mRenderPassParams;
    GLboolean mRenderPassColorWrite{};
    GLboolean mRenderPassDepthWrite{};
    GLboolean mRenderPassStencilWrite{};

    GLRenderPrimitive const* mBoundRenderPrimitive = nullptr;
    bool mValidProgram = false;


    void clearWithRasterPipe(TargetBufferFlags clearFlags,
            math::float4 const& linearColor, GLfloat depth, GLint stencil) noexcept;

    void setScissor(Viewport const& scissor) noexcept;

    // ES2 only. Uniform buffer emulation binding points
    GLuint mLastAssignedEmulatedUboId = 0;

    // sampler buffer binding points (nullptr if not used)
    std::array<GLSamplerGroup*, Program::SAMPLER_BINDING_COUNT> mSamplerBindings = {};   // 4 pointers

    // this must be accessed from the driver thread only
    std::vector<GLTexture*> mTexturesWithStreamsAttached;

    // the must be accessed from the user thread only
    std::vector<GLStream*> mStreamsWithPendingAcquiredImage;

    void attachStream(GLTexture* t, GLStream* stream) noexcept;
    void detachStream(GLTexture* t) noexcept;
    void replaceStream(GLTexture* t, GLStream* stream) noexcept;

    void updateTextureLodRange(GLTexture* texture, int8_t targetLevel) noexcept;

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    // tasks executed on the main thread after the fence signaled
    void whenGpuCommandsComplete(const std::function<void()>& fn) noexcept;
    void executeGpuCommandsCompleteOps() noexcept;
    std::vector<std::pair<GLsync, std::function<void()>>> mGpuCommandCompleteOps;

    void whenFrameComplete(const std::function<void()>& fn) noexcept;
    std::vector<std::function<void()>> mFrameCompleteOps;
#endif

    // tasks regularly executed on the main thread at until they return true
    void runEveryNowAndThen(std::function<bool()> fn) noexcept;
    void executeEveryNowAndThenOps() noexcept;
    std::vector<std::function<bool()>> mEveryNowAndThenOps;

    const Platform::DriverConfig mDriverConfig;
    Platform::DriverConfig const& getDriverConfig() const noexcept { return mDriverConfig; }

    // for ES2 sRGB support
    GLSwapChain* mCurrentDrawSwapChain = nullptr;
    bool mRec709OutputColorspace = false;

    PushConstantBundle* mCurrentPushConstants = nullptr;
};

// ------------------------------------------------------------------------------------------------


} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_OPENGL_OPENGLDRIVER_H
