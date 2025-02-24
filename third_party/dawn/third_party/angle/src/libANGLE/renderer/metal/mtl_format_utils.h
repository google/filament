//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// mtl_format_utils.h:
//      Declares Format conversion utilities classes that convert from angle formats
//      to respective MTLPixelFormat and MTLVertexFormat.
//

#ifndef LIBANGLE_RENDERER_METAL_MTL_FORMAT_UTILS_H_
#define LIBANGLE_RENDERER_METAL_MTL_FORMAT_UTILS_H_

#import <Metal/Metal.h>

#include <unordered_map>

#include "common/angleutils.h"
#include "libANGLE/Caps.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/copyvertex.h"
#include "libANGLE/renderer/renderer_utils.h"

namespace rx
{
class DisplayMtl;

namespace mtl
{
class ContextDevice;

struct FormatBase
{
    inline bool operator==(const FormatBase &rhs) const
    {
        return intendedFormatId == rhs.intendedFormatId && actualFormatId == rhs.actualFormatId;
    }

    inline bool operator!=(const FormatBase &rhs) const { return !((*this) == rhs); }

    const angle::Format &actualAngleFormat() const;
    const angle::Format &intendedAngleFormat() const;

    angle::FormatID actualFormatId   = angle::FormatID::NONE;
    angle::FormatID intendedFormatId = angle::FormatID::NONE;
};

struct FormatCaps
{
    bool isRenderable() const { return colorRenderable || depthRenderable; }

    bool filterable           = false;
    bool writable             = false;
    bool colorRenderable      = false;
    bool depthRenderable      = false;
    bool blendable            = false;
    bool multisample          = false;  // can be used as MSAA target
    bool resolve              = false;  // Can be used as resolve target
    bool compressed           = false;
    NSUInteger pixelBytes     = 0;
    NSUInteger pixelBytesMSAA = 0;
    NSUInteger channels       = 0;
    uint8_t alignment         = 0;
};

// Pixel format
struct Format : public FormatBase
{
    Format() = default;

    static angle::FormatID MetalToAngleFormatID(MTLPixelFormat formatMtl);

    const gl::InternalFormat &intendedInternalFormat() const;
    const gl::InternalFormat &actualInternalFormat() const;

    bool valid() const { return metalFormat != MTLPixelFormatInvalid; }
    bool hasDepthAndStencilBits() const
    {
        return actualAngleFormat().depthBits && actualAngleFormat().stencilBits;
    }
    bool hasDepthOrStencilBits() const
    {
        return actualAngleFormat().depthBits || actualAngleFormat().stencilBits;
    }
    bool isPVRTC() const;

    const FormatCaps &getCaps() const { return caps; }

    // Need conversion between source format and this format?
    bool needConversion(angle::FormatID srcFormatId) const;

    MTLPixelFormat metalFormat = MTLPixelFormatInvalid;

    LoadFunctionMap textureLoadFunctions       = nullptr;
    InitializeTextureDataFunction initFunction = nullptr;

    FormatCaps caps;

    bool swizzled = false;
    std::array<GLenum, 4> swizzle;

  private:
    void init(const DisplayMtl *display, angle::FormatID intendedFormatId);

    friend class FormatTable;
};

// Vertex format
struct VertexFormat : public FormatBase
{
    VertexFormat() = default;

    MTLVertexFormat metalFormat = MTLVertexFormatInvalid;

    VertexCopyFunction vertexLoadFunction = nullptr;

    uint32_t defaultAlpha = 0;
    // Intended and actual format have same GL type, and possibly only differ in number of
    // components?
    bool actualSameGLType = true;

  private:
    void init(angle::FormatID angleFormatId, bool tightlyPacked = false);

    friend class FormatTable;
};

class FormatTable final : angle::NonCopyable
{
  public:
    FormatTable()  = default;
    ~FormatTable() = default;

    angle::Result initialize(const DisplayMtl *display);

    void generateTextureCaps(const DisplayMtl *display, gl::TextureCapsMap *capsMapOut);

    const Format &getPixelFormat(angle::FormatID angleFormatId) const;
    const FormatCaps &getNativeFormatCaps(MTLPixelFormat mtlFormat) const;

    // tightlyPacked means this format will be used in a tightly packed vertex buffer.
    // In that case, it's easier to just convert everything to float to ensure
    // Metal alignment requirements between 2 elements inside the buffer will be met regardless
    // of how many components each element has.
    const VertexFormat &getVertexFormat(angle::FormatID angleFormatId, bool tightlyPacked) const;

    uint32_t getMaxSamples() const { return mMaxSamples; }

  private:
    void initNativeFormatCapsAutogen(const DisplayMtl *display);
    void initNativeFormatCaps(const DisplayMtl *display);

    void setFormatCaps(MTLPixelFormat formatId,
                       bool filterable,
                       bool writable,
                       bool blendable,
                       bool multisample,
                       bool resolve,
                       bool colorRenderable);

    void setFormatCaps(MTLPixelFormat formatId,
                       bool filterable,
                       bool writable,
                       bool blendable,
                       bool multisample,
                       bool resolve,
                       bool colorRenderable,
                       NSUInteger bytesPerChannel,
                       NSUInteger channels);

    void setFormatCaps(MTLPixelFormat formatId,
                       bool filterable,
                       bool writable,
                       bool blendable,
                       bool multisample,
                       bool resolve,
                       bool colorRenderable,
                       bool depthRenderable);

    void setFormatCaps(MTLPixelFormat formatId,
                       bool filterable,
                       bool writable,
                       bool blendable,
                       bool multisample,
                       bool resolve,
                       bool colorRenderable,
                       bool depthRenderable,
                       NSUInteger bytesPerChannel,
                       NSUInteger channels);

    void setCompressedFormatCaps(MTLPixelFormat formatId, bool filterable);

    void adjustFormatCapsForDevice(const mtl::ContextDevice &device,
                                   MTLPixelFormat id,
                                   bool supportsiOS2,
                                   bool supportsiOS4);

    std::array<Format, angle::kNumANGLEFormats> mPixelFormatTable;
    angle::HashMap<MTLPixelFormat, FormatCaps> mNativePixelFormatCapsTable;
    // One for tightly packed buffers, one for general cases.
    std::array<VertexFormat, angle::kNumANGLEFormats> mVertexFormatTables[2];

    uint32_t mMaxSamples;
};

}  // namespace mtl
}  // namespace rx

#endif /* LIBANGLE_RENDERER_METAL_MTL_FORMAT_UTILS_H_ */
