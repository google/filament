/*!
\brief Contains useful low level utils for shaders (loading, compiling) into low level Api object wrappers.
\file PVRUtils/Vulkan/ShaderUtilsVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/stream/Stream.h"
#include "PVRVk/DeviceVk.h"
#include "PVRUtils/Vulkan/HelperVk.h"

namespace pvr {
namespace utils {
/// <summary>Load a ShaderModule from shader source using glslang.</summary>
/// <param name="device">A device from which to create the ShaderModule</param>
/// <param name="shaderSource">A string containing the shader source text data</param>
/// <param name="shaderStageFlags">The type (stage) of the shader (vertex, fragment...)</param>
/// <param name="flags">A set of pvrvk::ShaderModuleCreateFlags controlling how the ShaderModule will be created</param>
/// <param name="defines">A number of preprocessor definitions that will be passed to the shader</param>
/// <param name="numDefines">The number of defines</param>
/// <returns>The created ShaderModule object</returns>
pvrvk::ShaderModule createShaderModule(pvrvk::Device& device, std::string& shaderSource, pvrvk::ShaderStageFlags shaderStageFlags,
	pvrvk::ShaderModuleCreateFlags flags = pvrvk::ShaderModuleCreateFlags::e_NONE, const char* const* defines = nullptr, uint32_t numDefines = 0);

/// <summary>Load a ShaderModule from shader source using glslang.</summary>
/// <param name="device">A device from which to create the ShaderModule</param>
/// <param name="shaderSource">A stream containing the shader source text data</param>
/// <param name="shaderStageFlags">The type (stage) of the shader (vertex, fragment...)</param>
/// <param name="flags">A set of pvrvk::ShaderModuleCreateFlags controlling how the ShaderModule will be created</param>
/// <param name="defines">A number of preprocessor definitions that will be passed to the shader</param>
/// <param name="numDefines">The number of defines</param>
/// <returns>The created ShaderModule object</returns>
pvrvk::ShaderModule createShaderModule(pvrvk::Device& device, const Stream& shaderSource, pvrvk::ShaderStageFlags shaderStageFlags,
	pvrvk::ShaderModuleCreateFlags flags = pvrvk::ShaderModuleCreateFlags::e_NONE, const char* const* defines = nullptr, uint32_t numDefines = 0);
} // namespace utils
} // namespace pvr
