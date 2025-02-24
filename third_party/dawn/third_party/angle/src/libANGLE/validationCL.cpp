//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// validationCL.cpp: Validation functions for generic CL entry point parameters
// based on the OpenCL Specification V3.0.7, see https://www.khronos.org/registry/OpenCL/
// Each used CL error code is preceeded by a citation of the relevant rule in the spec.

#include "libANGLE/cl_utils.h"
#include "libANGLE/validationCL_autogen.h"

#include "common/string_utils.h"

#define ANGLE_VALIDATE_VERSION(version, major, minor)          \
    do                                                         \
    {                                                          \
        if (version < CL_MAKE_VERSION(major##u, minor##u, 0u)) \
        {                                                      \
            return CL_INVALID_VALUE;                           \
        }                                                      \
    } while (0)

#define ANGLE_VALIDATE_EXTENSION(extension) \
    do                                      \
    {                                       \
        if (!extension)                     \
        {                                   \
            return CL_INVALID_VALUE;        \
        }                                   \
    } while (0)

#define ANGLE_VALIDATE(expression)            \
    do                                        \
    {                                         \
        const cl_int _errorCode = expression; \
        if (_errorCode != CL_SUCCESS)         \
        {                                     \
            return _errorCode;                \
        }                                     \
    } while (0)

#define ANGLE_VALIDATE_VERSION_OR_EXTENSION(version, major, minor, extension) \
    do                                                                        \
    {                                                                         \
        if (version < CL_MAKE_VERSION(major##u, minor##u, 0u))                \
        {                                                                     \
            ANGLE_VALIDATE_EXTENSION(extension);                              \
        }                                                                     \
    } while (0)

namespace cl
{

namespace
{

cl_int ValidateContextProperties(const cl_context_properties *properties, const Platform *&platform)
{
    platform         = nullptr;
    bool hasUserSync = false;
    if (properties != nullptr)
    {
        while (*properties != 0)
        {
            switch (*properties++)
            {
                case CL_CONTEXT_PLATFORM:
                {
                    // CL_INVALID_PROPERTY if the same property name is specified more than once.
                    if (platform != nullptr)
                    {
                        return CL_INVALID_PROPERTY;
                    }
                    cl_platform_id nativePlatform = reinterpret_cast<cl_platform_id>(*properties++);
                    // CL_INVALID_PLATFORM if platform value specified in properties
                    // is not a valid platform.
                    if (!Platform::IsValid(nativePlatform))
                    {
                        return CL_INVALID_PLATFORM;
                    }
                    platform = &nativePlatform->cast<Platform>();
                    break;
                }
                case CL_CONTEXT_INTEROP_USER_SYNC:
                {
                    // CL_INVALID_PROPERTY if the value specified for a supported property name
                    // is not valid, or if the same property name is specified more than once.
                    if ((*properties != CL_FALSE && *properties != CL_TRUE) || hasUserSync)
                    {
                        return CL_INVALID_PROPERTY;
                    }
                    ++properties;
                    hasUserSync = true;
                    break;
                }
                default:
                {
                    // CL_INVALID_PROPERTY if context property name in properties
                    // is not a supported property name.
                    return CL_INVALID_PROPERTY;
                }
            }
        }
    }
    return CL_SUCCESS;
}

bool ValidateMemoryFlags(MemFlags flags, const Platform &platform)
{
    // CL_MEM_READ_WRITE, CL_MEM_WRITE_ONLY, and CL_MEM_READ_ONLY are mutually exclusive.
    MemFlags allowedFlags(CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY);
    if (!flags.areMutuallyExclusive(CL_MEM_READ_WRITE, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY))
    {
        return false;
    }
    // CL_MEM_USE_HOST_PTR is mutually exclusive with either of the other two flags.
    allowedFlags.set(CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR);
    if (!flags.areMutuallyExclusive(CL_MEM_USE_HOST_PTR,
                                    CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR))
    {
        return false;
    }
    if (platform.isVersionOrNewer(1u, 2u))
    {
        // CL_MEM_HOST_WRITE_ONLY, CL_MEM_HOST_READ_ONLY,
        // and CL_MEM_HOST_NO_ACCESS are mutually exclusive.
        allowedFlags.set(CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS);
        if (!flags.areMutuallyExclusive(CL_MEM_HOST_WRITE_ONLY, CL_MEM_HOST_READ_ONLY,
                                        CL_MEM_HOST_NO_ACCESS))
        {
            return false;
        }
    }
    if (platform.isVersionOrNewer(2u, 0u))
    {
        allowedFlags.set(CL_MEM_KERNEL_READ_AND_WRITE);
    }
    if (flags.hasOtherBitsThan(allowedFlags))
    {
        return false;
    }
    return true;
}

bool ValidateMapFlags(MapFlags flags, const Platform &platform)
{
    MemFlags allowedFlags(CL_MAP_READ | CL_MAP_WRITE);
    if (platform.isVersionOrNewer(1u, 2u))
    {
        // CL_MAP_READ or CL_MAP_WRITE and CL_MAP_WRITE_INVALIDATE_REGION are mutually exclusive.
        allowedFlags.set(CL_MAP_WRITE_INVALIDATE_REGION);
        if (!flags.areMutuallyExclusive(CL_MAP_WRITE_INVALIDATE_REGION, CL_MAP_READ | CL_MAP_WRITE))
        {
            return false;
        }
    }
    if (flags.hasOtherBitsThan(allowedFlags))
    {
        return false;
    }
    return true;
}

bool ValidateMemoryProperties(const cl_mem_properties *properties)
{
    if (properties != nullptr)
    {
        // OpenCL 3.0 does not define any optional properties.
        // This function is reserved for extensions and future use.
        if (*properties != 0)
        {
            return false;
        }
    }
    return true;
}

cl_int ValidateCommandQueueAndEventWaitList(cl_command_queue commandQueue,
                                            bool validateImageSupport,
                                            cl_uint numEvents,
                                            const cl_event *events)
{
    // CL_INVALID_COMMAND_QUEUE if command_queue is not a valid host command-queue.
    if (!CommandQueue::IsValid(commandQueue))
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    const CommandQueue &queue = commandQueue->cast<CommandQueue>();
    if (!queue.isOnHost())
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if (validateImageSupport)
    {
        // CL_INVALID_OPERATION if the device associated with command_queue does not support images.
        if (queue.getDevice().getInfo().imageSupport == CL_FALSE)
        {
            return CL_INVALID_OPERATION;
        }
    }

    // CL_INVALID_EVENT_WAIT_LIST if event_wait_list is NULL and num_events_in_wait_list > 0,
    // or event_wait_list is not NULL and num_events_in_wait_list is 0, ...
    if ((events == nullptr) != (numEvents == 0u))
    {
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    while (numEvents-- != 0u)
    {
        // or if event objects in event_wait_list are not valid events.
        if (!Event::IsValid(*events))
        {
            return CL_INVALID_EVENT_WAIT_LIST;
        }

        // CL_INVALID_CONTEXT if the context associated with command_queue
        // and events in event_wait_list are not the same.
        if (&queue.getContext() != &(*events++)->cast<Event>().getContext())
        {
            return CL_INVALID_CONTEXT;
        }
    }

    return CL_SUCCESS;
}

cl_int ValidateEnqueueBuffer(const CommandQueue &queue,
                             cl_mem buffer,
                             bool hostRead,
                             bool hostWrite)
{
    // CL_INVALID_MEM_OBJECT if buffer is not a valid buffer object.
    if (!Buffer::IsValid(buffer))
    {
        return CL_INVALID_MEM_OBJECT;
    }
    const Buffer &buf = buffer->cast<Buffer>();

    // CL_INVALID_CONTEXT if the context associated with command_queue and buffer are not the same.
    if (&queue.getContext() != &buf.getContext())
    {
        return CL_INVALID_CONTEXT;
    }

    // CL_MISALIGNED_SUB_BUFFER_OFFSET if buffer is a sub-buffer object and offset specified
    // when the sub-buffer object is created is not aligned to CL_DEVICE_MEM_BASE_ADDR_ALIGN
    // value (which is in bits!) for device associated with queue.
    if (buf.isSubBuffer() &&
        (buf.getOffset() % (queue.getDevice().getInfo().memBaseAddrAlign / 8u)) != 0u)
    {
        return CL_MISALIGNED_SUB_BUFFER_OFFSET;
    }

    // CL_INVALID_OPERATION if a read function is called on buffer which
    // has been created with CL_MEM_HOST_WRITE_ONLY or CL_MEM_HOST_NO_ACCESS.
    if (hostRead && buf.getFlags().intersects(CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS))
    {
        return CL_INVALID_OPERATION;
    }

    // CL_INVALID_OPERATION if a write function is called on buffer which
    // has been created with CL_MEM_HOST_READ_ONLY or CL_MEM_HOST_NO_ACCESS.
    if (hostWrite && buf.getFlags().intersects(CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS))
    {
        return CL_INVALID_OPERATION;
    }

    return CL_SUCCESS;
}

cl_int ValidateBufferRect(const Buffer &buffer,
                          const size_t *origin,
                          const size_t *region,
                          size_t rowPitch,
                          size_t slicePitch)
{
    // CL_INVALID_VALUE if origin or region is NULL.
    if (origin == nullptr || region == nullptr)
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_VALUE if any region array element is 0.
    if (region[0] == 0u || region[1] == 0u || region[2] == 0u)
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_VALUE if row_pitch is not 0 and is less than region[0].
    if (rowPitch == 0u)
    {
        rowPitch = region[0];
    }
    else if (rowPitch < region[0])
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_VALUE if slice_pitch is not 0 and is less than
    // region[1] x row_pitch and not a multiple of row_pitch.
    if (slicePitch == 0u)
    {
        slicePitch = region[1] * rowPitch;
    }
    else if (slicePitch < region[1] * rowPitch || (slicePitch % rowPitch) != 0u)
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_VALUE if the region being read or written specified
    // by (origin, region, row_pitch, slice_pitch) is out of bounds.
    if (!buffer.isRegionValid(
            origin[2] * slicePitch + origin[1] * rowPitch + origin[0],
            (region[2] - 1u) * slicePitch + (region[1] - 1u) * rowPitch + region[0]))
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateHostRect(const size_t *hostOrigin,
                        const size_t *region,
                        size_t hostRowPitch,
                        size_t hostSlicePitch,
                        const void *ptr)
{
    // CL_INVALID_VALUE if host_origin or region is NULL.
    if (hostOrigin == nullptr || region == nullptr)
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_VALUE if any region array element is 0.
    if (region[0] == 0u || region[1] == 0u || region[2] == 0u)
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_VALUE if host_row_pitch is not 0 and is less than region[0].
    if (hostRowPitch == 0u)
    {
        hostRowPitch = region[0];
    }
    else if (hostRowPitch < region[0])
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_VALUE if host_slice_pitch is not 0 and is less than
    // region[1] x host_row_pitch and not a multiple of host_row_pitch.
    if (hostSlicePitch != 0u &&
        (hostSlicePitch < region[1] * hostRowPitch || (hostSlicePitch % hostRowPitch) != 0u))
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_VALUE if ptr is NULL.
    if (ptr == nullptr)
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateEnqueueImage(const CommandQueue &queue, cl_mem image, bool hostRead, bool hostWrite)
{
    // CL_INVALID_MEM_OBJECT if image is not a valid image object.
    if (!Image::IsValid(image))
    {
        return CL_INVALID_MEM_OBJECT;
    }
    const Image &img = image->cast<Image>();

    // CL_INVALID_CONTEXT if the context associated with command_queue and image are not the same.
    if (&queue.getContext() != &img.getContext())
    {
        return CL_INVALID_CONTEXT;
    }

    // CL_INVALID_OPERATION if a read function is called on image which
    // has been created with CL_MEM_HOST_WRITE_ONLY or CL_MEM_HOST_NO_ACCESS.
    if (hostRead && img.getFlags().intersects(CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS))
    {
        return CL_INVALID_OPERATION;
    }

    // CL_INVALID_OPERATION if a write function is called on image which
    // has been created with CL_MEM_HOST_READ_ONLY or CL_MEM_HOST_NO_ACCESS.
    if (hostWrite && img.getFlags().intersects(CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS))
    {
        return CL_INVALID_OPERATION;
    }

    return CL_SUCCESS;
}

cl_int ValidateImageForDevice(const Image &image,
                              const Device &device,
                              const size_t *origin,
                              const size_t *region)
{
    // CL_INVALID_VALUE if origin or region is NULL.
    if (origin == nullptr || region == nullptr)
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_VALUE if values in origin and region do not follow rules
    // described in the argument description for origin and region.
    // The values in region cannot be 0.
    if (region[0] == 0u || region[1] == 0u || region[2] == 0u)
    {
        return CL_INVALID_VALUE;
    }
    switch (image.getType())
    {
        // If image is a 1D image or 1D image buffer object,
        // origin[1] and origin[2] must be 0 and region[1] and region[2] must be 1.
        case MemObjectType::Image1D:
        case MemObjectType::Image1D_Buffer:
            if (origin[1] != 0u || origin[2] != 0u || region[1] != 1u || region[2] != 1u)
            {
                return CL_INVALID_VALUE;
            }
            break;
        // If image is a 2D image object or a 1D image array object,
        // origin[2] must be 0 and region[2] must be 1.
        case MemObjectType::Image2D:
        case MemObjectType::Image1D_Array:
            if (origin[2] != 0u || region[2] != 1u)
            {
                return CL_INVALID_VALUE;
            }
            break;
        case MemObjectType::Image3D:
        case MemObjectType::Image2D_Array:
            break;
        default:
            ASSERT(false);
            return CL_INVALID_IMAGE_DESCRIPTOR;
    }

    // CL_INVALID_VALUE if the region being read or written
    // specified by origin and region is out of bounds.

    if (!image.isRegionValid(cl::MemOffsets{origin[0], origin[1], origin[2]},
                             cl::Coordinate{region[0], region[1], region[2]}))
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_IMAGE_SIZE if image dimensions (image width, height, specified or compute
    // row and/or slice pitch) for image are not supported by device associated with queue.
    if (!device.supportsImageDimensions(image.getDescriptor()))
    {
        return CL_INVALID_IMAGE_SIZE;
    }

    return CL_SUCCESS;
}

cl_int ValidateHostRegionForImage(const Image &image,
                                  const size_t region[3],
                                  size_t rowPitch,
                                  size_t slicePitch,
                                  const void *ptr)
{
    // CL_INVALID_VALUE if row_pitch is not 0 and is less than the element size in bytes x width.
    if (rowPitch == 0u)
    {
        rowPitch = image.getElementSize() * region[0];
    }
    else if (rowPitch < image.getElementSize() * region[0])
    {
        return CL_INVALID_VALUE;
    }
    if (slicePitch != 0u)
    {
        // TODO(jplate) Follow up with https://github.com/KhronosGroup/OpenCL-Docs/issues/624
        // This error is missing in the OpenCL spec.
        // slice_pitch must be 0 if image is a 1D or 2D image.
        if (image.getType() == MemObjectType::Image1D ||
            image.getType() == MemObjectType::Image1D_Buffer ||
            image.getType() == MemObjectType::Image2D)
        {
            return CL_INVALID_VALUE;
        }
        else if (slicePitch < rowPitch)
        {
            return CL_INVALID_VALUE;
        }
        // CL_INVALID_VALUE if slice_pitch is not 0 and is less than row_pitch x height.
        else if (((image.getType() == MemObjectType::Image2D_Array) ||
                  (image.getType() == MemObjectType::Image3D)) &&
                 (slicePitch < rowPitch * region[1]))
        {
            return CL_INVALID_VALUE;
        }
    }

    // CL_INVALID_VALUE if ptr is NULL.
    if (ptr == nullptr)
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

}  // namespace

// CL 1.0
cl_int ValidateGetPlatformIDs(cl_uint num_entries,
                              const cl_platform_id *platforms,
                              const cl_uint *num_platforms)
{
    // CL_INVALID_VALUE if num_entries is equal to zero and platforms is not NULL
    // or if both num_platforms and platforms are NULL.
    if ((num_entries == 0u && platforms != nullptr) ||
        (platforms == nullptr && num_platforms == nullptr))
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateGetPlatformInfo(cl_platform_id platform,
                               PlatformInfo param_name,
                               size_t param_value_size,
                               const void *param_value,
                               const size_t *param_value_size_ret)
{
    // CL_INVALID_PLATFORM if platform is not a valid platform.
    if (!Platform::IsValidOrDefault(platform))
    {
        return CL_INVALID_PLATFORM;
    }

    // CL_INVALID_VALUE if param_name is not one of the supported values.
    const cl_version version = platform->cast<Platform>().getVersion();
    switch (param_name)
    {
        case PlatformInfo::HostTimerResolution:
            ANGLE_VALIDATE_VERSION(version, 2, 1);
            break;
        case PlatformInfo::NumericVersion:
        case PlatformInfo::ExtensionsWithVersion:
            ANGLE_VALIDATE_VERSION(version, 3, 0);
            break;
        case PlatformInfo::InvalidEnum:
            return CL_INVALID_VALUE;
        default:
            // All remaining possible values for param_name are valid for all versions.
            break;
    }

    return CL_SUCCESS;
}

cl_int ValidateGetDeviceIDs(cl_platform_id platform,
                            DeviceType device_type,
                            cl_uint num_entries,
                            const cl_device_id *devices,
                            const cl_uint *num_devices)
{
    // CL_INVALID_PLATFORM if platform is not a valid platform.
    if (!Platform::IsValidOrDefault(platform))
    {
        return CL_INVALID_PLATFORM;
    }

    // CL_INVALID_DEVICE_TYPE if device_type is not a valid value.
    if (!Device::IsValidType(device_type))
    {
        return CL_INVALID_DEVICE_TYPE;
    }

    // CL_INVALID_VALUE if num_entries is equal to zero and devices is not NULL
    // or if both num_devices and devices are NULL.
    if ((num_entries == 0u && devices != nullptr) || (num_devices == nullptr && devices == nullptr))
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateGetDeviceInfo(cl_device_id device,
                             DeviceInfo param_name,
                             size_t param_value_size,
                             const void *param_value,
                             const size_t *param_value_size_ret)
{
    // CL_INVALID_DEVICE if device is not a valid device.
    if (!Device::IsValid(device))
    {
        return CL_INVALID_DEVICE;
    }
    const Device &dev = device->cast<Device>();

    // CL_INVALID_VALUE if param_name is not one of the supported values
    // or if param_name is a value that is available as an extension
    // and the corresponding extension is not supported by the device.
    const cl_version version           = dev.getVersion();
    const rx::CLDeviceImpl::Info &info = dev.getInfo();
    // Enums ordered within their version block as they appear in the OpenCL spec V3.0.7, table 5
    switch (param_name)
    {
        case DeviceInfo::PreferredVectorWidthHalf:
        case DeviceInfo::NativeVectorWidthChar:
        case DeviceInfo::NativeVectorWidthShort:
        case DeviceInfo::NativeVectorWidthInt:
        case DeviceInfo::NativeVectorWidthLong:
        case DeviceInfo::NativeVectorWidthFloat:
        case DeviceInfo::NativeVectorWidthDouble:
        case DeviceInfo::NativeVectorWidthHalf:
        case DeviceInfo::HostUnifiedMemory:
        case DeviceInfo::OpenCL_C_Version:
            ANGLE_VALIDATE_VERSION(version, 1, 1);
            break;

        case DeviceInfo::ImageMaxBufferSize:
        case DeviceInfo::ImageMaxArraySize:
        case DeviceInfo::LinkerAvailable:
        case DeviceInfo::BuiltInKernels:
        case DeviceInfo::PrintfBufferSize:
        case DeviceInfo::PreferredInteropUserSync:
        case DeviceInfo::ParentDevice:
        case DeviceInfo::PartitionMaxSubDevices:
        case DeviceInfo::PartitionProperties:
        case DeviceInfo::PartitionAffinityDomain:
        case DeviceInfo::PartitionType:
        case DeviceInfo::ReferenceCount:
            ANGLE_VALIDATE_VERSION(version, 1, 2);
            break;

        case DeviceInfo::MaxReadWriteImageArgs:
        case DeviceInfo::ImagePitchAlignment:
        case DeviceInfo::ImageBaseAddressAlignment:
        case DeviceInfo::MaxPipeArgs:
        case DeviceInfo::PipeMaxActiveReservations:
        case DeviceInfo::PipeMaxPacketSize:
        case DeviceInfo::MaxGlobalVariableSize:
        case DeviceInfo::GlobalVariablePreferredTotalSize:
        case DeviceInfo::QueueOnDeviceProperties:
        case DeviceInfo::QueueOnDevicePreferredSize:
        case DeviceInfo::QueueOnDeviceMaxSize:
        case DeviceInfo::MaxOnDeviceQueues:
        case DeviceInfo::MaxOnDeviceEvents:
        case DeviceInfo::SVM_Capabilities:
        case DeviceInfo::PreferredPlatformAtomicAlignment:
        case DeviceInfo::PreferredGlobalAtomicAlignment:
        case DeviceInfo::PreferredLocalAtomicAlignment:
            ANGLE_VALIDATE_VERSION(version, 2, 0);
            break;

        case DeviceInfo::IL_Version:
        case DeviceInfo::MaxNumSubGroups:
        case DeviceInfo::SubGroupIndependentForwardProgress:
            ANGLE_VALIDATE_VERSION(version, 2, 1);
            break;

        case DeviceInfo::ILsWithVersion:
        case DeviceInfo::BuiltInKernelsWithVersion:
        case DeviceInfo::NumericVersion:
        case DeviceInfo::OpenCL_C_AllVersions:
        case DeviceInfo::OpenCL_C_Features:
        case DeviceInfo::ExtensionsWithVersion:
        case DeviceInfo::AtomicMemoryCapabilities:
        case DeviceInfo::AtomicFenceCapabilities:
        case DeviceInfo::NonUniformWorkGroupSupport:
        case DeviceInfo::WorkGroupCollectiveFunctionsSupport:
        case DeviceInfo::GenericAddressSpaceSupport:
        case DeviceInfo::DeviceEnqueueCapabilities:
        case DeviceInfo::PipeSupport:
        case DeviceInfo::PreferredWorkGroupSizeMultiple:
        case DeviceInfo::LatestConformanceVersionPassed:
            ANGLE_VALIDATE_VERSION(version, 3, 0);
            break;

        case DeviceInfo::DoubleFpConfig:
            // This extension became a core query from OpenCL 1.2 onward.
            // Only need to validate for OpenCL versions less than 1.2 here.
            ANGLE_VALIDATE_VERSION_OR_EXTENSION(version, 1, 2, info.khrFP64);
            break;

        case DeviceInfo::InvalidEnum:
            return CL_INVALID_VALUE;
        default:
            // All remaining possible values for param_name are valid for all versions.
            break;
    }

    return CL_SUCCESS;
}

cl_int ValidateCreateContext(const cl_context_properties *properties,
                             cl_uint num_devices,
                             const cl_device_id *devices,
                             void(CL_CALLBACK *pfn_notify)(const char *errinfo,
                                                           const void *private_info,
                                                           size_t cb,
                                                           void *user_data),
                             const void *user_data)
{
    // CL_INVALID_VALUE if devices is NULL or if num_devices is equal to zero
    // or if pfn_notify is NULL but user_data is not NULL.
    if (devices == nullptr || num_devices == 0u || (pfn_notify == nullptr && user_data != nullptr))
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_DEVICE if any device in devices is not a valid device.
    for (cl_uint i = 0; i < num_devices; ++i)
    {
        if (!Device::IsValid(devices[i]))
        {
            return CL_INVALID_DEVICE;
        }
    }

    // Because ANGLE can have one or more platforms here (e.g. passthrough, Vulkan, etc.), if a
    // context platform is not explicitly specified in the properties, spec says to default to an
    // implementation-defined platform. In ANGLE's case, we can derive the platform from the device
    // object.
    const Platform *platform = nullptr;
    ANGLE_VALIDATE(ValidateContextProperties(properties, platform));
    if (platform == nullptr)
    {
        // Just use/pick the first device's platform object here
        platform = &(devices[0])->cast<Device>().getPlatform();
    }

    // Ensure that each device in device list is derived from the same platform object
    for (cl_uint i = 0; i < num_devices; ++i)
    {
        if (platform != &(devices[i])->cast<Device>().getPlatform())
        {
            return CL_INVALID_PLATFORM;
        }
    }

    return CL_SUCCESS;
}

cl_int ValidateCreateContextFromType(const cl_context_properties *properties,
                                     DeviceType device_type,
                                     void(CL_CALLBACK *pfn_notify)(const char *errinfo,
                                                                   const void *private_info,
                                                                   size_t cb,
                                                                   void *user_data),
                                     const void *user_data)
{
    // CL_INVALID_DEVICE_TYPE if device_type is not a valid value.
    if (!Device::IsValidType(device_type))
    {
        return CL_INVALID_DEVICE_TYPE;
    }

    const Platform *platform = nullptr;
    ANGLE_VALIDATE(ValidateContextProperties(properties, platform));
    if (platform == nullptr)
    {
        platform = Platform::GetDefault();
        if (platform == nullptr)
        {
            return CL_INVALID_PLATFORM;
        }
    }

    if (!platform->hasDeviceType(device_type))
    {
        return CL_DEVICE_NOT_FOUND;
    }

    // CL_INVALID_VALUE if pfn_notify is NULL but user_data is not NULL.
    if (pfn_notify == nullptr && user_data != nullptr)
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateRetainContext(cl_context context)
{
    // CL_INVALID_CONTEXT if context is not a valid OpenCL context.
    return Context::IsValid(context) ? CL_SUCCESS : CL_INVALID_CONTEXT;
}

cl_int ValidateReleaseContext(cl_context context)
{
    // CL_INVALID_CONTEXT if context is not a valid OpenCL context.
    return Context::IsValid(context) ? CL_SUCCESS : CL_INVALID_CONTEXT;
}

cl_int ValidateGetContextInfo(cl_context context,
                              ContextInfo param_name,
                              size_t param_value_size,
                              const void *param_value,
                              const size_t *param_value_size_ret)
{
    // CL_INVALID_CONTEXT if context is not a valid context.
    if (!Context::IsValid(context))
    {
        return CL_INVALID_CONTEXT;
    }

    // CL_INVALID_VALUE if param_name is not one of the supported values.
    if (param_name == ContextInfo::InvalidEnum)
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateRetainCommandQueue(cl_command_queue command_queue)
{
    // CL_INVALID_COMMAND_QUEUE if command_queue is not a valid command-queue.
    return CommandQueue::IsValid(command_queue) ? CL_SUCCESS : CL_INVALID_COMMAND_QUEUE;
}

cl_int ValidateReleaseCommandQueue(cl_command_queue command_queue)
{
    // CL_INVALID_COMMAND_QUEUE if command_queue is not a valid command-queue.
    return CommandQueue::IsValid(command_queue) ? CL_SUCCESS : CL_INVALID_COMMAND_QUEUE;
}

cl_int ValidateGetCommandQueueInfo(cl_command_queue command_queue,
                                   CommandQueueInfo param_name,
                                   size_t param_value_size,
                                   const void *param_value,
                                   const size_t *param_value_size_ret)
{
    // CL_INVALID_COMMAND_QUEUE if command_queue is not a valid command-queue
    if (!CommandQueue::IsValid(command_queue))
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    const CommandQueue &queue = command_queue->cast<CommandQueue>();

    // CL_INVALID_VALUE if param_name is not one of the supported values.
    const cl_version version = queue.getDevice().getVersion();
    switch (param_name)
    {
        case CommandQueueInfo::Size:
            ANGLE_VALIDATE_VERSION(version, 2, 0);
            break;
        case CommandQueueInfo::DeviceDefault:
            ANGLE_VALIDATE_VERSION(version, 2, 1);
            break;
        case CommandQueueInfo::PropertiesArray:
            ANGLE_VALIDATE_VERSION(version, 3, 0);
            break;
        case CommandQueueInfo::InvalidEnum:
            return CL_INVALID_VALUE;
        default:
            // All remaining possible values for param_name are valid for all versions.
            break;
    }

    // CL_INVALID_COMMAND_QUEUE if command_queue is not a valid command-queue for param_name.
    if (param_name == CommandQueueInfo::Size)
    {
        if (queue.isOnDevice() && !queue.getDevice().hasDeviceEnqueueCaps())
        {
            return CL_INVALID_COMMAND_QUEUE;
        }
        if (queue.getDevice().getPlatform().isVersionOrNewer(3u, 0u) && !queue.isOnDevice())
        {
            // Device-side enqueue and on-device queues are optional for devices supporting
            // OpenCL 3.0. When device-side enqueue is not supported:
            // - clGetCommandQueueInfo, passing CL_QUEUE_SIZE Returns CL_INVALID_COMMAND_QUEUE since
            // command_queue cannot be a valid device command-queue.
            return CL_INVALID_COMMAND_QUEUE;
        }
    }

    return CL_SUCCESS;
}

cl_int ValidateCreateBuffer(cl_context context, MemFlags flags, size_t size, const void *host_ptr)
{
    // CL_INVALID_CONTEXT if context is not a valid context.
    if (!Context::IsValid(context))
    {
        return CL_INVALID_CONTEXT;
    }
    const Context &ctx = context->cast<Context>();

    // CL_INVALID_VALUE if values specified in flags are not valid
    // as defined in the Memory Flags table.
    if (!ValidateMemoryFlags(flags, ctx.getPlatform()))
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_BUFFER_SIZE if size is 0 ...
    if (size == 0u)
    {
        CL_INVALID_BUFFER_SIZE;
    }
    for (const DevicePtr &device : ctx.getDevices())
    {
        // or if size is greater than CL_DEVICE_MAX_MEM_ALLOC_SIZE for all devices in context.
        if (size > device->getInfo().maxMemAllocSize)
        {
            return CL_INVALID_BUFFER_SIZE;
        }
    }

    // CL_INVALID_HOST_PTR
    // if host_ptr is NULL and CL_MEM_USE_HOST_PTR or CL_MEM_COPY_HOST_PTR are set in flags or
    // if host_ptr is not NULL but CL_MEM_COPY_HOST_PTR or CL_MEM_USE_HOST_PTR are not set in flags.
    if ((host_ptr != nullptr) != flags.intersects(CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR))
    {
        return CL_INVALID_HOST_PTR;
    }

    return CL_SUCCESS;
}

cl_int ValidateRetainMemObject(cl_mem memobj)
{
    // CL_INVALID_MEM_OBJECT if memobj is not a valid memory object.
    return Memory::IsValid(memobj) ? CL_SUCCESS : CL_INVALID_MEM_OBJECT;
}

cl_int ValidateReleaseMemObject(cl_mem memobj)
{
    // CL_INVALID_MEM_OBJECT if memobj is not a valid memory object.
    return Memory::IsValid(memobj) ? CL_SUCCESS : CL_INVALID_MEM_OBJECT;
}

cl_int ValidateGetSupportedImageFormats(cl_context context,
                                        MemFlags flags,
                                        MemObjectType image_type,
                                        cl_uint num_entries,
                                        const cl_image_format *image_formats,
                                        const cl_uint *num_image_formats)
{
    // CL_INVALID_CONTEXT if context is not a valid context.
    if (!Context::IsValid(context))
    {
        return CL_INVALID_CONTEXT;
    }
    const Context &ctx = context->cast<Context>();

    // CL_INVALID_VALUE if flags or image_type are not valid,
    if (!ValidateMemoryFlags(flags, ctx.getPlatform()) || !Image::IsTypeValid(image_type))
    {
        return CL_INVALID_VALUE;
    }
    // or if num_entries is 0 and image_formats is not NULL.
    if (num_entries == 0u && image_formats != nullptr)
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateGetMemObjectInfo(cl_mem memobj,
                                MemInfo param_name,
                                size_t param_value_size,
                                const void *param_value,
                                const size_t *param_value_size_ret)
{
    // CL_INVALID_MEM_OBJECT if memobj is a not a valid memory object.
    if (!Memory::IsValid(memobj))
    {
        return CL_INVALID_MEM_OBJECT;
    }

    // CL_INVALID_VALUE if param_name is not valid.
    const cl_version version = memobj->cast<Memory>().getContext().getPlatform().getVersion();
    switch (param_name)
    {
        case MemInfo::AssociatedMemObject:
        case MemInfo::Offset:
            ANGLE_VALIDATE_VERSION(version, 1, 1);
            break;
        case MemInfo::UsesSVM_Pointer:
            ANGLE_VALIDATE_VERSION(version, 2, 0);
            break;
        case MemInfo::Properties:
            ANGLE_VALIDATE_VERSION(version, 3, 0);
            break;
        case MemInfo::InvalidEnum:
            return CL_INVALID_VALUE;
        default:
            // All remaining possible values for param_name are valid for all versions.
            break;
    }

    return CL_SUCCESS;
}

cl_int ValidateGetImageInfo(cl_mem image,
                            ImageInfo param_name,
                            size_t param_value_size,
                            const void *param_value,
                            const size_t *param_value_size_ret)
{
    // CL_INVALID_MEM_OBJECT if image is a not a valid image object.
    if (!Image::IsValid(image))
    {
        return CL_INVALID_MEM_OBJECT;
    }

    // CL_INVALID_VALUE if param_name is not valid.
    const cl_version version = image->cast<Image>().getContext().getPlatform().getVersion();
    switch (param_name)
    {
        case ImageInfo::ArraySize:
        case ImageInfo::Buffer:
        case ImageInfo::NumMipLevels:
        case ImageInfo::NumSamples:
            ANGLE_VALIDATE_VERSION(version, 1, 2);
            break;
        case ImageInfo::InvalidEnum:
            return CL_INVALID_VALUE;
        default:
            // All remaining possible values for param_name are valid for all versions.
            break;
    }

    return CL_SUCCESS;
}

cl_int ValidateRetainSampler(cl_sampler sampler)
{
    // CL_INVALID_SAMPLER if sampler is not a valid sampler object.
    return Sampler::IsValid(sampler) ? CL_SUCCESS : CL_INVALID_SAMPLER;
}

cl_int ValidateReleaseSampler(cl_sampler sampler)
{
    // CL_INVALID_SAMPLER if sampler is not a valid sampler object.
    return Sampler::IsValid(sampler) ? CL_SUCCESS : CL_INVALID_SAMPLER;
}

cl_int ValidateGetSamplerInfo(cl_sampler sampler,
                              SamplerInfo param_name,
                              size_t param_value_size,
                              const void *param_value,
                              const size_t *param_value_size_ret)
{
    // CL_INVALID_SAMPLER if sampler is a not a valid sampler object.
    if (!Sampler::IsValid(sampler))
    {
        return CL_INVALID_SAMPLER;
    }

    // CL_INVALID_VALUE if param_name is not valid.
    const cl_version version = sampler->cast<Sampler>().getContext().getPlatform().getVersion();
    switch (param_name)
    {
        case SamplerInfo::Properties:
            ANGLE_VALIDATE_VERSION(version, 3, 0);
            break;
        case SamplerInfo::InvalidEnum:
            return CL_INVALID_VALUE;
        default:
            // All remaining possible values for param_name are valid for all versions.
            break;
    }

    return CL_SUCCESS;
}

cl_int ValidateCreateProgramWithSource(cl_context context,
                                       cl_uint count,
                                       const char **strings,
                                       const size_t *lengths)
{
    // CL_INVALID_CONTEXT if context is not a valid context.
    if (!Context::IsValid(context))
    {
        return CL_INVALID_CONTEXT;
    }

    // CL_INVALID_VALUE if count is zero or if strings or any entry in strings is NULL.
    if (count == 0u || strings == nullptr)
    {
        return CL_INVALID_VALUE;
    }
    while (count-- != 0u)
    {
        if (*strings++ == nullptr)
        {
            return CL_INVALID_VALUE;
        }
    }

    return CL_SUCCESS;
}

cl_int ValidateCreateProgramWithBinary(cl_context context,
                                       cl_uint num_devices,
                                       const cl_device_id *device_list,
                                       const size_t *lengths,
                                       const unsigned char **binaries,
                                       const cl_int *binary_status)
{
    // CL_INVALID_CONTEXT if context is not a valid context.
    if (!Context::IsValid(context))
    {
        return CL_INVALID_CONTEXT;
    }
    const Context &ctx = context->cast<Context>();

    // CL_INVALID_VALUE if device_list is NULL or num_devices is zero.
    // CL_INVALID_VALUE if lengths or binaries is NULL.
    if (device_list == nullptr || num_devices == 0u || lengths == nullptr || binaries == nullptr)
    {
        return CL_INVALID_VALUE;
    }
    while (num_devices-- != 0u)
    {
        // CL_INVALID_DEVICE if any device in device_list
        // is not in the list of devices associated with context.
        if (!ctx.hasDevice(*device_list++))
        {
            return CL_INVALID_DEVICE;
        }

        // CL_INVALID_VALUE if any entry in lengths[i] is zero or binaries[i] is NULL.
        if (*lengths++ == 0u || *binaries++ == nullptr)
        {
            return CL_INVALID_VALUE;
        }
    }

    return CL_SUCCESS;
}

cl_int ValidateRetainProgram(cl_program program)
{
    // CL_INVALID_PROGRAM if program is not a valid program object.
    return Program::IsValid(program) ? CL_SUCCESS : CL_INVALID_PROGRAM;
}

cl_int ValidateReleaseProgram(cl_program program)
{
    // CL_INVALID_PROGRAM if program is not a valid program object.
    return Program::IsValid(program) ? CL_SUCCESS : CL_INVALID_PROGRAM;
}

cl_int ValidateBuildProgram(cl_program program,
                            cl_uint num_devices,
                            const cl_device_id *device_list,
                            const char *options,
                            void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
                            const void *user_data)
{
    // CL_INVALID_PROGRAM if program is not a valid program object.
    if (!Program::IsValid(program))
    {
        return CL_INVALID_PROGRAM;
    }
    const Program &prog = program->cast<Program>();

    // CL_INVALID_VALUE if device_list is NULL and num_devices is greater than zero,
    // or if device_list is not NULL and num_devices is zero.
    if ((device_list != nullptr) != (num_devices != 0u))
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_DEVICE if any device in device_list
    // is not in the list of devices associated with program.
    while (num_devices-- != 0u)
    {
        if (!prog.hasDevice(*device_list++))
        {
            return CL_INVALID_DEVICE;
        }
    }

    // CL_INVALID_VALUE if pfn_notify is NULL but user_data is not NULL.
    if (pfn_notify == nullptr && user_data != nullptr)
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_OPERATION if the build of a program executable for any of the devices listed
    // in device_list by a previous call to clBuildProgram for program has not completed.
    if (prog.isBuilding())
    {
        return CL_INVALID_OPERATION;
    }

    // CL_INVALID_OPERATION if there are kernel objects attached to program.
    if (prog.hasAttachedKernels())
    {
        return CL_INVALID_OPERATION;
    }

    // If program was created with clCreateProgramWithBinary and device does not have a valid
    // program binary loaded
    std::vector<size_t> binSizes{prog.getDevices().size()};
    std::vector<std::vector<unsigned char *>> bins{prog.getDevices().size()};
    if (IsError(prog.getInfo(ProgramInfo::BinarySizes, binSizes.size() * sizeof(size_t),
                             binSizes.data(), nullptr)))
    {
        return CL_INVALID_PROGRAM;
    }
    for (size_t i = 0; i < prog.getDevices().size(); ++i)
    {
        cl_program_binary_type binType;
        bins.at(i).resize(binSizes[i]);

        if (IsError(prog.getInfo(ProgramInfo::Binaries, sizeof(unsigned char *) * bins.size(),
                                 bins.data(), nullptr)))
        {
            return CL_INVALID_VALUE;
        }
        if (IsError(prog.getBuildInfo(prog.getDevices()[i]->getNative(),
                                      ProgramBuildInfo::BinaryType, sizeof(cl_program_binary_type),
                                      &binType, nullptr)))
        {
            return CL_INVALID_VALUE;
        }
        if ((binType != CL_PROGRAM_BINARY_TYPE_NONE) && bins[i].empty())
        {
            return CL_INVALID_BINARY;
        }
    }

    return CL_SUCCESS;
}

cl_int ValidateGetProgramInfo(cl_program program,
                              ProgramInfo param_name,
                              size_t param_value_size,
                              const void *param_value,
                              const size_t *param_value_size_ret)
{
    // CL_INVALID_PROGRAM if program is not a valid program object.
    if (!Program::IsValid(program))
    {
        return CL_INVALID_PROGRAM;
    }
    const Program &prog = program->cast<Program>();

    // CL_INVALID_VALUE if param_name is not valid.
    const cl_version version = prog.getContext().getPlatform().getVersion();
    switch (param_name)
    {
        case ProgramInfo::NumKernels:
        case ProgramInfo::KernelNames:
            ANGLE_VALIDATE_VERSION(version, 1, 2);
            break;
        case ProgramInfo::IL:
            ANGLE_VALIDATE_VERSION(version, 2, 1);
            break;
        case ProgramInfo::ScopeGlobalCtorsPresent:
        case ProgramInfo::ScopeGlobalDtorsPresent:
            ANGLE_VALIDATE_VERSION(version, 2, 2);
            break;
        case ProgramInfo::InvalidEnum:
            return CL_INVALID_VALUE;
        default:
            // All remaining possible values for param_name are valid for all versions.
            break;
    }

    // CL_INVALID_VALUE if size in bytes specified by param_value_size is < size of return type
    // as described in the Program Object Queries table and param_value is not NULL.
    if (param_value != nullptr)
    {
        size_t valueSizeRet = 0;
        if (IsError(prog.getInfo(param_name, 0, nullptr, &valueSizeRet)) ||
            param_value_size < valueSizeRet)
        {
            return CL_INVALID_VALUE;
        }
    }

    return CL_SUCCESS;
}

cl_int ValidateGetProgramBuildInfo(cl_program program,
                                   cl_device_id device,
                                   ProgramBuildInfo param_name,
                                   size_t param_value_size,
                                   const void *param_value,
                                   const size_t *param_value_size_ret)
{
    // CL_INVALID_PROGRAM if program is not a valid program object.
    if (!Program::IsValid(program))
    {
        return CL_INVALID_PROGRAM;
    }
    const Program &prog = program->cast<Program>();

    // CL_INVALID_DEVICE if device is not in the list of devices associated with program.
    if (!prog.hasDevice(device))
    {
        return CL_INVALID_DEVICE;
    }

    // CL_INVALID_VALUE if param_name is not valid.
    const cl_version version = prog.getContext().getPlatform().getVersion();
    switch (param_name)
    {
        case ProgramBuildInfo::BinaryType:
            ANGLE_VALIDATE_VERSION(version, 1, 2);
            break;
        case ProgramBuildInfo::GlobalVariableTotalSize:
            ANGLE_VALIDATE_VERSION(version, 2, 0);
            break;
        case ProgramBuildInfo::InvalidEnum:
            return CL_INVALID_VALUE;
        default:
            // All remaining possible values for param_name are valid for all versions.
            break;
    }

    // CL_INVALID_VALUE if size in bytes specified by param_value_size is < size of return type
    // as described in the Program Object Queries table and param_value is not NULL.
    if (param_value != nullptr)
    {
        size_t valueSizeRet = 0;
        if (IsError(prog.getBuildInfo(device, param_name, 0, nullptr, &valueSizeRet)) ||
            param_value_size < valueSizeRet)
        {
            return CL_INVALID_VALUE;
        }
    }

    return CL_SUCCESS;
}

cl_int ValidateCreateKernel(cl_program program, const char *kernel_name)
{
    // CL_INVALID_PROGRAM if program is not a valid program object.
    if (!Program::IsValid(program))
    {
        return CL_INVALID_PROGRAM;
    }
    cl::Program &prog = program->cast<cl::Program>();

    // CL_INVALID_VALUE if kernel_name is NULL.
    if (kernel_name == nullptr)
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_PROGRAM_EXECUTABLE if there is no successfully built executable for program.
    std::vector<cl_device_id> associatedDevices;
    size_t associatedDeviceCount = 0;
    bool isAnyDeviceProgramBuilt = false;
    if (IsError(prog.getInfo(ProgramInfo::Devices, 0, nullptr, &associatedDeviceCount)))
    {
        return CL_INVALID_PROGRAM;
    }
    associatedDevices.resize(associatedDeviceCount / sizeof(cl_device_id));
    if (IsError(prog.getInfo(ProgramInfo::Devices, associatedDeviceCount, associatedDevices.data(),
                             nullptr)))
    {
        return CL_INVALID_PROGRAM;
    }
    for (const cl_device_id &device : associatedDevices)
    {
        cl_build_status status = CL_BUILD_NONE;
        if (IsError(prog.getBuildInfo(device, ProgramBuildInfo::Status, sizeof(cl_build_status),
                                      &status, nullptr)))
        {
            return CL_INVALID_PROGRAM;
        }

        if (status == CL_BUILD_SUCCESS)
        {
            isAnyDeviceProgramBuilt = true;
            break;
        }
    }
    if (!isAnyDeviceProgramBuilt)
    {
        return CL_INVALID_PROGRAM_EXECUTABLE;
    }

    // CL_INVALID_KERNEL_NAME if kernel_name is not found in program.
    std::string kernelNames;
    size_t kernelNamesSize = 0;
    if (IsError(prog.getInfo(ProgramInfo::KernelNames, 0, nullptr, &kernelNamesSize)))
    {
        return CL_INVALID_PROGRAM;
    }
    kernelNames.resize(kernelNamesSize);
    if (IsError(
            prog.getInfo(ProgramInfo::KernelNames, kernelNamesSize, kernelNames.data(), nullptr)))
    {
        return CL_INVALID_PROGRAM;
    }
    std::vector<std::string> tokenizedKernelNames =
        angle::SplitString(kernelNames.c_str(), ";", angle::WhitespaceHandling::TRIM_WHITESPACE,
                           angle::SplitResult::SPLIT_WANT_NONEMPTY);
    if (std::find(tokenizedKernelNames.begin(), tokenizedKernelNames.end(), kernel_name) ==
        tokenizedKernelNames.end())
    {
        return CL_INVALID_KERNEL_NAME;
    }

    return CL_SUCCESS;
}

cl_int ValidateCreateKernelsInProgram(cl_program program,
                                      cl_uint num_kernels,
                                      const cl_kernel *kernels,
                                      const cl_uint *num_kernels_ret)
{
    // CL_INVALID_PROGRAM if program is not a valid program object.
    if (!Program::IsValid(program))
    {
        return CL_INVALID_PROGRAM;
    }

    // CL_INVALID_VALUE if kernels is not NULL and num_kernels is less than the number of kernels in
    // program.
    size_t kernelCount = 0;
    cl::Program &prog  = program->cast<cl::Program>();
    if (IsError(prog.getInfo(ProgramInfo::NumKernels, sizeof(size_t), &prog, nullptr)))
    {
        return CL_INVALID_PROGRAM;
    }
    if (kernels != nullptr && num_kernels < kernelCount)
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateRetainKernel(cl_kernel kernel)
{
    // CL_INVALID_KERNEL if kernel is not a valid kernel object.
    return Kernel::IsValid(kernel) ? CL_SUCCESS : CL_INVALID_KERNEL;
}

cl_int ValidateReleaseKernel(cl_kernel kernel)
{
    // CL_INVALID_KERNEL if kernel is not a valid kernel object.
    return Kernel::IsValid(kernel) ? CL_SUCCESS : CL_INVALID_KERNEL;
}

cl_int ValidateSetKernelArg(cl_kernel kernel,
                            cl_uint arg_index,
                            size_t arg_size,
                            const void *arg_value)
{
    // CL_INVALID_KERNEL if kernel is not a valid kernel object.
    if (!Kernel::IsValid(kernel))
    {
        return CL_INVALID_KERNEL;
    }
    const Kernel &krnl = kernel->cast<Kernel>();

    // CL_INVALID_ARG_INDEX if arg_index is not a valid argument index.
    if (arg_index >= krnl.getInfo().args.size())
    {
        return CL_INVALID_ARG_INDEX;
    }

    if (arg_size == sizeof(cl_mem) && arg_value != nullptr)
    {
        const std::string &typeName = krnl.getInfo().args[arg_index].typeName;

        // CL_INVALID_MEM_OBJECT for an argument declared to be a memory object
        // when the specified arg_value is not a valid memory object.
        if (typeName == "image1d_t")
        {
            const cl_mem image = *static_cast<const cl_mem *>(arg_value);
            if (!Image::IsValid(image) || image->cast<Image>().getType() != MemObjectType::Image1D)
            {
                return CL_INVALID_MEM_OBJECT;
            }
        }
        else if (typeName == "image2d_t")
        {
            const cl_mem image = *static_cast<const cl_mem *>(arg_value);
            if (!Image::IsValid(image) || image->cast<Image>().getType() != MemObjectType::Image2D)
            {
                return CL_INVALID_MEM_OBJECT;
            }
        }
        else if (typeName == "image3d_t")
        {
            const cl_mem image = *static_cast<const cl_mem *>(arg_value);
            if (!Image::IsValid(image) || image->cast<Image>().getType() != MemObjectType::Image3D)
            {
                return CL_INVALID_MEM_OBJECT;
            }
        }
        else if (typeName == "image1d_array_t")
        {
            const cl_mem image = *static_cast<const cl_mem *>(arg_value);
            if (!Image::IsValid(image) ||
                image->cast<Image>().getType() != MemObjectType::Image1D_Array)
            {
                return CL_INVALID_MEM_OBJECT;
            }
        }
        else if (typeName == "image2d_array_t")
        {
            const cl_mem image = *static_cast<const cl_mem *>(arg_value);
            if (!Image::IsValid(image) ||
                image->cast<Image>().getType() != MemObjectType::Image2D_Array)
            {
                return CL_INVALID_MEM_OBJECT;
            }
        }
        else if (typeName == "image1d_buffer_t")
        {
            const cl_mem image = *static_cast<const cl_mem *>(arg_value);
            if (!Image::IsValid(image) ||
                image->cast<Image>().getType() != MemObjectType::Image1D_Buffer)
            {
                return CL_INVALID_MEM_OBJECT;
            }
        }
        // CL_INVALID_SAMPLER for an argument declared to be of type sampler_t
        // when the specified arg_value is not a valid sampler object.
        else if (typeName == "sampler_t")
        {
            static_assert(sizeof(cl_mem) == sizeof(cl_sampler), "api object size check failed");
            if (!Sampler::IsValid(*static_cast<const cl_sampler *>(arg_value)))
            {
                return CL_INVALID_SAMPLER;
            }
        }
        // CL_INVALID_DEVICE_QUEUE for an argument declared to be of type queue_t
        // when the specified arg_value is not a valid device queue object.
        else if (typeName == "queue_t")
        {
            static_assert(sizeof(cl_mem) == sizeof(cl_command_queue),
                          "api object size check failed");
            const cl_command_queue queue = *static_cast<const cl_command_queue *>(arg_value);
            if (!CommandQueue::IsValid(queue) || !queue->cast<CommandQueue>().isOnDevice())
            {
                return CL_INVALID_DEVICE_QUEUE;
            }
        }
    }

    return CL_SUCCESS;
}

cl_int ValidateGetKernelInfo(cl_kernel kernel,
                             KernelInfo param_name,
                             size_t param_value_size,
                             const void *param_value,
                             const size_t *param_value_size_ret)
{
    // CL_INVALID_KERNEL if kernel is a not a valid kernel object.
    if (!Kernel::IsValid(kernel))
    {
        return CL_INVALID_KERNEL;
    }

    // CL_INVALID_VALUE if param_name is not valid.
    const cl_version version =
        kernel->cast<Kernel>().getProgram().getContext().getPlatform().getVersion();
    switch (param_name)
    {
        case KernelInfo::Attributes:
            ANGLE_VALIDATE_VERSION(version, 1, 2);
            break;
        case KernelInfo::InvalidEnum:
            return CL_INVALID_VALUE;
        default:
            // All remaining possible values for param_name are valid for all versions.
            break;
    }

    return CL_SUCCESS;
}

cl_int ValidateGetKernelWorkGroupInfo(cl_kernel kernel,
                                      cl_device_id device,
                                      KernelWorkGroupInfo param_name,
                                      size_t param_value_size,
                                      const void *param_value,
                                      const size_t *param_value_size_ret)
{
    // CL_INVALID_KERNEL if kernel is a not a valid kernel object.
    if (!Kernel::IsValid(kernel))
    {
        return CL_INVALID_KERNEL;
    }
    const Kernel &krnl = kernel->cast<Kernel>();

    const Device *dev = nullptr;
    if (device != nullptr)
    {
        // CL_INVALID_DEVICE if device is not in the list of devices associated with kernel ...
        if (krnl.getProgram().getContext().hasDevice(device))
        {
            dev = &device->cast<Device>();
        }
        else
        {
            return CL_INVALID_DEVICE;
        }
    }
    else
    {
        // or if device is NULL but there is more than one device associated with kernel.
        if (krnl.getProgram().getContext().getDevices().size() == 1u)
        {
            dev = krnl.getProgram().getContext().getDevices().front().get();
        }
        else
        {
            return CL_INVALID_DEVICE;
        }
    }

    // CL_INVALID_VALUE if param_name is not valid.
    const cl_version version = krnl.getProgram().getContext().getPlatform().getInfo().version;
    switch (param_name)
    {
        case KernelWorkGroupInfo::GlobalWorkSize:
            ANGLE_VALIDATE_VERSION(version, 1, 2);
            // CL_INVALID_VALUE if param_name is CL_KERNEL_GLOBAL_WORK_SIZE and
            // device is not a custom device and kernel is not a built-in kernel.
            if (!dev->supportsBuiltInKernel(krnl.getInfo().functionName))
            {
                return CL_INVALID_VALUE;
            }
            break;
        case KernelWorkGroupInfo::InvalidEnum:
            return CL_INVALID_VALUE;
        default:
            // All remaining possible values for param_name are valid for all versions.
            break;
    }

    return CL_SUCCESS;
}

cl_int ValidateWaitForEvents(cl_uint num_events, const cl_event *event_list)
{
    // CL_INVALID_VALUE if num_events is zero or event_list is NULL.
    if (num_events == 0u || event_list == nullptr)
    {
        return CL_INVALID_VALUE;
    }

    const Context *context = nullptr;
    while (num_events-- != 0u)
    {
        // CL_INVALID_EVENT if event objects specified in event_list are not valid event objects.
        if (!Event::IsValid(*event_list))
        {
            return CL_INVALID_EVENT;
        }

        // CL_INVALID_CONTEXT if events specified in event_list do not belong to the same context.
        const Context *eventContext = &(*event_list++)->cast<Event>().getContext();
        if (context == nullptr)
        {
            context = eventContext;
        }
        else if (context != eventContext)
        {
            return CL_INVALID_CONTEXT;
        }
    }

    return CL_SUCCESS;
}

cl_int ValidateGetEventInfo(cl_event event,
                            EventInfo param_name,
                            size_t param_value_size,
                            const void *param_value,
                            const size_t *param_value_size_ret)
{
    // CL_INVALID_EVENT if event is a not a valid event object.
    if (!Event::IsValid(event))
    {
        return CL_INVALID_EVENT;
    }

    // CL_INVALID_VALUE if param_name is not valid.
    const cl_version version = event->cast<Event>().getContext().getPlatform().getVersion();
    switch (param_name)
    {
        case EventInfo::Context:
            ANGLE_VALIDATE_VERSION(version, 1, 1);
            break;
        case EventInfo::InvalidEnum:
            return CL_INVALID_VALUE;
        default:
            // All remaining possible values for param_name are valid for all versions.
            break;
    }

    return CL_SUCCESS;
}

cl_int ValidateRetainEvent(cl_event event)
{
    // CL_INVALID_EVENT if event is not a valid event object.
    return Event::IsValid(event) ? CL_SUCCESS : CL_INVALID_EVENT;
}

cl_int ValidateReleaseEvent(cl_event event)
{
    // CL_INVALID_EVENT if event is not a valid event object.
    return Event::IsValid(event) ? CL_SUCCESS : CL_INVALID_EVENT;
}

cl_int ValidateGetEventProfilingInfo(cl_event event,
                                     ProfilingInfo param_name,
                                     size_t param_value_size,
                                     const void *param_value,
                                     const size_t *param_value_size_ret)
{
    // CL_INVALID_EVENT if event is a not a valid event object.
    if (!Event::IsValid(event))
    {
        return CL_INVALID_EVENT;
    }
    const Event &evt = event->cast<Event>();

    // CL_PROFILING_INFO_NOT_AVAILABLE if event is a user event object,
    if (evt.getCommandType() == CL_COMMAND_USER)
    {
        return CL_PROFILING_INFO_NOT_AVAILABLE;
    }
    // or if the CL_QUEUE_PROFILING_ENABLE flag is not set for the command-queue.
    if (evt.getCommandQueue()->getProperties().excludes(CL_QUEUE_PROFILING_ENABLE))
    {
        return CL_PROFILING_INFO_NOT_AVAILABLE;
    }

    // CL_INVALID_VALUE if param_name is not valid.
    const cl_version version = evt.getContext().getPlatform().getVersion();
    switch (param_name)
    {
        case ProfilingInfo::CommandComplete:
            ANGLE_VALIDATE_VERSION(version, 2, 0);
            break;
        case ProfilingInfo::InvalidEnum:
            return CL_INVALID_VALUE;
        default:
            // All remaining possible values for param_name are valid for all versions.
            break;
    }

    return CL_SUCCESS;
}

cl_int ValidateFlush(cl_command_queue command_queue)
{
    // CL_INVALID_COMMAND_QUEUE if command_queue is not a valid host command-queue.
    if (!CommandQueue::IsValid(command_queue) || !command_queue->cast<CommandQueue>().isOnHost())
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    return CL_SUCCESS;
}

cl_int ValidateFinish(cl_command_queue command_queue)
{
    // CL_INVALID_COMMAND_QUEUE if command_queue is not a valid host command-queue.
    if (!CommandQueue::IsValid(command_queue) || !command_queue->cast<CommandQueue>().isOnHost())
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    return CL_SUCCESS;
}

cl_int ValidateEnqueueReadBuffer(cl_command_queue command_queue,
                                 cl_mem buffer,
                                 cl_bool blocking_read,
                                 size_t offset,
                                 size_t size,
                                 const void *ptr,
                                 cl_uint num_events_in_wait_list,
                                 const cl_event *event_wait_list,
                                 const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, false,
                                                        num_events_in_wait_list, event_wait_list));
    ANGLE_VALIDATE(ValidateEnqueueBuffer(command_queue->cast<CommandQueue>(), buffer, true, false));

    // CL_INVALID_VALUE if the region being read or written specified
    // by (offset, size) is out of bounds or if ptr is a NULL value.
    if (!buffer->cast<Buffer>().isRegionValid(offset, size) || ptr == nullptr)
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateEnqueueWriteBuffer(cl_command_queue command_queue,
                                  cl_mem buffer,
                                  cl_bool blocking_write,
                                  size_t offset,
                                  size_t size,
                                  const void *ptr,
                                  cl_uint num_events_in_wait_list,
                                  const cl_event *event_wait_list,
                                  const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, false,
                                                        num_events_in_wait_list, event_wait_list));
    ANGLE_VALIDATE(ValidateEnqueueBuffer(command_queue->cast<CommandQueue>(), buffer, false, true));

    // CL_INVALID_VALUE if the region being read or written specified
    // by (offset, size) is out of bounds or if ptr is a NULL value.
    if (!buffer->cast<Buffer>().isRegionValid(offset, size) || ptr == nullptr)
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateEnqueueCopyBuffer(cl_command_queue command_queue,
                                 cl_mem src_buffer,
                                 cl_mem dst_buffer,
                                 size_t src_offset,
                                 size_t dst_offset,
                                 size_t size,
                                 cl_uint num_events_in_wait_list,
                                 const cl_event *event_wait_list,
                                 const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, false,
                                                        num_events_in_wait_list, event_wait_list));
    const CommandQueue &queue = command_queue->cast<CommandQueue>();

    ANGLE_VALIDATE(ValidateEnqueueBuffer(queue, src_buffer, false, false));
    const Buffer &src = src_buffer->cast<Buffer>();

    ANGLE_VALIDATE(ValidateEnqueueBuffer(queue, dst_buffer, false, false));
    const Buffer &dst = dst_buffer->cast<Buffer>();

    // CL_INVALID_VALUE if src_offset, dst_offset, size, src_offset + size or dst_offset + size
    // require accessing elements outside the src_buffer and dst_buffer buffer objects respectively.
    if (!src.isRegionValid(src_offset, size) || !dst.isRegionValid(dst_offset, size))
    {
        return CL_INVALID_VALUE;
    }

    // CL_MEM_COPY_OVERLAP if src_buffer and dst_buffer are the same buffer or sub-buffer object
    // and the source and destination regions overlap or if src_buffer and dst_buffer are
    // different sub-buffers of the same associated buffer object and they overlap.
    if ((src.isSubBuffer() ? src.getParent().get() : &src) ==
        (dst.isSubBuffer() ? dst.getParent().get() : &dst))
    {
        // Only sub-buffers have offsets larger than zero
        src_offset += src.getOffset();
        dst_offset += dst.getOffset();

        if (OverlapRegions(src_offset, dst_offset, size))
        {
            return CL_MEM_COPY_OVERLAP;
        }
    }

    return CL_SUCCESS;
}

cl_int ValidateEnqueueReadImage(cl_command_queue command_queue,
                                cl_mem image,
                                cl_bool blocking_read,
                                const size_t *origin,
                                const size_t *region,
                                size_t row_pitch,
                                size_t slice_pitch,
                                const void *ptr,
                                cl_uint num_events_in_wait_list,
                                const cl_event *event_wait_list,
                                const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, true,
                                                        num_events_in_wait_list, event_wait_list));
    const CommandQueue &queue = command_queue->cast<CommandQueue>();

    ANGLE_VALIDATE(ValidateEnqueueImage(queue, image, true, false));
    const Image &img = image->cast<Image>();

    ANGLE_VALIDATE(ValidateImageForDevice(img, queue.getDevice(), origin, region));
    ANGLE_VALIDATE(ValidateHostRegionForImage(img, region, row_pitch, slice_pitch, ptr));

    return CL_SUCCESS;
}

cl_int ValidateEnqueueWriteImage(cl_command_queue command_queue,
                                 cl_mem image,
                                 cl_bool blocking_write,
                                 const size_t *origin,
                                 const size_t *region,
                                 size_t input_row_pitch,
                                 size_t input_slice_pitch,
                                 const void *ptr,
                                 cl_uint num_events_in_wait_list,
                                 const cl_event *event_wait_list,
                                 const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, true,
                                                        num_events_in_wait_list, event_wait_list));
    const CommandQueue &queue = command_queue->cast<CommandQueue>();

    ANGLE_VALIDATE(ValidateEnqueueImage(queue, image, false, true));
    const Image &img = image->cast<Image>();

    ANGLE_VALIDATE(ValidateImageForDevice(img, queue.getDevice(), origin, region));
    ANGLE_VALIDATE(
        ValidateHostRegionForImage(img, region, input_row_pitch, input_slice_pitch, ptr));

    return CL_SUCCESS;
}

cl_int ValidateEnqueueCopyImage(cl_command_queue command_queue,
                                cl_mem src_image,
                                cl_mem dst_image,
                                const size_t *src_origin,
                                const size_t *dst_origin,
                                const size_t *region,
                                cl_uint num_events_in_wait_list,
                                const cl_event *event_wait_list,
                                const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, true,
                                                        num_events_in_wait_list, event_wait_list));
    const CommandQueue &queue = command_queue->cast<CommandQueue>();

    ANGLE_VALIDATE(ValidateEnqueueImage(queue, src_image, false, false));
    const Image &src = src_image->cast<Image>();

    ANGLE_VALIDATE(ValidateEnqueueImage(queue, dst_image, false, false));
    const Image &dst = dst_image->cast<Image>();

    // CL_IMAGE_FORMAT_MISMATCH if src_image and dst_image do not use the same image format.
    if (src.getFormat().image_channel_order != dst.getFormat().image_channel_order ||
        src.getFormat().image_channel_data_type != dst.getFormat().image_channel_data_type)
    {
        return CL_IMAGE_FORMAT_MISMATCH;
    }

    ANGLE_VALIDATE(ValidateImageForDevice(src, queue.getDevice(), src_origin, region));
    ANGLE_VALIDATE(ValidateImageForDevice(dst, queue.getDevice(), dst_origin, region));

    // CL_MEM_COPY_OVERLAP if src_image and dst_image are the same image object
    // and the source and destination regions overlap.
    if (&src == &dst)
    {
        const MemObjectType type = src.getType();
        // Check overlap in first dimension
        if (OverlapRegions(src_origin[0], dst_origin[0], region[0]))
        {
            if (type == MemObjectType::Image1D || type == MemObjectType::Image1D_Buffer)
            {
                return CL_MEM_COPY_OVERLAP;
            }

            // Check overlap in second dimension
            if (OverlapRegions(src_origin[1], dst_origin[1], region[1]))
            {
                if (type == MemObjectType::Image2D || type == MemObjectType::Image1D_Array)
                {
                    return CL_MEM_COPY_OVERLAP;
                }

                // Check overlap in third dimension
                if (OverlapRegions(src_origin[2], dst_origin[2], region[2]))
                {
                    return CL_MEM_COPY_OVERLAP;
                }
            }
        }
    }

    return CL_SUCCESS;
}

cl_int ValidateEnqueueCopyImageToBuffer(cl_command_queue command_queue,
                                        cl_mem src_image,
                                        cl_mem dst_buffer,
                                        const size_t *src_origin,
                                        const size_t *region,
                                        size_t dst_offset,
                                        cl_uint num_events_in_wait_list,
                                        const cl_event *event_wait_list,
                                        const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, true,
                                                        num_events_in_wait_list, event_wait_list));
    const CommandQueue &queue = command_queue->cast<CommandQueue>();

    ANGLE_VALIDATE(ValidateEnqueueImage(queue, src_image, false, false));
    const Image &src = src_image->cast<Image>();

    ANGLE_VALIDATE(ValidateEnqueueBuffer(queue, dst_buffer, false, false));
    const Buffer &dst = dst_buffer->cast<Buffer>();

    // CL_INVALID_MEM_OBJECT if src_image is a 1D image buffer object created from dst_buffer.
    if (src.getType() == MemObjectType::Image1D_Buffer && src.getParent() == &dst)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    ANGLE_VALIDATE(ValidateImageForDevice(src, queue.getDevice(), src_origin, region));

    // CL_INVALID_VALUE if the region specified by dst_offset and dst_offset + dst_cb
    // refer to a region outside dst_buffer.
    const MemObjectType type = src.getType();
    size_t dst_cb            = src.getElementSize() * region[0];
    if (type != MemObjectType::Image1D && type != MemObjectType::Image1D_Buffer)
    {
        dst_cb *= region[1];
        if (type != MemObjectType::Image2D && type != MemObjectType::Image1D_Array)
        {
            dst_cb *= region[2];
        }
    }
    if (!dst.isRegionValid(dst_offset, dst_cb))
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateEnqueueCopyBufferToImage(cl_command_queue command_queue,
                                        cl_mem src_buffer,
                                        cl_mem dst_image,
                                        size_t src_offset,
                                        const size_t *dst_origin,
                                        const size_t *region,
                                        cl_uint num_events_in_wait_list,
                                        const cl_event *event_wait_list,
                                        const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, true,
                                                        num_events_in_wait_list, event_wait_list));
    const CommandQueue &queue = command_queue->cast<CommandQueue>();

    ANGLE_VALIDATE(ValidateEnqueueBuffer(queue, src_buffer, false, false));
    const Buffer &src = src_buffer->cast<Buffer>();

    ANGLE_VALIDATE(ValidateEnqueueImage(queue, dst_image, false, false));
    const Image &dst = dst_image->cast<Image>();

    // CL_INVALID_MEM_OBJECT if dst_image is a 1D image buffer object created from src_buffer.
    if (dst.getType() == MemObjectType::Image1D_Buffer && dst.getParent() == &src)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    ANGLE_VALIDATE(ValidateImageForDevice(dst, queue.getDevice(), dst_origin, region));

    // CL_INVALID_VALUE if the region specified by src_offset and src_offset + src_cb
    // refer to a region outside src_buffer.
    const MemObjectType type = dst.getType();
    size_t src_cb            = dst.getElementSize() * region[0];
    if (type != MemObjectType::Image1D && type != MemObjectType::Image1D_Buffer)
    {
        src_cb *= region[1];
        if (type != MemObjectType::Image2D && type != MemObjectType::Image1D_Array)
        {
            src_cb *= region[2];
        }
    }
    if (!src.isRegionValid(src_offset, src_cb))
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateEnqueueMapBuffer(cl_command_queue command_queue,
                                cl_mem buffer,
                                cl_bool blocking_map,
                                MapFlags map_flags,
                                size_t offset,
                                size_t size,
                                cl_uint num_events_in_wait_list,
                                const cl_event *event_wait_list,
                                const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, false,
                                                        num_events_in_wait_list, event_wait_list));
    const CommandQueue &queue = command_queue->cast<CommandQueue>();

    // CL_INVALID_OPERATION if buffer has been created with CL_MEM_HOST_WRITE_ONLY or
    // CL_MEM_HOST_NO_ACCESS and CL_MAP_READ is set in map_flags
    // or if buffer has been created with CL_MEM_HOST_READ_ONLY or CL_MEM_HOST_NO_ACCESS
    // and CL_MAP_WRITE or CL_MAP_WRITE_INVALIDATE_REGION is set in map_flags.
    ANGLE_VALIDATE(
        ValidateEnqueueBuffer(queue, buffer, map_flags.intersects(CL_MAP_READ),
                              map_flags.intersects(CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION)));

    // CL_INVALID_VALUE if region being mapped given by (offset, size) is out of bounds
    // or if size is 0 or if values specified in map_flags are not valid.
    if (!buffer->cast<Buffer>().isRegionValid(offset, size) || size == 0u ||
        !ValidateMapFlags(map_flags, queue.getContext().getPlatform()))
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateEnqueueMapImage(cl_command_queue command_queue,
                               cl_mem image,
                               cl_bool blocking_map,
                               MapFlags map_flags,
                               const size_t *origin,
                               const size_t *region,
                               const size_t *image_row_pitch,
                               const size_t *image_slice_pitch,
                               cl_uint num_events_in_wait_list,
                               const cl_event *event_wait_list,
                               const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, true,
                                                        num_events_in_wait_list, event_wait_list));
    const CommandQueue &queue = command_queue->cast<CommandQueue>();

    // CL_INVALID_OPERATION if image has been created with CL_MEM_HOST_WRITE_ONLY or
    // CL_MEM_HOST_NO_ACCESS and CL_MAP_READ is set in map_flags
    // or if image has been created with CL_MEM_HOST_READ_ONLY or CL_MEM_HOST_NO_ACCESS
    // and CL_MAP_WRITE or CL_MAP_WRITE_INVALIDATE_REGION is set in map_flags.
    ANGLE_VALIDATE(
        ValidateEnqueueImage(queue, image, map_flags.intersects(CL_MAP_READ),
                             map_flags.intersects(CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION)));
    const Image &img = image->cast<Image>();

    ANGLE_VALIDATE(ValidateImageForDevice(img, queue.getDevice(), origin, region));

    // CL_INVALID_VALUE if values specified in map_flags are not valid.
    if (!ValidateMapFlags(map_flags, queue.getContext().getPlatform()))
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_VALUE if image_row_pitch is NULL.
    if (image_row_pitch == nullptr)
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_VALUE if image is a 3D image, 1D or 2D image array object
    // and image_slice_pitch is NULL.
    if ((img.getType() == MemObjectType::Image3D || img.getType() == MemObjectType::Image1D_Array ||
         img.getType() == MemObjectType::Image2D_Array) &&
        image_slice_pitch == nullptr)
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateEnqueueUnmapMemObject(cl_command_queue command_queue,
                                     cl_mem memobj,
                                     const void *mapped_ptr,
                                     cl_uint num_events_in_wait_list,
                                     const cl_event *event_wait_list,
                                     const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, false,
                                                        num_events_in_wait_list, event_wait_list));
    const CommandQueue &queue = command_queue->cast<CommandQueue>();

    // CL_INVALID_MEM_OBJECT if memobj is not a valid memory object or is a pipe object.
    if (!Memory::IsValid(memobj))
    {
        return CL_INVALID_MEM_OBJECT;
    }
    const Memory &memory = memobj->cast<Memory>();
    if (memory.getType() == MemObjectType::Pipe)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    // CL_INVALID_CONTEXT if context associated with command_queue and memobj are not the same.
    if (&queue.getContext() != &memory.getContext())
    {
        return CL_INVALID_CONTEXT;
    }

    return CL_SUCCESS;
}

cl_int ValidateEnqueueNDRangeKernel(cl_command_queue command_queue,
                                    cl_kernel kernel,
                                    cl_uint work_dim,
                                    const size_t *global_work_offset,
                                    const size_t *global_work_size,
                                    const size_t *local_work_size,
                                    cl_uint num_events_in_wait_list,
                                    const cl_event *event_wait_list,
                                    const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, false,
                                                        num_events_in_wait_list, event_wait_list));
    const CommandQueue &queue = command_queue->cast<CommandQueue>();
    const Device &device      = queue.getDevice();

    // CL_INVALID_KERNEL if kernel is not a valid kernel object.
    if (!Kernel::IsValid(kernel))
    {
        return CL_INVALID_KERNEL;
    }
    const Kernel &krnl = kernel->cast<Kernel>();

    // CL_INVALID_CONTEXT if context associated with command_queue and kernel are not the same.
    if (&queue.getContext() != &krnl.getProgram().getContext())
    {
        return CL_INVALID_CONTEXT;
    }

    // CL_INVALID_WORK_DIMENSION if work_dim is not a valid value.
    if (work_dim == 0u || work_dim > device.getInfo().maxWorkItemSizes.size())
    {
        return CL_INVALID_WORK_DIMENSION;
    }

    // CL_INVALID_GLOBAL_OFFSET if global_work_offset is non-NULL before version 1.1.
    if (!queue.getContext().getPlatform().isVersionOrNewer(1u, 1u) && global_work_offset != nullptr)
    {
        return CL_INVALID_GLOBAL_OFFSET;
    }

    // CL_INVALID_KERNEL_ARGS if all the kernel arguments have not been set for the kernel
    if (!krnl.areAllArgsSet())
    {
        return CL_INVALID_KERNEL_ARGS;
    }

    size_t compileWorkGroupSize[3] = {0, 0, 0};
    if (IsError(krnl.getWorkGroupInfo(const_cast<cl_device_id>(device.getNative()),
                                      KernelWorkGroupInfo::CompileWorkGroupSize,
                                      sizeof(compileWorkGroupSize), compileWorkGroupSize, nullptr)))
    {
        return CL_INVALID_VALUE;
    }
    if (local_work_size != nullptr)
    {
        // CL_INVALID_WORK_GROUP_SIZE when non-uniform work-groups are not supported, the size of
        // each work-group must be uniform. If local_work_size is specified, the values specified in
        // global_work_size[0],...,global_work_size[work_dim - 1] must be evenly divisible by
        // the corresponding values specified in local_work_size[0],...,
        // local_work_size[work_dim-1].
        if (!device.supportsNonUniformWorkGroups())
        {
            for (cl_uint i = 0; i < work_dim; ++i)
            {
                if (global_work_size[i] % local_work_size[i] != 0)
                {
                    return CL_INVALID_WORK_GROUP_SIZE;
                }
            }
        }

        for (cl_uint i = 0; i < work_dim; ++i)
        {
            // CL_INVALID_WORK_GROUP_SIZE when non-uniform work-groups are not supported, the size
            // of each work-group must be uniform. If local_work_size is specified, the values
            // specified in global_work_size[0],..., global_work_size[work_dim - 1] must be
            // evenly divisible by the corresponding values specified in local_work_size[0],...,
            // local_work_size[work_dim-1].
            if (local_work_size[i] == 0)
            {
                return CL_INVALID_WORK_GROUP_SIZE;
            }

            // CL_INVALID_WORK_GROUP_SIZE if local_work_size is specified and does not match the
            // required work-group size for kernel in the program source.
            if (compileWorkGroupSize[i] != 0 && local_work_size[i] != compileWorkGroupSize[i])
            {
                return CL_INVALID_WORK_GROUP_SIZE;
            }
        }
    }

    // CL_INVALID_GLOBAL_WORK_SIZE if global_work_size is NULL or if any of the values
    // specified in global_work_size[0] ... global_work_size[work_dim - 1] are 0.
    // Returning this error code under these circumstances is deprecated by version 2.1.
    if (!queue.getContext().getPlatform().isVersionOrNewer(2u, 1u))
    {
        if (global_work_size == nullptr)
        {
            return CL_INVALID_GLOBAL_WORK_SIZE;
        }
        for (cl_uint dim = 0u; dim < work_dim; ++dim)
        {
            if (global_work_size[dim] == 0u)
            {
                return CL_INVALID_GLOBAL_WORK_SIZE;
            }
        }
    }

    // CL_INVALID_GLOBAL_WORK_SIZE if any of the values specified in global_work_size[0], ...
    // global_work_size[work_dim - 1] exceed the maximum value representable by size_t on the device
    // on which the kernel-instance will be enqueued.
    if (global_work_size != nullptr)
    {
        for (cl_uint dim = 0u; dim < work_dim; ++dim)
        {
            if (global_work_size[dim] > UINT32_MAX)
            {
                // Set hard limit in ANGLE to 2^32 for all backends (regardless of device support).
                return CL_INVALID_GLOBAL_WORK_SIZE;
            }
        }
    }

    // CL_INVALID_GLOBAL_OFFSET if the value specified in global_work_size + the corresponding
    // values in global_work_offset for any dimensions is greater than the maximum value
    // representable by size t on the device on which the kernel-instance will be enqueued
    if (global_work_offset != nullptr)
    {
        for (cl_uint dim = 0u; dim < work_dim; ++dim)
        {
            if (static_cast<uint32_t>((global_work_offset[dim] + global_work_size[dim])) <
                global_work_offset[dim])
            {
                // Set hard limit in ANGLE to 2^32 for all backends (regardless of device support).
                return CL_INVALID_GLOBAL_OFFSET;
            }
        }
    }

    if (local_work_size != nullptr)
    {
        size_t numWorkItems = 1u;  // Initialize with neutral element for multiplication

        // CL_INVALID_WORK_ITEM_SIZE if the number of work-items specified
        // in any of local_work_size[0] ... local_work_size[work_dim - 1]
        // is greater than the corresponding values specified by
        // CL_DEVICE_MAX_WORK_ITEM_SIZES[0] ... CL_DEVICE_MAX_WORK_ITEM_SIZES[work_dim - 1].
        for (cl_uint dim = 0u; dim < work_dim; ++dim)
        {
            if (local_work_size[dim] > device.getInfo().maxWorkItemSizes[dim])
            {
                return CL_INVALID_WORK_ITEM_SIZE;
            }
            numWorkItems *= local_work_size[dim];
        }

        // CL_INVALID_WORK_GROUP_SIZE if local_work_size is specified
        // and the total number of work-items in the work-group computed as
        // local_work_size[0] x ... local_work_size[work_dim - 1] is greater than the value
        // specified by CL_KERNEL_WORK_GROUP_SIZE in the Kernel Object Device Queries table.
        if (numWorkItems > krnl.getInfo().workGroups[queue.getDeviceIndex()].workGroupSize)
        {
            return CL_INVALID_WORK_GROUP_SIZE;
        }
    }

    return CL_SUCCESS;
}

cl_int ValidateEnqueueNativeKernel(cl_command_queue command_queue,
                                   void(CL_CALLBACK *user_func)(void *),
                                   const void *args,
                                   size_t cb_args,
                                   cl_uint num_mem_objects,
                                   const cl_mem *mem_list,
                                   const void **args_mem_loc,
                                   cl_uint num_events_in_wait_list,
                                   const cl_event *event_wait_list,
                                   const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, false,
                                                        num_events_in_wait_list, event_wait_list));
    const CommandQueue &queue = command_queue->cast<CommandQueue>();

    // CL_INVALID_OPERATION if the device associated with command_queue
    // cannot execute the native kernel.
    if (queue.getDevice().getInfo().execCapabilities.excludes(CL_EXEC_NATIVE_KERNEL))
    {
        return CL_INVALID_OPERATION;
    }

    // CL_INVALID_VALUE if user_func is NULL.
    if (user_func == nullptr)
    {
        return CL_INVALID_VALUE;
    }

    if (args == nullptr)
    {
        // CL_INVALID_VALUE if args is a NULL value and cb_args > 0 or num_mem_objects > 0.
        if (cb_args > 0u || num_mem_objects > 0u)
        {
            return CL_INVALID_VALUE;
        }
    }
    else
    {
        // CL_INVALID_VALUE if args is not NULL and cb_args is 0.
        if (cb_args == 0u)
        {
            return CL_INVALID_VALUE;
        }
    }

    if (num_mem_objects == 0u)
    {
        // CL_INVALID_VALUE if num_mem_objects = 0 and mem_list or args_mem_loc are not NULL.
        if (mem_list != nullptr || args_mem_loc != nullptr)
        {
            return CL_INVALID_VALUE;
        }
    }
    else
    {
        // CL_INVALID_VALUE if num_mem_objects > 0 and mem_list or args_mem_loc are NULL.
        if (mem_list == nullptr || args_mem_loc == nullptr)
        {
            return CL_INVALID_VALUE;
        }

        // CL_INVALID_MEM_OBJECT if one or more memory objects
        // specified in mem_list are not valid or are not buffer objects.
        while (num_mem_objects-- != 0u)
        {
            if (!Buffer::IsValid(*mem_list++))
            {
                return CL_INVALID_MEM_OBJECT;
            }
        }
    }

    return CL_SUCCESS;
}

cl_int ValidateSetCommandQueueProperty(cl_command_queue command_queue,
                                       CommandQueueProperties properties,
                                       cl_bool enable,
                                       const cl_command_queue_properties *old_properties)
{
    // CL_INVALID_COMMAND_QUEUE if command_queue is not a valid command-queue.
    if (!CommandQueue::IsValid(command_queue))
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    // CL_INVALID_VALUE if values specified in properties are not valid.
    if (properties.hasOtherBitsThan(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
                                    CL_QUEUE_PROFILING_ENABLE))
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateCreateImage2D(cl_context context,
                             MemFlags flags,
                             const cl_image_format *image_format,
                             size_t image_width,
                             size_t image_height,
                             size_t image_row_pitch,
                             const void *host_ptr)
{
    const cl_image_desc desc = {CL_MEM_OBJECT_IMAGE2D, image_width, image_height, 0u, 0u,
                                image_row_pitch,       0u,          0u,           0u, {nullptr}};
    return ValidateCreateImage(context, flags, image_format, &desc, host_ptr);
}

cl_int ValidateCreateImage3D(cl_context context,
                             MemFlags flags,
                             const cl_image_format *image_format,
                             size_t image_width,
                             size_t image_height,
                             size_t image_depth,
                             size_t image_row_pitch,
                             size_t image_slice_pitch,
                             const void *host_ptr)
{
    const cl_image_desc desc = {
        CL_MEM_OBJECT_IMAGE3D, image_width,       image_height, image_depth, 0u,
        image_row_pitch,       image_slice_pitch, 0u,           0u,          {nullptr}};
    return ValidateCreateImage(context, flags, image_format, &desc, host_ptr);
}

cl_int ValidateEnqueueMarker(cl_command_queue command_queue, const cl_event *event)
{
    // CL_INVALID_COMMAND_QUEUE if command_queue is not a valid host command-queue.
    if (!CommandQueue::IsValid(command_queue) || !command_queue->cast<CommandQueue>().isOnHost())
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    // CL_INVALID_VALUE if event is NULL.
    if (event == nullptr)
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateEnqueueWaitForEvents(cl_command_queue command_queue,
                                    cl_uint num_events,
                                    const cl_event *event_list)
{
    // CL_INVALID_COMMAND_QUEUE if command_queue is not a valid host command-queue.
    if (!CommandQueue::IsValid(command_queue))
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    const CommandQueue &queue = command_queue->cast<CommandQueue>();
    if (!queue.isOnHost())
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    // CL_INVALID_VALUE if num_events is 0 or event_list is NULL.
    if (num_events == 0u || event_list == nullptr)
    {
        return CL_INVALID_VALUE;
    }

    while (num_events-- != 0u)
    {
        // The documentation for invalid events is missing.
        if (!Event::IsValid(*event_list))
        {
            return CL_INVALID_VALUE;
        }

        // CL_INVALID_CONTEXT if context associated with command_queue
        // and events in event_list are not the same.
        if (&queue.getContext() != &(*event_list++)->cast<Event>().getContext())
        {
            return CL_INVALID_CONTEXT;
        }
    }

    return CL_SUCCESS;
}

cl_int ValidateEnqueueBarrier(cl_command_queue command_queue)
{
    // CL_INVALID_COMMAND_QUEUE if command_queue is not a valid host command-queue.
    if (!CommandQueue::IsValid(command_queue) || !command_queue->cast<CommandQueue>().isOnHost())
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    return CL_SUCCESS;
}

cl_int ValidateUnloadCompiler()
{
    return CL_SUCCESS;
}

cl_int ValidateGetExtensionFunctionAddress(const char *func_name)
{
    return func_name != nullptr && *func_name != '\0' ? CL_SUCCESS : CL_INVALID_VALUE;
}

cl_int ValidateCreateCommandQueue(cl_context context,
                                  cl_device_id device,
                                  CommandQueueProperties properties)
{
    // CL_INVALID_CONTEXT if context is not a valid context.
    if (!Context::IsValid(context))
    {
        return CL_INVALID_CONTEXT;
    }

    // CL_INVALID_DEVICE if device is not a valid device or is not associated with context.
    if (!context->cast<Context>().hasDevice(device))
    {
        return CL_INVALID_DEVICE;
    }

    // CL_INVALID_VALUE if values specified in properties are not valid.
    if (properties.hasOtherBitsThan(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
                                    CL_QUEUE_PROFILING_ENABLE))
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateCreateSampler(cl_context context,
                             cl_bool normalized_coords,
                             AddressingMode addressing_mode,
                             FilterMode filter_mode)
{
    // CL_INVALID_CONTEXT if context is not a valid context.
    if (!Context::IsValid(context))
    {
        return CL_INVALID_CONTEXT;
    }

    // CL_INVALID_VALUE if addressing_mode, filter_mode, normalized_coords
    // or a combination of these arguements are not valid.
    if ((normalized_coords != CL_FALSE && normalized_coords != CL_TRUE) ||
        addressing_mode == AddressingMode::InvalidEnum || filter_mode == FilterMode::InvalidEnum)
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_OPERATION if images are not supported by any device associated with context.
    if (!context->cast<Context>().supportsImages())
    {
        return CL_INVALID_OPERATION;
    }

    return CL_SUCCESS;
}

cl_int ValidateEnqueueTask(cl_command_queue command_queue,
                           cl_kernel kernel,
                           cl_uint num_events_in_wait_list,
                           const cl_event *event_wait_list,
                           const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, false,
                                                        num_events_in_wait_list, event_wait_list));

    // CL_INVALID_KERNEL if kernel is not a valid kernel object.
    if (!Kernel::IsValid(kernel))
    {
        return CL_INVALID_KERNEL;
    }

    // CL_INVALID_CONTEXT if context associated with command_queue and kernel are not the same.
    if (&command_queue->cast<CommandQueue>().getContext() !=
        &kernel->cast<Kernel>().getProgram().getContext())
    {
        return CL_INVALID_CONTEXT;
    }

    return CL_SUCCESS;
}

// CL 1.1
cl_int ValidateCreateSubBuffer(cl_mem buffer,
                               MemFlags flags,
                               cl_buffer_create_type buffer_create_type,
                               const void *buffer_create_info)
{
    // CL_INVALID_MEM_OBJECT if buffer is not a valid buffer object or is a sub-buffer object.
    if (!Buffer::IsValid(buffer))
    {
        return CL_INVALID_MEM_OBJECT;
    }
    const Buffer &buf = buffer->cast<Buffer>();
    if (buf.isSubBuffer() || !buf.getContext().getPlatform().isVersionOrNewer(1u, 1u))
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (!ValidateMemoryFlags(flags, buf.getContext().getPlatform()))
    {
        return CL_INVALID_VALUE;
    }

    const MemFlags bufFlags = buf.getFlags();
    // CL_INVALID_VALUE if buffer was created with CL_MEM_WRITE_ONLY
    // and flags specifies CL_MEM_READ_WRITE or CL_MEM_READ_ONLY,
    if ((bufFlags.intersects(CL_MEM_WRITE_ONLY) &&
         flags.intersects(CL_MEM_READ_WRITE | CL_MEM_READ_ONLY)) ||
        // or if buffer was created with CL_MEM_READ_ONLY
        // and flags specifies CL_MEM_READ_WRITE or CL_MEM_WRITE_ONLY,
        (bufFlags.intersects(CL_MEM_READ_ONLY) &&
         flags.intersects(CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY)) ||
        // or if flags specifies CL_MEM_USE_HOST_PTR, CL_MEM_ALLOC_HOST_PTR or CL_MEM_COPY_HOST_PTR.
        flags.intersects(CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR))
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_VALUE if buffer was created with CL_MEM_HOST_WRITE_ONLY
    // and flags specify CL_MEM_HOST_READ_ONLY,
    if ((bufFlags.intersects(CL_MEM_HOST_WRITE_ONLY) && flags.intersects(CL_MEM_HOST_READ_ONLY)) ||
        // or if buffer was created with CL_MEM_HOST_READ_ONLY
        // and flags specify CL_MEM_HOST_WRITE_ONLY,
        (bufFlags.intersects(CL_MEM_HOST_READ_ONLY) && flags.intersects(CL_MEM_HOST_WRITE_ONLY)) ||
        // or if buffer was created with CL_MEM_HOST_NO_ACCESS
        // and flags specify CL_MEM_HOST_READ_ONLY or CL_MEM_HOST_WRITE_ONLY.
        (bufFlags.intersects(CL_MEM_HOST_NO_ACCESS) &&
         flags.intersects(CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY)))
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_VALUE if the value specified in buffer_create_type is not valid.
    if (buffer_create_type != CL_BUFFER_CREATE_TYPE_REGION)
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_VALUE if value(s) specified in buffer_create_info
    // (for a given buffer_create_type) is not valid or if buffer_create_info is NULL.
    // CL_INVALID_VALUE if the region specified by the cl_buffer_region structure
    // passed in buffer_create_info is out of bounds in buffer.
    const cl_buffer_region *region = static_cast<const cl_buffer_region *>(buffer_create_info);
    if (region == nullptr || !buf.isRegionValid(*region))
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_BUFFER_SIZE if the size field of the cl_buffer_region structure
    // passed in buffer_create_info is 0.
    if (region->size == 0u)
    {
        return CL_INVALID_BUFFER_SIZE;
    }

    // CL_MISALIGNED_SUB_BUFFER_OFFSET when the sub-buffer object is created with an offset that is
    // not aligned to CL_DEVICE_MEM_BASE_ADDR_ALIGN value (which is in bits!) for devices associated
    // with the context.
    const Memory &memory = buffer->cast<Memory>();
    for (const DevicePtr &device : memory.getContext().getDevices())
    {
        if (region->origin % (device->getInfo().memBaseAddrAlign / CHAR_BIT) != 0)
        {
            return CL_MISALIGNED_SUB_BUFFER_OFFSET;
        }
    }

    return CL_SUCCESS;
}

cl_int ValidateSetMemObjectDestructorCallback(cl_mem memobj,
                                              void(CL_CALLBACK *pfn_notify)(cl_mem memobj,
                                                                            void *user_data),
                                              const void *user_data)
{
    // CL_INVALID_MEM_OBJECT if memobj is not a valid memory object.
    if (!Memory::IsValid(memobj))
    {
        return CL_INVALID_MEM_OBJECT;
    }

    // CL_INVALID_VALUE if pfn_notify is NULL.
    if (pfn_notify == nullptr)
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateCreateUserEvent(cl_context context)
{
    // CL_INVALID_CONTEXT if context is not a valid context.
    return Context::IsValidAndVersionOrNewer(context, 1u, 1u) ? CL_SUCCESS : CL_INVALID_CONTEXT;
}

cl_int ValidateSetUserEventStatus(cl_event event, cl_int execution_status)
{
    // CL_INVALID_EVENT if event is not a valid user event object.
    if (!Event::IsValid(event))
    {
        return CL_INVALID_EVENT;
    }
    const Event &evt = event->cast<Event>();
    if (!evt.getContext().getPlatform().isVersionOrNewer(1u, 1u) ||
        evt.getCommandType() != CL_COMMAND_USER)
    {
        return CL_INVALID_EVENT;
    }

    // CL_INVALID_VALUE if the execution_status is not CL_COMPLETE or a negative integer value.
    if (execution_status != CL_COMPLETE && execution_status >= 0)
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_OPERATION if the execution_status for event has already been changed
    // by a previous call to clSetUserEventStatus.
    if (evt.wasStatusChanged())
    {
        return CL_INVALID_OPERATION;
    }

    return CL_SUCCESS;
}

cl_int ValidateSetEventCallback(cl_event event,
                                cl_int command_exec_callback_type,
                                void(CL_CALLBACK *pfn_notify)(cl_event event,
                                                              cl_int event_command_status,
                                                              void *user_data),
                                const void *user_data)
{
    // CL_INVALID_EVENT if event is not a valid event object.
    if (!Event::IsValid(event) ||
        !event->cast<Event>().getContext().getPlatform().isVersionOrNewer(1u, 1u))
    {
        return CL_INVALID_EVENT;
    }

    // CL_INVALID_VALUE if pfn_event_notify is NULL
    // or if command_exec_callback_type is not CL_SUBMITTED, CL_RUNNING, or CL_COMPLETE.
    if (pfn_notify == nullptr ||
        (command_exec_callback_type != CL_SUBMITTED && command_exec_callback_type != CL_RUNNING &&
         command_exec_callback_type != CL_COMPLETE))
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateEnqueueReadBufferRect(cl_command_queue command_queue,
                                     cl_mem buffer,
                                     cl_bool blocking_read,
                                     const size_t *buffer_origin,
                                     const size_t *host_origin,
                                     const size_t *region,
                                     size_t buffer_row_pitch,
                                     size_t buffer_slice_pitch,
                                     size_t host_row_pitch,
                                     size_t host_slice_pitch,
                                     const void *ptr,
                                     cl_uint num_events_in_wait_list,
                                     const cl_event *event_wait_list,
                                     const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, false,
                                                        num_events_in_wait_list, event_wait_list));
    const CommandQueue &queue = command_queue->cast<CommandQueue>();
    if (!queue.getContext().getPlatform().isVersionOrNewer(1u, 1u))
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    ANGLE_VALIDATE(ValidateEnqueueBuffer(queue, buffer, true, false));
    ANGLE_VALIDATE(ValidateBufferRect(buffer->cast<Buffer>(), buffer_origin, region,
                                      buffer_row_pitch, buffer_slice_pitch));
    ANGLE_VALIDATE(ValidateHostRect(host_origin, region, host_row_pitch, host_slice_pitch, ptr));

    return CL_SUCCESS;
}

cl_int ValidateEnqueueWriteBufferRect(cl_command_queue command_queue,
                                      cl_mem buffer,
                                      cl_bool blocking_write,
                                      const size_t *buffer_origin,
                                      const size_t *host_origin,
                                      const size_t *region,
                                      size_t buffer_row_pitch,
                                      size_t buffer_slice_pitch,
                                      size_t host_row_pitch,
                                      size_t host_slice_pitch,
                                      const void *ptr,
                                      cl_uint num_events_in_wait_list,
                                      const cl_event *event_wait_list,
                                      const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, false,
                                                        num_events_in_wait_list, event_wait_list));
    const CommandQueue &queue = command_queue->cast<CommandQueue>();
    if (!queue.getContext().getPlatform().isVersionOrNewer(1u, 1u))
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    ANGLE_VALIDATE(ValidateEnqueueBuffer(queue, buffer, false, true));
    ANGLE_VALIDATE(ValidateBufferRect(buffer->cast<Buffer>(), buffer_origin, region,
                                      buffer_row_pitch, buffer_slice_pitch));
    ANGLE_VALIDATE(ValidateHostRect(host_origin, region, host_row_pitch, host_slice_pitch, ptr));

    return CL_SUCCESS;
}

cl_int ValidateEnqueueCopyBufferRect(cl_command_queue command_queue,
                                     cl_mem src_buffer,
                                     cl_mem dst_buffer,
                                     const size_t *src_origin,
                                     const size_t *dst_origin,
                                     const size_t *region,
                                     size_t src_row_pitch,
                                     size_t src_slice_pitch,
                                     size_t dst_row_pitch,
                                     size_t dst_slice_pitch,
                                     cl_uint num_events_in_wait_list,
                                     const cl_event *event_wait_list,
                                     const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, false,
                                                        num_events_in_wait_list, event_wait_list));
    const CommandQueue &queue = command_queue->cast<CommandQueue>();
    if (!queue.getContext().getPlatform().isVersionOrNewer(1u, 1u))
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    ANGLE_VALIDATE(ValidateEnqueueBuffer(queue, src_buffer, false, false));
    const Buffer &src = src_buffer->cast<Buffer>();

    ANGLE_VALIDATE(ValidateEnqueueBuffer(queue, dst_buffer, false, false));
    const Buffer &dst = dst_buffer->cast<Buffer>();

    ANGLE_VALIDATE(ValidateBufferRect(src, src_origin, region, src_row_pitch, src_slice_pitch));
    ANGLE_VALIDATE(ValidateBufferRect(dst, dst_origin, region, dst_row_pitch, dst_slice_pitch));

    // CL_INVALID_VALUE if src_buffer and dst_buffer are the same buffer object and src_slice_pitch
    // is not equal to dst_slice_pitch or src_row_pitch is not equal to dst_row_pitch.
    if (&src == &dst && (src_slice_pitch != dst_slice_pitch || src_row_pitch != dst_row_pitch))
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

// CL 1.2
cl_int ValidateCreateSubDevices(cl_device_id in_device,
                                const cl_device_partition_property *properties,
                                cl_uint num_devices,
                                const cl_device_id *out_devices,
                                const cl_uint *num_devices_ret)
{
    // CL_INVALID_DEVICE if in_device is not a valid device.
    if (!Device::IsValid(in_device))
    {
        return CL_INVALID_DEVICE;
    }
    const Device &device = in_device->cast<Device>();
    if (!device.isVersionOrNewer(1u, 2u))
    {
        return CL_INVALID_DEVICE;
    }

    // CL_INVALID_VALUE if values specified in properties are not valid
    // or if values specified in properties are valid but not supported by the device
    const std::vector<cl_device_partition_property> &devProps =
        device.getInfo().partitionProperties;
    if (properties == nullptr ||
        std::find(devProps.cbegin(), devProps.cend(), *properties) == devProps.cend())
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateRetainDevice(cl_device_id device)
{
    // CL_INVALID_DEVICE if device is not a valid device.
    if (!Device::IsValid(device) || !device->cast<Device>().isVersionOrNewer(1u, 2u))
    {
        return CL_INVALID_DEVICE;
    }
    return CL_SUCCESS;
}

cl_int ValidateReleaseDevice(cl_device_id device)
{
    // CL_INVALID_DEVICE if device is not a valid device.
    if (!Device::IsValid(device) || !device->cast<Device>().isVersionOrNewer(1u, 2u))
    {
        return CL_INVALID_DEVICE;
    }
    return CL_SUCCESS;
}

cl_int ValidateCreateImage(cl_context context,
                           MemFlags flags,
                           const cl_image_format *image_format,
                           const cl_image_desc *image_desc,
                           const void *host_ptr)
{
    // CL_INVALID_CONTEXT if context is not a valid context.
    if (!Context::IsValidAndVersionOrNewer(context, 1u, 2u))
    {
        return CL_INVALID_CONTEXT;
    }
    const Context &ctx = context->cast<Context>();

    // CL_INVALID_VALUE if values specified in flags are not valid.
    if (!ValidateMemoryFlags(flags, ctx.getPlatform()))
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_IMAGE_FORMAT_DESCRIPTOR if values specified in image_format are not valid
    // or if image_format is NULL.
    if (!IsValidImageFormat(image_format, ctx.getPlatform().getInfo()))
    {
        return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    }

    // CL_INVALID_IMAGE_DESCRIPTOR if image_desc is NULL.
    if (image_desc == nullptr)
    {
        return CL_INVALID_IMAGE_DESCRIPTOR;
    }

    const size_t elemSize = GetElementSize(*image_format);
    if (elemSize == 0u)
    {
        ASSERT(false);
        ERR() << "Failed to calculate image element size";
        return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    }
    const size_t rowPitch = image_desc->image_row_pitch != 0u ? image_desc->image_row_pitch
                                                              : image_desc->image_width * elemSize;
    const size_t imageHeight =
        image_desc->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY ? 1u : image_desc->image_height;
    const size_t sliceSize = imageHeight * rowPitch;

    // CL_INVALID_IMAGE_DESCRIPTOR if values specified in image_desc are not valid.
    switch (FromCLenum<MemObjectType>(image_desc->image_type))
    {
        case MemObjectType::Image1D:
            if (image_desc->image_width == 0u)
            {
                return CL_INVALID_IMAGE_DESCRIPTOR;
            }
            break;
        case MemObjectType::Image2D:
            if (image_desc->image_width == 0u || image_desc->image_height == 0u)
            {
                return CL_INVALID_IMAGE_DESCRIPTOR;
            }
            break;
        case MemObjectType::Image3D:
            if (image_desc->image_width == 0u || image_desc->image_height == 0u ||
                image_desc->image_depth == 0u)
            {
                return CL_INVALID_IMAGE_DESCRIPTOR;
            }
            break;
        case MemObjectType::Image1D_Array:
            if (image_desc->image_width == 0u || image_desc->image_array_size == 0u)
            {
                return CL_INVALID_IMAGE_DESCRIPTOR;
            }
            break;
        case MemObjectType::Image2D_Array:
            if (image_desc->image_width == 0u || image_desc->image_height == 0u ||
                image_desc->image_array_size == 0u)
            {
                return CL_INVALID_IMAGE_DESCRIPTOR;
            }
            break;
        case MemObjectType::Image1D_Buffer:
            if (image_desc->image_width == 0u)
            {
                return CL_INVALID_IMAGE_DESCRIPTOR;
            }
            break;
        default:
            return CL_INVALID_IMAGE_DESCRIPTOR;
    }
    if (image_desc->image_row_pitch != 0u)
    {
        // image_row_pitch must be 0 if host_ptr is NULL.
        if (host_ptr == nullptr)
        {
            return CL_INVALID_IMAGE_DESCRIPTOR;
        }
        // image_row_pitch can be either 0
        // or >= image_width * size of element in bytes if host_ptr is not NULL.
        if (image_desc->image_row_pitch < image_desc->image_width * elemSize)
        {
            return CL_INVALID_IMAGE_DESCRIPTOR;
        }
        // If image_row_pitch is not 0, it must be a multiple of the image element size in bytes.
        if ((image_desc->image_row_pitch % elemSize) != 0u)
        {
            return CL_INVALID_IMAGE_DESCRIPTOR;
        }
    }
    if (image_desc->image_slice_pitch != 0u)
    {
        // image_slice_pitch must be 0 if host_ptr is NULL.
        if (host_ptr == nullptr)
        {
            return CL_INVALID_IMAGE_DESCRIPTOR;
        }
        // If host_ptr is not NULL, image_slice_pitch can be either 0
        // or >= image_row_pitch * image_height for a 2D image array or 3D image
        // and can be either 0 or >= image_row_pitch for a 1D image array.
        if (image_desc->image_slice_pitch < sliceSize)
        {
            return CL_INVALID_IMAGE_DESCRIPTOR;
        }
        // If image_slice_pitch is not 0, it must be a multiple of the image_row_pitch.
        if ((image_desc->image_slice_pitch % rowPitch) != 0u)
        {
            return CL_INVALID_IMAGE_DESCRIPTOR;
        }
    }
    // num_mip_levels and num_samples must be 0.
    if (image_desc->num_mip_levels != 0u || image_desc->num_samples != 0u)
    {
        return CL_INVALID_IMAGE_DESCRIPTOR;
    }
    // buffer can be a buffer memory object if image_type is CL_MEM_OBJECT_IMAGE1D_BUFFER or
    // CL_MEM_OBJECT_IMAGE2D. buffer can be an image object if image_type is CL_MEM_OBJECT_IMAGE2D.
    // Otherwise it must be NULL.
    if (image_desc->buffer != nullptr &&
        (!Buffer::IsValid(image_desc->buffer) ||
         (image_desc->image_type != CL_MEM_OBJECT_IMAGE1D_BUFFER &&
          image_desc->image_type != CL_MEM_OBJECT_IMAGE2D)) &&
        (!Image::IsValid(image_desc->buffer) || image_desc->image_type != CL_MEM_OBJECT_IMAGE2D))
    {
        return CL_INVALID_IMAGE_DESCRIPTOR;
    }

    // CL_INVALID_OPERATION if there are no devices in context that support images.
    if (!ctx.supportsImages())
    {
        return CL_INVALID_OPERATION;
    }

    // Returns CL_INVALID_OPERATION if no devices in context support creating a 2D image from a
    // buffer.
    const bool isImage2dFromBuffer =
        image_desc->image_type == CL_MEM_OBJECT_IMAGE2D && image_desc->mem_object != NULL;
    if (isImage2dFromBuffer && !ctx.supportsImage2DFromBuffer())
    {
        return CL_INVALID_OPERATION;
    }

    // CL_INVALID_IMAGE_SIZE if image dimensions specified in image_desc exceed the maximum
    // image dimensions described in the Device Queries table for all devices in context.
    const DevicePtrs &devices = ctx.getDevices();
    if (std::find_if(devices.cbegin(), devices.cend(), [&](const DevicePtr &ptr) {
            return ptr->supportsNativeImageDimensions(*image_desc);
        }) == devices.cend())
    {
        return CL_INVALID_IMAGE_SIZE;
    }

    // CL_INVALID_HOST_PTR
    // if host_ptr is NULL and CL_MEM_USE_HOST_PTR or CL_MEM_COPY_HOST_PTR are set in flags or
    // if host_ptr is not NULL but CL_MEM_COPY_HOST_PTR or CL_MEM_USE_HOST_PTR are not set in flags.
    if ((host_ptr != nullptr) != flags.intersects(CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR))
    {
        return CL_INVALID_HOST_PTR;
    }

    return CL_SUCCESS;
}

cl_int ValidateCreateProgramWithBuiltInKernels(cl_context context,
                                               cl_uint num_devices,
                                               const cl_device_id *device_list,
                                               const char *kernel_names)
{
    // CL_INVALID_CONTEXT if context is not a valid context.
    if (!Context::IsValidAndVersionOrNewer(context, 1u, 2u))
    {
        return CL_INVALID_CONTEXT;
    }
    const Context &ctx = context->cast<Context>();

    // CL_INVALID_VALUE if device_list is NULL or num_devices is zero or if kernel_names is NULL.
    if (device_list == nullptr || num_devices == 0u || kernel_names == nullptr)
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_DEVICE if any device in device_list
    // is not in the list of devices associated with context.
    for (size_t index = 0u; index < num_devices; ++index)
    {
        if (!ctx.hasDevice(device_list[index]))
        {
            return CL_INVALID_DEVICE;
        }
    }

    // CL_INVALID_VALUE if kernel_names contains a kernel name
    // that is not supported by any of the devices in device_list.
    const char *start = kernel_names;
    do
    {
        const char *end = start;
        while (*end != '\0' && *end != ';')
        {
            ++end;
        }
        const size_t length = end - start;
        if (length != 0u && !ctx.supportsBuiltInKernel(std::string(start, length)))
        {
            return CL_INVALID_VALUE;
        }
        start = end;
    } while (*start++ != '\0');

    return CL_SUCCESS;
}

cl_int ValidateCompileProgram(cl_program program,
                              cl_uint num_devices,
                              const cl_device_id *device_list,
                              const char *options,
                              cl_uint num_input_headers,
                              const cl_program *input_headers,
                              const char **header_include_names,
                              void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
                              const void *user_data)
{
    // CL_INVALID_PROGRAM if program is not a valid program object.
    if (!Program::IsValid(program))
    {
        return CL_INVALID_PROGRAM;
    }
    const Program &prog = program->cast<Program>();
    if (!prog.getContext().getPlatform().isVersionOrNewer(1u, 2u))
    {
        return CL_INVALID_PROGRAM;
    }

    // CL_INVALID_OPERATION if we do not have source code.
    if (prog.getSource().empty())
    {
        ERR() << "No OpenCL C source available from program object (" << &prog << ")!";
        return CL_INVALID_OPERATION;
    }

    // CL_INVALID_VALUE if device_list is NULL and num_devices is greater than zero,
    // or if device_list is not NULL and num_devices is zero.
    if ((device_list != nullptr) != (num_devices != 0u))
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_DEVICE if any device in device_list
    // is not in the list of devices associated with program.
    while (num_devices-- != 0u)
    {
        if (!prog.hasDevice(*device_list++))
        {
            return CL_INVALID_DEVICE;
        }
    }

    // CL_INVALID_VALUE if num_input_headers is zero and header_include_names
    // or input_headers are not NULL
    // or if num_input_headers is not zero and header_include_names or input_headers are NULL.
    if ((num_input_headers != 0u) != (header_include_names != nullptr) ||
        (num_input_headers != 0u) != (input_headers != nullptr))
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_VALUE if pfn_notify is NULL but user_data is not NULL.
    if (pfn_notify == nullptr && user_data != nullptr)
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_OPERATION if the build of a program executable for any of the devices listed
    // in device_list by a previous call to clBuildProgram for program has not completed.
    if (prog.isBuilding())
    {
        return CL_INVALID_OPERATION;
    }

    // CL_INVALID_OPERATION if there are kernel objects attached to program.
    if (prog.hasAttachedKernels())
    {
        return CL_INVALID_OPERATION;
    }

    return CL_SUCCESS;
}

cl_int ValidateLinkProgram(cl_context context,
                           cl_uint num_devices,
                           const cl_device_id *device_list,
                           const char *options,
                           cl_uint num_input_programs,
                           const cl_program *input_programs,
                           void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
                           const void *user_data)
{
    // CL_INVALID_CONTEXT if context is not a valid context.
    if (!Context::IsValidAndVersionOrNewer(context, 1u, 2u))
    {
        return CL_INVALID_CONTEXT;
    }
    const Context &ctx = context->cast<Context>();

    // CL_INVALID_OPERATION if the compilation or build of a program executable for any of the
    // devices listed in device_list by a previous call to clCompileProgram or clBuildProgram for
    // program has not completed.
    for (size_t i = 0; i < num_devices; ++i)
    {
        Device &device = device_list[i]->cast<Device>();
        for (size_t j = 0; j < num_input_programs; ++j)
        {
            cl_build_status buildStatus = CL_BUILD_NONE;
            Program &program            = input_programs[j]->cast<Program>();
            if (IsError(program.getBuildInfo(device.getNative(), ProgramBuildInfo::Status,
                                             sizeof(cl_build_status), &buildStatus, nullptr)))
            {
                return CL_INVALID_PROGRAM;
            }

            if (buildStatus != CL_BUILD_SUCCESS)
            {
                return CL_INVALID_OPERATION;
            }
        }
    }

    // CL_INVALID_OPERATION if the rules for devices containing compiled binaries or libraries as
    // described in input_programs argument below are not followed.
    //
    // All programs specified by input_programs contain a compiled binary or library for the device.
    // In this case, a link is performed to generate a program executable for this device.
    //
    // None of the programs contain a compiled binary or library for that device. In this case, no
    // link is performed and there will be no program executable generated for this device.
    //
    // All other cases will return a CL_INVALID_OPERATION error.
    BitField libraryOrObject(CL_PROGRAM_BINARY_TYPE_LIBRARY |
                             CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT);
    for (size_t i = 0; i < num_devices; ++i)
    {
        bool foundAnyLibraryOrObject = false;
        Device &device               = device_list[i]->cast<Device>();
        for (size_t j = 0; j < num_input_programs; ++j)
        {
            cl_program_binary_type binaryType = CL_PROGRAM_BINARY_TYPE_NONE;
            Program &program                  = input_programs[j]->cast<Program>();
            if (IsError(program.getBuildInfo(device.getNative(), ProgramBuildInfo::BinaryType,
                                             sizeof(cl_program_binary_type), &binaryType, nullptr)))
            {
                return CL_INVALID_PROGRAM;
            }

            if (libraryOrObject.excludes(binaryType))
            {
                if (foundAnyLibraryOrObject)
                {
                    return CL_INVALID_OPERATION;
                }
            }
            else
            {
                foundAnyLibraryOrObject = true;
            }
        }
    }

    // CL_INVALID_VALUE if device_list is NULL and num_devices is greater than zero,
    // or if device_list is not NULL and num_devices is zero.
    if ((device_list != nullptr) != (num_devices != 0u))
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_DEVICE if any device in device_list
    // is not in the list of devices associated with context.
    while (num_devices-- != 0u)
    {
        if (!ctx.hasDevice(*device_list++))
        {
            return CL_INVALID_DEVICE;
        }
    }

    // CL_INVALID_VALUE if num_input_programs is zero or input_programs is NULL.
    if (num_input_programs == 0u || input_programs == nullptr)
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_PROGRAM if programs specified in input_programs are not valid program objects.
    while (num_input_programs-- != 0u)
    {
        if (!Program::IsValid(*input_programs++))
        {
            return CL_INVALID_PROGRAM;
        }
    }

    // CL_INVALID_VALUE if pfn_notify is NULL but user_data is not NULL.
    if (pfn_notify == nullptr && user_data != nullptr)
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateUnloadPlatformCompiler(cl_platform_id platform)
{
    // CL_INVALID_PLATFORM if platform is not a valid platform.
    if (!Platform::IsValid(platform) || !platform->cast<Platform>().isVersionOrNewer(1u, 2u))
    {
        return CL_INVALID_PLATFORM;
    }
    return CL_SUCCESS;
}

cl_int ValidateGetKernelArgInfo(cl_kernel kernel,
                                cl_uint arg_index,
                                KernelArgInfo param_name,
                                size_t param_value_size,
                                const void *param_value,
                                const size_t *param_value_size_ret)
{
    // CL_INVALID_KERNEL if kernel is a not a valid kernel object.
    if (!Kernel::IsValid(kernel))
    {
        return CL_INVALID_KERNEL;
    }
    const Kernel &krnl = kernel->cast<Kernel>();
    if (!krnl.getProgram().getContext().getPlatform().isVersionOrNewer(1u, 2u))
    {
        return CL_INVALID_KERNEL;
    }

    // CL_INVALID_ARG_INDEX if arg_index is not a valid argument index.
    if (arg_index >= krnl.getInfo().args.size())
    {
        return CL_INVALID_ARG_INDEX;
    }

    // CL_KERNEL_ARG_INFO_NOT_AVAILABLE if the argument information is not available for kernel.
    if (!krnl.getInfo().args[arg_index].isAvailable())
    {
        return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
    }

    // CL_INVALID_VALUE if param_name is not valid.
    if (param_name == KernelArgInfo::InvalidEnum)
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateEnqueueFillBuffer(cl_command_queue command_queue,
                                 cl_mem buffer,
                                 const void *pattern,
                                 size_t pattern_size,
                                 size_t offset,
                                 size_t size,
                                 cl_uint num_events_in_wait_list,
                                 const cl_event *event_wait_list,
                                 const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, false,
                                                        num_events_in_wait_list, event_wait_list));
    const CommandQueue &queue = command_queue->cast<CommandQueue>();
    if (!queue.getContext().getPlatform().isVersionOrNewer(1u, 2u))
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    ANGLE_VALIDATE(ValidateEnqueueBuffer(queue, buffer, false, false));

    // CL_INVALID_VALUE if offset or offset + size require accessing
    // elements outside the buffer object respectively.
    if (!buffer->cast<Buffer>().isRegionValid(offset, size))
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_VALUE if pattern is NULL or if pattern_size is 0 or
    // if pattern_size is not one of { 1, 2, 4, 8, 16, 32, 64, 128 }.
    if (pattern == nullptr || pattern_size == 0u || pattern_size > 128u ||
        (pattern_size & (pattern_size - 1u)) != 0u)
    {
        return CL_INVALID_VALUE;
    }

    // CL_INVALID_VALUE if offset and size are not a multiple of pattern_size.
    if ((offset % pattern_size) != 0u || (size % pattern_size) != 0u)
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateEnqueueFillImage(cl_command_queue command_queue,
                                cl_mem image,
                                const void *fill_color,
                                const size_t *origin,
                                const size_t *region,
                                cl_uint num_events_in_wait_list,
                                const cl_event *event_wait_list,
                                const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, true,
                                                        num_events_in_wait_list, event_wait_list));
    const CommandQueue &queue = command_queue->cast<CommandQueue>();
    if (!queue.getContext().getPlatform().isVersionOrNewer(1u, 2u))
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    ANGLE_VALIDATE(ValidateEnqueueImage(queue, image, false, false));
    const Image &img = image->cast<Image>();

    ANGLE_VALIDATE(ValidateImageForDevice(img, queue.getDevice(), origin, region));

    // CL_INVALID_VALUE if fill_color is NULL.
    if (fill_color == nullptr)
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateEnqueueMigrateMemObjects(cl_command_queue command_queue,
                                        cl_uint num_mem_objects,
                                        const cl_mem *mem_objects,
                                        MemMigrationFlags flags,
                                        cl_uint num_events_in_wait_list,
                                        const cl_event *event_wait_list,
                                        const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, false,
                                                        num_events_in_wait_list, event_wait_list));
    const CommandQueue &queue = command_queue->cast<CommandQueue>();
    if (!queue.getContext().getPlatform().isVersionOrNewer(1u, 2u))
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    // CL_INVALID_VALUE if num_mem_objects is zero or if mem_objects is NULL.
    if (num_mem_objects == 0u || mem_objects == nullptr)
    {
        return CL_INVALID_VALUE;
    }

    while (num_mem_objects-- != 0u)
    {
        // CL_INVALID_MEM_OBJECT if any of the memory objects
        // in mem_objects is not a valid memory object.
        if (!Memory::IsValid(*mem_objects))
        {
            return CL_INVALID_MEM_OBJECT;
        }

        // CL_INVALID_CONTEXT if the context associated with command_queue
        // and memory objects in mem_objects are not the same.
        if (&queue.getContext() != &(*mem_objects++)->cast<Memory>().getContext())
        {
            return CL_INVALID_CONTEXT;
        }
    }

    // CL_INVALID_VALUE if flags is not 0 or is not any of the values described in the table.
    const MemMigrationFlags allowedFlags(CL_MIGRATE_MEM_OBJECT_HOST |
                                         CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED);
    if (flags.hasOtherBitsThan(allowedFlags))
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateEnqueueMarkerWithWaitList(cl_command_queue command_queue,
                                         cl_uint num_events_in_wait_list,
                                         const cl_event *event_wait_list,
                                         const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, false,
                                                        num_events_in_wait_list, event_wait_list));
    if (!command_queue->cast<CommandQueue>().getContext().getPlatform().isVersionOrNewer(1u, 2u))
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    return CL_SUCCESS;
}

cl_int ValidateEnqueueBarrierWithWaitList(cl_command_queue command_queue,
                                          cl_uint num_events_in_wait_list,
                                          const cl_event *event_wait_list,
                                          const cl_event *event)
{
    ANGLE_VALIDATE(ValidateCommandQueueAndEventWaitList(command_queue, false,
                                                        num_events_in_wait_list, event_wait_list));
    if (!command_queue->cast<CommandQueue>().getContext().getPlatform().isVersionOrNewer(1u, 2u))
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    return CL_SUCCESS;
}

cl_int ValidateGetExtensionFunctionAddressForPlatform(cl_platform_id platform,
                                                      const char *func_name)
{
    if (!Platform::IsValid(platform) || func_name == nullptr || *func_name == '\0')
    {
        return CL_INVALID_VALUE;
    }
    return CL_SUCCESS;
}

// CL 2.0
cl_int ValidateCreateCommandQueueWithProperties(cl_context context,
                                                cl_device_id device,
                                                const cl_queue_properties *properties)
{
    // CL_INVALID_CONTEXT if context is not a valid context.
    if (!Context::IsValidAndVersionOrNewer(context, 2u, 0u))
    {
        return CL_INVALID_CONTEXT;
    }

    // CL_INVALID_DEVICE if device is not a valid device or is not associated with context.
    if (!context->cast<Context>().hasDevice(device) ||
        !device->cast<Device>().isVersionOrNewer(2u, 0u))
    {
        return CL_INVALID_DEVICE;
    }

    // CL_INVALID_VALUE if values specified in properties are not valid.
    if (properties != nullptr)
    {
        bool isQueueOnDevice = false;
        bool hasQueueSize    = false;
        while (*properties != 0)
        {
            switch (*properties++)
            {
                case CL_QUEUE_PROPERTIES:
                {
                    const CommandQueueProperties props(*properties++);
                    const CommandQueueProperties validProps(
                        CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE |
                        CL_QUEUE_ON_DEVICE | CL_QUEUE_ON_DEVICE_DEFAULT);
                    if (props.hasOtherBitsThan(validProps) ||
                        // If CL_QUEUE_ON_DEVICE is set, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE
                        // must also be set.
                        (props.intersects(CL_QUEUE_ON_DEVICE) &&
                         !props.intersects(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE)) ||
                        // CL_QUEUE_ON_DEVICE_DEFAULT can only be used with CL_QUEUE_ON_DEVICE.
                        (props.intersects(CL_QUEUE_ON_DEVICE_DEFAULT) &&
                         !props.intersects(CL_QUEUE_ON_DEVICE)))
                    {
                        return CL_INVALID_VALUE;
                    }
                    isQueueOnDevice = props.intersects(CL_QUEUE_ON_DEVICE);
                    break;
                }
                case CL_QUEUE_SIZE:
                {
                    // CL_QUEUE_SIZE must be a value <= CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE.
                    if (*properties++ > device->cast<Device>().getInfo().queueOnDeviceMaxSize)
                    {
                        return CL_INVALID_VALUE;
                    }
                    hasQueueSize = true;
                    break;
                }
                default:
                    return CL_INVALID_VALUE;
            }
        }

        // CL_QUEUE_SIZE can only be specified if CL_QUEUE_ON_DEVICE is set in CL_QUEUE_PROPERTIES.
        if (hasQueueSize && !isQueueOnDevice)
        {
            return CL_INVALID_VALUE;
        }
    }

    return CL_SUCCESS;
}

cl_int ValidateCreatePipe(cl_context context,
                          MemFlags flags,
                          cl_uint pipe_packet_size,
                          cl_uint pipe_max_packets,
                          const cl_pipe_properties *properties)
{
    return CL_SUCCESS;
}

cl_int ValidateGetPipeInfo(cl_mem pipe,
                           PipeInfo param_name,
                           size_t param_value_size,
                           const void *param_value,
                           const size_t *param_value_size_ret)
{
    return CL_SUCCESS;
}

cl_int ValidateSVMAlloc(cl_context context, SVM_MemFlags flags, size_t size, cl_uint alignment)
{
    return CL_SUCCESS;
}

cl_int ValidateSVMFree(cl_context context, const void *svm_pointer)
{
    return CL_SUCCESS;
}

cl_int ValidateCreateSamplerWithProperties(cl_context context,
                                           const cl_sampler_properties *sampler_properties)
{
    // CL_INVALID_CONTEXT if context is not a valid context.
    if (!Context::IsValidAndVersionOrNewer(context, 2u, 0u))
    {
        return CL_INVALID_CONTEXT;
    }

    // CL_INVALID_VALUE if the property name in sampler_properties is not a supported property name,
    // if the value specified for a supported property name is not valid,
    // or if the same property name is specified more than once.
    if (sampler_properties != nullptr)
    {
        bool hasNormalizedCoords            = false;
        bool hasAddressingMode              = false;
        bool hasFilterMode                  = false;
        const cl_sampler_properties *propIt = sampler_properties;
        while (*propIt != 0)
        {
            switch (*propIt++)
            {
                case CL_SAMPLER_NORMALIZED_COORDS:
                    if (hasNormalizedCoords || (*propIt != CL_FALSE && *propIt != CL_TRUE))
                    {
                        return CL_INVALID_VALUE;
                    }
                    hasNormalizedCoords = true;
                    ++propIt;
                    break;
                case CL_SAMPLER_ADDRESSING_MODE:
                    if (hasAddressingMode || FromCLenum<AddressingMode>(static_cast<CLenum>(
                                                 *propIt++)) == AddressingMode::InvalidEnum)
                    {
                        return CL_INVALID_VALUE;
                    }
                    hasAddressingMode = true;
                    break;
                case CL_SAMPLER_FILTER_MODE:
                    if (hasFilterMode || FromCLenum<FilterMode>(static_cast<CLenum>(*propIt++)) ==
                                             FilterMode::InvalidEnum)
                    {
                        return CL_INVALID_VALUE;
                    }
                    hasFilterMode = true;
                    break;
                default:
                    return CL_INVALID_VALUE;
            }
        }
    }

    // CL_INVALID_OPERATION if images are not supported by any device associated with context.
    if (!context->cast<Context>().supportsImages())
    {
        return CL_INVALID_OPERATION;
    }

    return CL_SUCCESS;
}

cl_int ValidateSetKernelArgSVMPointer(cl_kernel kernel, cl_uint arg_index, const void *arg_value)
{
    return CL_SUCCESS;
}

cl_int ValidateSetKernelExecInfo(cl_kernel kernel,
                                 KernelExecInfo param_name,
                                 size_t param_value_size,
                                 const void *param_value)
{
    return CL_SUCCESS;
}

cl_int ValidateEnqueueSVMFree(cl_command_queue command_queue,
                              cl_uint num_svm_pointers,
                              void *const svm_pointers[],
                              void(CL_CALLBACK *pfn_free_func)(cl_command_queue queue,
                                                               cl_uint num_svm_pointers,
                                                               void *svm_pointers[],
                                                               void *user_data),
                              const void *user_data,
                              cl_uint num_events_in_wait_list,
                              const cl_event *event_wait_list,
                              const cl_event *event)
{
    return CL_SUCCESS;
}

cl_int ValidateEnqueueSVMMemcpy(cl_command_queue command_queue,
                                cl_bool blocking_copy,
                                const void *dst_ptr,
                                const void *src_ptr,
                                size_t size,
                                cl_uint num_events_in_wait_list,
                                const cl_event *event_wait_list,
                                const cl_event *event)
{
    return CL_SUCCESS;
}

cl_int ValidateEnqueueSVMMemFill(cl_command_queue command_queue,
                                 const void *svm_ptr,
                                 const void *pattern,
                                 size_t pattern_size,
                                 size_t size,
                                 cl_uint num_events_in_wait_list,
                                 const cl_event *event_wait_list,
                                 const cl_event *event)
{
    return CL_SUCCESS;
}

cl_int ValidateEnqueueSVMMap(cl_command_queue command_queue,
                             cl_bool blocking_map,
                             MapFlags flags,
                             const void *svm_ptr,
                             size_t size,
                             cl_uint num_events_in_wait_list,
                             const cl_event *event_wait_list,
                             const cl_event *event)
{
    return CL_SUCCESS;
}

cl_int ValidateEnqueueSVMUnmap(cl_command_queue command_queue,
                               const void *svm_ptr,
                               cl_uint num_events_in_wait_list,
                               const cl_event *event_wait_list,
                               const cl_event *event)
{
    return CL_SUCCESS;
}

// CL 2.1
cl_int ValidateSetDefaultDeviceCommandQueue(cl_context context,
                                            cl_device_id device,
                                            cl_command_queue command_queue)
{
    return CL_SUCCESS;
}

cl_int ValidateGetDeviceAndHostTimer(cl_device_id device,
                                     const cl_ulong *device_timestamp,
                                     const cl_ulong *host_timestamp)
{
    return CL_SUCCESS;
}

cl_int ValidateGetHostTimer(cl_device_id device, const cl_ulong *host_timestamp)
{
    return CL_SUCCESS;
}

cl_int ValidateCreateProgramWithIL(cl_context context, const void *il, size_t length)
{
    // CL_INVALID_CONTEXT if context is not a valid context.
    if (!Context::IsValidAndVersionOrNewer(context, 2u, 1u))
    {
        return CL_INVALID_CONTEXT;
    }
    const Context &ctx = context->cast<Context>();

    // CL_INVALID_OPERATION if no devices in context support intermediate language programs.
    if (!ctx.supportsIL())
    {
        return CL_INVALID_OPERATION;
    }

    // CL_INVALID_VALUE if il is NULL or if length is zero.
    if (il == nullptr || length == 0u)
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

cl_int ValidateCloneKernel(cl_kernel source_kernel)
{
    if (!Kernel::IsValid(source_kernel))
    {
        return CL_INVALID_KERNEL;
    }
    return CL_SUCCESS;
}

cl_int ValidateGetKernelSubGroupInfo(cl_kernel kernel,
                                     cl_device_id device,
                                     KernelSubGroupInfo param_name,
                                     size_t input_value_size,
                                     const void *input_value,
                                     size_t param_value_size,
                                     const void *param_value,
                                     const size_t *param_value_size_ret)
{
    return CL_SUCCESS;
}

cl_int ValidateEnqueueSVMMigrateMem(cl_command_queue command_queue,
                                    cl_uint num_svm_pointers,
                                    const void **svm_pointers,
                                    const size_t *sizes,
                                    MemMigrationFlags flags,
                                    cl_uint num_events_in_wait_list,
                                    const cl_event *event_wait_list,
                                    const cl_event *event)
{
    return CL_SUCCESS;
}

// CL 2.2
cl_int ValidateSetProgramReleaseCallback(cl_program program,
                                         void(CL_CALLBACK *pfn_notify)(cl_program program,
                                                                       void *user_data),
                                         const void *user_data)
{
    return CL_SUCCESS;
}

cl_int ValidateSetProgramSpecializationConstant(cl_program program,
                                                cl_uint spec_id,
                                                size_t spec_size,
                                                const void *spec_value)
{
    return CL_SUCCESS;
}

// CL 3.0
cl_int ValidateSetContextDestructorCallback(cl_context context,
                                            void(CL_CALLBACK *pfn_notify)(cl_context context,
                                                                          void *user_data),
                                            const void *user_data)
{
    return CL_SUCCESS;
}

cl_int ValidateCreateBufferWithProperties(cl_context context,
                                          const cl_mem_properties *properties,
                                          MemFlags flags,
                                          size_t size,
                                          const void *host_ptr)
{
    ANGLE_VALIDATE(ValidateCreateBuffer(context, flags, size, host_ptr));

    // CL_INVALID_CONTEXT if context is not a valid context.
    if (!context->cast<Context>().getPlatform().isVersionOrNewer(3u, 0u))
    {
        return CL_INVALID_CONTEXT;
    }

    // CL_INVALID_PROPERTY if a property name in properties is not a supported property name,
    // if the value specified for a supported property name is not valid,
    // or if the same property name is specified more than once.
    if (!ValidateMemoryProperties(properties))
    {
        return CL_INVALID_PROPERTY;
    }

    return CL_SUCCESS;
}

cl_int ValidateCreateImageWithProperties(cl_context context,
                                         const cl_mem_properties *properties,
                                         MemFlags flags,
                                         const cl_image_format *image_format,
                                         const cl_image_desc *image_desc,
                                         const void *host_ptr)
{
    ANGLE_VALIDATE(ValidateCreateImage(context, flags, image_format, image_desc, host_ptr));

    // CL_INVALID_CONTEXT if context is not a valid context.
    if (!context->cast<Context>().getPlatform().isVersionOrNewer(3u, 0u))
    {
        return CL_INVALID_CONTEXT;
    }

    // CL_INVALID_PROPERTY if a property name in properties is not a supported property name,
    // if the value specified for a supported property name is not valid,
    // or if the same property name is specified more than once.
    if (!ValidateMemoryProperties(properties))
    {
        return CL_INVALID_PROPERTY;
    }

    return CL_SUCCESS;
}

// cl_khr_icd
cl_int ValidateIcdGetPlatformIDsKHR(cl_uint num_entries,
                                    const cl_platform_id *platforms,
                                    const cl_uint *num_platforms)
{
    if ((num_entries == 0u && platforms != nullptr) ||
        (platforms == nullptr && num_platforms == nullptr))
    {
        return CL_INVALID_VALUE;
    }
    return CL_SUCCESS;
}

}  // namespace cl
