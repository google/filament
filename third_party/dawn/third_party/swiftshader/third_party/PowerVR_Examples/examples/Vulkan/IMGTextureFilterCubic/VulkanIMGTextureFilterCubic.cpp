/*!
\brief Shows how to use the PowerVR device extension VK_IMG_filter_cubic.
\file VulkanIMGTextureFilterCubic.cpp
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

struct DeviceResources
{
	pvrvk::Instance instance;
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
	pvrvk::Device device;
	pvrvk::Swapchain swapchain;
	pvrvk::CommandPool cmdPool;
	pvrvk::DescriptorPool descriptorPool;
	pvrvk::Queue queue;
	pvr::utils::vma::Allocator vmaAllocator;
	pvrvk::Semaphore imageAcquiredSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore presentationSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameResourcesFences[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	// The Vertex buffer object handle array.
	pvrvk::Buffer quadVbo;

	// the framebuffer used in the demo
	pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;

	// main command buffer used to store rendering commands
	pvr::Multi<pvrvk::CommandBuffer> cmdBuffers;

	// Command buffer used to upload data to the GPU
	pvrvk::CommandBuffer uploadCmdBuffer;

	pvrvk::ImageView baseImageView;

	// descriptor sets
	pvrvk::DescriptorSet textureDescriptorSet;

	// descriptor set layouts
	pvrvk::DescriptorSetLayout texDescriptorSetLayout;

	// pipeline layout
	pvrvk::PipelineLayout pipelineLayout;

	// graphics pipeline
	pvrvk::GraphicsPipeline pipeline;

	pvrvk::PipelineCache pipelineCache;

	pvrvk::Sampler linearSampler;
	pvrvk::Sampler cubicSampler;

	// UIRenderer used to display text
	pvr::ui::UIRenderer uiRenderer;

	~DeviceResources()
	{
		if (device) { device->waitIdle(); }
		if (swapchain)
		{
			uint32_t l = swapchain->getSwapchainLength();
			for (uint32_t i = 0; i < l; ++i)
			{
				if (perFrameResourcesFences[i]) perFrameResourcesFences[i]->wait();
			}
		}
	}
};

/// <summary>implementing the pvr::Shell functions.</summary>
class VulkanIMGTextureFilterCubic : public pvr::Shell
{
	std::unique_ptr<DeviceResources> _deviceResources;

	std::vector<glm::vec3> _vertices;

	uint32_t _frameId;

	glm::vec3 _clearColor;

	glm::mat4 _projection;
	glm::mat4 _viewProjection;
	glm::mat4 _modelViewProjection;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void createDescriptorSet();
	void recordCommandBuffers();
	void createPipeline();
	void createDescriptorSetLayout();
	void createTextures();
	void loadVbo();
};

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanIMGTextureFilterCubic::initApplication()
{
	_frameId = 0;

	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
///	If the rendering context is lost, quitApplication() will not be called.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanIMGTextureFilterCubic::quitApplication() { return pvr::Result::Success; }

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.).</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanIMGTextureFilterCubic::initView()
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

	// Add the device extension VK_IMG_filter_cubic to the list of device extensions which will be enabled if supported
	pvr::utils::DeviceExtensions deviceExtensions = pvr::utils::DeviceExtensions();
	deviceExtensions.addExtension(VK_IMG_FILTER_CUBIC_EXTENSION_NAME);

	// Create the device and retrieve its queues
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), &queuePopulateInfo, 1, &queueAccessInfo, deviceExtensions);

	// Determine whether there is support for VK_IMG_filter_cubic
	if (!_deviceResources->device->getEnabledExtensionTable().imgFilterCubicEnabled) { throw pvrvk::ErrorExtensionNotPresent(VK_IMG_FILTER_CUBIC_EXTENSION_NAME); }

	// Support has been found for the device extension VK_IMG_filter_cubic. We can now make use of cubic filtering as will be shown in this example

	// Get the queue
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{ swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT; } // Create the swapchain
	// Create the Swapchain, its renderpass, attachments and framebuffers. Will support MSAA if enabled through command line.
	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage).enableDepthBuffer(false));

	_deviceResources->swapchain = swapChainCreateOutput.swapchain;
	_deviceResources->onScreenFramebuffer = swapChainCreateOutput.framebuffer;

	// Create the Command pool & Descriptor pool
	_deviceResources->cmdPool =
		_deviceResources->device->createCommandPool(pvrvk::CommandPoolCreateInfo(queueAccessInfo.familyId, pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(
		pvrvk::DescriptorPoolCreateInfo().addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 16).setMaxDescriptorSets(16));

	// Create per swapchain resource
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);

		_deviceResources->cmdBuffers[i] = _deviceResources->cmdPool->allocateCommandBuffer();
	}

	_deviceResources->uploadCmdBuffer = _deviceResources->cmdPool->allocateCommandBuffer();

	// create the descriptor set layouts and pipeline layouts
	createDescriptorSetLayout();

	_deviceResources->uploadCmdBuffer->begin();

	loadVbo();

	// Create the texture which will be sampled from using a sampler with magnification filter of pvrvk::Filter::e_CUBIC_IMG using VK_IMG_filter_cubic
	createTextures();

	_deviceResources->uploadCmdBuffer->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->uploadCmdBuffer;
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle();

	// create the descriptor sets
	createDescriptorSet();

	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->cmdPool, _deviceResources->queue);
	_deviceResources->uiRenderer.getDefaultTitle()->setText("IMGTextureFilterCubic").commitUpdates();
	_deviceResources->uiRenderer.getDefaultDescription()->setText("Left: Bilinear Filtering.\nRight: Cubic Filtering.");
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	// create demo graphics pipeline
	createPipeline();

	glm::vec3 clearColorLinearSpace(0.0f, 0.45f, 0.41f);
	_clearColor = clearColorLinearSpace;
	if (getBackBufferColorspace() != pvr::ColorSpace::sRGB)
	{
		// Gamma correct the clear colour
		_clearColor = pvr::utils::convertLRGBtoSRGB(clearColorLinearSpace);
	}

	// Is the screen rotated
	const bool bRotate = this->isScreenRotated();

	//  Calculate the projection and rotate it by 90 degree if the screen is rotated.
	_projection = (bRotate
			? pvr::math::perspectiveFov(pvr::Api::Vulkan, 45.0f, static_cast<float>(this->getHeight()), static_cast<float>(this->getWidth()), 0.01f, 100.0f, glm::pi<float>() * .5f)
			: pvr::math::perspectiveFov(pvr::Api::Vulkan, 45.0f, static_cast<float>(this->getWidth()), static_cast<float>(this->getHeight()), 0.01f, 100.0f));

	// Set up the view and _projection matrices from the camera
	glm::mat4 mView;
	// We can build the model view matrix from the camera position, target and an up vector.
	// For this we use glm::lookAt()
	mView = glm::lookAt(glm::vec3(0.0, 0.1, 1.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	_viewProjection = _projection * mView;

	// record the rendering commands
	recordCommandBuffers();

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanIMGTextureFilterCubic::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred</returns>
pvr::Result VulkanIMGTextureFilterCubic::renderFrame()
{
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameResourcesFences[swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[swapchainIndex]->reset();

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
		pvr::utils::takeScreenshot(_deviceResources->queue, _deviceResources->cmdPool, _deviceResources->swapchain, swapchainIndex, this->getScreenshotFileName(),
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

void VulkanIMGTextureFilterCubic::loadVbo()
{
	{
		_vertices.reserve(6);

		_vertices.push_back(glm::vec3(-10.0f, 10.0f, 0.0f));
		_vertices.push_back(glm::vec3(-10.0f, -10.0f, 0.0f));
		_vertices.push_back(glm::vec3(10.0f, 10.0f, 0.0f));

		_vertices.push_back(glm::vec3(10.0f, 10.0f, 0.0f));
		_vertices.push_back(glm::vec3(-10.0f, -10.0f, 0.0f));
		_vertices.push_back(glm::vec3(10.0f, -10.0f, 0.0f));
	}

	_deviceResources->quadVbo = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(pvr::getSize(pvr::GpuDatatypes::vec3) * 6, pvrvk::BufferUsageFlags::e_VERTEX_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator);

	bool isBufferHostVisible = (_deviceResources->quadVbo->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT) != 0;

	if (isBufferHostVisible)
	{ pvr::utils::updateHostVisibleBuffer(_deviceResources->quadVbo, static_cast<const void*>(_vertices.data()), 0, pvr::getSize(pvr::GpuDatatypes::vec3) * 6, true); }
	else
	{
		pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->quadVbo, pvrvk::CommandBufferBase(_deviceResources->uploadCmdBuffer),
			static_cast<const void*>(_vertices.data()), 0, pvr::getSize(pvr::GpuDatatypes::vec3) * 6, _deviceResources->vmaAllocator);
	}
}

void VulkanIMGTextureFilterCubic::createTextures()
{
	Log(LogLevel::Information, "Generating the Image to be sampled from using pvrvk::Filter::e_CUBIC_IMG and pvrvk::Filter::e_LINEAR.");

	// Determine the number of mipmap levels - see the Vulkan specification section "Image Mip level Sizing"
	uint32_t numMipLevels = static_cast<uint32_t>(floor(log2(std::max(getWidth(), getHeight()))) + 1);

	// Ensure images have pvrvk::ImageUsageFlags including pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT
	pvrvk::ImageUsageFlags imageUsage = pvrvk::ImageUsageFlags::e_SAMPLED_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT;
	pvrvk::Extent3D extent = pvrvk::Extent3D(static_cast<uint32_t>(getWidth()), static_cast<uint32_t>(getHeight()), 1);

	// Our use of vkBlitImage to generate the mip-chain requires support for pvrvk::FormatFeatureFlags::e_BLIT_SRC_BIT | pvrvk::FormatFeatureFlags::e_BLIT_DST_BIT
	// We have chosen to use an image with format pvrvk::Format::e_R8G8B8A8_UNORM which implementations must provide support for pvrvk::FormatFeatureFlags::e_BLIT_SRC_BIT | pvrvk::FormatFeatureFlags::e_BLIT_DST_BIT
	pvrvk::Image linearImage =
		pvr::utils::createImage(_deviceResources->device, pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvrvk::Format::e_R8G8B8A8_UNORM, extent, imageUsage, numMipLevels, 1),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, _deviceResources->vmaAllocator);

	_deviceResources->baseImageView = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(linearImage));

	// Transition image from pvrvk::ImageLayout::e_UNDEFINED to pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL ready for data transfer
	pvr::utils::setImageLayout(
		_deviceResources->baseImageView->getImage(), pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, _deviceResources->uploadCmdBuffer);

	// Generate image data which will be used as the source for the mip maps levels
	std::vector<unsigned char> img(this->getWidth() * this->getHeight() * 4);

	uint32_t size = 4;
	uint32_t half = size / 2;

	for (uint32_t y = 0; y < this->getHeight(); ++y)
	{
		for (uint32_t x = 0; x < this->getWidth(); ++x)
		{
			if (x % size < half && y % size < half)
			{
				img[(x + y * this->getWidth()) * 4 + 0] = 255;
				img[(x + y * this->getWidth()) * 4 + 1] = 0;
				img[(x + y * this->getWidth()) * 4 + 2] = 0;
				img[(x + y * this->getWidth()) * 4 + 3] = 255;
			}
			else if (x % size >= half && y % size < half)
			{
				img[(x + y * this->getWidth()) * 4 + 0] = 255;
				img[(x + y * this->getWidth()) * 4 + 1] = 0;
				img[(x + y * this->getWidth()) * 4 + 2] = 127;
				img[(x + y * this->getWidth()) * 4 + 3] = 255;
			}
			else if (x % size < half && y % size >= half)
			{
				img[(x + y * this->getWidth()) * 4 + 0] = 0;
				img[(x + y * this->getWidth()) * 4 + 1] = 0;
				img[(x + y * this->getWidth()) * 4 + 2] = 255;
				img[(x + y * this->getWidth()) * 4 + 3] = 255;
			}
			else
			{
				img[(x + y * this->getWidth()) * 4 + 0] = 0;
				img[(x + y * this->getWidth()) * 4 + 1] = 255;
				img[(x + y * this->getWidth()) * 4 + 2] = 0;
				img[(x + y * this->getWidth()) * 4 + 3] = 255;
			}
		}
	}

	// Upload the generated image data to the top mip level
	pvr::utils::ImageUpdateInfo updateInfo;
	updateInfo.imageWidth = getWidth();
	updateInfo.imageHeight = getHeight();
	updateInfo.dataWidth = getWidth();
	updateInfo.dataHeight = getHeight();
	updateInfo.depth = 1;
	updateInfo.arrayIndex = 0;
	updateInfo.cubeFace = 0;
	updateInfo.mipLevel = 0;
	updateInfo.data = img.data();
	updateInfo.dataSize = static_cast<uint32_t>(img.size());

	pvr::utils::updateImage(_deviceResources->device, _deviceResources->uploadCmdBuffer, &updateInfo, 1, pvrvk::Format::e_R8G8B8A8_UNORM,
		pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, false, _deviceResources->baseImageView->getImage(), _deviceResources->vmaAllocator);

	// Transition image from pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL to pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL ready for data transfer
	pvr::utils::setImageLayout(
		_deviceResources->baseImageView->getImage(), pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, _deviceResources->uploadCmdBuffer);

	// Generate the mip chain using vkCmdBlitImage
	Log(LogLevel::Information, "\tGenerating %u mipmap levels for Image to be sampled from.", numMipLevels);

	// Generate mip map levels all the way down to the lowest mip map level
	for (uint32_t i = 1; i < numMipLevels; i++)
	{
		// Transition current mip level pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL to pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL ready for data transfer
		pvr::utils::setImageLayoutAndQueueFamilyOwnership(_deviceResources->uploadCmdBuffer, pvrvk::CommandBufferBase(), static_cast<uint32_t>(-1), static_cast<uint32_t>(-1),
			pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, _deviceResources->baseImageView->getImage(), i, 1, 0,
			static_cast<uint32_t>(_deviceResources->baseImageView->getImage()->getNumArrayLayers()), pvrvk::ImageAspectFlags::e_COLOR_BIT);

		// Generate the current mip level for the linear image
		{
			// Setup the blit region
			pvrvk::Offset3D sourceOffsets[2];
			sourceOffsets[0] = pvrvk::Offset3D(0, 0, 0);
			int32_t sourceMipWidth = _deviceResources->baseImageView->getImage()->getExtent().getWidth() >> (i - 1);
			int32_t sourceMipHeight = _deviceResources->baseImageView->getImage()->getExtent().getHeight() >> (i - 1);
			sourceOffsets[1] = pvrvk::Offset3D(sourceMipWidth, sourceMipHeight, 1);
			pvrvk::Offset3D destinationOffsets[2];
			destinationOffsets[0] = pvrvk::Offset3D(0, 0, 0);
			int32_t destinationMipWidth = static_cast<int32_t>(std::max(1u, _deviceResources->baseImageView->getImage()->getExtent().getWidth() >> i));
			int32_t destinationMipHeight = static_cast<int32_t>(std::max(1u, _deviceResources->baseImageView->getImage()->getExtent().getHeight() >> i));
			destinationOffsets[1] = pvrvk::Offset3D(destinationMipWidth, destinationMipHeight, 1);
			pvrvk::ImageBlit blitRegion = pvrvk::ImageBlit(pvrvk::ImageSubresourceLayers(pvrvk::ImageAspectFlags::e_COLOR_BIT, i - 1, 0, 1), &sourceOffsets[0],
				pvrvk::ImageSubresourceLayers(pvrvk::ImageAspectFlags::e_COLOR_BIT, i, 0, 1), &destinationOffsets[0]);

			// Perform the blit using a linear filter
			_deviceResources->uploadCmdBuffer->blitImage(_deviceResources->baseImageView->getImage(), _deviceResources->baseImageView->getImage(), &blitRegion, 1,
				pvrvk::Filter::e_LINEAR, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL);
		}

		// Transition current mip level pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL to pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL ready to be used as a source for the next transfer
		pvr::utils::setImageLayoutAndQueueFamilyOwnership(_deviceResources->uploadCmdBuffer, pvrvk::CommandBufferBase(), static_cast<uint32_t>(-1), static_cast<uint32_t>(-1),
			pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, _deviceResources->baseImageView->getImage(), i, 1, 0,
			static_cast<uint32_t>(_deviceResources->baseImageView->getImage()->getNumArrayLayers()), pvrvk::ImageAspectFlags::e_COLOR_BIT);
	}

	// All of the images are now in pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL

	// Transition images from pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL to pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL ready for sampling
	pvr::utils::setImageLayout(
		_deviceResources->baseImageView->getImage(), pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, _deviceResources->uploadCmdBuffer);
}

/// <summary>Pre-record the commands.</summary>
void VulkanIMGTextureFilterCubic::recordCommandBuffers()
{
	pvrvk::ClearValue clearValue = pvrvk::ClearValue(_clearColor.x, _clearColor.y, _clearColor.z, 1.0f);
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		// begin recording commands
		_deviceResources->cmdBuffers[i]->begin();

		// begin the renderpass
		_deviceResources->cmdBuffers[i]->beginRenderPass(_deviceResources->onScreenFramebuffer[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, &clearValue, 1);

		// bind the VBO for the mesh
		_deviceResources->cmdBuffers[i]->bindVertexBuffer(_deviceResources->quadVbo, 0, 0);

		_deviceResources->cmdBuffers[i]->bindPipeline(_deviceResources->pipeline);

		_modelViewProjection = _viewProjection * glm::rotate(glm::radians(-90.f), glm::vec3(1.0f, 0.0f, 0.f));

		float pushConstantWidth = (float)getWidth();
		_deviceResources->cmdBuffers[i]->pushConstants(_deviceResources->pipeline->getPipelineLayout(),
			pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::mat4x4)), &_modelViewProjection);
		_deviceResources->cmdBuffers[i]->pushConstants(_deviceResources->pipeline->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT,
			static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::mat4x4)), static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Float)), &pushConstantWidth);

		// Bind the descriptor set which contains the base texture bound with linear and cubic samplers
		_deviceResources->cmdBuffers[i]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->pipelineLayout, 0u, _deviceResources->textureDescriptorSet);
		_deviceResources->cmdBuffers[i]->draw(0, 6);

		// add ui effects using ui renderer
		_deviceResources->uiRenderer.beginRendering(_deviceResources->cmdBuffers[i]);
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getDefaultDescription()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.endRendering();
		_deviceResources->cmdBuffers[i]->endRenderPass();
		_deviceResources->cmdBuffers[i]->end();
	}
}

/// <summary>Creates the descriptor set layouts used throughout the demo.</summary>
void VulkanIMGTextureFilterCubic::createDescriptorSetLayout()
{
	// create the texture descriptor set layout and pipeline layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		_deviceResources->texDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
	}

	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(_deviceResources->texDescriptorSetLayout);
	pipeLayoutInfo.setPushConstantRange(0,
		pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, 0,
			static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::mat4x4)) + static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Float))));
	_deviceResources->pipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
}

/// <summary>Creates the graphics pipeline used in the demo.</summary>
void VulkanIMGTextureFilterCubic::createPipeline()
{
	pvrvk::GraphicsPipelineCreateInfo pipelineInfo;

	pipelineInfo.viewport.setViewportAndScissor(
		0, pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(getWidth()), static_cast<float>(getHeight())), pvrvk::Rect2D(0, 0, getWidth(), getHeight()));

	pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);
	pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);
	pipelineInfo.depthStencil.enableDepthWrite(false);
	pipelineInfo.depthStencil.enableDepthTest(false);
	pipelineInfo.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS_OR_EQUAL);
	pipelineInfo.depthStencil.enableStencilTest(false);
	pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

	pipelineInfo.vertexShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(this->getAssetStream(VertShaderFileName)->readToEnd<uint32_t>())));
	pipelineInfo.fragmentShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(this->getAssetStream(FragShaderFileName)->readToEnd<uint32_t>())));

	pipelineInfo.vertexInput.clear();
	pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);

	pvrvk::PipelineVertexInputStateCreateInfo pipelineVertexInputCreateInfo;

	{
		const pvrvk::VertexInputAttributeDescription attribDesc(0, 0, pvrvk::Format::e_R32G32B32A32_SFLOAT, 0);
		const pvrvk::VertexInputBindingDescription bindingDesc(0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec3)), pvrvk::VertexInputRate::e_VERTEX);
		pipelineVertexInputCreateInfo.addInputAttribute(attribDesc).addInputBinding(bindingDesc);

		pipelineInfo.vertexInput = pipelineVertexInputCreateInfo;
	}

	pipelineInfo.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
	pipelineInfo.pipelineLayout = _deviceResources->pipelineLayout;

	_deviceResources->pipeline = _deviceResources->device->createGraphicsPipeline(pipelineInfo, _deviceResources->pipelineCache);
}

/// <summary>Create combined texture and sampler descriptor set for the materials in the _scene</summary>
/// <return>Return true on success.</return>
void VulkanIMGTextureFilterCubic::createDescriptorSet()
{
	// create the sampler objects
	{
		pvrvk::SamplerCreateInfo linearSamplerInfo;
		linearSamplerInfo.minFilter = pvrvk::Filter::e_LINEAR;
		linearSamplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
		linearSamplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
		linearSamplerInfo.wrapModeU = linearSamplerInfo.wrapModeV = pvrvk::SamplerAddressMode::e_REPEAT;
		_deviceResources->linearSampler = _deviceResources->device->createSampler(linearSamplerInfo);
	}

	// Create the sampler object which uses pvrvk::Filter::e_CUBIC_IMG filtering mode through the use of the PowerVR extension VK_IMG_filter_cubic
	{
		pvrvk::SamplerCreateInfo cubicSamplerInfo;
		cubicSamplerInfo.minFilter = pvrvk::Filter::e_LINEAR;
		cubicSamplerInfo.magFilter = pvrvk::Filter::e_CUBIC_IMG;
		cubicSamplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
		cubicSamplerInfo.wrapModeU = cubicSamplerInfo.wrapModeV = pvrvk::SamplerAddressMode::e_REPEAT;
		_deviceResources->cubicSampler = _deviceResources->device->createSampler(cubicSamplerInfo);
	}

	_deviceResources->textureDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->texDescriptorSetLayout);

	// Add the Linear and Cubic samplers along with the Image to the descriptor set to be bound
	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;
	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->textureDescriptorSet, 0));
	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->textureDescriptorSet, 1));
	writeDescSets[0].setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->baseImageView, _deviceResources->linearSampler));
	writeDescSets[1].setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->baseImageView, _deviceResources->cubicSampler));

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanIMGTextureFilterCubic>(); }
