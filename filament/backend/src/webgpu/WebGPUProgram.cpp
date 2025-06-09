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
#include <utils/ostream.h>

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace filament::backend {

namespace {

/**
 * Given the source code of a [WGSL] WebGPU shader in text (string view to it) and the constants
 * to be overwritten now (at runtime), returns updated [WGSL] shader code text, where the constants
 * have been replaced.
 * @param shaderLabel Something to call this shader for troubleshooting, error messaging, etc.
 * @param shaderSource The original WGSL WebGPU shader code as text to be processed. This is a
 *                     view to that text and this function does not change it; it is immutable.
 *                     Instead, the processed version of this source is returned by the function.
 * @param specConstants The constants to replace in the shader code, indexed by constant id.
 * @return Processed version of the WGSL WebGPU shader code provided, where the constants have
 * been replaced with the values provided by the specConstants parameter.
 */
[[nodiscard]] std::string replaceSpecConstants(std::string_view shaderLabel,
        std::string_view shaderSource,
        std::unordered_map<uint32_t, std::variant<int32_t, float, bool>> const&
                specConstants) {
    // this function is not expected to be called at all if no spec constants are to be replaced
    assert_invariant(!specConstants.empty());
    static constexpr std::string_view specConstantPrefix = "FILAMENT_SPEC_CONST_";
    static constexpr size_t specConstantPrefixSize = specConstantPrefix.size();
    const char* const sourceData = shaderSource.data();
    std::stringstream processedShaderSource{};
    size_t pos = 0;
    while (pos < shaderSource.size()) {
        const size_t posOfNextSpecConstant = shaderSource.find(specConstantPrefix, pos);
        if (posOfNextSpecConstant == std::string::npos) {
            // no more spec constants, so just stream the rest of the source code string
            processedShaderSource << std::string_view(sourceData + pos, shaderSource.size() - pos);
            break;
        }
        const size_t posOfId = posOfNextSpecConstant + specConstantPrefixSize;
        const size_t posAfterId = shaderSource.find('_', posOfId);
        FILAMENT_CHECK_POSTCONDITION(posAfterId != std::string::npos)
                << "malformed " << shaderLabel << ". Found spec constant prefix '"
                << specConstantPrefix << "' without an id or '_' after it.";
        const std::string_view idStr =
                std::string_view(sourceData + posOfId, posAfterId - posOfId);
        const size_t posEndOfStatement = shaderSource.find(';', posAfterId);
        FILAMENT_CHECK_POSTCONDITION(posEndOfStatement != std::string::npos)
                << "malformed " << shaderLabel << ". Found spec constant assignment with id "
                << idStr << " without a terminating ';' character?";
        // this is a view into part of the statement, from after the id to the ';'
        const std::string_view statementSegment =
                std::string_view(sourceData + posAfterId, posEndOfStatement - posAfterId);
        size_t posOfEqual = statementSegment.find('=');
        if (posOfEqual == std::string::npos) {
            // not an assignment statement, so stream to the end of the statement and continue...
            processedShaderSource << std::string_view(sourceData + pos,
                    posEndOfStatement + 1 - pos);
            pos = posEndOfStatement + 1;
            continue;
        }
        posOfEqual += posAfterId; // position in original source overall, not just the segment
        int constantId = 0;
        try {
            constantId = std::stoi(idStr.data());
        } catch (const std::invalid_argument& e) {
            PANIC_POSTCONDITION("Invalid spec constant id '%s' in %s (not a valid integer?): %s",
                    idStr.data(), shaderLabel.data(), e.what());
        } catch (const std::out_of_range& e) {
            PANIC_POSTCONDITION(
                    "Invalid spec constant id '%s' in %s (not an integer? out of range?): %s",
                    idStr.data(), shaderLabel.data(), e.what());
        }
        const auto newValueItr = specConstants.find(static_cast<uint32_t>(constantId));
        if (newValueItr == specConstants.end()) {
            // not going to override the constant,
            // as the specConstants parameter doesn't specify it. So, we will keep the default
            // already in the source text
            // (stream to the end of the statement)...
            processedShaderSource << std::string_view(sourceData + pos,
                    posEndOfStatement + 1 - pos);
            pos = posEndOfStatement + 1;
            continue;
        }
        // need to override the constant...
        const std::variant<int32_t, float, bool> newValue = newValueItr->second;
        // stream up to the equal sign...
        processedShaderSource << std::string_view(sourceData + pos, posOfEqual + 1 - pos);
        // stream the new value...
        if (auto* v = std::get_if<int32_t>(&newValue)) {
            processedShaderSource << " " << *v << "i";
        } else if (auto* f = std::get_if<float>(&newValue)) {
            processedShaderSource << " " << *f << "f";
        } else if (auto* b = std::get_if<bool>(&newValue)) {
            processedShaderSource << " " << ((*b) ? "true" : "false");
        }
        // end the statement...
        processedShaderSource << ";";
        // and skip to after the end of the statement in the original source and continue...
        pos = posEndOfStatement + 1;
    }
    return processedShaderSource.str();
}

/**
 * Creates a WebGPU shader module for a given "program" "stage", accounting for override constants.
 * Effectively, this function is responsible for preprocessing the shader source and compiling it.
 * @param device The WebGPU device, which is the WebGPU API entry point for creating/registering
 * a shader module
 * @param program The "program" to compile/create the shader, which includes the shader source
 * @param stage The stage (e.g. vertex, fragment, etc.) to create the shader module
 * @param specConstants Override constants to apply when creating/compiling the shader module.
 * The expectation is that this is consistent with the program's spec constants, just in a map
 * format for quick access
 * @return the proper WebGPU shader module compiled/created from the input parameters. This might
 * wrap a null handle if the shader is not present (if the shader source is empty), such as
 * a missing fragment or compute shader.
 */
[[nodiscard]] wgpu::ShaderModule createShaderModule(wgpu::Device const& device,
        Program const& program, const ShaderStage stage,
        std::unordered_map<uint32_t, std::variant<int32_t, float, bool>> const& specConstants) {
    const char* const programName = program.getName().c_str_safe();
    std::array<utils::FixedCapacityVector<uint8_t>, Program::SHADER_TYPE_COUNT> const&
            shaderSource = program.getShadersSource();
    utils::FixedCapacityVector<uint8_t> const& sourceBytes =
            shaderSource[static_cast<size_t>(stage)];
    if (sourceBytes.empty()) {
        return nullptr;// nothing to compile/create, the shader was not provided
    }
    std::stringstream labelStream;
    labelStream << programName << " " << filamentShaderStageToString(stage) << " shader";
    const auto label = labelStream.str();
    const std::string processedShaderSource =
            specConstants.empty()
                    ? reinterpret_cast<const char*>(sourceBytes.data())
                    : replaceSpecConstants(label, reinterpret_cast<const char*>(sourceBytes.data()),
                              specConstants);
    wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
    wgslDescriptor.code = wgpu::StringView(processedShaderSource);
    const wgpu::ShaderModuleDescriptor descriptor{
        .nextInChain = &wgslDescriptor,
        .label = label.data()
    };
    const wgpu::ShaderModule shaderModule = device.CreateShaderModule(&descriptor);
    const wgpu::Instance instance = device.GetAdapter().GetInstance();
    // synchronously creates the shader module...
    const wgpu::WaitStatus waitResult = instance.WaitAny(
            shaderModule.GetCompilationInfo(wgpu::CallbackMode::WaitAnyOnly,
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
    FILAMENT_CHECK_POSTCONDITION(shaderModule) << "Failed to create " << descriptor.label;
    return shaderModule;
}

/**
 * Convenience function to convert the array structure of constants to a map indexed by constant
 * id.
 * @param specConstants Original spec constant structure (immutable)
 * @param outConstantById Output map of spec constants indexed by constant id
 */
void toMap(utils::FixedCapacityVector<Program::SpecializationConstant> const& specConstants,
        std::unordered_map<uint32_t, std::variant<int32_t, float, bool>>& outConstantById) {
    outConstantById.reserve(specConstants.size());
    for (auto const& specConstant: specConstants) {
        outConstantById.emplace(specConstant.id, specConstant.value);
    }
}

}// namespace

WebGPUProgram::WebGPUProgram(wgpu::Device const& device, Program const& program)
    : HwProgram{ program.getName() } {
    std::unordered_map<uint32_t, std::variant<int32_t, float, bool>> specConstants;
    toMap(program.getSpecializationConstants(), specConstants);
    // TODO consider creating/compiling these shaders in parallel
    vertexShaderModule = createShaderModule(device, program, ShaderStage::VERTEX, specConstants);
    fragmentShaderModule =
            createShaderModule(device, program, ShaderStage::FRAGMENT, specConstants);
    computeShaderModule = createShaderModule(device, program, ShaderStage::COMPUTE, specConstants);
}

}// namespace filament::backend
