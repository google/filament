/*!
\brief Implements a deferred shading technique supporting point and directional lights using pfx.
\file VulkanDeferredShadingPFX.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRVk/PVRVk.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRPfx/RenderManagerVk.h"
#include "PVRCore/pfx/PFXParser.h"

// Light mesh nodes
enum class LightNodes
{
	PointLightMeshNode = 0,
	NumberOfPointLightMeshNodes
};

// mesh nodes
enum class MeshNodes
{
	Satyr = 0,
	Floor = 1,
	NumberOfMeshNodes
};

static const pvr::StringHash PFXSemanticsStr[] = { "MODELVIEWPROJECTIONMATRIX", "MODELVIEWMATRIX", "MODELWORLDITMATRIX", "VIEWPOSITION", "PROXYMODELVIEWPROJECTIONMATRIX",
	"PROXYMODELVIEWMATRIX", "PROXYVIEWPOSITION", "LIGHTINTENSITY", "LIGHTRADIUS", "LIGHTCOLOR", "LIGHTSOURCECOLOR", "FARCLIPDIST" };

enum class PFXSemanticId
{
	MODELVIEWPROJECTIONMATRIX,
	MODELVIEWMATRIX,
	MODELWORLDITMATRIX,
	VIEWPOSITION,
	PROXYMODELVIEPROJECTIONMATRIX,
	PROXYMODELVIEWMATRIX,
	PROXYVIEWPOSITION,
	LIGHTINTENSITY,
	LIGHTRADIUS,
	LIGHTCOLOR,
	LIGHTSOURCECOLOR,
	FARCLIPDIST
};

/// <summary>Structures used for storing the shared point light data for the point light passes.</summary>
struct PointLightPasses
{
	struct PointLightProperties
	{
		glm::mat4 worldViewProjectionMatrix;
		glm::mat4 proxyWorldViewMatrix;
		glm::mat4 proxyWorldViewProjectionMatrix;
		glm::vec4 proxyViewSpaceLightPosition;
		glm::vec4 lightColor;
		glm::vec4 lightSourceColor;
		float lightIntensity;
		float lightRadius;
	};

	std::vector<PointLightProperties> lightProperties;

	struct InitialData
	{
		float radial_vel;
		float axial_vel;
		float vertical_vel;
		float angle;
		float distance;
		float height;
	};

	std::vector<InitialData> initialData;
};

/// <summary>structure used to render directional lighting.</summary>
struct DrawDirectionalLight
{
	struct DirectionalLightProperties
	{
		glm::vec4 lightIntensity;
		glm::vec4 viewSpaceLightDirection;
	};
	std::vector<DirectionalLightProperties> lightProperties;
};

/// <summary>structure used to fill the GBuffer.</summary>
struct DrawGBuffer
{
	struct Objects
	{
		pvr::FreeValue world;
		pvr::FreeValue worldView;
		pvr::FreeValue worldViewProj;
		pvr::FreeValue worldViewIT4x4;
	};
	std::vector<Objects> objects;
};

/// <summary>structure used to hold the rendering information for the demo.</summary>
struct RenderData
{
	DrawGBuffer storeLocalMemoryPass; // Subpass 0
	DrawDirectionalLight directionalLightPass; // Subpass 1
	PointLightPasses pointLightPasses; // holds point light data
};

/// <summary>Shader names for all of the demo passes.</summary>
namespace Files {
const char* const SceneFile = "SatyrAndTable.pod";
const char* const EffectPfx = "effect_MRT_PFX3.pfx";
const char* const PointLightModelFile = "pointlight.pod";
} // namespace Files

/// Application wide configuration data
namespace ApplicationConfiguration {
const float FrameRate = 1.0f / 120.0f;
}

/// Directional lighting configuration data
namespace DirectionalLightConfiguration {
static bool AdditionalDirectionalLight = true;
const float DirectionalLightIntensity = .1f;
const glm::vec4 AmbientLightColor = glm::vec4(.005f, .005f, .005f, 0.0f);

} // namespace DirectionalLightConfiguration

/// Point lighting configuration data
namespace PointLightConfiguration {
const float LightMaxDistance = 40.f;
const float LightMinDistance = 20.f;
const float LightMinHeight = -30.f;
const float LightMaxHeight = 40.f;
const float LightAxialVelocityChange = .01f;
const float LightRadialVelocityChange = .003f;
const float LightVerticalVelocityChange = .01f;
const float LightMaxAxialVelocity = 5.f;
const float LightMaxRadialVelocity = 1.5f;
const float LightMaxVerticalVelocity = 5.f;

static int32_t MaxScenePointLights = 5;
static int32_t NumProceduralPointLights = 10;
float PointlightIntensity = 20.f;
const float PointLightMinIntensityForCuttoff = 10.f / 255.f;
const float PointLightMaxRadius = 1.5f * glm::sqrt(PointLightConfiguration::PointlightIntensity / PointLightConfiguration::PointLightMinIntensityForCuttoff);
// The "Max radius" value we find is 50% more than the radius where we reach a specific light value.
// Light attenuation is quadratic: Light value = Intensity / Distance ^2
// The problem is that with this equation, light has infinite radius, as it asymptotically goes to zero as distance increases
// Very big radius is in general undesirable for deferred shading where you wish to have a lot of small lights, and where there
// contribution will be small to none, but a sharp cut-off is usually quite visible on dark scenes.
// For that reason, we have implemented an attenuation equation which begins close to the light following this value,
// but then after a predetermined value, switches to linear falloff and continues to zero following the same slope.
// This can be tweaked through this vale: It basically says "At which light intensity should the quadratic equation
// be switched to a linear one and trail to zero?".
// Following the numbers, if we follow the slope of 1/x^2 linearly, the value becomes exactly zero at 1.5 x distance.
// Good guide values here are around 5.f/255.f for a sharp falloff (but hence better performance as less pixels are shaded
// up to ~1.f/255.f for almost undetectably soft falloff in pitch-black scenes (hence more correct, but shading a lot
// of pixels that have a miniscule lighting contribution).
// Additionally, if there is a strong ambient or directional, this value can be increased (hence reducing the number of pixels
// shaded) as the ambient light will completely hide the small contributions of the edges of the point lights. Reversely,
// a completely dark scene would only be acceptable with values less than 2.f as otherwise the cut-off of the lights would be
// quite visible.
// NUMBERS: ( Symbols: Light Value: LV, Differential of LV: LV' Intensity: I, Distance: D, Distance of switch quadratic->linear:A)
// After doing some number-crunching, starting with LV = I / D^2
// LV = I * (3 * A^2 - 2 * D / A^3). See the PointLightPass2FragmentShader.
// Finally, crunching more numbers you will find that LV drops to zero when D = 1.5 * A, so we need to render the lights
// with a radius of 1.5 * A. In the shader, this is reversed to precisely find the point where we switch from quadratic to linear.
} // namespace PointLightConfiguration

// Subpasses used in the renderpass
enum class RenderPassSubpass
{
	GBuffer,
	Lighting,
	NumberOfSubpasses,
};

// Lighting Subpass's groups
enum class LightingSubpassGroup
{
	DirectionalLight,
	PointLightStep1, // Stencil
	PointLightStep2, // Proxy
	PointLightStep3, // Render Source
	Count
};

// Lighting Subpass groups pipelines
enum class LightingSubpassPipeline
{
	DirectionalLighting,

	// Point light passes
	PointLightStencil,
	PointLightProxy,
	PointLightSource,
	NumPipelines
};

struct DeviceResources
{
	pvrvk::Instance instance;
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
	pvrvk::Device device;
	pvrvk::Queue queue;
	pvr::utils::vma::Allocator vmaAllocator;
	pvrvk::Swapchain swapchain;

	pvrvk::CommandPool commandPool;
	pvrvk::DescriptorPool descriptorPool;

	pvrvk::Semaphore imageAcquiredSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore presentationSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameResourcesFences[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	//// Command Buffers ////
	// Main Primary Command Buffer
	pvr::Multi<pvrvk::CommandBuffer> cmdBufferMain;
	pvr::utils::RenderManager render_mgr;

	// UIRenderer used to display text
	pvr::ui::UIRenderer uiRenderer;
	~DeviceResources()
	{
		if (device)
		{
			device->waitIdle();
			uint32_t l = swapchain->getSwapchainLength();
			for (uint32_t i = 0; i < l; ++i)
			{
				if (perFrameResourcesFences[i]) perFrameResourcesFences[i]->wait();
			}
		}
	}
};

/// <summary>Class implementing the Shell functions.</summary>
class VulkanDeferredShadingPFX : public pvr::Shell
{
	struct Material
	{
		pvrvk::GraphicsPipeline materialPipeline;
		std::vector<pvrvk::DescriptorSet> materialDescriptorSet;
		float specularStrength;
		glm::vec3 diffuseColor;
	};

	//// Frame ////
	uint32_t _numSwapImages;
	uint32_t _swapchainIndex;
	uint32_t _frameId;
	// Putting all API objects into a pointer just makes it easier to release them all together with RAII
	std::unique_ptr<DeviceResources> _deviceResources;

	// Frame counters for animation
	float _frameNumber;
	bool _isPaused;
	uint32_t _cameraId;
	bool _animateCamera;

	uint32_t _numberOfPointLights;
	uint32_t _numberOfDirectionalLights;

	// Projection and Model View matrices
	glm::vec3 _cameraPosition;
	glm::mat4 _viewMatrix;
	glm::mat4 _projectionMatrix;
	glm::mat4 _viewProjectionMatrix;
	glm::mat4 _inverseViewMatrix;
	float _farClipDistance;

	uint32_t _windowWidth;
	uint32_t _windowHeight;
	int32_t _framebufferWidth;
	int32_t _framebufferHeight;

	int32_t _viewportOffsets[2];

	// Light models
	pvr::assets::ModelHandle _pointLightModel;

	// Object model
	pvr::assets::ModelHandle _mainScene;

	RenderData _renderInfo;

public:
	VulkanDeferredShadingPFX()
	{
		_animateCamera = false;
		_isPaused = false;
	}

	//	Overridden from Shell
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void recordCommandsPointLightGeometryStencil(pvrvk::CommandBuffer& cmdBuffers, uint32_t swapChainIndex, const uint32_t pointLight);
	void recordMainCommandBuffer();
	void allocateLights();
	void uploadStaticData();
	void uploadStaticSceneData();
	void uploadStaticDirectionalLightData();
	void uploadStaticPointLightData();
	void initialiseStaticLightProperties();
	void updateDynamicSceneData(uint32_t swapchain);
	void updateDynamicLightData(uint32_t swapchain);
	void updateAnimation();
	void updateProceduralPointLight(PointLightPasses::InitialData& data, PointLightPasses::PointLightProperties& pointLightProperties, uint32_t pointLightIndex);

	void updateGBufferPass()
	{
		auto& pipeline = _deviceResources->render_mgr.toPipeline(0, 0, static_cast<uint32_t>(RenderPassSubpass::GBuffer), 0, 0);
		pipeline.updateAutomaticModelSemantics(0);
		_deviceResources->render_mgr.toSubpassGroupModel(0, 0, static_cast<uint32_t>(RenderPassSubpass::GBuffer), 0, 0);
	}

	void eventMappedInput(pvr::SimplifiedInput key)
	{
		switch (key)
		{
		// Handle input
		case pvr::SimplifiedInput::ActionClose: exitShell(); break;
		case pvr::SimplifiedInput::Action1: _isPaused = !_isPaused; break;
		case pvr::SimplifiedInput::Action2: _animateCamera = !_animateCamera; break;
		default: break;
		}
	}
	pvr::assets::ModelHandle createFullScreenQuadMesh();
	void setProceduralPointLightInitialData(PointLightPasses::InitialData& data, PointLightPasses::PointLightProperties& pointLightProperties);

private:
};

/// <summary> Code in initApplication() will be called by pvr::Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns> Return true if no error occurred. </returns>
pvr::Result VulkanDeferredShadingPFX::initApplication()
{
	// This demo application makes heavy use of the stencil buffer
	setStencilBitsPerPixel(8);

	_frameNumber = 0.0f;
	_isPaused = false;
	_cameraId = 0;
	_frameId = 0;

	//  Load the scene and the light
	_mainScene = pvr::assets::loadModel(*this, Files::SceneFile);

	if (_mainScene->getNumCameras() == 0)
	{
		setExitMessage("ERROR: The main scene to display must contain a camera.\n");
		return pvr::Result::UnknownError;
	}

	//  Load light proxy geometry
	_pointLightModel = pvr::assets::loadModel(*this, Files::PointLightModelFile);

	return pvr::Result::Success;
}

pvr::assets::ModelHandle VulkanDeferredShadingPFX::createFullScreenQuadMesh()
{
	pvr::assets::ModelHandle model = std::make_shared<pvr::assets::Model>();
	model->allocMeshes(_numberOfDirectionalLights);
	model->allocMeshNodes(_numberOfDirectionalLights);
	// create a dummy material with a material attribute which will be identified by the pfx.
	model->addMaterial(pvr::assets::Material());
	model->getMaterial(0).setMaterialAttribute("DIR_LIGHT", pvr::FreeValue());
	for (uint32_t i = 0; i < _numberOfDirectionalLights; ++i)
	{
		model->getMesh(i).setPrimitiveType(pvr::PrimitiveTopology::TriangleStrip);
		model->getMesh(i).setNumVertices(3);
		model->connectMeshWithMeshNode(i, i);
		model->getMeshNode(i).setMaterialIndex(0);
	}
	return model;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.).</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanDeferredShadingPFX::initView()
{
	// Create the empty API objects.
	_deviceResources = std::make_unique<DeviceResources>();

	// Create instance and retrieve compatible physical devices
	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName());

	if (_deviceResources->instance->getNumPhysicalDevices() == 0)
	{
		setExitMessage("Unable not find a compatible Vulkan physical device.");
		return pvr::Result::UnknownError;
	}

	// Create the surface
	pvrvk::Surface surface =
		pvr::utils::createSurface(_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(0), this->getWindow(), this->getDisplay(), this->getConnection());

	// Create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance);

	pvr::utils::QueuePopulateInfo queueFlagsInfo[] = {
		{ pvrvk::QueueFlags::e_GRAPHICS_BIT, surface },
	};
	pvr::utils::QueueAccessInfo queueAccessInfo;

	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), queueFlagsInfo, ARRAY_SIZE(queueFlagsInfo), &queueAccessInfo);

	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{ swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT; } // Create the swapchain
	_deviceResources->swapchain = pvr::utils::createSwapchain(_deviceResources->device, surface, getDisplayAttributes(), swapchainImageUsage);

	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	// Get the number of swap images
	_numSwapImages = _deviceResources->swapchain->getSwapchainLength();

	// Get current swap index
	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	// calculate the frame buffer width and heights
	_framebufferWidth = _windowWidth = this->getWidth();
	_framebufferHeight = _windowHeight = this->getHeight();

	const pvr::CommandLine& commandOptions = getCommandLine();

	commandOptions.getIntOption("-fbowidth", _framebufferWidth);
	_framebufferWidth = glm::min<int32_t>(_framebufferWidth, _windowWidth);
	commandOptions.getIntOption("-fboheight", _framebufferHeight);
	_framebufferHeight = glm::min<int32_t>(_framebufferHeight, _windowHeight);
	commandOptions.getIntOption("-numlights", PointLightConfiguration::NumProceduralPointLights);
	commandOptions.getFloatOption("-lightintensity", PointLightConfiguration::PointlightIntensity);

	_viewportOffsets[0] = (_windowWidth - _framebufferWidth) / 2;
	_viewportOffsets[1] = (_windowHeight - _framebufferHeight) / 2;

	Log("Framebuffer dimensions: %d x %d\n", _framebufferWidth, _framebufferHeight);
	Log("On-screen Framebuffer dimensions: %d x %d\n", _windowWidth, _windowHeight);

	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	// create the command pool
	_deviceResources->commandPool =
		_deviceResources->device->createCommandPool(pvrvk::CommandPoolCreateInfo(queueAccessInfo.familyId, pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, 48)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 48)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 48)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_INPUT_ATTACHMENT, 48)
																						  .setMaxDescriptorSets(32));

	// Initialise lighting structures
	allocateLights();

	// Setup per swapchain Resources
	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		_deviceResources->cmdBufferMain[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);

		if (i == 0) { _deviceResources->cmdBufferMain[0]->begin(); }
		pvr::utils::setImageLayout(_deviceResources->swapchain->getImage(i), pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_PRESENT_SRC_KHR, _deviceResources->cmdBufferMain[0]);
	}

	const float_t aspectRatio = (float_t)_deviceResources->swapchain->getDimension().getWidth() / _deviceResources->swapchain->getDimension().getHeight();
	_projectionMatrix = pvr::math::perspective(pvr::Api::Vulkan, _mainScene->getCamera(0).getFOV(), aspectRatio, _mainScene->getCamera(0).getNear(), _mainScene->getCamera(0).getFar());

	// allocate number of point light mesh nodes which will uses the same material and the mesh
	_numberOfPointLights = PointLightConfiguration::NumProceduralPointLights;

	_pointLightModel->allocMeshNodes(_numberOfPointLights);
	_pointLightModel->connectMeshWithMeshNodes(0, 0, _numberOfPointLights - 1);
	_pointLightModel->addMaterial(pvr::assets::Material());
	_pointLightModel->getMaterial(0).setMaterialAttribute("POINT_LIGHT", pvr::FreeValue());
	_pointLightModel->assignMaterialToMeshNodes(0, 0, _numberOfPointLights - 1);

	//--- create the pfx effect
	pvr::Effect effect = pvr::pfx::readPFX(*getAssetStream(Files::EffectPfx), this);

	if (!_deviceResources->render_mgr.init(*this, _deviceResources->swapchain, _deviceResources->descriptorPool)) { return pvr::Result::UnknownError; }
	_deviceResources->render_mgr.addEffect(effect, _deviceResources->cmdBufferMain[0]);

	//--- Gbuffer renders the scene
	_deviceResources->render_mgr.addModelForAllSubpassGroups(_mainScene, 0, static_cast<uint32_t>(RenderPassSubpass::GBuffer), 0);

	//--- add the full screen quad mesh to the directional light subpass group in lighting subpass
	_deviceResources->render_mgr.addModelForSubpassGroup(
		createFullScreenQuadMesh(), 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::DirectionalLight));

	//--- add the point lights to the Pointlight subpass groups in lighting subpass
	_deviceResources->render_mgr.addModelForSubpassGroup(
		_pointLightModel, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1));

	_deviceResources->render_mgr.addModelForSubpassGroup(
		_pointLightModel, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep2));

	_deviceResources->render_mgr.addModelForSubpassGroup(
		_pointLightModel, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep3));

	// build all the renderman objects
	_deviceResources->render_mgr.buildRenderObjects(_deviceResources->cmdBufferMain[0]);

	_deviceResources->cmdBufferMain[0]->end();
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->cmdBufferMain[0];
	submitInfo.numCommandBuffers = 1;

	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle(); // wait for the commands to be flushed

	// initialize the UIRenderer and set the title text
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->render_mgr.toPass(0, 0).getFramebuffer(0)->getRenderPass(),
		static_cast<uint32_t>(RenderPassSubpass::Lighting), getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("DeferredShadingPFX").commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Action1: Pause\nAction2: Orbit Camera\n");
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	// initialise the gbuffer renderpass list
	_renderInfo.storeLocalMemoryPass.objects.resize(_mainScene->getNumMeshNodes());

	// calculate the frame buffer width and heights
	_framebufferWidth = _windowWidth = this->getWidth();
	_framebufferHeight = _windowHeight = this->getHeight();

	// Upload static data
	initialiseStaticLightProperties();
	uploadStaticData();

	for (uint32_t i = 0; i < _numSwapImages; ++i) { updateDynamicSceneData(i); }

	// Record the main command buffer
	recordMainCommandBuffer();
	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanDeferredShadingPFX::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
///	If the rendering context is lost, quitApplication() will not be called.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanDeferredShadingPFX::quitApplication()
{
	_mainScene.reset();
	_pointLightModel.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred</returns>
pvr::Result VulkanDeferredShadingPFX::renderFrame()
{
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameResourcesFences[_swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[_swapchainIndex]->reset();

	//  Handle user input and update object animations
	updateAnimation();

	_deviceResources->render_mgr.updateAutomaticSemantics(_swapchainIndex);

	// update the scene dynamic buffer
	updateDynamicSceneData(_swapchainIndex);

	// update dynamic light buffers
	updateDynamicLightData(_swapchainIndex);

	// submit the main command buffer
	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags pipeWaitStage = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.commandBuffers = &_deviceResources->cmdBufferMain[_swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->imageAcquiredSemaphores[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.waitDstStageMask = &pipeWaitStage;
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[_swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->queue, _deviceResources->commandPool, _deviceResources->swapchain, _swapchainIndex, this->getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	//--------------------
	// present
	pvrvk::PresentInfo presentInfo;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.imageIndices = &_swapchainIndex;
	_deviceResources->queue->present(presentInfo);

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	return pvr::Result::Success;
}

/// <summary>Upload the static data to the buffers which do not change per frame.</summary>
void VulkanDeferredShadingPFX::uploadStaticSceneData()
{
	// static scene properties buffer
	_farClipDistance = _mainScene->getCamera(0).getFar();

	pvr::FreeValue farClipDist;
	farClipDist.setValue(_farClipDistance);

	pvr::FreeValue specStrength;
	specStrength.setValue(0.0f);

	pvr::FreeValue diffColor;
	diffColor.setValue(glm::vec4(0.f));

	pvr::utils::RendermanSubpassGroupModel& model = _deviceResources->render_mgr.toSubpassGroupModel(0, 0, static_cast<uint32_t>(RenderPassSubpass::GBuffer), 0, 0);

	_deviceResources->render_mgr.toEffect(0).updateBufferEntryEffectSemantic("FARCLIPDIST", farClipDist, 0);

	for (uint32_t i = 0; i < model.getNumRendermanNodes(); ++i)
	{
		auto& node = model.toRendermanNode(i);

		pvr::assets::Material& material = _mainScene->getMaterial(_mainScene->getMeshNode(node.assetNodeId).getMaterialIndex());
		specStrength.setValue(material.defaultSemantics().getShininess());
		diffColor.setValue(glm::vec4(material.defaultSemantics().getDiffuse(), 1.f));
		node.updateNodeValueSemantic("SPECULARSTRENGTH", specStrength, 0);
		node.updateNodeValueSemantic("DIFFUSECOLOR", diffColor, 0);
	}
}

/// <summary>Upload the static data to the buffers which do not change per frame.</summary>
void VulkanDeferredShadingPFX::uploadStaticDirectionalLightData()
{
	pvr::FreeValue mem;
	for (uint32_t i = 0; i < _numberOfDirectionalLights; ++i)
	{
		mem.setValue(_renderInfo.directionalLightPass.lightProperties[i].lightIntensity);
		_deviceResources->render_mgr
			.toPipeline(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::DirectionalLight),
				static_cast<uint32_t>(LightingSubpassPipeline::DirectionalLighting))
			.updateBufferEntryNodeSemantic("LIGHTINTENSITY", mem, 0,
				_deviceResources->render_mgr
					.toSubpassGroupModel(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::DirectionalLight),
						static_cast<uint32_t>(LightingSubpassPipeline::DirectionalLighting))
					.toRendermanNode(i));

		mem.setValue(DirectionalLightConfiguration::AmbientLightColor);
		_deviceResources->render_mgr
			.toPipeline(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::DirectionalLight),
				static_cast<uint32_t>(LightingSubpassPipeline::DirectionalLighting))
			.updateBufferEntryNodeSemantic("AMBIENTLIGHT", mem, 0,
				_deviceResources->render_mgr
					.toSubpassGroupModel(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::DirectionalLight),
						static_cast<uint32_t>(LightingSubpassPipeline::DirectionalLighting))
					.toRendermanNode(i));
	}
}

/// <summary>Upload the static data to the buffers which do not change per frame.</summary>
void VulkanDeferredShadingPFX::uploadStaticPointLightData()
{
	// static point lighting buffer
	pvr::FreeValue values[4];
	for (uint32_t lightGroups = 0; lightGroups < 3; ++lightGroups)
	{
		for (uint32_t i = 0; i < _numberOfPointLights; ++i)
		{
			// LIGHTINTENSITY
			values[0].setValue(_renderInfo.pointLightPasses.lightProperties[i].lightIntensity);
			// LIGHTRADIUS
			values[1].setValue(_renderInfo.pointLightPasses.lightProperties[i].lightRadius);
			// LIGHTCOLOR
			values[2].setValue(_renderInfo.pointLightPasses.lightProperties[i].lightColor);
			// LIGHTSOURCECOLOR
			values[3].setValue(_renderInfo.pointLightPasses.lightProperties[i].lightSourceColor);

			// Point light data
			{
				pvr::utils::RendermanNode& node =
					_deviceResources->render_mgr
						.toSubpassGroupModel(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1) + lightGroups, 0)
						.toRendermanNode(i);
				_deviceResources->render_mgr
					.toPipeline(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1) + lightGroups,
						static_cast<uint32_t>(0))
					.updateBufferEntryNodeSemantic(PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::LIGHTINTENSITY)], values[0], 0, node);
			}
			{
				pvr::utils::RendermanNode& node =
					_deviceResources->render_mgr
						.toSubpassGroupModel(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1) + lightGroups, 0)
						.toRendermanNode(i);
				_deviceResources->render_mgr
					.toPipeline(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1) + lightGroups,
						static_cast<uint32_t>(0))
					.updateBufferEntryNodeSemantic(PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::LIGHTRADIUS)], values[1], 0, node);
			}

			{
				pvr::utils::RendermanNode& node =
					_deviceResources->render_mgr
						.toSubpassGroupModel(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1) + lightGroups, 0)
						.toRendermanNode(i);
				_deviceResources->render_mgr
					.toPipeline(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1) + lightGroups,
						static_cast<uint32_t>(0))
					.updateBufferEntryNodeSemantic(PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::LIGHTCOLOR)], values[2], 0, node);
			}

			{
				pvr::utils::RendermanNode& node =
					_deviceResources->render_mgr
						.toSubpassGroupModel(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1) + lightGroups, 0)
						.toRendermanNode(i);
				_deviceResources->render_mgr
					.toPipeline(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1) + lightGroups,
						static_cast<uint32_t>(0))
					.updateBufferEntryNodeSemantic(PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::LIGHTSOURCECOLOR)], values[3], 0, node);
			}
		}
	}
}

/// <summary>Upload the static data to the buffers which do not change per frame.</summary>
void VulkanDeferredShadingPFX::uploadStaticData()
{
	uploadStaticDirectionalLightData();
	uploadStaticSceneData();
	uploadStaticPointLightData();
}

/// <summary>Update the CPU visible buffers containing dynamic data.</summary>
void VulkanDeferredShadingPFX::updateDynamicSceneData(uint32_t swapchain)
{
	RenderData& pass = _renderInfo;

	// update the model matrices
	static glm::mat4 world, worldView;
	auto subpassGroupModels = _deviceResources->render_mgr.toSubpassGroupModel(0, 0, static_cast<uint32_t>(RenderPassSubpass::GBuffer), 0, 0).nodes;

	for (auto it = subpassGroupModels.begin(); it != subpassGroupModels.end(); ++it)
	{
		pvr::utils::RendermanNode& rendermanNode = *it;
		const pvr::assets::Model::Node& node = *rendermanNode.assetNode;
		world = _mainScene->getWorldMatrix(node.getObjectId());
		worldView = _viewMatrix * world;
		pass.storeLocalMemoryPass.objects[rendermanNode.assetNodeId].world.setValue(world);
		pass.storeLocalMemoryPass.objects[rendermanNode.assetNodeId].worldView.setValue(worldView);
		pass.storeLocalMemoryPass.objects[rendermanNode.assetNodeId].worldViewIT4x4.setValue(glm::inverseTranspose(worldView));
		pass.storeLocalMemoryPass.objects[rendermanNode.assetNodeId].worldViewProj.setValue(_projectionMatrix * worldView);

		pvr::utils::RendermanPipeline& pipe = rendermanNode.toRendermanPipeline();
		pipe.updateBufferEntryNodeSemantic(PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::MODELVIEWPROJECTIONMATRIX)],
			pass.storeLocalMemoryPass.objects[rendermanNode.assetNodeId].worldViewProj, swapchain, rendermanNode);

		pipe.updateBufferEntryNodeSemantic(PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::MODELVIEWMATRIX)],
			pass.storeLocalMemoryPass.objects[rendermanNode.assetNodeId].worldView, swapchain, rendermanNode);

		pipe.updateBufferEntryNodeSemantic(PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::MODELWORLDITMATRIX)],
			pass.storeLocalMemoryPass.objects[rendermanNode.assetNodeId].worldViewIT4x4, swapchain, rendermanNode);
	}
}

/// <summary>Update the CPU visible buffers containing dynamic data.</summary>
void VulkanDeferredShadingPFX::updateDynamicLightData(uint32_t swapchain)
{
	uint32_t pointLight = 0;
	uint32_t directionalLight = 0;
	RenderData& pass = _renderInfo;
	// update the lighting data
	for (uint32_t i = 0; i < _mainScene->getNumLightNodes(); ++i)
	{
		const pvr::assets::Node& lightNode = _mainScene->getLightNode(i);
		const pvr::assets::Light& light = _mainScene->getLight(lightNode.getObjectId());
		switch (light.getType())
		{
		case pvr::assets::Light::Point: {
			if (pointLight >= static_cast<uint32_t>(PointLightConfiguration::MaxScenePointLights)) { continue; }

			const glm::mat4& transMtx = _mainScene->getWorldMatrix(_mainScene->getNodeIdFromLightNodeId(i));
			const glm::mat4& proxyScale = glm::scale(glm::vec3(PointLightConfiguration::PointLightMaxRadius)) * PointLightConfiguration::PointlightIntensity;

			const glm::mat4 mWorldScale = transMtx * proxyScale;

			// POINT LIGHT GEOMETRY : The spheres that will be used for the stencil pass
			pass.pointLightPasses.lightProperties[pointLight].proxyWorldViewProjectionMatrix = _viewProjectionMatrix * mWorldScale;

			// POINT LIGHT PROXIES : The "drawcalls" that will perform the actual rendering
			pass.pointLightPasses.lightProperties[pointLight].proxyWorldViewMatrix = _viewMatrix * mWorldScale;
			// Translation component of the view matrix
			pass.pointLightPasses.lightProperties[pointLight].proxyViewSpaceLightPosition = glm::vec4((_viewMatrix * transMtx)[3]);

			// POINT LIGHT SOURCES : The little balls that we render to show the lights
			pass.pointLightPasses.lightProperties[pointLight].worldViewProjectionMatrix = _viewProjectionMatrix * transMtx;

			++pointLight;
		}
		break;
		case pvr::assets::Light::Directional: {
			const glm::mat4& transMtx = _mainScene->getWorldMatrix(_mainScene->getNodeIdFromLightNodeId(i));
			pass.directionalLightPass.lightProperties[directionalLight].viewSpaceLightDirection = _viewMatrix * transMtx * glm::vec4(0.f, -1.f, 0.f, 0.f);
			++directionalLight;
		}
		break;
		default: break;
		}
	}

	uint32_t numSceneLights = pointLight;
	if (DirectionalLightConfiguration::AdditionalDirectionalLight)
	{
		pass.directionalLightPass.lightProperties[directionalLight].viewSpaceLightDirection = _viewMatrix * glm::vec4(1.f, -1.f, -0.5f, 0.f);
		++directionalLight;
	}

	// update the directional light pipeline
	for (uint32_t i = 0; i < _numberOfDirectionalLights; ++i)
	{
		pvr::FreeValue viewDir;
		viewDir.setValue(pass.directionalLightPass.lightProperties[i].viewSpaceLightDirection);
		auto& pipeline = _deviceResources->render_mgr.toPipeline(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting),
			static_cast<uint32_t>(LightingSubpassGroup::DirectionalLight), static_cast<uint32_t>(LightingSubpassPipeline::DirectionalLighting));

		pvr::utils::RendermanNode& node =
			_deviceResources->render_mgr
				.toSubpassGroupModel(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::DirectionalLight), 0)
				.toRendermanNode(i);
		pipeline.updateBufferEntryNodeSemantic("VIEWDIRECTION", viewDir, swapchain, node);
	}

	// update the procedural point lights
	for (; pointLight < numSceneLights + _numberOfPointLights; ++pointLight)
	{ updateProceduralPointLight(pass.pointLightPasses.initialData[pointLight], _renderInfo.pointLightPasses.lightProperties[pointLight], pointLight); }
}

void VulkanDeferredShadingPFX::setProceduralPointLightInitialData(PointLightPasses::InitialData& data, PointLightPasses::PointLightProperties& pointLightProperties)
{
	data.distance = pvr::randomrange(PointLightConfiguration::LightMinDistance, PointLightConfiguration::LightMaxDistance);
	data.angle = pvr::randomrange(-glm::pi<float>(), glm::pi<float>());
	data.height = pvr::randomrange(PointLightConfiguration::LightMinHeight, PointLightConfiguration::LightMaxHeight);
	data.axial_vel = pvr::randomrange(-PointLightConfiguration::LightMaxAxialVelocity, PointLightConfiguration::LightMaxAxialVelocity);
	data.radial_vel = pvr::randomrange(-PointLightConfiguration::LightMaxRadialVelocity, PointLightConfiguration::LightMaxRadialVelocity);
	data.vertical_vel = pvr::randomrange(-PointLightConfiguration::LightMaxVerticalVelocity, PointLightConfiguration::LightMaxVerticalVelocity);

	glm::vec3 lightColor = glm::vec3(pvr::randomrange(0, 1), pvr::randomrange(0, 1), pvr::randomrange(0, 1));

	pointLightProperties.lightColor = glm::vec4(lightColor, 1.); // random-looking
	pointLightProperties.lightSourceColor = glm::vec4(lightColor, .8); // random-looking
	pointLightProperties.lightIntensity = PointLightConfiguration::PointlightIntensity;
	pointLightProperties.lightRadius = PointLightConfiguration::PointLightMaxRadius;
}

/// <summary>Update the procedural point lights.</summary>
void VulkanDeferredShadingPFX::updateProceduralPointLight(PointLightPasses::InitialData& data, PointLightPasses::PointLightProperties& pointLightProperties, uint32_t pointLightIndex)
{
	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();
	if (!_isPaused) // Skip for the first m_frameNumber, as sometimes this moves the light too far...
	{
		float_t dt = static_cast<float>(std::min(getFrameTime(), uint64_t(30)));
		if (data.distance < PointLightConfiguration::LightMinDistance)
		{ data.axial_vel = glm::abs(data.axial_vel) + (PointLightConfiguration::LightMaxAxialVelocity * dt * .001f); }
		if (data.distance > PointLightConfiguration::LightMaxDistance)
		{ data.axial_vel = -glm::abs(data.axial_vel) - (PointLightConfiguration::LightMaxAxialVelocity * dt * .001f); }
		if (data.height < PointLightConfiguration::LightMinHeight)
		{ data.vertical_vel = glm::abs(data.vertical_vel) + (PointLightConfiguration::LightMaxAxialVelocity * dt * .001f); }
		if (data.height > PointLightConfiguration::LightMaxHeight)
		{ data.vertical_vel = -glm::abs(data.vertical_vel) - (PointLightConfiguration::LightMaxAxialVelocity * dt * .001f); }

		data.axial_vel += pvr::randomrange(-PointLightConfiguration::LightAxialVelocityChange, PointLightConfiguration::LightAxialVelocityChange) * dt;

		data.radial_vel += pvr::randomrange(-PointLightConfiguration::LightRadialVelocityChange, PointLightConfiguration::LightRadialVelocityChange) * dt;

		data.vertical_vel += pvr::randomrange(-PointLightConfiguration::LightVerticalVelocityChange, PointLightConfiguration::LightVerticalVelocityChange) * dt;

		if (glm::abs(data.axial_vel) > PointLightConfiguration::LightMaxAxialVelocity) { data.axial_vel *= .8f; }
		if (glm::abs(data.radial_vel) > PointLightConfiguration::LightMaxRadialVelocity) { data.radial_vel *= .8f; }
		if (glm::abs(data.vertical_vel) > PointLightConfiguration::LightMaxVerticalVelocity) { data.vertical_vel *= .8f; }

		data.distance += data.axial_vel * dt * 0.001f;
		data.angle += data.radial_vel * dt * 0.001f;
		data.height += data.vertical_vel * dt * 0.001f;
	}

	const float x = sin(data.angle) * data.distance;
	const float z = cos(data.angle) * data.distance;
	const float y = data.height;
	const glm::mat4& transMtx = glm::translate(glm::vec3(x, y, z));
	const glm::mat4& proxyScale = glm::scale(glm::vec3(PointLightConfiguration::PointLightMaxRadius));
	const glm::mat4 mWorldScale = transMtx * proxyScale;

	// POINT LIGHT GEOMETRY : The spheres that will be used for the stencil pass
	pointLightProperties.proxyWorldViewProjectionMatrix = _viewProjectionMatrix * mWorldScale;

	// POINT LIGHT PROXIES : The "drawcalls" that will perform the actual rendering
	pointLightProperties.proxyWorldViewMatrix = _viewMatrix * mWorldScale;
	pointLightProperties.proxyViewSpaceLightPosition = glm::vec4((_viewMatrix * transMtx)[3]); // Translation component of the view matrix

	// POINT LIGHT SOURCES : The little balls that we render to show the lights
	pointLightProperties.worldViewProjectionMatrix = _viewProjectionMatrix * transMtx;

	pvr::FreeValue val;

	// update the Point Light step 1
	{
		pvr::utils::RendermanNode& pointLightNode =
			_deviceResources->render_mgr
				.toSubpassGroupModel(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1), 0)
				.toRendermanNode(pointLightIndex);

		pvr::utils::RendermanPipeline& pipeline =
			_deviceResources->render_mgr.toPipeline(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1), 0);

		val.setValue(pointLightProperties.proxyWorldViewProjectionMatrix);
		pipeline.updateBufferEntryNodeSemantic(PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::PROXYMODELVIEPROJECTIONMATRIX)], val, swapchainIndex, pointLightNode);
	}

	// update the point light step 2
	{
		pvr::utils::RendermanNode& pointLightNode =
			_deviceResources->render_mgr
				.toSubpassGroupModel(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep2), 0)
				.toRendermanNode(pointLightIndex);

		pvr::utils::RendermanPipeline& pipeline =
			_deviceResources->render_mgr.toPipeline(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep2), 0);

		val.setValue(pointLightProperties.proxyWorldViewMatrix);
		pipeline.updateBufferEntryNodeSemantic(PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::PROXYMODELVIEWMATRIX)], val, swapchainIndex, pointLightNode);

		val.setValue(pointLightProperties.proxyWorldViewProjectionMatrix);
		pipeline.updateBufferEntryNodeSemantic(PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::PROXYMODELVIEPROJECTIONMATRIX)], val, swapchainIndex, pointLightNode);

		val.setValue(pointLightProperties.proxyViewSpaceLightPosition);
		pipeline.updateBufferEntryNodeSemantic(PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::PROXYVIEWPOSITION)], val, swapchainIndex, pointLightNode);
	}

	// update the Point Light step 3
	{
		pvr::utils::RendermanNode& pointLightNode =
			_deviceResources->render_mgr
				.toSubpassGroupModel(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep3), 0)
				.toRendermanNode(pointLightIndex);

		pvr::utils::RendermanPipeline& pipeline =
			_deviceResources->render_mgr.toPipeline(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep3), 0);

		// update the Point light's dynamic buffers
		val.setValue(pointLightProperties.worldViewProjectionMatrix);
		pipeline.updateBufferEntryNodeSemantic(PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::MODELVIEWPROJECTIONMATRIX)], val, swapchainIndex, pointLightNode);
	}
}

/// <summary>Updates animation variables and camera matrices.</summary>
void VulkanDeferredShadingPFX::updateAnimation()
{
	glm::vec3 vTo, vUp;
	float fov;
	_mainScene->getCameraProperties(_cameraId, fov, _cameraPosition, vTo, vUp);

	// Update camera matrices
	static float angle = 0;
	if (_animateCamera) { angle += getFrameTime() / 5000.f; }
	_viewMatrix = glm::lookAt(glm::vec3(sin(angle) * 100.f + vTo.x, vTo.y + 30., cos(angle) * 100.f + vTo.z), vTo, vUp);
	_viewProjectionMatrix = _projectionMatrix * _viewMatrix;
	_inverseViewMatrix = glm::inverse(_viewMatrix);
}

/// <summary>Initialise the static light properties.</summary>
void VulkanDeferredShadingPFX::initialiseStaticLightProperties()
{
	RenderData& pass = _renderInfo;

	int32_t pointLight = 0;
	uint32_t directionalLight = 0;
	for (uint32_t i = 0; i < _mainScene->getNumLightNodes(); ++i)
	{
		const pvr::assets::Node& lightNode = _mainScene->getLightNode(i);
		const pvr::assets::Light& light = _mainScene->getLight(lightNode.getObjectId());
		switch (light.getType())
		{
		case pvr::assets::Light::Point: {
			if (pointLight >= PointLightConfiguration::MaxScenePointLights) { continue; }

			// POINT LIGHT GEOMETRY : The spheres that will be used for the stencil pass
			pass.pointLightPasses.lightProperties[pointLight].lightColor = glm::vec4(light.getColor(), 1.f);

			// POINT LIGHT PROXIES : The "drawcalls" that will perform the actual rendering
			pass.pointLightPasses.lightProperties[pointLight].lightIntensity = PointLightConfiguration::PointlightIntensity;

			// POINT LIGHT PROXIES : The "drawcalls" that will perform the actual rendering
			pass.pointLightPasses.lightProperties[pointLight].lightRadius = PointLightConfiguration::PointLightMaxRadius;

			// POINT LIGHT SOURCES : The little balls that we render to show the lights
			pass.pointLightPasses.lightProperties[pointLight].lightSourceColor = glm::vec4(light.getColor(), .8f);
			++pointLight;
		}
		break;
		case pvr::assets::Light::Directional: {
			pass.directionalLightPass.lightProperties[directionalLight].lightIntensity = glm::vec4(light.getColor(), 1.0f) * DirectionalLightConfiguration::DirectionalLightIntensity;
			++directionalLight;
		}
		break;
		default: break;
		}
	}

	if (DirectionalLightConfiguration::AdditionalDirectionalLight)
	{
		pass.directionalLightPass.lightProperties[directionalLight].lightIntensity = glm::vec4(1, 1, 1, 1) * DirectionalLightConfiguration::DirectionalLightIntensity;
		++directionalLight;
	}
}

/// <summary>Allocate memory for lighting data.</summary>
void VulkanDeferredShadingPFX::allocateLights()
{
	uint32_t countPoint = 0;
	uint32_t countDirectional = 0;
	for (uint32_t i = 0; i < _mainScene->getNumLightNodes(); ++i)
	{
		switch (_mainScene->getLight(_mainScene->getLightNode(i).getObjectId()).getType())
		{
		case pvr::assets::Light::Directional: ++countDirectional; break;
		case pvr::assets::Light::Point: ++countPoint; break;
		default: break;
		}
	}

	if (DirectionalLightConfiguration::AdditionalDirectionalLight) { ++countDirectional; }

	if (countPoint >= static_cast<uint32_t>(PointLightConfiguration::MaxScenePointLights)) { countPoint = PointLightConfiguration::MaxScenePointLights; }

	countPoint += PointLightConfiguration::NumProceduralPointLights;

	_numberOfPointLights = countPoint;
	_numberOfDirectionalLights = countDirectional;

	_renderInfo.directionalLightPass.lightProperties.resize(countDirectional);
	_renderInfo.pointLightPasses.lightProperties.resize(countPoint);
	_renderInfo.pointLightPasses.initialData.resize(countPoint);

	for (uint32_t i = countPoint - PointLightConfiguration::NumProceduralPointLights; i < countPoint; ++i)
	{ setProceduralPointLightInitialData(_renderInfo.pointLightPasses.initialData[i], _renderInfo.pointLightPasses.lightProperties[i]); }
}

/// <summary>Records main command buffer.</summary>
void VulkanDeferredShadingPFX::recordMainCommandBuffer()
{
	const pvrvk::Rect2D renderArea(0, 0, _windowWidth, _windowHeight);

	// Populate the clear values
	pvrvk::ClearValue clearValue[8];
	pvr::utils::populateClearValues(
		_deviceResources->render_mgr.toPass(0, 0).getFramebuffer(0)->getRenderPass(), pvrvk::ClearValue(0.f, 0.0f, 0.0f, 1.0f), pvrvk::ClearValue(1.f, 0u), clearValue);

	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		_deviceResources->cmdBufferMain[i]->begin();

		pvrvk::Framebuffer framebuffer = _deviceResources->render_mgr.toPass(0, 0).getFramebuffer(i);

		// Prepare the image for Presenting
		pvr::utils::setImageLayout(
			_deviceResources->swapchain->getImage(i), pvrvk::ImageLayout::e_PRESENT_SRC_KHR, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, _deviceResources->cmdBufferMain[i]);

		/// 1) Begin the render pass
		_deviceResources->cmdBufferMain[i]->beginRenderPass(_deviceResources->render_mgr.toPass(0, 0).framebuffer[i], renderArea, true, clearValue, framebuffer->getNumAttachments());

		/// 2) Record the scene in to the gbuffer
		_deviceResources->render_mgr.toSubpass(0, 0, static_cast<uint32_t>(RenderPassSubpass::GBuffer)).recordRenderingCommands(_deviceResources->cmdBufferMain[i], (uint16_t)i, false);

		/// 3) Begin the next subpass
		_deviceResources->cmdBufferMain[i]->nextSubpass(pvrvk::SubpassContents::e_INLINE);

		/// 4) record the directional lights Geometry stencil. Draw stencil to discard useless pixels
		_deviceResources->render_mgr.toSubpassGroup(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::DirectionalLight))
			.recordRenderingCommands(_deviceResources->cmdBufferMain[i], (uint16_t)i);

		for (uint32_t j = 0; j < _numberOfPointLights; j++)
		{
			/// 5) record the point light stencil
			recordCommandsPointLightGeometryStencil(_deviceResources->cmdBufferMain[i], i, j);

			/// 6) record the point light proxy
			_deviceResources->render_mgr.toSubpassGroup(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep2))
				.toSubpassGroupModel(0)
				.nodes[j]
				.recordRenderingCommands(_deviceResources->cmdBufferMain[i], (uint16_t)i);
		}

		/// 7) record the point light source
		_deviceResources->render_mgr.toSubpassGroup(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep3))
			.recordRenderingCommands(_deviceResources->cmdBufferMain[i], (uint16_t)i);

		/// 8) Render ui
		_deviceResources->uiRenderer.beginRendering(_deviceResources->cmdBufferMain[i]);
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getDefaultControls()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.endRendering();
		_deviceResources->cmdBufferMain[i]->endRenderPass();

		// Prepare the image for Presenting
		pvr::utils::setImageLayout(
			_deviceResources->swapchain->getImage(i), pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, pvrvk::ImageLayout::e_PRESENT_SRC_KHR, _deviceResources->cmdBufferMain[i]);
		_deviceResources->cmdBufferMain[i]->end();
	}
}

/// <summary>Record point light stencil commands.</summary>
/// <param name="cmdBuffers"> SecondaryCommandBuffer to record.</param>
/// <param name="swapChainIndex"> Current swap chain index.</param>
/// <param name="subpass"> Current sub pass.</param>
void VulkanDeferredShadingPFX::recordCommandsPointLightGeometryStencil(pvrvk::CommandBuffer& cmdBuffers, uint32_t swapChainIndex, const uint32_t pointLight)
{
	pvrvk::ClearRect clearArea(pvrvk::Rect2D(0, 0, _framebufferWidth, _framebufferHeight));
	if ((_framebufferWidth != _windowWidth) || (_framebufferHeight != _windowHeight))
	{ clearArea.setRect(pvrvk::Rect2D(_viewportOffsets[0], _viewportOffsets[1], _framebufferWidth, _framebufferHeight)); }

	// clear stencil to 0's to make use of it again for point lights
	cmdBuffers->clearAttachment(pvrvk::ClearAttachment::createStencilClearAttachment(0u), clearArea);

	// record the rendering commands for the point light stencil pass
	_deviceResources->render_mgr.toSubpassGroup(0, 0, static_cast<uint32_t>(RenderPassSubpass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1))
		.toSubpassGroupModel(0)
		.nodes[pointLight]
		.recordRenderingCommands(cmdBuffers, static_cast<uint16_t>(swapChainIndex));
}

/// <summary>This function must be implemented by the user of the shell. The user should return its Shell object defining the
/// behaviour of the application.</summary>
/// <returns>Return an unique_ptr to a new Demo class,supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanDeferredShadingPFX>(); }
