/*!
\brief Shows how to perform skinning combined with Dot3 (normal-mapped) lighting
\file OpenGLESSkinning.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsGles.h"
#include "PVRUtils/OpenGLES/ModelGles.h"
#include <fstream>
#include <iomanip>

namespace Configuration {
const char EffectFile[] = "Skinning.pfx";

// POD scene files
const char SceneFile[] = "Robot.pod";

const char* DefaultVertShaderFile = "DefaultVertShader.vsh";
const char* DefaultFragShaderFile = "DefaultFragShader.fsh";
const char* SkinnedVertShaderFile = "SkinnedVertShader.vsh";
const char* SkinnedFragShaderFile = "SkinnedFragShader.fsh";

const char* DefaultAttributeNames[] = { "inVertex", "inNormal", "inTexCoord" };
const pvr::StringHash DefaultAttributeSemantics[] = { "POSITION", "NORMAL", "UV0" };
const uint16_t DefaultAttributeIndices[] = { 0, 1, 2 };
const char* SkinnedAttributeNames[] = { "inVertex", "inNormal", "inTangent", "inBiNormal", "inTexCoord", "inBoneWeights", "inBoneIndex" };
const pvr::StringHash SkinnedAttributeSemantics[] = { "POSITION", "NORMAL", "TANGENT", "BINORMAL", "UV0", "BONEWEIGHT", "BONEINDEX" };
const uint16_t SkinnedAttributeIndices[] = { 0, 1, 2, 3, 4, 5, 6 };

const char* DefaultUniformNames[] = { "ModelMatrix", "MVPMatrix", "ModelWorldIT3x3", "LightPos", "sTexture" };

const char* SkinnedUniformNames[] = { "ViewProjMatrix", "LightPos", "BoneCount", "BoneMatrixArray", "BoneMatrixArrayIT", "sTexture", "sNormalMap" };
} // namespace Configuration
enum class SkinnedUniforms : uint32_t
{
	ViewProjMatrix,
	LightPos,
	BoneCount,
	BoneMatrixArray,
	BoneMatrixArrayIT,
	TextureDiffuse,
	TextureNormal,
	Count
};
enum class DefaultUniforms : uint32_t
{
	ModelMatrix,
	MVPMatrix,
	ModelWorldIT3x3,
	LightPos,
	TextureDiffuse,
	Count
};

/// <summary>Class implementing the Shell functions.</summary>
class OpenGLESSkinning : public pvr::Shell
{
	// Put all API managed objects in a struct so that we can one-line free them...
	struct DeviceResources
	{
		pvr::EglContext context;
		pvr::utils::ModelGles cookedScene;
		GLuint programDefault;
		GLuint programSkinned;

		pvr::utils::StructuredBufferView ssboView;
		std::vector<GLuint> ssbos;
		pvr::utils::StructuredBufferView uboView;
		GLuint ubo;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;

		DeviceResources() : programDefault(0), programSkinned(0) {}
		~DeviceResources()
		{
			if (programDefault)
			{
				gl::DeleteProgram(programDefault);
				programDefault = 0;
			}
			if (programSkinned)
			{
				gl::DeleteProgram(programSkinned);
				programSkinned = 0;
			}
		}
	};

	int32_t _defaultUniformLocations[static_cast<uint32_t>(DefaultUniforms::Count)];
	int32_t _skinnedUniformLocations[static_cast<uint32_t>(SkinnedUniforms::Count)];

	std::unique_ptr<DeviceResources> _deviceResources;

	// 3D Model
	pvr::assets::ModelHandle _scene;
	glm::mat4x4 _projectionMatrix;

	uint32_t _lightPositionIdx;
	uint32_t _viewProjectionIdx;
	uint32_t _boneCountIdx;
	uint32_t _bonesIdx;
	uint32_t _boneMatrixIdx;
	uint32_t _boneMatrixItIdx;

	bool _isPaused;

	// Variables to handle the animation in a time-based manner
	float _currentFrame;

	glm::vec3 _clearColor;

public:
	OpenGLESSkinning() : _isPaused(false), _currentFrame(0) {}

	void renderNode(uint32_t nodeId, const glm::mat4& viewProjMatrix, bool& optimizer);

	pvr::Result initApplication();
	pvr::Result initView();
	pvr::Result releaseView();
	pvr::Result quitApplication();
	pvr::Result renderFrame();

	void setDefaultOpenglState();
	void eventMappedInput(pvr::SimplifiedInput action);
};

/// <summary>Handle the input event.</summary>
/// <param name="action">input actions to handle.</param>
void OpenGLESSkinning::eventMappedInput(pvr::SimplifiedInput action)
{
	switch (action)
	{
	case pvr::SimplifiedInput::Action1:
	case pvr::SimplifiedInput::Action2:
	case pvr::SimplifiedInput::Action3: _isPaused = !_isPaused; break;
	case pvr::SimplifiedInput::ActionClose: exitShell(); break;
	default: break;
	}
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result OpenGLESSkinning::initApplication()
{
	_scene = pvr::assets::loadModel(*this, Configuration::SceneFile);

	// The cameras are stored in the file. We check it contains at least one.
	if (_scene->getNumCameras() == 0)
	{
		setExitMessage("Error: The _scene does not contain a camera.");
		return pvr::Result::InitializationError;
	}

	// Check the _scene contains at least one light
	if (_scene->getNumLights() == 0)
	{
		setExitMessage("Error: The _scene does not contain a light.");
		return pvr::Result::InitializationError;
	}

	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by Shell once per run, just before exiting the program.
/// If the rendering context is lost, quitApplication() will not be called.</summary>
/// <returns> Return Result::Success if no error occurred.</returns>
pvr::Result OpenGLESSkinning::quitApplication()
{
	_scene.reset();
	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.).</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result OpenGLESSkinning::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();

	_currentFrame = 0.;
	_deviceResources->context = pvr::createEglContext();
	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes(), pvr::Api::OpenGLES31);

	glm::vec3 clearColorLinearSpace(0.0f, 0.45f, 0.41f);
	_clearColor = clearColorLinearSpace;
	if (getBackBufferColorspace() != pvr::ColorSpace::sRGB)
	{
		// Gamma correct the clear colour
		_clearColor = pvr::utils::convertLRGBtoSRGB(_clearColor);
	}

	GLint vertexShaderStorageBlocks = 0;
	gl::GetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &vertexShaderStorageBlocks);

	if (vertexShaderStorageBlocks < 1)
	{
		setExitMessage("Skinning requires support for at least 1 vertex shader storage block whereas the maximum supported by this device is: '%i'.", vertexShaderStorageBlocks);
		return pvr::Result::UnsupportedRequest;
	}

	{
		glm::vec3 from, to, up(0.0f, 1.0f, 0.0f);
		float fov, nearClip, farClip;
		_scene->getCameraProperties(0, fov, from, to, up, nearClip, farClip); // vTo is calculated from the rotation

		_projectionMatrix = pvr::math::perspective(pvr::Api::OpenGLES2, fov, static_cast<float>(getWidth()) / static_cast<float>(getHeight()), nearClip, farClip);
	}

	_deviceResources->cookedScene.init(*this, *_scene);

	const char* defines[] = { "FRAMEBUFFER_SRGB" };
	uint32_t numDefines = 1;
	if (getBackBufferColorspace() != pvr::ColorSpace::sRGB) { numDefines = 0; }

	// Load the shaders, create the programs
	_deviceResources->programDefault = pvr::utils::createShaderProgram(*this, Configuration::DefaultVertShaderFile, Configuration::DefaultFragShaderFile,
		Configuration::DefaultAttributeNames, Configuration::DefaultAttributeIndices, 3, defines, numDefines);
	_deviceResources->programSkinned = pvr::utils::createShaderProgram(*this, Configuration::SkinnedVertShaderFile, Configuration::SkinnedFragShaderFile,
		Configuration::SkinnedAttributeNames, Configuration::SkinnedAttributeIndices, 7, defines, numDefines);

	for (uint32_t i = 0; i < static_cast<uint32_t>(DefaultUniforms::Count); ++i)
	{ _defaultUniformLocations[i] = gl::GetUniformLocation(_deviceResources->programDefault, Configuration::DefaultUniformNames[i]); }
	for (uint32_t i = 0; i < static_cast<uint32_t>(SkinnedUniforms::Count); ++i)
	{ _skinnedUniformLocations[i] = gl::GetUniformLocation(_deviceResources->programSkinned, Configuration::SkinnedUniformNames[i]); }
	gl::UseProgram(_deviceResources->programDefault);
	gl::Uniform1i(_defaultUniformLocations[static_cast<uint32_t>(DefaultUniforms::TextureDiffuse)], 0);
	gl::UseProgram(_deviceResources->programSkinned);
	gl::Uniform1i(_skinnedUniformLocations[static_cast<uint32_t>(SkinnedUniforms::TextureDiffuse)], 0);
	gl::Uniform1i(_skinnedUniformLocations[static_cast<uint32_t>(SkinnedUniforms::TextureNormal)], 1);
	setDefaultOpenglState();

	// Create a buffer/buffers for the skinning data

	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), getBackBufferColorspace() == pvr::ColorSpace::sRGB);
	// clang-format off
	pvr::utils::StructuredMemoryDescription desc(
		"SSbo", 1, { 
			{ "Bones", 1, 
				{ 
					{ "BoneMatrix", pvr::GpuDatatypes::mat4x4 }, 
					{ "BoneMatrixIT", pvr::GpuDatatypes::mat3x3 } 
				} 
			} 
		});
	// clang-format on
	auto& ssboView = _deviceResources->ssboView;
	ssboView.init(desc);

	pvr::utils::StructuredMemoryDescription descUbo("Ubo", 1, { { "ViewProjMatrix", pvr::GpuDatatypes::mat4x4 }, { "LightPos", pvr::GpuDatatypes::vec3 } });

	_deviceResources->uboView.init(descUbo);

	gl::GenBuffers(1, &_deviceResources->ubo);
	gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->ubo);
	gl::BufferData(GL_UNIFORM_BUFFER, (GLsizeiptr)_deviceResources->uboView.getSize(), NULL, GL_DYNAMIC_DRAW);

	auto& ssbos = _deviceResources->ssbos;
	ssbos.resize(_scene->getNumMeshes());

	for (uint32_t meshId = 0, end = _scene->getNumMeshes(); meshId < end; ++meshId)
	{
		auto& mesh = _scene->getMesh(meshId);
		if (mesh.getMeshInfo().isSkinned)
		{
			auto& ssboMesh = ssbos[meshId];

			const pvr::assets::Skeleton& skeleton = _scene->getSkeleton(mesh.getSkeletonId());

			gl::GenBuffers(static_cast<GLsizei>(1), &ssboMesh);

			ssboView.setLastElementArraySize(static_cast<uint32_t>(skeleton.bones.size()));

			gl::BindBuffer(GL_SHADER_STORAGE_BUFFER, ssboMesh);
			gl::BufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(ssboView.getSize()), nullptr, GL_DYNAMIC_DRAW);
		}
		else
		{
			ssbos[meshId] = 0;
		}
	}
	_bonesIdx = _deviceResources->ssboView.getIndex("Bones");
	_boneCountIdx = _deviceResources->ssboView.getIndex("BoneCount");
	_boneMatrixIdx = _deviceResources->ssboView.getElement(_bonesIdx, 0).getIndex("BoneMatrix");
	_boneMatrixItIdx = _deviceResources->ssboView.getElement(_bonesIdx, 0).getIndex("BoneMatrixIT");
	_viewProjectionIdx = _deviceResources->uboView.getIndex("ViewProjMatrix");
	_lightPositionIdx = _deviceResources->uboView.getIndex("LightPos");

	_deviceResources->uiRenderer.getDefaultTitle()->setText("Skinning");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultDescription()->setText("Skinning with Normal Mapped Per Pixel Lighting");
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Any Action Key : Pause");
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	return pvr::Result::Success;
}

void OpenGLESSkinning::setDefaultOpenglState()
{
	gl::Enable(GL_DEPTH_TEST);
	gl::EnableVertexAttribArray(0);
	gl::EnableVertexAttribArray(1);
	gl::EnableVertexAttribArray(2);
	gl::Enable(GL_CULL_FACE);
	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);
}

/// <summary>Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result OpenGLESSkinning::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result OpenGLESSkinning::renderFrame()
{
	// Calculates the frame number to animate in a time-based manner.
	// Uses the shell function this->getTime() to get the time in milliseconds.
	float fDelta = static_cast<float>(getFrameTime());
	pvr::assets::AnimationInstance animInst = _scene->getAnimationInstance(0);
	if (_scene->getNumFrames() > 1)
	{
		if (fDelta > 0.0001f)
		{
			if (!_isPaused) { _currentFrame += fDelta; }
			if (_currentFrame > animInst.getTotalTimeInMs()) { _currentFrame = 0; }
		}
	}

	animInst.updateAnimation(_currentFrame);
	// Setting up the "view projection" matrix only once - it doesn't change with the object
	// Technically the camera projection stats COULD be animated, but we don't check for that
	// and we assume the camera projection parameters are static - hence we set it up just once,
	// in initApplication. So here we only need to get the world->view parameters (view matrix)
	// and concatenate this into a "view projection" matrix.
	glm::mat4x4 viewMatrix;
	glm::mat4x4 viewProjMatrix;
	{
		glm::vec3 from, to, up(0.0f, 1.0f, 0.0f);
		float fov, nearClip, farClip;
		_scene->getCameraProperties(0, fov, from, to, up, nearClip, farClip); // vTo is calculated from the rotation

		viewMatrix = glm::lookAt(from, to, up);
		viewProjMatrix = _projectionMatrix * viewMatrix;
	}

	gl::ClearColor(_clearColor.r, _clearColor.g, _clearColor.b, 1.f);
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setting up "as if" our last rendered node was not skinned
	gl::UseProgram(_deviceResources->programSkinned);
	gl::EnableVertexAttribArray(3);
	gl::EnableVertexAttribArray(4);
	gl::EnableVertexAttribArray(5);
	gl::EnableVertexAttribArray(6);

	static bool lastMeshRenderedWasSkinned;
	lastMeshRenderedWasSkinned = true;
	auto& uboView = _deviceResources->uboView;

	gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->ubo);
	gl::BindBufferBase(GL_UNIFORM_BUFFER, 0, _deviceResources->ubo);
	void* uboData = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, (GLsizeiptr)_deviceResources->uboView.getSize(), GL_MAP_WRITE_BIT);
	uboView.pointToMappedMemory(uboData);
	uboView.getElement(_viewProjectionIdx).setValue(viewProjMatrix);
	uboView.getElement(_lightPositionIdx).setValue(_scene->getLightPosition(0));
	gl::UnmapBuffer(GL_UNIFORM_BUFFER);

	// Get a new world view camera and light position
	// Update all node-specific matrices (World view, bone array etc).
	// Should be called before updating anything to optimise map/unmap. Suggest call once per frame.
	// For each mesh:
	// Bind the material of the mesh
	// Call the corresponding set state (skinned, non-skinned)
	// Bind the corresponding program
	// If skinned, bind the bones of the mesh
	// If skinned, update the bones
	// Update the matrices
	// Bind vertex buffer
	// Bind index buffer
	// Enable/disable vertex attributes
	for (uint32_t i = 0; i < _scene->getNumMeshNodes(); ++i) { renderNode(i, viewProjMatrix, lastMeshRenderedWasSkinned); }

	_deviceResources->uiRenderer.beginRendering();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.endRendering();

	if (this->shouldTakeScreenshot()) { pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight()); }

	_deviceResources->context->swapBuffers();

	return pvr::Result::Success;
}

void OpenGLESSkinning::renderNode(uint32_t nodeId, const glm::mat4& viewProjMatrix, bool& lastRenderWasSkinned)
{
	debugThrowOnApiError("OpenGLESSkinning::renderNode Enter");
	auto& node = _scene->getNode(nodeId);
	uint32_t meshId = node.getObjectId();
	auto& mesh = _scene->getMesh(meshId);
	uint32_t materialId = node.getMaterialIndex();
	auto& material = _scene->getMaterial(materialId);

	int32_t diffuseTexId = material.getTextureIndex("DIFFUSETEXTURE");
	int32_t bumpTexId = material.getTextureIndex("NORMALTEXTURE");

	GLuint diffuseTex = _deviceResources->cookedScene.getApiTextureById(diffuseTexId);
	GLuint vbo = _deviceResources->cookedScene.getVboByMeshId(meshId, 0);
	GLuint ibo = _deviceResources->cookedScene.getIboByMeshId(meshId);

	gl::BindTexture(GL_TEXTURE_2D, diffuseTex);
	gl::BindBuffer(GL_ARRAY_BUFFER, vbo);
	gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	if (mesh.getMeshInfo().isSkinned && bumpTexId != 0)
	{
		if (!lastRenderWasSkinned)
		{
			gl::EnableVertexAttribArray(3);
			gl::EnableVertexAttribArray(4);
			gl::EnableVertexAttribArray(5);
			gl::EnableVertexAttribArray(6);
			gl::UseProgram(_deviceResources->programSkinned);
			lastRenderWasSkinned = true;
		}
		GLuint normalTex = _deviceResources->cookedScene.getApiTextureById(bumpTexId);
		gl::ActiveTexture(GL_TEXTURE1);
		gl::BindTexture(GL_TEXTURE_2D, normalTex);
		gl::ActiveTexture(GL_TEXTURE0);

		for (uint32_t i = 0; i < sizeof(Configuration::SkinnedAttributeSemantics) / sizeof(Configuration::SkinnedAttributeSemantics[0]); ++i)
		{
			auto& attr = *mesh.getVertexAttributeByName(Configuration::SkinnedAttributeSemantics[i]);
			gl::VertexAttribPointer(i, attr.getN(), pvr::utils::convertToGles(attr.getVertexLayout().dataType), pvr::dataTypeIsNormalised(attr.getVertexLayout().dataType),
				mesh.getStride(0), reinterpret_cast<const void*>(static_cast<uintptr_t>(attr.getOffset())));
		}
		debugThrowOnApiError("OpenGLESSkinning::renderNode Skinned Setup");

		// Only bone batch 0 supported
		const pvr::assets::Skeleton& skeleton = _scene->getSkeleton(mesh.getSkeletonId());

		const uint32_t numBones = static_cast<uint32_t>(skeleton.bones.size());
		gl::BindBuffer(GL_SHADER_STORAGE_BUFFER, _deviceResources->ssbos[meshId]);
		gl::BindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _deviceResources->ssbos[meshId]);

		void* bones = gl::MapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, static_cast<GLsizeiptr>(_deviceResources->ssboView.getSize()), GL_MAP_WRITE_BIT);
		if (!bones) { debugThrowOnApiError("OpenGLESSkinning::renderNode Mapping"); }
		_deviceResources->ssboView.pointToMappedMemory(bones);
		auto root = _deviceResources->ssboView;
		for (uint32_t boneId = 0; boneId < numBones; ++boneId)
		{
			const auto& bone = _scene->getBoneWorldMatrix(nodeId, boneId);

			auto bonesArrayRoot = root.getElement(_bonesIdx, boneId);
			bonesArrayRoot.getElement(_boneMatrixIdx).setValue(bone);
			bonesArrayRoot.getElement(_boneMatrixItIdx).setValue(glm::mat3x4(glm::inverseTranspose(glm::mat3(bone))));
		}

		gl::UnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		debugThrowOnApiError("OpenGLESSkinning::renderNode Skinned Set uniforms");
		gl::DrawElements(GL_TRIANGLES, mesh.getNumFaces() * 3, pvr::utils::convertToGles(mesh.getFaces().getDataType()), reinterpret_cast<const void*>(0));
		debugThrowOnApiError("OpenGLESSkinning::renderNode Skinned Draw");
	}
	else
	{
		gl::UseProgram(_deviceResources->programDefault);
		if (lastRenderWasSkinned)
		{
			gl::EnableVertexAttribArray(0);
			gl::EnableVertexAttribArray(1);
			gl::EnableVertexAttribArray(2);
			gl::DisableVertexAttribArray(3);
			gl::DisableVertexAttribArray(4);
			gl::DisableVertexAttribArray(5);
			gl::DisableVertexAttribArray(6);
			lastRenderWasSkinned = false;
		}
		for (uint32_t i = 0; i < sizeof(Configuration::DefaultAttributeSemantics) / sizeof(Configuration::DefaultAttributeSemantics[0]); ++i)
		{
			auto& attr = *mesh.getVertexAttributeByName(Configuration::DefaultAttributeSemantics[i]);
			debug_assertion(attr.getDataIndex() == 0, "Only a single interleaved VBO supported for this demo");

			auto n = attr.getN();
			auto dt = pvr::utils::convertToGles(attr.getVertexLayout().dataType);
			auto norm = pvr::dataTypeIsNormalised(attr.getVertexLayout().dataType);
			auto str = mesh.getStride(0);
			auto off = reinterpret_cast<const void*>(static_cast<uintptr_t>(attr.getOffset()));
			gl::VertexAttribPointer(i, n, dt, norm, str, off);
		}
		debugThrowOnApiError("OpenGLESSkinning::renderNode Unskinned Setup");

		const auto& mw = _scene->getWorldMatrix(nodeId);
		const auto& mvp = viewProjMatrix * mw;
		const auto& mwit = glm::inverseTranspose(glm::mat3(mw));

		auto lp = _scene->getLightPosition(0);

		gl::Uniform3fv(_defaultUniformLocations[static_cast<uint32_t>(DefaultUniforms::LightPos)], 1, glm::value_ptr(lp));
		debugThrowOnApiError("OpenGLESSkinning::renderNode Unskinned Set uniforms 0");
		gl::UniformMatrix4x3fv(_defaultUniformLocations[static_cast<uint32_t>(DefaultUniforms::ModelMatrix)], 1, GL_FALSE, glm::value_ptr(glm::mat4x3(mw)));
		debugThrowOnApiError("OpenGLESSkinning::renderNode Unskinned Set uniforms 1");
		gl::UniformMatrix4fv(_defaultUniformLocations[static_cast<uint32_t>(DefaultUniforms::MVPMatrix)], 1, GL_FALSE, glm::value_ptr(mvp));
		debugThrowOnApiError("OpenGLESSkinning::renderNode Unskinned Set uniforms 2");
		gl::UniformMatrix3fv(_defaultUniformLocations[static_cast<uint32_t>(DefaultUniforms::ModelWorldIT3x3)], 1, GL_FALSE, glm::value_ptr(mwit));
		debugThrowOnApiError("OpenGLESSkinning::renderNode Unskinned Set uniforms 3");

		gl::DrawElements(GL_TRIANGLES, mesh.getNumFaces() * 3, pvr::utils::convertToGles(mesh.getFaces().getDataType()), 0);
		debugThrowOnApiError("OpenGLESSkinning::renderNode Unskinned Draw ");
	}
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<OpenGLESSkinning>(); }
