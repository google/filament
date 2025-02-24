//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_RENDERER_WGPU_WGPU_UTILS_H_
#define LIBANGLE_RENDERER_WGPU_WGPU_UTILS_H_

#include <dawn/webgpu_cpp.h>
#include <stdint.h>
#include <climits>

#include "libANGLE/Caps.h"
#include "libANGLE/Error.h"
#include "libANGLE/angletypes.h"

#define ANGLE_WGPU_TRY(context, command)                                                     \
    do                                                                                       \
    {                                                                                        \
        auto ANGLE_LOCAL_VAR = command;                                                      \
        if (ANGLE_UNLIKELY(::rx::webgpu::IsWgpuError(ANGLE_LOCAL_VAR)))                      \
        {                                                                                    \
            (context)->handleError(GL_INVALID_OPERATION, "Internal WebGPU error.", __FILE__, \
                                   ANGLE_FUNCTION, __LINE__);                                \
            return angle::Result::Stop;                                                      \
        }                                                                                    \
    } while (0)

#define ANGLE_WGPU_BEGIN_DEBUG_ERROR_SCOPE(context)                             \
    ::rx::webgpu::DebugErrorScope(context->getInstance(), context->getDevice(), \
                                  wgpu::ErrorFilter::Validation)
#define ANGLE_WGPU_END_DEBUG_ERROR_SCOPE(context, scope) \
    ANGLE_TRY(scope.PopScope(context, __FILE__, ANGLE_FUNCTION, __LINE__))

#define ANGLE_WGPU_SCOPED_DEBUG_TRY(context, command)                                            \
    do                                                                                           \
    {                                                                                            \
        ::rx::webgpu::DebugErrorScope _errorScope = ANGLE_WGPU_BEGIN_DEBUG_ERROR_SCOPE(context); \
        (command);                                                                               \
        ANGLE_WGPU_END_DEBUG_ERROR_SCOPE(context, _errorScope);                                  \
    } while (0)

#define ANGLE_GL_OBJECTS_X(PROC) \
    PROC(Buffer)                 \
    PROC(Context)                \
    PROC(Framebuffer)            \
    PROC(Query)                  \
    PROC(Program)                \
    PROC(ProgramExecutable)      \
    PROC(Sampler)                \
    PROC(Texture)                \
    PROC(TransformFeedback)      \
    PROC(VertexArray)

#define ANGLE_EGL_OBJECTS_X(PROC) \
    PROC(Display)                 \
    PROC(Image)                   \
    PROC(Surface)                 \
    PROC(Sync)

namespace rx
{

class ContextWgpu;
class DisplayWgpu;

#define ANGLE_PRE_DECLARE_WGPU_OBJECT(OBJ) class OBJ##Wgpu;

ANGLE_GL_OBJECTS_X(ANGLE_PRE_DECLARE_WGPU_OBJECT)
ANGLE_EGL_OBJECTS_X(ANGLE_PRE_DECLARE_WGPU_OBJECT)

namespace webgpu
{
template <typename T>
struct ImplTypeHelper;

#define ANGLE_IMPL_TYPE_HELPER(frontendNamespace, OBJ) \
    template <>                                        \
    struct ImplTypeHelper<frontendNamespace::OBJ>      \
    {                                                  \
        using ImplType = rx::OBJ##Wgpu;                \
    };
#define ANGLE_IMPL_TYPE_HELPER_GL(OBJ) ANGLE_IMPL_TYPE_HELPER(gl, OBJ)
#define ANGLE_IMPL_TYPE_HELPER_EGL(OBJ) ANGLE_IMPL_TYPE_HELPER(egl, OBJ)

ANGLE_GL_OBJECTS_X(ANGLE_IMPL_TYPE_HELPER_GL)
ANGLE_EGL_OBJECTS_X(ANGLE_IMPL_TYPE_HELPER_EGL)

#undef ANGLE_IMPL_TYPE_HELPER_GL
#undef ANGLE_IMPL_TYPE_HELPER_EGL

template <typename T>
using GetImplType = typename ImplTypeHelper<T>::ImplType;

template <typename T>
GetImplType<T> *GetImpl(const T *glObject)
{
    return GetImplAs<GetImplType<T>>(glObject);
}

constexpr size_t kUnpackedDepthIndex   = gl::IMPLEMENTATION_MAX_DRAW_BUFFERS;
constexpr size_t kUnpackedStencilIndex = gl::IMPLEMENTATION_MAX_DRAW_BUFFERS + 1;
constexpr uint32_t kUnpackedColorBuffersMask =
    angle::BitMask<uint32_t>(gl::IMPLEMENTATION_MAX_DRAW_BUFFERS);
// WebGPU image level index.
using LevelIndex = gl::LevelIndexWrapper<uint32_t>;

class ErrorScope : public angle::NonCopyable
{
  public:
    ErrorScope(wgpu::Instance instance, wgpu::Device device, wgpu::ErrorFilter errorType);
    ~ErrorScope();

    angle::Result PopScope(ContextWgpu *context,
                           const char *file,
                           const char *function,
                           unsigned int line);

  private:
    wgpu::Instance mInstance;
    wgpu::Device mDevice;
    bool mActive = false;
};

class NoOpErrorScope : public angle::NonCopyable
{
  public:
    NoOpErrorScope(wgpu::Instance instance, wgpu::Device device, wgpu::ErrorFilter errorType) {}
    ~NoOpErrorScope() {}

    angle::Result PopScope(ContextWgpu *context,
                           const char *file,
                           const char *function,
                           unsigned int line)
    {
        return angle::Result::Continue;
    }
};

#if defined(ANGLE_ENABLE_ASSERTS)
using DebugErrorScope = ErrorScope;
#else
using DebugErrorScope = NoOpErrorScope;
#endif

enum class RenderPassClosureReason
{
    NewRenderPass,
    FramebufferBindingChange,
    FramebufferInternalChange,
    GLFlush,
    GLFinish,
    EGLSwapBuffers,
    GLReadPixels,
    IndexRangeReadback,
    VertexArrayStreaming,
    VertexArrayLineLoop,

    InvalidEnum,
    EnumCount = InvalidEnum,
};

struct ClearValues
{
    wgpu::Color clearColor;
    uint32_t depthSlice;
    float depthValue;
    uint32_t stencilValue;
};

class ClearValuesArray final
{
  public:
    ClearValuesArray();
    ~ClearValuesArray();

    ClearValuesArray(const ClearValuesArray &other);
    ClearValuesArray &operator=(const ClearValuesArray &rhs);

    void store(uint32_t index, const ClearValues &clearValues);

    gl::DrawBufferMask getColorMask() const;
    void reset()
    {
        mValues.fill({});
        mEnabled.reset();
    }
    void reset(size_t index)
    {
        mValues[index] = {};
        mEnabled.reset(index);
    }
    void resetDepth()
    {
        mValues[kUnpackedDepthIndex] = {};
        mEnabled.reset(kUnpackedDepthIndex);
    }
    void resetStencil()
    {
        mValues[kUnpackedStencilIndex] = {};
        mEnabled.reset(kUnpackedStencilIndex);
    }
    const ClearValues &operator[](size_t index) const { return mValues[index]; }

    bool empty() const { return mEnabled.none(); }
    bool any() const { return mEnabled.any(); }

    bool test(size_t index) const { return mEnabled.test(index); }

    float getDepthValue() const { return mValues[kUnpackedDepthIndex].depthValue; }
    uint32_t getStencilValue() const { return mValues[kUnpackedStencilIndex].stencilValue; }
    bool hasDepth() const { return mEnabled.test(kUnpackedDepthIndex); }
    bool hasStencil() const { return mEnabled.test(kUnpackedStencilIndex); }

  private:
    gl::AttachmentArray<ClearValues> mValues;
    gl::AttachmentsMask mEnabled;
};

void GenerateCaps(const wgpu::Limits &limitWgpu,
                  gl::Caps *glCaps,
                  gl::TextureCapsMap *glTextureCapsMap,
                  gl::Extensions *glExtensions,
                  gl::Limitations *glLimitations,
                  egl::Caps *eglCaps,
                  egl::DisplayExtensions *eglExtensions,
                  gl::Version *maxSupportedESVersion);

DisplayWgpu *GetDisplay(const gl::Context *context);
wgpu::Device GetDevice(const gl::Context *context);
wgpu::Instance GetInstance(const gl::Context *context);
wgpu::RenderPassColorAttachment CreateNewClearColorAttachment(wgpu::Color clearValue,
                                                              uint32_t depthSlice,
                                                              wgpu::TextureView textureView);
wgpu::RenderPassDepthStencilAttachment CreateNewDepthStencilAttachment(
    float depthClearValue,
    uint32_t stencilClearValue,
    wgpu::TextureView textureView,
    bool hasDepthValue   = false,
    bool hasStencilValue = false);

bool IsWgpuError(wgpu::WaitStatus waitStatus);
bool IsWgpuError(WGPUBufferMapAsyncStatus mapBufferStatus);

bool IsStripPrimitiveTopology(wgpu::PrimitiveTopology topology);

// Required alignments for buffer sizes and mapping
constexpr size_t kBufferSizeAlignment         = 4;
constexpr size_t kBufferCopyToBufferAlignment = 4;
constexpr size_t kBufferMapSizeAlignment   = kBufferSizeAlignment;
constexpr size_t kBufferMapOffsetAlignment = 8;

// Required alignments for texture row uploads
constexpr size_t kTextureRowSizeAlignment = 256;

}  // namespace webgpu

namespace wgpu_gl
{
gl::LevelIndex getLevelIndex(webgpu::LevelIndex levelWgpu, gl::LevelIndex baseLevel);
gl::Extents getExtents(wgpu::Extent3D wgpuExtent);
}  // namespace wgpu_gl

namespace gl_wgpu
{
webgpu::LevelIndex getLevelIndex(gl::LevelIndex levelGl, gl::LevelIndex baseLevel);
wgpu::TextureDimension getWgpuTextureDimension(gl::TextureType glTextureType);
wgpu::Extent3D getExtent3D(const gl::Extents &glExtent);

wgpu::PrimitiveTopology GetPrimitiveTopology(gl::PrimitiveMode mode);

wgpu::IndexFormat GetIndexFormat(gl::DrawElementsType drawElementsTYpe);
wgpu::FrontFace GetFrontFace(GLenum frontFace);
wgpu::CullMode GetCullMode(gl::CullFaceMode mode, bool cullFaceEnabled);
wgpu::ColorWriteMask GetColorWriteMask(bool r, bool g, bool b, bool a);

wgpu::CompareFunction GetCompareFunc(const GLenum glCompareFunc, bool testEnabled);
wgpu::StencilOperation getStencilOp(const GLenum glStencilOp);

uint32_t GetFirstIndexForDrawCall(gl::DrawElementsType indexType, const void *indices);
}  // namespace gl_wgpu

// Number of reserved binding slots to implement the default uniform block
constexpr uint32_t kReservedPerStageDefaultUniformSlotCount = 0;

}  // namespace rx

#define ANGLE_WGPU_WRAPPER_OBJECTS_X(PROC) \
    PROC(BindGroup)                        \
    PROC(Buffer)                           \
    PROC(RenderPipeline)

// Add a hash function for all wgpu cpp wrappers that hashes the underlying C object pointer.
#define ANGLE_WGPU_WRAPPER_OBJECT_HASH(OBJ)               \
    namespace std                                         \
    {                                                     \
    template <>                                           \
    struct hash<wgpu::OBJ>                                \
    {                                                     \
        size_t operator()(const wgpu::OBJ &wrapper) const \
        {                                                 \
            std::hash<decltype(wrapper.Get())> cTypeHash; \
            return cTypeHash(wrapper.Get());              \
        }                                                 \
    };                                                    \
    }

ANGLE_WGPU_WRAPPER_OBJECTS_X(ANGLE_WGPU_WRAPPER_OBJECT_HASH)
#undef ANGLE_WGPU_WRAPPER_OBJECT_HASH

// Add a hash function for all wgpu cpp wrappers that compares the underlying C object pointer.
#define ANGLE_WGPU_WRAPPER_OBJECT_EQUALITY(OBJ)        \
    namespace wgpu                                     \
    {                                                  \
    inline bool operator==(const OBJ &a, const OBJ &b) \
    {                                                  \
        return a.Get() == b.Get();                     \
    }                                                  \
    }

ANGLE_WGPU_WRAPPER_OBJECTS_X(ANGLE_WGPU_WRAPPER_OBJECT_EQUALITY)
#undef ANGLE_WGPU_WRAPPER_OBJECT_EQUALITY

#undef ANGLE_WGPU_WRAPPER_OBJECTS_X

#endif  // LIBANGLE_RENDERER_WGPU_WGPU_UTILS_H_
