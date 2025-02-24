/*!
\brief Contains Texture utility helpers.
\file PVRUtils/PBRUtils.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRCore/glm.h"
#include "PVRCore/texture/Texture.h"
namespace pvr {
namespace utils {
namespace {

inline glm::vec2 hammersley(uint32_t i, uint32_t N)
{
	// Radical inverse based on http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
	uint32_t bits = (i << 16u) | (i >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	float rdi = float(bits) * static_cast<float>(2.3283064365386963e-10);
	return glm::vec2(float(i) / float(N), rdi);
}

inline float G1(float k, float NoV) { return NoV / (NoV * (1.0f - k) + k); }

// Geometric Shadowing function
float gSmith(float NoL, float NoV, float roughness)
{
	float k = (roughness * roughness) * 0.5f;
	return G1(k, NoL) * G1(k, NoV);
}

// Sample a half-vector in world space
// Based on http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_slides.pdf
glm::vec3 importanceSampleGGX(glm::vec2 Xi, float roughness, glm::vec3 N)
{
	// Maps a 2D point to a hemisphere with spread based on roughness
	float a = roughness * roughness;
	float phi = 2.0f * glm::pi<float>() * Xi.x;
	float cosTheta = sqrt(glm::clamp((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y), 0.0f, 1.0f));
	float sinTheta = sqrt(glm::clamp(1.0f - cosTheta * cosTheta, 0.0f, 1.0f));

	glm::vec3 H = glm::vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

	glm::vec3 up = glm::abs(N.z) < 0.999 ? glm::vec3(0, 0, 1) : glm::vec3(1, 0, 0);
	glm::vec3 tangent = glm::normalize(glm::cross(up, N));
	glm::vec3 bitangent = glm::cross(N, tangent);

	return glm::normalize(tangent * H.x + bitangent * H.y + N * H.z);
}

// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
glm::vec2 integrateBRDF(float roughness, float NoV)
{
	const glm::vec3 N = glm::vec3(0.0, 0.0, 1.0); // normal always pointing forward.
	const glm::vec3 V = glm::vec3(sqrt(glm::clamp(1.0 - NoV * NoV, 0.0, 1.0)), 0.0, NoV);
	float A = 0.0f;
	float B = 0.0f;

	uint32_t numSamples = 1024u;
	for (uint32_t i = 0u; i < numSamples; ++i)
	{
		glm::vec2 Xi = hammersley(i, numSamples);
		glm::vec3 H = importanceSampleGGX(Xi, roughness, N);
		glm::vec3 L = 2.0f * glm::dot(V, H) * H - V;

		float NoL = glm::max(glm::dot(N, L), 0.0f);
		if (NoL > 0.0f)
		{
			float NoH = glm::max(glm::dot(N, H), 0.001f);
			float VoH = glm::max(glm::dot(V, H), 0.001f);
			float currentNoV = glm::max(glm::dot(N, V), 0.001f);

			const float G = gSmith(NoL, currentNoV, roughness);

			const float G_Vis = (G * VoH) / (NoH * currentNoV /*avoid division by zero*/);
			const float Fc = pow(1.0f - VoH, 5.0f);

			A += (1.0f - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}

	return glm::vec2(A, B) / float(numSamples);
}
} // namespace

/// <summary>Generates BRDF lookup table image.</summary>
/// <param name="mapDim">Out put image size. Default 256</param>
/// <returns>A generated texture containing a Cook Torrance BRDF Lookup table</returns>
inline pvr::Texture generateCookTorranceBRDFLUT(uint32_t mapDim = 256)
{
	pvr::TextureHeader header;
	header.setWidth(mapDim);
	header.setHeight(mapDim);
	header.setChannelType(pvr::VariableType::SignedFloat);
	header.setNumFaces(1);
	header.setNumMipMapLevels(1);
	header.setPixelFormat(pvr::PixelFormat::RG_1616());

	pvr::Texture returnTex(header);

	const uint32_t stride = sizeof(glm::detail::hdata);
	const uint32_t formatStride = stride * 2;

	uint32_t offset = 0;
	for (uint32_t j = 0; j < mapDim; ++j) // y
	{
		for (uint32_t i = 0; i < mapDim; ++i) // x
		{
			glm::vec2 v2 = integrateBRDF((static_cast<float>(j) + .5f) / static_cast<float>(mapDim), ((static_cast<float>(i) + .5f) / static_cast<float>(mapDim)));
			glm::detail::hdata halfR = glm::detail::toFloat16(v2.r);
			glm::detail::hdata halfG = glm::detail::toFloat16(v2.g);

			memcpy(returnTex.getDataPointer() + offset, &halfR, stride);
			memcpy(returnTex.getDataPointer() + offset + stride, &halfG, stride);
			offset += formatStride;
		}
	}

	return returnTex;
}
} // namespace utils
} // namespace pvr
