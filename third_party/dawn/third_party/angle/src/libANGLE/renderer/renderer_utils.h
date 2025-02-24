//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// renderer_utils:
//   Helper methods pertaining to most or all back-ends.
//

#ifndef LIBANGLE_RENDERER_RENDERER_UTILS_H_
#define LIBANGLE_RENDERER_RENDERER_UTILS_H_

#include <cstdint>

#include <limits>
#include <map>

#include "GLSLANG/ShaderLang.h"
#include "common/angleutils.h"
#include "common/utilities.h"
#include "libANGLE/ImageIndex.h"
#include "libANGLE/angletypes.h"

namespace angle
{
struct FeatureSetBase;
struct Format;
struct ImageLoadContext;
enum class FormatID : uint8_t;
}  // namespace angle

namespace gl
{
struct FormatType;
struct InternalFormat;
class ProgramExecutable;
class State;
}  // namespace gl

namespace egl
{
class AttributeMap;
struct DisplayState;
}  // namespace egl

namespace sh
{
struct BlockMemberInfo;
}

namespace rx
{
class ContextImpl;

// The possible rotations of the surface/draw framebuffer, particularly for the Vulkan back-end on
// Android.
enum class SurfaceRotation
{
    Identity,
    Rotated90Degrees,
    Rotated180Degrees,
    Rotated270Degrees,
    FlippedIdentity,
    FlippedRotated90Degrees,
    FlippedRotated180Degrees,
    FlippedRotated270Degrees,

    InvalidEnum,
    EnumCount = InvalidEnum,
};

bool IsRotatedAspectRatio(SurfaceRotation rotation);

using SpecConstUsageBits = angle::PackedEnumBitSet<sh::vk::SpecConstUsage, uint32_t>;

void RotateRectangle(const SurfaceRotation rotation,
                     const bool flipY,
                     const int framebufferWidth,
                     const int framebufferHeight,
                     const gl::Rectangle &incoming,
                     gl::Rectangle *outgoing);

using MipGenerationFunction = void (*)(size_t sourceWidth,
                                       size_t sourceHeight,
                                       size_t sourceDepth,
                                       const uint8_t *sourceData,
                                       size_t sourceRowPitch,
                                       size_t sourceDepthPitch,
                                       uint8_t *destData,
                                       size_t destRowPitch,
                                       size_t destDepthPitch);

typedef void (*PixelReadFunction)(const uint8_t *source, uint8_t *dest);
typedef void (*PixelWriteFunction)(const uint8_t *source, uint8_t *dest);
typedef void (*FastCopyFunction)(const uint8_t *source,
                                 int srcXAxisPitch,
                                 int srcYAxisPitch,
                                 uint8_t *dest,
                                 int destXAxisPitch,
                                 int destYAxisPitch,
                                 int width,
                                 int height);

class FastCopyFunctionMap
{
  public:
    struct Entry
    {
        angle::FormatID formatID;
        FastCopyFunction func;
    };

    constexpr FastCopyFunctionMap() : FastCopyFunctionMap(nullptr, 0) {}

    constexpr FastCopyFunctionMap(const Entry *data, size_t size) : mSize(size), mData(data) {}

    bool has(angle::FormatID formatID) const;
    FastCopyFunction get(angle::FormatID formatID) const;

  private:
    size_t mSize;
    const Entry *mData;
};

struct PackPixelsParams
{
    PackPixelsParams();
    PackPixelsParams(const gl::Rectangle &area,
                     const angle::Format &destFormat,
                     GLuint outputPitch,
                     bool reverseRowOrderIn,
                     gl::Buffer *packBufferIn,
                     ptrdiff_t offset);

    gl::Rectangle area;
    const angle::Format *destFormat;
    GLuint outputPitch;
    gl::Buffer *packBuffer;
    bool reverseRowOrder;
    ptrdiff_t offset;
    SurfaceRotation rotation;
};

void PackPixels(const PackPixelsParams &params,
                const angle::Format &sourceFormat,
                int inputPitch,
                const uint8_t *source,
                uint8_t *destination);

angle::Result GetPackPixelsParams(const gl::InternalFormat &sizedFormatInfo,
                                  GLuint outputPitch,
                                  const gl::PixelPackState &packState,
                                  gl::Buffer *packBuffer,
                                  const gl::Rectangle &area,
                                  const gl::Rectangle &clippedArea,
                                  rx::PackPixelsParams *paramsOut,
                                  GLuint *skipBytesOut);

using InitializeTextureDataFunction = void (*)(size_t width,
                                               size_t height,
                                               size_t depth,
                                               uint8_t *output,
                                               size_t outputRowPitch,
                                               size_t outputDepthPitch);

using LoadImageFunction = void (*)(const angle::ImageLoadContext &context,
                                   size_t width,
                                   size_t height,
                                   size_t depth,
                                   const uint8_t *input,
                                   size_t inputRowPitch,
                                   size_t inputDepthPitch,
                                   uint8_t *output,
                                   size_t outputRowPitch,
                                   size_t outputDepthPitch);

struct LoadImageFunctionInfo
{
    LoadImageFunctionInfo() : loadFunction(nullptr), requiresConversion(false) {}
    LoadImageFunctionInfo(LoadImageFunction loadFunction, bool requiresConversion)
        : loadFunction(loadFunction), requiresConversion(requiresConversion)
    {}

    LoadImageFunction loadFunction;
    bool requiresConversion;
};

using LoadFunctionMap = LoadImageFunctionInfo (*)(GLenum);

bool ShouldUseDebugLayers(const egl::AttributeMap &attribs);

void CopyImageCHROMIUM(const uint8_t *sourceData,
                       size_t sourceRowPitch,
                       size_t sourcePixelBytes,
                       size_t sourceDepthPitch,
                       PixelReadFunction pixelReadFunction,
                       uint8_t *destData,
                       size_t destRowPitch,
                       size_t destPixelBytes,
                       size_t destDepthPitch,
                       PixelWriteFunction pixelWriteFunction,
                       GLenum destUnsizedFormat,
                       GLenum destComponentType,
                       size_t width,
                       size_t height,
                       size_t depth,
                       bool unpackFlipY,
                       bool unpackPremultiplyAlpha,
                       bool unpackUnmultiplyAlpha);

// Incomplete textures are 1x1 textures filled with black, used when samplers are incomplete.
// This helper class encapsulates handling incomplete textures. Because the GL back-end
// can take advantage of the driver's incomplete textures, and because clearing multisample
// textures is so difficult, we can keep an instance of this class in the back-end instead
// of moving the logic to the Context front-end.

// This interface allows us to call-back to init a multisample texture.
class MultisampleTextureInitializer
{
  public:
    virtual ~MultisampleTextureInitializer() {}
    virtual angle::Result initializeMultisampleTextureToBlack(const gl::Context *context,
                                                              gl::Texture *glTexture) = 0;
};

class IncompleteTextureSet final : angle::NonCopyable
{
  public:
    IncompleteTextureSet();
    ~IncompleteTextureSet();

    void onDestroy(const gl::Context *context);

    angle::Result getIncompleteTexture(const gl::Context *context,
                                       gl::TextureType type,
                                       gl::SamplerFormat format,
                                       MultisampleTextureInitializer *multisampleInitializer,
                                       gl::Texture **textureOut);

  private:
    using TextureMapWithSamplerFormat = angle::PackedEnumMap<gl::SamplerFormat, gl::TextureMap>;

    TextureMapWithSamplerFormat mIncompleteTextures;
};

// Helpers to set a matrix uniform value based on GLSL or HLSL semantics.
// The return value indicate if the data was updated or not.
template <int cols, int rows>
struct SetFloatUniformMatrixGLSL
{
    static void Run(unsigned int arrayElementOffset,
                    unsigned int elementCount,
                    GLsizei countIn,
                    GLboolean transpose,
                    const GLfloat *value,
                    uint8_t *targetData);
};

template <int cols, int rows>
struct SetFloatUniformMatrixHLSL
{
    static void Run(unsigned int arrayElementOffset,
                    unsigned int elementCount,
                    GLsizei countIn,
                    GLboolean transpose,
                    const GLfloat *value,
                    uint8_t *targetData);
};

// Helper method to de-tranpose a matrix uniform for an API query.
void GetMatrixUniform(GLenum type, GLfloat *dataOut, const GLfloat *source, bool transpose);

template <typename NonFloatT>
void GetMatrixUniform(GLenum type, NonFloatT *dataOut, const NonFloatT *source, bool transpose);

// Contains a CPU-side buffer and its data layout, used as a shadow buffer for default uniform
// blocks in VK and WGPU backends.
struct BufferAndLayout final : private angle::NonCopyable
{
    BufferAndLayout();
    ~BufferAndLayout();

    // Shadow copies of the shader uniform data.
    angle::MemoryBuffer uniformData;

    // Tells us where to write on a call to a setUniform method. They are arranged in uniform
    // location order.
    std::vector<sh::BlockMemberInfo> uniformLayout;
};

template <typename T>
void UpdateBufferWithLayout(GLsizei count,
                            uint32_t arrayIndex,
                            int componentCount,
                            const T *v,
                            const sh::BlockMemberInfo &layoutInfo,
                            angle::MemoryBuffer *uniformData);

template <typename T>
void ReadFromBufferWithLayout(int componentCount,
                              uint32_t arrayIndex,
                              T *dst,
                              const sh::BlockMemberInfo &layoutInfo,
                              const angle::MemoryBuffer *uniformData);

using DefaultUniformBlockMap = gl::ShaderMap<std::shared_ptr<BufferAndLayout>>;

template <typename T>
void SetUniform(const gl::ProgramExecutable *executable,
                GLint location,
                GLsizei count,
                const T *v,
                GLenum entryPointType,
                DefaultUniformBlockMap *defaultUniformBlocks,
                gl::ShaderBitSet *defaultUniformBlocksDirty);

template <int cols, int rows>
void SetUniformMatrixfv(const gl::ProgramExecutable *executable,
                        GLint location,
                        GLsizei count,
                        GLboolean transpose,
                        const GLfloat *value,
                        DefaultUniformBlockMap *defaultUniformBlocks,
                        gl::ShaderBitSet *defaultUniformBlocksDirty);

template <typename T>
void GetUniform(const gl::ProgramExecutable *executable,
                GLint location,
                T *v,
                GLenum entryPointType,
                const DefaultUniformBlockMap *defaultUniformBlocks);

const angle::Format &GetFormatFromFormatType(GLenum format, GLenum type);

angle::Result ComputeStartVertex(ContextImpl *contextImpl,
                                 const gl::IndexRange &indexRange,
                                 GLint baseVertex,
                                 GLint *firstVertexOut);

angle::Result GetVertexRangeInfo(const gl::Context *context,
                                 GLint firstVertex,
                                 GLsizei vertexOrIndexCount,
                                 gl::DrawElementsType indexTypeOrInvalid,
                                 const void *indices,
                                 GLint baseVertex,
                                 GLint *startVertexOut,
                                 size_t *vertexCountOut);

gl::Rectangle ClipRectToScissor(const gl::State &glState, const gl::Rectangle &rect, bool invertY);

// Helper method to intialize a FeatureSet with overrides from the DisplayState
void ApplyFeatureOverrides(angle::FeatureSetBase *features,
                           const angle::FeatureOverrides &overrides);

template <typename In>
uint32_t LineLoopRestartIndexCountHelper(GLsizei indexCount, const uint8_t *srcPtr)
{
    constexpr In restartIndex = gl::GetPrimitiveRestartIndexFromType<In>();
    const In *inIndices       = reinterpret_cast<const In *>(srcPtr);
    uint32_t numIndices       = 0;
    // See CopyLineLoopIndicesWithRestart() below for more info on how
    // numIndices is calculated.
    GLsizei loopStartIndex = 0;
    for (GLsizei curIndex = 0; curIndex < indexCount; curIndex++)
    {
        In vertex = inIndices[curIndex];
        if (vertex != restartIndex)
        {
            numIndices++;
        }
        else
        {
            if (curIndex > loopStartIndex)
            {
                if (curIndex > (loopStartIndex + 1))
                {
                    numIndices += 1;
                }
                numIndices += 1;
            }
            loopStartIndex = curIndex + 1;
        }
    }
    if (indexCount > (loopStartIndex + 1))
    {
        numIndices++;
    }
    return numIndices;
}

inline uint32_t GetLineLoopWithRestartIndexCount(gl::DrawElementsType glIndexType,
                                                 GLsizei indexCount,
                                                 const uint8_t *srcPtr)
{
    switch (glIndexType)
    {
        case gl::DrawElementsType::UnsignedByte:
            return LineLoopRestartIndexCountHelper<uint8_t>(indexCount, srcPtr);
        case gl::DrawElementsType::UnsignedShort:
            return LineLoopRestartIndexCountHelper<uint16_t>(indexCount, srcPtr);
        case gl::DrawElementsType::UnsignedInt:
            return LineLoopRestartIndexCountHelper<uint32_t>(indexCount, srcPtr);
        default:
            UNREACHABLE();
            return 0;
    }
}

// Writes the line-strip vertices for a line loop to outPtr,
// where outLimit is calculated as in GetPrimitiveRestartIndexCount.
template <typename In, typename Out>
void CopyLineLoopIndicesWithRestart(GLsizei indexCount, const uint8_t *srcPtr, uint8_t *outPtr)
{
    constexpr In restartIndex     = gl::GetPrimitiveRestartIndexFromType<In>();
    constexpr Out outRestartIndex = gl::GetPrimitiveRestartIndexFromType<Out>();
    const In *inIndices           = reinterpret_cast<const In *>(srcPtr);
    Out *outIndices               = reinterpret_cast<Out *>(outPtr);
    GLsizei loopStartIndex        = 0;
    for (GLsizei curIndex = 0; curIndex < indexCount; curIndex++)
    {
        In vertex = inIndices[curIndex];
        if (vertex != restartIndex)
        {
            *(outIndices++) = static_cast<Out>(vertex);
        }
        else
        {
            if (curIndex > loopStartIndex)
            {
                if (curIndex > (loopStartIndex + 1))
                {
                    // Emit an extra vertex only if the loop has more than one vertex.
                    *(outIndices++) = inIndices[loopStartIndex];
                }
                // Then restart the strip.
                *(outIndices++) = outRestartIndex;
            }
            loopStartIndex = curIndex + 1;
        }
    }
    if (indexCount > (loopStartIndex + 1))
    {
        // Close the last loop if it has more than one vertex.
        *(outIndices++) = inIndices[loopStartIndex];
    }
}

void GetSamplePosition(GLsizei sampleCount, size_t index, GLfloat *xy);

angle::Result MultiDrawArraysGeneral(ContextImpl *contextImpl,
                                     const gl::Context *context,
                                     gl::PrimitiveMode mode,
                                     const GLint *firsts,
                                     const GLsizei *counts,
                                     GLsizei drawcount);
angle::Result MultiDrawArraysIndirectGeneral(ContextImpl *contextImpl,
                                             const gl::Context *context,
                                             gl::PrimitiveMode mode,
                                             const void *indirect,
                                             GLsizei drawcount,
                                             GLsizei stride);
angle::Result MultiDrawArraysInstancedGeneral(ContextImpl *contextImpl,
                                              const gl::Context *context,
                                              gl::PrimitiveMode mode,
                                              const GLint *firsts,
                                              const GLsizei *counts,
                                              const GLsizei *instanceCounts,
                                              GLsizei drawcount);
angle::Result MultiDrawElementsGeneral(ContextImpl *contextImpl,
                                       const gl::Context *context,
                                       gl::PrimitiveMode mode,
                                       const GLsizei *counts,
                                       gl::DrawElementsType type,
                                       const GLvoid *const *indices,
                                       GLsizei drawcount);
angle::Result MultiDrawElementsIndirectGeneral(ContextImpl *contextImpl,
                                               const gl::Context *context,
                                               gl::PrimitiveMode mode,
                                               gl::DrawElementsType type,
                                               const void *indirect,
                                               GLsizei drawcount,
                                               GLsizei stride);
angle::Result MultiDrawElementsInstancedGeneral(ContextImpl *contextImpl,
                                                const gl::Context *context,
                                                gl::PrimitiveMode mode,
                                                const GLsizei *counts,
                                                gl::DrawElementsType type,
                                                const GLvoid *const *indices,
                                                const GLsizei *instanceCounts,
                                                GLsizei drawcount);
angle::Result MultiDrawArraysInstancedBaseInstanceGeneral(ContextImpl *contextImpl,
                                                          const gl::Context *context,
                                                          gl::PrimitiveMode mode,
                                                          const GLint *firsts,
                                                          const GLsizei *counts,
                                                          const GLsizei *instanceCounts,
                                                          const GLuint *baseInstances,
                                                          GLsizei drawcount);
angle::Result MultiDrawElementsInstancedBaseVertexBaseInstanceGeneral(ContextImpl *contextImpl,
                                                                      const gl::Context *context,
                                                                      gl::PrimitiveMode mode,
                                                                      const GLsizei *counts,
                                                                      gl::DrawElementsType type,
                                                                      const GLvoid *const *indices,
                                                                      const GLsizei *instanceCounts,
                                                                      const GLint *baseVertices,
                                                                      const GLuint *baseInstances,
                                                                      GLsizei drawcount);

// RAII object making sure reset uniforms is called no matter whether there's an error in draw calls
class ResetBaseVertexBaseInstance : angle::NonCopyable
{
  public:
    ResetBaseVertexBaseInstance(gl::ProgramExecutable *executable,
                                bool resetBaseVertex,
                                bool resetBaseInstance);

    ~ResetBaseVertexBaseInstance();

  private:
    gl::ProgramExecutable *mExecutable;
    bool mResetBaseVertex;
    bool mResetBaseInstance;
};

angle::FormatID ConvertToSRGB(angle::FormatID formatID);
angle::FormatID ConvertToLinear(angle::FormatID formatID);
bool IsOverridableLinearFormat(angle::FormatID formatID);

template <bool swizzledLuma = true>
const gl::ColorGeneric AdjustBorderColor(const angle::ColorGeneric &borderColorGeneric,
                                         const angle::Format &format,
                                         bool stencilMode);

template <typename LargerInt>
GLint LimitToInt(const LargerInt physicalDeviceValue)
{
    static_assert(sizeof(LargerInt) >= sizeof(int32_t), "Incorrect usage of LimitToInt");
    return static_cast<GLint>(
        std::min(physicalDeviceValue, static_cast<LargerInt>(std::numeric_limits<int32_t>::max())));
}

bool TextureHasAnyRedefinedLevels(const gl::CubeFaceArray<gl::TexLevelMask> &redefinedLevels);
bool IsTextureLevelRedefined(const gl::CubeFaceArray<gl::TexLevelMask> &redefinedLevels,
                             gl::TextureType textureType,
                             gl::LevelIndex level);

enum class TextureLevelDefinition
{
    Compatible   = 0,
    Incompatible = 1,

    InvalidEnum = 2
};

enum class TextureLevelAllocation
{
    WithinAllocatedImage  = 0,
    OutsideAllocatedImage = 1,

    InvalidEnum = 2
};
// Returns true if the image should be released after the level is redefined, false otherwise.
bool TextureRedefineLevel(const TextureLevelAllocation levelAllocation,
                          const TextureLevelDefinition levelDefinition,
                          bool immutableFormat,
                          uint32_t levelCount,
                          const uint32_t layerIndex,
                          const gl::ImageIndex &index,
                          gl::LevelIndex imageFirstAllocatedLevel,
                          gl::CubeFaceArray<gl::TexLevelMask> *redefinedLevels);

void TextureRedefineGenerateMipmapLevels(gl::LevelIndex baseLevel,
                                         gl::LevelIndex maxLevel,
                                         gl::LevelIndex firstGeneratedLevel,
                                         gl::CubeFaceArray<gl::TexLevelMask> *redefinedLevels);

enum class ImageMipLevels
{
    EnabledLevels                 = 0,
    FullMipChainForGenerateMipmap = 1,

    InvalidEnum = 2,
};

enum class PipelineType
{
    Graphics = 0,
    Compute  = 1,

    InvalidEnum = 2,
    EnumCount   = 2,
};

// Return the log of samples.  Assumes |sampleCount| is a power of 2.  The result can be used to
// index an array based on sample count.
inline size_t PackSampleCount(int32_t sampleCount)
{
    if (sampleCount == 0)
    {
        sampleCount = 1;
    }

    // We currently only support up to 16xMSAA.
    ASSERT(1 <= sampleCount && sampleCount <= 16);
    ASSERT(gl::isPow2(sampleCount));
    return gl::ScanForward(static_cast<uint32_t>(sampleCount));
}

}  // namespace rx

// MultiDraw macro patterns
// These macros are to avoid too much code duplication as we don't want to have if detect for
// hasDrawID/BaseVertex/BaseInstance inside for loop in a multiDrawANGLE call Part of these are put
// in the header as we want to share with specialized context impl on some platforms for multidraw
#define ANGLE_SET_DRAW_ID_UNIFORM_0(drawID) \
    {}
#define ANGLE_SET_DRAW_ID_UNIFORM_1(drawID) executable->setDrawIDUniform(drawID)
#define ANGLE_SET_DRAW_ID_UNIFORM(cond) ANGLE_SET_DRAW_ID_UNIFORM_##cond

#define ANGLE_SET_BASE_VERTEX_UNIFORM_0(baseVertex) \
    {}
#define ANGLE_SET_BASE_VERTEX_UNIFORM_1(baseVertex) executable->setBaseVertexUniform(baseVertex);
#define ANGLE_SET_BASE_VERTEX_UNIFORM(cond) ANGLE_SET_BASE_VERTEX_UNIFORM_##cond

#define ANGLE_SET_BASE_INSTANCE_UNIFORM_0(baseInstance) \
    {}
#define ANGLE_SET_BASE_INSTANCE_UNIFORM_1(baseInstance) \
    executable->setBaseInstanceUniform(baseInstance)
#define ANGLE_SET_BASE_INSTANCE_UNIFORM(cond) ANGLE_SET_BASE_INSTANCE_UNIFORM_##cond

#define ANGLE_NOOP_DRAW_ context->noopDraw(mode, counts[drawID])
#define ANGLE_NOOP_DRAW_INSTANCED \
    context->noopDrawInstanced(mode, counts[drawID], instanceCounts[drawID])
#define ANGLE_NOOP_DRAW(_instanced) ANGLE_NOOP_DRAW##_instanced

#define ANGLE_MARK_TRANSFORM_FEEDBACK_USAGE_ \
    gl::MarkTransformFeedbackBufferUsage(context, counts[drawID], 1)
#define ANGLE_MARK_TRANSFORM_FEEDBACK_USAGE_INSTANCED \
    gl::MarkTransformFeedbackBufferUsage(context, counts[drawID], instanceCounts[drawID])
#define ANGLE_MARK_TRANSFORM_FEEDBACK_USAGE(instanced) \
    ANGLE_MARK_TRANSFORM_FEEDBACK_USAGE##instanced

#endif  // LIBANGLE_RENDERER_RENDERER_UTILS_H_
