/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "WebGPUHandles.h"

#include "WebGPUConstants.h"
#include "WebGPUStrings.h"

#include "DriverBase.h"
#include <backend/DriverEnums.h>
#include <backend/Program.h>

#include <utils/Panic.h>
#include <utils/ostream.h>

#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>

namespace filament::backend {

namespace {

[[nodiscard]] wgpu::ShaderModule createShaderModule(wgpu::Device& device, const char* programName,
        std::array<utils::FixedCapacityVector<uint8_t>, Program::SHADER_TYPE_COUNT> const&
                shaderSource,
        ShaderStage stage) {
    utils::FixedCapacityVector<uint8_t> const& sourceBytes =
            shaderSource[static_cast<size_t>(stage)];
    if (sourceBytes.empty()) {
        return nullptr;// nothing to compile, the shader was not provided
    }
    wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
    wgslDescriptor.code = wgpu::StringView(reinterpret_cast<const char*>(sourceBytes.data()));
    std::stringstream labelStream;
    labelStream << programName << " " << filamentShaderStageToString(stage) << " shader";
    auto label = labelStream.str();
    wgpu::ShaderModuleDescriptor descriptor{
        .nextInChain = &wgslDescriptor,
        .label = label.data()
    };
    wgpu::ShaderModule module = device.CreateShaderModule(&descriptor);
    FILAMENT_CHECK_POSTCONDITION(module != nullptr) << "Failed to create " << descriptor.label;

    wgpu::Instance instance = device.GetAdapter().GetInstance();
    instance.WaitAny(
            module.GetCompilationInfo(wgpu::CallbackMode::WaitAnyOnly,
                    [&descriptor](auto const& status,
                            wgpu::CompilationInfo const* info) {
                        switch (status) {
                            case wgpu::CompilationInfoRequestStatus::CallbackCancelled:
                                FWGPU_LOGW << "Shader compilation info callback cancelled for "
                                           << descriptor.label << "?" << utils::io::endl;
                                return;
                            case wgpu::CompilationInfoRequestStatus::Success:
                                break;
                        }
                        if (info != nullptr) {
                            std::stringstream errorStream;
                            int errorCount = 0;
                            for (size_t msgIndex = 0; msgIndex < info->messageCount; msgIndex++) {
                                wgpu::CompilationMessage const& message = info->messages[msgIndex];
                                switch (message.type) {
                                    case wgpu::CompilationMessageType::Info:
                                        FWGPU_LOGI << descriptor.label << ": " << message.message
                                                   << " line#:" << message.lineNum
                                                   << " linePos:" << message.linePos
                                                   << " offset:" << message.offset
                                                   << " length:" << message.length
                                                   << utils::io::endl;
                                        break;
                                    case wgpu::CompilationMessageType::Warning:
                                        FWGPU_LOGW
                                                << "Warning compiling " << descriptor.label << ": "
                                                << message.message << " line#:" << message.lineNum
                                                << " linePos:" << message.linePos
                                                << " offset:" << message.offset
                                                << " length:" << message.length << utils::io::endl;
                                        break;
                                    case wgpu::CompilationMessageType::Error:
                                        errorCount++;
                                        errorStream << "Error " << errorCount << " : "
                                                    << std::string_view(message.message)
                                                    << " line#:" << message.lineNum
                                                    << " linePos:" << message.linePos
                                                    << " offset:" << message.offset
                                                    << " length:" << message.length << "\n";
                                        break;
                                }
                            }
                            FILAMENT_CHECK_POSTCONDITION(errorCount < 1)
                                    << errorCount << " error(s) compiling " << descriptor.label
                                    << ":\n"
                                    << errorStream.str();
                        }
#if FWGPU_ENABLED(FWGPU_DEBUG_VALIDATION)
                        FWGPU_LOGD << descriptor.label << " compiled successfully"
                                   << utils::io::endl;
#endif
                    }),
            SHADER_COMPILATION_TIMEOUT_NANOSECONDS);
    return module;
}

void adjustPaddingValue(std::string& shaderSource, size_t desiredZeroCount) {

    std::string regexPattern = R"((-?\d+)i)";
    std::regex pattern("PADDING_VALUE = " + regexPattern + ";");

    std::smatch match;

    if (std::regex_search(shaderSource, match, pattern)) {
        std::string newNumStr;
        if (desiredZeroCount == 0) {
            newNumStr = "1";
        } else {
            newNumStr = "1";
            newNumStr.append(desiredZeroCount, '0');
        }
        std::string replacement_str = "PADDING_VALUE = " + newNumStr + "i;";
        shaderSource = std::regex_replace(shaderSource, pattern, replacement_str);
    }
}

void replaceFilamentSpecConstValue(std::string& sourceString, const std::string& idToFind,
        const std::string& newValue) {
    std::string patternStr = "FILAMENT_SPEC_CONST_" + idToFind + R"(_\w+\s*=\s*([^;]+);)";
    std::regex regexPattern(patternStr);
    std::smatch match;
    if (std::regex_search(sourceString, match, regexPattern)) {
        // match[0] is the whole matched string
        // match[1] is the captured group (the current value)
        std::string fullMatch = match[0].str();
        std::string oldValue = match[1].str();
        // Construct the replacement string
        // This takes the part before the value, inserts the newValue, and then adds the semicolon.
        std::string replacement = fullMatch;
        size_t valueStart = fullMatch.find(oldValue);
        if (valueStart != std::string::npos) {
            replacement.replace(valueStart, oldValue.length(), newValue);
            // Replace the original line with the modified line
            sourceString.replace(sourceString.find(fullMatch), fullMatch.length(), replacement);
        }
    }
}


void replaceSpecConstant(std::string& shaderSource,
        utils::FixedCapacityVector<filament::backend::Program::SpecializationConstant> const&
                constantsInfo) {
    std::string replaceValue;
    size_t originalSize = shaderSource.length();

    // Reset the padding value
    adjustPaddingValue(shaderSource, (0));

    for (filament::backend::Program::SpecializationConstant const& constant: constantsInfo) {
        // CONFIG_MAX_INSTANCES (1) and CONFIG_FROXEL_BUFFER_HEIGHT (4) will not be present
        // as constant overrides in the generated WGSL, because WGSL doesn't support specialization
        // constants as an array length
        // More information at https://github.com/gpuweb/gpuweb/issues/572#issuecomment-649760005
        // CONFIG_SRGB_SWAPCHAIN_EMULATION (3) is being skipped all together since it's only
        // included for the case of mFeatureLevel == FeatureLevel::FEATURE_LEVEL_0, which should
        // not be possible for WebGPU
        if (constant.id == 1 || constant.id == 3 || constant.id == 4) {
            continue;
        }
        if (auto* v = std::get_if<int32_t>(&constant.value)) {
            replaceValue = std::to_string(*v) + "i";
        } else if (auto* f = std::get_if<float>(&constant.value)) {
            replaceValue = std::to_string(*f) + "f";
        } else if (auto* b = std::get_if<bool>(&constant.value)) {
            replaceValue = (*b) ? "true" : "false";
        }
        replaceFilamentSpecConstValue(shaderSource, std::to_string(constant.id), replaceValue);
    }
    if (shaderSource.length() < originalSize) {
        adjustPaddingValue(shaderSource, (originalSize - shaderSource.length()));
    }
}

wgpu::ShaderModule setShaderModule(wgpu::Device& device, Program& program,
        backend::ShaderStage shaderStage) {

    const char* programName = program.getName().c_str_safe();
    auto& shaderSource = program.getShadersSource();
    if (shaderStage != ShaderStage::COMPUTE) {
        utils::FixedCapacityVector<uint8_t>& sourceBytes =
                shaderSource[static_cast<size_t>(shaderStage)];
        std::string shaderSourceString = reinterpret_cast<const char*>(sourceBytes.data());
        replaceSpecConstant(shaderSourceString, program.getSpecializationConstants());
        std::memcpy(sourceBytes.data(), shaderSourceString.data(), shaderSourceString.length());
    }
    return createShaderModule(device, programName, shaderSource, shaderStage);
}

}// namespace

WGPUProgram::WGPUProgram(wgpu::Device& device, Program& program)
    : HwProgram(program.getName()),
      vertexShaderModule(setShaderModule(device, program, ShaderStage::VERTEX)),
      fragmentShaderModule(setShaderModule(device, program, ShaderStage::FRAGMENT)),
      computeShaderModule(setShaderModule(device, program, ShaderStage::COMPUTE)) {}

}// namespace filament::backend
