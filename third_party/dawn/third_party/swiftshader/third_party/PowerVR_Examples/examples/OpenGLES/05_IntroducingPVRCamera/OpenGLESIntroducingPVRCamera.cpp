/*!
\brief Demonstrates texture streaming using platform-specific functionality
\file OpenGLESIntroducingPVRCamera.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsGles.h"
#include "PVRCamera/PVRCamera.h"

namespace Configuration {
#if defined(__ANDROID__)
const char* ShaderDefines[] = { "ANDROID=1" };
int NumShaderDefines = 1;
#elif defined(TARGET_OS_IPHONE)
const char* ShaderDefines[] = { "IOS=1" };
int NumShaderDefines = 1;
#else
const char** ShaderDefines = NULL;
int NumShaderDefines = 0;
#endif
const char* VertexShaderFile = "VertShader.vsh";
const char* FragShaderFile = "FragShader.fsh";
} // namespace Configuration

/// <summary>Class implementing the pvr::Shell functions.</summary>
class OpenGLESIntroducingPVRCamera : public pvr::Shell
{
	pvr::EglContext _context;
	int32_t _uvTransformLocation;
	GLuint _program;

	// UIRenderer class used to display text
	pvr::ui::UIRenderer _uiRenderer;

	// Camera interface
	pvr::CameraInterface _camera;

public:
	OpenGLESIntroducingPVRCamera() : _program(0) {}
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();
};

const glm::vec2 VBOmem[] = {
	// POSITION,
	{ 1., -1. }, // 1:BR
	{ -1., -1. }, // 0:BL
	{ 1., 1. }, // 2:TR
	{ -1., 1. }, // 3:TL
};

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it(e.g.external modules, loading meshes, etc.).If the rendering
/// context is lost, initApplication() will not be called again.</summary>
/// <returns> Result::Success if no error occurred.</returns>
pvr::Result OpenGLESIntroducingPVRCamera::initApplication()
{
	setBackBufferColorspace(pvr::ColorSpace::lRGB); // Because the camera values are normally in the sRGB colorspace,
	// if we use an sRGB backbuffer, we would need to reverse gamma-correct the values before performing operations
	// on the values. We are not doing this here for simplicity, so we need to make sure that the framebuffer does not
	// gamma correct. Note that if we perform maths on the camera texture values, this is not strictly correct to do on
	// the sRGB colorspace and may have adverse effects on the hue.
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by Shell just before app termination. Most of the time no cleanup is necessary here as app will exit anyway. </summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESIntroducingPVRCamera::quitApplication() { return pvr::Result::Success; }

/// <summary>Code in initView() will be called by Shell upon initialization or after a change  in the rendering context. Used to initialize variables that are dependent on the
/// rendering context(e.g.textures, vertex buffers, etc.).</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESIntroducingPVRCamera::initView()
{
	_context = pvr::createEglContext();
	_context->init(getWindow(), getDisplay(), getDisplayAttributes());

	_camera.initializeSession(pvr::HWCamera::Front, getWidth(), getHeight());

	//  Load and compile the shaders & link programs
	{
		const char* attribNames[] = { "inVertex" };
		uint16_t attribIndices[] = { 0 };
		uint16_t numAttribs = 1;
		_program = pvr::utils::createShaderProgram(*this, Configuration::VertexShaderFile, Configuration::FragShaderFile, attribNames, attribIndices, numAttribs,
			Configuration::ShaderDefines, Configuration::NumShaderDefines);
		if (!_program) { return pvr::Result::UnknownError; }
		_uvTransformLocation = gl::GetUniformLocation(_program, "uvTransform");
	}
	_uiRenderer.init(getWidth(), getHeight(), isFullScreen(), getBackBufferColorspace() == pvr::ColorSpace::sRGB);
	_uiRenderer.getDefaultDescription()->setText("Streaming of hardware Camera video preview");
	_uiRenderer.getDefaultDescription()->commitUpdates();
	_uiRenderer.getDefaultTitle()->setText("IntroducingPVRCamera");
	_uiRenderer.getDefaultTitle()->commitUpdates();

	gl::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by Shell when the application quits.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESIntroducingPVRCamera::releaseView()
{
	// Clean up AV capture
	_camera.destroySession();

	// Release UIRenderer resources
	_uiRenderer.release();
	if (_program) gl::DeleteProgram(_program);
	_context.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESIntroducingPVRCamera::renderFrame()
{
	gl::Clear(GL_COLOR_BUFFER_BIT);
	_camera.updateImage();

	gl::BindBuffer(GL_ARRAY_BUFFER, 0);
	gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (_camera.isReady())
	{
#if !TARGET_OS_IPHONE
		if (_camera.hasRgbTexture())
		{
			gl::ActiveTexture(GL_TEXTURE0);
#if defined(__ANDROID__)
			gl::BindTexture(GL_TEXTURE_EXTERNAL_OES, _camera.getRgbTexture());
#else
			gl::BindTexture(GL_TEXTURE_2D, _camera.getRgbTexture());
#endif
		}
		else
#endif
		{
			gl::ActiveTexture(GL_TEXTURE0);
			gl::BindTexture(GL_TEXTURE_2D, _camera.getLuminanceTexture());
			gl::ActiveTexture(GL_TEXTURE1);
			gl::BindTexture(GL_TEXTURE_2D, _camera.getChrominanceTexture());
		}

		gl::UseProgram(_program);
		gl::EnableVertexAttribArray(0);
		gl::DisableVertexAttribArray(1);
		gl::DisableVertexAttribArray(2);

		static uint32_t pw = 0, ph = 0;
		uint32_t width, height;
		_camera.getCameraResolution(width, height);

		float aspectX = ((float)width * (float)getHeight()) / ((float)height * (float)getWidth());
		if (pw != width || ph != height)
		{
			Log(LogLevel::Debug, "Camera rendering with parameters:\n\tFramebuffer: %dx%d\tCamera %dx%d - ASPECT: %f", width, height, getWidth(), getHeight(), aspectX);
			pw = width;
			ph = height;
		}

		glm::mat4x4 tmp = _camera.getProjectionMatrix() * glm::translate(glm::vec3(0.5f, 0.5f, 0.5f)) * glm::scale(glm::vec3(1 / aspectX, 1.f, 1.f)) *
			glm::translate(glm::vec3(-0.5f, -0.5f, -0.5f));

		gl::UniformMatrix4fv(_uvTransformLocation, 1, GL_FALSE, glm::value_ptr(tmp));
		gl::VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, VBOmem);
		gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	else
	{
		Log(LogLevel::Debug, "Camera is NOT ready, skipping texture rendering.");
	}
	_uiRenderer.beginRendering();
	_uiRenderer.getDefaultTitle()->render();
	_uiRenderer.getDefaultDescription()->render();
	_uiRenderer.getSdkLogo()->render();
	_uiRenderer.endRendering();

	if (this->shouldTakeScreenshot()) { pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight()); }

	_context->swapBuffers();
	return pvr::Result::Success;
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<OpenGLESIntroducingPVRCamera>(); }
