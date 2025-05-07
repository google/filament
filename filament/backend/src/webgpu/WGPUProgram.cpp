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

// This is a 1 to 1 mapping of the ReservedSpecializationConstants enum in EngineEnums.h
// The _hack is a workaround until https://issues.chromium.org/issues/42250586 is resolved
// This workaround is the same one being used on the generateSpecializationConstant() function
wgpu::StringView getSpecConstantStringId(uint32_t id) {
    switch (id) {
        case 0:
            return "0";// BACKEND_FEATURE_LEVEL_hack
        case 1:
            return "1";// CONFIG_MAX_INSTANCES_hack
        case 2:
            return "2";// ONFIG_STATIC_TEXTURE_TARGET_WORKAROUND_hack
        case 3:
            return "3";// CONFIG_SRGB_SWAPCHAIN_EMULATION_hack
        case 4:
            return "4";// CONFIG_FROXEL_BUFFER_HEIGHT_hack
        case 5:
            return "5";// CONFIG_POWER_VR_SHADER_WORKAROUNDS_hack
        case 6:
            return "6";// CONFIG_DEBUG_DIRECTIONAL_SHADOWMAP_hack
        case 7:
            return "7";// CONFIG_DEBUG_FROXEL_VISUALIZATION_hack
        case 8:
            return "8";// CONFIG_STEREO_EYE_COUNT_hack
        case 9:
            return "9";// CONFIG_SH_BANDS_COUNT_hack
        case 10:
            return "10";// CONFIG_SHADOW_SAMPLING_METHOD_hack
        default:
            PANIC_POSTCONDITION("Unknown/unhandled spec constant key/id: %d", id);
    }
}

std::vector<wgpu::ConstantEntry> convertConstants(
        utils::FixedCapacityVector<filament::backend::Program::SpecializationConstant> const&
                constantsInfo) {
    std::vector<wgpu::ConstantEntry> constants;
    constants.reserve(constantsInfo.size());
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
        double value = 0.0;
        if (auto* v = std::get_if<int32_t>(&constant.value)) {
            value = static_cast<double>(*v);
        } else if (auto* f = std::get_if<float>(&constant.value)) {
            value = static_cast<double>(*f);
        } else if (auto* b = std::get_if<bool>(&constant.value)) {
            value = *b ? 0.0 : 1.0;
        }
        constants.push_back(
                wgpu::ConstantEntry{ .key = getSpecConstantStringId(constant.id), .value = value });
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
