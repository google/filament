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

#include "WebGPUProgram.h"

#include "WebGPUConstants.h"
#include "WebGPUStrings.h"

#include "DriverBase.h"
#include <backend/DriverEnums.h>
#include <backend/Program.h>

#include <utils/FixedCapacityVector.h>
#include <utils/Panic.h>
#include <utils/debug.h>

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace filament::backend {

namespace {

/**
 * Creates a WebGPU shader module for a given "program" "stage", accounting for override constants.
 * Effectively, this function is responsible for preprocessing the shader source and compiling it.
 * @param device The WebGPU device, which is the WebGPU API entry point for creating/registering
 * a shader module
 * @param program The "program" to compile/create the shader, which includes the shader source
 * @param stage The stage (e.g. vertex, fragment, etc.) to create the shader module
 * @return the proper WebGPU shader module compiled/created from the input parameters. This might
 * wrap a null handle if the shader is not present (if the shader source is empty), such as
 * a missing fragment or compute shader.
 */
[[nodiscard]] wgpu::ShaderModule createShaderModule(wgpu::Device const& device,
        Program const& program, const ShaderStage stage) {
    const char* const programName = program.getName().c_str_safe();
    std::array<utils::FixedCapacityVector<uint8_t>, Program::SHADER_TYPE_COUNT> const&
            shaderSource = program.getShadersSource();
    utils::FixedCapacityVector<uint8_t> const& sourceBytes =
            shaderSource[static_cast<size_t>(stage)];
    if (sourceBytes.empty()) {
        return nullptr; // No shader source to compile.
    }
    std::stringstream labelStream;
    labelStream << programName << " " << filamentShaderStageToString(stage) << " shader";
    const auto label = labelStream.str();

    wgpu::ShaderSourceWGSL wgslDescriptor{};
    wgslDescriptor.code = wgpu::StringView(reinterpret_cast<const char*>(sourceBytes.data()));
    const wgpu::ShaderModuleDescriptor descriptor{
        .nextInChain = &wgslDescriptor,
        .label = label.data()
    };
    const wgpu::ShaderModule shaderModule = device.CreateShaderModule(&descriptor);

#if !defined(__EMSCRIPTEN__)
    // TODO: We don't really need to wait for compilation info in production. It's helpful only
    // for debugging.
    const wgpu::Instance instance = device.GetAdapter().GetInstance();

    // Synchronously compile the shader module.
    const wgpu::WaitStatus waitResult = instance.WaitAny(
            shaderModule.GetCompilationInfo(wgpu::CallbackMode::WaitAnyOnly,
                    [&descriptor](wgpu::CompilationInfoRequestStatus status,
                            wgpu::CompilationInfo const* info) {
                        switch (status) {
                            case wgpu::CompilationInfoRequestStatus::CallbackCancelled:
                                FWGPU_LOGW << "Shader compilation info callback cancelled for "
                                           << descriptor.label << "?";
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
                                                   << " length:" << message.length;
                                        break;
                                    case wgpu::CompilationMessageType::Warning:
                                        FWGPU_LOGW
                                                << "Warning compiling " << descriptor.label << ": "
                                                << message.message << " line#:" << message.lineNum
                                                << " linePos:" << message.linePos
                                                << " offset:" << message.offset
                                                << " length:" << message.length;
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
                        FWGPU_LOGD << descriptor.label << " compiled successfully";
#endif
                    }),
            FILAMENT_WEBGPU_SHADER_COMPILATION_TIMEOUT_NANOSECONDS);
    switch (waitResult) {
        case wgpu::WaitStatus::Success:
            break;
        case wgpu::WaitStatus::Error:
            PANIC_POSTCONDITION("Error creating/compiling shader %s (detected after wait).",
                    descriptor.label.data);
            break;
        case wgpu::WaitStatus::TimedOut:
            PANIC_POSTCONDITION("Timed out creating/compiling shader %s", descriptor.label.data);
            break;
    }
#endif
    FILAMENT_CHECK_POSTCONDITION(shaderModule) << "Failed to create " << descriptor.label;
    return shaderModule;
}

}// namespace

WebGPUProgram::WebGPUProgram(wgpu::Device const& device, Program const& program)
        : HwProgram{ program.getName() } {
    // TODO: Consider creating/compiling these shaders in parallel.
    vertexShaderModule = createShaderModule(device, program, ShaderStage::VERTEX);
    fragmentShaderModule = createShaderModule(device, program, ShaderStage::FRAGMENT);
    computeShaderModule = createShaderModule(device, program, ShaderStage::COMPUTE);
}

}// namespace filament::backend
