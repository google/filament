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
    module.GetCompilationInfo(wgpu::CallbackMode::AllowSpontaneous,
            [&descriptor](auto const& status, wgpu::CompilationInfo const* info) {
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
                                           << " length:" << message.length << utils::io::endl;
                                break;
                            case wgpu::CompilationMessageType::Warning:
                                FWGPU_LOGW << "Warning compiling " << descriptor.label << ": "
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
                            << errorCount << " error(s) compiling " << descriptor.label << ":\n"
                            << errorStream.str();
                }
                FWGPU_LOGD << descriptor.label << " compiled successfully" << utils::io::endl;
            });
    return module;
}

std::vector<wgpu::ConstantEntry> convertConstants(
        utils::FixedCapacityVector<filament::backend::Program::SpecializationConstant> const&
                constantsInfo) {
    std::vector<wgpu::ConstantEntry> constants(constantsInfo.size());
    for (size_t i = 0; i < constantsInfo.size(); i++) {
        filament::backend::Program::SpecializationConstant const& specConstant = constantsInfo[i];
        wgpu::ConstantEntry& constantEntry = constants[i];
        constantEntry.key = wgpu::StringView(std::to_string(specConstant.id));
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
