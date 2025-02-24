/*!
\brief Contains useful low level utils for shaders (loading, compiling) into low level Api object wrappers.
\file PVRUtils/OpenGLES/ShaderUtilsGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "BindingsGles.h"
#include "PVRUtils/OpenGLES/ConvertToGlesTypes.h"
#include "PVRCore/IAssetProvider.h"
#include "PVRUtils/PVRUtilsTypes.h"
#include "PVRCore/texture/TextureLoad.h"

namespace pvr {
namespace utils {

/// <summary>Load shader from shader source. Will implicitly load on the current context.</summary>
/// <param name="shaderSource">A stream containing the shader source text data</param>
/// <param name="shaderType">The type (stage) of the shader (vertex, fragment...)</param>
/// <param name="defines">A number of preprocessor definitions that will be passed to the shader</param>
/// <param name="numDefines">The number of defines</param>
/// <returns>The shader object</returns>
GLuint loadShader(const Stream& shaderSource, ShaderType shaderType, const char* const* defines, uint32_t numDefines);

/// <summary>Load shader from shader source. Will implicitly load on the current context.</summary>
/// <param name="shaderSource">A string containing the shader source text data</param>
/// <param name="shaderType">The type (stage) of the shader (vertex, fragment...)</param>
/// <param name="defines">A number of preprocessor definitions that will be passed to the shader</param>
/// <param name="numDefines">The number of defines</param>
/// <returns>The shader object</returns>
GLuint loadShader(const std::string& shaderSource, ShaderType shaderType, const char* const* defines, uint32_t numDefines);

/// <summary>Create a native shader program from an array of native shader handles. Will implicitly load on the current context.</summary>
/// <param name="pShaders">An array of shaders</param>
/// <param name="shadersCount">The number shaders in <paramref name="pShaders">pShaders</paramref></param>
/// <param name="attribNames">The list of names of the attributes in the shader, as a c-style array of c-style strings</param>
/// <param name="attribIndices">The list of attribute binding indices, corresponding to <paramref name="attribNames">attribNames</paramref></param>
/// <param name="attribsCount">Number of attributes in <paramref name="attribNames">attribNames</paramref> and <paramref name="attribIndices">attribIndices</paramref>.</param>
/// <param name="infolog">OPTIONAL Output, the infolog of the shader</param>
/// <returns>The program object</returns>
GLuint createShaderProgram(
	const GLuint pShaders[], uint32_t shadersCount, const char** const attribNames, const uint16_t* attribIndices, uint32_t attribsCount, std::string* infolog = NULL);

/// <summary>Create a native shader program from a compute shader</summary>
/// <param name="app">An AssetProvider to use for loading shaders from memory</param>
/// <param name="compShaderFilename">The filename of a compute shader</param>
/// <param name="defines">A list of defines to be added to the shaders</param>
/// <param name="numDefines">The number of defines to be added to the shaders</param>
/// <returns>The program object</returns>
inline GLuint createComputeShaderProgram(IAssetProvider& app, const char* compShaderFilename, const char* const* defines = 0, uint32_t numDefines = 0)
{
	GLuint shader = 0;
	GLuint program = 0;

	auto compShaderSrc = app.getAssetStream(compShaderFilename);

	if (!compShaderSrc)
	{
		Log("Failed to open compute shader %s", compShaderFilename);
		return false;
	}

	shader = pvr::utils::loadShader(*compShaderSrc, pvr::ShaderType::ComputeShader, defines, numDefines);

	program = pvr::utils::createShaderProgram(&shader, 1, 0, 0, 0);
	gl::DeleteShader(shader);

	return program;
}

/// <summary>Create a native shader program from a vertex, fragment, tessellation control, tessellation evaluation and geometry shader</summary>
/// <param name="app">An AssetProvider to use for loading shaders from memory</param>
/// <param name="vertShaderFilename">The filename of a vertex shader</param>
/// <param name="tessCtrlShaderFilename">The filename of a tessellation control shader</param>
/// <param name="tessEvalShaderFilename">The filename of a tessellation evaluation shader</param>
/// <param name="geometryShaderFilename">The filename of a geometry shader</param>
/// <param name="fragShaderFilename">The filename of a fragment shader</param>
/// <param name="attribNames">The list of names of the attributes in the shader, as a c-style array of c-style strings</param>
/// <param name="attribIndices">The list of attribute binding indices, corresponding to <paramref name="attribNames">attribNames</paramref></param>
/// <param name="numAttribs">Number of attributes in <paramref name="attribNames">attribNames</paramref> and <paramref name="attribIndices">attribIndices</paramref>.</param>
/// <param name="defines">A list of defines to be added to the shaders</param>
/// <param name="numDefines">The number of defines to be added to the shaders</param>
/// <returns>The program object</returns>
inline GLuint createShaderProgram(const IAssetProvider& app, const char* vertShaderFilename, const char* tessCtrlShaderFilename, const char* tessEvalShaderFilename,
	const char* geometryShaderFilename, const char* fragShaderFilename, const char** attribNames, const uint16_t* attribIndices, uint32_t numAttribs,
	const char* const* defines = 0, uint32_t numDefines = 0)
{
	GLuint shaders[6] = { 0 };
	GLuint program = 0;
	uint32_t count = 0;
	if (vertShaderFilename)
	{
		auto vertShaderSrc = app.getAssetStream(vertShaderFilename);
		shaders[count++] = pvr::utils::loadShader(*vertShaderSrc, pvr::ShaderType::VertexShader, defines, numDefines);
	}

	if (tessCtrlShaderFilename)
	{
		auto texCtrShaderSrc = app.getAssetStream(tessCtrlShaderFilename);
		shaders[count++] = pvr::utils::loadShader(*texCtrShaderSrc, pvr::ShaderType::TessControlShader, defines, numDefines);
	}

	if (tessEvalShaderFilename)
	{
		auto texEvalShaderSrc = app.getAssetStream(tessEvalShaderFilename);
		shaders[count++] = pvr::utils::loadShader(*texEvalShaderSrc, pvr::ShaderType::TessEvaluationShader, defines, numDefines);
	}

	if (geometryShaderFilename)
	{
		auto geometryShaderSrc = app.getAssetStream(geometryShaderFilename);
		shaders[count++] = pvr::utils::loadShader(*geometryShaderSrc, pvr::ShaderType::GeometryShader, defines, numDefines);
	}

	if (fragShaderFilename)
	{
		auto fragShaderSrc = app.getAssetStream(fragShaderFilename);
		shaders[count++] = pvr::utils::loadShader(*fragShaderSrc, pvr::ShaderType::FragmentShader, defines, numDefines);
	}

	program = pvr::utils::createShaderProgram(shaders, count, attribNames, attribIndices, numAttribs);
	for (uint32_t i = 0; i < count; ++i) { gl::DeleteShader(shaders[i]); }

	return program;
}

/// <summary>Create a native shader program from a vertex and fragment shader</summary>
/// <param name="app">An AssetProvider to use for loading shaders from memory</param>
/// <param name="vertShaderFilename">The filename of a vertex shader</param>
/// <param name="fragShaderFilename">The filename of a fragment shader</param>
/// <param name="attribNames">The list of names of the attributes in the shader, as a c-style array of c-style strings</param>
/// <param name="attribIndices">The list of attribute binding indices, corresponding to <paramref name="attribNames">attribNames</paramref></param>
/// <param name="numAttribs">Number of attributes in <paramref name="attribNames">attribNames</paramref> and <paramref name="attribIndices">attribIndices</paramref>.</param>
/// <param name="defines">A list of defines to be added to the shaders</param>
/// <param name="numDefines">The number of defines to be added to the shaders</param>
/// <returns>The program object</returns>
inline GLuint createShaderProgram(const IAssetProvider& app, const char* vertShaderFilename, const char* fragShaderFilename, const char** attribNames, const uint16_t* attribIndices,
	uint32_t numAttribs, const char* const* defines = 0, uint32_t numDefines = 0)
{
	return createShaderProgram(app, vertShaderFilename, nullptr, nullptr, nullptr, fragShaderFilename, attribNames, attribIndices, numAttribs, defines, numDefines);
}
} // namespace utils
} // namespace pvr
