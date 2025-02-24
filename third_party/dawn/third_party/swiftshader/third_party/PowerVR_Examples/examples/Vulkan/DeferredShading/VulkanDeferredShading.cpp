/*!
\brief Implements a deferred shading technique supporting point and directional lights.
\file VulkanDeferredShading.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRVk/PVRVk.h"
#include "PVRUtils/PVRUtilsVk.h"

// Maximum number of swap images supported
enum CONSTANTS
{
	MAX_NUMBER_OF_SWAP_IMAGES = 4
};

// Shader vertex Bindings
const pvr::utils::VertexBindings_Name vertexBindings[] = { { "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoords" }, { "TANGENT", "inTangent" } };

const pvr::utils::VertexBindings_Name floorVertexBindings[] = { { "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoords" } };

const pvr::utils::VertexBindings_Name pointLightVertexBindings[] = { { "POSITION", "inVertex" } };

// Framebuffer colour attachment indices
namespace FramebufferGBufferAttachments {
enum Enum
{
	Albedo = 0,
	Normal,
	Depth,
	Count
};
}

// Light mesh nodes
namespace LightNodes {
enum Enum
{
	PointLightMeshNode = 0,
	NumberOfPointLightMeshNodes
};
}

// mesh nodes
namespace MeshNodes {
enum Enum
{
	Satyr = 0,
	Floor = 1,
	NumberOfMeshNodes
};
}

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

/// <summary>structure used to draw the point light sources.</summary>
struct DrawPointLightSources
{
	pvrvk::GraphicsPipeline pipeline;
};

/// <summary>structure used to draw the proxy point light.</summary>
struct DrawPointLightProxy
{
	pvrvk::GraphicsPipeline pipeline;
};

/// <summary>structure used to fill the stencil buffer used for optimising the proxy point light pass.</summary>
struct PointLightGeometryStencil
{
	pvrvk::GraphicsPipeline pipeline;
};

/// <summary>structure used to render directional lighting.</summary>
struct DrawDirectionalLight
{
	pvrvk::GraphicsPipeline pipeline;
	struct DirectionalLightProperties
	{
		glm::vec4 lightIntensity;
		glm::vec4 ambientLight;
		glm::vec4 viewSpaceLightDirection;
	};
	std::vector<DirectionalLightProperties> lightProperties;
};

/// <summary>structure used to fill the GBuffer.</summary>
struct DrawGBuffer
{
	struct Objects
	{
		pvrvk::GraphicsPipeline pipeline;
		glm::mat4 world;
		glm::mat4 worldView;
		glm::mat4 worldViewProj;
		glm::mat4 worldViewIT4x4;
	};
	std::vector<Objects> objects;
};

/// <summary>structure used to hold the rendering information for the demo.</summary>
struct RenderData
{
	DrawGBuffer storeLocalMemoryPass; // Subpass 0
	DrawDirectionalLight directionalLightPass; // Subpass 1
	PointLightGeometryStencil pointLightGeometryStencilPass; // Subpass 1
	DrawPointLightProxy pointLightProxyPass; // Subpass 1
	DrawPointLightSources pointLightSourcesPass; // Subpass 1
	PointLightPasses pointLightPasses; // holds point light data
};

/// <summary>Shader names for all of the demo passes.</summary>
namespace Files {
const char* const PointLightModelFile = "pointlight.pod";
const char* const SceneFile = "SatyrAndTable.pod";

const char* const GBufferVertexShader = "GBufferVertexShader.vsh.spv";
const char* const GBufferFragmentShader = "GBufferFragmentShader.fsh.spv";

const char* const GBufferFloorVertexShader = "GBufferFloorVertexShader.vsh.spv";
const char* const GBufferFloorFragmentShader = "GBufferFloorFragmentShader.fsh.spv";

const char* const AttributelessVertexShader = "AttributelessVertexShader.vsh.spv";

const char* const DirectionalLightingFragmentShader = "DirectionalLightFragmentShader.fsh.spv";

const char* const PointLightPass1FragmentShader = "PointLightPass1FragmentShader.fsh.spv";
const char* const PointLightPass1VertexShader = "PointLightPass1VertexShader.vsh.spv";

const char* const PointLightPass2FragmentShader = "PointLightPass2FragmentShader.fsh.spv";
const char* const PointLightPass2VertexShader = "PointLightPass2VertexShader.vsh.spv";

const char* const PointLightPass3FragmentShader = "PointLightPass3FragmentShader.fsh.spv";
const char* const PointLightPass3VertexShader = "PointLightPass3VertexShader.vsh.spv";
} // namespace Files

/// <summary>buffer entry names used for the structured memory views used throughout the demo.
/// These entry names must match the variable names used in the demo shaders.</summary>
namespace BufferEntryNames {
namespace PerScene {
const char* const FarClipDistance = "fFarClipDistance";
}

namespace PerModelMaterial {
const char* const SpecularStrength = "fSpecularStrength";
const char* const DiffuseColor = "vDiffuseColor";
} // namespace PerModelMaterial

namespace PerModel {
const char* const WorldViewProjectionMatrix = "mWorldViewProjectionMatrix";
const char* const WorldViewMatrix = "mWorldViewMatrix";
const char* const WorldViewITMatrix = "mWorldViewITMatrix";
} // namespace PerModel

namespace PerPointLight {
const char* const LightIntensity = "vLightIntensity";
const char* const LightRadius = "vLightRadius";
const char* const LightColor = "vLightColor";
const char* const LightSourceColor = "vLightSourceColor";
const char* const WorldViewProjectionMatrix = "mWorldViewProjectionMatrix";
const char* const ProxyLightViewPosition = "vViewPosition";
const char* const ProxyWorldViewProjectionMatrix = "mProxyWorldViewProjectionMatrix";
const char* const ProxyWorldViewMatrix = "mProxyWorldViewMatrix";
} // namespace PerPointLight

namespace PerDirectionalLight {
const char* const LightIntensity = "fLightIntensity";
const char* const LightViewDirection = "vViewDirection";
const char* const AmbientLight = "fAmbientLight";
} // namespace PerDirectionalLight
} // namespace BufferEntryNames

// Application wide configuration data
namespace ApplicationConfiguration {
const float FrameRate = 1.0f / 120.0f;
}

// Directional lighting configuration data
namespace DirectionalLightConfiguration {
static bool AdditionalDirectionalLight = true;
const float DirectionalLightIntensity = .1f;
const glm::vec4 AmbientLightColor = glm::vec4(.005f, .005f, .005f, 0.0f);
} // namespace DirectionalLightConfiguration

// Point lighting configuration data
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

int32_t MaxScenePointLights = 5;
int32_t NumProceduralPointLights = 10;
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
namespace RenderPassSubpasses {
const uint32_t GBuffer = 0;
// lighting pass
const uint32_t Lighting = 1;
// UI pass
const uint32_t UIRenderer = 1;

const uint32_t NumberOfSubpasses = 2;
} // namespace RenderPassSubpasses

struct Material
{
	pvrvk::GraphicsPipeline materialPipeline;
	std::vector<pvrvk::DescriptorSet> materialDescriptorSet;
	float specularStrength;
	glm::vec3 diffuseColor;
};

struct DeviceResources
{
	pvrvk::Instance instance;
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
	pvrvk::Device device;
	pvrvk::Queue queue;
	pvrvk::Swapchain swapchain;
	pvr::utils::vma::Allocator vmaAllocator;
	pvrvk::CommandPool commandPool;
	pvrvk::DescriptorPool descriptorPool;

	// Local memory frame buffer
	pvr::Multi<pvrvk::Framebuffer> onScreenLocalMemoryFramebuffer;
	pvr::Multi<pvrvk::ImageView> depthStencilImages;
	pvr::Multi<pvrvk::FramebufferCreateInfo> onScreenFramebufferCreateInfos;

	// Stores Texture views for the Images used as attachments on the local memory frame buffer
	pvr::Multi<pvrvk::ImageView> framebufferGbufferImages[FramebufferGBufferAttachments::Count];

	// Common renderpass used for the demo
	pvrvk::RenderPass onScreenLocalMemoryRenderPass;

	// Vbo and Ibos used for lighting data
	pvrvk::Buffer pointLightVbo;
	pvrvk::Buffer pointLightIbo;

	//// Command Buffers ////
	// Main Primary Command Buffer
	pvrvk::CommandBuffer cmdBufferMain[MAX_NUMBER_OF_SWAP_IMAGES];

	// Secondary command buffers used for each pass
	pvrvk::SecondaryCommandBuffer cmdBufferRenderToLocalMemory[MAX_NUMBER_OF_SWAP_IMAGES];
	pvrvk::SecondaryCommandBuffer cmdBufferLighting[MAX_NUMBER_OF_SWAP_IMAGES];

	////  Descriptor Set Layouts ////
	// Layouts used for GBuffer rendering
	pvrvk::DescriptorSetLayout staticSceneLayout;
	pvrvk::DescriptorSetLayout noSamplerLayout;
	pvrvk::DescriptorSetLayout oneSamplerLayout;
	pvrvk::DescriptorSetLayout twoSamplerLayout;
	pvrvk::DescriptorSetLayout threeSamplerLayout;
	pvrvk::DescriptorSetLayout fourSamplerLayout;

	// Directional lighting descriptor set layout
	pvrvk::DescriptorSetLayout directionalLightingDescriptorLayout;
	// Point light stencil pass descriptor set layout
	pvrvk::DescriptorSetLayout pointLightGeometryStencilDescriptorLayout;
	// Point Proxy light pass descriptor set layout used for buffers
	pvrvk::DescriptorSetLayout pointLightProxyDescriptorLayout;
	// Point Proxy light pass descriptor set layout used for local memory
	pvrvk::DescriptorSetLayout pointLightProxyLocalMemoryDescriptorLayout;
	// Point light source descriptor set layout used for buffers
	pvrvk::DescriptorSetLayout pointLightSourceDescriptorLayout;

	////  Descriptor Sets ////
	// GBuffer Materials structures
	std::vector<Material> materials;
	// Directional Lighting descriptor set
	pvr::Multi<pvrvk::DescriptorSet> directionalLightingDescriptorSets;
	// Point light stencil descriptor set
	pvr::Multi<pvrvk::DescriptorSet> pointLightGeometryStencilDescriptorSets;
	// Point light Proxy descriptor set
	pvr::Multi<pvrvk::DescriptorSet> pointLightProxyDescriptorSets;
	pvr::Multi<pvrvk::DescriptorSet> pointLightProxyLocalMemoryDescriptorSets;
	// Point light Source descriptor set
	pvr::Multi<pvrvk::DescriptorSet> pointLightSourceDescriptorSets;
	// Scene wide descriptor set
	pvrvk::DescriptorSet sceneDescriptorSet;

	//// Pipeline Layouts ////
	// GBuffer pipeline layouts
	pvrvk::PipelineLayout pipeLayoutNoSamplers;
	pvrvk::PipelineLayout pipeLayoutOneSampler;
	pvrvk::PipelineLayout pipeLayoutTwoSamplers;
	pvrvk::PipelineLayout pipeLayoutThreeSamplers;
	pvrvk::PipelineLayout pipeLayoutFourSamplers;

	// Directional lighting pipeline layout
	pvrvk::PipelineLayout directionalLightingPipelineLayout;
	// Point lighting stencil pipeline layout
	pvrvk::PipelineLayout pointLightGeometryStencilPipelineLayout;
	// Point lighting proxy pipeline layout
	pvrvk::PipelineLayout pointLightProxyPipelineLayout;
	// Point lighting source pipeline layout
	pvrvk::PipelineLayout pointLightSourcePipelineLayout;
	// Scene Wide pipeline layout
	pvrvk::PipelineLayout scenePipelineLayout;

	// scene Vbos and Ibos
	std::vector<pvrvk::Buffer> sceneVbos;
	std::vector<pvrvk::Buffer> sceneIbos;

	//// Structured Memory Views ////
	// scene wide buffers
	pvr::utils::StructuredBufferView farClipDistanceBufferView;
	pvrvk::Buffer farClipDistanceBuffer;
	// Static materials buffers
	pvr::utils::StructuredBufferView modelMaterialBufferView;
	pvrvk::Buffer modelMaterialBuffer;
	// Dynamic matrices buffers
	pvr::utils::StructuredBufferView modelMatrixBufferView;
	pvrvk::Buffer modelMatrixBuffer;
	// Static point light buffers
	pvr::utils::StructuredBufferView staticPointLightBufferView;
	pvrvk::Buffer staticPointLightBuffer;
	// Dynamic point light buffer
	pvr::utils::StructuredBufferView dynamicPointLightBufferView;
	pvrvk::Buffer dynamicPointLightBuffer;
	// Static Directional lighting buffer
	pvr::utils::StructuredBufferView staticDirectionalLightBufferView;
	pvrvk::Buffer staticDirectionalLightBuffer;
	// Dynamic Directional lighting buffers
	pvr::utils::StructuredBufferView dynamicDirectionalLightBufferView;
	pvrvk::Buffer dynamicDirectionalLightBuffer;

	pvrvk::Semaphore imageAcquiredSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore presentationSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameResourcesFences[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	RenderData renderInfo;

	pvrvk::PipelineCache pipelineCache;

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
class VulkanDeferredShading : public pvr::Shell
{
public:
	//// Frame ////
	uint32_t _numSwapImages;
	uint32_t _swapchainIndex;
	// Putting all API objects into a pointer just makes it easier to release them all together with RAII
	std::unique_ptr<DeviceResources> _deviceResources;

	// Frame counters for animation
	uint32_t _frameId;
	float _frameNumber;
	bool _isPaused;
	uint32_t _cameraId;
	bool _animateCamera;

	uint32_t _numberOfPointLights;
	uint32_t _numberOfDirectionalLights;

	// Projection and Model View matrices
	glm::mat4 _viewMatrix;
	glm::mat4 _projectionMatrix;
	glm::mat4 _viewProjectionMatrix;
	glm::mat4 _inverseViewMatrix;
	float _farClipDistance;

	uint32_t _windowWidth;
	uint32_t _windowHeight;
	uint32_t _framebufferWidth;
	uint32_t _framebufferHeight;

	int32_t _viewportOffsets[2];

	// Light models
	pvr::assets::ModelHandle _pointLightModel;

	// Object model
	pvr::assets::ModelHandle _mainScene;

	VulkanDeferredShading()
	{
		_animateCamera = false;
		_isPaused = false;
	}

	//  Overridden from pvr::Shell
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void createFramebufferAndRenderPass();
	void createPipelines();
	void createModelPipelines();
	void createDirectionalLightingPipeline();
	void createPointLightStencilPipeline();
	void createPointLightProxyPipeline();
	void createPointLightSourcePipeline();
	void recordCommandBufferRenderGBuffer(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapChainIndex, uint32_t subpass);
	void recordCommandsDirectionalLights(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapChainIndex);
	void recordCommandsPointLightGeometryStencil(
		pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapChainIndex, uint32_t subpass, const uint32_t pointLight, const pvr::assets::Mesh& pointLightMesh);
	void recordCommandsPointLightProxy(
		pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapChainIndex, uint32_t subpass, const uint32_t pointLight, const pvr::assets::Mesh& pointLightMesh);
	void recordCommandsPointLightSourceLighting(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapChainIndex, uint32_t subpass);
	void recordMainCommandBuffer();
	void recordCommandUIRenderer(pvrvk::SecondaryCommandBuffer& cmdBuffers);
	void recordSecondaryCommandBuffers();
	void allocateLights();
	void createMaterialsAndDescriptorSets(pvrvk::CommandBuffer& uploadCmd);
	void createStaticSceneDescriptorSet();
	void loadVbos(pvrvk::CommandBuffer& uploadCmd);
	void uploadStaticData();
	void uploadStaticSceneData();
	void uploadStaticModelData();
	void uploadStaticDirectionalLightData();
	void uploadStaticPointLightData();
	void initialiseStaticLightProperties();
	void updateDynamicSceneData();
	void createBuffers();
	void createSceneWideBuffers();
	void updateAnimation();
	void updateProceduralPointLight(PointLightPasses::InitialData& data, PointLightPasses::PointLightProperties& pointLightProperties, bool initia);
	void createModelBuffers();
	void createLightingBuffers();
	void createDirectionalLightingBuffers();
	void createPointLightBuffers();
	void createDirectionalLightDescriptorSets();
	void createPointLightGeometryStencilPassDescriptorSets();
	void createPointLightProxyPassDescriptorSets();
	void createPointLightSourcePassDescriptorSets();

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
};

/// <summary> Code in initApplication() will be called by pvr::Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns> Return true if no error occurred. </returns>
pvr::Result VulkanDeferredShading::initApplication()
{
	// This demo application makes heavy use of the stencil buffer
	setStencilBitsPerPixel(8);
	_frameNumber = 0.0f;
	_isPaused = false;
	_cameraId = 0;
	_frameId = 0;

	//  Load the scene and the light
	_mainScene = pvr::assets::loadModel(*this, Files::SceneFile);

	if (_mainScene->getNumCameras() == 0) { throw std::runtime_error("ERROR: The main scene to display must contain a camera.\n"); }

	//  Load light proxy geometry
	_pointLightModel = pvr::assets::loadModel(*this, Files::PointLightModelFile);

	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.).</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanDeferredShading::initView()
{
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

	// Create the device and retrieve its queues
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), queueFlagsInfo, ARRAY_SIZE(queueFlagsInfo), &queueAccessInfo);

	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{ swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT; } // create the swapchain

	// We do not support automatic MSAA for this demo.
	if (getDisplayAttributes().aaSamples > 1)
	{
		Log(LogLevel::Warning, "Full Screen Multisample Antialiasing requested, but not supported for this demo's configuration.");
		getDisplayAttributes().aaSamples = 1;
	}
	
	// Create the Swapchain
	_deviceResources->swapchain = pvr::utils::createSwapchain(_deviceResources->device, surface, getDisplayAttributes(), swapchainImageUsage);
	// Get the number of swap images
	_numSwapImages = _deviceResources->swapchain->getSwapchainLength();

	// Create the Depth/Stencil buffer images
	pvr::utils::createAttachmentImages(_deviceResources->depthStencilImages, _deviceResources->device, _numSwapImages,
		pvr::utils::getSupportedDepthStencilFormat(_deviceResources->device, getDisplayAttributes()), _deviceResources->swapchain->getDimension(),
		pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT, pvrvk::SampleCountFlags::e_1_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT, "DepthStencilBufferImages");

	// Get current swap index
	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	// initialise the gbuffer renderpass list
	_deviceResources->renderInfo.storeLocalMemoryPass.objects.resize(_mainScene->getNumMeshNodes());

	// calculate the frame buffer width and heights
	_framebufferWidth = _windowWidth = this->getWidth();
	_framebufferHeight = _windowHeight = this->getHeight();

	const pvr::CommandLine& commandOptions = getCommandLine();
	int32_t intFramebufferWidth = -1;
	int32_t intFramebufferHeight = -1;
	commandOptions.getIntOption("-fbowidth", intFramebufferWidth);
	_framebufferWidth = static_cast<uint32_t>(intFramebufferWidth);
	_framebufferWidth = glm::min<int32_t>(_framebufferWidth, _windowWidth);
	commandOptions.getIntOption("-fboheight", intFramebufferHeight);
	_framebufferHeight = static_cast<uint32_t>(intFramebufferHeight);
	_framebufferHeight = glm::min<int32_t>(_framebufferHeight, _windowHeight);
	commandOptions.getIntOption("-numlights", PointLightConfiguration::NumProceduralPointLights);
	commandOptions.getFloatOption("-lightintensity", PointLightConfiguration::PointlightIntensity);

	_viewportOffsets[0] = (_windowWidth - _framebufferWidth) / 2;
	_viewportOffsets[1] = (_windowHeight - _framebufferHeight) / 2;

	Log(LogLevel::Information, "Framebuffer dimensions: %d x %d\n", _framebufferWidth, _framebufferHeight);
	Log(LogLevel::Information, "On-screen Framebuffer dimensions: %d x %d\n", _windowWidth, _windowHeight);

	// create the command pool
	_deviceResources->commandPool =
		_deviceResources->device->createCommandPool(pvrvk::CommandPoolCreateInfo(queueAccessInfo.familyId, pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, 48)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 48)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 48)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_INPUT_ATTACHMENT, 48)
																						  .setMaxDescriptorSets(32));

	// setup command buffers
	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		// main command buffer
		_deviceResources->cmdBufferMain[i] = _deviceResources->commandPool->allocateCommandBuffer();

		// Subpass 0
		_deviceResources->cmdBufferRenderToLocalMemory[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();

		// Subpass 1
		_deviceResources->cmdBufferLighting[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();

		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
	}

	// Create the renderpass using subpasses
	createFramebufferAndRenderPass();

	// Initialise lighting structures
	allocateLights();

	// Create buffers used in the demo
	createBuffers();

	// Initialise the static light properties
	initialiseStaticLightProperties();

	// Create static scene wide descriptor set
	createStaticSceneDescriptorSet();

	_deviceResources->cmdBufferMain[0]->begin();
	// Create the descriptor sets used for the GBuffer pass
	createMaterialsAndDescriptorSets(_deviceResources->cmdBufferMain[0]);

	//  Load objects from the scene into VBOs
	loadVbos(_deviceResources->cmdBufferMain[0]);

	_deviceResources->cmdBufferMain[0]->end();
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->cmdBufferMain[0];
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle(); // wait

	// Upload static data
	uploadStaticData();

	// Create lighting descriptor sets
	createDirectionalLightDescriptorSets();
	createPointLightGeometryStencilPassDescriptorSets();
	createPointLightProxyPassDescriptorSets();
	createPointLightSourcePassDescriptorSets();

	// setup UI renderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenLocalMemoryRenderPass, RenderPassSubpasses::UIRenderer,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue);
	_deviceResources->uiRenderer.getDefaultTitle()->setText("DeferredShading");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Action1: Pause\nAction2: Orbit Camera\n");
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	// Handle device rotation
	bool isRotated = this->isScreenRotated();
	if (isRotated)
	{
		_projectionMatrix = pvr::math::perspective(pvr::Api::Vulkan, _mainScene->getCamera(0).getFOV(), static_cast<float>(this->getHeight()) / static_cast<float>(this->getWidth()),
			_mainScene->getCamera(0).getNear(), _mainScene->getCamera(0).getFar(), glm::pi<float>() * .5f);
	}
	else
	{
		_projectionMatrix = pvr::math::perspective(pvr::Api::Vulkan, _mainScene->getCamera(0).getFOV(),
			static_cast<float>(this->getWidth()) / static_cast<float>(this->getHeight()), _mainScene->getCamera(0).getNear(), _mainScene->getCamera(0).getFar());
	}

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	// Create demo pipelines
	createPipelines();

	// Record all secondary command buffers
	recordSecondaryCommandBuffers();

	// Record the main command buffer
	recordMainCommandBuffer();

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanDeferredShading::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
///	If the rendering context is lost, quitApplication() will not be called.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanDeferredShading::quitApplication()
{
	_mainScene.reset();
	_pointLightModel.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred</returns>
pvr::Result VulkanDeferredShading::renderFrame()
{
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameResourcesFences[_swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[_swapchainIndex]->reset();

	//  Handle user input and update object animations
	updateAnimation();

	// update dynamic buffers
	updateDynamicSceneData();

	//--------------------
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
	// Present
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

/// <summary>Creates directional lighting descriptor sets.</summary>
void VulkanDeferredShading::createDirectionalLightDescriptorSets()
{
	{
		// create the descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;

		// Buffers
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		// Input attachments
		descSetInfo.setBinding(2, pvrvk::DescriptorType::e_INPUT_ATTACHMENT, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetInfo.setBinding(3, pvrvk::DescriptorType::e_INPUT_ATTACHMENT, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetInfo.setBinding(4, pvrvk::DescriptorType::e_INPUT_ATTACHMENT, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		_deviceResources->directionalLightingDescriptorLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);

		{
			// create the pipeline layout
			pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

			pipeLayoutInfo.setDescSetLayout(0, _deviceResources->directionalLightingDescriptorLayout);
			_deviceResources->directionalLightingPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
		}
		pvrvk::WriteDescriptorSet descSetUpdate[pvrvk::FrameworkCaps::MaxSwapChains * 5];

		// create the swapchain descriptor sets with corresponding buffers/images
		for (uint32_t i = 0; i < _numSwapImages; ++i)
		{
			_deviceResources->directionalLightingDescriptorSets.add(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->directionalLightingDescriptorLayout));
			descSetUpdate[i * 5]
				.set(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->directionalLightingDescriptorSets[i], 0)
				.setBufferInfo(
					0, pvrvk::DescriptorBufferInfo(_deviceResources->staticDirectionalLightBuffer, 0, _deviceResources->staticDirectionalLightBufferView.getDynamicSliceSize()));

			descSetUpdate[i * 5 + 1]
				.set(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->directionalLightingDescriptorSets[i], 1)
				.setBufferInfo(
					0, pvrvk::DescriptorBufferInfo(_deviceResources->dynamicDirectionalLightBuffer, 0, _deviceResources->dynamicDirectionalLightBufferView.getDynamicSliceSize()));

			descSetUpdate[i * 5 + 2]
				.set(pvrvk::DescriptorType::e_INPUT_ATTACHMENT, _deviceResources->directionalLightingDescriptorSets[i], 2)
				.setImageInfo(0,
					pvrvk::DescriptorImageInfo(_deviceResources->framebufferGbufferImages[FramebufferGBufferAttachments::Albedo][i], pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

			descSetUpdate[i * 5 + 3]
				.set(pvrvk::DescriptorType::e_INPUT_ATTACHMENT, _deviceResources->directionalLightingDescriptorSets[i], 3)
				.setImageInfo(0,
					pvrvk::DescriptorImageInfo(_deviceResources->framebufferGbufferImages[FramebufferGBufferAttachments::Normal][i], pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

			descSetUpdate[i * 5 + 4]
				.set(pvrvk::DescriptorType::e_INPUT_ATTACHMENT, _deviceResources->directionalLightingDescriptorSets[i], 4)
				.setImageInfo(0,
					pvrvk::DescriptorImageInfo(_deviceResources->framebufferGbufferImages[FramebufferGBufferAttachments::Depth][i], pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
		}
		_deviceResources->device->updateDescriptorSets(descSetUpdate, _numSwapImages * 5, nullptr, 0);
	}
}

/// <summary>Creates point lighting stencil pass descriptor sets.</summary>
void VulkanDeferredShading::createPointLightGeometryStencilPassDescriptorSets()
{
	{
		// create descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;

		// buffers
		descSetLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetLayoutInfo.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, pvrvk::ShaderStageFlags::e_VERTEX_BIT);

		_deviceResources->pointLightGeometryStencilDescriptorLayout = _deviceResources->device->createDescriptorSetLayout(descSetLayoutInfo);

		{
			// create the pipeline layout
			pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

			pipeLayoutInfo.setDescSetLayout(0, _deviceResources->staticSceneLayout);
			pipeLayoutInfo.setDescSetLayout(1, _deviceResources->pointLightGeometryStencilDescriptorLayout);
			_deviceResources->pointLightGeometryStencilPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
		}
		std::vector<pvrvk::WriteDescriptorSet> writeDescSets(_numSwapImages * 2);
		// create the swapchain descriptor sets with corresponding buffers
		for (uint32_t i = 0; i < _numSwapImages; ++i)
		{
			_deviceResources->pointLightGeometryStencilDescriptorSets.add(
				_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->pointLightGeometryStencilDescriptorLayout));

			pvrvk::WriteDescriptorSet* descSetUpdate = &writeDescSets[i * 2];
			descSetUpdate->set(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->pointLightGeometryStencilDescriptorSets[i], 0);
			descSetUpdate->setBufferInfo(
				0, pvrvk::DescriptorBufferInfo(_deviceResources->staticPointLightBuffer, 0, _deviceResources->staticPointLightBufferView.getDynamicSliceSize()));

			descSetUpdate = &writeDescSets[i * 2 + 1];
			descSetUpdate->set(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->pointLightGeometryStencilDescriptorSets[i], 1);
			descSetUpdate->setBufferInfo(
				0, pvrvk::DescriptorBufferInfo(_deviceResources->dynamicPointLightBuffer, 0, _deviceResources->dynamicPointLightBufferView.getDynamicSliceSize()));
		}
		_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
	}
}

/// <summary> Creates point lighting proxy pass descriptor sets.</summary>
void VulkanDeferredShading::createPointLightProxyPassDescriptorSets()
{
	{
		// create buffer descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;

		// Buffers
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		_deviceResources->pointLightProxyDescriptorLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);

		pvrvk::DescriptorSetLayoutCreateInfo localMemoryDescSetInfo;

		// Input attachment descriptor set layout
		localMemoryDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_INPUT_ATTACHMENT, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		localMemoryDescSetInfo.setBinding(1, pvrvk::DescriptorType::e_INPUT_ATTACHMENT, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		localMemoryDescSetInfo.setBinding(2, pvrvk::DescriptorType::e_INPUT_ATTACHMENT, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		_deviceResources->pointLightProxyLocalMemoryDescriptorLayout = _deviceResources->device->createDescriptorSetLayout(localMemoryDescSetInfo);

		{
			// create the pipeline layout
			pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
			pipeLayoutInfo.setDescSetLayout(0, _deviceResources->staticSceneLayout);
			pipeLayoutInfo.setDescSetLayout(1, _deviceResources->pointLightProxyDescriptorLayout);
			pipeLayoutInfo.setDescSetLayout(2, _deviceResources->pointLightProxyLocalMemoryDescriptorLayout);
			_deviceResources->pointLightProxyPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
		}

		// create the swapchain descriptor sets with corresponding buffers
		std::vector<pvrvk::WriteDescriptorSet> descSetWrites;
		for (uint32_t i = 0; i < _numSwapImages; ++i)
		{
			_deviceResources->pointLightProxyDescriptorSets.add(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->pointLightProxyDescriptorLayout));

			descSetWrites.push_back(
				pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->pointLightProxyDescriptorSets[i], 0)
					.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->staticPointLightBuffer, 0, _deviceResources->staticPointLightBufferView.getDynamicSliceSize())));

			descSetWrites.push_back(
				pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->pointLightProxyDescriptorSets[i], 1)
					.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->dynamicPointLightBuffer, 0, _deviceResources->dynamicPointLightBufferView.getDynamicSliceSize())));
		}

		_deviceResources->pointLightProxyLocalMemoryDescriptorLayout = _deviceResources->device->createDescriptorSetLayout(localMemoryDescSetInfo);
		// create the swapchain descriptor sets with corresponding images
		for (uint32_t i = 0; i < _numSwapImages; ++i)
		{
			_deviceResources->pointLightProxyLocalMemoryDescriptorSets.add(
				_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->pointLightProxyLocalMemoryDescriptorLayout));
			descSetWrites.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_INPUT_ATTACHMENT, _deviceResources->pointLightProxyLocalMemoryDescriptorSets[i], 0)
										.setImageInfo(0,
											pvrvk::DescriptorImageInfo(_deviceResources->framebufferGbufferImages[FramebufferGBufferAttachments::Albedo][i],
												pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

			descSetWrites.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_INPUT_ATTACHMENT, _deviceResources->pointLightProxyLocalMemoryDescriptorSets[i], 1)
										.setImageInfo(0,
											pvrvk::DescriptorImageInfo(_deviceResources->framebufferGbufferImages[FramebufferGBufferAttachments::Normal][i],
												pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

			descSetWrites.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_INPUT_ATTACHMENT, _deviceResources->pointLightProxyLocalMemoryDescriptorSets[i], 2)
										.setImageInfo(0,
											pvrvk::DescriptorImageInfo(_deviceResources->framebufferGbufferImages[FramebufferGBufferAttachments::Depth][i],
												pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
		}
		_deviceResources->device->updateDescriptorSets(descSetWrites.data(), static_cast<uint32_t>(descSetWrites.size()), nullptr, 0);
	}
}

/// <summary>Creates point lighting source pass descriptor sets.</summary>
void VulkanDeferredShading::createPointLightSourcePassDescriptorSets()
{
	{
		// create descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;

		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, pvrvk::ShaderStageFlags::e_VERTEX_BIT);

		_deviceResources->pointLightSourceDescriptorLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);

		{
			// create the pipeline layout
			pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

			pipeLayoutInfo.setDescSetLayout(0, _deviceResources->staticSceneLayout);
			pipeLayoutInfo.setDescSetLayout(1, _deviceResources->pointLightSourceDescriptorLayout);
			_deviceResources->pointLightSourcePipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
		}

		// create the swapchain descriptor sets with corresponding buffers
		std::vector<pvrvk::WriteDescriptorSet> descSetUpdate;
		for (uint32_t i = 0; i < _numSwapImages; ++i)
		{
			_deviceResources->pointLightSourceDescriptorSets.add(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->pointLightSourceDescriptorLayout));
			descSetUpdate.push_back(
				pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->pointLightSourceDescriptorSets[i], 0)
					.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->staticPointLightBuffer, 0, _deviceResources->staticPointLightBufferView.getDynamicSliceSize())));

			descSetUpdate.push_back(
				pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->pointLightSourceDescriptorSets[i], 1)
					.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->dynamicPointLightBuffer, 0, _deviceResources->dynamicPointLightBufferView.getDynamicSliceSize())));
		}
		_deviceResources->device->updateDescriptorSets(descSetUpdate.data(), static_cast<uint32_t>(descSetUpdate.size()), nullptr, 0);
	}
}

/// <summary>Creates static scene wide descriptor set.</summary>
void VulkanDeferredShading::createStaticSceneDescriptorSet()
{
	// static per scene buffer
	pvrvk::DescriptorSetLayoutCreateInfo staticSceneDescSetInfo;
	staticSceneDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	_deviceResources->staticSceneLayout = _deviceResources->device->createDescriptorSetLayout(staticSceneDescSetInfo);

	// Create static descriptor set for the scene

	_deviceResources->sceneDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->staticSceneLayout);
	pvrvk::WriteDescriptorSet descSetUpdate(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->sceneDescriptorSet, 0);
	descSetUpdate.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->farClipDistanceBuffer, 0, _deviceResources->farClipDistanceBufferView.getDynamicSliceSize()));
	_deviceResources->device->updateDescriptorSets(&descSetUpdate, 1, nullptr, 0);
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

	pipeLayoutInfo.setDescSetLayout(0, _deviceResources->staticSceneLayout);
	_deviceResources->scenePipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
}

/// <summary>Loads the textures required for this example and sets up the GBuffer descriptor sets.</summary>
/// <returns>Return true if no error occurred.</returns>
void VulkanDeferredShading::createMaterialsAndDescriptorSets(pvrvk::CommandBuffer& uploadCmd)
{
	if (_mainScene->getNumMaterials() == 0) { throw std::runtime_error("ERROR: The scene does not contain any materials."); }
	// CREATE THE SAMPLERS
	// create trilinear sampler
	pvrvk::SamplerCreateInfo samplerDesc;
	samplerDesc.wrapModeU = samplerDesc.wrapModeV = samplerDesc.wrapModeW = pvrvk::SamplerAddressMode::e_REPEAT;

	samplerDesc.minFilter = pvrvk::Filter::e_LINEAR;
	samplerDesc.magFilter = pvrvk::Filter::e_LINEAR;
	samplerDesc.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
	pvrvk::Sampler samplerTrilinear = _deviceResources->device->createSampler(samplerDesc);

	// CREATE THE DESCRIPTOR SET LAYOUTS
	// Per Model Descriptor set layout
	pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
	// create the ubo descriptor set layout
	// static material ubo
	descSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	// static model ubo
	descSetInfo.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, pvrvk::ShaderStageFlags::e_VERTEX_BIT);

	// no texture sampler layout
	_deviceResources->noSamplerLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);

	// Single texture sampler layout
	descSetInfo.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	_deviceResources->oneSamplerLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);

	// Two textures sampler layout
	descSetInfo.setBinding(3, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	_deviceResources->twoSamplerLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);

	// Three textures sampler layout
	descSetInfo.setBinding(4, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	_deviceResources->threeSamplerLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);

	// Four textures sampler layout (for GBuffer rendering)
	descSetInfo.setBinding(5, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	_deviceResources->fourSamplerLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);

	// create the pipeline layouts
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

	pipeLayoutInfo.setDescSetLayout(0, _deviceResources->staticSceneLayout);

	pipeLayoutInfo.setDescSetLayout(1, _deviceResources->noSamplerLayout);
	_deviceResources->pipeLayoutNoSamplers = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	pipeLayoutInfo.setDescSetLayout(1, _deviceResources->oneSamplerLayout);
	_deviceResources->pipeLayoutOneSampler = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	pipeLayoutInfo.setDescSetLayout(1, _deviceResources->twoSamplerLayout);
	_deviceResources->pipeLayoutTwoSamplers = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	pipeLayoutInfo.setDescSetLayout(1, _deviceResources->threeSamplerLayout);
	_deviceResources->pipeLayoutThreeSamplers = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	pipeLayoutInfo.setDescSetLayout(1, _deviceResources->fourSamplerLayout);
	_deviceResources->pipeLayoutFourSamplers = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	// CREATE DESCRIPTOR SETS FOR EACH MATERIAL
	_deviceResources->materials.resize(_mainScene->getNumMaterials());
	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;
	for (uint32_t i = 0; i < _mainScene->getNumMaterials(); ++i)
	{
		_deviceResources->materials[i].materialDescriptorSet.resize(_numSwapImages);
		// get the current material
		const pvr::assets::Model::Material& material = _mainScene->getMaterial(i);
		// get material properties
		_deviceResources->materials[i].specularStrength = material.defaultSemantics().getShininess();
		_deviceResources->materials[i].diffuseColor = material.defaultSemantics().getDiffuse();
		pvrvk::ImageView diffuseMap;
		uint32_t numTextures = 0;
		pvrvk::ImageView bumpMap;
		if (material.defaultSemantics().getDiffuseTextureIndex() != static_cast<uint32_t>(-1))
		{
			// Load the diffuse texture map
			diffuseMap = pvr::utils::loadAndUploadImageAndView(_deviceResources->device,
				_mainScene->getTexture(material.defaultSemantics().getDiffuseTextureIndex()).getName().c_str(), true, uploadCmd, *this, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
				pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
			++numTextures;
		}
		if (material.defaultSemantics().getBumpMapTextureIndex() != static_cast<uint32_t>(-1))
		{
			// Load the bump map
			bumpMap = pvr::utils::loadAndUploadImageAndView(_deviceResources->device,
				_mainScene->getTexture(material.defaultSemantics().getBumpMapTextureIndex()).getName().c_str(), true, uploadCmd, *this, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
				pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);

			++numTextures;
		}
		for (uint32_t j = 0; j < _numSwapImages; ++j)
		{
			// based on the number of textures select the correct descriptor set
			switch (numTextures)
			{
			case 0: _deviceResources->materials[i].materialDescriptorSet[j] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->noSamplerLayout); break;
			case 1: _deviceResources->materials[i].materialDescriptorSet[j] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->oneSamplerLayout); break;
			case 2: _deviceResources->materials[i].materialDescriptorSet[j] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->twoSamplerLayout); break;
			case 3: _deviceResources->materials[i].materialDescriptorSet[j] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->threeSamplerLayout); break;
			case 4: _deviceResources->materials[i].materialDescriptorSet[j] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->fourSamplerLayout); break;
			default: break;
			}

			writeDescSets.push_back(
				pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->materials[i].materialDescriptorSet[j], 0)
					.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->modelMaterialBuffer, 0, _deviceResources->modelMaterialBufferView.getDynamicSliceSize())));

			writeDescSets.push_back(
				pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->materials[i].materialDescriptorSet[j], 1)
					.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->modelMatrixBuffer, 0, _deviceResources->modelMatrixBufferView.getDynamicSliceSize())));

			if (diffuseMap)
			{
				writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->materials[i].materialDescriptorSet[j], 2)
											.setImageInfo(0, pvrvk::DescriptorImageInfo(diffuseMap, samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
			}
			if (bumpMap)
			{
				writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->materials[i].materialDescriptorSet[j], 3)
											.setImageInfo(0, pvrvk::DescriptorImageInfo(bumpMap, samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
			}
		}
	}
	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

/// <summary>Creates model pipelines.</summary>
void VulkanDeferredShading::createModelPipelines()
{
	pvrvk::GraphicsPipelineCreateInfo renderGBufferPipelineCreateInfo;
	renderGBufferPipelineCreateInfo.viewport.setViewportAndScissor(0,
		pvrvk::Viewport(
			0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().getWidth()), static_cast<float>(_deviceResources->swapchain->getDimension().getHeight())),
		pvrvk::Rect2D(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight()));
	// enable back face culling
	renderGBufferPipelineCreateInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);

	// set counter clockwise winding order for front faces
	renderGBufferPipelineCreateInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

	// enable depth testing
	renderGBufferPipelineCreateInfo.depthStencil.enableDepthTest(true);
	renderGBufferPipelineCreateInfo.depthStencil.enableDepthWrite(true);

	// set the blend state for the colour attachments
	pvrvk::PipelineColorBlendAttachmentState renderGBufferColorAttachment;
	// number of colour blend states must equal number of colour attachments for the subpass
	renderGBufferPipelineCreateInfo.colorBlend.setAttachmentState(0, renderGBufferColorAttachment);
	renderGBufferPipelineCreateInfo.colorBlend.setAttachmentState(1, renderGBufferColorAttachment);
	renderGBufferPipelineCreateInfo.colorBlend.setAttachmentState(2, renderGBufferColorAttachment);

	// load and create appropriate shaders
	renderGBufferPipelineCreateInfo.vertexShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::GBufferVertexShader)->readToEnd<uint32_t>())));

	renderGBufferPipelineCreateInfo.fragmentShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::GBufferFragmentShader)->readToEnd<uint32_t>())));

	// setup vertex inputs
	renderGBufferPipelineCreateInfo.vertexInput.clear();
	pvr::utils::populateInputAssemblyFromMesh(
		_mainScene->getMesh(MeshNodes::Satyr), vertexBindings, 4, renderGBufferPipelineCreateInfo.vertexInput, renderGBufferPipelineCreateInfo.inputAssembler);

	// renderpass/subpass
	renderGBufferPipelineCreateInfo.renderPass = _deviceResources->onScreenLocalMemoryRenderPass;
	renderGBufferPipelineCreateInfo.subpass = RenderPassSubpasses::GBuffer;

	// enable stencil testing
	pvrvk::StencilOpState stencilState;

	// only replace stencil buffer when the depth test passes
	stencilState.setFailOp(pvrvk::StencilOp::e_KEEP);
	stencilState.setDepthFailOp(pvrvk::StencilOp::e_KEEP);
	stencilState.setPassOp(pvrvk::StencilOp::e_REPLACE);
	stencilState.setCompareOp(pvrvk::CompareOp::e_ALWAYS);

	// set stencil reference to 1
	stencilState.setReference(1);

	// enable stencil writing
	stencilState.setWriteMask(0xFF);

	// enable the stencil tests
	renderGBufferPipelineCreateInfo.depthStencil.enableStencilTest(true);
	// set stencil states
	renderGBufferPipelineCreateInfo.depthStencil.setStencilFront(stencilState);
	renderGBufferPipelineCreateInfo.depthStencil.setStencilBack(stencilState);

	renderGBufferPipelineCreateInfo.pipelineLayout = _deviceResources->pipeLayoutTwoSamplers;
	_deviceResources->renderInfo.storeLocalMemoryPass.objects[MeshNodes::Satyr].pipeline =
		_deviceResources->device->createGraphicsPipeline(renderGBufferPipelineCreateInfo, _deviceResources->pipelineCache);

	// load and create appropriate shaders
	renderGBufferPipelineCreateInfo.vertexShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::GBufferFloorVertexShader)->readToEnd<uint32_t>())));

	renderGBufferPipelineCreateInfo.fragmentShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::GBufferFloorFragmentShader)->readToEnd<uint32_t>())));

	// setup vertex inputs
	renderGBufferPipelineCreateInfo.vertexInput.clear();
	pvr::utils::populateInputAssemblyFromMesh(
		_mainScene->getMesh(MeshNodes::Floor), floorVertexBindings, 3, renderGBufferPipelineCreateInfo.vertexInput, renderGBufferPipelineCreateInfo.inputAssembler);

	renderGBufferPipelineCreateInfo.pipelineLayout = _deviceResources->pipeLayoutOneSampler;
	_deviceResources->renderInfo.storeLocalMemoryPass.objects[MeshNodes::Floor].pipeline =
		_deviceResources->device->createGraphicsPipeline(renderGBufferPipelineCreateInfo, _deviceResources->pipelineCache);
}

/// <summary>Creates directional lighting pipeline.</summary>
void VulkanDeferredShading::createDirectionalLightingPipeline()
{
	// DIRECTIONAL LIGHTING - A full-screen quad that will apply any global (ambient/directional) lighting
	// disable the depth write as we do not want to modify the depth buffer while rendering directional lights

	pvrvk::GraphicsPipelineCreateInfo renderDirectionalLightingPipelineInfo;
	renderDirectionalLightingPipelineInfo.viewport.setViewportAndScissor(0,
		pvrvk::Viewport(
			0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().getWidth()), static_cast<float>(_deviceResources->swapchain->getDimension().getHeight())),
		pvrvk::Rect2D(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight()));
	// enable back face culling
	renderDirectionalLightingPipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);

	// set counter clockwise winding order for front faces
	renderDirectionalLightingPipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

	// Make use of the stencil buffer contents to only shade pixels where actual geometry is located.
	pvrvk::StencilOpState stencilState;

	// keep the stencil states the same as the previous pass. These aren't important to this pass.
	stencilState.setFailOp(pvrvk::StencilOp::e_KEEP);
	stencilState.setDepthFailOp(pvrvk::StencilOp::e_KEEP);
	stencilState.setPassOp(pvrvk::StencilOp::e_REPLACE);

	// if the stencil is equal to the value specified then stencil passes
	stencilState.setCompareOp(pvrvk::CompareOp::e_EQUAL);

	// if for the current fragment the stencil has been filled then there is geometry present
	// and directional lighting calculations should be carried out
	stencilState.setReference(1);

	stencilState.setWriteMask(0x00);

	// disable depth writing and depth testing
	renderDirectionalLightingPipelineInfo.depthStencil.enableDepthWrite(false);
	renderDirectionalLightingPipelineInfo.depthStencil.enableDepthTest(false);

	// enable stencil testing
	renderDirectionalLightingPipelineInfo.depthStencil.enableStencilTest(true);
	renderDirectionalLightingPipelineInfo.depthStencil.setStencilFront(stencilState);
	renderDirectionalLightingPipelineInfo.depthStencil.setStencilBack(stencilState);

	// set the blend state for the colour attachments
	renderDirectionalLightingPipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

	// load and create appropriate shaders
	renderDirectionalLightingPipelineInfo.vertexShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::AttributelessVertexShader)->readToEnd<uint32_t>())));
	renderDirectionalLightingPipelineInfo.fragmentShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::DirectionalLightingFragmentShader)->readToEnd<uint32_t>())));

	// setup vertex inputs
	renderDirectionalLightingPipelineInfo.vertexInput.clear();
	renderDirectionalLightingPipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);

	renderDirectionalLightingPipelineInfo.pipelineLayout = _deviceResources->directionalLightingPipelineLayout;

	// renderpass/subpass
	renderDirectionalLightingPipelineInfo.renderPass = _deviceResources->onScreenLocalMemoryRenderPass;
	renderDirectionalLightingPipelineInfo.subpass = RenderPassSubpasses::Lighting;

	_deviceResources->renderInfo.directionalLightPass.pipeline =
		_deviceResources->device->createGraphicsPipeline(renderDirectionalLightingPipelineInfo, _deviceResources->pipelineCache);
}

/// <summary>Creates point lighting stencil pass pipeline.</summary>
void VulkanDeferredShading::createPointLightStencilPipeline()
{
	// POINT LIGHTS GEOMETRY STENCIL PASS
	// Render the front face of each light volume
	// Z function is set as Less/Equal
	// Z test passes will leave the stencil as 0 i.e. the front of the light is in front of all geometry in the current pixel
	//    This is the condition we want for determining whether the geometry can be affected by the point lights
	// Z test fails will increment the stencil to 1. i.e. the front of the light is behind all of the geometry in the current pixel
	//    Under this condition the current pixel cannot be affected by the current point light as the geometry is in front of the front of the point light
	pvrvk::GraphicsPipelineCreateInfo pointLightStencilPipelineCreateInfo;
	pointLightStencilPipelineCreateInfo.viewport.setViewportAndScissor(0,
		pvrvk::Viewport(
			0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().getWidth()), static_cast<float>(_deviceResources->swapchain->getDimension().getHeight())),
		pvrvk::Rect2D(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight()));
	pvrvk::PipelineColorBlendAttachmentState stencilPassColorAttachmentBlendState;
	stencilPassColorAttachmentBlendState.setColorWriteMask(static_cast<pvrvk::ColorComponentFlags>(0));

	// set the blend state for the colour attachments
	pointLightStencilPipelineCreateInfo.colorBlend.setAttachmentState(0, stencilPassColorAttachmentBlendState);

	// enable back face culling
	pointLightStencilPipelineCreateInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);

	// set counter clockwise winding order for front faces
	pointLightStencilPipelineCreateInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

	// disable depth write. This pass reuses previously written depth buffer
	pointLightStencilPipelineCreateInfo.depthStencil.enableDepthTest(true);
	pointLightStencilPipelineCreateInfo.depthStencil.enableDepthWrite(false);

	// set depth comparison to less/equal
	pointLightStencilPipelineCreateInfo.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS_OR_EQUAL).enableStencilTest(true);

	// load and create appropriate shaders
	pointLightStencilPipelineCreateInfo.vertexShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::PointLightPass1VertexShader)->readToEnd<uint32_t>())));
	pointLightStencilPipelineCreateInfo.fragmentShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::PointLightPass1FragmentShader)->readToEnd<uint32_t>())));

	// setup vertex inputs
	pointLightStencilPipelineCreateInfo.vertexInput.clear();
	pvr::utils::populateInputAssemblyFromMesh(_pointLightModel->getMesh(LightNodes::PointLightMeshNode), pointLightVertexBindings, 1,
		pointLightStencilPipelineCreateInfo.vertexInput, pointLightStencilPipelineCreateInfo.inputAssembler);

	pvrvk::StencilOpState stencilState;
	stencilState.setCompareOp(pvrvk::CompareOp::e_ALWAYS);
	// keep current value if the stencil test fails
	stencilState.setFailOp(pvrvk::StencilOp::e_KEEP);
	// if the depth test fails then increment wrap
	stencilState.setDepthFailOp(pvrvk::StencilOp::e_INCREMENT_AND_WRAP);
	stencilState.setPassOp(pvrvk::StencilOp::e_KEEP);

	stencilState.setReference(0);

	// set stencil state for the front face of the light sources
	pointLightStencilPipelineCreateInfo.depthStencil.setStencilFront(stencilState);

	// set stencil state for the back face of the light sources
	stencilState.setDepthFailOp(pvrvk::StencilOp::e_KEEP);
	pointLightStencilPipelineCreateInfo.depthStencil.setStencilBack(stencilState);

	// renderpass/subpass
	pointLightStencilPipelineCreateInfo.renderPass = _deviceResources->onScreenLocalMemoryRenderPass;
	pointLightStencilPipelineCreateInfo.subpass = RenderPassSubpasses::Lighting;

	pointLightStencilPipelineCreateInfo.pipelineLayout = _deviceResources->pointLightGeometryStencilPipelineLayout;

	_deviceResources->renderInfo.pointLightGeometryStencilPass.pipeline =
		_deviceResources->device->createGraphicsPipeline(pointLightStencilPipelineCreateInfo, _deviceResources->pipelineCache);
}

/// <summary>Creates point lighting proxy pass pipeline.</summary>
void VulkanDeferredShading::createPointLightProxyPipeline()
{
	// POINT LIGHTS PROXIES - Actually light the pixels touched by a point light.
	// Render the back faces of the light volumes
	// Z function is set as Greater/Equal
	// Z test passes signify that there is geometry in front of the back face of the light volume i.e. for the current pixel there is
	// some geometry in front of the back face of the light volume
	// Stencil function is Equal i.e. the stencil reference is set to 0
	// Stencil passes signify that for the current pixel there exists a front face of a light volume in front of the current geometry
	// Point light calculations occur every time a pixel passes both the stencil AND Z test
	pvrvk::GraphicsPipelineCreateInfo pointLightProxyPipelineCreateInfo;
	pointLightProxyPipelineCreateInfo.viewport.setViewportAndScissor(0,
		pvrvk::Viewport(
			0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().getWidth()), static_cast<float>(_deviceResources->swapchain->getDimension().getHeight())),
		pvrvk::Rect2D(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight()));

	// enable front face culling - cull the front faces of the light sources
	pointLightProxyPipelineCreateInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);

	// set counter clockwise winding order for front faces
	pointLightProxyPipelineCreateInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

	// enable stencil testing
	pointLightProxyPipelineCreateInfo.depthStencil.enableStencilTest(true);

	// enable depth testing
	pointLightProxyPipelineCreateInfo.depthStencil.enableDepthTest(true);
	pointLightProxyPipelineCreateInfo.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_GREATER_OR_EQUAL);
	// disable depth writes
	pointLightProxyPipelineCreateInfo.depthStencil.enableDepthWrite(false);

	// enable blending
	// blend lighting on top of existing directional lighting
	pvrvk::PipelineColorBlendAttachmentState blendConfig;
	blendConfig.setBlendEnable(true);
	blendConfig.setSrcColorBlendFactor(pvrvk::BlendFactor::e_ONE);
	blendConfig.setSrcAlphaBlendFactor(pvrvk::BlendFactor::e_ONE);
	blendConfig.setDstColorBlendFactor(pvrvk::BlendFactor::e_ONE);
	blendConfig.setDstAlphaBlendFactor(pvrvk::BlendFactor::e_ONE);
	pointLightProxyPipelineCreateInfo.colorBlend.setAttachmentState(0, blendConfig);

	// load and create appropriate shaders
	pointLightProxyPipelineCreateInfo.vertexShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::PointLightPass2VertexShader)->readToEnd<uint32_t>())));

	pointLightProxyPipelineCreateInfo.fragmentShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::PointLightPass2FragmentShader)->readToEnd<uint32_t>())));

	// setup vertex states
	pointLightProxyPipelineCreateInfo.vertexInput.clear();
	pvr::utils::populateInputAssemblyFromMesh(_pointLightModel->getMesh(LightNodes::PointLightMeshNode), pointLightVertexBindings, 1, pointLightProxyPipelineCreateInfo.vertexInput,
		pointLightProxyPipelineCreateInfo.inputAssembler);

	// if stencil state equals 0 then the lighting should take place as there is geometry inside the point lights area
	pvrvk::StencilOpState stencilState;
	stencilState.setCompareOp(pvrvk::CompareOp::e_ALWAYS);
	stencilState.setReference(0);

	pointLightProxyPipelineCreateInfo.depthStencil.setStencilFront(stencilState);
	pointLightProxyPipelineCreateInfo.depthStencil.setStencilBack(stencilState);

	// renderpass/subpass
	pointLightProxyPipelineCreateInfo.renderPass = _deviceResources->onScreenLocalMemoryRenderPass;
	pointLightProxyPipelineCreateInfo.subpass = RenderPassSubpasses::Lighting;

	pointLightProxyPipelineCreateInfo.pipelineLayout = _deviceResources->pointLightProxyPipelineLayout;

	_deviceResources->renderInfo.pointLightProxyPass.pipeline = _deviceResources->device->createGraphicsPipeline(pointLightProxyPipelineCreateInfo, _deviceResources->pipelineCache);
}

/// <summary>Creates point lighting source pass pipeline.</summary>
void VulkanDeferredShading::createPointLightSourcePipeline()
{
	// LIGHT SOURCES : Rendering the "will-o-wisps" that are the sources of the light
	pvrvk::GraphicsPipelineCreateInfo pointLightSourcePipelineCreateInfo;
	pointLightSourcePipelineCreateInfo.viewport.setViewportAndScissor(0,
		pvrvk::Viewport(
			0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().getWidth()), static_cast<float>(_deviceResources->swapchain->getDimension().getHeight())),
		pvrvk::Rect2D(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight()));
	// enable back face culling
	pointLightSourcePipelineCreateInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);

	// set counter clockwise winding order for front faces
	pointLightSourcePipelineCreateInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

	// disable stencil testing
	pointLightSourcePipelineCreateInfo.depthStencil.enableStencilTest(false);

	// enable depth testing
	pointLightSourcePipelineCreateInfo.depthStencil.enableDepthTest(true);
	pointLightSourcePipelineCreateInfo.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS_OR_EQUAL);
	pointLightSourcePipelineCreateInfo.depthStencil.enableDepthWrite(true);

	// enable blending
	pvrvk::PipelineColorBlendAttachmentState colorAttachment;
	colorAttachment.setBlendEnable(true);
	colorAttachment.setSrcColorBlendFactor(pvrvk::BlendFactor::e_ONE);
	colorAttachment.setSrcAlphaBlendFactor(pvrvk::BlendFactor::e_ONE);
	colorAttachment.setDstColorBlendFactor(pvrvk::BlendFactor::e_ONE);
	colorAttachment.setDstAlphaBlendFactor(pvrvk::BlendFactor::e_ONE);
	pointLightSourcePipelineCreateInfo.colorBlend.setAttachmentState(0, colorAttachment);

	// load and create appropriate shaders
	pointLightSourcePipelineCreateInfo.vertexShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::PointLightPass3VertexShader)->readToEnd<uint32_t>())));

	pointLightSourcePipelineCreateInfo.fragmentShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::PointLightPass3FragmentShader)->readToEnd<uint32_t>())));

	// setup vertex states
	pointLightSourcePipelineCreateInfo.vertexInput.clear();
	pvr::utils::populateInputAssemblyFromMesh(_pointLightModel->getMesh(LightNodes::PointLightMeshNode), pointLightVertexBindings, 1,
		pointLightSourcePipelineCreateInfo.vertexInput, pointLightSourcePipelineCreateInfo.inputAssembler);

	// renderpass/subpass
	pointLightSourcePipelineCreateInfo.renderPass = _deviceResources->onScreenLocalMemoryRenderPass;
	pointLightSourcePipelineCreateInfo.subpass = RenderPassSubpasses::Lighting;

	pointLightSourcePipelineCreateInfo.pipelineLayout = _deviceResources->pointLightSourcePipelineLayout;

	_deviceResources->renderInfo.pointLightSourcesPass.pipeline = _deviceResources->device->createGraphicsPipeline(pointLightSourcePipelineCreateInfo, _deviceResources->pipelineCache);
}

/// <summary>Create the pipelines for this example.</summary>
void VulkanDeferredShading::createPipelines()
{
	createModelPipelines();
	createDirectionalLightingPipeline();
	createPointLightStencilPipeline();
	createPointLightProxyPipeline();
	createPointLightSourcePipeline();
}

/// <summary>Create the renderpass using local memory for this example.</summary>
void VulkanDeferredShading::createFramebufferAndRenderPass()
{
	pvrvk::RenderPassCreateInfo renderPassInfo;

	// On-Screen attachment
	renderPassInfo.setAttachmentDescription(0,
		pvrvk::AttachmentDescription::createColorDescription(_deviceResources->swapchain->getImageFormat(), pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_PRESENT_SRC_KHR,
			pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT));

	pvrvk::Format normalFormat = pvrvk::Format::e_B10G11R11_UFLOAT_PACK32;
	pvrvk::FormatProperties prop = _deviceResources->instance->getPhysicalDevice(0)->getFormatProperties(normalFormat);
	if ((prop.getOptimalTilingFeatures() & pvrvk::FormatFeatureFlags::e_COLOR_ATTACHMENT_BIT) == 0) { normalFormat = pvrvk::Format::e_R16G16B16A16_SFLOAT; }

	Log(LogLevel::Information, "Using a format of %s for the normals attachment\n", pvrvk::to_string(normalFormat).c_str());

	const pvrvk::Format renderpassStorageFormats[FramebufferGBufferAttachments::Count] = {
		pvrvk::Format::e_R8G8B8A8_UNORM, // albedo
		normalFormat, // normal
		pvrvk::Format::e_R16_SFLOAT, // depth attachment
	};

	renderPassInfo.setAttachmentDescription(1,
		pvrvk::AttachmentDescription::createColorDescription(renderpassStorageFormats[FramebufferGBufferAttachments::Albedo], pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE, pvrvk::SampleCountFlags::e_1_BIT));

	renderPassInfo.setAttachmentDescription(2,
		pvrvk::AttachmentDescription::createColorDescription(renderpassStorageFormats[FramebufferGBufferAttachments::Normal], pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE, pvrvk::SampleCountFlags::e_1_BIT));

	renderPassInfo.setAttachmentDescription(3,
		pvrvk::AttachmentDescription::createColorDescription(renderpassStorageFormats[FramebufferGBufferAttachments::Depth], pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE, pvrvk::SampleCountFlags::e_1_BIT));

	renderPassInfo.setAttachmentDescription(4,
		pvrvk::AttachmentDescription::createDepthStencilDescription(_deviceResources->depthStencilImages[0]->getImage()->getFormat(), pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE, pvrvk::AttachmentLoadOp::e_CLEAR,
			pvrvk::AttachmentStoreOp::e_DONT_CARE, pvrvk::SampleCountFlags::e_1_BIT));

	// Create on-screen-renderpass/framebuffer with its subpasses
	pvrvk::SubpassDescription localMemorySubpasses[RenderPassSubpasses::NumberOfSubpasses];

	// GBuffer subpass
	localMemorySubpasses[RenderPassSubpasses::GBuffer].setColorAttachmentReference(0, pvrvk::AttachmentReference(1, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	localMemorySubpasses[RenderPassSubpasses::GBuffer].setColorAttachmentReference(1, pvrvk::AttachmentReference(2, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	localMemorySubpasses[RenderPassSubpasses::GBuffer].setColorAttachmentReference(2, pvrvk::AttachmentReference(3, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	localMemorySubpasses[RenderPassSubpasses::GBuffer].setDepthStencilAttachmentReference(pvrvk::AttachmentReference(4, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
	localMemorySubpasses[RenderPassSubpasses::GBuffer].setPreserveAttachmentReference(0, 0);

	// Main scene lighting
	localMemorySubpasses[RenderPassSubpasses::Lighting].setInputAttachmentReference(0, pvrvk::AttachmentReference(1, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	localMemorySubpasses[RenderPassSubpasses::Lighting].setInputAttachmentReference(1, pvrvk::AttachmentReference(2, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	localMemorySubpasses[RenderPassSubpasses::Lighting].setInputAttachmentReference(2, pvrvk::AttachmentReference(3, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	localMemorySubpasses[RenderPassSubpasses::Lighting].setDepthStencilAttachmentReference(pvrvk::AttachmentReference(4, pvrvk::ImageLayout::e_DEPTH_STENCIL_READ_ONLY_OPTIMAL));
	localMemorySubpasses[RenderPassSubpasses::Lighting].setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));

	// add subpasses to the renderpass
	renderPassInfo.setSubpass(RenderPassSubpasses::GBuffer, localMemorySubpasses[RenderPassSubpasses::GBuffer]);
	renderPassInfo.setSubpass(RenderPassSubpasses::Lighting, localMemorySubpasses[RenderPassSubpasses::Lighting]);

	// add the sub pass dependency between sub passes
	pvrvk::SubpassDependency subpassDependency;
	subpassDependency.setSrcStageMask(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT | pvrvk::PipelineStageFlags::e_LATE_FRAGMENT_TESTS_BIT);
	subpassDependency.setDstStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT | pvrvk::PipelineStageFlags::e_EARLY_FRAGMENT_TESTS_BIT);

	subpassDependency.setSrcAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT | pvrvk::AccessFlags::e_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
	subpassDependency.setDstAccessMask(pvrvk::AccessFlags::e_INPUT_ATTACHMENT_READ_BIT | pvrvk::AccessFlags::e_DEPTH_STENCIL_ATTACHMENT_READ_BIT);

	subpassDependency.setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

	// GBuffer -> Directional Lighting
	subpassDependency.setSrcSubpass(RenderPassSubpasses::GBuffer);
	subpassDependency.setDstSubpass(RenderPassSubpasses::Lighting);
	renderPassInfo.addSubpassDependency(subpassDependency);

	// Add external subpass dependencies to avoid the overly cautious implicit subpass dependencies
	pvrvk::SubpassDependency externalDependencies[2];
	externalDependencies[0] = pvrvk::SubpassDependency(pvrvk::SubpassExternal, RenderPassSubpasses::GBuffer, pvrvk::PipelineStageFlags::e_BOTTOM_OF_PIPE_BIT,
		pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::AccessFlags::e_NONE,
		pvrvk::AccessFlags::e_COLOR_ATTACHMENT_READ_BIT | pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::DependencyFlags::e_BY_REGION_BIT);
	externalDependencies[1] = pvrvk::SubpassDependency(RenderPassSubpasses::Lighting, pvrvk::SubpassExternal, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
		pvrvk::PipelineStageFlags::e_BOTTOM_OF_PIPE_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_READ_BIT | pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT,
		pvrvk::AccessFlags::e_NONE, pvrvk::DependencyFlags::e_BY_REGION_BIT);

	renderPassInfo.addSubpassDependency(externalDependencies[0]);
	renderPassInfo.addSubpassDependency(externalDependencies[1]);

	// Create the renderpass
	_deviceResources->onScreenLocalMemoryRenderPass = _deviceResources->device->createRenderPass(renderPassInfo);

	// create and add the transient framebuffer attachments used as colour/input attachments
	const pvrvk::Extent3D& dimension = pvrvk::Extent3D(_deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight(), 1u);
	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		pvrvk::FramebufferCreateInfo onScreenFramebufferCreateInfo;
		onScreenFramebufferCreateInfo.setAttachment(0, _deviceResources->swapchain->getImageView(i));

		// Allocate the render targets
		for (uint32_t currentIndex = 0; currentIndex < FramebufferGBufferAttachments::Count; ++currentIndex)
		{
			pvrvk::Image transientColorAttachmentTexture = pvr::utils::createImage(_deviceResources->device,
				pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, renderpassStorageFormats[currentIndex], dimension,
					pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_INPUT_ATTACHMENT_BIT),
				pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT,
				_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);

			_deviceResources->framebufferGbufferImages[currentIndex].add(_deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(transientColorAttachmentTexture)));
			onScreenFramebufferCreateInfo.setAttachment(currentIndex + 1, _deviceResources->framebufferGbufferImages[currentIndex][i]);
		}
		onScreenFramebufferCreateInfo.setAttachment(FramebufferGBufferAttachments::Count + 1u, _deviceResources->depthStencilImages[i]);
		onScreenFramebufferCreateInfo.setDimensions(_deviceResources->swapchain->getDimension());
		onScreenFramebufferCreateInfo.setRenderPass(_deviceResources->onScreenLocalMemoryRenderPass);
		_deviceResources->onScreenLocalMemoryFramebuffer[i] = _deviceResources->device->createFramebuffer(onScreenFramebufferCreateInfo);
		_deviceResources->onScreenFramebufferCreateInfos.add(onScreenFramebufferCreateInfo);
	}
}

/// <summary>Loads the mesh data required for this example into vertex buffer objects.</summary>
/// <returns>Return true if no error occurred.</returns>
void VulkanDeferredShading::loadVbos(pvrvk::CommandBuffer& uploadCmd)
{
	bool requiresCommandBufferSubmission = false;

	pvr::utils::appendSingleBuffersFromModel(_deviceResources->device, *_mainScene, _deviceResources->sceneVbos, _deviceResources->sceneIbos, uploadCmd,
		requiresCommandBufferSubmission, _deviceResources->vmaAllocator);

	pvr::utils::createSingleBuffersFromMesh(_deviceResources->device, _pointLightModel->getMesh(LightNodes::PointLightMeshNode), _deviceResources->pointLightVbo,
		_deviceResources->pointLightIbo, uploadCmd, requiresCommandBufferSubmission, _deviceResources->vmaAllocator);
}

/// <summary>Creates the buffers used for rendering the models.</summary>
void VulkanDeferredShading::createModelBuffers()
{
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement(BufferEntryNames::PerModelMaterial::SpecularStrength, pvr::GpuDatatypes::Float);
		desc.addElement(BufferEntryNames::PerModelMaterial::DiffuseColor, pvr::GpuDatatypes::vec3);

		_deviceResources->modelMaterialBufferView.initDynamic(desc, _mainScene->getNumMeshNodes(), pvr::BufferUsageFlags::UniformBuffer,
			_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment());

		_deviceResources->modelMaterialBuffer = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->modelMaterialBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->modelMaterialBufferView.pointToMappedMemory(_deviceResources->modelMaterialBuffer->getDeviceMemory()->getMappedData());
	}

	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement(BufferEntryNames::PerModel::WorldViewProjectionMatrix, pvr::GpuDatatypes::mat4x4);
		desc.addElement(BufferEntryNames::PerModel::WorldViewMatrix, pvr::GpuDatatypes::mat4x4);
		desc.addElement(BufferEntryNames::PerModel::WorldViewITMatrix, pvr::GpuDatatypes::mat4x4);
		_deviceResources->modelMatrixBufferView.initDynamic(desc, _mainScene->getNumMeshNodes() * _numSwapImages, pvr::BufferUsageFlags::UniformBuffer,
			_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment());

		_deviceResources->modelMatrixBuffer = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->modelMatrixBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->modelMatrixBufferView.pointToMappedMemory(_deviceResources->modelMatrixBuffer->getDeviceMemory()->getMappedData());
	}
}

/// <summary>Creates the buffers used for rendering the directional lighting.</summary>
void VulkanDeferredShading::createDirectionalLightingBuffers()
{
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement(BufferEntryNames::PerDirectionalLight::LightIntensity, pvr::GpuDatatypes::vec4);
		desc.addElement(BufferEntryNames::PerDirectionalLight::AmbientLight, pvr::GpuDatatypes::vec4);

		_deviceResources->staticDirectionalLightBufferView.initDynamic(desc, _numberOfDirectionalLights, pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

		_deviceResources->staticDirectionalLightBuffer = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->staticDirectionalLightBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
			pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->staticDirectionalLightBufferView.pointToMappedMemory(_deviceResources->staticDirectionalLightBuffer->getDeviceMemory()->getMappedData());
	}

	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement(BufferEntryNames::PerDirectionalLight::LightViewDirection, pvr::GpuDatatypes::vec4);

		_deviceResources->dynamicDirectionalLightBufferView.initDynamic(desc, _numberOfDirectionalLights * _numSwapImages, pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

		_deviceResources->dynamicDirectionalLightBuffer = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->dynamicDirectionalLightBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
			pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->dynamicDirectionalLightBufferView.pointToMappedMemory(_deviceResources->dynamicDirectionalLightBuffer->getDeviceMemory()->getMappedData());
	}
}

/// <summary>Creates the buffers used for rendering the point lighting.</summary>
void VulkanDeferredShading::createPointLightBuffers()
{
	// create static point light buffers
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement(BufferEntryNames::PerPointLight::LightIntensity, pvr::GpuDatatypes::Float);
		desc.addElement(BufferEntryNames::PerPointLight::LightRadius, pvr::GpuDatatypes::Float);
		desc.addElement(BufferEntryNames::PerPointLight::LightColor, pvr::GpuDatatypes::vec4);
		desc.addElement(BufferEntryNames::PerPointLight::LightSourceColor, pvr::GpuDatatypes::vec4);

		_deviceResources->staticPointLightBufferView.initDynamic(desc, _numberOfPointLights, pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
		_deviceResources->staticPointLightBuffer = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->staticPointLightBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->staticPointLightBufferView.pointToMappedMemory(_deviceResources->staticPointLightBuffer->getDeviceMemory()->getMappedData());
	}

	// create point light buffers
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement(BufferEntryNames::PerPointLight::WorldViewProjectionMatrix, pvr::GpuDatatypes::mat4x4);
		desc.addElement(BufferEntryNames::PerPointLight::ProxyLightViewPosition, pvr::GpuDatatypes::vec4);
		desc.addElement(BufferEntryNames::PerPointLight::ProxyWorldViewProjectionMatrix, pvr::GpuDatatypes::mat4x4);
		desc.addElement(BufferEntryNames::PerPointLight::ProxyWorldViewMatrix, pvr::GpuDatatypes::mat4x4);

		_deviceResources->dynamicPointLightBufferView.initDynamic(desc, _numberOfPointLights * _numSwapImages, pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
		_deviceResources->dynamicPointLightBuffer = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->dynamicPointLightBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
			pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->dynamicPointLightBufferView.pointToMappedMemory(_deviceResources->dynamicPointLightBuffer->getDeviceMemory()->getMappedData());
	}
}

/// <summary>Creates the buffers used for rendering the lighting.</summary>
void VulkanDeferredShading::createLightingBuffers()
{
	// directional light sources
	createDirectionalLightingBuffers();

	// point light sources
	createPointLightBuffers();
}

/// <summary>Creates the scene wide buffer used throughout the demo.</summary>
void VulkanDeferredShading::createSceneWideBuffers()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::PerScene::FarClipDistance, pvr::GpuDatatypes::Float);

	_deviceResources->farClipDistanceBufferView.init(desc);
	_deviceResources->farClipDistanceBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->farClipDistanceBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

	_deviceResources->farClipDistanceBufferView.pointToMappedMemory(_deviceResources->farClipDistanceBuffer->getDeviceMemory()->getMappedData());
}

/// <summary>Creates the buffers used throughout the demo.</summary>
void VulkanDeferredShading::createBuffers()
{
	// create scene wide buffer
	createSceneWideBuffers();

	// create model buffers
	createModelBuffers();

	// create lighting buffers
	createLightingBuffers();
}

/// <summary>Upload the static data to the buffers which do not change per frame.</summary>
void VulkanDeferredShading::uploadStaticSceneData()
{
	// static scene properties buffer
	_farClipDistance = _mainScene->getCamera(0).getFar();
	_deviceResources->farClipDistanceBufferView.getElementByName(BufferEntryNames::PerScene::FarClipDistance).setValue(_farClipDistance);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->farClipDistanceBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{ _deviceResources->farClipDistanceBuffer->getDeviceMemory()->flushRange(0, _deviceResources->farClipDistanceBufferView.getDynamicSliceSize()); }
}

/// <summary>Upload the static data to the buffers which do not change per frame.</summary>
void VulkanDeferredShading::uploadStaticModelData()
{
	// static model buffer
	for (uint32_t i = 0; i < _mainScene->getNumMeshNodes(); ++i)
	{
		_deviceResources->modelMaterialBufferView.getElementByName(BufferEntryNames::PerModelMaterial::SpecularStrength, 0, i).setValue(_deviceResources->materials[i].specularStrength);
		_deviceResources->modelMaterialBufferView.getElementByName(BufferEntryNames::PerModelMaterial::DiffuseColor, 0, i).setValue(_deviceResources->materials[i].diffuseColor);
	}

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->modelMaterialBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{ _deviceResources->modelMaterialBuffer->getDeviceMemory()->flushRange(0, _deviceResources->modelMaterialBufferView.getSize()); }
}

/// <summary>Upload the static data to the buffers which do not change per frame.</summary>
void VulkanDeferredShading::uploadStaticDirectionalLightData()
{
	// static directional lighting buffer
	for (uint32_t i = 0; i < _numberOfDirectionalLights; ++i)
	{
		_deviceResources->staticDirectionalLightBufferView.getElementByName(BufferEntryNames::PerDirectionalLight::LightIntensity, 0, i)
			.setValue(_deviceResources->renderInfo.directionalLightPass.lightProperties[i].lightIntensity);
		_deviceResources->staticDirectionalLightBufferView.getElementByName(BufferEntryNames::PerDirectionalLight::AmbientLight, 0, i)
			.setValue(_deviceResources->renderInfo.directionalLightPass.lightProperties[i].ambientLight);
	}

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->staticDirectionalLightBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{ _deviceResources->staticDirectionalLightBuffer->getDeviceMemory()->flushRange(0, _deviceResources->staticDirectionalLightBufferView.getSize()); }
}

/// <summary>Upload the static data to the buffers which do not change per frame.</summary>
void VulkanDeferredShading::uploadStaticPointLightData()
{
	// static point lighting buffer
	for (uint32_t i = 0; i < _numberOfPointLights; ++i)
	{
		_deviceResources->staticPointLightBufferView.getElementByName(BufferEntryNames::PerPointLight::LightIntensity, 0, i)
			.setValue(_deviceResources->renderInfo.pointLightPasses.lightProperties[i].lightIntensity);
		_deviceResources->staticPointLightBufferView.getElementByName(BufferEntryNames::PerPointLight::LightRadius, 0, i)
			.setValue(_deviceResources->renderInfo.pointLightPasses.lightProperties[i].lightRadius);
		_deviceResources->staticPointLightBufferView.getElementByName(BufferEntryNames::PerPointLight::LightColor, 0, i)
			.setValue(_deviceResources->renderInfo.pointLightPasses.lightProperties[i].lightColor);
		_deviceResources->staticPointLightBufferView.getElementByName(BufferEntryNames::PerPointLight::LightSourceColor, 0, i)
			.setValue(_deviceResources->renderInfo.pointLightPasses.lightProperties[i].lightSourceColor);
	}

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->staticPointLightBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{ _deviceResources->staticPointLightBuffer->getDeviceMemory()->flushRange(0, _deviceResources->staticPointLightBufferView.getSize()); }
}

/// <summary>Upload the static data to the buffers which do not change per frame.</summary>
void VulkanDeferredShading::uploadStaticData()
{
	uploadStaticSceneData();
	uploadStaticModelData();
	uploadStaticDirectionalLightData();
	uploadStaticPointLightData();
}

/// <summary>Update the CPU visible buffers containing dynamic data.</summary>
void VulkanDeferredShading::updateDynamicSceneData()
{
	RenderData& pass = _deviceResources->renderInfo;

	// update the model matrices
	for (uint32_t i = 0; i < _mainScene->getNumMeshNodes(); ++i)
	{
		uint32_t dynamicSlice = i + _swapchainIndex * _mainScene->getNumMeshNodes();

		const pvr::assets::Model::Node& node = _mainScene->getNode(i);
		pass.storeLocalMemoryPass.objects[i].world = _mainScene->getWorldMatrix(node.getObjectId());
		pass.storeLocalMemoryPass.objects[i].worldView = _viewMatrix * pass.storeLocalMemoryPass.objects[i].world;
		pass.storeLocalMemoryPass.objects[i].worldViewProj = _viewProjectionMatrix * pass.storeLocalMemoryPass.objects[i].world;
		pass.storeLocalMemoryPass.objects[i].worldViewIT4x4 = glm::inverseTranspose(pass.storeLocalMemoryPass.objects[i].worldView);

		_deviceResources->modelMatrixBufferView.getElementByName(BufferEntryNames::PerModel::WorldViewMatrix, 0, dynamicSlice).setValue(pass.storeLocalMemoryPass.objects[i].worldView);

		_deviceResources->modelMatrixBufferView.getElementByName(BufferEntryNames::PerModel::WorldViewProjectionMatrix, 0, dynamicSlice)
			.setValue(pass.storeLocalMemoryPass.objects[i].worldViewProj);

		_deviceResources->modelMatrixBufferView.getElementByName(BufferEntryNames::PerModel::WorldViewITMatrix, 0, dynamicSlice).setValue(pass.storeLocalMemoryPass.objects[i].worldViewIT4x4);
	}

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->modelMatrixBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->modelMatrixBuffer->getDeviceMemory()->flushRange(
			_deviceResources->modelMatrixBufferView.getDynamicSliceOffset(_swapchainIndex * _mainScene->getNumMeshNodes()),
			_deviceResources->modelMatrixBufferView.getDynamicSliceSize() * _mainScene->getNumMeshNodes());
	}

	uint32_t pointLight = 0u;
	uint32_t directionalLight = 0;

	// update the lighting data
	for (uint32_t i = 0; i < _mainScene->getNumLightNodes(); ++i)
	{
		const pvr::assets::Node& lightNode = _mainScene->getLightNode(i);
		const pvr::assets::Light& light = _mainScene->getLight(lightNode.getObjectId());
		switch (light.getType())
		{
		case pvr::assets::Light::Point: {
			if ((uint32_t)pointLight >= PointLightConfiguration::MaxScenePointLights) { continue; }

			const glm::mat4& transMtx = _mainScene->getWorldMatrix(_mainScene->getNodeIdFromLightNodeId(i));
			const glm::mat4& proxyScale = glm::scale(glm::vec3(PointLightConfiguration::PointLightMaxRadius));
			const glm::mat4 mWorldScale = transMtx * proxyScale;

			// POINT LIGHT GEOMETRY : The spheres that will be used for the stencil pass
			pass.pointLightPasses.lightProperties[pointLight].proxyWorldViewProjectionMatrix = _viewProjectionMatrix * mWorldScale;

			// POINT LIGHT PROXIES : The "drawcalls" that will perform the actual rendering
			pass.pointLightPasses.lightProperties[pointLight].proxyWorldViewMatrix = _viewMatrix * mWorldScale;
			pass.pointLightPasses.lightProperties[pointLight].proxyViewSpaceLightPosition = glm::vec4((_viewMatrix * transMtx)[3]); // Translation component of the view matrix

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
		pass.directionalLightPass.lightProperties[directionalLight].viewSpaceLightDirection = _viewMatrix * glm::vec4(1.f, -1.f, -.5f, 0.f);
		++directionalLight;
	}

	for (; pointLight < numSceneLights + PointLightConfiguration::NumProceduralPointLights; ++pointLight)
	{ updateProceduralPointLight(pass.pointLightPasses.initialData[pointLight], _deviceResources->renderInfo.pointLightPasses.lightProperties[pointLight], false); }
	{
		// directional Light data
		for (uint32_t i = 0; i < _numberOfDirectionalLights; ++i)
		{
			uint32_t dynamicSlice = i + _swapchainIndex * _numberOfDirectionalLights;
			_deviceResources->dynamicDirectionalLightBufferView.getElementByName(BufferEntryNames::PerDirectionalLight::LightViewDirection, 0, dynamicSlice)
				.setValue(_deviceResources->renderInfo.directionalLightPass.lightProperties[i].viewSpaceLightDirection);
		}

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(_deviceResources->dynamicDirectionalLightBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->dynamicDirectionalLightBuffer->getDeviceMemory()->flushRange(
				_deviceResources->dynamicDirectionalLightBufferView.getDynamicSliceOffset(_swapchainIndex * _numberOfDirectionalLights),
				_deviceResources->dynamicDirectionalLightBufferView.getDynamicSliceSize() * _numberOfDirectionalLights);
		}
	}

	{
		// dynamic point light data
		for (uint32_t i = 0; i < _numberOfPointLights; ++i)
		{
			uint32_t dynamicSlice = i + _swapchainIndex * _numberOfPointLights;
			_deviceResources->dynamicPointLightBufferView.getElementByName(BufferEntryNames::PerPointLight::ProxyWorldViewProjectionMatrix, 0, dynamicSlice)
				.setValue(_deviceResources->renderInfo.pointLightPasses.lightProperties[i].proxyWorldViewProjectionMatrix);

			_deviceResources->dynamicPointLightBufferView.getElementByName(BufferEntryNames::PerPointLight::ProxyWorldViewMatrix, 0, dynamicSlice)
				.setValue(_deviceResources->renderInfo.pointLightPasses.lightProperties[i].proxyWorldViewMatrix);

			_deviceResources->dynamicPointLightBufferView.getElementByName(BufferEntryNames::PerPointLight::ProxyLightViewPosition, 0, dynamicSlice)
				.setValue(_deviceResources->renderInfo.pointLightPasses.lightProperties[i].proxyViewSpaceLightPosition);

			_deviceResources->dynamicPointLightBufferView.getElementByName(BufferEntryNames::PerPointLight::WorldViewProjectionMatrix, 0, dynamicSlice)
				.setValue(_deviceResources->renderInfo.pointLightPasses.lightProperties[i].worldViewProjectionMatrix);
		}

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(_deviceResources->dynamicPointLightBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->dynamicPointLightBuffer->getDeviceMemory()->flushRange(
				_deviceResources->dynamicPointLightBufferView.getDynamicSliceOffset(_swapchainIndex * _numberOfPointLights),
				_deviceResources->dynamicPointLightBufferView.getDynamicSliceSize() * _numberOfPointLights);
		}
	}
}

/// <summary>Update the procedural point lights.</summary>
void VulkanDeferredShading::updateProceduralPointLight(PointLightPasses::InitialData& data, PointLightPasses::PointLightProperties& pointLightProperties, bool initial)
{
	if (initial)
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

	if (!initial && !_isPaused) // Skip for the first _frameNumber, as sometimes this moves the light too far...
	{
		float dt = static_cast<float>(std::min(getFrameTime(), uint64_t(30)));
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

	float x = sin(data.angle) * data.distance;
	float z = cos(data.angle) * data.distance;
	float y = data.height;

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
}

/// <summary>Updates animation variables and camera matrices.</summary>
void VulkanDeferredShading::updateAnimation()
{
	glm::vec3 vFrom, vTo, vUp;
	float fov;
	_mainScene->getCameraProperties(_cameraId, fov, vFrom, vTo, vUp);

	// Update camera matrices
	static float angle = 0;
	if (_animateCamera) { angle += getFrameTime() / 5000.f; }
	_viewMatrix = glm::lookAt(glm::vec3(sin(angle) * 100.f + vTo.x, vTo.y + 30., cos(angle) * 100.f + vTo.z), vTo, vUp);
	_viewProjectionMatrix = _projectionMatrix * _viewMatrix;
	_inverseViewMatrix = glm::inverse(_viewMatrix);
}

/// <summary>Records main command buffer.</summary>
void VulkanDeferredShading::recordMainCommandBuffer()
{
	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		_deviceResources->cmdBufferMain[i]->begin();

		pvrvk::Rect2D renderArea(0, 0, _windowWidth, _windowHeight);

		// specify a clear colour per attachment
		const uint32_t numClearValues = FramebufferGBufferAttachments::Count + 1u + 1u;

		pvrvk::ClearValue clearValues[numClearValues] = { pvrvk::ClearValue(0.0, 0.0, 0.0, 1.0f), pvrvk::ClearValue(0.0, 0.0, 0.0, 1.0f), pvrvk::ClearValue(0.0, 0.0, 0.0, 1.0f),
			pvrvk::ClearValue(0.0, 0.0, 0.0, 1.0f), pvrvk::ClearValue(1.f, 0u) };

		// begin the local memory renderpass
		_deviceResources->cmdBufferMain[i]->beginRenderPass(_deviceResources->onScreenLocalMemoryFramebuffer[i], renderArea, false, clearValues, numClearValues);

		// Render the models
		_deviceResources->cmdBufferMain[i]->executeCommands(_deviceResources->cmdBufferRenderToLocalMemory[i]);

		// Render lighting + ui render text
		_deviceResources->cmdBufferMain[i]->nextSubpass(pvrvk::SubpassContents::e_SECONDARY_COMMAND_BUFFERS);
		_deviceResources->cmdBufferMain[i]->executeCommands(_deviceResources->cmdBufferLighting[i]);

		_deviceResources->cmdBufferMain[i]->endRenderPass();
		_deviceResources->cmdBufferMain[i]->end();
	}
}

/// <summary>Initialise the static light properties.</summary>
void VulkanDeferredShading::initialiseStaticLightProperties()
{
	RenderData& pass = _deviceResources->renderInfo;

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
			pass.directionalLightPass.lightProperties[directionalLight].ambientLight = glm::vec4(0.f, 0.f, 0.f, 0.f);
			++directionalLight;
		}
		break;
		default: break;
		}
	}
	if (DirectionalLightConfiguration::AdditionalDirectionalLight)
	{
		pass.directionalLightPass.lightProperties[directionalLight].lightIntensity = glm::vec4(1, 1, 1, 1) * DirectionalLightConfiguration::DirectionalLightIntensity;
		pass.directionalLightPass.lightProperties[directionalLight].ambientLight = DirectionalLightConfiguration::AmbientLightColor;
		++directionalLight;
	}
}

/// <summary>Allocate memory for lighting data.</summary>
void VulkanDeferredShading::allocateLights()
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

	_deviceResources->renderInfo.directionalLightPass.lightProperties.resize(countDirectional);
	_deviceResources->renderInfo.pointLightPasses.lightProperties.resize(countPoint);
	_deviceResources->renderInfo.pointLightPasses.initialData.resize(countPoint);

	for (uint32_t i = countPoint - PointLightConfiguration::NumProceduralPointLights; i < countPoint; ++i)
	{ updateProceduralPointLight(_deviceResources->renderInfo.pointLightPasses.initialData[i], _deviceResources->renderInfo.pointLightPasses.lightProperties[i], true); }
}

/// <summary>Record all the secondary command buffers.</summary>
void VulkanDeferredShading::recordSecondaryCommandBuffers()
{
	pvrvk::Rect2D renderArea(0, 0, _framebufferWidth, _framebufferHeight);
	if ((_framebufferWidth != _windowWidth) || (_framebufferHeight != _windowHeight))
	{ renderArea = pvrvk::Rect2D(_viewportOffsets[0], _viewportOffsets[1], _framebufferWidth, _framebufferHeight); }

	pvrvk::ClearValue clearStenciLValue(pvrvk::ClearValue::createStencilClearValue(0));

	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		_deviceResources->cmdBufferRenderToLocalMemory[i]->begin(_deviceResources->onScreenLocalMemoryFramebuffer[i], RenderPassSubpasses::GBuffer);
		recordCommandBufferRenderGBuffer(_deviceResources->cmdBufferRenderToLocalMemory[i], i, RenderPassSubpasses::GBuffer);
		_deviceResources->cmdBufferRenderToLocalMemory[i]->end();

		_deviceResources->cmdBufferLighting[i]->begin(_deviceResources->onScreenLocalMemoryFramebuffer[i], RenderPassSubpasses::Lighting);
		recordCommandsDirectionalLights(_deviceResources->cmdBufferLighting[i], i);

		_deviceResources->cmdBufferLighting[i]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->scenePipelineLayout, 0u, _deviceResources->sceneDescriptorSet);

		const pvr::assets::Mesh& pointLightMesh = _pointLightModel->getMesh(LightNodes::PointLightMeshNode);

		// Bind the vertex and index buffer for the point light
		_deviceResources->cmdBufferLighting[i]->bindVertexBuffer(_deviceResources->pointLightVbo, 0, 0);
		_deviceResources->cmdBufferLighting[i]->bindIndexBuffer(_deviceResources->pointLightIbo, 0, pvr::utils::convertToPVRVk(pointLightMesh.getFaces().getDataType()));

		for (uint32_t j = 0; j < _numberOfPointLights; j++)
		{
			// clear stencil to 0's to make use of it again for point lights
			_deviceResources->cmdBufferLighting[i]->clearAttachment(
				pvrvk::ClearAttachment(pvrvk::ImageAspectFlags::e_STENCIL_BIT, FramebufferGBufferAttachments::Count + 1u, clearStenciLValue), pvrvk::ClearRect(renderArea));

			recordCommandsPointLightGeometryStencil(_deviceResources->cmdBufferLighting[i], i, RenderPassSubpasses::Lighting, j, pointLightMesh);
			recordCommandsPointLightProxy(_deviceResources->cmdBufferLighting[i], i, RenderPassSubpasses::Lighting, j, pointLightMesh);
		}
		recordCommandsPointLightSourceLighting(_deviceResources->cmdBufferLighting[i], i, RenderPassSubpasses::Lighting);

		recordCommandUIRenderer(_deviceResources->cmdBufferLighting[i]);
		_deviceResources->cmdBufferLighting[i]->end();
	}
}

/// <summary>Record rendering G-Buffer commands.</summary>
/// <param name="cmdBuffers">SecondaryCommandbuffer to record.</param>
/// <param name="swapChainIndex">Current swap chain index.</param>
/// <param name="subpass">Current sub pass.</param>
void VulkanDeferredShading::recordCommandBufferRenderGBuffer(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapChainIndex, uint32_t /*subpass*/)
{
	DrawGBuffer& pass = _deviceResources->renderInfo.storeLocalMemoryPass;

	cmdBuffers->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->scenePipelineLayout, 0u, _deviceResources->sceneDescriptorSet);

	for (uint32_t i = 0; i < _mainScene->getNumMeshNodes(); ++i)
	{
		cmdBuffers->bindPipeline(pass.objects[i].pipeline);

		const pvr::assets::Model::Node& node = _mainScene->getNode(i);
		const pvr::assets::Mesh& mesh = _mainScene->getMesh(node.getObjectId());

		const Material& material = _deviceResources->materials[node.getMaterialIndex()];

		uint32_t offsets[2];
		offsets[0] = _deviceResources->modelMaterialBufferView.getDynamicSliceOffset(i);
		offsets[1] = _deviceResources->modelMatrixBufferView.getDynamicSliceOffset(i + swapChainIndex * _mainScene->getNumMeshNodes());

		cmdBuffers->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, pass.objects[i].pipeline->getPipelineLayout(), 1u, material.materialDescriptorSet[swapChainIndex], offsets, 2u);

		cmdBuffers->bindVertexBuffer(_deviceResources->sceneVbos[node.getObjectId()], 0, 0);
		cmdBuffers->bindIndexBuffer(_deviceResources->sceneIbos[node.getObjectId()], 0, pvr::utils::convertToPVRVk(mesh.getFaces().getDataType()));
		cmdBuffers->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
	}
}

/// <summary>Record UIRenderer commands.</summary>
/// <param name="commandBuff">Commandbuffer to record.</param>
void VulkanDeferredShading::recordCommandUIRenderer(pvrvk::SecondaryCommandBuffer& commandBuff)
{
	_deviceResources->uiRenderer.beginRendering(commandBuff);
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.endRendering();
}

/// <summary>Record directional light draw commands</summary>
/// <param name="cmdBuffers">SecondaryCommandBuffer to record.</param>
/// <param name="swapChainIndex">Current swap chain index.</param>
/// <param name="subpass">Current sub pass.</param>
void VulkanDeferredShading::recordCommandsDirectionalLights(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapChainIndex)
{
	DrawDirectionalLight& directionalPass = _deviceResources->renderInfo.directionalLightPass;

	cmdBuffers->bindPipeline(directionalPass.pipeline);

	// keep the descriptor set bound even though for this pass we don't need it
	// avoids unbinding before rebinding in the next passes
	cmdBuffers->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->scenePipelineLayout, 0u, _deviceResources->sceneDescriptorSet);

	// Make use of the stencil buffer contents to only shade pixels where actual geometry is located.
	// Reset the stencil buffer to 0 at the same time to avoid the stencil clear operation afterwards.
	// bind the albedo and normal textures from the gbuffer
	for (uint32_t i = 0; i < _numberOfDirectionalLights; i++)
	{
		uint32_t offsets[2] = {};
		offsets[0] = _deviceResources->staticDirectionalLightBufferView.getDynamicSliceOffset(i);
		offsets[1] = _deviceResources->dynamicDirectionalLightBufferView.getDynamicSliceOffset(i + swapChainIndex * _numberOfDirectionalLights);

		cmdBuffers->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, directionalPass.pipeline->getPipelineLayout(), 0u,
			_deviceResources->directionalLightingDescriptorSets[swapChainIndex], offsets, 2u);

		// Draw a quad
		cmdBuffers->draw(0, 3);
	}
}

/// <summary>Record point light stencil commands.</summary>
/// <param name="cmdBuffers">SecondaryCommandBuffer to record.</param>
/// <param name="swapChainIndex">Current swap chain index.</param>
/// <param name="subpass">Current sub pass.</param>
void VulkanDeferredShading::recordCommandsPointLightGeometryStencil(
	pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapChainIndex, uint32_t /*subpass*/, const uint32_t pointLight, const pvr::assets::Mesh& pointLightMesh)
{
	PointLightGeometryStencil& pointGeometryStencilPass = _deviceResources->renderInfo.pointLightGeometryStencilPass;
	PointLightPasses& pointPasses = _deviceResources->renderInfo.pointLightPasses;

	// POINT LIGHTS: 1) Draw stencil to discard useless pixels
	cmdBuffers->bindPipeline(pointGeometryStencilPass.pipeline);

	uint32_t offsets[2] = {};
	offsets[0] = _deviceResources->staticPointLightBufferView.getDynamicSliceOffset(pointLight);
	offsets[1] = _deviceResources->dynamicPointLightBufferView.getDynamicSliceOffset(pointLight + swapChainIndex * static_cast<uint32_t>(pointPasses.lightProperties.size()));

	cmdBuffers->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pointGeometryStencilPass.pipeline->getPipelineLayout(), 1u,
		_deviceResources->pointLightGeometryStencilDescriptorSets[swapChainIndex], offsets, 2u);

	cmdBuffers->drawIndexed(0, pointLightMesh.getNumFaces() * 3, 0, 0, 1);
}

/// <summary>Record point light proxy commands.</summary>
/// <param name="cmdBuffers">SecondaryCommandBuffer to record.</param>
/// <param name="swapChainIndex">Current swap chain index.</param>
/// <param name="subpass">Current sub pass.</param>
void VulkanDeferredShading::recordCommandsPointLightProxy(
	pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapChainIndex, uint32_t /*subpass*/, const uint32_t pointLight, const pvr::assets::Mesh& pointLightMesh)
{
	DrawPointLightProxy& pointLightProxyPass = _deviceResources->renderInfo.pointLightProxyPass;
	PointLightPasses& pointPasses = _deviceResources->renderInfo.pointLightPasses;

	// Any of the geompointlightpass, lightsourcepointlightpass
	// or pointlightproxiepass's uniforms have the same number of elements
	if (pointPasses.lightProperties.empty()) { return; }

	cmdBuffers->bindPipeline(_deviceResources->renderInfo.pointLightProxyPass.pipeline);

	const uint32_t numberOfOffsets = 2;
	uint32_t offsets[numberOfOffsets] = {};
	offsets[0] = _deviceResources->staticPointLightBufferView.getDynamicSliceOffset(pointLight);
	offsets[1] = _deviceResources->dynamicPointLightBufferView.getDynamicSliceOffset(pointLight + swapChainIndex * static_cast<uint32_t>(pointPasses.lightProperties.size()));

	cmdBuffers->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pointLightProxyPass.pipeline->getPipelineLayout(), 1u,
		_deviceResources->pointLightProxyDescriptorSets[swapChainIndex], offsets, numberOfOffsets);

	cmdBuffers->bindDescriptorSet(
		pvrvk::PipelineBindPoint::e_GRAPHICS, pointLightProxyPass.pipeline->getPipelineLayout(), 2u, _deviceResources->pointLightProxyLocalMemoryDescriptorSets[swapChainIndex]);

	cmdBuffers->drawIndexed(0, pointLightMesh.getNumFaces() * 3, 0, 0, 1);
}

/// <summary>Record point light source commands.</summary>
/// <param name="cmdBuffers">SecondaryCommandBuffer to record.</param>
/// <param name="swapChainIndex">Current swap chain index.</param>
/// <param name="subpass">Current sub pass.</param>
void VulkanDeferredShading::recordCommandsPointLightSourceLighting(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapChainIndex, uint32_t subpass)
{
	(void)subpass;
	DrawPointLightSources& pointLightSourcePass = _deviceResources->renderInfo.pointLightSourcesPass;
	PointLightPasses& pointPasses = _deviceResources->renderInfo.pointLightPasses;

	const pvr::assets::Mesh& mesh = _pointLightModel->getMesh(LightNodes::PointLightMeshNode);

	// POINT LIGHTS: 3) Light sources
	cmdBuffers->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->scenePipelineLayout, 0u, _deviceResources->sceneDescriptorSet);

	cmdBuffers->bindPipeline(pointLightSourcePass.pipeline);
	cmdBuffers->bindVertexBuffer(_deviceResources->pointLightVbo, 0, 0);
	cmdBuffers->bindIndexBuffer(_deviceResources->pointLightIbo, 0, pvr::utils::convertToPVRVk(mesh.getFaces().getDataType()));

	for (uint32_t i = 0; i < pointPasses.lightProperties.size(); ++i)
	{
		const uint32_t numberOfOffsets = 2u;

		uint32_t offsets[numberOfOffsets] = {};
		offsets[0] = _deviceResources->staticPointLightBufferView.getDynamicSliceOffset(i);
		offsets[1] = _deviceResources->dynamicPointLightBufferView.getDynamicSliceOffset(i + swapChainIndex * static_cast<uint32_t>(pointPasses.lightProperties.size()));

		cmdBuffers->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pointLightSourcePass.pipeline->getPipelineLayout(), 1u,
			_deviceResources->pointLightSourceDescriptorSets[swapChainIndex], offsets, numberOfOffsets);

		cmdBuffers->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
	}
}

/// <summary>This function must be implemented by the user of the shell. The user should return its Shell object defining the
/// behaviour of the application.</summary>
/// <returns>Return an unique_ptr to a new Demo class,supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanDeferredShading>(); }
