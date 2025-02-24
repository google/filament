//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// cl_utils.cpp: Helper functions for the CL front end

#include "libANGLE/cl_utils.h"

#include "common/PackedCLEnums_autogen.h"
#include "libANGLE/renderer/CLExtensions.h"

namespace cl
{

size_t GetChannelCount(cl_channel_order channelOrder)
{
    size_t count = 0u;
    switch (channelOrder)
    {
        case CL_R:
        case CL_A:
        case CL_LUMINANCE:
        case CL_INTENSITY:
        case CL_DEPTH:
            count = 1u;
            break;
        case CL_RG:
        case CL_RA:
        case CL_Rx:
            count = 2u;
            break;
        case CL_RGB:
        case CL_RGx:
        case CL_sRGB:
            count = 3u;
            break;
        case CL_RGBA:
        case CL_ARGB:
        case CL_BGRA:
        case CL_ABGR:
        case CL_RGBx:
        case CL_sRGBA:
        case CL_sBGRA:
        case CL_sRGBx:
            count = 4u;
            break;
        default:
            break;
    }
    return count;
}

size_t GetElementSize(const cl_image_format &image_format)
{
    size_t size = 0u;
    switch (image_format.image_channel_data_type)
    {
        case CL_SNORM_INT8:
        case CL_UNORM_INT8:
        case CL_SIGNED_INT8:
        case CL_UNSIGNED_INT8:
            size = GetChannelCount(image_format.image_channel_order);
            break;
        case CL_SNORM_INT16:
        case CL_UNORM_INT16:
        case CL_SIGNED_INT16:
        case CL_UNSIGNED_INT16:
        case CL_HALF_FLOAT:
            size = 2u * GetChannelCount(image_format.image_channel_order);
            break;
        case CL_SIGNED_INT32:
        case CL_UNSIGNED_INT32:
        case CL_FLOAT:
            size = 4u * GetChannelCount(image_format.image_channel_order);
            break;
        case CL_UNORM_SHORT_565:
        case CL_UNORM_SHORT_555:
            size = 2u;
            break;
        case CL_UNORM_INT_101010:
        case CL_UNORM_INT_101010_2:
            size = 4u;
            break;
        default:
            break;
    }
    return size;
}

bool IsValidImageFormat(const cl_image_format *imageFormat, const rx::CLExtensions &extensions)
{
    if (imageFormat == nullptr)
    {
        return false;
    }
    switch (imageFormat->image_channel_order)
    {
        case CL_R:
        case CL_A:
        case CL_LUMINANCE:
        case CL_INTENSITY:
        case CL_RG:
        case CL_RA:
        case CL_RGB:
        case CL_RGBA:
        case CL_ARGB:
        case CL_BGRA:
            break;

        case CL_Rx:
        case CL_RGx:
        case CL_RGBx:
            if (extensions.version < CL_MAKE_VERSION(1, 1, 0))
            {
                return false;
            }
            break;

        case CL_ABGR:
        case CL_sRGB:
        case CL_sRGBA:
        case CL_sBGRA:
        case CL_sRGBx:
            if (extensions.version < CL_MAKE_VERSION(2, 0, 0))
            {
                return false;
            }
            break;

        case CL_DEPTH:
            // CL_DEPTH can only be used if channel data type = CL_UNORM_INT16 or CL_FLOAT.
            if (imageFormat->image_channel_data_type != CL_UNORM_INT16 &&
                imageFormat->image_channel_data_type != CL_FLOAT)
            {
                return false;
            }
            if (!extensions.khrDepthImages)
            {
                return false;
            }
            break;

        default:
            return false;
    }
    switch (imageFormat->image_channel_data_type)
    {
        case CL_SNORM_INT8:
        case CL_SNORM_INT16:
        case CL_UNORM_INT8:
        case CL_UNORM_INT16:
        case CL_SIGNED_INT8:
        case CL_SIGNED_INT16:
        case CL_SIGNED_INT32:
        case CL_UNSIGNED_INT8:
        case CL_UNSIGNED_INT16:
        case CL_UNSIGNED_INT32:
        case CL_HALF_FLOAT:
        case CL_FLOAT:
            break;

        case CL_UNORM_SHORT_565:
        case CL_UNORM_SHORT_555:
        case CL_UNORM_INT_101010:
            if (imageFormat->image_channel_order != CL_RGB &&
                imageFormat->image_channel_order != CL_RGBx)
            {
                return false;
            }
            break;

        case CL_UNORM_INT_101010_2:
            if (extensions.version < CL_MAKE_VERSION(2, 1, 0) ||
                imageFormat->image_channel_order != CL_RGBA)
            {
                return false;
            }
            break;

        default:
            return false;
    }
    return true;
}

bool IsImageType(cl::MemObjectType type)
{
    return (type >= cl::MemObjectType::Image2D && type <= cl::MemObjectType::Image1D_Buffer);
}

bool IsBufferType(cl::MemObjectType type)
{
    return type == cl::MemObjectType::Buffer;
}

bool IsArrayType(cl::MemObjectType type)
{
    return (type == cl::MemObjectType::Image1D_Array || type == cl::MemObjectType::Image2D_Array);
}

bool Is3DImage(cl::MemObjectType type)
{
    return (type == cl::MemObjectType::Image3D);
}

bool Is2DImage(cl::MemObjectType type)
{
    return (type == cl::MemObjectType::Image2D || type == cl::MemObjectType::Image2D_Array);
}

bool Is1DImage(cl::MemObjectType type)
{
    return (type >= cl::MemObjectType::Image1D && type <= cl::MemObjectType::Image1D_Buffer);
}

cl::Extents GetExtentFromDescriptor(cl::ImageDescriptor desc)
{
    cl::Extents extent{};

    extent.width  = desc.width;
    extent.height = desc.height;
    extent.depth  = desc.depth;

    // user can supply random values for height and depth for formats that dont need them
    switch (desc.type)
    {
        case cl::MemObjectType::Image1D:
        case cl::MemObjectType::Image1D_Array:
        case cl::MemObjectType::Image1D_Buffer:
            extent.height = 1;
            extent.depth  = 1;
            break;
        case cl::MemObjectType::Image2D:
        case cl::MemObjectType::Image2D_Array:
            extent.depth = 1;
            break;
        default:
            break;
    }
    return extent;
}

thread_local cl_int gClErrorTls;

}  // namespace cl
