/*!
\brief Contains implementation for the ApiErrors utilities
\file PVRUtils/OpenGLES/ErrorsGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRUtils/OpenGLES/ErrorsGles.h"
#include "PVRUtils/OpenGLES/BindingsGles.h"

namespace pvr {
namespace utils {
const char* getGlErrorString(GLuint apiError)
{
	static char buffer[64];
	switch (apiError)
	{
	case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
	case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
	case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
	case GL_NO_ERROR: return "GL_NO_ERROR";
	}
	// Return the HEX code of the error as a std::string.

	sprintf(buffer, "0x%X", apiError);

	return buffer;
}

void throwOnGlError(const char* note)
{
	GLuint err = static_cast<GLuint>(gl::GetError());
	if (err != GL_NO_ERROR) { throw GlError(err, note); }
}

bool succeeded(Result res)
{
	if (res == Result::Success) { return true; }
	else
	{
		throwOnGlError("ApiErrors::succeeded");
		Log(LogLevel::Error, "%s", getResultCodeString(res));
	}
	return false;
}

} // namespace utils
} // namespace pvr
//!\endcond
