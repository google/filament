/*!
\brief Shows how to use the PVRApi library together with loading models from POD files and rendering them with effects from PFX files.
\file OpenGLESIntroducingPVRUtils.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PVRShell/PVRShell.h"
#include "PVRUtils/OpenGLES/BindingsGles.h"
#include "PVRUtils/PVRUtilsGles.h"

namespace Uniforms {
enum Enum
{
	WorldViewProjection,
	WorldViewIT,
	LightDirEye,
	AlbedoTexture,
	Count
};

const char* names[] = { "WVPMatrix", "WorldViewIT", "LightDirection", "sTexture" };
} // namespace Uniforms

// Content file names
const char VertexShaderFile[] = "VertShader.vsh"; // Effect file
const char FragmentShaderFile[] = "FragShader.fsh"; // Effect file

const char SceneFileName[] = "GnomeToy.pod"; // POD _scene files

/// <summary>Class implementing the Shell functions.</summary>
class OpenGLESIntroducingPVRUtils : public pvr::Shell
{
	struct DeviceResources
	{
		pvr::EglContext context;

		// The Vertex buffer object handle array.
		std::vector<GLuint> vbos;
		std::vector<GLuint> ibos;
		GLuint program;
		std::vector<GLuint> textures;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;

		DeviceResources() : program(0) {}
		~DeviceResources()
		{
			if (program) { gl::DeleteProgram(program); }
			program = 0;
			if (vbos.size())
			{
				gl::DeleteBuffers((GLsizei)vbos.size(), vbos.data());
				vbos.clear();
			}
			if (ibos.size())
			{
				gl::DeleteBuffers((GLsizei)ibos.size(), ibos.data());
				ibos.clear();
			}
			if (textures.size())
			{
				gl::DeleteTextures((GLsizei)textures.size(), textures.data());
				textures.clear();
			}
		}
	};

	std::unique_ptr<DeviceResources> _deviceResources;

	glm::vec3 _clearColor;

	// 3D Model
	pvr::assets::ModelHandle _scene;

	// Projection and Model View matrices
	glm::mat4 _projMtx, _viewMtx;

	// Variables to handle the animation in a time-based manner
	float _frame;

	pvr::utils::VertexConfiguration _vertexConfiguration;

	int32_t _uniformLocations[Uniforms::Count];

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void renderMesh(uint32_t meshNodeId);
	void setOpenglState();
};

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it(e.g.external modules, loading meshes, etc.).If the rendering
/// context is lost, initApplication() will not be called again.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESIntroducingPVRUtils::initApplication()
{
	// Load the scene
	_scene = pvr::assets::loadModel(*this, SceneFileName);

	// The cameras are stored in the file. We check it contains at least one.
	if (_scene->getNumCameras() == 0) { throw pvr::InvalidDataError("The scene does not contain a camera"); }
	// We check the _scene contains at least one light
	if (_scene->getNumLights() == 0) { throw pvr::InvalidDataError("The scene does not contain a light"); }

	// Ensure that all meshes use an indexed triangle list
	for (uint32_t i = 0; i < _scene->getNumMeshes(); ++i)
	{
		if (_scene->getMesh(i).getPrimitiveType() != pvr::PrimitiveTopology::TriangleList || _scene->getMesh(i).getFaces().getDataSize() == 0)
		{
			throw pvr::InvalidDataError(" The meshes in the scene must use an indexed triangle list");
			return pvr::Result::UnknownError;
		}
	}

	// Initialize variables used for the animation
	_frame = 0;

	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.</summary>
/// <returns>Result::Success if no error occurred</returns>.
pvr::Result OpenGLESIntroducingPVRUtils::quitApplication()
{
	_scene.reset();
	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change
/// in the rendering context. Used to initialize variables that are dependent on the
/// rendering context(e.g.textures, vertex buffers, etc.).</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESIntroducingPVRUtils::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();
	_deviceResources->context = pvr::createEglContext();

	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes());

	debugThrowOnApiError("InitView: Initialise Context");

	pvr::utils::appendSingleBuffersFromModel(*_scene, _deviceResources->vbos, _deviceResources->ibos);

	_deviceResources->uiRenderer.init(
		getWidth(), getHeight(), isFullScreen(), (_deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2) || (getBackBufferColorspace() == pvr::ColorSpace::sRGB));
	_deviceResources->uiRenderer.getDefaultTitle()->setText("IntroducingPVRUtils");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();

	debugThrowOnApiError("InitView: Initialise UIRenderer");

	static const char* attribs[] = { "inVertex", "inNormal", "inTexCoord" };
	static const uint16_t attribIndices[] = { 0, 1, 2 };

	// Enable or disable gamma correction based on if it is automatically performed on the framebuffer or we need to do it in the shader.
	const char* defines[] = { "FRAMEBUFFER_SRGB" };
	uint32_t numDefines = 1;
	glm::vec3 clearColorLinearSpace(0.0f, 0.45f, 0.41f);
	_clearColor = clearColorLinearSpace;
	if (getBackBufferColorspace() != pvr::ColorSpace::sRGB)
	{
		// Gamma correct the clear colour
		_clearColor = pvr::utils::convertLRGBtoSRGB(clearColorLinearSpace);
		// However, OpenGL ES2 should not be gamma corrected because the textures will unfortunately not be correctly read in linear values
		if (_deviceResources->context->getApiVersion() > pvr::Api::OpenGLES2) { numDefines = 0; }
	}

	GLuint program = _deviceResources->program = pvr::utils::createShaderProgram(*this, VertexShaderFile, FragmentShaderFile, attribs, attribIndices, 3, defines, numDefines);

	_uniformLocations[Uniforms::WorldViewProjection] = gl::GetUniformLocation(program, Uniforms::names[Uniforms::WorldViewProjection]);
	_uniformLocations[Uniforms::WorldViewIT] = gl::GetUniformLocation(program, Uniforms::names[Uniforms::WorldViewIT]);
	_uniformLocations[Uniforms::LightDirEye] = gl::GetUniformLocation(program, Uniforms::names[Uniforms::LightDirEye]);
	_uniformLocations[Uniforms::AlbedoTexture] = gl::GetUniformLocation(program, Uniforms::names[Uniforms::AlbedoTexture]);

	const pvr::assets::Mesh& mesh = _scene->getMesh(0);
	pvr::utils::VertexBindings_Name vertexBindings[] = { { "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoord" } };
	_vertexConfiguration = createInputAssemblyFromMesh(mesh, vertexBindings, 3);

	debugThrowOnApiError("InitView: Initialise program");

	uint32_t i = 0;

	_deviceResources->textures.resize(_scene->getNumMaterials());

	for (auto it = _deviceResources->textures.begin(), end = _deviceResources->textures.end(); it != end; ++it) { *it = 0; }

	while (i < _scene->getNumMaterials() && _scene->getMaterial(i).defaultSemantics().getDiffuseTextureIndex() != static_cast<uint32_t>(-1))
	{
		// create the texture object
		GLuint texture = 0;

		const pvr::assets::Model::Material& material = _scene->getMaterial(i);

		// Load the diffuse texture map
		texture = pvr::utils::textureUpload(
			*this, _scene->getTexture(material.defaultSemantics().getDiffuseTextureIndex()).getName().c_str(), _deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2);
		gl::BindTexture(GL_TEXTURE_2D, texture);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		_deviceResources->textures[i] = texture;
		++i;
	}

	// Calculates the projection matrix
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

void OpenGLESIntroducingPVRUtils::setOpenglState()
{
	gl::DepthMask(GL_TRUE);
	gl::ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	gl::CullFace(GL_BACK);
	gl::Enable(GL_CULL_FACE);
	gl::FrontFace(GL_CCW);
	gl::Enable(GL_DEPTH_TEST);
	gl::ClearColor(_clearColor.r, _clearColor.g, _clearColor.b, 1.f);
	gl::EnableVertexAttribArray(0);
	gl::EnableVertexAttribArray(1);
	gl::EnableVertexAttribArray(2);
}

/// <summary>Code in releaseView() will be called by Shell when the application quits.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESIntroducingPVRUtils::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLESIntroducingPVRUtils::renderFrame()
{
	debugThrowOnApiError("RenderFrame: Entrance");

	//  Calculates the _frame number to animate in a time-based manner.
	//  get the time in milliseconds.
	pvr::assets::AnimationInstance& animInst = _scene->getAnimationInstance(0);
	_frame += static_cast<float>(getFrameTime());

	if (_frame >= animInst.getTotalTimeInMs()) { _frame = 0; }

	// Sets the _scene animation to this _frame
	animInst.updateAnimation(_frame);

	setOpenglState();

	gl::UseProgram(_deviceResources->program);
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl::Uniform1i(_uniformLocations[Uniforms::AlbedoTexture], 0);

	for (uint32_t i = 0; i < _scene->getNumMeshNodes(); ++i) { renderMesh(i); }

	_deviceResources->uiRenderer.beginRendering();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.endRendering();

	if (this->shouldTakeScreenshot()) { pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight()); }

	_deviceResources->context->swapBuffers();
	debugThrowOnApiError("RenderFrame: Exit");
	return pvr::Result::Success;
}

void OpenGLESIntroducingPVRUtils::renderMesh(uint32_t meshNodeId)
{
	//  We can build the world view matrix from the camera position, target and an up vector.
	//  A _scene is composed of nodes. There are 3 types of nodes:
	//  - MeshNodes :
	//    references a mesh in the getMesh().
	//    These nodes are at the beginning of the Nodes array.
	//    And there are nNumMeshNode number of them.
	//    This way the .pod format can instantiate several times the same mesh
	//    with different attributes.
	//  - lights
	//  - cameras
	//  To draw a _scene, you must go through all the MeshNodes and draw the referenced meshes.
	glm::mat4 worldView;
	glm::vec3 lightDir3;

	_scene->getLightDirection(0, lightDir3);

	// A _scene is composed of nodes. There are 3 types of nodes:
	// - MeshNodes :
	// references a mesh in the getMesh().
	// These nodes are at the beginning of the Nodes array.
	// And there are nNumMeshNode number of them.
	// This way the .pod format can instantiate several times the same mesh
	// with different attributes.
	// - lights
	// - cameras
	// To draw a _scene, you must go through all the MeshNodes and draw the referenced meshes.

	// Gets the node model matrix
	worldView = _viewMtx * _scene->getWorldMatrix(meshNodeId);

	gl::ActiveTexture(GL_TEXTURE0);
	// Passes the world-view-projection matrix (WVP) to the shader to transform the vertices
	gl::UniformMatrix4fv(_uniformLocations[Uniforms::WorldViewProjection], 1, GL_FALSE, glm::value_ptr(_projMtx * worldView));
	// Passes the inverse-transpose of the world-view-projection matrix (WVP) to the shader to transform the normals
	gl::UniformMatrix4fv(_uniformLocations[Uniforms::WorldViewIT], 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(worldView)));
	// Passes the eye-space light direction to light the _scene
	gl::Uniform3fv(_uniformLocations[Uniforms::LightDirEye], 1, glm::value_ptr(glm::normalize(glm::mat3(_viewMtx) * lightDir3)));

	const pvr::assets::Model::Node* pNode = &_scene->getMeshNode(meshNodeId);
	// Gets pMesh referenced by the pNode
	const pvr::assets::Mesh* pMesh = &_scene->getMesh(pNode->getObjectId());
	int32_t matId = pNode->getMaterialIndex();

	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->textures[matId]);

	gl::BindBuffer(GL_ARRAY_BUFFER, _deviceResources->vbos[pNode->getObjectId()]);
	gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _deviceResources->ibos[pNode->getObjectId()]);

	for (uint32_t i = 0; i < 3; ++i)
	{
		auto& attrib = _vertexConfiguration.attributes[i];
		auto& binding = _vertexConfiguration.bindings[0];
		gl::VertexAttribPointer(attrib.index, attrib.width, pvr::utils::convertToGles(attrib.format), dataTypeIsNormalised(attrib.format), binding.strideInBytes,
			reinterpret_cast<const void*>(static_cast<uintptr_t>(attrib.offsetInBytes)));
	}

	gl::DrawElements(GL_TRIANGLES, pMesh->getNumFaces() * 3, pvr::utils::convertToGles(pMesh->getFaces().getDataType()), 0);
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<OpenGLESIntroducingPVRUtils>(); }
