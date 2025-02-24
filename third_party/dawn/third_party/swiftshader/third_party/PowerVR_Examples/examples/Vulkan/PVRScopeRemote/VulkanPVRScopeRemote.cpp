/*!
\brief Shows how to use our example PVRScope graph code.
\file VulkanPVRScopeRemote.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRScopeComms.h"

// Source and binary shaders
const char FragShaderSrcFile[] = "FragShader.fsh.spv";
const char VertShaderSrcFile[] = "VertShader.vsh.spv";

// PVR texture files
const char TextureFile[] = "Marble.pvr";

// POD scene files
const char SceneFile[] = "Satyr.pod";
enum
{
	MaxSwapChains = 8
};
namespace CounterDefs {
enum Enum
{
	Counter,
	Counter10,
	NumCounter
};
}

namespace PipelineConfigs {
// Pipeline Descriptor sets
enum DescriptorSetIds
{
	Model,
	Lighting
};
} // namespace PipelineConfigs
namespace BufferEntryNames {
namespace Matrices {
const char* const MVPMatrix = "mVPMatrix";
const char* const MVInverseTransposeMatrix = "mVITMatrix";
} // namespace Matrices
namespace Materials {
const char* const AlbedoModulation = "albedoModulation";
const char* const SpecularExponent = "specularExponent";
const char* const Metallicity = "metallicity";
const char* const Reflectivity = "reflectivity";
} // namespace Materials
} // namespace BufferEntryNames

const char* FrameDefs[CounterDefs::NumCounter] = { "Frames", "Frames10" };

struct DeviceResources
{
	pvrvk::Instance instance;
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
	pvrvk::Surface surface;
	pvrvk::Device device;
	pvrvk::Swapchain swapchain;
	pvrvk::Queue queue;
	pvr::utils::vma::Allocator vmaAllocator;
	pvrvk::CommandPool commandPool;
	pvrvk::DescriptorPool descriptorPool;

	pvr::Multi<pvrvk::ImageView> depthStencilImages;

	pvrvk::Semaphore imageAcquiredSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore presentationSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameResourcesFences[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	pvrvk::GraphicsPipeline pipeline;
	pvrvk::ImageView texture;
	std::vector<pvrvk::Buffer> vbos;
	std::vector<pvrvk::Buffer> ibos;
	std::vector<pvrvk::CommandBuffer> cmdBuffers;

	pvr::utils::StructuredBufferView uboMatricesBufferView;
	pvrvk::Buffer uboMatrices;
	pvr::utils::StructuredBufferView uboMaterialBufferView;
	pvrvk::Buffer uboMaterial;
	pvr::utils::StructuredBufferView uboLightingBufferView;
	pvrvk::Buffer uboLighting;

	pvrvk::DescriptorSetLayout modelDescriptorSetLayout;
	pvrvk::DescriptorSetLayout lightingDescriptorSetLayout;

	pvrvk::PipelineLayout pipelineLayout;

	pvrvk::DescriptorSet modelDescriptorSets[MaxSwapChains];
	pvrvk::DescriptorSet lightingDescriptorSet;

	pvrvk::DescriptorSetLayout descriptorSetLayout;
	pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;

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

/// <summary>Class implementing the PVRShell functions.</summary>
class VulkanPVRScopeRemote : public pvr::Shell
{
	std::unique_ptr<DeviceResources> _deviceResources;

	uint32_t _frameId;
	glm::mat4 _projectionMtx;
	glm::mat4 _viewMtx;

	// 3D Model
	pvr::assets::ModelHandle _scene;

	struct UboMaterialData
	{
		glm::vec3 albedo;
		float specularExponent;
		float metallicity;
		float reflectivity;
		bool isDirty;
	} _uboMatData;

	// The translation and Rotate parameter of Model
	float _angleY;

	// Data connection to PVRPerfServer
	bool _hasCommunicationError;
	SSPSCommsData* _spsCommsData;
	SSPSCommsLibraryTypeFloat _commsLibSpecularExponent;
	SSPSCommsLibraryTypeFloat _commsLibMetallicity;
	SSPSCommsLibraryTypeFloat _commsLibReflectivity;
	SSPSCommsLibraryTypeFloat _commsLibAlbedoR;
	SSPSCommsLibraryTypeFloat _commsLibAlbedoG;
	SSPSCommsLibraryTypeFloat _commsLibAlbedoB;
	uint32_t _frameCounter;
	uint32_t _frame10Counter;
	uint32_t _counterReadings[CounterDefs::NumCounter];

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void createBuffers();
	void recordCommandBuffer(uint32_t swapchain);
	void createDescriptorSetLayouts();
	void createPipeline();
	void loadVbos(pvrvk::CommandBuffer& uploadCmd);
	void drawMesh(int i32NodeIndex, pvrvk::CommandBuffer& command);
	void createDescriptorSet();
	void updateUbo(uint32_t swapchain);
};

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it(e.g.external modules, loading meshes, etc.).If the rendering
/// context is lost, initApplication() will not be called again.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanPVRScopeRemote::initApplication()
{
	_frameId = 0;

	// Load the scene
	_scene = pvr::assets::loadModel(*this, SceneFile);

	// We want a data connection to PVRPerfServer
	{
		_spsCommsData = pplInitialise("PVRScopeRemote", 14);
		_hasCommunicationError = false;

		// Demonstrate that there is a good chance of the initial data being
		// lost - the connection is normally completed asynchronously.
		pplSendMark(_spsCommsData, "lost", static_cast<uint32_t>(strlen("lost")));

		// This is entirely optional. Wait for the connection to succeed, it will
		// timeout if e.g. PVRPerfServer is not running.
		int isConnected;
		pplWaitForConnection(_spsCommsData, &isConnected, 1, 200);
	}
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	_uboMatData.specularExponent = 5.0f; // Width of the specular highlights (using low exponent for a brushed metal look)
	_uboMatData.albedo = glm::vec3(1.0f, 0.563f, 0.087f); // Overall colour
	_uboMatData.metallicity = 1.f; // Is the colour of the specular white (non-metallic), or coloured by the object(metallic)
	_uboMatData.reflectivity = 0.9f; // Percentage of contribution of diffuse / specular
	_uboMatData.isDirty = true;
	_frameCounter = 0;
	_frame10Counter = 0;

	// set angle of rotation
	_angleY = 0.0f;

	//	Remotely editable library items
	if (_spsCommsData)
	{
		std::vector<SSPSCommsLibraryItem> communicableItems;

		// Editable: Specular Exponent
		communicableItems.push_back(SSPSCommsLibraryItem());
		_commsLibSpecularExponent.fCurrent = _uboMatData.specularExponent;
		_commsLibSpecularExponent.fMin = 1.1f;
		_commsLibSpecularExponent.fMax = 300.0f;
		communicableItems.back().pszName = "Specular Exponent";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibSpecularExponent;
		communicableItems.back().nDataLength = sizeof(_commsLibSpecularExponent);

		communicableItems.push_back(SSPSCommsLibraryItem());
		// Editable: Metallicity
		_commsLibMetallicity.fCurrent = _uboMatData.metallicity;
		_commsLibMetallicity.fMin = 0.0f;
		_commsLibMetallicity.fMax = 1.0f;
		communicableItems.back().pszName = "Metallicity";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibMetallicity;
		communicableItems.back().nDataLength = sizeof(_commsLibMetallicity);

		// Editable: Reflectivity
		communicableItems.push_back(SSPSCommsLibraryItem());
		_commsLibReflectivity.fCurrent = _uboMatData.reflectivity;
		_commsLibReflectivity.fMin = 0.;
		_commsLibReflectivity.fMax = 1.;
		communicableItems.back().pszName = "Reflectivity";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibReflectivity;
		communicableItems.back().nDataLength = sizeof(_commsLibReflectivity);

		// Editable: Albedo R channel
		communicableItems.push_back(SSPSCommsLibraryItem());
		_commsLibAlbedoR.fCurrent = _uboMatData.albedo.r;
		_commsLibAlbedoR.fMin = 0.0f;
		_commsLibAlbedoR.fMax = 1.0f;
		communicableItems.back().pszName = "Albedo R";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibAlbedoR;
		communicableItems.back().nDataLength = sizeof(_commsLibAlbedoR);

		// Editable: Albedo R channel
		communicableItems.push_back(SSPSCommsLibraryItem());
		_commsLibAlbedoG.fCurrent = _uboMatData.albedo.g;
		_commsLibAlbedoG.fMin = 0.0f;
		_commsLibAlbedoG.fMax = 1.0f;
		communicableItems.back().pszName = "Albedo G";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibAlbedoG;
		communicableItems.back().nDataLength = sizeof(_commsLibAlbedoG);

		// Editable: Albedo R channel
		communicableItems.push_back(SSPSCommsLibraryItem());
		_commsLibAlbedoB.fCurrent = _uboMatData.albedo.b;
		_commsLibAlbedoB.fMin = 0.0f;
		_commsLibAlbedoB.fMax = 1.0f;
		communicableItems.back().pszName = "Albedo B";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibAlbedoB;
		communicableItems.back().nDataLength = sizeof(_commsLibAlbedoB);

		// OK, submit our library
		if (!pplLibraryCreate(_spsCommsData, communicableItems.data(), static_cast<uint32_t>(communicableItems.size())))
		{ Log(LogLevel::Debug, "PVRScopeRemote: pplLibraryCreate() failed\n"); } // User defined counters
		SSPSCommsCounterDef counterDefines[CounterDefs::NumCounter];
		for (uint32_t i = 0; i < CounterDefs::NumCounter; ++i)
		{
			counterDefines[i].pszName = FrameDefs[i];
			counterDefines[i].nNameLength = static_cast<uint32_t>(strlen(FrameDefs[i]));
		}

		if (!pplCountersCreate(_spsCommsData, counterDefines, CounterDefs::NumCounter)) { Log(LogLevel::Debug, "PVRScopeRemote: pplCountersCreate() failed\n"); }
	}
	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change  in the rendering context. Used to initialize variables that are dependent on the
/// rendering context(e.g.textures, vertex buffers, etc.).</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanPVRScopeRemote::initView()
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

	pvr::utils::QueuePopulateInfo queuePopulateInfo = { pvrvk::QueueFlags::e_GRAPHICS_BIT, surface };
	pvr::utils::QueueAccessInfo queueAccessInfo;

	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), &queuePopulateInfo, 1, &queueAccessInfo);

	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{ swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT; } // create the swapchain
	// Create the Swapchain, its renderpass, attachments and framebuffers. Will support MSAA if enabled through command line.
	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));

	_deviceResources->swapchain = swapChainCreateOutput.swapchain;
	_deviceResources->onScreenFramebuffer = swapChainCreateOutput.framebuffer;

	// Create the Command pool and Descriptor pool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(
		pvrvk::CommandPoolCreateInfo(_deviceResources->queue->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 16)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 16)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, 16));
	const uint32_t swapchainLength = _deviceResources->swapchain->getSwapchainLength();
	_deviceResources->cmdBuffers.resize(swapchainLength);
	for (uint32_t i = 0; i < swapchainLength; ++i)
	{
		_deviceResources->cmdBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
	}

	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	_deviceResources->cmdBuffers[0]->begin();

	//	Initialize VBO data
	loadVbos(_deviceResources->cmdBuffers[0]);

	_deviceResources->texture = pvr::utils::loadAndUploadImageAndView(_deviceResources->device, TextureFile, true, _deviceResources->cmdBuffers[0], *this,
		pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	_deviceResources->cmdBuffers[0]->end();
	// submit the texture upload commands
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->cmdBuffers[0];
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle();

	createDescriptorSetLayouts();

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	createPipeline();

	createBuffers();

	// create the pipeline
	createDescriptorSet();

	_deviceResources->uboLightingBufferView.getElementByName("viewLightDirection").setValue(glm::normalize(glm::vec3(1., 1., -1.)));

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->uboLighting->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{ _deviceResources->uboLighting->getDeviceMemory()->flushRange(0, _deviceResources->uboLightingBufferView.getSize()); } //	Initialize the UI Renderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue);

	// create the PVRScope connection pass and fail text
	_deviceResources->uiRenderer.getDefaultTitle()->setText("PVRScopeRemote");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();

	_deviceResources->uiRenderer.getDefaultDescription()->setScale(glm::vec2(.5, .5));
	_deviceResources->uiRenderer.getDefaultDescription()->setText("Use PVRTune to remotely control the parameters of this application.");
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();

	// Calculate the projection and view matrices
	// Is the screen rotated?
	bool isRotated = this->isScreenRotated();
	_viewMtx = glm::lookAt(glm::vec3(0.f, 0.f, 75.f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	_projectionMtx = pvr::math::perspectiveFov(pvr::Api::Vulkan, glm::pi<float>() / 6, static_cast<float>(getWidth()), static_cast<float>(getHeight()),
		_scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(), isRotated ? glm::pi<float>() * .5f : 0.0f);

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i) { recordCommandBuffer(i); }
	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by Shell when the application quits.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanPVRScopeRemote::releaseView()
{
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);
	// Release UIRenderer
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.</summary>
/// <returns>Result::Success if no error occurred</returns>.
pvr::Result VulkanPVRScopeRemote::quitApplication()
{
	if (_spsCommsData)
	{
		_hasCommunicationError |= !pplSendProcessingBegin(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

		// Close the data connection to PVRPerfServer
		for (uint32_t i = 0; i < 40; ++i)
		{
			char buf[128];
			const int nLen = sprintf(buf, "test %u", i);
			_hasCommunicationError |= !pplSendMark(_spsCommsData, buf, nLen);
		}
		_hasCommunicationError |= !pplSendProcessingEnd(_spsCommsData);
		pplShutdown(_spsCommsData);
	}
	_scene.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanPVRScopeRemote::renderFrame()
{
	if (_spsCommsData)
	{
		_hasCommunicationError |= !pplSendProcessingBegin(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

		if (!_hasCommunicationError)
		{
			// mark every N frames
			if (!(_frameCounter % 100))
			{
				char buf[128];
				const int nLen = sprintf(buf, "frame %u", _frameCounter);
				_hasCommunicationError |= !pplSendMark(_spsCommsData, buf, nLen);
			}

			// Check for dirty items
			_hasCommunicationError |= !pplSendProcessingBegin(_spsCommsData, "dirty", static_cast<uint32_t>(strlen("dirty")), _frameCounter);
			{
				uint32_t nItem, nNewDataLen;
				const char* pData;
				while (pplLibraryDirtyGetFirst(_spsCommsData, &nItem, &nNewDataLen, &pData))
				{
					Log(LogLevel::Debug, "dirty item %u %u 0x%08x\n", nItem, nNewDataLen, pData);
					switch (nItem)
					{
					case 0:
						if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
						{
							const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
							_uboMatData.specularExponent = psData->fCurrent;
							_uboMatData.isDirty = true;
							Log(LogLevel::Information, "Setting Specular Exponent to value [%6.2f]", _uboMatData.specularExponent);
						}
						break;
					case 1:
						if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
						{
							const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
							_uboMatData.metallicity = psData->fCurrent;
							_uboMatData.isDirty = true;
							Log(LogLevel::Information, "Setting Metallicity to value [%3.2f]", _uboMatData.metallicity);
						}
						break;
					case 2:
						if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
						{
							const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
							_uboMatData.reflectivity = psData->fCurrent;
							_uboMatData.isDirty = true;
							Log(LogLevel::Information, "Setting Reflectivity to value [%3.2f]", _uboMatData.reflectivity);
						}
						break;
					case 3:
						if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
						{
							const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
							_uboMatData.albedo.r = psData->fCurrent;
							_uboMatData.isDirty = true;
							Log(LogLevel::Information, "Setting Albedo Red channel to value [%3.2f]", _uboMatData.albedo.r);
						}
						break;
					case 4:
						if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
						{
							const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
							_uboMatData.albedo.g = psData->fCurrent;
							_uboMatData.isDirty = true;
							Log(LogLevel::Information, "Setting Albedo Green channel to value [%3.2f]", _uboMatData.albedo.g);
						}
						break;
					case 5:
						if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
						{
							const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
							_uboMatData.albedo.b = psData->fCurrent;
							_uboMatData.isDirty = true;
							Log(LogLevel::Information, "Setting Albedo Blue channel to value [%3.2f]", _uboMatData.albedo.b);
						}
						break;
					}
				}
			}
			_hasCommunicationError |= !pplSendProcessingEnd(_spsCommsData);
		}
	}

	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameResourcesFences[swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[swapchainIndex]->reset();

	if (_spsCommsData) { _hasCommunicationError |= !pplSendProcessingBegin(_spsCommsData, "draw", static_cast<uint32_t>(strlen("draw")), _frameCounter); }

	updateUbo(swapchainIndex);

	// Set eye position in model space
	// Now that the uniforms are set, call another function to actually draw the mesh.
	if (_spsCommsData)
	{
		_hasCommunicationError |= !pplSendProcessingEnd(_spsCommsData);
		_hasCommunicationError |= !pplSendProcessingBegin(_spsCommsData, "UIRenderer", static_cast<uint32_t>(strlen("UIRenderer")), _frameCounter);
	}

	if (_hasCommunicationError)
	{
		_deviceResources->uiRenderer.getDefaultControls()->setText("Communication Error:\nPVRScopeComms failed\n"
																   "Is PVRPerfServer connected?");
		_deviceResources->uiRenderer.getDefaultControls()->setColor(glm::vec4(.8f, .3f, .3f, 1.0f));
		_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();
		_hasCommunicationError = false;
	}
	else
	{
		_deviceResources->uiRenderer.getDefaultControls()->setText("PVRScope Communication established.");
		_deviceResources->uiRenderer.getDefaultControls()->setColor(glm::vec4(1.f));
		_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();
	}

	if (_spsCommsData) { _hasCommunicationError |= !pplSendProcessingEnd(_spsCommsData); }

	// send counters
	_counterReadings[CounterDefs::Counter] = _frameCounter;
	_counterReadings[CounterDefs::Counter10] = _frame10Counter;
	if (_spsCommsData) { _hasCommunicationError |= !pplCountersUpdate(_spsCommsData, _counterReadings); }

	// update some counters
	++_frameCounter;
	if (0 == (_frameCounter / 10) % 10) { _frame10Counter += 10; }

	// SUBMIT
	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags pipeWaitStageFlags = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.commandBuffers = &_deviceResources->cmdBuffers[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->imageAcquiredSemaphores[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.waitDstStageMask = &pipeWaitStageFlags;
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->queue, _deviceResources->commandPool, _deviceResources->swapchain, swapchainIndex, this->getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	// PRESENT
	pvrvk::PresentInfo presentInfo;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.imageIndices = &swapchainIndex;
	_deviceResources->queue->present(presentInfo);

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	if (_spsCommsData)
	{
		_hasCommunicationError |= !pplSendProcessingEnd(_spsCommsData);
		_hasCommunicationError |= !pplSendFlush(_spsCommsData);
	}

	return pvr::Result::Success;
}

void VulkanPVRScopeRemote::createDescriptorSetLayouts()
{
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1u, pvrvk::ShaderStageFlags::e_VERTEX_BIT);
		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetInfo.setBinding(2, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		_deviceResources->modelDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
	}

	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		_deviceResources->lightingDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
	}
}

/// <summary>Creates the graphics pipeline used in the demo.</summary>
void VulkanPVRScopeRemote::createPipeline()
{
	// Mapping of mesh semantic names to shader variables
	pvr::utils::VertexBindings_Name vertexBindings[] = { { "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoord" } };

	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
	pipeLayoutInfo.setDescSetLayout(0, _deviceResources->modelDescriptorSetLayout);
	pipeLayoutInfo.setDescSetLayout(1, _deviceResources->lightingDescriptorSetLayout);
	_deviceResources->pipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	pvrvk::GraphicsPipelineCreateInfo pipeDesc;
	pipeDesc.pipelineLayout = _deviceResources->pipelineLayout;

	/* Load and compile the shaders from files. */
	pipeDesc.vertexShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(VertShaderSrcFile)->readToEnd<uint32_t>())));
	pipeDesc.fragmentShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(FragShaderSrcFile)->readToEnd<uint32_t>())));

	pvr::utils::populateViewportStateCreateInfo(_deviceResources->onScreenFramebuffer[0], pipeDesc.viewport);
	pipeDesc.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);
	pipeDesc.depthStencil.enableDepthTest(true);
	pipeDesc.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS);
	pipeDesc.depthStencil.enableDepthWrite(true);
	pipeDesc.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
	pipeDesc.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
	pvr::utils::populateInputAssemblyFromMesh(_scene->getMesh(0), vertexBindings, 3, pipeDesc.vertexInput, pipeDesc.inputAssembler);

	_deviceResources->pipeline = _deviceResources->device->createGraphicsPipeline(pipeDesc, _deviceResources->pipelineCache);
}

/// <summary>Loads the mesh data required for this training course into vertex buffer objects.</summary>
void VulkanPVRScopeRemote::loadVbos(pvrvk::CommandBuffer& uploadCmd)
{
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	//	Load vertex data of all meshes in the scene into VBOs
	//	The meshes have been exported with the "Interleave Vectors" option,
	//	so all data is interleaved in the buffer at pMesh->pInterleaved.
	//	Interleaving data improves the memory access pattern and cache efficiency,
	//	thus it can be read faster by the hardware.
	bool requiresCommandBufferSubmission = false;
	pvr::utils::appendSingleBuffersFromModel(
		_deviceResources->device, *_scene, _deviceResources->vbos, _deviceResources->ibos, uploadCmd, requiresCommandBufferSubmission, _deviceResources->vmaAllocator);
}

/// <summary>Draws a assets::Mesh after the model view matrix has been set and the material prepared.</summary>
/// <param name="nodeIndex">Node index of the mesh to draw.</param>
void VulkanPVRScopeRemote::drawMesh(int nodeIndex, pvrvk::CommandBuffer& command)
{
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	const int32_t meshIndex = _scene->getNode(nodeIndex).getObjectId();
	const pvr::assets::Mesh& mesh = _scene->getMesh(meshIndex);
	// bind the VBO for the mesh
	command->bindVertexBuffer(_deviceResources->vbos[meshIndex], 0, 0);

	//	The geometry can be exported in 4 ways:
	//	- Indexed Triangle list
	//	- Non-Indexed Triangle list
	//	- Indexed Triangle strips
	//	- Non-Indexed Triangle strips
	if (mesh.getNumStrips() == 0)
	{
		if (_deviceResources->ibos[meshIndex])
		{
			// Indexed Triangle list
			command->bindIndexBuffer(_deviceResources->ibos[meshIndex], 0, pvrvk::IndexType::e_UINT16);
			command->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
		}
		else
		{
			// Non-Indexed Triangle list
			command->draw(0, mesh.getNumFaces(), 0, 1);
		}
	}
	else
	{
		for (uint32_t i = 0; i < mesh.getNumStrips(); ++i)
		{
			int offset = 0;
			if (_deviceResources->ibos[static_cast<size_t>(meshIndex)])
			{
				command->bindIndexBuffer(_deviceResources->ibos[meshIndex], 0, pvrvk::IndexType::e_UINT16);

				// Indexed Triangle strips
				command->drawIndexed(0, mesh.getStripLength(i) + 2, offset * 2, 0, 1);
			}
			else
			{
				// Non-Indexed Triangle strips
				command->draw(0, mesh.getStripLength(i) + 2, 0, 1);
			}
			offset += mesh.getStripLength(i) + 2;
		}
	}
}

void VulkanPVRScopeRemote::createDescriptorSet()
{
	const uint32_t swapchainLength = _deviceResources->swapchain->getSwapchainLength();
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	// create the MVP ubo
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement(BufferEntryNames::Matrices::MVPMatrix, pvr::GpuDatatypes::mat4x4);
		desc.addElement(BufferEntryNames::Matrices::MVInverseTransposeMatrix, pvr::GpuDatatypes::mat3x3);

		_deviceResources->uboMatricesBufferView.initDynamic(desc, swapchainLength, pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

		_deviceResources->uboMatrices = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->uboMatricesBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->uboMatricesBufferView.pointToMappedMemory(_deviceResources->uboMatrices->getDeviceMemory()->getMappedData());
	}

	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.minFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
	samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
	pvrvk::Sampler trilinearSampler = _deviceResources->device->createSampler(samplerInfo);

	std::vector<pvrvk::WriteDescriptorSet> descSetWrites;

	for (uint32_t i = 0; i < swapchainLength; ++i)
	{
		_deviceResources->modelDescriptorSets[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->modelDescriptorSetLayout);

		descSetWrites.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->modelDescriptorSets[i], 0)
									.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->uboMatrices, 0, _deviceResources->uboMatricesBufferView.getDynamicSliceSize())));

		descSetWrites.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->modelDescriptorSets[i], 1)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->texture, trilinearSampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		descSetWrites.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->modelDescriptorSets[i], 2)
									.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->uboMaterial, 0, _deviceResources->uboMaterialBufferView.getDynamicSliceSize())));
	}

	_deviceResources->lightingDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->lightingDescriptorSetLayout);
	descSetWrites.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->lightingDescriptorSet, 0)
								.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->uboLighting, 0, _deviceResources->uboLightingBufferView.getSize())));

	_deviceResources->device->updateDescriptorSets(descSetWrites.data(), static_cast<uint32_t>(descSetWrites.size()), nullptr, 0);
}

void VulkanPVRScopeRemote::updateUbo(uint32_t swapchain)
{
	// Rotate and Translation the model matrix
	const glm::mat4 modelMtx = glm::rotate(_angleY, glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.6f)) * _scene->getWorldMatrix(0);
	_angleY += (2 * glm::pi<float>() * getFrameTime() / 1000) / 10;

	// Set model view projection matrix
	const glm::mat4 mvMatrix = _viewMtx * modelMtx;

	{
		_deviceResources->uboMatricesBufferView.getElementByName(BufferEntryNames::Matrices::MVPMatrix, 0, swapchain).setValue(_projectionMtx * mvMatrix);
		_deviceResources->uboMatricesBufferView.getElementByName(BufferEntryNames::Matrices::MVInverseTransposeMatrix, 0, swapchain)
			.setValue(glm::mat3x4(glm::inverseTranspose(glm::mat3(mvMatrix))));

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(_deviceResources->uboMatrices->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->uboMatrices->getDeviceMemory()->flushRange(
				_deviceResources->uboMatricesBufferView.getDynamicSliceOffset(swapchain), _deviceResources->uboMatricesBufferView.getDynamicSliceSize());
		}
	}

	if (_uboMatData.isDirty)
	{
		_deviceResources->device->waitIdle();
		_deviceResources->uboMaterialBufferView.getElementByName(BufferEntryNames::Materials::AlbedoModulation).setValue(glm::vec4(_uboMatData.albedo, 0.0f));
		_deviceResources->uboMaterialBufferView.getElementByName(BufferEntryNames::Materials::SpecularExponent).setValue(_uboMatData.specularExponent);
		_deviceResources->uboMaterialBufferView.getElementByName(BufferEntryNames::Materials::Metallicity).setValue(_uboMatData.metallicity);
		_deviceResources->uboMaterialBufferView.getElementByName(BufferEntryNames::Materials::Reflectivity).setValue(_uboMatData.reflectivity);
		_uboMatData.isDirty = false;

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(_deviceResources->uboMaterial->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{ _deviceResources->uboMaterial->getDeviceMemory()->flushRange(0, _deviceResources->uboMaterialBufferView.getSize()); }
	}
}

void VulkanPVRScopeRemote::createBuffers()
{
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement(BufferEntryNames::Materials::AlbedoModulation, pvr::GpuDatatypes::vec3);
		desc.addElement(BufferEntryNames::Materials::SpecularExponent, pvr::GpuDatatypes::Float);
		desc.addElement(BufferEntryNames::Materials::Metallicity, pvr::GpuDatatypes::Float);
		desc.addElement(BufferEntryNames::Materials::Reflectivity, pvr::GpuDatatypes::Float);

		_deviceResources->uboMaterialBufferView.init(desc);
		_deviceResources->uboMaterial = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->uboMaterialBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->uboMaterialBufferView.pointToMappedMemory(_deviceResources->uboMaterial->getDeviceMemory()->getMappedData());
	}

	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("viewLightDirection", pvr::GpuDatatypes::vec3);

		_deviceResources->uboLightingBufferView.init(desc);
		_deviceResources->uboLighting = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->uboLightingBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->uboLightingBufferView.pointToMappedMemory(_deviceResources->uboLighting->getDeviceMemory()->getMappedData());
	}
}

/// <summary>pre-record the rendering the commands.</summary>
void VulkanPVRScopeRemote::recordCommandBuffer(uint32_t swapchain)
{
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	_deviceResources->cmdBuffers[swapchain]->begin();
	const pvrvk::ClearValue clearValues[2] = { pvrvk::ClearValue(0.0f, 0.40f, 0.39f, 1.0f), pvrvk::ClearValue(1.f, 0u) };
	_deviceResources->cmdBuffers[swapchain]->beginRenderPass(
		_deviceResources->onScreenFramebuffer[swapchain], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, clearValues, ARRAY_SIZE(clearValues));

	// Use shader program
	_deviceResources->cmdBuffers[swapchain]->bindPipeline(_deviceResources->pipeline);

	// Bind texture
	_deviceResources->cmdBuffers[swapchain]->bindDescriptorSet(
		pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->pipeline->getPipelineLayout(), 0, _deviceResources->modelDescriptorSets[swapchain], 0);
	_deviceResources->cmdBuffers[swapchain]->bindDescriptorSet(
		pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->pipeline->getPipelineLayout(), 1, _deviceResources->lightingDescriptorSet, 0);

	drawMesh(0, _deviceResources->cmdBuffers[swapchain]);

	_deviceResources->uiRenderer.beginRendering(_deviceResources->cmdBuffers[swapchain]);
	// Displays the demo name using the tools. For a detailed explanation, see the example
	// IntroUIRenderer
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.endRendering();
	_deviceResources->cmdBuffers[swapchain]->endRenderPass();
	_deviceResources->cmdBuffers[swapchain]->end();
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanPVRScopeRemote>(); }
