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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUSTRINGS_H
#define TNT_FILAMENT_BACKEND_WEBGPUSTRINGS_H

#include "WebGPUConstants.h"

#include <backend/DriverEnums.h>

#include <utils/Panic.h>
#include <utils/ostream.h>

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
#include <dawn/webgpu_cpp_print.h>
#endif
#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
#include <sstream>
#endif
#include <string_view>

/**
 * Reusable set of convenience functions for strings -- generally string views, literals, &
 * streaming -- used in the WebGPU backend
 */

namespace filament::backend {

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
/**
 * Convenience template to print WGPU types (which don't support Filament's utils::io::ostream)
 * @tparam WebGPUPrintable e.g. wgpu::FeatureName type
 * @param out stream to print to
 * @param printable the instance to print
 * @return the stream printed to
 *
 * Example usage (because including the template operator override itself does not compile):
 * #if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
 * utils::io::ostream& operator<<(utils::io::ostream& out,
 *       const wgpu::FeatureName featureName) noexcept {
 *   return streamInsertWebGPUPrintable(out, featureName);
 * }
 * #endif
 */
template<typename WebGPUPrintable>
utils::io::ostream& streamInsertWebGPUPrintable(utils::io::ostream& out,
        const WebGPUPrintable printable) {
    std::stringstream printableStream;
    printableStream << printable;
    out << printableStream.str();
    return out;
}
#endif

[[nodiscard]] constexpr std::string_view errorTypeToString(const wgpu::ErrorType errorType) {
    switch (errorType) {
        case wgpu::ErrorType::NoError:     return "NO_ERROR";
        case wgpu::ErrorType::Validation:  return "VALIDATION";
        case wgpu::ErrorType::OutOfMemory: return "OUT_OF_MEMORY";
        case wgpu::ErrorType::Internal:    return "INTERNAL";
        case wgpu::ErrorType::Unknown:     return "UNKNOWN";
    }
}

[[nodiscard]] constexpr std::string_view powerPreferenceToString(
        const wgpu::PowerPreference powerPreference) {
    switch (powerPreference) {
        case wgpu::PowerPreference::Undefined:       return "UNDEFINED";
        case wgpu::PowerPreference::LowPower:        return "LOW_POWER";
        case wgpu::PowerPreference::HighPerformance: return "HIGH_PERFORMANCE";
    }
}

[[nodiscard]] static inline std::string_view powerPreferenceToString(
        const wgpu::DawnAdapterPropertiesPowerPreference powerPreference) {
    return powerPreferenceToString(powerPreference.powerPreference);
}

[[nodiscard]] constexpr std::string_view backendTypeToString(const wgpu::BackendType backendType) {
    switch (backendType) {
        case wgpu::BackendType::Undefined: return "UNDEFINED";
        case wgpu::BackendType::Null:      return "NULL";
        case wgpu::BackendType::WebGPU:    return "WEBGPU";
        case wgpu::BackendType::D3D11:     return "D3D11";
        case wgpu::BackendType::D3D12:     return "D3D12";
        case wgpu::BackendType::Metal:     return "METAL";
        case wgpu::BackendType::Vulkan:    return "VULKAN";
        case wgpu::BackendType::OpenGL:    return "OPENGL";
        case wgpu::BackendType::OpenGLES:  return "OPENGLES";
    }
}

[[nodiscard]] constexpr std::string_view adapterTypeToString(const wgpu::AdapterType adapterType) {
    switch (adapterType) {
        case wgpu::AdapterType::DiscreteGPU:   return "DISCRETE_GPU";
        case wgpu::AdapterType::IntegratedGPU: return "INTEGRATED_GPU";
        case wgpu::AdapterType::CPU:           return "CPU";
        case wgpu::AdapterType::Unknown:       return "UNKNOWN";
    }
}

/**
 * Convenience template to print wgpu::RequestAdapterOptions
 * @tparam STREAM_TYPE useful namely for streaming to either an utils::io::ostream OR
 * utils::details::PanicStream, but also for any kind of stream
 * @param out stream to print to
 * @param options instance to be printed
 * @return stream printed to
 *
 * Example usage (because including the template operator override itself does not compile):
 * template<typename STREAM_TYPE>
 * STREAM_TYPE& operator<<(STREAM_TYPE& out, wgpu::RequestAdapterOptions const& options) noexcept {
 *   return streamInsertRequestAdapterOptions(out, options);
 * }
 */
template<typename STREAM_TYPE>
STREAM_TYPE& streamInsertRequestAdapterOptions(STREAM_TYPE& out,
        wgpu::RequestAdapterOptions const& options) noexcept {
    out << "power preference " << powerPreferenceToString(options.powerPreference)
        << " force fallback adapter " << bool(options.forceFallbackAdapter) << " backend type "
        << backendTypeToString(options.backendType);
    return out;
}

/**
 * Convenience template to print wgpu::AdapterInfo
 * @tparam STREAM_TYPE useful namely for streaming to either an utils::io::ostream OR
 * utils::details::PanicStream, but also for any kind of stream
 * @param out stream to print to
 * @param options instance to be printed
 * @return stream printed to
 *
 * Example usage (because including the template operator override itself does not compile):
 * template<typename STREAM_TYPE>
 * STREAM_TYPE& operator<<(STREAM_TYPE& out, wgpu::AdapterInfo const& info) noexcept {
 *   return streamInsertRequestAdapterInfo(out, info);
 * }
 */
template<class STREAM_TYPE>
STREAM_TYPE& streamInsertRequestAdapterInfo(STREAM_TYPE& out,
        wgpu::AdapterInfo const& info) noexcept {
    out << "vendor (" << info.vendorID << ") '" << info.vendor
        << "' device (" << info.deviceID << ") '" << info.device
        << "' adapter " << adapterTypeToString(info.adapterType)
        << " backend " << backendTypeToString(info.backendType)
        << " architecture '" << info.architecture
        << "' subgroupMinSize " << info.subgroupMinSize
        << " subgroupMaxSize " << info.subgroupMaxSize;
    return out;
}

[[nodiscard]] constexpr std::string_view deviceLostReasonToString(
        const wgpu::DeviceLostReason reason) {
    switch (reason) {
        case wgpu::DeviceLostReason::Unknown:           return "UNKNOWN";
        case wgpu::DeviceLostReason::Destroyed:         return "DESTROYED";
        case wgpu::DeviceLostReason::CallbackCancelled: return "CALLBACK_CANCELLED";
        case wgpu::DeviceLostReason::FailedCreation:    return "FAILED_CREATION";
    }
}

[[nodiscard]] constexpr std::string_view filamentShaderStageToString(ShaderStage stage) {
    switch (stage) {
        case ShaderStage::VERTEX:   return "vertex";
        case ShaderStage::FRAGMENT: return "fragment";
        case ShaderStage::COMPUTE:  return "compute";
    }
}

}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_WEBGPUSTRINGS_H
