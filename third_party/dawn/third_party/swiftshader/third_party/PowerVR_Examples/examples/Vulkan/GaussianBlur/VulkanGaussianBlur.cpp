/*!
\brief Shows how to perform a separated Gaussian Blur using a Compute shader and Fragment shader for carrying
	   out the horizontal and vertical passes respectively.
\file GaussianBlur.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PVRShell/PVRShell.h"
#include "PVRVk/ApiObjectsVk.h"
#include "PVRUtils/PVRUtilsVk.h"

// Source and binary shaders
const char FragShaderSrcFile[] = "FragShader.fsh.spv";
const char VertShaderSrcFile[] = "VertShader.vsh.spv";
const char CompShaderSrcFile[] = "CompShader.csh.spv";

// PVR texture files
const char StatueTexFile[] = "Lenna.pvr";

const uint32_t GaussianKernelSize = 19;

/// <summary>Prints the Gaussian weights and offsets provided in the vectors.</summary>
/// <param name="gaussianOffsets">The list of Gaussian offsets to print.</param>
/// <param name="gaussianWeights">The list of Gaussian weights to print.</param>
void printGaussianWeightsAndOffsets(std::vector<double>& gaussianOffsets, std::vector<double>& gaussianWeights)
{
	Log(LogLevel::Information, "Number of Gaussian Weights and Offsets = %u;", gaussianWeights.size());

	Log(LogLevel::Information, "Weights =");
	Log(LogLevel::Information, "{");
	for (uint32_t i = 0; i < gaussianWeights.size(); i++) { Log(LogLevel::Information, "%.15f,", gaussianWeights[i]); }
	Log(LogLevel::Information, "};");

	Log(LogLevel::Information, "Offsets =");
	Log(LogLevel::Information, "{");
	for (uint32_t i = 0; i < gaussianOffsets.size(); i++) { Log(LogLevel::Information, "%.15f,", gaussianOffsets[i]); }
	Log(LogLevel::Information, "};");
}

struct DeviceResources
{
	pvrvk::Instance instance;
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
	pvrvk::Device device;
	pvrvk::Queue queues[2];
	pvr::utils::vma::Allocator vmaAllocator;
	pvrvk::Swapchain swapchain;

	pvrvk::DescriptorPool descriptorPool;
	pvrvk::CommandPool commandPool;

	pvrvk::Buffer graphicsGaussianConfigBuffer;

	pvrvk::Semaphore imageAcquiredSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore presentationSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameResourcesFences[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;
	pvr::Multi<pvrvk::CommandBuffer> mainCommandBuffers;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> uiRendererCommandBuffers;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> graphicsCommandBuffers;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> computeCommandBuffers;
	pvr::Multi<pvrvk::ImageView> horizontallyBlurredImageViews;

	// Compute based Horizontal Gaussian Blur pass
	pvr::Multi<pvrvk::DescriptorSet> computeDescriptorSets;

	// Compute based Horizontal Gaussian Blur pass
	pvr::Multi<pvrvk::DescriptorSet> graphicsDescriptorSets;

	// Descriptor set layouts
	pvrvk::DescriptorSetLayout computeDescriptorSetLayout;
	pvrvk::DescriptorSetLayout graphicsDescriptorSetLayout;

	pvrvk::ImageView inputImageView;

	pvrvk::GraphicsPipeline graphicsPipeline;
	pvrvk::ComputePipeline computePipeline;

	pvrvk::PipelineLayout computePipelinelayout;
	pvrvk::PipelineLayout graphicsPipelinelayout;

	pvrvk::Sampler nearestSampler;
	pvrvk::Sampler bilinearSampler;

	// UIRenderer used to display text
	pvr::ui::UIRenderer uiRenderer;

	pvrvk::PipelineCache pipelineCache;

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

/// <summary>implementing the Shell functions.</summary>
class VulkanGaussianBlur : public pvr::Shell
{
private:
	std::unique_ptr<DeviceResources> _deviceResources;
	uint32_t _frameId;
	uint32_t _queueIndex;

	// Linear Optimised Gaussian offsets and weights
	std::vector<double> _linearGaussianOffsets;
	std::vector<double> _linearGaussianWeights;

	// Gaussian offsets and weights
	std::vector<double> _gaussianOffsets;
	std::vector<double> _gaussianWeights;

	uint32_t _graphicsSsboSize;
	bool _useMultiQueue;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void initialiseGaussianWeightsAndOffsets();
	void loadTextures(pvrvk::CommandBuffer& cmdBuffers);
	void createResources();
	void createPipelines();
	void updateResources();
	void recordCommandBuffer();
};

/// <summary>Loads the textures used throughout the demo. The commands required for uploading image data into the
/// texture are recorded into the provided command buffer.</summary>
/// <param name="cmdBuffers">The commands required for uploading image data into the texture are recorded into this command buffer.</param>
void VulkanGaussianBlur::loadTextures(pvrvk::CommandBuffer& cmdBuffers)
{
	// Load the Texture PVR file from the disk
	pvr::Texture texture = pvr::textureLoad(*getAssetStream(StatueTexFile), pvr::TextureFileFormat::PVR);

	pvr::ImageDataFormat imageformat;
	imageformat.colorSpace = texture.getColorSpace();
	imageformat.format = texture.getPixelFormat();

	// Create and Allocate Textures.
	_deviceResources->inputImageView =
		pvr::utils::uploadImageAndView(_deviceResources->device, texture, true, cmdBuffers, pvrvk::ImageUsageFlags::e_SAMPLED_BIT | pvrvk::ImageUsageFlags::e_STORAGE_BIT,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);

	// Create 1 intermediate image per frame.
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); i++)
	{
		pvrvk::Image intermediateTexture = pvr::utils::createImage(_deviceResources->device,
			pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvr::utils::convertToPVRVkPixelFormat(texture.getPixelFormat(), texture.getColorSpace(), texture.getChannelType()),
				pvrvk::Extent3D(texture.getWidth(), texture.getHeight(), 1u), pvrvk::ImageUsageFlags::e_STORAGE_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator,
			pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);

		// transfer the layout from UNDEFINED to SHADER_READ_ONLY_OPTIMAL
		pvr::utils::setImageLayout(intermediateTexture, pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, cmdBuffers);
		_deviceResources->horizontallyBlurredImageViews[i] = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(intermediateTexture));
	}
}

/// <summary>Code in  createResources() loads the compute, fragment and vertex shaders and associated buffers used by them.</ summary>
void VulkanGaussianBlur::createResources()
{
	// Create the compute descriptor set layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descriptorSetLayoutParams;
		descriptorSetLayoutParams.setBinding(0, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		descriptorSetLayoutParams.setBinding(1, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);

		_deviceResources->computeDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(descriptorSetLayoutParams);
	}

	// Create the Compute Pipeline layout
	{
		pvrvk::PipelineLayoutCreateInfo createInfo;
		createInfo.addDescSetLayout(_deviceResources->computeDescriptorSetLayout);
		_deviceResources->computePipelinelayout = _deviceResources->device->createPipelineLayout(createInfo);
	}

	// Create the graphics descriptor set layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descriptorSetLayoutParams;
		descriptorSetLayoutParams.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descriptorSetLayoutParams.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descriptorSetLayoutParams.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		_deviceResources->graphicsDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(descriptorSetLayoutParams);
	}

	// Create the Graphics Pipeline layout
	{
		pvrvk::PipelineLayoutCreateInfo createInfo;
		createInfo.addDescSetLayout(_deviceResources->graphicsDescriptorSetLayout);
		_deviceResources->graphicsPipelinelayout = _deviceResources->device->createPipelineLayout(createInfo);
	}

	// Create the samplers
	{
		pvrvk::SamplerCreateInfo samplerInfo;
		samplerInfo.wrapModeU = samplerInfo.wrapModeV = samplerInfo.wrapModeW = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;
		samplerInfo.magFilter = pvrvk::Filter::e_NEAREST;
		samplerInfo.minFilter = pvrvk::Filter::e_NEAREST;
		samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;

		_deviceResources->nearestSampler = _deviceResources->device->createSampler(samplerInfo);

		samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
		samplerInfo.minFilter = pvrvk::Filter::e_LINEAR;
		_deviceResources->bilinearSampler = _deviceResources->device->createSampler(samplerInfo);
	}

	// Create the buffer used in the vertical fragment pass
	{
		_graphicsSsboSize = static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2));

		_deviceResources->graphicsGaussianConfigBuffer =
			pvr::utils::createBuffer(_deviceResources->device, pvrvk::BufferCreateInfo(_graphicsSsboSize, pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
				pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
				_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	}

	{
		// Update the descriptor sets
		std::vector<pvrvk::WriteDescriptorSet> writeDescSets;
		for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
		{
			// Compute descriptor sets
			{
				_deviceResources->computeDescriptorSets[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->computeDescriptorSetLayout);

				writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, _deviceResources->computeDescriptorSets[i], 0)
											.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->inputImageView, pvrvk::ImageLayout::e_GENERAL)));

				writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, _deviceResources->computeDescriptorSets[i], 1)
											.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->horizontallyBlurredImageViews[i], pvrvk::ImageLayout::e_GENERAL)));
			}

			// Graphics descriptor sets
			{
				_deviceResources->graphicsDescriptorSets[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->graphicsDescriptorSetLayout);
				writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->graphicsDescriptorSets[i], 0)
											.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->graphicsGaussianConfigBuffer, 0, _graphicsSsboSize)));

				writeDescSets.push_back(
					pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->graphicsDescriptorSets[i], 1)
						.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->inputImageView, _deviceResources->nearestSampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

				writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->graphicsDescriptorSets[i], 2)
											.setImageInfo(0,
												pvrvk::DescriptorImageInfo(_deviceResources->horizontallyBlurredImageViews[i], _deviceResources->bilinearSampler,
													pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
			}
		}
		_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
	}
}

/// <summary>Updates the buffers used by the compute and graphics passes for controlling the Gaussian Blurs.</summary>
void VulkanGaussianBlur::updateResources()
{
	// Update the Gaussian configuration buffer used for the graphics based vertical pass
	{
		uint32_t windowWidth = this->getWidth();
		float inverseImageHeight = 1.0f / _deviceResources->inputImageView->getCreateInfo().getImage()->getHeight();
		glm::vec2 config = glm::vec2(windowWidth, inverseImageHeight);

		memcpy(static_cast<char*>(_deviceResources->graphicsGaussianConfigBuffer->getDeviceMemory()->getMappedData()), &config,
			static_cast<size_t>(pvr::getSize(pvr::GpuDatatypes::vec2)));

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(_deviceResources->graphicsGaussianConfigBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{ _deviceResources->graphicsGaussianConfigBuffer->getDeviceMemory()->flushRange(0, _graphicsSsboSize); }
	}
}

/// <summary>Loads and compiles the shaders and create the pipelines.</summary>
void VulkanGaussianBlur::createPipelines()
{
	// Load the shaders from their files
	pvrvk::ShaderModule computeShader = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(CompShaderSrcFile)->readToEnd<uint32_t>()));
	pvrvk::ShaderModule vertexShader = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(VertShaderSrcFile)->readToEnd<uint32_t>()));
	pvrvk::ShaderModule fragShader = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(FragShaderSrcFile)->readToEnd<uint32_t>()));

	// Create the compute pipeline
	{
		pvrvk::ComputePipelineCreateInfo createInfo;
		createInfo.computeShader.setShader(computeShader);
		createInfo.pipelineLayout = _deviceResources->computePipelinelayout;
		_deviceResources->computePipeline = _deviceResources->device->createComputePipeline(createInfo, _deviceResources->pipelineCache);
	}

	// Create the graphics pipeline
	{
		pvrvk::GraphicsPipelineCreateInfo createInfo;

		const pvrvk::Rect2D rect(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight());
		createInfo.viewport.setViewportAndScissor(0,
			pvrvk::Viewport(static_cast<float>(rect.getOffset().getX()), static_cast<float>(rect.getOffset().getY()), static_cast<float>(rect.getExtent().getWidth()),
				static_cast<float>(rect.getExtent().getHeight())),
			rect);

		pvrvk::PipelineColorBlendAttachmentState colorAttachmentState;
		colorAttachmentState.setBlendEnable(false);
		createInfo.vertexShader.setShader(vertexShader);
		createInfo.fragmentShader.setShader(fragShader);

		// enable back face culling
		createInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);

		// set counter clockwise winding order for front faces
		createInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

		// setup vertex inputs
		createInfo.vertexInput.clear();
		createInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);

		createInfo.colorBlend.setAttachmentState(0, colorAttachmentState);
		createInfo.pipelineLayout = _deviceResources->graphicsPipelinelayout;
		createInfo.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
		createInfo.subpass = 0;

		_deviceResources->graphicsPipeline = _deviceResources->device->createGraphicsPipeline(createInfo, _deviceResources->pipelineCache);
	}
}

/// <summary>Pre record the commands.</summary>
void VulkanGaussianBlur::recordCommandBuffer()
{
	const pvrvk::ClearValue clearValue[] = { pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.0f) };

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		// UI Renderer
		_deviceResources->uiRendererCommandBuffers[i]->begin(_deviceResources->onScreenFramebuffer[i], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
		_deviceResources->uiRenderer.beginRendering(_deviceResources->uiRendererCommandBuffers[i]);
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getDefaultDescription()->render();
		_deviceResources->uiRenderer.endRendering();
		_deviceResources->uiRendererCommandBuffers[i]->end();

		// Compute Command Buffer
		{
			_deviceResources->computeCommandBuffers[i]->begin();
			pvr::utils::beginCommandBufferDebugLabel(_deviceResources->computeCommandBuffers[i], pvrvk::DebugUtilsLabel("Compute Blur Horizontal"));
			{
				pvrvk::MemoryBarrierSet barrierSet;

				// Set up a barrier to transition the image layouts from e_SHADER_READ_ONLY_OPTIMAL to e_GENERAL
				barrierSet.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::AccessFlags::e_SHADER_WRITE_BIT,
					_deviceResources->horizontallyBlurredImageViews[i]->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT),
					pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::ImageLayout::e_GENERAL, _deviceResources->queues[0]->getFamilyIndex(),
					_deviceResources->queues[0]->getFamilyIndex()));

				barrierSet.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::AccessFlags::e_SHADER_WRITE_BIT,
					_deviceResources->inputImageView->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL,
					pvrvk::ImageLayout::e_GENERAL, _deviceResources->queues[0]->getFamilyIndex(), _deviceResources->queues[0]->getFamilyIndex()));

				_deviceResources->computeCommandBuffers[i]->pipelineBarrier(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, barrierSet);
			}

			// Bind the compute pipeline & the descriptor set.
			_deviceResources->computeCommandBuffers[i]->bindPipeline(_deviceResources->computePipeline);
			_deviceResources->computeCommandBuffers[i]->bindDescriptorSet(
				pvrvk::PipelineBindPoint::e_COMPUTE, _deviceResources->computePipelinelayout, 0, _deviceResources->computeDescriptorSets[i]);

			// dispatch x = image.height / 32
			// dispatch y = 1
			// dispatch z = 1
			_deviceResources->computeCommandBuffers[i]->dispatch(getHeight() / 32, 1, 1);

			{
				pvrvk::MemoryBarrierSet barrierSet;

				// Set up a barrier to pass the image from our compute shader to fragment shader.
				barrierSet.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
					_deviceResources->horizontallyBlurredImageViews[i]->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_GENERAL,
					pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, _deviceResources->queues[0]->getFamilyIndex(), _deviceResources->queues[0]->getFamilyIndex()));

				barrierSet.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
					_deviceResources->inputImageView->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_GENERAL,
					pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, _deviceResources->queues[0]->getFamilyIndex(), _deviceResources->queues[0]->getFamilyIndex()));

				_deviceResources->computeCommandBuffers[i]->pipelineBarrier(pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, barrierSet);
			}

			pvr::utils::endCommandBufferDebugLabel(_deviceResources->computeCommandBuffers[i]);
			_deviceResources->computeCommandBuffers[i]->end();
		}

		// Graphics Command Buffer
		{
			_deviceResources->graphicsCommandBuffers[i]->begin(_deviceResources->onScreenFramebuffer[i], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
			pvr::utils::beginCommandBufferDebugLabel(_deviceResources->graphicsCommandBuffers[i], pvrvk::DebugUtilsLabel("Linear Gaussian Blur (vertical)"));
			_deviceResources->graphicsCommandBuffers[i]->bindPipeline(_deviceResources->graphicsPipeline);
			_deviceResources->graphicsCommandBuffers[i]->bindDescriptorSet(
				pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->graphicsPipelinelayout, 0, _deviceResources->graphicsDescriptorSets[i]);
			_deviceResources->graphicsCommandBuffers[i]->draw(0, 3);
			pvr::utils::endCommandBufferDebugLabel(_deviceResources->graphicsCommandBuffers[i]);
			_deviceResources->graphicsCommandBuffers[i]->end();
		}

		// Begin recording to the command buffer
		_deviceResources->mainCommandBuffers[i]->begin();
		_deviceResources->mainCommandBuffers[i]->executeCommands(_deviceResources->computeCommandBuffers[i]);
		_deviceResources->mainCommandBuffers[i]->beginRenderPass(
			_deviceResources->onScreenFramebuffer[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), false, clearValue, ARRAY_SIZE(clearValue));
		_deviceResources->mainCommandBuffers[i]->executeCommands(_deviceResources->graphicsCommandBuffers[i]);
		// enqueue the command buffer containing ui renderer commands
		_deviceResources->mainCommandBuffers[i]->executeCommands(_deviceResources->uiRendererCommandBuffers[i]);
		// End RenderPass and recording.
		_deviceResources->mainCommandBuffers[i]->endRenderPass();
		_deviceResources->mainCommandBuffers[i]->end();
	}
}

/// <summary>Initialises the Gaussian weights and offsets used in the compute shader and vertex/fragment shader carrying out the
/// horizontal and vertical Gaussian blur passes respectively.</summary>
void VulkanGaussianBlur::initialiseGaussianWeightsAndOffsets()
{
	// Generate a full set of Gaussian weights and offsets to be used in our compute shader
	{
		pvr::math::generateGaussianKernelWeightsAndOffsets(GaussianKernelSize, false, false, _gaussianWeights, _gaussianOffsets);

		Log(LogLevel::Information, "Gaussian Weights and Offsets:");
		printGaussianWeightsAndOffsets(_gaussianOffsets, _gaussianWeights);
	}

	// Generate a set of Gaussian weights and offsets optimised for linear sampling
	{
		pvr::math::generateGaussianKernelWeightsAndOffsets(GaussianKernelSize, false, true, _linearGaussianWeights, _linearGaussianOffsets);

		Log(LogLevel::Information, "Linear Sampling Optimized Gaussian Weights and Offsets:");
		printGaussianWeightsAndOffsets(_linearGaussianOffsets, _linearGaussianWeights);
	}
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.) If the rendering
/// context is lost, initApplication() will not be called again.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanGaussianBlur::initApplication()
{
	_frameId = 0;
	_queueIndex = 0;

	this->setDepthBitsPerPixel(0);
	this->setStencilBitsPerPixel(0);

	initialiseGaussianWeightsAndOffsets();

	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context(e.g.textures, vertex buffers, etc.)</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanGaussianBlur::initView()
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

	pvr::utils::QueuePopulateInfo queueCreateInfos[] = {
		{ pvrvk::QueueFlags::e_GRAPHICS_BIT | pvrvk::QueueFlags::e_COMPUTE_BIT, surface }, // Queue 0
		{ pvrvk::QueueFlags::e_GRAPHICS_BIT | pvrvk::QueueFlags::e_COMPUTE_BIT, surface } // Queue 1
	};
	pvr::utils::QueueAccessInfo queueAccessInfos[2];
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), queueCreateInfos, 2, queueAccessInfos);

	_deviceResources->queues[0] = _deviceResources->device->getQueue(queueAccessInfos[0].familyId, queueAccessInfos[0].queueId);

	// In the future we may want to improve our flexibility with regards to making use of multiple queues but for now to support multi queue the queue must support
	// Graphics + Compute + WSI support.
	// Other multi queue approaches may be possible i.e. making use of additional queues which do not support graphics/WSI
	_useMultiQueue = false;

	if (queueAccessInfos[1].familyId != -1 && queueAccessInfos[1].queueId != -1)
	{
		_deviceResources->queues[1] = _deviceResources->device->getQueue(queueAccessInfos[1].familyId, queueAccessInfos[1].queueId);

		if (_deviceResources->queues[0]->getFamilyIndex() == _deviceResources->queues[1]->getFamilyIndex())
		{
			_useMultiQueue = true;
			Log(LogLevel::Information, "Multiple queues support e_GRAPHICS_BIT + e_COMPUTE_BIT + WSI. These queues will be used to ping-pong work each frame");
		}
		else
		{
			Log(LogLevel::Information, "Queues are from a different Family. We cannot ping-pong work each frame");
		}
	}
	else
	{
		Log(LogLevel::Information, "Only a single queue supports e_GRAPHICS_BIT + e_COMPUTE_BIT + WSI. We cannot ping-pong work each frame");
	}

	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{ swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT; } // Create the swapchain

	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage).enableDepthBuffer(false));

	_deviceResources->swapchain = swapChainCreateOutput.swapchain;
	_deviceResources->onScreenFramebuffer = swapChainCreateOutput.framebuffer;

	_deviceResources->commandPool = _deviceResources->device->createCommandPool(
		pvrvk::CommandPoolCreateInfo(_deviceResources->queues[0]->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	_deviceResources->descriptorPool =
		_deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo(10).addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_IMAGE, 16));

	// Create per frame resource
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->mainCommandBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->uiRendererCommandBuffers[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->graphicsCommandBuffers[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->computeCommandBuffers[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();

		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
	}

	// Load the textures used by in the demo
	_deviceResources->mainCommandBuffers[0]->begin();
	loadTextures(_deviceResources->mainCommandBuffers[0]);
	_deviceResources->mainCommandBuffers[0]->end();

	// Submit the image upload command buffer
	pvrvk::SubmitInfo submit;
	submit.commandBuffers = &_deviceResources->mainCommandBuffers[0];
	submit.numCommandBuffers = 1;
	_deviceResources->queues[0]->submit(&submit, 1);
	_deviceResources->queues[0]->waitIdle();

	_deviceResources->mainCommandBuffers[0]->reset(pvrvk::CommandBufferResetFlags::e_RELEASE_RESOURCES_BIT);

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	createResources();
	createPipelines();

	updateResources();

	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queues[0]);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("GaussianBlur");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultDescription()->setText("Left: Original Texture\n"
																  "Right: Gaussian Blurred Texture");
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();

	recordCommandBuffer();

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanGaussianBlur::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by Shell once per run, just before exiting the program.
/// quitApplication() will not be called every time the rendering context is lost, only before application exit.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanGaussianBlur::quitApplication() { return pvr::Result::Success; }

/// <summary>Main rendering loop function of the program. The shell will call this function every frame</summary>
/// <returns>Result::Success if no error occurred.</summary>
pvr::Result VulkanGaussianBlur::renderFrame()
{
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameResourcesFences[swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[swapchainIndex]->reset();

	// Submit
	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags pipeWaitStageFlags = pvrvk::PipelineStageFlags::e_ALL_GRAPHICS_BIT | pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT;
	submitInfo.commandBuffers = &_deviceResources->mainCommandBuffers[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->imageAcquiredSemaphores[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.waitDstStageMask = &pipeWaitStageFlags;

	// Ping pong between multiple VkQueues
	// It's important to realise that in Vulkan, pipeline barriers only observe their barriers within the VkQueue they are submitted to.
	// This demo uses a Compute -> Fragment chain, which if left
	// unattended can cause compute/graphics pipeline bubbles meaning we can quite easily hit into per frame workload serialisation as shown below:
	// Compute Workload             |1----|      |2----|
	// Fragment Workload     |1----|       |2---|       |3---|

	// The Compute -> Fragment pipeline used after our Compute pipeline stage for synchronising between the pipeline stages has further, less obvious unintended consequences
	// in that when using only a single VkQueue this pipeline barrier enforces a barrier between all Compute work *before* the barrier and all Fragment work *after* the barrier.
	// This barrier means that even though we can see compute pipeline bubbles that could potentially be interleaved with Fragment work the barrier enforces against this behaviour.
	// This is where Vulkan really shines over OpenGL ES in terms of giving explicit control of work submission to the application.
	// We make use of two Vulkan VkQueue objects which are submitted to in a ping-ponged fashion. Each VkQueue only needs to observe barriers used in command buffers which
	// are submitted to them meaning there are no barriers enforced between the two sets of separate commands other than the presentation synchronisation logic.
	// This simple change allows us to observe the following workload scheduling:
	// Compute Workload              |1----||2----||3----|
	// Fragment Workload      |1----||2----||3----||4----|
	_deviceResources->queues[_queueIndex]->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->queues[_queueIndex], _deviceResources->commandPool, _deviceResources->swapchain, swapchainIndex, this->getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	// Present
	pvrvk::PresentInfo presentInfo;
	presentInfo.imageIndices = &swapchainIndex;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numWaitSemaphores = 1;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	presentInfo.numSwapchains = 1;
	// As above we must present using the same VkQueue as submitted to previously
	_deviceResources->queues[_queueIndex]->present(presentInfo);

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	if (_useMultiQueue) { _queueIndex = (_queueIndex + 1) % 2; }

	return pvr::Result::Success;
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanGaussianBlur>(); }
