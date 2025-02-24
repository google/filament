//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Utilities to map clspv interface variables to OpenCL and Vulkan mappings.
//

#include "libANGLE/renderer/vulkan/clspv_utils.h"
#include "common/log_utils.h"
#include "libANGLE/renderer/vulkan/CLDeviceVk.h"

#include "libANGLE/CLDevice.h"
#include "libANGLE/renderer/driver_utils.h"

#include <mutex>
#include <string>

#include "CL/cl_half.h"

#include "clspv/Compiler.h"

#include "spirv-tools/libspirv.h"
#include "spirv-tools/libspirv.hpp"

namespace rx
{
constexpr std::string_view kPrintfConversionSpecifiers = "diouxXfFeEgGaAcsp";
constexpr std::string_view kPrintfFlagsSpecifiers      = "-+ #0";
constexpr std::string_view kPrintfPrecisionSpecifiers  = "123456789.";
constexpr std::string_view kPrintfVectorSizeSpecifiers = "23468";

namespace
{

template <typename T>
T ReadPtrAs(const unsigned char *data)
{
    return *(reinterpret_cast<const T *>(data));
}

template <typename T>
T ReadPtrAsAndIncrement(unsigned char *&data)
{
    T out = *(reinterpret_cast<T *>(data));
    data += sizeof(T);
    return out;
}

char getPrintfConversionSpecifier(std::string_view formatString)
{
    return formatString.at(formatString.find_first_of(kPrintfConversionSpecifiers));
}

bool IsVectorFormat(std::string_view formatString)
{
    ASSERT(formatString.at(0) == '%');

    // go past the flags, field width and precision
    size_t pos = formatString.find_first_not_of(kPrintfFlagsSpecifiers, 1ul);
    pos        = formatString.find_first_not_of(kPrintfPrecisionSpecifiers, pos);

    return (formatString.at(pos) == 'v');
}

// Printing an individual formatted string into a std::string
// snprintf is used for parsing as OpenCL C printf is similar to printf
std::string PrintFormattedString(const std::string &formatString,
                                 const unsigned char *data,
                                 size_t size)
{
    ASSERT(std::count(formatString.begin(), formatString.end(), '%') == 1);

    size_t outSize = 1024;
    std::vector<char> out(outSize);
    out[0] = '\0';

    char conversion = std::tolower(getPrintfConversionSpecifier(formatString));
    bool finished   = false;
    while (!finished)
    {
        int bytesWritten = 0;
        switch (conversion)
        {
            case 's':
            {
                bytesWritten = snprintf(out.data(), outSize, formatString.c_str(), data);
                break;
            }
            case 'f':
            case 'e':
            case 'g':
            case 'a':
            {
                // all floats with same convention as snprintf
                if (size == 2)
                    bytesWritten = snprintf(out.data(), outSize, formatString.c_str(),
                                            cl_half_to_float(ReadPtrAs<cl_half>(data)));
                else if (size == 4)
                    bytesWritten =
                        snprintf(out.data(), outSize, formatString.c_str(), ReadPtrAs<float>(data));
                else
                    bytesWritten = snprintf(out.data(), outSize, formatString.c_str(),
                                            ReadPtrAs<double>(data));
                break;
            }
            default:
            {
                if (size == 1)
                    bytesWritten = snprintf(out.data(), outSize, formatString.c_str(),
                                            ReadPtrAs<uint8_t>(data));
                else if (size == 2)
                    bytesWritten = snprintf(out.data(), outSize, formatString.c_str(),
                                            ReadPtrAs<uint16_t>(data));
                else if (size == 4)
                    bytesWritten = snprintf(out.data(), outSize, formatString.c_str(),
                                            ReadPtrAs<uint32_t>(data));
                else
                    bytesWritten = snprintf(out.data(), outSize, formatString.c_str(),
                                            ReadPtrAs<uint64_t>(data));
                break;
            }
        }
        if (bytesWritten < 0)
        {
            out[0]   = '\0';
            finished = true;
        }
        else if (bytesWritten < static_cast<long>(outSize))
        {
            finished = true;
        }
        else
        {
            // insufficient size redo above post increment of size
            outSize *= 2;
            out.resize(outSize);
        }
    }

    return std::string(out.data());
}

// Spec mention vn modifier to be printed in the form v1,v2...vn
std::string PrintVectorFormatIntoString(std::string formatString,
                                        const unsigned char *data,
                                        const uint32_t size)
{
    ASSERT(IsVectorFormat(formatString));

    size_t conversionPos = formatString.find_first_of(kPrintfConversionSpecifiers);
    // keep everything after conversion specifier in remainingFormat
    std::string remainingFormat = formatString.substr(conversionPos + 1);
    formatString                = formatString.substr(0, conversionPos + 1);

    size_t vectorPos       = formatString.find_first_of('v');
    size_t vectorLengthPos = ++vectorPos;
    size_t vectorLengthPosEnd =
        formatString.find_first_not_of(kPrintfVectorSizeSpecifiers, vectorLengthPos);

    std::string preVectorString  = formatString.substr(0, vectorPos - 1);
    std::string postVectorString = formatString.substr(vectorLengthPosEnd, formatString.size());
    std::string vectorLengthStr  = formatString.substr(vectorLengthPos, vectorLengthPosEnd);
    int vectorLength             = std::atoi(vectorLengthStr.c_str());

    // skip the vector specifier
    formatString = preVectorString + postVectorString;

    // Get the length modifier
    int elementSize = 0;
    if (postVectorString.find("hh") != std::string::npos)
    {
        elementSize = 1;
    }
    else if (postVectorString.find("hl") != std::string::npos)
    {
        elementSize = 4;
        // snprintf doesn't recognize the hl modifier so strip it
        size_t hl = formatString.find("hl");
        formatString.erase(hl, 2);
    }
    else if (postVectorString.find("h") != std::string::npos)
    {
        elementSize = 2;
    }
    else if (postVectorString.find("l") != std::string::npos)
    {
        elementSize = 8;
    }
    else
    {
        WARN() << "Vector specifier is used without a length modifier. Guessing it from "
                  "vector length and argument sizes in PrintInfo. Kernel modification is "
                  "recommended.";
        elementSize = size / vectorLength;
    }

    std::string out{""};
    for (int i = 0; i < vectorLength - 1; i++)
    {
        out += PrintFormattedString(formatString, data, size / vectorLength) + ",";
        data += elementSize;
    }
    out += PrintFormattedString(formatString, data, size / vectorLength) + remainingFormat;

    return out;
}

// Process the printf stream by breaking them down into individual format specifier and processing
// them.
void ProcessPrintfStatement(unsigned char *&data,
                            const angle::HashMap<uint32_t, ClspvPrintfInfo> *descs,
                            const unsigned char *dataEnd)
{
    // printf storage buffer contents - | id | formatString | argSizes... |
    uint32_t printfID               = ReadPtrAsAndIncrement<uint32_t>(data);
    const std::string &formatString = descs->at(printfID).formatSpecifier;

    std::string printfOutput = "";

    // formatString could be "<string literal> <% format specifiers ...> <string literal>"
    // print the literal part if any first
    size_t nextFormatSpecPos = formatString.find_first_of('%');
    printfOutput += formatString.substr(0, nextFormatSpecPos);

    // print each <% format specifier> + any string literal separately using snprintf
    size_t idx = 0;
    while (nextFormatSpecPos < formatString.size() - 1)
    {
        // Get the part of the format string before the next format specifier
        size_t partStart             = nextFormatSpecPos;
        size_t partEnd               = formatString.find_first_of('%', partStart + 1);
        std::string partFormatString = formatString.substr(partStart, partEnd - partStart);

        // Handle special cases
        if (partEnd == partStart + 1)
        {
            printfOutput += "%";
            nextFormatSpecPos = partEnd + 1;
            continue;
        }
        else if (partEnd == std::string::npos && idx >= descs->at(printfID).argSizes.size())
        {
            // If there are no remaining arguments, the rest of the format
            // should be printed verbatim
            printfOutput += partFormatString;
            break;
        }

        // The size of the argument that this format part will consume
        const uint32_t &size = descs->at(printfID).argSizes[idx];

        if (data + size > dataEnd)
        {
            data += size;
            return;
        }

        // vector format need special care for snprintf
        if (!IsVectorFormat(partFormatString))
        {
            // not a vector format can be printed through snprintf
            // except for %s
            if (getPrintfConversionSpecifier(partFormatString) == 's')
            {
                uint32_t stringID = ReadPtrAs<uint32_t>(data);
                printfOutput +=
                    PrintFormattedString(partFormatString,
                                         reinterpret_cast<const unsigned char *>(
                                             descs->at(stringID).formatSpecifier.c_str()),
                                         size);
            }
            else
            {
                printfOutput += PrintFormattedString(partFormatString, data, size);
            }
            data += size;
        }
        else
        {
            printfOutput += PrintVectorFormatIntoString(partFormatString, data, size);
            data += size;
        }

        // Move to the next format part and prepare to handle the next arg
        nextFormatSpecPos = partEnd;
        idx++;
    }

    std::printf("%s", printfOutput.c_str());
}

std::string GetSpvVersionAsClspvString(spv_target_env spvVersion)
{
    switch (spvVersion)
    {
        default:
        case SPV_ENV_VULKAN_1_0:
            return "1.0";
        case SPV_ENV_VULKAN_1_1:
            return "1.3";
        case SPV_ENV_VULKAN_1_1_SPIRV_1_4:
            return "1.4";
        case SPV_ENV_VULKAN_1_2:
            return "1.5";
        case SPV_ENV_VULKAN_1_3:
            return "1.6";
    }
}

std::vector<std::string> GetNativeBuiltins(const vk::Renderer *renderer)
{
    if (renderer->getFeatures().usesNativeBuiltinClKernel.enabled)
    {
        return std::vector<std::string>({"fma", "half_exp2", "exp2"});
    }

    return {};
}
}  // anonymous namespace

// Process the data recorded into printf storage buffer along with the info in printfino descriptor
// and write it to stdout.
angle::Result ClspvProcessPrintfBuffer(unsigned char *buffer,
                                       const size_t bufferSize,
                                       const angle::HashMap<uint32_t, ClspvPrintfInfo> *infoMap)
{
    // printf storage buffer contains a series of uint32_t values
    // the first integer is offset from second to next available free memory -- this is the amount
    // of data written by kernel.
    const size_t bytesWritten = ReadPtrAsAndIncrement<uint32_t>(buffer) * sizeof(uint32_t);
    const size_t dataSize     = bufferSize - sizeof(uint32_t);
    const size_t limit        = std::min(bytesWritten, dataSize);

    const unsigned char *dataEnd = buffer + limit;
    while (buffer < dataEnd)
    {
        ProcessPrintfStatement(buffer, infoMap, dataEnd);
    }

    if (bufferSize < bytesWritten)
    {
        WARN() << "Printf storage buffer was not sufficient for all printfs. Around "
               << 100.0 * (float)(bytesWritten - bufferSize) / bytesWritten
               << "% of them have been skipped.";
    }

    return angle::Result::Continue;
}

std::string ClspvGetCompilerOptions(const CLDeviceVk *device)
{
    ASSERT(device && device->getRenderer());
    const vk::Renderer *rendererVk = device->getRenderer();
    std::string options{""};
    std::vector<std::string> featureMacros;

    cl_uint addressBits;
    if (IsError(device->getInfoUInt(cl::DeviceInfo::AddressBits, &addressBits)))
    {
        // This shouldn't fail here
        ASSERT(false);
    }
    options += addressBits == 64 ? " -arch=spir64" : " -arch=spir";

    // select SPIR-V version target
    options += " --spv-version=" + GetSpvVersionAsClspvString(device->getSpirvVersion());

    cl_uint nonUniformNDRangeSupport;
    if (IsError(device->getInfoUInt(cl::DeviceInfo::NonUniformWorkGroupSupport,
                                    &nonUniformNDRangeSupport)))
    {
        // This shouldn't fail here
        ASSERT(false);
    }
    // This "cl-arm-non-uniform-work-group-size" flag is needed to generate region reflection
    // instructions since clspv builtin pass is conditionally dependant on it:
    /*
        bool NonUniformNDRangeSupported() {
            return ((Language() == SourceLanguage::OpenCL_CPP) ||
                    (Language() == SourceLanguage::OpenCL_C_20) ||
                    (Language() == SourceLanguage::OpenCL_C_30) ||
                    ArmNonUniformWorkGroupSize()) &&
                    !UniformWorkgroupSize();
        }
        ...
            Value *Ret = GidBase;
            if (clspv::Option::NonUniformNDRangeSupported()) {
                auto Ptr = GetPushConstantPointer(BB, clspv::PushConstant::RegionOffset);
                auto DimPtr = Builder.CreateInBoundsGEP(VT, Ptr, Indices);
                auto Size = Builder.CreateLoad(IT, DimPtr);
                ...
    */
    options += nonUniformNDRangeSupport == CL_TRUE ? " -cl-arm-non-uniform-work-group-size" : "";

    // Other internal Clspv compiler flags that are needed/required
    options += " --long-vector";
    options += " --global-offset";
    options += " --enable-printf";
    options += " --cl-kernel-arg-info";

    // 8 bit storage buffer support
    if (!rendererVk->getFeatures().supports8BitStorageBuffer.enabled)
    {
        options += " --no-8bit-storage=ssbo";
    }
    if (!rendererVk->getFeatures().supports8BitUniformAndStorageBuffer.enabled)
    {
        options += " --no-8bit-storage=ubo";
    }
    if (!rendererVk->getFeatures().supports8BitPushConstant.enabled)
    {
        options += " --no-8bit-storage=pushconstant";
    }

    // 16 bit storage options
    if (!rendererVk->getFeatures().supports16BitStorageBuffer.enabled)
    {
        options += " --no-16bit-storage=ssbo";
    }
    if (!rendererVk->getFeatures().supports16BitUniformAndStorageBuffer.enabled)
    {
        options += " --no-16bit-storage=ubo";
    }
    if (!rendererVk->getFeatures().supports16BitPushConstant.enabled)
    {
        options += " --no-16bit-storage=pushconstant";
    }

    if (rendererVk->getFeatures().supportsUniformBufferStandardLayout.enabled)
    {
        options += " --std430-ubo-layout";
    }

    std::string nativeBuiltins{""};
    for (const std::string &builtin : GetNativeBuiltins(rendererVk))
    {
        nativeBuiltins += builtin + ",";
    }
    options += " --use-native-builtins=" + nativeBuiltins;
    std::vector<std::string> rteModes;
    if (rendererVk->getFeatures().supportsRoundingModeRteFp32.enabled)
    {
        rteModes.push_back("32");
    }
    if (rendererVk->getFeatures().supportsShaderFloat16.enabled)
    {
        options += " --fp16";
        if (rendererVk->getFeatures().supportsRoundingModeRteFp16.enabled)
        {
            rteModes.push_back("16");
        }
    }
    if (rendererVk->getFeatures().supportsShaderFloat64.enabled)
    {
        options += " --fp64";
        featureMacros.push_back("__opencl_c_fp64");
        if (rendererVk->getFeatures().supportsRoundingModeRteFp64.enabled)
        {
            rteModes.push_back("64");
        }
    }
    else
    {
        options += " --fp64=0";
    }

    if (device->getFrontendObject().getInfo().imageSupport)
    {
        featureMacros.push_back("__opencl_c_images");
        featureMacros.push_back("__opencl_c_3d_image_writes");
        featureMacros.push_back("__opencl_c_read_write_images");
    }

    if (rendererVk->getEnabledFeatures().features.shaderInt64)
    {
        featureMacros.push_back("__opencl_c_int64");
    }

    if (!rteModes.empty())
    {
        options += " --rounding-mode-rte=";
        options += std::reduce(std::next(rteModes.begin()), rteModes.end(), rteModes[0],
                               [](const auto a, const auto b) { return a + "," + b; });
    }
    if (!featureMacros.empty())
    {
        options += " --enable-feature-macros=";
        options +=
            std::reduce(std::next(featureMacros.begin()), featureMacros.end(), featureMacros[0],
                        [](const std::string a, const std::string b) { return a + "," + b; });
    }

    return options;
}

// A locked wrapper for clspvCompileFromSourcesString - the underneath LLVM parser is non-rentrant.
// So protecting it with mutex.
ClspvError ClspvCompileSource(const size_t programCount,
                              const size_t *programSizes,
                              const char **programs,
                              const char *options,
                              char **outputBinary,
                              size_t *outputBinarySize,
                              char **outputLog)
{
    [[clang::no_destroy]] static angle::SimpleMutex mtx;

    std::lock_guard<angle::SimpleMutex> lock(mtx);

    return clspvCompileFromSourcesString(programCount, programSizes, programs, options,
                                         outputBinary, outputBinarySize, outputLog);
}

spv_target_env ClspvGetSpirvVersion(const vk::Renderer *renderer)
{
    uint32_t vulkanApiVersion = renderer->getDeviceVersion();
    if (vulkanApiVersion < VK_API_VERSION_1_1)
    {
        // Minimum supported Vulkan version is 1.1 by Angle
        UNREACHABLE();
        return SPV_ENV_MAX;
    }
    else if (vulkanApiVersion < VK_API_VERSION_1_2)
    {
        // TODO: Might be worthwhile to make Vulkan 1.3 as minimum requirement
        // http://anglebug.com/383824579
        if (renderer->getFeatures().supportsSPIRV14.enabled)
        {
            return SPV_ENV_VULKAN_1_1_SPIRV_1_4;
        }
        return SPV_ENV_VULKAN_1_1;
    }
    else if (vulkanApiVersion < VK_API_VERSION_1_3)
    {
        return SPV_ENV_VULKAN_1_2;
    }
    else
    {
        // return the latest supported version
        return SPV_ENV_VULKAN_1_3;
    }
}

bool ClspvValidate(vk::Renderer *rendererVk, const angle::spirv::Blob &blob)
{
    spvtools::SpirvTools spvTool(ClspvGetSpirvVersion(rendererVk));
    spvTool.SetMessageConsumer([](spv_message_level_t level, const char *,
                                  const spv_position_t &position, const char *message) {
        switch (level)
        {
            case SPV_MSG_FATAL:
            case SPV_MSG_ERROR:
            case SPV_MSG_INTERNAL_ERROR:
                ERR() << "SPV validation error (" << position.line << "." << position.column
                      << "): " << message;
                break;
            case SPV_MSG_WARNING:
                WARN() << "SPV validation warning (" << position.line << "." << position.column
                       << "): " << message;
                break;
            case SPV_MSG_INFO:
                INFO() << "SPV validation info (" << position.line << "." << position.column
                       << "): " << message;
                break;
            case SPV_MSG_DEBUG:
                INFO() << "SPV validation debug (" << position.line << "." << position.column
                       << "): " << message;
                break;
            default:
                UNREACHABLE();
                break;
        }
    });

    spvtools::ValidatorOptions options;
    if (rendererVk->getFeatures().supportsUniformBufferStandardLayout.enabled)
    {
        // Allow UBO layouts that conform to std430 (SSBO) layout requirements
        options.SetUniformBufferStandardLayout(true);
    }

    return spvTool.Validate(blob.data(), blob.size(), options);
}

}  // namespace rx
