//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLKernelVk.cpp: Implements the class methods for CLKernelVk.

#include "common/PackedEnums.h"

#include "libANGLE/renderer/vulkan/CLContextVk.h"
#include "libANGLE/renderer/vulkan/CLDeviceVk.h"
#include "libANGLE/renderer/vulkan/CLKernelVk.h"
#include "libANGLE/renderer/vulkan/CLMemoryVk.h"
#include "libANGLE/renderer/vulkan/CLProgramVk.h"
#include "libANGLE/renderer/vulkan/vk_wrapper.h"

#include "libANGLE/CLBuffer.h"
#include "libANGLE/CLContext.h"
#include "libANGLE/CLKernel.h"
#include "libANGLE/CLProgram.h"
#include "libANGLE/cl_utils.h"
#include "spirv/unified1/NonSemanticClspvReflection.h"

namespace rx
{

CLKernelVk::CLKernelVk(const cl::Kernel &kernel,
                       std::string &name,
                       std::string &attributes,
                       CLKernelArguments &args)
    : CLKernelImpl(kernel),
      mProgram(&kernel.getProgram().getImpl<CLProgramVk>()),
      mContext(&kernel.getProgram().getContext().getImpl<CLContextVk>()),
      mName(name),
      mAttributes(attributes),
      mArgs(args),
      mPodBuffer(nullptr)
{
    mShaderProgramHelper.setShader(gl::ShaderType::Compute,
                                   mKernel.getProgram().getImpl<CLProgramVk>().getShaderModule());
}

CLKernelVk::~CLKernelVk()
{
    mComputePipelineCache.destroy(mContext);
    mShaderProgramHelper.destroy(mContext->getRenderer());

    if (mPodBuffer)
    {
        // mPodBuffer assignment will make newly created buffer
        // return refcount of 2, so need to release by 1
        mPodBuffer->release();
    }
}

angle::Result CLKernelVk::init()
{
    vk::DescriptorSetLayoutDesc &descriptorSetLayoutDesc =
        mDescriptorSetLayoutDescs[DescriptorSetIndex::KernelArguments];
    VkPushConstantRange pcRange = mProgram->getDeviceProgramData(mName.c_str())->pushConstRange;
    size_t podBufferSize        = 0;

    bool podFound = false;
    for (const auto &arg : getArgs())
    {
        VkDescriptorType descType = VK_DESCRIPTOR_TYPE_MAX_ENUM;
        switch (arg.type)
        {
            case NonSemanticClspvReflectionArgumentStorageBuffer:
                descType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                break;
            case NonSemanticClspvReflectionArgumentUniform:
            case NonSemanticClspvReflectionArgumentPointerUniform:
                descType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                break;
            case NonSemanticClspvReflectionArgumentPodUniform:
            case NonSemanticClspvReflectionArgumentPodStorageBuffer:
            {
                uint32_t newPodBufferSize = arg.podStorageBufferOffset + arg.podStorageBufferSize;
                podBufferSize = newPodBufferSize > podBufferSize ? newPodBufferSize : podBufferSize;
                if (podFound)
                {
                    continue;
                }
                descType = arg.type == NonSemanticClspvReflectionArgumentPodUniform
                               ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
                               : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                podFound = true;
                break;
            }
            case NonSemanticClspvReflectionArgumentPodPushConstant:
                // Get existing push constant range and see if we need to update
                if (arg.pushConstOffset + arg.pushConstantSize > pcRange.offset + pcRange.size)
                {
                    pcRange.size = arg.pushConstOffset + arg.pushConstantSize - pcRange.offset;
                }
                continue;
            case NonSemanticClspvReflectionArgumentSampledImage:
                descType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                break;
            case NonSemanticClspvReflectionArgumentStorageImage:
                descType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                break;
            case NonSemanticClspvReflectionArgumentSampler:
                descType = VK_DESCRIPTOR_TYPE_SAMPLER;
                break;
            case NonSemanticClspvReflectionArgumentStorageTexelBuffer:
                descType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
                break;
            case NonSemanticClspvReflectionArgumentUniformTexelBuffer:
                descType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
                break;
            default:
                continue;
        }
        if (descType != VK_DESCRIPTOR_TYPE_MAX_ENUM)
        {
            descriptorSetLayoutDesc.addBinding(arg.descriptorBinding, descType, 1,
                                               VK_SHADER_STAGE_COMPUTE_BIT, nullptr);
        }
    }

    if (podBufferSize > 0)
    {
        mPodBuffer =
            cl::MemoryPtr(cl::Buffer::Cast(this->mContext->getFrontendObject().createBuffer(
                nullptr, cl::MemFlags(CL_MEM_READ_ONLY), podBufferSize, nullptr)));
    }

    if (usesPrintf())
    {
        mDescriptorSetLayoutDescs[DescriptorSetIndex::Printf].addBinding(
            mProgram->getDeviceProgramData(mName.c_str())
                ->reflectionData.printfBufferStorage.binding,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr);
    }

    // Get pipeline layout from cache (creates if missed)
    // A given kernel need not have resulted in use of all the descriptor sets. Unless the
    // graphicsPipelineLibrary extension is supported, the pipeline layout need all the descriptor
    // set layouts to be valide. So set them up in the order of their occurrence.
    mPipelineLayoutDesc = {};
    for (DescriptorSetIndex index : angle::AllEnums<DescriptorSetIndex>())
    {
        if (!mDescriptorSetLayoutDescs[index].empty())
        {
            mPipelineLayoutDesc.updateDescriptorSetLayout(index, mDescriptorSetLayoutDescs[index]);
        }
    }

    // push constant setup
    // push constant size must be multiple of 4
    pcRange.size = roundUpPow2(pcRange.size, 4u);
    mPodArgumentPushConstants.resize(pcRange.size);

    // push constant offset must be multiple of 4, round down to ensure this
    pcRange.offset = roundDownPow2(pcRange.offset, 4u);

    mPipelineLayoutDesc.updatePushConstantRange(pcRange.stageFlags, pcRange.offset, pcRange.size);

    // initialize the descriptor pools
    // descriptor pools are setup as per their indices
    return initializeDescriptorPools();
}

angle::Result CLKernelVk::setArg(cl_uint argIndex, size_t argSize, const void *argValue)
{
    auto &arg = mArgs.at(argIndex);
    if (arg.used)
    {
        switch (arg.type)
        {
            case NonSemanticClspvReflectionArgumentPodPushConstant:
                ASSERT(mPodArgumentPushConstants.size() >=
                       arg.pushConstantSize + arg.pushConstOffset);
                arg.handle     = &mPodArgumentPushConstants[arg.pushConstOffset];
                arg.handleSize = argSize;
                if (argSize > 0 && argValue != nullptr)
                {
                    // Copy the contents since app is free to delete/reassign the contents after
                    memcpy(arg.handle, argValue, arg.handleSize);
                }
                break;
            case NonSemanticClspvReflectionArgumentPodUniform:
            case NonSemanticClspvReflectionArgumentPodStorageBuffer:
                ASSERT(mPodBuffer->getSize() >= argSize + arg.podUniformOffset);
                if (argSize > 0 && argValue != nullptr)
                {
                    ANGLE_TRY(mPodBuffer->getImpl<CLBufferVk>().copyFrom(
                        argValue, arg.podStorageBufferOffset, argSize));
                }
                break;
            case NonSemanticClspvReflectionArgumentWorkgroup:
                ASSERT(arg.workgroupSize != 0);
                mSpecConstants.push_back(
                    KernelSpecConstant{.ID   = arg.workgroupSpecId,
                                       .data = static_cast<uint32_t>(argSize / arg.workgroupSize)});
                break;
            case NonSemanticClspvReflectionArgumentUniform:
            case NonSemanticClspvReflectionArgumentStorageBuffer:
            case NonSemanticClspvReflectionArgumentStorageImage:
            case NonSemanticClspvReflectionArgumentSampledImage:
            case NonSemanticClspvReflectionArgumentUniformTexelBuffer:
            case NonSemanticClspvReflectionArgumentStorageTexelBuffer:
                ASSERT(argSize == sizeof(cl_mem *));
                arg.handle     = *static_cast<const cl_mem *>(argValue);
                arg.handleSize = argSize;
                break;
            default:
                // Just store ptr and size (if we end up here)
                arg.handle     = const_cast<void *>(argValue);
                arg.handleSize = argSize;
                break;
        }
    }

    return angle::Result::Continue;
}

angle::Result CLKernelVk::createInfo(CLKernelImpl::Info *info) const
{
    info->functionName = mName;
    info->attributes   = mAttributes;
    info->numArgs      = static_cast<cl_uint>(mArgs.size());
    for (const auto &arg : mArgs)
    {
        ArgInfo argInfo;
        argInfo.name             = arg.info.name;
        argInfo.typeName         = arg.info.typeName;
        argInfo.accessQualifier  = arg.info.accessQualifier;
        argInfo.addressQualifier = arg.info.addressQualifier;
        argInfo.typeQualifier    = arg.info.typeQualifier;
        info->args.push_back(std::move(argInfo));
    }

    auto &ctx = mKernel.getProgram().getContext();
    info->workGroups.resize(ctx.getDevices().size());
    const CLProgramVk::DeviceProgramData *deviceProgramData = nullptr;
    for (auto i = 0u; i < ctx.getDevices().size(); ++i)
    {
        auto &workGroup     = info->workGroups[i];
        const auto deviceVk = &ctx.getDevices()[i]->getImpl<CLDeviceVk>();
        deviceProgramData   = mProgram->getDeviceProgramData(ctx.getDevices()[i]->getNative());
        if (deviceProgramData == nullptr)
        {
            continue;
        }

        // TODO: http://anglebug.com/42267005
        ANGLE_TRY(
            deviceVk->getInfoSizeT(cl::DeviceInfo::MaxWorkGroupSize, &workGroup.workGroupSize));

        // TODO: http://anglebug.com/42267004
        workGroup.privateMemSize = 0;
        workGroup.localMemSize   = 0;

        workGroup.prefWorkGroupSizeMultiple = 16u;
        workGroup.globalWorkSize            = {0, 0, 0};
        if (deviceProgramData->reflectionData.kernelCompileWorkgroupSize.contains(mName))
        {
            workGroup.compileWorkGroupSize = {
                deviceProgramData->reflectionData.kernelCompileWorkgroupSize.at(mName)[0],
                deviceProgramData->reflectionData.kernelCompileWorkgroupSize.at(mName)[1],
                deviceProgramData->reflectionData.kernelCompileWorkgroupSize.at(mName)[2]};
        }
        else
        {
            workGroup.compileWorkGroupSize = {0, 0, 0};
        }
    }

    return angle::Result::Continue;
}

angle::Result CLKernelVk::initPipelineLayout()
{
    PipelineLayoutCache *pipelineLayoutCache = mContext->getPipelineLayoutCache();
    return pipelineLayoutCache->getPipelineLayout(mContext, mPipelineLayoutDesc,
                                                  mDescriptorSetLayouts, &mPipelineLayout);
}

angle::Result CLKernelVk::getOrCreateComputePipeline(vk::PipelineCacheAccess *pipelineCache,
                                                     const cl::NDRange &ndrange,
                                                     const cl::Device &device,
                                                     vk::PipelineHelper **pipelineOut)
{
    const CLProgramVk::DeviceProgramData *devProgramData =
        getProgram()->getDeviceProgramData(device.getNative());
    ASSERT(devProgramData != nullptr);

    // Populate program specialization constants (if any)
    uint32_t constantDataOffset = 0;
    std::vector<uint32_t> specConstantData;
    std::vector<VkSpecializationMapEntry> mapEntries;
    for (const auto specConstantUsed : devProgramData->reflectionData.specConstantsUsed)
    {
        switch (specConstantUsed)
        {
            case SpecConstantType::WorkDimension:
                specConstantData.push_back(ndrange.workDimensions);
                break;
            case SpecConstantType::WorkgroupSizeX:
                specConstantData.push_back(ndrange.localWorkSize[0]);
                break;
            case SpecConstantType::WorkgroupSizeY:
                specConstantData.push_back(ndrange.localWorkSize[1]);
                break;
            case SpecConstantType::WorkgroupSizeZ:
                specConstantData.push_back(ndrange.localWorkSize[2]);
                break;
            case SpecConstantType::GlobalOffsetX:
                specConstantData.push_back(ndrange.globalWorkOffset[0]);
                break;
            case SpecConstantType::GlobalOffsetY:
                specConstantData.push_back(ndrange.globalWorkOffset[1]);
                break;
            case SpecConstantType::GlobalOffsetZ:
                specConstantData.push_back(ndrange.globalWorkOffset[2]);
                break;
            default:
                UNIMPLEMENTED();
                continue;
        }
        mapEntries.push_back(VkSpecializationMapEntry{
            .constantID = devProgramData->reflectionData.specConstantIDs[specConstantUsed],
            .offset     = constantDataOffset,
            .size       = sizeof(uint32_t)});
        constantDataOffset += sizeof(uint32_t);
    }
    // Populate kernel specialization constants (if any)
    for (const auto &specConstant : mSpecConstants)
    {
        specConstantData.push_back(specConstant.data);
        mapEntries.push_back(VkSpecializationMapEntry{
            .constantID = specConstant.ID, .offset = constantDataOffset, .size = sizeof(uint32_t)});
        constantDataOffset += sizeof(uint32_t);
    }
    VkSpecializationInfo computeSpecializationInfo{
        .mapEntryCount = static_cast<uint32_t>(mapEntries.size()),
        .pMapEntries   = mapEntries.data(),
        .dataSize      = specConstantData.size() * sizeof(uint32_t),
        .pData         = specConstantData.data(),
    };

    // Now get or create (on compute pipeline cache miss) compute pipeline and return it
    vk::ComputePipelineOptions options = vk::GetComputePipelineOptions(
        vk::PipelineRobustness::NonRobust, vk::PipelineProtectedAccess::Unprotected);
    return mShaderProgramHelper.getOrCreateComputePipeline(
        mContext, &mComputePipelineCache, pipelineCache, getPipelineLayout(), options,
        PipelineSource::Draw, pipelineOut, mName.c_str(), &computeSpecializationInfo);
}

bool CLKernelVk::usesPrintf() const
{
    return mProgram->getDeviceProgramData(mName.c_str())->getKernelFlags(mName) &
           NonSemanticClspvReflectionMayUsePrintf;
}

angle::Result CLKernelVk::initializeDescriptorPools()
{
    for (DescriptorSetIndex index : angle::AllEnums<DescriptorSetIndex>())
    {
        if (!mDescriptorSetLayoutDescs[index].empty())
        {
            ANGLE_TRY(mContext->getMetaDescriptorPool().bindCachedDescriptorPool(
                mContext, mDescriptorSetLayoutDescs[index], 1,
                mContext->getDescriptorSetLayoutCache(), &mDynamicDescriptorPools[index]));
        }
    }
    return angle::Result::Continue;
}

angle::Result CLKernelVk::allocateDescriptorSet(
    DescriptorSetIndex index,
    angle::EnumIterator<DescriptorSetIndex> layoutIndex,
    vk::OutsideRenderPassCommandBufferHelper *computePassCommands)
{
    if (mDescriptorSets[index] && mDescriptorSets[index]->valid())
    {
        if (mDescriptorSets[index]->usedByCommandBuffer(computePassCommands->getQueueSerial()))
        {
            mDescriptorSets[index].reset();
        }
        else
        {
            return angle::Result::Continue;
        }
    }

    if (mDynamicDescriptorPools[index]->valid())
    {
        ANGLE_TRY(mDynamicDescriptorPools[index]->allocateDescriptorSet(
            mContext, *mDescriptorSetLayouts[*layoutIndex], &mDescriptorSets[index]));
        computePassCommands->retainResource(mDescriptorSets[index].get());
    }

    return angle::Result::Continue;
}
}  // namespace rx
