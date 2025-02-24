/*!
\brief Contains OpenGL ES-specific utilities to facilitate Physically Based Rendering tasks, such as generating irradiance maps and BRDF lookup tables.
\file PVRUtils/OpenGLES/PBRUtilsGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PBRUtilsGles.h"
#include "PVRCore/IAssetProvider.h"
#include "PVRCore/textureio/TextureWriterPVR.h"
#include "PVRCore/texture/PVRTDecompress.h"
#include "PVRUtils/PVRUtilsTypes.h"
#include "PVRCore/texture/TextureLoad.h"
#include "PVRUtils/OpenGLES/TextureUtilsGles.h"
#include "PVRUtils/OpenGLES/ShaderUtilsGles.h"
#include <iterator>

namespace pvr {
namespace utils {
void generateIrradianceMap(GLuint environmentMap, pvr::Texture& outTexture, GLuint& outTextureGles, uint32_t mapSize, uint32_t mapNumSamples)
{
	// Shaders used for generating the diffuse irradiance map
	// clang-format off
	static const char* vertShaderSrc = 
R"(#version 310 es
layout(location = 0) uniform highp mat3 cubeView;
layout(location = 0) out highp vec3 position;
void main(){
	// Create the quad vertices.
	const mediump vec3 positions[6]= vec3[]
	(
		vec3(-1.0f, 1.0f, 1.0f),// top left
		vec3(-1.0f, -1.0f, 1.0f),// bottom left
		vec3(1.0f, 1.0f, 1.0f),// top right
		vec3(1.0f, 1.0f, 1.0f),// top right
		vec3(-1.0f, -1.0f, 1.0f),// bottom left
		vec3(1.0f, -1.0f, 1.0f)// bottom right
	);

	highp vec3 inVertex = positions[gl_VertexID];

	// Set position
	position = cubeView * inVertex;
	// Calculate ray direction
	gl_Position = vec4(inVertex, 1.0);
})";

	static const char* fragShaderSrcUnTemplated = 
R"(#version 310 es
#define PI 3.1415926535897932384626433832795
layout(binding = 0) uniform highp samplerCube envMap;
layout(location = 0) in highp vec3 position;
layout(location = 0) out highp vec3 outColor;
const highp float NUM_SAMPLES_PER_DIR = %d.;
const highp float DELTA_THETA = 1./NUM_SAMPLES_PER_DIR;
const highp float DELTA_PHI  = 1./NUM_SAMPLES_PER_DIR;

void main()
{
	highp vec3 norm = normalize(position);
	const highp float twoPI = PI * 2.0;
	const highp float halfPI = PI * 0.5;

	highp vec3 out_col_tmp = vec3(0.0);
    
	highp int num_samples = 0;

	// Ensure we are not missing (too many) texels - taking into consideration bilinear filtering and the fact that we are
	// doing a cubemap, we should be looking at a number of samples on the order of more than one sample per "texel".
	
	// Cube faces are square anyway
	highp float tex_size = float(textureSize(envMap, 0).x); 
	highp float lod = max(log2(tex_size / float(NUM_SAMPLES_PER_DIR)) + 1.0, 0.0) ;

	for(highp float theta = 0.0; theta < twoPI; theta += DELTA_THETA)
	{
		for(highp float phi  = 0.; phi < twoPI; phi += DELTA_PHI)
		{
			highp float cosTheta = cos(theta);
			highp float sinPhi = sin(phi);
			highp float sinTheta = sin(theta);
			highp float cosPhi = cos(phi);
			highp vec3 L = normalize(vec3(sinTheta * cosPhi, sinPhi, cosPhi * cosTheta));
			
			highp float factor = dot(N, L);
			if (factor > 0.0001)
			{
				out_col_tmp += textureLod(envMap, L, lod).rgb * factor;
			}
			num_samples += 1;
		}
	}
	outColor = out_col_tmp * PI / float(num_samples);
})";
	// clang-format on

	// Provide the template value for the number of samples to use
	const char* fragShaderSrc = strings::createFormatted(fragShaderSrcUnTemplated, mapNumSamples).c_str();

	GLint stateViewport[4];

	GLint stateActiveProgram = 0;
	GLint stateActiveTexture = 0;
	GLint stateTextureCube0 = 0;
	GLint stateActiveReadFramebuffer = 0;
	GLint stateActiveDrawFramebuffer = 0;

	debugThrowOnApiError("[pvr::utils::generateIrradianceMap] Error on entrance to function");

	// Retrieve various pieces of state which we can reset after generating the diffuse irradiance map
	gl::GetIntegerv(GL_VIEWPORT, stateViewport);
	gl::GetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &stateActiveReadFramebuffer);
	gl::GetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &stateActiveDrawFramebuffer);
	gl::GetIntegerv(GL_ACTIVE_TEXTURE, &stateActiveTexture);
	gl::GetIntegerv(GL_CURRENT_PROGRAM, &stateActiveProgram);
	gl::GetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &stateTextureCube0);

	debugThrowOnApiError("[pvr::utils::generateIrradianceMap] Error storing state");

	// Calculate the maximum number of mip map levels required to create a full mip chain for the output texture
	const uint32_t numMipLevels = static_cast<uint32_t>(log2(static_cast<float>(mapSize)) + 1.0f);

	// calcuate the mip level dimensions
	std::vector<uint32_t> mipLevelDimensions(numMipLevels);
	for (uint32_t i = 0; i < mipLevelDimensions.size(); ++i) { mipLevelDimensions[i] = static_cast<uint32_t>(pow(2, numMipLevels - i - 1)); }

	// Create the shaders
	GLuint shaders[2] = { pvr::utils::loadShader(vertShaderSrc, ShaderType::VertexShader, NULL, 0), pvr::utils::loadShader(fragShaderSrc, ShaderType::FragmentShader, NULL, 0) };
	debugThrowOnApiError("[pvr::utils::generateIrradianceMap] Error creating shaders");

	// Create the program
	GLuint program = pvr::utils::createShaderProgram(shaders, 2, nullptr, nullptr, 0);
	debugThrowOnApiError("[pvr::utils::generateIrradianceMap] Error creating shader program");

	// Create the destination OpenGL ES texture
	gl::GenTextures(1, &outTextureGles);
	gl::ActiveTexture(GL_TEXTURE0);
	gl::BindTexture(GL_TEXTURE_CUBE_MAP, outTextureGles);

	GLenum texFormat = GL_RGB9_E5;
	gl::TexStorage2D(GL_TEXTURE_CUBE_MAP, numMipLevels, texFormat, mapSize, mapSize);
	debugThrowOnApiError("[pvr::utils::generateIrradianceMap] Error creating texture");

	const uint32_t formatStride = 4; // We are going to save this into an GL_RGB9_E5 = 4 bytes
	std::vector<unsigned char> texData(formatStride * mapSize * mapSize * numMipLevels * 6, 0);
	uint32_t dataOffset = 0;

	const glm::mat3 cubeView[6] = {
		glm::mat3(glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(90.f), glm::vec3(0.0f, 1.0f, 0.f))), // +X
		glm::mat3(glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(-90.f), glm::vec3(0.0f, 1.0f, 0.f))), // -X
		glm::mat3(glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(90.f), glm::vec3(1.0f, .0f, 0.f))), // +Y
		glm::mat3(glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(-90.f), glm::vec3(1.0f, .0f, 0.f))), // -Y
		glm::mat3(glm::scale(glm::vec3(1.0f, -1.0f, 1.f))), // +Z
		glm::mat3(glm::scale(glm::vec3(-1.0f, -1.0f, -1.f))), // -Z
	};

	gl::UseProgram(program);
	GLint cubeViewLocation = gl::GetUniformLocation(program, "cubeView");
	gl::BindTexture(GL_TEXTURE_CUBE_MAP, environmentMap);

	// Generate the diffuse irradiance map
	for (uint32_t i = 0; i < numMipLevels; ++i)
	{
		for (uint32_t j = 0; j < 6; ++j)
		{
			// Update the cube view matrix
			gl::UniformMatrix3fv(cubeViewLocation, 1, false, glm::value_ptr(cubeView[j]));

			// Create a temporary framebuffer per face per mipmap
			GLuint fbo;
			gl::GenFramebuffers(1, &fbo);
			gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
			gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, outTextureGles, i);
			debug_assertion(gl::CheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Invalid fbo");
			debugThrowOnApiError("[pvr::utils::generateIrradianceMap] Error creating temporary framebuffer per face per mipmap");

			// Perform the current render
			gl::Viewport(0, 0, mipLevelDimensions[i], mipLevelDimensions[i]);
			gl::DrawArrays(GL_TRIANGLES, 0, 6);
			debugThrowOnApiError("[pvr::utils::generateIrradianceMap] Error DrawArrays");

			// Use the resulting rendered image as the source for a ReadPixels call
			gl::BindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
			gl::ReadBuffer(GL_COLOR_ATTACHMENT0);
			gl::ReadPixels(0, 0, mipLevelDimensions[i], mipLevelDimensions[i], GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV, texData.data() + dataOffset);
			debugThrowOnApiError("[pvr::utils::generateIrradianceMap] Error reading pixels");

			dataOffset += formatStride * mipLevelDimensions[i] * mipLevelDimensions[i];
			gl::DeleteFramebuffers(1, &fbo);
			debugThrowOnApiError("[pvr::utils::generateIrradianceMap] Error deleting temporary framebuffer");
		}
	}

	// Reset stored state
	gl::BindTexture(GL_TEXTURE_CUBE_MAP, stateTextureCube0);
	gl::ActiveTexture(stateActiveTexture);
	gl::BindFramebuffer(GL_READ_FRAMEBUFFER, stateActiveReadFramebuffer);
	gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, stateActiveDrawFramebuffer);
	gl::UseProgram(static_cast<GLuint>(stateActiveProgram));
	gl::Viewport(stateViewport[0], stateViewport[1], stateViewport[2], stateViewport[3]);

	gl::DeleteProgram(program);
	gl::DeleteShader(shaders[0]);
	gl::DeleteShader(shaders[1]);
	debugThrowOnApiError("[pvr::utils::generateIrradianceMap] Error resetting state");

	// Copy the resulting diffuse irradiance map to file
	pvr::TextureHeader texHeader;
	texHeader.setChannelType(pvr::VariableType::UnsignedFloat);
	texHeader.setColorSpace(pvr::ColorSpace::lRGB);
	texHeader.setDepth(1);
	texHeader.setWidth(mapSize);
	texHeader.setHeight(mapSize);
	texHeader.setNumMipMapLevels(numMipLevels);
	texHeader.setNumFaces(6);
	texHeader.setNumArrayMembers(1);
	texHeader.setPixelFormat(pvr::PixelFormat(CompressedPixelFormat::SharedExponentR9G9B9E5));
	outTexture = pvr::Texture(texHeader, texData.data());
}

void generatePreFilteredMapMipMapStyle(
	GLuint environmentMap, pvr::Texture& outTexture, GLuint& outTextureGles, uint32_t mapSize, bool zeroRoughnessIsExternal, int numMipLevelsToDiscard, uint32_t mapNumSamples)
{
	// Shaders used for generating the prefiltered specular map
	// clang-format off
	static const char* vertShaderSrc = 
R"(#version 310 es
layout(location = 0) uniform highp mat3 cubeView;
layout(location = 0) out highp vec3 position;
void main(){
	// Create the quad vertices.
	const mediump vec3 positions[6]= vec3[]
	(
		vec3(-1.0f, 1.0f, 1.0f),// top left
		vec3(-1.0f, -1.0f, 1.0f),// bottom left
		vec3(1.0f, 1.0f, 1.0f),// top right
		vec3(1.0f, 1.0f, 1.0f),// top right
		vec3(-1.0f, -1.0f, 1.0f),// bottom left
		vec3(1.0f, -1.0f, 1.0f)// bottom right
	);

	highp vec3 inVertex = positions[gl_VertexID];

	// Set position
	position = cubeView * inVertex;
	// Calculate ray direction
	gl_Position = vec4(inVertex, 1.0);
})";

	static const char* fragShaderSrcUnTemplated = 
R"(#version 310 es
#define PI 3.1415926535897932384626433832795

layout (location = 0) in highp vec3 position;
layout (location = 0) out highp vec3 outColor;

layout(binding = 0) uniform  highp samplerCube envMap;

layout(location = 1) uniform highp float roughness;

highp vec2 hammersley(uint i, uint N)
{
	highp float vdc = float(bitfieldReverse(i)) * 2.3283064365386963e-10; // Van der Corput
	return vec2(float(i) / float(N), vdc);
}

// Normal Distribution function
highp  float D_GGX(highp float dotNH, highp float roughness)
{
	highp float a = roughness * roughness;
	highp float a2 = a * a;
	highp float denom = dotNH * dotNH * (a2 - 1.0) + 1.0;
	return a2 /(PI * denom * denom);
}

// Sourced from http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
highp  vec3 importanceSampleCGX(highp vec2 xi, highp float roughness, highp vec3 N)
{
	highp float a = roughness * roughness;
	highp float phi = 2.0 * PI * xi.x;
	highp float cosTheta = sqrt( (1.0f - xi.y) / ( 1.0f + (a*a - 1.0f) * xi.y ));
	highp float sinTheta = sqrt( 1.0f - cosTheta * cosTheta);
	highp vec3 h = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
	highp vec3 upVector = abs(N.z) < 0.999f ? vec3(0.0f,0.0f,1.0f) : vec3(1.0f,0.0f,0.0f);
	highp vec3 tangentX = normalize( cross( upVector, N ) );
	highp vec3 tangentY = cross( N, tangentX );
	// Tangent to world space
	return (tangentX * h.x) + (tangentY * h.y) + (N * h.z);
}

// Sourced from http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
void preFilterEnvMap(highp vec3 R, highp float roughness)
{
	highp vec3 N = R;
	highp vec3 V = R;

	highp vec4 result0 = vec4(0.0);
	highp const uint numSamples = %du;
	highp float mapSize = float(textureSize(envMap, 0).x);

	highp float omegaP = 4.0 * PI / (6.0 * mapSize * mapSize);
	highp float mipBias = 1.0f; // Original paper suggest biasing the mip to improve the results

	for(uint i = 0u; i < numSamples; ++i)
	{
		highp  vec2 Xi = hammersley(i, numSamples);
		highp  vec3 H = importanceSampleCGX(Xi, roughness, N);
		highp  vec3 L = 2.0 * dot(V, H) * H - V;

		highp  float NoL = max(dot(N, L), 0.0);
		if(NoL > 0.0)
		{
			// We will usually not do roughness == 0. We should start from the first roughness value
			if(roughness == 0.0)
			{
				result0 = vec4(textureLod(envMap, L, 0.0).rgb * NoL, 0.0);
				break;
			}

			// optmize: https://placeholderart.wordpress.com/2015/07/28/implementation-notes-runtime-environment-map-filtering-for-image-based-lighting/
			highp float NoH = max(dot(N, H), 0.0);
			highp float VoH = max(dot(V,H), 0.0);
			highp float NoV = max(dot(N, V), 0.0);
			// Probability Distribution Function
			highp float pdf = D_GGX(NoH, roughness) * NoH / ((4.0f * VoH) + 0.0001) /*avoid division by 0*/;

			// Solid angle represented by this sample
			highp float omegaS = 1.0 / (float(numSamples) * pdf);
			// Solid angle covered by 1 pixel with 6 faces that are EnvMapSize X EnvMapSize

			highp float mipLevel = max(0.5 * log2(omegaS / omegaP) + mipBias, 0.0f);
			result0 += vec4(textureLod(envMap, L, mipLevel).rgb * NoL, NoL);
		}
	}
	if(result0.w != 0.0)
	{
		result0.rgb = result0.rgb / result0.w; // divide by the weight
	}
	outColor = result0.rgb;
}

void main()
{
	preFilterEnvMap(normalize(position), roughness);
})";
	// clang-format on

	// Provide the template value for the number of samples to use
	const char* fragShaderSrc = strings::createFormatted(fragShaderSrcUnTemplated, mapNumSamples).c_str();

	GLint stateViewport[4];

	GLint stateActiveProgram = 0;
	GLint stateActiveTexture = 0;
	GLint stateTextureCube0 = 0;
	GLint stateActiveReadFramebuffer = 0;
	GLint stateActiveDrawFramebuffer = 0;

	debugThrowOnApiError("[pvr::utils::generatePreFilteredMap] Error on entrance to function");

	// Retrieve various pieces of state which we can reset after generating the prefiltered specular map
	gl::GetIntegerv(GL_VIEWPORT, stateViewport);
	gl::GetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &stateActiveReadFramebuffer);
	gl::GetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &stateActiveDrawFramebuffer);
	gl::GetIntegerv(GL_ACTIVE_TEXTURE, &stateActiveTexture);
	gl::GetIntegerv(GL_CURRENT_PROGRAM, &stateActiveProgram);
	gl::GetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &stateTextureCube0);

	debugThrowOnApiError("[pvr::utils::generatePreFilteredMap] Error storing state");

	// Create the shaders
	GLuint shaders[2] = { pvr::utils::loadShader(vertShaderSrc, ShaderType::VertexShader, NULL, 0), pvr::utils::loadShader(fragShaderSrc, ShaderType::FragmentShader, NULL, 0) };
	debugThrowOnApiError("[pvr::utils::generatePreFilteredMap] Error creating shaders");

	// Creating the program
	GLuint program = pvr::utils::createShaderProgram(shaders, 2, nullptr, nullptr, 0);
	debugThrowOnApiError("[pvr::utils::generatePreFilteredMap] Error creating shader program");

	// Calculate the maximum number of mip map levels required to create a full mip chain for the output texture
	uint32_t numMipLevels = static_cast<uint32_t>(log2(static_cast<float>(mapSize)) + 1.0f - numMipLevelsToDiscard); // prefilterMap

	// calcuate the mip level dimensions
	std::vector<uint32_t> mipLevelDimensions(numMipLevels);
	for (uint32_t i = 0; i < mipLevelDimensions.size(); ++i) { mipLevelDimensions[i] = static_cast<uint32_t>(pow(2, numMipLevels + numMipLevelsToDiscard - 1 - i)); }

	// Create the destination OpenGL ES texture
	gl::GenTextures(1, &outTextureGles);
	gl::ActiveTexture(GL_TEXTURE0);
	gl::BindTexture(GL_TEXTURE_CUBE_MAP, outTextureGles);

	const GLenum texFormat = GL_RGB9_E5;
	gl::TexStorage2D(GL_TEXTURE_CUBE_MAP, numMipLevels, texFormat, static_cast<GLsizei>(mapSize), static_cast<GLsizei>(mapSize));
	debugThrowOnApiError("[pvr::utils::generatePreFilteredMap] Error creating texture");

	const uint32_t formatStride = 4; // We are going to save this into an GL_RGB9_E5 = 4 bytes

	std::vector<unsigned char> texDataIrrRoughness(formatStride * mapSize * mapSize * numMipLevels * 6);
	uint32_t dataOffset = 0;

	const glm::mat3 cubeView[6] = {
		glm::mat3(glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(90.f), glm::vec3(0.0f, 1.0f, 0.f))), // +X
		glm::mat3(glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(-90.f), glm::vec3(0.0f, 1.0f, 0.f))), // -X
		glm::mat3(glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(90.f), glm::vec3(1.0f, .0f, 0.f))), // +Y
		glm::mat3(glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(-90.f), glm::vec3(1.0f, .0f, 0.f))), // -Y
		glm::mat3(glm::scale(glm::vec3(1.0f, -1.0f, 1.f))), // +Z
		glm::mat3(glm::scale(glm::vec3(-1.0f, -1.0f, -1.f))), // -Z
	};

	gl::UseProgram(program);
	GLint cubeViewLocation = gl::GetUniformLocation(program, "cubeView");
	gl::BindTexture(GL_TEXTURE_CUBE_MAP, environmentMap);

	gl::ActiveTexture(GL_TEXTURE1);
	gl::BindTexture(GL_TEXTURE_CUBE_MAP, outTextureGles);

	float maxmip = (float)(numMipLevels - 1);

	for (uint32_t mipLevel = 0; mipLevel < numMipLevels; ++mipLevel)
	{
		for (uint32_t j = 0; j < 6; ++j)
		{
			// We are generate roughnesses for all mipmaps but the first.
			// In other words, it is assumed that the shader will use the environment map itself for roughness = 0;

			// In the general case where we do roughness 0..1 to all mips, roughness = miplevel / maxmiplevel
			float mip = static_cast<float>(mipLevel);
			float roughness = mip / maxmip;
			if (zeroRoughnessIsExternal)
			{
				// ... But, in the case where we skip the top mip level (because we plan on just using the
				// environment map for it), the equation is a bit more involved, so that
				// it correctly calculates where we switch from interpolating among lods in the prefiltered map to
				// interpolating between the environment map and the first lod of the prefiltered map
				// LOD = maxmip * (roughness - 1/maxmip) / (1 - 1/maxmip)
				// = > ... => roughness = (LOD / maxmip) * (1 - 1/maxmip) + 1/maxmip

				roughness = mip * (1.f / maxmip) * (1.f - 1.f / maxmip) + 1.f / maxmip;
			}

			// Update the cube view matrix
			gl::UniformMatrix3fv(cubeViewLocation, 1, false, glm::value_ptr(cubeView[j]));

			// Create a temporary framebuffer per face per mipmap
			GLuint fbo;
			gl::GenFramebuffers(1, &fbo);
			gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
			gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, outTextureGles, mipLevel);
			debug_assertion(gl::CheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Invalid fbo");
			debugThrowOnApiError("[pvr::utils::generatePreFilteredMap] Error creating temporary framebuffer per face per mipmap");

			// Perform the current render
			GLenum bufs[1] = { GL_COLOR_ATTACHMENT0 };
			gl::DrawBuffers(ARRAY_SIZE(bufs), bufs);
			gl::Viewport(0, 0, mipLevelDimensions[mipLevel], mipLevelDimensions[mipLevel]);
			gl::Uniform1f(1, roughness);
			gl::DrawArrays(GL_TRIANGLES, 0, 6);

			debugThrowOnApiError("[pvr::utils::generatePreFilteredMap] Error DrawArrays");

			// Use the resulting rendered image as the source for a ReadPixels call
			gl::BindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
			gl::ReadBuffer(GL_COLOR_ATTACHMENT0);
			gl::ReadPixels(0, 0, mipLevelDimensions[mipLevel], mipLevelDimensions[mipLevel], GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV, texDataIrrRoughness.data() + dataOffset);
			debugThrowOnApiError("[pvr::utils::generatePreFilteredMap] Error reading pixels");

			dataOffset += formatStride * mipLevelDimensions[mipLevel] * mipLevelDimensions[mipLevel];
			gl::DeleteFramebuffers(1, &fbo);
			debugThrowOnApiError("[pvr::utils::generatePreFilteredMap] Error deleting temporary framebuffer");
		}
	}

	// Reset stored state
	gl::BindTexture(GL_TEXTURE_CUBE_MAP, stateTextureCube0);
	gl::ActiveTexture(stateActiveTexture);
	gl::BindFramebuffer(GL_READ_FRAMEBUFFER, stateActiveReadFramebuffer);
	gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, stateActiveDrawFramebuffer);
	gl::UseProgram(static_cast<GLuint>(stateActiveProgram));
	gl::Viewport(stateViewport[0], stateViewport[1], stateViewport[2], stateViewport[3]);

	gl::DeleteProgram(program);
	gl::DeleteProgram(program);
	gl::DeleteShader(shaders[0]);
	gl::DeleteShader(shaders[1]);
	debugThrowOnApiError("[pvr::utils::generatePreFilteredMap] Error resetting state");

	// Copy the resulting prefiltered specular map to file
	pvr::TextureHeader texHeader;
	texHeader.setChannelType(pvr::VariableType::UnsignedFloat);
	texHeader.setColorSpace(pvr::ColorSpace::lRGB);
	texHeader.setDepth(1);
	texHeader.setWidth(mapSize);
	texHeader.setHeight(mapSize);
	texHeader.setNumMipMapLevels(numMipLevels);
	texHeader.setNumFaces(6);
	texHeader.setNumArrayMembers(1);
	texHeader.setPixelFormat(pvr::PixelFormat(pvr::PixelFormat::RGB_111110()));
	outTexture = Texture(texHeader, texDataIrrRoughness.data());

} // namespace utils
} // namespace utils
} // namespace pvr
