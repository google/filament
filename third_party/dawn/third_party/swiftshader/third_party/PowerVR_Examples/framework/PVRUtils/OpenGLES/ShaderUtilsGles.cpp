/*!
\brief Implementations of the shader utility functions
\file PVRUtils/OpenGLES/ShaderUtilsGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRCore/stream/Stream.h"
#include "PVRCore/strings/StringFunctions.h"
#include "PVRUtils/OpenGLES/ShaderUtilsGles.h"
#include "PVRUtils/OpenGLES/BindingsGles.h"
#include "PVRUtils/OpenGLES/ErrorsGles.h"

namespace pvr {
namespace utils {

namespace {
GLenum getGlslShaderType(ShaderType shaderType)
{
	GLenum glEnum = static_cast<GLenum>(-1);
	switch (shaderType)
	{
	case ShaderType::VertexShader: glEnum = GL_VERTEX_SHADER; break;
	case ShaderType::FragmentShader: glEnum = GL_FRAGMENT_SHADER; break;
	case ShaderType::ComputeShader:
#if defined(GL_COMPUTE_SHADER)
		glEnum = GL_COMPUTE_SHADER;
#else
		throw InvalidOperationError("loadShader: Compute Shader not supported on this context");
#endif
		break;
	case ShaderType::GeometryShader:
#if defined(GL_GEOMETRY_SHADER_EXT)
		glEnum = GL_GEOMETRY_SHADER_EXT;
#else
		throw InvalidOperationError("loadShader: Geometry Shader not supported on this context");
#endif

		break;
	case ShaderType::TessControlShader:
#if defined(GL_TESS_CONTROL_SHADER_EXT)
		glEnum = GL_TESS_CONTROL_SHADER_EXT;
#else
		throw InvalidOperationError("loadShader: Tessellation not supported on this context");
#endif
		break;
	case ShaderType::TessEvaluationShader:
#if defined(GL_TESS_EVALUATION_SHADER_EXT)
		glEnum = GL_TESS_EVALUATION_SHADER_EXT;
#else
		throw InvalidOperationError("loadShader: Tessellation not supported on this context");
#endif
		break;
	default: throw InvalidOperationError("loadShader: Unknown shader type requested.");
	}
	return glEnum;
}
} // namespace

static inline GLuint loadShaderUtil(const std::string& shaderSrc, ShaderType shaderType, const char* const* defines, uint32_t defineCount)
{
	throwOnGlError("loadShader: Error on entry!");

	GLuint outShader = gl::CreateShader(getGlslShaderType(shaderType));

	// Determine whether a version string is present
	std::string::size_type versionBegin = shaderSrc.find("#version");
	std::string::size_type versionEnd = 0;
	std::string sourceDataStr;
	// If a version string is present then update the versionBegin variable to the position after the version string
	if (versionBegin != std::string::npos)
	{
		versionEnd = shaderSrc.find("\n", versionBegin);
		sourceDataStr.append(shaderSrc.begin() + versionBegin, shaderSrc.begin() + versionBegin + versionEnd);
		sourceDataStr.append("\n");
	}
	else
	{
		versionBegin = 0;
	}
	// Insert the defines after the version string if one is present
	for (uint32_t i = 0; i < defineCount; ++i)
	{
		sourceDataStr.append("#define ");
		sourceDataStr.append(defines[i]);
		sourceDataStr.append("\n");
	}
	sourceDataStr.append("\n");
	sourceDataStr.append(shaderSrc.begin() + versionBegin + versionEnd, shaderSrc.end());
	const char* pSource = sourceDataStr.c_str();

	gl::ShaderSource(outShader, 1, &pSource, NULL);
	throwOnGlError("CreateShader::glShaderSource error");
	gl::CompileShader(outShader);

	throwOnGlError("CreateShader::glCompile error");

	return outShader;
}

GLuint loadShader(const std::string& shaderSrc, ShaderType shaderType, const char* const* defines, uint32_t defineCount)
{
	GLuint outShader = loadShaderUtil(shaderSrc, shaderType, defines, defineCount);

	// error checking
	GLint glRslt;
	gl::GetShaderiv(outShader, GL_COMPILE_STATUS, &glRslt);
	if (!glRslt)
	{
		int infoLogLength, charsWritten;
		// get the length of the log
		gl::GetShaderiv(outShader, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<char> pLog;
		pLog.resize(infoLogLength);
		gl::GetShaderInfoLog(outShader, infoLogLength, &charsWritten, pLog.data());
		std::string typestring = to_string(shaderType);

		std::string str = strings::createFormatted("Failed to compile %s shader.\n "
												   "==========Infolog:==========\n%s\n============================",
			typestring.c_str(), pLog.data());
		Log(LogLevel::Error, str.c_str());
		throw InvalidOperationError(str);
	}
	throwOnGlError("CreateShader::exit");
	return outShader;
}

GLuint loadShader(const Stream& shaderSource, ShaderType shaderType, const char* const* defines, uint32_t defineCount)
{
#ifdef DEBUG
	static int idx = 0;
	Log(LogLevel::Information, "Compiling shader %d", idx++);
#endif
	throwOnGlError("loadShader: Error on entry!");

	std::string shaderSrc;
	shaderSource.readIntoString(shaderSrc);

	GLuint outShader = loadShaderUtil(shaderSrc, shaderType, defines, defineCount);

	// error checking
	GLint glRslt;
	gl::GetShaderiv(outShader, GL_COMPILE_STATUS, &glRslt);
	if (!glRslt)
	{
		int infoLogLength, charsWritten;
		// get the length of the log
		gl::GetShaderiv(outShader, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<char> pLog;
		pLog.resize(infoLogLength);
		gl::GetShaderInfoLog(outShader, infoLogLength, &charsWritten, pLog.data());
		std::string typestring = to_string(shaderType);

		std::string str = strings::createFormatted("Failed to compile %s shader: %s.\n "
												   "==========Infolog:==========\n%s\n============================",
			typestring.c_str(), shaderSource.getFileName().c_str(), pLog.data());
		Log(LogLevel::Error, str.c_str());
		throw InvalidOperationError(str);
	}
	throwOnGlError("CreateShader::exit");
	return outShader;
}

GLuint createShaderProgram(const GLuint pShaders[], uint32_t count, const char** const sAttribs, const uint16_t* attribIndex, uint32_t attribCount, std::string* infologptr)
{
	pvr::utils::throwOnGlError("createShaderProgram begin");
	GLuint outShaderProg = gl::CreateProgram();

	for (uint32_t i = 0; i < count; ++i)
	{
		pvr::utils::throwOnGlError("createShaderProgram begin AttachShader");
		gl::AttachShader(outShaderProg, pShaders[i]);
		pvr::utils::throwOnGlError("createShaderProgram end AttachShader");
	}
	if (sAttribs && attribCount)
	{
		for (uint32_t i = 0; i < attribCount; ++i) { gl::BindAttribLocation(outShaderProg, attribIndex[i], sAttribs[i]); }
	}
	pvr::utils::throwOnGlError("createShaderProgram begin linkProgram");
	gl::LinkProgram(outShaderProg);
	pvr::utils::throwOnGlError("createShaderProgram end linkProgram");
	// check for link sucess
	GLint glStatus;

	gl::GetProgramiv(outShaderProg, GL_LINK_STATUS, &glStatus);
	std::string default_infolog;
	std::string& infolog = infologptr ? *infologptr : default_infolog;
	if (!glStatus)
	{
		int32_t infoLogLength, charWriten;
		gl::GetProgramiv(outShaderProg, GL_INFO_LOG_LENGTH, &infoLogLength);
		infolog.resize(infoLogLength);
		if (infoLogLength)
		{
			gl::GetProgramInfoLog(outShaderProg, infoLogLength, &charWriten, &infolog[0]);
			Log(LogLevel::Debug, infolog.c_str());
			throw InvalidOperationError("Failed to link program with infolog " + infolog);
		}
		throw InvalidOperationError("Failed to link shader");
	}
	pvr::utils::throwOnGlError("createShaderProgram end");
	return outShaderProg;
}
} // namespace utils
} // namespace pvr
//!\endcond
