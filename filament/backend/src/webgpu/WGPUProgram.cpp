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
#include <vector>

namespace filament::backend {

namespace {

[[nodiscard]] constexpr std::string_view toString(ShaderStage stage) {
    switch (stage) {
        case ShaderStage::VERTEX:
            return "vertex";
        case ShaderStage::FRAGMENT:
            return "fragment";
        case ShaderStage::COMPUTE:
            return "compute";
    }
}

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
    labelStream << programName << " " << toString(stage) << " shader";
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
                        FWGPU_LOGD << descriptor.label << " compiled successfully"
                                   << utils::io::endl;
                    }),
            UINT16_MAX);
    return module;
}

// This is a 1 to 1 mapping of the ReservedSpecializationConstants enum in EngineEnums.h
std::string getSpecConstantStringId(uint32_t id) {
    switch (id) {
        case 0:
            return "BACKEND_FEATURE_LEVEL";
        case 1:
            return "CONFIG_MAX_INSTANCES";
        case 2:
            return "CONFIG_STATIC_TEXTURE_TARGET_WORKAROUND";
        case 3:
            return "CONFIG_SRGB_SWAPCHAIN_EMULATION";
        case 4:
            return "CONFIG_FROXEL_BUFFER_HEIGHT";
        case 5:
            return "CONFIG_POWER_VR_SHADER_WORKAROUNDS";
        case 6:
            return "CONFIG_DEBUG_DIRECTIONAL_SHADOWMAP";
        case 7:
            return "CONFIG_DEBUG_FROXEL_VISUALIZATION";
        case 8:
            return "CONFIG_STEREO_EYE_COUNT";
        case 9:
            return "CONFIG_SH_BANDS_COUNT";
        case 10:
            return "CONFIG_SHADOW_SAMPLING_METHOD";
        default:
            return "spec_const" + std::to_string(id);
            // PANIC_POSTCONDITION("Unknown/unhandled spec constant key/id: %d", id);
    }
}

void addPadding(std::string& shaderSource, size_t paddingSize) {
    for (size_t i = 0; i < paddingSize; i++) {
        shaderSource += " ";
    }
}

void replaceSpecConstant(std::string& shaderSource,
        utils::FixedCapacityVector<filament::backend::Program::SpecializationConstant> const&
                constantsInfo) {
    std::string regexPattern;
    std::string replaceValue;
    std::vector<wgpu::ConstantEntry> constants;
    size_t originalSize = shaderSource.length();

    constants.reserve(constantsInfo.size());

    for (filament::backend::Program::SpecializationConstant const& constant: constantsInfo) {
        // CONFIG_MAX_INSTANCES (1) and CONFIG_FROXEL_BUFFER_HEIGHT (4) will not be present
        // as constant overrides in the generated WGSL, because WGSL doesn't support specialization
        // constants as an array length
        // More information at https://github.com/gpuweb/gpuweb/issues/572#issuecomment-649760005
        // CONFIG_SRGB_SWAPCHAIN_EMULATION (3) is being skipped all together since it's only
        // included for the case of mFeatureLevel == FeatureLevel::FEATURE_LEVEL_0, which should
        // not be possible for WebGPU
        // if (constant.id > 10) {
        //     printf("Wrong specialization constant id: %d\n", constant.id);
        // }
        if (constant.id == 1 || constant.id == 3 || constant.id == 4) {
            continue;
        }
        double value = 0.0;
        if (auto* v = std::get_if<int32_t>(&constant.value)) {
            value = static_cast<double>(*v);
            replaceValue = std::to_string(static_cast<int32_t>(value)) + "i";
            regexPattern = R"((-?\d+)i)";
        } else if (auto* f = std::get_if<float>(&constant.value)) {
            value = static_cast<double>(*f);
            replaceValue = std::to_string(value);
            regexPattern = R"((-?\d+)f)";
        } else if (auto* b = std::get_if<bool>(&constant.value)) {
            value = *b ? 0.0 : 1.0;
            replaceValue = value == 0 ? "false" : "true";
            regexPattern = R"((true|false))";
        }

        // Construct the full regex pattern:
        // constantName followed by ' = ', then the value (captured), and then ';'

        std::regex pattern(getSpecConstantStringId(constant.id) + R"( = )" + regexPattern + R"(;)");

        // Construct the replacement string using the captured group
        // replacement_str represents the entire matched string (constantName = value;)
        // We want to replace it with constantName = newValue;
        std::string replacement_str =
                getSpecConstantStringId(constant.id) + " = " + replaceValue + ";";
        shaderSource = std::regex_replace(shaderSource, pattern, replacement_str);
    }
    size_t newSize = shaderSource.length();
    addPadding(shaderSource, (originalSize - newSize));
}

wgpu::ShaderModule setShaderModule(wgpu::Device& device, Program& program,
        backend::ShaderStage shaderStage) {

    const char* programName = program.getName().c_str_safe();
    auto& shaderSource = program.getShadersSource();
    if (shaderStage != ShaderStage::COMPUTE) {
        auto& src = program.getShadersSource();
        utils::FixedCapacityVector<uint8_t>& sourceBytes = src[static_cast<size_t>(shaderStage)];
        std::string shaderSourceString = reinterpret_cast<const char*>(sourceBytes.data());
        replaceSpecConstant(shaderSourceString, program.getSpecializationConstants());
        sourceBytes.resize(shaderSourceString.length());
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
