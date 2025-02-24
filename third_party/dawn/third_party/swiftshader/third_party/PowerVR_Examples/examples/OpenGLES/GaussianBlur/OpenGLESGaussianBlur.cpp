/*!
\brief  Shows how to perform a separated Gaussian Blur using a Compute shader and Fragment shader for carrying out the horizontal and vertical passes respectively.
\file OpenGLESGaussianBlur.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PVRShell/PVRShell.h"
#include "PVRAssets/PVRAssets.h"
#include "PVRUtils/PVRUtilsGles.h"

// Source and binary shaders
const char FragShaderSrcFile[] = "FragShader_ES3.fsh";
const char VertShaderSrcFile[] = "VertShader_ES3.vsh";
const char CompShaderSrcFile[] = "CompShader_ES3.csh";

// PVR texture files
const char StatueTexFile[] = "Lenna.pvr";

const char* attribNames[] = {
	"inPosition",
	"inTexCoord",
};

pvr::utils::VertexBindings_Name vertexBindings[] = {
	{ "POSITION", "inPosition" },
	{ "UV0", "inTexCoord" },
};

const uint16_t attribIndices[] = {
	0,
	1,
};

const uint32_t GaussianKernelSize = 19;

/// <summary>Prints the Gaussian weights and offsets provided in the vectors.</summary>
/// <param name="gaussianOffsets">The list of Gaussian offsets to print.</param>
/// <param name="gaussianWeights">The list of Gaussian weights to print.</param>
void printGaussianWeightsAndOffsets(std::vector<double>& gaussianOffsets, std::vector<double>& gaussianWeights)
{
	Log(LogLevel::Information, "Number of Gaussian Weights and Offsets = %u;", gaussianWeights.size());

	Log(LogLevel::Information, "Weights =");
	Log(LogLevel::Information, "{");
	for (uint32_t i = 0; i < gaussianWeights.size(); i++) { Log(LogLevel::Information, "%.15f,", gaussianWeights[i]); }
	Log(LogLevel::Information, "};");

	Log(LogLevel::Information, "Offsets =");
	Log(LogLevel::Information, "{");
	for (uint32_t i = 0; i < gaussianOffsets.size(); i++) { Log(LogLevel::Information, "%.15f,", gaussianOffsets[i]); }
	Log(LogLevel::Information, "};");
}

/// <summary>Class implementing the Shell functions.</summary>
class OpenGLESGaussianBlur : public pvr::Shell
{
private:
	struct Framebuffer
	{
		GLuint fbo;
		GLuint renderTex;
		pvr::Rectanglei renderArea;

		Framebuffer() : fbo(0), renderTex(0) {}

		~Framebuffer()
		{
			if (fbo)
			{
				gl::DeleteFramebuffers(1, &fbo);
				fbo = 0;
			}
		}
	};

	struct DeviceResources
	{
		pvr::EglContext context;
		// Fbo
		Framebuffer fbo;

		pvr::Texture texture;
		GLuint inputTexture;
		GLuint horizontallyBlurredTexture;

		GLuint samplerNearest;
		GLuint samplerBilinear;

		GLuint computeProgram;
		GLuint graphicsProgram;

		GLuint graphicsGaussianConfigBuffer;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;

		DeviceResources() : inputTexture(0), horizontallyBlurredTexture(0) {}
	};

	// Linear Optimised Gaussian offsets and weights
	std::vector<double> _linearGaussianOffsets;
	std::vector<double> _linearGaussianWeights;

	// Gaussian offsets and weights
	std::vector<double> _gaussianOffsets;
	std::vector<double> _gaussianWeights;

	uint32_t _graphicsUboSize;

	void* _mappedGraphicsBufferMemory;

	std::unique_ptr<DeviceResources> _deviceResources;

public:
	OpenGLESGaussianBlur() {}

	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void initialiseGaussianWeightsAndOffsets();
	void createResources();
	void render();
	void renderUI();
	void updateResources();
};

/// <summary>Code in  createResources() loads the compute, fragment and vertex shaders and associated buffers used by them. It loads the input texture on which
/// we'll perform the Gaussian blur. It also generates the output texture that will be filled by the compute shader and used by the fragment shader.</ summary>
void OpenGLESGaussianBlur::createResources()
{
	// Load the compute shader and create the associated program.
	_deviceResources->computeProgram = pvr::utils::createComputeShaderProgram(*this, CompShaderSrcFile);
	pvr::utils::throwOnGlError("Failed to create compute based horizontal Gaussian Blur program");

	// Create the buffer used in the vertical fragment pass
	{
		_graphicsUboSize = static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2));

		gl::GenBuffers(1, &_deviceResources->graphicsGaussianConfigBuffer);
		gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->graphicsGaussianConfigBuffer);
		gl::BufferData(GL_UNIFORM_BUFFER, static_cast<size_t>(_graphicsUboSize), nullptr, GL_DYNAMIC_DRAW);
	}

	// Enable or disable gamma correction based on if it is automatically performed on the framebuffer or we need to do it in the shader.
	const char* defines[] = { "FRAMEBUFFER_SRGB" };
	uint32_t numDefines = 1;
	if (getBackBufferColorspace() != pvr::ColorSpace::sRGB) { numDefines = 0; }

	// Load the fragment and vertex shaders and create the associated programs.
	_deviceResources->graphicsProgram = pvr::utils::createShaderProgram(*this, VertShaderSrcFile, FragShaderSrcFile, attribNames, attribIndices, 2, defines, numDefines);
	pvr::utils::throwOnGlError("Failed to create fragment based vertical Gaussian Blur program");

	// Load the texture from disk
	_deviceResources->inputTexture = pvr::utils::textureUpload(*this, "Lenna.pvr", _deviceResources->texture);

	// Create and Allocate Output texture.
	gl::GenTextures(1, &_deviceResources->horizontallyBlurredTexture);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->horizontallyBlurredTexture);
	gl::TexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, _deviceResources->texture.getDimensions().width, _deviceResources->texture.getDimensions().height);

	gl::GenSamplers(1, &_deviceResources->samplerBilinear);
	gl::SamplerParameteri(_deviceResources->samplerBilinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_deviceResources->samplerBilinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_deviceResources->samplerBilinear, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->samplerBilinear, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->samplerBilinear, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	gl::BindTexture(GL_TEXTURE_2D, 0);

	gl::GenSamplers(1, &_deviceResources->samplerNearest);
	gl::SamplerParameteri(_deviceResources->samplerNearest, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl::SamplerParameteri(_deviceResources->samplerNearest, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl::SamplerParameteri(_deviceResources->samplerNearest, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->samplerNearest, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->samplerNearest, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	pvr::utils::throwOnGlError("[OpenGLESGaussianBlur::createResources] Failed to create textures");
}

/// <summary>Performs the actual rendering each frame, first a compute shader is used to perform a horizontal compute based Gaussian blur, after this a
/// fragment shader based vertical Gaussian blur is used.</summary>
void OpenGLESGaussianBlur::render()
{
	// We Execute the Compute shader, we bind the input and output texture.
	gl::UseProgram(_deviceResources->computeProgram);
	gl::BindImageTexture(0, _deviceResources->inputTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
	gl::BindImageTexture(1, _deviceResources->horizontallyBlurredTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	gl::DispatchCompute(_deviceResources->texture.getDimensions().height / 32, 1, 1);

	// Use a memory barrier to ensure memory accesses using shader image load store
	gl::MemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

	// Execute the Graphic program (Vertex and Fragment) and pass the output texture
	gl::UseProgram(_deviceResources->graphicsProgram);
	gl::BindBufferBase(GL_UNIFORM_BUFFER, 0, _deviceResources->graphicsGaussianConfigBuffer);
	gl::ActiveTexture(GL_TEXTURE0);
	gl::BindSampler(0, _deviceResources->samplerNearest);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->inputTexture);
	gl::ActiveTexture(GL_TEXTURE1);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->horizontallyBlurredTexture);
	gl::BindSampler(1, _deviceResources->samplerBilinear);
	gl::Uniform1i(gl::GetUniformLocation(_deviceResources->graphicsProgram, "sOriginalTexture"), 0);
	gl::Uniform1i(gl::GetUniformLocation(_deviceResources->graphicsProgram, "sTexture"), 1);
	gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
}

/// <summary>Renders the UI.</summary>
void OpenGLESGaussianBlur::renderUI()
{
	_deviceResources->uiRenderer.beginRendering();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.endRendering();
}

/// <summary>Updates the buffer used by the graphics pass for controlling the Gaussian Blurs.</summary>
void OpenGLESGaussianBlur::updateResources()
{
	// Update the Gaussian configuration buffer used for the graphics based vertical pass
	{
		gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->graphicsGaussianConfigBuffer);
		_mappedGraphicsBufferMemory = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, static_cast<GLsizei>(_graphicsUboSize), GL_MAP_WRITE_BIT);

		uint32_t windowWidth = this->getWidth();
		float inverseImageHeight = 1.0f / _deviceResources->texture.getHeight();
		glm::vec2 config = glm::vec2(windowWidth, inverseImageHeight);

		memcpy(static_cast<char*>(_mappedGraphicsBufferMemory), &config, static_cast<size_t>(pvr::getSize(pvr::GpuDatatypes::vec2)));

		gl::UnmapBuffer(GL_UNIFORM_BUFFER);
	}
}

/// <summary>Initialises the Gaussian weights and offsets used in the compute shader and vertex/fragment shader carrying out the
/// horizontal and vertical Gaussian blur passes respectively.</summary>
void OpenGLESGaussianBlur::initialiseGaussianWeightsAndOffsets()
{
	// Generate a full set of Gaussian weights and offsets to be used in our compute shader
	{
		pvr::math::generateGaussianKernelWeightsAndOffsets(GaussianKernelSize, false, false, _gaussianWeights, _gaussianOffsets);

		Log(LogLevel::Information, "Gaussian Weights and Offsets:");
		printGaussianWeightsAndOffsets(_gaussianOffsets, _gaussianWeights);
	}

	// Generate a set of Gaussian weights and offsets optimised for linear sampling
	{
		pvr::math::generateGaussianKernelWeightsAndOffsets(GaussianKernelSize, false, true, _linearGaussianWeights, _linearGaussianOffsets);

		Log(LogLevel::Information, "Linear Sampling Optimized Gaussian Weights and Offsets:");
		printGaussianWeightsAndOffsets(_linearGaussianOffsets, _linearGaussianWeights);
	}
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.) If the rendering
/// context is lost, initApplication() will not be called again.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESGaussianBlur::initApplication()
{
	this->setDepthBitsPerPixel(0);
	this->setStencilBitsPerPixel(0);

	initialiseGaussianWeightsAndOffsets();

	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context(e.g.textures, vertex buffers, etc.)</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESGaussianBlur::initView()
{
	// initialize the device resources object
	_deviceResources = std::make_unique<DeviceResources>();

	// create an OpenGLES context
	_deviceResources->context = pvr::createEglContext();
	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes(), pvr::Api::OpenGLES31);

	// set up the application for rendering.
	createResources();

	// Set up the FBO to render to screen.
	_deviceResources->fbo.fbo = _deviceResources->context->getOnScreenFbo();
	_deviceResources->fbo.renderArea = pvr::Rectanglei(0, 0, getWidth(), getHeight());

	updateResources();

	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), getBackBufferColorspace() == pvr::ColorSpace::sRGB);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("GaussianBlur");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultDescription()->setText("Left: Original Texture\n"
																  "Right: Gaussian Blurred Texture");
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();

	gl::Disable(GL_DEPTH_TEST);
	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESGaussianBlur::releaseView()
{
	_deviceResources.reset();

	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by Shell once per run, just before exiting the program.
/// quitApplication() will not be called every time the rendering context is lost, only before application exit.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESGaussianBlur::quitApplication() { return pvr::Result::Success; }

/// <summary>Main rendering loop function of the program. The shell will call this function every frame</summary>
/// <returns>Result::Success if no error occurred.</summary>
pvr::Result OpenGLESGaussianBlur::renderFrame()
{
	debugThrowOnApiError("Frame begin");

	// Setup the Framebuffer for rendering
	gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _deviceResources->fbo.fbo);
	gl::Viewport(0, 0, _deviceResources->fbo.renderArea.width, _deviceResources->fbo.renderArea.height);
	gl::Clear(GL_COLOR_BUFFER_BIT);

	render();
	renderUI();
	debugThrowOnApiError("Frame end");

	if (this->shouldTakeScreenshot()) { pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight()); }

	_deviceResources->context->swapBuffers();
	return pvr::Result::Success;
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<OpenGLESGaussianBlur>(); }
