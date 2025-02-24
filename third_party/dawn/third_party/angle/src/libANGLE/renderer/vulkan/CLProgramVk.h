//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLProgramVk.h: Defines the class interface for CLProgramVk, implementing CLProgramImpl.

#ifndef LIBANGLE_RENDERER_VULKAN_CLPROGRAMVK_H_
#define LIBANGLE_RENDERER_VULKAN_CLPROGRAMVK_H_

#include <cstdint>

#include "common/SimpleMutex.h"
#include "common/hash_containers.h"

#include "libANGLE/renderer/vulkan/CLContextVk.h"
#include "libANGLE/renderer/vulkan/CLKernelVk.h"
#include "libANGLE/renderer/vulkan/cl_types.h"
#include "libANGLE/renderer/vulkan/clspv_utils.h"
#include "libANGLE/renderer/vulkan/vk_cache_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

#include "libANGLE/renderer/CLProgramImpl.h"

#include "libANGLE/CLProgram.h"

#include "clspv/Compiler.h"

#include "vulkan/vulkan_core.h"

#include "spirv/unified1/NonSemanticClspvReflection.h"

namespace rx
{

class CLProgramVk : public CLProgramImpl
{
  public:
    using Ptr = std::unique_ptr<CLProgramVk>;
    // TODO: Look into moving this information in CLKernelArgument
    // https://anglebug.com/378514267
    struct ImagePushConstant
    {
        VkPushConstantRange pcRange;
        uint32_t ordinal;
    };
    struct SpvReflectionData
    {
        angle::HashMap<uint32_t, uint32_t> spvIntLookup;
        angle::HashMap<uint32_t, std::string> spvStrLookup;
        angle::HashMap<uint32_t, CLKernelVk::ArgInfo> kernelArgInfos;
        angle::HashMap<std::string, uint32_t> kernelFlags;
        angle::HashMap<std::string, std::string> kernelAttributes;
        angle::HashMap<std::string, std::array<uint32_t, 3>> kernelCompileWorkgroupSize;
        angle::HashMap<uint32_t, VkPushConstantRange> pushConstants;
        angle::PackedEnumMap<SpecConstantType, uint32_t> specConstantIDs;
        angle::PackedEnumBitSet<SpecConstantType, uint32_t> specConstantsUsed;
        angle::HashMap<uint32_t, std::vector<ImagePushConstant>> imagePushConstants;
        CLKernelArgsMap kernelArgsMap;
        angle::HashMap<std::string, CLKernelArgument> kernelArgMap;
        angle::HashSet<uint32_t> kernelIDs;
        ClspvPrintfBufferStorage printfBufferStorage;
        angle::HashMap<uint32_t, ClspvPrintfInfo> printfInfoMap;
    };

    // Output binary structure (for CL_PROGRAM_BINARIES query)
    static constexpr uint32_t kBinaryVersion = 2;
    struct ProgramBinaryOutputHeader
    {
        uint32_t headerVersion{kBinaryVersion};
        cl_program_binary_type binaryType{CL_PROGRAM_BINARY_TYPE_NONE};
        cl_build_status buildStatus{CL_BUILD_NONE};
    };

    struct ScopedClspvContext : angle::NonCopyable
    {
        ScopedClspvContext() = default;
        ~ScopedClspvContext() { clspvFreeOutputBuildObjs(mOutputBin, mOutputBuildLog); }

        size_t mOutputBinSize{0};
        char *mOutputBin{nullptr};
        char *mOutputBuildLog{nullptr};
    };

    struct ScopedProgramCallback : angle::NonCopyable
    {
        ScopedProgramCallback() = delete;
        ScopedProgramCallback(cl::Program *notify) : mNotify(notify) {}
        ~ScopedProgramCallback()
        {
            if (mNotify)
            {
                mNotify->callback();
            }
        }

        cl::Program *mNotify{nullptr};
    };

    enum class BuildType
    {
        BUILD = 0,
        COMPILE,
        LINK,
        BINARY
    };

    struct DeviceProgramData
    {
        std::vector<char> IR;
        std::string buildLog;
        angle::spirv::Blob binary;
        SpvReflectionData reflectionData;
        VkPushConstantRange pushConstRange{};
        cl_build_status buildStatus{CL_BUILD_NONE};
        cl_program_binary_type binaryType{CL_PROGRAM_BINARY_TYPE_NONE};
        spv_target_env spirvVersion;

        size_t numKernels() const { return reflectionData.kernelArgsMap.size(); }

        size_t numKernelArgs(const std::string &kernelName) const
        {
            return containsKernel(kernelName) ? getKernelArgsMap().at(kernelName).size() : 0;
        }

        const CLKernelArgsMap &getKernelArgsMap() const { return reflectionData.kernelArgsMap; }

        bool containsKernel(const std::string &name) const
        {
            return reflectionData.kernelArgsMap.contains(name);
        }

        std::string getKernelNames() const
        {
            std::string names;
            for (auto name = getKernelArgsMap().begin(); name != getKernelArgsMap().end(); ++name)
            {
                names += name->first + (std::next(name) != getKernelArgsMap().end() ? ";" : "\0");
            }
            return names;
        }

        uint32_t getKernelFlags(const std::string &kernelName) const
        {
            if (containsKernel(kernelName))
            {
                return reflectionData.kernelFlags.at(kernelName);
            }
            return 0;
        }

        CLKernelArguments getKernelArguments(const std::string &kernelName) const
        {
            CLKernelArguments kargsCopy;
            if (containsKernel(kernelName))
            {
                const CLKernelArguments &kargs = getKernelArgsMap().at(kernelName);
                for (const CLKernelArgument &karg : kargs)
                {
                    kargsCopy.push_back(karg);
                }
            }
            return kargsCopy;
        }

        cl::WorkgroupSize getCompiledWorkgroupSize(const std::string &kernelName) const
        {
            cl::WorkgroupSize compiledWorkgroupSize{0, 0, 0};
            if (reflectionData.kernelCompileWorkgroupSize.contains(kernelName))
            {
                for (size_t i = 0; i < compiledWorkgroupSize.size(); ++i)
                {
                    compiledWorkgroupSize[i] =
                        reflectionData.kernelCompileWorkgroupSize.at(kernelName)[i];
                }
            }
            return compiledWorkgroupSize;
        }

        std::string getKernelAttributes(const std::string &kernelName) const
        {
            if (containsKernel(kernelName))
            {
                return reflectionData.kernelAttributes.at(kernelName.c_str());
            }
            return std::string{};
        }

        const VkPushConstantRange *getPushConstantRangeFromClspvReflectionType(
            NonSemanticClspvReflectionInstructions type) const
        {
            const VkPushConstantRange *pushConstantRangePtr = nullptr;
            if (reflectionData.pushConstants.contains(type))
            {
                pushConstantRangePtr = &reflectionData.pushConstants.at(type);
            }
            return pushConstantRangePtr;
        }

        inline const VkPushConstantRange *getGlobalOffsetRange() const
        {
            return getPushConstantRangeFromClspvReflectionType(
                NonSemanticClspvReflectionPushConstantGlobalOffset);
        }

        inline const VkPushConstantRange *getGlobalSizeRange() const
        {
            return getPushConstantRangeFromClspvReflectionType(
                NonSemanticClspvReflectionPushConstantGlobalSize);
        }

        inline const VkPushConstantRange *getEnqueuedLocalSizeRange() const
        {
            return getPushConstantRangeFromClspvReflectionType(
                NonSemanticClspvReflectionPushConstantEnqueuedLocalSize);
        }

        inline const VkPushConstantRange *getNumWorkgroupsRange() const
        {
            return getPushConstantRangeFromClspvReflectionType(
                NonSemanticClspvReflectionPushConstantNumWorkgroups);
        }

        inline const VkPushConstantRange *getRegionOffsetRange() const
        {
            return getPushConstantRangeFromClspvReflectionType(
                NonSemanticClspvReflectionPushConstantRegionOffset);
        }

        inline const VkPushConstantRange *getRegionGroupOffsetRange() const
        {
            return getPushConstantRangeFromClspvReflectionType(
                NonSemanticClspvReflectionPushConstantRegionGroupOffset);
        }

        const VkPushConstantRange *getImageDataChannelOrderRange(size_t ordinal) const
        {
            const VkPushConstantRange *pushConstantRangePtr = nullptr;
            if (reflectionData.imagePushConstants.contains(
                    NonSemanticClspvReflectionImageArgumentInfoChannelOrderPushConstant))
            {
                for (const auto &imageConstant : reflectionData.imagePushConstants.at(
                         NonSemanticClspvReflectionImageArgumentInfoChannelOrderPushConstant))
                {
                    if (static_cast<size_t>(imageConstant.ordinal) == ordinal)
                    {
                        pushConstantRangePtr = &imageConstant.pcRange;
                    }
                }
            }
            return pushConstantRangePtr;
        }

        const VkPushConstantRange *getImageDataChannelDataTypeRange(size_t ordinal) const
        {
            const VkPushConstantRange *pushConstantRangePtr = nullptr;
            if (reflectionData.imagePushConstants.contains(
                    NonSemanticClspvReflectionImageArgumentInfoChannelDataTypePushConstant))
            {
                for (const auto &imageConstant : reflectionData.imagePushConstants.at(
                         NonSemanticClspvReflectionImageArgumentInfoChannelDataTypePushConstant))
                {
                    if (static_cast<size_t>(imageConstant.ordinal) == ordinal)
                    {
                        pushConstantRangePtr = &imageConstant.pcRange;
                    }
                }
            }
            return pushConstantRangePtr;
        }

        const VkPushConstantRange *getNormalizedSamplerMaskRange(size_t ordinal) const
        {
            const VkPushConstantRange *pushConstantRangePtr = nullptr;
            if (reflectionData.imagePushConstants.contains(
                    NonSemanticClspvReflectionNormalizedSamplerMaskPushConstant))
            {
                for (const auto &imageConstant : reflectionData.imagePushConstants.at(
                         NonSemanticClspvReflectionNormalizedSamplerMaskPushConstant))
                {
                    if (static_cast<size_t>(imageConstant.ordinal) == ordinal)
                    {
                        pushConstantRangePtr = &imageConstant.pcRange;
                    }
                }
            }
            return pushConstantRangePtr;
        }
    };
    using DevicePrograms   = angle::HashMap<const _cl_device_id *, DeviceProgramData>;
    using LinkPrograms     = std::vector<const DeviceProgramData *>;
    using LinkProgramsList = std::vector<LinkPrograms>;

    CLProgramVk(const cl::Program &program);

    ~CLProgramVk() override;

    angle::Result init();
    angle::Result init(const size_t *lengths, const unsigned char **binaries, cl_int *binaryStatus);

    angle::Result build(const cl::DevicePtrs &devices,
                        const char *options,
                        cl::Program *notify) override;

    angle::Result compile(const cl::DevicePtrs &devices,
                          const char *options,
                          const cl::ProgramPtrs &inputHeaders,
                          const char **headerIncludeNames,
                          cl::Program *notify) override;

    angle::Result getInfo(cl::ProgramInfo name,
                          size_t valueSize,
                          void *value,
                          size_t *valueSizeRet) const override;

    angle::Result getBuildInfo(const cl::Device &device,
                               cl::ProgramBuildInfo name,
                               size_t valueSize,
                               void *value,
                               size_t *valueSizeRet) const override;

    angle::Result createKernel(const cl::Kernel &kernel,
                               const char *name,
                               CLKernelImpl::Ptr *kernelOut) override;

    angle::Result createKernels(cl_uint numKernels,
                                CLKernelImpl::CreateFuncs &createFuncs,
                                cl_uint *numKernelsRet) override;

    const DeviceProgramData *getDeviceProgramData(const char *kernelName) const;
    const DeviceProgramData *getDeviceProgramData(const _cl_device_id *device) const;
    CLPlatformVk *getPlatform() { return mContext->getPlatform(); }
    const vk::ShaderModulePtr &getShaderModule() const { return mShader; }

    bool buildInternal(const cl::DevicePtrs &devices,
                       std::string options,
                       std::string internalOptions,
                       BuildType buildType,
                       const LinkProgramsList &LinkProgramsList);
    angle::spirv::Blob stripReflection(const DeviceProgramData *deviceProgramData);

    // Sets the status for given associated device programs
    void setBuildStatus(const cl::DevicePtrs &devices, cl_build_status status);

    const angle::HashMap<uint32_t, ClspvPrintfInfo> *getPrintfDescriptors(
        const std::string &kernelName) const;

  private:
    CLContextVk *mContext;
    std::string mProgramOpts;
    vk::ShaderModulePtr mShader;
    DevicePrograms mAssociatedDevicePrograms;
    angle::SimpleMutex mProgramMutex;

    std::shared_ptr<angle::WaitableEvent> mAsyncBuildEvent;
};

class CLAsyncBuildTask : public angle::Closure
{
  public:
    CLAsyncBuildTask(CLProgramVk *programVk,
                     const cl::DevicePtrs &devices,
                     std::string options,
                     std::string internalOptions,
                     CLProgramVk::BuildType buildType,
                     const CLProgramVk::LinkProgramsList &LinkProgramsList,
                     cl::Program *notify)
        : mProgramVk(programVk),
          mDevices(devices),
          mOptions(options),
          mInternalOptions(internalOptions),
          mBuildType(buildType),
          mLinkProgramsList(LinkProgramsList),
          mNotify(notify)
    {}

    void operator()() override;

  private:
    CLProgramVk *mProgramVk;
    const cl::DevicePtrs mDevices;
    std::string mOptions;
    std::string mInternalOptions;
    CLProgramVk::BuildType mBuildType;
    const CLProgramVk::LinkProgramsList mLinkProgramsList;
    cl::Program *mNotify;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_CLPROGRAMVK_H_
