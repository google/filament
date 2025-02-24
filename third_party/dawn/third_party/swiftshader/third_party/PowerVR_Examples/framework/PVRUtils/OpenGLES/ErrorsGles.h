/*!
\brief Convenience functions for automatically logging OpenGL ES errors. Some functions NOP on release builds.
\file PVRUtils/OpenGLES/ErrorsGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRUtils/OpenGLES/BindingsGles.h"
namespace pvr {
namespace utils {
/// <summary>Retrieves a string representation of an OpenGLES error code.</summary>
/// <param name="apiError">The OpenGLES error code to stringify.</param>
/// <returns>The string representation of the given OpenGLES error code.</returns>
const char* getGlErrorString(GLuint apiError);
} // namespace utils
/// <summary>A simple std::runtime_error wrapper for OpenGLES error codes.</summary>
class GlError : public PvrError
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="errorCode">The OpenGLES error code to log.</param>
	GlError(GLuint errorCode) : PvrError(std::string("OpenGL ES Error occured: [") + utils::getGlErrorString(errorCode) + "]") {}
	/// <summary>Constructor.</summary>
	/// <param name="errorCode">The OpenGLES error code to log.</param>
	/// <param name="message">A message to log alongside the OpenGLES error code.</param>
	GlError(GLuint errorCode, const std::string& message) : PvrError(std::string("OpenGL ES Error occured: [") + utils::getGlErrorString(errorCode) + "] -- " + message) {}
	/// <summary>Constructor.</summary>
	/// <param name="errorCode">The OpenGLES error code to log.</param>
	/// <param name="message">A message to log alongside the OpenGLES error code.</param>
	GlError(GLuint errorCode, const char* message)
		: PvrError(message ? std::string("OpenGL ES Error occured: [") + utils::getGlErrorString(errorCode) + "] -- " + message
						   : std::string("OpenGL ES Error occured: [") + utils::getGlErrorString(errorCode) + "]")
	{}
};

/// <summary>A simple std::runtime_error wrapper for OpenGLES extension not supported error codes.</summary>
class GlExtensionNotSupportedError : public PvrError
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="extensionString">The unsunpported OpenGLES extension name to log.</param>
	GlExtensionNotSupportedError(const std::string& extensionString) : PvrError("Error: Required extension not supported [" + extensionString + "]") {}
	/// <summary>Constructor.</summary>
	/// <param name="extensionString">The unsunpported OpenGLES extension name to log.</param>
	/// <param name="message">A message to log alongside the unsunpported OpenGLES extension name.</param>
	GlExtensionNotSupportedError(const std::string& extensionString, const std::string& message)
		: PvrError("Error: Required extension not supported [" + extensionString + "] -- " + message)
	{}
};

namespace utils {
/// <summary>Checks and returns api error if appropriate.</summary>
/// <param name="errOutStr">error std::string to be output.</param>
/// <returns>api error code</returns>
int checkApiError(std::string* errOutStr = NULL);

/// <summary>Checks and logs api errors if appropriate.</summary>
/// <param name="note">A c-style std::string that will be prepended to the error description if an error is found.</param>
void throwOnGlError(const char* note);
} // namespace utils
} // namespace pvr

#ifdef DEBUG
/// <summary>Checks for API errors if the API supports them. If an error is detected, logs relevant error
/// information. Only works in debug builds, and compiles to a NOP in release builds.</summary>
/// <param name="note">A note that will be prepended to the error log, if an error is detected.</param>
#define debugThrowOnApiError(note) ::pvr::utils::throwOnGlError(note)
#else
/// <summary>Checks for API errors if the API supports them. If an error is detected, logs relevant error
/// information. Only works in debug builds, and compiles to a NOP in release builds.</summary>
#define debugThrowOnApiError(dummy)
#endif
