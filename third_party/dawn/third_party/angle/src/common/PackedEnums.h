// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PackedGLEnums_autogen.h:
//   Declares ANGLE-specific enums classes for GLEnum and functions operating
//   on them.

#ifndef COMMON_PACKEDGLENUMS_H_
#define COMMON_PACKEDGLENUMS_H_

#include "common/PackedEGLEnums_autogen.h"
#include "common/PackedGLEnums_autogen.h"

#include <array>
#include <bitset>
#include <cstddef>

#include <EGL/egl.h>

#include "common/bitset_utils.h"

namespace angle
{

// Return the number of elements of a packed enum, including the InvalidEnum element.
template <typename E>
constexpr size_t EnumSize()
{
    using UnderlyingType = typename std::underlying_type<E>::type;
    return static_cast<UnderlyingType>(E::EnumCount);
}

// Implementation of AllEnums which allows iterating over all the possible values for a packed enums
// like so:
//     for (auto value : AllEnums<MyPackedEnum>()) {
//         // Do something with the enum.
//     }

template <typename E>
class EnumIterator final
{
  private:
    using UnderlyingType = typename std::underlying_type<E>::type;

  public:
    EnumIterator(E value) : mValue(static_cast<UnderlyingType>(value)) {}
    EnumIterator &operator++()
    {
        mValue++;
        return *this;
    }
    bool operator==(const EnumIterator &other) const { return mValue == other.mValue; }
    bool operator!=(const EnumIterator &other) const { return mValue != other.mValue; }
    E operator*() const { return static_cast<E>(mValue); }

  private:
    UnderlyingType mValue;
};

template <typename E, size_t MaxSize = EnumSize<E>()>
struct AllEnums
{
    EnumIterator<E> begin() const { return {static_cast<E>(0)}; }
    EnumIterator<E> end() const { return {static_cast<E>(MaxSize)}; }
};

// PackedEnumMap<E, T> is like an std::array<T, E::EnumCount> but is indexed with enum values. It
// implements all of the std::array interface except with enum values instead of indices.
template <typename E, typename T, size_t MaxSize = EnumSize<E>()>
class PackedEnumMap
{
    using UnderlyingType = typename std::underlying_type<E>::type;
    using Storage        = std::array<T, MaxSize>;

  public:
    using InitPair = std::pair<E, T>;

    constexpr PackedEnumMap() = default;

    constexpr PackedEnumMap(std::initializer_list<InitPair> init) : mPrivateData{}
    {
        // We use a for loop instead of range-for to work around a limitation in MSVC.
        for (const InitPair *it = init.begin(); it != init.end(); ++it)
        {
            mPrivateData[static_cast<UnderlyingType>(it->first)] = it->second;
        }
    }

    // types:
    using value_type      = T;
    using pointer         = T *;
    using const_pointer   = const T *;
    using reference       = T &;
    using const_reference = const T &;

    using size_type       = size_t;
    using difference_type = ptrdiff_t;

    using iterator               = typename Storage::iterator;
    using const_iterator         = typename Storage::const_iterator;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // No explicit construct/copy/destroy for aggregate type
    void fill(const T &u) { mPrivateData.fill(u); }
    void swap(PackedEnumMap<E, T, MaxSize> &a) noexcept { mPrivateData.swap(a.mPrivateData); }

    // iterators:
    iterator begin() noexcept { return mPrivateData.begin(); }
    const_iterator begin() const noexcept { return mPrivateData.begin(); }
    iterator end() noexcept { return mPrivateData.end(); }
    const_iterator end() const noexcept { return mPrivateData.end(); }

    reverse_iterator rbegin() noexcept { return mPrivateData.rbegin(); }
    const_reverse_iterator rbegin() const noexcept { return mPrivateData.rbegin(); }
    reverse_iterator rend() noexcept { return mPrivateData.rend(); }
    const_reverse_iterator rend() const noexcept { return mPrivateData.rend(); }

    // capacity:
    constexpr size_type size() const noexcept { return mPrivateData.size(); }
    constexpr size_type max_size() const noexcept { return mPrivateData.max_size(); }
    constexpr bool empty() const noexcept { return mPrivateData.empty(); }

    // element access:
    reference operator[](E n)
    {
        ASSERT(static_cast<size_t>(n) < mPrivateData.size());
        return mPrivateData[static_cast<UnderlyingType>(n)];
    }

    constexpr const_reference operator[](E n) const
    {
        ASSERT(static_cast<size_t>(n) < mPrivateData.size());
        return mPrivateData[static_cast<UnderlyingType>(n)];
    }

    const_reference at(E n) const { return mPrivateData.at(static_cast<UnderlyingType>(n)); }
    reference at(E n) { return mPrivateData.at(static_cast<UnderlyingType>(n)); }

    reference front() { return mPrivateData.front(); }
    const_reference front() const { return mPrivateData.front(); }
    reference back() { return mPrivateData.back(); }
    const_reference back() const { return mPrivateData.back(); }

    T *data() noexcept { return mPrivateData.data(); }
    const T *data() const noexcept { return mPrivateData.data(); }

    bool operator==(const PackedEnumMap &rhs) const { return mPrivateData == rhs.mPrivateData; }
    bool operator!=(const PackedEnumMap &rhs) const { return mPrivateData != rhs.mPrivateData; }

    template <typename SubT = T>
    typename std::enable_if<std::is_integral<SubT>::value>::type operator+=(
        const PackedEnumMap<E, SubT, MaxSize> &rhs)
    {
        for (E e : AllEnums<E, MaxSize>())
        {
            at(e) += rhs[e];
        }
    }

  private:
    Storage mPrivateData;
};

// PackedEnumBitSetE> is like an std::bitset<E::EnumCount> but is indexed with enum values. It
// implements the std::bitset interface except with enum values instead of indices.
template <typename E, typename DataT = uint32_t>
using PackedEnumBitSet = BitSetT<EnumSize<E>(), DataT, E>;

}  // namespace angle

#define ANGLE_DEFINE_ID_TYPE(Type)          \
    class Type;                             \
    struct Type##ID                         \
    {                                       \
        GLuint value;                       \
    };                                      \
    template <>                             \
    struct ResourceTypeToID<Type>           \
    {                                       \
        using IDType = Type##ID;            \
    };                                      \
    template <>                             \
    struct IsResourceIDType<Type##ID>       \
    {                                       \
        static constexpr bool value = true; \
    };

namespace gl
{

TextureType TextureTargetToType(TextureTarget target);
TextureTarget NonCubeTextureTypeToTarget(TextureType type);

TextureTarget CubeFaceIndexToTextureTarget(size_t face);
size_t CubeMapTextureTargetToFaceIndex(TextureTarget target);
bool IsCubeMapFaceTarget(TextureTarget target);

constexpr TextureTarget kCubeMapTextureTargetMin = TextureTarget::CubeMapPositiveX;
constexpr TextureTarget kCubeMapTextureTargetMax = TextureTarget::CubeMapNegativeZ;
constexpr TextureTarget kAfterCubeMapTextureTargetMax =
    static_cast<TextureTarget>(static_cast<uint8_t>(kCubeMapTextureTargetMax) + 1);
struct AllCubeFaceTextureTargets
{
    angle::EnumIterator<TextureTarget> begin() const { return kCubeMapTextureTargetMin; }
    angle::EnumIterator<TextureTarget> end() const { return kAfterCubeMapTextureTargetMax; }
};

constexpr std::array<ShaderType, 2> kAllGLES2ShaderTypes = {ShaderType::Vertex,
                                                            ShaderType::Fragment};

constexpr ShaderType kShaderTypeMin = ShaderType::Vertex;
constexpr ShaderType kShaderTypeMax = ShaderType::Compute;
constexpr ShaderType kAfterShaderTypeMax =
    static_cast<ShaderType>(static_cast<uint8_t>(kShaderTypeMax) + 1);
struct AllShaderTypes
{
    angle::EnumIterator<ShaderType> begin() const { return kShaderTypeMin; }
    angle::EnumIterator<ShaderType> end() const { return kAfterShaderTypeMax; }
};

constexpr size_t kGraphicsShaderCount = static_cast<size_t>(ShaderType::EnumCount) - 1u;
// Arrange the shader types in the order of rendering pipeline
constexpr std::array<ShaderType, kGraphicsShaderCount> kAllGraphicsShaderTypes = {
    ShaderType::Vertex, ShaderType::TessControl, ShaderType::TessEvaluation, ShaderType::Geometry,
    ShaderType::Fragment};

using ShaderBitSet = angle::PackedEnumBitSet<ShaderType, uint8_t>;
static_assert(sizeof(ShaderBitSet) == sizeof(uint8_t), "Unexpected size");

template <typename T>
using ShaderMap = angle::PackedEnumMap<ShaderType, T>;

const char *ShaderTypeToString(ShaderType shaderType);

TextureType SamplerTypeToTextureType(GLenum samplerType);
TextureType ImageTypeToTextureType(GLenum imageType);

bool IsMultisampled(gl::TextureType type);
bool IsArrayTextureType(gl::TextureType type);
bool IsLayeredTextureType(gl::TextureType type);

bool IsStaticBufferUsage(BufferUsage useage);

enum class PrimitiveMode : uint8_t
{
    Points                 = 0x0,
    Lines                  = 0x1,
    LineLoop               = 0x2,
    LineStrip              = 0x3,
    Triangles              = 0x4,
    TriangleStrip          = 0x5,
    TriangleFan            = 0x6,
    Unused1                = 0x7,
    Unused2                = 0x8,
    Unused3                = 0x9,
    LinesAdjacency         = 0xA,
    LineStripAdjacency     = 0xB,
    TrianglesAdjacency     = 0xC,
    TriangleStripAdjacency = 0xD,
    Patches                = 0xE,

    InvalidEnum = 0xF,
    EnumCount   = 0xF,
};

template <>
constexpr PrimitiveMode FromGLenum<PrimitiveMode>(GLenum from)
{
    if (from >= static_cast<GLenum>(PrimitiveMode::EnumCount))
    {
        return PrimitiveMode::InvalidEnum;
    }

    return static_cast<PrimitiveMode>(from);
}

constexpr GLenum ToGLenum(PrimitiveMode from)
{
    return static_cast<GLenum>(from);
}

static_assert(ToGLenum(PrimitiveMode::Points) == GL_POINTS, "PrimitiveMode violation");
static_assert(ToGLenum(PrimitiveMode::Lines) == GL_LINES, "PrimitiveMode violation");
static_assert(ToGLenum(PrimitiveMode::LineLoop) == GL_LINE_LOOP, "PrimitiveMode violation");
static_assert(ToGLenum(PrimitiveMode::LineStrip) == GL_LINE_STRIP, "PrimitiveMode violation");
static_assert(ToGLenum(PrimitiveMode::Triangles) == GL_TRIANGLES, "PrimitiveMode violation");
static_assert(ToGLenum(PrimitiveMode::TriangleStrip) == GL_TRIANGLE_STRIP,
              "PrimitiveMode violation");
static_assert(ToGLenum(PrimitiveMode::TriangleFan) == GL_TRIANGLE_FAN, "PrimitiveMode violation");
static_assert(ToGLenum(PrimitiveMode::LinesAdjacency) == GL_LINES_ADJACENCY,
              "PrimitiveMode violation");
static_assert(ToGLenum(PrimitiveMode::LineStripAdjacency) == GL_LINE_STRIP_ADJACENCY,
              "PrimitiveMode violation");
static_assert(ToGLenum(PrimitiveMode::TrianglesAdjacency) == GL_TRIANGLES_ADJACENCY,
              "PrimitiveMode violation");
static_assert(ToGLenum(PrimitiveMode::TriangleStripAdjacency) == GL_TRIANGLE_STRIP_ADJACENCY,
              "PrimitiveMode violation");

std::ostream &operator<<(std::ostream &os, PrimitiveMode value);

enum class DrawElementsType : size_t
{
    UnsignedByte  = 0,
    UnsignedShort = 1,
    UnsignedInt   = 2,
    InvalidEnum   = 3,
    EnumCount     = 3,
};

template <>
constexpr DrawElementsType FromGLenum<DrawElementsType>(GLenum from)
{

    GLenum scaled = (from - GL_UNSIGNED_BYTE);
    // This code sequence generates a ROR instruction on x86/arm. We want to check if the lowest bit
    // of scaled is set and if (scaled >> 1) is greater than a non-pot value. If we rotate the
    // lowest bit to the hightest bit both conditions can be checked with a single test.
    static_assert(sizeof(GLenum) == 4, "Update (scaled << 31) to sizeof(GLenum) * 8 - 1");
    GLenum packed = (scaled >> 1) | (scaled << 31);

    // operator ? with a simple assignment usually translates to a cmov instruction and thus avoids
    // a branch.
    packed = (packed >= static_cast<GLenum>(DrawElementsType::EnumCount))
                 ? static_cast<GLenum>(DrawElementsType::InvalidEnum)
                 : packed;

    return static_cast<DrawElementsType>(packed);
}

constexpr GLenum ToGLenum(DrawElementsType from)
{
    return ((static_cast<GLenum>(from) << 1) + GL_UNSIGNED_BYTE);
}

#define ANGLE_VALIDATE_PACKED_ENUM(type, packed, glenum)                 \
    static_assert(ToGLenum(type::packed) == glenum, #type " violation"); \
    static_assert(FromGLenum<type>(glenum) == type::packed, #type " violation")

ANGLE_VALIDATE_PACKED_ENUM(DrawElementsType, UnsignedByte, GL_UNSIGNED_BYTE);
ANGLE_VALIDATE_PACKED_ENUM(DrawElementsType, UnsignedShort, GL_UNSIGNED_SHORT);
ANGLE_VALIDATE_PACKED_ENUM(DrawElementsType, UnsignedInt, GL_UNSIGNED_INT);

std::ostream &operator<<(std::ostream &os, DrawElementsType value);

enum class BlendEquationType
{
    Add             = 0,  // GLenum == 0x8006
    Min             = 1,  // GLenum == 0x8007
    Max             = 2,  // GLenum == 0x8008
    Unused          = 3,
    Subtract        = 4,  // GLenum == 0x800A
    ReverseSubtract = 5,  // GLenum == 0x800B

    Multiply   = 6,   // GLenum == 0x9294
    Screen     = 7,   // GLenum == 0x9295
    Overlay    = 8,   // GLenum == 0x9296
    Darken     = 9,   // GLenum == 0x9297
    Lighten    = 10,  // GLenum == 0x9298
    Colordodge = 11,  // GLenum == 0x9299
    Colorburn  = 12,  // GLenum == 0x929A
    Hardlight  = 13,  // GLenum == 0x929B
    Softlight  = 14,  // GLenum == 0x929C
    Unused2    = 15,
    Difference = 16,  // GLenum == 0x929E
    Unused3    = 17,
    Exclusion  = 18,  // GLenum == 0x92A0

    HslHue        = 19,  // GLenum == 0x92AD
    HslSaturation = 20,  // GLenum == 0x92AE
    HslColor      = 21,  // GLenum == 0x92AF
    HslLuminosity = 22,  // GLenum == 0x92B0

    InvalidEnum = 23,
    EnumCount   = InvalidEnum
};

using BlendEquationBitSet = angle::PackedEnumBitSet<gl::BlendEquationType>;

template <>
constexpr BlendEquationType FromGLenum<BlendEquationType>(GLenum from)
{
    if (from <= GL_FUNC_REVERSE_SUBTRACT)
    {
        const GLenum scaled = (from - GL_FUNC_ADD);
        return (scaled == static_cast<GLenum>(BlendEquationType::Unused))
                   ? BlendEquationType::InvalidEnum
                   : static_cast<BlendEquationType>(scaled);
    }
    if (from <= GL_EXCLUSION_KHR)
    {
        const GLenum scaled =
            (from - GL_MULTIPLY_KHR + static_cast<uint32_t>(BlendEquationType::Multiply));
        return (scaled == static_cast<GLenum>(BlendEquationType::Unused2) ||
                scaled == static_cast<GLenum>(BlendEquationType::Unused3))
                   ? BlendEquationType::InvalidEnum
                   : static_cast<BlendEquationType>(scaled);
    }
    if (from <= GL_HSL_LUMINOSITY_KHR)
    {
        return static_cast<BlendEquationType>(from - GL_HSL_HUE_KHR +
                                              static_cast<uint32_t>(BlendEquationType::HslHue));
    }
    return BlendEquationType::InvalidEnum;
}

constexpr GLenum ToGLenum(BlendEquationType from)
{
    if (from <= BlendEquationType::ReverseSubtract)
    {
        return static_cast<GLenum>(from) + GL_FUNC_ADD;
    }
    if (from <= BlendEquationType::Exclusion)
    {
        return static_cast<GLenum>(from) - static_cast<GLenum>(BlendEquationType::Multiply) +
               GL_MULTIPLY_KHR;
    }
    return static_cast<GLenum>(from) - static_cast<GLenum>(BlendEquationType::HslHue) +
           GL_HSL_HUE_KHR;
}

ANGLE_VALIDATE_PACKED_ENUM(BlendEquationType, Add, GL_FUNC_ADD);
ANGLE_VALIDATE_PACKED_ENUM(BlendEquationType, Min, GL_MIN);
ANGLE_VALIDATE_PACKED_ENUM(BlendEquationType, Max, GL_MAX);
ANGLE_VALIDATE_PACKED_ENUM(BlendEquationType, Subtract, GL_FUNC_SUBTRACT);
ANGLE_VALIDATE_PACKED_ENUM(BlendEquationType, ReverseSubtract, GL_FUNC_REVERSE_SUBTRACT);
ANGLE_VALIDATE_PACKED_ENUM(BlendEquationType, Multiply, GL_MULTIPLY_KHR);
ANGLE_VALIDATE_PACKED_ENUM(BlendEquationType, Screen, GL_SCREEN_KHR);
ANGLE_VALIDATE_PACKED_ENUM(BlendEquationType, Overlay, GL_OVERLAY_KHR);
ANGLE_VALIDATE_PACKED_ENUM(BlendEquationType, Darken, GL_DARKEN_KHR);
ANGLE_VALIDATE_PACKED_ENUM(BlendEquationType, Lighten, GL_LIGHTEN_KHR);
ANGLE_VALIDATE_PACKED_ENUM(BlendEquationType, Colordodge, GL_COLORDODGE_KHR);
ANGLE_VALIDATE_PACKED_ENUM(BlendEquationType, Colorburn, GL_COLORBURN_KHR);
ANGLE_VALIDATE_PACKED_ENUM(BlendEquationType, Hardlight, GL_HARDLIGHT_KHR);
ANGLE_VALIDATE_PACKED_ENUM(BlendEquationType, Softlight, GL_SOFTLIGHT_KHR);
ANGLE_VALIDATE_PACKED_ENUM(BlendEquationType, Difference, GL_DIFFERENCE_KHR);
ANGLE_VALIDATE_PACKED_ENUM(BlendEquationType, Exclusion, GL_EXCLUSION_KHR);
ANGLE_VALIDATE_PACKED_ENUM(BlendEquationType, HslHue, GL_HSL_HUE_KHR);
ANGLE_VALIDATE_PACKED_ENUM(BlendEquationType, HslSaturation, GL_HSL_SATURATION_KHR);
ANGLE_VALIDATE_PACKED_ENUM(BlendEquationType, HslColor, GL_HSL_COLOR_KHR);
ANGLE_VALIDATE_PACKED_ENUM(BlendEquationType, HslLuminosity, GL_HSL_LUMINOSITY_KHR);

std::ostream &operator<<(std::ostream &os, BlendEquationType value);

enum class BlendFactorType
{
    Zero = 0,  // GLenum == 0
    One  = 1,  // GLenum == 1

    MinSrcDstType    = 2,
    SrcColor         = 2,   // GLenum == 0x0300
    OneMinusSrcColor = 3,   // GLenum == 0x0301
    SrcAlpha         = 4,   // GLenum == 0x0302
    OneMinusSrcAlpha = 5,   // GLenum == 0x0303
    DstAlpha         = 6,   // GLenum == 0x0304
    OneMinusDstAlpha = 7,   // GLenum == 0x0305
    DstColor         = 8,   // GLenum == 0x0306
    OneMinusDstColor = 9,   // GLenum == 0x0307
    SrcAlphaSaturate = 10,  // GLenum == 0x0308
    MaxSrcDstType    = 10,

    MinConstantType       = 11,
    ConstantColor         = 11,  // GLenum == 0x8001
    OneMinusConstantColor = 12,  // GLenum == 0x8002
    ConstantAlpha         = 13,  // GLenum == 0x8003
    OneMinusConstantAlpha = 14,  // GLenum == 0x8004
    MaxConstantType       = 14,

    // GL_EXT_blend_func_extended

    Src1Alpha = 15,  // GLenum == 0x8589

    Src1Color         = 16,  // GLenum == 0x88F9
    OneMinusSrc1Color = 17,  // GLenum == 0x88FA
    OneMinusSrc1Alpha = 18,  // GLenum == 0x88FB

    InvalidEnum = 19,
    EnumCount   = 19
};

template <>
constexpr BlendFactorType FromGLenum<BlendFactorType>(GLenum from)
{
    if (from <= 1)
        return static_cast<BlendFactorType>(from);
    if (from >= GL_SRC_COLOR && from <= GL_SRC_ALPHA_SATURATE)
        return static_cast<BlendFactorType>(from - GL_SRC_COLOR + 2);
    if (from >= GL_CONSTANT_COLOR && from <= GL_ONE_MINUS_CONSTANT_ALPHA)
        return static_cast<BlendFactorType>(from - GL_CONSTANT_COLOR + 11);
    if (from == GL_SRC1_ALPHA_EXT)
        return BlendFactorType::Src1Alpha;
    if (from >= GL_SRC1_COLOR_EXT && from <= GL_ONE_MINUS_SRC1_ALPHA_EXT)
        return static_cast<BlendFactorType>(from - GL_SRC1_COLOR_EXT + 16);
    return BlendFactorType::InvalidEnum;
}

constexpr GLenum ToGLenum(BlendFactorType from)
{
    const GLenum value = static_cast<GLenum>(from);
    if (value <= 1)
        return value;
    if (from >= BlendFactorType::MinSrcDstType && from <= BlendFactorType::MaxSrcDstType)
        return value - 2 + GL_SRC_COLOR;
    if (from >= BlendFactorType::MinConstantType && from <= BlendFactorType::MaxConstantType)
        return value - 11 + GL_CONSTANT_COLOR;
    if (from == BlendFactorType::Src1Alpha)
        return GL_SRC1_ALPHA_EXT;
    return value - 16 + GL_SRC1_COLOR_EXT;
}

ANGLE_VALIDATE_PACKED_ENUM(BlendFactorType, Zero, GL_ZERO);
ANGLE_VALIDATE_PACKED_ENUM(BlendFactorType, One, GL_ONE);
ANGLE_VALIDATE_PACKED_ENUM(BlendFactorType, SrcColor, GL_SRC_COLOR);
ANGLE_VALIDATE_PACKED_ENUM(BlendFactorType, OneMinusSrcColor, GL_ONE_MINUS_SRC_COLOR);
ANGLE_VALIDATE_PACKED_ENUM(BlendFactorType, SrcAlpha, GL_SRC_ALPHA);
ANGLE_VALIDATE_PACKED_ENUM(BlendFactorType, OneMinusSrcAlpha, GL_ONE_MINUS_SRC_ALPHA);
ANGLE_VALIDATE_PACKED_ENUM(BlendFactorType, DstAlpha, GL_DST_ALPHA);
ANGLE_VALIDATE_PACKED_ENUM(BlendFactorType, OneMinusDstAlpha, GL_ONE_MINUS_DST_ALPHA);
ANGLE_VALIDATE_PACKED_ENUM(BlendFactorType, DstColor, GL_DST_COLOR);
ANGLE_VALIDATE_PACKED_ENUM(BlendFactorType, OneMinusDstColor, GL_ONE_MINUS_DST_COLOR);
ANGLE_VALIDATE_PACKED_ENUM(BlendFactorType, SrcAlphaSaturate, GL_SRC_ALPHA_SATURATE);
ANGLE_VALIDATE_PACKED_ENUM(BlendFactorType, ConstantColor, GL_CONSTANT_COLOR);
ANGLE_VALIDATE_PACKED_ENUM(BlendFactorType, OneMinusConstantColor, GL_ONE_MINUS_CONSTANT_COLOR);
ANGLE_VALIDATE_PACKED_ENUM(BlendFactorType, ConstantAlpha, GL_CONSTANT_ALPHA);
ANGLE_VALIDATE_PACKED_ENUM(BlendFactorType, OneMinusConstantAlpha, GL_ONE_MINUS_CONSTANT_ALPHA);
ANGLE_VALIDATE_PACKED_ENUM(BlendFactorType, Src1Alpha, GL_SRC1_ALPHA_EXT);
ANGLE_VALIDATE_PACKED_ENUM(BlendFactorType, Src1Color, GL_SRC1_COLOR_EXT);
ANGLE_VALIDATE_PACKED_ENUM(BlendFactorType, OneMinusSrc1Color, GL_ONE_MINUS_SRC1_COLOR_EXT);
ANGLE_VALIDATE_PACKED_ENUM(BlendFactorType, OneMinusSrc1Alpha, GL_ONE_MINUS_SRC1_ALPHA_EXT);

std::ostream &operator<<(std::ostream &os, BlendFactorType value);

enum class VertexAttribType
{
    Byte               = 0,   // GLenum == 0x1400
    UnsignedByte       = 1,   // GLenum == 0x1401
    Short              = 2,   // GLenum == 0x1402
    UnsignedShort      = 3,   // GLenum == 0x1403
    Int                = 4,   // GLenum == 0x1404
    UnsignedInt        = 5,   // GLenum == 0x1405
    Float              = 6,   // GLenum == 0x1406
    Unused1            = 7,   // GLenum == 0x1407
    Unused2            = 8,   // GLenum == 0x1408
    Unused3            = 9,   // GLenum == 0x1409
    Unused4            = 10,  // GLenum == 0x140A
    HalfFloat          = 11,  // GLenum == 0x140B
    Fixed              = 12,  // GLenum == 0x140C
    MaxBasicType       = 12,
    UnsignedInt2101010 = 13,  // GLenum == 0x8368
    HalfFloatOES       = 14,  // GLenum == 0x8D61
    Int2101010         = 15,  // GLenum == 0x8D9F
    UnsignedInt1010102 = 16,  // GLenum == 0x8DF6
    Int1010102         = 17,  // GLenum == 0x8DF7
    InvalidEnum        = 18,
    EnumCount          = 18,
};

template <>
constexpr VertexAttribType FromGLenum<VertexAttribType>(GLenum from)
{
    GLenum packed = from - GL_BYTE;
    if (packed <= static_cast<GLenum>(VertexAttribType::MaxBasicType))
        return static_cast<VertexAttribType>(packed);
    if (from == GL_UNSIGNED_INT_2_10_10_10_REV)
        return VertexAttribType::UnsignedInt2101010;
    if (from == GL_HALF_FLOAT_OES)
        return VertexAttribType::HalfFloatOES;
    if (from == GL_INT_2_10_10_10_REV)
        return VertexAttribType::Int2101010;
    if (from == GL_UNSIGNED_INT_10_10_10_2_OES)
        return VertexAttribType::UnsignedInt1010102;
    if (from == GL_INT_10_10_10_2_OES)
        return VertexAttribType::Int1010102;
    return VertexAttribType::InvalidEnum;
}

constexpr GLenum ToGLenum(VertexAttribType from)
{
    // This could be optimized using a constexpr table.
    if (from == VertexAttribType::Int2101010)
        return GL_INT_2_10_10_10_REV;
    if (from == VertexAttribType::HalfFloatOES)
        return GL_HALF_FLOAT_OES;
    if (from == VertexAttribType::UnsignedInt2101010)
        return GL_UNSIGNED_INT_2_10_10_10_REV;
    if (from == VertexAttribType::UnsignedInt1010102)
        return GL_UNSIGNED_INT_10_10_10_2_OES;
    if (from == VertexAttribType::Int1010102)
        return GL_INT_10_10_10_2_OES;
    return static_cast<GLenum>(from) + GL_BYTE;
}

ANGLE_VALIDATE_PACKED_ENUM(VertexAttribType, Byte, GL_BYTE);
ANGLE_VALIDATE_PACKED_ENUM(VertexAttribType, UnsignedByte, GL_UNSIGNED_BYTE);
ANGLE_VALIDATE_PACKED_ENUM(VertexAttribType, Short, GL_SHORT);
ANGLE_VALIDATE_PACKED_ENUM(VertexAttribType, UnsignedShort, GL_UNSIGNED_SHORT);
ANGLE_VALIDATE_PACKED_ENUM(VertexAttribType, Int, GL_INT);
ANGLE_VALIDATE_PACKED_ENUM(VertexAttribType, UnsignedInt, GL_UNSIGNED_INT);
ANGLE_VALIDATE_PACKED_ENUM(VertexAttribType, Float, GL_FLOAT);
ANGLE_VALIDATE_PACKED_ENUM(VertexAttribType, HalfFloat, GL_HALF_FLOAT);
ANGLE_VALIDATE_PACKED_ENUM(VertexAttribType, Fixed, GL_FIXED);
ANGLE_VALIDATE_PACKED_ENUM(VertexAttribType, Int2101010, GL_INT_2_10_10_10_REV);
ANGLE_VALIDATE_PACKED_ENUM(VertexAttribType, HalfFloatOES, GL_HALF_FLOAT_OES);
ANGLE_VALIDATE_PACKED_ENUM(VertexAttribType, UnsignedInt2101010, GL_UNSIGNED_INT_2_10_10_10_REV);
ANGLE_VALIDATE_PACKED_ENUM(VertexAttribType, Int1010102, GL_INT_10_10_10_2_OES);
ANGLE_VALIDATE_PACKED_ENUM(VertexAttribType, UnsignedInt1010102, GL_UNSIGNED_INT_10_10_10_2_OES);

std::ostream &operator<<(std::ostream &os, VertexAttribType value);

enum class TessEvaluationType
{
    Triangles             = 0,
    Quads                 = 1,
    Isolines              = 2,
    EqualSpacing          = 3,
    FractionalEvenSpacing = 4,
    FractionalOddSpacing  = 5,
    Cw                    = 6,
    Ccw                   = 7,
    PointMode             = 8,
    InvalidEnum           = 9,
    EnumCount             = 9
};

template <>
constexpr TessEvaluationType FromGLenum<TessEvaluationType>(GLenum from)
{
    if (from == GL_TRIANGLES)
        return TessEvaluationType::Triangles;
    if (from == GL_QUADS)
        return TessEvaluationType::Quads;
    if (from == GL_ISOLINES)
        return TessEvaluationType::Isolines;
    if (from == GL_EQUAL)
        return TessEvaluationType::EqualSpacing;
    if (from == GL_FRACTIONAL_EVEN)
        return TessEvaluationType::FractionalEvenSpacing;
    if (from == GL_FRACTIONAL_ODD)
        return TessEvaluationType::FractionalOddSpacing;
    if (from == GL_CW)
        return TessEvaluationType::Cw;
    if (from == GL_CCW)
        return TessEvaluationType::Ccw;
    if (from == GL_TESS_GEN_POINT_MODE)
        return TessEvaluationType::PointMode;
    return TessEvaluationType::InvalidEnum;
}

constexpr GLenum ToGLenum(TessEvaluationType from)
{
    switch (from)
    {
        case TessEvaluationType::Triangles:
            return GL_TRIANGLES;
        case TessEvaluationType::Quads:
            return GL_QUADS;
        case TessEvaluationType::Isolines:
            return GL_ISOLINES;
        case TessEvaluationType::EqualSpacing:
            return GL_EQUAL;
        case TessEvaluationType::FractionalEvenSpacing:
            return GL_FRACTIONAL_EVEN;
        case TessEvaluationType::FractionalOddSpacing:
            return GL_FRACTIONAL_ODD;
        case TessEvaluationType::Cw:
            return GL_CW;
        case TessEvaluationType::Ccw:
            return GL_CCW;
        case TessEvaluationType::PointMode:
            return GL_TESS_GEN_POINT_MODE;
        default:
            return GL_INVALID_ENUM;
    }
}

ANGLE_VALIDATE_PACKED_ENUM(TessEvaluationType, Triangles, GL_TRIANGLES);
ANGLE_VALIDATE_PACKED_ENUM(TessEvaluationType, Quads, GL_QUADS);
ANGLE_VALIDATE_PACKED_ENUM(TessEvaluationType, Isolines, GL_ISOLINES);
ANGLE_VALIDATE_PACKED_ENUM(TessEvaluationType, EqualSpacing, GL_EQUAL);
ANGLE_VALIDATE_PACKED_ENUM(TessEvaluationType, FractionalEvenSpacing, GL_FRACTIONAL_EVEN);
ANGLE_VALIDATE_PACKED_ENUM(TessEvaluationType, FractionalOddSpacing, GL_FRACTIONAL_ODD);
ANGLE_VALIDATE_PACKED_ENUM(TessEvaluationType, Cw, GL_CW);
ANGLE_VALIDATE_PACKED_ENUM(TessEvaluationType, Ccw, GL_CCW);
ANGLE_VALIDATE_PACKED_ENUM(TessEvaluationType, PointMode, GL_TESS_GEN_POINT_MODE);

std::ostream &operator<<(std::ostream &os, TessEvaluationType value);

// Typesafe object handles.

template <typename T>
struct ResourceTypeToID;

template <typename T>
struct IsResourceIDType;

#define ANGLE_GL_ID_TYPES_OP(X) \
    X(Buffer)                   \
    X(Context)                  \
    X(FenceNV)                  \
    X(Framebuffer)              \
    X(MemoryObject)             \
    X(Path)                     \
    X(ProgramPipeline)          \
    X(Query)                    \
    X(Renderbuffer)             \
    X(Sampler)                  \
    X(Semaphore)                \
    X(Sync)                     \
    X(Texture)                  \
    X(TransformFeedback)        \
    X(VertexArray)

ANGLE_GL_ID_TYPES_OP(ANGLE_DEFINE_ID_TYPE)

#undef ANGLE_GL_ID_TYPES_OP

// Shaders and programs are a bit special as they share IDs.
struct ShaderProgramID
{
    GLuint value;
};

template <>
struct IsResourceIDType<ShaderProgramID>
{
    constexpr static bool value = true;
};

class Shader;
template <>
struct ResourceTypeToID<Shader>
{
    using IDType = ShaderProgramID;
};

class Program;
template <>
struct ResourceTypeToID<Program>
{
    using IDType = ShaderProgramID;
};

template <typename T>
struct ResourceTypeToID
{
    using IDType = void;
};

template <typename T>
struct IsResourceIDType
{
    static constexpr bool value = false;
};

template <typename T>
bool ValueEquals(T lhs, T rhs)
{
    return lhs.value == rhs.value;
}

// Util funcs for resourceIDs
template <typename T>
typename std::enable_if<IsResourceIDType<T>::value, bool>::type operator==(const T &lhs,
                                                                           const T &rhs)
{
    return lhs.value == rhs.value;
}

template <typename T>
typename std::enable_if<IsResourceIDType<T>::value, bool>::type operator!=(const T &lhs,
                                                                           const T &rhs)
{
    return lhs.value != rhs.value;
}

template <typename T>
typename std::enable_if<IsResourceIDType<T>::value, bool>::type operator<(const T &lhs,
                                                                          const T &rhs)
{
    return lhs.value < rhs.value;
}

// Used to unbox typed values.
template <typename ResourceIDType>
GLuint GetIDValue(ResourceIDType id);

template <typename ResourceIDType>
inline GLuint GetIDValue(ResourceIDType id)
{
    return id.value;
}

struct UniformLocation
{
    int value;
};

bool operator<(const UniformLocation &lhs, const UniformLocation &rhs);

struct UniformBlockIndex
{
    uint32_t value;
};

bool IsEmulatedCompressedFormat(GLenum format);
}  // namespace gl

namespace egl
{
MessageType ErrorCodeToMessageType(EGLint errorCode);

struct Config;
class Device;
class Display;
class Image;
class Surface;
class Stream;
class Sync;

#define ANGLE_EGL_ID_TYPES_OP(X) \
    X(Image)                     \
    X(Surface)                   \
    X(Sync)

template <typename T>
struct ResourceTypeToID;

template <typename T>
struct IsResourceIDType;

ANGLE_EGL_ID_TYPES_OP(ANGLE_DEFINE_ID_TYPE)

#undef ANGLE_EGL_ID_TYPES_OP

template <>
struct IsResourceIDType<gl::ContextID>
{
    static constexpr bool value = true;
};

template <typename T>
struct IsResourceIDType
{
    static constexpr bool value = false;
};

// Util funcs for resourceIDs
template <typename T>
typename std::enable_if<IsResourceIDType<T>::value && !std::is_same<T, gl::ContextID>::value,
                        bool>::type
operator==(const T &lhs, const T &rhs)
{
    return lhs.value == rhs.value;
}

template <typename T>
typename std::enable_if<IsResourceIDType<T>::value && !std::is_same<T, gl::ContextID>::value,
                        bool>::type
operator<(const T &lhs, const T &rhs)
{
    return lhs.value < rhs.value;
}
}  // namespace egl

#undef ANGLE_DEFINE_ID_TYPE

namespace egl_gl
{
gl::TextureTarget EGLCubeMapTargetToCubeMapTarget(EGLenum eglTarget);
gl::TextureTarget EGLImageTargetToTextureTarget(EGLenum eglTarget);
gl::TextureType EGLTextureTargetToTextureType(EGLenum eglTarget);
}  // namespace egl_gl

namespace gl
{
// First case: handling packed enums.
template <typename EnumT, typename FromT>
typename std::enable_if<std::is_enum<EnumT>::value, EnumT>::type PackParam(FromT from)
{
    return FromGLenum<EnumT>(from);
}

// Second case: handling non-pointer resource ids.
template <typename EnumT, typename FromT>
typename std::enable_if<!std::is_pointer<FromT>::value && !std::is_enum<EnumT>::value, EnumT>::type
PackParam(FromT from)
{
    return {from};
}

template <typename EnumT>
using IsEGLImage = std::is_same<EnumT, egl::ImageID>;

template <typename EnumT>
using IsGLSync = std::is_same<EnumT, gl::SyncID>;

template <typename EnumT>
using IsEGLSync = std::is_same<EnumT, egl::SyncID>;

// Third case: handling EGLImage, GLSync and EGLSync pointer resource ids.
template <typename EnumT, typename FromT>
typename std::enable_if<IsEGLImage<EnumT>::value || IsGLSync<EnumT>::value ||
                            IsEGLSync<EnumT>::value,
                        EnumT>::type
PackParam(FromT from)
{
    return {static_cast<GLuint>(reinterpret_cast<uintptr_t>(from))};
}

// Fourth case: handling non-EGLImage non-GLsync resource ids.
template <typename EnumT, typename FromT>
typename std::enable_if<std::is_pointer<FromT>::value && !std::is_enum<EnumT>::value &&
                            !IsEGLImage<EnumT>::value && !IsGLSync<EnumT>::value,
                        EnumT>::type
PackParam(FromT from)
{
    static_assert(sizeof(typename std::remove_pointer<EnumT>::type) ==
                      sizeof(typename std::remove_pointer<FromT>::type),
                  "Types have different sizes");
    static_assert(
        std::is_same<
            decltype(std::remove_pointer<EnumT>::type::value),
            typename std::remove_const<typename std::remove_pointer<FromT>::type>::type>::value,
        "Data types are different");
    return reinterpret_cast<EnumT>(from);
}

// Optimized specialization to avoid function call in common cases
template <>
ANGLE_INLINE typename gl::BufferBinding PackParam<gl::BufferBinding>(GLenum from)
{
    if (ANGLE_LIKELY(from == GL_ARRAY_BUFFER))
    {
        return gl::BufferBinding::Array;
    }
    if (ANGLE_LIKELY(from == GL_ELEMENT_ARRAY_BUFFER))
    {
        return gl::BufferBinding::ElementArray;
    }
    if (ANGLE_LIKELY(from == GL_UNIFORM_BUFFER))
    {
        return gl::BufferBinding::Uniform;
    }

    // Fall back to the default implementation
    return FromGLenum<gl::BufferBinding>(from);
}
}  // namespace gl

#endif  // COMMON_PACKEDGLENUMS_H_
