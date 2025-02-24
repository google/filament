//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// utilities.h: Conversion functions and other utility routines.

#ifndef COMMON_UTILITIES_H_
#define COMMON_UTILITIES_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLSLANG/ShaderLang.h>

#include <math.h>
#include <string>
#include <vector>

#include "angle_gl.h"

#include "common/PackedEnums.h"
#include "common/mathutil.h"
#include "common/platform.h"

namespace sh
{
struct ShaderVariable;
}

namespace gl
{

int VariableComponentCount(GLenum type);
GLenum VariableComponentType(GLenum type);
size_t VariableComponentSize(GLenum type);
size_t VariableInternalSize(GLenum type);
size_t VariableExternalSize(GLenum type);
int VariableRowCount(GLenum type);
int VariableColumnCount(GLenum type);
bool IsSamplerType(GLenum type);
bool IsSamplerCubeType(GLenum type);
bool IsSamplerYUVType(GLenum type);
bool IsImageType(GLenum type);
bool IsImage2DType(GLenum type);
bool IsAtomicCounterType(GLenum type);
bool IsOpaqueType(GLenum type);
bool IsMatrixType(GLenum type);
GLenum TransposeMatrixType(GLenum type);
int VariableRegisterCount(GLenum type);
int MatrixRegisterCount(GLenum type, bool isRowMajorMatrix);
int MatrixComponentCount(GLenum type, bool isRowMajorMatrix);
int VariableSortOrder(GLenum type);
GLenum VariableBoolVectorType(GLenum type);
std::string GetGLSLTypeString(GLenum type);

int AllocateFirstFreeBits(unsigned int *bits, unsigned int allocationSize, unsigned int bitsSize);

// Parse the base resource name and array indices. Returns the base name of the resource.
// If the provided name doesn't index an array, the outSubscripts vector will be empty.
// If the provided name indexes an array, the outSubscripts vector will contain indices with
// outermost array indices in the back. If an array index is invalid, GL_INVALID_INDEX is added to
// outSubscripts.
std::string ParseResourceName(const std::string &name, std::vector<unsigned int> *outSubscripts);

bool IsBuiltInName(const char *name);
ANGLE_INLINE bool IsBuiltInName(const std::string &name)
{
    return IsBuiltInName(name.c_str());
}

// Strips only the last array index from a resource name.
std::string StripLastArrayIndex(const std::string &name);

bool SamplerNameContainsNonZeroArrayElement(const std::string &name);

// Find the range of index values in the provided indices pointer.  Primitive restart indices are
// only counted in the range if primitive restart is disabled.
IndexRange ComputeIndexRange(DrawElementsType indexType,
                             const GLvoid *indices,
                             size_t count,
                             bool primitiveRestartEnabled);

// Get the primitive restart index value for the given index type.
GLuint GetPrimitiveRestartIndex(DrawElementsType indexType);

// Get the primitive restart index value with the given C++ type.
template <typename T>
constexpr T GetPrimitiveRestartIndexFromType()
{
    return std::numeric_limits<T>::max();
}

static_assert(GetPrimitiveRestartIndexFromType<uint8_t>() == 0xFF,
              "verify restart index for uint8_t values");
static_assert(GetPrimitiveRestartIndexFromType<uint16_t>() == 0xFFFF,
              "verify restart index for uint8_t values");
static_assert(GetPrimitiveRestartIndexFromType<uint32_t>() == 0xFFFFFFFF,
              "verify restart index for uint8_t values");

bool IsTriangleMode(PrimitiveMode drawMode);
bool IsPolygonMode(PrimitiveMode mode);

namespace priv
{
extern const angle::PackedEnumMap<PrimitiveMode, bool> gLineModes;
}  // namespace priv

ANGLE_INLINE bool IsLineMode(PrimitiveMode primitiveMode)
{
    return priv::gLineModes[primitiveMode];
}

bool IsIntegerFormat(GLenum unsizedFormat);

// Returns the product of the sizes in the vector, or 1 if the vector is empty. Doesn't currently
// perform overflow checks.
unsigned int ArraySizeProduct(const std::vector<unsigned int> &arraySizes);
// Returns the product of the sizes in the vector except for the outermost dimension, or 1 if the
// vector is empty.
unsigned int InnerArraySizeProduct(const std::vector<unsigned int> &arraySizes);
// Returns the outermost array dimension, or 1 if the vector is empty.
unsigned int OutermostArraySize(const std::vector<unsigned int> &arraySizes);

// Return the array index at the end of name, and write the length of name before the final array
// index into nameLengthWithoutArrayIndexOut. In case name doesn't include an array index, return
// GL_INVALID_INDEX and write the length of the original string.
unsigned int ParseArrayIndex(const std::string &name, size_t *nameLengthWithoutArrayIndexOut);

enum class SamplerFormat : uint8_t
{
    Float    = 0,
    Unsigned = 1,
    Signed   = 2,
    Shadow   = 3,

    InvalidEnum = 4,
    EnumCount   = 4,
};

struct UniformTypeInfo final : angle::NonCopyable
{
    inline constexpr UniformTypeInfo(GLenum type,
                                     GLenum componentType,
                                     GLenum textureType,
                                     GLenum transposedMatrixType,
                                     GLenum boolVectorType,
                                     SamplerFormat samplerFormat,
                                     int rowCount,
                                     int columnCount,
                                     int componentCount,
                                     size_t componentSize,
                                     size_t internalSize,
                                     size_t externalSize,
                                     bool isSampler,
                                     bool isMatrixType,
                                     bool isImageType);

    GLenum type;
    GLenum componentType;
    GLenum textureType;
    GLenum transposedMatrixType;
    GLenum boolVectorType;
    SamplerFormat samplerFormat;
    int rowCount;
    int columnCount;
    int componentCount;
    size_t componentSize;
    size_t internalSize;
    size_t externalSize;
    bool isSampler;
    bool isMatrixType;
    bool isImageType;
};

inline constexpr UniformTypeInfo::UniformTypeInfo(GLenum type,
                                                  GLenum componentType,
                                                  GLenum textureType,
                                                  GLenum transposedMatrixType,
                                                  GLenum boolVectorType,
                                                  SamplerFormat samplerFormat,
                                                  int rowCount,
                                                  int columnCount,
                                                  int componentCount,
                                                  size_t componentSize,
                                                  size_t internalSize,
                                                  size_t externalSize,
                                                  bool isSampler,
                                                  bool isMatrixType,
                                                  bool isImageType)
    : type(type),
      componentType(componentType),
      textureType(textureType),
      transposedMatrixType(transposedMatrixType),
      boolVectorType(boolVectorType),
      samplerFormat(samplerFormat),
      rowCount(rowCount),
      columnCount(columnCount),
      componentCount(componentCount),
      componentSize(componentSize),
      internalSize(internalSize),
      externalSize(externalSize),
      isSampler(isSampler),
      isMatrixType(isMatrixType),
      isImageType(isImageType)
{}

struct UniformTypeIndex
{
    uint16_t value;
};
const UniformTypeInfo &GetUniformTypeInfo(GLenum uniformType);
UniformTypeIndex GetUniformTypeIndex(GLenum uniformType);

const char *GetGenericErrorMessage(GLenum error);

unsigned int ElementTypeSize(GLenum elementType);

bool IsMipmapFiltered(GLenum minFilterMode);

template <typename T>
T GetClampedVertexCount(size_t vertexCount)
{
    static constexpr size_t kMax = static_cast<size_t>(std::numeric_limits<T>::max());
    return static_cast<T>(vertexCount > kMax ? kMax : vertexCount);
}

enum class PipelineType
{
    GraphicsPipeline = 0,
    ComputePipeline  = 1,
};

PipelineType GetPipelineType(ShaderType shaderType);

// For use with KHR_debug.
const char *GetDebugMessageSourceString(GLenum source);
const char *GetDebugMessageTypeString(GLenum type);
const char *GetDebugMessageSeverityString(GLenum severity);

// For use with EXT_texture_sRGB_decode
// A texture may be forced to skip decoding to a linear colorspace even if its format
// is in sRGB colorspace.
//
// Default - decode data according to the image's format's colorspace
// Skip - data is not decoded during sampling
enum class SrgbDecode
{
    Default = 0,
    Skip
};

// For use with EXT_texture_format_sRGB_override
// A texture may be forced to decode data to linear colorspace even if its format
// is in linear colorspace.
//
// Default - decode data according to the image's format's colorspace
// SRGB - data will be decoded to linear colorspace irrespective of texture's format
enum class SrgbOverride
{
    Default = 0,
    SRGB
};

// For use with EXT_sRGB_write_control
// A framebuffer may be forced to not encode data to sRGB colorspace even if its format
// is in sRGB colorspace.
//
// Default - encode data according to the image's format's colorspace
// Linear - data will not be encoded into sRGB colorspace
enum class SrgbWriteControlMode
{
    Default = 0,
    Linear  = 1
};

// For use with EXT_YUV_target
// A sampler of external YUV textures may either implicitly perform RGB conversion (regular
// samplerExternalOES) or skip the conversion and sample raw YUV values (__samplerExternal2DY2Y).
enum class YuvSamplingMode
{
    Default = 0,
    Y2Y     = 1
};

ShaderType GetShaderTypeFromBitfield(size_t singleShaderType);
GLbitfield GetBitfieldFromShaderType(ShaderType shaderType);
bool ShaderTypeSupportsTransformFeedback(ShaderType shaderType);
// Given a set of shader stages, returns the last vertex processing stage.  This is the stage that
// interfaces the fragment shader.
ShaderType GetLastPreFragmentStage(ShaderBitSet shaderTypes);

}  // namespace gl

namespace egl
{
// For use with EGL_EXT_image_gl_colorspace
// An EGLImage can be created with attributes that override the color space of underlying image data
// when rendering to the image, or sampling from the image. The possible values are -
//  Default - EGLImage source's colorspace should be preserved
//  sRGB - EGLImage targets will assume sRGB colorspace
//  Linear - EGLImage targets will assume linear colorspace
enum class ImageColorspace
{
    Default = 0,
    SRGB,
    Linear
};

static const EGLenum FirstCubeMapTextureTarget = EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR;
static const EGLenum LastCubeMapTextureTarget  = EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR;
bool IsCubeMapTextureTarget(EGLenum target);
size_t CubeMapTextureTargetToLayerIndex(EGLenum target);
EGLenum LayerIndexToCubeMapTextureTarget(size_t index);
bool IsTextureTarget(EGLenum target);
bool IsRenderbufferTarget(EGLenum target);
bool IsExternalImageTarget(EGLenum target);

const char *GetGenericErrorMessage(EGLint error);
}  // namespace egl

namespace egl_gl
{
GLuint EGLClientBufferToGLObjectHandle(EGLClientBuffer buffer);
}

namespace gl_egl
{
EGLenum GLComponentTypeToEGLColorComponentType(GLenum glComponentType);
EGLClientBuffer GLObjectHandleToEGLClientBuffer(GLuint handle);
}  // namespace gl_egl

namespace angle
{

// All state that modify attachment's colorspace
struct ColorspaceState
{
  public:
    ColorspaceState() { reset(); }
    void reset()
    {
        hasStaticTexelFetchAccess = false;
        srgbDecode                = gl::SrgbDecode::Default;
        srgbOverride              = gl::SrgbOverride::Default;
        srgbWriteControl          = gl::SrgbWriteControlMode::Default;
        eglImageColorspace        = egl::ImageColorspace::Default;
    }

    // States that affect read operations
    bool hasStaticTexelFetchAccess;
    gl::SrgbDecode srgbDecode;
    gl::SrgbOverride srgbOverride;

    // States that affect write operations
    gl::SrgbWriteControlMode srgbWriteControl;

    // States that affect both read and write operations
    egl::ImageColorspace eglImageColorspace;
};

template <typename T>
constexpr size_t ConstStrLen(T s)
{
    if (s == nullptr)
    {
        return 0;
    }
    return std::char_traits<char>::length(s);
}

bool IsDrawEntryPoint(EntryPoint entryPoint);
bool IsDispatchEntryPoint(EntryPoint entryPoint);
bool IsClearEntryPoint(EntryPoint entryPoint);
bool IsQueryEntryPoint(EntryPoint entryPoint);

template <typename T>
void FillWithNullptr(T *array)
{
    // std::array::fill(nullptr) yields unoptimized, unrolled loop over array items
    memset(array->data(), 0, array->size() * sizeof(*array->data()));
    // sanity check for non-0 nullptr
    ASSERT(array->data()[0] == nullptr);
}
}  // namespace angle

void writeFile(const char *path, const void *data, size_t size);

// Get the underlying type. Useful for indexing into arrays with enum values by avoiding the clutter
// of the extraneous static_cast<>() calls.
// https://stackoverflow.com/a/8357462
template <typename E>
constexpr typename std::underlying_type<E>::type ToUnderlying(E e) noexcept
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

#endif  // COMMON_UTILITIES_H_
