/*!
\brief Shows how to write the vulkan pipeline cache to disk for use later and retrieve it on the next execution.
\file VulkanPipelineCache.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRCore/PVRCore.h"
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"

pvr::utils::VertexBindings Attributes[] = { { "POSITION", 0 }, { "NORMAL", 1 }, { "UV0", 2 } };

// Content file names
const char VertShaderFileName[] = "VertShader.vsh.spv";
const char FragShaderFileName[] = "FragShader.fsh.spv";
const char SceneFileName[] = "GnomeToy.pod"; // POD scene files

const char PipelineCacheFileName[] = "PipelineCache.bin";

typedef std::pair<int32_t, pvrvk::DescriptorSet> MaterialDescSet;
struct DeviceResources
{
	pvrvk::Instance instance;
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
	pvrvk::Surface surface;
	pvrvk::Device device;
	pvrvk::Swapchain swapchain;
	pvr::Multi<pvrvk::ImageView> depthStencilImages;
	pvrvk::Queue queue;

	pvr::utils::vma::Allocator vmaAllocator;

	pvrvk::CommandPool commandPool;
	pvrvk::DescriptorPool descriptorPool;

	pvrvk::Semaphore imageAcquiredSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore presentationSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameResourcesFences[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	// The Vertex buffer object handle array.
	std::vector<pvrvk::Buffer> vbos;
	std::vector<pvrvk::Buffer> ibos;

	// the framebuffer used in the demo
	pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;

	// main command buffer used to store rendering commands
	pvr::Multi<pvrvk::CommandBuffer> cmdBuffers;

	// descriptor sets
	std::vector<MaterialDescSet> texDescSets;
	pvr::Multi<pvrvk::DescriptorSet> matrixUboDescSets;
	pvr::Multi<pvrvk::DescriptorSet> lightUboDescSets;

	// structured memory views
	pvr::utils::StructuredBufferView matrixMemoryView;
	pvrvk::Buffer matrixBuffer;
	pvr::utils::StructuredBufferView lightMemoryView;
	pvrvk::Buffer lightBuffer;

	// samplers
	pvrvk::Sampler samplerTrilinear;

	// descriptor set layouts
	pvrvk::DescriptorSetLayout texDescSetLayout;
	pvrvk::DescriptorSetLayout uboDescSetLayoutDynamic, uboDescSetLayoutStatic;

	// pipeline layout
	pvrvk::PipelineLayout pipelineLayout;

	// graphics pipeline
	pvrvk::GraphicsPipeline pipeline;

	pvrvk::PipelineCache pipelineCache;

	// UIRenderer used to display text
	pvr::ui::UIRenderer uiRenderer;

	~DeviceResources()
	{
		if (device) { device->waitIdle(); }
		uint32_t l = swapchain->getSwapchainLength();
		for (uint32_t i = 0; i < l; ++i)
		{
			if (perFrameResourcesFences[i]) perFrameResourcesFences[i]->wait();
		}
	}
};

/// <summary>Class implementing the pvr::Shell functions.</summary>
class VulkanPipelineCache : public pvr::Shell
{
	std::unique_ptr<DeviceResources> _deviceResources;

	// 3D Model
	pvr::assets::ModelHandle _scene;

	// Projection and Model View matrices
	glm::mat4 _projMtx;
	glm::mat4 _viewMtx;

	// Variables to handle the animation in a time-based manner
	float _frame;

	uint32_t _frameId;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void createBuffers();
	void createDescriptorSets(pvrvk::CommandBuffer& cmdBuffers);
	void recordCommandBuffers();
	void createPipeline();
	void createDescriptorSetLayouts();

	void savePipelineCacheToDisk();
	bool loadPipelineCacheFromDisk();
};

struct DescripotSetComp
{
	int32_t id;
	DescripotSetComp(int32_t id) : id(id) {}
	bool operator()(std::pair<int32_t, pvrvk::DescriptorSet> const& pair) { return pair.first == id; }
};

/// <summary>Saves Pipeline cache data to disk to use for the next execution.</summary>
void VulkanPipelineCache::savePipelineCacheToDisk()
{
	std::vector<char> pipelineCacheBuffer;
	pipelineCacheBuffer.resize(_deviceResources->pipelineCache->getCacheMaxDataSize());

	_deviceResources->pipelineCache->getCacheData(pipelineCacheBuffer.size(), pipelineCacheBuffer.data());

	std::unique_ptr<pvr::Stream> pipelineCacheWriter = getWriteAssetStream(PipelineCacheFileName, true, true);

	pipelineCacheWriter->writeExact(pipelineCacheBuffer.size(), 1, pipelineCacheBuffer.data());
}

/// <summary>Loads Pipeline cache data from disk to saved from an earlier execution.</summary>
/// <returns>Returns false if the binary file was not found on disk, true if it was found.</returns>
bool VulkanPipelineCache::loadPipelineCacheFromDisk()
{
	std::unique_ptr<pvr::Stream> pipelineCacheReader = this->getAssetStream(PipelineCacheFileName, false);

	if (!pipelineCacheReader) { return false; }

	std::vector<char> pipelineCacheBuffer(pipelineCacheReader->readToEnd<char>());

	pvrvk::PipelineCacheCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.setInitialDataSize(pipelineCacheBuffer.size());
	pipelineCreateInfo.setInitialData(pipelineCacheBuffer.data());

	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache(pipelineCreateInfo);

	return true;
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
///	Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.). If the rendering
///	context is lost, initApplication() will not be called again.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanPipelineCache::initApplication()
{
	// Load the _scene
	_scene = pvr::assets::loadModel(*this, SceneFileName);

	// The cameras are stored in the file. We check it contains at least one.
	if (_scene->getNumCameras() == 0) { throw pvr::InvalidDataError("ERROR: The scene does not contain a camera"); }

	// We check the scene contains at least one light
	if (_scene->getNumLights() == 0) { throw pvr::InvalidDataError("The scene does not contain a light\n"); }

	// Ensure that all meshes use an indexed triangle list
	for (uint32_t i = 0; i < _scene->getNumMeshes(); ++i)
	{
		if (_scene->getMesh(i).getPrimitiveType() != pvr::PrimitiveTopology::TriangleList || _scene->getMesh(i).getFaces().getDataSize() == 0)
		{ throw pvr::InvalidDataError("ERROR: The meshes in the scene should use an indexed triangle list\n"); }
	}

	// Initialize variables used for the animation
	_frame = 0;
	_frameId = 0;

	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.
/// If the rendering context is lost, quitApplication() will not be called.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanPipelineCache::quitApplication()
{
	_scene.reset();
	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.).</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanPipelineCache::initView()
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
	auto surface =
		pvr::utils::createSurface(_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(0), this->getWindow(), this->getDisplay(), this->getConnection());

	// Create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance);

	pvr::utils::QueueAccessInfo queueAccessInfo;
	const pvr::utils::QueuePopulateInfo queuePopulateInfo = { pvrvk::QueueFlags::e_GRAPHICS_BIT, surface };

	// Create the device and retrieve its queues
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), &queuePopulateInfo, 1, &queueAccessInfo);

	// Get the queue
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{ swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT; } // Create the swapchain and depth stencil images

	// Create the Swapchain, its renderpass, attachments and framebuffers. Will support MSAA if enabled through command line.
	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));

	_deviceResources->swapchain = swapChainCreateOutput.swapchain;
	_deviceResources->onScreenFramebuffer = swapChainCreateOutput.framebuffer;

	// Create the Command pool & Descriptor pool
	_deviceResources->commandPool =
		_deviceResources->device->createCommandPool(pvrvk::CommandPoolCreateInfo(queueAccessInfo.familyId, pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 16)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 16)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, 16)
																						  .setMaxDescriptorSets(16));

	// create demo buffers
	createBuffers();

	// Create per swapchain resource
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);

		_deviceResources->cmdBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
	}

	_deviceResources->cmdBuffers[0]->begin();
	bool requiresCommandBufferSubmission = false;
	pvr::utils::appendSingleBuffersFromModel(_deviceResources->device, *_scene, _deviceResources->vbos, _deviceResources->ibos, _deviceResources->cmdBuffers[0],
		requiresCommandBufferSubmission, _deviceResources->vmaAllocator);

	// create the descriptor set layouts and pipeline layouts
	createDescriptorSetLayouts();

	// create the descriptor sets
	createDescriptorSets(_deviceResources->cmdBuffers[0]);
	_deviceResources->cmdBuffers[0]->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->cmdBuffers[0];
	submitInfo.numCommandBuffers = 1;

	// submit the queue and wait for it to become idle
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle();

	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue);
	_deviceResources->uiRenderer.getDefaultTitle()->setText("PipelineCache").commitUpdates();
	_deviceResources->cmdBuffers[0]->reset(pvrvk::CommandBufferResetFlags::e_RELEASE_RESOURCES_BIT);

	if (!loadPipelineCacheFromDisk())
	{
		// Create the pipeline cache
		_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();
		// Create the pipeline
		createPipeline();
		// Save the pipeline to disk in binary format
		savePipelineCacheToDisk();
	}
	else
	{
		// Create the pipeline
		createPipeline();
	}

	// record the rendering commands
	recordCommandBuffers();

	// Calculates the projection matrix
	bool isRotated = this->isScreenRotated();
	if (isRotated)
	{
		_projMtx = pvr::math::perspective(pvr::Api::Vulkan, _scene->getCamera(0).getFOV(), static_cast<float>(this->getHeight()) / static_cast<float>(this->getWidth()),
			_scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(), glm::pi<float>() * .5f);
	}
	else
	{
		_projMtx = pvr::math::perspective(pvr::Api::Vulkan, _scene->getCamera(0).getFOV(), static_cast<float>(this->getWidth()) / static_cast<float>(this->getHeight()),
			_scene->getCamera(0).getNear(), _scene->getCamera(0).getFar());
	}

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanPipelineCache::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanPipelineCache::renderFrame()
{
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameResourcesFences[swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[swapchainIndex]->reset();
	pvr::assets::AnimationInstance& animInst = _scene->getAnimationInstance(0);

	//  Calculates the _frame number to animate in a time-based manner.
	//  get the time in milliseconds.
	_frame += static_cast<float>(getFrameTime()); // design-time target fps for animation

	if (_frame >= animInst.getTotalTimeInMs()) { _frame = 0; }

	// Sets the _scene animation to this _frame
	animInst.updateAnimation(_frame);

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
	float fov;
	glm::vec3 cameraPos, cameraTarget, cameraUp;
	_scene->getCameraProperties(0, fov, cameraPos, cameraTarget, cameraUp);
	_viewMtx = glm::lookAt(cameraPos, cameraTarget, cameraUp);

	{
		// update the matrix uniform buffer
		glm::mat4 tempMtx;
		for (uint32_t i = 0; i < _scene->getNumMeshNodes(); ++i)
		{
			uint32_t dynamicSlice = i + swapchainIndex * _scene->getNumMeshNodes();
			tempMtx = _viewMtx * _scene->getWorldMatrix(i);
			_deviceResources->matrixMemoryView.getElementByName("MVP", 0, dynamicSlice).setValue(_projMtx * tempMtx);
			_deviceResources->matrixMemoryView.getElementByName("WorldViewItMtx", 0, dynamicSlice).setValue(glm::inverseTranspose(glm::mat3x3(tempMtx)));
		}

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(_deviceResources->matrixBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->matrixBuffer->getDeviceMemory()->flushRange(_deviceResources->matrixMemoryView.getDynamicSliceOffset(swapchainIndex * _scene->getNumMeshNodes()),
				_deviceResources->matrixMemoryView.getDynamicSliceSize() * _scene->getNumMeshNodes());
		}
	}

	{
		// update the light direction ubo
		glm::vec3 lightDir3;
		_scene->getLightDirection(0, lightDir3);
		lightDir3 = glm::normalize(glm::mat3(_viewMtx) * lightDir3);
		_deviceResources->lightMemoryView.getElementByName("LightDirection", 0, swapchainIndex).setValue(glm::vec4(lightDir3, 1.f));

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(_deviceResources->lightBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->lightBuffer->getDeviceMemory()->flushRange(
				_deviceResources->lightMemoryView.getDynamicSliceOffset(swapchainIndex), _deviceResources->lightMemoryView.getDynamicSliceSize());
		}
	}

	// Submit
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

	// Present
	pvrvk::PresentInfo presentInfo;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.imageIndices = &swapchainIndex;
	_deviceResources->queue->present(presentInfo);

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	return pvr::Result::Success;
}

/// <summary>Pre-record the commands.</summary>
void VulkanPipelineCache::recordCommandBuffers()
{
	glm::vec3 clearColorLinearSpace(0.0f, 0.45f, 0.41f);

	pvrvk::ClearValue clearValues[2] = { pvrvk::ClearValue(clearColorLinearSpace.x, clearColorLinearSpace.y, clearColorLinearSpace.z, 1.0f), pvrvk::ClearValue(1.f, 0u) };
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		// begin recording commands
		_deviceResources->cmdBuffers[i]->begin();

		// begin the renderpass
		_deviceResources->cmdBuffers[i]->beginRenderPass(
			_deviceResources->onScreenFramebuffer[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, clearValues, ARRAY_SIZE(clearValues));

		// bind the graphics pipeline
		_deviceResources->cmdBuffers[i]->bindPipeline(_deviceResources->pipeline);

		// A scene is composed of nodes. There are 3 types of nodes:
		// - MeshNodes :
		// references a mesh in the getMesh().
		// These nodes are at the beginning of the Nodes array.
		// And there are nNumMeshNode number of them.
		// This way the .pod format can instantiate several times the same mesh
		// with different attributes.
		// - lights
		// - cameras
		// To draw a scene, you must go through all the MeshNodes and draw the referenced meshes.
		uint32_t offsets[2];
		offsets[0] = 0;
		offsets[1] = 0;

		pvrvk::DescriptorSet descriptorSets[3];
		descriptorSets[1] = _deviceResources->matrixUboDescSets[i];
		descriptorSets[2] = _deviceResources->lightUboDescSets[i];
		for (uint32_t j = 0; j < _scene->getNumMeshNodes(); ++j)
		{
			// get the current mesh node
			const pvr::assets::Model::Node* pNode = &_scene->getMeshNode(j);

			// Gets pMesh referenced by the pNode
			const pvr::assets::Mesh* pMesh = &_scene->getMesh(pNode->getObjectId());

			// get the material id
			int32_t matId = pNode->getMaterialIndex();

			// find the texture descriptor set which matches the current material
			auto found = std::find_if(_deviceResources->texDescSets.begin(), _deviceResources->texDescSets.end(), DescripotSetComp(matId));
			descriptorSets[0] = found->second;

			// get the matrix buffer array offset
			offsets[0] = _deviceResources->matrixMemoryView.getDynamicSliceOffset(j + i * _scene->getNumMeshNodes());
			offsets[1] = _deviceResources->lightMemoryView.getDynamicSliceOffset(i);

			// bind the descriptor sets
			_deviceResources->cmdBuffers[i]->bindDescriptorSets(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->pipelineLayout, 0, descriptorSets, 3, offsets, 2);

			// bind the vbo and ibos for the current mesh node
			_deviceResources->cmdBuffers[i]->bindVertexBuffer(_deviceResources->vbos[pNode->getObjectId()], 0, 0);
			_deviceResources->cmdBuffers[i]->bindIndexBuffer(_deviceResources->ibos[pNode->getObjectId()], 0, pvr::utils::convertToPVRVk(pMesh->getFaces().getDataType()));

			// draw
			_deviceResources->cmdBuffers[i]->drawIndexed(0, pMesh->getNumFaces() * 3, 0, 0, 1);
		}

		// add ui effects using ui renderer
		_deviceResources->uiRenderer.beginRendering(_deviceResources->cmdBuffers[i]);
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.endRendering();
		_deviceResources->cmdBuffers[i]->endRenderPass();
		_deviceResources->cmdBuffers[i]->end();
	}
}

/// <summary>Creates the descriptor set layouts used throughout the demo.</summary>
void VulkanPipelineCache::createDescriptorSetLayouts()
{
	// create the texture descriptor set layout and pipeline layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		_deviceResources->texDescSetLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
	}

	// create the ubo descriptor set layouts
	{
		// dynamic ubo
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT); /*binding 0*/
		_deviceResources->uboDescSetLayoutDynamic = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
	}
	{
		// static ubo
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT); /*binding 0*/
		_deviceResources->uboDescSetLayoutStatic = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
	}

	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(_deviceResources->texDescSetLayout); /* set 0 */
	pipeLayoutInfo.addDescSetLayout(_deviceResources->uboDescSetLayoutDynamic); /* set 1 */
	pipeLayoutInfo.addDescSetLayout(_deviceResources->uboDescSetLayoutStatic); /* set 2 */
	_deviceResources->pipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
}

/// <summary>Creates the graphics pipeline used in the demo.</summary>
void VulkanPipelineCache::createPipeline()
{
	pvrvk::GraphicsPipelineCreateInfo pipeDesc;
	pipeDesc.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
	pipeDesc.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);
	pvr::utils::populateViewportStateCreateInfo(_deviceResources->onScreenFramebuffer[0], pipeDesc.viewport);
	pvr::utils::populateInputAssemblyFromMesh(_scene->getMesh(0), Attributes, 3, pipeDesc.vertexInput, pipeDesc.inputAssembler);

	std::unique_ptr<pvr::Stream> vertSource = getAssetStream(VertShaderFileName);
	std::unique_ptr<pvr::Stream> fragSource = getAssetStream(FragShaderFileName);

	pipeDesc.vertexShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(vertSource->readToEnd<uint32_t>())));
	pipeDesc.fragmentShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(fragSource->readToEnd<uint32_t>())));

	pipeDesc.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
	pipeDesc.depthStencil.enableDepthTest(true);
	pipeDesc.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS);
	pipeDesc.depthStencil.enableDepthWrite(true);
	pipeDesc.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);
	pipeDesc.subpass = 0;

	pipeDesc.pipelineLayout = _deviceResources->pipelineLayout;

	_deviceResources->pipeline = _deviceResources->device->createGraphicsPipeline(pipeDesc, _deviceResources->pipelineCache);
}

/// <summary>Creates the buffers used throughout the demo.</summary>
void VulkanPipelineCache::createBuffers()
{
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("MVP", pvr::GpuDatatypes::mat4x4);
		desc.addElement("WorldViewItMtx", pvr::GpuDatatypes::mat3x3);

		_deviceResources->matrixMemoryView.initDynamic(desc, _scene->getNumMeshNodes() * _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
		_deviceResources->matrixBuffer = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->matrixMemoryView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
		_deviceResources->matrixMemoryView.pointToMappedMemory(_deviceResources->matrixBuffer->getDeviceMemory()->getMappedData());
	}

	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("LightDirection", pvr::GpuDatatypes::vec4);

		_deviceResources->lightMemoryView.initDynamic(desc, _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
		_deviceResources->lightBuffer = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->lightMemoryView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
		_deviceResources->lightMemoryView.pointToMappedMemory(_deviceResources->lightBuffer->getDeviceMemory()->getMappedData());
	}
}

/// <summary>Create combined texture and sampler descriptor set for the materials in the _scene.</summary>
/// <returns>Return true on success.</returns>
void VulkanPipelineCache::createDescriptorSets(pvrvk::CommandBuffer& cmdBuffers)
{
	// create the sampler object
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.minFilter = samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
	samplerInfo.wrapModeU = samplerInfo.wrapModeV = pvrvk::SamplerAddressMode::e_REPEAT;
	_deviceResources->samplerTrilinear = _deviceResources->device->createSampler(samplerInfo);

	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;
	for (uint32_t i = 0; i < _scene->getNumMaterials(); ++i)
	{
		if (_scene->getMaterial(i).defaultSemantics().getDiffuseTextureIndex() == static_cast<uint32_t>(-1)) { continue; }

		MaterialDescSet matDescSet = std::make_pair(i, _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->texDescSetLayout));
		_deviceResources->texDescSets.push_back(matDescSet);

		writeDescSets.push_back(pvrvk::WriteDescriptorSet());
		pvrvk::WriteDescriptorSet& writeDescSet = writeDescSets.back();
		writeDescSet.set(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, matDescSet.second, 0);
		const pvr::assets::Model::Material& material = _scene->getMaterial(i);

		// Load the diffuse texture map
		const char* fileName = _scene->getTexture(material.defaultSemantics().getDiffuseTextureIndex()).getName().c_str();

		pvrvk::ImageView diffuseMap = pvr::utils::loadAndUploadImageAndView(_deviceResources->device, fileName, true, cmdBuffers, *this, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);

		writeDescSet.setImageInfo(0, pvrvk::DescriptorImageInfo(diffuseMap, _deviceResources->samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	}

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->lightUboDescSets.add(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboDescSetLayoutStatic));
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->lightUboDescSets[i], 0)
									.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->lightBuffer, 0, _deviceResources->lightMemoryView.getDynamicSliceSize())));

		_deviceResources->matrixUboDescSets.add(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboDescSetLayoutDynamic));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->matrixUboDescSets[i], 0));
		pvrvk::WriteDescriptorSet& writeDescSet = writeDescSets.back();
		writeDescSet.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->matrixBuffer, 0, _deviceResources->matrixMemoryView.getDynamicSliceSize()));
	}

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanPipelineCache>(); }
