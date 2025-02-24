//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libANGLE/renderer/wgpu/wgpu_format_utils.h"

#include "libANGLE/renderer/load_functions_table.h"
namespace rx
{
namespace
{
void FillTextureCaps(const angle::Format &angleFormat,
                     angle::FormatID formatID,
                     gl::TextureCaps *outTextureCaps)
{
    if (formatID != angle::FormatID::NONE)
    {
        outTextureCaps->texturable = true;
    }
    outTextureCaps->filterable   = true;
    outTextureCaps->renderbuffer = true;
    outTextureCaps->blendable    = true;
}
}  // namespace

namespace webgpu
{
Format::Format()
    : mIntendedFormatID(angle::FormatID::NONE),
      mIntendedGLFormat(GL_NONE),
      mActualImageFormatID(angle::FormatID::NONE),
      mActualBufferFormatID(angle::FormatID::NONE),
      mImageInitializerFunction(nullptr),
      mIsRenderable(false)
{}
void Format::initImageFallback(const ImageFormatInitInfo *info, int numInfo)
{
    UNIMPLEMENTED();
}

void Format::initBufferFallback(const BufferFormatInitInfo *fallbackInfo, int numInfo)
{
    UNIMPLEMENTED();
}

FormatTable::FormatTable() {}
FormatTable::~FormatTable() {}

void FormatTable::initialize()
{
    for (size_t formatIndex = 0; formatIndex < angle::kNumANGLEFormats; ++formatIndex)
    {
        Format &format                           = mFormatData[formatIndex];
        const auto intendedFormatID              = static_cast<angle::FormatID>(formatIndex);
        const angle::Format &intendedAngleFormat = angle::Format::Get(intendedFormatID);

        format.initialize(intendedAngleFormat);
        format.mIntendedFormatID = intendedFormatID;

        gl::TextureCaps textureCaps;
        FillTextureCaps(format.getActualImageFormat(), format.mActualImageFormatID, &textureCaps);
        if (textureCaps.texturable)
        {
            format.mTextureLoadFunctions =
                GetLoadFunctionsMap(format.mIntendedGLFormat, format.mActualImageFormatID);
        }
    }
}
}  // namespace webgpu
}  // namespace rx
