/*!
\brief Contains OpenGL ES-specific utilities to facilitate Physically Based Rendering tasks, such as generating irradiance maps and BRDF lookup tables.
\file PVRUtils/OpenGLES/PBRUtilsGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRCore/texture/Texture.h"
#include "PVRUtils/OpenGLES/ConvertToGlesTypes.h"
#include "PVRUtils/OpenGLES/ErrorsGles.h"
#include "PVRUtils/PBRUtils.h"

namespace pvr {
namespace utils {

/// <summary>Generates a mipmapped diffuse irradiance map.</summary>
/// <param name="environmentMap">The OpenGL ES texture to use as the source for the diffuse irradiance map.</param>
/// <param name="outTexture">a pvr::Texture to use for the output diffuse irradiance map.</param>
/// <param name="outTextureGles">An OpenGL ES texture to use as the output for the diffuse irradiance map.</param>
/// <param name="mapSize">The size of the prefiltered environment map</param>
/// <param name="mapNumSamples">The number of samples to use when generating the diffuse irradiance map</param>
void generateIrradianceMap(GLuint environmentMap, pvr::Texture& outTexture, GLuint& outTextureGles, uint32_t mapSize = 64, uint32_t mapNumSamples = 128);

/// <summary>Generate specular irradiance map. Each level of the specular mip map gets blurred corresponding to a roughness value from 0 to 1.0.</summary>
/// <param name="environmentMap">The OpenGL ES texture to use as the source for the prefiltered environment map.</param>
/// <param name="outTexture">a pvr::Texture to use for the output prefiltered environment map.</param>
/// <param name="outTextureGles">An OpenGL ES texture to use as the output for the prefiltered environment map.</param>
/// <param name="mapSize">The size of the prefiltered environment map</param>
/// <param name="zeroRoughnessIsExternal">Denotes that the source environment map itself will be used for the prefiltered environment map mip map level corresponding to a roughness
/// of 0.</param>
/// <param name="numMipLevelsToDiscard">Denotes the number of mip map levels to discard from the bottom of the chain. Generally using the last n mip maps may introduce
/// artifacts.</param>
/// <param name="mapNumSamples">The number of samples to use when generating the prefiltered environment map</param>
void generatePreFilteredMapMipMapStyle(GLuint environmentMap, pvr::Texture& outTexture, GLuint& outTextureGles, uint32_t mapSize, bool zeroRoughnessIsExternal,
	int numMipLevelsToDiscard, uint32_t mapNumSamples = 65536);
} // namespace utils
} // namespace pvr
