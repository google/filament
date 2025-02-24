/*!
\brief The 3D navigation example demonstrates the entire process of creating a navigational map from raw XML data.
\file VulkanNavigation3D.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRCore/PVRCore.h"
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"
#define NAV_3D
#include "../../common/NavDataProcess.h"
#include "glm/gtx/vector_angle.hpp"
#include "PVRCore/math/AxisAlignedBox.h"
#include "PVRCore/cameras/TPSCamera.h"
namespace ColourUniforms {
enum ColourUniforms
{
	Clear,
	RoadArea,
	Motorway,
	Trunk,
	Primary,
	Secondary,
	Service,
	Other,
	Parking,
	Building,
	Outline,
	Count
};
}

namespace SetBinding {
enum SetBinding
{
	UBODynamic = 0,
	UBOStatic = 1,
	TextureSampler = 2
};
}
static const float CamHeight = .35f;
static uint32_t routeIndex = 0;

struct DeviceResources
{
	pvrvk::Instance instance;
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
	pvrvk::Device device;
	pvrvk::Swapchain swapchain;
	pvrvk::Queue queue;

	pvr::utils::vma::Allocator vmaAllocator;

	pvrvk::CommandPool commandPool;
	pvrvk::DescriptorPool descPool;

	struct Ubo
	{
		pvrvk::DescriptorSetLayout layout;
		pvr::utils::StructuredBufferView bufferView;
		pvrvk::Buffer buffer;
		pvrvk::DescriptorSet set;
	};

	Ubo uboDynamic;
	Ubo uboStatic;

	// Pipelines
	pvrvk::GraphicsPipeline roadPipe;
	pvrvk::GraphicsPipeline fillPipe;
	pvrvk::GraphicsPipeline outlinePipe;
	pvrvk::GraphicsPipeline planarShadowPipe;
	pvrvk::GraphicsPipeline buildingPipe;

	// Descriptor set for texture
	pvrvk::DescriptorSet imageSamplerDescSet;
	pvrvk::DescriptorSetLayout texDescSetLayout;
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
	pvrvk::PipelineLayout pipeLayout;
	pvrvk::PipelineCache pipelineCache;

	// Frame and primary command buffers
	pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;
	pvr::Multi<pvrvk::CommandBuffer> cbos;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> uiElementsCbo;
	pvr::Multi<pvrvk::Semaphore> acquireSemaphore;
	pvr::Multi<pvrvk::Semaphore> submitSemaphore;
	pvr::Multi<pvrvk::Fence> fencePerFrame;
	pvrvk::Sampler samplerTrilinear;

	// UI object for road text and icons.
	pvr::ui::UIRenderer uiRenderer;
	pvr::ui::Font font;
	pvr::ui::Text text[pvrvk::FrameworkCaps::MaxSwapChains];

	~DeviceResources()
	{
		if (device)
		{
			device->waitIdle();
			uint32_t l = swapchain->getSwapchainLength();
			for (uint32_t i = 0; i < l; ++i)
			{
				if (fencePerFrame[i]) fencePerFrame[i]->wait();
			}
		}
	}
};

struct TileRenderingResources
{
	pvrvk::Buffer vbo;
	pvrvk::Buffer ibo;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> secCbo;
};

// Alpha, luminance texture.
static const char* RoadTexFile = "Road.pvr";
static const char* MapFile = "map.osm";
static const char* FontFile = "font.pvr";

// Camera Settings
static const float CameraMoveSpeed = 2.f;
static const float CameraRotationSpeed = .5f;
static const float CamRotationTime = 10000.f;

inline float cameraRotationTimeInMs(float angleDeg) { return glm::abs(angleDeg / 360.f * CamRotationTime); }

/// <summary>Class implementing the pvr::Shell functions.</summary>
class VulkanNavigation3D : public pvr::Shell
{
public:
	// PVR shell functions
	pvr::Result initApplication() override;
	pvr::Result quitApplication() override;
	pvr::Result initView() override;
	pvr::Result releaseView() override;
	pvr::Result renderFrame() override;

private:
	uint32_t _frameId;
	std::unique_ptr<NavDataProcess> _OSMdata;

	// Graphics resources - buffers, samplers, descriptors.
	std::unique_ptr<DeviceResources> _deviceResources;

	std::vector<std::vector<std::unique_ptr<TileRenderingResources>>> _tileRenderingResources;

	// Uniforms
	glm::mat4 _viewProjMatrix;
	glm::mat4 _viewMatrix;

	glm::vec3 _lightDir;

	// Transformation variables
	glm::mat4 _perspectiveMatrix;

	pvr::math::ViewingFrustum _viewFrustum;
	glm::dvec2 _mapWorldDim;
	// Window variables
	uint32_t _windowWidth;
	uint32_t _windowHeight;

	// Map tile dimensions
	uint32_t _numRows;
	uint32_t _numCols;

	float _totalRouteDistance;
	float _keyFrameTime;
	std::string _currentRoad;

	glm::mat4 _shadowMatrix;

	glm::vec4 _clearColor;

	glm::vec4 _roadAreaColor;
	glm::vec4 _motorwayColor;
	glm::vec4 _trunkRoadColor;
	glm::vec4 _primaryRoadColor;
	glm::vec4 _secondaryRoadColor;
	glm::vec4 _serviceRoadColor;
	glm::vec4 _otherRoadColor;
	glm::vec4 _parkingColor;
	glm::vec4 _outlineColor;

	uint32_t _updateText[pvrvk::FrameworkCaps::MaxSwapChains] = {
		uint32_t(-1),
		uint32_t(-1),
		uint32_t(-1),
		uint32_t(-1),
	};

	void createBuffers(pvrvk::CommandBuffer& uploadCmd);
	bool createUbos();
	void initTextureAndSampler(pvrvk::CommandBuffer& uploadCmdBuffer);

	/*!*********************************************************************************************************************
	\param	tile The tile to generate indices for.
	\param	way	The vector of ways to generate indices for.
	\param	type The road type to generate indices for.
	\return	uint32_t The number of indices added (used for offset to index IBO)
	\brief	Generate indices for a given tile and way - specifically for road types (i.e motorway, primary etc.).
	***********************************************************************************************************************/
	uint32_t generateIndices(Tile& tile, std::vector<Way>& way, RoadTypes::RoadTypes type);

	/*!*********************************************************************************************************************
	\param	tile The tile to generate indices for.
	\param	way	The vector of outlines to generate indices for.
	\return	uint32_t The number of indices added (used for offset to index IBO)
	\brief	Generate indices for a given tile and outline (i.e road, area etc.).
	***********************************************************************************************************************/
	uint32_t generateIndices(Tile& tile, std::vector<uint64_t>& way);

	void setUniforms();
	void createShadowMatrix();

	void recordPrimaryCBO(uint32_t swapchain);
	void updateCommandBuffer(const uint32_t swapchain);
	void updateAnimation();
	void calculateTransform();
	void calculateClipPlanes();
	bool inFrustum(glm::vec2 min, glm::vec2 max);

	bool createPipelines();

	void recordUICommands();
	struct CameraTracking
	{
		glm::vec3 translation;
		glm::mat4 camRotation;
		glm::vec3 look;
		glm::vec3 up;

		CameraTracking() : translation(0.0f), camRotation(0.0f) {}
	} cameraInfo;
	pvr::TPSCamera _camera;
	float calculateRouteKeyFrameTime(const glm::dvec2& start, const glm::dvec2& end);

public:
	VulkanNavigation3D() : _totalRouteDistance(0.0f), _keyFrameTime(0.0f), _shadowMatrix(1.0) {}
};

// Calculate the key frame time between one point to another.
float VulkanNavigation3D::calculateRouteKeyFrameTime(const glm::dvec2& start, const glm::dvec2& end)
{
	return ::calculateRouteKeyFrameTime(start, end, _totalRouteDistance, CameraMoveSpeed);
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanNavigation3D::initApplication()
{
	// Disable gamma correction in the framebuffer.
	//
	setBackBufferColorspace(pvr::ColorSpace::lRGB);
	// WARNING: This should not be done lightly. This example has taken care of linear/sRGB colour space conversion appropriately and has been tuned specifically
	// for performance/colour space correctness.

	_OSMdata = std::make_unique<NavDataProcess>(getAssetStream(MapFile), glm::ivec2(_windowWidth, _windowHeight));
	pvr::Result result = _OSMdata->loadAndProcessData();
	if (result != pvr::Result::Success) { return result; }

	createShadowMatrix();

	_frameId = 0;

	// perform gamma correction of the linear space colours so that they can do used directly without further thinking about Linear/sRGB colour space conversions
	// This should not be done lightly. This example in most cases only passes through hard coded colour values and uses them directly without applying any
	// maths to their values and so can be performed safely.
	// When rendering the buildings we do apply maths to these colours and as such the colour space conversion has been taken care of appropriately

	// Note that for the clear colour floating point values will be converted to the format of the image with the clear value being treated as linear if the image is sRGB
	_clearColor = pvr::utils::convertLRGBtoSRGB(ClearColorLinearSpace);

	_roadAreaColor = pvr::utils::convertLRGBtoSRGB(RoadAreaColorLinearSpace);
	_motorwayColor = pvr::utils::convertLRGBtoSRGB(MotorwayColorLinearSpace);
	_trunkRoadColor = pvr::utils::convertLRGBtoSRGB(TrunkRoadColorLinearSpace);
	_primaryRoadColor = pvr::utils::convertLRGBtoSRGB(PrimaryRoadColorLinearSpace);
	_secondaryRoadColor = pvr::utils::convertLRGBtoSRGB(SecondaryRoadColorLinearSpace);
	_serviceRoadColor = pvr::utils::convertLRGBtoSRGB(ServiceRoadColorLinearSpace);
	_otherRoadColor = pvr::utils::convertLRGBtoSRGB(OtherRoadColorLinearSpace);
	_parkingColor = pvr::utils::convertLRGBtoSRGB(ParkingColorLinearSpace);
	_outlineColor = pvr::utils::convertLRGBtoSRGB(OutlineColorLinearSpace);

	Log(LogLevel::Information, "Initialising Tile Data");
	_OSMdata->initTiles();

	return pvr::Result::Success;
}

inline float calculateRotateTime(float angleRad) { return 1000.f * angleRad / glm::pi<float>() * 2.f; }

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.).</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanNavigation3D::initView()
{
	for (int i = 0; i < pvrvk::FrameworkCaps::MaxSwapChains; ++i) { _updateText[i] = static_cast<uint32_t>(-1); }

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

	const pvr::utils::QueuePopulateInfo queuePopulate = {
		pvrvk::QueueFlags::e_GRAPHICS_BIT, surface, // One queue supporting Graphics and presentation
	};

	pvr::utils::QueueAccessInfo queueAccessInfo;
	// create the device
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), &queuePopulate, 1, &queueAccessInfo);

	// get the queue
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	// setup the vma allocators
	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	// Create the command pool
	_deviceResources->commandPool =
		_deviceResources->device->createCommandPool(pvrvk::CommandPoolCreateInfo(queueAccessInfo.familyId, pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{ swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT; }
	// Create the swapchain, on screen framebuffers and renderpass
	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));
	_deviceResources->swapchain = swapChainCreateOutput.swapchain;
	_deviceResources->onScreenFramebuffer = swapChainCreateOutput.framebuffer;

	// Initialise uiRenderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue, true, true, true,
		4 + _deviceResources->swapchain->getSwapchainLength(), 4 + _deviceResources->swapchain->getSwapchainLength());

	_windowWidth = static_cast<uint32_t>(_deviceResources->uiRenderer.getRenderingDimX());
	_windowHeight = static_cast<uint32_t>(_deviceResources->uiRenderer.getRenderingDimY());

	_numRows = _OSMdata->getNumRows();
	_numCols = _OSMdata->getNumCols();
	_tileRenderingResources.resize(_numCols);

	for (uint32_t i = 0; i < _numCols; ++i) { _tileRenderingResources[i].resize(_numRows); }

	_deviceResources->uiRenderer.getDefaultTitle()->setText("Navigation3D");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();

	// Create primary command buffers.
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->cbos.add(_deviceResources->commandPool->allocateCommandBuffer());
		_deviceResources->uiElementsCbo.add(_deviceResources->commandPool->allocateSecondaryCommandBuffer());
		_deviceResources->fencePerFrame[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->acquireSemaphore[i] = _deviceResources->device->createSemaphore();
		_deviceResources->submitSemaphore[i] = _deviceResources->device->createSemaphore();
	}

	// Create descriptor pool
	// 1 Image sampler
	// n Uniform buffer dynamic for the transformation data, n is number of swapchain images
	// 1 Uniform buffer for shadow matrix.
	_deviceResources->descPool = _deviceResources->device->createDescriptorPool(
		pvrvk::DescriptorPoolCreateInfo()
			.addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1)
			.addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, (uint16_t)_deviceResources->swapchain->getSwapchainLength())
			.addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1));

	_deviceResources->cbos[0]->begin();
	initTextureAndSampler(_deviceResources->cbos[0]);
	if (!createUbos()) { return pvr::Result::UnknownError; }

	// create a  text with 255 max length.
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->text[i] = _deviceResources->uiRenderer.createText("DUMMY", 255);
		_deviceResources->text[i]->setColor(0.0f, 0.0f, 0.0f, 1.0f);
		_deviceResources->text[i]->setPixelOffset(0.0f, static_cast<float>(-int32_t(_windowHeight / 3)));
		_deviceResources->text[i]->commitUpdates();
	}

	if (!createPipelines())
	{
		setExitMessage("Failed to create pipelines");
		return pvr::Result::UnknownError;
	}

	setUniforms();
	createBuffers(_deviceResources->cbos[0]);
	recordUICommands();
	_OSMdata->convertRoute(glm::dvec2(0), 0, 0, _totalRouteDistance);
	_deviceResources->cbos[0]->end();
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->cbos[0];
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);
	// wait for the queue to finish
	_deviceResources->queue->waitIdle();

	cameraInfo.translation.x = static_cast<float>(_OSMdata->getRouteData()[0].point.x);
	cameraInfo.translation.z = static_cast<float>(_OSMdata->getRouteData()[0].point.y);
	cameraInfo.translation.y = CamHeight;

	const glm::dvec2& camStartPosition = _OSMdata->getRouteData()[routeIndex].point;
	_camera.setTargetPosition(glm::vec3(camStartPosition.x, 0.f, camStartPosition.y));
	_camera.setHeight(CamHeight);
	_camera.setDistanceFromTarget(1.0f);

	_currentRoad = _OSMdata->getRouteData()[routeIndex].name;
	return pvr::Result::Success;
}

/// <summary>Create static and dynamic UBOs. Static UBO used to hold transform matrix and is updated once per frame.
/// Dynamic UBO is used to hold colour data for map elements and is only updated once during initialisation.</summary>
bool VulkanNavigation3D::createUbos()
{
	const pvrvk::PhysicalDeviceProperties& props = _deviceResources->device->getPhysicalDevice()->getProperties();
	const uint32_t numSwapchainLength = _deviceResources->swapchain->getSwapchainLength();

	{
		pvr::utils::StructuredMemoryDescription memDesc;
		memDesc.addElement("transform", pvr::GpuDatatypes::mat4x4).addElement("viewMatrix", pvr::GpuDatatypes::mat4x4).addElement("lightDir", pvr::GpuDatatypes::vec3);

		_deviceResources->uboDynamic.bufferView.initDynamic(memDesc, numSwapchainLength, pvr::BufferUsageFlags::UniformBuffer, props.getLimits().getMinUniformBufferOffsetAlignment());

		_deviceResources->uboDynamic.buffer = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->uboDynamic.bufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->uboDynamic.bufferView.pointToMappedMemory(_deviceResources->uboDynamic.buffer->getDeviceMemory()->getMappedData());
	}
	// Static Buffer creation. Contains Shadow matrix and colour uniform which get uploaded once.
	{
		pvr::utils::StructuredMemoryDescription memDesc;
		memDesc.addElement("shadowMatrix", pvr::GpuDatatypes::mat4x4);
		_deviceResources->uboStatic.bufferView.init(memDesc);
		// Create the buffer
		const VkDeviceSize bufferSize = _deviceResources->uboStatic.bufferView.getSize();

		// Create the buffer from device local heap and Host visible if supported.
		_deviceResources->uboStatic.buffer = pvr::utils::createBuffer(_deviceResources->device, pvrvk::BufferCreateInfo(bufferSize, pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
			pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		// Write in to the buffer
		_deviceResources->uboStatic.bufferView.pointToMappedMemory(_deviceResources->uboStatic.buffer->getDeviceMemory()->getMappedData());
		_deviceResources->uboStatic.bufferView.getElement(0).setValue(_shadowMatrix);

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(_deviceResources->uboStatic.buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{ _deviceResources->uboStatic.buffer->getDeviceMemory()->flushRange(0, _deviceResources->uboStatic.bufferView.getSize()); }
	}

	// Create the descriptor set layouts
	pvrvk::DescriptorSetLayoutCreateInfo layoutDesc;
	layoutDesc.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);
	_deviceResources->uboDynamic.layout = _deviceResources->device->createDescriptorSetLayout(layoutDesc);

	layoutDesc.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);
	_deviceResources->uboStatic.layout = _deviceResources->device->createDescriptorSetLayout(layoutDesc);

	_deviceResources->uboDynamic.set = _deviceResources->descPool->allocateDescriptorSet(_deviceResources->uboDynamic.layout);
	_deviceResources->uboStatic.set = _deviceResources->descPool->allocateDescriptorSet(_deviceResources->uboStatic.layout);

	pvrvk::WriteDescriptorSet descSetUpdate[] = {
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->uboDynamic.set, 0),
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->uboStatic.set, 0),
	};

	descSetUpdate[0].setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->uboDynamic.buffer, 0, _deviceResources->uboDynamic.bufferView.getDynamicSliceSize()));
	descSetUpdate[1].setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->uboStatic.buffer, 0, _deviceResources->uboStatic.bufferView.getSize()));
	_deviceResources->device->updateDescriptorSets(descSetUpdate, ARRAY_SIZE(descSetUpdate), 0, 0);
	return true;
}

/// <summary>Load a texture from file using PVR Asset Store, create a trilinear sampler, create a description set.</summary>
/// <returns>Return true if no error occurred, false if the sampler descriptor set is not valid.</returns>
void VulkanNavigation3D::initTextureAndSampler(pvrvk::CommandBuffer& uploadCmdBuffer)
{
	pvrvk::ImageView texBase;

	// create a tri-linear sampler
	pvrvk::SamplerCreateInfo samplerInfo(
		pvrvk::Filter::e_LINEAR, pvrvk::Filter::e_LINEAR, pvrvk::SamplerMipmapMode::e_LINEAR, pvrvk::SamplerAddressMode::e_REPEAT, pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE);

	_deviceResources->samplerTrilinear = _deviceResources->device->createSampler(samplerInfo);

	texBase = pvr::utils::loadAndUploadImageAndView(_deviceResources->device, RoadTexFile, true, uploadCmdBuffer, *this, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
		pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);

	// create the descriptor set layout
	pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
	descSetLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	_deviceResources->texDescSetLayout = _deviceResources->device->createDescriptorSetLayout(descSetLayoutInfo);

	// create the descriptor set
	_deviceResources->imageSamplerDescSet = _deviceResources->descPool->allocateDescriptorSet(_deviceResources->texDescSetLayout);

	pvrvk::WriteDescriptorSet descSetCreateInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->imageSamplerDescSet);

	descSetCreateInfo.setImageInfo(0, pvrvk::DescriptorImageInfo(texBase, _deviceResources->samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

	_deviceResources->device->updateDescriptorSets(&descSetCreateInfo, 1, nullptr, 0);

	// upload the font
	pvr::Texture fontHeader;
	pvrvk::ImageView fontTex = pvr::utils::loadAndUploadImageAndView(_deviceResources->device, FontFile, true, uploadCmdBuffer, *this, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
		pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, &fontHeader, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);

	samplerInfo.wrapModeU = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;
	samplerInfo.wrapModeV = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;

	pvrvk::Sampler sampler = _deviceResources->device->createSampler(samplerInfo);
	_deviceResources->font = _deviceResources->uiRenderer.createFont(fontTex, fontHeader, sampler);
}

inline VkDeviceSize getColorUniformSlice(ColourUniforms::ColourUniforms uniforms, uint32_t swapchain) { return ColourUniforms::Count * swapchain + uniforms; }

/// <summary>Setup uniforms used for drawing the map. Fill dynamic UBO with uniform data.</summary>
void VulkanNavigation3D::setUniforms()
{
	_perspectiveMatrix =
		_deviceResources->uiRenderer.getScreenRotation() * pvr::math::perspectiveFov(pvr::Api::Vulkan, glm::radians(45.0f), float(_windowWidth), float(_windowHeight), 0.01f, 5.f);
}

/// <summary>Creates a special matrix which will be used to project 3D volumes onto a plane with respect to the ground (0, 1, 0)
/// and light direction (1, 1, -1). This matrix will be used to render the planar shadows for buildings.</summary>
void VulkanNavigation3D::createShadowMatrix()
{
	const glm::vec4 ground = glm::vec4(0.0, 1.0, 0.0, 0.0);
	const glm::vec4 light = glm::vec4(glm::normalize(glm::vec3(0.25f, 2.4f, -1.15f)), 0.0f);
	const float d = glm::dot(ground, light);

	_shadowMatrix[0][0] = static_cast<float>(d - light.x * ground.x);
	_shadowMatrix[1][0] = static_cast<float>(0.0 - light.x * ground.y);
	_shadowMatrix[2][0] = static_cast<float>(0.0 - light.x * ground.z);
	_shadowMatrix[3][0] = static_cast<float>(0.0 - light.x * ground.w);

	_shadowMatrix[0][1] = static_cast<float>(0.0 - light.y * ground.x);
	_shadowMatrix[1][1] = static_cast<float>(d - light.y * ground.y);
	_shadowMatrix[2][1] = static_cast<float>(0.0 - light.y * ground.z);
	_shadowMatrix[3][1] = static_cast<float>(0.0 - light.y * ground.w);

	_shadowMatrix[0][2] = static_cast<float>(0.0 - light.z * ground.x);
	_shadowMatrix[1][2] = static_cast<float>(0.0 - light.z * ground.y);
	_shadowMatrix[2][2] = static_cast<float>(d - light.z * ground.z);
	_shadowMatrix[3][2] = static_cast<float>(0.0 - light.z * ground.w);

	_shadowMatrix[0][3] = static_cast<float>(0.0 - light.w * ground.x);
	_shadowMatrix[1][3] = static_cast<float>(0.0 - light.w * ground.y);
	_shadowMatrix[2][3] = static_cast<float>(0.0 - light.w * ground.z);
	_shadowMatrix[3][3] = static_cast<float>(d - light.w * ground.w);
}

/// <summary>Creates vertex and index buffers and records the secondary command buffers for each tile.</summary>
void VulkanNavigation3D::createBuffers(pvrvk::CommandBuffer& uploadCmd)
{
	uint32_t col = 0;
	uint32_t row = 0;

	const uint32_t swapchainLength = _deviceResources->swapchain->getSwapchainLength();
	for (auto& tileCol : _OSMdata->getTiles())
	{
		for (Tile& tile : tileCol)
		{
			_tileRenderingResources[col][row] = std::make_unique<TileRenderingResources>();

			// Set the min and max coordinates for the tile
			tile.screenMin = remap(tile.min, _OSMdata->getTiles()[0][0].min, _OSMdata->getTiles()[0][0].max, glm::dvec2(-5, -5), glm::dvec2(5, 5));
			tile.screenMax = remap(tile.max, _OSMdata->getTiles()[0][0].min, _OSMdata->getTiles()[0][0].max, glm::dvec2(-5, -5), glm::dvec2(5, 5));

			// Create vertices for tile
			for (auto nodeIterator = tile.nodes.begin(); nodeIterator != tile.nodes.end(); ++nodeIterator)
			{
				nodeIterator->second.index = static_cast<uint32_t>(tile.vertices.size());

				glm::vec2 remappedPos =
					glm::vec2(remap(nodeIterator->second.coords, _OSMdata->getTiles()[0][0].min, _OSMdata->getTiles()[0][0].max, glm::dvec2(-5, -5), glm::dvec2(5, 5)));
				glm::vec3 vertexPos = glm::vec3(remappedPos.x, nodeIterator->second.height, remappedPos.y);
				tile.vertices.push_back(Tile::VertexData(vertexPos, nodeIterator->second.texCoords));
			}

			// Add car parking to indices
			uint32_t parkingNum = ::generateIndices(tile, tile.parkingWays);

			// Add road area ways to indices
			uint32_t areaNum = ::generateIndices(tile, tile.areaWays);

			// Add road area outlines to indices
			uint32_t roadAreaOutlineNum = ::generateIndices(tile, tile.areaOutlineIds);

			// Add roads to indices
			uint32_t motorwayNum = ::generateIndices(tile, tile.roadWays, RoadTypes::Motorway);
			uint32_t trunkRoadNum = ::generateIndices(tile, tile.roadWays, RoadTypes::Trunk);
			uint32_t primaryRoadNum = ::generateIndices(tile, tile.roadWays, RoadTypes::Primary);
			uint32_t secondaryRoadNum = ::generateIndices(tile, tile.roadWays, RoadTypes::Secondary);
			uint32_t serviceRoadNum = ::generateIndices(tile, tile.roadWays, RoadTypes::Service);
			uint32_t otherRoadNum = ::generateIndices(tile, tile.roadWays, RoadTypes::Other);

			// Add buildings to indices
			uint32_t buildNum = ::generateIndices(tile, tile.buildWays);

			// Add inner ways to indices
			uint32_t innerNum = ::generateIndices(tile, tile.innerWays);

			::generateNormals(tile, static_cast<uint32_t>(tile.indices.size() - (innerNum + buildNum)), buildNum);

			// Create vertex and index buffers
			// Interleaved vertex buffer (vertex position + texCoord)
			auto& tileRes = _tileRenderingResources[col][row];
			// Create vertex and index buffers
			// Interleaved vertex buffer (vertex position + texCoord)

			{
				const pvrvk::DeviceSize vboSize = static_cast<pvrvk::DeviceSize>(tile.vertices.size() * sizeof(tile.vertices[0]));
				tileRes->vbo = pvr::utils::createBuffer(_deviceResources->device,
					pvrvk::BufferCreateInfo(vboSize, pvrvk::BufferUsageFlags::e_VERTEX_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT),
					pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
					pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
					_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

				bool isBufferHostVisible = (tileRes->vbo->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT) != 0;
				if (isBufferHostVisible) { pvr::utils::updateHostVisibleBuffer(tileRes->vbo, tile.vertices.data(), 0, vboSize, true); }
				else
				{
					pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, tileRes->vbo, uploadCmd, tile.vertices.data(), 0, vboSize, _deviceResources->vmaAllocator);
				}
			}

			{
				const pvrvk::DeviceSize iboSize = static_cast<pvrvk::DeviceSize>(tile.indices.size() * sizeof(tile.indices[0]));

				tileRes->ibo = pvr::utils::createBuffer(_deviceResources->device,
					pvrvk::BufferCreateInfo(iboSize, pvrvk::BufferUsageFlags::e_INDEX_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT),
					pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
					pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
					_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

				bool isBufferHostVisible = (tileRes->ibo->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT) != 0;
				if (isBufferHostVisible) { pvr::utils::updateHostVisibleBuffer(tileRes->ibo, tile.indices.data(), 0, iboSize, true); }
				else
				{
					pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, tileRes->ibo, uploadCmd, tile.indices.data(), 0, iboSize, _deviceResources->vmaAllocator);
				}
			}

			uint32_t uboOffset = 0;

			// Record Secondary commands
			for (uint32_t i = 0; i < swapchainLength; ++i)
			{
				uint32_t offset = 0;
				_tileRenderingResources[col][row]->secCbo.add(_deviceResources->commandPool->allocateSecondaryCommandBuffer());
				pvrvk::SecondaryCommandBuffer& cmdBuffer = _tileRenderingResources[col][row]->secCbo[i];

				uboOffset = _deviceResources->uboDynamic.bufferView.getDynamicSliceOffset(i);
				cmdBuffer->begin(_deviceResources->onScreenFramebuffer[i]);

				// Bind the Dynamic and static buffers
				pvrvk::DescriptorSet descSets[] = { _deviceResources->uboDynamic.set, _deviceResources->uboStatic.set };
				cmdBuffer->bindDescriptorSets(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->fillPipe->getPipelineLayout(), 0, descSets, ARRAY_SIZE(descSets), &uboOffset, 1);

				// Bind the vertex and index buffers for the tile
				cmdBuffer->bindVertexBuffer(_tileRenderingResources[col][row]->vbo, 0, 0);
				cmdBuffer->bindIndexBuffer(_tileRenderingResources[col][row]->ibo, 0, pvrvk::IndexType::e_UINT32);

				pvrvk::GraphicsPipeline lastBoundPipeline;
				// Draw the car parking
				if (parkingNum > 0)
				{
					glm::vec4 colorId = _parkingColor;
					cmdBuffer->pushConstants(_deviceResources->fillPipe->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0,
						static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4)), &colorId);
					cmdBuffer->bindPipeline(_deviceResources->fillPipe);
					lastBoundPipeline = _deviceResources->fillPipe;
					cmdBuffer->drawIndexed(0, parkingNum);
					offset += parkingNum;
				}

				// Draw the road areas
				if (areaNum > 0)
				{
					const glm::vec4 colorId = _roadAreaColor;
					cmdBuffer->pushConstants(_deviceResources->fillPipe->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0,
						static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4)), &colorId);
					if (lastBoundPipeline != _deviceResources->fillPipe)
					{
						cmdBuffer->bindPipeline(_deviceResources->fillPipe);
						lastBoundPipeline = _deviceResources->fillPipe;
					}
					cmdBuffer->drawIndexed(offset, areaNum);
					offset += areaNum;
				}

				// Draw the outlines for road areas
				if (roadAreaOutlineNum > 0)
				{
					const glm::vec4 colorId = _outlineColor;
					cmdBuffer->pushConstants(_deviceResources->outlinePipe->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0,
						static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4)), &colorId);
					if (lastBoundPipeline != _deviceResources->outlinePipe)
					{
						cmdBuffer->bindPipeline(_deviceResources->outlinePipe);
						lastBoundPipeline = _deviceResources->outlinePipe;
					}
					cmdBuffer->drawIndexed(offset, roadAreaOutlineNum);
					offset += roadAreaOutlineNum;
				}

				/**** Draw the roads ****/
				if (lastBoundPipeline != _deviceResources->roadPipe && (motorwayNum + trunkRoadNum + primaryRoadNum + secondaryRoadNum + serviceRoadNum + otherRoadNum) > 0)
				{
					cmdBuffer->bindPipeline(_deviceResources->roadPipe);
					lastBoundPipeline = _deviceResources->roadPipe;
					cmdBuffer->bindDescriptorSet(
						pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->roadPipe->getPipelineLayout(), SetBinding::TextureSampler, _deviceResources->imageSamplerDescSet);
				}

				// Motorways
				if (motorwayNum > 0)
				{
					const glm::vec4 colorId = _motorwayColor;
					cmdBuffer->pushConstants(_deviceResources->roadPipe->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0,
						static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4)), &colorId);
					cmdBuffer->bindPipeline(_deviceResources->roadPipe);
					lastBoundPipeline = _deviceResources->roadPipe;
					cmdBuffer->drawIndexed(offset, motorwayNum);
					offset += motorwayNum;
				}

				// Trunk Roads
				if (trunkRoadNum > 0)
				{
					const glm::vec4 colorId = _trunkRoadColor;
					cmdBuffer->pushConstants(_deviceResources->roadPipe->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0,
						static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4)), &colorId);
					cmdBuffer->bindPipeline(_deviceResources->roadPipe);
					lastBoundPipeline = _deviceResources->roadPipe;
					cmdBuffer->drawIndexed(offset, trunkRoadNum);
					offset += trunkRoadNum;
				}

				// Primary Roads
				if (primaryRoadNum > 0)
				{
					const glm::vec4 colorId = _primaryRoadColor;
					cmdBuffer->pushConstants(_deviceResources->roadPipe->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0,
						static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4)), &colorId);
					cmdBuffer->bindPipeline(_deviceResources->roadPipe);
					lastBoundPipeline = _deviceResources->roadPipe;
					cmdBuffer->drawIndexed(offset, primaryRoadNum);
					offset += primaryRoadNum;
				}

				// Secondary Roads
				if (secondaryRoadNum > 0)
				{
					const glm::vec4 colorId = _secondaryRoadColor;
					cmdBuffer->pushConstants(_deviceResources->roadPipe->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0,
						static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4)), &colorId);
					cmdBuffer->bindPipeline(_deviceResources->roadPipe);
					lastBoundPipeline = _deviceResources->roadPipe;
					cmdBuffer->drawIndexed(offset, secondaryRoadNum);
					offset += secondaryRoadNum;
				}

				// Service Roads
				if (serviceRoadNum > 0)
				{
					const glm::vec4 colorId = _serviceRoadColor;
					cmdBuffer->pushConstants(_deviceResources->roadPipe->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0,
						static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4)), &colorId);
					cmdBuffer->bindPipeline(_deviceResources->roadPipe);
					lastBoundPipeline = _deviceResources->roadPipe;
					cmdBuffer->drawIndexed(offset, serviceRoadNum);
					offset += serviceRoadNum;
				}

				// Other (any other roads)
				if (otherRoadNum > 0)
				{
					const glm::vec4 colorId = _otherRoadColor;
					cmdBuffer->bindPipeline(_deviceResources->roadPipe);
					cmdBuffer->pushConstants(_deviceResources->roadPipe->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0,
						static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4)), &colorId);
					lastBoundPipeline = _deviceResources->roadPipe;
					cmdBuffer->drawIndexed(offset, otherRoadNum);
					offset += otherRoadNum;
				}

				// Draw the buildings & shadows
				if (buildNum > 0)
				{
					const glm::vec4 colorId = BuildingColorLinearSpace;
					cmdBuffer->pushConstants(_deviceResources->buildingPipe->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0,
						static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4)), &colorId);
					if (lastBoundPipeline != _deviceResources->buildingPipe)
					{
						cmdBuffer->bindPipeline(_deviceResources->buildingPipe);
						lastBoundPipeline = _deviceResources->buildingPipe;
					}
					cmdBuffer->drawIndexed(offset, buildNum);

					cmdBuffer->bindPipeline(_deviceResources->planarShadowPipe);
					lastBoundPipeline = _deviceResources->planarShadowPipe;
					cmdBuffer->drawIndexed(offset, buildNum);
					offset += buildNum;
				}

				// Draw the insides of car parking and buildings for polygons with holes
				if (innerNum > 0)
				{
					if (lastBoundPipeline != _deviceResources->fillPipe)
					{
						cmdBuffer->bindPipeline(_deviceResources->fillPipe);
						lastBoundPipeline = _deviceResources->fillPipe;
					}
					const glm::vec4 colorId = _clearColor;
					cmdBuffer->pushConstants(_deviceResources->fillPipe->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0,
						static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4)), &colorId);
					cmdBuffer->drawIndexed(offset, innerNum);
					offset += innerNum;
				}
				cmdBuffer->end();
			}
			row++;
		}
		row = 0;
		col++;
	}
}

uint32_t VulkanNavigation3D::generateIndices(Tile& tile, std::vector<Way>& way, RoadTypes::RoadTypes type)
{
	uint32_t count = 0;
	for (uint32_t i = 0; i < way.size(); ++i)
	{
		if (way[i].roadType == type)
		{
			for (uint32_t j = 0; j < way[i].nodeIds.size(); ++j)
			{
				tile.indices.push_back(tile.nodes.find(way[i].nodeIds[j])->second.index);
				count++;
			}
		}
	}
	return count;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanNavigation3D::renderFrame()
{
	updateAnimation();
	calculateTransform();
	calculateClipPlanes();
	_deviceResources->fencePerFrame[_frameId]->wait();
	_deviceResources->fencePerFrame[_frameId]->reset();
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->acquireSemaphore[_frameId]);
	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();
	_deviceResources->uboDynamic.bufferView.getElement(0, 0, swapchainIndex).setValue(_viewProjMatrix);
	_deviceResources->uboDynamic.bufferView.getElement(1, 0, swapchainIndex).setValue(_viewMatrix);
	_deviceResources->uboDynamic.bufferView.getElement(2, 0, swapchainIndex).setValue(_lightDir);

	// Flush only if the memory does not support HOST_COHERENT_BIT
	if (uint32_t(_deviceResources->uboDynamic.buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->uboDynamic.buffer->getDeviceMemory()->flushRange(
			_deviceResources->uboDynamic.bufferView.getDynamicSliceOffset(swapchainIndex), _deviceResources->uboDynamic.bufferView.getDynamicSliceSize());
	}

	// Update commands
	recordPrimaryCBO(swapchainIndex);
	pvrvk::SubmitInfo submitInfo;
	// Wait for the semaphore which get signalled by the acquireNextImage
	submitInfo.waitSemaphores = &_deviceResources->acquireSemaphore[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->submitSemaphore[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.commandBuffers = &_deviceResources->cbos[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	// Only wait for the semaphore when writing to the colour output, therefore the other stage can run before that.
	pvrvk::PipelineStageFlags waitStage = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.waitDstStageMask = &waitStage;
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->fencePerFrame[_frameId]);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->queue, _deviceResources->commandPool, _deviceResources->swapchain, swapchainIndex, getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	// Present
	pvrvk::PresentInfo presentInfo;
	presentInfo.imageIndices = &swapchainIndex;
	presentInfo.numSwapchains = 1;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.waitSemaphores = &_deviceResources->submitSemaphore[_frameId];
	presentInfo.numWaitSemaphores = 1;
	_deviceResources->queue->present(presentInfo);
	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();
	return pvr::Result::Success;
}

/// <summary>Handle user input.</summary>
void VulkanNavigation3D::updateAnimation()
{
	if (_OSMdata->getRouteData().size() == 0) { return; }

	static const float rotationOffset = -90.f;
	static bool turning = false;
	static float animTime = 0.0f;
	static float rotateTime = 0.0f;
	static float currentRotationTime = 0.0f;
	static float currentRotation = static_cast<float>(_OSMdata->getRouteData()[routeIndex].rotation);
	static glm::dvec2 camStartPosition = _OSMdata->getRouteData()[routeIndex].point;
	static glm::dvec2 camEndPosition;
	static glm::dvec2 camLerpPos = glm::dvec2(0.0f);
	static bool destinationReached = false;
	static float routeRestartTime = 0.;
	float dt = float(getFrameTime());
	camEndPosition = _OSMdata->getRouteData()[routeIndex + 1].point;
	const uint32_t lastRouteIndex = routeIndex;
	_keyFrameTime = calculateRouteKeyFrameTime(camStartPosition, camEndPosition);
	// Do the translation if the camera is not turning
	if (destinationReached && routeRestartTime >= 2000)
	{
		destinationReached = false;
		routeRestartTime = 0.0f;
	}
	if (destinationReached)
	{
		routeRestartTime += dt;
		return;
	}

	if (!turning)
	{
		// Interpolate between two positions.
		camLerpPos = glm::mix(camStartPosition, camEndPosition, animTime / _keyFrameTime);

		cameraInfo.translation = glm::vec3(camLerpPos.x, CamHeight, camLerpPos.y);
		_camera.setTargetPosition(glm::vec3(camLerpPos.x, 0.0f, camLerpPos.y));
		_camera.setTargetLookAngle(currentRotation + rotationOffset);
	}
	if (animTime >= _keyFrameTime)
	{
		const float r1 = static_cast<float>(_OSMdata->getRouteData()[routeIndex].rotation);
		const float r2 = static_cast<float>(_OSMdata->getRouteData()[routeIndex + 1].rotation);

		if ((!turning && fabs(r2 - r1) > 3.f) || (turning))
		{
			float diff = r2 - r1;
			float absDiff = fabs(diff);
			if (absDiff > 180.f)
			{
				if (diff > 0.f) // if the difference is positive angle then do negative rotation
					diff = -(360.f - absDiff);
				else // else do a positive rotation
					diff = (360.f - absDiff);
			}
			absDiff = fabs(diff); // get the abs
			rotateTime = 18.f * absDiff; // 18ms for an angle * angle diff

			currentRotationTime += dt;
			currentRotationTime = glm::clamp(currentRotationTime, 0.0f, rotateTime);
			if (currentRotationTime >= rotateTime) { turning = false; }
			else
			{
				turning = true;
				currentRotation = glm::mix(r1, r1 + diff, currentRotationTime / rotateTime);
				_camera.setTargetLookAngle(currentRotation + rotationOffset);
			}
		}
	}
	if (animTime >= _keyFrameTime && !turning)
	{
		turning = false;
		currentRotationTime = 0.0f;
		rotateTime = 0.0f;
		// Iterate through the route
		if (++routeIndex == _OSMdata->getRouteData().size() - 1)
		{
			currentRotation = static_cast<float>(_OSMdata->getRouteData()[0].rotation);
			routeIndex = 0;
			destinationReached = true;
			routeRestartTime = 0.f;
		}
		else
		{
			currentRotation = static_cast<float>(_OSMdata->getRouteData()[routeIndex].rotation);
		}
		animTime = 0.0f;
		// Reset the route.
		camStartPosition = _OSMdata->getRouteData()[routeIndex].point;
	}
	if (lastRouteIndex != routeIndex) { _currentRoad = _OSMdata->getRouteData()[routeIndex].name; }
	_viewMatrix = _camera.getViewMatrix();

	animTime += dt;
}

/// <summary>Calculate the View Projection Matrix.</summary>
void VulkanNavigation3D::calculateTransform()
{
	_lightDir = glm::normalize(glm::mat3(_viewMatrix) * glm::vec3(0.25f, -2.4f, -1.15f));
	_viewProjMatrix = _perspectiveMatrix * _viewMatrix;
}

/// <summary>Record the primary command buffer.</summary>
void VulkanNavigation3D::recordPrimaryCBO(uint32_t swapchain)
{
	const pvrvk::ClearValue clearValues[] = { pvrvk::ClearValue(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a), pvrvk::ClearValue::createDefaultDepthStencilClearValue() };

	_deviceResources->cbos[swapchain]->begin();
	_deviceResources->cbos[swapchain]->beginRenderPass(_deviceResources->onScreenFramebuffer[swapchain], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), false, clearValues, 2);
	updateCommandBuffer(swapchain);
	_deviceResources->cbos[swapchain]->endRenderPass();
	_deviceResources->cbos[swapchain]->end();
}

/// <summary>Find the tiles that need to be rendered.</summary>
void VulkanNavigation3D::updateCommandBuffer(const uint32_t swapchain)
{
	for (uint32_t i = 0; i < _numCols; ++i)
	{
		for (uint32_t j = 0; j < _numRows; ++j)
		{
			// Only queue up commands if the tile is visible.
			if (inFrustum(_OSMdata->getTiles()[i][j].screenMin, _OSMdata->getTiles()[i][j].screenMax))
			{ _deviceResources->cbos[swapchain]->executeCommands(_tileRenderingResources[i][j]->secCbo[swapchain]); }
		}
	}
	// Draw text elements
	if (_updateText[swapchain] != routeIndex)
	{
		_updateText[swapchain] = routeIndex;
		// Render UI elements.
		_deviceResources->text[swapchain]->setText(_currentRoad);
		_deviceResources->text[swapchain]->commitUpdates();
	}

	_deviceResources->cbos[swapchain]->executeCommands(_deviceResources->uiElementsCbo[swapchain]);
}

/// <summary>Capture frustum planes from the current View Projection matrix.</summary>
void VulkanNavigation3D::calculateClipPlanes() { pvr::math::getFrustumPlanes(pvr::Api::Vulkan, _viewProjMatrix, _viewFrustum); }

/// <summary>Tests whether a 2D bounding box is intersected or enclosed by a view frustum.
/// Only the near, far, left and right planes of the view frustum are taken into consideration to optimize the intersection test.</ summary>
/// <param name="min">The minimum co - ordinates of the bounding box.</param>
/// <param name="max">The maximum co - ordinates of the bounding box.</param>
/// <return>boolean True if inside the view frustum, false if outside.</returns>
bool VulkanNavigation3D::inFrustum(glm::vec2 min, glm::vec2 max)
{
	// Test the axis-aligned bounding box against each frustum plane,
	// cull if all points are outside of one the view frustum planes.
	pvr::math::AxisAlignedBox aabb;
	aabb.setMinMax(glm::vec3(min.x, 0.f, min.y), glm::vec3(max.x, 5.0f, max.y));
	return pvr::math::aabbInFrustum(aabb, _viewFrustum);
}

bool VulkanNavigation3D::createPipelines()
{
	// create the pipeline layout
	_deviceResources->pipeLayoutInfo.addDescSetLayout(_deviceResources->uboDynamic.layout); // Set 0
	_deviceResources->pipeLayoutInfo.addDescSetLayout(_deviceResources->uboStatic.layout); // Set 1
	_deviceResources->pipeLayoutInfo.addDescSetLayout(_deviceResources->texDescSetLayout); // Set 2

	_deviceResources->pipeLayoutInfo.setPushConstantRange(
		0, pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4))));
	_deviceResources->pipeLayout = _deviceResources->device->createPipelineLayout(_deviceResources->pipeLayoutInfo);

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	// Pipeline parameters
	pvrvk::GraphicsPipelineCreateInfo roadInfo;
	pvrvk::GraphicsPipelineCreateInfo fillInfo;
	pvrvk::GraphicsPipelineCreateInfo outlineInfo;
	pvrvk::GraphicsPipelineCreateInfo planarShadowInfo;
	pvrvk::GraphicsPipelineCreateInfo buildingInfo;

	// Vertex input info.
	const pvrvk::VertexInputAttributeDescription posAttrib(0, 0, pvrvk::Format::e_R32G32B32_SFLOAT, 0);
	const pvrvk::VertexInputAttributeDescription texAttrib(1, 0, pvrvk::Format::e_R32G32_SFLOAT, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Float) * 3));
	const pvrvk::VertexInputAttributeDescription normalAttrib(2, 0, pvrvk::Format::e_R32G32B32_SFLOAT, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Float) * 5));

	// Set parameters shared by all pipelines
	roadInfo.vertexInput.addInputBinding(pvrvk::VertexInputBindingDescription(0, sizeof(Tile::VertexData)));
	roadInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState(false));
	roadInfo.vertexShader = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("VertShader.vsh.spv")->readToEnd<uint32_t>()));
	roadInfo.fragmentShader = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("FragShader.fsh.spv")->readToEnd<uint32_t>()));
	roadInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);
	roadInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_NONE);
	roadInfo.depthStencil.enableDepthWrite(true).enableDepthTest(true).setDepthCompareFunc(pvrvk::CompareOp::e_LESS_OR_EQUAL);
	roadInfo.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
	roadInfo.pipelineLayout = _deviceResources->pipeLayout;
	roadInfo.viewport.setViewportAndScissor(
		0, pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(getWidth()), static_cast<float>(getHeight())), pvrvk::Rect2D(0, 0, getWidth(), getHeight()));
	roadInfo.rasterizer.setPolygonMode(pvrvk::PolygonMode::e_FILL);
	fillInfo = outlineInfo = planarShadowInfo = buildingInfo = roadInfo;
	fillInfo.vertexInput.addInputAttribute(posAttrib);
	fillInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);
	outlineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);

	// Road pipeline specific parameters. Classic Alpha blending, but preserving framebuffer alpha to avoid artefacts on
	// compositors that actually use the alpha value.
	roadInfo.colorBlend.setAttachmentState(0,
		pvrvk::PipelineColorBlendAttachmentState(
			true, pvrvk::BlendFactor::e_SRC_ALPHA, pvrvk::BlendFactor::e_ONE_MINUS_SRC_ALPHA, pvrvk::BlendOp::e_ADD, pvrvk::BlendFactor::e_ZERO, pvrvk::BlendFactor::e_ONE));
	roadInfo.vertexShader = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("AA_VertShader.vsh.spv")->readToEnd<uint32_t>()));
	roadInfo.fragmentShader = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("AA_FragShader.fsh.spv")->readToEnd<uint32_t>()));
	roadInfo.vertexInput.addInputAttribute(posAttrib).addInputAttribute(texAttrib);

	// Outline pipeline specific parameters
	outlineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_LINE_LIST);
	outlineInfo.vertexInput.addInputAttribute(posAttrib);

	// Building pipeline specific parameters
	buildingInfo.vertexShader = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("PerVertexLight_VertShader.vsh.spv")->readToEnd<uint32_t>()));
	buildingInfo.vertexInput.addInputAttribute(posAttrib).addInputAttribute(normalAttrib);

	int32_t doGammaCorrection = 1;
	pvrvk::ShaderConstantInfo shaderConstant = pvrvk::ShaderConstantInfo(0, &doGammaCorrection, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Integer)));
	buildingInfo.fragmentShader.setShaderConstant(0, shaderConstant);

	// Planar shadow pipeline specific parameters
	planarShadowInfo.vertexShader =
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("PlanarShadow_VertShader.vsh.spv")->readToEnd<uint32_t>()));
	planarShadowInfo.fragmentShader =
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("PlanarShadow_FragShader.fsh.spv")->readToEnd<uint32_t>()));
	planarShadowInfo.colorBlend.setAttachmentState(0,
		pvrvk::PipelineColorBlendAttachmentState(
			true, pvrvk::BlendFactor::e_SRC_ALPHA, pvrvk::BlendFactor::e_ONE_MINUS_SRC_ALPHA, pvrvk::BlendOp::e_ADD, pvrvk::BlendFactor::e_ZERO, pvrvk::BlendFactor::e_ONE));
	planarShadowInfo.vertexInput.addInputAttribute(posAttrib);

	pvrvk::StencilOpState stencilState;
	stencilState.setCompareOp(pvrvk::CompareOp::e_EQUAL);
	stencilState.setReference(0x0);
	stencilState.setCompareMask(0xff);
	stencilState.setFailOp(pvrvk::StencilOp::e_KEEP); // stencil fail
	stencilState.setDepthFailOp(pvrvk::StencilOp::e_KEEP); // depth fail, stencil pass
	stencilState.setPassOp(pvrvk::StencilOp::e_INCREMENT_AND_WRAP); // both pass
	planarShadowInfo.depthStencil.enableStencilTest(true).setStencilFrontAndBack(stencilState);

	// Create pipeline objects
	_deviceResources->roadPipe = _deviceResources->device->createGraphicsPipeline(roadInfo, _deviceResources->pipelineCache);
	_deviceResources->fillPipe = _deviceResources->device->createGraphicsPipeline(fillInfo, _deviceResources->pipelineCache);
	_deviceResources->outlinePipe = _deviceResources->device->createGraphicsPipeline(outlineInfo, _deviceResources->pipelineCache);
	_deviceResources->buildingPipe = _deviceResources->device->createGraphicsPipeline(buildingInfo, _deviceResources->pipelineCache);
	_deviceResources->planarShadowPipe = _deviceResources->device->createGraphicsPipeline(planarShadowInfo, _deviceResources->pipelineCache);

	return _deviceResources->roadPipe && _deviceResources->fillPipe && _deviceResources->outlinePipe && _deviceResources->buildingPipe && _deviceResources->planarShadowPipe;
}

void VulkanNavigation3D::recordUICommands()
{
	for (uint32_t swapchain = 0; swapchain < _deviceResources->swapchain->getSwapchainLength(); ++swapchain)
	{
		_deviceResources->uiElementsCbo[swapchain]->begin(_deviceResources->onScreenFramebuffer[swapchain]);
		// Render UI elements.
		_deviceResources->uiRenderer.beginRendering(_deviceResources->uiElementsCbo[swapchain]);
		_deviceResources->text[swapchain]->render();

		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.endRendering();
		_deviceResources->uiElementsCbo[swapchain]->end();
	}
}

/// <summary>Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanNavigation3D::releaseView()
{
	_tileRenderingResources.clear();

	// Reset context and associated resources.
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
///	If the rendering context is lost, quitApplication() will not be called.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanNavigation3D::quitApplication()
{
	_OSMdata.reset();
	return pvr::Result::Success;
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanNavigation3D>(); }
