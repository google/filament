//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLContextVk.cpp: Implements the class methods for CLContextVk.

#include "libANGLE/renderer/vulkan/CLContextVk.h"
#include "common/PackedEnums.h"
#include "libANGLE/renderer/vulkan/CLCommandQueueVk.h"
#include "libANGLE/renderer/vulkan/CLEventVk.h"
#include "libANGLE/renderer/vulkan/CLMemoryVk.h"
#include "libANGLE/renderer/vulkan/CLProgramVk.h"
#include "libANGLE/renderer/vulkan/CLSamplerVk.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/vk_cache_utils.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

#include "libANGLE/CLBuffer.h"
#include "libANGLE/CLContext.h"
#include "libANGLE/CLEvent.h"
#include "libANGLE/CLImage.h"
#include "libANGLE/CLProgram.h"
#include "libANGLE/cl_utils.h"

namespace rx
{

CLContextVk::CLContextVk(const cl::Context &context, const cl::DevicePtrs devicePtrs)
    : CLContextImpl(context),
      vk::Context(getPlatform()->getRenderer()),
      mAssociatedDevices(devicePtrs)
{
    mDeviceQueueIndex = mRenderer->getDefaultDeviceQueueIndex();
}

CLContextVk::~CLContextVk()
{
    mMetaDescriptorPool.destroy(getRenderer());
    mDescriptorSetLayoutCache.destroy(getRenderer());
    mPipelineLayoutCache.destroy(getRenderer());
}

void CLContextVk::handleError(VkResult errorCode,
                              const char *file,
                              const char *function,
                              unsigned int line)
{
    ASSERT(errorCode != VK_SUCCESS);

    CLenum clErrorCode = CL_SUCCESS;
    switch (errorCode)
    {
        case VK_ERROR_TOO_MANY_OBJECTS:
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            clErrorCode = CL_OUT_OF_HOST_MEMORY;
            break;
        default:
            clErrorCode = CL_INVALID_OPERATION;
    }
    ERR() << "Internal Vulkan error (" << errorCode << "): " << VulkanResultString(errorCode);
    ERR() << "  CL error (" << clErrorCode << ")";

    if (errorCode == VK_ERROR_DEVICE_LOST)
    {
        handleDeviceLost();
    }
    ANGLE_CL_SET_ERROR(clErrorCode);
}

void CLContextVk::handleDeviceLost() const
{
    // For now just notify the renderer
    getRenderer()->notifyDeviceLost();
}

angle::Result CLContextVk::getDevices(cl::DevicePtrs *devicePtrsOut) const
{
    ASSERT(!mAssociatedDevices.empty());
    *devicePtrsOut = mAssociatedDevices;
    return angle::Result::Continue;
}

angle::Result CLContextVk::createCommandQueue(const cl::CommandQueue &commandQueue,
                                              CLCommandQueueImpl::Ptr *commandQueueOut)
{
    CLCommandQueueVk *queueImpl = new CLCommandQueueVk(commandQueue);
    if (queueImpl == nullptr)
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }
    ANGLE_TRY(queueImpl->init());
    *commandQueueOut = CLCommandQueueVk::Ptr(std::move(queueImpl));
    return angle::Result::Continue;
}

angle::Result CLContextVk::createBuffer(const cl::Buffer &buffer,
                                        void *hostPtr,
                                        CLMemoryImpl::Ptr *bufferOut)
{
    CLBufferVk *memory = new (std::nothrow) CLBufferVk(buffer);
    if (memory == nullptr)
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }
    ANGLE_TRY(memory->create(hostPtr));
    *bufferOut = CLMemoryImpl::Ptr(memory);
    mAssociatedObjects->mMemories.emplace(buffer.getNative());
    return angle::Result::Continue;
}

angle::Result CLContextVk::createImage(const cl::Image &image,
                                       void *hostPtr,
                                       CLMemoryImpl::Ptr *imageOut)
{
    CLImageVk *memory = new (std::nothrow) CLImageVk(image);
    if (memory == nullptr)
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }
    ANGLE_TRY(memory->create(hostPtr));
    *imageOut = CLMemoryImpl::Ptr(memory);
    mAssociatedObjects->mMemories.emplace(image.getNative());
    return angle::Result::Continue;
}

VkFormat CLContextVk::getVkFormatFromCL(cl_image_format format)
{
    angle::FormatID formatID;
    switch (format.image_channel_order)
    {
        case CL_R:
            formatID = angle::Format::CLRFormatToID(format.image_channel_data_type);
            break;
        case CL_RG:
            formatID = angle::Format::CLRGFormatToID(format.image_channel_data_type);
            break;
        case CL_RGB:
            formatID = angle::Format::CLRGBFormatToID(format.image_channel_data_type);
            break;
        case CL_RGBA:
            formatID = angle::Format::CLRGBAFormatToID(format.image_channel_data_type);
            break;
        case CL_BGRA:
            formatID = angle::Format::CLBGRAFormatToID(format.image_channel_data_type);
            break;
        case CL_sRGBA:
            formatID = angle::Format::CLsRGBAFormatToID(format.image_channel_data_type);
            break;
        default:
            return VK_FORMAT_UNDEFINED;
    }
    return getPlatform()->getRenderer()->getFormat(formatID).getActualRenderableImageVkFormat(
        getPlatform()->getRenderer());
}

angle::Result CLContextVk::getSupportedImageFormats(cl::MemFlags flags,
                                                    cl::MemObjectType imageType,
                                                    cl_uint numEntries,
                                                    cl_image_format *imageFormats,
                                                    cl_uint *numImageFormats)
{
    VkPhysicalDevice physicalDevice = getPlatform()->getRenderer()->getPhysicalDevice();
    std::vector<cl_image_format> supportedFormats;
    std::vector<cl_image_format> minSupportedFormats;
    if (flags.intersects((CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY)))
    {
        minSupportedFormats.insert(minSupportedFormats.end(),
                                   std::begin(kMinSupportedFormatsReadOrWrite),
                                   std::end(kMinSupportedFormatsReadOrWrite));
    }
    else
    {
        minSupportedFormats.insert(minSupportedFormats.end(),
                                   std::begin(kMinSupportedFormatsReadAndWrite),
                                   std::end(kMinSupportedFormatsReadAndWrite));
    }
    for (cl_image_format format : minSupportedFormats)
    {
        VkFormatProperties formatProperties;
        VkFormat vkFormat = getVkFormatFromCL(format);
        ASSERT(vkFormat != VK_FORMAT_UNDEFINED);
        vkGetPhysicalDeviceFormatProperties(physicalDevice, vkFormat, &formatProperties);
        if (formatProperties.optimalTilingFeatures != 0)
        {
            supportedFormats.push_back(format);
        }
    }
    if (numImageFormats != nullptr)
    {
        *numImageFormats = static_cast<cl_uint>(supportedFormats.size());
    }
    if (imageFormats != nullptr)
    {
        memcpy(imageFormats, supportedFormats.data(),
               sizeof(cl_image_format) *
                   std::min(static_cast<cl_uint>(supportedFormats.size()), numEntries));
    }

    return angle::Result::Continue;
}

angle::Result CLContextVk::createSampler(const cl::Sampler &sampler, CLSamplerImpl::Ptr *samplerOut)
{
    CLSamplerVk *samplerVk = new (std::nothrow) CLSamplerVk(sampler);
    if (samplerVk == nullptr)
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }
    ANGLE_TRY(samplerVk->create());
    *samplerOut = CLSamplerImpl::Ptr(samplerVk);
    return angle::Result::Continue;
}

angle::Result CLContextVk::createProgramWithSource(const cl::Program &program,
                                                   const std::string &source,
                                                   CLProgramImpl::Ptr *programOut)
{
    CLProgramVk *programVk = new (std::nothrow) CLProgramVk(program);
    if (programVk == nullptr)
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }
    ANGLE_TRY(programVk->init());
    *programOut = CLProgramImpl::Ptr(std::move(programVk));

    return angle::Result::Continue;
}

angle::Result CLContextVk::createProgramWithIL(const cl::Program &program,
                                               const void *il,
                                               size_t length,
                                               CLProgramImpl::Ptr *programOut)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

angle::Result CLContextVk::createProgramWithBinary(const cl::Program &program,
                                                   const size_t *lengths,
                                                   const unsigned char **binaries,
                                                   cl_int *binaryStatus,
                                                   CLProgramImpl::Ptr *programOut)
{
    CLProgramVk *programVk = new (std::nothrow) CLProgramVk(program);
    if (programVk == nullptr)
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }
    ANGLE_TRY(programVk->init(lengths, binaries, binaryStatus));
    *programOut = CLProgramImpl::Ptr(std::move(programVk));

    return angle::Result::Continue;
}

angle::Result CLContextVk::createProgramWithBuiltInKernels(const cl::Program &program,
                                                           const char *kernel_names,
                                                           CLProgramImpl::Ptr *programOut)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

angle::Result CLContextVk::linkProgram(const cl::Program &program,
                                       const cl::DevicePtrs &devices,
                                       const char *options,
                                       const cl::ProgramPtrs &inputPrograms,
                                       cl::Program *notify,
                                       CLProgramImpl::Ptr *programOut)
{
    const cl::DevicePtrs &devicePtrs = !devices.empty() ? devices : mContext.getDevices();

    CLProgramVk::Ptr programImpl = CLProgramVk::Ptr(new (std::nothrow) CLProgramVk(program));
    if (programImpl == nullptr)
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }
    ANGLE_TRY(programImpl->init());

    cl::DevicePtrs linkDeviceList;
    CLProgramVk::LinkProgramsList linkProgramsList;
    cl::BitField libraryOrObject(CL_PROGRAM_BINARY_TYPE_LIBRARY |
                                 CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT);
    for (const cl::DevicePtr &devicePtr : devicePtrs)
    {
        CLProgramVk::LinkPrograms linkPrograms;
        for (const cl::ProgramPtr &inputProgram : inputPrograms)
        {
            const CLProgramVk::DeviceProgramData *deviceProgramData =
                inputProgram->getImpl<CLProgramVk>().getDeviceProgramData(devicePtr->getNative());

            // Should be valid at this point
            ASSERT(deviceProgramData != nullptr);

            if (libraryOrObject.intersects(deviceProgramData->binaryType))
            {
                linkPrograms.push_back(deviceProgramData);
            }
        }
        if (!linkPrograms.empty())
        {
            linkDeviceList.push_back(devicePtr);
            linkProgramsList.push_back(linkPrograms);
        }
    }

    programImpl->setBuildStatus(linkDeviceList, CL_BUILD_IN_PROGRESS);

    // Perform link
    if (notify)
    {
        std::shared_ptr<angle::WaitableEvent> asyncEvent =
            mContext.getPlatform().getMultiThreadPool()->postWorkerTask(
                std::make_shared<CLAsyncBuildTask>(
                    programImpl.get(), linkDeviceList, std::string(options ? options : ""), "",
                    CLProgramVk::BuildType::LINK, linkProgramsList, notify));
        ASSERT(asyncEvent != nullptr);
    }
    else
    {
        if (!programImpl->buildInternal(linkDeviceList, std::string(options ? options : ""), "",
                                        CLProgramVk::BuildType::LINK, linkProgramsList))
        {
            ANGLE_CL_RETURN_ERROR(CL_LINK_PROGRAM_FAILURE);
        }
    }

    *programOut = std::move(programImpl);
    return angle::Result::Continue;
}

angle::Result CLContextVk::createUserEvent(const cl::Event &event, CLEventImpl::Ptr *eventOut)
{
    *eventOut = CLEventImpl::Ptr(new (std::nothrow) CLEventVk(event));
    if (*eventOut == nullptr)
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }
    return angle::Result::Continue;
}

angle::Result CLContextVk::waitForEvents(const cl::EventPtrs &events)
{
    for (auto &event : events)
    {
        CLEventVk *eventVk = &event.get()->getImpl<CLEventVk>();
        if (eventVk->isUserEvent())
        {
            ANGLE_TRY(eventVk->waitForUserEventStatus());
        }
        else
        {
            // TODO rework this to instead (flush w/ ResourceUse serial wait) once we move away from
            // spawning a submit-thread/Task for flush routine
            // https://anglebug.com/42267107
            ANGLE_TRY(event->getCommandQueue()->finish());
        }
    }

    return angle::Result::Continue;
}

angle::Result CLContextVk::allocateDescriptorSet(
    CLKernelVk *kernelVk,
    DescriptorSetIndex index,
    angle::EnumIterator<DescriptorSetIndex> layoutIndex,
    vk::OutsideRenderPassCommandBufferHelper *computePassCommands)
{
    std::lock_guard<angle::SimpleMutex> lock(mDescriptorSetMutex);

    return kernelVk->allocateDescriptorSet(index, layoutIndex, computePassCommands);
}

}  // namespace rx
