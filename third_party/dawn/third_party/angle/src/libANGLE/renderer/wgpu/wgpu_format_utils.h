//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// wgpu_format_utils:
//   Helper for WebGPU format code.

#ifndef LIBANGLE_RENDERER_WGPU_WGPU_FORMAT_UTILS_H_
#define LIBANGLE_RENDERER_WGPU_WGPU_FORMAT_UTILS_H_

#include <dawn/webgpu_cpp.h>

#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/Format.h"
#include "libANGLE/renderer/copyvertex.h"

namespace rx
{
namespace webgpu
{

struct ImageFormatInitInfo final
{
    angle::FormatID format;
    InitializeTextureDataFunction initializer;
};

struct BufferFormatInitInfo final
{
    angle::FormatID format;
    VertexCopyFunction VertexLoadFunction;
    bool VertexLoadRequiresConversion;
};

wgpu::TextureFormat GetWgpuTextureFormatFromFormatID(angle::FormatID formatID);
angle::FormatID GetFormatIDFromWgpuTextureFormat(wgpu::TextureFormat wgpuFormat);
wgpu::VertexFormat GetWgpuVertexFormatFromFormatID(angle::FormatID formatID);
angle::FormatID GetFormatIDFromWgpuBufferFormat(wgpu::VertexFormat wgpuFormat);

// Describes a WebGPU format. WebGPU has separate formats for images and vertex buffers, this class
// describes both.
class Format final : private angle::NonCopyable
{
  public:
    Format();

    bool valid() const { return mIntendedGLFormat != 0; }

    // The intended format is the front-end format. For Textures this usually correponds to a
    // GLenum in the headers. Buffer formats don't always have a corresponding GLenum type.
    // Some Surface formats and unsized types also don't have a corresponding GLenum.
    angle::FormatID getIntendedFormatID() const { return mIntendedFormatID; }
    const angle::Format &getIntendedFormat() const { return angle::Format::Get(mIntendedFormatID); }

    // The actual Image format is used to implement the front-end format for Texture/Renderbuffers.
    const angle::Format &getActualImageFormat() const
    {
        return angle::Format::Get(getActualImageFormatID());
    }

    LoadImageFunctionInfo getTextureLoadFunction(GLenum type) const
    {
        return mTextureLoadFunctions(type);
    }

    wgpu::TextureFormat getActualWgpuTextureFormat() const
    {
        return GetWgpuTextureFormatFromFormatID(mActualImageFormatID);
    }
    wgpu::VertexFormat getActualWgpuVertexFormat() const
    {
        return GetWgpuVertexFormatFromFormatID(mActualBufferFormatID);
    }

    angle::FormatID getActualImageFormatID() const { return mActualImageFormatID; }

    // The actual Buffer format is used to implement the front-end format for Buffers.  This format
    // is used by vertex buffers as well as texture buffers.
    const angle::Format &getActualBufferFormat() const
    {
        return angle::Format::Get(mActualBufferFormatID);
    }

    // |intendedGLFormat| always correponds to a valid GLenum type. For types that don't have a
    // corresponding GLenum we do our best to specify a GLenum that is "close".
    const gl::InternalFormat &getInternalFormatInfo(GLenum type) const
    {
        return gl::GetInternalFormatInfo(mIntendedGLFormat, type);
    }

  private:
    friend class FormatTable;
    // This is an auto-generated method in vk_format_table_autogen.cpp.
    void initialize(const angle::Format &intendedAngleFormat);

    // These are used in the format table init.
    void initImageFallback(const ImageFormatInitInfo *info, int numInfo);

    void initBufferFallback(const BufferFormatInitInfo *fallbackInfo, int numInfo);

    angle::FormatID mIntendedFormatID;
    GLenum mIntendedGLFormat;
    angle::FormatID mActualImageFormatID;
    angle::FormatID mActualBufferFormatID;

    InitializeTextureDataFunction mImageInitializerFunction;
    LoadFunctionMap mTextureLoadFunctions;
    VertexCopyFunction mVertexLoadFunction;

    bool mVertexLoadRequiresConversion;
    bool mIsRenderable;
};

bool operator==(const Format &lhs, const Format &rhs);
bool operator!=(const Format &lhs, const Format &rhs);

class FormatTable final : angle::NonCopyable
{
  public:
    FormatTable();
    ~FormatTable();

    void initialize();

    ANGLE_INLINE const Format &operator[](GLenum internalFormat) const
    {
        angle::FormatID formatID = angle::Format::InternalFormatToID(internalFormat);
        return mFormatData[static_cast<size_t>(formatID)];
    }

    ANGLE_INLINE const Format &operator[](angle::FormatID formatID) const
    {
        return mFormatData[static_cast<size_t>(formatID)];
    }

  private:
    // The table data is indexed by angle::FormatID.
    std::array<Format, angle::kNumANGLEFormats> mFormatData;
};
}  // namespace webgpu
}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_WGPU_FORMAT_UTILS_H_
