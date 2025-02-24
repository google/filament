/*!
\brief
\file OpenGLESOpenCLExample.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRAssets/PVRAssets.h"
#include "PVRUtils/PVRUtilsGles.h"
#include "PVRUtils/OpenCL/OpenCLUtils.h"

/// Content file names
namespace Files {
const char QuadVertShaderSrc[] = "QuadVertShader_ES3.vsh";
const char QuadFragShaderSrc[] = "QuadFragShader_ES3.fsh";
const char ImageTexture[] = "Lenna.pvr";
const char KernelSrc[] = "ConvolutionKernel.cl";
} // namespace Files
namespace Kernel {
enum Enum
{
	Box,
	Erode,
	Dilate,
	EdgeDetect,
	Sobel,
	Guassian,
	Emboss,
	Sharpen,
	Count,
	Copy = Count
};

const char* entry[Count + 1] = { "box_3x3", "erode_3x3", "dilate_3x3", "edgedetect_3x3", "sobel_3x3", "gaussian_3x3", "emboss_3x3", "sharpen_3x3", "copy" };

const char* names[Count + 1] = { "Box filter", "Erode", "Dilate", "Edge Detection", "Sobel", "Gaussian", "Emboss", "Sharpen", "Original" };
} // namespace Kernel
struct OpenCLObjects
{
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue commandqueue;
	cl_program program;
	cl_kernel kernels[Kernel::Count + 1];
};

/// <summary>Implements the pvr::Shell functions.</summary>
class OpenGLESOpenCLExample : public pvr::Shell
{
	struct DeviceResources
	{
		pvr::EglContext context;
		OpenCLObjects oclContext;
		// Programs
		GLuint progDefault;

		GLuint sharedImageGl;

		// The shared image
		EGLImage sharedImageEgl;

		cl_mem imageCl_Input;
		cl_mem imageCl_ClToGl;
		cl_mem imageCl_Backup;
		cl_sampler samplerCl;

		bool supportsEglImage;
		bool supportsEglClSharing;

		// Vbos/Ibos
		std::vector<GLuint> vbos;
		std::vector<GLuint> ibos;
		bool useEglClSharing() { return supportsEglImage && supportsEglClSharing; }

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;

		DeviceResources() : progDefault(0), sharedImageGl(0), sharedImageEgl(0), imageCl_Input(0), imageCl_ClToGl(0), imageCl_Backup(0), samplerCl(0) {}
		~DeviceResources()
		{
			if (vbos.size())
			{
				gl::DeleteBuffers(static_cast<GLsizei>(vbos.size()), vbos.data());
				vbos.clear();
			}
			if (ibos.size())
			{
				gl::DeleteBuffers(static_cast<GLsizei>(ibos.size()), ibos.data());
				ibos.clear();
			}

			if (progDefault)
			{
				gl::DeleteProgram(progDefault);
				progDefault = 0;
			}

			if (samplerCl) { cl::ReleaseSampler(samplerCl); }

			if (imageCl_Input) { cl::ReleaseMemObject(imageCl_Input); }
			if (imageCl_ClToGl) { cl::ReleaseMemObject(imageCl_ClToGl); }
			if (imageCl_Backup) { cl::ReleaseMemObject(imageCl_Backup); }

			if (sharedImageGl) { gl::DeleteTextures(1, &sharedImageGl); }
			if (sharedImageEgl) { egl::ext::DestroyImageKHR(egl::GetCurrentDisplay(), sharedImageEgl); }
		}
	};

	std::unique_ptr<DeviceResources> _deviceResources;

	pvr::utils::VertexConfiguration _vertexConfig;

	std::vector<uint8_t> _rawImageData;

	uint32_t _currentKernel;
	float _kernelTime = 0;
	float _modeTime = 0;
	bool _demoMode = true;
	bool _mode = false;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void createPipeline();
	void updateSubtitleText();
	void drawAxisAlignedQuad();
	void eventMappedInput(pvr::SimplifiedInput e);
	void createOpenCLObjects();
	void initClImages();
	void initKernels();
	pvr::Texture imageData;
	std::vector<unsigned char> imageTexels;
};

void OpenGLESOpenCLExample::createOpenCLObjects()
{
	imageData = pvr::textureLoad(*getAssetStream(Files::ImageTexture), pvr::TextureFileFormat::PVR);

	auto& clo = _deviceResources->oclContext;
	clutils::createOpenCLContext(clo.platform, clo.device, clo.context, clo.commandqueue, 0, CL_DEVICE_TYPE_GPU, 0, 0);

	auto kernelSrc = getAssetStream(Files::KernelSrc);

	clo.program = clutils::loadKernelProgram(clo.context, clo.device, *kernelSrc);

	imageTexels.resize(imageData.getWidth() * imageData.getHeight() * 4);

	gl::GenTextures(1, &_deviceResources->sharedImageGl);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->sharedImageGl);
	gl::TexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, imageData.getWidth(), imageData.getHeight());
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	gl::TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageData.getWidth(), imageData.getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, imageData.getDataPointer());

	_deviceResources->supportsEglImage = egl::isEglExtensionSupported("EGL_KHR_image");
	_deviceResources->supportsEglClSharing = clutils::isExtensionSupported(_deviceResources->oclContext.platform, "cl_khr_egl_image");
	if (_deviceResources->supportsEglImage && _deviceResources->supportsEglClSharing)
	{
		Log(LogLevel::Information, "Using EGL Image sharing with CL extension [EGL_KHR_image and cl_khr_egl_image].\n");
		_deviceResources->sharedImageEgl =
			egl::ext::CreateImageKHR(egl::GetCurrentDisplay(), egl::GetCurrentContext(), EGL_GL_TEXTURE_2D_KHR, (EGLClientBuffer)(size_t)_deviceResources->sharedImageGl, NULL);
		assertion(egl::GetError() == EGL_SUCCESS, "Failed to create KHR image");
		Log(LogLevel::Information, "Created EGL object %d as shared from %d", _deviceResources->sharedImageEgl);
	}
	else
	{
		if (!_deviceResources->supportsEglImage) { Log(LogLevel::Information, "EGL_KHR_image extension not supported\n"); }
		if (!_deviceResources->supportsEglClSharing) { Log(LogLevel::Information, "cl_khr_egl_image extension not supported\n"); }
		Log(LogLevel::Information, "Extensions necessary for Image sharing (interop) path not available. Using CPU fallback.\n");
	}

	initClImages();
}

void OpenGLESOpenCLExample::initKernels()
{
	auto& clobj = _deviceResources->oclContext;

	for (uint32_t i = 0; i < Kernel::Count + 1; ++i)
	{
		cl_int errcode = 0;

		// Create kernel based on function name
		clobj.kernels[i] = cl::CreateKernel(clobj.program, Kernel::entry[i], &errcode);

		if (clobj.kernels[i] == NULL || errcode != CL_SUCCESS)
		{
			throw pvr::InvalidOperationError(pvr::strings::createFormatted("Error: Failed to create kernel [%s] with code [%s]", Kernel::entry[i], clutils::getOpenCLError(errcode)));
		}

		// Set all kernel arguments
		errcode |= cl::SetKernelArg(clobj.kernels[i], 0, sizeof(cl_mem), &_deviceResources->imageCl_Input);
		errcode |= cl::SetKernelArg(clobj.kernels[i], 1, sizeof(cl_mem), &_deviceResources->imageCl_ClToGl);
		errcode |= cl::SetKernelArg(clobj.kernels[i], 2, sizeof(cl_sampler), &_deviceResources->samplerCl);

		if (errcode != CL_SUCCESS)
		{
			throw pvr::InvalidOperationError(
				pvr::strings::createFormatted("Error: Failed to set kernel arguments for kernel [%s] with error [%s]", Kernel::entry[i], clutils::getOpenCLError(errcode)));
		}
	}
}

void OpenGLESOpenCLExample::initClImages()
{
	cl_int errcode;
	cl_image_format format;

	cl_image_desc imageDescriptor = cl_image_desc();
	imageDescriptor.image_type = CL_MEM_OBJECT_IMAGE2D;
	imageDescriptor.image_height = imageData.getWidth();
	imageDescriptor.image_width = imageData.getWidth();

	if (imageData.getPixelFormat() != pvr::PixelFormat::RGBA_8888())
	{ throw pvr::InvalidDataError("Only RGBA8888 format supported for the input image of this application. Please replace InputImage.pvr with a compatible image."); }

	format.image_channel_order = CL_RGBA;
	format.image_channel_data_type = CL_UNORM_INT8;
	auto& clobj = _deviceResources->oclContext;

	_deviceResources->imageCl_Input = cl::CreateImage(clobj.context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE, &format, &imageDescriptor, NULL, &errcode);
	if (errcode != CL_SUCCESS || _deviceResources->imageCl_Input == NULL)
	{ throw pvr::InvalidOperationError(pvr::strings::createFormatted("Failed to create shared OpenCL image input with code %s", clutils::getOpenCLError(errcode))); }

	size_t origin[] = { 0, 0, 0 };
	size_t region[] = { imageData.getWidth(), imageData.getHeight(), 1 };
	size_t image_row_pitch = imageData.getWidth() * 4;
	char* mappedMemory =
		(char*)cl::EnqueueMapImage(clobj.commandqueue, _deviceResources->imageCl_Input, CL_TRUE, CL_MAP_WRITE, origin, region, &image_row_pitch, NULL, 0, NULL, NULL, &errcode);

	if (errcode != CL_SUCCESS || mappedMemory == NULL)
	{ throw pvr::InvalidOperationError(pvr::strings::createFormatted("ERROR: Failed to map buffer width code %s", clutils::getOpenCLError(errcode))); }

	memcpy(mappedMemory, imageData.getDataPointer(), imageData.getHeight() * imageData.getWidth() * 4);

	if (CL_SUCCESS != cl::EnqueueUnmapMemObject(clobj.commandqueue, _deviceResources->imageCl_Input, mappedMemory, 0, NULL, NULL))
	{ throw pvr::InvalidOperationError(pvr::strings::createFormatted("ERROR: Failed to unmap input image", clutils::getOpenCLError(errcode))); }

	if (_deviceResources->useEglClSharing())
	{
		clCreateFromEGLImageKHR_fn clCreateFromEGLImageKHR = (clCreateFromEGLImageKHR_fn)cl::GetExtensionFunctionAddressForPlatform(clobj.platform, "clCreateFromEGLImageKHR");
		_deviceResources->imageCl_ClToGl = clCreateFromEGLImageKHR(clobj.context, NULL, (CLeglImageKHR)_deviceResources->sharedImageEgl, CL_MEM_READ_WRITE, NULL, &errcode);
		Log(LogLevel::Information, "Created OpenCL image as shared from object %d", _deviceResources->sharedImageEgl);
	}
	else
	{
		_deviceResources->imageCl_ClToGl = cl::CreateImage(clobj.context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE, &format, &imageDescriptor, NULL, &errcode);
	}

	if (_deviceResources->imageCl_ClToGl == NULL || errcode != CL_SUCCESS)
	{ throw pvr::InvalidOperationError(pvr::strings::createFormatted("ERROR: Failed to create shared image object (output) with code %s", clutils::getOpenCLError(errcode))); }

	_deviceResources->imageCl_Backup = cl::CreateImage(clobj.context, CL_MEM_READ_WRITE, &format, &imageDescriptor, NULL, &errcode);
	if (_deviceResources->imageCl_Backup == NULL || errcode != CL_SUCCESS)
	{ throw pvr::InvalidOperationError(pvr::strings::createFormatted("ERROR: Failed to create shared image object (backup) with code %s", clutils::getOpenCLError(errcode))); }

	mappedMemory =
		(char*)cl::EnqueueMapImage(clobj.commandqueue, _deviceResources->imageCl_Backup, CL_TRUE, CL_MAP_WRITE, origin, region, &image_row_pitch, NULL, 0, NULL, NULL, &errcode);
	if (errcode != CL_SUCCESS || mappedMemory == NULL)
	{ throw pvr::InvalidOperationError(pvr::strings::createFormatted("ERROR: Failed to map image (backup) with code %s", clutils::getOpenCLError(errcode))); }

	memcpy(mappedMemory, imageData.getDataPointer(), imageData.getHeight() * imageData.getWidth() * 4);

	if (CL_SUCCESS != cl::EnqueueUnmapMemObject(clobj.commandqueue, _deviceResources->imageCl_Backup, mappedMemory, 0, NULL, NULL))
	{ throw pvr::InvalidOperationError(pvr::strings::createFormatted("ERROR: Failed to unmap backup image with code %s", clutils::getOpenCLError(errcode))); }

#if (CL_VERSION_2_0)
	cl_sampler_properties properties[] = { CL_SAMPLER_NORMALIZED_COORDS, CL_FALSE, CL_SAMPLER_ADDRESSING_MODE, CL_ADDRESS_CLAMP_TO_EDGE, CL_SAMPLER_FILTER_MODE, CL_FILTER_NEAREST, 0 };

	_deviceResources->samplerCl = cl::CreateSamplerWithProperties(clobj.context, properties, &errcode);
#else // Deprecated call
	_deviceResources->samplerCl = cl::CreateSampler(clobj.context, false, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_NEAREST, &errcode);
#endif

	if (_deviceResources->samplerCl == NULL || errcode != CL_SUCCESS)
	{ throw pvr::InvalidOperationError(pvr::strings::createFormatted("ERROR: Failed to create OpenCL sampler with code %s", clutils::getOpenCLError(errcode))); }

	cl::Finish(clobj.commandqueue);

	initKernels();
}

/// <summary>Handles user imageCl_Input and updates live variables accordingly.</summary>
void OpenGLESOpenCLExample::eventMappedInput(pvr::SimplifiedInput e)
{
	// Object+Bloom, object, bloom
	switch (e)
	{
	case pvr::SimplifiedInput::Left:
		if (_currentKernel == 0) { _currentKernel = Kernel::Count - 1; }
		else
		{
			--_currentKernel;
		}
		_kernelTime = 0;
		_modeTime = 0;
		_mode = true;
		updateSubtitleText();
		break;
	case pvr::SimplifiedInput::Right:
		if (++_currentKernel == Kernel::Count) { _currentKernel = 0; }
		_kernelTime = 0;
		_modeTime = 0;
		_mode = true;
		updateSubtitleText();
		break;
	case pvr::SimplifiedInput::Action1:
	case pvr::SimplifiedInput::Action2:
	case pvr::SimplifiedInput::Action3:
		_demoMode = !_demoMode;
		_kernelTime = 0;
		_modeTime = 0;
		_mode = true;
		break;
	case pvr::SimplifiedInput::ActionClose: this->exitShell(); break;
	default: break;
	}
}

/// <summary>Loads and compiles the shaders and links the shader programs.</summary>
/// <returns>Return true if no error occurred required for this training course</returns>
void OpenGLESOpenCLExample::createPipeline()
{
	// Enable or disable gamma correction based on if it is automatically performed on the framebuffer or we need to do it in the shader.
	const char* defines[] = { "FRAMEBUFFER_SRGB" };
	uint32_t numDefines = 1;
	if (getBackBufferColorspace() != pvr::ColorSpace::sRGB) { numDefines = 0; }
	_deviceResources->progDefault = pvr::utils::createShaderProgram(*this, Files::QuadVertShaderSrc, Files::QuadFragShaderSrc, NULL, NULL, 0, defines, numDefines);

	// Set the sampler2D variable to the first texture unit
	gl::UseProgram(_deviceResources->progDefault);
	GLint loc = gl::GetUniformLocation(_deviceResources->progDefault, "sTexture");
	gl::Uniform1i(loc, 0);
	gl::UseProgram(0);
}

/// <summary>Code in initApplication() will be called by pvr::Shell once per run, before the rendering
/// context is created. Used to initialize variables that are not dependent on it (e.g. external modules,
/// loading meshes, etc.) If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result OpenGLESOpenCLExample::initApplication()
{
	_currentKernel = 0;
	_kernelTime = 0;
	_modeTime = 0;
	_mode = true;
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.
/// quitApplication() will not be called every time the rendering context is lost, only before application exit.</summary>
/// <returns>Returns pvr::Result::Success if no error occurred.</returns>
pvr::Result OpenGLESOpenCLExample::quitApplication() { return pvr::Result::Success; }

/// <summary>Code in initView() will be called by pvr::Shell upon initialization or after a change
/// in the rendering context. Used to initialize variables that are dependent on the rendering
/// context (e.g. textures, vertex buffers, etc.).</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result OpenGLESOpenCLExample::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();
	_deviceResources->context = pvr::createEglContext();
	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes(), pvr::Api::OpenGLES3);

	std::vector<cl_platform_id> platforms;

	createOpenCLObjects();
	createPipeline();
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), getBackBufferColorspace() == pvr::ColorSpace::sRGB);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("OpenCLExample");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Left / right: Rendering mode\n");
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();
	updateSubtitleText();

	gl::Enable(GL_CULL_FACE);
	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);
	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by pvr::Shell when the application quits or before
/// a change in the rendering context.</summary>
/// <returns>Return pvr::Result::Success if no error occurred</returns>
pvr::Result OpenGLESOpenCLExample::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result OpenGLESOpenCLExample::renderFrame()
{
	debugThrowOnApiError("Frame begin");

	const float modeDuration = 1500.f;
	const float numfilterDisplays = 6.f;

	if (_demoMode)
	{
		_modeTime += getFrameTime();
		_kernelTime += getFrameTime();
		if (_modeTime > modeDuration)
		{
			_mode = !_mode;
			_modeTime = 0.f;
		}
		if (_kernelTime > modeDuration * numfilterDisplays)
		{
			if (++_currentKernel == Kernel::Count) { _currentKernel = 0; }
			_kernelTime = 0.f;
			updateSubtitleText();
		}
	}
	gl::Finish();

	cl_int errcode = 0;

	size_t global_size[] = { imageData.getWidth(), imageData.getHeight() };
	size_t local_size[] = { 8, 4 };
	size_t global_offset[] = { 0, 0 };

	auto& clobj = _deviceResources->oclContext;
	auto& queue = _deviceResources->oclContext.commandqueue;
	auto& kernel = clobj.kernels[_mode ? _currentKernel : Kernel::Copy];
	if (_deviceResources->useEglClSharing())
	{
		static clEnqueueAcquireEGLObjectsKHR_fn clEnqueueAcquireEGLObjectsKHR =
			(clEnqueueAcquireEGLObjectsKHR_fn)cl::GetExtensionFunctionAddressForPlatform(clobj.platform, "clEnqueueAcquireEGLObjectsKHR");
		errcode = clEnqueueAcquireEGLObjectsKHR(queue, 1, &_deviceResources->imageCl_ClToGl, 0, NULL, NULL);
		if (errcode != CL_SUCCESS)
		{ throw pvr::InvalidOperationError(pvr::strings::createFormatted("Failed to acquire EGL Objects with code %s", clutils::getOpenCLError(errcode))); }
	}
	// Use the original image as starting point for the first iteration
	if (cl::SetKernelArg(kernel, 0, sizeof(cl_mem), &_deviceResources->imageCl_Backup) != CL_SUCCESS)
	{ throw pvr::InvalidOperationError(pvr::strings::createFormatted("Failed to set kernel arg 0 with code %s", clutils::getOpenCLError(errcode))); }
	if (cl::SetKernelArg(kernel, 1, sizeof(cl_mem), &_deviceResources->imageCl_ClToGl) != CL_SUCCESS)
	{ throw pvr::InvalidOperationError(pvr::strings::createFormatted("Failed to set kernel arg 1 with code %s", clutils::getOpenCLError(errcode))); }

	// Launch kernel
	errcode = cl::EnqueueNDRangeKernel(queue, kernel, 2, global_offset, global_size, local_size, 0, NULL, NULL);
	if (errcode != CL_SUCCESS) { throw pvr::InvalidOperationError(pvr::strings::createFormatted("Failed to execute kernel with code %s", clutils::getOpenCLError(errcode))); }

	if (_deviceResources->useEglClSharing()) // Release the shared image from CL ownership, so we can render with it,...
	{
		static clEnqueueReleaseEGLObjectsKHR_fn clEnqueueReleaseEGLObjectsKHR =
			(clEnqueueReleaseEGLObjectsKHR_fn)cl::GetExtensionFunctionAddressForPlatform(clobj.platform, "clEnqueueReleaseEGLObjectsKHR");
		errcode = clEnqueueReleaseEGLObjectsKHR(queue, 1, &_deviceResources->imageCl_ClToGl, 0, NULL, NULL);
		if (errcode != CL_SUCCESS)
		{ throw pvr::InvalidOperationError(pvr::strings::createFormatted("Failed to release EGL Objects with code %s", clutils::getOpenCLError(errcode))); }
	}
	else // Otherwise, copy the data from the shared image...
	{
		const size_t origin[] = { 0, 0, 0 };
		const size_t region[] = { imageData.getWidth(), imageData.getHeight(), 1 };
		const size_t row_pitch = imageData.getWidth() * 4;
		errcode = cl::EnqueueReadImage(queue, _deviceResources->imageCl_ClToGl, CL_TRUE, origin, region, row_pitch, 0, imageTexels.data(), 0, NULL, NULL);
		if (errcode != CL_SUCCESS)
		{ throw pvr::InvalidOperationError(pvr::strings::createFormatted("Failed to Failed to enqueue read image with code %s", clutils::getOpenCLError(errcode))); }
	}
	cl::Finish(queue);

	gl::UseProgram(_deviceResources->progDefault);
	// Draw quad
	gl::ClearDepthf(1.0f);
	gl::Viewport(0, 0, getWidth(), getHeight());
	gl::Clear(GL_COLOR_BUFFER_BIT);
	gl::Disable(GL_DEPTH_TEST);

	// bind the  texture
	gl::Uniform1i(gl::GetUniformLocation(_deviceResources->progDefault, "sTexture"), 0);
	gl::ActiveTexture(GL_TEXTURE0);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->sharedImageGl);

	// Read back convolved data and feed back into texture if we're not using CL_KHR_egl_image.
	// If we ARE using CL_KHR_egl_image, there's no point - they're already in the shared image.
	if (!_deviceResources->useEglClSharing())
	{ gl::TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageData.getWidth(), imageData.getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, imageTexels.data()); }
	drawAxisAlignedQuad();

	// UIRENDERER
	{
		// record the commands
		_deviceResources->uiRenderer.beginRendering();

		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getDefaultControls()->render();
		_deviceResources->uiRenderer.getDefaultDescription()->render();
		_deviceResources->uiRenderer.endRendering();
	}
	debugThrowOnApiError("Frame end");

	if (this->shouldTakeScreenshot()) { pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight()); }

	_deviceResources->context->swapBuffers();
	return pvr::Result::Success;
}

/// <summary>Update the subtitle sprite.</summary>
void OpenGLESOpenCLExample::updateSubtitleText()
{
	_deviceResources->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("%s", Kernel::names[_currentKernel]));
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
}

/// <summary>Add the draw commands for a full screen quad to a commandbuffer.</summary>
void OpenGLESOpenCLExample::drawAxisAlignedQuad()
{
	gl::DisableVertexAttribArray(0);
	gl::DisableVertexAttribArray(1);
	gl::DisableVertexAttribArray(2);
	gl::BindBuffer(GL_ARRAY_BUFFER, 0);
	gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	gl::DrawArrays(GL_TRIANGLES, 0, 3);
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<OpenGLESOpenCLExample>(); }
