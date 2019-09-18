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

#ifndef TNT_FILAMENT_DRIVER_OPENGLDRIVER_H
#define TNT_FILAMENT_DRIVER_OPENGLDRIVER_H

#include "private/backend/Driver.h"
#include "DriverBase.h"
#include "OpenGLContext.h"

#include <utils/compiler.h>
#include <utils/Allocator.h>

#include <math/vec4.h>

#include <tsl/robin_map.h>

#include <set>

#include <assert.h>


namespace filament {

namespace backend {
class OpenGLPlatform;
class PixelBufferDescriptor;
class TargetBufferInfo;
} // namespace backend

class OpenGLProgram;
class OpenGLBlitter;

class OpenGLDriver final : public backend::DriverBase {
    inline explicit OpenGLDriver(backend::OpenGLPlatform* platform) noexcept;
    ~OpenGLDriver() noexcept final;

public:
    static backend::Driver* create(backend::OpenGLPlatform* platform, void* sharedGLContext) noexcept;

    // OpenGLDriver specific fields
    struct GLBuffer {
        GLuint id = 0;
        uint32_t capacity = 0;
        uint32_t base = 0;
        uint32_t size = 0;
        backend::BufferUsage usage = {};
    };

    struct GLVertexBuffer : public backend::HwVertexBuffer {
        using HwVertexBuffer::HwVertexBuffer;
        struct {
            std::array<GLuint, backend::MAX_VERTEX_ATTRIBUTE_COUNT> buffers;  // 4 * MAX_VERTEX_ATTRIBUTE_COUNT bytes
        } gl;
    };

    struct GLIndexBuffer : public backend::HwIndexBuffer {
        using HwIndexBuffer::HwIndexBuffer;
        struct {
            GLuint buffer;
        } gl;
    };

    struct GLUniformBuffer : public backend::HwUniformBuffer {
        GLUniformBuffer(uint32_t capacity, backend::BufferUsage usage) noexcept {
            gl.ubo.capacity = capacity;
            gl.ubo.usage = usage;
        }
        struct {
            GLBuffer ubo;
        } gl;
    };

    struct GLSamplerGroup : public backend::HwSamplerGroup {
        using HwSamplerGroup::HwSamplerGroup;
        struct {
        } gl;
    };

    struct GLRenderPrimitive : public backend::HwRenderPrimitive {
        using HwRenderPrimitive::HwRenderPrimitive;
        OpenGLContext::RenderPrimitive gl;
    };

    struct GLTexture : public backend::HwTexture {
        using HwTexture::HwTexture;
        struct {
            GLuint id;              // texture or renderbuffer id
            mutable GLuint rb = 0;  // multi-sample sidecar renderbuffer
            GLenum target;
            GLenum internalFormat;
            mutable GLsync fence = nullptr;

            // texture parameters go here too
            GLfloat anisotropy = 1.0;
            int8_t baseLevel = 127;
            int8_t maxLevel = -1;
            uint8_t targetIndex = 0;    // optimization: index corresponding to target
        } gl;

        void* platformPImpl = nullptr;
    };

    class DebugMarker {
        OpenGLDriver& driver;
    public:
        DebugMarker(OpenGLDriver& driver, const char* string) noexcept;
        ~DebugMarker() noexcept;
    };

    struct GLStream : public backend::HwStream {
        static constexpr size_t ROUND_ROBIN_TEXTURE_COUNT = 3;      // 3 maximum
        using HwStream::HwStream;
        bool isNativeStream() const { return gl.externalTextureId == 0; }
        struct Info {
            // storage for the read/write textures below
            backend::Platform::ExternalTexture* ets = nullptr;
            GLuint width = 0;
            GLuint height = 0;
        };
        struct {
            // id of the texture where the external frames are streamed (i.e. the texture
            // used by SurfaceTexture on Android)
            GLuint externalTextureId = 0;

            /*
             * This is for making a cpu copy of the camera frame
             */
            GLuint externalTexture2DId = 0;
            GLuint fbo = 0;
        } gl;   // 20 bytes

        /*
         * The fields below are access from the main application thread
         * (not the GL thread)
         */
        struct {
            // texture id used to texture from, always used in the GL thread
            GLuint read[ROUND_ROBIN_TEXTURE_COUNT];     // 12 bytes
            // texture id to write into, always used from the user thread
            GLuint write[ROUND_ROBIN_TEXTURE_COUNT];    // 12 bytes
            Info infos[ROUND_ROBIN_TEXTURE_COUNT];      // 48 bytes
            int64_t timestamp = 0;
            uint8_t cur = 0;
        } user_thread;
    };

    struct GLRenderTarget : public backend::HwRenderTarget {
        using HwRenderTarget::HwRenderTarget;
        struct GL {
            struct RenderBuffer {
                GLTexture* texture = nullptr;
                uint8_t level = 0; // level when attached to a texture
            };
            // field ordering to optimize size on 64-bits
            RenderBuffer color;
            RenderBuffer depth;
            RenderBuffer stencil;
            GLuint fbo = 0;
            mutable GLuint fbo_read = 0;
            mutable backend::TargetBufferFlags resolve = backend::TargetBufferFlags::NONE; // attachments in fbo_draw to resolve
            uint8_t samples : 4;
        } gl;
    };

    OpenGLDriver(OpenGLDriver const&) = delete;
    OpenGLDriver& operator=(OpenGLDriver const&) = delete;


private:
    OpenGLContext mContext;

    OpenGLContext& getContext() noexcept { return mContext; }

    backend::ShaderModel getShaderModel() const noexcept final;

    /*
     * Driver interface
     */

    template<typename T>
    friend class backend::ConcreteDispatcher;

#define DECL_DRIVER_API(methodName, paramsDecl, params) \
    UTILS_ALWAYS_INLINE inline void methodName(paramsDecl);

#define DECL_DRIVER_API_SYNCHRONOUS(RetType, methodName, paramsDecl, params) \
    RetType methodName(paramsDecl) override;

#define DECL_DRIVER_API_RETURN(RetType, methodName, paramsDecl, params) \
    RetType methodName##S() noexcept override; \
    UTILS_ALWAYS_INLINE inline void methodName##R(RetType, paramsDecl);

#include "private/backend/DriverAPI.inc"


    // Memory management...

    class HandleAllocator {
        utils::PoolAllocator< 16, 16>   mPool0;
        utils::PoolAllocator< 64, 32>   mPool1;
        utils::PoolAllocator<208, 32>   mPool2;
    public:
        static constexpr size_t MIN_ALIGNMENT_SHIFT = 4;
        explicit HandleAllocator(const utils::HeapArea& area);
        void* alloc(size_t size, size_t alignment, size_t extra = 0) noexcept;
        void free(void* p, size_t size) noexcept;
    };

    // the arenas for handle allocation needs to be thread-safe
#ifndef NDEBUG
    using HandleArena = utils::Arena<HandleAllocator,
            utils::LockingPolicy::SpinLock,
            utils::TrackingPolicy::Debug>;
#else
    using HandleArena = utils::Arena<HandleAllocator,
            utils::LockingPolicy::SpinLock>;
#endif

    HandleArena mHandleArena;

    backend::HandleBase::HandleId allocateHandle(size_t size) noexcept;

    template<typename D, typename B, typename ... ARGS>
    typename std::enable_if<std::is_base_of<B, D>::value, D>::type*
            construct(backend::Handle<B> const& handle, ARGS&& ... args) noexcept;

    template<typename B, typename D,
            typename = typename std::enable_if<std::is_base_of<B, D>::value, D>::type>
    void destruct(backend::Handle<B>& handle, D const* p) noexcept;


    /*
     * handle_cast
     *
     * casts a Handle<> to a pointer to the data it refers to.
     */

    template<typename Dp, typename B>
    inline
    typename std::enable_if<
            std::is_pointer<Dp>::value &&
            std::is_base_of<B, typename std::remove_pointer<Dp>::type>::value, Dp>::type
    handle_cast(backend::Handle<B>& handle) noexcept {
        char* const base = (char *)mHandleArena.getArea().begin();
        size_t offset = handle.getId() << HandleAllocator::MIN_ALIGNMENT_SHIFT;
        return static_cast<Dp>(static_cast<void *>(base + offset));
    }

    template<typename Dp, typename B>
    inline
    typename std::enable_if<
            std::is_pointer<Dp>::value &&
            std::is_base_of<B, typename std::remove_pointer<Dp>::type>::value, Dp>::type
    handle_cast(backend::Handle<B> const& handle) noexcept {
        return handle_cast<Dp>(const_cast<backend::Handle<B>&>(handle));
    }

    friend class OpenGLProgram;

    /* Extension management... */

    using MustCastToRightType = void (*)();
    using GetProcAddressType = MustCastToRightType (*)(const char* name);
    GetProcAddressType getProcAddress = nullptr;

    /* Misc... */

    void framebufferTexture(backend::TargetBufferInfo const& binfo,
            GLRenderTarget const* rt, GLenum attachment) noexcept;

    void framebufferRenderbuffer(GLTexture const* t,
            GLRenderTarget const* rt, GLenum attachment) noexcept;

    void setRasterStateSlow(backend::RasterState rs) noexcept;
    void setRasterState(backend::RasterState rs) noexcept {
        if (UTILS_UNLIKELY(rs != mRasterState)) {
            setRasterStateSlow(rs);
        }
    }

    void setTextureData(GLTexture* t,
            uint32_t level,
            uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
            uint32_t width, uint32_t height, uint32_t depth,
            backend::PixelBufferDescriptor&& data, backend::FaceOffsets const* faceOffsets);

    void setCompressedTextureData(GLTexture* t,
            uint32_t level,
            uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
            uint32_t width, uint32_t height, uint32_t depth,
            backend::PixelBufferDescriptor&& data, backend::FaceOffsets const* faceOffsets);

    void renderBufferStorage(GLuint rbo, GLenum internalformat, uint32_t width,
            uint32_t height, uint8_t samples) const noexcept;

    void textureStorage(GLTexture* t,
            uint32_t width, uint32_t height, uint32_t depth) noexcept;

    /* State tracking GL wrappers... */

           void bindTexture(GLuint unit, GLTexture const* t) noexcept;
    inline void useProgram(OpenGLProgram* p) noexcept;

    enum class ResolveAction { LOAD, STORE };
    void resolvePass(ResolveAction action, GLRenderTarget const* rt,
            backend::TargetBufferFlags discardFlags) noexcept;

    GLuint getSamplerSlow(backend::SamplerParams sp) const noexcept;

    inline GLuint getSampler(backend::SamplerParams sp) const noexcept {
        auto pos = mSamplerMap.find(sp.u);
        if (UTILS_UNLIKELY(pos == mSamplerMap.end())) {
            return getSamplerSlow(sp);
        }
        return pos->second;
    }

    const std::array<backend::HwSamplerGroup*, backend::Program::SAMPLER_BINDING_COUNT>& getSamplerBindings() const {
        return mSamplerBindings;
    }

    GLsizei getAttachments(std::array<GLenum, 3>& attachments,
            GLRenderTarget const* rt, backend::TargetBufferFlags buffers) const noexcept;

    backend::RasterState mRasterState;

    // state required to represent the current render pass
    backend::Handle<backend::HwRenderTarget> mRenderPassTarget;
    backend::RenderPassParams mRenderPassParams;

    // state needed for clearing the viewport "by hand", i.e. with a triangle
    GLuint mClearVertexShader;
    GLuint mClearFragmentShader;
    GLuint mClearProgram;
    GLint mClearColorLocation;
    GLint mClearDepthLocation;
    static const math::float2 mClearTriangle[3];
    void initClearProgram() noexcept;
    void terminateClearProgram() noexcept;
    void clearWithRasterPipe(bool clearColor, math::float4 const& linearColor,
            bool clearDepth, double depth,
            bool clearStencil, uint32_t stencil) noexcept;
    void clearWithGeometryPipe(bool clearColor, math::float4 const& linearColor,
            bool clearDepth, double depth,
            bool clearStencil, uint32_t stencil) noexcept;

    void setViewportScissor(backend::Viewport const& viewportScissor) noexcept;

    // sampler buffer binding points (nullptr if not used)
    std::array<backend::HwSamplerGroup*, backend::Program::SAMPLER_BINDING_COUNT> mSamplerBindings;   // 8 pointers

    mutable tsl::robin_map<uint32_t, GLuint> mSamplerMap;
    mutable std::vector<GLTexture*> mExternalStreams;


    void attachStream(GLTexture* t, GLStream* stream) noexcept;
    void detachStream(GLTexture* t) noexcept;
    void replaceStream(GLTexture* t, GLStream* stream) noexcept;

    backend::OpenGLPlatform& mPlatform;

    OpenGLBlitter* mOpenGLBlitter = nullptr;
    void updateStream(GLTexture* t, backend::DriverApi* driver) noexcept;
    void updateBuffer(GLenum target, GLBuffer* buffer, backend::BufferDescriptor const& p, uint32_t alignment = 16) noexcept;
    void updateTextureLodRange(GLTexture* texture, int8_t targetLevel) noexcept;
};

// ------------------------------------------------------------------------------------------------


} // namespace filament

#endif // TNT_FILAMENT_DRIVER_OPENGLDRIVER_H
