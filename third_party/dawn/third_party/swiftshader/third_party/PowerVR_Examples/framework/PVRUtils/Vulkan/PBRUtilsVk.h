/*!
\brief Contains Vulkan-specific utilities to facilitate Physically Based Rendering tasks, such as generating irradiance maps and BRDF lookup tables.
\file PVRUtils/Vulkan/PBRUtilsVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRCore/texture/Texture.h"
#include "PVRVk/PVRVk.h"
#include "PVRUtils/Vulkan/MemoryAllocator.h"
#include "PVRUtils/PBRUtils.h"

namespace pvr {
namespace utils {

/// <summary>Generates a mipmapped diffuse irradiance map.</summary>
/// <param name="queue">A queue to which command buffers containing commands for generating the diffuse irradiance map will be added.</param>
/// <param name="environmentMap">The source environment map to use in the generation</param>
/// <param name="outputPixelFormat">The format to use for the diffuse irradiance map generated using this function.</param>
/// <param name="outputVariableType">The variable type to use for the generated irradiance map generated using this function.</param>
/// <param name="mapSize">The size of the map to generate</param>
/// <param name="mapNumSamples">The number of samples to use when generating the diffuse irradiance map</param>
/// <returns>The pvr::Texture generated</returns>
Texture generateIrradianceMap(pvrvk::Queue queue, pvrvk::ImageView environmentMap, pvr::PixelFormat outputPixelFormat, pvr::VariableType outputVariableType, uint32_t mapSize = 64,
	uint32_t mapNumSamples = 16384);

/// <summary>Generate specular irradiance map. Each level of the specular mip map gets blurred corresponding to a roughness value from 0 to 1.0.</summary>
/// <param name="queue">A queue to which command buffers containing commands for generating the diffuse irradiance map will be added.</param>
/// <param name="environmentMap">The source environment map to use in the generation</param>
/// <param name="outputPixelFormat">The format to use for the diffuse irradiance map generated using this function.</param>
/// <param name="outputVariableType">The variable type to use for the generated irradiance map generated using this function</param>
/// <param name="mapSize">The size of the map to generate</param>
/// <param name="zeroRoughnessIsExternal">Denotes that the source environment map itself will be used for the prefiltered environment map mip map level corresponding to a roughness
/// of 0.</param>
/// <param name="numMipLevelsToDiscard">Denotes the number of mip map levels to discard from the bottom of the chain. Generally using the last n mip maps may introduce
/// artifacts.</param>
/// <param name="mapNumSamples">The number of samples to use when generating the prefiltered environment map</param>
/// <returns>The pvr::Texture generated</returns>
Texture generatePreFilteredMapMipmapStyle(pvrvk::Queue queue, pvrvk::ImageView environmentMap, pvr::PixelFormat outputPixelFormat, pvr::VariableType outputVariableType,
	uint32_t mapSize, bool zeroRoughnessIsExternal, int numMipLevelsToDiscard, uint32_t mapNumSamples = 65536);
} // namespace utils
} // namespace pvr
