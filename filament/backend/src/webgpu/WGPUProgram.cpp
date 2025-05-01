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

#include <sstream>
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

// This is a 1 to 1 mapping of the ReservedSpecializationConstants enum in filament/EngineEnums.h
// The _hack is a workaround until https://issues.chromium.org/issues/42250586 is resolved
// This workaround is the same one being used on the generateSpecializationConstant() function
constexpr wgpu::StringView getSpecConstantStringValue(uint32_t value) {
    switch (value) {
        case 0:
            return "BACKEND_FEATURE_LEVEL_hack";
        case 1:
            return "CONFIG_MAX_INSTANCES_hack";
        case 2:
            return "CONFIG_STATIC_TEXTURE_TARGET_WORKAROUND_hack";
        case 3:
            return "CONFIG_SRGB_SWAPCHAIN_EMULATION_hack";
        case 4:
            return "CONFIG_FROXEL_BUFFER_HEIGHT_hack";
        case 5:
            return "CONFIG_POWER_VR_SHADER_WORKAROUNDS_hack";
        case 6:
            return "CONFIG_DEBUG_DIRECTIONAL_SHADOWMAP_hack";
        case 7:
            return "CONFIG_DEBUG_FROXEL_VISUALIZATION_hack";
        case 8:
            return "CONFIG_STEREO_EYE_COUNT_hack";
        case 9:
            return "CONFIG_SH_BANDS_COUNT_hack";
        case 10:
            return "CONFIG_SHADOW_SAMPLING_METHOD_hack";
        default:
            PANIC_POSTCONDITION("Invalid spect constant value passed: %u", value);
    }
}

std::vector<wgpu::ConstantEntry> convertConstants(
        utils::FixedCapacityVector<filament::backend::Program::SpecializationConstant> const&
                constantsInfo) {
    std::vector<wgpu::ConstantEntry> constants(constantsInfo.size());
    for (size_t i = 0; i < constantsInfo.size(); i++) {
        filament::backend::Program::SpecializationConstant const& specConstant = constantsInfo[i];
        wgpu::ConstantEntry& constantEntry = constants[i];
        constantEntry.key = getSpecConstantStringValue(specConstant.id);
        if (auto* v = std::get_if<int32_t>(&specConstant.value)) {
            constantEntry.value = static_cast<double>(*v);
        } else if (auto* f = std::get_if<float>(&specConstant.value)) {
            constantEntry.value = static_cast<double>(*f);
        } else if (auto* b = std::get_if<bool>(&specConstant.value)) {
            constantEntry.value = *b ? 0.0 : 1.0;
        }
    }
    return constants;
}

}// namespace

WGPUProgram::WGPUProgram(wgpu::Device& device, Program& program)
    : HwProgram(program.getName()),
      vertexShaderModule(createShaderModule(device, name.c_str_safe(), program.getShadersSource(),
              ShaderStage::VERTEX)),
      fragmentShaderModule(createShaderModule(device, name.c_str_safe(), program.getShadersSource(),
              ShaderStage::FRAGMENT)),
      computeShaderModule(createShaderModule(device, name.c_str_safe(), program.getShadersSource(),
              ShaderStage::COMPUTE)),
      constants(convertConstants(program.getSpecializationConstants())) {}

}// namespace filament::backend
