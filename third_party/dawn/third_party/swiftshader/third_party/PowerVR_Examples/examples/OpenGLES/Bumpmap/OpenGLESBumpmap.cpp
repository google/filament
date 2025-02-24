/*!
\brief  Shows how to perform tangent space bump mapping.
\file OpenGLESBumpmap.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsGles.h"
#include <thread>
#include <mutex>

const float RotateY = glm::pi<float>() / 150;
const glm::vec4 LightDir(.24f, .685f, -.685f, 0.0f);

// shader uniforms
namespace Uniforms {
enum Enum
{
	MVPMatrix,
	LightDir,
	Count
};

const char* names[] = { "MVPMatrix", "LightDirModel" };

} // namespace Uniforms

// Content file names

// Shader Source
const char VertexShaderFile[] = "VertShader_ES3.vsh";
const char FragmentShaderFile[] = "FragShader_ES3.fsh";

// PVR texture files
const char TextureFileName[] = "Marble.pvr";
const char BumpTextureFileName[] = "MarbleNormalMap.pvr";

// POD scene files
const char SceneFileName[] = "Satyr.pod";

/// <summary>Class implementing the pvr::Shell functions.</summary>
class OpenGLESBumpmap : public pvr::Shell
{
	// UIRenderer class used to display text
	pvr::ui::UIRenderer uiRenderer;

	// 3D Model
	pvr::assets::ModelHandle _scene;

	// Projection and view matrix
	glm::mat4 _projMtx, _viewMtx;

	struct DrawPass
	{
		glm::mat4 mvp;
		glm::vec3 lightDir;
	};

	struct DeviceResources
	{
		pvr::EglContext context;

		// The Vertex buffer object handle array.
		std::vector<GLuint> vbos;
		std::vector<GLuint> ibos;
		GLuint program;
		GLuint texture;
		GLuint bumpTexture;
		GLuint onScreenFbo;
		// Samplers
		GLuint samplerTrilinear;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;

		DeviceResources()
		{
			program = 0;
			texture = 0;
			bumpTexture = 0;
			onScreenFbo = 0;
			samplerTrilinear = 0;
		};
		~DeviceResources()
		{
			if (!vbos.empty()) { gl::DeleteBuffers(vbos.size(), vbos.data()); }

			if (!ibos.empty()) { gl::DeleteBuffers(ibos.size(), ibos.data()); }

			if (program) { gl::DeleteProgram(program); }

			if (texture) { gl::DeleteTextures(1, &texture); }
			if (bumpTexture) { gl::DeleteTextures(1, &bumpTexture); }

			if (samplerTrilinear) { gl::DeleteSamplers(1, &samplerTrilinear); }
			if (onScreenFbo) { gl::DeleteFramebuffers(1, &onScreenFbo); }

			context.reset();
		}
	};

	glm::vec3 _clearColor;

	// The translation and Rotate parameter of Model
	float _angleY;
	glm::vec3 _lightdir;
	std::unique_ptr<DeviceResources> _deviceResources;

	pvr::utils::VertexConfiguration _vertexConfiguration;

	int32_t _uniformLocations[Uniforms::Count];

public:
	OpenGLESBumpmap() {}
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	// void eventMappedInput(pvr::SimplifiedInput key);

	// void executeGlCommands();
	void createProgram();
	void renderMesh(uint32_t nodeIndex);
};

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result OpenGLESBumpmap::initApplication()
{
	_scene = pvr::assets::loadModel(*this, SceneFileName);
	_angleY = 0.0f;

	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by Shell once per run, just before exiting the program.
///	If the rendering context is lost, QuitApplication() will not be called.</summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result OpenGLESBumpmap::quitApplication() { return pvr::Result::Success; }

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.</summary>
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result OpenGLESBumpmap::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();
	_deviceResources->context = pvr::createEglContext();
	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes());

	// create the default fbo using default params
	_deviceResources->onScreenFbo = _deviceResources->context->getOnScreenFbo();

	_deviceResources->texture = pvr::utils::textureUpload(*this, TextureFileName, _deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2);
	_deviceResources->bumpTexture = pvr::utils::textureUpload(*this, BumpTextureFileName, _deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2);
	pvr::utils::throwOnGlError("Texture creation failed");

	gl::GenSamplers(1, &_deviceResources->samplerTrilinear);
	gl::SamplerParameteri(_deviceResources->samplerTrilinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gl::SamplerParameteri(_deviceResources->samplerTrilinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_deviceResources->samplerTrilinear, GL_TEXTURE_WRAP_R, GL_REPEAT);
	gl::SamplerParameteri(_deviceResources->samplerTrilinear, GL_TEXTURE_WRAP_S, GL_REPEAT);
	gl::SamplerParameteri(_deviceResources->samplerTrilinear, GL_TEXTURE_WRAP_T, GL_REPEAT);
	pvr::utils::throwOnGlError("Sampler creation failed");

	// Load the vbo and ibo data
	pvr::utils::appendSingleBuffersFromModel(*_scene, _deviceResources->vbos, _deviceResources->ibos);

	_deviceResources->uiRenderer.init(
		getWidth(), getHeight(), isFullScreen(), (_deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2) || (getBackBufferColorspace() == pvr::ColorSpace::sRGB));
	_deviceResources->uiRenderer.getDefaultTitle()->setText("Bumpmap");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();

	createProgram();

	bool isRotated = this->isScreenRotated();
	if (!isRotated)
	{
		_projMtx = glm::perspective(_scene->getCamera(0).getFOV(), static_cast<float>(this->getWidth()) / static_cast<float>(this->getHeight()), _scene->getCamera(0).getNear(),
			_scene->getCamera(0).getFar());
	}
	else
	{
		_projMtx = pvr::math::perspective(pvr::Api::OpenGLES2, _scene->getCamera(0).getFOV(), static_cast<float>(this->getHeight()) / static_cast<float>(this->getWidth()),
			_scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(), glm::pi<float>() * .5f);
	}

	float fov;
	glm::vec3 cameraPos, cameraTarget, cameraUp;

	_scene->getCameraProperties(0, fov, cameraPos, cameraTarget, cameraUp);
	_viewMtx = glm::lookAt(cameraPos, cameraTarget, cameraUp);
	debugThrowOnApiError("InitView: Exit");

	return pvr::Result::Success;
}

void OpenGLESBumpmap::createProgram()
{
	static const char* attribs[] = { "inVertex", "inNormal", "inTexCoord" };
	static const uint16_t attribIndices[] = { 0, 1, 2 };

	// Enable or disable gamma correction based on if it is automatically performed on the framebuffer or we need to do it in the shader.
	const char* defines[] = { "FRAMEBUFFER_SRGB" };
	uint32_t numDefines = 1;

	glm::vec3 clearColorLinearSpace(0.0f, 0.45f, 0.41f);
	_clearColor = clearColorLinearSpace;
	if (getBackBufferColorspace() != pvr::ColorSpace::sRGB)
	{
		_clearColor = pvr::utils::convertLRGBtoSRGB(clearColorLinearSpace); // Gamma correct the clear colour...
		numDefines = 0;
	}

	GLuint program = _deviceResources->program = pvr::utils::createShaderProgram(*this, VertexShaderFile, FragmentShaderFile, attribs, attribIndices, 3, defines, numDefines);

	_uniformLocations[Uniforms::MVPMatrix] = gl::GetUniformLocation(_deviceResources->program, Uniforms::names[Uniforms::MVPMatrix]);
	_uniformLocations[Uniforms::LightDir] = gl::GetUniformLocation(_deviceResources->program, Uniforms::names[Uniforms::LightDir]);

	gl::UseProgram(_deviceResources->program);
	gl::Uniform1i(gl::GetUniformLocation(_deviceResources->program, "sBaseTex"), 0);
	gl::Uniform1i(gl::GetUniformLocation(_deviceResources->program, "sNormalMap"), 1);

	const pvr::utils::VertexBindings_Name vertexBindings[] = {
		{ "POSITION", "inVertex" },
		{ "NORMAL", "inNormal" },
		{ "UV0", "inTexCoord" },
		{ "TANGENT", "inTangent" },
	};

	_vertexConfiguration = pvr::utils::createInputAssemblyFromMesh(_scene->getMesh(0), vertexBindings, 4);
}

/// <summary>Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context. </summary>
/// <returns>Return pvr::Result::Success if no error occurred </returns>
pvr::Result OpenGLESBumpmap::releaseView()
{
	_deviceResources.reset();
	uiRenderer.release();
	_scene.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result OpenGLESBumpmap::renderFrame()
{
	debugThrowOnApiError("RenderFrame: Entrance");

	// Rotate and Translation the model matrix
	gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->onScreenFbo);
	gl::ClearColor(_clearColor.r, _clearColor.g, _clearColor.b, 1.f);
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl::Enable(GL_CULL_FACE);
	gl::UseProgram(_deviceResources->program);

	gl::StencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	gl::StencilFunc(GL_ALWAYS, 1, 255);
	gl::StencilMask(255);

	//--- create the pipeline layout
	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);
	gl::Enable(GL_DEPTH_TEST);

	gl::ActiveTexture(GL_TEXTURE0);
	gl::BindSampler(0, _deviceResources->samplerTrilinear);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->texture);

	gl::ActiveTexture(GL_TEXTURE1);
	gl::BindSampler(1, _deviceResources->samplerTrilinear);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->bumpTexture);

	glm::mat4 mModel = glm::rotate(_angleY, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(1.8f));
	_angleY += -RotateY * 0.05f * getFrameTime();

	gl::Uniform3fv(_uniformLocations[Uniforms::LightDir], 1, glm::value_ptr(LightDir * mModel));

	glm::mat4 mvp = (_projMtx * _viewMtx) * mModel * _scene->getWorldMatrix(_scene->getNode(0).getObjectId());
	gl::UniformMatrix4fv(_uniformLocations[Uniforms::MVPMatrix], 1, GL_FALSE, glm::value_ptr(mvp));

	// Now that the uniforms are set, call another function to actually draw the mesh.
	renderMesh(0);

	_deviceResources->uiRenderer.beginRendering();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.endRendering();

	if (this->shouldTakeScreenshot()) { pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight()); }

	_deviceResources->context->swapBuffers();

	return pvr::Result::Success;
}

/// <summary>Draws a pvr::assets::Mesh after the model view matrix has been set and the material prepared.</summary>
/// <param name="nodeIndex">Node index of the mesh to draw </param>
void OpenGLESBumpmap::renderMesh(uint32_t nodeIndex)
{
	const pvr::assets::Model::Node& node = _scene->getNode(nodeIndex);
	const pvr::assets::Mesh& mesh = _scene->getMesh(node.getObjectId());

	gl::BindBuffer(GL_ARRAY_BUFFER, _deviceResources->vbos[node.getObjectId()]);

	assertion(_vertexConfiguration.bindings.size() == 1, "This demo assumes only one VBO per mesh");

	for (auto it = _vertexConfiguration.attributes.begin(), end = _vertexConfiguration.attributes.end(); it != end; ++it)
	{
		gl::EnableVertexAttribArray(it->index);
		gl::VertexAttribPointer(it->index, it->width, pvr::utils::convertToGles(it->format), pvr::dataTypeIsNormalised(it->format),
			_vertexConfiguration.bindings[it->binding].strideInBytes, reinterpret_cast<const void*>(static_cast<uintptr_t>(it->offsetInBytes)));
	}

	GLenum indexType = mesh.getFaces().getDataType() == pvr::IndexType::IndexType32Bit ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;

	// The geometry can be exported in 4 ways:
	// - Indexed Triangle list
	// - Non-Indexed Triangle list
	// - Indexed Triangle strips
	// - Non-Indexed Triangle strips
	if (mesh.getNumStrips() == 0)
	{
		if (_deviceResources->ibos[node.getObjectId()])
		{
			// Indexed Triangle list
			gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _deviceResources->ibos[node.getObjectId()]);
			gl::DrawElements(GL_TRIANGLES, mesh.getNumFaces() * 3, indexType, 0);
		}
		else
		{
			// Non-Indexed Triangle list
			gl::DrawArrays(GL_TRIANGLES, 0, mesh.getNumFaces());
		}
	}
	else
	{
		for (int32_t i = 0; i < (int32_t)mesh.getNumStrips(); ++i)
		{
			int offset = 0;
			if (_deviceResources->ibos[node.getObjectId()])
			{
				// Indexed Triangle strips
				gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _deviceResources->ibos[node.getObjectId()]);
				gl::DrawElements(GL_TRIANGLE_STRIP, mesh.getStripLength(i) + 2, indexType, 0);
			}
			else
			{
				// Non-Indexed Triangle strips
				gl::DrawArrays(GL_TRIANGLE_STRIP, 0, mesh.getStripLength(i) + 2);
			}
			offset += mesh.getStripLength(i) + 2;
		}
	}
	for (auto it = _vertexConfiguration.attributes.begin(), end = _vertexConfiguration.attributes.end(); it != end; ++it) { gl::DisableVertexAttribArray(it->index); }
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<pvr::Shell>(new OpenGLESBumpmap()); }
