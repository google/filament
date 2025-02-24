/*!
\brief Shows how to use the PVRApi library together with loading models from POD files and rendering them with effects from PFX files.
\file VulkanMultiSampling.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRCore/PVRCore.h"
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"

const pvrvk::SampleCountFlags NumSamples = pvrvk::SampleCountFlags::e_4_BIT;
pvr::utils::VertexBindings Attributes[] = { { "POSITION", 0 }, { "NORMAL", 1 }, { "UV0", 2 } };

// Content file names
const char VertShaderFileName[] = "VertShader.vsh.spv";
const char FragShaderFileName[] = "FragShader.fsh.spv";
const char SceneFileName[] = "GnomeToy.pod"; // POD scene files

typedef std::pair<int32_t, pvrvk::DescriptorSet> MaterialDescSet;
struct DeviceResources
{
	pvrvk::Instance instance;
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
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
	pvrvk::GraphicsPipeline uiPipeline;

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

/// <summary>Class implementing the pvr::Shell functions.</summary>
class VulkanMultiSampling : public pvr::Shell
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

	void createMultiSampleFramebufferAndRenderPass();
	void createBuffers();
	void createDescriptorSets(pvrvk::CommandBuffer& cmdBuffers);
	void recordCommandBuffers();
	void createPipeline();
	void createDescriptorSetLayouts();
};

struct DescripotSetComp
{
	int32_t id;
	DescripotSetComp(int32_t id) : id(id) {}
	bool operator()(std::pair<int32_t, pvrvk::DescriptorSet> const& pair) { return pair.first == id; }
};

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it(e.g.external modules, loading meshes, etc.).If the rendering
/// context is lost, initApplication() will not be called again.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanMultiSampling::initApplication()
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

/// <summary>Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.</summary>
/// <returns>Result::Success if no error occurred</returns>.
pvr::Result VulkanMultiSampling::quitApplication()
{
	_scene.reset();
	return pvr::Result::Success;
}

void VulkanMultiSampling::createMultiSampleFramebufferAndRenderPass()
{
	// Create the Framebuffer with the following configurations
	// Attachment 0: MultiSample Colour
	// Attachment 1: MultiSample DepthStencil
	// Attachment 2: Swapchain Colour (Resolve)
	// Attachment 3: DepthStencil  (Resolve)
	// Subpass 0: Renders in to the Multisample attachments(0,1) and then resolve in to the final image(2,3)

	pvrvk::Format msColorDsFmt[] = {
		_deviceResources->swapchain->getImageFormat(), // colour
		_deviceResources->depthStencilImages[0]->getImage()->getFormat() // depth stencil
	};

	// set up the renderpass.
	pvrvk::SubpassDescription subpass;

	// We need two subpass dependency here.
	// First dependency does the image memory barrier before a render pass and its only subpass.
	// It Transition the image from memory access(from presentation engine) to colour read and write operation.
	//
	// The second dependency is defined for operations occurring inside a subpass and after the render pass.
	// it transition the barrier from colour read/ write operation to memory read so the presentation engine can read
	// them.
	pvrvk::SubpassDependency dependencies[2] = {
		pvrvk::SubpassDependency(pvrvk::SubpassExternal, 0, pvrvk::PipelineStageFlags::e_BOTTOM_OF_PIPE_BIT, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
			pvrvk::AccessFlags::e_NONE, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_READ_BIT | pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::DependencyFlags::e_BY_REGION_BIT),

		pvrvk::SubpassDependency(0, pvrvk::SubpassExternal, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_BOTTOM_OF_PIPE_BIT,
			pvrvk::AccessFlags::e_COLOR_ATTACHMENT_READ_BIT | pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_NONE, pvrvk::DependencyFlags::e_BY_REGION_BIT)
	};

	// multi sample colour attachment
	subpass.setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	// multi sample depth stencil attachment
	subpass.setDepthStencilAttachmentReference(pvrvk::AttachmentReference(1, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));

	// resolve colour attachment == presentation image
	subpass.setResolveAttachmentReference(0, pvrvk::AttachmentReference(2, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));

	// resolve depth stencil attachment
	subpass.setResolveAttachmentReference(1, pvrvk::AttachmentReference(3, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));

	pvrvk::RenderPassCreateInfo rpInfo;
	// The image will get  resolved in to the final swapchain image, so don't care about the store.
	rpInfo.setAttachmentDescription(0,
		pvrvk::AttachmentDescription::createColorDescription(msColorDsFmt[0], pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL,
			pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE, NumSamples));

	rpInfo.setAttachmentDescription(1,
		pvrvk::AttachmentDescription::createDepthStencilDescription(msColorDsFmt[1], pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE, NumSamples));

	// We don't care about the load op since they will get overridden during resolving.
	rpInfo.setAttachmentDescription(2,
		pvrvk::AttachmentDescription::createColorDescription(
			msColorDsFmt[0], pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_PRESENT_SRC_KHR, pvrvk::AttachmentLoadOp::e_DONT_CARE, pvrvk::AttachmentStoreOp::e_STORE));

	// resolving depth-stencil image we render.
	rpInfo.setAttachmentDescription(3,
		pvrvk::AttachmentDescription::createDepthStencilDescription(msColorDsFmt[1], pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			pvrvk::AttachmentLoadOp::e_DONT_CARE, pvrvk::AttachmentStoreOp::e_DONT_CARE, pvrvk::AttachmentLoadOp::e_DONT_CARE, pvrvk::AttachmentStoreOp::e_DONT_CARE));

	rpInfo.setSubpass(0, subpass);
	rpInfo.addSubpassDependencies(dependencies, 2);

	// create the renderpass
	pvrvk::RenderPass renderPass = _deviceResources->device->createRenderPass(rpInfo);

	// create the framebuffer
	pvr::Multi<pvrvk::FramebufferCreateInfo> framebufferInfo;
	const pvrvk::Extent3D& dimension = pvrvk::Extent3D(_deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight(), 1u);
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		pvrvk::FramebufferCreateInfo& info = framebufferInfo[i];
		// allocate the multisample colour and depth stencil attachment
		// colour attachment. The attachment will transient
		pvrvk::ImageView msColor = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(pvr::utils::createImage(_deviceResources->device,
			pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, msColorDsFmt[0], dimension,
				pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT, 1, 1, NumSamples),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT)));

		// depth stencil attachment. The attachment will be transient
		pvrvk::ImageView msDs = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(pvr::utils::createImage(_deviceResources->device,
			pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, msColorDsFmt[1], dimension,
				pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT, 1, 1, NumSamples),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT)));

		pvrvk::ImageView ds = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(pvr::utils::createImage(_deviceResources->device,
			pvrvk::ImageCreateInfo(
				pvrvk::ImageType::e_2D, msColorDsFmt[1], dimension, pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT)));

		info.setAttachment(0, msColor);
		info.setAttachment(1, msDs);
		info.setAttachment(2, _deviceResources->swapchain->getImageView(i));
		info.setAttachment(3, ds);
		info.setRenderPass(renderPass);
		info.setDimensions(_deviceResources->swapchain->getDimension());
		_deviceResources->onScreenFramebuffer[i] = _deviceResources->device->createFramebuffer(info);
	}
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change  in the rendering context. Used to initialize variables that are dependent on the
/// rendering context(e.g.textures, vertex buffers, etc.).</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanMultiSampling::initView()
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

	pvr::utils::QueueAccessInfo queueAccessInfo;
	const pvr::utils::QueuePopulateInfo queuePopulateInfo = { pvrvk::QueueFlags::e_GRAPHICS_BIT, surface };

	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), &queuePopulateInfo, 1, &queueAccessInfo);

	// Get the queue
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{ swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT; } // Create the swapchain and depth stencil images

	_deviceResources->swapchain = pvr::utils::createSwapchain(_deviceResources->device, surface, getDisplayAttributes(), swapchainImageUsage);

	pvr::utils::createAttachmentImages(_deviceResources->depthStencilImages, _deviceResources->device, _deviceResources->swapchain->getSwapchainLength(),
		pvr::utils::getSupportedDepthStencilFormat(_deviceResources->device, getDisplayAttributes()), _deviceResources->swapchain->getDimension(),
		pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT, pvrvk::SampleCountFlags::e_1_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT, "DepthStencilBufferImages");

	createMultiSampleFramebufferAndRenderPass();

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

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("MultiSampling").commitUpdates();

	// create demo graphics pipeline
	createPipeline();

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

/// <summary>Code in releaseView() will be called by Shell when the application quits.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanMultiSampling::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanMultiSampling::renderFrame()
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

/// <summary>Pre-record the rendering commands.</sumamry>
void VulkanMultiSampling::recordCommandBuffers()
{
	pvrvk::ClearValue clearValues[2] = { pvrvk::ClearValue(0.0f, 0.40f, .39f, 1.0f), pvrvk::ClearValue(1.f, 0u) };
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
void VulkanMultiSampling::createDescriptorSetLayouts()
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
void VulkanMultiSampling::createPipeline()
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
	pipeDesc.multiSample.setNumRasterizationSamples(NumSamples);

	pipeDesc.pipelineLayout = _deviceResources->pipelineLayout;

	_deviceResources->pipeline = _deviceResources->device->createGraphicsPipeline(pipeDesc, _deviceResources->pipelineCache);
}

/// <summary>Creates the buffers used throughout the demo.</summary>
void VulkanMultiSampling::createBuffers()
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
void VulkanMultiSampling::createDescriptorSets(pvrvk::CommandBuffer& cmdBuffers)
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
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanMultiSampling>(); }
