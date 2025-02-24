//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLProgramVk.cpp: Implements the class methods for CLProgramVk.

#include "libANGLE/renderer/vulkan/CLProgramVk.h"
#include "libANGLE/renderer/vulkan/CLContextVk.h"
#include "libANGLE/renderer/vulkan/clspv_utils.h"
#include "libANGLE/renderer/vulkan/vk_cache_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

#include "libANGLE/CLContext.h"
#include "libANGLE/CLKernel.h"
#include "libANGLE/CLProgram.h"
#include "libANGLE/cl_utils.h"

#include "common/log_utils.h"
#include "common/string_utils.h"
#include "common/system_utils.h"

#include "clspv/Compiler.h"

#include "spirv/unified1/NonSemanticClspvReflection.h"
#include "spirv/unified1/spirv.hpp"

#include "spirv-tools/libspirv.hpp"
#include "spirv-tools/optimizer.hpp"

namespace rx
{

namespace
{
#if defined(ANGLE_ENABLE_ASSERTS)
constexpr bool kAngleDebug = true;
#else
constexpr bool kAngleDebug = false;
#endif

// Used by SPIRV-Tools to parse reflection info
spv_result_t ParseReflection(CLProgramVk::SpvReflectionData &reflectionData,
                             const spv_parsed_instruction_t &spvInstr)
{
    // Parse spir-v opcodes
    switch (spvInstr.opcode)
    {
        // --- Clspv specific parsing for below cases ---
        case spv::OpExtInst:
        {
            if (spvInstr.ext_inst_type != SPV_EXT_INST_TYPE_NONSEMANTIC_CLSPVREFLECTION)
            {
                break;
            }
            switch (spvInstr.words[4])
            {
                case NonSemanticClspvReflectionKernel:
                {
                    // Extract kernel name and args - add to kernel args map
                    std::string functionName = reflectionData.spvStrLookup[spvInstr.words[6]];
                    uint32_t numArgs         = reflectionData.spvIntLookup[spvInstr.words[7]];
                    reflectionData.kernelArgsMap[functionName] = CLKernelArguments();
                    reflectionData.kernelArgsMap[functionName].resize(numArgs);

                    // Store kernel flags and attributes
                    reflectionData.kernelFlags[functionName] =
                        reflectionData.spvIntLookup[spvInstr.words[8]];
                    reflectionData.kernelAttributes[functionName] =
                        reflectionData.spvStrLookup[spvInstr.words[9]];

                    // Save kernel name to reflection table for later use/lookup in parser routine
                    reflectionData.kernelIDs.insert(spvInstr.words[2]);
                    reflectionData.spvStrLookup[spvInstr.words[2]] = std::string(functionName);

                    // If we already parsed some args ahead of time, populate them now
                    if (reflectionData.kernelArgMap.contains(functionName))
                    {
                        for (const auto &arg : reflectionData.kernelArgMap)
                        {
                            uint32_t ordinal = arg.second.ordinal;
                            reflectionData.kernelArgsMap[functionName].at(ordinal) =
                                std::move(arg.second);
                        }
                    }
                    break;
                }
                case NonSemanticClspvReflectionArgumentInfo:
                {
                    CLKernelVk::ArgInfo kernelArgInfo;
                    kernelArgInfo.name = reflectionData.spvStrLookup[spvInstr.words[5]];
                    // If instruction has more than 5 instruction operands (minus instruction
                    // name/opcode), that means we have arg qualifiers. ArgumentInfo also counts as
                    // an operand for OpExtInst. In below example, [ %e %f %g %h ] are the arg
                    // qualifier operands.
                    //
                    // %a = OpExtInst %b %c ArgumentInfo %d [ %e %f %g %h ]
                    if (spvInstr.num_operands > 5)
                    {
                        kernelArgInfo.typeName = reflectionData.spvStrLookup[spvInstr.words[6]];
                        kernelArgInfo.addressQualifier =
                            reflectionData.spvIntLookup[spvInstr.words[7]];
                        kernelArgInfo.accessQualifier =
                            reflectionData.spvIntLookup[spvInstr.words[8]];
                        kernelArgInfo.typeQualifier =
                            reflectionData.spvIntLookup[spvInstr.words[9]];
                    }
                    // Store kern arg for later lookup
                    reflectionData.kernelArgInfos[spvInstr.words[2]] = std::move(kernelArgInfo);
                    break;
                }
                case NonSemanticClspvReflectionArgumentPodUniform:
                case NonSemanticClspvReflectionArgumentPointerUniform:
                case NonSemanticClspvReflectionArgumentPodStorageBuffer:
                {
                    CLKernelArgument kernelArg;
                    if (spvInstr.num_operands == 11)
                    {
                        const CLKernelVk::ArgInfo &kernelArgInfo =
                            reflectionData.kernelArgInfos[spvInstr.words[11]];
                        kernelArg.info.name             = kernelArgInfo.name;
                        kernelArg.info.typeName         = kernelArgInfo.typeName;
                        kernelArg.info.addressQualifier = kernelArgInfo.addressQualifier;
                        kernelArg.info.accessQualifier  = kernelArgInfo.accessQualifier;
                        kernelArg.info.typeQualifier    = kernelArgInfo.typeQualifier;
                    }
                    kernelArg.type    = spvInstr.words[4];
                    kernelArg.used    = true;
                    kernelArg.ordinal = reflectionData.spvIntLookup[spvInstr.words[6]];
                    kernelArg.op3     = reflectionData.spvIntLookup[spvInstr.words[7]];
                    kernelArg.op4     = reflectionData.spvIntLookup[spvInstr.words[8]];
                    kernelArg.op5     = reflectionData.spvIntLookup[spvInstr.words[9]];
                    kernelArg.op6     = reflectionData.spvIntLookup[spvInstr.words[10]];

                    if (reflectionData.kernelIDs.contains(spvInstr.words[5]))
                    {
                        CLKernelArguments &kernelArgs =
                            reflectionData
                                .kernelArgsMap[reflectionData.spvStrLookup[spvInstr.words[5]]];
                        kernelArgs.at(kernelArg.ordinal) = std::move(kernelArg);
                    }
                    else
                    {
                        // Reflection kernel not yet parsed, place in temp storage for now
                        reflectionData
                            .kernelArgMap[reflectionData.spvStrLookup[spvInstr.words[5]]] =
                            std::move(kernelArg);
                    }

                    break;
                }
                case NonSemanticClspvReflectionArgumentUniform:
                case NonSemanticClspvReflectionArgumentWorkgroup:
                case NonSemanticClspvReflectionArgumentSampler:
                case NonSemanticClspvReflectionArgumentStorageImage:
                case NonSemanticClspvReflectionArgumentSampledImage:
                case NonSemanticClspvReflectionArgumentStorageBuffer:
                case NonSemanticClspvReflectionArgumentStorageTexelBuffer:
                case NonSemanticClspvReflectionArgumentUniformTexelBuffer:
                case NonSemanticClspvReflectionArgumentPodPushConstant:
                case NonSemanticClspvReflectionArgumentPointerPushConstant:
                {
                    CLKernelArgument kernelArg;
                    if (spvInstr.num_operands == 9)
                    {
                        const CLKernelVk::ArgInfo &kernelArgInfo =
                            reflectionData.kernelArgInfos[spvInstr.words[9]];
                        kernelArg.info.name             = kernelArgInfo.name;
                        kernelArg.info.typeName         = kernelArgInfo.typeName;
                        kernelArg.info.addressQualifier = kernelArgInfo.addressQualifier;
                        kernelArg.info.accessQualifier  = kernelArgInfo.accessQualifier;
                        kernelArg.info.typeQualifier    = kernelArgInfo.typeQualifier;
                    }

                    kernelArg.type    = spvInstr.words[4];
                    kernelArg.used    = true;
                    kernelArg.ordinal = reflectionData.spvIntLookup[spvInstr.words[6]];
                    kernelArg.op3     = reflectionData.spvIntLookup[spvInstr.words[7]];
                    kernelArg.op4     = reflectionData.spvIntLookup[spvInstr.words[8]];

                    if (reflectionData.kernelIDs.contains(spvInstr.words[5]))
                    {
                        CLKernelArguments &kernelArgs =
                            reflectionData
                                .kernelArgsMap[reflectionData.spvStrLookup[spvInstr.words[5]]];
                        kernelArgs.at(kernelArg.ordinal) = std::move(kernelArg);
                    }
                    else
                    {
                        // Reflection kernel not yet parsed, place in temp storage for now
                        reflectionData
                            .kernelArgMap[reflectionData.spvStrLookup[spvInstr.words[5]]] =
                            std::move(kernelArg);
                    }
                    break;
                }
                case NonSemanticClspvReflectionPushConstantGlobalSize:
                case NonSemanticClspvReflectionPushConstantGlobalOffset:
                case NonSemanticClspvReflectionPushConstantRegionOffset:
                case NonSemanticClspvReflectionPushConstantNumWorkgroups:
                case NonSemanticClspvReflectionPushConstantRegionGroupOffset:
                case NonSemanticClspvReflectionPushConstantEnqueuedLocalSize:
                {
                    uint32_t offset = reflectionData.spvIntLookup[spvInstr.words[5]];
                    uint32_t size   = reflectionData.spvIntLookup[spvInstr.words[6]];
                    reflectionData.pushConstants[spvInstr.words[4]] = {
                        .stageFlags = 0, .offset = offset, .size = size};
                    break;
                }
                case NonSemanticClspvReflectionSpecConstantWorkgroupSize:
                {
                    reflectionData.specConstantIDs[SpecConstantType::WorkgroupSizeX] =
                        reflectionData.spvIntLookup[spvInstr.words[5]];
                    reflectionData.specConstantIDs[SpecConstantType::WorkgroupSizeY] =
                        reflectionData.spvIntLookup[spvInstr.words[6]];
                    reflectionData.specConstantIDs[SpecConstantType::WorkgroupSizeZ] =
                        reflectionData.spvIntLookup[spvInstr.words[7]];
                    reflectionData.specConstantsUsed[SpecConstantType::WorkgroupSizeX] = true;
                    reflectionData.specConstantsUsed[SpecConstantType::WorkgroupSizeY] = true;
                    reflectionData.specConstantsUsed[SpecConstantType::WorkgroupSizeZ] = true;
                    break;
                }
                case NonSemanticClspvReflectionPropertyRequiredWorkgroupSize:
                {
                    reflectionData.kernelCompileWorkgroupSize
                        [reflectionData.spvStrLookup[spvInstr.words[5]]] = {
                        reflectionData.spvIntLookup[spvInstr.words[6]],
                        reflectionData.spvIntLookup[spvInstr.words[7]],
                        reflectionData.spvIntLookup[spvInstr.words[8]]};
                    break;
                }
                case NonSemanticClspvReflectionSpecConstantWorkDim:
                {
                    reflectionData.specConstantIDs[SpecConstantType::WorkDimension] =
                        reflectionData.spvIntLookup[spvInstr.words[5]];
                    reflectionData.specConstantsUsed[SpecConstantType::WorkDimension] = true;
                    break;
                }
                case NonSemanticClspvReflectionSpecConstantGlobalOffset:
                    reflectionData.specConstantIDs[SpecConstantType::GlobalOffsetX] =
                        reflectionData.spvIntLookup[spvInstr.words[5]];
                    reflectionData.specConstantIDs[SpecConstantType::GlobalOffsetY] =
                        reflectionData.spvIntLookup[spvInstr.words[6]];
                    reflectionData.specConstantIDs[SpecConstantType::GlobalOffsetZ] =
                        reflectionData.spvIntLookup[spvInstr.words[7]];
                    reflectionData.specConstantsUsed[SpecConstantType::GlobalOffsetX] = true;
                    reflectionData.specConstantsUsed[SpecConstantType::GlobalOffsetY] = true;
                    reflectionData.specConstantsUsed[SpecConstantType::GlobalOffsetZ] = true;
                    break;
                case NonSemanticClspvReflectionPrintfInfo:
                {
                    // Info on the format string used in the builtin printf call in kernel
                    uint32_t printfID        = reflectionData.spvIntLookup[spvInstr.words[5]];
                    std::string formatString = reflectionData.spvStrLookup[spvInstr.words[6]];
                    reflectionData.printfInfoMap[printfID].id              = printfID;
                    reflectionData.printfInfoMap[printfID].formatSpecifier = formatString;
                    for (int i = 6; i < spvInstr.num_operands; i++)
                    {
                        uint16_t offset = spvInstr.operands[i].offset;
                        size_t size     = reflectionData.spvIntLookup[spvInstr.words[offset]];
                        reflectionData.printfInfoMap[printfID].argSizes.push_back(
                            static_cast<uint32_t>(size));
                    }

                    break;
                }
                case NonSemanticClspvReflectionPrintfBufferStorageBuffer:
                {
                    // Info about the printf storage buffer that contains the formatted content
                    uint32_t set     = reflectionData.spvIntLookup[spvInstr.words[5]];
                    uint32_t binding = reflectionData.spvIntLookup[spvInstr.words[6]];
                    uint32_t size    = reflectionData.spvIntLookup[spvInstr.words[7]];
                    reflectionData.printfBufferStorage = {set, binding, 0, size};
                    break;
                }
                case NonSemanticClspvReflectionPrintfBufferPointerPushConstant:
                {
                    ERR() << "Shouldn't be here. Support of printf builtin function is enabled "
                             "through "
                             "PrintfBufferStorageBuffer. Check optins passed down to clspv";
                    UNREACHABLE();
                    return SPV_UNSUPPORTED;
                }
                case NonSemanticClspvReflectionNormalizedSamplerMaskPushConstant:
                case NonSemanticClspvReflectionImageArgumentInfoChannelOrderPushConstant:
                case NonSemanticClspvReflectionImageArgumentInfoChannelDataTypePushConstant:
                {
                    uint32_t ordinal            = reflectionData.spvIntLookup[spvInstr.words[6]];
                    uint32_t offset             = reflectionData.spvIntLookup[spvInstr.words[7]];
                    uint32_t size               = reflectionData.spvIntLookup[spvInstr.words[8]];
                    VkPushConstantRange pcRange = {.stageFlags = 0, .offset = offset, .size = size};
                    reflectionData.imagePushConstants[spvInstr.words[4]].push_back(
                        {.pcRange = pcRange, .ordinal = ordinal});
                    break;
                }
                default:
                    break;
            }
            break;
        }
        // --- Regular SPIR-V opcode parsing for below cases ---
        case spv::OpString:
        {
            reflectionData.spvStrLookup[spvInstr.words[1]] =
                reinterpret_cast<const char *>(&spvInstr.words[2]);
            break;
        }
        case spv::OpConstant:
        {
            reflectionData.spvIntLookup[spvInstr.words[2]] = spvInstr.words[3];
            break;
        }
        default:
            break;
    }
    return SPV_SUCCESS;
}

std::string ProcessBuildOptions(const std::vector<std::string> &optionTokens,
                                CLProgramVk::BuildType buildType)
{
    std::string processedOptions;

    // Need to remove/replace options that are not 1-1 mapped to clspv
    for (const std::string &optionToken : optionTokens)
    {
        if (optionToken == "-create-library" && buildType == CLProgramVk::BuildType::LINK)
        {
            processedOptions += " --output-format=bc";
            continue;
        }
        processedOptions += " " + optionToken;
    }

    switch (buildType)
    {
        case CLProgramVk::BuildType::COMPILE:
            processedOptions += " --output-format=bc";
            break;
        case CLProgramVk::BuildType::LINK:
            processedOptions += " -x ir";
            break;
        default:
            break;
    }

    return processedOptions;
}

}  // namespace

void CLAsyncBuildTask::operator()()
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CLProgramVk::buildInternal (async)");
    CLProgramVk::ScopedProgramCallback spc(mNotify);
    if (!mProgramVk->buildInternal(mDevices, mOptions, mInternalOptions, mBuildType,
                                   mLinkProgramsList))
    {
        ERR() << "Async build failed for program (" << mProgramVk
              << ")! Check the build status or build log for details.";
    }
}

CLProgramVk::CLProgramVk(const cl::Program &program)
    : CLProgramImpl(program),
      mContext(&program.getContext().getImpl<CLContextVk>()),
      mAsyncBuildEvent(std::make_shared<angle::WaitableEventDone>())
{}

angle::Result CLProgramVk::init()
{
    cl::DevicePtrs devices;
    ANGLE_TRY(mContext->getDevices(&devices));

    // The devices associated with the program object are the devices associated with context
    for (const cl::DevicePtr &device : devices)
    {
        DeviceProgramData deviceProgramData{};
        mAssociatedDevicePrograms[device->getNative()] = std::move(deviceProgramData);
    }

    return angle::Result::Continue;
}

angle::Result CLProgramVk::init(const size_t *lengths,
                                const unsigned char **binaries,
                                cl_int *binaryStatus)
{
    // The devices associated with program come from device_list param from
    // clCreateProgramWithBinary
    for (const cl::DevicePtr &device : mProgram.getDevices())
    {
        const unsigned char *binaryHandle = *binaries++;
        size_t binarySize                 = *lengths++;

        // Check for header
        if (binarySize < sizeof(ProgramBinaryOutputHeader))
        {
            if (binaryStatus)
            {
                *binaryStatus++ = CL_INVALID_BINARY;
            }
            ANGLE_CL_RETURN_ERROR(CL_INVALID_BINARY);
        }
        binarySize -= sizeof(ProgramBinaryOutputHeader);

        // Check for valid binary version from header
        const ProgramBinaryOutputHeader *binaryHeader =
            reinterpret_cast<const ProgramBinaryOutputHeader *>(binaryHandle);
        if (binaryHeader == nullptr)
        {
            ERR() << "NULL binary header!";
            if (binaryStatus)
            {
                *binaryStatus++ = CL_INVALID_BINARY;
            }
            ANGLE_CL_RETURN_ERROR(CL_INVALID_BINARY);
        }
        else if (binaryHeader->headerVersion < kBinaryVersion)
        {
            ERR() << "Binary version not compatible with runtime!";
            if (binaryStatus)
            {
                *binaryStatus++ = CL_INVALID_BINARY;
            }
            ANGLE_CL_RETURN_ERROR(CL_INVALID_BINARY);
        }
        binaryHandle += sizeof(ProgramBinaryOutputHeader);

        // See what kind of binary we have (i.e. SPIR-V or LLVM Bitcode)
        // https://llvm.org/docs/BitCodeFormat.html#llvm-ir-magic-number
        // https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#_magic_number
        constexpr uint32_t LLVM_BC_MAGIC = 0xDEC04342;
        constexpr uint32_t SPIRV_MAGIC   = 0x07230203;
        const uint32_t &firstWord        = reinterpret_cast<const uint32_t *>(binaryHandle)[0];
        bool isBC                        = firstWord == LLVM_BC_MAGIC;
        bool isSPV                       = firstWord == SPIRV_MAGIC;
        if (!isBC && !isSPV)
        {
            ERR() << "Binary is neither SPIR-V nor LLVM Bitcode!";
            if (binaryStatus)
            {
                *binaryStatus++ = CL_INVALID_BINARY;
            }
            ANGLE_CL_RETURN_ERROR(CL_INVALID_BINARY);
        }

        // Add device binary to program
        DeviceProgramData deviceBinary;
        deviceBinary.spirvVersion = device->getImpl<CLDeviceVk>().getSpirvVersion();
        deviceBinary.binaryType  = binaryHeader->binaryType;
        deviceBinary.buildStatus = binaryHeader->buildStatus;
        switch (deviceBinary.binaryType)
        {
            case CL_PROGRAM_BINARY_TYPE_EXECUTABLE:
                deviceBinary.binary.assign(binarySize / sizeof(uint32_t), 0);
                std::memcpy(deviceBinary.binary.data(), binaryHandle, binarySize);
                break;
            case CL_PROGRAM_BINARY_TYPE_LIBRARY:
            case CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT:
                deviceBinary.IR.assign(binarySize, 0);
                std::memcpy(deviceBinary.IR.data(), binaryHandle, binarySize);
                break;
            default:
                UNREACHABLE();
                ERR() << "Invalid binary type!";
                if (binaryStatus)
                {
                    *binaryStatus++ = CL_INVALID_BINARY;
                }
                ANGLE_CL_RETURN_ERROR(CL_INVALID_BINARY);
        }
        mAssociatedDevicePrograms[device->getNative()] = std::move(deviceBinary);
        if (binaryStatus)
        {
            *binaryStatus++ = CL_SUCCESS;
        }
    }

    return angle::Result::Continue;
}

CLProgramVk::~CLProgramVk() {}

angle::Result CLProgramVk::build(const cl::DevicePtrs &devices,
                                 const char *options,
                                 cl::Program *notify)
{
    BuildType buildType = !mProgram.getSource().empty() ? BuildType::BUILD : BuildType::BINARY;
    const cl::DevicePtrs &devicePtrs = !devices.empty() ? devices : mProgram.getDevices();

    setBuildStatus(devicePtrs, CL_BUILD_IN_PROGRESS);

    if (notify)
    {
        mAsyncBuildEvent =
            getPlatform()->postMultiThreadWorkerTask(std::make_shared<CLAsyncBuildTask>(
                this, devicePtrs, std::string(options ? options : ""), "", buildType,
                LinkProgramsList{}, notify));
        ASSERT(mAsyncBuildEvent != nullptr);
    }
    else
    {
        if (!buildInternal(devicePtrs, std::string(options ? options : ""), "", buildType,
                           LinkProgramsList{}))
        {
            ANGLE_CL_RETURN_ERROR(CL_BUILD_PROGRAM_FAILURE);
        }
    }
    return angle::Result::Continue;
}

angle::Result CLProgramVk::compile(const cl::DevicePtrs &devices,
                                   const char *options,
                                   const cl::ProgramPtrs &inputHeaders,
                                   const char **headerIncludeNames,
                                   cl::Program *notify)
{
    const cl::DevicePtrs &devicePtrs = !devices.empty() ? devices : mProgram.getDevices();

    // Ensure OS temp dir is available
    std::string internalCompileOpts;
    Optional<std::string> tmpDir = angle::GetTempDirectory();
    if (!tmpDir.valid())
    {
        ERR() << "Failed to open OS temp dir";
        ANGLE_CL_RETURN_ERROR(CL_INVALID_OPERATION);
    }
    internalCompileOpts += inputHeaders.empty() ? "" : " -I" + tmpDir.value();

    // Dump input headers to OS temp directory
    for (size_t i = 0; i < inputHeaders.size(); ++i)
    {
        const std::string &inputHeaderSrc =
            inputHeaders.at(i)->getImpl<CLProgramVk>().mProgram.getSource();
        std::string headerFilePath(angle::ConcatenatePath(tmpDir.value(), headerIncludeNames[i]));

        // Sanitize path so we can use "/" as universal path separator
        angle::MakeForwardSlashThePathSeparator(headerFilePath);
        size_t baseDirPos = headerFilePath.find_last_of("/");

        // Ensure parent dir(s) exists
        if (!angle::CreateDirectories(headerFilePath.substr(0, baseDirPos)))
        {
            ERR() << "Failed to create output path(s) for header(s)!";
            ANGLE_CL_RETURN_ERROR(CL_INVALID_OPERATION);
        }
        writeFile(headerFilePath.c_str(), inputHeaderSrc.data(), inputHeaderSrc.size());
    }

    setBuildStatus(devicePtrs, CL_BUILD_IN_PROGRESS);

    // Perform compile
    if (notify)
    {
        mAsyncBuildEvent = mProgram.getContext().getPlatform().getMultiThreadPool()->postWorkerTask(
            std::make_shared<CLAsyncBuildTask>(
                this, devicePtrs, std::string(options ? options : ""), internalCompileOpts,
                BuildType::COMPILE, LinkProgramsList{}, notify));
        ASSERT(mAsyncBuildEvent != nullptr);
    }
    else
    {
        mAsyncBuildEvent = std::make_shared<angle::WaitableEventDone>();
        if (!buildInternal(devicePtrs, std::string(options ? options : ""), internalCompileOpts,
                           BuildType::COMPILE, LinkProgramsList{}))
        {
            ANGLE_CL_RETURN_ERROR(CL_COMPILE_PROGRAM_FAILURE);
        }
    }

    return angle::Result::Continue;
}

angle::Result CLProgramVk::getInfo(cl::ProgramInfo name,
                                   size_t valueSize,
                                   void *value,
                                   size_t *valueSizeRet) const
{
    cl_uint valUInt            = 0u;
    cl_bool valBool            = CL_FALSE;
    void *valPointer           = nullptr;
    const void *copyValue      = nullptr;
    size_t copySize            = 0u;
    unsigned char **outputBins = reinterpret_cast<unsigned char **>(value);
    std::string kernelNamesList;
    std::vector<size_t> vBinarySizes;

    switch (name)
    {
        case cl::ProgramInfo::NumKernels:
            for (const auto &deviceProgram : mAssociatedDevicePrograms)
            {
                valUInt += static_cast<decltype(valUInt)>(deviceProgram.second.numKernels());
            }
            copyValue = &valUInt;
            copySize  = sizeof(valUInt);
            break;
        case cl::ProgramInfo::BinarySizes:
        {
            for (const auto &deviceProgram : mAssociatedDevicePrograms)
            {
                vBinarySizes.push_back(
                    sizeof(ProgramBinaryOutputHeader) +
                    (deviceProgram.second.binaryType == CL_PROGRAM_BINARY_TYPE_EXECUTABLE
                         ? deviceProgram.second.binary.size() * sizeof(uint32_t)
                         : deviceProgram.second.IR.size()));
            }
            valPointer = vBinarySizes.data();
            copyValue  = valPointer;
            copySize   = vBinarySizes.size() * sizeof(size_t);
            break;
        }
        case cl::ProgramInfo::Binaries:
            for (const auto &deviceProgram : mAssociatedDevicePrograms)
            {
                const void *bin =
                    deviceProgram.second.binaryType == CL_PROGRAM_BINARY_TYPE_EXECUTABLE
                        ? reinterpret_cast<const void *>(deviceProgram.second.binary.data())
                        : reinterpret_cast<const void *>(deviceProgram.second.IR.data());
                size_t binSize =
                    deviceProgram.second.binaryType == CL_PROGRAM_BINARY_TYPE_EXECUTABLE
                        ? deviceProgram.second.binary.size() * sizeof(uint32_t)
                        : deviceProgram.second.IR.size();
                ProgramBinaryOutputHeader header{.headerVersion = kBinaryVersion,
                                                 .binaryType    = deviceProgram.second.binaryType,
                                                 .buildStatus   = deviceProgram.second.buildStatus};

                if (outputBins != nullptr)
                {
                    if (*outputBins != nullptr)
                    {
                        std::memcpy(*outputBins, &header, sizeof(ProgramBinaryOutputHeader));
                        std::memcpy((*outputBins) + sizeof(ProgramBinaryOutputHeader), bin,
                                    binSize);
                    }
                    outputBins++;
                }

                // Spec just wants pointer size here
                copySize += sizeof(unsigned char *);
            }
            // We already copied the (headers + binaries) over - nothing else left to copy
            copyValue = nullptr;
            break;
        case cl::ProgramInfo::KernelNames:
            for (const auto &deviceProgram : mAssociatedDevicePrograms)
            {
                kernelNamesList = deviceProgram.second.getKernelNames();
            }
            valPointer = kernelNamesList.data();
            copyValue  = valPointer;
            copySize   = kernelNamesList.size() + 1;
            break;
        case cl::ProgramInfo::ScopeGlobalCtorsPresent:
        case cl::ProgramInfo::ScopeGlobalDtorsPresent:
            // These are deprecated by version 3.0 and are currently not supported
            copyValue = &valBool;
            copySize  = sizeof(cl_bool);
            break;
        default:
            UNREACHABLE();
    }

    if ((value != nullptr) && (copyValue != nullptr))
    {
        std::memcpy(value, copyValue, copySize);
    }

    if (valueSizeRet != nullptr)
    {
        *valueSizeRet = copySize;
    }

    return angle::Result::Continue;
}

angle::Result CLProgramVk::getBuildInfo(const cl::Device &device,
                                        cl::ProgramBuildInfo name,
                                        size_t valueSize,
                                        void *value,
                                        size_t *valueSizeRet) const
{
    cl_uint valUInt                            = 0;
    cl_build_status valStatus                  = 0;
    const void *copyValue                      = nullptr;
    size_t copySize                            = 0;
    const DeviceProgramData *deviceProgramData = getDeviceProgramData(device.getNative());

    switch (name)
    {
        case cl::ProgramBuildInfo::Status:
            valStatus = deviceProgramData->buildStatus;
            copyValue = &valStatus;
            copySize  = sizeof(valStatus);
            break;
        case cl::ProgramBuildInfo::Log:
            copyValue = deviceProgramData->buildLog.c_str();
            copySize  = deviceProgramData->buildLog.size() + 1;
            break;
        case cl::ProgramBuildInfo::Options:
            copyValue = mProgramOpts.c_str();
            copySize  = mProgramOpts.size() + 1;
            break;
        case cl::ProgramBuildInfo::BinaryType:
            valUInt   = deviceProgramData->binaryType;
            copyValue = &valUInt;
            copySize  = sizeof(valUInt);
            break;
        case cl::ProgramBuildInfo::GlobalVariableTotalSize:
            // Returns 0 if device does not support program scope global variables.
            valUInt   = 0;
            copyValue = &valUInt;
            copySize  = sizeof(valUInt);
            break;
        default:
            UNREACHABLE();
    }

    if ((value != nullptr) && (copyValue != nullptr))
    {
        memcpy(value, copyValue, std::min(valueSize, copySize));
    }

    if (valueSizeRet != nullptr)
    {
        *valueSizeRet = copySize;
    }

    return angle::Result::Continue;
}

angle::Result CLProgramVk::createKernel(const cl::Kernel &kernel,
                                        const char *name,
                                        CLKernelImpl::Ptr *kernelOut)
{
    // Wait for the compile to finish
    mAsyncBuildEvent->wait();

    std::scoped_lock<angle::SimpleMutex> sl(mProgramMutex);
    const auto devProgram = getDeviceProgramData(name);
    ASSERT(devProgram != nullptr);

    // Create kernel
    CLKernelArguments kernelArgs = devProgram->getKernelArguments(name);
    std::string kernelAttributes = devProgram->getKernelAttributes(name);
    std::string kernelName       = std::string(name ? name : "");
    CLKernelVk::Ptr kernelImpl   = CLKernelVk::Ptr(
        new (std::nothrow) CLKernelVk(kernel, kernelName, kernelAttributes, kernelArgs));
    if (kernelImpl == nullptr)
    {
        ERR() << "Could not create kernel obj!";
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }

    ANGLE_TRY(kernelImpl->init());
    *kernelOut = std::move(kernelImpl);

    return angle::Result::Continue;
}

angle::Result CLProgramVk::createKernels(cl_uint numKernels,
                                         CLKernelImpl::CreateFuncs &createFuncs,
                                         cl_uint *numKernelsRet)
{
    size_t numDevKernels = 0;
    for (const auto &dev : mAssociatedDevicePrograms)
    {
        numDevKernels += dev.second.numKernels();
    }
    if (numKernelsRet != nullptr)
    {
        *numKernelsRet = static_cast<cl_uint>(numDevKernels);
    }

    if (numKernels != 0)
    {
        for (const auto &dev : mAssociatedDevicePrograms)
        {
            for (const auto &kernArgMap : dev.second.getKernelArgsMap())
            {
                createFuncs.emplace_back([this, &kernArgMap](const cl::Kernel &kern) {
                    CLKernelImpl::Ptr implPtr = nullptr;
                    ANGLE_CL_IMPL_TRY(this->createKernel(kern, kernArgMap.first.c_str(), &implPtr));
                    return CLKernelImpl::Ptr(std::move(implPtr));
                });
            }
        }
    }
    return angle::Result::Continue;
}

const CLProgramVk::DeviceProgramData *CLProgramVk::getDeviceProgramData(
    const _cl_device_id *device) const
{
    if (!mAssociatedDevicePrograms.contains(device))
    {
        WARN() << "Device (" << device << ") is not associated with program (" << this << ") !";
        return nullptr;
    }
    return &mAssociatedDevicePrograms.at(device);
}

const CLProgramVk::DeviceProgramData *CLProgramVk::getDeviceProgramData(
    const char *kernelName) const
{
    for (const auto &deviceProgram : mAssociatedDevicePrograms)
    {
        if (deviceProgram.second.containsKernel(kernelName))
        {
            return &deviceProgram.second;
        }
    }
    WARN() << "Kernel name (" << kernelName << ") is not associated with program (" << this
           << ") !";
    return nullptr;
}

bool CLProgramVk::buildInternal(const cl::DevicePtrs &devices,
                                std::string options,
                                std::string internalOptions,
                                BuildType buildType,
                                const LinkProgramsList &LinkProgramsList)
{
    std::scoped_lock<angle::SimpleMutex> sl(mProgramMutex);

    // Cache original options string
    mProgramOpts = options;

    // Process options and append any other internal (required) options for clspv
    std::vector<std::string> optionTokens;
    angle::SplitStringAlongWhitespace(options + " " + internalOptions, &optionTokens);
    const bool createLibrary     = std::find(optionTokens.begin(), optionTokens.end(),
                                             "-create-library") != optionTokens.end();
    std::string processedOptions = ProcessBuildOptions(optionTokens, buildType);

    // Build for each associated device
    for (size_t i = 0; i < devices.size(); ++i)
    {
        const cl::RefPointer<cl::Device> &device = devices.at(i);
        DeviceProgramData &deviceProgramData     = mAssociatedDevicePrograms[device->getNative()];
        deviceProgramData.spirvVersion           = device->getImpl<CLDeviceVk>().getSpirvVersion();

        // add clspv compiler options based on device features
        processedOptions += ClspvGetCompilerOptions(&device->getImpl<CLDeviceVk>());

        if (buildType != BuildType::BINARY)
        {
            // Invoke clspv
            switch (buildType)
            {
                case BuildType::BUILD:
                case BuildType::COMPILE:
                {
                    ScopedClspvContext clspvCtx;
                    const char *clSrc = mProgram.getSource().c_str();

                    ClspvError clspvRet = ClspvCompileSource(
                        1, NULL, static_cast<const char **>(&clSrc), processedOptions.c_str(),
                        &clspvCtx.mOutputBin, &clspvCtx.mOutputBinSize, &clspvCtx.mOutputBuildLog);
                    deviceProgramData.buildLog =
                        clspvCtx.mOutputBuildLog != nullptr ? clspvCtx.mOutputBuildLog : "";
                    if (clspvRet != CLSPV_SUCCESS)
                    {
                        ERR() << "OpenCL build failed with: ClspvError(" << clspvRet << ")!";
                        deviceProgramData.buildStatus = CL_BUILD_ERROR;
                        return false;
                    }

                    if (buildType == BuildType::COMPILE)
                    {
                        deviceProgramData.IR.assign(clspvCtx.mOutputBinSize, 0);
                        std::memcpy(deviceProgramData.IR.data(), clspvCtx.mOutputBin,
                                    clspvCtx.mOutputBinSize);
                        deviceProgramData.binaryType = CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT;
                    }
                    else
                    {
                        deviceProgramData.binary.assign(clspvCtx.mOutputBinSize / sizeof(uint32_t),
                                                        0);
                        std::memcpy(deviceProgramData.binary.data(), clspvCtx.mOutputBin,
                                    clspvCtx.mOutputBinSize);
                        deviceProgramData.binaryType = CL_PROGRAM_BINARY_TYPE_EXECUTABLE;
                    }
                    break;
                }
                case BuildType::LINK:
                {
                    ScopedClspvContext clspvCtx;
                    std::vector<size_t> vSizes;
                    std::vector<const char *> vBins;
                    const LinkPrograms &linkPrograms = LinkProgramsList.at(i);
                    for (const CLProgramVk::DeviceProgramData *linkProgramData : linkPrograms)
                    {
                        vSizes.push_back(linkProgramData->IR.size());
                        vBins.push_back(linkProgramData->IR.data());
                    }

                    ClspvError clspvRet = ClspvCompileSource(
                        linkPrograms.size(), vSizes.data(), vBins.data(), processedOptions.c_str(),
                        &clspvCtx.mOutputBin, &clspvCtx.mOutputBinSize, &clspvCtx.mOutputBuildLog);
                    deviceProgramData.buildLog =
                        clspvCtx.mOutputBuildLog != nullptr ? clspvCtx.mOutputBuildLog : "";
                    if (clspvRet != CLSPV_SUCCESS)
                    {
                        ERR() << "OpenCL build failed with: ClspvError(" << clspvRet << ")!";
                        deviceProgramData.buildStatus = CL_BUILD_ERROR;
                        return false;
                    }

                    if (createLibrary)
                    {
                        deviceProgramData.IR.assign(clspvCtx.mOutputBinSize, 0);
                        std::memcpy(deviceProgramData.IR.data(), clspvCtx.mOutputBin,
                                    clspvCtx.mOutputBinSize);
                        deviceProgramData.binaryType = CL_PROGRAM_BINARY_TYPE_LIBRARY;
                    }
                    else
                    {
                        deviceProgramData.binary.assign(clspvCtx.mOutputBinSize / sizeof(uint32_t),
                                                        0);
                        std::memcpy(deviceProgramData.binary.data(),
                                    reinterpret_cast<char *>(clspvCtx.mOutputBin),
                                    clspvCtx.mOutputBinSize);
                        deviceProgramData.binaryType = CL_PROGRAM_BINARY_TYPE_EXECUTABLE;
                    }
                    break;
                }
                default:
                    UNREACHABLE();
                    return false;
            }
        }

        // Extract reflection info from spv binary and populate reflection data, as well as create
        // the shader module
        if (deviceProgramData.binaryType == CL_PROGRAM_BINARY_TYPE_EXECUTABLE)
        {
            // Report SPIR-V validation failure as a build failure
            if (!ClspvValidate(mContext->getRenderer(), deviceProgramData.binary))
            {
                ERR() << "Failed to validate SPIR-V binary!";
                deviceProgramData.buildStatus = CL_BUILD_ERROR;
                return false;
            }

            spvtools::SpirvTools spvTool(deviceProgramData.spirvVersion);
            bool parseRet = spvTool.Parse(
                deviceProgramData.binary,
                [](const spv_endianness_t endianess, const spv_parsed_header_t &instruction) {
                    return SPV_SUCCESS;
                },
                [&deviceProgramData](const spv_parsed_instruction_t &instruction) {
                    return ParseReflection(deviceProgramData.reflectionData, instruction);
                });
            if (!parseRet)
            {
                ERR() << "Failed to parse reflection info from SPIR-V!";
                deviceProgramData.buildStatus = CL_BUILD_ERROR;
                return false;
            }

            if (mShader)
            {
                mShader.reset();
            }
            // Strip SPIR-V binary if Vk implementation does not support non-semantic info
            angle::spirv::Blob spvBlob =
                !mContext->getFeatures().supportsShaderNonSemanticInfo.enabled
                    ? stripReflection(&deviceProgramData)
                    : deviceProgramData.binary;
            ASSERT(!spvBlob.empty());
            if (IsError(vk::InitShaderModule(mContext, &mShader, spvBlob.data(),
                                             spvBlob.size() * sizeof(uint32_t))))
            {
                ERR() << "Failed to init Vulkan Shader Module!";
                deviceProgramData.buildStatus = CL_BUILD_ERROR;
                return false;
            }

            // Setup inital push constant range
            uint32_t pushConstantMinOffet = UINT32_MAX, pushConstantMaxOffset = 0,
                     pushConstantMaxSize = 0;
            for (const auto &pushConstant : deviceProgramData.reflectionData.pushConstants)
            {
                pushConstantMinOffet = pushConstant.second.offset < pushConstantMinOffet
                                           ? pushConstant.second.offset
                                           : pushConstantMinOffet;
                if (pushConstant.second.offset >= pushConstantMaxOffset)
                {
                    pushConstantMaxOffset = pushConstant.second.offset;
                    pushConstantMaxSize   = pushConstant.second.size;
                }
            }
            for (const auto &pushConstant : deviceProgramData.reflectionData.imagePushConstants)
            {
                for (const auto imageConstant : pushConstant.second)
                {
                    pushConstantMinOffet = imageConstant.pcRange.offset < pushConstantMinOffet
                                               ? imageConstant.pcRange.offset
                                               : pushConstantMinOffet;
                    if (imageConstant.pcRange.offset >= pushConstantMaxOffset)
                    {
                        pushConstantMaxOffset = imageConstant.pcRange.offset;
                        pushConstantMaxSize   = imageConstant.pcRange.size;
                    }
                }
            }
            deviceProgramData.pushConstRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            deviceProgramData.pushConstRange.offset =
                pushConstantMinOffet == UINT32_MAX ? 0 : pushConstantMinOffet;
            deviceProgramData.pushConstRange.size = pushConstantMaxOffset + pushConstantMaxSize;

            if (kAngleDebug)
            {
                if (mContext->getFeatures().clDumpVkSpirv.enabled)
                {
                    angle::spirv::Print(deviceProgramData.binary);
                }
            }
        }
        deviceProgramData.buildStatus = CL_BUILD_SUCCESS;
    }
    return true;
}

angle::spirv::Blob CLProgramVk::stripReflection(const DeviceProgramData *deviceProgramData)
{
    angle::spirv::Blob binaryStripped;
    spvtools::Optimizer opt(deviceProgramData->spirvVersion);
    opt.RegisterPass(spvtools::CreateStripReflectInfoPass());
    spvtools::OptimizerOptions optOptions;
    optOptions.set_run_validator(false);
    if (!opt.Run(deviceProgramData->binary.data(), deviceProgramData->binary.size(),
                 &binaryStripped, optOptions))
    {
        ERR() << "Could not strip reflection data from binary!";
    }
    return binaryStripped;
}

void CLProgramVk::setBuildStatus(const cl::DevicePtrs &devices, cl_build_status status)
{
    std::scoped_lock<angle::SimpleMutex> sl(mProgramMutex);

    for (const auto &device : devices)
    {
        ASSERT(mAssociatedDevicePrograms.contains(device->getNative()));
        DeviceProgramData &deviceProgram = mAssociatedDevicePrograms.at(device->getNative());
        deviceProgram.buildStatus        = status;
    }
}

const angle::HashMap<uint32_t, ClspvPrintfInfo> *CLProgramVk::getPrintfDescriptors(
    const std::string &kernelName) const
{
    const DeviceProgramData *deviceProgram = getDeviceProgramData(kernelName.c_str());
    if (deviceProgram)
    {
        return &deviceProgram->reflectionData.printfInfoMap;
    }
    return nullptr;
}

}  // namespace rx
