//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// angletypes.h : Defines a variety of structures and enum types that are used throughout libGLESv2

#ifndef LIBANGLE_ANGLETYPES_H_
#define LIBANGLE_ANGLETYPES_H_

#include <anglebase/sha1.h>
#include "common/Color.h"
#include "common/FixedVector.h"
#include "common/MemoryBuffer.h"
#include "common/PackedEnums.h"
#include "common/bitset_utils.h"
#include "common/hash_utils.h"
#include "common/vector_utils.h"
#include "libANGLE/Constants.h"
#include "libANGLE/Error.h"
#include "libANGLE/RefCountObject.h"

#include <inttypes.h>
#include <stdint.h>

#include <bitset>
#include <functional>
#include <map>
#include <memory>
#include <unordered_map>

namespace angle
{
template <typename T>
struct Extents
{
    Extents() : width(0), height(0), depth(0) {}
    Extents(T width_, T height_, T depth_) : width(width_), height(height_), depth(depth_) {}

    Extents(const Extents &other)            = default;
    Extents &operator=(const Extents &other) = default;

    bool empty() const { return (width * height * depth) == 0; }

    T width;
    T height;
    T depth;
};

template <typename T>
struct Offset
{
  public:
    constexpr Offset() : x(0), y(0), z(0) {}
    constexpr Offset(T x_in, T y_in, T z_in) : x(x_in), y(y_in), z(z_in) {}

    T x;
    T y;
    T z;
};

template <typename T>
inline bool operator==(const Extents<T> &lhs, const Extents<T> &rhs)
{
    return lhs.width == rhs.width && lhs.height == rhs.height && lhs.depth == rhs.depth;
}

template <typename T>
inline bool operator!=(const Extents<T> &lhs, const Extents<T> &rhs)
{
    return !(lhs == rhs);
}

template <typename T>
inline bool operator==(const Offset<T> &a, const Offset<T> &b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

template <typename T>
inline bool operator!=(const Offset<T> &a, const Offset<T> &b)
{
    return !(a == b);
}

}  // namespace angle

namespace gl
{
class Buffer;
class Texture;

enum class Command
{
    // The Blit command carries the bitmask of which buffers are being blit.  The command passed to
    // the backends is:
    //
    //     Blit + (Color?0x1) + (Depth?0x2) + (Stencil?0x4)
    Blit,
    BlitAll = Blit + 0x7,
    Clear,
    ClearTexture,
    CopyImage,
    Dispatch,
    Draw,
    GenerateMipmap,
    Invalidate,
    ReadPixels,
    TexImage,
    Other,
};

enum CommandBlitBuffer
{
    CommandBlitBufferColor   = 0x1,
    CommandBlitBufferDepth   = 0x2,
    CommandBlitBufferStencil = 0x4,
};

enum class InitState
{
    MayNeedInit,
    Initialized,
};

template <typename T>
struct RectangleImpl
{
    RectangleImpl() : x(T(0)), y(T(0)), width(T(0)), height(T(0)) {}
    constexpr RectangleImpl(T x_in, T y_in, T width_in, T height_in)
        : x(x_in), y(y_in), width(width_in), height(height_in)
    {}
    explicit constexpr RectangleImpl(const T corners[4])
        : x(corners[0]),
          y(corners[1]),
          width(corners[2] - corners[0]),
          height(corners[3] - corners[1])
    {}
    template <typename S>
    explicit constexpr RectangleImpl(const RectangleImpl<S> rect)
        : x(rect.x), y(rect.y), width(rect.width), height(rect.height)
    {}

    T x0() const { return x; }
    T y0() const { return y; }
    T x1() const { return x + width; }
    T y1() const { return y + height; }

    bool isReversedX() const { return width < T(0); }
    bool isReversedY() const { return height < T(0); }

    // Returns a rectangle with the same area but flipped in X, Y, neither or both.
    RectangleImpl<T> flip(bool flipX, bool flipY) const
    {
        RectangleImpl flipped = *this;
        if (flipX)
        {
            flipped.x     = flipped.x + flipped.width;
            flipped.width = -flipped.width;
        }
        if (flipY)
        {
            flipped.y      = flipped.y + flipped.height;
            flipped.height = -flipped.height;
        }
        return flipped;
    }

    // Returns a rectangle with the same area but with height and width guaranteed to be positive.
    RectangleImpl<T> removeReversal() const { return flip(isReversedX(), isReversedY()); }

    bool encloses(const RectangleImpl<T> &inside) const
    {
        return x0() <= inside.x0() && y0() <= inside.y0() && x1() >= inside.x1() &&
               y1() >= inside.y1();
    }

    bool empty() const;

    T x;
    T y;
    T width;
    T height;
};

template <typename T>
bool operator==(const RectangleImpl<T> &a, const RectangleImpl<T> &b);
template <typename T>
bool operator!=(const RectangleImpl<T> &a, const RectangleImpl<T> &b);

using Rectangle = RectangleImpl<int>;

// Calculate the intersection of two rectangles.  Returns false if the intersection is empty.
[[nodiscard]] bool ClipRectangle(const Rectangle &source,
                                 const Rectangle &clip,
                                 Rectangle *intersection);
// Calculate the smallest rectangle that covers both rectangles.  This rectangle may cover areas
// not covered by the two rectangles, for example in this situation:
//
//   +--+        +----+
//   | ++-+  ->  |    |
//   +-++ |      |    |
//     +--+      +----+
//
void GetEnclosingRectangle(const Rectangle &rect1, const Rectangle &rect2, Rectangle *rectUnion);
// Extend the source rectangle to cover parts (or all of) the second rectangle, in such a way that
// no area is covered that isn't covered by both rectangles.  For example:
//
//             +--+        +--+
//  source --> |  |        |  |
//            ++--+-+  ->  |  |
//            |+--+ |      |  |
//            +-----+      +--+
//
void ExtendRectangle(const Rectangle &source, const Rectangle &extend, Rectangle *extended);

using Extents = angle::Extents<int>;
using Offset  = angle::Offset<int>;
constexpr Offset kOffsetZero(0, 0, 0);

struct Box
{
    Box() : x(0), y(0), z(0), width(0), height(0), depth(0) {}
    Box(int x_in, int y_in, int z_in, int width_in, int height_in, int depth_in)
        : x(x_in), y(y_in), z(z_in), width(width_in), height(height_in), depth(depth_in)
    {}
    template <typename O, typename E>
    Box(const O &offset, const E &size)
        : x(offset.x),
          y(offset.y),
          z(offset.z),
          width(size.width),
          height(size.height),
          depth(size.depth)
    {}
    bool valid() const;
    bool operator==(const Box &other) const;
    bool operator!=(const Box &other) const;
    Rectangle toRect() const;

    // Whether the Box has offset 0 and the same extents as argument.
    bool coversSameExtent(const Extents &size) const;

    bool contains(const Box &other) const;
    size_t volume() const;
    void extend(const Box &other);

    int x;
    int y;
    int z;
    int width;
    int height;
    int depth;
};

struct RasterizerState final
{
    // This will zero-initialize the struct, including padding.
    RasterizerState();
    RasterizerState(const RasterizerState &other);
    RasterizerState &operator=(const RasterizerState &other);

    bool cullFace;
    CullFaceMode cullMode;
    GLenum frontFace;

    PolygonMode polygonMode;

    bool polygonOffsetPoint;
    bool polygonOffsetLine;
    bool polygonOffsetFill;
    GLfloat polygonOffsetFactor;
    GLfloat polygonOffsetUnits;
    GLfloat polygonOffsetClamp;

    bool depthClamp;

    // pointDrawMode/multiSample are only used in the D3D back-end right now.
    bool pointDrawMode;
    bool multiSample;

    bool rasterizerDiscard;

    bool dither;

    bool isPolygonOffsetEnabled() const
    {
        static_assert(static_cast<int>(PolygonMode::Point) == 0, "PolygonMode::Point");
        static_assert(static_cast<int>(PolygonMode::Line) == 1, "PolygonMode::Line");
        static_assert(static_cast<int>(PolygonMode::Fill) == 2, "PolygonMode::Fill");
        return (1 << static_cast<int>(polygonMode)) &
               ((polygonOffsetPoint << 0) | (polygonOffsetLine << 1) | (polygonOffsetFill << 2));
    }
};

bool operator==(const RasterizerState &a, const RasterizerState &b);
bool operator!=(const RasterizerState &a, const RasterizerState &b);

struct BlendState final
{
    // This will zero-initialize the struct, including padding.
    BlendState();
    BlendState(const BlendState &other);

    bool blend;
    GLenum sourceBlendRGB;
    GLenum destBlendRGB;
    GLenum sourceBlendAlpha;
    GLenum destBlendAlpha;
    GLenum blendEquationRGB;
    GLenum blendEquationAlpha;

    bool colorMaskRed;
    bool colorMaskGreen;
    bool colorMaskBlue;
    bool colorMaskAlpha;
};

bool operator==(const BlendState &a, const BlendState &b);
bool operator!=(const BlendState &a, const BlendState &b);

struct DepthStencilState final
{
    // This will zero-initialize the struct, including padding.
    DepthStencilState();
    DepthStencilState(const DepthStencilState &other);
    DepthStencilState &operator=(const DepthStencilState &other);

    bool isDepthMaskedOut() const;
    bool isStencilMaskedOut(GLuint framebufferStencilSize) const;
    bool isStencilNoOp(GLuint framebufferStencilSize) const;
    bool isStencilBackNoOp(GLuint framebufferStencilSize) const;

    bool depthTest;
    GLenum depthFunc;
    bool depthMask;

    bool stencilTest;
    GLenum stencilFunc;
    GLuint stencilMask;
    GLenum stencilFail;
    GLenum stencilPassDepthFail;
    GLenum stencilPassDepthPass;
    GLuint stencilWritemask;
    GLenum stencilBackFunc;
    GLuint stencilBackMask;
    GLenum stencilBackFail;
    GLenum stencilBackPassDepthFail;
    GLenum stencilBackPassDepthPass;
    GLuint stencilBackWritemask;
};

bool operator==(const DepthStencilState &a, const DepthStencilState &b);
bool operator!=(const DepthStencilState &a, const DepthStencilState &b);

// Packs a sampler state for completeness checks:
// * minFilter: 5 values (3 bits)
// * magFilter: 2 values (1 bit)
// * wrapS:     3 values (2 bits)
// * wrapT:     3 values (2 bits)
// * compareMode: 1 bit (for == GL_NONE).
// This makes a total of 9 bits. We can pack this easily into 32 bits:
// * minFilter: 8 bits
// * magFilter: 8 bits
// * wrapS:     8 bits
// * wrapT:     4 bits
// * compareMode: 4 bits

struct PackedSamplerCompleteness
{
    uint8_t minFilter;
    uint8_t magFilter;
    uint8_t wrapS;
    uint8_t wrapTCompareMode;
};

static_assert(sizeof(PackedSamplerCompleteness) == sizeof(uint32_t), "Unexpected size");

// State from Table 6.10 (state per sampler object)
class SamplerState final
{
  public:
    // This will zero-initialize the struct, including padding.
    SamplerState();
    SamplerState(const SamplerState &other);

    SamplerState &operator=(const SamplerState &other);

    static SamplerState CreateDefaultForTarget(TextureType type);

    GLenum getMinFilter() const { return mMinFilter; }

    bool setMinFilter(GLenum minFilter);

    GLenum getMagFilter() const { return mMagFilter; }

    bool setMagFilter(GLenum magFilter);

    GLenum getWrapS() const { return mWrapS; }

    bool setWrapS(GLenum wrapS);

    GLenum getWrapT() const { return mWrapT; }

    bool setWrapT(GLenum wrapT);

    GLenum getWrapR() const { return mWrapR; }

    bool setWrapR(GLenum wrapR);

    bool usesBorderColor() const
    {
        return mWrapS == GL_CLAMP_TO_BORDER || mWrapT == GL_CLAMP_TO_BORDER ||
               mWrapR == GL_CLAMP_TO_BORDER;
    }

    float getMaxAnisotropy() const { return mMaxAnisotropy; }

    bool setMaxAnisotropy(float maxAnisotropy);

    GLfloat getMinLod() const { return mMinLod; }

    bool setMinLod(GLfloat minLod);

    GLfloat getMaxLod() const { return mMaxLod; }

    bool setMaxLod(GLfloat maxLod);

    GLenum getCompareMode() const { return mCompareMode; }

    bool setCompareMode(GLenum compareMode);

    GLenum getCompareFunc() const { return mCompareFunc; }

    bool setCompareFunc(GLenum compareFunc);

    GLenum getSRGBDecode() const { return mSRGBDecode; }

    bool setSRGBDecode(GLenum sRGBDecode);

    bool setBorderColor(const ColorGeneric &color);

    const ColorGeneric &getBorderColor() const { return mBorderColor; }

    bool sameCompleteness(const SamplerState &samplerState) const
    {
        return mCompleteness.packed == samplerState.mCompleteness.packed;
    }

  private:
    void updateWrapTCompareMode();

    GLenum mMinFilter;
    GLenum mMagFilter;

    GLenum mWrapS;
    GLenum mWrapT;
    GLenum mWrapR;

    // From EXT_texture_filter_anisotropic
    float mMaxAnisotropy;

    GLfloat mMinLod;
    GLfloat mMaxLod;

    GLenum mCompareMode;
    GLenum mCompareFunc;

    GLenum mSRGBDecode;

    ColorGeneric mBorderColor;

    union Completeness
    {
        uint32_t packed;
        PackedSamplerCompleteness typed;
    };

    Completeness mCompleteness;
};

bool operator==(const SamplerState &a, const SamplerState &b);
bool operator!=(const SamplerState &a, const SamplerState &b);

struct DrawArraysIndirectCommand
{
    GLuint count;
    GLuint instanceCount;
    GLuint first;
    GLuint baseInstance;
};
static_assert(sizeof(DrawArraysIndirectCommand) == 16,
              "Unexpected size of DrawArraysIndirectCommand");

struct DrawElementsIndirectCommand
{
    GLuint count;
    GLuint primCount;
    GLuint firstIndex;
    GLint baseVertex;
    GLuint baseInstance;
};
static_assert(sizeof(DrawElementsIndirectCommand) == 20,
              "Unexpected size of DrawElementsIndirectCommand");

struct ImageUnit
{
    ImageUnit();
    ImageUnit(const ImageUnit &other);
    ~ImageUnit();

    BindingPointer<Texture> texture;
    GLint level;
    GLboolean layered;
    GLint layer;
    GLenum access;
    GLenum format;
};

using ImageUnitTextureTypeMap = std::map<unsigned int, gl::TextureType>;

struct PixelStoreStateBase
{
    GLint alignment   = 4;
    GLint rowLength   = 0;
    GLint skipRows    = 0;
    GLint skipPixels  = 0;
    GLint imageHeight = 0;
    GLint skipImages  = 0;
};

struct PixelUnpackState : PixelStoreStateBase
{};

struct PixelPackState : PixelStoreStateBase
{
    bool reverseRowOrder = false;
};

// Used in VertexArray.
using VertexArrayBufferBindingMask = angle::BitSet<MAX_VERTEX_ATTRIB_BINDINGS>;

// Used in Program and VertexArray.
using AttributesMask = angle::BitSet<MAX_VERTEX_ATTRIBS>;

// Used in Program
using ProgramUniformBlockMask = angle::BitSet<IMPLEMENTATION_MAX_COMBINED_SHADER_UNIFORM_BUFFERS>;
template <typename T>
using ProgramUniformBlockArray = std::array<T, IMPLEMENTATION_MAX_COMBINED_SHADER_UNIFORM_BUFFERS>;
template <typename T>
using UniformBufferBindingArray = std::array<T, IMPLEMENTATION_MAX_UNIFORM_BUFFER_BINDINGS>;

// Used in Framebuffer / Program
using DrawBufferMask = angle::BitSet8<IMPLEMENTATION_MAX_DRAW_BUFFERS>;

class BlendStateExt final
{
    static_assert(IMPLEMENTATION_MAX_DRAW_BUFFERS == 8, "Only up to 8 draw buffers supported.");

  public:
    template <typename ElementType, size_t ElementCount>
    struct StorageType final
    {
        static_assert(ElementCount <= 256, "ElementCount cannot exceed 256.");

#if defined(ANGLE_IS_64_BIT_CPU)
        // Always use uint64_t on 64-bit systems
        static constexpr size_t kBits = 8;
#else
        static constexpr size_t kBits = ElementCount > 16 ? 8 : 4;
#endif

        using Type = typename std::conditional<kBits == 8, uint64_t, uint32_t>::type;

        static constexpr Type kMaxValueMask = (kBits == 8) ? 0xFF : 0xF;

        static constexpr Type GetMask(const size_t drawBuffers)
        {
            ASSERT(drawBuffers > 0);
            ASSERT(drawBuffers <= IMPLEMENTATION_MAX_DRAW_BUFFERS);
            return static_cast<Type>(0xFFFFFFFFFFFFFFFFull >> (64 - drawBuffers * kBits));
        }

        // A multiplier that is used to replicate 4- or 8-bit value 8 times.
        static constexpr Type kReplicator = (kBits == 8) ? 0x0101010101010101ull : 0x11111111;

        // Extract packed `Bits`-bit value of index `index`. `values` variable contains up to 8
        // packed values.
        static constexpr ElementType GetValueIndexed(const size_t index, const Type values)
        {
            ASSERT(index < IMPLEMENTATION_MAX_DRAW_BUFFERS);

            return static_cast<ElementType>((values >> (index * kBits)) & kMaxValueMask);
        }

        // Replicate `Bits`-bit value 8 times and mask the result.
        static constexpr Type GetReplicatedValue(const ElementType value, const Type mask)
        {
            ASSERT(static_cast<size_t>(value) <= kMaxValueMask);
            return (static_cast<size_t>(value) * kReplicator) & mask;
        }

        // Replace `Bits`-bit value of index `index` in `target` with `value`.
        static constexpr void SetValueIndexed(const size_t index,
                                              const ElementType value,
                                              Type *target)
        {
            ASSERT(static_cast<size_t>(value) <= kMaxValueMask);
            ASSERT(index < IMPLEMENTATION_MAX_DRAW_BUFFERS);

            // Bitmask with set bits that contain the value of index `index`.
            const Type selector = kMaxValueMask << (index * kBits);

            // Shift the new `value` to its position in the packed value.
            const Type builtValue = static_cast<Type>(value) << (index * kBits);

            // Mark differing bits of `target` and `builtValue`, then flip the bits on those
            // positions in `target`.
            // Taken from https://graphics.stanford.edu/~seander/bithacks.html#MaskedMerge
            *target = *target ^ ((*target ^ builtValue) & selector);
        }

        // Compare two packed sets of eight 4-bit values and return an 8-bit diff mask.
        static constexpr DrawBufferMask GetDiffMask(const uint32_t packedValue1,
                                                    const uint32_t packedValue2)
        {
            uint32_t diff = packedValue1 ^ packedValue2;

            // For each 4-bit value that is different between inputs, set the msb to 1 and other
            // bits to 0.
            diff = (diff | ((diff & 0x77777777) + 0x77777777)) & 0x88888888;

            // By this point, `diff` looks like a...b...c...d...e...f...g...h... (dots mean zeros).
            // To get DrawBufferMask, we need to compress this 32-bit value to 8 bits, i.e. abcdefgh

            // Multiplying the lower half of `diff` by 0x249 (0x200 + 0x40 + 0x8 + 0x1) produces:
            // ................e...f...g...h... +
            // .............e...f...g...h...... +
            // ..........e...f...g...h......... +
            // .......e...f...g...h............
            // ________________________________ =
            // .......e..ef.efgefghfgh.gh..h...
            //                 ^^^^
            // Similar operation is applied to the upper word.
            // This calculation could be replaced with a single PEXT instruction from BMI2 set.
            diff = ((((diff & 0xFFFF0000) * 0x249) >> 24) & 0xF0) | (((diff * 0x249) >> 12) & 0xF);

            return DrawBufferMask(static_cast<uint8_t>(diff));
        }

        // Compare two packed sets of eight 8-bit values and return an 8-bit diff mask.
        static constexpr DrawBufferMask GetDiffMask(const uint64_t packedValue1,
                                                    const uint64_t packedValue2)
        {
            uint64_t diff = packedValue1 ^ packedValue2;

            // For each 8-bit value that is different between inputs, set the msb to 1 and other
            // bits to 0.
            diff = (diff | ((diff & 0x7F7F7F7F7F7F7F7F) + 0x7F7F7F7F7F7F7F7F)) & 0x8080808080808080;

            // By this point, `diff` looks like (dots mean zeros):
            // a.......b.......c.......d.......e.......f.......g.......h.......
            // To get DrawBufferMask, we need to compress this 64-bit value to 8 bits, i.e. abcdefgh

            // Multiplying `diff` by 0x0002040810204081 produces:
            // a.......b.......c.......d.......e.......f.......g.......h....... +
            // .b.......c.......d.......e.......f.......g.......h.............. +
            // ..c.......d.......e.......f.......g.......h..................... +
            // ...d.......e.......f.......g.......h............................ +
            // ....e.......f.......g.......h................................... +
            // .....f.......g.......h.......................................... +
            // ......g.......h................................................. +
            // .......h........................................................
            // ________________________________________________________________ =
            // abcdefghbcdefgh.cdefgh..defgh...efgh....fgh.....gh......h.......
            // ^^^^^^^^
            // This operation could be replaced with a single PEXT instruction from BMI2 set.
            diff = 0x0002040810204081 * diff >> 56;

            return DrawBufferMask(static_cast<uint8_t>(diff));
        }
    };

    using FactorStorage    = StorageType<BlendFactorType, angle::EnumSize<BlendFactorType>()>;
    using EquationStorage  = StorageType<BlendEquationType, angle::EnumSize<BlendEquationType>()>;
    using ColorMaskStorage = StorageType<uint8_t, 16>;
    static_assert(std::is_same<FactorStorage::Type, uint64_t>::value &&
                      std::is_same<EquationStorage::Type, uint64_t>::value,
                  "Factor and Equation storage must be 64-bit.");

    BlendStateExt(const size_t drawBuffers = 1);

    BlendStateExt(const BlendStateExt &other);
    BlendStateExt &operator=(const BlendStateExt &other);

    ///////// Blending Toggle /////////

    void setEnabled(const bool enabled);
    void setEnabledIndexed(const size_t index, const bool enabled);

    ///////// Color Write Mask /////////

    static constexpr size_t PackColorMask(const bool red,
                                          const bool green,
                                          const bool blue,
                                          const bool alpha)
    {
        return (red ? 1 : 0) | (green ? 2 : 0) | (blue ? 4 : 0) | (alpha ? 8 : 0);
    }

    static constexpr void UnpackColorMask(const size_t value,
                                          bool *red,
                                          bool *green,
                                          bool *blue,
                                          bool *alpha)
    {
        *red   = static_cast<bool>(value & 1);
        *green = static_cast<bool>(value & 2);
        *blue  = static_cast<bool>(value & 4);
        *alpha = static_cast<bool>(value & 8);
    }

    ColorMaskStorage::Type expandColorMaskValue(const bool red,
                                                const bool green,
                                                const bool blue,
                                                const bool alpha) const;
    ColorMaskStorage::Type expandColorMaskIndexed(const size_t index) const;
    void setColorMask(const bool red, const bool green, const bool blue, const bool alpha);
    void setColorMaskIndexed(const size_t index, const uint8_t value);
    void setColorMaskIndexed(const size_t index,
                             const bool red,
                             const bool green,
                             const bool blue,
                             const bool alpha);
    uint8_t getColorMaskIndexed(const size_t index) const;
    void getColorMaskIndexed(const size_t index,
                             bool *red,
                             bool *green,
                             bool *blue,
                             bool *alpha) const;
    DrawBufferMask compareColorMask(ColorMaskStorage::Type other) const;

    ///////// Blend Equation /////////

    EquationStorage::Type expandEquationValue(const GLenum mode) const;
    EquationStorage::Type expandEquationValue(const gl::BlendEquationType equation) const;
    EquationStorage::Type expandEquationColorIndexed(const size_t index) const;
    EquationStorage::Type expandEquationAlphaIndexed(const size_t index) const;
    void setEquations(const GLenum modeColor, const GLenum modeAlpha);
    void setEquationsIndexed(const size_t index, const GLenum modeColor, const GLenum modeAlpha);
    void setEquationsIndexed(const size_t index,
                             const size_t otherIndex,
                             const BlendStateExt &other);
    BlendEquationType getEquationColorIndexed(size_t index) const
    {
        ASSERT(index < mDrawBufferCount);
        return EquationStorage::GetValueIndexed(index, mEquationColor);
    }
    BlendEquationType getEquationAlphaIndexed(size_t index) const
    {
        ASSERT(index < mDrawBufferCount);
        return EquationStorage::GetValueIndexed(index, mEquationAlpha);
    }
    DrawBufferMask compareEquations(const EquationStorage::Type color,
                                    const EquationStorage::Type alpha) const;
    DrawBufferMask compareEquations(const BlendStateExt &other) const
    {
        return compareEquations(other.mEquationColor, other.mEquationAlpha);
    }

    ///////// Blend Factors /////////

    FactorStorage::Type expandFactorValue(const GLenum func) const;
    FactorStorage::Type expandFactorValue(const gl::BlendFactorType func) const;
    FactorStorage::Type expandSrcColorIndexed(const size_t index) const;
    FactorStorage::Type expandDstColorIndexed(const size_t index) const;
    FactorStorage::Type expandSrcAlphaIndexed(const size_t index) const;
    FactorStorage::Type expandDstAlphaIndexed(const size_t index) const;
    void setFactors(const GLenum srcColor,
                    const GLenum dstColor,
                    const GLenum srcAlpha,
                    const GLenum dstAlpha);
    void setFactorsIndexed(const size_t index,
                           const gl::BlendFactorType srcColorFactor,
                           const gl::BlendFactorType dstColorFactor,
                           const gl::BlendFactorType srcAlphaFactor,
                           const gl::BlendFactorType dstAlphaFactor);
    void setFactorsIndexed(const size_t index,
                           const GLenum srcColor,
                           const GLenum dstColor,
                           const GLenum srcAlpha,
                           const GLenum dstAlpha);
    void setFactorsIndexed(const size_t index, const size_t otherIndex, const BlendStateExt &other);
    BlendFactorType getSrcColorIndexed(size_t index) const
    {
        ASSERT(index < mDrawBufferCount);
        return FactorStorage::GetValueIndexed(index, mSrcColor);
    }
    BlendFactorType getDstColorIndexed(size_t index) const
    {
        ASSERT(index < mDrawBufferCount);
        return FactorStorage::GetValueIndexed(index, mDstColor);
    }
    BlendFactorType getSrcAlphaIndexed(size_t index) const
    {
        ASSERT(index < mDrawBufferCount);
        return FactorStorage::GetValueIndexed(index, mSrcAlpha);
    }
    BlendFactorType getDstAlphaIndexed(size_t index) const
    {
        ASSERT(index < mDrawBufferCount);
        return FactorStorage::GetValueIndexed(index, mDstAlpha);
    }
    DrawBufferMask compareFactors(const FactorStorage::Type srcColor,
                                  const FactorStorage::Type dstColor,
                                  const FactorStorage::Type srcAlpha,
                                  const FactorStorage::Type dstAlpha) const;
    DrawBufferMask compareFactors(const BlendStateExt &other) const
    {
        return compareFactors(other.mSrcColor, other.mDstColor, other.mSrcAlpha, other.mDstAlpha);
    }

    constexpr FactorStorage::Type getSrcColorBits() const { return mSrcColor; }
    constexpr FactorStorage::Type getSrcAlphaBits() const { return mSrcAlpha; }
    constexpr FactorStorage::Type getDstColorBits() const { return mDstColor; }
    constexpr FactorStorage::Type getDstAlphaBits() const { return mDstAlpha; }

    constexpr EquationStorage::Type getEquationColorBits() const { return mEquationColor; }
    constexpr EquationStorage::Type getEquationAlphaBits() const { return mEquationAlpha; }

    constexpr ColorMaskStorage::Type getAllColorMaskBits() const { return mAllColorMask; }
    constexpr ColorMaskStorage::Type getColorMaskBits() const { return mColorMask; }

    constexpr DrawBufferMask getAllEnabledMask() const { return mAllEnabledMask; }
    constexpr DrawBufferMask getEnabledMask() const { return mEnabledMask; }

    constexpr DrawBufferMask getUsesAdvancedBlendEquationMask() const
    {
        return mUsesAdvancedBlendEquationMask;
    }

    constexpr DrawBufferMask getUsesExtendedBlendFactorMask() const
    {
        return mUsesExtendedBlendFactorMask;
    }

    constexpr uint8_t getDrawBufferCount() const { return mDrawBufferCount; }

    constexpr void setSrcColorBits(const FactorStorage::Type srcColor) { mSrcColor = srcColor; }
    constexpr void setSrcAlphaBits(const FactorStorage::Type srcAlpha) { mSrcAlpha = srcAlpha; }
    constexpr void setDstColorBits(const FactorStorage::Type dstColor) { mDstColor = dstColor; }
    constexpr void setDstAlphaBits(const FactorStorage::Type dstAlpha) { mDstAlpha = dstAlpha; }

    constexpr void setEquationColorBits(const EquationStorage::Type equationColor)
    {
        mEquationColor = equationColor;
    }
    constexpr void setEquationAlphaBits(const EquationStorage::Type equationAlpha)
    {
        mEquationAlpha = equationAlpha;
    }

    constexpr void setColorMaskBits(const ColorMaskStorage::Type colorMask)
    {
        mColorMask = colorMask;
    }

    constexpr void setEnabledMask(const DrawBufferMask enabledMask) { mEnabledMask = enabledMask; }

    ///////// Data Members /////////
  private:
    uint64_t mParameterMask;

    FactorStorage::Type mSrcColor;
    FactorStorage::Type mDstColor;
    FactorStorage::Type mSrcAlpha;
    FactorStorage::Type mDstAlpha;

    EquationStorage::Type mEquationColor;
    EquationStorage::Type mEquationAlpha;

    ColorMaskStorage::Type mAllColorMask;
    ColorMaskStorage::Type mColorMask;

    DrawBufferMask mAllEnabledMask;
    DrawBufferMask mEnabledMask;

    // Cache of whether the blend equation for each index is from KHR_blend_equation_advanced.
    DrawBufferMask mUsesAdvancedBlendEquationMask;

    // Cache of whether the blend factor for each index is from EXT_blend_func_extended.
    DrawBufferMask mUsesExtendedBlendFactorMask;

    uint8_t mDrawBufferCount;

    ANGLE_MAYBE_UNUSED_PRIVATE_FIELD uint8_t kUnused[3] = {};
};

static_assert(sizeof(BlendStateExt) == sizeof(uint64_t) +
                                           (sizeof(BlendStateExt::FactorStorage::Type) * 4 +
                                            sizeof(BlendStateExt::EquationStorage::Type) * 2 +
                                            sizeof(BlendStateExt::ColorMaskStorage::Type) * 2 +
                                            sizeof(DrawBufferMask) * 4 + sizeof(uint8_t)) +
                                           sizeof(uint8_t) * 3,
              "The BlendStateExt class must not contain gaps.");

// Used in StateCache
using StorageBuffersMask = angle::BitSet<IMPLEMENTATION_MAX_SHADER_STORAGE_BUFFER_BINDINGS>;

template <typename T>
using SampleMaskArray = std::array<T, IMPLEMENTATION_MAX_SAMPLE_MASK_WORDS>;

template <typename T>
using TexLevelArray = std::array<T, IMPLEMENTATION_MAX_TEXTURE_LEVELS>;

using TexLevelMask = angle::BitSet<IMPLEMENTATION_MAX_TEXTURE_LEVELS>;

enum class ComponentType
{
    Float       = 0,
    Int         = 1,
    UnsignedInt = 2,
    NoType      = 3,
    EnumCount   = 4,
    InvalidEnum = 4,
};

constexpr ComponentType GLenumToComponentType(GLenum componentType)
{
    switch (componentType)
    {
        case GL_FLOAT:
            return ComponentType::Float;
        case GL_INT:
            return ComponentType::Int;
        case GL_UNSIGNED_INT:
            return ComponentType::UnsignedInt;
        case GL_NONE:
            return ComponentType::NoType;
        default:
            return ComponentType::InvalidEnum;
    }
}

constexpr angle::PackedEnumMap<ComponentType, uint32_t> kComponentMasks = {{
    {ComponentType::Float, 0x10001},
    {ComponentType::Int, 0x00001},
    {ComponentType::UnsignedInt, 0x10000},
}};

constexpr size_t kMaxComponentTypeMaskIndex = 16;
using ComponentTypeMask                     = angle::BitSet<kMaxComponentTypeMaskIndex * 2>;

ANGLE_INLINE void SetComponentTypeMask(ComponentType type, size_t index, ComponentTypeMask *mask)
{
    ASSERT(index <= kMaxComponentTypeMaskIndex);
    *mask &= ~(0x10001 << index);
    *mask |= kComponentMasks[type] << index;
}

ANGLE_INLINE ComponentType GetComponentTypeMask(ComponentTypeMask mask, size_t index)
{
    ASSERT(index <= kMaxComponentTypeMaskIndex);
    uint32_t mask_bits = mask.bits() >> index & 0x10001;
    switch (mask_bits)
    {
        case 0x10001:
            return ComponentType::Float;
        case 0x00001:
            return ComponentType::Int;
        case 0x10000:
            return ComponentType::UnsignedInt;
        default:
            return ComponentType::InvalidEnum;
    }
}

ANGLE_INLINE ComponentTypeMask GetActiveComponentTypeMask(gl::AttributesMask activeAttribLocations)
{
    const uint32_t activeAttribs = static_cast<uint32_t>(activeAttribLocations.bits());

    // Ever attrib index takes one bit from the lower 16-bits and another bit from the upper
    // 16-bits at the same index.
    return ComponentTypeMask(activeAttribs << kMaxComponentTypeMaskIndex | activeAttribs);
}

ANGLE_INLINE DrawBufferMask GetComponentTypeMaskDiff(ComponentTypeMask mask1,
                                                     ComponentTypeMask mask2)
{
    const uint32_t diff = static_cast<uint32_t>((mask1 ^ mask2).bits());
    return DrawBufferMask(static_cast<uint8_t>(diff | (diff >> gl::kMaxComponentTypeMaskIndex)));
}

bool ValidateComponentTypeMasks(unsigned long outputTypes,
                                unsigned long inputTypes,
                                unsigned long outputMask,
                                unsigned long inputMask);

// Helpers for performing WebGL 2.0 clear validation
// Extracted component type has always one of these four values:
// * 0x10001 - float or normalized
// * 0x00001 - int
// * 0x10000 - unsigned int
// * 0x00000 - unused or disabled

// The following functions rely on these.
static_assert(kComponentMasks[ComponentType::Float] == 0x10001);
static_assert(kComponentMasks[ComponentType::Int] == 0x00001);
static_assert(kComponentMasks[ComponentType::UnsignedInt] == 0x10000);

// Used for clearBufferuiv
ANGLE_INLINE bool IsComponentTypeFloatOrInt(ComponentTypeMask mask, size_t index)
{
    ASSERT(index <= kMaxComponentTypeMaskIndex);
    // 0x10001 or 0x00001
    return ((mask.bits() >> index) & 0x00001) != 0;
}

// Used for clearBufferiv
ANGLE_INLINE bool IsComponentTypeFloatOrUnsignedInt(ComponentTypeMask mask, size_t index)
{
    ASSERT(index <= kMaxComponentTypeMaskIndex);
    // 0x10001 or 0x10000
    return ((mask.bits() >> index) & 0x10000) != 0;
}

// Used for clearBufferfv
ANGLE_INLINE bool IsComponentTypeIntOrUnsignedInt(ComponentTypeMask mask, size_t index)
{
    ASSERT(index <= kMaxComponentTypeMaskIndex);
    // 0x00001 or 0x10000; this expression is more efficient than two explicit comparisons
    return ((((mask.bits() >> kMaxComponentTypeMaskIndex) ^ mask.bits()) >> index) & 1) != 0;
}

// Used for clear
ANGLE_INLINE DrawBufferMask GetIntOrUnsignedIntDrawBufferMask(ComponentTypeMask mask)
{
    static_assert(DrawBufferMask::size() <= 8);
    return DrawBufferMask(
        static_cast<uint8_t>((mask.bits() >> kMaxComponentTypeMaskIndex) ^ mask.bits()));
}

// GL_ANGLE_blob_cache state
struct BlobCacheCallbacks
{
    GLSETBLOBPROCANGLE setFunction = nullptr;
    GLGETBLOBPROCANGLE getFunction = nullptr;
    const void *userParam          = nullptr;
};

enum class RenderToTextureImageIndex
{
    // The default image of the texture, where data is expected to be.
    Default = 0,

    // Intermediate multisampled images for EXT_multisampled_render_to_texture.
    // These values must match log2(SampleCount).
    IntermediateImage2xMultisampled  = 1,
    IntermediateImage4xMultisampled  = 2,
    IntermediateImage8xMultisampled  = 3,
    IntermediateImage16xMultisampled = 4,

    // We currently only support up to 16xMSAA in backends that use this enum.
    InvalidEnum = 5,
    EnumCount   = 5,
};

template <typename T>
using RenderToTextureImageMap = angle::PackedEnumMap<RenderToTextureImageIndex, T>;

constexpr size_t kCubeFaceCount = 6;

template <typename T>
using CubeFaceArray = std::array<T, kCubeFaceCount>;

template <typename T>
using TextureTypeMap = angle::PackedEnumMap<TextureType, T>;
using TextureMap     = TextureTypeMap<BindingPointer<Texture>>;

// ShaderVector can contain one item per shader.  It differs from ShaderMap in that the values are
// not indexed by ShaderType.
template <typename T>
using ShaderVector = angle::FixedVector<T, static_cast<size_t>(ShaderType::EnumCount)>;

template <typename T>
using AttachmentArray = std::array<T, IMPLEMENTATION_MAX_FRAMEBUFFER_ATTACHMENTS>;

template <typename T>
using AttachmentVector = angle::FixedVector<T, IMPLEMENTATION_MAX_FRAMEBUFFER_ATTACHMENTS>;

using AttachmentsMask = angle::BitSet<IMPLEMENTATION_MAX_FRAMEBUFFER_ATTACHMENTS>;

template <typename T>
using DrawBuffersArray = std::array<T, IMPLEMENTATION_MAX_DRAW_BUFFERS>;

template <typename T>
using DrawBuffersVector = angle::FixedVector<T, IMPLEMENTATION_MAX_DRAW_BUFFERS>;

template <typename T>
using AttribArray = std::array<T, MAX_VERTEX_ATTRIBS>;

template <typename T>
using AttribVector = angle::FixedVector<T, MAX_VERTEX_ATTRIBS>;

using ActiveTextureMask = angle::BitSet<IMPLEMENTATION_MAX_ACTIVE_TEXTURES>;

template <typename T>
using ActiveTextureArray = std::array<T, IMPLEMENTATION_MAX_ACTIVE_TEXTURES>;

using ActiveTextureTypeArray = ActiveTextureArray<TextureType>;

using ImageUnitMask = angle::BitSet<IMPLEMENTATION_MAX_IMAGE_UNITS>;

using SupportedSampleSet = std::set<GLuint>;

template <typename T>
using TransformFeedbackBuffersArray =
    std::array<T, gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS>;

using ClipDistanceEnableBits = angle::BitSet32<IMPLEMENTATION_MAX_CLIP_DISTANCES>;

template <typename T>
using QueryTypeMap = angle::PackedEnumMap<QueryType, T>;

constexpr size_t kBarrierVectorDefaultSize = 16;

template <typename T>
using BarrierVector = angle::FastVector<T, kBarrierVectorDefaultSize>;

using BufferBarrierVector = BarrierVector<Buffer *>;

using SamplerBindingVector = std::vector<BindingPointer<Sampler>>;
using BufferVector         = std::vector<OffsetBindingPointer<Buffer>>;

struct TextureAndLayout
{
    Texture *texture;
    GLenum layout;
};
using TextureBarrierVector = BarrierVector<TextureAndLayout>;

// OffsetBindingPointer.getSize() returns the size specified by the user, which may be larger than
// the size of the bound buffer. This function reduces the returned size to fit the bound buffer if
// necessary. Returns 0 if no buffer is bound or if integer overflow occurs.
GLsizeiptr GetBoundBufferAvailableSize(const OffsetBindingPointer<Buffer> &binding);

// A texture level index.
template <typename T>
class LevelIndexWrapper
{
  public:
    LevelIndexWrapper() = default;
    explicit constexpr LevelIndexWrapper(T levelIndex) : mLevelIndex(levelIndex) {}
    constexpr LevelIndexWrapper(const LevelIndexWrapper &other)            = default;
    constexpr LevelIndexWrapper &operator=(const LevelIndexWrapper &other) = default;

    constexpr T get() const { return mLevelIndex; }

    LevelIndexWrapper &operator++()
    {
        ++mLevelIndex;
        return *this;
    }
    constexpr bool operator<(const LevelIndexWrapper &other) const
    {
        return mLevelIndex < other.mLevelIndex;
    }
    constexpr bool operator<=(const LevelIndexWrapper &other) const
    {
        return mLevelIndex <= other.mLevelIndex;
    }
    constexpr bool operator>(const LevelIndexWrapper &other) const
    {
        return mLevelIndex > other.mLevelIndex;
    }
    constexpr bool operator>=(const LevelIndexWrapper &other) const
    {
        return mLevelIndex >= other.mLevelIndex;
    }
    constexpr bool operator==(const LevelIndexWrapper &other) const
    {
        return mLevelIndex == other.mLevelIndex;
    }
    constexpr bool operator!=(const LevelIndexWrapper &other) const
    {
        return mLevelIndex != other.mLevelIndex;
    }
    constexpr LevelIndexWrapper operator+(T other) const
    {
        return LevelIndexWrapper(mLevelIndex + other);
    }
    constexpr LevelIndexWrapper operator-(T other) const
    {
        return LevelIndexWrapper(mLevelIndex - other);
    }
    constexpr T operator-(LevelIndexWrapper other) const { return mLevelIndex - other.mLevelIndex; }

  private:
    T mLevelIndex;
};

// A GL texture level index.
using LevelIndex = LevelIndexWrapper<GLint>;

enum class MultisamplingMode
{
    // Regular multisampling
    Regular = 0,
    // GL_EXT_multisampled_render_to_texture renderbuffer/texture attachments which perform implicit
    // resolve of multisampled data.
    MultisampledRenderToTexture,
};
}  // namespace gl

namespace rx
{
// A macro that determines whether an object has a given runtime type.
#if defined(__clang__)
#    if __has_feature(cxx_rtti)
#        define ANGLE_HAS_DYNAMIC_CAST 1
#    endif
#elif !defined(NDEBUG) && (!defined(_MSC_VER) || defined(_CPPRTTI)) &&              \
    (!defined(__GNUC__) || __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 3) || \
     defined(__GXX_RTTI))
#    define ANGLE_HAS_DYNAMIC_CAST 1
#endif

#ifdef ANGLE_HAS_DYNAMIC_CAST
#    define ANGLE_HAS_DYNAMIC_TYPE(type, obj) (dynamic_cast<type>(obj) != nullptr)
#    undef ANGLE_HAS_DYNAMIC_CAST
#else
#    define ANGLE_HAS_DYNAMIC_TYPE(type, obj) (obj != nullptr)
#endif

// Downcast a base implementation object (EG TextureImpl to TextureD3D)
template <typename DestT, typename SrcT>
inline DestT *GetAs(SrcT *src)
{
    ASSERT(ANGLE_HAS_DYNAMIC_TYPE(DestT *, src));
    return static_cast<DestT *>(src);
}

template <typename DestT, typename SrcT>
inline const DestT *GetAs(const SrcT *src)
{
    ASSERT(ANGLE_HAS_DYNAMIC_TYPE(const DestT *, src));
    return static_cast<const DestT *>(src);
}

#undef ANGLE_HAS_DYNAMIC_TYPE

// Downcast a GL object to an Impl (EG gl::Texture to rx::TextureD3D)
template <typename DestT, typename SrcT>
inline DestT *GetImplAs(SrcT *src)
{
    return GetAs<DestT>(src->getImplementation());
}

template <typename DestT, typename SrcT>
inline DestT *SafeGetImplAs(SrcT *src)
{
    return src != nullptr ? GetAs<DestT>(src->getImplementation()) : nullptr;
}

}  // namespace rx

#include "angletypes.inc"

namespace angle
{
enum class NativeWindowSystem
{
    X11,
    Wayland,
    Gbm,
    Other,
};

struct FeatureOverrides
{
    std::vector<std::string> enabled;
    std::vector<std::string> disabled;
    bool allDisabled = false;
};

// 160-bit SHA-1 hash key used for hasing a program.  BlobCache opts in using fixed keys for
// simplicity and efficiency.
static constexpr size_t kBlobCacheKeyLength = angle::base::kSHA1Length;
using BlobCacheKey                          = std::array<uint8_t, kBlobCacheKeyLength>;
class BlobCacheValue  // To be replaced with std::span when C++20 is required
{
  public:
    BlobCacheValue() : mPtr(nullptr), mSize(0) {}
    BlobCacheValue(const uint8_t *ptr, size_t size) : mPtr(ptr), mSize(size) {}

    // A very basic struct to hold the pointer and size together.  The objects of this class
    // don't own the memory.
    const uint8_t *data() { return mPtr; }
    size_t size() { return mSize; }

    const uint8_t &operator[](size_t pos) const
    {
        ASSERT(pos < mSize);
        return mPtr[pos];
    }

  private:
    const uint8_t *mPtr;
    size_t mSize;
};

bool CompressBlob(const size_t cacheSize, const uint8_t *cacheData, MemoryBuffer *compressedData);
bool DecompressBlob(const uint8_t *compressedData,
                    const size_t compressedSize,
                    size_t maxUncompressedDataSize,
                    MemoryBuffer *uncompressedData);
uint32_t GenerateCRC32(const uint8_t *data, size_t size);
uint32_t InitCRC32();
uint32_t UpdateCRC32(uint32_t prevCrc32, const uint8_t *data, size_t size);
}  // namespace angle

namespace std
{
template <>
struct hash<angle::BlobCacheKey>
{
    // Simple routine to hash four ints.
    size_t operator()(const angle::BlobCacheKey &key) const
    {
        return angle::ComputeGenericHash(key.data(), key.size());
    }
};
}  // namespace std

namespace angle
{
// Under certain circumstances, such as for increased parallelism, the backend may defer an
// operation to be done at the end of a call after the locks have been unlocked.  The entry point
// function passes an |UnlockedTailCall| through the frontend to the backend.  If it is set, the
// entry point would execute it at the end of the call.
//
// Since the function is called without any locks, care must be taken to minimize the amount of work
// in such calls and ensure thread safety (for example by using fine grained locks inside the call
// itself).
//
// Some entry points pass a void pointer argument to UnlockedTailCall::run method intended to
// contain the return value filled by the backend, the rest of the entry points pass in a
// nullptr.  Regardless, Display::terminate runs pending tail calls passing in a nullptr, so
// the tail calls that return a value in the argument still have to guard against a nullptr
// parameter.
class UnlockedTailCall final : angle::NonCopyable
{
  public:
    using CallType = std::function<void(void *)>;

    UnlockedTailCall();
    ~UnlockedTailCall();

    void add(CallType &&call);
    ANGLE_INLINE void run(void *resultOut)
    {
        if (!mCalls.empty())
        {
            runImpl(resultOut);
        }
    }

    bool any() const { return !mCalls.empty(); }

  private:
    void runImpl(void *resultOut);

    // Typically, there is only one tail call.  It is possible to end up with 2 tail calls currently
    // with unMakeCurrent destroying both the read and draw surfaces, each adding a tail call in the
    // Vulkan backend.
    //
    // Some apps will create multiple windows surfaces and not call corresponding destroy api, which
    // cause many tail calls been added, so remove the max call count limitations.
    std::vector<CallType> mCalls;
};

enum class JobThreadSafety
{
    Safe,
    Unsafe,
};

enum class JobResultExpectancy
{
    // Whether the compile or link job's results are immediately needed.  This is the case for GLES1
    // programs for example, or shader compilation in glCreateShaderProgramv.
    Immediate,
    // Whether the compile or link job's results are needed after the end of the current entry point
    // call.  In this case, the job may be done in an unlocked tail call.
    Future,
};

// Zero-based for better array indexing
enum FramebufferBinding
{
    FramebufferBindingRead = 0,
    FramebufferBindingDraw,
    FramebufferBindingSingletonMax,
    FramebufferBindingBoth = FramebufferBindingSingletonMax,
    FramebufferBindingMax,
    FramebufferBindingUnknown = FramebufferBindingMax,
};

inline FramebufferBinding EnumToFramebufferBinding(GLenum enumValue)
{
    switch (enumValue)
    {
        case GL_READ_FRAMEBUFFER:
            return FramebufferBindingRead;
        case GL_DRAW_FRAMEBUFFER:
            return FramebufferBindingDraw;
        case GL_FRAMEBUFFER:
            return FramebufferBindingBoth;
        default:
            UNREACHABLE();
            return FramebufferBindingUnknown;
    }
}

inline GLenum FramebufferBindingToEnum(FramebufferBinding binding)
{
    switch (binding)
    {
        case FramebufferBindingRead:
            return GL_READ_FRAMEBUFFER;
        case FramebufferBindingDraw:
            return GL_DRAW_FRAMEBUFFER;
        case FramebufferBindingBoth:
            return GL_FRAMEBUFFER;
        default:
            UNREACHABLE();
            return GL_NONE;
    }
}

template <typename ObjT, typename ContextT>
class DestroyThenDelete
{
  public:
    DestroyThenDelete() = default;
    DestroyThenDelete(const ContextT *context) : mContext(context) {}

    void operator()(ObjT *obj)
    {
        (void)(obj->onDestroy(mContext));
        delete obj;
    }

  private:
    const ContextT *mContext = nullptr;
};

template <typename ObjT, typename ContextT>
using UniqueObjectPointer = std::unique_ptr<ObjT, DestroyThenDelete<ObjT, ContextT>>;

}  // namespace angle

namespace gl
{
class State;

// Focal Point information for foveated rendering
struct FocalPoint
{
    float focalX;
    float focalY;
    float gainX;
    float gainY;
    float foveaArea;

    constexpr FocalPoint() : focalX(0), focalY(0), gainX(0), gainY(0), foveaArea(0) {}

    FocalPoint(float fX, float fY, float gX, float gY, float fArea)
        : focalX(fX), focalY(fY), gainX(gX), gainY(gY), foveaArea(fArea)
    {}
    FocalPoint(const FocalPoint &other)            = default;
    FocalPoint &operator=(const FocalPoint &other) = default;

    bool operator==(const FocalPoint &other) const
    {
        return focalX == other.focalX && focalY == other.focalY && gainX == other.gainX &&
               gainY == other.gainY && foveaArea == other.foveaArea;
    }
    bool operator!=(const FocalPoint &other) const { return !(*this == other); }

    bool valid() const { return gainX > 0 && gainY > 0; }
};

constexpr FocalPoint kDefaultFocalPoint = FocalPoint();

class FoveationState
{
  public:
    FoveationState()
    {
        mConfigured          = false;
        mFoveatedFeatureBits = 0;
        mMinPixelDensity     = 0.0f;
        mFocalPoints.fill(kDefaultFocalPoint);
    }
    FoveationState &operator=(const FoveationState &other) = default;

    void configure() { mConfigured = true; }
    bool isConfigured() const { return mConfigured; }
    bool isFoveated() const
    {
        // Consider foveated if at least 1 focal point is valid
        return std::any_of(mFocalPoints.begin(), mFocalPoints.end(),
                           [](const FocalPoint &focalPoint) { return focalPoint.valid(); });
    }
    bool operator==(const FoveationState &other) const
    {
        return mConfigured == other.mConfigured &&
               mFoveatedFeatureBits == other.mFoveatedFeatureBits &&
               mMinPixelDensity == other.mMinPixelDensity && mFocalPoints == other.mFocalPoints;
    }
    bool operator!=(const FoveationState &other) const { return !(*this == other); }

    void setFoveatedFeatureBits(const GLuint features) { mFoveatedFeatureBits = features; }
    GLuint getFoveatedFeatureBits() const { return mFoveatedFeatureBits; }
    void setMinPixelDensity(const GLfloat density) { mMinPixelDensity = density; }
    GLfloat getMinPixelDensity() const { return mMinPixelDensity; }
    GLuint getMaxNumFocalPoints() const { return gl::IMPLEMENTATION_MAX_FOCAL_POINTS; }
    void setFocalPoint(uint32_t layer, uint32_t focalPointIndex, const FocalPoint &focalPoint)
    {
        mFocalPoints[getIndex(layer, focalPointIndex)] = focalPoint;
    }
    const FocalPoint &getFocalPoint(uint32_t layer, uint32_t focalPointIndex) const
    {
        return mFocalPoints[getIndex(layer, focalPointIndex)];
    }
    GLuint getSupportedFoveationFeatures() const { return GL_FOVEATION_ENABLE_BIT_QCOM; }

  private:
    size_t getIndex(uint32_t layer, uint32_t focalPointIndex) const
    {
        ASSERT(layer < IMPLEMENTATION_MAX_NUM_LAYERS &&
               focalPointIndex < IMPLEMENTATION_MAX_FOCAL_POINTS);
        return (layer * IMPLEMENTATION_MAX_FOCAL_POINTS) + focalPointIndex;
    }
    bool mConfigured;
    GLuint mFoveatedFeatureBits;
    GLfloat mMinPixelDensity;

    static constexpr size_t kMaxFocalPoints =
        IMPLEMENTATION_MAX_NUM_LAYERS * IMPLEMENTATION_MAX_FOCAL_POINTS;
    std::array<FocalPoint, kMaxFocalPoints> mFocalPoints;
};

}  // namespace gl

#endif  // LIBANGLE_ANGLETYPES_H_
