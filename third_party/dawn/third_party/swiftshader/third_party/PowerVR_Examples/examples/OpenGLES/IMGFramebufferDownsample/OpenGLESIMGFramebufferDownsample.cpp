/*!
\brief Shows how to load POD files and play the animation with basic lighting
\file OpenGLESIMGFramebufferDownsample.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

// Main include file for the PVRShell library. Use this file when you will not be linking the PVRApi library.
#include "PVRShell/PVRShell.h"
// Main include file for the PVRAssets library.
#include "PVRAssets/PVRAssets.h"
// The OpenGL ES bindings used throughout this SDK. Use by calling the ES functions from the gl::namespace.
// The functions are loaded dynamically at first call and require no linking in the application.
// (So, glTextImage2D becomes gl::TexImage2D)
#include "PVRUtils/PVRUtilsGles.h"

// Index to bind the attributes to vertex shaders
const uint32_t VertexArray = 0;

// Shader files
const char FragShaderSrcFile[] = "FragShader.fsh";
const char VertShaderSrcFile[] = "VertShader.vsh";

const char HalfAndHalfFragShaderSrcFile[] = "HalfAndHalfFragShader.fsh";
const char HalfAndHalfVertShaderSrcFile[] = "HalfAndHalfVertShader.vsh";

const char BlitFragShaderSrcFile[] = "BlitFragShader.fsh";
const char BlitVertShaderSrcFile[] = "BlitVertShader.vsh";

/// <summary>implementing the pvr::Shell functions </summary>
class OpenGLESIMGFramebufferDownsample : public pvr::Shell
{
	pvr::EglContext _context;

	std::vector<glm::vec3> _vertices;
	GLuint _triVbo;

	GLuint _fullTex, _halfTex, _depthTexture;

	GLuint _downsampleFBO;

	// Group shader programs and their uniform locations together
	struct Program
	{
		GLuint handle;
		uint32_t uiMVPMatrixLoc;
		Program() : handle(0), uiMVPMatrixLoc(0) {}
	} _ShaderProgram, _BlitShaderProgram, _HalfAndHalfShaderProgram;

	// Variables to handle the animation in a time-based manner
	float _frame;
	glm::mat4 _projection;

	// UIRenderer class used to display text
	pvr::ui::UIRenderer _uiRenderer;

	bool _useFramebufferDownsample;
	bool _useFullDimensionFramebuffer;
	bool _useHalfAndHalf;

public:
	OpenGLESIMGFramebufferDownsample() : _triVbo(0), _fullTex(0), _halfTex(0), _depthTexture(0), _downsampleFBO(0) {}
	// pvr::Shell implementation.
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void loadShaders();
	void LoadVbos();
	void getDownScaleFactor(GLint& xDownscale, GLint& yDownscale);
	void updateMode(int mode);
	void updateSubtitleText();
	void eventMappedInput(pvr::SimplifiedInput e);
};

void OpenGLESIMGFramebufferDownsample::updateMode(int mode)
{
	if (mode == 0)
	{
		_useHalfAndHalf = true;
		_useFramebufferDownsample = false;
		_useFullDimensionFramebuffer = false;
	}
	else if (mode == 1)
	{
		_useFramebufferDownsample = true;
		_useHalfAndHalf = false;
		_useFullDimensionFramebuffer = false;
	}
	else
	{
		_useFullDimensionFramebuffer = true;
		_useFramebufferDownsample = false;
		_useHalfAndHalf = false;
	}
}

/// <summary>Handles user input and updates live variables accordingly.</summary>
void OpenGLESIMGFramebufferDownsample::eventMappedInput(pvr::SimplifiedInput e)
{
	static int mode = 0;
	// half and half, downsample only, full dimension
	switch (e)
	{
	case pvr::SimplifiedInput::Left:
		if (--mode < 0) { mode = 2; }
		updateMode(mode);
		updateSubtitleText();
		break;
	case pvr::SimplifiedInput::Right:
		++mode %= 3;
		updateMode(mode);
		updateSubtitleText();
		break;
	case pvr::SimplifiedInput::ActionClose: this->exitShell(); break;
	default: break;
	}
}

/// <summary>update the subtitle sprite.</summary>
void OpenGLESIMGFramebufferDownsample::updateSubtitleText()
{
	if (_useHalfAndHalf)
	{
		_uiRenderer.getDefaultDescription()->setText("Using GL_IMG_framebuffer_downsample.\nLeft: Samples full-res texture.\nRight: Samples half-res texture "
													 "(GL_IMG_framebuffer_downsample)");
	}
	else if (_useFullDimensionFramebuffer)
	{
		_uiRenderer.getDefaultDescription()->setText("Not using GL_IMG_framebuffer_downsample.\nSamples full-res texture.");
	}
	else
	{
		_uiRenderer.getDefaultDescription()->setText("Using GL_IMG_framebuffer_downsample.\nSamples half-res texture (GL_IMG_framebuffer_downsample)");
	}

	_uiRenderer.getDefaultDescription()->commitUpdates();
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it(e.g.external modules, loading meshes, etc.).
/// If the rendering context is lost, InitApplication() will not be called again.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESIMGFramebufferDownsample::initApplication()
{
	setBackBufferColorspace(pvr::ColorSpace::lRGB); // Example visuals are tweaked to directly use sRGB values to avoid shader gamma correction
	// Initialize variables used for the animation
	_frame = 0;
	updateMode(0);

	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change  in the rendering context. Used to initialize variables that are dependent on the
/// rendering context(e.g.textures, vertex buffers, etc.).</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESIMGFramebufferDownsample::initView()
{
	_context = pvr::createEglContext();
	_context->init(getWindow(), getDisplay(), getDisplayAttributes(), pvr::Api::OpenGLES2);

	if (!gl::isGlExtensionSupported("GL_OES_depth_texture")) { throw pvr::GlExtensionNotSupportedError("GL_OES_depth_texture"); }

	if (!gl::isGlExtensionSupported("GL_IMG_framebuffer_downsample")) { throw pvr::GlExtensionNotSupportedError("GL_IMG_framebuffer_downsample"); }

	_triVbo = 0;

	LoadVbos();
	loadShaders();

	_uiRenderer.init(getWidth(), getHeight(), isFullScreen(), getBackBufferColorspace() == pvr::ColorSpace::sRGB);
	_uiRenderer.getDefaultControls()->setText("Left / Right: Change render mode\n");
	_uiRenderer.getDefaultControls()->commitUpdates();
	_uiRenderer.getDefaultTitle()->setText("IMGFramebufferDownsample");
	_uiRenderer.getDefaultTitle()->commitUpdates();

	updateMode(0);
	updateSubtitleText();

	//	Set OpenGL ES render states needed for this training course
	// Enable backface culling and depth test
	gl::CullFace(GL_BACK);
	gl::Enable(GL_CULL_FACE);
	gl::Enable(GL_DEPTH_TEST);

	gl::Viewport(0, 0, this->getWidth(), this->getHeight());

	glm::vec3 clearColorLinearSpace(0.0f, 0.45f, 0.41f);
	glm::vec3 clearColor = clearColorLinearSpace;
	if (getBackBufferColorspace() != pvr::ColorSpace::sRGB)
	{
		// Gamma correct the clear colour
		clearColor = pvr::utils::convertLRGBtoSRGB(clearColorLinearSpace);
	}

	gl::ClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);

	_projection = glm::ortho(0.0, 1.0, 0.0, 1.0, 0.0, 10.0);

	GLint xDownscale, yDownscale;
	getDownScaleFactor(xDownscale, yDownscale);

	Log("Using GL_IMG_framebuffer_downsample");
	Log("Downsampling factor: %i, %i", xDownscale, yDownscale);

	// Create depth texture. Depth and stencil buffers must be full size
	gl::GenTextures(1, &_depthTexture);
	gl::BindTexture(GL_TEXTURE_2D, _depthTexture);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::TexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, this->getWidth(), this->getHeight(), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
	gl::BindTexture(GL_TEXTURE_2D, 0);

	gl::GenTextures(1, &_fullTex);
	gl::BindTexture(GL_TEXTURE_2D, _fullTex);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, this->getWidth(), this->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	gl::BindTexture(GL_TEXTURE_2D, 0);

	gl::GenTextures(1, &_halfTex);
	gl::BindTexture(GL_TEXTURE_2D, _halfTex);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, this->getWidth() / xDownscale, this->getHeight() / yDownscale, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	gl::BindTexture(GL_TEXTURE_2D, 0);

	gl::GenFramebuffers(1, &_downsampleFBO);
	gl::BindFramebuffer(GL_FRAMEBUFFER, _downsampleFBO);
	gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTexture, 0);
	gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _fullTex, 0);
	gl::ext::FramebufferTexture2DDownsampleIMG(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _halfTex, 0, xDownscale, yDownscale);
	assert(gl::CheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	GLenum drawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	gl::DrawBuffers(2, drawBuffers);

	gl::BindFramebuffer(GL_FRAMEBUFFER, _context->getOnScreenFbo());

	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESIMGFramebufferDownsample::renderFrame()
{
	// Set up the view and _projection matrices from the camera
	glm::mat4 mView;

	gl::Disable(GL_CULL_FACE);
	gl::Disable(GL_DEPTH_TEST);
	gl::Enable(GL_BLEND);

	// We can build the model view matrix from the camera position, target and an up vector.
	// For this we use glm::lookAt()
	mView = glm::lookAt(glm::vec3(0.0, 0.0, 3.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 vp = _projection * mView;

	gl::BindFramebuffer(GL_FRAMEBUFFER, _downsampleFBO);
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use the default shader program to render the triangle to the offscreen fbo
	gl::UseProgram(_ShaderProgram.handle);
	gl::UniformMatrix4fv(_ShaderProgram.uiMVPMatrixLoc, 1, GL_FALSE, glm::value_ptr(vp));

	gl::EnableVertexAttribArray(VertexArray);
	gl::BindBuffer(GL_ARRAY_BUFFER, _triVbo);
	gl::VertexAttribPointer(VertexArray, 3, GL_FLOAT, GL_FALSE, 0, 0);
	gl::DrawArrays(GL_TRIANGLES, 0, 3);

	gl::DisableVertexAttribArray(VertexArray);
	gl::BindBuffer(GL_ARRAY_BUFFER, 0);

	// Blit the result to screen
	gl::BindFramebuffer(GL_FRAMEBUFFER, _context->getOnScreenFbo());
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (_useFramebufferDownsample || _useFullDimensionFramebuffer)
	{
		// Use the blit shader program to simply blit from one of the framebuffer textures to the main framebuffer
		gl::UseProgram(_BlitShaderProgram.handle);

		if (_useFramebufferDownsample)
		{
			gl::ActiveTexture(GL_TEXTURE0);
			gl::BindTexture(GL_TEXTURE_2D, _halfTex);
		}
		else if (_useFullDimensionFramebuffer)
		{
			gl::ActiveTexture(GL_TEXTURE0);
			gl::BindTexture(GL_TEXTURE_2D, _fullTex);
		}
	}

	if (_useHalfAndHalf)
	{
		// Use the half and half blit shader program to blit the full dimension framebuffer to the left hand side of the final image and the half size image to the right hand side
		// of the final image
		gl::UseProgram(_HalfAndHalfShaderProgram.handle);

		// bind both the textures
		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_2D, _fullTex);

		gl::ActiveTexture(GL_TEXTURE1);
		gl::BindTexture(GL_TEXTURE_2D, _halfTex);
	}

	gl::Enable(GL_CULL_FACE);
	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);

	gl::Disable(GL_DEPTH_TEST);
	gl::DepthMask(GL_FALSE);

	gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);

	// UIRENDERER
	{
		// record the commands
		_uiRenderer.beginRendering();

		_uiRenderer.getSdkLogo()->render();
		_uiRenderer.getDefaultTitle()->render();
		_uiRenderer.getDefaultControls()->render();
		_uiRenderer.getDefaultDescription()->render();
		_uiRenderer.endRendering();
	}

	if (this->shouldTakeScreenshot()) { pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight()); }

	_context->swapBuffers();

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by Shell when the application quits.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESIMGFramebufferDownsample::releaseView()
{
	_uiRenderer.release();

	if (_triVbo) gl::DeleteBuffers(1, &_triVbo);

	if (_fullTex) gl::DeleteTextures(1, &_fullTex);
	if (_halfTex) gl::DeleteTextures(1, &_halfTex);
	if (_depthTexture) gl::DeleteTextures(1, &_depthTexture);

	if (_downsampleFBO) gl::DeleteFramebuffers(1, &_downsampleFBO);

	if (_ShaderProgram.handle) gl::DeleteProgram(_ShaderProgram.handle);
	if (_BlitShaderProgram.handle) gl::DeleteProgram(_BlitShaderProgram.handle);
	if (_HalfAndHalfShaderProgram.handle) gl::DeleteProgram(_HalfAndHalfShaderProgram.handle);

	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.</summary>
/// <returns>Result::Success if no error occurred</returns>.
pvr::Result OpenGLESIMGFramebufferDownsample::quitApplication() { return pvr::Result::Success; }

void OpenGLESIMGFramebufferDownsample::getDownScaleFactor(GLint& xDownscale, GLint& yDownscale)
{
	xDownscale = 1;
	yDownscale = 1;

	// Query the number of available scales
	GLint numScales;
	gl::GetIntegerv(GL_NUM_DOWNSAMPLE_SCALES_IMG, &numScales);

	// 2 scale modes are supported as minimum, so only need to check for
	// better than 2x2 if more modes are exposed.
	if (numScales > 2)
	{
		// Try to select most aggressive scaling.
		GLint bestScale = 1;
		GLint tempScale[2];
		GLint i;
		for (i = 0; i < numScales; ++i)
		{
			gl::GetIntegeri_v(GL_DOWNSAMPLE_SCALES_IMG, i, tempScale);

			// If the scaling is more aggressive, update our x/y scale values.
			if (tempScale[0] * tempScale[1] > bestScale)
			{
				xDownscale = tempScale[0];
				yDownscale = tempScale[1];
			}
		}
	}
	else
	{
		xDownscale = 2;
		yDownscale = 2;
	}
}

/// <summary>Loads the mesh data required for this training course into vertex buffer objects.</summary>
/// <returns>Return true if no error occurred.</return>
void OpenGLESIMGFramebufferDownsample::LoadVbos()
{
	{
		_vertices.clear();
		_vertices.reserve(3);
		_vertices.push_back(glm::vec3(0.1f, 0.1f, 0.0f));
		_vertices.push_back(glm::vec3(0.9f, 0.1f, 0.0f));
		_vertices.push_back(glm::vec3(0.5f, 0.9f, 0.0f));

		if (_triVbo)
		{
			gl::DeleteBuffers(1, &_triVbo);
			_triVbo = 0;
		}

		gl::GenBuffers(1, &_triVbo);
		gl::BindBuffer(GL_ARRAY_BUFFER, _triVbo);
		gl::BufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(glm::vec3), _vertices.data(), GL_STATIC_DRAW);
	}

	gl::BindBuffer(GL_ARRAY_BUFFER, 0);
}

/// <summary>Loads and compiles the shaders and links the shader programs required for this training course.</summary>
/// <returns>true if no error occurred.</return>
void OpenGLESIMGFramebufferDownsample::loadShaders()
{
	// Load and compile the shaders from files.
	const char* attributes[] = { "inVertex" };
	const uint16_t attributeIndices[] = { 0 };

	_ShaderProgram.handle = pvr::utils::createShaderProgram(*this, VertShaderSrcFile, FragShaderSrcFile, attributes, attributeIndices, 1);

	gl::UseProgram(_ShaderProgram.handle);
	// Store the location of uniforms for later use
	_ShaderProgram.uiMVPMatrixLoc = gl::GetUniformLocation(_ShaderProgram.handle, "MVPMatrix");

	_BlitShaderProgram.handle = pvr::utils::createShaderProgram(*this, BlitVertShaderSrcFile, BlitFragShaderSrcFile, attributes, attributeIndices, 1);

	// Set the sampler2D variable to the first texture unit
	gl::UseProgram(_BlitShaderProgram.handle);
	gl::Uniform1i(gl::GetUniformLocation(_BlitShaderProgram.handle, "tex"), 0);

	_HalfAndHalfShaderProgram.handle = pvr::utils::createShaderProgram(*this, HalfAndHalfVertShaderSrcFile, HalfAndHalfFragShaderSrcFile, NULL, NULL, 0);

	gl::UseProgram(_HalfAndHalfShaderProgram.handle);
	gl::Uniform1i(gl::GetUniformLocation(_HalfAndHalfShaderProgram.handle, "fullDimensionColor"), 0);
	gl::Uniform1i(gl::GetUniformLocation(_HalfAndHalfShaderProgram.handle, "halfDimensionColor"), 1);
	gl::Uniform1f(gl::GetUniformLocation(_HalfAndHalfShaderProgram.handle, "WindowWidth"), (GLfloat)this->getWidth());
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<OpenGLESIMGFramebufferDownsample>(); }
