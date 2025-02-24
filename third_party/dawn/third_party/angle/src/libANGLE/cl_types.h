//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// cl_types.h: Defines common types for the OpenCL support in ANGLE.

#ifndef LIBANGLE_CLTYPES_H_
#define LIBANGLE_CLTYPES_H_

#if defined(ANGLE_ENABLE_CL)
#    include "libANGLE/CLBitField.h"
#    include "libANGLE/CLRefPointer.h"
#    include "libANGLE/Debug.h"
#    include "libANGLE/angletypes.h"

#    include "common/PackedCLEnums_autogen.h"
#    include "common/angleutils.h"

// Include frequently used standard headers
#    include <algorithm>
#    include <array>
#    include <functional>
#    include <list>
#    include <memory>
#    include <string>
#    include <utility>
#    include <vector>

namespace cl
{

class Buffer;
class CommandQueue;
class Context;
class Device;
class Event;
class Image;
class Kernel;
class Memory;
class Object;
class Platform;
class Program;
class Sampler;

using BufferPtr       = RefPointer<Buffer>;
using CommandQueuePtr = RefPointer<CommandQueue>;
using ContextPtr      = RefPointer<Context>;
using DevicePtr       = RefPointer<Device>;
using EventPtr        = RefPointer<Event>;
using KernelPtr       = RefPointer<Kernel>;
using MemoryPtr       = RefPointer<Memory>;
using PlatformPtr     = RefPointer<Platform>;
using ProgramPtr      = RefPointer<Program>;
using SamplerPtr      = RefPointer<Sampler>;

using BufferPtrs   = std::vector<BufferPtr>;
using DevicePtrs   = std::vector<DevicePtr>;
using EventPtrs    = std::vector<EventPtr>;
using KernelPtrs   = std::vector<KernelPtr>;
using MemoryPtrs   = std::vector<MemoryPtr>;
using PlatformPtrs = std::vector<PlatformPtr>;
using ProgramPtrs  = std::vector<ProgramPtr>;

using WorkgroupSize    = std::array<uint32_t, 3>;
using GlobalWorkOffset = std::array<uint32_t, 3>;
using GlobalWorkSize   = std::array<uint32_t, 3>;
using WorkgroupCount   = std::array<uint32_t, 3>;

template <typename T>
using EventStatusMap = std::array<T, 3>;

using Extents = angle::Extents<size_t>;
using Offset  = angle::Offset<size_t>;
constexpr Offset kOffsetZero(0, 0, 0);

struct KernelArg
{
    bool isSet;
    cl_uint index;
    size_t size;
    const void *valuePtr;
};

struct BufferRect
{
    BufferRect(const Offset &offset,
               const Extents &size,
               const size_t row_pitch,
               const size_t slice_pitch,
               const size_t element_size = 1)
        : mOrigin(offset),
          mSize(size),
          mRowPitch(row_pitch == 0 ? element_size * size.width : row_pitch),
          mSlicePitch(slice_pitch == 0 ? mRowPitch * size.height : slice_pitch),
          mElementSize(element_size)
    {}
    bool valid() const
    {
        return mSize.width != 0 && mSize.height != 0 && mSize.depth != 0 &&
               mRowPitch >= mSize.width * mElementSize && mSlicePitch >= mRowPitch * mSize.height &&
               mElementSize > 0;
    }
    bool operator==(const BufferRect &other) const
    {
        return (mOrigin == other.mOrigin && mSize == other.mSize && mRowPitch == other.mRowPitch &&
                mSlicePitch == other.mSlicePitch && mElementSize == other.mElementSize);
    }
    bool operator!=(const BufferRect &other) const { return !(*this == other); }

    size_t getRowOffset(size_t slice, size_t row) const
    {
        return ((mRowPitch * (mOrigin.y + row)) + (mOrigin.x * mElementSize)) +  // row offset
               (mSlicePitch * (mOrigin.z + slice));                              // slice offset
    }

    size_t getRowPitch() { return mRowPitch; }
    size_t getSlicePitch() { return mSlicePitch; }
    Offset mOrigin;
    Extents mSize;
    size_t mRowPitch;
    size_t mSlicePitch;
    size_t mElementSize;
};

struct ImageDescriptor
{
    MemObjectType type;
    size_t width;
    size_t height;
    size_t depth;
    size_t arraySize;
    size_t rowPitch;
    size_t slicePitch;
    cl_uint numMipLevels;
    cl_uint numSamples;

    ImageDescriptor(MemObjectType type_,
                    size_t width_,
                    size_t height_,
                    size_t depth_,
                    size_t arraySize_,
                    size_t rowPitch_,
                    size_t slicePitch_,
                    cl_uint numMipLevels_,
                    cl_uint numSamples_)
        : type(type_),
          width(width_),
          height(height_),
          depth(depth_),
          arraySize(arraySize_),
          rowPitch(rowPitch_),
          slicePitch(slicePitch_),
          numMipLevels(numMipLevels_),
          numSamples(numSamples_)
    {
        if (type == MemObjectType::Image1D || type == MemObjectType::Image1D_Array ||
            type == MemObjectType::Image1D_Buffer)
        {
            depth  = 1;
            height = 1;
        }
        if (type == MemObjectType::Image2D || type == MemObjectType::Image2D_Array)
        {
            depth = 1;
        }
        if (!(type == MemObjectType::Image1D_Array || type == MemObjectType::Image2D_Array))
        {
            arraySize = 1;
        }
    }
};

struct MemOffsets
{
    size_t x, y, z;
};
constexpr MemOffsets kMemOffsetsZero{0, 0, 0};

struct Coordinate
{
    size_t x, y, z;
};
constexpr Coordinate kCoordinateZero{0, 0, 0};

struct NDRange
{
    NDRange(cl_uint workDimensionsIn,
            const size_t *globalWorkOffsetIn,
            const size_t *globalWorkSizeIn,
            const size_t *localWorkSizeIn)
        : workDimensions(workDimensionsIn),
          globalWorkOffset({0, 0, 0}),
          globalWorkSize({1, 1, 1}),
          localWorkSize({1, 1, 1}),
          nullLocalWorkSize(localWorkSizeIn == nullptr)
    {
        for (cl_uint dim = 0; dim < workDimensionsIn; dim++)
        {
            if (globalWorkOffsetIn != nullptr)
            {
                ASSERT(!(static_cast<uint32_t>((globalWorkOffsetIn[dim] + globalWorkSizeIn[dim])) <
                         globalWorkOffsetIn[dim]));
                globalWorkOffset[dim] = static_cast<uint32_t>(globalWorkOffsetIn[dim]);
            }
            if (globalWorkSizeIn != nullptr)
            {
                ASSERT(globalWorkSizeIn[dim] <= UINT32_MAX);
                globalWorkSize[dim] = static_cast<uint32_t>(globalWorkSizeIn[dim]);
            }
            if (localWorkSizeIn != nullptr)
            {
                ASSERT(localWorkSizeIn[dim] <= UINT32_MAX);
                localWorkSize[dim] = static_cast<uint32_t>(localWorkSizeIn[dim]);
            }
        }
    }

    cl::WorkgroupCount getWorkgroupCount() const
    {
        ASSERT(localWorkSize[0] > 0 && localWorkSize[1] > 0 && localWorkSize[2] > 0);
        return cl::WorkgroupCount{rx::UnsignedCeilDivide(globalWorkSize[0], localWorkSize[0]),
                                  rx::UnsignedCeilDivide(globalWorkSize[1], localWorkSize[1]),
                                  rx::UnsignedCeilDivide(globalWorkSize[2], localWorkSize[2])};
    }

    bool isUniform() const
    {
        for (cl_uint dim = 0; dim < workDimensions; dim++)
        {
            if (globalWorkSize[dim] % localWorkSize[dim] != 0)
            {
                return false;
            }
        }
        return true;
    }

    std::vector<NDRange> createUniformRegions(
        const std::array<uint32_t, 3> maxComputeWorkGroupCount) const
    {
        std::vector<NDRange> regions;
        regions.push_back(*this);
        regions.front().globalWorkOffset = {0};
        uint32_t regionCount             = 1;
        for (uint32_t regionPos = 0; regionPos < regionCount; ++regionPos)
        {
            // "Work-group sizes could be non-uniform in multiple dimensions, potentially producing
            // work-groups of up to 4 different sizes in a 2D range and 8 different sizes in a 3D
            // range."
            // https://registry.khronos.org/OpenCL/specs/3.0-unified/html/OpenCL_API.html#_mapping_work_items_onto_an_nd_range
            ASSERT(regionPos < 8);

            for (uint32_t dim = 0; dim < workDimensions; dim++)
            {
                NDRange &region    = regions.at(regionPos);
                uint32_t remainder = region.globalWorkSize[dim] % region.localWorkSize[dim];
                if (remainder != 0)
                {
                    // Split the range along this dimension. The original range's global work size
                    // (e.g. 19) is clipped to a multiple of the local work size (e.g. 8). A new
                    // range is added for the remainder (in this example 3) where the global and
                    // local work sizes are identical to the remainder (i.e. it's also a uniform
                    // range).
                    NDRange newRegion(region);
                    newRegion.globalWorkSize[dim] = newRegion.localWorkSize[dim] = remainder;
                    region.globalWorkSize[dim] = newRegion.globalWorkOffset[dim] =
                        (region.globalWorkSize[dim] - remainder);
                    regions.push_back(newRegion);
                    regionCount++;
                }
            }
        }
        // Break into uniform regions that fit into given maxComputeWorkGroupCount (if needed)
        uint32_t limitRegionCount = 1;
        std::vector<NDRange> regionsWithinDeviceLimits;
        for (const auto &region : regions)
        {
            regionsWithinDeviceLimits.push_back(region);
            for (uint32_t regionPos = 0; regionPos < limitRegionCount; ++regionPos)
            {
                NDRange &currentRegion = regionsWithinDeviceLimits.at(regionPos);
                for (uint32_t dim = 0; dim < workDimensions; dim++)
                {
                    uint32_t maxGwsForRegion = gl::clampCast<uint32_t, uint64_t>(
                        static_cast<uint64_t>(maxComputeWorkGroupCount[dim]) *
                        static_cast<uint64_t>(currentRegion.localWorkSize[dim]));

                    if (currentRegion.globalWorkSize[dim] > maxGwsForRegion)
                    {
                        uint32_t remainderGws = currentRegion.globalWorkSize[dim] - maxGwsForRegion;
                        if (remainderGws > 0)
                        {
                            NDRange remainderRegion             = currentRegion;
                            remainderRegion.globalWorkSize[dim] = remainderGws;
                            remainderRegion.globalWorkOffset[dim] =
                                currentRegion.globalWorkOffset[dim] +
                                (currentRegion.globalWorkSize[dim] - remainderGws);
                            currentRegion.globalWorkSize[dim] = maxGwsForRegion;
                            regionsWithinDeviceLimits.push_back(remainderRegion);
                            limitRegionCount++;
                        }
                    }
                }
            }
        }
        return regionsWithinDeviceLimits;
    }

    cl_uint workDimensions;
    GlobalWorkOffset globalWorkOffset;
    GlobalWorkSize globalWorkSize;
    WorkgroupSize localWorkSize;
    bool nullLocalWorkSize{false};
};

}  // namespace cl

#endif  // ANGLE_ENABLE_CL

#endif  // LIBANGLE_CLTYPES_H_
