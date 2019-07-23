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
#include "GLUtils.h"

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
        struct {
            GLuint vao = 0;
            GLenum indicesType = GL_UNSIGNED_INT;
            GLuint elementArray = 0;
            utils::bitset32 vertexAttribArray;
        } gl;
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

    void useProgram(GLuint program) noexcept;

    OpenGLDriver(OpenGLDriver const&) = delete;
    OpenGLDriver& operator=(OpenGLDriver const&) = delete;

    constexpr static inline size_t getIndexForTextureTarget(GLuint target) noexcept;

private:
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
            utils::TrackingPolicy::HighWatermark>;
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

    typedef math::details::TVec4<GLint> vec4gli;

    friend class OpenGLProgram;

    /* Extension management... */

    using MustCastToRightType = void (*)();
    using GetProcAddressType = MustCastToRightType (*)(const char* name);
    GetProcAddressType getProcAddress = nullptr;

    // this is chosen to minimize code size
    using ExtentionSet = std::set<utils::StaticString>;
    static bool hasExtension(ExtentionSet const& exts, utils::StaticString ext) noexcept;
    void initExtensionsGLES(GLint major, GLint minor, ExtentionSet const& extensionsMap);
    void initExtensionsGL(GLint major, GLint minor, ExtentionSet const& extensionsMap);


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

    constexpr inline size_t getIndexForCap(GLenum cap) noexcept;
    constexpr static inline size_t getIndexForBufferTarget(GLenum target) noexcept;

    inline void pixelStore(GLenum, GLint) noexcept;
    inline void activeTexture(GLuint unit) noexcept;
           void bindTexture(GLuint unit, GLTexture const* t) noexcept;
           void bindTexture(GLuint unit, GLuint target, GLuint texId, size_t targetIndex) noexcept;

    inline void unbindTexture(GLenum target, GLuint id) noexcept;
    inline void bindSampler(GLuint unit, GLuint sampler) noexcept;
    inline void unbindSampler(GLuint sampler) noexcept;

    inline void useProgram(OpenGLProgram* p) noexcept;

    inline void bindBuffer(GLenum target, GLuint buffer) noexcept;
    inline void bindBufferRange(GLenum target, GLuint index, GLuint buffer,
            GLintptr offset, GLsizeiptr size) noexcept;

    inline void bindFramebuffer(GLenum target, GLuint buffer) noexcept;

    inline void bindVertexArray(GLRenderPrimitive const* vao) noexcept;
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

    inline void setScissor(GLint left, GLint bottom, GLsizei width, GLsizei height) noexcept;
    inline void setViewport(GLint left, GLint bottom, GLsizei width, GLsizei height) noexcept;

    inline void setClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) noexcept;
    inline void setClearDepth(GLfloat depth) noexcept;
    inline void setClearStencil(GLint stencil) noexcept;

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
            GLRenderTarget const* rt, uint8_t buffers) const noexcept;

    static constexpr const size_t MAX_TEXTURE_UNIT_COUNT = 16;   // All mobile GPUs as of 2016
    static constexpr const size_t MAX_BUFFER_BINDINGS = 32;

    GLRenderPrimitive mDefaultVAO;

    template <typename T, typename F>
    inline void update_state(T& state, T const& expected, F functor, bool force = false) noexcept {
        if (UTILS_UNLIKELY(force || state != expected)) {
            state = expected;
            functor();
        }
    }

    // Try to keep the State structure sorted by data-access patterns
    struct State {
        GLuint draw_fbo = 0;
        GLuint read_fbo = 0;

        struct {
            GLuint use = 0;
        } program;

        struct {
            GLRenderPrimitive* p = nullptr;
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
            vec4gli scissor = 0;
            vec4gli viewport = 0;
        } window;

        struct {
            math::float4 color = {};
            GLfloat depth = 1.0f;
            GLint stencil = 0;
        } clears;

    } state;

    static constexpr const size_t TEXTURE_TARGET_COUNT =
            sizeof(state.textures.units[0].targets) / sizeof(state.textures.units[0].targets[0]);

    backend::RasterState mRasterState;

    GLfloat mMaxAnisotropy = 0.0f;
    backend::ShaderModel mShaderModel;

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

    // sampler buffer binding points (nullptr if not used)
    std::array<backend::HwSamplerGroup*, backend::Program::SAMPLER_BINDING_COUNT> mSamplerBindings;   // 8 pointers

    mutable tsl::robin_map<uint32_t, GLuint> mSamplerMap;
    mutable std::vector<GLTexture*> mExternalStreams;

    // glGet*() values
    struct {
        GLint max_renderbuffer_size = 0;
        GLint max_uniform_block_size = 0;
        GLint uniform_buffer_offset_alignment = 256;
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
        bool EXT_multisampled_render_to_texture = false;
    } ext;

    struct {
        // Some drivers have issues with UBOs in the fragment shader when
        // glFlush() is called between draw calls.
        bool disable_glFlush = false;

        // Some drivers seem to not store the GL_ELEMENT_ARRAY_BUFFER binding
        // in the VAO state.
        bool vao_doesnt_store_element_array_buffer_binding = false;

        // On some drivers, glClear() cancels glInvalidateFrameBuffer() which results
        // in extra GPU memory loads.
        bool clears_hurt_performance = false;

        // Some drivers have gl state issues when drawing from shared contexts
        bool disable_shared_context_draws = false;

        // Some drivers require the GL_TEXTURE_EXTERNAL_OES target to be bound when
        // the texture image changes, even if it's already bound to that texture
        bool texture_external_needs_rebind = false;

        // Some web browsers seem to immediately clear the default framebuffer when calling
        // glInvalidateFramebuffer with WebGL 2.0
        bool disable_invalidate_framebuffer = false;
    } bugs;

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

constexpr size_t OpenGLDriver::getIndexForTextureTarget(GLuint target) noexcept {
    switch (target) {
        case GL_TEXTURE_2D:             return 0;
        case GL_TEXTURE_2D_ARRAY:       return 1;
        case GL_TEXTURE_CUBE_MAP:       return 2;
        case GL_TEXTURE_2D_MULTISAMPLE: return 3;
        case GL_TEXTURE_EXTERNAL_OES:   return 4;
        default:                        return 0;
    }
}

constexpr size_t OpenGLDriver::getIndexForCap(GLenum cap) noexcept {
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

constexpr size_t OpenGLDriver::getIndexForBufferTarget(GLenum target) noexcept {
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

void OpenGLDriver::activeTexture(GLuint unit) noexcept {
    assert(unit < MAX_TEXTURE_UNIT_COUNT);
    update_state(state.textures.active, unit, [&]() {
        glActiveTexture(GL_TEXTURE0 + unit);
    });
}

void OpenGLDriver::bindSampler(GLuint unit, GLuint sampler) noexcept {
    assert(unit < MAX_TEXTURE_UNIT_COUNT);
    update_state(state.textures.units[unit].sampler, sampler, [&]() {
        glBindSampler(unit, sampler);
    });
}

} // namespace filament

#endif // TNT_FILAMENT_DRIVER_OPENGLDRIVER_H
